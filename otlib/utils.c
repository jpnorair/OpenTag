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





#ifndef EXTF_otutils_calc_longtimeout
ot_ulong otutils_calc_longtimeout(ot_u16 timeout_code) {
	ot_uint 	shift;
	ot_ulong    timeout;
	shift       = (timeout_code >> 8) & 0x001C;
	timeout     = (timeout_code & 0x3FF);
	timeout    += (shift != 0);
	timeout    *= (1 << shift);
	return timeout;
}
#endif

#ifndef EXTF_otutils_encode_longtimeout
ot_u16 otutils_encode_longtimeout(ot_ulong timeout_ticks) {
    ot_uint exp;

	if (timeout_ticks < 0x0400) {
		return (ot_u16)timeout_ticks;
	}

	exp 			= 1;
	timeout_ticks >>= 3;

	while (timeout_ticks > 0x0400) {
		exp += 1;
		timeout_ticks >>= 3;
	}
	timeout_ticks--;

	return (exp << 10) + (ot_u8)timeout_ticks;
}
#endif

#ifndef EXTF_otutils_calc_timeout
ot_ulong otutils_calc_timeout(ot_u8 timeout_code) {
	ot_uint		shift;
	ot_ulong    timeout;
	shift       = (timeout_code >> 3) & 0x000E;
	timeout     = (timeout_code & 0x0E);
	timeout    += (shift != 0);
	timeout    *= (1 << shift);
	return timeout;
}
#endif

#ifndef EXTF_otutils_encode_timeout
ot_u8 otutils_encode_timeout(ot_u16 timeout_ticks) {
	ot_uint exp;

	if (timeout_ticks < 0x10) {
		return (ot_u8)timeout_ticks;
	}

	exp 			= 1;
	timeout_ticks >>= 2;

	while (timeout_ticks > 0x10) {
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



