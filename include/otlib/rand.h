/* Copyright 2013-14 JP Norair
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
  * @file       /include/otlib/rand.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Mar 2014
  * @brief      OTlib random number generator interface
  * @defgroup   Rand (Random Number Module)
  * @ingroup    Rand
  *
  * These functions are defined in OTlib and almost always implemented as
  * drivers in the platform section.
  *
  ******************************************************************************
  */


#ifndef __OTLIB_RAND_H
#define __OTLIB_RAND_H

#include <otstd.h>


#ifndef RAND_MAX
#   define RAND_MAX    0xFFFFFFFF
#endif

// Hooks to traditional features
#define srand(x)    rand_seed32(x)
#define rand()      rand_prn32()



/** @brief A random number generator.  Used within OpenTag.
  * @param rand_out     (ot_u8*) Pointer to the output random data
  * @param bytes_out    (ot_int) Number of random bytes to output
  * @retval none
  * @ingroup Rand
  *
  * @note The quality and performance of the random number generation are
  * implementation dependent.  It is a design goal for the random data output
  * to be FIPS 140-2 compliant and be computable in under 100us/byte.  Ideally,
  * it should be much faster, and one way to make it faster is to apply a zener
  * diode, which guarantees more bits of noise per acquisition.  A good zener
  * circuit and a 200ksps 12 bit ADC can therefore provide 5us/byte.
  */
void rand_stream(ot_u8* rand_out, ot_int bytes_out);



/** @brief Quick and dirty 8 bit pseudo-random value
  * @param None
  * @retval ot_u8       8 bit pseudo random number
  * @ingroup Platform
  *
  * A quickly generated 8 bit random number, not recommended for crypto.
  */
ot_u8 rand_prn8();



/** @brief Quick and dirty 16 bit pseudo-random value
  * @param None
  * @retval ot_u16       16 bit pseudo random number
  * @ingroup Platform
  *
  * A quickly generated 16 bit random number, not recommended for crypto.
  */
ot_u16 rand_prn16();



/** @brief Quick and dirty 32 bit pseudo-random value
  * @param None
  * @retval ot_u32       32 bit pseudo random number
  * @ingroup Platform
  *
  * A quickly generated 32 bit random number, not recommended for crypto.
  */
ot_u32 rand_prn32();



void rand_prnseed(ot_u32);


#endif
