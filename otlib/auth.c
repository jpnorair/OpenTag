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



/// User aliases for sandboxed processes only
const id_tmpl*   auth_root;
const id_tmpl*   auth_user;
const id_tmpl*   auth_guest;



#if !defined(OT_PARAM_MAX_CRYPTO_KEYS)
#   define _SEC_KEYS    16
#elif (OT_PARAM(MAX_CRYPTO_KEYS) < 2)
#   define _SEC_KEYS    2
#else
#   define _SEC_KEYS    OT_PARAM(MAX_CRYPTO_KEYS)
#endif

#ifndef AES_EXPKEYS
#   define AES_EXPKEYS  0
#endif

#if AES_EXPKEYS
#   define _SEC_TWINKEYS    1
#   define _SEC_CACHESIZE   (_SEC_KEYSIZE*2)
#else
#   define _SEC_CACHESIZE   16
#endif



ot_u8 _nonce[8];


typedef struct {
    auth_info   info;
    ot_u32      cache[_SEC_CACHESIZE];
} auth_dlls_struct;


#if (_SEC_DLL)
/// Presently, EAX is the only type supported
    auth_dlls_struct    auth_key[_SEC_KEYS];
#endif





// Maybe in the future, but probably some type of malloc will get implemented
// instead of this special-purpose heap
#if 0

    /** @typedef crypto_Heap_Type
      * Struct used for the Key Heap Data Type
      *
      * ot_uint end:            byte offset to end of heap where there is free space
      * ot_u8   heapdata[]:     heap data storage.  Stores crypto_Entries mixed with
      *                         key data.
      */
    typedef struct {
        ot_u8   alloc;
        id_tmpl id;
    } heap_item;

    typedef struct {
        ot_int  free_space;
        ot_int  end;
        ot_u8   data[_SEC_HEAPSIZE];
    } auth_heap_struct;

#   define _SEC_HEAPSIZE   ((16+sizeof(heap_item))*2)  //OT_PARAM(AUTH_HEAP_SIZE)
#   define _SEC_TABLESIZE  2   //OT_PARAM(AUTH_TABLE_SIZE)

#   if (_SEC_NLS)
        auth_ctl          auth_table[_SEC_TABLESIZE];
        auth_heap_struct    auth_heap;
#   endif

#endif











// internal stuff

/** @brief Sorts key table based in order of which key is closest to expiration.
  * @param none
  * @retval none
  */
void crypto_sort();

/** @brief Wipes out expired keys
  * @param none
  * @retval none
  */
void crypto_cull();

/** @brief Reorganizes (cleans) the Key Heap & table
  * @param none
  * @retval none
  */
void crypto_clean();





/** Subroutines <BR>
  * ========================================================================<BR>
  */
#if (_SEC_DLL || _SEC_NL)
void sub_expand_key(auth_dlls_struct* key) {
/// Storing expanded keys is not presently supported
#if (0 && (AES_EXPKEYS))
#   if (_SEC_TWINKEYS == 0)
        key->info.options = 0;
        key->info.length  = 44;
        AES_expand_enckey(key->cache, key->cache);
#   else
        ot_u32* enckey;
        key->info.options = 1;
        key->info.length  = 44;
        enckey = key->cache + 44;
        AES_expand_enckey(key->cache, enckey);
        AES_expand_deckey(key->cache, key->cache);
#   endif
#endif
}



///@todo Bring this into OT Utils?
ot_bool sub_idcmp(id_tmpl* user_id, auth_id* auth_id) {
    ot_bool id_check;

    if (user_id->length != auth_id->length) {
        return False;
    }
    if (user_id->length == 2) {
        return (((ot_u16*)user_id->value)[0] == ((ot_u16*)auth_id->value)[0]);
    }

    id_check    = (((ot_u32*)user_id->value)[0] == ((ot_u32*)auth_id->value)[0]);
    id_check   &= (((ot_u32*)user_id->value)[1] == ((ot_u32*)auth_id->value)[1]);
    return id_check;
}


