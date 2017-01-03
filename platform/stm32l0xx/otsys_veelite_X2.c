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
  * @file       /platform/stm32l0xx/otsys_veelite_X2.c
  * @author     JP Norair
  * @version    R100
  * @date       30 Dec 2016
  * @brief      X2 Method for Veelite Core Functions
  * @ingroup    Veelite
  *
  * @note This implementation also works as-is with most other STM32 devices, 
  *       although FLASH_...() library functions used within will need to be 
  *       linked to the respective platform library (L0, L1, F0, F1, etc).
  * 
  * Summary:
  * This module is part of the Veelite Core, which contains the low-level read
  * and write filesystem functionality.  This variant is meant for systems that
  * use NAND FLASH as the source of non-volatile memory.  In most cases you can
  * copy this file into any NAND-based platform without changing anything.
  * 
  * X2 Design Notes:
  * The X2 Method offers Flash wear leveling and has some tricks to reduce the
  * number of page erasures by 4x when compared to a naive approach and by
  * about 2X when compared to the X1 approach.  The main trick is to treat data
  * as the XNOR of the matching block addresses (offsets).
  *
  * The X2 Method is faster at reading and writing than the X1 Method, not to 
  * mention less complex and lighter on code size.  In typical cases, it has 
  * about the same storage overhead as the X1 method, although it scales better 
  * when more blocks are used.  It requires slightly more SRAM data for storing 
  * the block lookup tables.  The only real downside is that it is not as 
  * failsafe as the X1 method.  The vworm_save() function must be run before the
  * device's SRAM shuts off.  In most types of devices (anything without a user
  * removable battery) this is not a big problem.
  *
  *
  ******************************************************************************
  */

#include <otplatform.h>
#if defined(__STM32L0xx__) && defined(__VLX2__)

#include "stm32l0xx_hal.h"

#include <otsys/veelite_core.h>
#include <otlib/logger.h>


#ifndef OT_FEATURE_VLNVWRITE
#   define OT_FEATURE_VLNVWRITE ENABLED
#endif



/** MACROS for STM32F0xx library <BR>
  * ========================================================================<BR>
  * @note If you are upgrading the STM32 FW lib, you need to modify the file 
  * stm32f0xx_flash.h file in a simple way.  The FLASH_Status enum type needs to
  * have FLASH_COMPLETE = 0 instead of 4.
  */

/// NAND wrappers
ot_u8 NAND_erase_page(ot_u16* page_addr) {
    HAL_FLASH_Unlock();
    FLASH_ErasePage((uint32_t)page_addr);
    HAL_FLASH_Lock();
    return 0;
}


/// safe way of writing to flash, assuming it is unlocked
#define NAND_write_short(ADDR, DATA)    sub_write_short((ot_u32)ADDR, (ot_u16)DATA)

ot_u8 sub_write_short(ot_u32 addr, ot_u16 data) {
#   define _HWORD(VAL,OFFSET)   *((ot_u16*)(((ot_u8*)VAL)+OFFSET))
    ot_u32 word;

    // Clean the error context
    ProcFlash.ErrorCode = HAL_FLASH_ERROR_NONE;
    
    // STM32L0 doesn't have half word write, so need to fill the 
    // correct half of the word.
    word                    = *((ot_u32*)(addr & ~2));
    _HWORD(&word, (addr&2)) = data;
    *(__IO uint32_t*)addr   = word;
    
    //ot_u32, mask, shft;
    //shft                    = (addr&2) << 3;       // turn 2 into 16, or 0 into 0
    //mask                    = 0xFFFF0000 >> shft;
    //word                    = (ot_u32)data << shft;
    //*(__IO uint32_t*)addr   = 
    
    //return (FLASH_WaitForLastOperation(1024) != HAL_OK);
    
    while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);
    return 0;
    
#   undef _HWORD
}



/// VLX2 Debugging
/// This driver is quite stable, so debugging features are not implemented
/// even when __DEBUG__ is active
#ifdef __DEBUG__
//#   define _VLX2_DEBUG
#endif

/// Set Segmentation Fault (code 11) if trying to access an invalid virtual
/// address.  Vector to NMI
#if (defined _VLX2_DEBUG)
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
#if defined(_VLX2_DEBUG)
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



/// @note Subverting C pointer math rules:
/// There are some examples of heavy typecasting in this file.  They are used to
/// prevent the compiler from meddling with the offset addr, on account of C
/// pointer math rules.  Here is what this looks like:       <BR>
/// (ot_u16*)((ot_u8*)base_addr + offset_addr)
///
/// If this construct were not used, the C compiler would multiply offset_addr
/// by 2, because the operating pointer points to 2-byte data field (ot_u16).
#define PTR_OFFSET(PTR_BASE, OFFSET)    (ot_u16*)(((ot_u8*)PTR_BASE) + OFFSET)


