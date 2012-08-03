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
  * @file       /otlib/alp_main.c
  * @author     JP Norair
  * @version    V1.0
  * @date       20 July 2012
  * @brief      Application Layer Protocol Main Processor
  * @ingroup    ALP
  *
  * Application Layer protocols (ALP's) are directive-based protocols that
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

#include "OTAPI.h"
#include "alp.h"
#include "ndef.h"
#include "OT_platform.h"

#if ((OT_FEATURE(ALP) == ENABLED) && (OT_FEATURE(SERVER) == ENABLED))

#define ALP_FILESYSTEM  1
#define ALP_SENSORS     (OT_FEATURE(SENSORS) == ENABLED)
#define ALP_SECURITY    ((OT_FEATURE(NL_SECURITY) == ENABLED) || (OT_FEATURE(DLL_SECURITY) == ENABLED))
#define ALP_LOGGER      (LOG_FEATURE(ANY) == ENABLED)
#define ALP_DASHFORTH   (OT_FEATURE(DASHFORTH) == ENABLED)
#define ALP_API         ((OT_FEATURE(ALPAPI) == ENABLED)*3)
#define ALP_EXT         (OT_FEATURE(ALPEXT) == ENABLED)


#define ALP_MAX         9
#define ALP_FUNCTIONS   (   ALP_FILESYSTEM \
                          + ALP_SENSORS \
                          + ALP_SECURITY \
                          + ALP_LOGGER \
                          + ALP_DASHFORTH \
                          + ALP_API )


typedef ot_bool (*sub_proc)(alp_tmpl*, id_tmpl*);




ot_bool sub_proc_null(alp_tmpl* a0, id_tmpl* a1) {
	return False;
}




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



void sub_insert_header(alp_tmpl* alp, ot_u8* hdr_position, ot_u8 hdr_len) {
/// <LI> Add hdr_len to the queue length (cursors are already in place). </LI>
/// <LI> If using NDEF (hdr_len != 4), output header processing is ugly. </LI>
/// <LI> Pure ALP (hdr_len == 4) output header processing is universal. </LI>
/// <LI> Finally, always clear MB because now the first record is done. </LI>
    if (hdr_position == NULL) {
        hdr_position            = alp->outq->putcursor;
        alp->outq->putcursor   += hdr_len;
    }
    alp->outq->length  += hdr_len;
    
#   if (OT_FEATURE(NDEF) == ENABLED)
    if (hdr_len != 4) {
        *hdr_position++ = alp->outrec.flags;        //Flags byte (always)
        *hdr_position++ = 0;                        //Type Len (NDEF only)
        *hdr_position++ = alp->outrec.plength;      //Payload len
        if (alp->outrec.flags & ALP_FLAG_MB) {
            alp->outrec.flags  &= ~(NDEF_IL | 7);
            alp->outrec.flags  |= TNF_Unchanged;    // Make next record "Unchanged"
            *hdr_position++     = 2;                //NDEF ID Len
            *hdr_position++     = alp->outrec.id;   //ALP-ID    (ID 1)
            *hdr_position++     = alp->outrec.cmd;  //ALP-CMD   (ID 2)
        }
    }
    else
#   else
        platform_memcpy(hdr_position, &alp->outrec.flags, 4);
#   endif

    alp->outrec.flags  &= ~ALP_FLAG_MB;     
}




#ifndef EXTF_alp_break
void alp_break(alp_tmpl* alp) {
/// Break a running stream, and manually put a break record onto the stream
#if (OT_FEATURE(NDEF) == ENABLED)
    ot_u8 tnf;
    tnf = (alp->outrec.flags & 7);
#else
#   define tnf 0
#endif
    alp->outrec.flags   = (ALP_FLAG_MB | ALP_FLAG_ME | NDEF_SR | NDEF_IL) | tnf;
    alp->outrec.plength = 0;
    alp->outrec.id      = 0;
    alp->outrec.cmd     = 0;
    
    sub_insert_header(alp, NULL, sub_getheader_len(tnf));
    
}
#endif




