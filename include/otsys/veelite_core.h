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
  * @file       /otsys/veelite_core.h
  * @author     JP Norair
  * @version    R101
  * @date       23 Jan 2013
  * @brief      An interface for reading and writing to veelite virtual memory
  * @defgroup   Veelite (Veelite Module)
  * @ingroup    Veelite
  *
  * The module provides a virtual address space for managing different types of
  * memory: namely VWORM and VSRAM.  These do not all have to be implemented.
  *
  * The implementation is part of the platform.  The file veelite_core_NAND.c
  * is commonly used for platforms that use NAND flash
  *
  ******************************************************************************
  */


#ifndef __VEELITE_CORE_H
#define __VEELITE_CORE_H

#include <otstd.h>

#include <platform/config.h>



/** @note Virtual Address Space:
  * OpenTag's Virtual Address Space for nonvolatile memory is, at the moment,
  * limited to 64K.  The addresses available are 0x0000 to 0xFFFE (0xFFFF is
  * prohibited).
  *
  * There are two sectors that may work differently: VEEPROM and VWORM.
  *
  * VEEPROM: (deprecated)
  * An address space that can easily accommodate reads and writes.
  * It is optimized for storing a relatively small number of frequently accessed
  * & changed variables (which in NAND Flash requires wear-leveling). In the
  * standard VEEPROM implementation each virtual address points to 16 bits of
  * data.  The constant "VEEPROM_SIZE" is set to 2x maximum number of 16 bit
  * variables supported.  It is not really byte-addressable, though.
  * Performance will improve if this number is reduced to reflect the actual
  * amount of variables used (or close to it).
  *
  * VWORM:
  * An address space that is best for reading blocks of data.
  * It is optimized for "Write Once Read Multiple" (WORM) behavior and is best
  * for storing blocks of data that aren't written to very often.  Unlike a true
  * WORM memory VWORM can be written to whenever necessary, but in the case that
  * your platform uses NAND Flash the write process is slow.
  */



/** @typedef vas_loc
  * An enum to declare the position in virtual addressing space (vas)
  * vas_error=0, in_vworm, in_veeprom
  */
typedef enum {
    vas_error = 0,
    in_vworm,
    in_veeprom,
    in_vsram
} vas_loc;



/** @typedef vaddr
  * A currently 16 bit virtual address.  Later upgradable to 32 bit.
  * NULL_vaddr is not 0 but 0xFFFF, because in NAND Flash this value represents
  * uninitialized space.
  */
typedef ot_u16 vaddr;
#ifndef NULL_vaddr
#   define NULL_vaddr           0xFFFF
#endif

#if (OT_FEATURE(VLNVWRITE) == ENABLED)
#   if (NULL_vaddr == 0)
#   define VWORM_BASE_VADDR     (0x0002)
#   else
#   define VWORM_BASE_VADDR     (0x0000)
#endif
#	else
#   define VWORM_BASE_VADDR			FLASH_FS_ADDR
#endif

#define VWORM_PAGESIZE           FLASH_PAGE_SIZE
#define VWORM_NUM_PAGES          FLASH_FS_PAGES
#define VWORM_FALLOW_PAGES       FLASH_FS_FALLOWS
#define VWORM_PRIMARY_PAGES      (FLASH_FS_PAGES - FLASH_FS_FALLOWS)
#define VWORM_MAXBLOCKSIZE       VWORM_PAGESIZE
#define VWORM_BLOCKSIZE          VWORM_PAGESIZE
#define VWORM_BLOCKINFO_OFFSET   VWORM_BLOCKSIZE
#define VWORM_SIZE               (FLASH_FS_ALLOC)
#define VWORM_ALLOC              (FLASH_FS_ALLOC)

#define VSRAM_BASE_VADDR         ISF_MIRROR_VADDR
#define VSRAM_SIZE               ISF_MIRROR_HEAP_BYTES
#define VSRAM_ALLOC              VSRAM_SIZE

#define VWORM_BASE_PHYSICAL      FLASH_FS_ADDR
#define VWORM_PHYSICAL_ADDR(VAL) (VWORM_BASE_PHYSICAL + (VAL) - VWORM_BASE_VADDR)
//#define VSRAM_BASE_PHYSICAL      OTF_VSRAM_START_ADDR
//#define VSRAM_PHYSICAL_ADDR(VAL) (VSRAM_BASE_PHYSICAL + (VAL) - VSRAM_BASE_VADDR)

#ifndef VPROM_SIZE
#   define VPROM_SIZE 0
#   define VPROM_BASE_PHYSICAL 0
#endif




/** memory_faults
  * After a function returns false, the reason is in memory_faults.
  */

// STATUS FLAGS
#define MEM_OK               0x00
#define MEM_HW_FAULT         0x01
#define MEM_ADDR_FAULT       0x02
#define MEM_VWORM_FAULT      0x04
extern ot_u8 memory_faults;



/** @brief Checks which address space the supplied virtual address is in.
  * @param v_addr : (ot_uint) Virtual Address
  * @retval vas_loc : position of the address
  * @ingroup Veelite
  */
vas_loc vas_check(vaddr addr);



/** @brief Formats VWORM memory
  * @param none
  * @retval ot_u8 :       non-zero on memory fault
  * @ingroup Veelite
  *
  * This functions wipes all blocks and resets the virtual block structure.
  */
ot_u8 vworm_format( );



/** @brief Initializes the VWORM memory system
  * @param none
  * @retval ot_u8 :     Non-zero on memory fault
  * @ingroup Veelite
  *
  * VWORM keeps a table of active blocks in local SRAM, backed up in VEEPROM.
  * vworm_init() loads the backed up data into local SRAM.  If you have just
  * formatted, you do not need to init.
  */
