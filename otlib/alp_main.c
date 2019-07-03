/*  Copyright 2017, JP Norair
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
  * @file       /otlib/alp_main.c
  * @author     JP Norair
  * @version    R103
  * @date       31 Oct 2017
  * @brief      Application Layer Protocol Main Processor
  * @ingroup    ALP
  *
  * ALP is a library, not a task!
  *
  * The functions implemented in this file are for "main ALP."  Individual
  * protocol processing routines are implemented in other files named alp...c,
  * which should be in the same directory as this alp_main.c.
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>

#include <otlib/alp.h>
#include <otlib/ndef.h>
#include <otlib/memcpy.h>

#include <app/build_config.h>


#if ((OT_FEATURE(ALP) == ENABLED) && (OT_FEATURE(SERVER) == ENABLED))

#define ALP_FUNCTIONS       5

#if ALP(FILE_MGR)
#   define ALP_FILESYSTEM   1
#else
#   error "Filesystem must be supported"
#endif

#if (OT_FEATURE(SENSORS) == ENABLED)
#   define ALP_SENSORS      1
#else
#   define ALP_SENSORS      0
#endif

#define ALP_SECURITY        0
#if ALP(SECURE_MGR)
#   if ((OT_FEATURE(NL_SECURITY) == ENABLED) || (OT_FEATURE(DLL_SECURITY) == ENABLED))
#       undef ALP_SECURITY
#       define ALP_SECURITY 1
#   endif
#endif

#if ALP(LOGGER)
#   undef ALP_LOGGER
#   if (OT_FEATURE(LOGGER) == ENABLED)
#       define ALP_LOGGER   1
#   else
#       define ALP_LOGGER   0
#   endif
#endif

#if ALP(DASHFORTH)
#   undef ALP_DASHFORTH
#   if (OT_FEATURE(DASHFORTH) == ENABLED)
#       define ALP_DASHFORTH 1
#   else
#       define ALP_DASHFORTH 0
#   endif
#endif

#if (OT_FEATURE(ALPEXT) == ENABLED)
#   define ALP_EXT      1
#else
#   define ALP_EXT      0
#endif 





// Subroutines
ot_u8   sub_get_headerlen(ot_u8 tnf);
void    sub_insert_header(alp_tmpl* alp, ot_qcur hdr_position, ot_u8 hdr_len);




ot_u8 sub_get_headerlen(ot_u8 tnf) {
#if (OT_FEATURE(NDEF) == ENABLED)
    ot_u8 hdr_len;
    hdr_len     = 4;                        // Pure ALP
    hdr_len    += ((tnf == 5) << 1);        // Add Type len & ID len Fields
    hdr_len    -= (tnf == 6);               // Add Type len, Remove ID len & 2 byte ID
    return hdr_len;
#else
    return 4;
#endif
}



void sub_insert_header(alp_tmpl* alp, ot_qcur hdr_position, ot_u8 hdr_len) {
/// <LI> Add hdr_len to the queue length (cursors are already in place). </LI>
/// <LI> If using NDEF (hdr_len != 4), output header processing is ugly. </LI>
/// <LI> Pure ALP (hdr_len == 4) output header processing is universal. </LI>
/// <LI> Finally, always clear MB because now the first record is done. </LI>
    ot_qcur savedput = alp->outq->putcursor + hdr_len;
    
    ///@todo make sure this works, might need to have qcur be signed
    if (hdr_position != NULL) {
        alp->outq->putcursor = hdr_position;
    }

#   if (OT_FEATURE(NDEF) == ENABLED)
    if (hdr_len != 4) {
        q_writebyte(alp->outq, alp->OUTREC(FLAGS));     //Flags byte (always)
        q_writebyte(alp->outq, 0);                      //Type Len (NDEF only)
        q_writebyte(alp->outq, alp->OUTREC(PLEN));      //Payload len
        
        if (alp->OUTREC(FLAGS) & ALP_FLAG_MB) {
            alp->OUTREC(FLAGS)  &= ~(NDEF_IL | 7);
            alp->OUTREC(FLAGS)  |= TNF_Unchanged;       // Make next record "Unchanged"
            q_writebyte(alp->outq, 2);                  //NDEF ID Len
            q_writebyte(alp->outq, alp->OUTREC(ID));    //ALP-ID    (ID 1)
            q_writebyte(alp->outq, alp->OUTREC(CMD));   //ALP-CMD   (ID 2)
        }
    }
    else
#   else
    {
        q_writelong_be(alp->outq, alp->OUTREC(FLAGS));
        //ot_memcpy(hdr_position, &alp->OUTREC(FLAGS), 4);
    }
#   endif

    alp->outq->putcursor = savedput;
    alp->OUTREC(FLAGS)  &= ~ALP_FLAG_MB;
}







/** Protocol Processors <BR>
 * ========================================================================<BR>
 * The Null Processor is implemented here.  The rest of the processors are
 * implemented in separate C files, named alp_...c
 */
