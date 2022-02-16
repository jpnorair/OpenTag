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
