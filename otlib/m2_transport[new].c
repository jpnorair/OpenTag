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
  * @file       /otlib/m2_transport[new].c
  * @author     JP Norair
  * @version    V1.0
  * @date       20 March 2014
  * @brief      Mode 2 Query Protocol Implementation (Transport Layer)
  * @ingroup    M2QP
  *
  * Patch functions for M2QP
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





/** M2QP Transport Parameters   <BR>
  * ========================================================================<BR>
  */



/** M2QP Parsing Functions    <BR>
  * ========================================================================<BR>
  * -
  */



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
        dll.comm.rx_timeout = otutils_calc_timeout(timeout_code);    // original contention period
        dll.comm.tc         = dll.comm.rx_timeout;  // contention period counter
        
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
    if (m2qp.cmd.code & M2TT_REQ_QUERY) {
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
            m2np_header(active, 0, M2FI_FRDIALOG);             // Can use 0 in addressing for Response
            q_writebyte(&txq, (M2TT_RESPONSE | cmd_opcode));    // Write Cmd code byte
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






ot_int sub_process_query(m2session* active) {
///@note For sequential queries, the Listen Bit must be set in the MAC 
///Frame Info field.
    ot_u8 cmd_type = m2qp.cmd.code & M2TT_MASK;

    /// ACK check: Non-initial A2P only
    /// Look through the ack list for this host's device ID.  If it is
    /// there, then the query can exit.
    if (cmd_type > M2TT_REQ_M_INIT) {
        ot_bool id_test;
        ot_int  number_of_acks  = (ot_int)q_readbyte(&rxq);
        
        do {
            number_of_acks--;
            id_test = m2np_idcmp(m2np.rt.dlog.length, \
                                    q_markbyte(&rxq, m2np.rt.dlog.length));   
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
    if (cmd_type & M2TT_REQ_M) {
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
        
        /// Backtrace the queue to load-up the Local query
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





#endif
