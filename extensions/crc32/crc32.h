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
  * @file       /otlibext/ext/crc32.h
  * @author     JP Norair
  * @version    R100
  * @date       21 Jan 2014
  * @brief      32bit word-wise CRC32 library
  * @defgroup   CRC32
  * @ingroup    CRC32
  *
  * CRC32 is not required for DASH7, but some application protocols may use it,
  * and certainly ethernet uses it.  Yes, Virginia, there is going to be formal
  * support for IP and Ethernet in OpenTag, during 2014.
  *
  * Perhaps it need not be said, but the CRC32 implementation is the one 
  * compliant with the Ethernet spec.  This is far from the best performing
  * CRC32 checksum, but it is what we are stuck with.
  *
  ******************************************************************************
  */


#include <otsys/config.h>



ot_u32 crc32_calc_block(ot_u8* block_addr, ot_uint block_words);


void crc32_init_stream(ot_bool writeout, ot_uint stream_words, ot_u8* stream);


void crc32_calc_stream();


void crc32_calc_nstream(ot_uint n_words);


ot_bool crc32_check();


ot_u16 crc32_get();

