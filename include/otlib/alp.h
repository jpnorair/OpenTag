/*  Copyright 2013-2014, JP Norair
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
  * @file       /otlib/alp.h
  * @author     JP Norair
  * @version    V1.0
  * @date       25 Mar 2014
  * @brief      Application Layer Protocol Management Module
  * @defgroup   ALP
  * @ingroup    ALP
  *
  * "ALP" stands for Application Layer Protocol.  ALP defines a particular
  * format for message encapsulation.  Incidentally, this format is also very
  * useful for general purpose message passing in systems like OpenTag.
  *
  * @note "ALP" is an old term that doesn't anymore fit the wider scope of
  * modern ALP.  The data model will be changed to "CHOMP," meaning "Cascaded,
  * Header-Oriented Multi-Port."
  *
  * The ALP Module defines an ALP datatype and functional interface.  The
  * general usage model is to define an ALP "object" for usage by some number
  * of Applications/Tasks/Message-Sources, and then have these message sources
  * load their messages into ALP.  The next step is to parse the messages and
  * do some work according to the protocol in the message.  ALP takes care of
  * all these things.
  *
  * ALP is a library, not a task!  Task-like behavior for protocol processing
  * is possible by calling ALP library functions from your task.  Usually, this
  * is a task that manages some form of I/O, because protocol data has to come
  * from somewhere.
  *
  ******************************************************************************
  */

#ifndef __ALP_H_
#define __ALP_H_

#include <otstd.h>

#include <m2/tmpl.h>
#include <otlib/queue.h>


/** ALP Record Header
  * ALP Records can be used for NDEF and Pure-ALP.
  */

#define ALP_FLAG_MB         0x80    // Message Start bit
#define ALP_FLAG_ME         0x40    // Message End bit
#define ALP_FLAG_CF         0x20    // Chunk Flag
#define ALP_FLAG_SR         0x10
#define ALP_FLAG_WORKING    0x08    // For internal usage with ALP (no NDEF)
#define ALP_FLAG_INTRAMSG   0x07    // For internal usage with ALP (no NDEF)

/// Temporary, for transitioning some alp code that is being refactored
#define _O_CMD          cmd
#define _O_ID           id
#define _O_PLEN         plength
#define _O_FLAGS        flags
#define _I_CMD          -1
#define _I_ID           -2
#define _I_PLEN         -3
#define _I_FLAGS        -4

#define INREC(X)        inq->getcursor[_I_##X]
#define OUTREC(X)       outrec._O_##X
#define BOOKMARK_IN     bookmark_in
#define BOOKMARK_OUT    outrec.bookmark



typedef enum {
    MSG_Null        = 0,
    MSG_Chunking_Out= 2,
    MSG_Chunking_In = 4,
    MSG_End         = 6
} ALP_status;


///@note To keep data aligned cross platform, it might be advisable to use:
///      __attribute__ ((__packed__)) or __attribute__ ((aligned (2))).
///      On MSP430, ARM, and x86 these aren't needed, but maybe on some archs.
typedef struct {
    ot_u8   flags;              // ALP/NDEF flags
    ot_u8   plength;            // Payload Length
    ot_u8   id;                 // ALP ID (Similar to Destination Port)
    ot_u8   cmd;                // ALP CMD (ID-specific Command)
//    void*   bookmark;           // Internal use only (private)
} alp_record;


///@note The alp_tmpl structure is under redesign.  inrec and outrec will be
///      removed, and the application processors will be responsible to manage
///      their own record headers, with functional assitance from ALP module.
typedef struct {
    ot_u16      purge_id;       // Internal use only: for garbage collection

//    alp_record  inrec;        // Legacy: but using Macros [above] as bridge
    alp_record  outrec;         // Legacy: to be removed soon
    ot_queue*   inq;
    ot_queue*   outq;

    void*       sstack;         // Use NULL if the ALP is on an interface with no session stack
} alp_tmpl;



