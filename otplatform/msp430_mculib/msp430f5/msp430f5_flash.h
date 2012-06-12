/* Copyright 2009 JP Norair
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
/** @file       /OTplatform/~MCU_MSP430/MSP430F5/msp430f5_flash.h
  * @author     JP Norair
  * @version    V1.0
  * @date       15 Nov 2011
  * @brief      Library resources for Flash peripheral
  * @ingroup    MSP430F5 Library
  *
  * The flash memory is byte, word, and long-word addressable and programmable. 
  * The flash memory module has an integrated controller that controls 
  * programming and erase operations. The module contains three registers, a 
  * timing generator, and a voltage generator to supply program and erase 
  * voltages. The cumulative high-voltage time must not be exceeded, and each 
  * 32-bit word can be written not more than four times (in byte, word, or long 
  * word write modes) before another erase cycle (see device-specific datasheet
  * for details).
  * 
  * The flash memory features include: 
  * - Internal programming voltage generation
  * - Byte, word (2 bytes), and long (4 bytes) programmable
  * - Segment erase, bank erase (device specific), and mass erase
  * - Marginal 0 and marginal 1 read modes
  *
  *
  * @note 15 Nov 11 Update includes sections that block maskable interrupts
  * during runtime.  This is to protect interrupts from interfering with Flash
  * writes, per Errata FLASH31 of the MSP430F5 Errata list.  Errata FLASH29 and
  * FLASH33 (the other remaining errata) have been deemed superfluous.
  ******************************************************************************
  */


#ifndef __MSP430F5_LIB_FLASH_H
#define __MSP430F5_LIB_FLASH_H


#include "msp430f5_map.h"


typedef enum {
    None    = 0x0000,
    Segment = 0x0002,
    Bank    = 0x0004,
    All     = 0x0006
} FLASH_EraseType;


typedef enum {
    FLASH_OK        = 0,
    FLASH_Busy      = 0x01,
    FLASH_KeyError  = 0x02,
    FLASH_NotReady  = 0x08,
    FLASH_Locked    = 0x10,
    FLASH_ALocked   = 0x40,
    FLASH_VoltError = 0x80
} FlashStatus;


#define FLASH_COMPLETE          FLASH_OK


// FLASH Controller Registers
// FCTL1
// FCTL3
// FCTL4

#define FLASH_FRKEY             0x9600
#define FLASH_FWKEY             0xA500
#define FLASH_CTL_KEY           0xFF00
#define FLASH_CTL_BLKWRT        0x0080
#define FLASH_CTL_WRT           0x0040
#define FLASH_CTL_SWRT          0x0020
#define FLASH_CTL_MERAS         0x0004
#define FLASH_CTL_ERASE         0x0002

#define FLASH_CTL_LOCKA         0x0040
#define FLASH_CTL_LOCK          0x0010
#define FLASH_CTL_WAIT          0x0008
#define FLASH_CTL_ACCVIFG       0x0004
#define FLASH_CTL_KEYV          0x0002
#define FLASH_CTL_BUSY          0x0001

#define FLASH_CTL_LOCKINFO      0x0080
#define FLASH_CTL_MRG1          0x0020
#define FLASH_CTL_MRG0          0x0010
#define FLASH_CTL_VPE           0x0001

#define FLASH_SystemIT_ACCVIE   0x0010





// Pseudo Addresses in FLASH
#define FLASH_Base_BSL0         0x1000
#define FLASH_Base_BSL1         0x1200
#define FLASH_Base_BSL2         0x1400
#define FLASH_Base_BSL3         0x1600

#define FLASH_Base_TLVD         0x1A00
#define FLASH_Base_TLVC         0x1A80



// FLASH Segment Addresses (For MSP430F5 with 32 KB Flash)
#define FLASH_SegBase_InfoD     0x1800
#define FLASH_SegBase_InfoC     0x1880
#define FLASH_SegBase_InfoB     0x1900
#define FLASH_SegBase_InfoA     0x1980

#define FLASH_SegBase_Main0     0x8000
#define FLASH_SegBase_Main1     0x8200
#define FLASH_SegBase_Main2     0x8400
#define FLASH_SegBase_Main3     0x8600
#define FLASH_SegBase_Main4     0x8800
#define FLASH_SegBase_Main5     0x8A00
#define FLASH_SegBase_Main6     0x8C00
#define FLASH_SegBase_Main7     0x8E00