ot_u8 vworm_init( );



/** @brief Saves the state of the vworm system
  * @param none
  * @retval ot_u8       Non-zero on memory fault
  * @ingroup Veelite
  *
  * This is sometimes implemented as an empty wrapper, depending on the way the
  * VWORM system saves its parameters.  Nonetheless, it should be run prior to
  * shutting down the system, power-cut off, etc.
  */
ot_u8 vworm_save( );



/** @brief Reads 16 bits of data at the virtual address
  * @param addr : (vaddr) Variable virtual address
  * @retval ot_u16 : returned read data
  * @ingroup Veelite
  *
  * if given an odd v_addr (A1), vworm_read() will pull the 16 bits that
  * contain this byte (i.e. A1A0 or A0A1 depending on endian)
  */
ot_u16 vworm_read(vaddr addr);



/** @brief Write 16 bits to a virtual address in VWORM
  * @param addr : (vaddr) Variable virtual address to write onto
  * @param data : (ot_u16) 16 bits of data to write.
  * @retval ot_u8 : Non-zero on memory fault
  * @ingroup Veelite
  *
  * @note If the supplied addr is odd, only the upper/lower 8 bits are written
  * for little/big endian.
  *
  * vworm is often held in NAND flash, so the implementation for vworm_write()
  * will typically include some sort of mechanism to account for wear leveling
  * write performance.  Typically, one or more fallow blocks of NAND are used,
  * and vworm_write() will shuffle the vaddr look up table as it automatically
  * shuffles the blocks.
  */
ot_u8 vworm_write(vaddr addr, ot_u16 data);



/** @brief Mark a 16 bit value to a virtual address in VWORM
  * @param addr : (vaddr) virtual address to mark
  * @param value : (ot_u16) 16 bit value
  * @retval ot_u8 : Non-zero on memory fault
  * @ingroup Veelite
  *
  * vworm_mark() is similar functionally to vworm_write, and in some
  * implementations it may be identical.  In implementations with NAND flash,
  * vworm_mark() does not reshuffle blocks, so it is not guaranteed to work.
  * It is typically used to quickly zero a small amount of data, since NAND
  * writes can go "downhill" (1->0) easily but not "uphill" (0->1) without
  * additional processing.
  */
ot_u8 vworm_mark(vaddr addr, ot_u16 value);
ot_u8 vworm_mark_physical(ot_u16* addr, ot_u16 value);



/** @brief Returns a physical byte pointer to the virtual address in VWORM
  * @param v_addr : (ot_uint) Virtual Address
  * @retval ot_u8* : the physical pointer to VWORM
  * @ingroup Veelite
  *
  * @note Data in VWORM, in most cases, is NOT contiguous.  This function is
  *       here "just because."  Besides, it is useful for debugging.
  */
ot_u8* vworm_get(vaddr addr);



/** @brief Debugging function that prints out the state of the block table
  * @param none
  * @retval none
  * @ingroup Veelite
  */
void vworm_print_table();


/** @brief Wipe (erase) a block of data in VWORM
  * @param v_addr : (ot_uint) Base Virtual Address of somewhere in the block
  * @param wipe_span : (ot_uint) number of bytes to wipe, starting from v_addr
  * @retval ot_u8 : Non-zero on memory fault
  * @ingroup Veelite
  *
  * @note This writes 0xFFFF all the words in the span.  It is not a block
  *       erase function.  It is mostly for debugging.
  */
ot_u8 vworm_wipeblock(vaddr addr, ot_uint wipe_span);
//ot_u8 vworm_wipeblock_physical(ot_u8* addr, ot_uint wipe_span);







/** VSRAM is implemented in SRAM or something like it, that is byte addressable
  * and can be written to or read from without any overhead.
  */

/** @brief Reads 16 bits of data at the virtual address
  * @param addr : (vaddr) Variable virtual address
  * @retval (ot_u16*) data returned at supplied address
  * @ingroup Veelite
  *
  * if given an odd v_addr (A1), vsram_read() will pull the 16 bits that
  * contain this byte (i.e. A1A0 or A0A1 depending on endian)
  */
ot_u16 vsram_read(vaddr addr);


/** @brief Mark a 16 bit value to a virtual address in VSRAM
  * @param addr : (vaddr) virtual address to mark
  * @param value : (ot_u16) 16 bit value
  * @retval ot_u8 : Non-zero on memory fault
  * @ingroup Veelite
  *
  * In VSRAM, "write" and "mark" are the same thing.  There are also macros,
  * vsram_write() and vsram_write_physical(), that resolve to these functions.
  */
ot_u8 vsram_mark(vaddr addr, ot_u16 value);
ot_u8 vsram_mark_physical(ot_u16* addr, ot_u16 value);
#define vsram_write(VAL1, VAL2)             vsram_mark(VAL1, VAL2)
#define vsram_write_physical(VAL1, VAL2)    vsram_mark_physical(VAL1, VAL2)


/** @brief Returns a physical byte pointer to the virtual address in VSRAM
  * @param v_addr : (ot_uint) Virtual Address
  * @retval ot_u8* : the physical pointer to VSRAM
  * @ingroup Veelite
  */
ot_u8* vsram_get(vaddr addr);





ot_u16 vprom_read(vaddr addr);
ot_u8* vprom_get(vaddr addr);
ot_u8  vprom_write(vaddr addr, ot_u16 value);



#endif
