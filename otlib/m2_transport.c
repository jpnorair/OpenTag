/* Copyright 2010-2014 JP Norair
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
  * @file       /otlib/m2_transport.c
  * @author     JP Norair
  * @version    R102
  * @date       20 March 2014 
  * @brief      Mode 2 Query Protocol Implementation (Transport Layer)
  * @ingroup    M2QP
  *
  * @todo Move the ISF comp and call templates before the query template(s) in
  * the M2QP template ordering.  Requires reorganization of sub_load_query() 
  * and m2qp_isf_comp.
  *
  ******************************************************************************
  */

#include "OT_config.h"
#if (OT_FEATURE(SERVER) && OT_FEATURE(M2))

#include "m2_transport.h"
#include "OT_utils.h"
#include "OTAPI.h"

#include "alp.h"
#include "buffers.h"
#include "external.h"
#include "queue.h"
#include "m2_dll.h"
#include "m2_network.h"
#include "veelite.h"


// Sneaky stashing for internal data
// It overwrites part of the rxq that isn't used for comp & call commands
//#define LOCAL_U8(OFFSET)    rxq.front[rxq.alloc-32+(OFFSET)]
//#define LOCAL_U16(OFFSET)   *((ot_u16*)&rxq.front[rxq.alloc-32+(OFFSET)])
//#define LOCAL_U32(OFFSET)   *((ot_u32*)&rxq.front[rxq.alloc-32+(OFFSET)])
#define LOCAL_U8(OFFSET)    txq.front[txq.alloc-16+(OFFSET)]
#define LOCAL_U16(OFFSET)   *((ot_u16*)&txq.front[txq.alloc-16+(OFFSET)])
#define LOCAL_U32(OFFSET)   *((ot_u32*)&txq.front[txq.alloc-16+(OFFSET)])

// Argument Shortcut for some callbacks


#if defined(EXTF_m2qp_sig_isf)
#   define M2QP_CB_ISF()    m2qp_sig_isf(m2qp.cmd.code&0x70, m2qp.cmd.code&0x0f, &m2np.rt.dlog)
#elif OT_FEATURE(M2QP_CALLBACKS)
#   define M2QP_CB_ISF()    m2qp.sig.isf(m2qp.cmd.code&0x70, m2qp.cmd.code&0x0f, &m2np.rt.dlog)
#else
#   define M2QP_CB_ISF();
#endif

#if defined(EXTF_m2qp_sig_udp)
#   define M2QP_CB_UDP(SRC, DST)    m2qp_sig_udp(SRC, DST, &m2np.rt.dlog)
#elif OT_FEATURE(M2QP_CALLBACKS)
#   define M2QP_CB_UDP(SRC, DST)    m2qp.sig.udp(SRC, DST, &m2np.rt.dlog)
#else
#   define M2QP_CB_UDP(SRC, DST);
#endif

#if defined(EXTF_m2qp_sig_scpkt) && M2_FEATURE(DATASTREAM)
#   define M2QP_CB_DSPKT()    m2qp_sig_scpkt(0, 0, &m2np.rt.dlog)
#elif OT_FEATURE(M2QP_CALLBACKS) && M2_FEATURE(DATASTREAM)
#   define M2QP_CB_DSPKT()    m2qp.sig.scpkt(0, 0, &m2np.rt.dlog)
#else
#   define M2QP_CB_DSPKT();
#endif

#if defined(EXTF_m2qp_sig_scack) && M2_FEATURE(M2DP)
#   define M2QP_CB_DSACK()    m2qp_sig_scack(0, 0, &m2np.rt.dlog)
#elif OT_FEATURE(M2QP_CALLBACKS) && M2_FEATURE(M2DP)
#   define M2QP_CB_DSACK()    m2qp.sig.scack(0, 0, &m2np.rt.dlog)
#else
#   define M2QP_CB_DSACK();
#endif

