/* Copyright 2010-2012 JP Norair
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
  * @file       /otlib/ndef.h
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2012
  * @brief      OpenTag NDEF Interface
  * @defgroup   NDEF
  *
  * NDEF is a data wrapping format/protocol developed by the NFC Forum, and it
  * is primarily used as a client-server data wrapper for NFC devices.  OpenTag
  * also can use a subset of NDEF for client-server interfacing.  The DASH7 ALP
  * format is a streamlined version of NDEF, so the ALP module is used to 
  * process NDEF formatted data as well as Pure-ALP data.
  * 
  * Please refer to free documentation available on the NFCForum website if you
  * want to learn more about NDEF.  You can also look at the OpenTag wiki, 
  * which has a good summary of NDEF and ALP: http://wiki.indigresso.com.  
  * Additionally, there is some information below.
  *
  * <PRE>
  * OpenTag uses two types of NDEF headers, both shown below:
  * 
  * 0             8         16            24       32       40        48
  * +------------+----------+-------------+--------+--------+---------+
  * | NDEF Flags | Type Len | Payload Len | ID Len | ALP ID | ALP CMD |
  * +------------+----------+-------------+--------+--------+---------+
  * |  bitfield  |    0     |      N      |   2    |   X    |    Y    |
  * +------------+----------+-------------+--------+--------+---------+
  * Message initiation header type: flags are always 1--11101 (Unknown TNF),
  * so this header is always used as the first record in the NDEF message.
  * The N-byte payload follows the header, and then the record is over.
  *
  * 0             8         16            24
  * +------------+----------+-------------+
  * | NDEF Flags | Type Len | Payload Len |
  * +------------+----------+-------------+
  * |  bitfield  |    0     |      N      |
  * +------------+----------+-------------+
  * Message continuation header type: flags are always 0--10110 (Unchanged TNF),
  * so this header is never the first record in the NDEF message, but it is
  * always the subsequent record header in the NDEF message.  The N-byte payload
  * follows the header, and then the record is over.
  *
  * 0             8            16       24        32
  * +------------+-------------+--------+---------+
  * | ALP Flags  | Payload Len | ALP ID | ALP CMD |
  * +------------+-------------+--------+---------+
  * |  bitfield  |      N      |   X    |    Y    |
  * +------------+-------------+--------+---------+
  * Universal ALP header (for comparison): flags are always --z10000.  The 
  * NDEF chunk flag (z) is ignored, as the values of MB and ME are sufficent to 
  * implicitly determine the value of the chunk flag.
  *
  *
  * Below is the complete NDEF header spec, with OpenTag usage notes
  * 
  *   07   06   05   04   03   02   01   00        OpenTag NDEF Subset Usage
  * +----+----+----+----+----+----+----+----+  =================================
  * | MB | ME | CF | SR | IL |     TNF      |  --> 1--11101 or 0--10110
  * +----+----+----+----+----+----+----+----+
  * |      TYPE LENGTH (1 Byte: TLEN)       |  --> Always present, Value = 0
  * +----+----+----+----+----+----+----+----+
  * |  PAYLOAD LENGTH (1 or 4 Bytes: PLEN)  |  --> 1 byte on SR=1, Value = 0-255
  * +----+----+----+----+----+----+----+----+
  * |    ID LENGTH (0 or 1 Bytes: IDLEN)    |  --> Present on IL=1, Value = 2
  * +----+----+----+----+----+----+----+----+
  * |          TYPE (TLEN Bytes)            |  --> Never present
  * +----+----+----+----+----+----+----+----+
  * |           ID (IDLEN Bytes)            |  --> Present on IL=1, 2 bytes
  * +----+----+----+----+----+----+----+----+
  * |         PAYLOAD (PLEN Bytes)          |  --> 0-255 bytes
  * +----+----+----+----+----+----+----+----+
  * 
  * </PRE>
  ******************************************************************************
  */

#ifndef __NDEF_H
#define __NDEF_H

#include <otstd.h>


/** NDEF Record Control Bits 
  * In many (if not all) cases, these control bits are applied automatically by
  * the Record/Message constructor functions.
  */
#define NDEF_MB                 0x80    // Message Start bit
#define NDEF_ME                 0x40    // Message End bit
#define NDEF_CF                 0x20    // Chunk Flag
#define NDEF_SR                 0x10    // Short Record indicator
#define NDEF_IL                 0x08    // ID LENGTH field indicator


/** NDEF TNF Field Values
  * Constants and an enum to define the TNF (Type Name Field).  TNF is a 3 bit
  * id that goes in the same Message/Record byte as the NDEF Control Bits.
  *
  * OpenTag currently uses the "UNKNOWN" TNF exclusively, which means that the
  * Type field is omitted, and the client/server are implicitly programmed to
  * only deal with TNFs of this type.
  */
#define NDEF_TNF_EMPTY          0x00
#define NDEF_TNF_WELLKNOWN      0x01    // You could use this type in local context
#define NDEF_TNF_MEDIATYPE      0x02
#define NDEF_TNF_ABSOLUTEURI    0x03
#define NDEF_TNF_EXTERNAL       0x04    // or this type in any context
#define NDEF_TNF_UNKNOWN        0x05    // or this type when implicit knowledge is OK
#define NDEF_TNF_UNCHANGED      0x06
#define NDEF_TNF_RESERVED       0x07

typedef enum {
    TNF_Empty = 0x00,
    TNF_WellKnown,
    TNF_MediaType,
    TNF_AbsoluteURI,
    TNF_External,
    TNF_Unknown,
    TNF_Unchanged,
    TNF_Reserved
} NDEF_tnf;


#endif


