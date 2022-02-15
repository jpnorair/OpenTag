/* Copyright 2010-2013 JP Norair
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
  * @file       /otplatform/stm32l0xx/veelite_core_EE_STM32L1xx.c
  * @author     JP Norair
  * @version    R100
  * @date       17 Jan 2013
  * @brief      EE [EEPROM] Method for Veelite Core Functions
  * @ingroup    Veelite
  *
  * 
  * Summary:
  * This module is part of the Veelite Core, which contains the low-level read
  * and write filesystem functionality.  This variant is meant for systems that
  * use EEPROM as the source of non-volatile memory, or at least something
  * functionally similar.  STM32L, for example, seems to emulate EEPROM with
  * NOR flash in a transparent way.
  *
  ******************************************************************************
  */


#include <otplatform.h>
#if defined(__STM32L0xx__) && (!defined(__VLSRAM__) || defined(__VLEE__))

#include "stm32l0xx_hal.h"

#include <otlib/logger.h>
#include <otsys/veelite_core.h>

#ifndef OT_FEATURE_VLNVWRITE
#   define OT_FEATURE_VLNVWRITE ENABLED
#endif


// SCB_SHCSR_USGFAULTENA_Msk
// SCB_SHCSR_USGFAULTPENDED_Msk
// SCB_SHCSR_USGFAULTACT_Msk

/// VLEE Debugging
/// This driver is quite stable, so debugging features are not implemented
/// even when __DEBUG__ is active
#ifdef __DEBUG__
//#   define _VLEE_DEBUG
#endif

/// Set Segmentation Fault (code 11) if trying to access an invalid virtual
/// address.  Vector to NMI
#if (defined _VLEE_DEBUG)
#   define SEGFAULT_CHECK(ADDR, BANK, MSGLEN, MSG) \
        do { \
            if (vas_check(ADDR) != BANK) { \
                platform.error_code = 11; \
                SCB->ICSR |= SCB_ICSR_NMIPENDSET_Msk; \
            } \
        } while (0)
#else
#   define SEGFAULT_CHECK(ADDR, BANK, MSGLEN, MSG); 
#endif



/// Set Bus Error (code 7) on physical flash access faults (X2table errors).
/// Vector to Access Violation ISR 
#if defined(_VLEE_DEBUG)
#   define BUSERROR_CHECK(EXPR, MSGLEN, MSG) \
        do { \
            if (EXPR) { \
                platform.error_code = 7; \
                SCB->ICSR |= SCB_ICSR_NMIPENDSET_Msk; \
            } \
        } while (0)
#else
#   define BUSERROR_CHECK(EXPR, MSGLEN, MSG);
#endif



/// VSRAM (Mirror) memory buffer
#if (VSRAM_SIZE > 0)
    ot_u16 vsram[ (VSRAM_SIZE/2) ];
#endif



/** Fixit functions for bloated STM32 Cube FW
 * =================================================
 */
HAL_StatusTypeDef sub_WaitForLastOperation(ot_long Timeout) {
// Wait for the FLASH operation to complete by polling on BUSY flag to be reset.
// Even if the FLASH operation fails, the BUSY flag will be reset and an error
// flag will be set
    uint16_t tickstart = systim_get_clocker();
    uint32_t flash_sr_errors;
    
    while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET) { 
        Timeout -= (systim_get_clocker() - tickstart);
        if (Timeout <= 0) {
            return HAL_TIMEOUT;
        }
    }

    // Check FLASH End of Operation flag, clear if set
    if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP)) {
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
    }
    
    // Look for errors, and if necessary save the error code to ProcFlash
    // WARNING : On the first cut of STM32L031xx and STM32L041xx devices,
    //           (RefID = 0x1000) the FLASH_FLAG_OPTVERR bit was not behaving
    //           as expected. If the user run an application using the first
    //           cut of the STM32L031xx device or the first cut of the STM32L041xx
    //           device, this error should be ignored. The revId of the device
    //           can be retrieved via the HAL_GetREVID() function.
    flash_sr_errors = FLASH->SR & \
                     (FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR \
                    | FLASH_FLAG_OPTVERR | FLASH_FLAG_RDERR | FLASH_FLAG_FWWERR \
                    | FLASH_FLAG_NOTZEROERR);
    if (flash_sr_errors) {
        ProcFlash.ErrorCode = flash_sr_errors;
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR |
                                 FLASH_FLAG_OPTVERR | FLASH_FLAG_RDERR | FLASH_FLAG_FWWERR | 
                                 FLASH_FLAG_NOTZEROERR);
        return HAL_ERROR;
    }

    // There is no error flag set 
    return HAL_OK;  
}
    
    

/** Data Types for Veelite <BR>
  * ========================================================================<BR>
  */






/** Generic Veelite Core Function Implementations <BR>
  * ========================================================================<BR>
  * Used for any and all memory topologies
  */

