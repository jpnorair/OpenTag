/*  Copyright 2008-2022, JP Norair
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted provided that the following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright 
  *    notice, this list of conditions and the following disclaimer in the 
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
  * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
  * POSSIBILITY OF SUCH DAMAGE.
  */
/**
  * @file       /platform/stm32l4xx/otsys_veelite_SRAM.c
  * @author     JP Norair
  * @version    R100
  * @date       30 Dec 2019
  * @brief      SRAM Method for Veelite Core Functions
  * @ingroup    Veelite
  *
  * @note This implementation also works as-is with most other STM32 devices, 
  *       although FLASH_...() library functions used within will need to be 
  *       linked to the respective platform library (L0, L1, F0, F1, etc).
  * 
  * Summary:
  * This module is part of the Veelite Core, which contains the low-level read
  * and write filesystem functionality.  This variant is meant for systems that
  * use NAND FLASH as the source of non-volatile memory, and which have enough
  * SRAM to load the entire filesystem into a part of it.  At power down or
  * reset conditions, the contents of the Filesystem SRAM will be written back
  * to Flash.
  *
  ******************************************************************************
  */

#include <otplatform.h>
#if defined(__STM32WLxx__) && (defined(__NOEEPROM__) || defined(__VLSRAM__))

#include "stm32wlxx_hal_1.11.0/stm32wlxx_hal.h"

#include <otsys/veelite_core.h>
#include <otlib/logger.h>
#include <otlib/memcpy.h>


#if (CC_SUPPORT != GCC)
#   error "GCC required for this linker featureset."
#endif
#pragma DATA_SECTION(fsram, ".vl_sram")
static ot_u32 fsram[FLASH_FS_ALLOC/4];

#define FSRAM ((ot_u16*)fsram)




/// VLX2 Debugging
/// This driver is quite stable, so debugging features are not implemented
/// even when __DEBUG__ is active
#ifdef __DEBUG__
//#   define _VLSRAM_DEBUG
#endif

/// Set Segmentation Fault (code 11) if trying to access an invalid virtual
/// address.  Vector to NMI
#if (defined _VLSRAM_DEBUG)
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
#if defined(_VLSRAM_DEBUG)
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

#ifndef EXTF_vworm_format
ot_u8 vworm_format( ) {
    return 0;
}
#endif


#ifndef EXTF_vworm_fsalloc
ot_u32 vworm_fsalloc(const vlFSHEADER* fs) {
    ot_u32 alloc;

    if (fs == NULL) {
        alloc   = 0;
    }
    else {
        alloc   = fs->ftab_alloc;
        alloc  += fs->gfb.alloc;
        alloc  += fs->iss.alloc;
        alloc  += fs->isf.alloc;
    }

    return alloc;
}
#endif


#ifndef EXTF_vworm_init
ot_u8 vworm_init() {
    ot_memcpy_4(fsram, (ot_u32*)FLASH_FS_ADDR, sizeof(fsram)/4);
    return 0;
}
#endif


#ifndef EXTF_vworm_print_table
void vworm_print_table() {
}
#endif



#ifndef EXTF_vworm_save
ot_u8 vworm_save( ) {
/// Save data from fsram into FLASH
#   define FLASH_ROWSIZE 256
    void*   cursor_ram;
    void*   limit_ram;
    uint32_t    cursor_fl;

    HAL_StatusTypeDef status;

    
    HAL_FLASH_Unlock();
    
    // Erase FS that's currently backed-up in flash
    for (cursor_fl=FLASH_FS_PAGE0; cursor_fl<(FLASH_FS_PAGE0+FLASH_FS_PAGES); cursor_fl++) {
        FLASH_PageErase(cursor_fl);
    }

    // Do fast program, which programs 256 bytes at a time (32 u64's)
    for (cursor_ram=&fsram[0], limit_ram=((void*)fsram+sizeof(fsram)-FLASH_ROWSIZE), cursor_fl=FLASH_FS_ADDR;
        (cursor_fl<(FLASH_FS_END-FLASH_ROWSIZE)) && (cursor_ram<limit_ram);
        cursor_fl+=256, cursor_ram+=256) {

        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FAST, cursor_fl, (uint32_t)cursor_ram);
        if (status != HAL_OK) {
            goto vworm_save_EXIT;
        }
    }
    ///@note on L4 devices the 1st parameter is "FLASH_TYPEPROGRAM_FAST_AND_LAST"
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FAST, cursor_fl, (uint32_t)cursor_ram);

    ///@todo could log a hardware fault of some kind here, if status fails
    vworm_save_EXIT:
    HAL_FLASH_Lock();
    return (status == HAL_OK) ? 0 : 255;
}
#endif



#ifndef EXTF_vworm_read
ot_u16 vworm_read(vaddr addr) {
    ot_u16* data;
    addr   -= VWORM_BASE_VADDR;
    addr   &= ~1;
    data    = (ot_u16*)((ot_u8*)fsram + addr);
    return *data;
}
#endif



#ifndef EXTF_vworm_write
ot_u8 vworm_write(vaddr addr, ot_u16 data) {
    ot_u16* aptr;
    addr   -= VWORM_BASE_VADDR;
    addr   &= ~1;
    aptr    = (ot_u16*)((ot_u8*)fsram + addr);
    *aptr   = data;
    return 0;
}
#endif
    


#ifndef EXTF_vworm_mark
ot_u8 vworm_mark(vaddr addr, ot_u16 value) {
    return vworm_write(addr, value);
}
#endif



#ifndef EXTF_vworm_mark_physical
ot_u8 vworm_mark_physical(ot_u16* addr, ot_u16 value) {
#   if ((VWORM_SIZE > 0) && (OT_FEATURE_VLNVWRITE == ENABLED))
    BUSERROR_CHECK( (((ot_u32)addr < (ot_u32)fsram) || \
                    ((ot_u32)addr >= (ot_u32)(&fsram[FLASH_FS_ALLOC/4]))), 7, "VLC_"__LINE__);
                    
    *addr = value;
#   endif
    return 0;
}
#endif

#ifndef EXTF_vworm_get
ot_u8* vworm_get(vaddr addr) {
    addr -= VWORM_BASE_VADDR;
    return (ot_u8*)fsram + addr;
}
#endif

#ifndef EXTF_vworm_wipeblock
ot_u8 vworm_wipeblock(vaddr addr, ot_uint wipe_span) {
    return 0;
}
#endif





/** VSRAM Functions <BR>
  * ========================================================================<BR>
  * SRAM version of Veelite doesn't utilize VSRAM
  */

#ifndef EXTF_vsram_read
ot_u16 vsram_read(vaddr addr) {
    return vworm_read(addr);
}
#endif

#ifndef EXTF_vsram_mark
ot_u8 vsram_mark(vaddr addr, ot_u16 value) {
    return vworm_mark(addr, value);
}
#endif

#ifndef EXTF_vsram_mark_physical
ot_u8 vsram_mark_physical(ot_u16* addr, ot_u16 value) {
    return vworm_mark_physical(addr, value);
}
#endif

#ifndef EXTF_vsram_get
ot_u8* vsram_get(vaddr addr) {
    return vworm_get(addr);
}
#endif



#endif //from top