OT_WEAK ot_bool alp_proc_null(alp_tmpl* a0, const id_tmpl* a1) {
    return True;   // Atomic, with no payload data
}

#if (OT_FEATURE(ALPEXT) != ENABLED)
OT_WEAK ot_bool alp_ext_proc(alp_tmpl* alp, const id_tmpl* user_id) { return True; }
#endif





/** Internal Module Routines <BR>
 * ========================================================================<BR>
 */

typedef struct {
    alp_fn      callback;
    ot_queue*   appq;
} alp_elem_t;

static const alp_elem_t null_elem = { &alp_proc_null, NULL };

#if (ALP_EXT)
static const alp_elem_t ext_elem = { &alp_ext_proc, NULL };
#endif

///@todo If this gets much bigger, it's better to have a binary tree ID search
///      in order to conserve ROM/RAM space.
static const alp_elem_t alp_table[ALP_FUNCTIONS] = {
#if ALP_FILESYSTEM
    { &alp_proc_filedata, NULL },
#endif
#if ALP_SENSORS
    { &alp_proc_sensor, NULL },
#else
    { &alp_proc_null, NULL },
#endif
#if ALP_SECURITY
    { &alp_proc_security, NULL },
#else
    { &alp_proc_null, NULL },
#endif
#if ALP_LOGGER
    { &alp_proc_logger, NULL },
#else
    { &alp_proc_null, NULL },
#endif
#if ALP_DASHFORTH
    { &alp_proc_dashforth, NULL },
#else
    { &alp_proc_null, NULL },
#endif
};



static const alp_elem_t* sub_get_elem(ot_u8 alp_id) {
    const alp_elem_t* element;
    
    if (alp_id == 0) {
        element = &null_elem;
    }
    else {
        alp_id--;

#       if (ALP_API)
        if (alp_id >= 0x80) {
            alp_id -= (0x80 - (ALP_FUNCTIONS-ALP_API));
        }
#       endif
        if (alp_id >= ALP_FUNCTIONS) {
#           if (ALP_EXT)
            element = &ext_elem;
#           else
            element = &null_elem;
#           endif
        }
        else {
            element = &(alp_table[alp_id]);
        }
    }
    
    return (const alp_elem_t*)element;
}


