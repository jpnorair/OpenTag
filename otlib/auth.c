/* Copyright 2013 JP Norair
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
  * @file       /otlib/auth.c
  * @author     JP Norair
  * @version    R102
  * @date       21 Aug 2014
  * @brief      Authentication & Crypto Functionality
  * @ingroup    Authentication
  *
  *
  * @todo Entire context is stored in the key table.  Change this such that 
  *       only the key scheduler is stored.
  ******************************************************************************
  */

#include <otstd.h>

#include <otlib/auth.h>
#include <otlib/crypto.h>
#include <otlib/memcpy.h>
#include <otlib/rand.h>
#include <otsys/veelite.h>

#define _SEC_NL     OT_FEATURE(NLSECURITY)
#define _SEC_DLL    OT_FEATURE(DLL_SECURITY)
#define _SEC_ALL    (_SEC_NL && _SEC_DLL)
#define _SEC_ANY    (_SEC_NL || _SEC_DLL)

///@todo bring this into OT_config.h eventually, when the feature gets supported

/// Default minimum key lifetime is one hour (3600 seconds).  
/// This can be changed, but it's nonetheless far too short to exploit by force.
#if defined(OT_PARAM_KEYLIFE_MIN)
#   define AUTH_MIN_LIFETIME    OT_PARAM_KEYLIFE_MIN
#else
#   define AUTH_MIN_LIFETIME    3600
#endif 


/// User aliases for sandboxed processes only
const id_tmpl   auth_root;
const id_tmpl   auth_user;
const id_tmpl   auth_guest;


typedef enum {
    ID_localguest   = -1,       //must be -1
    ID_localroot    = 0,
    ID_localuser    = 1,
    ID_normal       = 2
} idclass_t;

typedef struct OT_PACKED {
    uint64_t    id;
    ot_u16      mflags;
    ot_u32      EOL;
} authinfo_t;

///@note keyfile_t must match the structure of the "root & user authentication
///      key" files stored in the filesystem.
///@note Hateful TI C Compiler does not support Packed structures despite the
///      manual mentioning it does.
#ifdef __TI_C__
typedef struct OT_PACKED {
    ot_u16  res0;
    ot_u16  ctl;
    ot_u32  EOL;
    ot_u32  key[4];
} keyfile_t;
#else
typedef struct OT_PACKED {
    ot_u16  ctl;
    ot_u32  EOL;
    ot_u32  key[4];
} keyfile_t;
#endif



#if (_SEC_ANY)
#   include <oteax.h>
    typedef eax_ctx authctx_t;

#   undef   AUTH_NUM_ELEMENTS
#   define  AUTH_NUM_ELEMENTS   3

    static uint32_t dlls_nonce;
    static ot_uint  dlls_size   = 0;

#   if (AUTH_NUM_ELEMENTS >= 0)
    // Static allocation:
    // First two elements are root and admin for the active device.
    static authctx_t    dlls_ctx[AUTH_NUM_ELEMENTS];
    static authinfo_t   dlls_info[AUTH_NUM_ELEMENTS];
    
#   elif (AUTH_NUM_ELEMENTS < 0)
    // Dynamic Allocation:
    // Is allocated at time of initialization.
    // Must be at least 2 elements.
    // Items will only be cleared during deinit/delete.
    static authctx_t* dlls_ctx = NULL;
    static authinfo_t* dlls_info = NULL;
    
#   endif

#else
#   undef   AUTH_NUM_ELEMENTS
#   define  AUTH_NUM_ELEMENTS   0

#endif








/** Subroutines <BR>
  * ========================================================================<BR>
  */
