/* Copyright 2008 STMicroelectronics
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
  * @file       /Platforms/.../aes128.h
  * @author     STMicro MCD Application Team 
  * @version    V1.0 (10 June 2008)
  * @date       17 November 2011
  * @brief      AES128 Driver
  * @defgroup   AES128
  *
  ******************************************************************************
  */
 

#ifndef __AES_128_H 
#define __AES_128_H 

#include "OT_types.h"
#include "OT_config.h"
#include "OT_platform.h" 


/** @note Note on AES configuration <BR>
  * ========================================================================<BR>
  * There are two possible AES implementations: "fast" is faster than "lite" but
  * "lite" uses much less Flash to store the lookup tables.  
  * 
  * lite - version with unique key-schedule using:
  *        256 + 256 + 10*4 bytes data = 552 bytes of look-up tables.  
  *      
  * fast - version with different key-schedule for encryption/decryption using: 
  *        256 + 256 + 10*4 + 256*4*2 bytes data = 2058 bytes of look-up table.
  */

#define AES_NEEDED      (OT_FEATURE(DLL_SECURITY) || OT_FEATURE(VL_SECURITY))
#define AES_USEHW       (AES_NEEDED && MCU_FEATURE(AES128))
#define AES_USEFAST     (AES_NEEDED && (MCU_FEATURE(AES128)==DISABLED))
#define AES_USELITE     (AES_NEEDED && (MCU_FEATURE(AES128)==DISABLED) && MCU_FEATURE(AES128_LITE))


// Number of 32 bit words to store an AES128 block
#define AES_BLOCK_SIZE  4

// Number of 32 bit words to store an AES128 key
#define AES_KEY_SIZE    4  

// Number of 32bits words to store in an AES128 expanded key ...
// The expanded key is the key after the keyschedule.
#define AES_EXPKEY_SIZE 44 
                           



void AES_load_static_key(ot_u8 key_id, ot_u32* key);

 
/** @brief According to key computes the expanded key exp for AES128 encryption. 
  * @ingroup AES128
  */ 
void AES_keyschedule_enc(ot_u32* key, ot_u32* exp);



/** @brief According to key computes the expanded key exp for AES128 decryption.
  * @ingroup AES128
  */
void AES_keyschedule_dec(ot_u32* key, ot_u32* exp); 
 
 
 
/** @brief Encrypts, according to the expanded key expkey, one block of 16 bytes  
  *        at address 'input_pointer' into the block at address 'output_pointer'
  *        (they can be the same).
  * @ingroup AES128
  */
void AES_encrypt(ot_u32* input_pointer, ot_u32* output_pointer, ot_u32* expkey); 
 
 
 
/** @brief Decrypts, according to the expanded key expkey, one block of 16 bytes  
  *        at address 'input_pointer' into the block at address 'output_pointer'
  *        (they can be the same).
  * @ingroup AES128
  */
void AES_decrypt(ot_u32* input_pointer, ot_u32* output_pointer, ot_u32* expkey); 
 
 

#endif
 
 
 
 


