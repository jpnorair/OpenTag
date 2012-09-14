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
  * @file       /otlib/alp_api_server.c
  * @author     JP Norair
  * @version    V1.0
  * @date       20 July 2012
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

typedef ot_u16 (*otapi_icmd)(Queue*);           // Irregular form routine
typedef ot_u16 (*otapi_cmd)(ot_u8*, void*);     // Standard form routine (used in M2QP)
typedef void (*sub_bdtmpl)(Queue*, void*);

ot_u16 icmd_session_number(Queue* in_q);
ot_u16 icmd_session_flush(Queue* in_q);
ot_u16 icmd_session_isblocked(Queue* in_q);

ot_u16 icmd_sys_init(Queue* in_q);
ot_u16 icmd_sys_newsession(Queue* in_q);
ot_u16 icmd_sys_openrequest(Queue* in_q);
ot_u16 icmd_sys_closerequest(Queue* in_q);
ot_u16 icmd_sys_startflood(Queue* in_q);
ot_u16 icmd_sys_startdialog(Queue* in_q);

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
void sub_breakdown_udp_tmpl(Queue* in_q, void* data_type);
void sub_breakdown_isfcomp_tmpl(Queue* in_q, void* data_type);
void sub_breakdown_isfcall_tmpl(Queue* in_q, void* data_type);


ot_u16 icmd_session_number(Queue* in_q) {
    return otapi_session_number();
}

ot_u16 icmd_session_flush(Queue* in_q) {
    return otapi_flush_sessions();
}

ot_u16 icmd_session_isblocked(Queue* in_q) {
    ot_u8 channel_id;
    sub_breakdown_u8(in_q, &channel_id);
    return otapi_is_session_blocked(channel_id);
}

ot_u16 icmd_sys_init(Queue* in_q) {
    return otapi_sysinit();
}

ot_u16 icmd_sys_newsession(Queue* in_q) {
    session_tmpl new_session;
    sub_breakdown_session_tmpl(in_q, &new_session);
    return otapi_new_session(&new_session, NULL);
}

ot_u16 icmd_sys_openrequest(Queue* in_q) {
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
    return otapi_open_request( (addr_type)addr_byte, (routing_tmpl*)routing );
}

ot_u16 icmd_sys_closerequest(Queue* in_q) {
    return otapi_close_request();
}

ot_u16 icmd_sys_startflood(Queue* in_q) {
    ot_u16 flood_duration;
    sub_breakdown_u16(in_q, &flood_duration);
    return otapi_start_flood(flood_duration);
}

ot_u16 icmd_sys_startdialog(Queue* in_q) {
    return otapi_start_dialog();
}




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

void sub_breakdown_udp_tmpl(Queue* in_q, void* data_type) {
    ot_int udp_data_length;
    ((udp_tmpl*)data_type)->src_port      = q_readbyte(in_q);
    ((udp_tmpl*)data_type)->dst_port      = q_readbyte(in_q);
    udp_data_length                       = q_readbyte(in_q);
    ((udp_tmpl*)data_type)->data_length   = udp_data_length;
    ((udp_tmpl*)data_type)->data          = q_markbyte(in_q, udp_data_length);
}

void sub_breakdown_isfcomp_tmpl(Queue* in_q, void* data_type) {
    ((isfcomp_tmpl*)data_type)->is_series   = q_readbyte(in_q);
    ((isfcomp_tmpl*)data_type)->isf_id      = q_readbyte(in_q);
    ((isfcomp_tmpl*)data_type)->offset      = q_readshort(in_q);
}

void sub_breakdown_isfcall_tmpl(Queue* in_q, void* data_type) {
    sub_breakdown_isfcomp_tmpl(in_q, data_type);
    ((isfcall_tmpl*)data_type)->max_return  = q_readshort(in_q);
}



static const sub_bdtmpl bdtmpl_cmd[15] = {
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
    &sub_breakdown_udp_tmpl     //14
};

