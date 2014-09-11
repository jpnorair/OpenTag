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
  * @file       /lib/CMSIS/Include/cm3_bitrotate.h
  * @author     JP Norair
  * @version    R100
  * @date       21 Aug 2014
  * @brief      Bit Rotation functions for ARM CM3
  *
  ******************************************************************************
  */
  

#ifndef __CM3_BITROTATE_H
#define __CM3_BITROTATE_H

#include <stdint.h>
#include <core_cm3.h>


/** bit rotation functions      <BR>
  * ========================================================================<BR>
  * ARM Cortex M3 contains rotate-right instruction but no rotate-left.  To do
  * rotate-left, simply rotate-right using the the bit conjugate.
  */  

#if defined(__GNUC__)

static inline __attribute__((always_inline))
uint32_t __rotr32_imm(uint32_t v, uint32_t sh) {
    uint32_t d;
    asm ("ROR %[Rd], %[Rm], %[Is]" : [Rd] "=r" (d) : [Rm] "r" (v), [Is] "i" (sh));
    return d;
}
 
static inline __attribute__((always_inline))
uint32_t __rotr32(uint32_t v, uint32_t sh) {
    uint32_t d;
    asm ("ROR %[Rd], %[Rm], %[Rs]" : [Rd] "=r" (d) : [Rm] "r" (v), [Rs] "r" (sh));
    return d;
}

static inline __attribute__((always_inline))
uint16_t __rotr16_imm(uint16_t v, uint32_t sh) {
    uint32_t d, tmp = v;
    asm ("ROR %[Rd], %[Rm], %[Is]" : [Rd] "=r" (d) : [Rm] "r" (tmp), [Is] "i" (sh));
    return (uint16_t)(d | (d >> 16));
}

static inline __attribute__((always_inline))
uint16_t __rotr16(uint16_t v, uint32_t sh) {
    uint32_t d, tmp = v;
    asm ("ROR %[Rd], %[Rm], %[Rs]" : [Rd] "=r" (d) : [Rm] "r" (tmp), [Rs] "r" (sh));
    return (uint16_t)(d | (d >> 16));
}

#define __rotl32_imm(v, sh)     __rotr32_imm(v, (32-sh))
#define __rotl32(v, sh)         __rotr32(v, (32-sh))
#define __rotl16_imm(v, sh)     __rotr16_imm(v, (16-sh))
#define __rotl16(v, sh)         __rotr16(v, (16-sh))




#else
#   error "Only GCC inline assembly is supported at the moment."
#endif




#endif
