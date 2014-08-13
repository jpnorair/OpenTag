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
  * @file       /otlib/m2_network.c
  * @author     JP Norair
  * @version    R101
  * @date       11 Sep 2013
  * @brief      Network Layer implementation for Mode 2
  * @ingroup    Network
  *
  * Includes implementation for the following protocols:
  * 1. M2AdvP
  * 2. M2ResP (not implemented in this version) 
  * 3. M2NP
  * 4. M2DP
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


//#ifdef __DEBUG__
// Place for Debug stuff (not used currently)
//#endif


/** Module Data Elements
  * ============================================================================
  */
m2np_struct m2np;

static const ot_int _idlen[2] = { 8, 2 };



/** Low-Level Network Functions
  * ============================================================================
  * - In some OSI models, these might be in the "LLC" layer of the MAC.  They
  *   fit more nicely and cleanly in this module, though.
  */
#ifndef EXTF_network_init
OT_WEAK void network_init() {
#   if (OT_FEATURE(M2NP_CALLBACKS) == ENABLED)
#   ifndef EXTF_network_sig_route
        m2np.signal.route = &otutils_sig2_null;
#   endif
#   endif

    // Hop code should be explicitly set when producing an anycast or unicast 
    // transmission.  OTAPI will do this for you.
    //m2np.rt.hop_code  = 0;
}
#endif
  


#ifndef EXTF_network_parse_bf
OT_WEAK m2session* network_parse_bf() {
/// Background Frame parsing: fill-in some stuff and treat it as foreground
// ========================================================================
/// General Background frame design
/// <PRE>   +---------+--------+---------+-------+
///         | TX EIRP | Subnet | Payload | CRC16 |
///         |   B0    |   B1   |  B2:4   | B5:6  |
///         +---------+--------+---------+-------+   </PRE>
// ========================================================================
    m2session*  s_next;
    ot_u8       bgpid;
    
    /// Load default attributes
    s_next  = NULL;
    bgpid   = rxq.getcursor[1] & 0x0F;
    
    /// Advertising Protocol has subnet =  0xYF, where "Y" is any four bits
    if (bgpid == 15) {
        ot_u8       scancode;
        ot_u8       netstate;
        ot_uni16    count;
        ot_int      slop;
        
        // Get the counter-ETA information from the inbound frame
        count.ubyte[UPPER]  = rxq.getcursor[3];
        count.ubyte[LOWER]  = rxq.getcursor[4];    
        count.ushort       &= 0x7FFF;

        // Account for "slop" due to clock deviation, process latency, 
        // and other such things.  Thus the follow-up session is 
        // either a second BG scan (if too much slop), or it is FG
        // listening for the request. 
        slop = (count.ushort / OT_GPTIM_ERRDIV);
        if (slop <= 8) {
            scancode    = 0x0F;
            netstate    = M2_NETSTATE_REQRX;
        }
        else {
            scancode    = 0x80;
            netstate    = M2_NETSTATE_REQRX | M2_NETFLAG_BG;
        }
        
        // Reduce the interval time by the slop amount, also ensuring 
        // that count value is never negative.
        count.ushort -= slop;
        if (count.sshort < 0) {
            count.sshort = 0;
        }
        
        // Block DLL idle tasks while waiting for this next session,
        // and create the session.
        //dll_block_idletasks();
        s_next              = session_top();
        s_next->applet      = &dll_scan_applet;
        s_next->counter     = count.ushort;
        s_next->channel     = rxq.getcursor[2];
        s_next->netstate    = netstate;
        s_next->extra       = scancode;
    }
    
    /// Reservation Protocol has subnet = 0xY3
    ///@todo Not presently supported
    else if (bgpid == 3) {
    }
    
    return s_next;
}
#endif




#ifndef EXTF_network_mark_ff
OT_WEAK void network_mark_ff() {
/// Mark a received frame as damaged by using the reserved position in the
/// Link Control bits.  This will never be transmitted over the air.
    rxq.front[1] |= M2LC_DMGMARK;
}
#endif






