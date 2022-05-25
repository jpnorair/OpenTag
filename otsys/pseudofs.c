/*  Copyright 2022, JP Norair
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
  * @file       /otsys/pseudofs.c
  * @author     JP Norair
  * @version    R100
  * @date       15 Apr 2022
  * @brief      Pseudo File System for Core Data Management
  * @ingroup    PseudoFS
  *
  * Pseudo FS replaces Veelite in OpenTag 2.0.  It is more general purpose 
  * than Veelite and more efficient for the basic types of workflows that 
  * OpenTag ended-up using the most.  It expects that all file content data is
  * buffered/mirrored in RAM during runtime.  A Veelite shim exists for 
  * backwards compatibility with veelite.
  *
  ******************************************************************************
  */

#include <otstd.h>

#if ((OT_FEATURE(PSEUDOFS) == ENABLED) || (OT_FEATURE(VEELITE) == ENABLED))

#include <platform/config.h>

#include <otlib/utils.h>
#include <otlib/auth.h>
#include <otlib/memcpy.h>

#include <otsys/pseudofs.h>
#include <otsys/time.h>

// For allocation features.  Not compiled or used if the allocation feature disabled.
#if (OS_FEATURE(MALLOC) != ENABLED)
#	include "pfs_ext/memmgr.h"
#	define PFS_MALLOC			memmgr_alloc
#	define PFS_FREE				memmgr_free
#	define PFS_MALLOC_INIT()	memmgr_init()
#else
#	define PFS_MALLOC			malloc
#	define PFS_FREE				free
#	define PFS_MALLOC_INIT()	do { } while(0)
#endif


/// PFS Handle ROM, Handle RAM, Contents ROM, Contents RAM
///@todo need to implement this, which will require a new defaults file table
///      implementation.  It would be nice to have a file table generator in
///      Python, which takes a JSON input.
#define PFS_HANDLE_ROM
#define PFS_HANDLE_RAM
#define PFS_CONTENTS_ROM
#define PFS_CONTENTS_RAM


///@todo decide if it's even worth having this file table, or if full headers should
/// be buffered into RAM, thus allowing an arbitrary number of open files.


static const ot_u8 data_offset[4] = {
	0,
	sizeof(pfs_drechdr_t),
	sizeof(pfs_bfhdr_t),
	0
};

/** @brief fdtab object: stores assigned file objects.
  *
  * There is a maximum number of file objects that can be open simultaneously.
  * File data can be used while a file is closed, but the act of opening and 
  * closing the file affects metadata, which is required for some features.
  */
typedef struct {
	ot_int next;
	pfs_file_t data[PFS_PARAM_MAXFILEPTRS];
} pfs_fdtab_t;

static pfs_fdtab_t fdtab;




#if PFS_FEATURE_FSALLOC
	/** @brief uaftab object: UAF = User Allocated Files.
	  *
	  */
	typedef struct {
		ot_int next;
		ot_int count;
		pfs_handle_t handle[PFS_PARAM_MAXUSERFILES];
	} pfs_uaftab_t;

	static pfs_uaftab_t uaftab;
#endif



/** @brief internal macros
  *
  * FP_ISVALID(fp_VAL): checks if a file pointer is valid.  There are two 
  *    implementations, one is faster and one more robust.
  *
  * HDR_ISREADONLY(fp_VAL):
  */
  
#define FD_ISVALID(fd_VAL)	\
	((ot_uint)(fd_VAL) < PFS_PARAM_MAXFILEPTRS)

#ifdef __PFS_NO_PROTECTION__
#	define FP_ISVALID(fp_VAL)  (fp_VAL != NULL)
#else
#	define FP_ISVALID(fp_VAL) \
	((fp_VAL >= &fdtab.data[0]) && (fp_VAL < &fdtab.data[fdtab.next]))
#endif

#define HDR_ISREADONLY(fp_VAL)	\
	((((pfs_file_t*)(fp_VAL))->hdr >= PFS_FLASH_START) && (((pfs_file_t*)(fp_VAL))->hdr < PFS_FLASH_END))

#define ID_INRANGE(id_VAL)	((ot_u16)(id_VAL) > 1024)






/** @brief ram-backed file headers (for dynamically created files)
  *
  * Files prepared at compile-time have Flash-backed headers.  If dynamically
  * allocated files are enabled, we need to buffer the headers in ram during runtime.
  * Dynamic files get wiped on reset.
  */