#if ( OT_FEATURE(ALP) )

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
ot_bool alp_ext_proc(alp_tmpl* alp, id_tmpl* user_id);
#endif



/** Main Library Functions <BR>
  * ========================================================================<BR>
  * These are the functions that OpenTag and your OpenTag-based code should be
  * calling.
  */

void alp_init(alp_tmpl* alp, ot_queue* inq, ot_queue* outq);



/** @brief  Check an ALP input to see if it has room for data
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  length      (ot_int) Number of bytes to check for availability
  * @retval ot_bool     True if "length" bytes are available in ALP
  * @ingroup ALP
  * @sa alp_load
  * @sa alp_purge
  *
  * alp_load() will call this function internally, and it is the preferred way
  * for apps that need to be thread-safe.
  *
  * alp_is_available() will call alp_purge() if there is not sufficient space
  * in the ALP input, in an effort to try to make enough space by culling dead
  * records.
  */
ot_bool alp_is_available(alp_tmpl* alp, ot_int reserve_bytes);




/** @brief  Thread-safe function for loading data into ALP input
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  src         (ot_u8*) source data to load into ALP
  * @param  length      (ot_int) Number of bytes to load from source into ALP
  * @retval ot_bool     True if load was possible, false if not
  * @ingroup ALP
  * @sa alp_is_available
  */
ot_bool alp_load(alp_tmpl* alp, ot_u8* src, ot_int length);




///@todo experimental
void alp_notify(alp_tmpl* alp, ot_sig callback);



/** @note User IDs for ALP's
  * Most ALP commands require some form of user authentication to run.  This is
  * analagous for a user having to log-in to a POSIX shell in order to run the
  * applications on the system.  The User ID needs to be authenticated to a
  * device ID via the auth system if the ALP's are being transmitted over the
  * air (via DASH7).
  */

/** @brief Header parser for ALP and also NDEF (ALP is a subset of NDEF)
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @retval ALP_status  Delivery Status of ALPs in message
  * @ingroup ALP
  *
  * This is the primary callable function for receiving and processing ALP
  * messages.  No other alp function needs to be called to process a received
  * ALP message.
  *
  * ALP messages wrap application protocol data.  The application may cause an
  * ALP message to be sent as output, related to the ALP message that has just
  * been parsed and processed.  Without caching or state-based control by the
  * application, one ALP input message may generate at most one output message.
  *
  * The function returns 0, 1, 2, 3 (enumerated in ALP_status), depending on
  * what is happening in the message processing.  If it returns 0 (MSG_Null)
  * or 3 (MSG_End), then processing has ceased due to error or that the message
  * is finished successfully.
  *
  * If it returns MSG_Chunking_In, then the input message is not over, but the
  * remaining data has not yet been received.  The communication layer should
  * get the next frame and call alp_parse_message() again, using the retained
  * "alp" object as input.  If it returns MSG_Chunking_Out, then the output
  * message is being dealt in much the same way.  alp_parse_message() reports
  * output ahead of input, therefore if messages are being chunked-in and out
  * at the same time, it will return MSG_Chunking_Out.
  */
ALP_status alp_parse_message(alp_tmpl* alp, id_tmpl* user_id);




/** @brief  Setup a new Application queue from the main alp input queue
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  appq        (ot_queue*) subordinate queue used by Application
  * @retval None
  * @ingroup ALP
  * @sa alp_append_appq
  * @sa alp_goto_next
  *
  * Non-atomic applications that run from a shared ALP usually need to maintain
  * an independent application queue.  The functions alp_goto_next() and
  * alp_retrieve_cmd() will will draw records from said queue, but before you
  * can use these, you must setup an Application queue in the first place.
  */
void alp_new_appq(alp_tmpl* alp, ot_queue* appq);



