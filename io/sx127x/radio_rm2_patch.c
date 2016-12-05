/* Copyright 2016 JP Norair
  *
  * Licensed under the OpenTag License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  * http://www.indigresso.com/wiki/doku.php?id=opentag:license_1_0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  */
/**
  * @file       /io/sx127x/radio_rm2_patch.c
  * @author     JP Norair
  * @version    R100
  * @date       24 Nov 2016
  * @brief      Patch functions used with SX127x
  * @ingroup    Radio
  *
  ******************************************************************************
  */

#include <otstd.h>
#if (OT_FEATURE(M2) == ENABLED)

// Local header for supporting this patch, or other patches
#include "radio_rm2.h"






/** Replacement Timing and Packet Duration functions <BR>
  * ========================================================================<BR>
  * rm2_rxtimeout_floor
  * rm2_default_tgd
  * rm2_pkt_duration
  * rm2_bgpkt_duration
  * rm2_scale_codec
  */

#define _SF 7
#define _CR 3

ot_ulong sub_symbols_to_miti(ot_uint symbols) {
/// default symbol is 256us = or 250miti
///@todo dynamic symbol durations
    ot_ulong miti;
    miti    = symbols;
    miti   *= 250;
    return miti;
}

ot_uint sub_symbols_to_ti(ot_uint symbols) {
    ot_ulong miti;
    miti    = sub_symbols_to_miti(symbols);
    miti   += 1023;
    return (ot_uint)(miti >> 10);
}



ot_uint rm2_rxtimeout_floor(ot_u8 chan_id) {
    return 7;
}

ot_uint rm2_default_tgd(ot_u8 chan_id) {
///@todo add multiple data rates in the future
    return 7;
}

ot_uint rm2_pkt_duration(ot_queue* pkt_q) {
    ot_uint pkt_bytes;
    ot_uint pkt_duration;

    pkt_bytes = q_length(pkt_q);
    if (pkt_q->front[1] & 0x40) {
        // If packet is using RS coding, adjust by the nominal rate (+25%).
        ///@todo expose the adaptive size amount function
        pkt_bytes += (pkt_bytes+3)>>2;
    }

    // Convert bytes to symbols (use same variable though)
    pkt_bytes   = (((8*pkt_bytes)-(4*_SF)+28)+(4*_SF-1)) / (4*_SF);
    pkt_bytes   = ((ot_int)pkt_bytes < 0) ? 8 : pkt_bytes+8;
    pkt_bytes  += RF_PARAM_PKT_OVERHEAD;
    
    return sub_symbols_to_ti(pkt_bytes);
}

ot_uint rm2_bgpkt_duration() {
    return sub_symbols_to_ti(RF_PARAM_BGPKT_SYMS);
}

ot_uint rm2_scale_codec(ot_uint buf_bytes) {
/// Turns a number of bytes (buf_bytes) into a number of ti units.
/// Generally unused on SX127x
    ot_int symbols;
    symbols = (((8*buf_bytes) - (4*_SF) + 8) + (4*_SF-1)) / (4*_SF);
    symbols = (symbols < 0) ? 8 : symbols+8;
    return sub_symbols_to_ti(symbols);
}

#undef _CR
#undef _SF





/** Replacement Encoder functions <BR>
  * ========================================================================<BR>
  * The Encoder/Decoder module in M2 module provides some generic encode and
  * decode functions.  They work for most radios.  The SX127x implementation
  * has these optimized encode and decode functions.
  *
  */

void em2_encode_newpacket() {
}


void em2_decode_newpacket() {
}


void em2_encode_newframe() {
/// LoRa doesn't use the CRC5 header (it has its own), nor does it utilize the
/// length byte at rxq.front[0].  txq.getcursor should start at txq.front[2]
    
    if (txq.options.ubyte[UPPER] != 0) {
        txq.getcursor   = &txq.front[2];
        crc_block(&em2.crc, True, q_span(&txq), txq.getcursor);
        
        txq.putcursor  += 2;
        txq.front[0]   += 2;
        txq.front[1]   |= 0x20;     // always set valid header bit
        
#       if (M2_FEATURE(RSCODE))
            em2.lctl = txq.front[1];
            if (em2.lctl & 0x40) {
                ot_int parity_bytes;
                parity_bytes    = em2_rs_init_encode(&txq);
                txq.front[0]   += parity_bytes;
                txq.putcursor  += parity_bytes;
            }
#       else
            em2.lctl        = txq.front[1] & ~0x60;
            txq.front[1]    = em2.lctl;
#       endif
    }
    
    em2.bytes = q_span(&txq);
}


void em2_decode_newframe() {
    rxq.putcursor   = &rxq.front[2];
    rxq.getcursor   = &rxq.front[2];
    em2.state       = 0;
}



void em2_encode_data(void) {
/// Fill FIFO using 24 byte SPI buffer
/// With LoRa, the CRC bytes are computed ahead of the encode stage
    ot_int fill;
    
#   if (M2_FEATURE(RSCODE))
    if ((txq.options.ubyte[UPPER] != 0) && (em2.lctl & 0x40)) {
        em2_rs_encode(em2.bytes);
    }
#   endif
    
    while (em2.bytes > 0) {
        fill        = (em2.bytes > 24) ? 24 : em2.bytes;
        em2.bytes  -= fill;
        sx127x_burstwrite(RFREG_LF_FIFO, fill, txq.getcursor);
    }
}


void em2_decode_data(void) {
/// With LoRa, decoding is all done at once, after packet is fully received
    ot_uint scratch;

    // Determine number of bytes that have been received, then retrieve them all
    scratch     = sx127x_read(RFREG_LR_RXNBBYTES);
    em2.bytes   = scratch;

    while (total > 0) {
        ot_u16 grab;
        ot_u8* data;
        grab            = (scratch > 24) ? 24 : scratch;
        scratch        -= grab;
        data            = rxq.putcursor;
        rxq.putcursor  += grab;
        sx127x_burstread(RFREG_LF_FIFO, grab, data);
    }

    // Do CRC decoding and optional RS decoding
    scratch = crc_block(&em2.crc, False, em2.bytes, rxq.getcursor);
    
    // Do Optional RS Decoding if CRC is failed
#   if (M2_FEATURE(RSCODE))
    if ((scratch != 0) && (em2.lctl & 0x40)) {
        em2_rs_decode(grab);
    }
#   endif
}




#endif  //if (OT_FEATURE(M2) == ENABLED)
