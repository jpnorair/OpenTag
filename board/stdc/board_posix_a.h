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
  * @file       /board/stdc/board_posix_a.h
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2012
  * @brief      Board Configuration for POSIX Type A, on STD C libaries
  * @ingroup    Platform
  *
  * Do not include this file, include OT_platform.h
  ******************************************************************************
  */
  

#ifndef __board_posix_a_H
#define __board_posix_a_H

#include "build_config.h"
#include "platform_stdc.h"

#include "radio_null.h"




/// Macro settings: ENABLED, DISABLED, NOT_AVAILABLE
#ifdef ENABLED
#   undef ENABLED
#endif
#define ENABLED  1

#ifdef DISABLED
#   undef DISABLED
#endif
#define DISABLED  0

#ifdef NOT_AVAILABLE
#   undef NOT_AVAILABLE
#endif
#define NOT_AVAILABLE   DISABLED




/** Additional RF Front End Parameters and Settings <BR>
  * ========================================================================<BR>
  */
#define RF_PARAM_BAND   433
#define RF_HDB_ATTEN    6       //Half dB attenuation (units = 0.5dB), used to scale TX power
#define RF_RSSI_OFFSET  3       //Offset applied to RSSI calculation



/** MCU Feature settings      <BR>
  * ========================================================================<BR>
  *
  */
#define MCU_FEATURE(VAL)                MCU_FEATURE_##VAL       // FEATURE                  AVAILABILITY
#define MCU_FEATURE_CRC                 DISABLED                 // CCITT CRC16              Low
#define MCU_FEATURE_AES128              DISABLED                 // AES128 engine            Moderate
#define MCU_FEATURE_ECC                 DISABLED                // ECC engine               Low
#define MCU_FEATURE_RADIODMA            DISABLED
#define MCU_FEATURE_RADIODMA_TXBYTES    0
#define MCU_FEATURE_RADIODMA_RXBYTES    0
#define MCU_FEATURE_MAPEEPROM           DISABLED
#define MCU_FEATURE_MPIPEDMA            DISABLED      // MPipe is only useful for debug mode
#define MCU_FEATURE_MEMCPYDMA           DISABLED

#define MCU_PARAM(VAL)                  MCU_PARAM_##VAL
#define MCU_PARAM_POINTERSIZE           sizeof(void*)

#define PLATFORM_FEATURE_USBCONVERTER    ENABLED







/** Board-based Feature Settings <BR>
  * ========================================================================<BR>
  */
#define BOARD_FEATURE(VAL)              BOARD_FEATURE_##VAL
#define BOARD_FEATURE_USBCONVERTER      ENABLED                 // Is UART connected via USB converter?
#define BOARD_FEATURE_MPIPE_QMGMT		ENABLED






/** Platform Memory Configuration <BR>
  * ========================================================================<BR>
  * OpenTag needs to know where it can put Nonvolatile memory (file system) and
  * how much space it can allocate for filesystem.
  */
#define SRAM_START_ADDR             0x0000
#define SRAM_SIZE                   (16*1024)
#define EEPROM_START_ADDR           0
#define EEPROM_SIZE                 0
#define FLASH_START_ADDR            0x8000
#define FLASH_START_PAGE            0
#define FLASH_PAGE_SIZE             512
#define FLASH_WORD_BYTES            2
#define FLASH_WORD_BITS             (FLASH_WORD_BYTES*8)
#define FLASH_NUM_PAGES             64
#define FLASH_FS_PAGES              8
#define FLASH_FS_ALLOC              (FLASH_PAGE_SIZE*FLASH_FS_PAGES)     //allocating total of 8 blocks (4KB)
#define FLASH_FS_ADDR               0x8000
#define FLASH_FS_FALLOWS            3

#define FLASH_PAGE_ADDR(VAL)    (FLASH_START_ADDR + ( (VAL) * FLASH_PAGE_SIZE) )









/** Note: Clocking for the Board's MCU      <BR>
  * ========================================================================<BR>
  */





