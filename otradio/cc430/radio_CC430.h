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
  * @file       /OTradio/CC430/radio_CC430.h
  * @author     JP Norair
  * @version    V1.0
  * @date       2 Feb 2012
  * @brief      Radio configuration file for CC430
  * @ingroup    Platform
  *
  ******************************************************************************
  */
  

#ifndef __radio_CC430_H
#define __radio_CC430_H

#include "OT_support.h"


#ifndef ENABLED
#   define ENABLED  1
#endif

#ifndef DISABLED
#   define DISABLED  0
#endif




/** CC430 RF Feature settings      <BR>
  * ========================================================================<BR>
  * These are the RF features that are available and currently implemented on
  * the CC430.  In addition, you can ENABLE the RXTIMER feature.  However, it
  * is only useful for some specific applications.  For general purpose
  * applications, it can cause timing glitches.
  */
#define RF_FEATURE(VAL)                  RF_FEATURE_##VAL        // FEATURE                  AVAILABILITY
#define RF_FEATURE_MSK                   ENABLED                 // MSK Modulation           Moderate
#define RF_FEATURE_55K                   ENABLED                 // 55kHz baudrate           High
#define RF_FEATURE_200K                  ENABLED                 // 200kHz baudrate          High
#define RF_FEATURE_PN9                   ENABLED                 // Integrated PN9 codec     Moderate (DASH7 has particular sequence)
#define RF_FEATURE_FEC                   DISABLED                // Integrated FEC codec     Moderate (DASH7 has particular sequence)
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








#endif