/// VSRAM (Mirror) memory buffer
#if (VSRAM_SIZE > 0)
    ot_u16 vsram[ (VSRAM_SIZE/2) ];
#endif


/// Constant used to quickly divide input address by the physical block size
#if (VWORM_SIZE > 0)    
#   define PRIMARY_PAGE_START   0
#   define FALLOW_PAGE_START    VWORM_PRIMARY_PAGES

#   if (VWORM_PAGESIZE == 128)
#       define VWORM_PAGESHIFT  7
#   elif (VWORM_PAGESIZE == 256)
#       define VWORM_PAGESHIFT  8
#   elif (VWORM_PAGESIZE == 512)
#       define VWORM_PAGESHIFT  9
#   elif (VWORM_PAGESIZE == 1024)
#       define VWORM_PAGESHIFT  10
#   elif (VWORM_PAGESIZE == 2048)
#       define VWORM_PAGESHIFT  11
#   else
#       error VWORM_PAGESHIFT cannot be defined because the Flash page size is \
              not optioned in veelite_core_X2.c.  This can be adjusted.
#   endif





/** Data Types for Veelite <BR>
  * ========================================================================<BR>
  */

#if (OT_FEATURE(VLNVWRITE) == ENABLED)

/** @typedef Pdata
  * A Union that allows saving a pointer.
  */
typedef union {
    ot_u16* pointer;
    ot_u16  data[PLATFORM_POINTER_SIZE/2];
} Pdata;

/** @typedef block_ptr
  * Each block contains a primary block.  A block may additionally contain an
  * ancillary block.  Blocks that contain both primary and ancillary blocks have
  * their XNOR'ed across these two blocks.  It is part of the wear-leveling 
  * technique.
  */
typedef struct {
    ot_u16* primary;
    ot_u16* ancillary;
} block_ptr;

/** @typedef X2_struct
  * The X2 wear leveling block allocation table.  Virtual addressing references
  * an address relative to this table, even though the physical memory blocks
  * rotate through the table.  The table also stores a series of "fallow" blocks
  * that are reserved for the wear-leveling block rotation process.  Using a
  * larger number of fallows improves performance but increases ROM overhead.
  */
typedef struct {
    block_ptr   block[VWORM_PRIMARY_PAGES];
    ot_u16*     fallow[VWORM_FALLOW_PAGES];
} X2_struct;

X2_struct X2table;
    




/** Local Subroutine Prototypes <BR>
  * ========================================================================<BR>
  */

/** @brief Recombines a primary and ancillary block
  * @param block_in     (block_ptr*) pointer to the block to recombine
  * @param skip         (ot_int) address to skip during the block recombination
  * @param span         (ot_int) number of addresses to skip
  * @retval ot_u16*     pointer of the skip address, on the new, combined block
  */
ot_u16* sub_recombine_block(block_ptr* block_in, ot_int skip, ot_int span);

/** @brief Attaches a fallow block
  * @param block_in     (block_ptr*) pointer to the block to attach the new fallow
  * @retval none
  */
void sub_attach_fallow(block_ptr* block_in);
    
    
#endif 
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
///@note If using GCC, you should be using the "KEEP" option in your linker
///      script to make sure the linker does not discard the filesystem data as
///      part of optimization.  If you still can't implement KEEP, you can set 
///      this define (below), which will run a routine during init that touches 
///      all the FS arrays.   

#ifndef EXTF_vworm_format
ot_u8 vworm_format( ) {
#if ((VWORM_SIZE > 0) && (OT_FEATURE(VLNVWRITE) == ENABLED))
    ot_int      i;
    ot_u16*     cursor;
    ot_u8       output = 0;

    /// 1. Load default cursor (using embedded method)
    cursor = (ot_u16*)(FLASH_FS_ADDR);

    /// 2. Format all Blocks, Put Block IDs into Primary Blocks
    for (i=0; i<VWORM_PRIMARY_PAGES; i++) {
        output |= NAND_erase_page(cursor);
        cursor  = PTR_OFFSET(cursor, VWORM_PAGESIZE);
    }
    for (i=0; i<VWORM_FALLOW_PAGES; i++) {
        output |= NAND_erase_page(cursor);
        cursor  = PTR_OFFSET(cursor, VWORM_PAGESIZE);
    }

    return output;
#else
    return 0;
#endif
}
#endif


