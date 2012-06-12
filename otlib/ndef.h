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
  * @file       /OTlib/ndef.h
  * @author     JP Norair
  * @version    V1.0
  * @date       8 Jun 2011
  * @brief      OpenTag API for NDEF data format (typically used for wireline messaging)
  * @defgroup   NDEF
  *
  * Please refer to free documentation available on the NFCForum website if you
  * want to learn more about NDEF.  You probably need a basic understanding of
  * NDEF in order to use the messaging API.  There is some limited documentation
  * paraphrased below from the DASH7 Mode 2 spec and NDEF spec.
  *
  * OpenTag API Messages follow the Application Subprotocol rubric, which is
  * shown below.  This format is encapsulated in NDEF when transmitted over the
  * API Messaging Interface (usually a 1:1 wireline, like USB/Serial)
  * +----------------+----------------+----------------+----------------+
  * | Directive Len  |  Directive ID  | Directive Cmd  | Directive Data |
  * +----------------+----------------+----------------+----------------+
  * |    2 Bytes     |    1 Byte      |    1 Byte      |    N-4 Bytes   |
  * +----------------+----------------+----------------+----------------+
  * |       N        | encoded value  | encoded value  |   bytestream   |
  * | (unsigned int) |    (char)      |    (char)      |    (char[])    |
  * +----------------+----------------+----------------+----------------+
  *
  * When encapsulated in NDEF, the above message format is slightly altered.
  * An NDEF Message consists of multiple "Records," which is an identical 
  * concept to the way a Subprotocol Message can contain multiple (batched)
  * Directives.  But the NDEF format has some additional overhead that we honor
  * (because we like NFC and Android).
  * 
  *   07   06   05   04   03   02   01   00        OpenTag Subset Usage
  * +----+----+----+----+----+----+----+----+  =================================
  * | MB | ME | CF | SR | IL |     TNF      |  --> xxx11101 or 0xx10110
  * +----+----+----+----+----+----+----+----+
  * |      TYPE LENGTH (1 Byte: TLEN)       |  --> Always zero
  * +----+----+----+----+----+----+----+----+
  * |  PAYLOAD LENGTH (1 or 4 Bytes: PLEN)  |  --> Always one byte, 0-255
  * +----+----+----+----+----+----+----+----+
  * |    ID LENGTH (0 or 1 Bytes: IDLEN)    |  --> When IL=1, value always is 2
  * +----+----+----+----+----+----+----+----+
  * |          TYPE (TLEN Bytes)            |  --> Never present
  * +----+----+----+----+----+----+----+----+
  * |           ID (IDLEN Bytes)            |  --> When IL=1, Two bytes
  * +----+----+----+----+----+----+----+----+
  * |         PAYLOAD (PLEN Bytes)          |  --> 0-255 bytes
  * +----+----+----+----+----+----+----+----+
  * 
  * For Application Layer Protocol directives packed inside NDEF records:
  * - Initial TNF uses the UNKNOWN setting, with TLEN=0 and no TYPE Field
  * - Subsequent TNFs (if chunking) use UNCHANGED
  * - the ID is always a 2 byte value containing the Directive ID and Cmd
  * - Directive Length is subtracted by 4 and stored in Payload Length field
  ******************************************************************************
  */

#ifndef __NDEF_H
#define __NDEF_H

#include "OT_config.h"

#if (OT_FEATURE(NDEF) == ENABLED)

#include "OT_types.h"
#include "OTAPI_tmpl.h"
#include "alp.h"
#include "queue.h"


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


typedef enum {
    MSG_Null        = 0,
    MSG_Chunking_In = 1,
    MSG_Chunking_Out= 2,
    MSG_End         = 3
} NDEF_status;




/** ndef_message is an internal data store that is exposed only for purposes of
  * transparency (this is an open source project).
  */
typedef struct {
    //Queue*  msgq;
    ot_u8   last_flags;
    //ot_u8   msg_tnf;
    //ot_int  msg_records;
} ndef_message;

//If you want to expose the message data store, uncomment this.
//extern ndef_message ndef;




/** @brief  Begins a new message.  A message can contain one or more records.
  * @param  output      (Queue*) Pointer to Queue where record/message is buffered
  * @retval ot_bool      False/True when output parameter is Null/Non-Null
  * @ingroup NDEF
  *
  * The output parameter should point to a Queue that is persistent in memory or
  * at lest persistent during the processing of the message.  In other terms, if
  * you have the Queue declared in the C stack (i.e. within a function), make 
  * sure that function does not return prior to the message being sent (this 
  * will cause the C stack to pop the Queue declaration before the NDEF message
  * is finished operating on it).  If you declare the Queue in persistent 
  * memory, there is nothing to worry about.
  *
  * If otapi_new_ndefmsg is called while another message is being built, the
  * latter call will wipe-out the prior message.
  */
ot_bool ndef_new_msg(Queue* output);




