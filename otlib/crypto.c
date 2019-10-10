/*  Copyright 2014 JP Norair
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
  */
/**
  * @file       /otlib/crypto.c
  * @author     JP Norair
  * @version    R102
  * @date       21 Aug 2014
  * @brief      EAX Wrapper
  * @ingroup    AES128
  * 
  * crypto.c includes a general data wrapper for all EAX cryptography jobs, 
  * in addition to default software-based implementations of the driver
  * functions.  These directly utilize the OTEAX routines.  
  *
  * Some platforms have AES hardware.  These platforms can get much better 
  * performance through the usage of customized versions of this code that 
  * utilize the HW.  You can find these HW-based implementations together with 
  * the platform code, called "otlib_eax.c"
  * 
  ******************************************************************************
  */

// The platform should include the AES Driver, which must specify some data 
// types and functions

///@todo put EXTF's around these functions

#include <otstd.h>
#if (OT_FEATURE(DLL_SECURITY) || OT_FEATURE(NL_SECURITY) || OT_FEATURE(VL_SECURITY))

#include <otlib/crypto.h>
#include <otlib/memcpy.h>

// One of the great things about EAX is that the cryptography process is
// largely symmetric.

ot_int __EAX_crypt( ot_u8* dst, ot_u8* src, ot_uint srclen, ot_u8* nonce, ot_u8* key,
                    ot_int (*__crypt)(void*, void*, ot_uint, void*)  ) {
    
    EAXdrv_t    context;        // Might benefit by putting this outside the stack
    ot_int      retval;
    
    ///@todo could add option parameter to prevent initialization of context
    retval = EAXdrv_init(key, &context);
    
    if (retval == 0) {
        if ((dst != src) && (dst != NULL) && (srclen != 0)) {
            ot_memcpy(dst, src, srclen);
        }
        if (__crypt(nonce, dst, srclen, &context) == 0) {
            return 4;
        }
        retval--;
    }
    
    return retval;
}


ot_int EAX_encrypt(void* dst, void* src, ot_uint srclen, void* nonce, void* key) {
    return __EAX_crypt(dst, src, srclen, nonce, key, &EAXdrv_encrypt);
}

ot_int EAX_decrypt(void* dst, void* src, ot_uint srclen, void* nonce, void* key) {
    return __EAX_crypt(dst, src, srclen, nonce, key, &EAXdrv_decrypt);
}





/** EAX default software driver (direct calls to OTEAX)<BR>
  * ========================================================================<BR>
  */
#include <oteax.h>

#ifndef EXTF_EAXdrv_init
OT_WEAK ot_int EAXdrv_init(void* key, void* context) {
    return (ot_int)eax_init_and_key(key, (EAXdrv_t*)context);
}
#endif

#ifndef EXTF_EAXdrv_clear
OT_WEAK ot_int EAXdrv_clear(void* context) {
    return (ot_int)eax_end((EAXdrv_t*)context);
}
#endif

#ifndef EXTF_EAXdrv_encrypt
OT_WEAK ot_int EAXdrv_encrypt(void* nonce, void* data, ot_uint datalen, void* context) {
    return (ot_int)eax_encrypt_message(nonce, data, datalen, (EAXdrv_t*)context);
}
#endif

#ifndef EXTF_EAXdrv_decrypt
OT_WEAK ot_int EAXdrv_decrypt(void* nonce, void* data, ot_uint datalen, void* context) {
    return (ot_int)eax_decrypt_message(nonce, data, datalen, (EAXdrv_t*)context);
}
#endif







// AES not supported in the build/platform
#else
ot_int EAX_encrypt(void* dst, void* src, ot_uint srclen, void* nonce, void* key) {
    return -1;
}
ot_int EAX_decrypt(void* dst, void* src, ot_uint srclen, void* nonce, void* key) {
    return -1;
}

   
#endif