#ifndef EXTF_vworm_init
ot_u8 vworm_init( ) {
#if ((VWORM_SIZE > 0) && (OT_FEATURE(VLNVWRITE) == ENABLED))
    ot_u8   test    = 0;
    ot_u16* s_ptr;

    s_ptr = (ot_u16*)(VWORM_BASE_PHYSICAL + (VWORM_PAGESIZE*(VWORM_NUM_PAGES-1)));

    /// 1. If the last block starts with 0000, assume that a format just
    ///    happened, in which case we can ignore doing anything.
    if (*s_ptr != 0x0000) {  //if (*s_ptr != 0xFFFF) {
        ot_u16* b_ptr;
        ot_int  i;

        /// 2. Load the addresses supplied in the saved data into the lookup table
        ///    @note init & save processes have not been tested enough.
        b_ptr = (ot_u16*)&X2table;
        for (i=0; i<(sizeof(X2_struct)/2); i++) {
            b_ptr[i] = s_ptr[i];
        }

        /// 3. Erase the last page, which is once again a fallow block
        test = NAND_erase_page( s_ptr );
    }

    /// Load the lookup table with initial values
    else {
        ot_u16* cursor;
        ot_int i;
        cursor = (ot_u16*)(FLASH_FS_ADDR);

        for (i=0; i<VWORM_PRIMARY_PAGES; i++) {
            X2table.block[i].primary    = cursor;
            X2table.block[i].ancillary  = NULL;
            cursor = PTR_OFFSET(cursor, VWORM_PAGESIZE);
        }
        for (i=0; i<VWORM_FALLOW_PAGES; i++) {
            X2table.fallow[i] = cursor;
            cursor = PTR_OFFSET(cursor, VWORM_PAGESIZE);
        }
    }

    return test;
#else
    return 0;
#endif
}
#endif



#ifndef EXTF_vworm_print_table
void vworm_print_table() {
#ifdef _VLX2_DEBUG
//    ot_int i;
//
//    printf("VWORM X2table: Primaries\n");
//    for (i=0; i<VWORM_PRIMARY_PAGES; i++) {
//        printf("%02d: %08X - %08X\n", i, \
//            (unsigned int)X2table.block[i].primary, \
//            (unsigned int)X2table.block[i].ancillary);
//    }
//
//    printf("\nVWORM X2table: Fallows\n");
//    for (i=0; i<VWORM_FALLOW_PAGES; i++) {
//        printf("%02d: %08X\n", i, (unsigned int)X2table.fallow[i]);
//    }
#endif
}
#endif



#ifndef EXTF_vworm_save
ot_u8 vworm_save( ) {
#if ((VWORM_SIZE > 0) && (OT_FEATURE_VLNVWRITE == ENABLED))
    /// @note init & save processes have not been tested enough.
    /// Saves the state of the vworm onto the last physical block, which may
    /// require recombination before being able to be used.

    ot_u8   test    = 0;
    ot_int  i;
    ot_u16* b_ptr;
    ot_u16* s_ptr   = (ot_u16*)(VWORM_BASE_PHYSICAL + \
                        (VWORM_PAGESIZE*(VWORM_PRIMARY_PAGES+VWORM_FALLOW_PAGES-1)));
    
    HAL_FLASH_Unlock();
    
    /// 1.  look through used blocks to see if the last physical block is
    ///     somewhere inside.  In this case, we need to recombine it.
    for (i=0; i<VWORM_PRIMARY_PAGES; i++) {
        if ( (X2table.block[i].primary == s_ptr) || (X2table.block[i].ancillary == s_ptr) ) {
            sub_recombine_block(&X2table.block[i], 0, 0);
            break;
        }
    }

    /// 2.  The last block is now in the list of fallows, either because it was
    ///     already there or because it was just erased and fallowed following
    ///     recombination.

    // 2a. Write [Number of Primary blocks]
    //test |= vworm_mark_physical(s_ptr, VWORM_PRIMARY_PAGES);    s_ptr++;

    // 2b. Write [Number of Fallow blocks]
    //test |= vworm_mark_physical(s_ptr, VWORM_FALLOW_PAGES);     s_ptr++;

    /// 2c. Write each Primary & Ancillar Base Pointer to the save
    /// 2d. Write each Fallow Pointer to the save
    /// @todo optimize the block & fallow data
    
    b_ptr = (ot_u16*)&X2table;
    for (i=0; i<(sizeof(X2_struct)/2); i++) {
        test |= vworm_mark_physical(s_ptr, b_ptr[i]);
        s_ptr++;
    }
    
    HAL_FLASH_Lock();

    return test;
#else
    return 0;
#endif
}
#endif



