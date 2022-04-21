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
  * @file       /otsys/pseudofs_vlshim.c
  * @author     JP Norair
  * @version    R100
  * @date       15 Apr 2022
  * @brief      Veelite Backwards Compatibility shim for Pseudo File System
  * @ingroup    PseudoFS
  *
  * Pseudo FS replaces Veelite in OpenTag 2.0.  It is more general purpose 
  * than Veelite, and it expects that all Data is in RAM during runtime. 
  * A Veelite shim exists for backwards compatibility with veelite.
  *
  ******************************************************************************
  */

#include <otstd.h>

#if (OT_FEATURE(VEELITE) == ENABLED)

#include <platform/config.h>

#include <otlib/utils.h>
#include <otlib/auth.h>
#include <otlib/memcpy.h>

#include <otsys/pseudofs.h>
#include <otsys/time.h>


// Veelite init function
#if (CC_SUPPORT == SIM_GCC)
#   include <otplatform.h>
#endif


// Backwards compatibility for unported projects that use Veelite
#if (OT_FEATURE(PSEUDOFS) != ENABLED)
#	define 	_PARAM_MAXFILEPTRS			OT_PARAM(VLFPS)
#	define  _PARAM_MAXFILEACTIONS		OT_PARAM(VLACTIONS)
#	define	_FEATURE_PFSALLOC			OT_FEATURE(VLNEW)

#else
#	define 	_PARAM_MAXFILEPTRS			OT_PARAM(MAXFILEPTRS)
#	define  _PARAM_MAXFILEACTIONS		OT_PARAM(MAXFILEACTIONS)
#	define	_FEATURE_PFSALLOC			OT_FEATURE(PFSALLOC)

#endif



OT_WEAK ot_u8 vl_init(void) {
	ot_int rc;
    rc = pfs_init();
    
    // pfs_init() returns 0 or a negative number
    return (ot_u8)-rc;
}


OT_WEAK ot_int vl_add_action(vlBLOCK block_id, ot_u8 data_id, ot_u8 condition, ot_procv action) {
	ot_u16 file_id = ((ot_u16)block_id << 8) | (ot_u16)data_id;
	ot_u16 act_flags = (ot_u16)condition;
	return pfs_attach_action(file_id, act_flags, action);
}


OT_WEAK void vl_remove_action(vlBLOCK block_id, ot_u8 data_id) {
	ot_u16 file_id = ((ot_u16)block_id << 8) | (ot_u16)data_id;
	return pfs_attach_action(file_id, act_flags, NULL);
}


///@todo This function needs to be shimmed -- right now it is the old impl
OT_WEAK ot_u32 vl_get_fsalloc(const vlFSHEADER* fshdr) {
    return vworm_fsalloc(fshdr);
}


OT_WEAK vlFILE* vl_get_fp(ot_int fd) {
	///@todo make sure client/user doesn't actually use the vlFILE handle
    return pfs_get_fp(fd);
}


OT_WEAK ot_int vl_get_fd(vlFILE* fp) {
    ///@todo make sure client/user doesn't actually use the vlFILE handle
	return pfs_get_fd((pfs_file_t*)fp);
}


OT_WEAK ot_u8 vl_new(vlFILE** fp_new, vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, ot_uint max_length, const id_tmpl* user_id) {
#if (OT_FEATURE(VLNEW) == ENABLED)
#	error "VL SHIM doesn't work with new file feature."
#else
    return 255;
#endif
}


OT_WEAK ot_u8 vl_delete(vlBLOCK block_id, ot_u8 data_id, const id_tmpl* user_id) {
#if (OT_FEATURE(VLNEW) == ENABLED)
#	error "VL SHIM doesn't work with new file feature."
#else
    return 255; //error, delete disabled
#endif
}


OT_WEAK ot_u8 vl_getheader_vaddr(vaddr* header, vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, const id_tmpl* user_id) {
/// The returned vaddr will be an offset from the start of handle-ROM.
/// VL-Shim functionality doesn't support user-allocated files.
///@todo finish this implementation.

    /// 1. Get the header from the supplied Block ID & Data ID
    switch (block_id) {
        case VL_GFB_BLOCKID:    *header = sub_gfb_search(data_id);      break;
        case VL_ISS_BLOCKID:    *header = sub_iss_search(data_id);     break;
        case VL_ISF_BLOCKID:    *header = sub_isf_search(data_id);      break;
        default:                return 255;
    }

    /// 2. Bail if header is NULL
    if (*header == NULL_vaddr) {
        return 0x01;
    }

    /// 3. Authenticate, when it's not a su call
    if (user_id != NULL) {
        ot_uni16 filemod;
        filemod.ushort = vworm_read(*header + 4);
        if ( auth_check(filemod.ubyte[1], mod, user_id) == 0 ) {
            return 0x04;
        }
    }

    return 0;
}