/** Peripheral definitions for this platform <BR>
  * ========================================================================<BR>
  * OT_GPTIM:   General Purpose Timer used by OpenTag kernel                <BR>
  * OT_TRIG:    Optional test trigger usable in OpenTag apps (often LEDs)   <BR>
  * MPIPE:      UART to use for the MPipe                                   <BR>
  */
  
#define OT_GPTIM            TIM1A3
#define OT_GPTIM_IRQ        TIM1A3_IRQChannel
#define OT_GPTIM_VECTOR     TIMER1_A1_VECTOR
#define OT_GPTIM_CLOCK      32768
#define OT_GPTIM_RES        1024
#define TI_TO_CLK(VAL)      ((OT_GPTIM_RES/1024)*VAL)
#define CLK_TO_TI(VAL)      (VAL/(OT_GPTIM_RES/1024))

#define OT_GPTIM_ERRDIV     32768 //this needs to be hard-coded, or else CCS shits in its pants







/** Basic GPIO Setup <BR>
  * ========================================================================<BR>
  * <LI>MPipe is a UART-based serial interface.  It needs to be specified with
  *     a UART peripheral number and RX/TX port & pin configurations. </LI>
  * <LI>OT Triggers are test outputs, usually LEDs.  They should be configured
  *     to ports & pins where there are LED or other trigger connections.</LI>
  * <LI>The GWN feature is part of a true-random-number generator.  It is
  *     optional.  It needs a port & pin for the ADC input (GWNADC) and also,
  *     optionally, one for a Zener driving output port & pin.  Using a Zener
  *     can greatly accelerate the random number generation process.</LI>
  */




/******* ALL SHIT BELOW HERE IS SUBJECT TO REDEFINITION **********/




/** Flash Memory Setup: 
  * "OTF" means "Open Tag Flash," but if flash is not used, it just means 
  * storage memory.  Unfortunately this does not begin with F.
  */
#define OTF_VWORM_PAGES         (FLASH_FS_ALLOC/FLASH_PAGE_SIZE)
#define OTF_VWORM_FALLOW_PAGES  3
#define OTF_VWORM_PAGESIZE      FLASH_PAGE_SIZE
#define OTF_VWORM_WORD_BYTES    FLASH_WORD_BYTES
#define OTF_VWORM_WORD_BITS     FLASH_WORD_BITS
#define OTF_VWORM_SIZE          (OTF_VWORM_PAGES * OTF_VWORM_PAGESIZE)
#define OTF_VWORM_START_PAGE    ((FLASH_FS_ADDR-FLASH_START_ADDR)/FLASH_PAGE_SIZE)
#define OTF_VWORM_START_ADDR    FLASH_FS_ADDR

#define OTF_CRC_TABLE           DISABLED
#define OTF_UHF_TABLE           DISABLED
#define OTF_UHF_TABLESIZE       0
#define OTF_M1_ENCODE_TABLE     DISABLED
#define OTF_M2_ENCODE_TABLE     ENABLED

// Total number of pages taken from program memory
#define OTF_TOTAL_PAGES         (OTF_VWORM_PAGES)




/** Abstracted Flash Organization: 
  * OpenTag uses Flash to store 2 kinds of data.  The default setup puts 
  * Filesystem memory in the back.
  * 1. Program code (obviously)
  * 2. Filesystem Memory
  *
  * FLASH_xxx constants are defined in the platform_config_xxx.h file.  
  */
#define OTF_TOTAL_SIZE          FLASH_FS_ALLOC
#define OTF_START_PAGE          OTF_VWORM_START_PAGE
#define OTF_START_ADDR          FLASH_FS_ADDR

#define OTF_VWORM_LAST_PAGE     (OTF_VWORM_START_PAGE + OTF_VWORM_PAGES - 1)
#define OTF_VWORM_END_ADDR      (FLASH_FS_ADDR + FLASH_FS_ALLOC - 1)




#endif
