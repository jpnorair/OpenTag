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
  * @file       /otlib/crypto_aes128.h
  * @author     JP Norair
  * @version    R101
  * @date       23 Sept 2013
  * @brief      AES128 Library
  * @defgroup   AES128
  *
  * Includes library functions for AES128-CBC encryption and decryption, as
  * well as low-level software function for implementing the AES128 block
  * encryption/decryption itself.  The library functions may be used with the
  * low-level software back-end, or alternatively with an set of routines
  * defined in the platform layer, using the same functional interfaces.
  * 
  ******************************************************************************
  */
 

#ifndef __AES_128_H 
#define __AES_128_H 

#include "OT_types.h"
#include "OT_config.h"
#include "OT_platform.h" 


/** AES128 High-level Library Functions <BR>
  * ========================================================================<BR>
  * AES_encrypt_cbc, AES_decrypt_cbc
  */

/** @brief High-level function for encrypting a stream in-place with AES128-CBC
  * @author JP Norair
  * @param stream   (ot_u32*) Word-pointer to I/O stream
  * @param expkey   (ot_u32*) Word-pointer to expanded key
  * @param length   (ot_int) Number of BYTES of the stream.
  * @retval None
  * @ingroup AES128
  * @sa AES_decrypt_cbc()
  *
  * This implementation will assume that the first 16 bytes (128 bits) of the
  * stream is the initialization vector (IV).  The IV will not be encrypted in
  * the output stream.  The length parameter should be 16-byte aligned.  If it
  * is not, the remainder of the input will not be encrypted.
  * 
  * The options value is presently defined only as 0/1.  In systems with
  * OT_FEATURE(AES128_EXPKEY) not enabled, you can pass-in an expanded key or
  * a standard key, which this function will expand before encrypting. 
  * <LI> Option = 0: the key is a standard, 128 bit key </LI>
  * <LI> Option = 1: the key is an expanded, 352 bit key </LI>
  */
void AES_encrypt_cbc(ot_u32* stream, ot_u32* key, ot_int length, ot_u16 options);



/** @brief High-level function for decrypting a stream in-place with AES128-CBC
  * @author JP Norair
  * @param stream   (ot_u32*) Word-pointer to I/O stream
  * @param expkey   (ot_u32*) Word-pointer to expanded key
  * @param length   (ot_int) Number of BYTES of the stream.
  * @param options  (ot_u16) Operational options
  * @retval None
  * @ingroup AES128
  * @sa AES_encrypt_cbc()
  *
  * This implementation will assume that the first 16 bytes (128 bits) of the
  * stream is the initialization vector (IV).  The IV will not be encrypted in
  * the output stream.  The length parameter should be 16-byte aligned.  If it
  * is not, the remainder of the input will not be encrypted.
  *
  * The options value is presently defined only as 0/1.  In systems with
  * OT_FEATURE(AES128_EXPKEY) not enabled, you can pass-in an expanded key or
  * a standard key, which this function will expand before decrypting. 
  * <LI> Option = 0: the key is a standard, 128 bit key </LI>
  * <LI> Option = 1: the key is an expanded, 320 bit key </LI>
  */
void AES_decrypt_cbc(ot_u32* stream, ot_u32* key, ot_int length, ot_u16 options);











/** AES software codec <BR>
  * ========================================================================<BR>
  * There are two possible soft-AES implementations: "fast" and "lite."
  * Fast is optimized for speed, Lite is optimized for flash size. 
  * 
  * lite - version with unique key-expansion using:
  *        256 + 256 + 10*4 bytes data = 552 bytes of look-up tables.  
  *      
  * fast - version with different key-expansion for encryption/decryption using: 
  *        256 + 256 + 10*4 + 256*4*2 bytes data = 2058 bytes of look-up table.
  *
  * Expanded keys are larger than stored keys are.  It may be practical to 
  * store expanded keys in order to improve encryption and decryption speed.
  * Note that using Lite-mode with pre-computed expanded keys is often faster
  * than using Fast-mode without pre-computed keys, especially for decryption.
  */

#define AES_NEEDED      (OT_FEATURE(DLL_SECURITY) || OT_FEATURE(VL_SECURITY))
#define AES_EXPKEYS     (AES_NEEDED && OT_FEATURE(AES128_EXPKEYS))
#define AES_USEHW       (AES_NEEDED && MCU_FEATURE(AES128))
#define AES_USELITE     (AES_NEEDED && (MCU_FEATURE(AES128)==DISABLED))
#define AES_USEFAST     (AES_NEEDED && (MCU_FEATURE(AES128)==DISABLED) && OT_FEATURE(AES128_FAST))



// Number of 32 bit words to store an AES128 block
#define AES_BLOCK_SIZE  4

// Number of 32 bit words to store an AES128 key
#define AES_KEY_SIZE    4  

// Number of *BYTES* to allocate for an AES128 expanded key ...
// Encryption key is 44 bytes (352 bits)
// Decryption key is 40 bytes (320 bits)
#define AES_EXPKEY_SIZE 44 
                           





/** @brief Computes 352 bit expanded encryption key based on 128 bit stored key
  * @author STMicro MCD Applications Team
  * @param key              (ot_u32*) stored key (128 bits)
  * @param exp              (ot_u32*) expanded encryption key (352 bits)
  * @retval None
  * @ingroup AES128
  * @ingroup AES128
  */ 
void AES_expand_enckey(ot_u32* key, ot_u32* exp);



/** @brief Computes 320 bit expanded decryption key based on 128 bit stored key
  * @author STMicro MCD Applications Team
  * @param key              (ot_u32*) stored key (128 bits)
  * @param exp              (ot_u32*) expanded decryption key (320 bits)
  * @retval None
  * @ingroup AES128
  */
void AES_expand_deckey(ot_u32* key, ot_u32* exp); 
 
 

/** @brief Encrypts one 128 bit block of data with supplied, expanded key
  * @author STMicro MCD Applications Team
  * @param input_pointer    (ot_u32*) input data block pointer (128 bits)
  * @param output_pointer   (ot_u32*) output data block pointer (128 bits)
  * @param expkey           (ot_u32*) expanded encryption key (352 bits)
  * @retval None
  * @ingroup AES128
  */
void AES_encrypt_block(ot_u32* input_pointer, ot_u32* output_pointer, ot_u32* expkey); 
 
 
 
/** @brief Decrypts one 128 bit block of data with supplied, expanded key
  * @author STMicro MCD Applications Team
  * @param input_pointer    (ot_u32*) input data block pointer (128 bits)
  * @param output_pointer   (ot_u32*) output data block pointer (128 bits)
  * @param expkey           (ot_u32*) expanded decryption key (320 bits)
  * @retval None
  * @ingroup AES128
  */
void AES_decrypt_block(ot_u32* input_pointer, ot_u32* output_pointer, ot_u32* expkey); 
 
 

#endif
 
 
 
 


