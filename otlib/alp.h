/*  Copyright 2010-2012, JP Norair
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
  * @date       31 July 2012
  * @brief      Application Layer Subprotocol header
  * @defgroup   ALP
  * @ingroup    ALP
  *
  * Application Layer Subprotocols (ALP's) are directive-based protocols that
  * interface only with the application layer.  Other protocols supported by
  * DASH7 Mode 2 exist at lower levels, even though they may interact with
  * higher level data (this is done to improve size and performance).  
  *
  * Some ALP's are defined in the DASH7 Mode 2 Specification.
  * - Filesystem access (Veelite)
  * - Sensor Configuration (pending draft of ISO 21451-7)
  * - Security Configuration (pending draft of ISO 29167-7)
  *
  *
  
  ******************************************************************************
  */

#ifndef __ALP_H_
#define __ALP_H_

#include "OT_types.h"
#include "OT_config.h"

#include "OTAPI_tmpl.h"
#include "queue.h"


/** ALP Record Header
  * ALP Records can be used for NDEF and Pure-ALP.
  */
  
#define ALP_FLAG_MB     0x80    // Message Start bit
#define ALP_FLAG_ME     0x40    // Message End bit
#define ALP_FLAG_CF     0x20    // Chunk Flag


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
    void*   bookmark;           // Internal use only (private)
} alp_record;

typedef struct {
    alp_record  inrec;
    alp_record  outrec;
    ot_queue*   inq;
    ot_queue*   outq;
} alp_tmpl;



#if ( OT_FEATURE(SERVER) && OT_FEATURE(ALP) )



void alp_init(alp_tmpl* alp, ot_queue* inq, ot_queue* outq);



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
  */
void alp_new_message(alp_tmpl* alp, ot_u8 payload_limit, ot_int payload_remaining);




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




/** @brief Header parser for ALP and also NDEF (ALP is a subset of NDEF)
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



/** @note User IDs for ALP's
  * Most ALP commands require some form of user authentication to run.  This is
  * analagous for a user having to log-in to a POSIX shell in order to run the
  * applications on the system.  The User ID needs to be authenticated to a 
  * device ID via the auth system if the ALP's are being transmitted over the 
  * air (via DASH7).
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
ot_bool alp_proc(alp_tmpl* alp, id_tmpl* user_id);



/** @brief  Purge one type of ALP ID records from an ALP Queue
  * @param  alpq        (ot_queue*) ALP Queue to purge
  * @param  id          (ot_u8) target ALP ID of records to purge
  * @retval None
  * @ingroup ALP
  */
void alp_purge(ot_queue* alpq, ot_u8 id);






/** @note Subprotocol processing functions
  * The functions below are exposed, but they never should be called unless you
  * are writing a patch function for alp_proc().  Otherwise, just use the 
  * built-in alp_proc() function with the parameters set appropriately.
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






#if (LOG_FEATURE(ANY) == ENABLED)
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






#endif  // if (OT_FEATURE(SERVER) == ENABLED) && (OT_FEATURE(ALP) == ENABLED)
#endif  // ifdef __ALP_H