#if (PFS_FEATURE_PFSALLOC)
	///@todo port this when I have an idea how to implement local headers
	static vlFSHEADER vlfs;
#endif


/** VWORM Memory Allocation
  * Base positions and maximum group allocations for data files stored in
  * VWORM.  The values are taken from platform.h.
  * Direct addressing may only work for reads, depending on the nature of the
  * VWORM memory.  NAND Flash, for example, cannot be written to arbitrarily.
  */
#if (CC_SUPPORT == SIM_GCC)
	///@todo port this when I have an idea what to do about it
#   include <otplatform.h>
    ot_u16* VWORM_Heap;
    ot_u16* VSRAM_Heap;
#endif




static const pfs_handle_t* sub_get_handle(ot_u16 file_id);

static void* sub_convert_front(const pfs_handle_t* handle);

static void sub_update_times(pfs_stat_t* status, ot_bool is_modded);

static void* sub_alloc(pfs_type_e type, ot_u16 alloc);

static void sub_delete(pfs_handle_t* handle);







OT_WEAK ot_int pfs_init(void) {
/// pfs_init() needs to perform unpacking of the nonvolatile memory bank as well as any
/// data memory preparations / initializations.
    ot_int i;

    /// Initialize environment variables
    ot_memset((void*)&fdtab, 0, sizeof(pfs_fdtab_t));
#	if PFS_FEATURE_FSALLOC
		ot_memset(uaftab, 0, sizeof(pfs_uaftab_t));
		PFS_MALLOC_INIT();
#	endif

    /// Initialize core
    /// @note This should be done already in platform_poweron()
    //vworm_init();

    /// Unpack the file contents from non-volatile storage
    if (pfs_unpack() != 0) {
    	///@todo hardware fault
    }

#	if (CC_SUPPORT == SIM_GCC)
	///@todo port this when I have an idea what to do about it
    // Set up memory files if using Simulator
    VWORM_Heap  = (ot_u16*)&(NAND.ubyte[VWORM_BASE_PHYSICAL]);
    //VSRAM_Heap  = (ot_u16*)&(NAND.ubyte[VSRAM_BASE_PHYSICAL]);    //vsram declared in veelite_core
#	endif

    return 0;
}


OT_WEAK pfs_file_t* pfs_get_fp(ot_int fd) {
	pfs_file_t* fp;
	
#	ifdef __PFS_NO_PROTECTION__
		fp = &fdtab.data[fd];
#	else
		fp = FD_ISVALID(fd) ? &fdtab.data[fd] : NULL;
#	endif

    return fp;
}


OT_WEAK ot_int pfs_get_fd(pfs_file_t* fp) {
    ot_uint fd;
    
	if (!FP_ISVALID(fp)) {
		return -1;
	}

    fd  = (ot_int)((ot_u8*)fp - (ot_u8*)fdtab.data);
    fd /= sizeof(pfs_file_t);

    return fd;
}


OT_WEAK ot_int pfs_new(pfs_type_e file_type, ot_u16 alloc, pfs_bfflags_t flags,
				const pfs_user_t* user_id) {
#if (PFS_FEATURE_ALLOC)
	pfs_handle_t* new_file;

    /// 1. If the user_id is not null, then this is an attempt at an authenticated
    ///    operation.  Only some file types support authenticated operations.
    if (user_id != NULL) {
    	if (file_type < PFS_Bytefile) {
    		return -4;
    	}
        if ( auth_check(BF_ACCESS_USER, BF_ACCESS_W, (const void*)user_id) == 0 ) {
            return -4;
        }
    }

    /// 2. Find the area and id that suit the allocation requirement.
    new_file = sub_alloc(file_type, alloc);
    if (new_file == NULL) {
    	return -6;
    }

    /// 3. Put the extended parameters into the ByteFile header, if the new file
    ///    is indeed a ByteFile.
    if (file_type == PFS_Bytefile) {
    	pfs_bfhdr_t* bfhdr = sub_convert_front(new_file);
    	if (bfhdr == NULL) {
			return -255;
		}
			
    	bfhdr->flags    = flags;
    	bfhdr->length   = 0;
    	bfhdr->action   = NULL;
    	sub_update_times(&bfhdr->stat, True);
    }

	/// 4. The new file's specifier is where to find its ID, which must be returned.
    return new_file->spec.id;

#else
	// Allocation Features not compiled-in
    return -255;

#endif
}


