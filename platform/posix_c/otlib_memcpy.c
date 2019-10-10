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
  * @file       /platform/stdc/otlib_memcpy.c
  * @author     JP Norair
  * @version    R100
  * @date       27 Oct 2017
  * @brief      Memcpy Driver for POSIX & STDC
  * @ingroup    Memcpy
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>
//#include <platform/memcpy.h>

#include <string.h>
#include <stdlib.h>

/** Platform memcpy Routine <BR>
  * ========================================================================<BR>
  * Similar to standard implementation of "memcpy"
  * Behavior is always blocking.  These functions either use the DMA or, if not
  * configured this way in the board support header, they use Duff's Device for
  * loop unrolling.
  */

#define DUFF_DEVICE_8(DST_OP, SRC_OP, LEN)      \
    if (LEN > 0) {                              \
        ot_int loops = (LEN + 7) >> 3;          \
        switch (LEN & 0x7) {                    \
            case 0: do {    DST_OP = SRC_OP;    \
            case 7:         DST_OP = SRC_OP;    \
            case 6:         DST_OP = SRC_OP;    \
            case 5:         DST_OP = SRC_OP;    \
            case 4:         DST_OP = SRC_OP;    \
            case 3:         DST_OP = SRC_OP;    \
            case 2:         DST_OP = SRC_OP;    \
            case 1:         DST_OP = SRC_OP;    \
                    } while (--loops > 0);      \
        }                                       \
    }





void ot_memcpy(ot_u8* dst, ot_u8* src, ot_uint length) {
    memcpy(dst, src, (size_t)length);
}

void ot_memcpy_2(ot_u16* dst, ot_u16* src, ot_uint length) {
    memcpy(dst, src, (size_t)length<<1);
}

void ot_memcpy_4(ot_u32* dst, ot_u32* src, ot_uint length) {
    memcpy(dst, src, (size_t)length<<2);
}


void ot_memset(ot_u8* dst, ot_u8 value, ot_uint length) {
    memset(dst, value, (size_t)length);
}


void ot_memset_2(ot_u16* dst, ot_u16 value, ot_uint length) {
    ot_memset( (ot_u8*)dst, (ot_u8)value, length<<1 );
}

void ot_memset_4(ot_u32* dst, ot_u32 value, ot_uint length) {
    ot_memset( (ot_u8*)dst, (ot_u8)value, length<<2 );
}