#ifndef EXTF_network_route_ff
/*
OT_WEAK ot_int network_route_ff(m2session* active) {
    ot_int route_val;

    // Strip CRC bytes from the end of the message.  RS bytes are stripped by
    // the decoder, if RS is enabled/used.
    //rxq.front[0]       -= 2;  //done in encoder now
    
    /// Acquire RSCODE flag from LC byte (0x40) and transpose to its position
    /// for session flags (0x08)
    active->flags       = (rxq.front[1] & M2LC_RSCODE) >> 3;
    
    /// Acquire Frame-Info flags from Frame-Info byte. (LISTEN, DLLS, NLS, VID, 
    /// XXX, STREAM, ADDR bits).  XXX is RSCODE from above.
    rxq.getcursor       = &rxq.front[4];
    m2np.header.fr_info = *rxq.getcursor++;
    active->flags      |= (m2np.header.fr_info & ~M2_FLAG_RSCODE);     

    /// Acquire EXT byte information
    m2np.header.ext_info = 0;
    if (m2np.header.fr_info & M2FI_EXT) {
        m2np.header.ext_info = *rxq.getcursor++;
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
    
    /// Handle M2NP
    /// Session Connection and Dialog Filtering:
    /// - if unassociated, connect now
    /// - if already connected, make sure the incremented dialog IDs are equal
    if ((m2np.header.fr_info & M2FI_STREAM) == 0) {
        if (active->netstate & M2_NETSTATE_CONNECTED) {     ///@todo make sure synchronization sets to connected
            active->dialog_id++;
            if (active->dialog_id != q_readbyte(&rxq)) {
                return -1;
            }
        }
        else {
            active->netstate  |= M2_NETSTATE_CONNECTED;
            active->subnet     = rxq.front[3];
            active->dialog_id  = q_readbyte(&rxq);
        }
        
        /// Grab Source Address from this packet (dialog address), which is 
        /// converted to the target address in the response.
        m2np.rt.dlog.length = (m2np.header.fr_info & M2_FLAG_VID) ? 2 : 8;
        m2np.rt.dlog.value  = q_markbyte(&rxq, m2np.rt.dlog.length);
        
        /// Network Layer Security
        /// @note Network Layer Security not supported at this time
        if (m2np.header.fr_info & M2_FLAG_NLS) {
#       if (OT_FEATURE(NL_SECURITY))
#       else
            return -1;
#       endif
        }
        
        /// Routing Information defaults
        m2np.rt.hop_code    = 0;
        m2np.rt.hop_ext     = 0;
        m2np.rt.orig.value  = NULL;
        m2np.rt.dest.value  = NULL;
        
        /// Routing information present in Unicast and Anycast frames.
        /// Target Address present in Unicast frames
        if ((m2np.header.fr_info & 1) == 0) {
            if ((m2np.header.fr_info & 3) == 0) {
                active->netstate |= M2_NETFLAG_FIRSTRX;
                
                // Bail-out on target address mismatch
                if ( m2np_idcmp(m2np.rt.dlog.length, q_markbyte(&rxq, m2np.rt.dlog.length)) == False ) {
                    return -1;
                }
            }
            
            /// Multihop Routing is not currently supported, so just move the cursor ahead
            m2np.rt.hop_code    = q_readbyte(&rxq);
            m2np.rt.orig.length = 8 >> ((m2np.rt.hop_code & M2HC_VID) >> 3);                       
            m2np.rt.dest.length = m2np.rt.orig.length;
            
            if (m2np.rt.hop_code & M2HC_EXT) {
                m2np.rt.hop_ext = q_readbyte(&rxq);
            }
            if (m2np.rt.hop_code & M2HC_ORIG) {
                m2np.rt.orig.value = q_markbyte(&rxq, m2np.rt.orig.length);
            }
            if (m2np.rt.hop_code & M2HC_DEST) {
                m2np.rt.dest.value = q_markbyte(&rxq, m2np.rt.dest.length);
            }
        }
    }
    
    /// M2DP gets parsed just like M2NP, but it uses the Network data
    /// stored from the last M2NP frame.  m2qp_parse_frame must return negative
    /// values for bad parsed requests as well as ALL parsed responses
    route_val = m2qp_parse_frame(active);
    
    /// Attach footer to response, if necessary
    if (route_val >= 0) {
        m2np_footer();
    }

#   if defined(EXTF_network_sig_route)
        network_sig_route((void*)&route_val, (void*)active);
#   elif (OT_FEATURE(M2NP_CALLBACKS) == ENABLED)
        m2np.signal.route((void*)&route_val, (void*)active);
#   endif
    
    return route_val;
}
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
#endif






/** M2NP Network Functions
  * ============================================================================
  * - M2NP = Mode 2 Network Protocol.
  * - Routable, primary data-networking protocol for DASH7 Mode 2.
  * @todo Make sure rxq.back is set to end of the M2QP payload
  */