#if defined(EXTF_m2qp_sig_control)
#   define M2QP_CB_ERROR(CODE, SUBCODE)    m2qp_sig_control(CODE, SUBCODE, &m2np.rt.dlog)
#elif OT_FEATURE(M2QP_CALLBACKS)
#   define M2QP_CB_ERROR(CODE, SUBCODE)    m2qp.sig.control(CODE, SUBCODE, &m2np.rt.dlog)
#else
#   define M2QP_CB_ERROR(CODE, SUBCODE)    False
#endif

#if defined(EXTF_m2qp_sig_a2p)
#   define M2QP_CB_A2P()    m2qp_sig_a2p(0, 0, &m2np.rt.dlog)
#elif OT_FEATURE(M2QP_CALLBACKS)
#   define M2QP_CB_A2P()    m2qp.sig.a2p(0, 0, &m2np.rt.dlog)
#else
#   define M2QP_CB_A2P()    False
#endif






/** M2QP Transport Parameters   <BR>
  * ========================================================================<BR>
  */

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





/** M2QP Parsing Functions    <BR>
  * ========================================================================<BR>
  * -
  */
typedef ot_int (*m2qp_parser)(m2session*);
typedef void (*m2qp_subproc)(void);

void    sub_opgroup_globalisf(void);
void    sub_opgroup_udp(void);
void    sub_opgroup_collection(void);
void    sub_opgroup_scinit(void);
void    sub_opgroup_sctransport(void);
void    sub_opgroup_rfu(void);

ot_int  sub_parse_response(m2session* active);
ot_int  sub_parse_control(m2session* active);
ot_int  sub_parse_request(m2session* active);
void    sub_renack(ot_int nack);
void    sub_load_query();
ot_int  sub_process_query(m2session* active);

/// This is a command vector table used to turn a command opcode into a
/// function call that is appropriate for processing that command.
static const ot_sub opgroup_proc[8] = { &sub_opgroup_globalisf,     //Announcement
                                        &sub_opgroup_globalisf,     //Inventory
                                        &sub_opgroup_udp,           //Inventory+UDP
                                        &sub_opgroup_collection,    //Collection
                                        &sub_opgroup_collection,    //Collection
                                        &sub_opgroup_scinit,        //DS init, DS negotiate
                                        &sub_opgroup_sctransport,   //DS send, DS ACK
                                        &sub_opgroup_rfu            //RFU
                                    };



#ifndef EXTF_m2qp_sig_null
ot_bool m2qp_sig_null(id_tmpl* responder_id, ot_int payload_length, ot_u8* payload) {
    return False;
}
#endif



///@note New patchwork code to accomodate universal ALP model.
///      Soon it will need buffers as well
alp_tmpl m2alp;



#ifndef EXTF_m2qp_init
OT_WEAK void m2qp_init() {
#if OT_FEATURE(M2QP_CALLBACKS)
#   if !defined(EXTF_m2qp_sig_isf)
        m2qp.sig.isf    = &m2qp_sig_null;
#   endif
#   if !defined(EXTF_m2qp_sig_udp)
        m2qp.sig.udp    = &m2qp_sig_null;
#   endif
#   if !defined(EXTF_m2qp_sig_scpkt) && M2_FEATURE(DATASTREAM)
        m2qp.sig.scpkt  = &m2qp_sig_null;
#   endif
#   if !defined(EXTF_m2qp_sig_scack) && M2_FEATURE(M2DP)
        m2qp.sig.scack  = &m2qp_sig_null;
#   endif
#   if !defined(EXTF_m2qp_sig_control) && M2QP_HANDLES_ERROR
        m2qp.sig.control = &m2qp_sig_null;
#   endif
#   if !defined(EXTF_m2qp_sig_a2p) && M2QP_HANDLES_A2P
        m2qp.sig.a2p    = &m2qp_sig_null;
#   endif
#endif

    //Initialize to an undefined code value
    m2qp.cmd.code = 0x1F; 
    
    ///@note New patchwork code to accomodate universal ALP model.
    ///      Soon it will need buffers as well.
    alp_init(&m2alp, &rxq, &txq);
}
#endif



