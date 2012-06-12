/* Copyright 2009-2011 JP Norair
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
  * @file       /OTlib/crc16.c
  * @author     JP Norair
  * @version    V1.0
  * @date       14 November 2011
  * @brief      Implements block and iterative calculation methods for CRC as
  *             described in the ISO 18000-7 standard document.
  * @ingroup    CRC16
  *
  ******************************************************************************
  */


#include "crc16.h"
#include "OT_platform.h"
#include "OT_utils.h"



#if (MCU_FEATURE(CRC) == DISABLED)
//#   error Warning: Comment this line if you want to enable the CRC Table
#   include "crc16_table.h"


    /** @var crc_table
      * @ingroup CRC16
      *
      * The CRC16 used here is the CCITT variant that yields 29BF from ASCII
      * input "123456789"
      */
        const ot_u16 crc_table[256] = { 
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
#endif



#if (MCU_FEATURE(CRC) == ENABLED)

/* Note for the future: implementing CRC on CC430 HW, in asm

mov     #0FFFFh,&CRCINIRES      ; initialize CRC
mov.b   #000xxh,&CRCDIRB_L      ; send in byte
...
cmp     #029B1h,&CRCINIRES      ; Check results
*/

#endif



/** @note CRCBASE, CRCPOLY
  * These make the "magic number" to use when starting up CRC, written to the
  * crc.value variable during init_crc.value.  They are not currently used,
  * having been substituted for the somewhat optimized tabular approach.  If
  * program memory becomes a problem, a bitwise CRC routine may be used in place
  * of the tabular method.
  */
#ifndef CRCBASE
#define CRCBASE    0xFFFF
#define CRCBASE_LO 0xFF
#define CRCBASE_HI 0xFF
#endif

#ifndef CRCPOLY
// 18000-7 & 18185 CRC polynomial is: x^16 + x^12 + x^5 + x^0
// 16th bit is implicit
#define CRCPOLY    0x1021
#define CRCPOLY_LO 0x21
#define CRCPOLY_HI 0x10
#endif




crc_struct crc;





#if (MCU_FEATURE(CRC) == DISABLED)
/**
  * @brief Internal function that handles the lookup table.  The input byte corresponds
  *        uniquely to a 16 bit value.
  * @param input : a (ot_u8) corresponding to byte to lookup
  * @retval ot_u16 : the 16 bit CRC16 value for the input byte
  * @ingroup CRC16
*/
//ot_u16 sub_lookup(ot_u8 input) {   return crc_table[input];    }


void sub_calc_byte() {
#if (1)
    crc.val = (crc.val << 8) ^ crc_table[((crc.val >> 8) & 0xff) ^ *crc.cursor++];
        
#else
    //crcv     = (ot_u8)(crcv >> 8) | (crcv << 8);
    //crcv    ^= (ot_u16)*block_addr;
    //crcv    ^= (ot_u8)(crcv & 0xff) >> 4;
    //crcv    ^= (crcv << 8) << 4;
    //crcv    ^= ((crcv & 0xff) << 4) << 1;

    //crcv = (ot_u8)(crcv >> 8) | (crcv << 8);
    //crcv ^= (ot_u16)*block_addr;
    //crcv ^= (ot_u8)(crcv & 0xff) >> 4;
    //crcv ^= crcv << 12;
    //crcv ^= (crcv & 0xff) << 5;
    
    // Known reference code (slow)
    ot_u16 j, c, bit;

    c = (ot_u16)*crc.cursor++;

    for (j=0x80; j; j>>=1) {

        bit         = crc.val & 0x8000;
        crc.val   <<= 1;
        if (c & j) 
            bit ^= 0x8000;
        if (bit)
            crc.val ^= 0x1021;
    }
        
#endif
}

#endif



void sub_stream2() {
    *crc.end 	= (ot_u8)crc.val;
    crc.stream 	= &otutils_null;
}

void sub_stream1() {
    *crc.end    = (ot_u8)(crc.val >> 8);
    crc.end++;
    crc.stream  = &sub_stream2;
}

void sub_stream0() {
#if (MCU_FEATURE(CRC) == ENABLED)
    platform_crc_byte( *crc.cursor++ );
    if (crc.cursor == crc.end) {
        crc.val    = platform_crc_result();
        crc.stream = &sub_stream1;
    }
#else
    sub_calc_byte();
    if (crc.cursor == crc.end) {
        crc.stream = &sub_stream1;
        //crc.end[-2] = crc.val[UPPER];
        //crc.end[-1] = crc.value.ubyte[LOWER];
    }
#endif
}





ot_u16 crc_calc_block(ot_int block_size, ot_u8* block_addr) {
/// In this function, you can see the difference between the table-based and
/// optimized, non-table-based computation methods.  By my estimation, the
/// table-based method is about twice as fast.

#if (MCU_FEATURE(CRC) == ENABLED)
    platform_crc_init();
    crc.cursor = block_addr;
    crc.end    = block_addr + block_size;
    crc.val    = platform_crc_block(block_addr, block_size);
    
#else
    crc.cursor = block_addr;
    //crc.end    = block_addr + block_size;
    crc.val    = CRCBASE;

    while (block_size > 0) {
        sub_calc_byte();
        block_size--;
    }
    
#endif

    return crc.val;
}




void crc_init_stream(ot_int stream_size, ot_u8* stream_addr) {
#if (MCU_FEATURE(CRC) == ENABLED)
    crc.cursor  = stream_addr;
    crc.end     = stream_addr + stream_size;
    crc.stream  = &sub_stream0;
    crc.val     = platform_crc_init();

#else
    crc.cursor  = stream_addr;
    crc.end     = stream_addr + stream_size;
    crc.stream  = &sub_stream0;
    crc.val     = CRCBASE;
#endif
}





void crc_calc_stream() {
    crc.stream();
}



void crc_update_stream(ot_u8* new_end) {
    crc.end = new_end;
}



ot_bool crc_check() {
    return (ot_bool)(crc.val == 0);
}



ot_u16 crc_get() {
    return crc.val;
}