/** @brief  Refresh an Application queue from the main alp input queue
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  appq        (ot_queue*) subordinate queue used by Application
  * @retval None
  * @ingroup ALP
  * @sa alp_new_appq
  *
  * After an Application queue has been created, you can append more parts of
  * the ALP input queue to the App queue.  alp_append_appq() will move the
  * getcursor of the ALP input queue past this latest-read record, and as such
  * the App queue putcursor is extended to this new position, to include this
  * latest-read record.
  *
  * @note Do not call alp_append_appq() immediately after alp_new_appq() unless
  * you want to append a second record into the App queue.  alp_new_appq()
  * automatically appends the first record into the App ot_queue.
  */
void alp_append_appq(alp_tmpl* alp, ot_queue* appq);




/** @brief  Application Traversal Function for ALP input
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  appq        (ot_queue*) subordinate queue used by Application
  * @param  target      (ot_u8) ALP ID used by the Application
  * @retval ot_u8       Record Flags for next matching record.  0 if none found
  * @ingroup ALP
  * @sa alp_retrieve_cmd
  * @sa alp_new_appq
  *
  * This function is important for Applications that require non-atomic
  * processing of ALP messages.  Since an ALP stream can be shared by multiple
  * applications, this function will traverse the ALP input stream
  */
ot_u8 alp_goto_next(alp_tmpl* alp, ot_queue* appq, ot_u8 target);




/** @brief  Read ALP record command value and mark record as read
  * @param  apprec      (alp_record*) output alp record header
  * @param  appq        (ot_queue*) Application ot_queue
  * @param  target      (ot_u8) ALP ID used by Application
  * @retval ot_u8*      Pointer to front of record, or NULL if no record
  * @ingroup ALP
  * @sa alp_goto_next
  *
  * A protocol processor should use this function following alp_goto_next(), to
  * return the Command (cmd) value of the new record and also to mark the new
  * record as the working record.
  */
ot_u8* alp_retrieve_record(alp_record* apprec, ot_queue* appq, ot_u8 target);



/** @brief  Releases (frees) the record in the buffer
  * @param  appq        (ot_queue*) Application ot_queue
  * @retval None
  * @ingroup ALP
  * @sa alp_retrieve_record
  *
  * A protocol processor may use this function after it is finished with a
  * record.  Issuing alp_retrieve_record() will mark the record as finished,
  * allowing the ALP garbage collector (alp_purge()) to reallocate the space
  * for future records.
  */
void alp_release_record(ot_queue* appq);



/** @brief  Purge finished ALP records from an ALP Stream
  * @param  alp         (alp_tmpl*) ALP stream to purge
  * @retval None
  * @ingroup ALP
  * @sa alp_kill
  *
  * Non-atomic ALPs must set record flags to 0 after they have completed the
  * input processing of said records.  The ALP module will call alp_purge()
  * when the stream gets too full, and it needs to free-up space in the stream
  * for new input records.  Users can call alp_purge() as well, although it is
  * not usually necessary.
  *
  * The alp_purge() implementation is opportunistic.  It will avoid stream
  * defragmentation whenever possible (this process is relatively expensive).
  */
void alp_purge(alp_tmpl* alp);




/** @brief  Kill an Application's ALP data
  * @param  alp         (alp_tmpl*) ALP stream to kill & purge
  * @param  kill_id     (ot_u8) ALP ID to purge
  * @retval None
  * @ingroup ALP
  * @sa alp_purge
  *
  * alp_purge() will only cull ALP records that have been successfully accessed
  * and marked for deletion.  alp_kill() will seek-out any ALP records that
  * match the supplied ALP ID, it will mark them for death, and purge them.
  *
  * alp_kill() is intended to be used when killing a thread that may have some
  * unused data still in an ALP stream.
  */
void alp_kill(alp_tmpl* alp, ot_u8 kill_id);








/** Internal Module Routines <BR>
  * ========================================================================<BR>
  * Under normal software design models, these functions likely would not be
  * exposed.  However, we expose pretty much everything in OpenTag.
  *
  * Use with caution.
  */



