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
  * @file       /otlib/m2_network[new].c
  * @author     JP Norair
  * @version    R101
  * @date       20 Mar 2014
  * @brief      Network Layer implementation for Mode 2
  * @ingroup    Network
  *
  * Patches to be applied shortly.
  * 
  ******************************************************************************
  */

#include "OT_config.h"
#include "OT_platform.h"
#if (OT_FEATURE(SERVER) && OT_FEATURE(M2))

#include "m2_network.h"
#include "m2_transport.h"

#include "OT_utils.h"
#include "OTAPI_tmpl.h"

#include "m2_dll.h"
#include "veelite.h"
#include "auth.h"
#include "buffers.h"
#include "queue.h"



static const ot_u8 _idlen[2] = { 8, 2 };



/** Low-Level Network Functions
  * ============================================================================
  * - In some OSI models, these might be in the "LLC" layer of the MAC.  They
  *   fit more nicely and cleanly in this module, though.
  */
  
// replaces older network_route_ff
ot_int network_route_ff(m2session* active) {
    ot_int route_val;
    ot_int use_m2np;

    // Strip CRC bytes from the end of the message.  RS bytes are stripped by
    // the decoder, if RS is enabled/used.
    //rxq.front[0]       -= 2;  //done in encoder now
    
    /// Acquire RSCODE flag from LC byte (0x40) and transpose to its position
    /// for session flags (0x08)
    active->flags       = (rxq.front[1] & M2LC_RSCODE) >> 3;
    
    /// Acquire Frame-Info flags from Frame-Info byte. 
    /// (LISTEN, CRYPTO1:0, VID-SRC, EXT, ROUTED, ADDRESSING1:0)
    /// The RSCODE bit overrides the EXT bit in the session flags.
    rxq.getcursor       = &rxq.front[4];
    m2np.header.fr_info = *rxq.getcursor++;
    active->flags      |= (m2np.header.fr_info & ~M2_FLAG_RSCODE);     

    /// Acquire EXT byte information
    m2np.header.ext_info = 0;
    if (m2np.header.fr_info & M2FI_EXT) {
        m2np.header.ext_info = *rxq.getcursor++;
    }
    
    /// Check if this is M2NP or M2DP Frame.
    use_m2np = ((m2np.header.fr_info & M2FI_ADDRMASK) != 0);
    
    /// Acquire Dialog ID and compare it:
    /// - if unassociated, connect now
    /// - if already connected, make sure the incremented dialog IDs are equal
    /// @todo make sure synchronization sets to connected
    if (active->netstate & M2_NETSTATE_CONNECTED) {
        active->dialog_id += use_m2np;
        if (active->dialog_id != q_readbyte(&rxq)) {
            return -1;
        }
    }
    else if (use_m2np) {
        active->netstate  |= M2_NETSTATE_CONNECTED;
        active->subnet     = rxq.front[3];
        active->dialog_id  = q_readbyte(&rxq);
    }
    else {
        return -1;  //M2DP cannot be first frame!
    }
    
    /// Data Link Layer Security
    if (m2np.header.fr_info & M2FI_DLLS) {
#   if (OT_FEATURE(DLL_SECURITY))
        ///@todo experimental
        AES_load_static_key(ISF_ID(user_authentication_key), (ot_u32*)txq.front);
        AES_keyschedule_dec((ot_u32*)txq.front, (ot_u32*)(txq.front+16));
        AES_decrypt(rxq.getcursor, rxq.getcursor, (ot_u32*)(txq.front+16));
#   else
        return -1;
#   endif
    }
    
    /// Handle rest of M2NP header fields
    if (use_m2np) {
        /// Grab Source Address from this packet (dialog address), which is 
        /// converted to the target address in the response.
        m2np.rt.dlog.length = _idlen[ ((m2np.header.fr_info & M2FI_VID) != 0) ];
        m2np.rt.dlog.value  = q_markbyte(&rxq, m2np.rt.dlog.length);
        
        /// Network Layer Security
        /// @note Network Layer Security not supported at this time
        if ((m2np.header.fr_info & M2_FLAG_CRYPTO) == M2_FLAG_NLS) {
#       if (OT_FEATURE(NL_SECURITY))
#       else
            return -1;
#       endif
        }
        
        /// Check Target Address if Unicasting.  Set the "First RX" session
        /// qualifier, also, so needless listening is avoided.
        if (m2np.header.fr_info & M2FI_UCAST) {
            ot_int target_len   = (m2np.header.fr_info & 1);
            active->flags      |= (target_len << 4);        //sets M2_FLAG_VID (SRC)
            active->netstate   |= M2_NETFLAG_FIRSTRX;
            target_len          = _idlen[ target_len ];
            
            if (m2np_idcmp(target_len, q_markbyte(&rxq, target_len)) == False) {
                return -1;
            }
        }
        
        /// By default, hopping is off
        m2np.rt.hop_code = 0;
        
        /// Explicit routing information is present when the Route bit is set.
        /// Multihop Routing is not currently supported, so just move the cursor ahead
        if (m2np.header.fr_info & M2FI_ROUTE) {
#           if (M2_FEATURE(MULTIHOP))
                m2np.rt.hop_code = q_readbyte(&rxq);
                if (m2np.rt.hop_code & M2HC_EXT) {
                    m2np.rt.hop_ext = q_readbyte(&rxq);
                }
                if (m2np.rt.hop_code & M2HC_ORIG) {
                    m2np.rt.orig.length = _idlen[ ((m2np.rt.hop_code & M2HC_VID) != 0) ];
                    m2np.rt.orig.value  = q_markbyte(&rxq, m2np.rt.orig.length);
                }
                else {
                    m2np.rt.orig.length = 0;
                    m2np.rt.orig.value  = NULL;
                }
                if (m2np.rt.hop_code & M2HC_DEST) {
                    m2np.rt.dest.length = _idlen[ (m2np.rt.hop_code & 1) ];
                    m2np.rt.dest.value  = q_markbyte(&rxq, m2np.rt.dest.length);
                }
                else {
                    m2np.rt.dest.length = 0;
                    m2np.rt.dest.value  = NULL;
                }
                route_val = -1;  // (or something)
#               if defined(EXTF_network_sig_route)
                    network_sig_route((void*)&route_val, (void*)active);
#               elif (OT_FEATURE(M2NP_CALLBACKS) == ENABLED)
                    m2np.signal.route((void*)&route_val, (void*)active);
#               endif
#           else
                return -1;
#           endif
        }
        else {
            route_val = m2qp_parse_frame(active);
        }
    }
    
    /// M2DP is merely a continuation of the last frame
    else {
        ///@todo m2dp_parse_frame()
        //route_val = m2dp_parse_frame(active);
    }
    
    /// Attach footer to response, if necessary
    if (route_val >= 0) {
        m2np_footer( /* active */);
    }

    return route_val;
}







