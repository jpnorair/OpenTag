/* Copyright 2010-2012 JP Norair
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
  * @file       /otplatform/msp430f5/veelite_core_fram_MSP430F5.c
  * @author     JP Norair
  * @version    R100
  * @date       8 Nov 2012
  * @brief      FRAM Method for Veelite Core Functions
  * @ingroup    Veelite
  *
  * Summary:
  * This module is part of the Veelite Core, which contains the low-level read
  * and write filesystem functionality.  This variant is meant for systems that
  * use NAND FLASH as the source of non-volatile memory.  In most cases you can
  * copy this file into any NAND-based platform without changing anything.
  *
  * FRAM Design Notes:
  * What is sophisticated in Flash is trivial in FRAM.  Yay FRAM!
  *
  ******************************************************************************
  */

#include "OT_platform.h"
#if defined(__MSP430F5__) && defined(__FRAM__)

#include "OTAPI.h"              // for logging faults
#include "veelite_core.h"

#ifndef OT_FEATURE_VLNVWRITE
#   define OT_FEATURE_VLNVWRITE ENABLED
#endif



/** MACROS for MSP430 library <BR>
  * ========================================================================<BR>
  */ 
#ifdef __DEBUG__
#   define SEGFAULT_CHECK(ADDR, BANK, MSGLEN, MSG) \
        do { \
            if (vas_check(ADDR) == 0) { \
                SFRIFG1 |= NMIIFG; \
            } \
        } while (0)
#else
#   define SEGFAULT_CHECK(ADDR, BANK, MSGLEN, MSG); 
#endif


ot_u16 vfram_read(vaddr addr);
ot_u8 vfram_mark(vaddr addr, ot_u16 value);
ot_u8 vfram_mark_physical(ot_u16* addr, ot_u16 value);
ot_u8* vfram_get(vaddr addr);
ot_u8 vfram_wipeblock(vaddr addr, ot_uint wipe_span);









/** Generic Veelite Core Function Implementations <BR>
  * ========================================================================<BR>
  * Used for any and all memory topologies
  */

vas_loc vas_check(vaddr addr) {
    return ((addr >= VWORM_BASE_VADDR) && \
        (addr < (VWORM_BASE_VADDR+(VWORM_PRIMARY_PAGES*VWORM_PAGESIZE)) ) );
}





/** VWORM Functions <BR>
  * ========================================================================<BR>
  */
  
///@note If using GCC, you should be using the "KEEP" option in your linker
///      script to make sure the linker does not discard the filesystem data as
///      part of optimization (Check STM32 RIDE linker scripts for examples).
///      If you still can't implement KEEP, you can set this define (below), 
///      which will run a routine during init that touches all the FS arrays.    

#if (CC_SUPPORT == GCC)
#   warn "Make sure to use KEEP() around filesystem arrays in your linker script."
#endif



#ifndef EXTF_vworm_format
ot_u8 vworm_format( ) {
#if ((VWORM_SIZE > 0) && (OT_FEATURE(VLNVWRITE) == ENABLED))
    //I could potentially use a memset to 0 over the FRAM contents, here
#endif
    return 0;
}
#endif

#ifndef EXTF_vworm_init
ot_u8 vworm_init( ) {
#if ((VWORM_SIZE > 0) && (OT_FEATURE(VLNVWRITE) == ENABLED))
    //Not necessary for FRAM
#endif
    return 0;
}
#endif

#ifndef EXTF_vworm_save
ot_u8 vworm_save( ) {
#if ((VWORM_SIZE > 0) && (OT_FEATURE_VLNVWRITE == ENABLED))
    //Not necessary for FRAM
#endif
    return 0;
}
#endif

#ifndef EXTF_vworm_read
ot_u16 vworm_read(vaddr addr) {
    return vfram_read(addr);
}
#endif

#ifndef EXTF_vworm_write
ot_u8 vworm_write(vaddr addr, ot_u16 data) {
    return vfram_mark(addr, data);
}
#endif

#ifndef EXTF_vworm_mark
ot_u8 vworm_mark(vaddr addr, ot_u16 value) {
    return vfram_mark(addr, value);
}
#endif

#ifndef EXTF_vworm_mark_physical
ot_u8 vworm_mark_physical(ot_u16* addr, ot_u16 value) {
    return vfram_mark_physical(addr, value);
}
#endif

#ifndef EXTF_vworm_wipeblock
ot_u8 vworm_wipeblock(vaddr addr, ot_uint wipe_span) {
    return vfram_wipeblock(addr, wipe_span);
}
#endif




/** VSRAM Functions <BR>
  * ========================================================================<BR>
  */
#ifndef EXTF_vsram_read
ot_u16 vsram_read(vaddr addr) {
    return vfram_read(addr);
}
#endif

#ifndef EXTF_vsram_mark
ot_u8 vsram_mark(vaddr addr, ot_u16 value) {
    return vfram_mark(addr, value);
}
#endif

#ifndef EXTF_vsram_mark_physical
ot_u8 vsram_mark_physical(ot_u16* addr, ot_u16 value) {
    return vfram_mark_physical(addr, value);
}
#endif

#ifndef EXTF_vsram_get
ot_u8* vsram_get(vaddr addr) {
    return vfram_get(addr);
}
#endif









/** VSRAM Functions <BR>
  * ========================================================================<BR>
  */
ot_u16 vfram_read(vaddr addr) {
    SEGFAULT_CHECK(addr, in_vworm, 10, "vfram_read");
    return *addr;
}


ot_u8 vfram_mark(vaddr addr, ot_u16 value) {
    SEGFAULT_CHECK(addr, in_vworm, 10, "vfram_mark");
    return vfram_mark_physical((ot_u16*)addr, value);
}


ot_u8 vfram_mark_physical(ot_u16* addr, ot_u16 value) {
    *addr = value;
    return 0;
}


ot_u8* vfram_get(vaddr addr) {
    SEGFAULT_CHECK(addr, in_vworm, 9, "vfram_get");
    return (ot_u8*)addr;
}


ot_u8 vfram_wipeblock(vaddr addr, ot_uint wipe_span) {
    SEGFAULT_CHECK(addr, in_vworm, 15, "vfram_wipeblock");
    platform_memset((ot_u16*)addr, 0, wipe_span);
    return 0;
}


#endif
