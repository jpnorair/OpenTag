/* Copyright 2010-2014 JP Norair
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
  * @file       /otradio/spirit1/radio_SPIRIT1.c
  * @author     JP Norair
  * @version    R100
  * @date       14 April 2014
  * @brief      Patch functions used with SPIRIT1
  * @ingroup    Radio
  *
  ******************************************************************************
  */

#include <otstd.h>
#if (OT_FEATURE(M2) == ENABLED)

// Local header for supporting this patch, or other patches
#include "radio_rm2.h"



/** Replacement Encoder functions <BR>
  * ========================================================================<BR>
  * The Encoder/Decoder module in OTlib provides some generic encode and 
  * decode functions.  They work for most radios.  The SPIRIT1 implementation
  * has these optimized encode and decode functions.
  *
  */ 

//ot_sig crc_stream_fn;

#ifdef EXTF_em2_encode_newpacket
void em2_encode_newpacket() {
}
#endif

#ifdef EXTF_em2_decode_newpacket
void em2_decode_newpacket() {
}
#endif


#ifdef EXTF_em2_encode_newframe
void em2_encode_newframe() {
    /// 1. Prepare the CRC and RS encoding, which need to be computed
    ///    when the upper options byte is set.  That is, it is non-zero
    ///    on the first packet and 0 for retransmissions.
    if (txq.options.ubyte[UPPER] != 0) {
        crc_init_stream(True, q_span(&txq), txq.getcursor);
        txq.putcursor  += 2;
        txq.front[0]   += 2;
        txq.front[1]   &= ~0x20;            // always clear this bit
        
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
        
        // Only appoint CRC5 when the first ten bits of txq are settled
        em2_add_crc5(txq.front);
	}

    /// 2. set initial values of encoder control variables
    em2.bytes = q_span(&txq);
}
#endif

#ifdef EXTF_em2_decode_newframe
void em2_decode_newframe() {
    em2.state   = 0;
    em2.bytes   = 8;      // dummy length until actual length is received
}
#endif



#ifndef _DSSS
void em2_encode_data(void) {
    ot_int  fill;
    ot_int  load;
    ot_u8   save[2];
    ot_u8*  cmd;
    
    // Loop unrolling for FIFO loading
    load = (rfctl.txlimit - spirit1_txbytes());
    
    while (1) {
        fill = (load < em2.bytes) ? load : em2.bytes;
        if (fill <= 0) break;
        
        if (fill > 24) fill = 24;
        load       -= fill;
        em2.bytes  -= fill;
        
        if (txq.options.ubyte[UPPER] != 0) {
            crc_calc_nstream(fill);
#           if (M2_FEATURE(RSCODE))
            if (em2.lctl & 0x40) {
                em2_rs_encode(fill);
            }
#           endif
        }

        cmd     = txq.getcursor;
        save[0] = *(--cmd);
        *cmd    = 0xff;
        save[1] = *(--cmd);
        *cmd    = 0x00;
        
        txq.getcursor += fill;
        spirit1_spibus_io(fill+2, 0, cmd);
        *cmd++  = save[1];
        *cmd    = save[0];
    }
    
    /// dummy SPI access to complete fill
    //spirit1_read(RFREG(IRQ_STATUS0));
    *(ot_u16*)save  = PLATFORM_ENDIAN16_C(0x8000);
    spirit1_spibus_io(2, 0, save);
}


void em2_decode_data(void) {
    static const ot_u8 cmd[] = { 0x01, 0xFF };
    ot_u16 grab;
    
    em2_decode_data_TOP:

    grab = spirit1_rxbytes();
    if (grab != 0) {
        if (grab > 24)  grab = 24;
        
        spirit1_spibus_io(2, grab, (ot_u8*)cmd);
        q_writestring(&rxq, spirit1.busrx, grab);
        
        if (em2.state == 0) {
            ot_int ext_bytes;
            em2.state--;
            em2.bytes       = 1 + (ot_int)rxq.front[0];
            rxq.front[1]   &= ~0x20;                    // always clear this bit
            em2.lctl        = rxq.front[1];
            em2.crc5        = em2_check_crc5();
            if (em2.crc5 != 0) {
                return;
            }

            ext_bytes = 0;
            if (em2.lctl & 0x40) {
                ext_bytes = em2_rs_init_decode(&rxq);
            }
            crc_init_stream(False, em2.bytes-ext_bytes, rxq.getcursor);
        }
        
        crc_calc_nstream(grab);
        
        ///@todo we can optimize this also by waiting until crc is done,
        ///      and then verifying that it is not accurate.  but we need
        ///      better speed profiling before doing that.
#       if (M2_FEATURE(RSCODE))
        if (em2.lctl & 0x40) {
            em2_rs_decode(grab);
        }
#       endif

        em2.bytes -= grab;
        if (em2.bytes > 0) {
            goto em2_decode_data_TOP;
        }
    }
}
#endif



#endif  //if (OT_FEATURE(M2) == ENABLED)
