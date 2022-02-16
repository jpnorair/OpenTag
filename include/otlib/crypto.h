/*  Copyright 2008-2022, JP Norair
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




