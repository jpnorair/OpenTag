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
  * @file       /otlib/alp.h
  * @author     JP Norair
  * @version    R103
  * @date       31 Oct 2017
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

#define INREC(ALP, X)   q_getcursor_val(ALP->inq, _I_##X)    /* inq->getcursor[_I_##X] */
#define OUTREC(X)       outrec._O_##X

/*
typedef struct {
    ot_u8 length;
    ot_u8* value;
} id_tmpl;
*/

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
} alp_record;


///@note The alp_tmpl structure is under redesign.  outrec will be
///      removed, and the application processors will be responsible to manage
///      their own record headers, with functional assitance from ALP module.
typedef struct {
    ot_u16      purge_id;       // Internal use only: for garbage collection

    alp_record  outrec;         // Legacy: to be removed soon
    ot_queue*   inq;
    ot_queue*   outq;
    void*       sstack;         // Use NULL if the ALP is on an interface with no session stack
} alp_tmpl;


typedef ot_bool (*alp_fn)(alp_tmpl*, const id_tmpl*);


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
ot_bool alp_ext_proc(alp_tmpl* alp, const id_tmpl* user_id);
#endif





/** Functions Under Review <BR>
  * ========================================================================<BR>
  * These are legacy functions.  They might get bundled into different
  * functions, changed, or removed.
  */

void alp_new_record(alp_tmpl* alp, ot_u8 flags, ot_u8 payload_limit, ot_int payload_remaining);



/** Main Library Functions <BR>
  * ========================================================================<BR>
  * These are the functions that OpenTag and your OpenTag-based code should be
  * calling.
  */

void alp_init(alp_tmpl* alp, ot_queue* inq, ot_queue* outq);



///@todo experimental
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
void alp_add_app(alp_tmpl* alp, ot_u8 alp_id, alp_fn callback, ot_queue appq);


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
ALP_status alp_parse_message(alp_tmpl* alp, const id_tmpl* user_id);




/** Internal Module Routines <BR>
  * ========================================================================<BR>
  * Under normal software design models, these functions likely would not be
  * exposed.  However, we expose pretty much everything in OpenTag.
  *
  * Use with caution.
  */

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
ot_bool alp_proc(alp_tmpl* alp, const id_tmpl* user_id);





/** Functions Under Review <BR>
  * ========================================================================<BR>
  * These are legacy functions.  They might get bundled into different
  * functions, changed, or removed.
  */


/** @brief  Common function for responding to ALP with a simple 16-bit return
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  respond     (ot_bool) True/False to respond or not
  * @param  out_dir_cmd (ot_u8) output dir cmd
  * @param  retval      (ot_u16) 16 bit return value
  * @retval None
  * @ingroup ALP
  */
ot_bool alp_load_retval(alp_tmpl* alp, ot_u16 retval);














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
ot_bool alp_proc_null(alp_tmpl* alp, const id_tmpl* user_id);


/** @brief  Process a received filesystem ALP record
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  user_id     (id_tmpl*) user id for performing the record
  * @retval ot_bool		True if atomic, False if this ALP needs delayed processing
  * @ingroup ALP
  */
ot_bool alp_proc_filedata(alp_tmpl* alp, const id_tmpl* user_id);




#if (OT_FEATURE(SENSORS) == ENABLED)
/* @brief  Process a received sensor configurator ALP record (not implemented)
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  user_id     (id_tmpl*) user id for performing the record
  * @retval ot_bool     True if atomic, False if this ALP needs delayed processing
  * @ingroup ALP
  */
ot_bool alp_proc_sensor(alp_tmpl* alp, const id_tmpl* user_id);
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
ot_bool alp_proc_dashforth(alp_tmpl* alp, const id_tmpl* user_id);
#endif






#if (OT_FEATURE(LOGGER) == ENABLED)
/** @brief  Process a received logger ALP record (typically client only)
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  user_id     (id_tmpl*) user id for performing the record
  * @retval ot_bool     True if atomic, False if this ALP needs delayed processing
  * @ingroup ALP
  */
ot_bool alp_proc_logger(alp_tmpl* alp, const id_tmpl* user_id);
#endif






#if (OT_FEATURE(SECURITY) == ENABLED)
/** @brief  Process a received Security/Auth ALP record (not implemented)
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  user_id     (id_tmpl*) user id for performing the record
  * @retval ot_bool     True if atomic, False if this ALP needs delayed processing
  * @ingroup ALP
  */
ot_bool alp_proc_sec(alp_tmpl* alp, const id_tmpl* user_id);
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

