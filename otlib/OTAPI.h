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
  * @file       /otlib/OTAPI.h
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

#include "OT_types.h"           // OpenTag types
#include "OTAPI_tmpl.h"         // Template data types used a lot in OT/DASH7
#include "OT_config.h"          // OpenTag library-level configuration
#include "OT_utils.h"           // Utility functions

#include "buffers.h"            // Built-in buffers
#include "queue.h"              // A buffer data-type module
#include "m2_dll.h"             // Data Link Layer
#include "m2_network.h"         // Network Layer
#include "m2_transport.h"       // Transport Layer
#include "veelite.h"            // Filesystem (Also Presentation Layer)


///@todo Kernel-level stuff should be encapsulated in generic system-layer functions
#if (OT_FEATURE(CUSTOM_KERNEL) == ENABLED)
    // no other kernels supported at the moment
#else
#   include "gulp/system_gulp.h"
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




#if (OT_FEATURE(ALPEXT) == ENABLED)
/** ALP Extension function          <BR>
  * ========================================================================<BR>
  * Custom/Proprietary user ALPs can be implemented using this function.  It
  * is not implemented inside OTlib (or otkernel, OTplatform, OTradio).  It 
  * should be implemented inside the application (OTlib or potentially inside 
  * OTlibext.
  */


/** @brief  Process a received user-proprietary ALP record
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  user_id     (id_tmpl*) user id for performing the record 
  * @retval ot_bool		True if output
  * @ingroup ALP
  *
  * Very important: if OT_FEATURE(ALPEXT) == ENABLED, the user application must
  * implement this function somewhere.  Any ALP ID's that get processed by ALP
  * which do not match known ID's will get dumped into this function.
  */
ot_bool otapi_alpext_proc(alp_tmpl* alp, id_tmpl* user_id);
#endif






/** Comm-Tasker function definitions        <BR>
  * ========================================================================<BR>
  * These are implemented in OTAPI_tasker.c.  They are API functions used to
  * create OpenTag communication (session) tasks of various formats.
  */


/** @brief  Creates a session task that is entered immediately
  * @param  s_tmpl      (session_tmpl*) Session Template provided by caller
  * @param  applet      (ot_app) Applet that gets called when Session activates
  * @retval m2session*  Pointer to the session that was created
  * @ingroup OTAPI
  * @sa otapi_task_schedule(), otapi_task_flood()
  *
  * If you call this function somewhere in your application, it will create a
  * new, ad hoc communication session and put it in the session stack.  The 
  * kernel will process this session as soon as it is free.
  *
  * Like all session tasks, there is an attached applet.  The applet is a 
  * callback function that the kernel calls when it activates the session.
  * The applet needs to do whatever is required to build the request frame.
  * See the applets that come with OpenTag (in /otlibext) for ideas.
  *
  * This is a good task function to use, because it is predictable.  Generally,
  * if you application is able to call this function at all, it means the
  * kernel is not blocking session creation in favor of some higher-priority
  * task (e.g. I/O drivers).  So, when you call it, the chance that for some
  * reason your session is blocked (and flushed) is effectively 0.
  */
m2session* otapi_task_immediate(session_tmpl* s_tmpl, ot_app applet);



/** @brief  Creates a session task that is entered at a scheduled, later time
  * @param  s_tmpl      (session_tmpl*) Session Template provided by caller
  * @param  applet      (ot_app) Applet that gets called when Session activates
  * @param  offset      (ot_u16) Time offset, in ticks, when session gets activated
  * @retval m2session*  Pointer to the session that was created
  * @ingroup OTAPI
  * @sa otapi_task_immediate(), otapi_task_flood()
  *
  * This function is identical to otapi_task_immediate() but it also includes
  * the "offset" parameter.  The value supplied into "offset" is the number of
  * ticks (1/1024s) between the time of calling and when the kernel should 
  * activate the session.
  *
  * The caveat of using this function is that the kernel might be engaged in a
  * higher-priority I/O task at the time the scheduling occurs.  In this case,
  * sometimes the kernel will try to run the session late and sometimes it will
  * just flush the session.  The decision of late-run/flush is dependent on the
  * kernel implementation.  Typically, tasks that run really fast (<=1 tick) 
  * will not cause the sessions to be flushed, but longer tasks will.
  */
m2session* otapi_task_schedule(session_tmpl* s_tmpl, ot_app applet, ot_u16 offset);



/** @brief  Creates a session task that includes advertising flooding
  * @param  adv_tmpl    (advert_tmpl*) Advertising Template provided by caller
  * @param  s_tmpl      (session_tmpl*) Session Template provided by caller
  * @param  applet      (ot_app) Applet that gets called when Session activates
  * @retval m2session*  Pointer to the session that was created
  * @ingroup OTAPI
  * @sa otapi_task_immediate(), otapi_task_schedule()
  *
  * This function is identical to otapi_task_immediate() but it also includes
  * the "adv_tmpl" parameter for specifying the advertising method.  DASH7 M2
  * can do "background advertising," a method of flooding tiny packets onto a
  * channel for the purpose of synchronizing a group of unsynchronized devices. 
  * A session (starting with request) always follows the advertising flood.
  *
  * Note on advertising: Advertising is a unique feature of DASH7.  Some other
  * systems so similar things, but there is no system that has as powerful a
  * method as DASH7 has.  Basic usage is to have duty = 100%.  In some regions
  * (i.e. USA) you can set duty_on/duty_off to reduce average TX power, which
  * makes long advertising durations practical even with oppressive FCC rules.
  *
  * <LI> advert_tmpl.duty_off: ticks to take a break from advertising, before
  *      resuming.  Set to 0 to enable 100% duty. </LI>
  * <LI> advert_tmpl.duty_on: ticks to advertise before taking a break.  It is
  *      ignored if duty_off == 0. </LI>
  * <LI> advert_tmpl.channel: Channel ID to do the advertising on.  The session
  *      request channel may be the same channel or a different one. </LI>
  * <LI> advert_tmpl.duration: ticks to run the entire flooding process.  After
  *      this number of ticks the advertising will end, and the session request
  *      will begin. </LI>
  */
m2session* otapi_task_advertise(advert_tmpl* adv_tmpl, session_tmpl* s_tmpl, ot_app applet);





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
  * @param  id_subcode	   (ot_u8) ALP Subcode field
  * @param  payload_length (ot_int) length of payload
  * @retval ot_bool        Returns False if the mpipe out-queue is out of space
  * @ingroup OTAPI_c
  *
  * This is for expert use, such as if you are loading formatted data into the
  * log buffer, and you don't want to double buffer.
  */
ot_bool otapi_log_header(ot_u8 id_subcode, ot_int payload_length);



/** @brief  Log a datastream directly (no double buffering)
  * @param  None
  * @retval ot_u16  0 on failure, non-zero on non-failure
  * @ingroup OTAPI_c
  *
  * This is the fastest way to log, but the data must be already formatted into
  * logging format (for official ports, this is NDEF) and stored in the MPIPE
  * output queue.
  */
void otapi_log_direct();



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