vas_loc vas_check(vaddr addr) {
    ot_s32 scratch;
    
    scratch = addr - VWORM_BASE_VADDR;
    if ((scratch >= 0) && (scratch < VWORM_SIZE))   return in_vworm;
    
    scratch = addr - VSRAM_BASE_VADDR;
    if ((scratch >= 0) && (scratch < VSRAM_SIZE))   return in_vsram;

    return vas_error;
}





/** VWORM Functions <BR>
  * ========================================================================<BR>
  */
///@note If using GCC, you should be using the "KEEP" option in your linker
///      script to make sure the linker does not discard the filesystem data as
///      part of optimization.  If you still can't implement KEEP, you can set 
///      this define (below), which will run a routine during init that touches 
///      all the FS arrays.   

#ifndef EXTF_vworm_format
ot_u8 vworm_format( ) {
#if ((VWORM_SIZE > 0) && (OT_FEATURE(VLNVWRITE) == ENABLED))
    ot_u32      cursor;
    ot_u8       output = 0;

    /// 1. Load default cursor (using embedded method)
    cursor = (ot_u32)(FLASH_FS_ADDR);

    /// 2. Format all Blocks, Put Block IDs into Primary Blocks
    while (cursor != (FLASH_FS_ADDR+FLASH_FS_ALLOC)) {
        output |= HAL_FLASHEx_DATAEEPROM_Erase(cursor);
        cursor += 4;
    }

    return output;
#else
    return 0;
#endif
}
#endif


#ifndef EXTF_vworm_init
ot_u8 vworm_init( ) {
    // Minimal initialization needed for EEPROM
    HAL_FLASHEx_DATAEEPROM_Unlock();
    FLASH->SR = 0xF00;
    
    ///@note may want to do initialization for vprom here, hard to say
    
    return 0;
}
#endif



#ifndef EXTF_vworm_print_table
void vworm_print_table() {
#ifdef _VLEE_DEBUG
// No table for EEPROM
#endif
}
#endif



#ifndef EXTF_vworm_save
ot_u8 vworm_save( ) {
#if ((VWORM_SIZE > 0) && (OT_FEATURE_VLNVWRITE == ENABLED))
    HAL_FLASHEx_DATAEEPROM_Lock();
#endif
    return 0;
}
#endif



#ifndef EXTF_vworm_read
ot_u16 vworm_read(vaddr addr) {
#if ((VWORM_SIZE > 0) && (OT_FEATURE(VLNVWRITE) == ENABLED))
    ot_u32  paddr;

    SEGFAULT_CHECK(addr, in_vworm, 7, "VLC_" __LINE__);   //__LINE__

    paddr   = (addr+VWORM_BASE_PHYSICAL);
    return *((ot_u16*)paddr);

#else
    return 0;
#endif
}
#endif



#ifndef EXTF_vworm_write
ot_u8 vworm_write(vaddr addr, ot_u16 data) {
#if ((VWORM_SIZE > 0) && (OT_FEATURE_VLNVWRITE == ENABLED))
    SEGFAULT_CHECK(addr, in_vworm, 7, "VLC_" __LINE__);   //__LINE__

    return vworm_mark_physical( (ot_u16*)(((ot_u32)addr)+VWORM_BASE_PHYSICAL), data );
#else
    return 0;
#endif
}
#endif
    


#ifndef EXTF_vworm_mark
ot_u8 vworm_mark(vaddr addr, ot_u16 value) {
    return vworm_write(addr, value);
}
#endif





#ifndef EXTF_vworm_mark_physical
ot_u8 vworm_mark_physical(ot_u16* addr, ot_u16 value) {
#if ((VWORM_SIZE > 0) && (OT_FEATURE_VLNVWRITE == ENABLED))
    ot_u8 retval;
    

    BUSERROR_CHECK( (((ot_u32)addr < VWORM_BASE_PHYSICAL) || \
                    ((ot_u32)addr >= (VWORM_BASE_PHYSICAL+VWORM_ALLOC))), 7, "VLC_" __LINE__);    //__LINE__

    //return (ot_u8)FLASH_DATAEEPROM_ProgramHalfWord( (uint32_t)addr, (uint16_t)value) );
    
    ///@todo don't allow task switching during this process
    *(__IO ot_u16*)addr = value;
    retval              = (ot_u8)sub_WaitForLastOperation((uint32_t)500);
    return retval;
    
#else
    return 0;
#endif
}
#endif



#ifndef EXTF_vworm_wipeblock
ot_u8 vworm_wipeblock(vaddr addr, ot_uint wipe_span) {
#if ((VWORM_SIZE > 0) && (OT_FEATURE_VLNVWRITE == ENABLED))
    ot_u8 output = 0;

    wipe_span += addr;
    for (; ((addr < (vaddr)wipe_span) && (output == 0)); addr+=2) {
        output |= vworm_write(addr, NULL_vaddr);
    }
    
    return output;
#else
    return 0;
#endif
}
#endif





