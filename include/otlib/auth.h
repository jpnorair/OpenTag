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
#define AUTH_GUEST  (id_tmpl*)auth_guest
#define AUTH_USER   (id_tmpl*)auth_user
#define AUTH_ROOT   NULL

#define AUTH_FLAG_ISGLOBAL  0x80
#define AUTH_FLAG_ISROOT    0x40


///@todo bring this into OT_config.h eventually, when the feature gets supported
#define AUTH_NUM_ELEMENTS 0

extern const id_tmpl*   auth_root;      // this is self-root, uses local root key
extern const id_tmpl*   auth_user;      // this is self-user, uses local user key
extern const id_tmpl*   auth_guest;



typedef struct {
    ot_int      length;
    ot_u8       value[8];
} auth_id;

typedef struct {
    ot_u8   index;
    ot_u8   options;
    ot_u8   length;
    ot_u8   protocol;
    ot_u32  lifetime;
} auth_info;

typedef struct {
    auth_info*  info;
    auth_id*    id;
    ot_u8*      keydata;
} auth_handle;




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
void auth_init();


/** @brief Encrypts the data in a ot_queue, in-place
  * @param q            (ot_queue*) input q containing encrypted data
  * @param key_index    (ot_u8) Key Index to use for encryption
  * @param options      (ot_u8) Decryption options specific to type of Crypto
  * @retval None
  * @ingroup auth
  * @sa auth_decrypt_q
  * @sa auth_decrypt
  * @sa auth_encrypt
  *
  * It is important to align the front of the input queue per specification of
  * the crypto format to be used.  There are some protections built-into this
  * function, but if you get it wrong the decrypted output will probably be
  * wrong as well.
  *
  * Encryption is performed on the entire queue, between front and putcursor.
  */
//ot_int auth_encrypt_q(ot_queue* q, ot_u8 key_index, ot_u8 options);


/** @brief Decrypts the data in a ot_queue, in-place
  * @param q            (ot_queue*) input q containing encrypted data
  * @param key_index    (ot_u8) Key Index to use for decryption
  * @param options      (ot_u8) Decryption options specific to type of Crypto
  * @retval None
  * @ingroup auth
  * @sa auth_encrypt_q
  * @sa auth_decrypt
  * @sa auth_encrypt
  *
  * It is important to align the front of the input queue per specification of
  * the crypto format to be used.  There are some protections built-into this
  * function, but if you get it wrong the decrypted output will probably be
  * wrong as well.
  *
  * Decryption is performed on the entire queue, between front and putcursor.
  */
//ot_int auth_decrypt_q(ot_queue* q, ot_u8 key_index, ot_u8 options);




void auth_putnonce(ot_u8* dst, ot_uint limit);


/** @brief Encrypts a datastream, in-place
  * @param nonce        (ot_u8*) A Cryptographic nonce or init vector (IV)
  * @param data         (ot_u8*) stream for in-place encryption
  * @param datalen      (ot_uint) length of stream in bytes
  * @param key_index    (ot_u8) Key Index to use for encryption
  * @param options      (ot_u8) Decryption options specific to type of Crypto
  * @retval ot_int      number of bytes added to stream as result of encryption,
  *                       or negative on error.
  * @ingroup auth
  * @sa auth_decrypt
  *
  */
ot_int auth_encrypt(ot_u8* nonce, ot_u8* data, ot_uint datalen, ot_u8 key_index, ot_u8 options);


/** @brief Decrypts a datastream, in-place
  * @param nonce        (ot_u8*) A Cryptographic nonce or init vector (IV)
  * @param data         (ot_u8*) stream for in-place decryption
  * @param datalen      (ot_uint) length of stream in bytes
  * @param key_index    (ot_u8) Key Index to use for decryption
  * @param options      (ot_u8) Decryption options specific to type of Crypto
  * @retval ot_int      number of bytes removed from stream as result of
  *                       decryption, or negative on error.
  * @ingroup auth
  * @sa auth_encrypt
  *
  * It is important to align the front of the iostream and length per the
  * specification of the crypto format to be used.
  */
ot_int auth_decrypt(ot_u8* nonce, ot_u8* data, ot_uint datalen, ot_u8 key_index, ot_u8 options);


/** @brief Returns Decryption-Key DATA of a given key index, but no Auth/Sec metadata
  * @param index    (ot_u8) Key Index input
  * @retval void*   Pointer to Key Data.  Always is word-aligned.
  * @ingroup auth
  * @sa auth_get_enckey
  *
  * This function is generally used by crypto libraries that interface with the
  * Authentication Module, in order to retrieve key data.
  */
ot_u8* auth_get_deckey(ot_u8 index);


