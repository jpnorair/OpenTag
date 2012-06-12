/*  Copyright 2010-2011, JP Norair
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
  */
/**
  * @file       /OTlib/veelite.c
  * @author     JP Norair
  * @version    V1.0
  * @date       30 Oct 2010
  * @brief      ISO 18000-7 veelite data files
  * @ingroup    Veelite
  *
  * Veelite is based on the GFB + ISF specification of ISO 18000-7 Mode 2.  It
  * may be used for anything, however, as long as the GFB + ISF organization 
  * makes sense.
  *
  ******************************************************************************
  */

#include "OT_config.h"

#if (OT_FEATURE(VEELITE) == ENABLED)

#include "OT_types.h"
#include "OT_utils.h"
#include "OT_platform.h"
#include "auth.h"
#include "veelite.h"

///@todo remove this legacy provision
#   ifndef ISF_NUM_EXT_FILES
#       define ISF_NUM_EXT_FILES 1
#   endif


// You can open a finite number of files simultaneously
vlFILE vl_file[OT_FEATURE(VLFPS)];


#define FP_ISVALID(fp_VAL)  (fp_VAL != NULL)

//Slower but more robust version of above
//#define FP_ISVALID(fp_VAL)  ((fp_VAL >= &vl_file[0]) && (fp_VAL <= &vl_file[OT_FEATURE(VLFPS)-1]))


// ISF Mirroring
#define MIRROR_TO_SRAM      0x00
#define MIRROR_TO_FLASH     0xFF


typedef ot_u8 (*sub_check)(ot_u8);
typedef vaddr (*sub_vaddr)(ot_u8);
typedef vlFILE* (*sub_new)(ot_u8, ot_u8, ot_u8);


/** VWORM Memory Allocation
  * Base positions and maximum group allocations for data files stored in
  * VWORM.  The values are taken from platform.h.
  * Direct addressing may only work for reads, depending on the nature of the
  * VWORM memory.  NAND Flash, for example, cannot be written to arbitrarily.
  */
#if (CC_SUPPORT == SIM_GCC)
    ot_u16*          VWORM_Heap;
    ot_u16*          VSRAM_Heap;
    //M1TAG_struct     M1TAG;
#endif









// Private Functions
vlFILE* sub_gfb_new(ot_u8 id, ot_u8 mod, ot_u8 null_arg);
vlFILE* sub_isfs_new(ot_u8 id, ot_u8 mod, ot_u8 null_arg);
vlFILE* sub_isf_new(ot_u8 id, ot_u8 mod, ot_u8 max_length);
ot_u8 sub_gfb_delete_check(ot_u8 id);
ot_u8 sub_isfs_delete_check(ot_u8 id);
ot_u8 sub_isf_delete_check(ot_u8 id);
vaddr sub_gfb_search(ot_u8 id);
vaddr sub_isfs_search(ot_u8 id);
vaddr sub_isf_search(ot_u8 id);


/** @brief Performs mirroring operations on ISF files
  * @param direction : (ot_u8) vworm->vsram or vsram->vworm
  * @retval ot_u8  
  * 
  * For @c direction @c 0 is vworm->vsram, and non-zero is vsram->vworm
  */
ot_u8 sub_isf_mirror(ot_u8 direction);




vlFILE* sub_new_fp();
vlFILE* sub_new_file(vl_header* new_header, vaddr heap_base, vaddr heap_end, vaddr header_base, ot_int header_window );
void sub_delete_file(vaddr del_header);
void sub_copy_header( vaddr header, ot_u16* output_header );

/** @brief Writes a block of data to the header
  * @param addr : (ot_u8*) physical address of the start of the write
  * @param data : (ot_u16*) pointer to half-word aligned data
  * @param length : (ot_uint) number of BYTES to write
  * @retval none
  *
  * @note @c addr @c parameter should be half word aligned (i.e. even). 
  *       Behavior is not guaranteed with non half-word aligned addresses
  */
void sub_write_header(vaddr header, ot_u16* data, ot_uint length );



