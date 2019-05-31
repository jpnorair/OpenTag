/*  Copyright 2013, JP Norair
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
  * @file       /otlib/auth.h
  * @author     JP Norair
  * @version    R102
  * @date       21 Aug 2014
  * @brief      Authentication & Cryptography Functionality
  * @defgroup   Authentication (Authentication Module)
  * @ingroup    Authentication
  *
  * The crypto module includes a table that stores key matches, and functions
  * that are mostly hooks to application or platform-level crypto functions.
  * The requirement is that the crypto table is wiped whenever power goes off
  * or the chip is reset.  In this implementation, that requirement is met by
  * putting the table in volatile RAM.
  ******************************************************************************
  */


#ifndef __AUTH_H
#define __AUTH_H

#include <otstd.h>
#include <otlib/queue.h>
#include <m2/tmpl.h>

/// Default user types
#define AUTH_GUEST  (id_tmpl*)&auth_guest
#define AUTH_USER   (id_tmpl*)&auth_user
#define AUTH_ROOT   NULL

#define AUTH_FLAG_ISGLOBAL  0x80
#define AUTH_FLAG_ISROOT    0x40


extern const id_tmpl   auth_root;      // this is self-root, uses local root key
extern const id_tmpl   auth_user;      // this is self-user, uses local user key
extern const id_tmpl   auth_guest;


typedef enum {
    AUTHMOD_guest = (0),
    AUTHMOD_user  = (7<<0),
    AUTHMOD_root  = (7<<3)
    //AUTHMOD_guest = (7<<0),
    //AUTHMOD_user  = (7<<3),
    //AUTHMOD_root  = (1<<6)
} authmod_t;

typedef enum {
    KEYTYPE_none    = 0,
    KEYTYPE_AES128  = 1,
} keytype_t;




/** High Level Cryptographic Interface Functions <BR>
  * ========================================================================<BR>
  * init, encrypt, decrypt
  */

/** @brief Authentication Module Initializer
  * @param None
  * @retval None
  * @ingroup Authentication
  *
  * Call once during startup, or whenever user-code decides it is a good idea
  * to refresh the Key table.
  */
void auth_init(void);



/** @brief Deinitialize Authentication Module
  * @param None
  * @retval None
  * @ingroup Authentication
  *
  * This will wipe and deallocate all internal data used by the auth module.
  * If your implementation does not use dynamic memory allocation, you can
  * ignore this function, however it also wipes the RAM for security purposes.
  */
void auth_deinit(void);



/** @brief Writes a 32 bit nonce to destination in memory, and advances nonce.
  * @param dst          (void*) destination in memory to write nonce
  * @param total_size   (ot_uint) total bytes to put to destination
  * @retval None
  * @ingroup Authentication
  * @sa auth_putnonce_q()
  * @sa auth_getnonce()
  * 
  * If the "total_size" input is less than 4, then only part of the nonce will be 
  * written.  If "total_size" input is greater than 4, then (total_size - 4)
  * bytes will skipped from dst, before writing the nonce.
  */
void auth_putnonce(void* dst, ot_uint total_size);


/** @brief Writes a pre-padded 32 bit nonce to a queue
  * @param q            (ot_queue*) destination in memory to write nonce
  * @param total_size   (ot_uint) total bytes to put to queue
  * @retval None
  * @ingroup Authentication
  * @sa auth_putnonce()
  * 
  * Identical to auth_putnonce() except with queue output.
  */
void auth_putnonce_q(ot_queue* q, ot_uint total_size);


/** @brief Returns a 32 bit nonce, and advances nonce.
  * @param None
  * @retval ot_u32      Internal 32 bit nonce
  * @ingroup Authentication
  * @sa auth_putnonce()
  * 
  * This function returns the internal nonce for you to do with as you wish.
  */
ot_u32 auth_getnonce(void);



/** @brief Encrypts a datastream, in-place
  * @param iv           (void*) A Cryptographic init vector (IV)
  * @param data         (void*) stream for in-place encryption
  * @param datalen      (ot_uint) length of stream in bytes
  * @param key_index    (ot_uint) Key Index to use for encryption
  * @retval ot_int      number of bytes added to stream as result of encryption,
  *                       or negative on error.
  * @ingroup auth
  * @sa auth_decrypt
  * @sa auth_putnonce
  * @sa auth_getnonce
  *
  * Usage of this function should be mostly self-explanatory, however some 
  * information should be provided about the usage of "nonce".
  *
  * The "iv" input will take 7 bytes from the iv pointer and use this as the iv.
  * If your device cannot address at the byte level, zero pad the 8th byte.
  *
  * In typical usage, auth_putnonce() is used before this function to write the
  * nonce to a packet buffer, then the raw data is put onto the packet buffer, 
  * then this function is called with iv pointing to 3 bytes ahead of the nonce
  * and data pointing to the start of the raw data.
  *
  * The data is encrypted IN PLACE.  There is no double bufferring required.
  */