OT_WEAK ot_u8 vl_getheader(vl_header_t* header, vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, const id_tmpl* user_id) {
///@todo This function is only used in dftp.c, which isn't real.  Consider method of
///      implementation to account for its non-use.
	return 255;
}


OT_WEAK vlFILE* vl_open_file(vaddr header) {
/// Used only in alp_filedata.c, together with vl_getheader_vaddr
/// header input describes an offset from the Handle ROM.
	pfs_handle_t* handle;
	pfs_file_t* fp;

	handle = (pfs_handle_t*)((ot_u8*)pfs_handle_rom + (unsigned int)header);
    pfs_open(&fp, handle->spec.id, PFS_Readwrite, NULL);
    return fp;
}


OT_WEAK vlFILE* vl_open(vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, const id_tmpl* user_id) {
	static const ot_u8 modmode[4] = {
		255,
		(ot_u8)PFS_Read,
		(ot_u8)PFS_Write,
		(ot_u8)PFS_ReadWrite
	};

    ot_u16 file_id;
    pfs_mode_e mode;
    pfs_file_t* fp;
    ot_u8 i = 0;
    
    if (mod & VL_ACCESS_R) i += 1;
    if (mod & VL_ACCESS_W) i += 2;
    mod = modmode[i];
    if (mod == 255) {
    	return NULL;
    }
    
    file_id = ((ot_u16)block_id << 8) | (ot_u16)data_id;
    pfs_open(&fp, file_id, (pfs_mode_e)mod, user_id);
    return fp;
}



static pfs_bfhdr_t* sub_get_bfhdr(pfs_file_t* fp) {

}

OT_WEAK ot_u32 vl_getmodtime(vlFILE* fp) {
    ///@todo get bfhdr and pull the modtime
}


OT_WEAK ot_u32 vl_getacctime(vlFILE* fp) {
	///@todo get bfhdr and pull the acctime
}


OT_WEAK ot_u8 vl_setmodtime(vlFILE* fp, ot_u32 newtime) {
	///@todo get bfhdr and set the modtime
}


OT_WEAK ot_u8 vl_setacctime(vlFILE* fp, ot_u32 newtime) {
	///@todo get bfhdr and set the acctime
}


OT_WEAK ot_u8 vl_chmod(vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, const id_tmpl* user_id) {
    ot_u16 file_id = ((ot_u16)block_id << 8) | (ot_u16)data_id;
    ot_int rc;
    
    rc = pfs_chmod(file_id, (ot_u16)mod << 8, (const pfs_user_t*)user_id);
    
    return (rc < 0) ? (ot_u8)-rc : (ot_u8)rc;
}


OT_WEAK ot_u8 vl_touch(vlBLOCK block_id, ot_u8 data_id, ot_u8 flags, const id_tmpl* user_id) {
    ot_u16 file_id = ((ot_u16)block_id << 8) | (ot_u16)data_id;
    ot_int rc;
    
    rc = pfs_touch(file_id, (const pfs_user_t*)user_id);
    
    return (rc < 0) ? (ot_u8)-rc : (ot_u8)rc;
}


OT_WEAK ot_u16 vl_read( vlFILE* fp, ot_uint offset ) {
	ot_u16* hw;
	pfs_file_t* pfp = (pfs_file_t*)fp;
	
	offset &= ~1;
	hw		= (ot_u16*)&(((ot_u8*)(pfp->data))[offset]);
	
    return *hw;
}


OT_WEAK ot_u8 vl_write( vlFILE* fp, ot_uint offset, ot_u16 data ) {
	ot_u16* hw;
	pfs_file_t* pfp = (pfs_file_t*)fp;
	pfs_bfhdr_t* bfhdr;
	
	if (offset >= pfp->alloc) {
        return 255;
    }
    offset &= ~1;
    
    if (pfp->spec.type == PFS_Bytefile) {
    	bfhdr = pfp->hdr;
    	if (offset >= bfhdr->length) {
    		fp->length  = offset + 2;
    		fp->flags  |= BF_FLAG_RESIZED;
    	}
    	fp->flags  |= BF_FLAG_MODDED;
    }
	
	hw = (ot_u16*)&(((ot_u8*)(pfp->data))[offset]);
	*hw = data;
	
    return 0;
}


OT_WEAK ot_u8* vl_memptr( vlFILE* fp ) {
	pfs_file_t* pfp = (pfs_file_t*)fp;
	return (ot_u8*)pfp->front;
}