#if (_SEC_ANY)
static idclass_t sub_make_id64(uint64_t* id64, const id_tmpl* user_id) {    
    ot_int i;
    
    if ((user_id == NULL) || (user_id == &auth_root)) {
        return ID_localroot;
    }
    if (user_id == &auth_user) {
        return ID_localuser;
    }
    if (user_id == &auth_guest) {
        return ID_localguest;
    }
    
    /// - user_id contains a length and a value (uint8_t array)
    /// - we need to copy it into the uint64_t, and make sure endian is converted from network endian
    /// - C2000 implementation is limited to user_id->length == 2
#   ifdef __C2000__
    *id64 = (uint64_t)PLATFORM_ENDIAN16(user_id->value[0]);

#   else
    *id64 = 0;
    i = 0;
    switch (user_id->length) {
        case 8: *id64 |= (uint64_t)user_id->value[i++] << 56;
        case 7: *id64 |= (uint64_t)user_id->value[i++] << 48;
        case 6: *id64 |= (uint64_t)user_id->value[i++] << 40;
        case 5: *id64 |= (uint64_t)user_id->value[i++] << 32;
        case 4: *id64 |= (uint64_t)user_id->value[i++] << 24;
        case 3: *id64 |= (uint64_t)user_id->value[i++] << 16;
        case 2: *id64 |= (uint64_t)user_id->value[i++] << 8;
        case 1: *id64 |= user_id->value[i];
       default: break;
    }
#   endif

    return ID_normal;
}


static void sub_expand_key(void* rawkey, authctx_t* ctx) {
/// This routine will expand the key (128 bits) into a much larger key sequence.
/// The key sequence is what is actually used to do cryptographic operations.    
    EAXdrv_init(rawkey, (void*)ctx);
}



///@todo Bring this into OT Utils?
static ot_bool sub_idcmp(const id_tmpl* user_id, uint64_t id) {
    ot_int length;
    length = (id < 65536) ? 2 : 8;
    return (ot_bool)((length == user_id->length) && (*(uint64_t*)user_id->value == id));
}


static ot_bool sub_authcmp(const id_tmpl* user_id, const id_tmpl* comp_id, ot_u8 mod_flags) {
    if ((user_id == NULL) || (user_id == comp_id))
        return True;

    return (ot_bool)auth_search_user(user_id, mod_flags);
}
#endif








/** High Level Cryptographic Interface Functions <BR>
  * ========================================================================<BR>
  * init, encrypt, decrypt
  */

#ifndef EXTF_auth_init
void auth_init(void) {
#if (_SEC_ANY)
#   define _KFILE_BYTES     (2 + 4 + 16)
    ot_int      i;
    vlFILE*     fp;
    keyfile_t   kfile;

    ///@todo put in code to:
    /// 1. initialize dynamic memory if it's enabled, and if pointer is NULL
    /// 2. 
#   if (AUTH_NUM_ELEMENTS < 0)
    /// Initialize dynamic memory if it's enabled.
    /// - if one of the tables is NULL, make sure to wipe both as clean start point.
    /// - Initialize the table in 64 unit chunks.
    if ((dlls_info == NULL) || (dlls_ctx == NULL)) {
        auth_deinit();
        
        ///@todo do the actual initialization.  Will be done through libjudy.
    }
    
#   endif

    /// If this function gets called when dlls_size < 2, it's a call with empty
    /// tables.
    if (dlls_size < 2) {
        dlls_size   = 2;
        dlls_nonce  = rand_prn32();
    }

    /// The first two keys are local keys that will change whenever the device
    /// reference changes.  Load them into the buffers.
    for (i=0; i<2; i++) {
        fp = ISF_open_su(i+ISF_ID(root_authentication_key));
        if (fp != NULL) {
            vl_load(fp, _KFILE_BYTES, (void*)&(kfile.ctl));
            dlls_info[i].id     = i;
            dlls_info[i].mflags = (i==0) ? AUTHMOD_root : AUTHMOD_user;
            dlls_info[i].EOL    = kfile.EOL;
            sub_expand_key((void*)kfile.key, &dlls_ctx[i]);
            vl_close(fp);
        }
    }
    
    /// Keys after the first two persist through calls of auth_init().

#   undef _KFILE_BYTES
#endif

#if (_SEC_NLS)
    ///@todo
#endif
}
#endif