static const otapi_icmd session_cmd[3] = {
	(otapi_icmd)&icmd_session_number,
    (otapi_icmd)&icmd_session_flush,
    (otapi_icmd)&icmd_session_isblocked
};

static const otapi_icmd system_cmd[6] = {
	(otapi_icmd)&icmd_sys_init,
	(otapi_icmd)&icmd_sys_newsession,
	(otapi_icmd)&icmd_sys_openrequest,
	(otapi_icmd)&icmd_sys_closerequest,
	(otapi_icmd)&icmd_sys_startflood,
	(otapi_icmd)&icmd_sys_startdialog
};

static const otapi_cmd m2qp_cmd[11] = {
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
    (otapi_cmd)&otapi_put_udp_tmpl         //14
};





ot_bool sub_proc_api_irregular(alp_tmpl* alp, id_tmpl* user_id, otapi_icmd* cmd, ot_u8 cmd_limit) {
    ot_u16  retval;
    ot_u8   respond     = (alp->inrec.cmd & 0x80);
    ot_u8   lookup_cmd  = (alp->inrec.cmd & ~0x80) - 1;
    
    /// Do boundary check, and make sure caller is root
    if ( (lookup_cmd >= cmd_limit) || (auth_isroot(user_id) == False) )
        return False;
        
    /// Lookup and process the irregular command, supplying it from input queue
    retval = cmd[lookup_cmd](alp->inq);
    
    /// Write back the twobye retval integer when response is enabled
    return alp_load_retval(alp, retval);
}


ot_bool alp_proc_api_session(alp_tmpl* alp, id_tmpl* user_id ) {   
    return sub_proc_api_irregular(alp, user_id, (otapi_icmd*)session_cmd, 3);
}


ot_bool alp_proc_api_system(alp_tmpl* alp, id_tmpl* user_id ) {   
    return sub_proc_api_irregular(alp, user_id, (otapi_icmd*)system_cmd, 6);
}



ot_bool alp_proc_api_query(alp_tmpl* alp, id_tmpl* user_id ) {
/// The M2QP API calls follow the rules that future extensions to the API shall
/// abide, apart from special cases which *must* be cleared by the developer
/// community prior to becoming official.
/// The form is: ot_u16 otapi_function(ot_u8*, void*)
    static const ot_u8 argmap[OTAPI_M2QP_FUNCTIONS] = \
        { 5, 8, 9, 10, 11, 12, 13, 13, 3, 3, 14 };

    //sub_bdtmpl  get_tmpl;
    ot_u8   dt_buf[24];   // 24 bytes is a safe amount, although less might suffice
    ot_u16  txq_len;
    ot_u8   status;
    ot_u8   lookup_cmd      = (alp->inrec.cmd & ~0x80) - 1;
    ot_bool respond         = (ot_bool)(alp->inrec.cmd & 0x80);
    
    //alp->outrec.plength = 0;
    
    if ( (lookup_cmd >= OTAPI_M2QP_FUNCTIONS) || (auth_isroot(user_id) == False) )
        return False;
    
    /// Load template from ALP dir cmd into C datatype
    bdtmpl_cmd[argmap[lookup_cmd]](alp->inq, (void*)dt_buf);
    
    /// Run ALP command, using input template
    txq_len = m2qp_cmd[lookup_cmd](&status, (void*)dt_buf);
    
    /// Response to ALP query command includes three bytes:
    /// byte 1 - status (0 is error)
    /// bytes 2 & 3 - 16 bit integer, length of TXQ
    if (respond) {
        alp->outrec.flags   &= ~ALP_FLAG_CF;
        alp->outrec.cmd     |= 0x40;
        alp->outrec.plength  = 3;
        q_writebyte(alp->outq, status);
        q_writeshort(alp->outq, txq_len);
    }
    
    return respond;
}












#endif