OT_WEAK ot_int pfs_delete(ot_u16 file_id, const pfs_user_t* user_id) {
#if (PFS_FEATURE_ALLOC)
    const pfs_handle_t* handle;

    /// 1. Get the file handle, which will inform if the File can be
    ///    deleted.
    handle = sub_get_handle(file_id);
    if (handle == NULL) {
    	return -1;
    }
    if (handle->spec.romdata != PFS_ROMDATA_NONE) {
    	return -3;
    }

	///2. If the file is a ByteFile or EXT, and if user_id is not
	///   implied root (user_id == NULL), do authentication check.
	if (user_id != NULL) {
		ot_u8 modbits;
		
		if (handle->spec.type < PFS_Bytefile) {
			return -4;
		}
		else if (handle->spec.type == PFS_Bytefile) {
			pfs_bfhdr_t* bfhdr = sub_convert_front(handle);
			if (bfhdr == NULL) {
				return -255;
			}
			modbits = (ot_u8)(bfhdr->flags.modbits);
		}
		else {
			modbits = BF_ACCESS_USER;
		}
		
		if ( auth_check(modbits, BF_ACCESS_RW, user_id) == 0 ) {
			return -4;
		}
	}
    
    /// 3. sub_delete() will remove the file header and data from FS memory.
    sub_delete(handle);

    return 0;
    
#else
	// Allocation Features not compiled-in
    return -255;

#endif
}


OT_WEAK ot_int pfs_open(pfs_file_t** fp, ot_u16 file_id, pfs_mode_e mode, const pfs_user_t* user_id) {
	const pfs_handle_t* handle;
	ot_int fd;
    
    ///1. Input checks, fp cannot be null.  Set its reference to NULL as default.
    if (fp == NULL) {
    	return -255;
    }
    *fp = NULL;
    
    ///2. If there are no more available file descriptors, return error.
    if (fdtab.next >= PFS_PARAM_MAXFILEPTRS) {
    	return -16;
    }
    
    ///3. Locate the file handle and copy the necessary elements into fp memory
    handle = sub_get_handle(file_id);
    if (handle == NULL) {
    	return -1;
    }

    ///4. Make sure the requested mode is compatible.  Some files are by nature
    ///   read only.
    if ((handle->spec.romdata == PFS_ROMDATA_ALL) && (mode > PFS_Read)) {
        return -3;
    }

    ///5. Once checks are complete, add the file descriptor & uptake the handle.
    ///   Bytefile types have more features, and for these we need to save the
    ///   the access mode in the bytefile header.
    fd			= fdtab.next++;
    *fp         = &fdtab.data[fd];
    (*fp)->hdr  = sub_convert_front(handle);

    if (handle->spec.type == PFS_Bytefile) {
        pfs_bfhdr_t* bfhdr = (*fp)->hdr;
        bfhdr->flags.mode = mode;
    }

    (*fp)->data = (*fp)->hdr + data_offset[handle->spec.type];
    (*fp)->spec = handle->spec;
    (*fp)->alloc= handle->alloc;
    
    ///6. Set the OPEN bit in the file spec
    (*fp)->spec.flags = PFS_FLAG_ISOPEN;
    
	return fd;
}


OT_WEAK const ot_u8* pfs_read(pfs_file_t* fp) {
/// Any type of file can be read.
	if (!FP_ISVALID(fp)) {
		return NULL;
	}
	return (const ot_u8*)fp->data;
}


OT_WEAK ot_u8* pfs_write(pfs_file_t* fp) {
/// Files with "read-only" tag cannot be written.
	ot_u8* data;

	if (!FP_ISVALID(fp)) {
		return NULL;
	}

	///2. Don't allow access to read-only files.
	///   Set modded flags to consider write.
	if (fp->spec.romdata == PFS_ROMDATA_ALL) {
		data = NULL;
	}
	else {
		fp->spec.flags = (PFS_FLAG_ISOPEN | PFS_FLAG_ISWRITTEN);
		data = (ot_u8*)fp->data;
	}
	
	return data;
}


