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


#include "crc16.h"
#include "OT_platform.h"
#include "OT_utils.h"




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


#if (0)
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



//void sub_stream2() {
//    *crc.cursor++   = (ot_u8)crc.val;
//    crc.stream 	    = &otutils_null;
//}

//void sub_stream1() {
//    *crc.cursor++   = (ot_u8)(crc.val >> 8);
//    crc.stream      = &sub_stream2;
//}


//#if (CC_SUPPORT == GCC)
//void sub_stream0() __attribute__((flatten));
//#endif

//void sub_stream0() {
//    platform_crc_byte( *crc.cursor++ );
//    if (crc.cursor == crc.end) {
//        crc.val    = platform_crc_result();
//        crc.stream = &sub_stream1;
//    }
//}




void crc_init_stream(ot_bool writeout, ot_int stream_size, ot_u8* stream) {
    crc.writeout= writeout;
    crc.cursor  = stream;
    crc.count   = stream_size+1;
  //crc.stream  = &sub_stream0;
    crc.val     = platform_crc_init();
}


void crc_calc_stream() {
    //crc.stream();
    if (--crc.count > 0) {
        platform_crc_byte( *crc.cursor++ );
    }
    else if ((crc.writeout) && (crc.count > -2)) {
        *crc.cursor++ = (ot_u8)(crc.val >> ((crc.count == 0) << 3));
    }
    
}


void crc_calc_nstream(ot_u16 n) {
    do {
        //crc.stream();
        crc_calc_stream();
    } while (--n);
}



ot_bool crc_check() {
///@todo deprecate this function in OT, in favor of crc_get(), and checking with 0.
    return (ot_bool)(crc.val == 0);
}


ot_u16 crc_get() {
    return crc.val;
}