/** @brief Searches for the first empty header
  * @param start_base : (vaddr*) physical pointer to the @c base @c file of a header
  * @param header_size : (size_t) number of bytes in the given header type
  * @param num_headers : (ot_int) number of headers to search through
  * @retval ot_int : Number of headers searched through until empty was found.
  *                  -1 if no headers were found.
  *
  * The return value corresponds to an index in the header array.  For example
  * XXX_Header_Array[retval + offset] is the index of the empty header, where 
  * offset is the index of the starting header.
  * 
  * @c sub_find_empty_header() @c will always return the nearest header to the front
  * of the array.
  */
vaddr sub_find_empty_header(vaddr header, ot_int num_headers);



/** @brief Searches for an amount of the empty space in the heap
  * @param base : (vaddr*) physical pointer to the @c base @c file of a header
  * @param alloc: (size_t) number of bytes needed to allocate
  * @param header_size : (size_t) number of bytes in the given header type
  * @param num_headers : (ot_int) number of headers to search through
  * @retval vaddr : virtual address of the spot in heap to put data.
  *                 returns @c NULL_vaddr @c if heap has no room
  *
  * @c sub_find_empty_heap() @c runs in quadratic time based on the @c num_headers
  * parameter.  Implementations on systems that are less memory constrained 
  * could be made faster.  @c sub_find_empty_heap() @c only runs when adding a ISF
  * file, which may never even happen.
  */
vaddr sub_find_empty_heap(  vaddr heap_base, vaddr heap_end, 
                        vaddr header, ot_uint new_alloc, ot_int num_headers);



/** @brief Defragments a given heap space
  * @param base : (vaddr) base vaddr of the heap space to defragment
  * @param size : (size_t) number of bytes in heap space
  * @retval vaddr : Non-zero on failure
  *
  * Run this function to clean out fragmented empty space in a heap.  For 
  * example, if @c sub_find_empty_heap() @c returns NULL_vaddr
  *
  * Not implemented at time being.
  */
ot_u8 sub_defragment_heap(vaddr base, ot_uint window);



/** @brief Searches for a header of given ID
  * @param search_id : (ot_u8) ID value to look for
  * @param start_id: (ot_u8*) pointer to first ID to start searching
  * @param header_size : (size_t) number of bytes in the given header type
  * @param num_headers : (ot_int) number of headers to search through
  * @retval ot_int : Number of headers searched through until ID was found.
  *                  -1 if no headers were found.
  */
vaddr sub_header_search(vaddr header, ot_u8 search_id, ot_int num_headers);








// Indexed Subroutines
static const ot_int num_files[]		= { GFB_NUM_USER_FILES, ISFS_NUM_USER_LISTS, ISF_NUM_USER_FILES };
static const ot_int stock_files[]	= { GFB_NUM_STOCK_FILES, ISFS_NUM_STOCK_LISTS, ISF_NUM_STOCK_FILES };
static const ot_u8 ext_files[]		= { 0, 0, ISF_NUM_EXT_FILES };

static const vaddr header_base[]    = { GFB_Header_START_USER, ISFS_Header_START_USER, ISF_Header_START_USER };

static const ot_u8 block_type[] = {	(((GFB_NUM_USER_FILES>0)*8)+((GFB_NUM_STOCK_FILES>0)*1)),
									4,
									(((ISF_NUM_USER_FILES>0)*8)+((ISF_NUM_EXT_FILES>0)*2)+((ISF_NUM_STOCK_FILES>0)*1)),
									0 };