#ifndef EXTF_auth_deinit
void auth_deinit(void) {
/// clear all memory used for key storage, and free it if necessary.
#   if (AUTH_NUM_ELEMENTS > 0)
    // Clear memory elements.  They are statically allocated in this case,
    // so no freeing is required.
    memset(dlls_info, 0, sizeof(dlls_info));
    memset(dlls_ctx, 0, sizeof(dlls_ctx));
    
#   elif (AUTH_NUM_ELEMENTS < 0)
    // Clear memory elements and free them.
    if (dlls_info != NULL) {
        memset(dlls_info, 0, sizeof(authinfo_t) * dlls_size);
        free(dlls_info);
    }
    if (dlls_ctx != NULL) {
        memset(dlls_ctx, 0, sizeof(authctx_t) * dlls_size);
        free(dlls_ctx);
    }
    dlls_size = 0;
    
#   endif
}
#endif



#ifndef EXTF_auth_putnonce
void auth_putnonce(void* dst, ot_uint total_size) {
#if (_SEC_ANY)
    ot_int      pad_bytes;
    ot_int      write_bytes;
    uint32_t    output_nonce;
    ot_u8*      dst_u8 = (ot_u8*)dst;
    
    /// If total_size is > 4 (size of nonce in bytes), we advance dst accordingly (Thus it is
    /// padded with its existing contents).
    /// If total_size is <= 4, then the write_bytes get shortened.
    write_bytes = 4;
    pad_bytes   = total_size - 4;
    if (pad_bytes > 0) {
        dst_u8 += pad_bytes;
    }
    else {
        write_bytes += pad_bytes;
    }
    
    /// Increment the internal nonce integer each time a nonce is put.
    /// It's also possible to change to network endian here, but it
    /// doesn't technically matter as long as the nonce data is 
    /// conveyed congruently.
    output_nonce = dlls_nonce++;
    
    ot_memcpy(dst_u8, &output_nonce, write_bytes);
#endif
}
#endif



#ifndef EXTF_auth_putnonce_q
void auth_putnonce_q(ot_queue* q, ot_uint total_size) {
#if (_SEC_ANY)
    ot_int      pad_bytes;
    ot_int      write_bytes;
    uint32_t    output_nonce;

    write_bytes = 4;
    pad_bytes   = total_size - 4;
    if (pad_bytes > 0) {
        q->putcursor += pad_bytes;
    }
    else {
        write_bytes += pad_bytes;
    }
    
    output_nonce = dlls_nonce++;
    q_writelong_be(q, output_nonce);
#endif
}
#endif



#ifndef EXTF_auth_getnonce
ot_u32 auth_getnonce(void) {
#if (_SEC_ANY)
    /// Increment the internal nonce integer each time a nonce is got.
    dlls_nonce++;
    return dlls_nonce;
#else
    return 0;
#endif
}
#endif



static ot_int sub_do_crypto(void* nonce, void* data, ot_uint datalen, ot_uint key_index,
                        ot_int (*EAXdrv_fn)(void*, void*, ot_uint, void*) ) {
#if (_SEC_ANY)
    /// Nonce input is 7 bytes.
    /// on Devices without byte access (C2000), nonce will be 8 bytes with last byte 0.

    /// DLL Encryption stage.
    /// Use AES context from auth_init() to do the encryption.
    ot_int retval;
    
    /// Error if key index is not available
    if (key_index >= dlls_size) {
        return -1;
    }
    
    retval  = EAXdrv_fn(nonce, data, datalen, (void*)&dlls_ctx[key_index]);

    return (retval != 0) ? -2 : 4;
    
#else
    return -1;
    
#endif
}



#ifndef EXTF_auth_encrypt
ot_int auth_encrypt(void* nonce, void* data, ot_uint datalen, ot_uint key_index) {
#if (_SEC_ANY)
    return sub_do_crypto(nonce, data, datalen, key_index, &EAXdrv_encrypt);
#else
    return -1;
#endif
}
#endif



