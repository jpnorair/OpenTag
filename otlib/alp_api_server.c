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
  * @file       /OTlib/alp_api_server.c
  * @author     JP Norair
  * @version    V1.0
  * @date       08 July 2011
  * @brief      Application Layer Protocol for API calls
  * @ingroup    ALP
  *
  * The OpenTag API includes several functions for dispatching directives.  Each
  * directive must be written to a linear array/string in order to be used by
  * OpenTag.  These directives are transferred by Mpipe or, potentially, DASH7.
  *
  * The ALP parsing implementation for API calls makes extensive usage of 
  * function tables.  Each ALP directive contains and ID (OTAPI module ID) and 
  * CMD (OTAPI function ID), which together constitue a function vector.  The 
  * vector breaks-down the function parameters and calls the corresponding C-API
  * function.
  *
  ******************************************************************************
  */

#include "alp.h"

#if (   (OT_FEATURE(SERVER) == ENABLED) \
     && (OT_FEATURE(ALP) == ENABLED) \
     && (OT_FEATURE(ALPAPI) == ENABLED) )

#include "OTAPI_c.h"
#include "auth.h"


#define F_OFFSET1   0
#define F_OFFSET2   OTAPI_SYSTEM_FUNCTIONS
#define F_OFFSET3   (F_OFFSET2 + OTAPI_SESSION_FUNCTIONS)
#define F_TOTAL     (F_OFFSET3 + OTAPI_M2QP_FUNCTIONS)


typedef enum {
    sysindex_null           = 0,
    sysindex_sysinit        = 1,
    sysindex_new_session    = 2,
    sysindex_open_request   = 3,
    sysindex_close_request  = 4,
    sysindex_start_flood    = 5,
    sysindex_start_dialog   = 6
} sysindex;

typedef enum {
    sesindex_null               = 0,
    sesindex_session_number     = 1,
    sesindex_flush_sessions     = 2,
    sesindex_is_session_blocked = 3
} sesindex;





// LLDP Data Type breakdown subroutines
// (Add more when new otapi functions are added, using new data types in args)

typedef void (*sub_bdtmpl)(Queue*, void*);

void sub_breakdown_u8(Queue* in_q, void* data_type);
void sub_breakdown_u16(Queue* in_q, void* data_type);
void sub_breakdown_u32(Queue* in_q, void* data_type);
void sub_breakdown_queue(Queue* in_q, void* data_type);
void sub_breakdown_session_tmpl(Queue* in_q, void* data_type);
void sub_breakdown_command_tmpl(Queue* in_q, void* data_type);
void sub_breakdown_id_tmpl(Queue* in_q, void* data_type);
void sub_breakdown_routing_tmpl(Queue* in_q, void* data_type);
void sub_breakdown_dialog_tmpl(Queue* in_q, void* data_type);
void sub_breakdown_query_tmpl(Queue* in_q, void* data_type);
void sub_breakdown_ack_tmpl(Queue* in_q, void* data_type);
void sub_breakdown_error_tmpl(Queue* in_q, void* data_type);   //Client only? 
void sub_breakdown_shell_tmpl(Queue* in_q, void* data_type);
void sub_breakdown_isfcomp_tmpl(Queue* in_q, void* data_type);
void sub_breakdown_isfcall_tmpl(Queue* in_q, void* data_type);


// Standard form routine
typedef ot_u16 (*otapi_cmd)(ot_u8*, void*);


void sub_breakdown_u8(Queue* in_q, void* data_type) {
    *(ot_u8*)data_type = q_readbyte(in_q); 
}

void sub_breakdown_u16(Queue* in_q, void* data_type) {
    *(ot_u16*)data_type = q_readshort(in_q);
}

void sub_breakdown_u32(Queue* in_q, void* data_type) {
    *(ot_u32*)data_type = q_readlong(in_q);
}