#ifndef EXTF_m2qp_parse_frame
OT_WEAK ot_int m2qp_parse_frame(m2session* active) {
/// Load Command Code & Extension, and pick the correct parser
/// - normal response (command type 000)
/// - control response (command type 001)
/// - all types requests (010 - 111)
/*
    static const m2qp_parser parse_fn[] = { &sub_parse_response, 
                                            &sub_parse_control,
                                            &sub_parse_request };
    ot_u8 cmd_type;
    cmd_type = (*rxq.getcursor >> 4) & 0x07;
    
    // Map requests to request parser
    // Deal with case where M2QP command type mismatches the Netstate
    if (cmd_type > 1) {
        cmd_type = 2;
        if (active->netstate & M2_NETSTATE_RESP) {
            return -1;
        }
    }
    
    return parse_fn[cmd_type](active);
    */
/** @todo this is the next-gen implementation of this function.  In addition,
  *       control reporting may be taken out of M2QP, or otherwise simplified 
  *       into a "notification" form much like ICMP.
  */
    ot_u8 cmd_type = (*rxq.getcursor >> 4) & 7;
    
    if (cmd_type == 0)  return sub_parse_response(active);
    if (cmd_type == 1)  return sub_parse_control(active);
    
    if (active->netstate & M2_NETSTATE_RESP) return -1;
    return sub_parse_request(active);
  
}
#endif




ot_int sub_parse_response(m2session* session) {
#if ((M2_FEATURE(GATEWAY) == ENABLED) || (M2_FEATURE(SUBCONTROLLER) == ENABLED))
/// Only Gateways and Subcontrollers do anything with the response.  Generally,
/// Gateways might have some sort of logging in the callbacks.  Response types
/// include: (1) Normal responses to NA2P standard dialog, (2) Arbitrated 
/// responses to A2P multicast dialog, (3) Responses that are part of 2, 3, 4,
/// or 5-way datastream session.
    ot_u8   req_cmdcode;
    ot_u8   test;

    /// Save request cmdcode (stored value) and load this cmdcode
    req_cmdcode     = m2qp.cmd.code;
    m2qp.cmd.code   = q_readbyte(&rxq);
    
    /// Response Handling:
    /// <LI> Response command opcode must match the last request's opcode       </LI>
    /// <LI> A2P responses callback the app, so it can plan the next request 
    ///      (command-data callbacks can still occur after this callback).      </LI>
    /// <LI> Announcement, Inventory, and Collection use global ISF callback    </LI>
    /// <LI> UDP-Inventory uses its own callback                                </LI>
    /// <LI> Request & Propose Datastream response handling is built-in         </LI>
    /// <LI> Acknowledge Datastream response handling is built-in               </LI>
    if (((req_cmdcode ^ m2qp.cmd.code) & 0x0F) == 0) {
        test = 1;
    
        /// If using A2P, put this responder's ID onto the ACK chain, reserve 
        /// 48 bytes at the back for query scratchpad, increment "Number of
        /// ACKs" on each ACK generation (txq.getcursor[0]), and do callback.  
        if (((req_cmdcode & 0x60) == M2TT_REQ_M_INIT) && (q_space(&txq) > 48)) {
            ///@todo check to make sure NumACKs is 0 on 1st run (might be done)
            ///@todo Might put in some type of return scoring, later
            txq.getcursor[0]++;
            q_writestring(&txq, m2np.rt.dlog.value, m2np.rt.dlog.length);
            test = (ot_u8)M2QP_CB_A2P();
        }
        
        /// Run command response processor (request cmdcode == response cmdcode)
        if (test) {
            opgroup_proc[((req_cmdcode>>1) & 7)]();
        }
    }
    return -1; //(ot_int)test - 1;
#else
    return -1;
#endif
}





ot_int sub_parse_control(m2session* active) {
#if ((M2_FEATURE(GATEWAY) == ENABLED) || (M2_FEATURE(SUBCONTROLLER) == ENABLED))
/// Forwards control payload to a callback.  
/// Deciding what to do with the control data is a job for the Application Layer
    ot_u8 code, subcode;
    code    = q_readbyte(&rxq);
    subcode = q_readbyte(&rxq);
    
    return (ot_int)M2QP_CB_ERROR(code, subcode);
#else
    return -1;
#endif
}