ot_int auth_encrypt(void* iv, void* data, ot_uint datalen, ot_uint key_index);



/** @brief Encrypts the data in a ot_queue, in-place
  * @param q            (ot_queue*) input q containing encrypted data
  * @param key_index    (ot_uint) Key Index to use for encryption
  * @retval None
  * @ingroup auth
  * @sa auth_decrypt_q
  * @sa auth_decrypt
  * @sa auth_encrypt
  */
ot_int auth_encrypt_q(ot_queue* q, ot_uint key_index);




/** @brief Decrypts a datastream, in-place
  * @param iv           (void*) A Cryptographic init vector (IV)
  * @param data         (void*) stream for in-place decryption
  * @param datalen      (ot_uint) length of stream in bytes
  * @param key_index    (ot_uint) Key Index to use for decryption
  * @retval ot_int      number of bytes removed from stream as result of
  *                       decryption, or negative on error.
  * @ingroup auth
  * @sa auth_encrypt
  *
  * This function is very similar to auth_encrypt(), but it should be used in 
  * decryption processes.  The major difference is that the decryption process
  * will validate the authentication tag footer and return 4 on success, 
  * whereas encryption adds this footer.
  */
ot_int auth_decrypt(void* iv, void* data, ot_uint datalen, ot_uint key_index);


/** @brief Decrypts the data in a ot_queue, in-place
  * @param q            (ot_queue*) input q containing encrypted data
  * @param key_index    (ot_uint) Key Index to use for decryption
  * @retval None
  * @ingroup auth
  * @sa auth_encrypt_q
  * @sa auth_decrypt
  * @sa auth_encrypt
  */
ot_int auth_decrypt_q(ot_queue* q, ot_uint key_index);



/** @brief Returns Decryption-Key DATA of a given key index, but no Auth/Sec metadata
  * @param key      (void**) Is loaded with pointer to expanded key.  Always word-aligned.
  * @param index    (ot_uint) Key Index input
  * @retval ot_int  returns sizeof the key data, or negative on error.
  * @ingroup auth
  * @sa auth_get_enckey
  *
  * For expert use only.
  *
  * @note for the standard EAX crypto implementation, there is only one key for 
  *       both encryption and decryption.
  */
ot_int auth_get_deckey(void** key, ot_uint index);



/** @brief Returns Encryption-Key DATA of a given key index, but no Auth/Sec metadata
  * @param key      (void**) Is loaded with pointer to expanded key.  Always word-aligned.
  * @param index    (ot_uint) Key Index input
  * @retval ot_int  returns sizeof the key data, or negative on error.
  * @ingroup auth
  * @sa auth_get_enckey
  *
  * For expert use only.
  *
  * @note for the standard EAX crypto implementation, there is only one key for 
  *       both encryption and decryption.
  */
ot_int auth_get_enckey(void** key, ot_uint index);









/** User Authentication Routines <BR>
  * ========================================================================<BR>
  * Intended to be used internally or via a ROOT-authenticated connection to
  * an Auth-Sec ALP.
  */

/** @brief Returns True if the supplied ID has root access
  * @param user_id      (id_tmpl*) pointer to a UID/VID template
  * @retval ot_bool     True when ID is root
  * @ingroup Authentication
  */
ot_bool auth_isroot(const id_tmpl* user_id);


/** @brief Returns True if the supplied ID has user access (root passes this)
  * @param user_id      (id_tmpl*) pointer to a UID/VID template
  * @retval ot_bool     True when ID is user or root
  * @ingroup Authentication
  */
ot_bool auth_isuser(const id_tmpl* user_id);


/** @brief Checks the authentication data per supplied user, and provides yes or no
  * @param req_mod      (ot_u8) Requested Veelite Mod value of the data element
  * @param rw_mod       (ot_u8) Read/Write Mod-mask for operation
  * @param user_id      (id_tmpl*) pointer to a UID/VID template
  * @retval ot_u8       Non-zero when authentication is OK
  * @ingroup Authentication
  */
ot_u8 auth_check(ot_u8 req_mod, ot_u8 rw_mod, const id_tmpl* user_id);