#ifndef EXTF_auth_decrypt
ot_int auth_decrypt(void* nonce, void* data, ot_uint datalen, ot_uint key_index) {
/// EAX cryptography is symmetric, so decrypt and encrypt are almost identical.
#if (_SEC_ANY)
    return sub_do_crypto(nonce, data, datalen-4, key_index, &EAXdrv_decrypt);
#else
    return -1;
#endif
}
#endif



static ot_int sub_crypt_q(ot_queue* q, ot_uint key_index, bool enc) {
#ifdef __C2000__
    ot_u32  nonce[2] = {0, 0};
    ot_int (*EAXdrv_fn)(void*, void*, ot_uint, void*);
    ot_u32* data;
    ot_int length;
    int rc;
    int i;
    
    // Extract the nonce, and zero-pad it.
    nonce[0] = ((ot_u32*)q->front)[0];
    nonce[1] = ((ot_u32*)q->front)[1];
    __byte((int*)nonce, 7) = 0;
    q->getcursor = 7;
    
    // Shift the queue 3 bytes forward.  This makes it 32bit aligned.  It has
    // the secondary benefit of guaranteeing that the last word is zero padded,
    // without violating any data boundaries.
    length  = q_span(q);
    data    = &((ot_u32*)q->front)[1];
    for (i=0; i<length; i++) {
        __byte((int*)data, i) = __byte((int*)data, i+3);
    }
    __byte((int*)data, i)   = 0;
    __byte((int*)data, i+1) = 0;
    __byte((int*)data, i+2) = 0;
    
    // Run cryptography, then put everything back into original alignment.
    if (enc) {
        EAXdrv_fn = &EAXdrv_encrypt;
    }
    else {
        EAXdrv_fn = &EAXdrv_decrypt;
        length -= 4;
    }
    rc = sub_do_crypto(nonce, data, length, key_index, EAXdrv_fn);
    
    for(i=length+3; i>=0; i--) {
        __byte((int*)data, i+3) = __byte((int*)data, i);
    }
    __byte((int*)data, 0) = __byte((int*)nonce, 4);
    __byte((int*)data, 1) = __byte((int*)nonce, 5);
    __byte((int*)data, 2) = __byte((int*)nonce, 6);
    
    return rc;
    
#else
    ot_u8* nonce;
    ot_u8* data;
    ot_uint length;
    ot_int (*EAXdrv_fn)(void*, void*, ot_uint, void*);

    nonce   = q_markbyte(q, 7);
    length  = q_span(q);
    data    = q->getcursor; //q_markbyte(q, length);

    if (enc) {
        EAXdrv_fn = &EAXdrv_encrypt;
    }
    else {
        EAXdrv_fn = &EAXdrv_decrypt;
        length -= 4;
    }
    
    return sub_do_crypto(nonce, data, length, key_index, EAXdrv_fn);
#endif
}


#ifndef EXTF_auth_encrypt_q
ot_int auth_encrypt_q(ot_queue* q, ot_uint key_index) {
#if (_SEC_ANY)
    ot_int tag_size;
    ot_qcur saved_get;

    saved_get       = q->getcursor;
    tag_size        = sub_crypt_q(q, key_index, true);
    q->getcursor    = saved_get;
    if (tag_size > 0) {
        q->putcursor += tag_size;
    }
    return tag_size;
#else
    return -1;
#endif
}
#endif



#ifndef EXTF_auth_decrypt_q
ot_int auth_decrypt_q(ot_queue* q, ot_uint key_index) {
#if (_SEC_ANY)
    ot_int tag_size;
    tag_size = sub_crypt_q(q, key_index, false);
    if (tag_size > 0) {
        q->putcursor   -= tag_size;
        q->back        -= tag_size;
    }
    return tag_size;
#else
    return -1;
#endif
}
#endif