/** VSRAM Functions <BR>
  * ========================================================================<BR>
  * VSRAM is held in RAM.  It is for mirrored ISF's.
  */

#ifndef EXTF_vsram_read
ot_u16 vsram_read(vaddr addr) {
#if (VSRAM_SIZE <= 0)
    return 0;
#else
    SEGFAULT_CHECK(addr, in_vsram, 7, "VLC_" __LINE__);   //__LINE__
    addr -= VSRAM_BASE_VADDR;
    return vsram[addr>>1];
#endif
}
#endif



#ifndef EXTF_vsram_mark
ot_u8 vsram_mark(vaddr addr, ot_u16 value) {
#if (VSRAM_SIZE <= 0)
    return ~0;
#else
    SEGFAULT_CHECK(addr, in_vsram, 7, "VLC_" __LINE__);   //__LINE__
    addr           -= VSRAM_BASE_VADDR;
    vsram[addr>>1]  = value;
    return 0;
#endif
}
#endif


#ifndef EXTF_vsram_mark_physical
ot_u8 vsram_mark_physical(ot_u16* addr, ot_u16 value) {
#if (VSRAM_SIZE <= 0)
    return ~0;
#else
    *addr = value;
    return 0;
#endif
}
#endif



#ifndef EXTF_vsram_get
ot_u8* vsram_get(vaddr addr) {
#if (VSRAM_SIZE <= 0)
    return NULL;
#else
    ot_u8* output;
    SEGFAULT_CHECK(addr, in_vsram, 7, "VLC_" __LINE__);
    addr   -= VSRAM_BASE_VADDR;
    output  = (ot_u8*)vsram + addr;
    return output;
#endif
}
#endif





/** VPROM Functions <BR>
  * ========================================================================<BR>
  * Real files should be stored in VWORM.  VPROM is meant for one thing and one 
  * thing only: A/B firmware uploads.
  */

#define _FLASH_END  (FLASH_START_ADDR + FLASH_AVAILABLE)
ot_u32 _vprom_base;


#ifndef EXTF_vprom_init
OT_WEAK ot_u8 vprom_init(ot_u32 base) {
/// vprom can be initialized to different locations in FLASH, in order to 
/// permit A/B firmware uploads, which is the main point of vprom.
    if ((base >= FLASH_START_ADDR) && (base < _FLASH_END)) {
        _vprom_base = base;
        return 0;
    }
    return 255;
}
#endif


#ifndef EXTF_vprom_read
OT_WEAK ot_u16 vprom_read(vaddr addr) {
/// Resolve the address and read the data -- simple
#if (VPROM_SIZE <= 0)
    return 0;
#else
    ot_u32 paddr;
    paddr = _vprom_base + addr;
    return *(ot_u16*)paddr;
#endif
}
#endif


#ifndef EXTF_vprom_get
OT_WEAK ot_u8* vprom_get(vaddr addr) {
/// Resolve the address and pass-back the pointer -- simple
#if (VPROM_SIZE <= 0)
    return NULL;
#else
    ot_u32 paddr;
    paddr = _vprom_base + addr;
    return (ot_u8*)paddr;
#endif
}
#endif


#ifndef EXTF_vprom_write
OT_WEAK ot_u8  vprom_write(vaddr addr, ot_u16 value) {
/// 1. Resolve actual address from virtual address -- simple
/// 2. If the address is at the start of the block, erase the block
/// 3. Write the value to the place it must go
#if (VPROM_SIZE <= 0)
    return ~0;
#else
    static ot_u32   word;       // storage for word-write using halfwords
    ot_u32          paddr;
    FLASH_Status    err;
    ot_int          offset;
    
    err     = FLASH_COMPLETE;
    paddr   = _vprom_base + addr;
    
    // STM32L0 uses 128 byte flash pages.  Erase page if on page-start address
    if ((paddr & (FLASH_PAGE_SIZE-1)) == 0) {
        err = FLASH_Erase_Page(paddr);
    }
    
    offset = (paddr & 2);
    *(((ot_u8*)&word) + offset) = value;
    
    // Prepare the word and write it, but only when a full word is supplied
    // We also do unlocking and locking immediately around the write, because 
    // there is some importance in protecting flash memory.
    if (offset != 0) {
        FLASH->PRGKEYR  = FLASH_PRGKEY1;
        FLASH->PRGKEYR  = FLASH_PRGKEY2;  
        err             = FLASH_Program_Word(paddr, word);
        FLASH->PECR    |= FLASH_PECR_PRGLOCK;
    }
    
    return err;
#endif
}
#endif



#endif //from top