#ifndef EXTF_alp_proc
OT_WEAK ot_bool alp_proc(alp_tmpl* alp, const id_tmpl* user_id) {
    const alp_elem_t*   proc_elem;
    ot_bool             output_code;
    
    // Always flush payload length of output before any data is written
    alp->OUTREC(PLEN) = 0;

    /// Get the processing element based on the input record ID
    proc_elem = sub_get_elem(INREC(alp, ID)); //sub_get_elem(alp->INREC(ID));

    /// If an application queue exists:
    /// <LI>Empty it on Message-Begin set</LI>
    /// <LI>Store payload to it in any case</LI>
    if (proc_elem->appq != NULL) {
        if (INREC(alp, FLAGS) & ALP_FLAG_MB) {
        //if (alp->inq->getcursor[0] & ALP_FLAG_MB) {
            q_empty(proc_elem->appq);
        }
        
        q_movedata(proc_elem->appq, alp->inq, INREC(alp, PLEN));
        //q_writestring(proc_elem->appq, alp->inq->getcursor, alp->INREC(PLEN));        // attempt 1
        //q_writestring(proc_elem->appq, alp->inq->getcursor, alp->inq->getcursor[1]);  // attempt 0
    }
    
    /// If the Message-End flag is set, then run the processor callback
    if (INREC(alp, FLAGS) & ALP_FLAG_ME) {
    //if (alp->inq->getcursor[0] & ALP_FLAG_ME) {
        output_code = proc_elem->callback(alp, user_id);
    }
    else {
        output_code = True;
    }

    ///@todo Presently responses are immediate.  This could be fixed later.
    alp->OUTREC(FLAGS) |= ALP_FLAG_ME;

    // Return 0 length (False) or non-zero length (True)
    return output_code;
}
#endif




/** Externally Callable Routines <BR>
 * ========================================================================<BR>
 */

#ifndef EXTF_alp_init
void alp_init(alp_tmpl* alp, ot_queue* inq, ot_queue* outq) {
    alp->purge_id       = 1;
    alp->OUTREC(FLAGS)  = (ALP_FLAG_MB | ALP_FLAG_ME | ALP_FLAG_SR);   ///@todo this will need to be removed soon
    alp->inq            = inq;
    alp->outq           = outq;
}
#endif


#ifndef EXTF_alp_load_retval
///@note alp_load_retval() is something of a special-purpose routine for a
/// legacy API.  Very likely it will be refactored.

OT_WEAK ot_bool alp_load_retval(alp_tmpl* alp, ot_u16 retval) {
/// This function is for writing a two-byte integer to the return record.  It
/// is useful for some types of API return sequences
    ot_bool respond = (ot_bool)(alp->OUTREC(CMD) & 0x80);

    if (respond)  {
      //alp->OUTREC(FLAGS)          &= ~ALP_FLAG_CF;
        alp->OUTREC(PLEN) = 2;
        alp->OUTREC(CMD)    |= 0x40;
        q_writeshort(alp->outq, retval);
    }

    return respond;
}
#endif



#ifndef EXTF_alp_notify
void alp_notify(alp_tmpl* alp, ot_sig callback) {
    ///@todo To be completed when transformation of ALP is complete
}
#endif


void alp_add_app(alp_tmpl* alp, ot_u8 alp_id, alp_fn callback, ot_queue appq) {
///@todo To be completed when transformation of ALP is complete
}



