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
  * @file       /otplatform/stm32l1xx/crc32_STM32L1xx.c
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


#include "OT_platform.h"
#include <ext/crc32.h>


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


