/*  Copyright 2011 JP Norair
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
  * @file       /otlib/utils.c
  * @author     JP Norair
  * @version    R101
  * @date       1 Aug 2013
  * @brief      Utility functions
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otlib/utils.h>
#include <otlib/memcpy.h>


// Null Signals
void otutils_null(void)                     { }
void otutils_sig_null(ot_int a)             { }
void otutils_sig2_null(ot_int a, ot_int b)  { }
void otutils_sigv_null(void* a)             { }



// Constant arrays used within
const ot_u8 otutils_hexlut[16] = "0123456789ABCDEF";




///@todo alias this in m2dll.h
#ifndef EXTF_otutils_calc_timeout
ot_ulong otutils_calc_timeout(ot_u8 timeout_code) {
    ot_u8       shift;
    ot_ulong    timeout;
    shift       = (timeout_code >> 3) & 0x0E;
    timeout     = (timeout_code & 0x0F);
    timeout    += (shift != 0);
    timeout    *= ((ot_u16)1 << shift);
    
    return timeout;
}
#endif



// Exp-Mantissa expansion for common 7-bit field
///@todo alias this in m2dll.h
#ifndef EXTF_otutils_encode_timeout
ot_u8 otutils_encode_timeout(ot_u16 timeout_ticks) {
    ot_u8 exp;
    
    if (timeout_ticks < 16) {
        return (ot_u8)timeout_ticks;
    }
    
    exp = 1;
    timeout_ticks >>= 2;
    
    while (timeout_ticks > 16) {
        exp += 2;
        timeout_ticks >>= 4;
    }
    timeout_ticks--;
    
    return (exp << 4) + (ot_u8)timeout_ticks;
}
#endif





#ifndef EXTF_otutils_byte2hex
ot_u16 otutils_byte2hex(ot_u8 input) {
    ot_u8 out[2];
    out[1]  = otutils_hexlut[(input & 0xF)];
    out[0]  = otutils_hexlut[(input >> 4)];
    
    return *((ot_u16*)out);
}
#endif


// Binary data to hex-text
#ifndef EXTF_otutils_bin2hex
ot_int otutils_bin2hex(ot_u8* dst, ot_u8* src, ot_int size) {
    //ot_u8* src_end;
    //src_end = src + size;
    ot_int bytes_out = size << 1;
    
    //while (src != src_end) {
    while (--size >= 0) {
        *dst++  = otutils_hexlut[(*src >> 4)];
        *dst++  = otutils_hexlut[(*src & 0x0F)];
        src++;
    }
    
    return bytes_out;
}
#endif


#ifndef EXTF_otutils_int2dec
ot_int otutils_int2dec(ot_u8* dst, ot_int data) {
    ot_u8*  dst_start;
    ot_bool force;
    ot_int  divider;
    ot_int  digit;

    dst_start = dst;

    *dst++  = ' ';  //delimiter

    if (data < 0) {
        data    = 0 - data;
        *dst++  = '-';
    }

    for (divider=10000, force=False; divider!=0; divider/=10) {
        digit = (data/divider);
        if (digit | force) {
            force   = True;
            *dst++  = digit + '0';
            data   -= digit*divider;
        }
    }

    return (ot_int)(dst - dst_start);
}
#endif


#ifndef EXTF_otutils_long2dec
ot_int otutils_long2dec(ot_u8* dst, ot_long data) {
    ot_u8*  dst_start;
    ot_bool force;
    ot_long  divider;
    ot_u8  digit;

    dst_start = dst;

    *dst++  = ' ';  //delimiter

    if (data < 0) {
        data    = 0 - data;
        *dst++  = '-';
    }

    for (divider=1000000000, force=False; divider!=0; divider/=10) {
        digit = (data/divider);
        if (digit | force) {
            force   = True;
            *dst++  = digit + '0';
            data   -= digit*divider;
        }
    }

    return (ot_int)(dst - dst_start);
}
#endif


ot_int slistf(ot_u8* dst, const char* label, char format, ot_u8 number, ot_u8* src) {
/// @note: this function is designed to use a big-endian input string.
    ot_u8* scratch;
    ot_u8* dst_start;

    dst_start   = dst;
    scratch     = (ot_u8*)label;
    while (*scratch != 0) {
        *dst++ = *scratch++;
    }

    // Hex format
    if (format == 'x') {
        dst += otutils_bin2hex(dst, (ot_u8*)src, number);
        goto slistf_END;
    }

    // byte or short format (raw integer data)
    if ((format == 'b') || (format == 's')) {
        ot_uni16 value;
        while (number > 0) {
            number--;
            if (format == 'b') {
                value.sshort = (ot_int)*((ot_s8*)src);
            }
            else {
                value.ubyte[UPPER] = *src++;
                value.ubyte[LOWER] = *src;
            }
            src++;
            dst += otutils_int2dec(dst, value.sshort);
        }
    }

    // No formatting (text)
    else {
        *dst++ = ' ';
        memcpy(dst, src, number);
        dst += number;
    }

    slistf_END:
    return (ot_int)(dst - dst_start);
}



