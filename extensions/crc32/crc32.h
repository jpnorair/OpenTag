/* Copyright 2014 JP Norair
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