ALP_status alp_parse_message(alp_tmpl* alp, const id_tmpl* user_id) {
    ALP_status  exit_code;
    ot_qcur     input_position;
    ot_qcur     hdr_position;
    //ot_u8       hdr_len;
    ot_int      proc_output;
    //ot_int      bytes;

    /// Lock the ot_queues while ALP is parsing/processing
    q_lock(alp->inq);
    q_lock(alp->outq);

    /// Safety check: make sure both queues have room remaining for the
    /// most minimal type of message, an empty message
    if ((q_readspace(alp->inq) < 4) || (q_writespace(alp->outq) < 4)) {
        exit_code = MSG_Null;
        goto alp_parse_message_END;
    }

    /// Load a new input record only when the last output record has the
    /// "Message End" flag set.  Therefore, it was the last record of a
    /// previous message.  If new input record header does not match
    /// OpenTag requirement, bypass it and go to the next.  Else, copy
    /// the input record to the output record.  alp_proc() will adjust
    /// the output payload length and flags, as necessary.
    if (alp->OUTREC(FLAGS) & ALP_FLAG_ME) {
        alp->OUTREC(FLAGS)  = q_getcursor_val(alp->inq, 0);
        alp->OUTREC(PLEN)   = 0;
        alp->OUTREC(ID)     = q_getcursor_val(alp->inq, 2);
        alp->OUTREC(CMD)    = q_getcursor_val(alp->inq, 3);
    }
    input_position          = alp->inq->getcursor;
    alp->inq->getcursor    += 4;

    ///@todo transform output creation part to a separate function call the
    ///      application should use when building response messages.  That
    ///      function should handle chunking.

    /// Reserve space in alp->outq for header data.  It is updated later.
    /// The flags and payload length are determined by processing, so this
    /// method is necessary.
    hdr_position            = alp->outq->putcursor;
    alp->outq->putcursor   += 4;

    /// ALP Proc must write appropriate data to alp->outrec, and it must
    /// make sure not to overrun the output queue.  It must write:
    /// <LI> NDEF_CF if the output record is chunking </LI>
    /// <LI> NDEF_ME if the output record is the last in the message </LI>
    /// <LI> The output record payload length </LI>
    proc_output = alp_proc(alp, user_id);
    if (alp->OUTREC(PLEN) == 0) {
        // Remove header and any output data if no data written
        // Also, remove output chunking flag
        alp->outq->putcursor   = hdr_position;
        alp->OUTREC(FLAGS)    &= ~NDEF_CF;
    }
    else {
        memcpy(hdr_position, &alp->OUTREC(FLAGS), 4);
        alp->OUTREC(FLAGS)  &= ~ALP_FLAG_MB;
    }

    /// The input record, now treated, shall be rewound
    alp->inq->putcursor = input_position;
    alp->inq->getcursor = input_position;
    
    /// Unlock the ot_queues after ALP is parsing/processing
    q_unlock(alp->inq);
    q_unlock(alp->outq);
    
    exit_code = MSG_End;

    alp_parse_message_END:
    return exit_code;
}







/** Functions Under Review <BR>
  * ========================================================================<BR>
  * These are legacy functions.  They might get bundled into different
  * functions, changed, or removed.
  */

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

/// @note This function is used by the logger (logger.c), but nowhere
/// else.  The ability to create a new output record/message is required, but
/// the method of doing it may likely get re-architected.
OT_WEAK void alp_new_record(alp_tmpl* alp, ot_u8 flags, ot_u8 payload_limit, ot_int payload_remaining) {
    // Clear control flags (begin, end, chunk)
	// Chunk and End will be intelligently set in this function, but Begin must
	// be set by the caller, AFTER this function.
	alp->OUTREC(FLAGS) |= flags;
	alp->OUTREC(FLAGS) |= NDEF_SR;
#   if (OT_FEATURE(NDEF))
	alp->OUTREC(FLAGS) &= ~(ALP_FLAG_ME | ALP_FLAG_CF | NDEF_IL);
#   else
    alp->OUTREC(FLAGS) &= (ALP_FLAG_MB | NDEF_SR);
#   endif

    // NDEF TNF needs to be 5 (with ID) on MB=1 and 6 (no ID) or MB=0
	// Pure ALP should always have IL=0 and TNF=0
#   if (OT_FEATURE(NDEF))
    if (alp->OUTREC(FLAGS) & 7) {
        alp->OUTREC(FLAGS) &= ~7;
        alp->OUTREC(FLAGS) |= (alp->OUTREC(FLAGS) & ALP_FLAG_MB) ? (NDEF_IL+5) : 6;
    }
#   endif

	// Automatically set Chunk or End.
	// "payload_remaining" is re-purposed to contain the number of bytes loaded
	// Chunk Flag is ignored by pure-ALP
	if (payload_remaining > payload_limit) {
		payload_remaining   = payload_limit;
#       if (OT_FEATURE(NDEF))
		alp->OUTREC(FLAGS)  |= ALP_FLAG_CF;
#       endif
	}
	else {
		alp->OUTREC(FLAGS)  |= ALP_FLAG_ME;
	}

	alp->OUTREC(PLEN) = (ot_u8)payload_remaining;
	sub_insert_header(alp, NULL, sub_get_headerlen(alp->OUTREC(FLAGS)&7));
}