vlFILE* file_new(ot_u8 block_id, ot_u8 id, ot_u8 mod, ot_u8 max_length) {
#if (OT_FEATURE(VLNEW) == ENABLED)
	static const vaddr heap_base[]      = { GFB_HEAP_USER_START, ISFS_HEAP_USER_START, ISF_HEAP_USER_START };
	static const vaddr heap_end[]       = { GFB_HEAP_END, ISFS_HEAP_END, ISF_HEAP_END };

    ot_uint 	block_filesize[3];
    Twobytes	scratch;
    vl_header   new_header;
    
    //block_id must be restricted to 0-3 prior to call
    if (block_type[block_id] & 8) {
#		if ((GFB_HEAP_BYTES > 0) && (GFB_NUM_USER_FILES > 0))
    		block_filesize[0] = GFB_FILE_BYTES;
#		endif
#		if (ISFS_NUM_USER_LISTS > 0)
    		block_filesize[1] = ISFS_MAX_default;
#		endif
#		if (ISF_NUM_USER_FILES > 0)
    		block_filesize[2] = ((ot_uint)max_length+1) & ~1
#		endif

    	new_header.length   = 0;
    	new_header.alloc    = block_filesize[block_id];
    	scratch.ubyte[0]    = id;
    	scratch.ubyte[1]    = mod;
    	new_header.idmod    = scratch.ushort;
    	new_header.mirror   = NULL_vaddr;
    
    	return sub_new_file(&new_header,
                        heap_base[block_id], 
                        heap_end[block_id],
                        header_base[block_id], 
                        num_files[block_id] );
    }
    else
#endif
    return NULL;
}




ot_u8 file_userheap_check(ot_u8 block_id, ot_u8 id) {
	static const ot_u8 idbound_low[] = {
		GFB_NUM_STOCK_FILES,
		(ISFS_ID_extended_service),
		(ISF_NUM_M1_FILES+ISF_NUM_M2_FILES),
		255
	};

	static const ot_u8 idbound_high[] = {
		255,
		255,
		(255-ISF_NUM_EXT_FILES),
		0
	};

	// block_id must be restricted to 0-3 prior to call
	return ((id >= idbound_low[block_id]) && (id <= idbound_high[block_id]));
}



ot_u8 file_delete_check(ot_u8 block_id, ot_u8 id) {
#if (OT_FEATURE(VL_NEW) == ENABLED)
	file_userheap_check(block_id, id);
#else
	return 0;
#endif
}


vaddr file_search(ot_u8 block_id, ot_u8 id) {
	//block_id must be restricted to 0-3 prior to call
	if (block_type[block_id] & 1) {
		// Block supports array search on stock files
		if (file_userheap_check(block_id, id) == 0) {
			if (id > (255-ext_files[block_id])) {
				id = (ot_u8)((ot_int)stock_files[block_id] + ((ot_int)id-256));
			}
			return (sizeof(vl_header) * id) + header_base[block_id];
		}
	}
	if (block_type[block_id] & 0x0C) {
		// Block has unstructured stock files, or user files
		return sub_header_search( header_base[block_id], id, num_files[block_id] );
	}

	return NULL_vaddr;
}




ot_u8 file_validate(vaddr* header, id_tmpl* user_id, ot_u8 access_mod) {
    /// 1. Bail if header is NULL
    if (*header == NULL_vaddr) {
        return 0x01;
    }
    
    /// 2. Authenticate, when it's not a su call
    if (user_id != NULL) {
        Twobytes filemod;
        filemod.ushort = vworm_read(*header + 4);
    
        if ( auth_check(filemod.ubyte[1], access_mod, user_id) == 0 ) {
            return 0x04;
        }
    }

    return 0;
}


    




// Public Functions
// Veelite init function

#ifndef EXTF_vl_init
void vl_init() {
    ot_int i;
    
    /// Initialize environment variables
    for (i=0; i<OT_FEATURE(VLFPS); i++) {
        vl_file[i].header   = NULL_vaddr;
        vl_file[i].start    = 0;
        vl_file[i].length   = 0;
        vl_file[i].read     = NULL;
        vl_file[i].write    = NULL;
    }

    /// Initialize core 
    /// @note This should be done already in platform_poweron()
    //vworm_init();
    
    // Copy to mirror
    ISF_loadmirror();
    
    
#if (CC_SUPPORT == SIM_GCC)

    // Set up memory files if using Simulator
    VWORM_Heap  = (ot_u16*)&(NAND.ubyte[VWORM_BASE_PHYSICAL]);
    //VSRAM_Heap  = (ot_u16*)&(NAND.ubyte[VSRAM_BASE_PHYSICAL]);    //vsram declared in veelite_core
    
    // Write dummy data to M1TAG file
    //M1TAG.manuf_id      = 0x0001;
    //M1TAG.serial        = 0x01020304;
    //M1TAG.model_no      = 0x01020304;
    //M1TAG.fw_version    = 0x00000001;
    //M1TAG.max_response  = 255;    
#endif
}
#endif





