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
  * @file       /otplatform/stm32f0xx/eax_driver.c
  * @author     JP Norair
  * @version    R100
  * @date       21 Aug 2014
  * @brief      EAX Cryptographic Driver for STM32F0xx
  * @defgroup   EAX Driver
  * @ingroup    EAX Driver
  *
  * At present, no STM32F0 devices have AES hardware built-in.  OTEAX is used
  * in software mode for all F0 devices.
  * 
  ******************************************************************************
  */

#include <otstd.h>
#if (OT_FEATURE(DLL_SECURITY) || OT_FEATURE(NL_SECURITY) || OT_FEATURE(VL_SECURITY))

// This is the OTlib crypto header that defines the driver functions
#include <otlib/crypto.h>

// This is the OTEAX library include, which does the heavy lifting
#include <oteax.h>


OT_INLINE ot_int EAXdrv_init(ot_u8* key, EAXdrv_t* context) {
    return (ot_int)eax_init_and_key((const unsigned char*)key, context);
}

OT_INLINE ot_int EAXdrv_clear(EAXdrv_t* context) {
    return (ot_int)eax_end(context);
}

OT_INLINE ot_int EAXdrv_encrypt(ot_u8* nonce, ot_u8* data, ot_uint datalen, EAXdrv_t* context) {
    return (ot_int)eax_encrypt_message(nonce, data, datalen, context);
}

OT_INLINE ot_int EAXdrv_decrypt(ot_u8* nonce, ot_u8* data, ot_uint datalen, EAXdrv_t* context) {
    return (ot_int)eax_decrypt_message(nonce, data, datalen, context);
}


#endif