#define FLASH_SegBase_Main8     0x9000
#define FLASH_SegBase_Main9     0x9200
#define FLASH_SegBase_Main10    0x9400
#define FLASH_SegBase_Main11    0x9600
#define FLASH_SegBase_Main12    0x9800
#define FLASH_SegBase_Main13    0x9A00
#define FLASH_SegBase_Main14    0x9C00
#define FLASH_SegBase_Main15    0x9E00

#define FLASH_SegBase_Main16    0xA000
#define FLASH_SegBase_Main17    0xA200
#define FLASH_SegBase_Main18    0xA400
#define FLASH_SegBase_Main19    0xA600
#define FLASH_SegBase_Main20    0xA800
#define FLASH_SegBase_Main21    0xAA00
#define FLASH_SegBase_Main22    0xAC00
#define FLASH_SegBase_Main23    0xAE00

#define FLASH_SegBase_Main24    0xB000
#define FLASH_SegBase_Main25    0xB200
#define FLASH_SegBase_Main26    0xB400
#define FLASH_SegBase_Main27    0xB600
#define FLASH_SegBase_Main28    0xB800
#define FLASH_SegBase_Main29    0xBA00
#define FLASH_SegBase_Main30    0xBC00
#define FLASH_SegBase_Main31    0xBE00

#define FLASH_SegBase_Main32    0xC000
#define FLASH_SegBase_Main33    0xC200
#define FLASH_SegBase_Main34    0xC400
#define FLASH_SegBase_Main35    0xC600
#define FLASH_SegBase_Main36    0xC800
#define FLASH_SegBase_Main37    0xCA00
#define FLASH_SegBase_Main38    0xCC00
#define FLASH_SegBase_Main39    0xCE00

#define FLASH_SegBase_Main40    0xD000
#define FLASH_SegBase_Main41    0xD200
#define FLASH_SegBase_Main42    0xD400
#define FLASH_SegBase_Main43    0xD600
#define FLASH_SegBase_Main44    0xD800
#define FLASH_SegBase_Main45    0xDA00
#define FLASH_SegBase_Main46    0xDC00
#define FLASH_SegBase_Main47    0xDE00

#define FLASH_SegBase_Main48    0xE000
#define FLASH_SegBase_Main49    0xE200
#define FLASH_SegBase_Main50    0xE400
#define FLASH_SegBase_Main51    0xE600
#define FLASH_SegBase_Main52    0xE800
#define FLASH_SegBase_Main53    0xEA00
#define FLASH_SegBase_Main54    0xEC00
#define FLASH_SegBase_Main55    0xEE00

#define FLASH_SegBase_Main56    0xF000
#define FLASH_SegBase_Main57    0xF200
#define FLASH_SegBase_Main58    0xF400
#define FLASH_SegBase_Main59    0xF600
#define FLASH_SegBase_Main60    0xF800
#define FLASH_SegBase_Main61    0xFA00
#define FLASH_SegBase_Main62    0xFC00
#define FLASH_SegBase_Main63    0xFE00



// FLASH Bank Addresses
#define FLASH_BankBase_Main0    FLASH_SegBase_Main0







/** @brief Enable/Disable the Flash controller's single interrupt (Access Violation)
  * @param NewState : (FunctionalState) ENABLE or DISABLE
  * @retval none
  * @ingroup MSP430F5_lib
  */
void FLASH_ITConfig( FunctionalState NewState );


/** @brief Check Flash controller's single interrupt (Access Violation)
  * @param none
  * @retval bool : TRUE if set
  * @ingroup MSP430F5_lib
  */
bool FLASH_GetITStatus( );



/** @brief Clear the Flash controller's single interrupt (Access Violation)
  * @param none
  * @retval none
  * @ingroup MSP430F5_lib
  */
void FLASH_ClearITPendingBit( );



/** @brief Get the status of the Flash Controller
  * @param none
  * @retval u8 : Logical OR'ed group of status flags as described in FlashStatus enum datatype
  * @ingroup MSP430F5_lib
  */
u8 FLASH_GetStatus( );

u8 FLASH_GetLockStatus( );



/** @brief Clears all Flash status flags that are clearable by software
  * @param none
  * @retval none
  * @ingroup MSP430F5_lib
  *
  * Clearable Flags are:
  * KEYV (Key Violation)
  * VPE (Voltage Programming Error)
  *
  * Use the unlock functions to clear LOCK flags
  */