#ifndef EXTF_vworm_read
ot_u16 vworm_read(vaddr addr) {
#if ((VWORM_SIZE > 0) && (OT_FEATURE(VLNVWRITE) == ENABLED))
    ot_u16* a_ptr;
    ot_u16* p_ptr;
    ot_int  offset;
    ot_int  index;

    SEGFAULT_CHECK(addr, in_vworm, 7, "VLC_"__LINE__);

    /// 1.  Resolve the vaddr directly
    offset  = addr & (VWORM_PAGESIZE-1);
    index   = (addr-VWORM_BASE_VADDR) >> VWORM_PAGESHIFT;
    p_ptr   = PTR_OFFSET(X2table.block[index].primary, offset);

    /// 2. return either the primary pointer in full or the XNOR
    if (X2table.block[index].ancillary == NULL) {
        return *p_ptr;
    }

    a_ptr   = PTR_OFFSET(X2table.block[index].ancillary, offset);

    return ~(*p_ptr ^ *a_ptr);

#elif (OT_FEATURE(VLNVWRITE) != ENABLED)
    return *((ot_u16*)addr);

#else
    return 0;
#endif
}
#endif



#ifndef EXTF_vworm_write
ot_u8 vworm_write(vaddr addr, ot_u16 data) {
#if ((VWORM_SIZE > 0) && (OT_FEATURE_VLNVWRITE == ENABLED))
    ot_u8   output;
    ot_int  index;
    ot_int  offset;
    ot_u16  wrtest;
    ot_u16* p_ptr;
    ot_u16* a_ptr;

    SEGFAULT_CHECK(addr, in_vworm, 7, "VLC_"__LINE__);

    /// 0. Unlock Flash
    HAL_FLASH_Unlock();

    /// 1.  Resolve the vaddr directly
    offset  = addr & (VWORM_PAGESIZE-1);
    index   = (addr-VWORM_BASE_VADDR) >> VWORM_PAGESHIFT;
    p_ptr   = PTR_OFFSET(X2table.block[index].primary, offset);

    /// 2. No ancillary block, but try a write anyway
    if (X2table.block[index].ancillary == NULL) {

        /// 2a. If no 0->1 write requirement, then we're good to go
        if ((data & ~(*p_ptr)) == 0) {
            goto vworm_write_WRITE;
        }

        /// 2b. Attach a fallow to this bitch (it becomes ancillary)
        sub_attach_fallow(&X2table.block[index]);
    }

    /// 3. There is ancillary block, so go through the logical write process,
    ///    which is designed to shake out a write out of whatever it can get.
    ///    The only bit combination that cannot be managed is [1->0 via 0,0]
    a_ptr   = PTR_OFFSET(X2table.block[index].ancillary, offset);
    wrtest  = ~data & ~(*p_ptr) & ~(*a_ptr);

    if (wrtest == 0) {
        output = 0;

        /// 3a. Adjust cases where [1->0 via 1,1] or [0->1 via 1,0]
        wrtest  = ~data & *p_ptr & *a_ptr;
        wrtest |= data & *p_ptr & ~(*a_ptr);
        if (wrtest != 0) {
            output |= vworm_mark_physical(p_ptr, *p_ptr ^ wrtest);
        }

        /// 3b. Adjust cases where [0->1 via 0,1]
        wrtest  = data & ~(*p_ptr) & *a_ptr;
        if (wrtest != 0) {
            output |= vworm_mark_physical(a_ptr, *a_ptr ^ wrtest);
        }
        
        goto vworm_write_LOCK;
    }

    /// 4. Recombine this block, with the exception of the given addr offset,
    ///    which we will then write-to
    p_ptr = sub_recombine_block(&X2table.block[index], offset, 2);
    
    vworm_write_WRITE:
    output = vworm_mark_physical(p_ptr, data);
    
    vworm_write_LOCK:
    HAL_FLASH_Lock();
    return output;
    
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
    BUSERROR_CHECK( (((ot_u16)addr < VWORM_BASE_PHYSICAL) || \
                    ((ot_u16)addr >= (VWORM_BASE_PHYSICAL+VWORM_ALLOC))), 7, "VLC_"__LINE__);
                    
    return NAND_write_short(addr, value);
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
  */

#ifndef EXTF_vsram_read
ot_u16 vsram_read(vaddr addr) {
#if (VSRAM_SIZE <= 0)
    return 0;
#else
    SEGFAULT_CHECK(addr, in_vsram, 7, "VLC_"__LINE__);
    addr -= VSRAM_BASE_VADDR;
    addr >>= 1;
    return vsram[addr];
#endif
}
#endif



#ifndef EXTF_vsram_mark
ot_u8 vsram_mark(vaddr addr, ot_u16 value) {
#if (VSRAM_SIZE <= 0)
    return ~0;
#else
    SEGFAULT_CHECK(addr, in_vsram, 7, "VLC_"__LINE__);
    addr 		   -= VSRAM_BASE_VADDR;
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
    SEGFAULT_CHECK(addr, in_vsram, 7, "VLC_"__LINE__);
    addr   -= VSRAM_BASE_VADDR;
    output  = (ot_u8*)vsram + addr;
    return output;
#endif
}
#endif






/** Subroutine Implementations <BR>
  * ========================================================================<BR>
  */

#if ((VWORM_SIZE > 0) && (OT_FEATURE_VLNVWRITE == ENABLED))
ot_u16* sub_recombine_block(block_ptr* block_in, ot_int skip, ot_int span) {
    ot_u8 test;
    ot_int i;
    ot_u16* new_ptr;
    ot_u16* f_ptr;
    ot_u16* p_ptr;
    ot_u16* a_ptr;

    /// 1. Assign pointers
    p_ptr   = block_in->primary;
    a_ptr   = block_in->ancillary;
    new_ptr = X2table.fallow[(VWORM_FALLOW_PAGES-1)];
    f_ptr   = new_ptr;
    
    /// 2. Combine the old blocks into the fallow block
    span+=skip;
    for (i=0; i<FLASH_PAGE_SIZE; i+=2) {
        if ((i<skip) || (i>=span)) {
            test |= vworm_mark_physical(f_ptr, ~(*p_ptr ^ *a_ptr));
        }
        f_ptr++;
        p_ptr++;
        a_ptr++;
    }
    
//    i += span;
//    PTR_OFFSET(f_ptr, span);
//    PTR_OFFSET(p_ptr, span);
//    PTR_OFFSET(a_ptr, span);
//    
//    for (; i<VWORM_PAGESIZE; i+=2) {
//        test |= vworm_mark_physical(f_ptr, ~(*p_ptr ^ *a_ptr));
//        f_ptr++;
//        p_ptr++;
//        a_ptr++;
//    }
    
    /// 3. Erase the old blocks
    NAND_erase_page( block_in->primary );
    NAND_erase_page( block_in->ancillary );
    
    /// 4. Make the two erased blocks fallow blocks. If we are in this function,
    /// we can deduce that there is at least one ancillary and one fallow, so we
    /// stop when NULL is discovered or when at the end of the fallows.
#   if (VWORM_FALLOW_PAGES >= 2)
        for (i=(VWORM_FALLOW_PAGES-1); X2table.fallow[i] != NULL; i--) {
            X2table.fallow[i] = X2table.fallow[i-1];
        }
        X2table.fallow[i+1] = block_in->primary;
        X2table.fallow[i]   = block_in->ancillary;
#   else
        X2table.fallow[1]   = block_in->primary;
        X2table.fallow[0]   = block_in->ancillary;
#   endif     
    
    /// 5. Set the primary block to its new position, and ancillary to NULL
    block_in->ancillary = NULL;
    block_in->primary   = new_ptr;
    
    /// 6. return the (physical) skip address
    return PTR_OFFSET(new_ptr, skip);
}




void sub_attach_fallow(block_ptr* block_in) {
    ot_int  i;
    
    /// If there is only one fallow block left, we need to recombine some other
    /// blocks first (the one fallow left will get rotated).
    if (X2table.fallow[(VWORM_FALLOW_PAGES-2)] == NULL) {
        for (i=0; i<VWORM_PRIMARY_PAGES; i++) {
            if (X2table.block[i].ancillary != NULL) {
                sub_recombine_block(&X2table.block[i], 0, 0);
                break;
            }
        } 
    }
    
    /// Make the fallow at the back of the fallow table become the new ancillary
    /// for the supplied primary.
    block_in->ancillary = X2table.fallow[(VWORM_FALLOW_PAGES-1)];
    
    /// Shift-up other fallow blocks and make the new bottom fallow NULL
    for (i=(VWORM_FALLOW_PAGES-1); i>0; i--) {
        X2table.fallow[i] = X2table.fallow[i-1];
    }
    X2table.fallow[0] = NULL;
}

    
#endif 

#endif //from top
