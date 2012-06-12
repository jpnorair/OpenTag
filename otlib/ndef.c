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
  * @file       /OTlib/ndef.c
  * @author     JP Norair
  * @version    V1.0
  * @date       5 Apr 2012
  * @brief      NDEF encapsulation protocol (typically used for wireline messaging)
  * @ingroup    NDEF (NDEF Module)
  *
  * NOTE: This code module is being re-architected.  At the moment, the only
  * functions that get used are:
  * <LI> otapi_ndef_idle()      </LI>
  * <LI> otapi_ndef_proc()      </LI>
  * <LI> ndef_parse_record()    </LI>
  * 
  ******************************************************************************
  */

#include "OT_config.h"

#if (OT_FEATURE(NDEF) == ENABLED)
#include "OTAPI.h"
#include "buffers.h"
#include "ndef.h"
#include "mpipe.h"
#include "auth.h"


// NDEF Module data
ndef_message ndef;



/***************************
  * NDEF Local Subroutines *
  **************************/
  
ot_bool sub_put_header(alp_record* record, Queue* q);








/** OTAPI functions within NDEF module      <BR>
  * ========================================================================<BR>
  * Use the MPipe to move NDEF packets in and out.  These functions should be
  * attached to the MPipe TXDONE callback (otapi_ndef_idle) and the RXDONE 
  * callback (otapi_ndef_proc) if you are building an NDEF server command pipe.
  */

#ifndef EXTF_otapi_ndef_idle
void otapi_ndef_idle(ot_int code) {
	ndef.last_flags = NDEF_MB | NDEF_SR | NDEF_IL | NDEF_TNF_UNKNOWN;
    q_empty(&dir_in);
    mpipe_rxndef(dir_in.front, False, MPIPE_Low);
}
#endif

#ifndef EXTF_otapi_ndef_proc
void otapi_ndef_proc(ot_int code) {
    switch (ndef_parse_record(&dir_in, &dir_out)) {
        //wipe queue and go back to idle listening
        case MSG_Null:          otapi_ndef_idle(0);
                                break;
        
        //listen for next record/message
        case MSG_Chunking_In:   mpipe_rxndef(dir_in.getcursor, False, MPIPE_High);
                                break;
        
        //transmit next record/message
        case MSG_Chunking_Out:
        case MSG_End:           mpipe_txndef(dir_out.front, False, MPIPE_High);
                                break;
    }
}
#endif






/*************************
  * NDEF OTlib Functions *
  ************************/
#define HEADER_LENGTH   6

ot_bool sub_put_header(alp_record* record, Queue* q) {
    if ((q->putcursor + HEADER_LENGTH) >= q->back)
        return False;

    /// Write record flags and Type Length (Type Length always 0)
    *q->putcursor++ = record->flags;
    *q->putcursor++ = 0;
    
    /// This driver requires short records
    *q->putcursor++ = (ot_u8)record->payload_length;
    
    /// Write ID, length (2) + 2 bytes
    *q->putcursor++ = 2;
    *q->putcursor++ = record->dir_id;
    *q->putcursor++ = record->dir_cmd;
    
    return True;
}



#ifndef EXTF_ndef_new_msg
ot_bool ndef_new_msg(Queue* output) {
    if (output == NULL) {
        return False;
    }
    
    output->putcursor   = output->front;
    output->getcursor   = output->front;
    ndef.last_flags     = NDEF_TNF_EMPTY;
    //ndef.msg_tnf        = NDEF_TNF_EMPTY;       // this is the null setting
    //ndef.msg_records    = 0;
    return True;
}
#endif



#ifndef EXTF_ndef_new_record
ot_bool ndef_new_record(alp_record* record, ot_u8* record_data, Queue* output) {
    ot_bool test;

    if ((record == NULL) || (output == NULL))
        return False;

    /// 1. Clear MB, SR, IL, TNF of record (safety)
    /// 2. Set message begin on first record in message
    /// 3. Set TNF and IL in case where this is not following a chunk
    record->flags  &= ~(NDEF_MB | NDEF_SR | NDEF_IL | NDEF_TNF_RESERVED);
    record->flags  |= (ndef.last_flags == 0) << 7;
    record->flags  |= (ndef.last_flags & NDEF_CF) ? NDEF_TNF_UNCHANGED : \
                                                    (NDEF_IL | NDEF_TNF_UNKNOWN); 
    
    /// 4. Load header to queue (check if space is available)
    /// 5. Copy Payload to the queue, if payload is provided.
    test = sub_put_header(record, output);
    if (test && (record_data != NULL)) {
        output->putcursor += HEADER_LENGTH;
        q_writestring(output, record_data, (ot_int)record->payload_length);
    }
    
    return test;
}
#endif




#ifndef EXTF_ndef_send_msg
ot_int ndef_send_msg(Queue* output) {
    return mpipe_txndef(output->front, False, lo_priority);
}
#endif