#ifndef EXTF_m2np_header
/*
OT_WEAK void m2np_header(m2session* active, ot_u8 addressing, ot_u8 nack) {
/// Build an M2NP header, which gets forwarded in all cases to M2QP at the 
/// transport layer, and which has NM2=0, Frame_Type={0,1}

    /// Prep txq 
    q_empty(&txq);
    //q_start(&txq, 0, 0);
    
    ///@todo consider updating m2np_header()... not sure addressing & nack are useful anymore
    active->flags      &= ~3;
    active->flags      |= addressing;
    
    /// Set FIRSTRX mode on Unicast
    active->netstate   |= (addressing) ? 0 : M2_NETFLAG_FIRSTRX;
    
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
    
    /// Save & Write Frame Info Byte
    m2np.header.fr_info     = (active->flags & ~M2_FLAG_RSCODE);
    m2np.header.fr_info    |= (active->extra & 0x0F) ? M2FI_EXT : 0;
    q_writebyte(&txq, m2np.header.fr_info);
    
    /// Write Extra Flags Byte, if set
    if (m2np.header.fr_info & M2FI_EXT) {
        q_writebyte(&txq, active->extra);
    }
    
    /// If required, enable DLLS encryption.  AES128 is the only currently
    /// supported crypto in OpenTag.  It will be applied in m2np_footer().  If
    /// DLLS is enabled, NLS will be forced-off, because both of them cannot
    /// be active on the same frame.
    /// @todo Experimental!
#   if (OT_FEATURE(DLL_SECURITY))
    if (m2np.header.fr_info & M2FI_DLLS) {
        m2np.header.fr_info &= ~M2FI_NLS;
        auth_setup(&txq, b00100000, 249);
    }
#   endif

    /// Write Dialog ID (always present in M2NP)
    q_writebyte(&txq, active->dialog_id);
    
    /// Write This Device ID (always present in M2NP)
    m2np_put_deviceid( (ot_bool)(m2np.header.fr_info & M2FI_VID) );

    /// If required, enabled NLS.  The rules are basically the same as DLLS.
    /// @todo Experimental!
#   if (OT_FEATURE(NL_SECURITY))
    if (m2np.header.fr_info & M2FI_NLS) {
        auth_setup(&txq, b00100000, q_span(&txq));
    }
#   endif
    
    /// Apply target address, if unicast enabled, and rebase it from the TX 
    /// Queue, which remains for the duration of the dialog.  Also apply 
    /// Multihop routing template, if unicast or anycast enabled.
    if ((m2np.header.fr_info & 1) == 0) {
        ot_u8 hopmask   = M2HC_ORIG;
        ot_u8 id_num    = 1;
        id_tmpl* id     = &m2np.rt.orig;
    
        if ((m2np.header.fr_info & 2) == 0) {
            q_writestring(&txq, m2np.rt.dlog.value, m2np.rt.dlog.length);
            m2np.rt.dlog.value = (txq.putcursor - m2np.rt.dlog.length);
        }
        
#       if (M2_FEATURE(MULTIHOP))
        q_writebyte(&txq, m2np.rt.hop_code);
        if ((m2np.rt.hop_code & M2HC_EXT) != 0) {
            q_writebyte(&txq, m2np.rt.hop_ext);
        }
        while (id_num != 0) {
            if (m2np.rt.hop_code & hopmask) {
                ot_u8* loc = txq.putcursor;
                q_writestring(&txq, id->value, id->length);
                id->value = loc;
            }
            hopmask = M2HC_DEST;
            id     += 1;            //moves to next id_tmpl (dest)
            id_num -= 1;
        }
#       else
        q_writebyte(&txq, 0);
#       endif
    } 
}
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
    if (use_vid) q_writeshort_be(&txq, *(ot_u16*)dll.netconf.vid);
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
#endif



#ifndef EXTF_m2np_footer
OT_WEAK void m2np_footer() {
    ot_int block_bytes;

#   if (OT_FEATURE(DLL_SECURITY))
    /// Add DLLS Padding and run encryption
    if (txq.getcursor[4] & M2FI_DLLS) {
        ///@todo Implement AES128 DLLS stuff
        auth_userkey_encrypt(&txq.getcursor[5], &txq);
        goto m2np_footer_END;
    }
#   endif    
#   if (OT_FEATURE(NL_SECURITY))
    if (txq.getcursor[4] & M2FI_NLS) {
        ///@todo Put footer: NLS not currently supported
        auth_userkey_encrypt(&txq.getcursor[??], &txq);
    }
#   endif

    /// RS and CRC will be appended during encoding, and their extra length
    /// will be added to the frame length field.  -1 is because the length
    /// byte is not included in the length field.
    m2np_footer_END:
    txq.getcursor[0] = q_span(&txq) - 1;
}
#endif



//#ifndef EXTF_m2np_put_device_id
//OT_WEAK void m2np_put_deviceid(ot_bool use_vid) {
//    vlFILE* fp;
//    
//    //file 0=network_settings, 1=device_features
//    fp = ISF_open_su( (ot_u8)(use_vid == False) );
//    ///@todo assert fp
//
//    q_writeshort_be( &txq, vl_read(fp, 0) );
//    if (use_vid == False) {
//    	q_writeshort_be( &txq, vl_read(fp, 2) );
//    	q_writeshort_be( &txq, vl_read(fp, 4) );
//    	q_writeshort_be( &txq, vl_read(fp, 6) );
//    }
//    
//    vl_close(fp);
//}
//#endif


//#ifndef EXTF_m2np_idcmp
//OT_WEAK ot_bool m2np_idcmp(ot_int length, void* id) {
//    ot_bool check   = True;
//    ot_u8   use_uid = (length == 8);
//    vlFILE* fp;
//    
//    //file 0=network_settings, 1=device_features
//    fp      = ISF_open_su( use_uid );
//    check  &= ( ((ot_u16*)id)[0] == vl_read(fp, 0));
//
//    if (use_uid) {
//        check &= ( ((ot_u16*)id)[1] == vl_read(fp, 2) );
//        check &= ( ((ot_u16*)id)[2] == vl_read(fp, 4) );
//        check &= ( ((ot_u16*)id)[3] == vl_read(fp, 6) );
//    }
//    
//    vl_close(fp);
//    return check;
//}
//#endif






/** M2AdvP Network Functions
  * ============================================================================
  * - M2AdvP = Advertising Protocol, i.e. flooding for group synchronization.
  */
  