/*
#ifndef EXTF_auth_get_enckey
ot_int auth_get_enckey(void** key, ot_uint index) {
///@todo not sure if this function should be removed
#if (_SEC_ANY)
    if ((key != NULL) && (index < dlls_size)) {
        *((ot_u32**)key) = &dlls_ctx[index];
        return sizeof(dlls_ctx[index]);
    }
#endif

    return -1;
}
#endif




#ifndef EXTF_auth_get_deckey
ot_int auth_get_deckey(void** key, ot_uint index) {
/// EAX cryptography is symmetric, so decrypt and encrypt are the same.
    return auth_get_enckey(key, index);
}
#endif
*/















/** User Authentication Routines <BR>
  * ========================================================================<BR>
  * Intended to be used internally or via a ROOT-authenticated connection to
  * an Auth-Sec ALP.
  */
  
static ot_int sub_search_user(uint64_t id64, authmod_t reqmod) {
#if (AUTH_NUM_ELEMENTS > 0)
///@todo Current implementation is linear search.  In the future maybe
///      implement binary search, although for small tables typical for
///      this static allocation, it might be faster with linear search.
    ot_int i; 

    // Linear Search
    // - Compare id: ID's must be unique.
    // - Inspect mod: ID's mod must be greater than the requested mod
    // - If key timeout is enabled (EOL != 0), then make sure key isn't expired
    // - If key is expired, wipe the context for security. (Do not actually delete key though)
    for (i=2; i<dlls_size; i++) {
        if ((id64 == dlls_info[i].id) && ((dlls_info[i].mflags & 0x3F) >= reqmod)) {
            if ((dlls_info[i].EOL != 0) && (dlls_info[i].EOL <= time_get_utc())) {
                memset((void*)&dlls_ctx[i], 0, sizeof(authctx_t));
                dlls_info[i].mflags = AUTHMOD_guest;
                continue;
            }
            // Key is found, and valid
            return i;
        }
    }
    
#elif (AUTH_NUM_ELEMENTS < 0)
    ///@todo Dynamic Allocation: uses libjudy
    return -1;
    
#endif
    // returns guest by default
    return -1;
}


ot_int auth_search_user(const id_tmpl* user_id, ot_u8 req_mod) {
/// Compare user-id and mod against stored keys.
/// The req_mod input is a bitfield with the structure: --rwxrwx, which is
/// defined by the veelite FS as follows:
/// --rwx--- = file perms for User
/// -----rwx = file perms for Guest
/// --000000 = file only accessible by root
/// 
/// The Authentication system doesn't work with Guests (just User and Root).
/// We need to convert req_mod into a form that works for Auth, which is 
/// below:
/// --rwx--- = key is suitable for Root read/write/exec access.
/// -----rwx = key is suitable for User read/write/exec access.
///
#if (_SEC_ANY)
    uint64_t id_u64;
    idclass_t idtype;

    ///1. Check req_mod first, it could be asking for guest access.
    ///   req_mod is converted from Veelite format (OpenTag API is this) into
    ///   internal auth format.
    req_mod = (req_mod == 0) ? AUTHMOD_root : (req_mod >> 3) & 7;
    if (req_mod == AUTHMOD_guest) {
        return (ot_int)ID_localguest;
    }
    
    ///2. Convert the ID from id_tmpl to 64 bit unsigned, which is used
    ///   internally for auth management.
    ///   If the user id is for a local user, we make sure it matches the
    ///   supplied mod value, and return the local user.
    ///   - root has all access.
    ///   - user has access to user
    ///   - guest has no authenticated access
    idtype = sub_make_id64(&id_u64, user_id);
    if (idtype != ID_normal) {
        if (  ((req_mod == AUTHMOD_root) && (idtype != ID_localroot)) 
           || ((req_mod == AUTHMOD_user) && (idtype < ID_localuser)) ) {
            return (ot_int)ID_localguest;
        }
        return (ot_int)idtype;
    }

    ///3. Preliminary input checks are done.
    ///   At this point, we need to search for a non-local user in the 
    ///   authentication table.  The implementation is slightly different
    ///   between static and dynamic configurations, via subroutine.
    return sub_search_user(id_u64, (authmod_t)req_mod);

#else
    return -1;
    
#endif
}