/** @brief  Internal function for resolving the index of an app processor
  * @param  alp_id      (ot_u8) ALP ID
  * @retval ot_u8       index of the processor routine
  * @ingroup ALP
  * @sa alp_proc
  *
  *
  */
ot_u8 alp_get_handle(ot_u8 alp_id);





/** @brief  Process a received ALP record (vectors to all supported ALP's)
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  user_id     (id_tmpl*) user id for performing the record
  * @retval ot_bool     True if atomic, False if this ALP needs delayed processing
  * @ingroup ALP
  *
  * ID values (from spec)
  * 0x00:       Null subprotocol
  * 0x01:       Filesystem subprotocol
  * 0x02:       Sensor Configuration subprotocol (pending)
  * 0x11-14:    Security subprotocols (pending)
  */
ot_bool alp_proc(alp_tmpl* alp, id_tmpl* user_id);












/** Functions Under Review <BR>
  * ========================================================================<BR>
  * These are legacy functions.  They might get bundled into different
  * functions, changed, or removed.
  */

/** @brief Break a running ALP stream (due to error), and load error record
  * @param  alp                 (alp_tmpl*) ALP I/O control structure
  * @retval None
  * @ingroup ALP
  *
  * NDEF and Pure-ALP transport may call this function to terminate an ongoing
  * NDEF/ALP stream.  The "User" is implicit, and always GUEST.
  *
  * alp_break() loads a Null-ALP NACK with MB=1 & ME=1 at the end of the
  * message stream.  NDEF users should recognize this as a valid NACK or as a
  * NACK also contributing a framing error (NDEF does not support nested
  * records, so if chunking-out, this will result in a framing error).  In
  * Pure-ALP mode, this will simply be recognized as a stream-abort NACK, and
  * both ends should disconnect and kill their sessions.
  */
void alp_break(alp_tmpl* alp);




/** @brief Initialize an ALP header for a new record
  * @param  alp                 (alp_tmpl*) ALP I/O control structure
  * @param  flags               (ot_u8) ALP Flags to force high
  * @param  payload_limit       (ot_u8) Limit of Payload bytes per __Record__
  * @param  payload_remaining   (ot_int) Remaining Payload bytes in __Message__
  *
  * @retval None
  * @ingroup ALP
  * @sa alp_new_message()
  *
  * <LI> The "alp" parameter must be allocated by the caller. </LI>
  * <LI> On return, the caller must use value from alp->outrec.payload_length
  *      to manage data buffering/queuing </LI>
  *
  * This function automatically assigns the ALP flags and payload length fields
  * of the ALP header.  It does not assign ID or CMD fields, or anything else.
  *
  * The flags parameter requires that you specify ALP_FLAG_MB if you want to
  * start a new message, and that you use TNF=0 for pure ALP or TNF!=0 for NDEF.
  * If you are continuing a generic ALP stream that you don't know is Pure ALP
  * or NDEF, use TNF=0.  The value existing in the alp_tmpl TNF will be used.
  */
void alp_new_record(alp_tmpl* alp, ot_u8 flags, ot_u8 payload_limit, ot_int payload_remaining);




/** @brief Initializes an ALP header for new message, and first record
  * @param  alp                 (alp_tmpl*) ALP I/O control structure
  * @param  payload_limit       (ot_u8) Limit of Payload bytes per __Record__
  * @param  payload_remaining   (ot_int) Remaining Payload bytes in __Message__
  * @retval None
  * @ingroup ALP
  * @sa alp_new_record()
  *
  * This function is identical to alp_new_record(), except that it also sets
  * the "Message Begin" flag high.  If you wish to use alp_new_record() instead
  * of alp_new_message(), you must set ALP_FLAG_MB in alp->outrec.flags AFTER
  * alp_new_record() returns.

void alp_new_message(alp_tmpl* alp, ot_u8 payload_limit, ot_int payload_remaining);
  */



