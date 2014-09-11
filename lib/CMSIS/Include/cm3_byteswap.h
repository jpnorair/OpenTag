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
  * @file       /lib/CMSIS/Include/cm3_byteswap.h
  * @author     JP Norair
  * @version    R100
  * @date       21 Aug 2014
  * @brief      Byteswap functions for ARM CM3
  *
  ******************************************************************************
  */
  

#ifndef __CM3_BYTESWAP_H
#define __CM3_BYTESWAP_H

#include <stdint.h>
#include <core_cm3.h>


/** byteswap.h implementation      <BR>
  * ========================================================================<BR>
  * Uses CMSIS assembly functions from core_cm3.h
  */  
#define __bswap_16(x)   __REV16(x)
#define __bswap_32(x)   __REV(x)

#if defined(__GNUC__)
static inline uint64_t __bswap_64(uint64_t word64)   { 
    register uint32_t a     = __REV( ((uint32_t*)&word64)[0] );
    register uint32_t b     = __REV( ((uint32_t*)&word64)[1] );
    ((uint32_t*)&word64)[0] = b;
    ((uint32_t*)&word64)[1] = a;
    return word64;
}

#endif


#endif
