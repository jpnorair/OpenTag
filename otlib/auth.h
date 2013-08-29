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
  * @version    R100
  * @date       12 Aug 2013
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

#include "OT_types.h"
#include "OTAPI.h"



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



/** @typedef auth_entry
  * The auth_entry type stores information about the key.
  *
  * mod         (ot_u8)     ISGLOBAL, ISROOT then 6 bits of file permissions
  * user_mod    (ot_u8)     User's available mod (permissions)
  * lifetime    (ot_u32)    UTC time of when key expires
  * id          (id_tmpl*)  Device ID of user
  * key         (ot_u8*)    Key of user (length implied from protocol id)
  */
typedef struct {
    ot_u8       mod;
    ot_u8       protocol;
    ot_u32      lifetime;
    id_tmpl*    id;
    ot_u8*      key;
} auth_entry;






/** @brief Authentication Module Initializer
  * @param None
  * @retval None
  * @ingroup Authentication
  */
void auth_init();



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



/** @brief Adds a new key entry and associated key data to the Crypto_Heap.
  * @param new_entry : (crypto_entry*) pointer to crypto_entry to be added
  * @param new_data : (ot_u8*) key data for this entry
  * @retval auth_entry* : pointer to Key in Heap.  NULL on error.
  * @ingroup Authentication
  *
  * If a new key is added, but there is no room left, the oldest key will be
  * deleted to make room for this new key.
  */
auth_entry* auth_new_nlsuser(auth_entry* new_user, ot_u8* new_data);



/** @brief Searches and returns a key based on UID or VID (if UID is NULL).
  * @param user_id      (id_tmpl*) Device ID of user
  * @param mod_flags   (ot_u8) extra user flags to require
  * @retval auth_entry* : pointer to Key in Heap.  NULL on error. 
  * @ingroup Authentication
  */
auth_entry* auth_search_user(id_tmpl* user_id, ot_u8 mod_flags);



/** @brief Returns the stored User or Root key that matches the protocol ID
  * @param protocol (ot_u8) Protocol ID of the DLLS method
  * @param options  (ot_u8) Protocol-specific options
  * @param header   (ot_u8*) optional header data (defined by protocol ID) 
  * @retval ot_u8*  Key Data
  * @ingroup Authentication

ot_u8* auth_get_dllskey(ot_u8 protocol, ot_u8 options, ot_u8* header);
  */



#endif





