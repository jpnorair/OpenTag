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

#include "OT_types.h"
#include "NULL_registers.h"
#include "NULL_defaults.h"



typedef enum {
    MODE_init   = 0,
    MODE_listen = 1,
    MODE_rxdata = 2,
    MODE_rxend  = 3,
    MODE_txcsma = 4,
    MODE_txdata = 5,
    MODE_txend  = 6
} imode_enum;

typedef struct {
    // Generic Data
    imode_enum  imode;
    ot_u8*      syncword;
    ot_u8       mod;
    ot_u8       fc;
    
    // Internal HW simulation
    ot_int      msg_counter;
    ot_u8*      msg;
    ot_int      fifo_bytes;
    ot_int      fifo_thr;
    ot_u8*      fifo_ptr;
    ot_u8*      air_ptr;
    ot_u8       fifo[64];
} null_radio_struct;

extern null_radio_struct null_radio;


/** Basic Control <BR>
  * ============================================================================
  */
void null_radio_init();
void null_radio_waitlisten();
void null_radio_waitrx();
void null_radio_waittx();
void null_radio_waitcsma();
void null_radio_waitrxend();
void null_radio_waittxend();


OT_INLINE_H void null_iocfg_listen() {
    null_radio.imode = MODE_listen;
    null_radio_waitlisten();
}

OT_INLINE_H void null_iocfg_rxdata() {
    null_radio.imode = MODE_rxdata;
    null_radio_waitrx();
}

OT_INLINE_H void null_iocfg_txcsma() {
    null_radio.imode = MODE_txcsma;
    null_radio_waitcsma();
}

OT_INLINE_H void null_iocfg_txdata() {
    null_radio.imode = MODE_txdata;
    null_radio_waittx();
}

OT_INLINE_H void null_iocfg_rxend() {
    null_radio.imode = MODE_rxend;
    null_radio_waitrxend();
}

OT_INLINE_H void null_iocfg_txend() {
    null_radio.imode = MODE_txend;
    null_radio_waittxend();
}

OT_INLINE_H void null_command_tx() { 
}

OT_INLINE_H void null_command_rx() { 
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