void sub_breakdown_queue(Queue* in_q, void* data_type) {
    ot_u16 queue_length;
    ot_u8* queue_front;
    queue_length                    = q_readshort(in_q);
    ((Queue*)data_type)->alloc      = queue_length;
    ((Queue*)data_type)->options    = in_q->options;
    ((Queue*)data_type)->length     = queue_length;
    queue_front                     = q_markbyte(in_q, queue_length);
    ((Queue*)data_type)->front      = queue_front;
    ((Queue*)data_type)->back       = queue_front+queue_length;
    ((Queue*)data_type)->getcursor  = queue_front;
    ((Queue*)data_type)->putcursor  = queue_front;
}

void sub_breakdown_session_tmpl(Queue* in_q, void* data_type) {
    ((session_tmpl*)data_type)->channel     = q_readbyte(in_q);
    ((session_tmpl*)data_type)->subnet      = q_readbyte(in_q);
    ((session_tmpl*)data_type)->subnetmask  = q_readbyte(in_q);
    ((session_tmpl*)data_type)->flags       = q_readbyte(in_q);
    ((session_tmpl*)data_type)->flagmask    = q_readbyte(in_q);
    ((session_tmpl*)data_type)->timeout     = q_readshort(in_q);
}

void sub_breakdown_command_tmpl(Queue* in_q, void* data_type) {
    ((command_tmpl*)data_type)->type        = q_readbyte(in_q);
    ((command_tmpl*)data_type)->opcode      = q_readbyte(in_q);
    ((command_tmpl*)data_type)->extension   = q_readbyte(in_q);
}

void sub_breakdown_routing_tmpl(Queue* in_q, void* data_type) {
    ot_u8 id_length;
    ot_u8 code_mask;
    ((routing_tmpl*)data_type)->hop_code = q_readbyte(in_q);
    
    if (((routing_tmpl*)data_type)->hop_code > 1) {
        ((routing_tmpl*)data_type)->hop_ext     = q_readbyte(in_q);
        code_mask                               = (((routing_tmpl*)data_type)->hop_ext != 0) << 7;
        id_length                               = q_readbyte(in_q);
        ((routing_tmpl*)data_type)->orig.length = id_length;
        code_mask                              |= (id_length != 0) << 6;
        code_mask                              |= (id_length == 2) << 4;
        ((routing_tmpl*)data_type)->orig.value  = q_markbyte(in_q, id_length);
        id_length                               = q_readbyte(in_q);
        ((routing_tmpl*)data_type)->dest.length = id_length;
        code_mask                              |= (id_length != 0) << 5;
        code_mask                              |= (id_length == 2) << 4;
        ((routing_tmpl*)data_type)->dest.value  = q_markbyte(in_q, id_length);
        ((routing_tmpl*)data_type)->hop_code   |= code_mask;
    }
}

void sub_breakdown_id_tmpl(Queue* in_q, void* data_type) {
    ot_int id_length;
    id_length                       = q_readbyte(in_q);
    ((id_tmpl*)data_type)->length   = id_length;
    ((id_tmpl*)data_type)->value    = q_markbyte(in_q, id_length);
}

void sub_breakdown_dialog_tmpl(Queue* in_q, void* data_type) {
    ((dialog_tmpl*)data_type)->timeout = q_readbyte(in_q);
    
    if (((dialog_tmpl*)data_type)->timeout & 0x80) {
        ((dialog_tmpl*)data_type)->channels = q_readbyte(in_q);
        ((dialog_tmpl*)data_type)->chanlist = \
            q_markbyte(in_q, ((dialog_tmpl*)data_type)->channels);
    }
}

void sub_breakdown_query_tmpl(Queue* in_q, void* data_type) {
    ot_u8   query_length;
    ot_u8   query_code;
    ot_u8*  query_mask;
    
    query_code      = q_readbyte(in_q);
    query_length    = q_readbyte(in_q);
    query_mask      = NULL;
    
    if (query_code & 0x80) {
        query_mask  = q_markbyte(in_q, query_length);
    }
    
    ((query_tmpl*)data_type)->code      = query_code;
    ((query_tmpl*)data_type)->length    = query_length;
    ((query_tmpl*)data_type)->mask      = query_mask;
    ((query_tmpl*)data_type)->value     = q_markbyte(in_q, query_length);
}


