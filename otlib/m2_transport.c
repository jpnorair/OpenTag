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
  * @file       /OTlib/m2_transport.c
  * @author     JP Norair
  * @version    V1.0
  * @date       2 November 2011
  * @brief      Mode 2 Query Protocol Implementation (Transport Layer)
  * @ingroup    M2QP
  ******************************************************************************
  */

#include "m2_transport.h"

#include "OT_config.h"
#include "OT_utils.h"
#include "OTAPI.h"

#include "alp.h"
#include "buffers.h"
#include "external.h"
#include "queue.h"
#include "system.h"
#include "m2_network.h"
#include "veelite.h"


// Sneaky stashing for internal data
// It overwrites part of the rxq that isn't used for comp & call commands
#define LOCAL_U8(OFFSET)    rxq.front[rxq.alloc-32+(OFFSET)]
#define LOCAL_U16(OFFSET)   *((ot_u16*)&rxq.front[rxq.alloc-32+(OFFSET)])
#define LOCAL_U32(OFFSET)   *((ot_u32*)&rxq.front[rxq.alloc-32+(OFFSET)])

// Argument Shortcut for some callbacks
#if (OT_FEATURE(M2QP_CALLBACKS) == ENABLED)
#   define M2QP_CALLBACK(VAL)   M2QP_CB_##VAL
#	ifndef EXTF_m2qp_sig_errresp
#   	define M2QP_CB_ERROR        m2qp.signal.error_response(&m2np.rt.dlog, (ot_int)(rxq.back-rxq.getcursor), rxq.getcursor)
#	else
#		define M2QP_CB_ERROR		m2qp_sig_errresp(&m2np.rt.dlog, (ot_int)(rxq.back-rxq.getcursor), rxq.getcursor)
#	endif

#	ifndef EXTF_m2qp_sig_stdresp
#   	define M2QP_CB_STANDARD     m2qp.signal.std_response(&m2np.rt.dlog, (ot_int)(rxq.back-rxq.getcursor), rxq.getcursor)
#	else
#		define M2QP_CB_STANDARD		m2qp_sig_stdresp(&m2np.rt.dlog, (ot_int)(rxq.back-rxq.getcursor), rxq.getcursor)
#	endif

#	ifndef EXTF_m2qp_sig_a2presp
#   	define M2QP_CB_A2P          m2qp.signal.a2p_response(&m2np.rt.dlog, (ot_int)(rxq.back-rxq.getcursor), rxq.getcursor)
#	else
#		define M2QP_CB_A2P		    m2qp_sig_a2presp(&m2np.rt.dlog, (ot_int)(rxq.back-rxq.getcursor), rxq.getcursor)
#	endif

#	ifndef EXTF_m2qp_sig_dsresp
#   	define M2QP_CB_DSPKT        m2qp.signal.dspkt_response(&m2np.rt.dlog, (ot_int)(rxq.back-rxq.getcursor), rxq.getcursor)
#	else
#		define M2QP_CB_DSPKT		m2qp_sig_dsresp(&m2np.rt.dlog, (ot_int)(rxq.back-rxq.getcursor), rxq.getcursor)
#	endif

#	ifndef EXTF_m2qp_sig_dsack
#   	define M2QP_CB_DSACK        m2qp.signal.dsack_response(&m2np.rt.dlog, (ot_int)(rxq.back-rxq.getcursor), rxq.getcursor)
#	else
#		define M2QP_CB_DSACK		m2qp_sig_dsack(&m2np.rt.dlog, (ot_int)(rxq.back-rxq.getcursor), rxq.getcursor)
#	endif

#	ifndef EXTF_m2qp_sig_udpreq
#   	define M2QP_CB_UDP        m2qp.signal.shell_request(&m2np.rt.dlog, (ot_int)(rxq.back-rxq.getcursor), rxq.getcursor)
#	else
#		define M2QP_CB_UDP		m2qp_sig_udpreq(&m2np.rt.dlog, (ot_int)(rxq.back-rxq.getcursor), rxq.getcursor)
#	endif

#else
#   define M2QP_CALLBACK(VAL)   False
#endif





/*****************************
 * M2QP Transport Parameters *
 *****************************/

//m2dp_struct m2dp;
m2qp_struct m2qp;



/** @brief Subroutine for use with m2qp_load_isf(): Loads arithmetic comparison.
  * @param cursor       (ot_int*)   Used by m2qp_load_isf()
  * @param data_byte    (ot_u8)     One byte of data to load (and process)
  * @retval ot_int      always returns 0
  */
ot_int sub_load_comparison(ot_int* cursor, ot_u8 data_byte);

