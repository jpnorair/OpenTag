/* Copyright 2009-2014 JP Norair
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
  * @file       /otradio/null/radio_null.h
  * @author     JP Norair
  * @version    R100
  * @date       4 Feb 2014
  * @brief      Radio configuration file for Null radio
  * @ingroup    Null_radio
  *
  * "NULL Radio" is for purposes of testing encoder and DLL without actually
  * sending any data over the air.  Right now it does not map to any hardware,
  * but in the future it should probably map to an SPI or serial interface of
  * some sort.
  *
  * Any platform/board can use the NULL radio as long as it is configured
  * properly.  Usually, this just means setting a compiler constant of:
  * __RADIO_NULL__
  * 
  ******************************************************************************
  */
  

#ifndef __radio_NULL_H
#define __radio_NULL_H

#include <otsys/config.h>
#include <otsys/support.h>
#include <otsys/types.h>


#ifndef ENABLED
#   define ENABLED  1
#endif

#ifndef DISABLED
#   define DISABLED  0
#endif




/** Null Radio RF Feature settings      <BR>
  * ========================================================================<BR>
  * Null Radio doesn't have any high level PHY/MAC features, but it does 
  * pretend to support a basic PHY (i.e. a FIFO for RX/TX).
  */
#define RF_FEATURE(VAL)                 RF_FEATURE_##VAL        // FEATURE                  AVAILABILITY
#define RF_FEATURE_MSK                  ENABLED                 // MSK Modulation           Moderate
#define RF_FEATURE_53K                  ENABLED                 // 55kHz baudrate           High
#define RF_FEATURE_212K                 ENABLED                 // 200kHz baudrate          High
#define RF_FEATURE_PN9                  DISABLED                // Integrated PN9 codec     Moderate (DASH7 has particular sequence)
#define RF_FEATURE_FEC                  DISABLED                // Integrated FEC codec     Moderate (DASH7 has particular sequence)
#define RF_FEATURE_FIFO                 ENABLED                 // RF TX/RX FIFO            High
#define RF_FEATURE_TXFIFO_BYTES         96
#define RF_FEATURE_RXFIFO_BYTES         96
#define RF_FEATURE_PACKET               ENABLED             // Packet Handler           High
#define RF_FEATURE_CRC                  DISABLED            // CCITT CRC16              High
#define RF_FEATURE_CCA                  ENABLED             // DASH7 CCA method         Moderate
#define RF_FEATURE_CSMA                 DISABLED            // DASH7 CSMA method        Low
#define RF_FEATURE_RXTIMER              DISABLED            // RX Timeout capability    Low
#define RF_FEATURE_TXTIMER              DISABLED            // TX MAC capability        DASH7-specific
#define RF_FEATURE_CSMATIMER            DISABLED

//More esoteric stuff
#define RF_FEATURE_SCANCYCLE            DISABLED            // Wake-on scan cycle       DASH7-specific
#define RF_FEATURE_MIRROR               DISABLED            // ISF Register Mirroring   DASH7-specific
#define RF_FEATURE_SYNCFILTER           DISABLED            // Synchronizer Filtering   DASH7-specific
#define RF_FEATURE_LBFILTER             DISABLED            // Link Budget Filtering    DASH7-specific
#define RF_FEATURE_SIDFILTER            DISABLED            // Session ID Filtering     DASH7-specific
#define RF_FEATURE_ADDRFILTER           DISABLED            // Address Filtering        DASH7-specific
#define RF_FEATURE_PARSEFILTER          DISABLED            // Full Parse Filtering     DASH7-specific
#define RF_FEATURE_MAC                  DISABLED            // Full Integrated MAC      DASH7-specific
#define RF_FEATURE_AES128               DISABLED            // AES128 engine            Rare
#define RF_FEATURE_ECC                  DISABLED            // ECC engine               Rare/None yet

#define RF_PARAM(VAL)                   RF_PARAM_##VAL
#define RF_PARAM_PKT_OVERHEAD           (2+6+4)




/** Radio Buffer Allocation constants: Null Radio implements SW Buffer
  */
#define BUFFER_ALLOC                    64


// Change these settings depending on your radio buffer
#define RADIO_BUFFER_TXMAX              RF_FEATURE(TXFIFO_BYTES)
#define RADIO_BUFFER_RXMAX              RF_FEATURE(RXFIFO_BYTES)




/** Internal Radio States
  *
  * b5:3        b2:0
  * TX States   RX States
  */
#define RADIO_STATE_RXSHIFT     0
#define RADIO_STATE_RXMASK      (3 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXINIT      (4 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXDONE      (5 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXAUTO      (0 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXPAGE      (1 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXSLOT      (2 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXMFP       (3 << RADIO_STATE_RXSHIFT)

#define RADIO_STATE_TXSHIFT     3
#define RADIO_STATE_TXMASK      (7 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXINIT      (1 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXCCA1      (2 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXCCA2      (3 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXSTART     (4 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXDATA      (5 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXDONE      (6 << RADIO_STATE_TXSHIFT)

/** Internal Radio Flags
*/
#define RADIO_FLAG_BG           (1 << 0)
#define RADIO_FLAG_CONT         (1 << 1)
#define RADIO_FLAG_BGFLOOD      (3 << 0)
#define RADIO_FLAG_CRC5         (1 << 2)
#define RADIO_FLAG_RESIZE       (1 << 3)
#define RADIO_FLAG_AUTOCAL		(1 << 4)
#define RADIO_FLAG_SETPWR		(1 << 5)
#define RADIO_FLAG_PWRMASK      (3 << 6)
#define RADIO_FLAG_XOOFF        (0 << 6)
#define RADIO_FLAG_XOON         (1 << 6)



/** Internal Radio Interrupt Flags
  * For performance reasons, sometimes interrupt flags will be stored locally
  * and used in later conditionals.  The usage is implementation dependent.
  */
#define RADIO_INT_CCA           (1 << 3)
#define RADIO_INT_CS            (1 << 4)



/** Radio HW Timing Parameters
  * Measurements for Radio HW performance, usually corresponding to timings
  * as the radio goes from one state to the next
  *
  * Reminder: sti is "short tick"
  * 1 sti = 32 ti = 1/32768 sec = ~30.52 us
  */
#define RADIO_COLDSTART_STI_MAX     11
#define RADIO_COLDSTART_STI_TYP     4
#define RADIO_WARMSTART_STI         1
#define RADIO_CALIBRATE_STI         2
#define RADIO_FASTHOP_STI           2
#define RADIO_KILLRX_STI            0
#define RADIO_KILLTX_STI            0
#define RADIO_TURNAROUND_STI        1



/** NULL RF Module local Data
  * Data that is useful for the internal use of this module.  The list below is
  * comprehensive, and it may not be needed in entirety for all implementations.
  * For implementations that don't use the values, comment out.
  *
  * state       Radio State, partially implementation-dependent
  * flags       A local store for usage flags
  * txlimit     An interrupt/event comes when tx buffer gets below this number of bytes
  * rxlimit     An interrupt/event comes when rx buffer gets above this number of bytes
  */
typedef struct {
    ot_u8   state;
    ot_u8   flags;
    ot_int  nextcal;
    ot_int  txlimit;
    ot_int  rxlimit;
} rfctl_struct;

extern rfctl_struct rfctl;



void null_radio_isr(void);


#endif
