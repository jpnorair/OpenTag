/* Copyright 2010-2013 JP Norair
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

#include "OT_config.h"
#include "OT_types.h"



/** @typedef crc_struct
  * @ingroup CRC16
  * Iterative storage of CRC16 value and cursor
  */
typedef struct {
    ot_u8*      cursor;
    ot_u8*      end;
    void        (*stream)();
    ot_u16      val;
} crc_struct;

extern crc_struct crc;




/** @brief Calculates the CRC16 for a complete data block
  * @param block_size : (ot_int) number of bytes in the data block
  * @param block_addr : (ot_u8*) address of the data block
  * @retval ot_u16 : CRC16 value
  * @ingroup CRC16
  */
ot_u16 crc_calc_block(ot_int block_size, ot_u8 *block_addr);





/** @brief Initializes streaming CRC16 engine
  * @param stream_size  (ot_int) Number of bytes in stream
  * @param stream       (ot_u8*) pointer to the data for CRC calculation
  * @retval None
  * @ingroup CRC16
  *
  * This must be run prior to engaging a streaming CRC calculation.  Only one
  * streaming CRC object may exist, so right now it's impossible to do streaming
  * CRC on parallel/concurrent data-streams.  init_crc_stream will zero the
  * crc_partial global variable before operating.
  */
void crc_init_stream(ot_int stream_size, ot_u8* stream);



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