/** M2NP Network Functions
  * ============================================================================
  * - M2NP = Mode 2 Network Protocol.
  * - Routable, primary data-networking protocol for DASH7 Mode 2.
  * @todo Make sure rxq.back is set to end of the M2QP payload
  */

void m2np_header(m2session* active, ot_u8 addressing, ot_u8 nack) {
/// Build an M2NP header, which gets forwarded in all cases to M2QP at the 
/// transport layer, and which has NM2=0, Frame_Type={0,1}

    /// Prep txq 
    q_empty(&txq);
    //q_start(&txq, 0, 0);

    /// If blockcoding not enabled, don't let it get used.
    /// max frame with blockcoding is 221 data bytes, max with CRC is 254.
    {   ot_int maxframe;
#       if (M2_FEATURE(RSCODE))
        maxframe        = (active->flags & M2_FLAG_RSCODE) ? 221 : 254;
#       else
        active->flags  &= ~M2_FLAG_RSCODE;
        maxframe        = 254;
#       endif
        txq.back        = txq.getcursor + maxframe;
    }
    
    /// Write early header bytes:
    /// <LI> Null Length (Actual length is deposited after frame done) </LI>
    /// <LI> Null LC bits (FR-Cont, Blockcode, CRC5 deposited in encoder)
    /// <LI> Null TX EIRP (Actual value deposited in PHY) </LI>
    /// <LI> Subnet Value </LI>
    q_writeshort(&txq, (ot_u16)((active->flags & M2_FLAG_RSCODE) << 3) );
    q_writeshort(&txq, (ot_u16)active->subnet);
    
    
    ///@todo consider updating m2np_header()... not sure addressing & nack are useful anymore
#   if (M2_FEATURE(MULTIHOP) != ENABLED)    
    active->flags &= M2_FLAG_ROUTE;
#   endif
    
    /// A response: negotiate usage of VIDs
    /// A request: set FIRSTRX mode on Unicast request
    if (active->netstate & M2_NETSTATE_RESP) {
        ot_u8 vid_valid;
        // VID can be used for source address in response if VID exists, and 
        // if request is Broadcast or Unicast-VID
        vid_valid   = (*(ot_u16*)dll.netconf.vid != 0) | (active->flags & 1);
        addressing  = active->flags & (M2FI_LISTEN | M2FI_CRYPTO);
        addressing |= M2FI_UCAST;                               // Response always unicast
        addressing |= vid_valid << 4;                           // Use Source VID if valid
        addressing |= (active->flags & M2_FLAG_VID) >> 4;       // Use Target VID if Request source was VID
    }
    else {
        active->flags      &= ~3;
        active->flags      |= addressing;
        active->netstate   |= (addressing & M2FI_UCAST);    // M2_NETFLAG_FIRSTRX == M2FI_UCAST
        addressing          = active->flags & ~M2_FLAG_RSCODE;
    }
    
    /// Save & Write Frame Info Byte
    m2np.header.fr_info     = addressing;
    m2np.header.fr_info    |= ((active->extra & 0x0F) != 0) << 3;   //M2FI_EXT
    q_writebyte(&txq, m2np.header.fr_info);
    
    /// Write Extra Flags Byte, if set
    if (m2np.header.fr_info & M2FI_EXT) {
        q_writebyte(&txq, active->extra);
    }
    
    /// Write Dialog ID (always present in M2NP)
    q_writebyte(&txq, active->dialog_id);
    
    /// If required, enable DLLS encryption.  AES128 is the only currently
    /// supported crypto in OpenTag.  It will be applied in m2np_footer().  If
    /// DLLS is enabled, NLS will be forced-off, because both of them cannot
    /// be active on the same frame.
    /// @todo Experimental!
#   if (OT_FEATURE(DLL_SECURITY))
    if (m2np.header.fr_info & M2FI_DLLS) {
        auth_setup(&txq, b00100000, 249);
    }
#   endif

    /// Write This Source Device ID (always present in M2NP)
    m2np_put_deviceid( (ot_bool)(m2np.header.fr_info & M2FI_VID) );

    /// If required, enabled NLS.  The rules are basically the same as DLLS.
    /// @todo Experimental!
#   if (OT_FEATURE(NL_SECURITY))
    if ((m2np.header.fr_info & M2FI_CRYPTO) == M2FI_NLS) {
        auth_setup(&txq, b00100000, q_span(&txq));
    }
#   endif
    
    /// Apply target address, if unicast enabled, and rebase it from the TX 
    /// Queue, which remains for the duration of the dialog.
    if (m2np.header.fr_info & M2FI_UCAST) {
        ot_u8* tmp_id       = m2np.rt.dlog.value;
        m2np.rt.dlog.value  = txq.putcursor;
        q_writestring(&txq, tmp_id, m2np.rt.dlog.length);
    }
    
#   if (M2_FEATURE(MULTIHOP))
    if (m2np.header.fr_info & M2FI_ROUTE) {
        m2np.rt.hop_code   |= (m2np.rt.orig.length != 0) << 5;  // ORIG bit
        m2np.rt.hop_code   |= (m2np.rt.orig.length == 2) << 4;  // ORIG VID bit
        m2np.rt.hop_code   |= (m2np.rt.dest.length == 2);       // Unicast VID bit
        m2np.rt.hop_code   |= (m2np.rt.dest.length != 0) << 1;  // Unicast bit
    
        q_writebyte(&txq, m2np.rt.hop_code);
        
        if ((m2np.rt.hop_code & M2HC_EXT) != 0) {
            q_writebyte(&txq, m2np.rt.hop_ext);
        }
        if (m2np.rt.orig.length != 0) {
            q_writestring(&txq, m2np.rt.orig.value, m2np.rt.orig.length);
        }
        if (m2np.rt.dest.length != 0) {
            q_writestring(&txq, m2np.rt.dest.value, m2np.rt.dest.length);
        }
    }  
#   endif
}