void FLASH_ClearStatus( );



/** @brief Completely Erases a single Segment of Flash
  * @param SegAddress : (u16*) absolute address in memory of the segment to erase
  * @retval u8 : Logical OR'ed group of status flags as described in FlashStatus enum datatype
  * @ingroup MSP430F5_lib
  *
  * If FLASH_EraseSegment returns anything other than 0, an error has occurred
  * and the Flash segment cannot be guaranteed erased. 
  */
u8 FLASH_EraseSegment( u16* SegAddress );



/** @brief Completely Erases a single Segment of Flash
  * @param BankAddress : (u16*) absolute address in memory of the bank to erase
  * @retval u8 : Logical OR'ed group of status flags as described in FlashStatus enum datatype
  * @ingroup MSP430F5_lib
  *
  * If FLASH_EraseBank returns anything other than 0, an error has occurred
  * and the Flash Bank cannot be guaranteed erased. 
  */
u8 FLASH_EraseBank( u16* BankAddress );



/** @brief Completely Erases Flash
  * @retval u8 : Logical OR'ed group of status flags as described in FlashStatus enum datatype
  * @ingroup MSP430F5_lib
  *
  * If FLASH_EraseAll returns anything other than 0, an error has occurred
  * and the Flash cannot be guaranteed erased. 
  */
u8 FLASH_EraseAll( );



/** @brief Writes a single byte to an address in Flash, from code running in Flash
  * @param Address : (u8*) An address in Flash
  * @param Data : (u8*) A byte to write to Flash
  * @param Length : (u16) Number of Bytes to write in sequence
  * @retval u8 : Logical OR'ed group of status flags as described in FlashStatus enum datatype
  * @ingroup MSP430F5_lib
  *
  * MSP430F5 Flash can be written to four times before it must be erased and 
  * rewritten!  The user must keep track of this, or simply erase each time.
  */
u8 FLASH_WriteByteBlock( u8* Address, u8* Data, u16 Length );
u8 FLASH_WriteByte( u8* Address, u8 Data );


/** @brief Writes a Short (two bytes) to an address in Flash, from code running in Flash
  * @param Address : (u16*) An address in Flash -- must be even
  * @param Data : (u16*) A Short to write to Flash
  * @param Length : (u16) Number of Shorts to write in sequence
  * @retval u8 : Logical OR'ed group of status flags as described in FlashStatus enum datatype
  * @ingroup MSP430F5_lib
  *
  * MSP430F5 Flash can be written to four times before it must be erased and 
  * rewritten!  The user must keep track of this, or simply erase each time.
  *
  * If the address supplied is not even, behavior is unpredictable.
  */
u8 FLASH_WriteShortBlock( u16* Address, u16* Data, u16 Length );
u8 FLASH_WriteShort( u16* Address, u16 Data );


/** @brief Writes Longs (4 bytes) to an address in Flash, from code running in Flash
  * @param Address : (u16*) An address in Flash -- must be 4-byte aligned
  * @param Data : (u32*) A Long to write to Flash
  * @param Length : (u16) Number of Longs to write in sequence
  * @retval u8 : Logical OR'ed group of status flags as described in FlashStatus enum datatype
  * @ingroup MSP430F5_lib
  *
  * MSP430F5 Flash can be written to four times before it must be erased and 
  * rewritten!  The user must keep track of this, or simply erase each time.
  *
  * If the address is not 4-byte aligned, behavior is unpredictable
  */
u8 FLASH_WriteLongBlock( u32* Address, u32* Data, u16 Length);
u8 FLASH_WriteLong( u32* Address, u32 Data );



/** @brief Writes an array of Longs (4 bytes) to an address in Flash, from code running in RAM
  * @param Address : (u32*) An address in Flash -- must be 4-byte aligned
  * @param Data : (u32*) An arroy of Longs to write to Flash, sequentially
  * @param Length : (u16) Number of Longs to write in sequence
  * @retval u8 : Logical OR'ed group of status flags as described in FlashStatus enum datatype
  * @ingroup MSP430F5_lib
  *
  * MSP430F5 Flash can be written to four times before it must be erased and 
  * rewritten!  The user must keep track of this, or simply erase each time.
  *
  * If the address is not 4-byte aligned, behavior is unpredictable
  */
u8 FLASH_ProgramBlock( u32* Address, u32* Data, u16 Length);



#endif

