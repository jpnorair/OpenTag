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
  * @file       /m2/capi.c
  * @author     JP Norair
  * @version    R102
  * @date       26 Aug 2014
  * @brief      OpenTag C API for some Mode 2 packaging functions
  *
  ******************************************************************************
  */

#include <otstd.h>

#if (OT_FEATURE(CAPI) && OT_FEATURE(M2))
#include <m2api.h>

#include <otlib/auth.h>
#include <otlib/alp.h>
#include <otlib/memcpy.h>
#include <otlib/buffers.h>
#include <otlib/queue.h>
#include <otsys/syskern.h>



#define OTAPI_Q     rxq


ot_u16 sub_session_handle(m2session* active) {
    return (active == NULL) ? 0 : *((ot_u16*)&active->channel);
}



ot_u16 otapi_sysinit() {
#   if (OT_FEATURE(M2))
    dll_refresh();
#   endif
    return 1;
}



ot_u16 otapi_new_dialog(session_tmpl* s_tmpl, void* applet) {
#if (SYS_SESSION == ENABLED)
    return sub_session_handle( m2task_immediate(s_tmpl, (ot_app)applet) );
#else
    return 0;
#endif
}



ot_u16 otapi_new_advdialog(advert_tmpl* adv_tmpl, session_tmpl* s_tmpl, void* applet) {
#if (SYS_FLOOD == ENABLED)
    return sub_session_handle( m2task_advertise(adv_tmpl, s_tmpl, (ot_app)applet) );
#else
    return 0;
#endif
}


///@todo this function is experimental, and it is subject to change
ot_u16 otapi_new_telegram(ot_u32 token, ot_u8 data_id, const ot_u8* data) {
/// XR Telegram Format
// ========================================================================
/// General Background frame design
/// <PRE>   +---------+--------+-------+-------+---------+--------+
///         | TX EIRP | Subnet | Token | PType | Payload | CRC16  |
///         |   B0    |   B1   | B2:5  | B6    | B7:13   | B14:15 |
///         +---------+--------+-------+-------+---------+--------+
/// </PRE>
// ========================================================================
/// Set the header if the session is valid.  Also conditionally write the header
/// depending on the address type (a parameter).
    if (session_notempty()) {
        m2session* s_active;
        s_active = session_top();

        // Set the dll parameters to a safe setting; can be changed later
        dll_set_defaults(s_active);

        q_empty(&txq);
        txq.getcursor += 2;         // Bypass unused length and Link CTL bytes

        q_writebyte(&txq, 14);      // Dummy Length value (not actually sent)
        q_writebyte(&txq, 0);       // Dummy Link-Control (not actually sent)
        q_writebyte(&txq, 0);       // Dummy TX-EIRP (updated by RF driver)

        // This byte is two nibbles: Subnet specifier and Page ID (1)
        q_writebyte(&txq, (s_active->subnet | 0x01));

        // Token
        q_writelong(&txq, token);

        // Payload ID (1 byte) & payload (7 bytes)
        q_writebyte(&txq, data_id);
        q_writestring(&txq, data, 7);

        return 1;
    }
    return 0;
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
        //if ((addr & M2QUERY_GLOBAL) == 0) {   
        //    ot_memcpy((ot_u8*)&m2np.rt, (ot_u8*)routing, sizeof(routing_tmpl));
        //}
        ///@note updated version of above, using modernized network control
        if (addr & M2FI_UCAST) {
            ot_memcpy((ot_u8*)&m2np.rt, (ot_u8*)routing, sizeof(routing_tmpl));
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
    
    ///@todo update null radio driver to modern interface
//#   ifndef __KERNEL_NONE__
//    if (radio.state != RADIO_Idle) {
//    	rm2_kill();
//    }
//#   endif

    if (timeout != 0) {
        dll.comm.tc = TI2CLK(timeout);
    }
    
//#   ifndef __KERNEL_NONE__
//    sys.task_RFA.event = 0;
//    sys_preempt(&sys.task_RFA, 0);
//#   endif
    
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
///@todo deprecated
    return (ot_u16)session_occupied(chan_id);
}
#endif






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
        if (m2qp.cmd.ext & 2) {
            dll.comm.rx_timeout = 0;
        }
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
        dialog->timeout    |= (dialog->channels != 0) << 7;     // 0 or 0x80
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

    if ((m2qp.cmd.code & M2TT_MASK) > M2TT_REQ_UB) {
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
    ot_queue    local_q;
    ot_u8       lq_data[4];
    
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
    ot_u16 space;
    vlFILE* fp = NULL;
    
    space = q_space(&txq);
    
#   if (M2_FEATURE(MULTIFRAME))
    ///@todo Multiframe support: it will still return error when the UDP data
    ///      length is bigger than the max packet payload size, but it will
    ///      detect boundaries across the packet rather than simply across the
    ///      frame.  It will probably require a network-layer function to 
    ///      determine the overhead when supplied payload length.
#   else
        
        if (udp->data == NULL) {
        	///@todo add a user to UDP type for access control
        	fp              = ISF_open(udp->src_port, VL_ACCESS_R, AUTH_GUEST );
        	udp->data_length= (fp != NULL) ? fp->length : 0;
        }
        
        space -= 4;
        if (space < udp->data_length) {
            *status = 0;
        }
        else {
            *status = 1;
            q_writebyte(&txq, (ALP_FLAG_MB | ALP_FLAG_ME));
            q_writebyte(&txq, (ot_u8)udp->data_length);
            q_writebyte(&txq, udp->dst_port);
            q_writebyte(&txq, udp->src_port);
            
            if (udp->data == NULL) {
                txq.putcursor += vl_load(fp, udp->data_length, txq.putcursor);
            }
            else {
            	q_writestring(&txq, udp->data, udp->data_length);
            }
        }
        
        vl_close(fp);
#   endif

    return q_length(&txq);
}
#endif


#ifndef EXTF_otapi_put_error_tmpl
ot_u16 otapi_put_error_tmpl(ot_u8* status, error_tmpl* error) {
///@todo M2QP native error reporting is deprecated.  Get rid of this.
    q_writebyte(&txq, error->code);
    q_writebyte(&txq, error->subcode);
    
    *status = 1;
    return q_length(&txq);
}
#endif


#ifndef EXTF_otapi_put_reqds
ot_u16 otapi_put_reqds(ot_u8* status, ot_queue* dsq) {
///@todo Datastream delivery is no longer an explicit feature, get rid of this.
    *status = 0;
    return 0;
}
#endif


#ifndef EXTF_otapi_put_propds
ot_u16 otapi_put_propds(ot_u8* status, ot_queue* dsq) {
///@todo Datastream delivery is no longer an explicit feature, get rid of this.
    *status = 0;
    return 0;
}
#endif







#endif


