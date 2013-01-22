/* Copyright 2010-2012 JP Norair
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
  * @file       /otplatform/stm32l1xx/platform_isr_STM32L.c
  * @author     JP Norair
  * @version    R100
  * @date       14 Jan 2013
  * @brief      Software CRC-CCITT implementation for STM32L
  * @ingroup    Platform
  *
  ******************************************************************************
  */


#include "OT_platform.h"
#include "crc16_table.h"


/** @var crc_table
  * @ingroup Platform
  *
  * The CRC16 used here is the CCITT variant that yields 29BF from ASCII
  * input "123456789"
  */
static const ot_u16 crc16_table[256] = { 
    CRCx00, CRCx01, CRCx02, CRCx03, CRCx04, CRCx05, CRCx06, CRCx07, 
    CRCx08, CRCx09, CRCx0A, CRCx0B, CRCx0C, CRCx0D, CRCx0E, CRCx0F, 
    CRCx10, CRCx11, CRCx12, CRCx13, CRCx14, CRCx15, CRCx16, CRCx17, 
    CRCx18, CRCx19, CRCx1A, CRCx1B, CRCx1C, CRCx1D, CRCx1E, CRCx1F, 
    CRCx20, CRCx21, CRCx22, CRCx23, CRCx24, CRCx25, CRCx26, CRCx27, 
    CRCx28, CRCx29, CRCx2A, CRCx2B, CRCx2C, CRCx2D, CRCx2E, CRCx2F, 
    CRCx30, CRCx31, CRCx32, CRCx33, CRCx34, CRCx35, CRCx36, CRCx37, 
    CRCx38, CRCx39, CRCx3A, CRCx3B, CRCx3C, CRCx3D, CRCx3E, CRCx3F, 
    CRCx40, CRCx41, CRCx42, CRCx43, CRCx44, CRCx45, CRCx46, CRCx47, 
    CRCx48, CRCx49, CRCx4A, CRCx4B, CRCx4C, CRCx4D, CRCx4E, CRCx4F, 
    CRCx50, CRCx51, CRCx52, CRCx53, CRCx54, CRCx55, CRCx56, CRCx57, 
    CRCx58, CRCx59, CRCx5A, CRCx5B, CRCx5C, CRCx5D, CRCx5E, CRCx5F, 
    CRCx60, CRCx61, CRCx62, CRCx63, CRCx64, CRCx65, CRCx66, CRCx67, 
    CRCx68, CRCx69, CRCx6A, CRCx6B, CRCx6C, CRCx6D, CRCx6E, CRCx6F, 
    CRCx70, CRCx71, CRCx72, CRCx73, CRCx74, CRCx75, CRCx76, CRCx77, 
    CRCx78, CRCx79, CRCx7A, CRCx7B, CRCx7C, CRCx7D, CRCx7E, CRCx7F, 
    CRCx80, CRCx81, CRCx82, CRCx83, CRCx84, CRCx85, CRCx86, CRCx87, 
    CRCx88, CRCx89, CRCx8A, CRCx8B, CRCx8C, CRCx8D, CRCx8E, CRCx8F, 
    CRCx90, CRCx91, CRCx92, CRCx93, CRCx94, CRCx95, CRCx96, CRCx97, 
    CRCx98, CRCx99, CRCx9A, CRCx9B, CRCx9C, CRCx9D, CRCx9E, CRCx9F, 
    CRCxA0, CRCxA1, CRCxA2, CRCxA3, CRCxA4, CRCxA5, CRCxA6, CRCxA7, 
    CRCxA8, CRCxA9, CRCxAA, CRCxAB, CRCxAC, CRCxAD, CRCxAE, CRCxAF, 
    CRCxB0, CRCxB1, CRCxB2, CRCxB3, CRCxB4, CRCxB5, CRCxB6, CRCxB7, 
    CRCxB8, CRCxB9, CRCxBA, CRCxBB, CRCxBC, CRCxBD, CRCxBE, CRCxBF, 
    CRCxC0, CRCxC1, CRCxC2, CRCxC3, CRCxC4, CRCxC5, CRCxC6, CRCxC7, 
    CRCxC8, CRCxC9, CRCxCA, CRCxCB, CRCxCC, CRCxCD, CRCxCE, CRCxCF, 
    CRCxD0, CRCxD1, CRCxD2, CRCxD3, CRCxD4, CRCxD5, CRCxD6, CRCxD7, 
    CRCxD8, CRCxD9, CRCxDA, CRCxDB, CRCxDC, CRCxDD, CRCxDE, CRCxDF, 
    CRCxE0, CRCxE1, CRCxE2, CRCxE3, CRCxE4, CRCxE5, CRCxE6, CRCxE7, 
    CRCxE8, CRCxE9, CRCxEA, CRCxEB, CRCxEC, CRCxED, CRCxEE, CRCxEF, 
    CRCxF0, CRCxF1, CRCxF2, CRCxF3, CRCxF4, CRCxF5, CRCxF6, CRCxF7, 
    CRCxF8, CRCxF9, CRCxFA, CRCxFB, CRCxFC, CRCxFD, CRCxFE, CRCxFF
}; 




/** Platform Default CRC Routine <BR>
  * ========================================================================<BR>
  * Uses the CRC table from the CRC module because STM32L1 does not have a 
  * standalone CRC16 peripheral.  The standard we use is the CCITT 0x1021 poly,
  * initialized with 0xFFFF.  Text string "123456789" yields 0x29B1.
  */
#ifndef EXTF_platform_crc_init
ot_u16 platform_crc_init() {
    platform_ext.crc16 = 0xFFFF;
}
#endif


#ifndef EXTF_platform_crc_block
ot_u16 platform_crc_block(ot_u8* block_addr, ot_int block_size) {
    ot_u16  crc_val;
    ot_u8   index;
    while (--block_size >= 0) {
        index   = ((ot_u8*)&crc_val)[UPPER] ^ *block_addr++;                    //((crc_val>>8) & 0xff) ^ *block_addr++;
        crc_val = (crc_val<<8) ^ crc16_table[index];
    }
}
#endif


#ifndef EXTF_platform_crc_byte
OT_INLINE void platform_crc_byte(ot_u8 databyte) {

    /// Option 1: implementation without table
//    ot_u16 crc;
//    crc     = ((ot_u8*)&platform_ext.crc16)[UPPER] ^ databyte;
//    crc    ^= crc >> 4;
//    crc     = (platform_ext.crc16 << 8) ^ (crc << 12) ^ (crc << 5) ^ crc;
//    platform_ext.crc16 = crc;
    
    /// Option 2: uses table
    databyte           ^= ((ot_u8*)&platform_ext.crc16)[UPPER];                 //((platform_ext.crc16>>8) & 0xff);
    platform_ext.crc16  = (platform_ext.crc16<<8) ^ crc16_table[databyte];
}
#endif


#ifndef EXTF_platform_crc_result
ot_u16 platform_crc_result() {
    return platform_ext.crc16;
}
#endif