#ifndef EXTF_ndef_load_msg
ot_bool ndef_load_msg(Queue* input) {
    ndef.last_flags = NDEF_TNF_EMPTY;
    if (*input->getcursor & NDEF_ME) {
        input->getcursor = input->front;
    }
    else {
        input->getcursor += input->getcursor[2];
    }
    return (ot_bool)(input != NULL);
}
#endif



ot_u8 sub_parse_header(alp_record* in_rec, Queue* in_q) {
    ot_u8 rec_tnf;
    ot_u8 rec_typelen;
    ot_u8 rec_idlen;
    ot_u8 ignore;

    /// Load up flags, type length, and store them for later parsing
    in_rec->flags   = q_readbyte(in_q);
    rec_typelen     = q_readbyte(in_q);
    rec_tnf         = in_rec->flags & 0x07;
    ignore          = (rec_tnf != NDEF_TNF_UNKNOWN) & \
                        (rec_tnf != NDEF_TNF_UNCHANGED);
    
    /// Load up the payload length (must be one byte)
    in_rec->payload_length = (ot_u8)q_readbyte(in_q);
    if ((in_rec->flags & NDEF_SR) == 0) {
        ignore++;
        in_q->getcursor += 3;
    }

    /// Load up ID length (1 byte) must be present
    rec_idlen = (in_rec->flags & NDEF_IL) ? q_readbyte(in_q) : 0;

    /// Bypass type field (always ignored)
    in_q->getcursor += rec_typelen;
    
    /// Record is ignored if ID length != 2, 
    /// unless TNF == UNCHANGED && ID length == 0
    if (rec_idlen == 2) {
        in_rec->dir_id  = q_readbyte(in_q);
        in_rec->dir_cmd = q_readbyte(in_q);
    }
    else {
        ignore          += (rec_tnf != NDEF_TNF_UNCHANGED);
        ignore          += (rec_idlen != 0);
        in_q->getcursor += rec_idlen;
    }
    
    return ignore;
}






/** ndef_parse_record()         <BR>
  * ========================================================================<BR>
  * This function is a major part of the OpenTag NDEF module, which is being 
  * re-architected.  Expect more optimizations and possibly more small changes.
  * One thing to note is that record chunking is still quite experimental.  It
  * would be a bad idea to use it in a production environment, using this code.
  *
  * When the re-architecting is done, many of the functions currently in this
  * Module will probably get removed, but this one is sure to stay.
  */
#ifndef EXTF_ndef_parse_record
NDEF_status ndef_parse_record(Queue* in_q, Queue* out_q) {
#if (OT_FEATURE(ALP) == ENABLED)
    alp_record in_rec;
    alp_record out_rec;
    ot_u8 error;
    
    /// Parse the new record header
    error = sub_parse_header(&in_rec, in_q);
    
    //if Message End is not present, skip processing
    if ((in_rec.flags & NDEF_ME) == 0) {
        ndef.last_flags     = NDEF_MB | NDEF_SR | NDEF_IL | NDEF_TNF_UNKNOWN;
        in_q->getcursor    += in_rec.payload_length;
        return MSG_Chunking_In;
    }
    
    //if an error, ignore
    else if (error != 0) {
        q_empty(in_q);
        return MSG_Null;
    }
    
    /// Else, Message End is present, so process the Message!
    /// This will need to be updated when chunking is enabled.
    else {
        q_empty(out_q);
        out_q->back    -= mpipe_footerbytes();
        in_q->getcursor = in_q->front;
        out_rec.flags   = ndef.last_flags;
        
        /// Loop through records in the input message
        do {
            ot_int initial_length;
            sub_parse_header(&in_rec, in_q);
        
            /// Tentatively write header data.  It will be updated later.
            out_rec.flags     |= (in_rec.flags & NDEF_ME);
            out_q->getcursor    = out_q->putcursor;
            out_q->putcursor   += HEADER_LENGTH;
            initial_length      = out_q->length;

            alp_proc(&in_rec, &out_rec, in_q, out_q, AUTH_ROOT);
            
            /// If there's no output data, rewind output queue to remove
            /// the parts added by sub_put_header().  If there is output
            /// data, put it on the output message queue.
            if (out_q->length == initial_length) {
                out_q->putcursor  = out_q->getcursor;
            }
            else {
                if (out_rec.flags & NDEF_CF) {
                    out_rec.flags &= ~NDEF_ME;
                }
                *out_q->getcursor++ = out_rec.flags;
                out_rec.flags      &= ~NDEF_MB;
                *out_q->getcursor++ = 0;
                *out_q->getcursor++ = out_rec.payload_length;
                *out_q->getcursor++ = 2;
                *out_q->getcursor++ = out_rec.dir_id;
                *out_q->getcursor   = out_rec.dir_cmd;
                out_q->length      += HEADER_LENGTH;
            }
            
            ndef.last_flags = out_rec.flags;
            
            /// Check the boundary to see if chunk-out needs to happen
            if ( out_q->putcursor >= out_q->back ) {
                return MSG_Chunking_Out;
            }
        }
        while ((out_rec.flags & NDEF_ME) == 0);
        
        return (out_q->length == 0) ? MSG_Null : MSG_End;
    }
#else
    return MSG_Null;
#endif
}
#endif






#endif


