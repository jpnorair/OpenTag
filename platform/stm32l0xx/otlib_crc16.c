/* Copyright 2013 JP Norair
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
  * @file       /platform/stm32l0xx/otlib_crc16.c
  * @author     JP Norair
  * @version    R100
  * @date       26 Aug 2014
  * @brief      CRC16 Hardware driver STM32L0
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

/*
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
*/


// Obsolete

//OT_INLINE void crc16drv_byte(ot_u8 databyte) {
//    (__IO ot_u8)CRC->DR = *(ot_u8*)block_addr;
//}

//OT_INLINE ot_u16 crc16drv_result() {
//    return (ot_u16)CRC->DR;
//}