// General File Functions
#ifndef EXTF_vl_get_fp
vlFILE* vl_get_fp(ot_int fd) {
    return &vl_file[fd];
}
#endif


#ifndef EXTF_vl_get_fd
ot_int vl_get_fd(vlFILE* fp) {
    ot_int fd;

    fd  = (ot_int)((ot_u8*)fp - (ot_u8*)vl_file);
    fd /= sizeof(vlFILE);

    return fd;
}
#endif


#ifndef EXTF_vl_new
ot_u8 vl_new(vlFILE** fp_new, vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, ot_uint max_length, id_tmpl* user_id) {
#if (OT_FEATURE(VLNEW) == ENABLED)
    vaddr header;
    //sub_vaddr search_fn;
    //sub_new   new_fn;
    
    /// 1. Authenticate, when it's not a su call
    if (user_id != NULL) {
        if ( auth_check(VL_ACCESS_USER, VL_ACCESS_W, user_id) == 0 ) {
            return 0x04;
        }
    }

    block_id--;
    block_id &= 3;

    /// 2. Make sure the file is not already there
    header = file_search(block_id, data_id);
    if (header != NULL_vaddr) {
        return 0x02;
    }
    
    /// 3. Create the file, if there is space
    *fp_new = file_new(block_id, data_id, mod, max_length);
    if (*fp_new == NULL) {
        return 0x06;
    }
    
    return 0;
#else
    return 255;
#endif
}
#endif



#ifndef EXTF_vl_delete
ot_u8 vl_delete(vlBLOCK block_id, ot_u8 data_id, id_tmpl* user_id) {
#if (OT_FEATURE(VLNEW) == ENABLED)
    vaddr header = NULL_vaddr;
    ot_u8 retval;

    block_id--;
    block_id & 3;

    /// 1. Get the header from the supplied Block ID & Data ID
    if (file_delete_check(block_id, data_id) != 0) {
        header = file_search(block_id, data_id);
    }
    
    /// 2. Validate that file exists and read+write access is OK, then delet
    retval = file_validate(&header, user_id, VL_ACCESS_RW);
    if (retval == 0) {
        sub_delete_file(header);
    }
    
    return retval;
#else
    return 255; //error, delete disabled
#endif
}
#endif



#ifndef EXTF_vl_getheader
ot_u8 vl_getheader_vaddr(vaddr* header, vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, id_tmpl* user_id) {

	block_id--;
	block_id &= 3;

    /// 1. Get the header from the supplied Block ID & Data ID
    *header = file_search(block_id, data_id);

    /// 2. Validate that file exists and read+write access is OK
    return file_validate(header, user_id, mod);
}
#endif



#ifndef EXTF_vl_getheader
ot_u8 vl_getheader(vl_header* header, vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, id_tmpl* user_id) {
    vaddr   header_vaddr = NULL_vaddr;
    ot_u8   output;

    output = vl_getheader_vaddr(&header_vaddr, block_id, data_id, mod, user_id);
    if (output == 0) {
        sub_copy_header( header_vaddr, (ot_u16*)header );
    }
    
    return output;
}
#endif