ot_u8 auth_get_user(id_tmpl* user_id, ot_uint key_index) {
#if (_SEC_ANY)
    if (user_id == NULL) {
        return 1;
    }
    if (key_index <= dlls_size) {
        return 2;
    }

    user_id->length = (dlls_info[key_index].id < 65536) ? 2 : 8;
    ot_memcpy(user_id->value, &dlls_info[key_index].id, user_id->length);
    
    return 0;

#else
    return -1;
    
#endif
}


const id_tmpl* auth_intrinsic_user(ot_uint index) {
	id_tmpl* output;
    switch (index) {
        case 0:     output = &auth_root;	break;
        case 1:     output = &auth_user;	break;
        default:    output = &auth_guest;	break;
    }
    return output;
}



static ot_bool sub_ismask(const id_tmpl* user_id, authmod_t authmask) {
#if (_SEC_ANY)
    if (user_id == NULL) {
        return true;
    }
    return (ot_bool)(auth_search_user(user_id, (ot_u8)authmask) >= 0);
    
#else
    return (ot_bool)(user_id == NULL);
    
#endif
}

ot_bool auth_isroot(const id_tmpl* user_id) {
    return sub_ismask(user_id, b00000000);
}

ot_bool auth_isuser(const id_tmpl* user_id) {
return sub_ismask(user_id, b00111000);
}



ot_u8 auth_check(ot_u8 req_mod, ot_u8 rw_mod, const id_tmpl* user_id) {
/// Find the ID in the table, then mask the user's mod with the file's mod
/// and the mod from the request (i.e. read, write).
#if (_SEC_ANY)
    ot_int user_type;
    ot_u8 test          = (req_mod & rw_mod);
    ot_u8 guest_test    = test & 0x07;

    if (guest_test) {
        return guest_test;
    }

    user_type = auth_search_user(user_id, req_mod);
    if (user_type >= 0) {
#   if (AUTH_NUM_ELEMENTS >= 0)
        if (user_type < dlls_size) {
            if (dlls_info[user_type].mflags == AUTHMOD_root) {
                return rw_mod;
            }
        }
        return test;

#   elif (AUTH_NUM_ELEMENTS < 0)
    ///@todo implement this
#   endif
    }

    return 0;

#else
    // Try guest access
    return (0x07 & req_mod & rw_mod);
#endif
}










/** Key Management Functions <BR>
  * ========================================================================<BR>
  * Intended to be used internally or via a ROOT-authenticated connection to
  * an Auth-Sec ALP.
  */
  
static ot_u8 sub_add_key(ot_uint* key_index, keytype_t type, ot_u32 lifetime, void* keydata, uint64_t id64) {
#if (AUTH_NUM_ELEMENTS >= 0)
    /// Static allocation
    /// - Make sure there is space to add.
    /// - Add it to end of table, and return key_index.
    if (dlls_size < AUTH_NUM_ELEMENTS) {
        *key_index = dlls_size++;
        
        dlls_info[*key_index].id     = id64;
        dlls_info[*key_index].mflags = AUTHMOD_user; ///@todo set this to appropriate bits.
        dlls_info[*key_index].EOL    = time_get_utc() + lifetime;
        sub_expand_key(keydata, &dlls_ctx[*key_index]);
        
        return 0;
    }

#elif (AUTH_NUM_ELEMENTS < 0)
    ///@todo implement this
    
#endif

    return 255;
}


ot_u8 auth_find_keyindex(ot_uint* key_index, const id_tmpl* user_id) {
    ot_int index;
    
    if (key_index == NULL) {
        return 1;
    }
    
    index = auth_search_user(user_id, b00111000);
    if (index < 0) {
        return 255;
    }
    
    *key_index = index;
    return 0;
}



