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
  * @version    R100
  * @date       12 Aug 2013
  * @brief      Authentication & Crypto Functionality
  * @ingroup    Authentication
  *
  ******************************************************************************
  */


///@todo VERY EXPERIMENTAL


#include "OT_config.h"

#include "auth.h"
#include "veelite.h"

#include "crypto_aes128.h"


#define _SEC_NL     0 //OT_FEATURE(NLSECURITY)
#define _SEC_DLL    0 //OT_FEATURE(DLLSECURITY)
#define _SEC_ALL    0 //(_SEC_NL && _SEC_DLL)
#define _SEC_ANY    0 //(_SEC_NL || _SEC_DLL)

#define AUTH_HEAP_SIZE 0


const id_tmpl*   auth_root;
const id_tmpl*   auth_user;
const id_tmpl*   auth_guest;





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


#define _SEC_HEAPSIZE   ((16+sizeof(heap_item))*2)  //OT_PARAM(AUTH_HEAP_SIZE)
#define _SEC_TABLESIZE  2   //OT_PARAM(AUTH_TABLE_SIZE)

typedef struct {
    ot_int  free_space;
    ot_int  end;
    ot_u8   data[_SEC_HEAPSIZE];
} auth_heap_struct;

typedef struct {
    ot_u8   reserved00;
    ot_u8   reserved01;
    ot_u8   length;
    ot_u8   protocol;
    ot_u32  cache[4];
} auth_dlls_struct;



#if (_SEC_NLS)
    auth_entry          auth_table[_SEC_TABLESIZE];
    auth_heap_struct    auth_heap;
#endif

#if (_SEC_DLL)
/// Presently, AES128 is the only type supported
    auth_dlls_struct    auth_key[4];
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





void auth_init() {
#if (_SEC_DLL)
    /// Load key files into cache for faster access.  We assume that there is
    /// only one type of crypto, which is AES128
    for (i=ISF_ID(root_authentication_key); i<ISF_ID(root_authentication_key)+2; i++) {
        fp  = isf_open_su(i);
        vl_load(fp, &(auth_key[i].length), 18);
        ///@todo Do decryption key processing
        vl_close(fp);
    }
    
#endif
#if (_SEC_NLS)
///@todo
#endif
}




ot_bool sub_idcmp(id_tmpl* user_id, auth_entry* auth_id) {
    ot_bool id_check;
    
    if (user_id->length != auth_id->id->length) {
        return False;
    }
    if (user_id->length == 2) {
        return (((ot_u16*)user_id->value)[0] == ((ot_u16*)auth_id->id->value)[0]);
    }

    id_check    = (((ot_u32*)user_id->value)[0] == ((ot_u32*)auth_id->id->value)[0]);
    id_check   &= (((ot_u32*)user_id->value)[1] == ((ot_u32*)auth_id->id->value)[1]);
    return id_check;
}


ot_bool sub_authcmp(id_tmpl* user_id, id_tmpl* comp_id, ot_u8 mod_flags) {
    if ((user_id == NULL) || (user_id == comp_id)) 
        return True;
        
    return (ot_bool)auth_search_user(user_id, mod_flags);
}


ot_bool auth_isroot(id_tmpl* user_id) {
/// NULL is how root is implemented in internal calls
#if (_SEC_NLS)
    return sub_authcmp(user_id, auth_root, AUTH_FLAG_ISROOT)
#elif (_SEC_DLL)
    return (ot_bool)((user_id == NULL) || (user_id == auth_root)) 
#else
    return (ot_bool)(user_id == NULL);
#endif
}


ot_bool auth_isuser(id_tmpl* user_id) {
/// NULL is how root is implemented in internal calls
#if (_SEC_NLS)
    return sub_authcmp(user_id, auth_user, AUTH_FLAG_ISUSER)
#elif (_SEC_DLL)
    return (ot_bool)((user_id == NULL) || (user_id == auth_user))
#else
    return (ot_bool)(user_id == NULL);
#endif
}



ot_u8 auth_check(ot_u8 data_mod, ot_u8 req_mod, id_tmpl* user_id) {
#if (_SEC_ANY)
/// Find the ID in the table, then mask the user's mod with the file's mod
/// and the mod from the request (i.e. read, write).
    return (ot_u8)auth_search_user(user_id, (data_mod & req_mod));
    
#else
/// If the code gets here then there was not a user match, or the device is not
/// implementing user authentication.  Try guest access.
    return (0x07 & data_mod & req_mod);
#endif
}




auth_entry* auth_new_nlsuser(auth_entry* new_user, ot_u8* new_data) {
#if (_SEC_NLS)
#endif
    return NULL;
}



auth_entry* auth_search_user(id_tmpl* user_id, ot_u8 mod_flags) {
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


ot_u8* auth_get_dllskey(ot_u8 protocol, ot_u8 options, ot_u8* header) {
#if (_SEC_DLL)
    /// This implementation assumes that AES128 is the only supported crypto.
    /// DASH7 AES128 has protocol-id = xx000000.
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
        Twobytes scratch;
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