void sub_breakdown_ack_tmpl(Queue* in_q, void* data_type) {
    ot_int ack_id_count;
    ot_int ack_id_length;
    ack_id_count                    = q_readbyte(in_q);
    ack_id_length                   = q_readbyte(in_q);
    ((ack_tmpl*)data_type)->count   = (ot_u8)ack_id_count;
    ((ack_tmpl*)data_type)->length  = (ot_u8)ack_id_length;
    ((ack_tmpl*)data_type)->list    = q_markbyte(in_q, ack_id_count*ack_id_length);
}

void sub_breakdown_error_tmpl(Queue* in_q, void* data_type) {
    ((error_tmpl*)data_type)->code      = q_readbyte(in_q);
    ((error_tmpl*)data_type)->subcode   = q_readbyte(in_q);
    ((error_tmpl*)data_type)->data      = in_q->getcursor;  ///@todo build a routine for code:subcode --> data length
}

void sub_breakdown_shell_tmpl(Queue* in_q, void* data_type) {
    ot_int shell_data_length;
    ((shell_tmpl*)data_type)->req_port      = q_readbyte(in_q);
    ((shell_tmpl*)data_type)->resp_port     = q_readbyte(in_q);
    shell_data_length                       = q_readbyte(in_q);
    ((shell_tmpl*)data_type)->data_length   = shell_data_length;
    ((shell_tmpl*)data_type)->data          = q_markbyte(in_q, shell_data_length);
}

void sub_breakdown_isfcomp_tmpl(Queue* in_q, void* data_type) {
    ((isfcomp_tmpl*)data_type)->is_series   = q_readbyte(in_q);
    ((isfcomp_tmpl*)data_type)->isf_id      = q_readbyte(in_q);
    ((isfcomp_tmpl*)data_type)->offset      = q_readshort(in_q);
}

void sub_breakdown_isfcall_tmpl(Queue* in_q, void* data_type) {
    ((isfcall_tmpl*)data_type)->is_series   = q_readbyte(in_q);
    ((isfcall_tmpl*)data_type)->isf_id      = q_readbyte(in_q);
    ((isfcall_tmpl*)data_type)->max_return  = q_readshort(in_q);
    ((isfcall_tmpl*)data_type)->offset      = q_readshort(in_q);
}











void alp_proc_api_session(alp_record* in_rec, alp_record* out_rec,
                                Queue* in_q, Queue* out_q, id_tmpl* user_id ) {
/// @note Usage of Session functions via API
/// Treatment of session functions via ALP is not really supported, and this
/// Function is probably useless for now.  OTAPI session usage is primarily for
/// C-API and DASHForth API users.

/// Session functions are of special form, so the parsing is programmatic.  
/// Standard form is ot_u16 otapi_function(ot_u8*, void*).
    ot_u16 retval;
    ot_bool respond         = (ot_bool)(in_rec->dir_cmd & 0x80);
    out_rec->payload_length = 0;

    if ( (in_rec->dir_cmd > 3) || (!auth_isroot(user_id)) )
        return;
    
    switch ( in_rec->dir_cmd ) {
        case sesindex_null:
            return;
            
        case sesindex_session_number:
            retval = otapi_session_number();
            break;
        
        case sesindex_flush_sessions:
            retval = otapi_flush_sessions();
            break;
            
        case sesindex_is_session_blocked: {
            ot_u8 channel_id;
            sub_breakdown_u8(in_q, &channel_id);
            retval = otapi_is_session_blocked(channel_id);
            break;
        }
    }
    
    /// Write back the twobye retval integer when response is enabled
    alp_load_retval(respond, (in_rec->dir_cmd | 0x40), retval, out_rec, out_q);
}