#if (SYS_FLOOD == ENABLED)
    //Queue   advq;
    //ot_u8   txadv_buffer[10];
#endif



#ifndef EXTF_m2advp_open
OT_WEAK void m2advp_open(m2session* follower) {
    q_empty(&txq);
    txq.getcursor += 2;     //Bypass unused length and Link CTL bytes
    
    q_writebyte(&txq, 6);   //Dummy Length value (not actually sent)
    q_writebyte(&txq, 0);   //Dummy Link-Control (not actually sent)
    q_writebyte(&txq, 0);   //Dummy TX-EIRP (updated by RF driver)
    
    // This byte is two nibbles: Subnet specifier and AdvP ID (F)
    q_writebyte(&txq, (follower->subnet | 0x0F));
    
    // Follower session channel
    q_writebyte(&txq, follower->channel);
    
    // Follower wait time is set to dll counter
    dll.counter = follower->counter;
    q_writeshort(&txq, follower->counter);
}

#endif



#ifndef EXTF_m2advp_update
OT_WEAK void m2advp_update(ot_u16 countdown) {
///@note In this function we manually reset the txq cursors to the places
///      they need to be for the encoder.  The encoder implementation may or
///      may not do this automatically, but it is safer to be redundant.
    txq.getcursor       = &txq.front[2];                //No length, link-CTL bytes in BG Frame
    txq.putcursor       = txq.getcursor;
    txq.putcursor      += 3;                            //Skip EIRP, Subnet, Channel bytes
    *txq.putcursor++    = ((ot_u8*)&countdown)[UPPER];  //Countdown (upper 8 bits)
    *txq.putcursor++    = ((ot_u8*)&countdown)[LOWER];  //Countdown (lower 8 bits)
}

#endif


#ifndef EXTF_m2advp_close
OT_WEAK void m2advp_close() {
}
#endif









/** M2DP Network Functions
  * ============================================================================
  */

#ifndef EXTF_m2dp_append
OT_WEAK void m2dp_append() {
///@note M2DP usage is being replaced, not as as an idependent frame type but
///      simply as a continuation frame for M2NP

}
#endif



#ifndef EXTF_m2dp_footer
OT_WEAK void m2dp_footer() {
/// M2DP is similar enough to M2NP that the same footer function may be used.
    m2np_footer();
}
#endif



#endif

