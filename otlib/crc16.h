/* Copyright 2010-2011 JP Norair
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
  * @file       OTlib/crc16.h
  * @author     JP Norair
  * @version    V1.0
  * @date       7 Jan 2011
  * @brief      CRC16 implementation
  * @defgroup   CRC16 (CRC16 Module)
  * @ingroup    CRC16
  *
  * 
  ******************************************************************************
  */

#ifndef __CRC16_H
#define __CRC16_H

#include "OT_config.h"
#include "OT_types.h"



/** @typedef crc_struct
  * @ingroup CRC16
  * Iterative storage of CRC16 value and cursor
  */
typedef struct {
    ot_u8*      cursor;
    ot_u8*      end;
    ot_u16      val;
    void        (*stream)();
} crc_struct;

extern crc_struct crc;


extern const ot_u16 crc_table[256];



/** @brief Calculates the CRC16 for a complete data block
  * @param block_size : (ot_int) number of bytes in the data block
  * @param block_addr : (ot_u8*) address of the data block
  * @retval ot_u16 : CRC16 value
  * @ingroup CRC16
  */
ot_u16 crc_calc_block(ot_int block_size, ot_u8 *block_addr);





/** @brief Initializes streaming CRC16 engine
  * @param stream_size  (ot_int) length of datastream
  * @param stream_addr  (ot_u8*) pointer to start of datastream
  * @retval None
  * @ingroup CRC16
  * This must be run prior to engaging a streaming CRC calculation.  Only one
  * streaming CRC object may exist, so right now it's impossible to do streaming
  * CRC on parallel/concurrent data-streams.  init_crc_stream will zero the
  * crc_partial global variable before operating.
  *
  * @note Make sure to set stream_size to 255 (or whatever is your max stream)
  *       if you are using a type of datastream that has variable length.  Use
  *       crc_update_stream() to change to the correct stream length once you
  *       find out what it is.
  */
void crc_init_stream(ot_int stream_size, ot_u8* stream_addr);



/** @brief Calculates the CRC from the stream, and eventually writes it.
  * @param None
  * @retval None
  * @ingroup CRC16
  *
  * This is state-based.  When the stream is done, the value of the CRC 
  * calculated from the stream data will be inserted onto the end of the stream
  * automatically.  If you keep calling it forever, it will keep writing the
  * CRC to the end of the stream.  It will not go off and start doing weird
  * shit to nearby data.
  */
void crc_calc_stream();



/** @brief Updates the end of the CRC stream
  * @param new_end      (ot_u8*) new end pointer for crc stream
  * @retval None
  * @ingroup CRC16
  *
  * Useful for receiving datastreams where the stream length is not known ahead 
  * of decoding.  Use this function to tell the CRC streamer where the end of
  * the stream is, once you know where it is.
  */
void crc_update_stream(ot_u8* new_end);



/** @brief Checks the CRC in the stream against the known, good value.
  * @param None
  * @retval ot_bool : TRUE if CRC passes check. 
  * @ingroup CRC16
  */
ot_bool crc_check();





/** @brief Gets the CRC value, as calculated by the most recent CRC operation
  * @param none
  * @retval ot_u16      The CRC value
  * @ingroup CRC16
  */
ot_u16 crc_get();




#endif