OT_WEAK ot_uint vl_load( vlFILE* fp, ot_uint length, ot_u8* data ) {
	pfs_file_t* pfp = (pfs_file_t*)fp;
	
	if (length > pfp->alloc) {
		length = pfp->alloc;
	}
	ot_memcpy(data, pfp->front, length);
	
	return length;
}


OT_WEAK ot_u8 vl_store( vlFILE* fp, ot_uint length, const ot_u8* data ) {
    pfs_file_t* pfp = (pfs_file_t*)fp;
	
	if (length > pfp->alloc) {
		length = pfp->alloc;
	}
	ot_memcpy(pfp->front, data, length);
    
    return 1;
}



OT_WEAK ot_u8 vl_append( vlFILE* fp, ot_uint length, const ot_u8* data ) {
    pfs_file_t* pfp = (pfs_file_t*)fp;
    ot_u8* dst;
    ot_u8 rc = 1;
	
	if (pfp->spec.type == PFS_Bytefile) {
		pfs_bfhdr_t* bfhdr = pfp->hdr;
		if ((bfhdr->length + length) > pfp->alloc) {
			length = pfp->alloc - bfhdr->length;
			rc = 0;
		}
		dst = bfhdr->length + (ot_u8*)pfp->data;
	}
	else if (length > pfp->alloc) {
		length = pfp->alloc;
		rc = 0;
	}
	
	ot_memcpy(dst, data, length);
    return rc;
}


OT_WEAK ot_u8 vl_execute(vlFILE* fp, ot_uint input_size, ot_u8* input_stream) {
    ot_u8 retval = 255;

#   if (OT_FEATURE(VLACTIONS) == ENABLED)
    retval = vl_store(fp, input_size, input_stream);
    
    if (retval != 0) {
    	pfs_bfhdr_t* bfhdr = ((pfs_file_t*)fp)->hdr;
    	if (bfhdr->action != NULL) {
        	retval = (ot_u8)bfhdr->action((pfs_file_t*)fp);
        }
    }
#   endif
    
    return retval;
}


OT_WEAK ot_u8 vl_close( vlFILE* fp ) {
    ot_int rc;
    rc = pfs_close((pfs_file_t*)fp);
    
    return (rc < 0) ? (ot_u8)-rc : (ot_u8)rc;
}


OT_WEAK ot_uint vl_checklength( vlFILE* fp ) {
	return pfs_checklength((pfs_file_t*)fp);
}


OT_WEAK ot_uint vl_checkalloc( vlFILE* fp ) {
	return pfs_checkalloc((pfs_file_t*)fp);
}




OT_WEAK vlFILE* GFB_open_su( ot_u8 id ) {
	return vl_open(VL_GFB_BLOCKID, id, VL_ACCESS_SU, NULL);
}

OT_WEAK vlFILE* ISS_open_su( ot_u8 id ) {
    return vl_open(VL_ISS_BLOCKID, id, VL_ACCESS_SU, NULL);
}

OT_WEAK vlFILE* ISF_open_su( ot_u8 id ) {
    return vl_open(VL_ISF_BLOCKID, id, VL_ACCESS_SU, NULL);
}


OT_WEAK vlFILE* GFB_open( ot_u8 id, ot_u8 mod, const id_tmpl* user_id ) {
	return vl_open(VL_GFB_BLOCKID, id, mod, user_id);
}

OT_WEAK vlFILE* ISS_open( ot_u8 id, ot_u8 mod, const id_tmpl* user_id ) {
    return vl_open(VL_ISS_BLOCKID, id, mod, user_id);
}

OT_WEAK vlFILE* ISF_open( ot_u8 id, ot_u8 mod, const id_tmpl* user_id ) {
    return vl_open(VL_ISF_BLOCKID, id, mod, user_id);
}


OT_WEAK ot_u8 GFB_chmod_su( ot_u8 id, ot_u8 mod ) {
	return vl_chmod(VL_GFB_BLOCKID, id, mod, NULL);
}

OT_WEAK ot_u8 ISS_chmod_su( ot_u8 id, ot_u8 mod ) {
    return vl_chmod(VL_ISS_BLOCKID, id, mod, NULL);
}

OT_WEAK ot_u8 ISF_chmod_su( ot_u8 id, ot_u8 mod ) {
    return vl_chmod(VL_ISF_BLOCKID, id, mod, NULL);
}


OT_WEAK ot_u8 ISF_syncmirror() {
	pfs_repack();
}

OT_WEAK ot_u8 ISF_loadmirror() {
	pfs_unpack();
}




#endif

