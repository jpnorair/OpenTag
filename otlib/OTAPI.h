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
  * @file       /OTlib/OTAPI.h
  * @author     JP Norair
  * @version    V1.0
  * @date       26 Jan 2012
  * @brief      OpenTag API for internal (C) and external (ALP) control
  * @defgroup   OTAPI
  *
  * A master file header for C and ALP-based API's.  A good include for any
  * program that is using OpenTag top-level resources.
  ******************************************************************************
  */


#ifndef __OTAPI_H
#define __OTAPI_H

#include "OT_types.h"       // OpenTag types
#include "OT_config.h"      // OpenTag library-level configuration
#include "OT_utils.h"       // Utility functions

#include "OTAPI_tmpl.h"     // Template data types used a lot in OT/DASH7

#include "buffers.h"        // Built-in buffers
#include "queue.h"          // A buffer data-type module

#include "system.h"         // Data Link Layer & Kernel
#include "m2_network.h"     // Network Layer
#include "m2_transport.h"           // Transport Layer

#include "veelite.h"

///@todo Kernel-level stuff should be encapsulated in generic system-layer functions
#if (OT_FEATURE(CUSTOM_KERNEL) == ENABLED)
    // no other kernels supported at the moment
#else
#   include "native/system_native.h"
#endif






/** Server/Client
  * Just in case you don't know client/server nomenclature, the device that is
  * remote is the server.  The local device is the client.  So, the client is
  * the one that probably doesn't have DASH7 on it, and the server is the one
  * that is running DASH7 and most of the OpenTag code.
  */
  

/// MPIPE is the "Message Pipe"
#if (OT_FEATURE(MPIPE) == ENABLED)
#   include "mpipe.h"
#endif


/// ALP-API Server and Features are part of the Application Layer Protocols (ALP)
#if (OT_FEATURE(ALP) == ENABLED)
#   include "alp.h"
#endif


/// C API Features, and Logging macros
#if (OT_FEATURE(CAPI) == ENABLED)
#   include "OTAPI_c.h"
#endif


/*
// Log all packets that are transmitted
#   if (LOG_FEATURE(TX) == ENABLED)
#       define OT_LOG_TX()      otapi_log_msg(3, txq.length, (ot_u8*)"TXP", txq.front)
#   else
#       define OT_LOG_TX()      while(0)
#   endif

// Log errors, even if they are not transmitted, or if LOG_TX is disabled.
#   if (LOG_FEATURE(ERRORS) == ENABLED)
#       define OT_LOG_ERROR()   otapi_log_msg(3, txq.length, (ot_u8*)"ERR", txq.front)
#   else
#       define OT_LOG_ERROR()   while(0)
#   endif

// Log the first recongnition of a advertising packet train
#   if (LOG_FEATURE(RX_M2ADVP) == ENABLED)
#       define OT_LOG_RXADV(ETA, SID)   do { \
                                            Fourbytes logdata; \
                                            logdata.ushort[0] = ETA; \
                                            logdata.ushort[1] = SID; \
                                            otapi_log_msg(3, 4, (ot_u8*)"ADV", &logdata.ubyte[0]); \
                                        } while (0)
#   else
#       define OT_LOG_RXADV()           while(0)
#   endif

// Log any m2 foreground frame packet received
#   if (LOG_FEATURE(RX_ANY) == ENABLED)
#       define OT_LOG_RXANY()   otapi_log_msg(7, rxq.length, (ot_u8*)"RXP_ANY", rxq.front)
#   else
#       define OT_LOG_RXANY()   while(0)
#   endif

// Log all foreground frames/packets that pass DLL filtering (tx eirp & subnet)
#   if (LOG_FEATURE(RX_DLLFILTER) == ENABLED)
#       define OT_LOG_RXDLL()   otapi_log_msg(7, rxq.length, (ot_u8*)"RXP_DLL", rxq.front)
#   else
#       define OT_LOG_RXDLL()   while(0)
#   endif

// Log all foreground frames that pass MAC Filtering (addressing and authentication)
#   if (LOG_FEATURE(RX_MACFILTER) == ENABLED)
#       define OT_LOG_RXMAC()   otapi_log_msg(7, rxq.length, (ot_u8*)"RXP_MAC", rxq.front)
#   else
#       define OT_LOG_RXMAC()   while(0)
#   endif

// Log all foreground frames that pass Network layer & Transport Layer addressing
#   if (LOG_FEATURE(RX_NETWORKED) == ENABLED)
#       define OT_LOG_RXNET()   otapi_log_msg(7, rxq.length, (ot_u8*)"RXP_NET", rxq.front)
#   else
#       define OT_LOG_RXNET()   while(0)
#   endif

// Log a system fault
#   if (LOG_FEATURE(FAULTS) == ENABLED)
#       define OT_LOG_FAULT(FCODE)      otapi_log_code(5, (ot_u8*)"FAULT", FCODE)
#   else
#       define OT_LOG_FAULT(FCODE)      while(0)
#   endif

// Log system errors
#   if (LOG_FEATURE(FAILS) == ENABLED)
#       define OT_LOG_FAILVL(FCODE)     otapi_log_code(6, (ot_u8*)"VL_ERR", FCODE)
#       define OT_LOG_FAILSYS(FCODE)    otapi_log_code(7, (ot_u8*)"SYS_ERR", FCODE-0x100)
#       define OT_LOG_FAILNET(FCODE)    otapi_log_code(7, (ot_u8*)"NET_ERR", FCODE-0x200)
#   else
#       define OT_LOG_FAILVL(FCODE)     while(0)
#       define OT_LOG_FAILSYS(FCODE)    while(0)
#       define OT_LOG_FAILNET(FCODE)    while(0)
#   endif
*/