void alp_proc_api_system(alp_record* in_rec, alp_record* out_rec,
                                Queue* in_q, Queue* out_q, id_tmpl* user_id ) {
/// System functions are of special form, so the parsing is programmatic.
/// Standard form is ot_u16 otapi_function(ot_u8*, void*).

    sysindex function_code;
    ot_u16  retval;
    
    ot_bool respond         = (ot_bool)(in_rec->dir_cmd & 0x80);
    in_rec->dir_cmd        &= 7;
    function_code           = (sysindex)in_rec->dir_cmd;
    out_rec->payload_length = 0;
    
    if ( (function_code > 6) || (!auth_isroot(user_id)) )
        return;
    
    switch ( function_code ) {
        case sysindex_null: 
            return;
        
        case sysindex_sysinit:
            retval = otapi_sysinit();
            break;
        
        case sysindex_new_session: {
            session_tmpl new_session;
            sub_breakdown_session_tmpl(in_q, &new_session);
            retval = otapi_new_session(&new_session);
            break;
        }
        
        case sysindex_open_request: {
            ot_u8 addr_byte;
            ot_u8 routing[sizeof(routing_tmpl)];
            sub_breakdown_u8(in_q, &addr_byte);
            
            // Use routing_tmpl for unicast or anycast and additionally grab
            // target id for unicast
            if ((addr_byte & 0x40) == 0) {
                if ((addr_byte & 0x80) == 0) {
                    sub_breakdown_id_tmpl(in_q, &((routing_tmpl*)routing)->dlog);
                }
                sub_breakdown_routing_tmpl(in_q, routing);
            }
            retval = otapi_open_request( (addr_type)addr_byte, (routing_tmpl*)routing );
            break;
        }
        
        case sysindex_close_request: 
            retval = otapi_close_request();
            break;
        
        case sysindex_start_flood: {
            ot_u16 flood_duration;
            sub_breakdown_u16(in_q, &flood_duration);
            retval = otapi_start_flood(flood_duration);
            break;
        }
        
        case sysindex_start_dialog:
            retval = otapi_start_dialog();
            break;
    }
    
    /// Write back the twobyte retval integer when response is enabled
    alp_load_retval(respond, (in_rec->dir_cmd | 0x40), retval, out_rec, out_q);
}