#if 0

/** @brief  Thread-safe function for loading data into ALP input
  * @param  alp         (alp_tmpl*) ALP I/O control structure
  * @param  src         (ot_u8*) source data to load into ALP
  * @param  length      (ot_int) Number of bytes to load from source into ALP
  * @retval ot_bool     True if load was possible, false if not
  * @ingroup ALP
  * @sa alp_is_available
  */
#ifndef EXTF_alp_load
ot_bool alp_load(alp_tmpl* alp, ot_u8* src, ot_int length) {
    if (alp_is_available(alp, length)) {
        alp->inq->options.ubyte[0] = 1;
        q_writestring(alp->inq, src, length);
        alp->inq->options.ubyte[0] = 0;
        return True;
    }
    return False;
}
#endif



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
#ifndef EXTF_alp_break
/// @note This function is used in a presently-disabled block of code for
/// parsing multiframe M2DP streams.  That whole piece of functionality --
/// multiframe M2DP -- is getting rearchitected.  alp_break() is likely to be
/// removed in future versions of OpenTag
OT_WEAK void alp_break(alp_tmpl* alp) {
/// Break a running stream, and manually put a break record onto the stream
#if (OT_FEATURE(NDEF) == ENABLED)
    ot_u8 tnf;
    tnf = (alp->OUTREC(FLAGS) & 7);
#else
#   define tnf 0
#endif
    alp->OUTREC(FLAGS)   = (ALP_FLAG_MB | ALP_FLAG_ME | NDEF_SR | NDEF_IL) | tnf;
    alp->OUTREC(PLEN) = 0;
    alp->OUTREC(ID)      = 0;
    alp->OUTREC(CMD)     = 0;

    sub_insert_header(alp, NULL, sub_get_headerlen(tnf));
}
#endif