#if ((OT_FEATURE(NDEF) == ENABLED) && (OT_FEATURE(MPIPE) == ENABLED))
/** OTAPI functions within NDEF module      <BR>
  * ========================================================================<BR>
  * Use the MPipe to move NDEF packets in and out.  These functions should be
  * attached to the MPipe TXDONE callback (otapi_ndef_idle) and the RXDONE 
  * callback (otapi_ndef_proc) if you are building an NDEF server command pipe.
  */

void otapi_ndef_idle(ot_int code);
void otapi_ndef_proc(ot_int code);
#endif





#if (OT_FEATURE(ALPEXT) == ENABLED)
/** ALP Extension function          <BR>
  * ========================================================================<BR>
  * Custom/Proprietary user ALPs can be implemented using this function.  It
  * is not implemented inside OTlib (or otkernel, OTplatform, OTradio).  It 
  * should be implemented inside the application (OTlib or potentially inside 
  * OTlibext.
  */


/** @brief  Process a received user-proprietary ALP record
  * @param  in_rec      (alp_record*) Header of input ALP record, to be processed
  * @param  out_rec     (alp_record*) Header of output ALP record, due to processing
  * @param  in_q        (Queue*) input queue containing record
  * @param  out_q       (Queue*) output queue for [optional] record response
  * @param  user_id     (id_tmpl*) user id for performing the record 
  * @retval None
  * @ingroup ALP
  *
  * Very important: if OT_FEATURE(ALPEXT) == ENABLED, the user application must
  * implement this function somewhere.  Any ALP ID's that get processed by ALP
  * which do not match known ID's will get dumped into this function.
  */
void otapi_alpext_proc(alp_record* in_rec, alp_record* out_rec, Queue* in_q, Queue* out_q, id_tmpl* user_id);
#endif






/** Logging function definitions         <BR>
  * ========================================================================<BR>
  * These are implemented in OTAPI_logger.c, just because logging is such a
  * common function.  They could be implemented by the user, however, bypassing
  * the implementations in OTAPI_logger.c
  */

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
  * @param  id_subcode	(ot_u8) ALP Subcode field
  * @param  payload_length (ot_int) length of payload
  * @retval None
  * @ingroup OTAPI_c
  *
  * This is for expert use, such as if you are loading formatted data into the
  * log buffer, and you don't want to double buffer.
  */