/** @brief Searches authentication tables for a key that qualifies ID and Mods
  * @param user_id      (id_tmpl*) pointer to a ID template for search
  * @param req_mod      (ot_u8) requested Mods for the key
  * @retval ot_int      Negative on error, or the index of the matching key.
  * @ingroup Authentication
  *
  * auth_search_user is intended for internal use.  It is exposed simply because
  * there's little value in hiding functions in open-source software.  For the
  * same functionality in a way that fits the Auth-Sec ALP API, use auth_check.
  */
ot_int auth_search_user(const id_tmpl* user_id, ot_u8 req_mod);


/** @brief Loads a User ID from the user ID store at an index in key table.
  * @param user_id      (id_tmpl*) pointer to a ID template to load into
  * @param key_index    (ot_uint) key_index for load
  * @retval ot_u8       Non-zero on error
  * @ingroup Authentication
  *
  * Like auth_search_user(), this is a bonus function intended for internal use.
  */
ot_u8 auth_get_user(id_tmpl* user_id, ot_uint key_index);



/** @brief Provides a intrinsic user id to a caller that has its own user management
  * @param index        (ot_uint) Surrogate Key Index: 0=root, 1=user, 2=guest
  * @retval id_tmpl*    pointer to surrogate user ID
  * @ingroup Authentication
  *
  * This function bypasses internal key management.  
  * Do not use it unless you know exactly what you are doing, otherwise you
  * might have a security vulnerability.  If you don't use it, there is no 
  * incidental vuln, because it works on different data than the key data.
  */
const id_tmpl* auth_intrinsic_user(ot_uint index);






/** Key Management Functions <BR>
  * ========================================================================<BR>
  * Intended to be used internally or via a ROOT-authenticated connection to
  * an Auth-Sec ALP.
  */

/** @brief Finds a Key-Index, given a User ID
  * @param key_index    (ot_uint*) returns a key index
  * @param user_id      (id_tmpl*) input user id
  * @retval ot_u8       Zero (0) on success, else an error code
  * @ingroup Authentication
  *
  * Errors:
  * 1   : key_index is NULL
  * 255 : key cannot be found for this ID
  */
ot_u8 auth_find_keyindex(ot_uint* key_index, const id_tmpl* user_id);


/** @brief Update Lifetime of an existing key.
  * @param key_index    (ot_uint*) Output Key Index.
  * @param new_lifetime (ot_u32) Key lifetime in seconds.
  * @param user_id      (const id_tmpl*) Pointer to ID that key is associated with.
  * @retval ot_u8       Zero (0) on success, else an error code
  * @ingroup Authentication
  *
  * Errors: 
  * 1   : key_index is NULL
  * 3   : new_lifetime is out of bounds
  * 255 : key cannot be found for this ID
  */
ot_u8 auth_refresh_key(ot_uint* key_index, ot_u32 new_lifetime, const id_tmpl* user_id);


/** @brief Create a Key given Auth/Sec information
  * @param key_index    (ot_uint*) Output Key Index
  * @param type         (keytype_t) Type of cryptographic key
  * @param lifetime     (ot_u32) Key lifetime in seconds.
  * @param keydata      (void*) pointer to key data.  Length is implicit from type param.
  * @param user_id      (const id_tmpl*) Pointer to ID that key will be associated with.
  * @retval ot_u8       Zero (0) on success, else an error code
  * @ingroup Authentication
  *
  * If one of the inputs is not valid, an error code will be returned corresponding to the 
  * erroneous input (e.g. "1" means that key_index is NULL, "2" means type is out of range,
  * etc).
  *
  * Other errors:
  * 254 : cannot create new key because ID is already found in table
  * 255 : cannot create new key due to full table (or no table).
  */
ot_u8 auth_create_key(ot_uint* key_index, keytype_t type, ot_u32 lifetime, void* keydata, const id_tmpl* user_id);


/** @brief Deletes an Auth/Sec Key Element, given key index
  * @param key_index    (ot_uint) input key index to delete
  * @retval ot_u8       Zero (0) on success, else an error code
  * @ingroup Authentication
  *
  * Errors:
  * 1   : key_index is out of range
  * 255 : cannot delete key (generally because no auth implementation)
  */
ot_u8 auth_delete_key(ot_uint key_index);



/** @brief Gets a pointer to a key from the key table
  * @param keydata      (void**) pointer to allocated key data pointer (void*)
  * @param key_index    (ot_uint) input key index to retrieve
  * @retval keytype_t   KEYTYPE_none on error, else corresponding key type.
  * @ingroup Authentication
  *
  * @note This function should not be exposed to external processes or
  * messaging protocols.  It is for privileged usage within the process only.
  */
keytype_t auth_get_key(void** keydata, ot_uint key_index);
















#endif