ot_int sub_parse_request(m2session* active) {
    ot_int  score   = 0;
    ot_u8   cmd_opcode;
    //ot_u8   nack    = 0;
    
    /// 1.  Universal Comm Processing:
    /// <LI> Load CCA type & CSMA disable from command extension        </LI>
    /// <LI> Load NA2P or A2P dialog type from command code             </LI>
    active->netstate      &= ~M2_NETSTATE_TMASK;
    m2qp.cmd.code           = q_readbyte(&rxq);
    m2qp.cmd.ext            = (m2qp.cmd.code & 0x80) ? q_readbyte(&rxq) : 0;
    dll.comm.redundants     = 1;
    dll.comm.csmaca_params  = m2qp.cmd.ext & (M2_CSMACA_CAMASK | M2_CSMACA_NOCSMA);
    dll.comm.csmaca_params |= m2qp.cmd.code & M2_CSMACA_ARBMASK;
    cmd_opcode              = m2qp.cmd.code & M2OP_MASK;
    
    /// 2.  All Requests contain the dialog template, so load it.
    /// <LI> [ num resp channels ] [ list of resp channels]             </LI>
    /// <LI> If number of resp channels is 0, use the current channel   </LI>
    {   ot_u8 timeout_code  = q_readbyte(&rxq);
        dll.comm.rx_timeout = otutils_calc_timeout(timeout_code);   // original contention period
        dll.comm.tc         = dll.comm.rx_timeout;                  // contention period counter
        
        if (timeout_code & 0x80) {
            dll.comm.tx_channels    = q_readbyte(&rxq);
            dll.comm.tx_chanlist    = q_markbyte(&rxq, dll.comm.tx_channels);
        }
        else {
            dll.comm.tx_channels    = 1;
            dll.comm.tx_chanlist    = &dll.comm.scratch[0];
            dll.comm.scratch[0]     = active->channel;
        }
    }
    
    /// 3. Handle Global Queries: (Anycast
    /// Multicast and anycast addressed requests include queries
    //if (m2np.header.fr_info & M2QUERY_GLOBAL) {
    if (m2qp.cmd.code & M2TT_REQ_QUERY) {     ///@todo future update code
        score = sub_process_query(active);
    }
    
    /// 4. If the query is good (sometimes this is trivial):
    /// <LI> If response enabled, prepare common response header.          </LI>
    /// <LI> If response disabled, don't move session to response TX.      </LI>
    /// <LI> Run command-specific dialog data processing                   </LI>
    if (score >= 0) {
        q_empty(&txq); // Flush TX Queue
        
        if (m2qp.cmd.ext & M2CE_NORESP) {
            active->netstate |= M2_NETFLAG_SCRAP;
        }
        else {
            active->netstate |= M2_NETSTATE_RESPTX;
            //m2np_header(active, (m2np.header.fr_info & M2FI_ADDRMASK), M2FI_FRDIALOG);
            m2np_header(active, 0, M2FI_FRDIALOG); ///@todo Future Update Code, Can use 0 in addressing for Response
            q_writebyte(&txq, (M2TT_RESPONSE | cmd_opcode));            // Write Cmd code byte
        }
        opgroup_proc[((cmd_opcode>>1) & 7)]();
    }
    
    /// Return the score, which when negative will cause cancellation of the 
    /// dialog (but not necessarily the session).  Non-negative values cause 
    /// continuation of the dialog per session rules.  Positive values can be
    /// be used to adaptively affect the congestion control parameters in the
    /// data-link-layer, e.g. they can prioritize higher scores by responding
    /// earlier in the response window.  That is DLL implementation dependent.
    return score;
}




void sub_opgroup_globalisf(void) {
/// Global ISF function doesn't do any processing, it just uses a callback
    M2QP_CB_ISF();
}



