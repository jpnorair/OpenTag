/* Copyright 2017 JP Norair
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
  * @file       /platform/stdc/otlib_rand.c
  * @author     JP Norair
  * @version    R100
  * @date       27 Oct 2017
  * @brief      Random Number driver for POSIX C & STD C
  * @ingroup    Rand
  *
  ******************************************************************************
  */


#include <otstd.h>
#include <otplatform.h>
#include <otlib/rand.h>

#include <time.h>
#include <stdlib.h>


void rand_stream(ot_u8* rand_out, ot_int bytes_out) {
    while (--bytes_out >= 0) {
        *rand_out++ = rand_prn8();
    }
}

void rand_prnseed(ot_u32 seed) {
    if (seed == 0) {
        seed = time(NULL);
    }
    srand(seed);
}


ot_u32 rand_prn32() {
    return (ot_u32)rand();
}


ot_u8 rand_prn8() {
    return (ot_u8)(rand() & 255);
}


ot_u16 rand_prn16() {
    return (ot_u8)(rand() & 65535);
}