#ifndef EXTF_alp_new_record
void alp_new_record(alp_tmpl* alp, ot_u8 flags, ot_u8 payload_limit, ot_int payload_remaining) {
    // Clear control flags (begin, end, chunk)
	// Chunk and End will be intelligently set in this function, but Begin must
	// be set by the caller, AFTER this function.
	alp->outrec.flags |= flags;
#   if (OT_FEATURE(NDEF))
	alp->outrec.flags &= ~(ALP_FLAG_ME | ALP_FLAG_CF | NDEF_IL);
#   else
    alp->outrec.flags &= ALP_FLAG_MB;
#   endif
	alp->outrec.flags |= NDEF_SR;

    // NDEF TNF needs to be 5 (with ID) on MB=1 and 6 (no ID) or MB=0
	// Pure ALP should always have IL=0 and TNF=0
#   if (OT_FEATURE(NDEF))
    if (alp->outrec.flags & 7) {
        alp->outrec.flags &= ~7;
        alp->outrec.flags |= (alp->outrec.flags & ALP_FLAG_MB) ? \
                                                        (NDEF_IL+5) : 6;
    }
#   endif

	// Automatically set Chunk or End.
	// "payload_remaining" is re-purposed to contain the number of bytes loaded
	// Chunk Flag is ignored by pure-ALP
	if (payload_remaining > payload_limit) {
		payload_remaining   = payload_limit;
#       if (OT_FEATURE(NDEF))
		alp->outrec.flags  |= ALP_FLAG_CF;
#       endif
	}
	else {
		alp->outrec.flags  |= ALP_FLAG_ME;
	}

	alp->outrec.plength = (ot_u8)payload_remaining;
	sub_insert_header(alp, NULL, sub_get_headerlen(alp->outrec.flags&7));
}
#endif



#ifndef EXTF_alp_new_message
void alp_new_message(alp_tmpl* alp, ot_u8 payload_limit, ot_int payload_remaining) {
/// Prepare the flags and payload length for the first record in a message
	alp_new_record(alp, ALP_FLAG_MB, payload_limit, payload_remaining);
}
#endif




#ifndef EXTF_alp_parse_message
ALP_status alp_parse_message(alp_tmpl* alp, id_tmpl* user_id) {
/// @note OpenTag may not completely support record chunking.  This function
///       should be able to support input and output chunking, but the ALP
///       processors themselves might not.  Don't assume that OpenTag supports
///       chunking until this message goes away.

#if (OT_FEATURE(ALP) == ENABLED)
    ALP_status exit_code = MSG_Null;
    
    /// Loop through records in the input message.  Each input message can
    /// generate 0 or 1 output messages.
    do {
        ot_u8*  hdr_position;
        ot_u8   hdr_len;
        
        /// Safety check: make sure both queues have room remaining for the
        /// most minimal type of message, an empty message
        if (((alp->inq->back - alp->inq->getcursor) < 6) || \
            ((alp->outq->back - alp->outq->putcursor) < 6)) {
            break;
        }
        
        /// Load a new input record only when the last output record is not
        /// a "chunk" record.  If the input record header does not match
        /// OpenTag requirement, bypass it and go to the next.  Else, copy
        /// the input record to the output record.  alp_proc() will adjust
        /// the output payload length and flags, as necessary.
        if ((alp->outrec.flags & (ALP_FLAG_MB|ALP_FLAG_ME)) == 0) {
            if (alp_parse_header(alp) == False) {
                return MSG_Null;
            }
            platform_memcpy(&alp->outrec.flags, &alp->inrec.flags, 4);
        }
                   
        /// Reserve space in alp->outq for header data.  It is updated later.
        /// The flags and payload length are determined by processing, so this
        /// method is necessary.
        hdr_len                 = sub_get_headerlen(alp->outrec.flags & 7);
        hdr_position            = alp->outq->putcursor;
        alp->outq->putcursor   += hdr_len;
        
        /// ALP Proc must write appropriate data to alp->outrec, and it must
        /// make sure not to overrun the output queue.  It must write:
        /// <LI> NDEF_CF if the output record is chunking </LI>
        /// <LI> NDEF_ME if the output record is the last in the message </LI>
        /// <LI> The output record payload length </LI>
        if (alp_proc(alp, user_id) == False) {
            // Remove header and any output data if return is false
            // Also, remove output chunking flag
            alp->outq->putcursor   = hdr_position;
            alp->outrec.flags     &= ~NDEF_CF;
        }
        else {
            sub_insert_header(alp, hdr_position, hdr_len);
        }
            
        /// alp_parse_message() should return to caller when:
        /// <LI> Output and Input Messages are done (MSG_End) </LI>
        /// <LI> Output data is exceeding the output queue (MSG_Chunking_Out) </LI>
        /// <LI> Input data is exceeding the input queue (MSG_Chunking_In) </LI>
        /// <LI> There is an error, or no data (MSG_Null) </LI>
        { 
            register ot_u8 scratch = MSG_Null;
            
            if (alp->inrec.flags & NDEF_CF) 
                scratch = MSG_Chunking_In;
            if (alp->outrec.flags & NDEF_CF) 
                scratch = MSG_Chunking_Out;
            else if (alp->outrec.flags & alp->inrec.flags & NDEF_ME) 
                scratch = MSG_End;
                
            exit_code = (ALP_status)scratch;
        }
    }
    while (exit_code != MSG_End);
        
    return exit_code;
#else
    return MSG_Null;
#endif
}
#endif




