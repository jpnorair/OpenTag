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
  * @file       /otlib/OTAPI_c.c
  * @author     JP Norair
  * @version    R100
  * @date       3 Oct 2013
  * @brief      OpenTag C API
  * @ingroup    OTAPI
  *
  ******************************************************************************
  */

#include "OT_types.h"
#include "OT_config.h"

#if (OT_FEATURE(CAPI) && OT_FEATURE(M2))

#include "OTAPI_tmpl.h"
#include "OTAPI_c.h"


#include "buffers.h"
#include "queue.h"



#define OTAPI_Q     rxq





#include "system.h"
#include "m2_dll.h"
#include "m2_network.h"
#include "session.h"

ot_u16 sub_session_handle(m2session* session) {
    return (session == NULL) ? 0 : *((ot_u16*)&session->channel);
}



ot_u16 otapi_sysinit() {
#   if (OT_FEATURE(M2))
    dll_refresh();
#   endif
    return 1;
}



ot_u16 otapi_new_dialog(session_tmpl* s_tmpl, void* applet) {
#if (SYS_SESSION == ENABLED)
    return sub_session_handle( otapi_task_immediate(s_tmpl, (ot_app)applet) );
#else
    return 0;
#endif
}



ot_u16 otapi_new_advdialog(advert_tmpl* adv_tmpl, session_tmpl* s_tmpl, void* applet) {
#if (SYS_FLOOD == ENABLED)
    return sub_session_handle( otapi_task_advertise(adv_tmpl, s_tmpl, (ot_app)applet) );
#else
    return 0;
#endif
}


ot_u16 otapi_open_request(addr_type addr, routing_tmpl* routing) {
/// Set the header if the session is valid.  Also conditionally write the header
/// depending on the address type (a parameter).  
    if (session_notempty()) {
        m2session* s_active;
        s_active = session_top();
        
        // Set the dll parameters to a safe setting; can be changed later
        dll_set_defaults(s_active);
    
        // Unicast/Anycast support routing, so copy the supplied template
        if ((addr & M2QUERY_GLOBAL) == 0) {   
            platform_memcpy((ot_u8*)&m2np.rt, (ot_u8*)routing, sizeof(routing_tmpl));
        }

        // Load the header
        m2np_header(s_active, (ot_u8)addr, M2FI_FRDIALOG);
        return 1;
    }
    return 0;
}


ot_u16 otapi_close_request() {
/// Set the footer if the session is valid
    if (session_notempty()) {
        m2np_footer( /* session_top() */ );
        return 1;
    }
    return 0;
}


ot_u16 otapi_start_dialog(ot_u16 timeout) {
/// Stop any ongoing processes and seed the event for the event manager.  The
/// radio killer will work in all cases, but it is bad form to kill sessions
/// that are moving data.
    if (timeout != 0) {
        dll.comm.tc = TI2CLK(timeout);
    }
    
    ///@todo update null radio driver to modern interface
#   ifndef __KERNEL_NONE__
    if (radio.state != RADIO_Idle) {
    	rm2_kill();
    }
#   endif

#   ifndef __KERNEL_NONE__
    sys.task_RFA.event = 0;
    sys_preempt(&sys.task_RFA, 0);
#   endif
    return 1;
}









#ifndef EXTF_otapi_session_number
ot_u16 otapi_session_number() {
    if (session_notempty()) {
        m2session* active = session.top;
        return *((ot_u16*)&(active->channel));
    }
    return 0;
}
#endif


#ifndef EXTF_otapi_flush_sessions
ot_u16 otapi_flush_sessions() {
    session_flush();
    return session_numfree();
}
#endif


#ifndef EXTF_otapi_is_session_blocked
ot_u16 otapi_is_session_blocked(ot_u8 chan_id) {
    return (ot_u16)session_occupied(chan_id);
}
#endif










#include "m2_transport.h"

#ifndef EXTF_otapi_put_command_tmpl
ot_u16 otapi_put_command_tmpl(ot_u8* status, command_tmpl* command) {    
    /// Check Opcodes to make sure this one is supported
    /// @todo base this on app_config.h settings.  Currently this is rudimentary
    ///       and hard-coded.  It just filters out Datastream and non-existing codes
    if (command->opcode > 15) {
        // command extension, not present at the moment
        *status = 0;
        return 0;
    }
    
    dll.comm.csmaca_params |= command->type & M2_CSMACA_A2P;
    m2qp.cmd.code           = command->type | command->opcode;
    m2qp.cmd.code          |= (command->extension != 0) << 7;
    m2qp.cmd.ext            = command->extension;
    q_writebyte(&txq, m2qp.cmd.code);
    
    if (m2qp.cmd.ext != 0) {
        q_writebyte(&txq, m2qp.cmd.ext);
    }
    
    *status = 1;
    return q_length(&txq);
}
#endif


