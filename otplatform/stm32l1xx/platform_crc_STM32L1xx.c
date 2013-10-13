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



static const ot_u8 crc8_table[256] = {
    CRC8x00, CRC8x01, CRC8x02, CRC8x03, CRC8x04, CRC8x05, CRC8x06, CRC8x07, 
    CRC8x08, CRC8x09, CRC8x0A, CRC8x0B, CRC8x0C, CRC8x0D, CRC8x0E, CRC8x0F, 
    CRC8x10, CRC8x11, CRC8x12, CRC8x13, CRC8x14, CRC8x15, CRC8x16, CRC8x17, 
    CRC8x18, CRC8x19, CRC8x1A, CRC8x1B, CRC8x1C, CRC8x1D, CRC8x1E, CRC8x1F, 
    CRC8x20, CRC8x21, CRC8x22, CRC8x23, CRC8x24, CRC8x25, CRC8x26, CRC8x27, 
    CRC8x28, CRC8x29, CRC8x2A, CRC8x2B, CRC8x2C, CRC8x2D, CRC8x2E, CRC8x2F, 
    CRC8x30, CRC8x31, CRC8x32, CRC8x33, CRC8x34, CRC8x35, CRC8x36, CRC8x37, 
    CRC8x38, CRC8x39, CRC8x3A, CRC8x3B, CRC8x3C, CRC8x3D, CRC8x3E, CRC8x3F, 
    CRC8x40, CRC8x41, CRC8x42, CRC8x43, CRC8x44, CRC8x45, CRC8x46, CRC8x47, 
    CRC8x48, CRC8x49, CRC8x4A, CRC8x4B, CRC8x4C, CRC8x4D, CRC8x4E, CRC8x4F, 
    CRC8x50, CRC8x51, CRC8x52, CRC8x53, CRC8x54, CRC8x55, CRC8x56, CRC8x57, 
    CRC8x58, CRC8x59, CRC8x5A, CRC8x5B, CRC8x5C, CRC8x5D, CRC8x5E, CRC8x5F, 
    CRC8x60, CRC8x61, CRC8x62, CRC8x63, CRC8x64, CRC8x65, CRC8x66, CRC8x67, 
    CRC8x68, CRC8x69, CRC8x6A, CRC8x6B, CRC8x6C, CRC8x6D, CRC8x6E, CRC8x6F, 
    CRC8x70, CRC8x71, CRC8x72, CRC8x73, CRC8x74, CRC8x75, CRC8x76, CRC8x77, 
    CRC8x78, CRC8x79, CRC8x7A, CRC8x7B, CRC8x7C, CRC8x7D, CRC8x7E, CRC8x7F, 
    CRC8x80, CRC8x81, CRC8x82, CRC8x83, CRC8x84, CRC8x85, CRC8x86, CRC8x87, 
    CRC8x88, CRC8x89, CRC8x8A, CRC8x8B, CRC8x8C, CRC8x8D, CRC8x8E, CRC8x8F, 
    CRC8x90, CRC8x91, CRC8x92, CRC8x93, CRC8x94, CRC8x95, CRC8x96, CRC8x97, 
    CRC8x98, CRC8x99, CRC8x9A, CRC8x9B, CRC8x9C, CRC8x9D, CRC8x9E, CRC8x9F, 
    CRC8xA0, CRC8xA1, CRC8xA2, CRC8xA3, CRC8xA4, CRC8xA5, CRC8xA6, CRC8xA7, 
    CRC8xA8, CRC8xA9, CRC8xAA, CRC8xAB, CRC8xAC, CRC8xAD, CRC8xAE, CRC8xAF, 
    CRC8xB0, CRC8xB1, CRC8xB2, CRC8xB3, CRC8xB4, CRC8xB5, CRC8xB6, CRC8xB7, 
    CRC8xB8, CRC8xB9, CRC8xBA, CRC8xBB, CRC8xBC, CRC8xBD, CRC8xBE, CRC8xBF, 
    CRC8xC0, CRC8xC1, CRC8xC2, CRC8xC3, CRC8xC4, CRC8xC5, CRC8xC6, CRC8xC7, 
    CRC8xC8, CRC8xC9, CRC8xCA, CRC8xCB, CRC8xCC, CRC8xCD, CRC8xCE, CRC8xCF, 
    CRC8xD0, CRC8xD1, CRC8xD2, CRC8xD3, CRC8xD4, CRC8xD5, CRC8xD6, CRC8xD7, 
    CRC8xD8, CRC8xD9, CRC8xDA, CRC8xDB, CRC8xDC, CRC8xDD, CRC8xDE, CRC8xDF, 
    CRC8xE0, CRC8xE1, CRC8xE2, CRC8xE3, CRC8xE4, CRC8xE5, CRC8xE6, CRC8xE7, 
    CRC8xE8, CRC8xE9, CRC8xEA, CRC8xEB, CRC8xEC, CRC8xED, CRC8xEE, CRC8xEF, 
    CRC8xF0, CRC8xF1, CRC8xF2, CRC8xF3, CRC8xF4, CRC8xF5, CRC8xF6, CRC8xF7, 
    CRC8xF8, CRC8xF9, CRC8xFA, CRC8xFB, CRC8xFC, CRC8xFD, CRC8xFE, CRC8xFF
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
    return 0xFFFF;
}
#endif


#ifndef EXTF_platform_crc8_block
ot_u8 platform_crc8_block(ot_u8* block_addr, ot_int block_size) {
    ot_u8 crc_val = 0xff;
    ot_u8 index;
    while (--block_size >= 0) {
        index   = crc_val ^ *block_addr++;
        crc_val^= crc8_table[index];
    }
    return crc_val;
}
#endif


#ifndef EXTF_platform_crc_block
ot_u16 platform_crc_block(ot_u8* block_addr, ot_int block_size) {
    ot_u16  crc_val = 0xffff;
    ot_u8   index;
    while (--block_size >= 0) {
        index   = ((ot_u8*)&crc_val)[UPPER] ^ *block_addr++;                    //((crc_val>>8) & 0xff) ^ *block_addr++;
        crc_val = (crc_val<<8) ^ crc16_table[index];
    }
    return crc_val;
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
    
    ///@todo validate that the below works:
    ///CRC16_CALCBYTE(crc16_table, platform_ext.crc16, databyte);
    
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

