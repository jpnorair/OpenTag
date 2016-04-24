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
  * @file       /otradio/null/null_defaults.h
  * @author     JP Norair
  * @version    R100
  * @date       15 Feb 2014
  * @brief      Null Radio register default definitions & macros.
  * @ingroup    Null_Radio
  *
  ******************************************************************************
  */

#ifndef __NULL_defaults_H
#define __NULL_defaults_H

#include <otsys/types.h>
#include <otsys/config.h>
#include "null_registers.h"


// Some radio register settings constants
// These are basically for the CC430, and the POSIX impl just ignores them
#ifndef RF_PARAM_BAND
#   define RF_PARAM_BAND    433
#endif

#if ( (RF_PARAM_BAND != 433) && \
	  (RF_PARAM_BAND != 866) )
#   error "RF_PARAM_BAND is not set to an implemented value"
#endif

/// Don't change these GDO values unless you really know what you are doing.
/// Some of the basic PHY/MAC features depend on these.
#define DRF_IOCFG2              GDO_CHIP_RDYn
#define DRF_IOCFG1              GDO_RSSI_VALID
#define DRF_IOCFG0              GDO_PLL_LOCK

#define DRF_FIFOTHR             (_CLOSE_IN_RX_0dB)

#define DRF_SYNC1               0x0B
#define DRF_SYNC0               0x67

#define DRF_PKTLEN              0xFF

#define DRF_PKTCTRL1            (_PQT(3) | _ADR_CHK_NONE)

#define DRF_PKTCTRL0            (_WHITE_DATA | _PKT_FORMAT_NORMAL | _PKT_LENCFG_VARIABLE)

#define DRF_ADDR                0x00
#define DRF_CHANNR              0x07

#define DRF_FSCTRL1             0x06    //From SRFS
#define DRF_FSCTRL0             0x00    //check SRFS

#if (RF_PARAM_BAND == 433)
#   define DRF_FREQ2            0x10    //From SRFS
#   define DRF_FREQ1            0xA8    //From SRFS
#   define DRF_FREQ0            0xFA    //From SRFS
#elif (RF_PARAM_BAND == 866)
#   define DRF_FREQ2            0x21    //From SRFS
#   define DRF_FREQ1            0x50    //From SRFS
#   define DRF_FREQ0            0x5C    //From SRFS
#else
#   warn "RF_PARAM_BAND set to an unsupported value.  PHY may not work properly."
#endif

#define DRF_MDMCFG4             0x9B    //From SRFS: 55kHz=9B, 200kHz=6C
#define DRF_MDMCFG4_HI          0x6C
#define DRF_MDMCFG3             0x18    //From SRFS: 55kHz=18, 200kHz=F8
#define DRF_MDMCFG3_HI          0xF8
#define DRF_MDMCFG2             (_DEM_DCFILT_OFF | _MOD_FORMAT_GFSK | _SYNC_MODE_16in16)
#define DRF_MDMCFG1             (_NUM_PREAMBLE_4B | _CHANSPC_E(2)) //From SRFS
#define DRF_MDMCFG1_HI          (_NUM_PREAMBLE_6B | _CHANSPC_E(2)) //From SRFS
#define DRF_MDMCFG0             0x11    //From SRFS

#define DRF_DEVIATN             0x50    //From SRFS (50 kHz)

#define DRF_MCSM2               0
#define DRF_MCSM1               (_CCA_MODE_ALWAYS | _RXOFF_MODE_IDLE | _TXOFF_MODE_IDLE)
#define DRF_MCSM0               (_FS_AUTOCAL_4THIDLE)

#define DRF_FOCCFG              0x16    //From SRFS
#define DRF_BSCFG               0x6D /* 0x6C */    //From SRFS

#define DRF_AGCCTRL2            0x03    //From SRFS
#define DRF_AGCCTRL1            0x40    //From SRFS
#define DRF_AGCCTRL0            0x91    //From SRFS

#define DRF_WOREVT1             0
#define DRF_WOREVT0             0

#define DRF_WORCTRL             (_RC_PD)

#define DRF_FREND1              0x56    //From SRFS
#define DRF_FREND0              0x17    //From SRFS

#define DRF_FSCAL3              0xEA    //From SRFS
#define DRF_FSCAL2              0x2A    //From SRFS
#define DRF_FSCAL1              0x00    //From SRFS
#define DRF_FSCAL0              0x1F    //From SRFS

#define DRF_FSTEST              0x59    //From SRFS
#define DRF_PTEST               0x7F    //From SRFS
#define DRF_AGCTEST             0x3F    //From SRFS
#define DRF_TEST2               0x88    //From SRFS
#define DRF_TEST1               0x31    //From SRFS
#define DRF_TEST0               0x09    //From SRFS


#endif