ot_u8 auth_refresh_key(ot_uint* key_index, ot_u32 new_lifetime, const id_tmpl* user_id) {
    ot_u8 status;
    
    if (new_lifetime < AUTH_MIN_LIFETIME) {
        return 3;
    }
    
    status = auth_find_keyindex(key_index, user_id);
    if (status == 0) {
        dlls_info[*key_index].EOL = time_get_utc() + new_lifetime;
    }
    
    return status;
}



ot_u8 auth_create_key(ot_uint* key_index, keytype_t type, ot_u32 lifetime, void* keydata, const id_tmpl* user_id) {
    uint64_t id64;
    idclass_t idtype;
    ot_int index;
    
    ///1. Input Checking
    if (key_index == NULL) {
        return 1;
    }
    if (type != KEYTYPE_AES128) {
        // Only type supported in this impl is AES128
        return 2;
    }
    if (lifetime < AUTH_MIN_LIFETIME) {
        return 3;
    }
    if (keydata == NULL) {
        return 4;
    }
    
    ///2. Convert user_id into id64 form.  
    ///   sub_make_id64 handles user_id == NULL and other special cases.
    ///   Escape for local ID types, which are handled specially
    idtype = sub_make_id64(&id64, user_id);
    if (idtype != ID_normal) {
        return 5;
    }

    ///3. Check if ID already exists.
    ///   - only one key per ID
    ///   - if ID already exists, return error
    index = sub_search_user(id64, AUTHMOD_user);
    if (index >= 0) {
        return 254;
    }

    return sub_add_key(key_index, type, lifetime, keydata, id64);
}



ot_u8 auth_delete_key(ot_uint key_index) {
/// This will delete a key from the table of keys.  It is up to the application
/// to maintain keys, and thus to use this function.
/// 
/// Static Allocation: Typically used with small key tables (e.g. for endpoints)
/// - wipe old key context and
/// - if key index is 0 or 1, these are special keys, don't displace them.
/// - else, shift table contents over deleted key
///
/// Dynamic Allocation: Typically used with large key tables (e.g. gateways)
/// - Wipe, delete, and free the key context & info
/// - reduce size and pointers of table accordingly

    if ((key_index < 2) || (key_index >= dlls_size)) {
        return 1;
    }
    
#   if (AUTH_NUM_ELEMENTS >= 0)
    {   ot_int i;
    
        memset((void*)&dlls_ctx[key_index], 0, sizeof(authctx_t));
        
        ///@todo validate this line
        dlls_info[key_index].mflags = (1<<7);    //AUTH_KEYFLAGS_INVALID;
        dlls_size--;
        
        // Shift keys back down
        for (i=key_index; i<dlls_size; i++) {
            memcpy(&dlls_info[i], &dlls_info[i+1], sizeof(authinfo_t));
            memcpy(&dlls_ctx[i], &dlls_ctx[i+1], sizeof(authctx_t));
        }
        
        return 0;
    }
    
#   elif (AUTH_NUM_ELEMENTS < 0)
    ///@todo implement this
    return 255;
    
#   else    
    return 255;
#   endif
}



keytype_t auth_get_key(void** keydata, ot_uint key_index) { 
    if (keydata == NULL) {
        return KEYTYPE_none;
    }
    
#   if (AUTH_NUM_ELEMENTS >= 0)
    if (key_index < dlls_size) {
        if (dlls_info[key_index].mflags < (1<<7)) {
            *keydata = (void*)&dlls_ctx[key_index];
            return KEYTYPE_AES128;
        }
    }

    *keydata = NULL;
    return KEYTYPE_none;
    
#   elif (AUTH_NUM_ELEMENTS < 0)
    ///@todo implement this
    return KEYTYPE_none;
    
#   else    
    return KEYTYPE_none;
#   endif
}

