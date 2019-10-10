/* Copyright 2014 JP Norair
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
  * @file       /otlib/crypto.h
  * @author     JP Norair
  * @version    R101
  * @date       21 Aug 2014
  * @brief      Crypto Library wrapper to driver functions
  * @defgroup   Crypto
  *
  * Includes library functions for encryption and decryption in accordance with
  * with the DASH7 Specification (basically, EAX).  The actual cryptographic
  * code is implemented in the platform module, because this way it can take
  * advantage of platform-dependent optimizations.  These functions in crypto.h
  * are mainly wrappers for that platform functionality.
  *
  * @note DASH7 and OpenTag specify EAX as the basic form of encryption and
  * authentication.  The exact specification is:
  * <LI> EAX operating mode: AES-CTR + OMAC </LI>
  * <LI> 128 bit keys </LI>
  * <LI> 56 bit (7 byte) Nonce </LI>
  * <LI> 32 bit (4 byte) MAC Authentication Tag </LI>
  *
  * EAX is relatively new, but it is much better than is CCM in terms of speed,
  * memory requirement, code requirement, and also ability to be used with
  * arbitrary length data streams.  CCM, which is older and used by the likes
  * of WiFi and ZigBee, is worse on all accounts.  EAX can additionally be
  * processed iteratively as data streams-in (or out), although this driver
  * doesn't quite support that yet.
  *
  ******************************************************************************
  */


#ifndef __OTLIB_CRYPTO_H
#define __OTLIB_CRYPTO_H

#include <otstd.h>
#include <platform/config.h>    // connects to proper crypto types


/** EAX Generic-data Library Functions <BR>
  * ========================================================================<BR>
  * EAX_encrypt, EAX_decrypt
  */

/** @brief High-level function for encrypting a stream with specified EAX
  * @author JP Norair
  * @param dst          (ot_u8*) destination pointer for cipher-data
  * @param src          (ot_u8*) source pointer for plain-data
  * @param srclen       (ot_uint) number of bytes of plain-data to encrypt
  * @param nonce        (ot_u8*) EAX Nonce data, aka IV.
  * @param key          (ot_u8*) EAX key for encryption
  * @retval ot_int      Number of bytes added to cipher-data, greater than plain-data
  * @ingroup Crypto
  * @sa EAX_decrypt()
  * @sa auth_encrypt()
  *
  * EAX_encrypt() will encrypt the data from src and write it to dst.  Some
  * drivers may be in-place only, in which case the plain-data src will be
  * copied to dst before the encryption occurs
  *
  * Since EAX adds no additional cryptographic padding, the return value of
  * this function will always be 4 on success, since this is the length of the
  * Authentication tag, per specification.
  */
ot_int EAX_encrypt(void* dst, void* src, ot_uint srclen, void* nonce, void* key);



/** @brief High-level function for decrypting a stream with specified EAX
  * @author JP Norair
  * @param dst          (ot_u8*) destination pointer for plain-data
  * @param src          (ot_u8*) source pointer for cipher-data
  * @param srclen       (ot_uint) number of bytes of cipher-data to decrypt
  * @param nonce        (ot_u8*) EAX Nonce data, aka IV.
  * @param key          (ot_u8*) EAX key for decryption
  * @retval ot_int      Number of bytes removed from cipher-data to make plain-data
  * @ingroup Crypto
  * @sa EAX_encrypt()
  * @sa auth_decrypt()
  *
  * EAX_encrypt() will decrypt the data from src and write it to dst.  Some
  * drivers may be in-place only, in which case the plain-data src will be
  * copied to dst before the encryption occurs
  *
  * Since EAX adds no additional cryptographic padding, the return value of
  * this function will always be 4 on success, since this is the length of the
  * Authentication tag, per specification.
  */
ot_int EAX_decrypt(void* dst, void* src, ot_uint srclen, void* nonce, void* key);






/** EAX Driver prototypes <BR>
  * ========================================================================<BR>
  * The driver/platform code must implement these functions.
  */

ot_int EAXdrv_init(void* key, void* context);
ot_int EAXdrv_clear(void* context);
ot_int EAXdrv_encrypt(void* nonce, void* data, ot_uint datalen, void* context);
ot_int EAXdrv_decrypt(void* nonce, void* data, ot_uint datalen, void* context);


#endif