#ifndef EXTF_otapi_put_dialog_tmpl
ot_u16 otapi_put_dialog_tmpl(ot_u8* status, dialog_tmpl* dialog) {
    if (dialog == NULL) {
        ///@todo "15" is hard-coded timeout.  Have this be a constant
        dll.comm.rx_timeout = (m2qp.cmd.ext & 2) ? 0 : 15;
        q_writebyte(&txq, (ot_u8)dll.comm.rx_timeout);
    }
    else {
        // Place dialog with timeout
        dll.comm.rx_timeout = otutils_calc_timeout(dialog->timeout);
        dialog->timeout    |= (dialog->channels == 0) ? 0 : 0x80;
        q_writebyte(&txq, dialog->timeout);
    
        // Write response list
        if (dialog->channels != 0) {
            dll.comm.rx_channels = dialog->channels;
            dll.comm.rx_chanlist = dialog->chanlist;
            q_writestring(&txq, dialog->chanlist, dialog->channels);
        }
    }

    *status = 1;
    return q_length(&txq);
}
#endif


#ifndef EXTF_otapi_put_query_tmpl
ot_u16 otapi_put_query_tmpl(ot_u8* status, query_tmpl* query) {
    /// Test for Anycast and Multicast addressing (query needs one of these)    
    if (m2np.header.fr_info & M2QUERY_GLOBAL) {
        q_writebyte(&txq, query->length);
        q_writebyte(&txq, query->code);
    
        if (query->code & 0x80) {
            q_writestring(&txq, query->mask, query->length);
        }
        q_writestring(&txq, query->value, query->length);
    
        *status = 1;
        return q_length(&txq);
    }
    *status = 0;
    return 0;
}
#endif


#ifndef EXTF_otapi_put_ack_tmpl
ot_u16 otapi_put_ack_tmpl(ot_u8* status, ack_tmpl* ack) {
    ot_int  i;
    ot_u8*  data_ptr    = ack->list;
    ot_u8*  limit       = txq.back - ack->length;
    
    for (i=0; (i < ack->count) && (txq.putcursor < limit); \
            i+=ack->length, data_ptr+=ack->length ) {
        q_writestring(&txq, data_ptr, ack->length);
    }
    
    *status = (ot_u8)i;
    return q_length(&txq);
}
#endif


#ifndef EXTF_otapi_put_isf_offset
void sub_put_isf_offset(ot_u8 is_series, ot_u16 offset) {
    if (is_series) {
        q_writeshort(&txq, offset);
    }
    else {
        q_writebyte(&txq, (ot_u8)offset);
    }
}
#endif


#ifndef EXTF_otapi_put_isf_comp
ot_u16 otapi_put_isf_comp(ot_u8* status, isfcomp_tmpl* isfcomp) {
    q_writebyte(&txq, isfcomp->isf_id);
    sub_put_isf_offset(isfcomp->is_series, isfcomp->offset);
    
    *status = 1;
    return q_length(&txq);
}
#endif


#ifndef EXTF_otapi_put_isf_call
ot_u16 otapi_put_isf_call(ot_u8* status, isfcall_tmpl* isfcall) {
    q_writebyte(&txq, isfcall->max_return);
    q_writebyte(&txq, isfcall->isf_id);
    sub_put_isf_offset(isfcall->is_series, isfcall->offset);
    
    *status = 1;
    return q_length(&txq);
}
#endif


#ifndef EXTF_otapi_put_isf_return
ot_u16 otapi_put_isf_return(ot_u8* status, isfcall_tmpl* isfcall) {
    ot_queue   local_q;
    ot_u8   lq_data[4];
    
    q_init(&local_q, lq_data, 4);
    q_writebyte(&local_q, (ot_u8)isfcall->max_return);
    q_writebyte(&local_q, (ot_u8)isfcall->isf_id);
    sub_put_isf_offset(isfcall->is_series, isfcall->offset);
    
    ///@note user_id is set to NULL here.  This stipulates that the API is not
    ///      ever going to be called by a non root user.  So, the application
    ///      layer should perform proper authentication of the user if neeeded.
    *status = (m2qp_isf_call(isfcall->is_series, &local_q, NULL) >= 0);

    return q_length(&txq);
}
#endif


#ifndef EXTF_otapi_put_udp_tmpl
ot_u16 otapi_put_udp_tmpl(ot_u8* status, udp_tmpl* udp) {
/// There is no error/exception handling in this implementation, but it is
/// possible to add in the future
    q_writebyte(&txq, udp->src_port);
    q_writebyte(&txq, udp->dst_port);
    q_writestring(&txq, udp->data, udp->data_length);
    
    *status = 1;
    return q_length(&txq);
}
#endif


#ifndef EXTF_otapi_put_error_tmpl
ot_u16 otapi_put_error_tmpl(ot_u8* status, error_tmpl* error) {
    q_writebyte(&txq, error->code);
    q_writebyte(&txq, error->subcode);
    
    *status = 1;
    return q_length(&txq);
}
#endif


#ifndef EXTF_otapi_put_reqds
ot_u16 otapi_put_reqds(ot_u8* status, ot_queue* dsq) {
/// Write values stored from configuration
/// @todo implement this in next version
    *status = 0;
    return 0;
}
#endif


#ifndef EXTF_otapi_put_propds
ot_u16 otapi_put_propds(ot_u8* status, ot_queue* dsq) {
/// Write values stored from configuration
/// @todo implement this in next version
    *status = 0;
    return 0;
}
#endif











#endif


