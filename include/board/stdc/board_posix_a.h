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
  * @version    R101
  * @date       24 Mar 2014
  * @brief      Board Configuration for POSIX Type A, on STD C libaries
  * @ingroup    Platform
  *
  * Do not include this file, include OT_platform.h
  ******************************************************************************
  */
  

#ifndef __board_posix_a_H
#define __board_posix_a_H

#include <app/build_config.h>
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
#define RF_HDB_ATTEN    3       //Half dB attenuation (units = 0.5dB), used to scale TX power
#define RF_HDB_RXATTEN  0
#define RF_RSSI_OFFSET  0       //Offset applied to RSSI calculation



/** MCU Feature settings      <BR>
  * ========================================================================<BR>
  *
  */
#define MCU_CONFIG(VAL)                 MCU_CONFIG_##VAL   // FEATURE 
#define MCU_CONFIG_MULTISPEED           DISABLED         // Allows usage of MF-HF clock boosting
#define MCU_CONFIG_MAPEEPROM            DISABLED
#define MCU_CONFIG_MPIPECDC             DISABLED        // USB-CDC MPipe implementation
#define MCU_CONFIG_MPIPEUART            DISABLED        // UART MPipe Implementation
#define MCU_CONFIG_MPIPEI2C             DISABLED        // I2C MPipe Implementation
#define MCU_CONFIG_MEMCPYDMA            DISABLED        // MEMCPY DMA should be lower priority than MPIPE DMA
#define MCU_CONFIG_USB                  DISABLED



/** Platform Memory Configuration <BR>
  * ========================================================================<BR>
  * OpenTag needs to know where it can put Nonvolatile memory (file system) and
  * how much space it can allocate for filesystem.  For this configuration, 
  * Veelite is put into FLASH.
  *
  * The STM32L uses 256 byte Flash pages and 4KB Flash sectors.  Therefore, it
  * is best to allocate the FS in Flash on 4KB boundaries because this is the 
  * resolution that can be write protected (or, as with FS, *NOT* write 
  * protected).  Best practice with STM32 chips is to put the FS at the back of
  * of the Flash space because this seems to work best with the debugger HW.
  */

#define SRAM_SIZE               (16*1024)
#define EEPROM_SIZE             (4*1024)
#define FLASH_SIZE              (64*1024)


#define FLASH_NUM_PAGES         (FLASH_SIZE/FLASH_PAGE_SIZE)
#define FLASH_FS_ADDR           FLASH_START_ADDR
#define FLASH_FS_PAGES          16
#define FLASH_FS_FALLOWS        3 
#define FLASH_FS_ALLOC          (FLASH_PAGE_SIZE*FLASH_FS_PAGES)





/** Board-based Feature Settings <BR>
  * ========================================================================<BR>
  */
#define BOARD_FEATURE(VAL)              BOARD_FEATURE_##VAL
#define BOARD_PARAM(VAL)                BOARD_PARAM_##VAL

#define BOARD_FEATURE_MPIPE             DISABLED
#define BOARD_FEATURE_USBCONVERTER      BOARD_FEATURE_MPIPE                 // Is UART connected via USB converter?
#define BOARD_FEATURE_MPIPE_DIRECT      BOARD_FEATURE_MPIPE
#define BOARD_FEATURE_MPIPE_BREAK       DISABLED                // Send/receive leading break for wakeup
#define BOARD_FEATURE_MPIPE_CS          DISABLED                // Chip-Select / DTR wakeup control
#define BOARD_FEATURE_MPIPE_FLOWCTL     DISABLED                // RTS/CTS style flow control







/** Note: Clocking for the Board's MCU      <BR>
  * ========================================================================<BR>
  */





/** Peripheral definitions for this platform <BR>
  * ========================================================================<BR>
  * OT_GPTIM:   General Purpose Timer used by OpenTag kernel                <BR>
  * OT_TRIG:    Optional test trigger usable in OpenTag apps (often LEDs)   <BR>
  * MPIPE:      UART to use for the MPipe                                   <BR>
  */
  
#define OT_GPTIM_ID         'R'
#define OT_GPTIM            RTC
#define OT_GPTIM_CLOCK      33300
#define OT_GPTIM_RES        1024
#define OT_GPTIM_SHIFT      0
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




* Abstracted Flash Organization: 
  * OpenTag uses Flash to store 2 kinds of data.  The default setup puts 
  * Filesystem memory in the back.
  * 1. Program code (obviously)
  * 2. Filesystem Memory
  *
  * FLASH_xxx constants are defined in the platform_config_xxx.h file.  

#define OTF_TOTAL_SIZE          FLASH_FS_ALLOC
#define OTF_START_PAGE          OTF_VWORM_START_PAGE
#define OTF_START_ADDR          FLASH_FS_ADDR

#define OTF_VWORM_LAST_PAGE     (OTF_VWORM_START_PAGE + OTF_VWORM_PAGES - 1)
#define OTF_VWORM_END_ADDR      (FLASH_FS_ADDR + FLASH_FS_ALLOC - 1)

*/


#endif