/** @brief  Creates a new record.  A message can contain one or more records.
  * @param  record      (alp_record*) Pointer to struct containing record attributes
  * @param  record_data (ot_u8*) Payload data for the record
  * @param  output      (Queue*) Queue for output message (usually &dir_out)
  * @retval ot_bool     False/True on error/no error
  * @ingroup NDEF
  *
  * The record payload does not need to be double buffered.  If parameter 
  * record_data == NULL, no bytes will be written to the output queue,
  * and the queue putcursor will stay at the insertion point of the payload.
  * This allows the user to manually load data into the record via the queue
  * write functions (see queue.h).  If the user fails to load in the appropriate
  * amount of bytes, as indicated by the record->payload_length parameter, the
  * next call to otapi_new_ndefrec() or otapi_send_ndefmsg() will automatically 
  * crop or pad the payload in order to make sure that the length value written
  * to the record matches exactly the number of bytes in the payload.  While 
  * this feature will protect the record structure, it may corrupt the data. 
  * Therefore, the user should take steps to ensure the payload data is loaded
  * to the output queue accurately.
  *
  * The return value will report an error when the record parameter is 
  * malformed.  On error, nothing will be written the message queue.
  */
ot_bool ndef_new_record(alp_record* record, ot_u8* record_data, Queue* output);




/** @brief  Sends over Mpipe the NDEF message stored in the suppiled queuep
  * @param  output      (Queue*) Queue containing output data from Queue.front
  * @retval ot_int      error if negative, if non-negative it equals the amount of bytes in mpipe
  * @ingroup NDEF
  *
  * ndef_send_msg() will automatically align the data in the queue to match the
  * specifications of the NDEF message & record headers.  Then it will push this
  * data to the Mpipe.
  */
ot_int ndef_send_msg(Queue* output);





/** @brief  Loads a queue into the NDEF module, for later parsing
  * @param  input       (Queue*) Pointer to Queue where record/message is buffered
  * @retval ot_bool     False/True when input parameter is Null/Non-Null
  * @ingroup NDEF
  * @sa ndef_send_msg()
  *
  * This is the RX version of ndef_send_msg().  It does not actually parse any
  * data, but it must be called before calling otapi_parse_ndefrec() is called.
  */
ot_bool ndef_load_msg(Queue* input);





/** @brief  Parses received NDEF data
  * @param  in_q        (Queue*) Input data queue
  * @param  out_q       (Queue*) Output data queue
  * @retval NDEF_status Enumerated value of parsing & message status
  * @ingroup NDEF
  *
  * The function returns:
  * <LI> MSG_End when processing is done and there is a response    </LI>
  * <LI> MSG_Null if processing is done and there is no response    </LI>
  * <LI> MSG_Chunking_In if the message is not finished being received  </LI>
  * <LI> MSG_Chunking_Out if processing is underway and response must be chunked out </LI>
  * 
  * This function is called "ndef_parse_record" because it works at the record
  * level, but it also manages most of the message handling.  In typical usage
  * it is called when MPipe finishes receiving a packet.  Because it does do
  * some message handling, if the total message is assembled across multiple
  * frames/records, this function will only do processing once the message is
  * fully transfered or if the chunk bit is set.
  *
  * A good usage example is in otapi_ndef_proc() (implemented inside ndef.c).
  * In the main app code, if the mpipe RXDONE callback is set to this, that is
  * a sufficient implementation in most cases.
  * 
  * In almost all circumstances, out_q should be &dir_out (see buffers.h). in_q
  * is more flexible, but it is usually &dir_in.  The parameters for in_rec and
  * out_rec just need to be allocated by the caller.  The caller can use them
  * for additional visibility into the NDEF parsing process, if desired.
  *
  * @note Parsing Capability Limitations
  * This version of the function is experimental and it is not designed to work
  * with record chunking (on input or output) or NDEF messages longer than the
  * input or output queues (nominally, 256 bytes).  In some cases it might work
  * with such long messages, but in other cases it might not.  In production
  * environments, keep total message length less than the allocation of dir_in.
  *
  * @note Usability of NDEF within OpenTag
  * At this time, OpenTag uses only a subset of NDEF.  Therefore, any records
  * that do not conform to this feature subset will be ignored.  The subset is
  * of the "UNKNOWN" TNF, where the ID is two bytes and the payload contents are
  * defined by the ID bytes.  The documentation at the top of this header file
  * goes into greater detail on this topic.
  */
NDEF_status ndef_parse_record(Queue* in_q, Queue* out_q);







/** Bonus functions (implementation is optional, and pending).  These are 
  * similar in design to Android (Gingerbread) NDEF API, although they are not
  * designed in a way that makes much sense for embedded usage (Not my fault,
  * Google's fault for designing an API with lots of data bufferring, a typical
  * design problem of OO projects).
  *
  * If you have a heavy client (like a PC or an OMAP), you may seek to implement
  * these on the client side.  They will probably never go into the server side.
  */

///void ndef_assemble_record(Queue* output, ndef_record* record);
///void ndef_assemble_message(Queue* output, ot_int num_records, ndef_record* record_array);
///void ndef_copy_record(Queue* output, ot_uint data_length ot_u8* data);
///void ndef_copy_message(Queue* output, ot_uint data_length ot_u8* data);


#endif


#endif