#ifndef EXTF_alp_parse_header
ot_bool alp_parse_header(alp_tmpl* alp) {
#if (OT_FEATURE(NDEF) != ENABLED)
    ot_bool tnf_is_valid;
    tnf_is_valid = (ot_bool)((alp->inrec.flags & 7) == 0);

    if (tnf_is_valid) {
        platform_memcpy(&alp->inrec.flags, alp->inq->getcursor, 4);
    }
    return tnf_is_valid;

#else
    // Load Flags byte (universal)
    alp->inrec.flags = q_readbyte(alp->inq);
    
    // Check for valid framing
    // Being strict here should help against hacking
    if (((alp->inrec.flags & (NDEF_MB+NDEF_SR+NDEF_IL+7)) == (NDEF_MB+NDEF_SR+NDEF_IL+5)) || \
        ((alp->inrec.flags & (NDEF_MB+NDEF_SR+NDEF_IL+7)) == (NDEF_SR+6)) || \
        ((alp->inrec.flags & (NDEF_SR+NDEF_IL+7)) == (NDEF_SR+0)) ) {
        
        alp->inrec.plength = q_readbyte(alp->inq);
        
        // IL==1 only implicitly when TNF==Unknown and MB==1
        if (alp->inrec.flags & NDEF_IL) {
            if (q_readbyte(alp->inq) != 2) {
                goto alp_parse_header_FAIL;
            }
        }
        
        // Update the id & cmd when TNF == 0 or 5.  The top-level if statement
        // ensures that only TNF = {0, 5, 6}
        if ((alp->inrec.flags & 2) == 0) {
            alp->inrec.id   = q_readbyte(alp->inq);
            alp->inrec.cmd  = q_readbyte(alp->inq);
        }

        return True;
    }
    
    alp_parse_header_FAIL:
    return False;

#endif
}
#endif




#ifndef EXTF_alp_load_retval
ot_bool alp_load_retval(alp_tmpl* alp, ot_u16 retval) {
/// This function is for writing a two-byte integer to the return record.  It
/// is useful for some types of API return sequences
    ot_bool respond = (ot_bool)(alp->outrec.cmd & 0x80);

    if (respond)  {
      //alp->outrec.flags          &= ~ALP_FLAG_CF;
        alp->outrec.plength = 2;
        alp->outrec.cmd    |= 0x40;
        q_writeshort(alp->outq, retval);
    }
    
    return respond;
}
#endif




#ifndef EXTF_alp_proc
ot_bool alp_proc(alp_tmpl* alp, id_tmpl* user_id) {
    static const sub_proc proc[] = {
        &sub_proc_null,
#   if (ALP_FILESYSTEM)
        &alp_proc_filedata,
#   endif
#   if (ALP_SENSORS)
        &sub_proc_null,         //Not implemented yet
#   endif
#   if (ALP_SECURITY)
        &sub_proc_null,         //Not implemented yet
#   endif
#   if (ALP_LOGGER)
        &alp_proc_logger,
#   endif
#   if (ALP_DASHFORTH)
        &sub_proc_null,         //Not implemented yet
#   endif
#   if (ALP_API)
        &alp_proc_api_session,
        &alp_proc_api_system,
        &alp_proc_api_query,
#   endif
#   if (ALP_EXT)
        &otapi_alpext_proc,
#   else
        &sub_proc_null
#   endif
    };

    ot_u8 alp_id;
    
    // Always flush payload length of output before any data is written
    alp->outrec.plength = 0;
    
    /// <LI> IDs in range 0-to-ALP_FUNCTIONS are standardized, or psuedo-standardized </LI>
    /// <LI> IDs in range 128-to-(128+ALP_API) are mapped OTAPI functions </LI>
    /// <LI> IDs outside this range get pushed to ALP_EXT </LI>
    alp_id = alp->inrec.id;
    
    if (alp_id >= 0x80) {
        alp_id -= (0x80-(ALP_FUNCTIONS-ALP_API));
    }
    if (alp_id > (ALP_FUNCTIONS+1)) {
        alp_id = (ALP_FUNCTIONS+1);
    }
    
    /// The proc function must set alp->outrec.plength based on how much
    /// data it writes to the output queue.  It must return False if the output
    /// should be canceled.
    alp_id = (ot_u8)proc[alp_id](alp, user_id);
    
    /// If the output bookmark is non-Null, there is output chunking.  Else, 
    /// the output message is complete (ended)
    ///@todo Output bookmark is not implemented yet in any ALP.
    alp->outrec.flags   &= ~ALP_FLAG_ME;
    alp->outrec.flags   |= (alp->outrec.bookmark) ? ALP_FLAG_CF : ALP_FLAG_ME;
}
#endif


#endif

