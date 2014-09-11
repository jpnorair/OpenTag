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
  * @file       /otlib/logger.h
  * @author     JP Norair
  * @version    R101
  * @date       26 Aug 2014
  * @brief      OpenTag Logger functions.  Typically sent over MPipe
  * @defgroup   Logger
  * @ingroup    Logger
  *
  ******************************************************************************
  */


#ifndef __OTLIB_LOGGER_H
#define __OTLIB_LOGGER_H

#include <otstd.h>


typedef enum {
	DATA_raw 		= 0,
	DATA_utf8 		= 1,
	DATA_utf16		= 2,
	DATA_utf8hex	= 3,
	MSG_raw 		= 4,
	MSG_utf8		= 5,
	MSG_utf16		= 6,
	MSG_utf8hex		= 7
} logmsg_type;

#if (OT_FEATURE(LOGGER) == ENABLED)

/** @brief  Loads a Logger header into the output queue
  * @param  id_subcode	   (ot_u8) ALP Subcode field
  * @param  payload_length (ot_int) length of payload
  * @retval ot_bool        Returns False if the mpipe out-queue is out of space
  * @ingroup Logger
  *
  * This is for expert use, such as if you are loading formatted data into the
  * log buffer, and you don't want to double buffer.
  */
ot_bool logger_header(ot_u8 id_subcode, ot_int payload_length);



/** @brief  Log a datastream directly (no double buffering)
  * @param  None
  * @retval ot_u16  0 on failure, non-zero on non-failure
  * @ingroup Logger
  *
  * This is the fastest way to log, but the data must be already formatted into
  * logging format (for official ports, this is NDEF) and stored in the MPIPE
  * output queue.
  */
void logger_direct();



/** @brief  Format and log a generic block of data
  * @param  subcode (ot_u8) Type of data that's being logged
  * @param  length  (ot_int) length in bytes of the supplied datastream
  * @param  data    (ot_u8*) the arbitrary datastream
  * @retval ot_u16  0 on failure, non-zero on non-failure
  * @ingroup Logger
  *
  * On the subcode: 
  * The logger directive uses directive ID = 0x04.  There is also a subcode byte
  * in the directive header.  System messages and error codes have subcode = 2.  
  * 0x01 is for response logging.  You can use any other subcode, although there 
  * might be some standards that emerge.
  */
void logger(ot_u8 subcode, ot_int length, ot_u8* data);
#define logger_response(LENGTH, DATA)    logger(0x01, LENGTH, DATA)



/** @brief  Log arbitrary "message," which is type + data.
  * @param  logcmd		(logmsg_type) enumerated value of message type
  * @param  label_len   (ot_int) number of bytes/chars in the label
  * @param  data_len    (ot_int) number of bytes/chars in the data
  * @param  label       (ot_u8*) label byte array
  * @param  data        (ot_u8*) data byte array
  * @retval ot_u16      0 on failure, non-zero on non-failure
  * @ingroup Logger
  * @sa logger_msg
  *
  * Similar to logger_raw() except that a label string is applied to the 
  * datastream prior to the data payload.  The label string is zero-terminated,
  * so it should take on ASCII values or similar.
  */
void logger_msg(logmsg_type logcmd, ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data);


/** @brief  Identical to logger_msg, but converts data into ASCII Hex
  * @param  label_len   (ot_int) number of bytes/chars in the label
  * @param  data_len    (ot_int) number of bytes/chars in the data
  * @param  label       (ot_u8*) label byte array
  * @param  data        (ot_u8*) data byte array
  * @retval ot_u16      0 on failure, non-zero on non-failure
  * @ingroup Logger
  * @sa logger_msg
  * 
  * The hex output is in capital letters.  One byte of data is converted into
  * three bytes of hex output -- two characters and a space -- therefore the
  * output buffer must be oversized in order for this function to print out
  * a full, 255 byte data frame.
  *
  * @note logger_hexmsg is intended for usage with debugging builds or to push
  * messages to clients that do not have message interpreters (namely, binary
  * to hex conversion on the client side).  Needless to say, it is a better
  * use of resources to do any kind of message interpreting on the cilent side.
  */
void logger_hexmsg(ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data);



/** @brief  Log a system code, such as an error code
  * @param  label_len   (ot_int) number of bytes/chars in the label
  * @param  label       (ot_u8*) label byte array
  * @param  code        (ot_u16) an integer code
  * @retval ot_u16      0 on failure, non-zero on non-failure
  * @ingroup Logger
  *
  * Similar to logger_msg() and, by extension, logger_raw(). Instead of a
  * byte datastream, though, the data payload consists of a two-byte code value,
  * which is translated to ASCII Hex on output
  *
  * @note parsing the payload on input via stdio.h can look like:
  *       scanf("%s %04X", &label_in, &code_in);
  */
void logger_code(ot_int label_len, ot_u8* label, ot_u16 code);

#endif


#endif