#ifndef EXTF_vl_open_file
vlFILE* vl_open_file(vaddr header) {
    vlFILE* fp;

    fp = sub_new_fp();
    
    if (fp != NULL) {
        fp->header  = header;
        fp->alloc   = vworm_read(header + 2);               //alloc
        fp->idmod   = vworm_read(header + 4);
        fp->start   = vworm_read(header + 8);               //mirror base addr
        
        if (fp->start != NULL_vaddr) {
            ot_u16 mlen = fp->start;
            fp->start  += 2;
            fp->write   = &vsram_mark;
            fp->read    = &vsram_read;
            fp->length  = vsram_read(mlen);
        }
        else {
            fp->write   = &vworm_write;
            fp->read    = &vworm_read;
            fp->length  = vworm_read(header + 0);           //length
            fp->start   = vworm_read(header + 6);           //vworm base addr
        }
    }
    return fp;
}
#endif



#ifndef EXTF_vl_open
vlFILE* vl_open(vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, id_tmpl* user_id) {
    vaddr header = NULL_vaddr;

    if (vl_getheader_vaddr(&header, block_id, data_id, mod, user_id) == 0) {
        return vl_open_file(header);
    }
    return NULL;
}
#endif



#ifndef EXTF_vl_chmod
ot_u8 vl_chmod(vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, id_tmpl* user_id) {
    vaddr header = NULL_vaddr;
    ot_u8 output;

    output = vl_getheader_vaddr(&header, block_id, data_id, VL_ACCESS_RW, user_id);
    if (output == 0) {
        Twobytes idmod;
        idmod.ubyte[0]  = data_id;
        idmod.ubyte[1]  = mod;
        
        sub_write_header((header+4), &idmod.ushort, 2);
    }

    return output;
}
#endif



#ifndef EXTF_vl_read
ot_u16 vl_read( vlFILE* fp, ot_uint offset ) {
    return fp->read( (ot_uint)(offset+fp->start) );
}
#endif



#ifndef EXTF_vl_write
ot_u8 vl_write( vlFILE* fp, ot_uint offset, ot_u16 data ) {
    if (offset >= fp->alloc) {
        return 255;
    }
    if (offset >= fp->length) {
        fp->length = offset+2;
    }
    
    return fp->write( (offset+fp->start), data);
}
#endif


#ifndef EXTF_vl_load
ot_uint vl_load( vlFILE* fp, ot_uint length, ot_u8* data ) {
    Twobytes    scratch;
    ot_uint     cursor;

    if (length > fp->length) {
        length = fp->length;
    }
    
    cursor      = fp->start;
    length      = cursor+length;
    
    for (; cursor<length; cursor++) {
        ot_u8 align = (cursor & 1);
        if (align == 0) {
            scratch.ushort = fp->read(cursor);
        }
        *data++ = scratch.ubyte[align];            
    }

    return (length - fp->start);
}
#endif


#ifndef EXTF_vl_store
ot_u8 vl_store( vlFILE* fp, ot_uint length, ot_u8* data ) {
    Twobytes    scratch;
    ot_uint     cursor;
    ot_u8       test;

    if (length > fp->alloc) {
        return 255;
    }
    
    fp->length  = length;
    cursor      = fp->start;
    length      = cursor+length;
    
    for (test=0; cursor<length; cursor+=2) {
        scratch.ubyte[0]    = *data++;
        scratch.ubyte[1]    = *data++;
        test               |= fp->write(cursor, scratch.ushort);
    }

    return test;
}
#endif



#ifndef EXTF_vl_close
ot_u8 vl_close( vlFILE* fp ) {
    if (FP_ISVALID(fp)) {        
        if (fp->read == &vsram_read) {
            ot_u16* mhead;
            mhead   = (ot_u16*)vsram_get(fp->start-2);
            *mhead  = (*mhead != fp->length) ? fp->length : *mhead;
        }
        else if ( vworm_read(fp->header+0) != fp->length ) {
            sub_write_header( (fp->header+0), &(fp->length), 2);
        }
            
        // Kill file attributes
        fp->start   = 0;
        fp->length  = 0;
        //fp->header  = NULL_vaddr;
        fp->read    = NULL;
        fp->write   = NULL;
        
        return 0;
    }
    return 255;
}
#endif



