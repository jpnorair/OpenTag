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
  * @file       /platform/stm32wl55_m0/otlib_crc16.c
  * @author     JP Norair
  * @version    R100
  * @date       16 Dec 2021
  * @brief      CRC16 Hardware driver STM32WL
  * @ingroup    CRC16
  *
  * It's not apparent this hardware actually works in an advantageous way.
  * Still testing that.
  * 
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>
#include <otlib/crc16.h>

#if (MCU_FEATURE(CRC16) == ENABLED)

OT_INLINE ot_u16 crc16drv_init() {
    //CRC->INIT   = 0xFFFF;
    //CRC->POL    = 0x8005;
    //CRC->CR     = (b01 << 3) | 1; 
    return 0xFFFF;
}




ot_u16 crc16drv_block_manual(ot_u8* block_addr, ot_int block_size, ot_u16 init) {
/// @note Interrupts must be disabled during the runtime of this function, in
/// order to prevent CRC hardware registers from getting borked by multiple 
/// users.  Fortunately, CRC HW is extremely fast, about 1 byte/cycle.  For 
/// doing long computations, though, it is still recommended to use the CRC16
/// streaming object rather than this function directly.
    //ot_int units;
    ot_u16 output;

    platform_disable_interrupts();
    CRC->INIT   = init;
    CRC->POL    = 0x8005;
    CRC->CR     = (b01 << 3) | 1; 
    
    ///@todo unroll this loop using alignment optimizations
    while (block_size--) {
        CRC->DR = *block_addr++;
    }
    
    output  = (__IO ot_u16)CRC->DR;
    platform_enable_interrupts();
    
    return output;
}




OT_INLINE ot_u16 crc16drv_block(ot_u8* block_addr, ot_int block_size) {
    return crc16drv_block_manual(block_addr, block_size, 0xFFFF);
}



// Obsolete

//OT_INLINE void crc16drv_byte(ot_u8 databyte) {
//    (__IO ot_u8)CRC->DR = *(ot_u8*)block_addr;
//}

//OT_INLINE ot_u16 crc16drv_result() {
//    return (ot_u16)CRC->DR;
//}



#endif


