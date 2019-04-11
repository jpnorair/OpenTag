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
  * @file       /include/io/sx127x/SX1276_defaults.h
  * @author     JP Norair
  * @version    R100
  * @date       20 Oct 2016
  * @brief      SX1276 LoRa Radio register default definitions & macros.
  * @ingroup    SX127x
  *
  ******************************************************************************
  */

#ifndef __SX1276_defaults_H
#define __SX1276_defaults_H

#include <otstd.h>
#include <board.h>
#include <platform/config.h>

#include "SX1272_defaults.h"

#include <io/sx127x/SX1276_registers.h>


// PARamp
#undef  DRF_LR_PARAMP
#define DRF_LR_PARAMP           (_PA_RAMP_40us)

// LNA Gain: Use max gain setting
// LNA Boost can be enabled or disabled following experimentation
#undef  DRF_LR_LNA
#if ((RF_PARAM_BAND > 779) && (RF_PARAM_BAND < 960))
#   define DRF_LR_LNA           (_LNA_GAIN_G1 | _LNA_BOOST_HF_ON)
//#   define DRF_LR_LNA           (_LNA_GAIN_G1 | _LNA_BOOST_HF_OFF)
#elif ((RF_PARAM_BAND > 150) && (RF_PARAM_BAND < 525))
#   define DRF_LR_LNA           (_LNA_GAIN_G1 | _LNA_BOOST_LF_ON)
//#   define DRF_LR_LNA           (_LNA_GAIN_G1 | _LNA_BOOST_LF_OFF)
#else
#   error "RF_PARAM_BAND is not in a usable range (150-525 MHz or 779-960 MHz)"
#endif



// Modem Config 1: Semtech forces a weak CRC poly on us, so fuck them, we don't use it
// For Foreground packets, however, we are forced to use their explicit header
#undef  DRF_LR_MODEMCONFIG1
#undef  DRF_LR_MODEMCONFIG1_BG
#undef  DRF_LR_MODEMCONFIG1_FG
#define DRF_LR_MODEMCONFIG1     (_BW_500_KHZ | _CODINGRATE_4_7 | _IMPLICITHEADER_ON | _LOWDATARATEOPTIMIZE_OFF)
#define DRF_LR_MODEMCONFIG1_BG  (_BW_500_KHZ | _CODINGRATE_4_7 | _IMPLICITHEADER_ON | _LOWDATARATEOPTIMIZE_OFF)
#define DRF_LR_MODEMCONFIG1_FG  (_BW_500_KHZ | _CODINGRATE_4_7 | _IMPLICITHEADER_OFF | _LOWDATARATEOPTIMIZE_OFF)

// Modem Config 2: 
// - we use SF7 @ 500 kHz
// - we set RX timeout to slightly more than the length of a BG packet, 
#undef  DRF_LR_MODEMCONFIG2     
#undef  DRF_LR_MODEMCONFIG2_BG  
#undef  DRF_LR_MODEMCONFIG2_FG  
#define DRF_LR_MODEMCONFIG2     (_SF_7 | _TX_CONT_OFF | _RXPAYLOADCRC_OFF | __SYMBTIMEOUT_MSB(0))
#define DRF_LR_MODEMCONFIG2_BG  (_SF_7 | _TX_CONT_OFF | _RXPAYLOADCRC_OFF | b00)
#define DRF_LR_MODEMCONFIG2_FG  (_SF_7 | _TX_CONT_OFF | _RXPAYLOADCRC_OFF | b11)


/// Chip default is to have AGC Automatic control as off.
/// Experiment with this value to get best results.
//#define DRF_LR_MODEMCONFIG3     (_LOWDATARATEOPTIMIZE_OFF | _AGC_AUTO_ON)
//#define DRF_LR_MODEMCONFIG3     (_LOWDATARATEOPTIMIZE_ON | _AGC_AUTO_OFF)
#define DRF_LR_MODEMCONFIG3     (_LOWDATARATEOPTIMIZE_OFF | _AGC_AUTO_OFF)




#ifdef __SX1276__
    // include overrides from the app, if they are available
#   ifdef RF_OVERRIDES_H
#       include <app/rf_overrides.h>
#   endif
    // Any other overrides that are for general purpose operation go here
#   ifdef __EVAL_RF__
#       include <io/sx127x/SX1276_override.h>
#   endif
#endif


#endif
