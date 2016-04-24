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
  * @file       /otradio/cc1101/radio_CC1101.h
  * @author     JP Norair
  * @version    R100
  * @date       31 Oct 2012
  * @brief      Radio configuration file for CC1101
  * @ingroup    Platform
  *
  ******************************************************************************
  */


#ifndef __radio_CC1101_H
#define __radio_CC1101_H

#include <otsys/types.h>
#include <otsys/support.h>


#ifndef ENABLED
#   define ENABLED  1
#endif

#ifndef DISABLED
#   define DISABLED  0
#endif




/** CC1101 RF Feature settings      <BR>
  * ========================================================================<BR>
  * These are the RF features that are available and currently implemented on
  * the CC1101.  In addition, you can ENABLE the RXTIMER feature.  However, it
  * is only useful for some specific applications.  For general purpose
  * applications, it can cause timing glitches.
  */
#define RF_FEATURE(VAL)                  RF_FEATURE_##VAL        // FEATURE                  AVAILABILITY
#define RF_FEATURE_MSK                   ENABLED                 // MSK Modulation           Moderate
#define RF_FEATURE_55K                   ENABLED                 // 55kHz baudrate           High
#define RF_FEATURE_200K                  ENABLED                 // 200kHz baudrate          High
#define RF_FEATURE_PN9                   ENABLED                 // Integrated PN9 codec     Moderate (DASH7 has particular sequence)
#define RF_FEATURE_FEC                   ENABLED                 // Integrated FEC codec     Moderate (DASH7 has particular sequence)
#define RF_FEATURE_FIFO                  ENABLED                 // RF TX/RX FIFO            High
#define RF_FEATURE_TXFIFO_BYTES          64
#define RF_FEATURE_RXFIFO_BYTES          64
#define RF_FEATURE_PACKET                ENABLED                 // Packet Handler           High
#define RF_FEATURE_CRC                   DISABLED                // CCITT CRC16              High
#define RF_FEATURE_CSMA                  DISABLED                // CSMA                     Low
#define RF_FEATURE_RXTIMER               DISABLED                // RX Timeout capability    Low
#define RF_FEATURE_TXTIMER               DISABLED                // TX MAC capability        DASH7-specific
#define RF_FEATURE_SCANCYCLE             DISABLED                // Wake-on scan cycle       DASH7-specific
#define RF_FEATURE_MIRROR                DISABLED                // UDB Register Mirroring   DASH7-specific
#define RF_FEATURE_SYNCFILTER            DISABLED                // Synchronizer Filtering   DASH7-specific
#define RF_FEATURE_LBFILTER              DISABLED                // Link Budget Filtering    DASH7-specific
#define RF_FEATURE_SIDFILTER             DISABLED                // Session ID Filtering     DASH7-specific
#define RF_FEATURE_ADDRFILTER            DISABLED                // Address Filtering        DASH7-specific
#define RF_FEATURE_PARSEFILTER           DISABLED                // Full Parse Filtering     DASH7-specific
#define RF_FEATURE_MAC                   DISABLED                // Full Integrated MAC      DASH7-specific
#define RF_FEATURE_AES128                DISABLED                // AES128 engine            Rare/None yet
#define RF_FEATURE_ECC                   DISABLED                // ECC engine               Rare/None yet
#define RF_FEATURE_ALGE                  DISABLED                // Algebraic Eraser engine  Rare/None yet

#define RF_PARAM_PKT_OVERHEAD           (2+4+2)










/** Radio Buffer Allocation constants
  * CC430 has internal buffer
  */
#define BUFFER_ALLOC         0


/** Recasting of some radio attributes
  * Most radios we use have internal FIFO.  But the buffer could also be a DMA
  * on the MCU.  If you do not have a DMA on your micro, make sure you are using
  * a radio with a FIFO because you need to have one or the other.
  */
#if (RF_FEATURE_FIFO == ENABLED)
#   define RADIO_BUFFER_TXMAX        RF_FEATURE(TXFIFO_BYTES)
#   define RADIO_BUFFER_RXMAX        RF_FEATURE(RXFIFO_BYTES)
#else
#   define RADIO_BUFFER_TXMAX        MCU_FEATURE(RADIODMA_TXBYTES)
#   define RADIO_BUFFER_RXMAX        MCU_FEATURE(RADIODMA_RXBYTES)
#endif




/** Internal Radio States
  *
  * b5:3        b2:0
  * TX States   RX States
  */
#define RADIO_STATE_RXSHIFT     0
#define RADIO_STATE_RXMASK      (3 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXAUTO      (0 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXMFP       (1 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXPAGE      (2 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXDONE      (4 << RADIO_STATE_RXSHIFT)

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
#define RADIO_FLAG_FRCONT       (1 << 0)
#define RADIO_FLAG_FLOOD        (1 << 1)
#define RADIO_FLAG_AUTO         (1 << 2)
#define RADIO_FLAG_RESIZE       (1 << 3)
#define RADIO_FLAG_AUTOCAL		(1 << 4)
#define RADIO_FLAG_SETPWR		(1 << 5)
//#define RADIO_FLAG_CCAFAIL      (1 << 6)  //this flag presently unused
//#define RADIO_FLAG_RXING        (1 << 6)  //unused
#define RADIO_FLAG_ASLEEP		(1 << 7)




/** Radio HW Timing Parameters
  * Measurements for Radio HW performance, usually corresponding to timings
  * as the radio goes from one state to the next
  *
  * Reminder: sti is "short tick"
  * 1 sti = 32 ti = 1/32768 sec = ~30.52 us
  */
#define RADIO_COLDSTART_STI_MAX     27
#define RADIO_COLDSTART_STI_TYP     9
#define RADIO_WARMSTART_STI         3
#define RADIO_CALIBRATE_STI         24
#define RADIO_FASTHOP_STI           5
#define RADIO_KILLRX_STI            0
#define RADIO_KILLTX_STI            0
#define RADIO_TURNAROUND_STI        1







/** Radio Module Data
  * Data that is useful for the internal use of this module.  The list below is
  * comprehensive, and it may not be needed in entirety for all implementations.
  * For implementations that don't use the values, comment out.
  *
  * state       Radio State, used with multi-call functions (RX/TX)
  * flags       A local store for usage flags
  * txlimit     An interrupt/event comes when tx buffer gets below this number of bytes
  * rxlimit     An interrupt/event comes when rx buffer gets above this number of bytes
  */
typedef struct {
    ot_u8   state;
    ot_u8   flags;
    ot_int  txlimit;
    ot_int  rxlimit;
} rfctl_struct;

extern rfctl_struct rfctl;






#endif