OT_WEAK ot_u8* pfs_readwrite(pfs_file_t* fp) {
	return pfs_write(fp);
}


OT_WEAK ot_int pfs_close(pfs_file_t* fp, void* w_cursor) {
/// pfs_close() needs to release the file descriptor and file pointer from local
/// memory and perform any necessary closing actions on the file.
	ot_int retval = 0;

	///1. initial parameter checks.  w_cursor is checked later.
	if (!FP_ISVALID(fp)) {
		return -255;
	}

	///2. if the file is a ByteFile, we need to grab its BF header and deal with it.
	///   This may include updating access & modfication times, updating the
	///   write cursor, and running file actions.
	if (fp->spec.type == PFS_Bytefile) {
		ot_long new_length;
		pfs_bfhdr_t* bfhdr 	= fp->hdr;
		ot_bool is_modded 	= (ot_bool)(fp->spec.flags & PFS_FLAG_ISWRITTEN);
	
		sub_update_times(&bfhdr->stat, is_modded);

		if (is_modded) {
			/// The new file length will be from the supplied cursor or the default.
			/// The Default cursor depends on the mode, write or append.
			if (w_cursor != NULL) {
				new_length = (ot_long)w_cursor - (ot_long)fp->data;
			}
			else {
				new_length = (bfhdr->flags.mode == PFS_Write) ? 0 : bfhdr->length;
			}
			if (new_length < 0) {
				bfhdr->length = 0;
				retval = -17;				///@todo formalize underwrite error.
			}
			else if (new_length <= fp->alloc) {
				bfhdr->length = (ot_u16)new_length;
			}
			else {
				bfhdr->length = fp->alloc;
				retval = -18;				///@todo formalize overwrite error.
			}
		}

		/// Unless there is an error condition, the action will be called if the file
		/// action usage flags don't mask it.
		if ((retval == 0) && (bfhdr->flags.action & fp->spec.flags) && (bfhdr->stat.action != NULL)) {
			retval = bfhdr->stat.action(fp);
		}
	}
	
	/// 3. Formally close the file by removing the file pointer and file descriptor.
	if (fdtab.next != 0) {
		ot_memset(fp, 0, sizeof(fp));
		fdtab.next--;
	}
	else {
		retval = -255;
	}
	
	return retval;

}


ot_uint pfs_checklength(pfs_file_t* fp) {
	ot_uint length = 0;
	if (FP_ISVALID(fp)) {
		if (fp->spec.type == PFS_Bytefile) {
			pfs_bfhdr_t* bfhdr = fp->hdr;
			if (bfhdr != NULL) {
				length = bfhdr->length;
			}
		}
		else {
			length = fp->alloc;
		}
	}
	
	return length;
}


ot_uint pfs_checkalloc(pfs_file_t* fp) {
	ot_uint alloc;
	if (FP_ISVALID(fp)) {
		alloc = fp->alloc;
	}
	else {
		alloc = 0;
	}
	
	return alloc;
}



ot_int pfs_restore(void) {
#if OT_FEATURE(FSRESTORE)
	///@todo copy restore flash to the working flash, and then do pfs_init().

#else
	return -255;
#endif
}


ot_int pfs_repack(void) {
/// pfs_repack() performs the inverse of pfs_unpack(), which is to copy all NV-backed
/// file and header contents actively held in RAM back to Flash.
	
	///@todo Implement this.  First impl will not do any data transformation.
	
	///1. If any files are open, return error.
	
	///2. Headers in RAM can be ignored because only persistent files have headers 
	///   stored in flash.  These headers are read-only and never change without a
	///   new build.
	
	///3. Write File contents for persistent files to contents flash.  Make sure the 
	///   "front" pointers in the headers match up with the place the file contents are
	///   saved -- this SHOULD be automatic based on the original organization.
	
	return 0;
}


ot_int pfs_unpack(void) {
/// pfs_unpack() performs the inverse of pfs_repack(), which is to take all NV-backed
/// contents from flash and expand them into RAM.
	
	///@todo Implement this.  First impl will not do any data transformation.
	
	///1. If any files are open, return error.
	
	///2. Write File contents for persistent files to contents RAM.  Make sure the 
	///   "front" pointers in the headers match up with the place the file contents are
	///   saved -- this SHOULD be automatic based on the original organization.

	return 0;
}


