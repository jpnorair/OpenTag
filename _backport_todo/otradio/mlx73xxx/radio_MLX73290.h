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
  * @file       /OTradio/MLX73xxx/radio_MLX73290.h
  * @author     JP Norair
  * @version    V1.0
  * @date       17 November 2011
  * @brief      Radio configuration file for MLX73290
  * @ingroup    Platform
  *
  ******************************************************************************
  */
  

#ifndef __radio_MLX73290_H
#define __radio_MLX73290_H

#include "OT_support.h"


#ifndef ENABLED
#   define ENABLED  1
#endif

#ifndef DISABLED
#   define DISABLED  0
#endif




/** MLX73290 Feature settings      <BR>
  * ========================================================================<BR>
  * These are the RF features that are available and currently implemented on
  * the MLX73290.  Some RF-level features (like internal timers, polling 
  * automation) are not implemented yet, but they could be in the future.
  */
#define RF_FEATURE(VAL)                 RF_FEATURE_##VAL        // FEATURE                  AVAILABILITY
#define RF_FEATURE_MSK                  DISABLED                // MSK Modulation           Moderate
#define RF_FEATURE_55K                  ENABLED                 // 55kHz baudrate           High
#define RF_FEATURE_200K                 ENABLED                 // 200kHz baudrate          High
#define RF_FEATURE_PN9                  ENABLED                 // Integrated PN9 codec     Moderate
#define RF_FEATURE_FEC                  DISABLED                // Integrated FEC codec     Moderate
#define RF_FEATURE_FIFO                 ENABLED                 // RF TX/RX FIFO            High
#define RF_FEATURE_TXFIFO_BYTES         128
#define RF_FEATURE_RXFIFO_BYTES         128
#define RF_FEATURE_PACKET               ENABLED                 // Packet Handler           High 
#define RF_FEATURE_CRC                  ENABLED                 // CCITT CRC16              High
#define RF_FEATURE_CSMA                 DISABLED                // CSMA                     Low
#define RF_FEATURE_RXTIMER              DISABLED                // RX Timeout capability    Low
#define RF_FEATURE_TXTIMER              DISABLED                // TX CSMA procedure        DASH7-specific
#define RF_FEATURE_SCANCYCLE            DISABLED                // Wake-on scan cycle       DASH7-specific
#define RF_FEATURE_MIRROR               DISABLED                // ISF Register Mirroring   DASH7-specific
#define RF_FEATURE_SYNCFILTER           DISABLED                // Synchronizer Filtering   DASH7-specific
#define RF_FEATURE_LBFILTER             DISABLED                // Link Budget Filtering    DASH7-specific
#define RF_FEATURE_SIDFILTER            DISABLED                // Session ID Filtering     DASH7-specific
#define RF_FEATURE_ADDRFILTER           DISABLED                // Address Filtering        DASH7-specific
#define RF_FEATURE_PARSEFILTER          DISABLED                // Full Parse Filtering     DASH7-specific
#define RF_FEATURE_MAC                  DISABLED                // Full Integrated MAC      DASH7-specific
#define RF_FEATURE_AES128               DISABLED                // AES128 engine            Rare/None yet
#define RF_FEATURE_ECC                  DISABLED                // ECC engine               Rare/None yet
#define RF_FEATURE_ALGE                 DISABLED                // Algebraic Eraser engine  Rare/None yet









#endif