/** @todo pending removal/refactoring
  * @brief Header parser for ALP and also NDEF (ALP is a subset of NDEF)
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @retval ot_u8       0 on success (valid header), non-zero on bad header.
  * @ingroup ALP
  *
  * alp_parse_header() will inspect the "Type Name Field" of the Flags byte.
  * In Pure-ALP, this is 0, and the function will read the rest of the header
  * as a pure-ALP header.  In NDEF, this will be Non-Zero, and the function
  * will read the rest of the header as NDEF header.  If your build does not
  * support NDEF, the NDEF parts won't be compiled.
  */
ot_bool alp_parse_header(alp_tmpl* alp);




/** @brief  Common function for responding to ALP with a simple 16-bit return
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  respond     (ot_bool) True/False to respond or not
  * @param  out_dir_cmd (ot_u8) output dir cmd
  * @param  retval      (ot_u16) 16 bit return value
  * @retval None
  * @ingroup ALP
  */
ot_bool alp_load_retval(alp_tmpl* alp, ot_u16 retval);






void alp_load_header(ot_queue* appq, alp_record* rec);















/** Protocol Processors <BR>
  * ========================================================================<BR>
  * @note Protocol Processors are exposed, but they never should be called
  * unless you are writing a patch function for alp_proc() or a patch function
  * for the Protocol Processor function itself.  Standard usage is to call
  * alp_parse_message() when there is a message in the ALP stream, which will
  * call alp_proc(), which will call one of these Protocol Processors.
  */

/** @brief  Used by parser when ALP ID is unsupported (returns False)
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  user_id     (id_tmpl*) user id for performing the record
  * @retval ot_bool		Always True
  * @ingroup ALP
  */
ot_bool alp_proc_null(alp_tmpl* alp, id_tmpl* user_id);


/** @brief  Process a received filesystem ALP record
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  user_id     (id_tmpl*) user id for performing the record
  * @retval ot_bool		True if atomic, False if this ALP needs delayed processing
  * @ingroup ALP
  */
ot_bool alp_proc_filedata(alp_tmpl* alp, id_tmpl* user_id);




#if (OT_FEATURE(SENSORS) == ENABLED)
/* @brief  Process a received sensor configurator ALP record (not implemented)
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  user_id     (id_tmpl*) user id for performing the record
  * @retval ot_bool     True if atomic, False if this ALP needs delayed processing
  * @ingroup ALP
  */
ot_bool alp_proc_sensor(alp_tmpl* alp, id_tmpl* user_id);
#endif





#if (OT_FEATURE(DASHFORTH) == ENABLED)
#   if (OT_FEATURE(CAPI) != ENABLED)
#       error For DASHForth to work, C-API must be ENABLED (it is not: check OT_config.h).
#   endif
/** @brief  Process a received DASHFORTH ALP record
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  user_id     (id_tmpl*) user id for performing the record
  * @retval ot_bool     True if atomic, False if this ALP needs delayed processing
  * @ingroup ALP
  */
ot_bool alp_proc_dashforth(alp_tmpl* alp, id_tmpl* user_id);
#endif






#if (OT_FEATURE(LOGGER) == ENABLED)
/** @brief  Process a received logger ALP record (typically client only)
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  user_id     (id_tmpl*) user id for performing the record
  * @retval ot_bool     True if atomic, False if this ALP needs delayed processing
  * @ingroup ALP
  */
ot_bool alp_proc_logger(alp_tmpl* alp, id_tmpl* user_id);
#endif







#if (OT_FEATURE(ALPAPI) == ENABLED)
#   if (OT_FEATURE(CAPI) != ENABLED)
#       error For ALP-API to work, C-API must be ENABLED (it is not: check OT_config.h).
#   endif