ot_int pfs_chmod(ot_u16 file_id, ot_u16 modbits, const pfs_user_t* user_id) {
/// pfs_chmod() only works for ByteFile types.

	const pfs_handle_t* handle;
	pfs_bfhdr_t* bfhdr;
	
	/// 1. Get the file handle, which will inform if the File can be chmod-ed.
    handle = sub_get_handle(file_id);
    if (handle == NULL) {
    	return -1;
    }
    if (handle->spec.type != PFS_Bytefile) {
    	return -3;
    }
    bfhdr = sub_convert_front(handle);
	if (bfhdr == NULL) {
		return -255;
	}
    
    /// 2. Authenticate.  Chmod is considered a read-write operation.
    if (user_id != NULL) {
		ot_u8 old_modbits;
		old_modbits = (ot_u8)(bfhdr->flags.modbits);

		///@todo argument alignment on user_id (it's the same)
		if ( auth_check(old_modbits, BF_ACCESS_RW, user_id) == 0 ) {
			return -4;
		}
	}
    
    /// 3. Update the modbits.
    bfhdr->flags.modbits = modbits;
    
    return 0;
}


ot_int pfs_attach_action(ot_u16 file_id, ot_u16 act_flags, ot_procv action) {
/// pfs_attach_action() only works for ByteFile types.  Additionally, attach_action()
/// is not part of an external API.  It only shall be used internally (i.e. not from
/// within a protocol).

	const pfs_handle_t* handle;
	pfs_bfhdr_t* bfhdr;
	
	/// 1. Get the file handle, which will inform if the File can be actioned.
    handle = sub_get_handle(file_id);
    if (handle == NULL) {
    	return -1;
    }
    if (handle->spec.type != PFS_Bytefile) {
    	return -3;
    }
    bfhdr = sub_convert_front(handle);
	if (bfhdr == NULL) {
		return -255;
	}

	/// 3. Attach Action Flags
    bfhdr->flags.action = (act_flags & BF_ACTION_MASK);
    bfhdr->stat.action  = action;
    
    return 0;
}


OT_WEAK ot_int pfs_touch(ot_u16 file_id, const pfs_user_t* user_id) {
///@todo Update access time (not mod time), and run appropriate actions.  
///      This only matters for ByteFile types, because only these have times and
///      actions in their metadata.
	return 0;
}


OT_WEAK ot_u8* pfs_get(ot_u16 file_id) {
///@todo Gets file contents without opening the file.  Useful especially with Record
///      file types, because these are essentially just RAM allocations on the heap.
///      This function shall not be called by external APIs.
	const pfs_handle_t* handle;
	void* file_contents;
	
	/// 1. Get the file handle, which will inform if the File can be actioned.
    handle = sub_get_handle(file_id);
    if (handle == NULL) {
    	return NULL;
    }
    
    /// 2. The pointer to file contents is offset by a certain amount from the front.
    file_contents   = sub_convert_front(handle);
    file_contents  += data_offset[handle->spec.type];
    return (ot_u8*)file_contents;
}




/** Static Subroutines
  * -----------------------------------------------------------------------
  */
  
static const pfs_handle_t* sub_search_hrom(ot_u16 file_id) {
/// This implementation of the ROM Handle Search assumes the handles are aligned and 
/// packed into a block of ROM with known start and end.  The handles are sorted by 
/// File ID, but not necessarily continuously numbered IDs.  A binary search is used.
	int l, r, i;

	// ROM-backed files can have any ID between 0 and MAX ID Value
	if (file_id > PFS_PARAM_MAXIDVAL) {
		return NULL;
	}
	
	l = 0, r = PFS_PARAM_MAXROMFILES - 1;

	while (r >= l) {
		i = (l + r) >> 1;
		
		if (pfs_handle_rom[i].spec.id < file_id) {
			r = i - 1;
		}
		else if (pfs_handle_rom[i].spec.id > file_id) {
			l = i + 1;
		}
		else {
			return &pfs_handle_rom[i];
		}
	}
        
    return NULL;
}

static pfs_handle_t* sub_search_hram(ot_u16 file_id) {
/// This implementation of the RAM Handle Search assumes the FileID's are indexed in
/// order in the Header RAM.  This could be made a lot more generic by having the
/// handle ram be dynamically allocated via PFS memmgr, and reallocated as needed.
	int index;
	
	// Transform File_ID
	index = (int)file_id - PFS_PARAM_USERIDOFFSET;
	if ((index < 0) || (index >= PFS_PARAM_MAXUSERFILES)) {
		return NULL;
	}
	return &uaftab.handle[index];
}