#ifndef EXTF_vl_checklength
ot_uint vl_checklength( vlFILE* fp ) {
#   ifdef DEBUG_ON
        return (fp->read != NULL) ? fp->length : 0;
#   else
        return fp->length;
#   endif
}
#endif



#ifndef EXTF_vl_checkalloc
ot_uint vl_checkalloc( vlFILE* fp ) {
#   ifdef DEBUG_ON
        return (fp->read != NULL) ? fp->alloc : 0;
#   else
        return fp->alloc;
#   endif
}
#endif











/// Public Block Functions
/// Usually these are just shortcuts

vlFILE* GFB_open_su( ot_u8 id ) {
#   if ( GFB_HEAP_BYTES > 0 )
        return vl_open(VL_GFB_BLOCKID, id, VL_ACCESS_SU, NULL);
#   else
        return NULL;
#   endif
}

vlFILE* ISFS_open_su( ot_u8 id ) {
    return vl_open(VL_ISFS_BLOCKID, id, VL_ACCESS_SU, NULL);
}

vlFILE* ISF_open_su( ot_u8 id ) {
    return vl_open(VL_ISF_BLOCKID, id, VL_ACCESS_SU, NULL);
}


vlFILE* GFB_open( ot_u8 id, ot_u8 mod, id_tmpl* user_id ) {
#   if ( GFB_HEAP_BYTES > 0 )
        return vl_open(VL_GFB_BLOCKID, id, mod, user_id);
#   else
        return NULL;
#   endif
}

vlFILE* ISFS_open( ot_u8 id, ot_u8 mod, id_tmpl* user_id ) {
    return vl_open(VL_ISFS_BLOCKID, id, mod, user_id);
}

vlFILE* ISF_open( ot_u8 id, ot_u8 mod, id_tmpl* user_id ) {
    return vl_open(VL_ISF_BLOCKID, id, mod, user_id);
}

    
ot_u8 GFB_chmod_su( ot_u8 id, ot_u8 mod ) {
#   if ( GFB_HEAP_BYTES > 0 )
        return vl_chmod(VL_GFB_BLOCKID, id, mod, NULL);
#   else
        return ~0;
#   endif
}
    
ot_u8 ISFS_chmod_su( ot_u8 id, ot_u8 mod ) {
    return vl_chmod(VL_ISFS_BLOCKID, id, mod, NULL);
}

ot_u8 ISF_chmod_su( ot_u8 id, ot_u8 mod ) {
    return vl_chmod(VL_ISFS_BLOCKID, id, mod, NULL);
}


ot_u8 ISF_syncmirror() {
#   if (ISF_MIRROR_HEAP_BYTES > 0)
        return sub_isf_mirror(MIRROR_TO_FLASH);
#   else
        return 0;
#   endif
}

ot_u8 ISF_loadmirror() {
#   if (ISF_MIRROR_HEAP_BYTES > 0)
        return sub_isf_mirror(MIRROR_TO_SRAM);
#   else
        return 0;
#   endif
}









/// Private Subroutines

ot_u8 sub_isf_mirror(ot_u8 direction) {
    vaddr   header;
    vaddr   header_base;
    vaddr   header_alloc;
    vaddr   header_mirror;
    //vaddr   header_end;
    ot_int  i;
    ot_u16* mirror_ptr;
    
    // Go through ISF Header array
    header = ISF_Header_START; 
    for (i=0; i<ISF_NUM_STOCK_FILES; i++, header+=sizeof(vl_header)) {
    
        //get header data
        header_alloc    = vworm_read(header+2);
        header_base     = vworm_read(header+6);
        header_mirror   = vworm_read(header+8);

        // Copy vworm to mirror if there is a mirror
        // 0. Skip unmirrored or uninitialized, or unallocated files
        // 1. Resolve Mirror Length (in vsram it is right ahead of the data)
        // 2. Load/Save Mirror Data (header_alloc is repurposed)
        if ((header_mirror != NULL_vaddr) && (header_alloc  != 0)) {
        	mirror_ptr = (ot_u16*)vsram_get(header_mirror);
            if (direction == MIRROR_TO_SRAM) {  // LOAD
                *mirror_ptr = vworm_read(header+0);
            }
            if (header_base == NULL_vaddr) {	// EXIT if file is mirror-only
            	continue;
            }
            if (direction != MIRROR_TO_SRAM) {  // SAVE
                vworm_write((header+0), *mirror_ptr);
            }
            
            header_alloc = header_base + *mirror_ptr;
            mirror_ptr++;
            for ( ; header_base<header_alloc; header_base+=2, mirror_ptr++) {
                if (direction == MIRROR_TO_SRAM) {
                    *mirror_ptr = vworm_read(header_base);
                }
                else { 
                    vworm_write(header_base, *mirror_ptr);
                }
            }
        }
    }
    
    return 0;
}