void otapi_log_header(ot_u8 id_subcode, ot_int payload_length);



/** @brief  Log a datastream directly (no double buffering)
  * @param  data    (ot_u8*) the pre-formatted datastream (NDEF)
  * @retval ot_u16  0 on failure, non-zero on non-failure
  * @ingroup OTAPI_c
  *
  * This is the fastest way to log, but the data must be already formatted into
  * logging format (for official ports, this is NDEF).  The data pointed-to by
  * the data parameter is not double-buffered into an output buffer, so use it
  * cautiously.
  */
void otapi_log_direct(ot_u8* data);



/** @brief  Format and log a generic block of data
  * @param  subcode (ot_u8) Type of data that's being logged
  * @param  length  (ot_int) length in bytes of the supplied datastream
  * @param  data    (ot_u8*) the arbitrary datastream
  * @retval ot_u16  0 on failure, non-zero on non-failure
  * @ingroup OTAPI_c
  *
  * On the subcode: 
  * The logger directive uses directive ID = 0x04.  There is also a subcode byte
  * in the directive header.  System messages and error codes have subcode = 2.  
  * 0x01 is for response logging.  You can use any other subcode, although there 
  * might be some standards that emerge.
  */
void otapi_log(ot_u8 subcode, ot_int length, ot_u8* data);
#define otapi_log_response(LENGTH, DATA)    otapi_log(0x01, LENGTH, DATA)



/** @brief  Log arbitrary "message," which is type + data.
  * @param  logcmd		(logmsg_type) enumerated value of message type
  * @param  label_len   (ot_int) number of bytes/chars in the label
  * @param  data_len    (ot_int) number of bytes/chars in the data
  * @param  label       (ot_u8*) label byte array
  * @param  data        (ot_u8*) data byte array
  * @retval ot_u16      0 on failure, non-zero on non-failure
  * @ingroup OTAPI_core
  * @sa otapi_log_msg
  *
  * Similar to otapi_log_raw() except that a label string is applied to the 
  * datastream prior to the data payload.  The label string is zero-terminated,
  * so it should take on ASCII values or similar.
  */
void otapi_log_msg(logmsg_type logcmd, ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data);


/** @brief  Identical to otapi_log_msg, but converts data into ASCII Hex
  * @param  label_len   (ot_int) number of bytes/chars in the label
  * @param  data_len    (ot_int) number of bytes/chars in the data
  * @param  label       (ot_u8*) label byte array
  * @param  data        (ot_u8*) data byte array
  * @retval ot_u16      0 on failure, non-zero on non-failure
  * @ingroup OTAPI_core
  * @sa otapi_log_msg
  * 
  * The hex output is in capital letters.  One byte of data is converted into
  * three bytes of hex output -- two characters and a space -- therefore the
  * output buffer must be oversized in order for this function to print out
  * a full, 255 byte data frame.
  *
  * @note otapi_log_hexmsg is intended for usage with debugging builds or to push
  * messages to clients that do not have message interpreters (namely, binary
  * to hex conversion on the client side).  Needless to say, it is a better
  * use of resources to do any kind of message interpreting on the cilent side.
  */
void otapi_log_hexmsg(ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data);



/** @brief  Log a system code, such as an error code
  * @param  label_len   (ot_int) number of bytes/chars in the label
  * @param  label       (ot_u8*) label byte array
  * @param  code        (ot_u16) an integer code
  * @retval ot_u16      0 on failure, non-zero on non-failure
  * @ingroup OTAPI_core
  *
  * Similar to otapi_log_msg() and, by extension, otapi_log_raw(). Instead of a
  * byte datastream, though, the data payload consists of a two-byte code value,
  * which is translated to ASCII Hex on output
  *
  * @note parsing the payload on input via stdio.h can look like:
  *       scanf("%s %04X", &label_in, &code_in);
  */
void otapi_log_code(ot_int label_len, ot_u8* label, ot_u16 code);

#endif


#endif