static const pfs_handle_t* sub_get_handle(ot_u16 file_id) {
/// Search for the File ID first among ROM handles (sorted) and then among RAM handles
/// (also sorted).  This subroutine uses its own subroutines for the search.
	const pfs_handle_t* handle;

	/// 1. First look in the header ROM.
	handle = sub_search_hrom(file_id);
	if (handle != NULL) {
		return handle;
	}
	
	/// 2. Second, header RAM
	handle = (const pfs_handle_t*)sub_search_hram(file_id);
	if (handle != NULL) {
		return handle;
	}
	
	/// 3. If nothing found, return NULL
	return NULL;
}



static void* sub_convert_front(const pfs_handle_t* handle) {
/// Purpose of this subroutine is to take a handle, which can come from ROM or RAM,
/// and make sure the pointer returned is the correct pointer to the front of the data,
/// which itself can be in ROM or RAM and not necessarily the same as the handle.
	void* front;
	
	switch (handle->spec.romdata) {
		// Handle in RAM, Contents in RAM
		// "front" is stored directly as a pointer.
		case PFS_ROMDATA_NONE:
			front = (void*)handle->front;
			break;
		
		// Handle in ROM, Contents in RAM
		// "front" contains an offset from the front of the Contents RAM.
		case PFS_ROMDATA_HANDLE:
			front = (void*)((ot_u8*)pfs_contents_ram + offset); 
			break;
		
		// Handle in ROM, Contents in ROM
		// "front" contains an offset from the front of the Contents ROM.
		case PFS_ROMDATA_ALL:
			front = (void*)((const ot_u8*)pfs_contents_rom + offset); 
			break;
		
		// Handle in ROM, Packed Contents in ROM, Active Contents in RAM
		// "front" contains an offset from the front of the Contents RAM.
		case PFS_ROMDATA_MIRRORED:
			front = (void*)((ot_u8*)pfs_contents_ram + offset); 
			break;
	}
	
	return front;
}


static void sub_update_times(pfs_stat_t* status, ot_bool is_modded) {
#if (PFS_FEATURE_MODTIME || PFS_FEATURE_ACCTIME)
	ot_u32 epoch_s = time_get_utc();
	
#	if (PFS_FEATURE_ACCTIME)
		status->acctime = epoch_s;
#	endif

#	if (PFS_FEATURE_MODTIME)
		if (is_modded) {
			status->modtime = epoch_s;
		}
#	endif
#endif
}


static void* sub_alloc(pfs_type_e type, ot_u16 alloc) {
#if (PFS_FEATURE_FSALLOC)
	void* front;
	pfs_handle_t* handle;
	ot_u16 index;
	
	/// 1. Check to make sure there is space in the handle ram.  Cheap check is to see
	///    if the handel ram is not full and the next file cursor is unused.  Expensive 
	///    check is to look for gaps, which is a linear search.
	if (uaftab.count >= PFS_PARAM_MAXUSERFILES) {
		return NULL;
	}
	
	/// 2. Perform allocation of file contents
	front = PFS_MALLOC(alloc + data_offset[type]);
	if (front == NULL) {
		return NULL;
	}
	
	/// 3. Insert handle data
	if (uaftab.next < uaftab.count) {
		index = uaftab.next;
	}
	else {
		for (index=0; index<uaftab.count; index++) {
			handle = &uaftab.handle[index];
			if (handle->front == NULL) {
				break;
			}
		}
	}
	
	handle = &uaftab.handle[index];
	
	uaftab.next++;
	uaftab.count++;
	
	return handle;
	
#else
	return NULL;
	
#endif
}


static void sub_delete(pfs_handle_t* handle) {
#if (PFS_FEATURE_FSALLOC)
	void* front = handle->front;
	
	/// 1. Remove the handle from handle RAM by setting it to zero.
	///    sub_delete() doesn't need to check the validity of the handle pointer.
	bzero(handle, sizeof(pfs_handle_t));
	
	/// 2. Free file contents
	PFS_FREE(front);

#endif
}



#endif

