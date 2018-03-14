/* Copyright 2013 JP Norair
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
  * @version    R101
  * @date       25 Mar 2014
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

#include <otlib/alp.h>

#if (   (OT_FEATURE(SERVER) == ENABLED) \
     && (OT_FEATURE(ALP) == ENABLED) \
     && (OT_FEATURE(ALPAPI) == ENABLED) \
     && (OT_FEATURE(M2) == ENABLED))

#include <m2/capi.h>
#include <otlib/auth.h>


#define F_OFFSET1   0
#define F_OFFSET2   OTAPI_SYSTEM_FUNCTIONS
#define F_OFFSET3   (F_OFFSET2 + OTAPI_SESSION_FUNCTIONS)
#define F_TOTAL     (F_OFFSET3 + OTAPI_M2QP_FUNCTIONS)

typedef enum {
    sysindex_null           = 0,
    sysindex_sysinit        = 1,
    sysindex_new_dialog     = 2,
    sysindex_new_advdialog  = 3,
    sysindex_open_request   = 4,
    sysindex_close_request  = 5,
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

typedef ot_u16 (*otapi_icmd)(ot_queue*);           // Irregular form routine
typedef ot_u16 (*otapi_cmd)(ot_u8*, void*);     // Standard form routine (used in M2QP)
typedef void (*sub_bdtmpl)(ot_queue*, void*);

ot_u16 icmd_session_number(ot_queue* in_q);
ot_u16 icmd_session_flush(ot_queue* in_q);
ot_u16 icmd_session_isblocked(ot_queue* in_q);

ot_u16 icmd_sys_init(ot_queue* in_q);
ot_u16 icmd_sys_newdialog(ot_queue* in_q);
ot_u16 icmd_sys_newadvdialog(ot_queue* in_q);
ot_u16 icmd_sys_openrequest(ot_queue* in_q);
ot_u16 icmd_sys_closerequest(ot_queue* in_q);
ot_u16 icmd_sys_startdialog(ot_queue* in_q);


ot_u16 icmd_session_number(ot_queue* in_q) {
    return otapi_session_number();
}

ot_u16 icmd_session_flush(ot_queue* in_q) {
    return otapi_flush_sessions();
}

ot_u16 icmd_session_isblocked(ot_queue* in_q) {
    ot_u8 channel_id;
    alp_breakdown_u8(in_q, &channel_id);
    return otapi_is_session_blocked(channel_id);
}

ot_u16 icmd_sys_init(ot_queue* in_q) {
    return otapi_sysinit();
}

ot_u16 icmd_sys_newdialog(ot_queue* in_q) {
    session_tmpl new_session;
    alp_breakdown_session_tmpl(in_q, &new_session);
    return otapi_new_dialog(&new_session, NULL);    ///@todo put in grabber applet (?)
}

ot_u16 icmd_sys_newadvdialog(ot_queue* in_q) {
    session_tmpl    new_session;
    advert_tmpl     new_adv;
    alp_breakdown_session_tmpl(in_q, &new_session);
    alp_breakdown_advert_tmpl(in_q, &new_adv);
    return otapi_new_advdialog(&new_adv, &new_session, NULL); 
}

ot_u16 icmd_sys_openrequest(ot_queue* in_q) {
    ot_u8 addr_byte;
    ot_u8 routing[sizeof(routing_tmpl)];
    alp_breakdown_u8(in_q, &addr_byte);
    
    // Use routing_tmpl for unicast or anycast and additionally grab
    // target id for unicast
    if ((addr_byte & 0x40) == 0) {
        if ((addr_byte & 0x80) == 0) {
            alp_breakdown_id_tmpl(in_q, &((routing_tmpl*)routing)->dlog);
        }
        alp_breakdown_routing_tmpl(in_q, routing);
    }
    return otapi_open_request( (addr_type)addr_byte, (routing_tmpl*)routing );
}

ot_u16 icmd_sys_closerequest(ot_queue* in_q) {
    return otapi_close_request();
}

ot_u16 icmd_sys_startdialog(ot_queue* in_q) {
    ot_u16 timeout;
    alp_breakdown_u16(in_q, (void*)&timeout);
    return otapi_start_dialog(timeout);
}




///@todo This is only used by M2QP, so it could be truncated to only have tmpls
///      that are included in M2QP API calls.
static const sub_bdtmpl bdtmpl_cmd[16] = {
    &alp_breakdown_u8,            //0
    &alp_breakdown_u16,           //1
    &alp_breakdown_u32,           //2
    &alp_breakdown_queue,         //3
    &alp_breakdown_session_tmpl,  //4
    &alp_breakdown_advert_tmpl,   //5
    &alp_breakdown_command_tmpl,  //6
    &alp_breakdown_id_tmpl,       //7
    &alp_breakdown_routing_tmpl,  //8
    &alp_breakdown_dialog_tmpl,   //9
    &alp_breakdown_query_tmpl,    //10
    &alp_breakdown_ack_tmpl,      //11
    &alp_breakdown_error_tmpl,    //12
    &alp_breakdown_isfcomp_tmpl,  //13
    &alp_breakdown_isfcall_tmpl,  //14
    &alp_breakdown_udp_tmpl     //15
};

static const otapi_icmd session_cmd[3] = {
	(otapi_icmd)&icmd_session_number,
    (otapi_icmd)&icmd_session_flush,
    (otapi_icmd)&icmd_session_isblocked
};

static const otapi_icmd system_cmd[6] = {
	(otapi_icmd)&icmd_sys_init,
	(otapi_icmd)&icmd_sys_newdialog,
	(otapi_icmd)&icmd_sys_newadvdialog,
	(otapi_icmd)&icmd_sys_openrequest,
	(otapi_icmd)&icmd_sys_closerequest,
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
    ot_u8   rec_cmd     = alp->inq->getcursor[3];   // record command
    ot_u8   respond     = (rec_cmd & 0x80);
    ot_u8   lookup_cmd  = (rec_cmd & ~0x80) - 1;
    
    alp->inq->getcursor += 4;
    
    /// Do boundary check, and make sure caller is root
    if ( (lookup_cmd >= cmd_limit) || (auth_isroot(user_id) == False) )
        return False;
        
    /// Lookup and process the irregular command, supplying it from input queue
    retval = cmd[lookup_cmd](alp->inq);
    
    /// Write back the twobye retval integer when response is enabled
    return alp_load_retval(alp, retval);
}


OT_WEAK ot_bool alp_proc_api_session(alp_tmpl* alp, id_tmpl* user_id ) {
    return sub_proc_api_irregular(alp, user_id, (otapi_icmd*)session_cmd, 3);
}


OT_WEAK ot_bool alp_proc_api_system(alp_tmpl* alp, id_tmpl* user_id ) {
    return sub_proc_api_irregular(alp, user_id, (otapi_icmd*)system_cmd, 6);
}



OT_WEAK ot_bool alp_proc_api_query(alp_tmpl* alp, id_tmpl* user_id ) {
/// The M2QP API calls follow the rules that future extensions to the API shall
/// abide, apart from special cases which *must* be cleared by the developer
/// community prior to becoming official.
/// The form is: ot_u16 otapi_function(ot_u8*, void*)
    static const ot_u8 argmap[OTAPI_M2QP_FUNCTIONS] = \
        { 6, 9, 10, 11, 12, 13, 14, 14, 15, 0, 0 };

    //sub_bdtmpl  get_tmpl;
    ot_u8   dt_buf[24];   // 24 bytes is a safe amount, although less might suffice
    ot_u16  txq_len;
    ot_u8   status          = alp->inq->getcursor[3];    // record cmd
    ot_u8   lookup_cmd      = (status & ~0x80) - 1;
    ot_bool respond         = (ot_bool)(status & 0x80);
    
    alp->inq->getcursor    += 4;
    
    if ((lookup_cmd < OTAPI_M2QP_FUNCTIONS) && auth_isroot(user_id)) {
        /// Load template from ALP dir cmd into C datatype
        bdtmpl_cmd[argmap[lookup_cmd]](alp->inq, (void*)dt_buf);
        
        /// Run ALP command, using input template
        txq_len = m2qp_cmd[lookup_cmd](&status, (void*)dt_buf);
        
        /// Response to ALP query command includes three bytes:
        /// byte 1 - status (0 is error)
        /// bytes 2 & 3 - 16 bit integer, length of TXQ
        if (respond) {
            //alp->outrec.flags   &= ~ALP_FLAG_CF;
            //alp->outrec.cmd     |= 0x40;
            //alp->outrec.plength  = 3;
            alp->OUTREC(FLAGS)   &= ~ALP_FLAG_CF;
            alp->OUTREC(PLEN)     = 3;
            alp->OUTREC(CMD)     |= 0x40;
            q_writebyte(alp->outq, status);
            q_writeshort(alp->outq, txq_len);
        }
    }
    
    return True;
}












#endif