#   if (OT_FEATURE(M2))
    /** @brief  Process a received Session API record
      * @param  alp         (alp_tmpl*) ALP I/O control structure
      * @param  user_id     (id_tmpl*) user id for performing the record
      * @retval ot_bool     True if atomic, False if this ALP needs delayed processing
      * @ingroup ALP
      */
    ot_bool alp_proc_api_session(alp_tmpl* alp, id_tmpl* user_id);
    
    /** @brief  Process a received System API record
      * @param  alp         (alp_tmpl*) ALP I/O control structure
      * @param  user_id     (id_tmpl*) user id for performing the record
      * @retval ot_bool     True if atomic, False if this ALP needs delayed processing
      * @ingroup ALP
      */
    ot_bool alp_proc_api_system(alp_tmpl* alp, id_tmpl* user_id);
    
    /** @brief  Process a received Query API record
      * @param  alp         (alp_tmpl*) ALP I/O control structure
      * @param  user_id     (id_tmpl*) user id for performing the record
      * @retval ot_bool     True if atomic, False if this ALP needs delayed processing
      * @ingroup ALP
      */
    ot_bool alp_proc_api_query(alp_tmpl* alp, id_tmpl* user_id);
    #endif
#endif








#if (OT_FEATURE(SECURITY) == ENABLED)
/** @brief  Process a received Security/Auth ALP record (not implemented)
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  user_id     (id_tmpl*) user id for performing the record
  * @retval ot_bool     True if atomic, False if this ALP needs delayed processing
  * @ingroup ALP
  */
ot_bool alp_proc_sec(alp_tmpl* alp, id_tmpl* user_id);
#endif








void alp_breakdown_u8(ot_queue* in_q, void* data_type);
void alp_breakdown_u16(ot_queue* in_q, void* data_type);
void alp_breakdown_u32(ot_queue* in_q, void* data_type);
void alp_breakdown_queue(ot_queue* in_q, void* data_type);
void alp_breakdown_session_tmpl(ot_queue* in_q, void* data_type);
void alp_breakdown_advert_tmpl(ot_queue* in_q, void* data_type);
void alp_breakdown_command_tmpl(ot_queue* in_q, void* data_type);
void alp_breakdown_id_tmpl(ot_queue* in_q, void* data_type);
void alp_breakdown_routing_tmpl(ot_queue* in_q, void* data_type);
void alp_breakdown_dialog_tmpl(ot_queue* in_q, void* data_type);
void alp_breakdown_query_tmpl(ot_queue* in_q, void* data_type);
void alp_breakdown_ack_tmpl(ot_queue* in_q, void* data_type);
void alp_breakdown_error_tmpl(ot_queue* in_q, void* data_type);   //Client only?
void alp_breakdown_udp_tmpl(ot_queue* in_q, void* data_type);
void alp_breakdown_isfcomp_tmpl(ot_queue* in_q, void* data_type);
void alp_breakdown_isfcall_tmpl(ot_queue* in_q, void* data_type);

void alp_stream_u8(ot_queue* out_q, void* data_type);
void alp_stream_u16(ot_queue* out_q, void* data_type);
void alp_stream_u32(ot_queue* out_q, void* data_type);
void alp_stream_queue(ot_queue* out_q, void* data_type);
void alp_stream_session_tmpl(ot_queue* out_q, void* data_type);
void alp_stream_advert_tmpl(ot_queue* out_q, void* data_type);
void alp_stream_command_tmpl(ot_queue* out_q, void* data_type);
void alp_stream_id_tmpl(ot_queue* out_q, void* data_type);
void alp_stream_routing_tmpl(ot_queue* out_q, void* data_type);
void alp_stream_dialog_tmpl(ot_queue* out_q, void* data_type);
void alp_stream_query_tmpl(ot_queue* out_q, void* data_type);
void alp_stream_ack_tmpl(ot_queue* out_q, void* data_type);
void alp_stream_error_tmpl(ot_queue* out_q, void* data_type);
void alp_stream_udp_tmpl(ot_queue* out_q, void* data_type);
void alp_stream_isfcomp_tmpl(ot_queue* out_q, void* data_type);
void alp_stream_isfcall_tmpl(ot_queue* out_q, void* data_type);






#endif  // (OT_FEATURE(ALP) == ENABLED)
#endif  // ifdef __ALP_H

