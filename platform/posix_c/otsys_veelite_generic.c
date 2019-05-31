/* Copyright 2017 JP Norair
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
  * @file       /platform/stdc/otsys_veelite_generic.c
  * @author     JP Norair
  * @version    R100
  * @date       3 Nov 2017
  * @brief      SRAM Method for Veelite Core Functions
  * @ingroup    Veelite
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

#include <otsys/veelite_core.h>
#include <otlib/logger.h>
#include <otlib/memcpy.h>


/// Patch: If Multi-FS is enabled, fsram location and size is defined through
/// vworm_init(), dynamically, selected via vworm_select(), and assigned to 
/// this context while used.
#if (OT_FEATURE(MULTIFS))
    static ot_u32* fsram;
#else
    static ot_u32 fsram[FLASH_FS_ALLOC/4];
#endif

#define FSRAM ((ot_u16*)fsram)


/// Set Bus Error (code 7) on physical flash access faults (X2table errors).
/// Vector to Access Violation ISR (CC430 Specific)
#if defined(VLX2_DEBUG_ON)
#   define BUSERROR_CHECK(EXPR, MSGLEN, MSG) \
        do { \
            if (EXPR) { \
                fprintf(stderr, "%s\n", (ot_u8*)MSG); \
            } \
        } while (0)
#else
#   define BUSERROR_CHECK(EXPR, MSGLEN, MSG) do { } while(0)
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
ot_uint sub_copy_section(ot_u32* section, void* defaults, ot_uint defaults_size, ot_uint input_size) {
    // Dynamic sizing variant would require re-setting the input struct...
    ot_uint  copylen;
    copylen = (defaults_size < input_size) ? defaults_size : input_size;
    copylen = ((copylen+3) / 4);
    
    ot_memcpy_4(section, defaults, copylen);
    
    //printf("defaults_size = %d\ninput_size = %d\n", defaults_size, input_size);
    return copylen;
    
    //ot_memcpy_4(section, (void*)overhead_files, copylen);
}




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



void vworm_fsheader_defload(vlFSHEADER* fs) {
    if (fs != NULL) {
        ot_memcpy((void*)fs, (void*)overhead_files, sizeof(vlFSHEADER));
    }
}




ot_uint vworm_fsdata_defload(void* fs_base, const vlFSHEADER* fs) {
    ot_u32* section;

    if ((fs_base == NULL) || (fs == NULL)) {
        return 0;
    }
    
    section = fs_base;
    
    sub_copy_section(section, (void*)overhead_files, OVERHEAD_TOTAL_BYTES, fs->ftab_alloc);
    section += fs->ftab_alloc / 4;
    
#   if (GFB_TOTAL_BYTES > 0)
    if (fs->gfb.alloc != 0) {
        sub_copy_section(section, (void*)gfb_stock_files, GFB_TOTAL_BYTES, fs->gfb.alloc);
        section += fs->gfb.alloc / 4;
    }
#   endif
#   if (ISS_TOTAL_BYTES > 0)
    if (fs->iss.alloc != 0) {
        sub_copy_section(section, (void*)iss_stock_files, ISS_STOCK_BYTES, fs->iss.alloc);
        section += fs->iss.alloc / 4;
    }
#   endif
#   if (ISF_TOTAL_BYTES > 0)
    if (fs->isf.alloc != 0) {
        sub_copy_section(section, (void*)isf_stock_files, ISF_STOCK_BYTES, fs->isf.alloc);
        section += fs->isf.alloc / 4;
    }
#   endif
    
    return (ot_uint)((void*)section - fs_base);
}



#ifndef EXTF_vworm_init
ot_u8 vworm_init(void* fs_base, const vlFSHEADER* fs) {
/// If MultiFS is not used, all the arguments can be NULL.
/// If MultiFS is required, the initialization process includes storing default
/// filesystem values into a memory-base supplied by the caller.

#   if (OT_FEATURE(MULTIFS))
    if (fs_base == NULL)  {
        return 1;
    }
    if (fs != NULL) {
        if (fs->ftab_alloc == 0) {
            /// Error when ftab has no bytes
            return 2; 
        }
        vworm_fsdata_defload(fs_base, fs);
    }

    fsram = (ot_u32*)fs_base;
    
    /// No MultiFS
#   else

    ot_memcpy_4(&fsram[OVERHEAD_START_VADDR/4], (void*)overhead_files, OVERHEAD_TOTAL_BYTES/4);
#   if (GFB_TOTAL_BYTES > 0)
    ot_memcpy_4(&fsram[GFB_START_VADDR/4], (void*)gfb_stock_files, GFB_TOTAL_BYTES/4);
#   endif
#   if (ISF_TOTAL_BYTES > 0)
    ot_memcpy_4(&fsram[ISF_START_VADDR/4], (void*)isf_stock_files, ISF_VWORM_STOCK_BYTES/4);
#   endif
    
#   endif

    return 0;
}
#endif

#ifndef EXTF_vworm_print_table
void vworm_print_table() {
}
#endif

#ifndef EXTF_vworm_save
ot_u8 vworm_save( ) {
/// @note This does nothing for pure STDC implementation, which is entirely 
///       RAM based.  For Microcontroller variant, this can save the file table
///       to nonvolatile memory.
    return 0;
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
void* vworm_get(vaddr addr) {
    addr -= VWORM_BASE_VADDR;
    return (void*)((ot_u8*)fsram + addr);
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
void* vsram_get(vaddr addr) {
    return (void*)vworm_get(addr);
}
#endif