ot_bool sub_authcmp(id_tmpl* user_id, id_tmpl* comp_id, ot_u8 mod_flags) {
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
void auth_init() {
#if (_SEC_DLL)
    ot_uint i;

    /// Start the nonce at a random value
    rand_stream(_nonce, 8);

    /// Load key files into cache for faster access.  We assume that there is
    /// only one type of crypto, which is AES128
    for (i=0; i<2; i++) {
        vlFILE* fp;
        fp = ISF_open_su(i+ISF_ID(root_authentication_key));
        vl_load(fp, 18, &(auth_key[i].info.length));
        vl_close(fp);

        /// @note This will do nothing until it is legal to store expanded keys
        sub_expand_key(&auth_key[i]);
    }
#endif
#if (_SEC_NLS)
    ///@todo
#endif
}
#endif



#ifndef EXTF_auth_init
void auth_putnonce(ot_u8* dst, ot_uint limit) {
    // Rotate the nonce
    *((ot_u32*)&_nonce[0]) += rand_prn16();
    *((ot_u32*)&_nonce[4]) ^= *((ot_u32*)&_nonce[0]);

    // Write it to dst
    memcpy(dst, _nonce, limit);
}
#endif



//#ifndef EXTF_auth_encrypt_q
//ot_int auth_encrypt_q(ot_queue* q, ot_u8 key_index, ot_u8 options) {
//    ot_int  datalen = q_span(q) - 7;
//    ot_u8*  data    = q->getcursor + 7;
//    ot_u8*  nonce   = q->getcursor;
//    auth_encrypt(data, nonce, datalen, key_index, options);
//}
//#endif
//
//
//
//#ifndef EXTF_auth_decrypt_q
//ot_int auth_decrypt_q(ot_queue* q, ot_u8 key_index, ot_u8 options) {
//    ot_int  datalen = q_span(q) - 7;
//    ot_u8*  data    = q->getcursor + 7;
//    ot_u8*  nonce   = q->getcursor;
//    auth_decrypt(data, nonce, datalen, key_index, options);
//}
//#endif
//


// EAX has a symmetric cipher (Yay!)
ot_int __eaxcrypt(ot_u8* nonce, ot_u8* data, ot_uint datalen, ot_u8 key_index, ot_u8 options,
                     ot_int (*__crypt)(ot_u8*, ot_u8*, ot_uint, EAXdrv_t*) )   {
    EAXdrv_t context;
    ot_int  retval;

    retval = EAXdrv_init(auth_get_deckey(key_index), &context);
    if (retval == 0) {
        retval = __crypt(nonce, data, datalen, &context);
        retval = retval ? -2 : 4;
    }
    return retval;
}


#ifndef EXTF_auth_encrypt
ot_int auth_encrypt(ot_u8* nonce, ot_u8* data, ot_uint datalen, ot_u8 key_index, ot_u8 options) {
/// "options" not presently used.
#if (_SEC_DLL)
    return __eaxcrypt(nonce, data, datalen, key_index, options, &EAXdrv_encrypt);
#else
    return -1;
#endif
}
#endif


#ifndef EXTF_auth_decrypt
ot_int auth_decrypt(ot_u8* nonce, ot_u8* data, ot_uint datalen, ot_u8 key_index, ot_u8 options) {
/// "options" not presently used.
#if (_SEC_DLL)
    return __eaxcrypt(nonce, data, datalen, key_index, options, &EAXdrv_decrypt);
#else
    return -1;
#endif
}
#endif


#ifndef EXTF_auth_get_deckey
ot_u8* auth_get_deckey(ot_u8 index) {
#if (_SEC_DLL)
    return (ot_u8*)auth_key[index].cache;
#else
    return NULL;
#endif
}
#endif


#ifndef EXTF_auth_get_enckey
ot_u8* auth_get_enckey(ot_u8 index) {
#if (_SEC_TWINKEYS)
    ot_u32* enckey;
    enckey  = auth_key[index].cache;
    enckey += (auth_key[index].info.options) ? auth_key[index].info.length : 0;
    return (ot_u8*)enckey;
#else
    return auth_get_deckey(index);
#endif
}
#endif













/** User Authentication Routines <BR>
  * ========================================================================<BR>
  * Specifically, the Auth-Sec ALP should have hooks into these functions.
  */
ot_bool auth_isroot(id_tmpl* user_id) {
/// NULL is how root is implemented in internal calls
#if (_SEC_NLS)
    return sub_authcmp(user_id, auth_root, AUTH_FLAG_ISROOT);
#elif (_SEC_DLL)
    return (ot_bool)((user_id == NULL) || (user_id == auth_root));
#else
    return (ot_bool)(user_id == NULL);
#endif
}


ot_bool auth_isuser(id_tmpl* user_id) {
/// NULL is how root is implemented in internal calls
#if (_SEC_NLS)
    return sub_authcmp(user_id, auth_user, AUTH_FLAG_ISUSER);
#elif (_SEC_DLL)
    return (ot_bool)((user_id == NULL) || (user_id == auth_user));
#else
    return (ot_bool)(user_id == NULL);
#endif
}



ot_u8 auth_check(ot_u8 data_mod, ot_u8 req_mod, id_tmpl* user_id) {
#if (_SEC_ANY)
/// Find the ID in the table, then mask the user's mod with the file's mod
/// and the mod from the request (i.e. read, write).

    ///@todo need to write auth_search_user()
    //return (ot_u8)auth_search_user(user_id, (data_mod & req_mod));
    return (0x07 & data_mod & req_mod);

#else
/// If the code gets here then there was not a user match, or the device is not
/// implementing user authentication.  Try guest access.
    return (0x07 & data_mod & req_mod);
#endif
}










/** Functions Typically Used with ALP <BR>
  * ========================================================================<BR>
  * Specifically, the Auth-Sec ALP should have hooks into these functions.
  * @todo implement!
  */

ot_u8 auth_find_keyindex(auth_handle* handle, id_tmpl* user_id) {
    return 255;
}

ot_u8 auth_read_key(auth_handle* handle, ot_u16 key_index) {
    return 255;
}

ot_u8 auth_update_key(auth_handle* handle, ot_u16 key_index) {
    return 255;
}

ot_u8 auth_create_key(ot_u16* key_index, auth_handle* handle) {
    return 255;
}

ot_u8 auth_delete_key(ot_u16 key_index) {
    return 255;
}





/** @todo these are old and should be worked-into above

ot_int auth_load_keydata(ot_u8* dst, ot_int limit, ot_u8 index) {
    ot_int data_size;
    auth_key[index].info.index  = index;
    data_size                   = 4 + (auth_key[index].info.length << auth_key[index].info.options);

    if (limit < data_size) {
        return 0;
    }
    memcpy(dst, (ot_u8*)auth_key[index], data_size);
    return data_size;
}

ot_u8 auth_store_keydata(auth_info* info, ot_u8* keydata) {
    auth_dlls_struct* local_key;

    if (info->protocol != 0)                                    return 1;
    if ((info->index < 2) || (info->index > _SEC_CACHESIZE))    return 2;
    if (info->length != 16)                                     return 3;

    local_key = &auth_key[info->index];

    memcpy(    (ot_u8*)&local_key->info, \
                        (ot_u8*)info, \
                        sizeof(auth_info)    );

    memcpy(    (ot_u8*)local_key->cache, \
                        (ot_u8*)keydata, \
                        16    );

    sub_expand_key(local_key);

    return 0;
}
*/









/** @todo NLS is not supported any time soon, but refactor according to new prototypes

ot_u8 auth_new_nlsuser(auth_handle* handle, id_tmpl* new_user, auth_info* new_info, ot_u8* new_key);
auth_ctl* auth_new_nlsuser(auth_ctl* new_user, ot_u8* new_data) {
#if (_SEC_NLS)
#endif
    return NULL;
}



ot_u8 auth_search_user(auth_handle* handle, id_tmpl* user_id, ot_u8 mod_flags
auth_ctl* auth_search_user(id_tmpl* user_id, ot_u8 mod_flags) {
#if (_SEC_ANY)
    ot_int i;

    for (i=0; (i<_SEC_TABLESIZE) && (auth_table[i]!=NULL); i++) {
        if (auth_table[i]->mod & mod_flags) {
            if (sub_idcmp(user_id, auth_table[i])) {
                return auth_table[i];
            }
        }
    }
#endif
    return NULL;
}
*/







/** @todo this function is very legacy and I don't know where it belongs in the
   refactored auth.c

ot_u8* auth_get_dllskey(ot_u8 protocol, ot_u8 options, ot_u8* header) {
#if (_SEC_DLL)
    /// This implementation assumes that AES128 is the only supported crypto.
    /// DASH7 AES128 has protocol-id = x0000000.
    if (protocol & 0x3F) {
        return NULL;
    }
    options    &= 2;
    options    += ((protocol & AUTH_FLAG_ISROOT) == 0);
    return (ot_u8*)auth_key.[options].cache;

#elif 0 //(_SEC_DLL)
    /// This implementation looks up a protocol-specific key in the appropriate
    /// ISF Key file.  It is over-engineered for implementations with only one
    /// type of DLLS crypto (e.g. AES128).
    ot_u8 offset;
    ot_u8 cursor;
    vlFILE* fp;

    offset      = ((protocol & AUTH_FLAG_ISROOT) == 0);
    protocol   &= 0x7F;
    fp          = isf_open_su(ISF_ID(root_authentication_key) + offset);
    cursor      = 0;

    /// Go through the Key list, see if there is one that matches the protocol
    while (cursor < fp->length) {
        ot_uni16 scratch;
        scratch.ushort  = vl_read(fp, cursor);
        cursor         += 2;

        if (scratch.ubyte[1] == protocol) {
            for (offset=0; offset<scratch.ubyte[0]; offset+=2, cursor+=2) {
                *((ot_u16*)(auth_keybuf+offset)) = vl_read(fp, cursor);
            }
            vl_close(fp);
            return (ot_u8*)auth_keybuf;
        }
        cursor += scratch.ubyte[0];
    }

    vl_close(fp);
    return NULL;

#else
    return NULL;
#endif
}
*/


