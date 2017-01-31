/* Copyright 2010-2014 JP Norair
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
  * @file       /otlib/crc16.c
  * @author     JP Norair
  * @version    R102
  * @date       12 Jan 2014
  * @brief      A streaming object for CRC16
  * @ingroup    CRC16
  *
  * The primary CRC routines are actually implemented in the platform module,
  * either as software or hardware based methods, and this module acts as a
  * wrapper and datatype.
  ******************************************************************************
  */


#include <otstd.h>
#include <platform/config.h>
#include <otlib/crc16.h>
#include <otlib/utils.h>



//crc_struct crc;


void crc_init_stream(crcstream_t* stream, ot_bool writeout, ot_int size, ot_u8* data) {
    stream->writeout    = writeout;
    stream->cursor      = data;
    stream->count       = size;
    stream->val         = crc16drv_init();
}


ot_u16 crc_block(crcstream_t* stream, ot_bool writeout, ot_int size, ot_u8* data) {
    crc_init_stream(stream, writeout, size, data);
    crc_calc_nstream(stream, size);
    return stream->val;
}


OT_INLINE void crc_calc_stream(crcstream_t* stream) {
    crc_calc_nstream(stream, 1);
}


void crc_calc_nstream(crcstream_t* stream, ot_u16 n) {
    if (stream->count > 0) {
        ot_u8* data;
        if (n > stream->count) {
            n = stream->count;
        }
        stream->count  -= n;
        data            = stream->cursor;
        stream->cursor += n;
        stream->val     = crc16drv_block_manual(data, n, stream->val);
    }
    if ((stream->count == 0) && (stream->writeout)) {
        stream->writeout    = False;
        *stream->cursor++   = (ot_u8)(stream->val >> 8);
        *stream->cursor++   = (ot_u8)(stream->val);
    }
}



ot_bool crc_check(crcstream_t* stream) {
///@todo deprecate this function in OT, in favor of crc_get(), and checking with 0.
    return (stream->val == 0);
    //return (crc16drv_result() == 0);
}


ot_u16 crc_get(crcstream_t* stream) {
    return stream->val;
    //return crc16drv_result();
}







/** Software-based CRC16 driver <BR>
  * ========================================================================<BR>
  * Platforms without HW CRC16 generators can use this.  If your platform does
  * have special HW for CRC, or possibly you just have some optimized assembly,
  * then make sure the platform configuration files set:
  * #define MCU_FEATURE_CRC16   ENABLED
  *
  * And you can put the optimized crc16drv_... functions in the platform
  * directory of your chip.
  */

#if (MCU_FEATURE(CRC16) != ENABLED)
#include <otlib/crc16_table.h>

//ot_u16 crc16_dr;



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


#ifndef EXTF_crc16drv_init
OT_INLINE ot_u16 crc16drv_init() {
    //crc16_dr = 0xFFFF;
    return 0xFFFF;
}
#endif


#ifndef EXTF_crc16drv_block_manual
OT_WEAK ot_u16 crc16drv_block_manual(ot_u8* block_addr, ot_int block_size, ot_u16 init) {
/// One nice thing about the software-only CRC16 method is that it doesn't
/// require any sort of interrupt blocking.  In practice, interrupt blocking
/// by CRC isn't much of an issue, but if you are getting weird CRC errors
/// when using streams, try comparing against this software implemetation.

    while (--block_size >= 0) {
        ot_u8 index = ((ot_u8*)&init)[UPPER] ^ *block_addr++;       //((crc_val>>8) & 0xff) ^ *block_addr++;
        init        = (init<<8) ^ crc16_table[index];
    }
    return init;
}
#endif


#ifndef EXTF_crc16drv_block
OT_INLINE ot_u16 crc16drv_block(ot_u8* block_addr, ot_int block_size) {
    return crc16drv_block_manual(block_addr, block_size, 0xFFFF);
}
#endif


//#ifndef EXTF_crc16drv_byte
//OT_INLINE void crc16drv_byte(ot_u8 databyte) {
//#if 0     /// Option 1: implementation without table
//    ot_u16 crc;
//    crc     = ((ot_u8*)&crc16_dr)[UPPER] ^ databyte;
//    crc    ^= crc >> 4;
//    crc     = (crc16_dr << 8) ^ (crc << 12) ^ (crc << 5) ^ crc;
//    crc16_dr = crc;
//
//#else     /// Option 2: uses table
//    databyte   ^= ((ot_u8*)&crc16_dr)[UPPER];                 //((crc16_dr>>8) & 0xff);
//    crc16_dr    = (crc16_dr<<8) ^ crc16_table[databyte];
//#endif
//}
//#endif


//#ifndef EXTF_crc16drv_result
//OT_INLINE ot_u16 crc16drv_result() {
//    return crc16_dr;
//}
//#endif


#endif



