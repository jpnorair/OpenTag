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
  * @file       /platform/stm32l0xx/otlib_rand.c
  * @author     JP Norair
  * @version    R100
  * @date       27 Aug 2014
  * @brief      Random Number driver for STM32L0
  * @ingroup    Rand
  *
  ******************************************************************************
  */


#include <otstd.h>
#include <otplatform.h>
#include <otlib/rand.h>




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



