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
  * @file       /platform/stm32wl55_m0/otlib_rand.c
  * @author     JP Norair
  * @version    R100
  * @date       16 Dec 2021
  * @brief      Random Number driver for STM32WL
  * @ingroup    Rand
  *
  * @todo the True Random Number Generator HW of STM32WL will get integrated.
  * 
  ******************************************************************************
  */


#include <otstd.h>
#include <otplatform.h>
#include <otlib/rand.h>
#include <otlib/crc16.h>



static ot_u32 prand_reg;


/** Platform Random Number Generation Routines <BR>
  * ========================================================================<BR>
  * The platform must be able to compute a strong random number (via function
  * platform_rand()) and a "pseudo" random number (via rand_prn8()).
  */


void rand_stream(ot_u8* rand_out, ot_int bytes_out) {
#if MCU_FEATURE(TRNG)

#else

#endif
}




void rand_prnseed(ot_u32 seed) {
    prand_reg = (ot_u16)seed;
}



ot_u32 rand_prn32() {
    ot_uni32 prn32;
    prn32.ushort[0] = rand_prn16();
    prn32.ushort[1] = rand_prn16();
    return prn32.ulong;
}




ot_u8 rand_prn8() {
    return (ot_u8)rand_prn16();
}



ot_u16 rand_prn16() {
    static ot_u16 _prn_freg;
    _prn_freg ^= LPTIM1->CNT;
    return crc16drv_block_manual( (ot_u8*)&_prn_freg, 2, _prn_freg );
}



