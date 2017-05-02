/* Copyright 2013-2014 JP Norair
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
  * @file       /include/io/sx127x/sx1276_registers.h
  * @author     JP Norair
  * @version    R101
  * @date       4 Nov 2016
  * @brief      Register Addressing Constants for SX1276
  * @ingroup    SX127x
  *
  ******************************************************************************
  */

#ifndef __SX1276_registers_H
#define __SX1276_registers_H

/** Most registers get inherited from 1272 series.
  * Some get modified below.
  * Some get added below.
  */
#include "SX1272_registers.h"


//#define RFREG_LR_LNA                                0x0C 
#   define _LNA_BOOST_LF_ON                         (3<<3)
#   define _LNA_BOOST_LF_OFF                        (0<<3)
#   define _LNA_BOOST_HF_ON                         (3<<0)
#   define _LNA_BOOST_HF_OFF                        (0<<0)


//#define RFREG_LR_MODEMCONFIG1                       0x1D
#   undef _BW_MASK
#   undef _BW_125_KHZ
#   undef _BW_250_KHZ
#   undef _BW_500_KHZ
#   undef _CODINGRATE_MASK 
#   undef _CODINGRATE_4_5 
#   undef _CODINGRATE_4_6 
#   undef _CODINGRATE_4_7 
#   undef _CODINGRATE_4_8 
#   undef _IMPLICITHEADER_MASK 
#   undef _IMPLICITHEADER_ON 
#   undef _IMPLICITHEADER_OFF 
#   undef _RXPAYLOADCRC_MASK 
#   undef _RXPAYLOADCRC_ON 
#   undef _RXPAYLOADCRC_OFF 
#   undef _LOWDATARATEOPTIMIZE_MASK 
#   undef _LOWDATARATEOPTIMIZE_ON 
#   undef _LOWDATARATEOPTIMIZE_OFF
#   define _BW_MASK                                 0x0F 
#   define _BW_7K8_HZ                               (0<<4)
#   define _BW_10K4_HZ                              (1<<4)
#   define _BW_15K6_HZ                              (2<<4)
#   define _BW_20K8_HZ                              (3<<4)
#   define _BW_31K25_HZ                             (4<<4)
#   define _BW_41K7_HZ                              (5<<4)
#   define _BW_62K5_HZ                              (6<<4)
#   define _BW_125_KHZ                              (7<<4)
#   define _BW_250_KHZ                              (8<<4) 
#   define _BW_500_KHZ                              (9<<4) 
#   define _CODINGRATE_MASK                         0xF1 
#   define _CODINGRATE_4_5                          (1<<1)
#   define _CODINGRATE_4_6                          (2<<1) // Default
#   define _CODINGRATE_4_7                          (3<<1)
#   define _CODINGRATE_4_8                          (4<<1) 
#   define _IMPLICITHEADER_MASK                     0xFE 
#   define _IMPLICITHEADER_ON                       0x01 
#   define _IMPLICITHEADER_OFF                      0x00 // Default


//#define RFREG_LR_MODEMCONFIG2                       0x1E 
#   undef _AGC_ON 
#   undef _AGC_OFF 
#   define _RXPAYLOADCRC_MASK                       (~(1<<2)) 
#   define _RXPAYLOADCRC_ON                         (1<<2) 
#   define _RXPAYLOADCRC_OFF                        (0<<2) // Default



#define RFREG_LR_MODEMCONFIG3                       0x26 
#   define _LOWDATARATEOPTIMIZE_MASK                (~(1<<3))
#   define _LOWDATARATEOPTIMIZE_ON                  (1<<3) 
#   define _LOWDATARATEOPTIMIZE_OFF                 (0<<3) // Default
#   define _AGC_ON                                  (1<<2)
#   define _AGC_OFF                                 (0<<2)


#define RFREG_LR_PPMCORRECTION                      0x27 
#   define __PPMCORRECTION(VAL)                     (0xFF(&(VAL)))


#endif