/** @brief Returns Encryption-Key DATA of a given key index, but no Auth/Sec metadata
  * @param index    (ot_u8) Key Index input
  * @retval void*   Pointer to Key Data.  Always is word-aligned.
  * @ingroup auth
  * @sa auth_get_deckey
  *
  * This function is generally used by crypto libraries that interface with the
  * Authentication Module, in order to retrieve key data.
  */
ot_u8* auth_get_enckey(ot_u8 index);









/** User Authentication Routines <BR>
  * ========================================================================<BR>
  * Specifically, the Auth-Sec ALP should have hooks into these functions.
  */

/** @brief Returns True if the supplied ID has root access
  * @param user_id      (id_tmpl*) pointer to a UID/VID template
  * @retval ot_bool     True when ID is root
  * @ingroup Authentication
  */
ot_bool auth_isroot(id_tmpl* user_id);



/** @brief Returns True if the supplied ID has user access (root passes this)
  * @param user_id      (id_tmpl*) pointer to a UID/VID template
  * @retval ot_bool     True when ID is user or root
  * @ingroup Authentication
  */
ot_bool auth_isuser(id_tmpl* user_id);



/** @brief Checks the authentication data per supplied user, and provides yes or no
  * @param data_mod     (ot_u8) Veelite Mod value of the desired data element
  * @param req_mod      (ot_u8) Requested Mod for operation (e.g. read or write)
  * @param user_id      (id_tmpl*) pointer to a UID/VID template
  * @retval ot_u8       Non-zero when authentication is OK
  * @ingroup Authentication
  */
ot_u8 auth_check(ot_u8 data_mod, ot_u8 req_mod, id_tmpl* user_id);











/** Functions Typically Used with ALP <BR>
  * ========================================================================<BR>
  * Specifically, the Auth-Sec ALP should have hooks into these functions.
  */

/** @brief Finds a Key-Index, given a User ID
  * @param handle       (auth_handle*) Output handle for key being read
  * @param user_id      (id_tmpl*) input user id
  * @retval ot_u8       Zero (0) on success, else an error code
  * @ingroup Authentication
  */
ot_u8 auth_find_keyindex(auth_handle* handle, id_tmpl* user_id);


/** @brief Reads an Auth/Sec Key Element, given key index
  * @param handle       (auth_handle*) Output handle for key being read
  * @param key_index    (ot_u16) input key index to read
  * @retval ot_u8       Zero (0) on success, else an error code
  * @ingroup Authentication
  */
ot_u8 auth_read_key(auth_handle* handle, ot_u16 key_index);


/** @brief Updates an Auth/Sec Key Element, given
  * @param handle       (auth_handle*) Input Key Information
  * @param key_index    (ot_u16) Input Key Index
  * @retval ot_u8       Zero (0) on success, else an error code
  * @ingroup Authentication
  *
  * Sub-elements in "handle" that are set to NULL will be skipped during the
  * update procedure.
  */
ot_u8 auth_update_key(auth_handle* handle, ot_u16 key_index);


/** @brief Create a Key given Auth/Sec information
  * @param key_index    (ot_u16*) Output Key Index
  * @param handle       (auth_handle*) Input Key & Auth/Sec Information
  * @retval ot_u8       Zero (0) on success, else an error code
  * @ingroup Authentication
  */
ot_u8 auth_create_key(ot_u16* key_index, auth_handle* handle);


/** @brief Deletes an Auth/Sec Key Element, given key index
  * @param key_index    (ot_u16) input key index to delete
  * @retval ot_u8       Zero (0) on success, else an error code
  * @ingroup Authentication
  */
ot_u8 auth_delete_key(ot_u16 key_index);







/* @brief Adds a new key entry and associated key data to the Crypto_Heap.
  * @param handle       (auth_handle*) Output handle for this new entry
  * @param new_user     (id_tmpl*) ID information for new user
  * @param new_info     (auth_info*) Auth/Sec parameters
  * @param new_key      (ot_u8*) cryptographic key data
  * @retval ot_u8       Zero (0) on success, else an error code
  * @ingroup Authentication
  *
  * If a new key is added, but there is no room left, the oldest key will be
  * deleted to make room for this new key.

ot_u8 auth_new_nlsuser(auth_handle* handle, id_tmpl* new_user, auth_info* new_info, ot_u8* new_key);
*/


/* @brief Searches and returns a key based on UID or VID (if UID is NULL).
  * @param handle       (auth_handle*) Output handle for this new entry
  * @param user_id      (id_tmpl*) Device ID of user to find auth/sec data
  * @param mod_flags    (ot_u8) extra user flags
  * @retval ot_u8       Zero (0) on success, else an error code
  * @ingroup Authentication

ot_u8 auth_search_user(auth_handle* handle, id_tmpl* user_id, ot_u8 mod_flags);
*/

#endif





