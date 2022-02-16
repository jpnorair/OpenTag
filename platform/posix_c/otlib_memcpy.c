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





