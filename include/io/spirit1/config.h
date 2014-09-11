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
  * @file       /io/spirit1/config.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Jan 2014
  * @brief      Radio configuration file for SPIRIT1
  * @ingroup    Platform
  *
  ******************************************************************************
  */
  

#ifndef __IO_SPIRIT1_CONFIG_H
#define __IO_SPIRIT1_CONFIG_H

#include <otstd.h>


#ifndef ENABLED
#   define ENABLED  1
#endif

#ifndef DISABLED
#   define DISABLED  0
#endif








/** SPIRIT1 RF Feature settings      <BR>
  * ========================================================================<BR>
  * These are the RF features that are available and currently implemented 
  * atomically in the SPIRIT1 driver.
  *
  * @note There is an erratum regarding the SPIRIT1 auto-calibrator, which 
  * behooves the implementer to perform offline manual calibration.  You can
  * call radio_calibrate() to do that.
  */
#define RF_FEATURE(VAL)                 RF_FEATURE_##VAL    // FEATURE                  AVAILABILITY
#define RF_FEATURE_AUTOCAL              ENABLED             // Automatic Calibration    High
#define RF_FEATURE_MSK                  ENABLED             // MSK Modulation           Moderate
#define RF_FEATURE_55K                  ENABLED             // 55kHz baudrate           High
#define RF_FEATURE_200K                 ENABLED             // 200kHz baudrate          High
#define RF_FEATURE_PN9                  ENABLED             // Integrated PN9 codec     Moderate (DASH7 has particular sequence)
#define RF_FEATURE_FEC                  ENABLED             // Integrated FEC codec     Moderate (DASH7 has particular sequence)
#define RF_FEATURE_FIFO                 ENABLED             // RF TX/RX FIFO            High   
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
#define RF_FEATURE_AES128               ENABLED             // AES128 engine            Rare
#define RF_FEATURE_ECC                  DISABLED            // ECC engine               Rare/None yet

#define RF_PARAM(VAL)                   RF_PARAM_##VAL
#define RF_PARAM_SSBIT_US               
#define RF_PARAM_RAMP_BYTES             1                   // Ramp-up + Down
#define RF_PARAM_PREAMBLE_BYTES         8                   // 8 seems to work best on SPIRIT1
#define RF_PARAM_SYNC_BYTES             4                   // Always 4
#define RF_PARAM_PKT_OVERHEAD           (RF_PARAM_RAMP_BYTES+RF_PARAM_PREAMBLE_BYTES+RF_PARAM_SYNC_BYTES)
#define RF_PARAM_RCO_CAL_INTERVAL       32                      //SPIRIT1-Specific
#define RF_PARAM_VCO_CAL_INTERVAL       32                      //SPIRIT1-Specific


/** Radio Buffer Allocation constants
  * SPIRIT1 has internal buffer
  */
#define BUFFER_ALLOC         0


/** Recasting of some radio attributes
  * Most radios we use have internal FIFO.  But the buffer could also be a DMA
  * on the MCU.  If you do not have a DMA on your micro, make sure you are using
  * a radio with a FIFO because you need to have one or the other.
  */
#define RADIO_BUFFER_TXMAX          RF_FEATURE(TXFIFO_BYTES)
#define RADIO_BUFFER_RXMAX          RF_FEATURE(RXFIFO_BYTES)



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








#endif