/** @brief Subroutine for use with m2qp_load_isf(): Performs string token search.
  * @param cursor       (ot_int*)   Used by m2qp_load_isf()
  * @param data_byte    (ot_u8)     One byte of data to load (and process)
  * @retval ot_int      returns 1 if there is a match, 0 if no match
  */
ot_int sub_load_charcorrelation(ot_int* cursor, ot_u8 data_byte);

/** @brief Subroutine for use with m2qp_load_isf(): Loads return template
  * @param cursor       (ot_int*)   Used by m2qp_load_isf()
  * @param data_byte    (ot_u8)     One byte of data to load (and process)
  * @retval ot_int      always returns 0
  */
ot_int sub_load_return(ot_int* cursor, ot_u8 data_byte);

/** @brief Subroutine for use with m2qp_load_isf(): Does nothing
  * @param cursor       (ot_int*)   Used by m2qp_load_isf()
  * @param data_byte    (ot_u8)     One byte of data to load (and process)
  * @retval ot_int      always returns 0
  */
ot_int sub_load_nonnull(ot_int* cursor, ot_u8 data_byte);







/** M2QP OTAPI Functions    
  * ============================================================================
  * -
  */
 
#if (OT_FEATURE(CAPI) == ENABLED)

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
    return txq.length;
}
#endif


#ifndef EXTF_otapi_put_dialog_tmpl
ot_u16 otapi_put_dialog_tmpl(ot_u8* status, dialog_tmpl* dialog) {
    if (dialog == NULL) {
        dll.comm.rx_timeout = (m2qp.cmd.ext & 2) ? 0 : 15;
        q_writebyte(&txq, 0);
    }
    else {
        // Calculate actual timeout and write timeout code field
        dll.comm.rx_timeout = otutils_calc_timeout(dialog->timeout);
        dialog->timeout    |= (dialog->channels != 0) ? 0 : 0x80;
        q_writebyte(&txq, dialog->timeout);
    
        // Write response list
        if (dialog->channels != 0) {
            dll.comm.rx_channels = dialog->channels;
            dll.comm.rx_chanlist = dialog->chanlist;
            q_writestring(&txq, dialog->chanlist, dialog->channels);
        }
    }

    *status = 1;
    return txq.length;
}
#endif


