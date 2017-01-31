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
#include <m2/encode.h>





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

//ot_uint rm2_pkt_duration(ot_queue* pkt_q) { return 1024; }

ot_uint rm2_pkt_duration(ot_queue* pkt_q) {
    ot_uint pkt_bytes;

    pkt_bytes = q_length(pkt_q);
    if (pkt_q->front[1] & 0x40) {
        // If packet is using RS coding, adjust by the nominal rate (+25%).
        ///@todo expose the adaptive size amount function
        pkt_bytes += (pkt_bytes+3)>>2;
    }

    // Convert bytes to symbols (use same variable though)
    ///@todo adapt for variable SF and CR
    pkt_bytes   = ((((8*pkt_bytes)-(4*_SF)+28)+(4*_SF-1)) / (4*_SF)) * (4+3);   // +3 in (4+3) is coding rate
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
        // Initialize CRC16 engine
        txq.getcursor   = &txq.front[2];
        crc_init_stream(&em2.crc, True, q_span(&txq), txq.getcursor);
        
        // Add two bytes to account for CRC
        txq.putcursor  += 2;
        txq.front[0]   += 2;
        
        // Initialize RS Code engine, if applicable
#       if (M2_FEATURE(RSCODE))
            txq.front[1]   |= 0x20;                 // On LoRa, header-ok bit always set
            em2.lctl        = txq.front[1];
            if (em2.lctl & 0x40) {
                ot_int parity_bytes;
                parity_bytes    = em2_rs_init_encode(&txq);
                txq.front[0]   += parity_bytes;
                txq.putcursor  += parity_bytes;
            }
            
        // No RS, so lctl always set to 0x20
#       else
            em2.lctl        = 0x20;
            txq.front[1]    = 0x20;
#       endif
    }
    
    em2.bytes = q_span(&txq);
}


void em2_decode_newframe() {
    
    ///@todo conditionally also set RS bit (0x40) on rxq.front[1]
    rxq.front[1]    = 0x20;     // always set valid header bit
    em2.lctl        = 0x20;
    
    rxq.putcursor   = &rxq.front[2];
    rxq.getcursor   = &rxq.front[2];
    em2.state       = 0;
}




void em2_encode_data(void) {
/// LoRa encodes the entire frame/packet at once, because it does not handle streaming.

    ///1. CRC16 Calculation: roll back -2 for CRC placeholder
    crc_calc_nstream(&em2.crc, em2.crc.count);
    
    ///2. RS Code Calculation if applicable
#   if (M2_FEATURE(RSCODE))
    if ((txq.options.ubyte[UPPER] != 0) && (em2.lctl & 0x40)) {
        em2_rs_encode(em2.bytes);
    }
#   endif
    
    ///@todo fix spibus_io function to have an internal buffer and such a loop as below.
    while (em2.bytes > 0) {
        ot_int fill = (em2.bytes > 24) ? 24 : em2.bytes;
        em2.bytes  -= fill;
        sx127x_burstwrite(RFREG_LR_FIFO, fill, q_markbyte(&txq, fill));
    }
}


void em2_decode_data(void) {
/// With LoRa, decoding is all done at once, after packet is fully received.
    ot_uint scratch;
    
    // Determine number of bytes that have been received, then retrieve them all
    scratch     = sx127x_read(RFREG_LR_RXNBBYTES);
    em2.bytes   = scratch;

    while (scratch > 0) {
        ot_u16 grab;
        ot_u8* data;
        grab            = (scratch > 24) ? 24 : scratch;
        scratch        -= grab;
        data            = rxq.putcursor;
        rxq.putcursor  += grab;
        sx127x_burstread(RFREG_LR_FIFO, grab, data);
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

ot_u16 em2_decode_endframe() {
/// Perform block-code error correction if available, strip blockcoding if its
/// there (after processing), and strip CRC
    ot_u16 framebytes;
    ot_u16 crc_invalid;
    ot_int corrections;

    ///1. Get the CRC16 information, which is already computed inline.  There
    ///   may be no reason to do RS decoding, even if it is available
    crc_invalid = crc_get(&em2.crc);
    
    // Length Byte + lctl Byte = 2, although it's canceled by 2 byte CRC below
    framebytes  = em2.bytes; //+ 2;        

    ///2. Remove the RS parity bytes from "framebytes" if RS parity bytes are
    ///     present.  If RS decoding is supported, also do postprocess error
    ///     correction, and re-do the CRC afterwards if it reports no remaining
    ///     errors, to verify that indeed all errors were corrected.
    corrections = 0;
    if (em2.lctl & 0x40) {
        framebytes -= em2_rs_paritylength( framebytes );
#       if (M2_FEATURE(RSCODE))
        if (crc_invalid) {
            corrections = em2_rs_postprocess();
            if (corrections >= 0) {
                crc_invalid = crc16drv_block(rxq.getcursor, framebytes);
                corrections = crc_invalid ? -1 : corrections;
            }
        }
#       endif
    }
#   if (OT_FEATURE(RF_LINKINFO))
        radio.link.corrections = corrections;
#   endif

    ///3. Strip the CRC16 bytes from "framebytes" (cancelled-out from above)
    //framebytes -= 2;

    ///4. Now the Frame Length byte is completely stripped of block data
    ///   (RS and CRC are actually both types of block codes)
    rxq.front[0] = (ot_u8)framebytes;

    ///5. If CRC is still invalid, report the packet is uncorrectably broken
    return crc_invalid;
}





#endif  //if (OT_FEATURE(M2) == ENABLED)