void sub_opgroup_udp(void) {
    /*
    ot_u8 src, dst;
    
    // Grab Source & Destination Ports.  DASH7 uses 8 bit ports.  The specific
    // mapping between IETF 16 bit ports and DASH7 ports is not defined yet,
    // but you can safely assume that ports 224-255 are user-defined.
    dst = rxq.getcursor[1];
    src = rxq.getcursor[2];
    
    // For Response, automatically swap source & destination ports in Request
    if ((m2qp.cmd.code & M2TT_MASK) != M2TT_RESPONSE) {
        q_writebyte(&txq, dst);
        q_writebyte(&txq, src);  
    }
    
    // Response always happens after Request, unless the global M2QP "No Resp"
    // bit is set.  The transport layer manages this independently of the 
    // application layer.
    M2QP_CB_UDP(src, dst);
    */
    
    ot_int      udp_record_size;
    ot_u8*      udp_record;
    ALP_status  status;
    
    udp_record_size = rxq.back - rxq.getcursor;
    udp_record      = rxq.getcursor;
    rxq.getcursor   = rxq.back;
    q_writestring(m2alp.inq, udp_record, udp_record_size);      // or is there alp add new?
    
    status  = alp_parse_message(&m2alp, &m2np.rt.dlog);
    if (status == MSG_Null) {
        ///@todo some exception management, although it might not be necessary
    }
}



void sub_opgroup_collection(void) {
/// This function is an extension of the global_isf opgroup.  The collection
/// command contains a payload, and the payload must be generated on requests.
/// the payload generation is skipped if the NO-RESP bit is set.

    //if (m2qp.cmd.code & 0x70) {
        //if ((m2qp.cmd.ext & M2CE_NORESP) == 0) {
    if ( (*(ot_u16*)&m2qp.cmd & ((0x0070<<8)|M2CE_NORESP)) == 0 ) {
            ot_int nack;
            nack = m2qp_isf_call((m2qp.cmd.code & 1), &rxq, &m2np.rt.dlog);
            if (nack != 0) {
                sub_renack(nack);
            }
        //}
    }
    
    sub_opgroup_globalisf();
}



void sub_opgroup_scinit(void) {
/// This is an SCTP command.  RFU
}



void sub_opgroup_sctransport(void) {
/// This is an SCTP command.  RFU
}



void sub_opgroup_rfu(void) {
    ///@note these functions aren't specified yet
    sub_renack(1);
}



void sub_renack(ot_int nack) {
    txq.getcursor[-1]  |= 0x10;     //M2QP Nack Bit
    txq.front[3]       |= 1;        //DLL Nack Bit ///@todo wrap into DLL layer
    q_writebyte(&txq, (ot_u8)nack);
}




void sub_load_query(void) {
/// @todo put this into it
    static const ot_u8 fixed_mask[16] = { \
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, \
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  \
    };

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
        
        //m2qp.qtmpl.mask = &rxq.front[rxq.alloc-16];
        //*((ot_u32*)&rxq.front[rxq.alloc-16]) = 0xFFFFFFFF;
        //*((ot_u32*)&rxq.front[rxq.alloc-12]) = 0xFFFFFFFF;
        //*((ot_u32*)&rxq.front[rxq.alloc-8])  = 0xFFFFFFFF;
        //*((ot_u32*)&rxq.front[rxq.alloc-4])  = 0xFFFFFFFF;
        
        m2qp.qtmpl.mask = (ot_u8*)fixed_mask;
    }

    m2qp.qtmpl.value  = q_markbyte(&rxq, m2qp.qtmpl.length);
}




