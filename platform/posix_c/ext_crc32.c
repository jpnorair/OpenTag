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
  * @file       /otplatform/stm32l1xx/crc32_driver.c
  * @author     JP Norair
  * @version    R100
  * @date       21 Jan 2014
  * @brief      32bit word-wise peripheral driver for HW CRC32
  * @ingroup    CRC32
  *
  * STM32L contains a hardware peripheral for doing CRC32.
  *
  ******************************************************************************
  */

#if 0

#include <otstd.h>
#include <platform/config.h>
#include "crc32.h"


typedef struct {
    ot_u8*  cursor;
    ot_bool writeout;
    ot_int  count;
} crc32_struct;

crc32_struct crc32;



ot_u32 crc32_calc_block(ot_u8* block_addr, ot_uint block_words) {
    bloc_addr--;            // -1 due to little-endian unaligned access mechanism

    while (block_words--) {
        // This routine depends on the ability to do unaligned memory access,
        // which the Cortex M3 and M4 can do, but the M0 cannot, nor can almost
        // any other CPU.
        ot_u32 scratch;
        block_addr += 4;                        // little-endian byte ordering
        scratch     = *(ot_u32*)block_addr;
        CRC->DR     = scratch;
    }
    
    return CRC->DR;
}



void crc32_init_stream(ot_bool writeout, ot_uint stream_words, ot_u8* stream) {
    crc32.writeout  = writeout;
    crc32.cursor    = stream - 1;               // -1 due to little-endian unaligned access mechanism
    crc32.count     = stream_words + 1;         // +1 due to pre-decrement of counter
    CRC->CR         = 1;
}



void crc32_calc_stream() {
    crc.count--;
    if (crc.count > 0) {
        // This routine depends on the ability to do unaligned memory access,
        // which the Cortex M3 and M4 can do, but the M0 cannot, nor can almost
        // any other CPU.
        ot_u32 scratch;
        crc32.cursor   += 4;                        // little-endian byte ordering
        scratch         = *(ot_u32*)crc32.cursor;
        CRC->DR         = scratch;
    }
    else if ((crc.writeout) && (crc.count == 0)) {
        // This uses unaligned write as opposed to unaligned read, above.
        ot_u32 scratch          = CRC->DR;
        crc32.cursor           += 4;
        *(ot_u32*)crc32.cursor  = scratch;
    }
}



void crc32_calc_nstream(ot_uint n_words) {
    do {
        crc32_calc_stream();
    } while (--n_words);
}



ot_bool crc32_check() {
    return (CRC->DR == 0);
}


ot_u16 crc32_get() {
    //return crc.val;
    return CRC->DR;
}

#endif