/// Generic Subroutines

vlFILE* sub_new_fp() {
#if (OT_FEATURE(VLFPS) < 8)
    ot_int fd;

    for (fd=0; fd<OT_FEATURE(VLFPS); fd++) {
        if (vl_file[fd].read == NULL) 
            return &vl_file[fd];
    }
#else
        ///@todo do a binary search
#endif
    
    return NULL;
}


vlFILE* sub_new_file(vl_header* new_header, vaddr heap_base, vaddr heap_end, vaddr header_base, ot_int header_window ) {
#if (OT_FEATURE(VLNEW) == ENABLED)
    //vlFILE* fp;
    //vaddr   new_base    = 0;
    vaddr   header_addr = 0;

    // Find where to put the new header, and if it's full
    header_addr = sub_find_empty_header( header_base, header_window );
    if (header_addr == NULL_vaddr) 
        return NULL;
    
    // Find where to put the new data, and if heap is full
    new_header->base = sub_find_empty_heap( heap_base, 
                                            heap_end, 
                                            header_base, 
                                            (ot_uint)new_header->alloc, 
                                            header_window );
    if (new_header->base == NULL_vaddr) 
        return NULL;
    
    // Make sure new header has the right base address
    //new_header->base = new_base;
    
    // Write header to the header array
    sub_write_header(header_addr, (ot_u16*)new_header, sizeof(vl_header));
    
    // Open a file, now that data is allocated
    //fp = vl_open_file( header_addr );
    
    //commented out... seems to be useless (and broken) legacy code
    //if (FP_ISVALID(fp)) {
        //fp->start   = 0;
        //fp->length  = 0;
    //}
    
    //return fp;
    
    return vl_open_file( header_addr );
#else
    return NULL;
#endif
}



void sub_delete_file(vaddr del_header) { 
#if (OT_FEATURE(VLNEW) == ENABLED)
    vaddr   header_base;
    ot_u16  header_alloc;
    
    header_alloc    = (ot_u16)vworm_read(del_header+2);
    header_base     = (vaddr)vworm_read(del_header+6);
    
    // Wipe the old data and mark header as deleted
    vworm_wipeblock(header_base, header_alloc);
    vworm_mark((del_header+2), 0);                //alloc
    vworm_mark((del_header+6), NULL_vaddr);       //base
#endif
}



vaddr sub_header_search(vaddr header, ot_u8 search_id, ot_int num_headers) {
    Twobytes    idmod;
    ot_u16      base;

    for (; num_headers > 0; num_headers--) {
        base            = vworm_read(header + 6);
        idmod.ushort    = vworm_read(header + 4);
        
        if ( base != 0 && base != 0xFFFF) {
            if (idmod.ubyte[0] == search_id) 
                return header;
        }
        
        header += sizeof(vl_header);
    }
    return NULL_vaddr;
}


void sub_copy_header( vaddr header, ot_u16* output_header ) {
    ot_int i;
    ot_int copy_length = sizeof(vl_header) / 2;

    for (i=0; i<copy_length; i++) {
        output_header[i] = vworm_read(header);
        header += 2;
    }
}


void sub_write_header(vaddr header, ot_u16* data, ot_uint length ) {
    ot_int i;

    for (i=0; i<length; i+=2, data++) {
        vworm_write( (header+i), *data);
    }
}