ot_int sub_process_query(m2session* active) {
///@note For sequential queries, the Listen Bit must be set in the MAC 
///Frame Info field.
    ot_u8 cmd_type = m2qp.cmd.code & 0x70;
    //ot_u8 cmd_type = m2qp.cmd.code & M2TT_MASK;   ///@todo future update code

    /// ACK check: Non-initial A2P only
    /// Look through the ack list for this host's device ID.  If it is
    /// there, then the query can exit.
    //if (cmd_type > 0x40) {
    if (cmd_type > M2TT_REQ_M_INIT) {     ///@todo future update code
        ot_bool id_test;
        ot_int  number_of_acks  = (ot_int)q_readbyte(&rxq);
        
        do {
            id_test = m2np_idcmp(m2np.rt.dlog.length, q_markbyte(&rxq, m2np.rt.dlog.length));
            number_of_acks--;
        }
        while ((number_of_acks >= 0) && (id_test == False));
        
        if (number_of_acks != 0) {
            goto sub_process_query_EXITA2P;
        }
    }

    /// Global Query: All Multicast and Anycast
    /// Load the primary query immediately, for all commands that use a 
    /// query (all multicast, all anycast)
    sub_load_query();
    
    /// Local Query: Multicast only.
    /// Save a pointer to the local query, if this is an initial multicast
    /// request.  This query will be run later.
    //if (m2np.header.fr_info & M2QUERY_LOCAL) {
    if (cmd_type & M2TT_REQ_M) {  ///@todo future update code
        ot_int  query_size;
        ot_u8*  local_ptr;
        
        /// Save a pointer to the local query, and skip to comp template
        local_ptr       = rxq.getcursor;
        query_size      = q_readbyte(&rxq);                 //token
        query_size    <<= ((q_readbyte(&rxq) & 0x80) != 0);  //mask
        rxq.getcursor  += query_size;
        
        /// run the first query... this will actually be the global query
        if (m2qp_isf_comp((m2qp.cmd.code & 1), &m2np.rt.dlog) < 0) {
            goto sub_process_query_EXITA2P;
        }
        
        /// Backtrace the queue to run the Local query
        rxq.getcursor   = local_ptr;
        sub_load_query();
    }

    /// Run the final query (in all non-exit cases)
    return m2qp_isf_comp((m2qp.cmd.code & 1), &m2np.rt.dlog);

    /// Exit case
    sub_process_query_EXITA2P:
    active->flags &= ~M2FI_LISTEN;
    return -1;
}







/** Protocol File System (ISF) Functions      
  * ============================================================================
  * - ISF manipulation is the core feature of M2QP.
  * - M2QP ISF manipulation can be done on single files or series of files.
  */
#ifndef EXTF_m2qp_isf_comp
OT_WEAK ot_int m2qp_isf_comp(ot_u8 is_series, id_tmpl* user_id) {
    ot_int  score;

    // Load the data from the file/series into the query buffer
    {   ot_int  (*load_function)(ot_int*, ot_u8);
        
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
    
    /// Manage search errors
    if (score < 0) {
        return score;
    }
    
    /// Manage String Searches:
    /// String search returns a positive number on success, or 0 on fail.
    ///@todo Implement better scoring mechanism
    else if (m2qp.qtmpl.code & M2QC_COR_SEARCH) {
        score -= (score == 0);
        // Replace with higher-resolution scoring mechanism
    }
    
    /// Manage Arithmetic Comparison:
    /// <LI> Arithmetic comp needs this extra check stage. </LI>
    /// <LI> m2qp_load_isf will return a score based on the load_function you 
    ///      give it.  In any case, anything less than 0 means fail. </LI>
    else if (m2qp.qtmpl.code & M2QC_ALU) {
        ot_int i, j, k;

        // Run Arithmetic comparison
        for (i=0; i<m2qp.qtmpl.length; i++) {
            ot_u8* local_buf = &LOCAL_U8(0);
            j = m2qp.qtmpl.mask[i] & m2qp.qtmpl.value[i];
            k = m2qp.qtmpl.mask[i] & local_buf[i];
            
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
OT_WEAK ot_int m2qp_isf_call( ot_u8 is_series, ot_queue* input_q, id_tmpl* user_id ) {
/// This function takes data from a queue.  That data is a ISF or ISFS Call
/// Template as described in the Mode 2 Spec.
    ot_uni16 scratch;
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
OT_WEAK ot_int m2qp_load_isf(   ot_u8       is_series, 
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
                ot_uni16 ldata;
                
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
    ot_u8* local_buf;
    
    /// The datastream is buffered in an unused part of the data-queue.
    /// The LOCAL_U8() macro behaves similar to array nomenclature
    local_buf = &LOCAL_U8(0);
    local_buf[*cursor] = data_byte;
    
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
            c += ( (local_buf[i] & m2qp.qtmpl.mask[i]) == \
                   (m2qp.qtmpl.value[i] & m2qp.qtmpl.mask[i]) ) << 1;
            c -= 1;
            
            local_buf[i-1] = local_buf[i];
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

#endif