void m2np_put_deviceid(ot_bool use_vid) {
    if (use_vid) q_writeshort_be(&txq, dll.netconf.vid);
    else         q_writestring(&txq, dll.netconf.uid, 8);
}



ot_bool m2np_idcmp(ot_int length, ot_u8* id) {
    ot_u8* stored_id;
    ot_u8  check;
    
    // Don't match on vid == 0, that is reserved as unassigned value
    if ((length == 2) && (*(ot_u16*)dll.netconf.vid == 0)) {
        return False;
    }
    
    stored_id   = (length == 8) ? dll.netconf.uid : dll.netconf.vid;
    length      = 8 - length;
    check       = 0;
    
    switch (length & 7) {
        case 0: check |= *stored_id++ ^ *id++; 
        case 1: check |= *stored_id++ ^ *id++; 
        case 2: check |= *stored_id++ ^ *id++; 
        case 3: check |= *stored_id++ ^ *id++; 
        case 4: check |= *stored_id++ ^ *id++; 
        case 5: check |= *stored_id++ ^ *id++; 
        case 6: check |= *stored_id++ ^ *id++; 
        case 7: check |= *stored_id ^ *id;
                break;
    }
    
    return (ot_bool)(check == 0);
}









/** M2DP Network Functions
  * ============================================================================
  */

void m2dp_append() {
///@note M2DP usage is being replaced, not as as an idependent frame type but
///      simply as a continuation frame for M2NP
}



OT_WEAK void m2dp_footer() {
///@note M2DP usage is being replaced, not as as an idependent frame type but
///      simply as a continuation frame for M2NP
}



#endif

