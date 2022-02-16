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
  * @file       /otlib/crc16.h
  * @author     JP Norair
  * @version    R101
  * @date       31 Jan 2013
  * @brief      A streaming object for CRC16
  * @defgroup   CRC16 (CRC16 Module)
  * @ingroup    CRC16
  *
  * CRC16 Module implements a streaming, iterative CRC16 calculation and check
  * apparatus.  The nominal algorithm is CRC16-CCITT with FFFF initialization.
  *
  * You could alter this module to be more like an object, such that the crc
  * datatype is passed into the functions to identify the stream.  For now,
  * that is not important because it only is used for RF.
  ******************************************************************************
  */

#ifndef __CRC16_H
#define __CRC16_H

#include <otstd.h>



/** @typedef crc_struct
  * @ingroup CRC16
  * Iterative storage of CRC16 value and cursor
  */
typedef struct {
    ot_u8*      cursor;
    ot_int      count;
    ot_bool     writeout;
    ot_u16      val;
} crcstream_t;


// This is a legacy, modular element
//extern crcstream_t crcstream;







/** @brief Initializes streaming CRC16 engine
  * @param stream       (crcstream_t*) allocated crcstream "object"
  * @param writeout     (ot_bool) True/False if CRC should be appended onto stream
  * @param size         (ot_int) Number of total bytes in stream
  * @param doto         (ot_u8*) pointer to the data getting streamed for CRC 
  * @retval None
  * @ingroup CRC16
  *
  * This must be run prior to engaging a streaming CRC calculation.  Only one
  * streaming CRC object may exist, so right now it's impossible to do streaming
  * CRC on parallel/concurrent data-streams.  init_crc_stream will zero the
  * crc_partial global variable before operating.
  */
void crc_init_stream(crcstream_t* stream, ot_bool writeout, ot_int size, ot_u8* data);



/** @brief Calculates a CRC, one-time, for a block of data
  * @param stream       (crcstream_t*) allocated crcstream "object"
  * @param writeout     (ot_bool) True/False if CRC should be appended onto stream
  * @param size         (ot_int) Number of total bytes in stream
  * @param doto         (ot_u8*) pointer to the data getting streamed for CRC 
  * @retval ot_u16      generated CRC16 value
  * @ingroup CRC16
  * @sa crc_init_stream()
  * 
  */
ot_u16 crc_block(crcstream_t* stream, ot_bool writeout, ot_int size, ot_u8* data);




/** @brief Calculates the CRC from the stream, and eventually writes it.
  * @param stream       (crcstream_t*) allocated crcstream "object"
  * @retval None
  * @ingroup CRC16
  * @sa crc_calc_nstream()
  * @sa crc_init_stream()
  *
  * This is state-based.  When the stream is done, the value of the CRC
  * calculated from the stream data will be inserted onto the end of the stream
  * automatically.  If you keep calling it forever, it will keep writing the
  * CRC to the end of the stream, and it will not overwrite subsequent data.
  */
void crc_calc_stream(crcstream_t* stream);




/** @brief Performs a stream CRC calculation on more than one byte
  * @param stream       (crcstream_t*) allocated crcstream "object"
  * @param n            (ot_u16) number of new stream bytes, must be positive
  * @retval None
  * @ingroup CRC16
  * @sa crc_calc_stream()
  * @sa crc_init_stream()
  */
void crc_calc_nstream(crcstream_t* stream, ot_u16 n);




/** @brief Checks the CRC in the stream against the known, good value.
  * @param stream       (crcstream_t*) allocated crcstream "object"
  * @retval ot_bool     TRUE if CRC passes check.
  * @ingroup CRC16
  */
ot_bool crc_check(crcstream_t* stream);





/** @brief Gets the CRC value, as calculated by the most recent CRC operation
  * @param stream       (crcstream_t*) allocated crcstream "object"
  * @retval ot_u16      The CRC value
  * @ingroup CRC16
  */
ot_u16 crc_get(crcstream_t* stream);








/** CRC16 driver functions
  * ========================================================================<BR>
  * Typically implemented in the platform drivers
  */
  
ot_u16 crc16drv_init();
ot_u16 crc16drv_block(ot_u8* block_addr, ot_int block_size);
ot_u16 crc16drv_block_manual(ot_u8* block_addr, ot_int block_size, ot_u16 init);

// Obsolete
//void crc16drv_byte(ot_u8 databyte);
//ot_u16 crc16drv_result();




#endif