vaddr sub_find_empty_header(vaddr header, ot_int num_headers) {
    vaddr header_base;
    
    for (; num_headers>0; num_headers--) {
        header_base = vworm_read( (header+6) );
        
        if ( header_base == NULL_vaddr ) {
            return header;
        }
        header += sizeof(vl_header);
    }
    
    return NULL_vaddr;
}


/// @todo This checks out in testing, although it is a complex function that
///       probably merits further testing
vaddr sub_find_empty_heap(  vaddr heap_base, vaddr heap_end, 
                        vaddr header, ot_uint new_alloc, ot_int num_headers) {
#if (OT_FEATURE(VLNEW) == ENABLED)
    //Search all header combinations to find:
    // - pairs of adjacent files in the heap
    // - gaps between these pairs
    // - the smallest gap that is big enough.
    vaddr   loop1           = header;
    vaddr   loop1_base;
    ot_uint loop1_alloc;
    vaddr   loop2;
    vaddr   loop2_base;
    //ot_uint loop2_alloc;	///@todo figure out why this is disused
    
    ot_int  i;
    ot_int  j;
    ot_int  gap;
    ot_int  closest_gap     = (ot_int)(heap_end - heap_base);
    ot_int  bestfit_alloc   = (ot_int)(32767);
    vaddr   bestfit_base    = NULL_vaddr;
    //vaddr   loop1_end       = heap_base;
    
    for (i=0; i<num_headers; i++, loop1+=sizeof(vl_header) ) {
        loop1_alloc = vworm_read(loop1 + 2);                                    // load alloc (max) from header
        loop1_base  = vworm_read(loop1 + 6);                                    // load base from header
        
        if ( loop1_base != NULL_vaddr ) {                                       // skip if header is deleted, or empty
            heap_base   = (vaddr)(loop1_base + loop1_alloc);
            loop2       = header;
        
            for (j=0; j<num_headers; j++, loop2+=sizeof(vl_header) ) {
                //loop2_alloc = vworm_read(loop2 + 2);
                loop2_base  = vworm_read(loop2 + 6);
            
                if ( (loop2_base != NULL_vaddr) && (loop2_base > heap_base) ) { // if header is valid ...
                    gap = (ot_int)(loop2_base - heap_base);                     // calc gap between the two files
                
                    if (gap < closest_gap) {                                    // The closest gap ...
                        closest_gap = gap;                                      // will be between two adjacent ...
                        //heap_base   = loop1_end;                                // files in the heap.
                    }
                }
            }
        }
        //if (closest_gap >= new_alloc) {                                         // If the gap between loop1 and next file ...
        //    if (closest_gap < bestfit_alloc) {                                  // is big enough for the data we need ...
        //        bestfit_alloc   = closest_gap;                                  // and is smaller than other big-enough gaps ...
        //        bestfit_base    = heap_base;                                    // then it is the gap we will write into.
        //    }
        //}
        if ((closest_gap >= new_alloc) && (closest_gap < bestfit_alloc))  {
            bestfit_alloc   = closest_gap;
            bestfit_base    = heap_base;
        }

    }
    
    return bestfit_base;
#else
    return NULL_vaddr;
#endif
}




//Save sub_defragment_heap for a rainy day
ot_u8 sub_defragment_heap(vaddr base, ot_uint window) {
    return ~0;
}






/*
ot_u8 M1TAG_write( M1TAG_struct* new_m1tag ) {
    ///@todo for now, only done at compile time
    return ~0;
}



ot_u8 M1TAG_get( M1TAG_struct* m1tag ) {
#   if (M1_FEATURESET == ENABLED)
        ot_int i;
        ot_u16* input;
        ot_u16* output;
        
        input   = (ot_u16*)M1TAG;
        output  = (ot_u16*)m1tag;
    
        for (i=0; i<(sizeof(M1TAG_Struct)/2); i++) {
            output[i] = input[i];
        }
    
        return 0;
        
#   else
        return ~0;
        

}
*/




#endif

