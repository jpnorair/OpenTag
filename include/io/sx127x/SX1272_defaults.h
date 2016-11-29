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
  * @file       /include/io/sx127x/SX1272_defaults.h
  * @author     JP Norair
  * @version    R100
  * @date       20 Oct 2016
  * @brief      SX1272 LoRa Radio register default definitions & macros.
  * @ingroup    SX127x
  *
  ******************************************************************************
  */

#ifndef __SX1272_defaults_H
#define __SX1272_defaults_H

#include <otstd.h>
#include <board.h>
#include <platform/config.h>
#include <io/sx127x/SX1272_registers.h>


// Some radio register settings constants
#ifndef RF_PARAM_BAND
#   define RF_PARAM_BAND    866
#endif

#if ((RF_PARAM_BAND != 866) && (RF_PARAM_BAND != 915))
#   error "RF_PARAM_BAND is not set to an implemented value, must be 866 or 915"
#endif

#if (BOARD_FEATURE_RFXTAL != ENABLED)
#   undef BOARD_PARAM_RFHz
#   define BOARD_PARAM_RFHz  BOARD_PARAM_HFHz
#   define _XTAL_SRC    _EXT_REF
#else
#   define _XTAL_SRC    0
#endif

#if (BOARD_PARAM_RFHz != 32000000)
#   define _32MHz
#else
#   error "BOARD_PARAM_RFHz must be 32 MHz"
#endif


//#define RFREG_LR_FIFO                               0x00 

#define DRF_LR_OPMODE           (_LORAMODE | _OPMODE_SLEEP)
           

// Frequency Select bytes
#define DRF_LR_FRMSB            ((_FRF_866MHz>>16) & 0xFF)
#define DRF_LR_FRFMIB           ((_FRF_866MHz>>8) & 0xFF)
#define DRF_LR_FRFLSB           ((_FRF_866MHz>>0) & 0xFF)


// Tx Output Power
#define DRF_LR_PACONFIG         (_PANORMAL | __OUTPUT_PWR(14))  //13dBm setting: -1 + 14

// Phase noise shouldn't matter so much for LoRa modulation, but worth a test.
// PA-Ramp is also mostly an FSK feature, so I choose the default setting.
#define DRF_LR_PARAMP           (_LOW_PN_TX_PLLOFF | _PA_RAMP_40us)

// Overcurrent protection: using chip defaults.
#define DRF_LR_OCP              (_OCP_ON | _OCP_TRIM_100mA)

// LNA Gain: Use max gain setting, and boost it
#define DRF_LR_LNA              (_LNA_GAIN_G1 | _LNA_BOOST_ON)

// LoRa FIFO Registers: Single FIFO mode.
#define DRF_LR_FIFOADDRPTR      0x00
#define DRF_LR_FIFOTXBASEADDR   0x00
#define DRF_LR_FIFORXBASEADDR   0x00

// IRQ Flag Mask: All IRQs are enabled
#define DRF_LR_IRQFLAGSMASK     (0)

// Modem Config 1: LoRa forces a weak CRC poly on us, so fuck them, we don't use it
// For Foreground packets, however, we are forced to use their explicit header
#define DRF_LR_MODEMCONFIG1     (_BW_500_KHZ | _CODINGRATE_4_7 | _IMPLICITHEADER_ON | _RXPAYLOADCRC_OFF | _LOWDATARATEOPTIMIZE_OFF)
#define DRF_LR_MODEMCONFIG1_BG  (_BW_500_KHZ | _CODINGRATE_4_7 | _IMPLICITHEADER_ON | _RXPAYLOADCRC_OFF | _LOWDATARATEOPTIMIZE_OFF)
#define DRF_LR_MODEMCONFIG1_FG  (_BW_500_KHZ | _CODINGRATE_4_7 | _IMPLICITHEADER_OFF | _RXPAYLOADCRC_OFF | _LOWDATARATEOPTIMIZE_OFF)

// Modem Config 2: 
// - we use SF7 @ 500 kHz
// - we set RX timeout to slightly more than the length of a BG packet, 
// - a BG packet is roughly 28.25 symbols in length, total, including 11.25 symbol preamble.
#define DRF_LR_MODEMCONFIG2     (_SF_7 | _TX_CONT_OFF | _AGC_ON | __SYMBTIMEOUT_MSB(0))
#define DRF_LR_SYMBTIMEOUTLSB   (29)

// Preamble length is set to 8
#define DRF_LR_PREAMBLEMSB      0
#define DRF_LR_PREAMBLELSB      8

// Payload length set to 6, the length of BG frame.  FG frames use explicit header mode
#define DRF_LR_PAYLOADLENGTH    6

// Payload max length to 255
#define DRF_LR_PAYLOADMAXLENGTH 255                         // chip default

// Hopping is unused
#define DRF_LR_HOPPERIOD        0                           // chip default

// Not using SF6
#define DRF_LR_DETECTOPTIMIZE   (_DETECT_SF7_TO_SF12)       // chip default

// there is a phantom 0x27 required in this register
#define DR_LR_INVERTIQ          (_INVERTIQ_RX_OFF | 0x27)   // chip default

// Not using SF6
#define DRF_LR_DETECTIONTHRESHOLD (_THRESH_SF7_TO_SF12)     // chip default

// Default is 0x12, LoRaWAN is 0x34, we use 0xD7!
// This syncword is not a normal binary sync word, so its value doesn't matter so much
#define DRF_LR_SYNCWORD         (0xD7)

// Undocumented Register.  Leave alone!
#define DRFREG_LR_INVERTIQ2     (_INVERTIQ2_OFF)    // let this stay at chip default


// DIO settings are provided for all modes of operation.  We don't really care 
// about DIO4 or DIO2 in the present implementation, and DIO3 is optional but
// basically unused.  DIO5 is used as a pin test but not required as a line 
// interrupt.
// - CAD: (_DIO0_10 | _DIO1_10 | _DIO3_00 | _DIO5_00)
// - BRX: (_DIO0_00 | _DIO1_00 | _DIO3_10 | _DIO5_00)
// - FRX: (_DIO0_00 | _DIO1_00 | _DIO3_01 | _DIO5_00)
// - BTX: (_DIO0_01 | _DIO1_xx | _DIO3_xx | _DIO5_00)
// - FTX: (_DIO0_01 | _DIO1_xx | _DIO3_xx | _DIO5_00)
#define DRF_LR_DIOMAPPING1      _DIOMAPPING1_CAD
#define DRF_LR_DIOMAPPING2      (_DIO5_00 | _DIO4_00 | _MAP_RSSI)  // chip default


// There is insufficient application information to select non-default values
// for the following registers
#define DRF_LR_AGCREF
#define DRF_LR_AGCTHRESH1
#define DRF_LR_AGCTHRESH2
#define DRF_LR_AGCTHRESH3
#define DRF_LR_PLLHOP
#define DRF_LR_TCXO
#define DRF_LR_PADAC
#define DRF_LR_PLL
#define DRF_LR_PLLLOWPN
#define DRF_LR_FORMERTEMP






#ifdef __EVAL_RF__
#   include <io/sx127x/SX1272_override.h>
#endif


#endif