void alp_proc_api_query(alp_record* in_rec, alp_record* out_rec,
                                Queue* in_q, Queue* out_q, id_tmpl* user_id ) {
/// The M2QP API calls follow the rules that future extensions to the API shall
/// abide, apart from special cases which *must* be cleared by the developer
/// community prior to becoming official.
/// The form is: ot_u16 otapi_function(ot_u8*, void*)
    static const ot_u8 argmap[OTAPI_M2QP_FUNCTIONS] = \
        { 5, 8, 9, 10, 11, 12, 13, 13, 3, 3, 14 };

    //sub_bdtmpl  get_tmpl;
    static const sub_bdtmpl tmpl[15] = {
        &sub_breakdown_u8,            //0
        &sub_breakdown_u16,           //1
        &sub_breakdown_u32,           //2
        &sub_breakdown_queue,         //3
        &sub_breakdown_session_tmpl,  //4
        &sub_breakdown_command_tmpl,  //5
        &sub_breakdown_id_tmpl,       //6
        &sub_breakdown_routing_tmpl,  //7
        &sub_breakdown_dialog_tmpl,   //8
        &sub_breakdown_query_tmpl,    //9
        &sub_breakdown_ack_tmpl,      //10
        &sub_breakdown_error_tmpl,    //11
        &sub_breakdown_isfcomp_tmpl,  //12
        &sub_breakdown_isfcall_tmpl,  //13
        &sub_breakdown_shell_tmpl     //14
    };

    //otapi_cmd   cmd;
    static const otapi_cmd cmd[11] = {
        (otapi_cmd)&otapi_put_command_tmpl,      //5
        (otapi_cmd)&otapi_put_dialog_tmpl,       //8
        (otapi_cmd)&otapi_put_query_tmpl,        //9
        (otapi_cmd)&otapi_put_ack_tmpl,          //10
        (otapi_cmd)&otapi_put_error_tmpl,        //11
        (otapi_cmd)&otapi_put_isf_comp,          //12
        (otapi_cmd)&otapi_put_isf_call,          //13
        (otapi_cmd)&otapi_put_isf_return,        //13
        (otapi_cmd)&otapi_put_reqds,             //3
        (otapi_cmd)&otapi_put_propds,            //3
        (otapi_cmd)&otapi_put_shell_tmpl         //14
    };

    ot_u8   dt_buf[24];   // 24 bytes is a safe amount, although less might suffice
    ot_u16  txq_len;
    ot_u8   status;
    ot_u8   lookup_cmd      = (in_rec->dir_cmd & ~0x80) - 1;
    ot_bool respond         = (ot_bool)(in_rec->dir_cmd & 0x80);
    out_rec->payload_length = 0;
    
    if ( (lookup_cmd >= OTAPI_M2QP_FUNCTIONS) || (auth_isroot(user_id) == False) )
        return;
    
    /// Load template from ALP dir cmd into C datatype
    /*
    switch (argmap[lookup_cmd]) {
        case 0:     get_tmpl = &sub_breakdown_u8;               break;
        case 1:     get_tmpl = &sub_breakdown_u16;              break;
        case 2:     get_tmpl = &sub_breakdown_u32;              break;
        case 3:     get_tmpl = &sub_breakdown_queue;            break;
        case 4:     get_tmpl = &sub_breakdown_session_tmpl;     break;
        case 5:     get_tmpl = &sub_breakdown_command_tmpl;     break;
        case 6:     get_tmpl = &sub_breakdown_id_tmpl;          break;
        case 7:     get_tmpl = &sub_breakdown_routing_tmpl;     break;
        case 8:     get_tmpl = &sub_breakdown_dialog_tmpl;      break;
        case 9:     get_tmpl = &sub_breakdown_query_tmpl;       break;
        case 10:    get_tmpl = &sub_breakdown_ack_tmpl;         break;
        case 11:    get_tmpl = &sub_breakdown_error_tmpl;       break;
        case 12:    get_tmpl = &sub_breakdown_isfcomp_tmpl;     break;
        case 13:    get_tmpl = &sub_breakdown_isfcall_tmpl;     break;
        case 14:    get_tmpl = &sub_breakdown_shell_tmpl;       break;
    }
    get_tmpl(in_q, (void*)dt_buf);
    */
    tmpl[argmap[lookup_cmd]](in_q, (void*)dt_buf);
    
    /// Run ALP command, using input template
    /*
    switch (lookup_cmd) {
        case 0: 	cmd = &otapi_put_command_tmpl;				break; 	//5
        case 1:     cmd = &otapi_put_dialog_tmpl;               break;	//8
        case 2:     cmd = &otapi_put_query_tmpl;                break;	//9
        case 3:     cmd = &otapi_put_ack_tmpl;                  break;	//10
        case 4:     cmd = &otapi_put_error_tmpl;                break;	//11
        case 5:     cmd = &otapi_put_isf_comp;                  break;	//12
        case 6:     cmd = &otapi_put_isf_call;                  break; 	//13
        case 7:     cmd = &otapi_put_isf_return;                break; 	//13
        case 8:     cmd = &otapi_put_reqds;                     break;	//3
        case 9:     cmd = &otapi_put_propds;                    break;	//3
        case 10:    cmd = &otapi_put_shell_tmpl;                break;	//14
    }
    txq_len = cmd(&status, (void*)dt_buf);
    */
    txq_len = cmd[lookup_cmd](&status, (void*)dt_buf);
    
    /// Response to ALP query command includes three bytes:
    /// byte 1 - status (0 is error)
    /// bytes 2 & 3 - 16 bit integer, length of TXQ
    if (respond) {
        out_rec->flags         &= ~ALP_FLAG_CF;
        out_rec->dir_cmd        = in_rec->dir_cmd | 0x40;
        out_rec->payload_length = 3;
        q_writebyte(out_q, status);
        q_writeshort(out_q, txq_len);
    }
}












#endif


