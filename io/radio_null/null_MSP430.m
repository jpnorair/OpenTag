/* Copyright 2009-2012 JP Norair
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
  * @file       /otradio/null/null_interface.h
  * @author     JP Norair
  * @version    V1.0
  * @date       5 April 2012
  * @brief      Functions for the Null transceiver interface
  * @defgroup   Null_RF (Null Radio)
  *
  ******************************************************************************
  */

#ifndef __NULL_interface_H
#define __NULL_interface_H

#include "null_interface.h"

/// Four valid DASH7 messages to receive randomly.  
/// You can change these if you like

static const ot_u8 message1[] = { \
    
};

static const ot_u8 message2[] = { \
    
};

static const ot_u8 message3[] = { \
    
};

static const ot_u8 message4[] = { \
    
};

static const ot_u8* messages[] = { message1, message2, message2, message4 };

ot_u8* active_message;
ot_int air_cursor;


/** Basic Control <BR>
  * ============================================================================
  */
OT_INTERRUPT void null_radio_isr(void) {
    // Clear timer interrupt
    
    radio_isr();
    
    LPM4_EXIT;
}


void sub_set_timer(ot_u16 value) {
    RADIO_TIM->CTL |= 0x0004;   //clear & stop timer
    RADIO_TIM->CTL &= ~0x0033;  //clear configuration
    RADIO_TIM->CCR0 = value;    //set next interval
    RADIO_TIM->CTL |= 0x0012;   //restart in upmode, with interrupt
}


void sub_flush_timer() {
    RADIO_TIM->CTL |= 0x0004;   //clear & stop timer
    RADIO_TIM->CTL &= ~0x0033;  //clear configuration
    RADIO_TIM->CCR0 = 65535;    //workaround to a known CC430 erratum
    RADIO_TIM->CTL |= 0x0020;   //restart in continuous-up, without interrupt
}


ot_u16 sub_get_timer() {
    return RADIO_TIM->R;
}



void sub_waitbytes(ot_u16 wait_bytes) {
    sub_set_timer(rm2_scale_codec(wait_bytes) << 5);
}



void null_radio_init() {
/// Init NRF timer to 32768 Hz (ACLK Hz)
    null_radio.imode    = MODE_init;
    RADIO_TIM->CTL      = TASSEL_1 | TIMA_FLG_TACLR | TAIE;
    RADIO_TIM->EX0      = 0;
}


void null_radio_listen() {
/// Wait 1 tick to send Sync interrupt
    sub_set_timer(32);
}


void null_radio_timeproc(const char* msg, ot_int bytes) {
    ot_u8* s    = msg;
    ot_u8* dst  = logdata;
    ot_u8 loops = False;
    logdata_len = 0;
    
    while (loops != 0) {
        switch (loops) {
           default: for (; *s!=0; ++logdata_len) {
                        *dst++ = *s++;
                    }
                    break;
        
            case 3: logdata_len += otutils_int2dec(dst, bytes);
                    break;
                    
            case 2: s = msg2;
                    break;

            
           
        }
        *dst++ = ' ';
        logdata_len++;
    
        loops--;
    }
    
    
    
    if (loops == 0) return;
    
    
    
    s = msg2;
    goto null_radio_timeproc_WRITE;
    
    
    platform_memcpy(logdata, msg, msg_len);
    sub_flush_timer();
}


void null_radio_endproc(ot_int label_len, const char *label) {
    ot_u8   logdata[8];
    ot_int  logdata_len;
    
    logdata_len = otutils_int2dec(logdata, sub_get_timer());
    
    otapi_log_msg(MSG_utf8, label_len, logdata_len, (ot_u8*)label, logdata);
}





void null_radio_cmdrx() {
/// Prepare a random message to RX
    null_radio.msg          = messages[ platform_prand_u8() & 3 ];
    null_radio.msg_counter  = (radio.flags & RADIO_FLAG_AUTO) ? \
                                    null_radio.msg[0] : 32767;
    null_radio.fifo_bytes   = 0;    
}


void null_radio_cmdtx() {
    null_radio.msg_counter  = (radio.flags & RADIO_FLAG_AUTO) ? \
                                    null_radio.fifo[0] : 32767;
}


void null_radio_procrx() {
/// Wait for bytes in RX buffer to get "filled" to a certain point.
    ot_int fifo_bytes = null_radio.fifo_put - null_radio.fifo;
    sub_waitbytes(null_radio.fifo_thr - fifo_bytes);
    sub_receivefifo();
}


void null_radio_procrxend() {
    null_radio.fifo_thr = (null_radio.fifo_put - null_radio.fifo) \
                            + null_radio.msg_counter;
    sub_waitbytes(null_radio.fifo_thr);
    sub_receivefifo();
}


void null_radio_proctx() {
/// Wait for bytes to get sent out by fake transmitter
    ot_int fifo_bytes = null_radio.fifo_put - null_radio.fifo;
    sub_waitbytes(fifo_bytes - null_radio.fifo_thr);
    sub_transmitfifo();
}


void null_radio_proctxend() {
    null_radio.fifo_thr = 0;
    
    sub_waitbytes(fifo_bytes - null_radio.fifo_thr);
    sub_transmitfifo();
}


void null_radio_setfifothr(ot_u8 fifothr) {
    null_radio.fifo_thr = fifothr;
}








ot_u8 null_radio_getbyte() {
    ot_u8 data_out;
    data_out = *null_radio.fifo_get++;
    
    //if (null_radio.fifo_get == null_radio.fifo_put) {
    //    underflow interrupt
    //}
    
    return data_out;
}


void null_radio_putbyte(ot_u8 data_in) {
    *null_radio.fifo_put++ = data_in;
    
    //if (null_radio.fifo_put == null_radio.fifo_get) {
        //overflow interrupt
    //}
}


void null_radio_flush() {
    null_radio.fifo_get     = null_radio.fifo;
    null_radio.fifo_put     = null_radio.fifo;
}


void sub_transmitfifo() {
    while (null_radio.fifo_put > (null_radio.fifo + null_radio.fifo_thr)) {
        null_radio.fifo_put--;
        null_radio.fifo_bytes--;
        null_radio.msg_cursor--;
    }
}


void sub_receivefifo() {
    while (null_radio.fifo_put < (null_radio.fifo + null_radio.fifo_thr)) {
        *null_radio.fifo_put++ = *null_radio.msg++;
        null_radio.fifo_bytes++;
        null_radio.msg_counter--;
    }
}






/** Stuff below this line is deprecated, and it may be removed later
  * ========================================================================<BR>
  */

/// Packet overhead & slop, 
/// These are parts of the packet that do not carry preamble, sync word, or 
/// frame data.  The units are in bytes.
#define RADIO_RAMP_UP       1
#define RADIO_RAMP_DOWN     1
#define RADIO_PKT_PADDING   1
#define RADIO_PKT_OVERHEAD  (RADIO_RAMP_UP + RADIO_RAMP_DOWN + RADIO_PKT_PADDING)


/// CS/CCA offsets
/// Used for setup of the CS RSSI Thresholding Mechanism.  The CCA can be handled
/// by the same mechanism, (the code for this is commented-out), but testing has
/// shown that a direct RSSI method is faster & more reliable.
#define RF_CSTHR_OFFSET     37
#define RF_CCATHR_OFFSET    37





#endif