#ifndef EXTF_otapi_put_query_tmpl
ot_u16 otapi_put_query_tmpl(ot_u8* status, query_tmpl* query) {
    /// Test for Anycast and Multicast addressing (query needs one of these)    
    if (m2np.header.addr_ctl & 0x80) {
        q_writebyte(&txq, query->length);
        q_writebyte(&txq, query->code);
    
        if (query->code & 0x80) {
            q_writestring(&txq, query->mask, query->length);
        }
        q_writestring(&txq, query->value, query->length);
    
        *status = 1;
        return txq.length;
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
    return txq.length;
}
#endif


#ifndef EXTF_otapi_put_isf_offset
void sub_put_isf_offset(ot_u8 is_series, ot_u16 offset) {
    if (is_series) {
        q_writeshort(&txq, (ot_u8)offset);
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
    return txq.length;
}
#endif


#ifndef EXTF_otapi_put_isf_call
ot_u16 otapi_put_isf_call(ot_u8* status, isfcall_tmpl* isfcall) {
    q_writebyte(&txq, isfcall->max_return);
    q_writebyte(&txq, isfcall->isf_id);
    sub_put_isf_offset(isfcall->is_series, isfcall->offset);
    
    *status = 1;
    return txq.length;
}
#endif


#ifndef EXTF_otapi_put_isf_return
ot_u16 otapi_put_isf_return(ot_u8* status, isfcall_tmpl* isfcall) {
    Queue   local_q;
    ot_u8   lq_data[4];
    
    q_init(&local_q, lq_data, 4);
    q_writebyte(&local_q, (ot_u8)isfcall->max_return);
    q_writebyte(&local_q, (ot_u8)isfcall->isf_id);
    sub_put_isf_offset(isfcall->is_series, isfcall->offset);
    
    ///@note user_id is set to NULL here.  This stipulates that the API is not
    ///      ever going to be called by a non root user.  So, the application
    ///      layer should perform proper authentication of the user if neeeded.
    *status = (m2qp_isf_call(isfcall->is_series, &local_q, NULL) >= 0);

    return txq.length;
}
#endif


#ifndef EXTF_otapi_put_shell_tmpl
ot_u16 otapi_put_shell_tmpl(ot_u8* status, shell_tmpl* shell) {
/// There is no error/exception handling in this implementation, but it is
/// possible to add in the future
    q_writebyte(&txq, shell->req_port);
    q_writebyte(&txq, shell->resp_port);
    q_writestring(&txq, shell->data, shell->data_length);
    
    *status = 1;
    return txq.length;
}
#endif


#ifndef EXTF_otapi_put_error_tmpl
ot_u16 otapi_put_error_tmpl(ot_u8* status, error_tmpl* error) {
    q_writebyte(&txq, error->code);
    q_writebyte(&txq, error->subcode);
    
    *status = 1;
    return txq.length;
}
#endif


#ifndef EXTF_otapi_put_reqds
ot_u16 otapi_put_reqds(ot_u8* status, Queue* dsq) {
/// Write values stored from configuration
/// @todo implement this in next version
    *status = 0;
    return 0;
}
#endif


#ifndef EXTF_otapi_put_propds
ot_u16 otapi_put_propds(ot_u8* status, Queue* dsq) {
/// Write values stored from configuration
/// @todo implement this in next version
    *status = 0;
    return 0;
}
#endif

#endif







/** M2QP Parsing Functions    <BR>
  * ========================================================================<BR>
  * -
  */
typedef ot_int (*m2qp_parser)(m2session*);
typedef void (*m2qp_subproc)(void);

void    sub_opgroup_null(void);
void    sub_opgroup_shell(void);
void    sub_opgroup_collection(void);
void    sub_opgroup_datastream(void);
void    sub_ack_datastream(void);
void    sub_opgroup_rfu(void);

ot_int  sub_parse_response(m2session* session);
ot_int  sub_parse_error(m2session* session);
ot_int  sub_parse_request(m2session* session);
void    sub_renack(ot_int nack);
void    sub_load_query();
ot_int  sub_process_query(m2session* session);


#ifndef EXTF_m2qp_sigresp_null
ot_bool m2qp_sigresp_null(id_tmpl* responder_id, ot_int payload_length, ot_u8* payload) {
}
#endif



#ifndef EXTF_m2qp_init
void m2qp_init() {
#if (OT_FEATURE(M2QP_CALLBACKS) == ENABLED)
    ///@todo udp shell request callback
    
    m2qp.cmd.code   = 0x3F; //Initialize to an undefined code value

#   if ((M2_FEATURE(GATEWAY) == ENABLED) || (M2_FEATURE(SUBCONTROLLER) == ENABLED))
        m2qp.signal.error_response  = &m2qp_sigresp_null;
        m2qp.signal.std_response    = &m2qp_sigresp_null;
        m2qp.signal.a2p_response    = &m2qp_sigresp_null;
#       if (M2_FEATURE(DATASTREAM) == ENABLED)    
            m2qp.signal.dspkt_response  = &m2qp_sigresp_null;
            m2qp.signal.dsack_response  = &m2qp_sigresp_null;
#       endif
#   endif
#endif
}
#endif



#ifndef EXTF_m2qp_parse_frame
ot_int m2qp_parse_frame(m2session* session) {
/// Load Command Code & Extension, and pick the correct parser
/// - normal response (command type 000)
/// - error response (command type 001)
/// - all types requests (010 - 111)
    ot_u8 cmd_type = *rxq.getcursor & 0x70;

    if (cmd_type > 0x10) 
        return sub_parse_request(session);
#if ((M2_FEATURE(GATEWAY) == ENABLED) || (M2_FEATURE(SUBCONTROLLER) == ENABLED))
    else if (dll.netconf.active & M2_SET_ENDPOINT)  
        return -1;
    else if (cmd_type == 0) 
        return sub_parse_response(session);
    else
        return sub_parse_error(session);
    
#else
    return -1;
#endif
}
#endif



#if ((M2_FEATURE(GATEWAY) == ENABLED) || (M2_FEATURE(SUBCONTROLLER) == ENABLED))
ot_int sub_parse_response(m2session* session) {
/// Only Gateways and Subcontrollers do anything with the response.  Generally,
/// Gateways might have some sort of logging in the callbacks.  Response types
/// include: (1) Normal responses to NA2P standard dialog, (2) Arbitrated 
/// responses to A2P multicast dialog, (3) Responses that are part of 2, 3, 4,
/// or 5-way datastream session.
    ot_u8   test;
    ot_u8   cmd_opcode;

    /// Make sure response command opcode matches the last request's opcode
    /// (this ensures that the response is to our request).
    cmd_opcode  = m2qp.cmd.code & 0x0F;
    test        = q_readbyte(&rxq) & 0x0F;  
    
    if (test == cmd_opcode) {
#       if ((M2_FEATURE(DATASTREAM) == ENABLED) && (OT_FEATURE(ALP) == ENABLED))
            /// Manage Responses to Request and Propose Datastream
            if ((cmd_opcode - M2OP_DS_REQUEST) <= 1) {
                ot_u16  ds_total_bytes  = q_readshort(&rxq);    // might be removed
                ot_u8   fr_per_pkt      = q_readbyte(&rxq);     // might be removed
                ot_u8   num_frames      = rxq.getcursor[0];     // might be removed
                rxq.getcursor          += ((cmd_opcode & 1) == 0);
                m2dp.out_rec.flags      = ALP_FLAG_MB;          // ds beginning
                
                // Run Callback (if enabled).  Callback can override processing
                // (Callback is not compiled when callbacks are disabled)
                test = (ot_u8)M2QP_CALLBACK(DSPKT);
                if ( test ) {
                    m2dp_dsproc();
                }
                ///@todo Might put in some type of return scoring, later
                return (ot_int)test;
            }
#           if (OT_FEATURE(M2QP_CALLBACKS) == ENABLED)
            else if (cmd_opcode == 10) {
            	test = (ot_u8)M2QP_CALLBACK(DSACK);
                if ( test ) {
                    ///@todo Prepare the next stream packet
                }
                return (ot_int)test;
            }
#           endif
            else if (((m2qp.cmd.code & 0x60) == 0x40) && \
                    ((txq.back - txq.putcursor) > 48) )

#       else //((M2_FEATURE(DATASTREAM) == ENABLED) && (OT_FEATURE(ALP) == ENABLED))
        if (((m2qp.cmd.code & 0x60) == 0x40) && \
            ((txq.back - txq.putcursor) > 48) )
#       endif
        
        /// If using A2P, put this responder's ID onto the ACK chain        <BR>
        /// - Reserve some room at the back for query data (48 bytes)       <BR>
        /// - Increment "Number of ACKs" on each use (txq.getcursor[0])     <BR>
        /// - Run the A2P callback (if enabled)        
        {
            ///@todo check to make sure NumACKs is 0 on 1st run (might be done)
            ///@todo Might put in some type of return scoring, later
            txq.getcursor[0]++;
            q_writestring(&txq, m2np.rt.dlog.value, m2np.rt.dlog.length);
            test = (ot_u8)M2QP_CALLBACK(A2P);
        }
        
        /// If nothing else, the response is a normal response (NA2P), so run
        /// the callback as normal
        else {
            test = (ot_u8)M2QP_CALLBACK(STANDARD);
        }
        
        /// Make into 0/-1 form for returning
        return (ot_int)test - 1;
    }
}
#endif



#if ((M2_FEATURE(GATEWAY) == ENABLED) || (M2_FEATURE(SUBCONTROLLER) == ENABLED))
ot_int sub_parse_error(m2session* session) {
/// Forwards error payload to a callback.  
/// Deciding what to do with the error data is a job for the Application Layer
    return (ot_int)M2QP_CALLBACK(ERROR);
}
#endif




ot_int sub_parse_request(m2session* session) {
    ot_int  score   = 0;
    ot_u8   cmd_opcode;
    //ot_u8   nack    = 0;
    
    /// 1.  Universal Comm Processing                                       <BR>
    ///     - Load CCA type & CSMA disable from command extension           <BR>
    ///     - Load NA2P or A2P dialog type from command code
    m2qp.cmd.code           = q_readbyte(&rxq);
    m2qp.cmd.ext            = (m2qp.cmd.code & 0x80) ? q_readbyte(&rxq) : 0;
    dll.comm.csmaca_params  = m2qp.cmd.ext & (M2_CSMACA_CAMASK | M2_CSMACA_NOCSMA);
    dll.comm.csmaca_params |= m2qp.cmd.code & M2_CSMACA_ARBMASK;
    cmd_opcode              = m2qp.cmd.code & M2OP_MASK;
    
    /// 2.  All Requests contain the dialog template, so load it.           <BR>
    ///     - [ num resp channels ] [ list of resp channels]                <BR>
    ///     - if number of resp channels is 0, use the current channel
    {
        ot_u8 timeout_code  = q_readbyte(&rxq);
        dll.comm.rx_timeout = otutils_calc_timeout(timeout_code);    // original contention period
        dll.comm.tc         = dll.comm.rx_timeout;  // contention period counter
        
        if (timeout_code & 0x80) {
            dll.comm.tx_channels    = q_readbyte(&rxq);
            dll.comm.tx_chanlist    = q_markbyte(&rxq, dll.comm.tx_channels);
        }
        else {
            dll.comm.tx_channels    = 1;
            dll.comm.tx_chanlist    = &dll.comm.scratch[0];
            dll.comm.scratch[0]     = session->channel;
        }
    }
    
    /// 3. Handle Command Queries (filtering)                               <BR>
    /// Multicast and anycast addressed requests include queries
    if (m2np.header.addr_ctl & 0x80) {
        score = sub_process_query(session);
    }
    
    /// 4. If the query is good (sometimes this is trivial):                <BR>
    ///    - Prepare the response header (same for all responses)           <BR>
    ///    - Run command-specific dialog data processing
    if (score >= 0) {
        q_empty(&txq); // Flush TX Queue
    
        if (m2qp.cmd.ext & M2CE_NORESP) {
            session->netstate  |= M2_NETFLAG_SCRAP;
        }
        else {
            ot_u8 addressing;
            session->netstate  &= ~M2_NETSTATE_TMASK;
            session->netstate  |= M2_NETSTATE_RESPTX;
            addressing          = ext_get_m2appflags();
            addressing         |= m2np.header.addr_ctl & 0x30;  // make unicast, retain VID & NLS                               
            m2np_header(session, addressing, 0);                // Create M2QP header
            q_writebyte(&txq, (M2TT_RESPONSE | cmd_opcode));    // Write Cmd code byte
        }
    
        switch ((cmd_opcode>>1) & 7) {
            case 0: 
            case 1: break;
            case 2: sub_opgroup_shell();        break;
            case 3:
            case 4: sub_opgroup_collection();   break;
            case 5: break;
            case 6: sub_opgroup_datastream();   break;
            case 7: sub_ack_datastream();       break;
        }
    }
    
    /// Return the score, which when negative will cause cancellation of the 
    /// dialog (but not necessarily the session).  If positive, this can be 
    /// used to affect the congestion control parameters, which must be passed
    /// back into the fc function.
    return score;  
}


void sub_opgroup_collection(void) {
    if ((m2qp.cmd.ext & M2CE_NORESP) == 0) {
        ot_int nack;
        nack = m2qp_isf_call((m2qp.cmd.code & 1), &rxq, &m2np.rt.dlog);
        if (nack != 0) {
            sub_renack(nack);
        }
    }
}

void sub_opgroup_datastream(void) {
    ///@todo process datastream request
}

void sub_ack_datastream(void) {
    ///@todo process datastream request
}

void sub_opgroup_rfu(void) {
    ///@note these functions aren't specified yet
    sub_renack(1);
}

void sub_opgroup_shell(void) {
/// Application Shell commands are mostly proprietary in structure.         <BR>
/// Ports 1 - 15 are GFs 0-15 (check if they are runable)                   <BR>
/// Ports 16 - 127 are callbacks                                            <BR>
/// Ports 128 - 255 are ISFs 128 - 255 (check if they are runable)

///@note None of the "runable file" features exist until DASHForth is ready.
	ot_bool dummy;

    q_writebyte(&txq, rxq.getcursor[1]);    // reverse source/dest for response
    q_writebyte(&txq, rxq.getcursor[0]);    
    
    dummy = M2QP_CALLBACK(UDP);
}



void sub_renack(ot_int nack) {
    txq.getcursor[-1]  |= 0x10;     //M2QP Nack Bit
    txq.front[3]       |= 1;        //DLL Nack Bit ///@todo wrap into DLL layer
    q_writebyte(&txq, (ot_u8)nack);
}



void sub_load_query(void) {    
    m2qp.qtmpl.length = q_readbyte(&rxq);
    m2qp.qtmpl.code   = q_readbyte(&rxq);
    
    if ((m2qp.qtmpl.code & M2QC_MASKED) != 0) {
        /// Option 1: there is a supplied mask
        m2qp.qtmpl.mask = q_markbyte(&rxq, m2qp.qtmpl.length);
    }
    else {
        /// Option 2: we need to stash a mask of FF's at the back of the queue!
        /// This is not a problem, because the RX queue will never get this far
        /// during comp & call commands.
        m2qp.qtmpl.mask = &rxq.front[rxq.alloc-16];

        *((ot_u32*)&rxq.front[rxq.alloc-16]) = 0xFFFFFFFF;
        *((ot_u32*)&rxq.front[rxq.alloc-12]) = 0xFFFFFFFF;
        *((ot_u32*)&rxq.front[rxq.alloc-8])  = 0xFFFFFFFF;
        *((ot_u32*)&rxq.front[rxq.alloc-4])  = 0xFFFFFFFF;
    }

    m2qp.qtmpl.value  = q_markbyte(&rxq, m2qp.qtmpl.length);
}



ot_int sub_process_query(m2session* session) {
///@note For sequential queries, the Listen Bit must be set in the MAC 
///Frame Info field.
    ot_u8 cmd_type = m2qp.cmd.code & 0x70;

    /// ACK check: Non-initial Multicast only
    /// Look through the ack list for this host's device ID.  If it is
    /// there, then the query can exit.
    if (cmd_type > 0x40) {
        ot_bool id_test;
        ot_int  number_of_acks  = (ot_int)q_readbyte(&rxq);
        
        do {
            number_of_acks--;
            id_test = m2np_idcmp(m2np.rt.dlog.length, \
                                    q_markbyte(&rxq, m2np.rt.dlog.length));   
        }
        while ((number_of_acks >= 0) && (id_test == False));
        
        if (number_of_acks != 0) {
            goto sub_process_query_exit;
        }
    }

    /// Global Query: All Multicast and Anycast
    /// Load the primary query immediately, for all commands that use a 
    /// query (all multicast, all anycast)
    sub_load_query();
    
    /// Local Query: Initial Multicast only
    /// Save a pointer to the local query, if this is an initial multicast
    /// request.  This query will be run later.
    if (cmd_type == 0x40) {
        ot_int  query_size;
        ot_u8*  local_ptr;
        
        /// Save a pointer to the local query, and skip to comp template
        local_ptr       = rxq.getcursor;
        query_size      = q_readbyte(&rxq);                 //token
        query_size    <<= ((q_readbyte(&rxq) & 0x80) != 0);  //mask
        rxq.getcursor  += query_size;
        
        /// run the first query... this will actually be the global query
        /// on commands that also contain a local query.
        if (m2qp_isf_comp((m2qp.cmd.code & 1), &m2np.rt.dlog) < 0) {
            goto sub_process_query_exit;
        }
        
        /// Backtrace the queue to run the Local query
        rxq.getcursor   = local_ptr;
        sub_load_query();
    }

    /// Run the final query (in all non-exit cases)
    return m2qp_isf_comp((m2qp.cmd.code & 1), &m2np.rt.dlog);

    /// Exit case
    sub_process_query_exit:
    session->flags &= ~M2FI_LISTEN;
    return -1;
}







/** Protocol File System (ISF) Functions      
  * ============================================================================
  * - ISF manipulation is the core feature of M2QP.
  * - M2QP ISF manipulation can be done on single files or series of files.
  */
#ifndef EXTF_m2qp_isf_comp
ot_int m2qp_isf_comp(ot_u8 is_series, id_tmpl* user_id) {
    ot_int  score;

    // Load the data from the file/series into the query buffer
    {
        ot_int  (*load_function)(ot_int*, ot_u8);
        
        // Assure length is 0 when Non-Null search is used, and set the load
        // function accordingly, depending on the query method
        m2qp.qtmpl.length   = (m2qp.qtmpl.code) ? m2qp.qtmpl.length : 0;
        load_function       = ((m2qp.qtmpl.code & M2QC_COR_SEARCH) != 0) ? \
                               &sub_load_charcorrelation : &sub_load_comparison;
    
        // Get ISF information from queue, and load data 
        m2qp.qdata.comp_id  = q_readbyte(&rxq);
        
        if (is_series)  m2qp.qdata.comp_offset  = q_readshort(&rxq);
        else            m2qp.qdata.comp_offset  = q_readbyte(&rxq);
            
        score   = m2qp_load_isf(is_series, m2qp.qdata.comp_id, m2qp.qdata.comp_offset, 
                                m2qp.qtmpl.length, load_function, user_id );
    }
    
    // Manage search errors
    if (score < 0) {
        return score;
    }
    
    // Manage String Searches:
    // String search returns a positive number on success, or 0 on fail.  We
    // need to modify the way it returns.    
    else if (m2qp.qtmpl.code & M2QC_COR_SEARCH) {
        score -= (score == 0);
        
    
        
    }
    
    // Manage Arithmetic Comparison:
    // - Other comps are managed by the load function.  For them, just return.
    // - m2qp_load_isf will return a score based on the load_function you give
    //   it.  In either of these cases, anything less than 0 means fail.
    else if (m2qp.qtmpl.code & M2QC_ALU) {
        ot_int i, j, k;

        // Run Arithmetic comparison
        for (i=0; i<m2qp.qtmpl.length; i++) {
            j = m2qp.qtmpl.mask[i] & m2qp.qtmpl.value[i];
            k = m2qp.qtmpl.mask[i] & LOCAL_U8(i);
            
            if (j != k) {
                switch (m2qp.qtmpl.code & 0x1F) {
                    case 0: return 0;                   // !=
                    case 1: return -1;                  // ==
                    case 2: 
                    case 3: return (j < k) - 1;         // <, <=
                    case 4: 
                    case 5: return (j > k) - 1;         // >, >=
                    default: return -1;
                }
            }
        }
        
        /// If you are here, j == k
        /// the even numbered codes are non-equalities (!=, <, >)
        /// so subtract 1 from the equality output to make -1/0
        return ((ot_int)m2qp.qtmpl.code & 1) - 1;
    }
    
    return score;
}
#endif




#ifndef EXTF_m2qp_isf_call
ot_int m2qp_isf_call( ot_u8 is_series, Queue* input_q, id_tmpl* user_id ) {
/// This function takes data from a queue.  That data is a ISF or ISFS Call
/// Template as described in the Mode 2 Spec.
    Twobytes scratch;
    ot_u8   isf_id;
    vlFILE* fp_f;
    ot_int  offset;
    ot_int  max_bytes;
    ot_int  total_length = 0;
    
    /// 1. Save Max Bytes & Data ID
    max_bytes   = (ot_int)q_readbyte(input_q);
    isf_id      = q_readbyte(input_q);
    q_writebyte(&txq, isf_id);
    
    /// 2. Break down a ISF Series Call Template, and build the header                  <BR>
    ///    - Open ISFS: If not accessible, don't respond by returning negative          <BR>
    ///    - Load data offset, and write the first part of the return template          <BR>
    ///    - Go through ISF Elements and write their ID+Length to the return template   <BR>
    ///    - Write total length to the marked spot on the queue
    if (is_series) {
        vlFILE* fp_s;
        ot_u8*  clength_ptr;
        ot_int  i;
    
        fp_s = ISFS_open( isf_id, VL_ACCESS_R, user_id );      
        if (fp_s == NULL) {
            return -2;
        }
        
        offset = q_readshort(input_q);
        q_writebyte( &txq, (ot_u8)fp_s->length );
        q_writeshort(&txq, offset );
        clength_ptr     = txq.putcursor;
        txq.putcursor  += 2;
        txq.length     += 2;
        
        for (i=0; i<fp_s->length; i++) {
            if ( (i&1) == 0 ) {
                scratch.ushort = vl_read(fp_s, i);
            }
            fp_f = ISF_open(scratch.ubyte[i&1], VL_ACCESS_R, user_id);
            if (fp_f != NULL) {
                q_writebyte(&txq, GET_B0_U16(fp_f->idmod) );
                q_writebyte(&txq, (ot_u8)fp_f->length );
                total_length += fp_f->length;
            }
            vl_close(fp_f);
        }
        vl_close(fp_s);
        
        scratch.ushort  = total_length;
        clength_ptr[0]  = scratch.ubyte[UPPER];
        clength_ptr[1]  = scratch.ubyte[LOWER];
    }
    
    /// 3. Break down a ISF Element Call Template, and build the header.
    ///    If ISF Element is not accessible, don't respond
    else {
        fp_f = ISF_open( isf_id, VL_ACCESS_R, user_id ); 
        if (fp_f == NULL) {        
            return -2;
        }
        offset          = q_readbyte(input_q);
        total_length    = fp_f->length;
        q_writebyte(&txq, (ot_u8)offset );
        q_writebyte(&txq, (ot_u8)total_length );
        vl_close(fp_f);
    }
    
    /// 4.  Check how much room is left in the frame.  If the number of bytes
    ///     used for max_bytes is more than there is room left, fix it
    scratch.sshort = (txq.back - txq.putcursor);
    if (max_bytes > scratch.sshort) {
        max_bytes = scratch.sshort;
    }
    
    /// 5.  Reduce max_bytes further, if it is currently larger than the amount
    ///     of data we are accessing.
    scratch.sshort = (total_length - offset);
    if (max_bytes > scratch.sshort) {
        max_bytes = scratch.sshort;
    }
    
    /// 6.  Call the ISF load function to dump-out the data
    return m2qp_load_isf(is_series, isf_id, offset, max_bytes, &sub_load_return, user_id);
}
#endif



#ifndef EXTF_m2qp_load_isf
ot_int m2qp_load_isf(   ot_u8       is_series, 
                        ot_u8       isf_id, 
                        ot_int      offset, 
                        ot_int      window_bytes,
                        ot_int      (*load_function)(ot_int*, ot_u8),
                        id_tmpl*    user_id ) {            
    Twobytes scratch;
    ot_int  i;
    vlFILE* fp_f;
    vlFILE* fp_s    = NULL;
    ot_int  n_files = 1;
    ot_int  j       = 0;
    ot_int  output  = 0;

    /// 1. Open the ISF Series, if enabled
    ///    Do not respond if the series is not accessible (return negative)
    if (is_series) {
        fp_s = ISFS_open( isf_id, VL_ACCESS_R, user_id );
        if (fp_s == NULL) {
            return -32768;
        }
        n_files = fp_s->length;
    }
    
    /// 2. If no Series, spoof a single file access as a series of size 1
    else {
        scratch.ubyte[0]    = isf_id;
        n_files             = 1;
    }

    /// 3.  Step through each file of the ISF Series, and process it.
    ///     This double-loop will extract and process a window of "window_bytes"
    ///     bytes that is stored contiguously across any number of files in a
    ///     series.  Processing stops when the series ends or the window is full.
    for (i=0; i<n_files; i++) {
        ot_u8 align = (i & 1);
    
        // Read the next file ID if we are processing a ISFS (series)
        if ((is_series) && (align == 0)) {
            scratch.ushort = vl_read(fp_s, i);
        }
        
        // Open the file: 
        // - If not accessible as a series, skip it
        // - If not accessible as an file, bail
        fp_f = ISF_open( scratch.ubyte[align], VL_ACCESS_R, user_id );
        if (fp_f == NULL) {
            vl_close(fp_s);
            return -32768;
        }
                
        // Subtract the file length from the offset value.  When the offset 
        // goes negative, the current file is the one we should start on.
        offset -= fp_f->length;
        if (offset < 0) {
            offset += fp_f->length;
            
            // Load the window and process it, using the open file
            while ( (j < window_bytes) && (offset < fp_f->length) ) {
                Twobytes ldata;
                
                align = offset & 1;
                if (align == 0) {
                    ldata.ushort = vl_read(fp_f, offset);
                }
                output += load_function( &j, ldata.ubyte[align] );
                offset++;
            }
            // reset offset to 0, so the process resumes at next series file
            offset = 0;
        }
        vl_close(fp_f);
    }
    vl_close(fp_s);
    
    ///@todo Here is where an algorithm could go to manage the way "scoring" is
    /// done for search-based load operations.
    
    return output;
}
#endif






 
/** Protocol Command Processing Functions <BR>
  * ========================================================================<BR>
  * - Currently unimplemented
  */








/** Protocol File Loading Subroutines <BR>
  * ========================================================================<BR>
  * - Used as the load_function() argument to sub_load_isf()
  */

ot_int sub_load_charcorrelation(ot_int* cursor, ot_u8 data_byte) {
/// This is a pure character-by-character correlation, which is not as efficient
/// for comparing strings as more specialized methods (The Boyer-Moore-Horspool
/// method might fit nicely here) but it gives the ability to report partial
/// matches, which the BM or BMH methods do not.
///
/// A correlation is a mathematic process for comparing two sequences, so check
/// Wikipedia for more info (http://en.wikipedia.org/wiki/Cross-correlation).
/// This function performs a correlation of a byte-wise token onto a byte-wise 
/// datastream.  The token is usually supplied in the command data (stored in 
/// shared memory), and the datastream is fed into this function byte-by-byte
/// (usually referenced from file data).

    ot_int i;
    ot_int c;
    
    /// The datastream is buffered in an unused part of the data-queue.
    /// The LOCAL_U8() macro behaves similar to array nomenclature
    LOCAL_U8(*cursor) = data_byte;
    
    /// If the datastream is *not* fully pre-buffered, return to the caller.
    /// If the datastream is fully pre-buffered, then proceed to correlation.
    if ( *cursor < (m2qp.qtmpl.length-1) ) {
        (*cursor)++;
        return 0;
    }
    
    /// Equality Correlation with data shifting of the buffer after each
    /// comparison operation.  The value c is the comparison score.  It is a
    /// bipolar accumulator, so a non-equality alters the score by -1 and an
    /// equality by +1.  It is implemented as c += (0 or 2) - 1
    else {
        for (i=0, c=0; i<m2qp.qtmpl.length; i++) {
            c += ( (LOCAL_U8(i) & m2qp.qtmpl.mask[i]) == \
                   (m2qp.qtmpl.value[i] & m2qp.qtmpl.mask[i]) ) << 1;
            c -= 1;
            
            LOCAL_U8(i-1) = LOCAL_U8(i);
        }
    }
    
    /// One parameter of the correlation query is a correlation threshold.  It
    /// occupies the lower 5 bits of the query code.  It is an integer value.
    /// Scores higher than the threshold are passing scores.  The query score
    /// indicates the number of hits the query made on the file data.
    return (c >= (ot_int)(m2qp.qtmpl.code & 0x1F));  
}


ot_int sub_load_comparison(ot_int* cursor, ot_u8 data_byte) {
/// Just loads comparison data, from the file system, into the local buffer.  
/// Comparison is limited to16 bytes per the Mode 2 Spec.
    LOCAL_U8(*cursor) = data_byte;
    (*cursor)++;
    return 0;  
}


ot_int sub_load_return(ot_int* cursor, ot_u8 data_byte) {
/// Just loads file data into the TX queue.
    q_writebyte(&txq, data_byte);
    (*cursor)++;
    return 0;  
}


ot_int sub_load_nonnull(ot_int* cursor, ot_u8 data_byte) {
/// Does Nothing: the nonnull comparison only requires that the specified file
/// exists on the device.
    return 0;
}