void sub_rebase_on_purged(alp_tmpl* alp, ot_queue* appq) {
    if ((alp->purge_id == 0) || (alp->purge_id != appq->options.ushort)) {
        appq->options.ushort = alp->purge_id;
        q_rebase(appq, alp->inq->front);
    }
}





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
#ifndef EXTF_alp_new_appq
void alp_new_appq(alp_tmpl* alp, ot_queue* appq) {
    //DEBUG_ASSERT

    appq->options.ushort= alp->purge_id;
    appq->alloc         = alp->inq->alloc;

    ///@note old -4 before alp_parse_header was fully refactored
    //appq->front         = alp->inq->getcursor - 4;
    appq->front         = alp->inq->getcursor;

    // This section will stay the same even after alp_parse_header is fully refactored
    appq->getcursor     = appq->front;
    appq->back          = appq->front + appq->front[1] + 4;
    appq->putcursor     = appq->back;

    //alp_parse_message() will manage this, no need to do it here
    //alp->inq->getcursor = appq->back;
}
#endif




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
#ifndef EXTF_alp_append_appq
void alp_append_appq(alp_tmpl* alp, ot_queue* appq) {
    ot_u8* next_alp;
    //DEBUG_ASSERT
    
    ///@todo this is added for the hack presently used with alp_purge()
    sub_rebase_on_purged(alp, appq);
    
    ///@note old offset until alp_parse_header is fully refactored
    //next_alp = alp->inq->getcursor + alp->inq->getcursor[-3];
    next_alp = alp->inq->getcursor + alp->inq->getcursor[1] + 4;
    
    ///@todo I'm not sure if putcursor matters for appq
    if (next_alp <= alp->inq->putcursor) {
        appq->putcursor = next_alp;
    }
}
#endif





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
#ifndef EXTF_alp_goto_next
ot_u8 alp_goto_next(alp_tmpl* alp, ot_queue* appq, ot_u8 target) {
    //DEBUG_ASSERT
    
    /// 1. If the alp queue has been purged in between operations of this app,
    ///    the app queue must be rebased.  If the alp purge id has done one lap
    ///    we also need to do this just to maintain synchronicity.
    sub_rebase_on_purged(alp, appq);
    
    /// 2. Find the next record with matching ID in the alp queue, and set the
    ///    app queue getcursor accordingly.
    while ((alp->inq->putcursor - appq->back) > 0) {
        if ((appq->getcursor[0] != 0) && (appq->getcursor[2] == target)) {
            return appq->getcursor[0];
        }
        appq->getcursor = appq->back;
        appq->back      = appq->getcursor + appq->getcursor[1] + 4;
        //appq->putcursor = appq->back;
    }
    //q_rebase(appq, appq->inq->front);     //not needed due to event-driven input model
    return 0;
}
#endif



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
#ifndef EXTF_alp_retrieve_record
ot_u8* alp_retrieve_record(alp_record* apprec, ot_queue* appq, ot_u8 target) {
    ///@todo If multiple interfaces share a single ALP, I should add a new function
    ///      called "alp_retrieve_next" that basically just calls alp_goto_next
    ///      followed by alp_retrieve_record.
    
    memcpy((ot_u8*)&apprec->flags, appq->getcursor, 4);
    
    if (apprec->id != target) {
        return NULL;
    }
    appq->front         = appq->getcursor;
    appq->getcursor[0]  = ALP_FLAG_WORKING;                   // Mark as working
    appq->getcursor    += 4;
    appq->back          = appq->getcursor + apprec->plength;
    
    return appq->front;
}
#endif



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
#ifndef EXTF_alp_release_record
void alp_release_record(ot_queue* appq) {
    appq->front[0] = 0;         // Mark as finished so alp_purge() will clear it
}
#endif


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
#ifndef EXTF_alp_kill
void alp_kill(alp_tmpl* alp, ot_u8 kill_id) {
    ot_u8*  cursor;
    
    for (cursor=alp->inq->front; cursor<alp->inq->putcursor; cursor+=(4+cursor[1])) {
        if (cursor[2] == kill_id) {
            cursor[0] = 0;
        }
    }
    alp_purge(alp);
}
#endif



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
#ifndef EXTF_alp_purge
void alp_purge(alp_tmpl* alp) {
    ot_u8* acursor;
    ot_u8* bcursor;
    ot_u8* ccursor;
    ot_int total_purge_bytes;
    
#   if (OT_FEATURE(NDEF))
#       warning "NDEF not yet supported for non-atomic alps"
#   endif
    
    ///@todo this is a temporary hack
    q_empty(alp->inq);
    
    /*
     /// 1. An ALP processor that has non-atomic handling ability must mark all
     ///    record flags to 0, after that record is processed.  In the special
     ///    case where all records are marked to 0, just empty the queue.  In
     ///    the special case where no records are markeds to 0, bail out.
     {   ot_u8 other_recs    = 0;
     ot_u8 marked_recs   = 0;
     ot_u8* cursor;
     
     for (cursor=alp->inq->front; cursor<alp->inq->putcursor; cursor+=(4+cursor[1])) {
     other_recs |= cursor[0];
     if ((cursor[0] | marked_recs) == 0) {   // first marked record
     marked_recs = 1;
     acursor     = cursor;
     }
     }
     if (other_recs == 0) {
     q_empty(alp->inq);
     alp->purge_id++;
     return;
     }
     if (marked_recs == 0) {
     return;
     }
     }
     
     
     ///@note This doesn't quite work and it needs further testing
     
     
     /// 2. Progress through ot_queue, purging targeted records by shifting later
     /// contents over them.  "acursor" has been set, in part 1, to the first
     /// instance of a record marked for purging.
     total_purge_bytes   = 0;
     bcursor             = acursor;
     
     alp_purge_PURGELOOP:
     while (1) {
     ot_int move_bytes;
     ot_int purge_bytes;
     
     // Go past consecutive instances of records marked for purging.
     while ((bcursor[0] == 0) && (bcursor < alp->inq->putcursor))  {
     bcursor += (4 + bcursor[1]);
     }
     
     // Go past consecutive instances of records not to be deleted.
     // If we find a working record (i.e. an app is using it now), it must
     // stay in the same location, it cannot be moved.
     ccursor = bcursor;
     while ((ccursor[0] != 0) && (ccursor < alp->inq->putcursor)) {
     ot_u8 scratch = ccursor[0];
     ccursor += (4 + ccursor[1]);
     
     if (scratch == ALP_FLAG_WORKING) {
     while ((ccursor[0] == ALP_FLAG_WORKING) && (ccursor < alp->inq->putcursor)) {
     ccursor += (4 + ccursor[1]);
     }
     bcursor = ccursor;
     acursor = ccursor;
     goto alp_purge_PURGELOOP;
     }
     }
     
     // getcursor management:
     // - if getcursor is less than acursor, leave alone
     // - else if getcursor is between acursor and bcursor, set to acursor
     // - else (getcursor > bcursor) subtract purge_bytes.
     purge_bytes         = (bcursor-acursor);
     total_purge_bytes  += purge_bytes;
     if (alp->inq->getcursor > acursor) {
     alp->inq->getcursor -= purge_bytes;
     if (alp->inq->getcursor < acursor)
     alp->inq->getcursor = acursor;
     }
     
     // If there is data to move, shift the records that are not to be
     // purged over the ones being purged.
     move_bytes = (ccursor-bcursor);
     if (move_bytes != 0) {
     ot_u8*  dst;
     dst     = acursor;
     acursor+= move_bytes;
     memcpy(dst, bcursor, move_bytes);
     }
     else {
     acursor = ccursor;
     }
     
     // The next loop starts looking for dead records where the last loop
     // left-off.  acursor is set (above) to the place where the next group
     // of records can be moved.
     bcursor = ccursor;
     }
     
     ///3. If any bytes have been purged, the app queues must be notified.  We
     ///   do this by incrementing the purge counter, which is compared against
     ///   a value in the app queue options field in alp_goto_next().
     alp->purge_id       += (total_purge_bytes != 0);
     
     ///4. Finally, retract the alp queue putcursor by purged bytes.
     alp->inq->putcursor -= total_purge_bytes;
     
     */
}
#endif







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
#ifndef EXTF_alp_is_available
ot_bool alp_is_available(alp_tmpl* alp, ot_int length) {
    ot_bool is_available;
    
    // Check if input queue is presently owned by some other app
    if (alp->inq->options.ubyte[0] != 0) {
        return False;
    }
    
    // Check if there is sufficient space available.  If not, try purging the
    // input and check again.
    is_available = (q_space(alp->inq) >= length);
    if (is_available == False) {
        alp_purge(alp);
        is_available = (q_space(alp->inq) >= length);
    }
    return is_available;
}
#endif


/** @brief  Internal function for resolving the index of an app processor
  * @param  alp_id      (ot_u8) ALP ID
  * @retval ot_u8       index of the processor routine
  * @ingroup ALP
  * @sa alp_proc
  *
  *
  *//** @brief  Internal function for resolving the index of an app processor
  * @param  alp_id      (ot_u8) ALP ID
  * @retval ot_u8       index of the processor routine
  * @ingroup ALP
  * @sa alp_proc
  *
  *
  */
#ifndef EXTF_alp_get_handle
OT_WEAK ot_u8 alp_get_handle(ot_u8 alp_id) {
#   if (ALP_API)
    if (alp_id >= 0x80) {
        alp_id -= (0x80-(ALP_FUNCTIONS-ALP_API));
    }
#   endif
    if (alp_id > (ALP_FUNCTIONS+1)) {
        alp_id = (ALP_FUNCTIONS+1);
    }
    return alp_id;
}
#endif


// if 0
#endif




// #if ((OT_FEATURE(ALP) == ENABLED) && (OT_FEATURE(SERVER) == ENABLED))
#endif 
