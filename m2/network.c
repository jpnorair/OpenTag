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

#include <otstd.h>
#if (OT_FEATURE(SERVER) && OT_FEATURE(M2))

#include <m2/network.h>
#include <m2/transport.h>
#include <m2/dll.h>
#include <m2/tmpl.h>

#include <otlib/auth.h>
#include <otlib/buffers.h>
#include <otlib/utils.h>
#include <otsys/veelite.h>


///@todo This board inclusion is temporary until the usage of CRC8 for 
///      background frames is made universal to all radio transceiver models.
///      Right now it's only implemented on LoRa Parts.
#include <board.h>
#if defined(__LORA__)
#   define BG_FRAMESIZE     6
#else
#   define BG_FRAMESIZE     7
#endif



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
    ot_u16		pkt_ti;
    
    /// Load default attributes
    s_next  = NULL;
    
    // stores the bg packet duration of the active channel.  We need this
    // in order to deal with timing skew.
    pkt_ti	= rm2_bgpkt_duration();

    /// Advertising Protocol has subnet =  0xYF, where "Y" is any four bits
    switch (rxq.getcursor[1] & 15) {
    case 15:    s_next = m2advp_parse();        break;
    //case 7:     /* reserved protocol */         break;
    //case 3:	    /* reserved protocol */         break;
    //case 1:     /* reserved protocol */         break;
    default:    break;
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
ot_int network_route_ff(m2session* active) {
///@note The encoder / decoder stage must set rxq.back to the end of the frame.
///      The "end-of-frame" is right before CRC (or technically, rxq.back should
///      be on the first CRC byte)
    ot_int  route_val;
    ot_int  use_m2np;
#   if (OT_FEATURE(DLL_SECURITY))
    ot_u8   dlls_key_index;
#   endif
    
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
    ///
    /// @note On DLLS-key selection based on netstate:
    /// If the netstate is CONNECTED then we use key-index-0, which points to 
    /// the last-used key.  If UNCONNECTED, then we use the key explicitly 
    /// provided in the frame-control: key 1 (default root) or 2 (default user).
    if (active->netstate & M2_NETSTATE_CONNECTED) {
        active->dialog_id += use_m2np;
        if (active->dialog_id != q_readbyte(&rxq)) {
            return -1;
        }
#       if (OT_FEATURE(DLL_SECURITY))
        dlls_key_index = 0;
#       endif
    }
    else if (use_m2np) {
        active->netstate  |= M2_NETSTATE_CONNECTED;
        active->subnet     = rxq.front[3];
        active->dialog_id  = q_readbyte(&rxq);
#       if (OT_FEATURE(DLL_SECURITY))
        dlls_key_index = 1 + ((m2np.header.fr_info & M2FI_DLLSINDEX) >> 5);
#       endif
    }
    else {
        return -1;  //M2DP cannot be first frame!
    }
    
    /// Data Link Layer Security: Decryption
    /// @note Still Experimental
    /// @todo make sure auth_decrypt() strips the Authentication bytes (4) and 
    ///       includes in return value.
    ///
    /// Do the decryption using the key selected above, and return error if the
    /// result is not authenticated properly or has any other sort of framing
    /// problem.  Decryption returns a value used to reframe the plain-data.
    if (m2np.header.fr_info & M2FI_DLLS) {
#   if (OT_FEATURE(DLL_SECURITY))
        ot_u8*  nonce;
        ot_int  lendiff;
        ot_uint datalen;
        nonce           = rxq.getcursor - 1;
        rxq.getcursor  += 6;
        datalen         = rxq.back - rxq.getcursor;
        lendiff         = auth_decrypt(nonce, rxq.getcursor, datalen, dlls_key_index, 0); 
        if (lendiff < 0) {
            return -1;
        }
        rxq.back -= lendiff;
#   else
        return -1;
#   endif
    }

    /// This is a callback/signal that network has accepted a frame.
	///@todo change the name of "network_sig_route" to "network_sig_frame" or something.
#   if defined(EXTF_network_sig_route)
        network_sig_route((void*)&route_val, (void*)active);
#   elif (OT_FEATURE(M2NP_CALLBACKS) == ENABLED)
        m2np.signal.route((void*)&route_val, (void*)active);
#   endif

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
        route_val = -1;     // placeholder until multiframe implemented
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
  */

#ifndef EXTF_m2np_header
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
    
    /// DLLS Nonce creation and control:
    /// @note Still Experimental
    /// If DLLS is active, we must place the Nonce here, it is either 5 or 6
    /// bytes depending on the presence of frame-control-extension field.
#   if (OT_FEATURE(DLL_SECURITY))
    if (m2np.header.fr_info & M2FI_DLLS) {
        ot_u8* nonce    = txq.putcursor;
        txq.putcursor  += 6;
        auth_putnonce(nonce, 6);
    }
#   endif

    /// Write This Source Device ID (always present in M2NP)
    m2np_put_deviceid( (ot_bool)(m2np.header.fr_info & M2FI_VID) );

    /// If required, enabled NLS.  The rules are basically the same as DLLS.
    /// @todo Experimental!
#   if (OT_FEATURE(NL_SECURITY))
    if ((m2np.header.fr_info & M2FI_CRYPTO) == M2FI_NLS) {
        //auth_setup(&txq, b00100000, q_span(&txq));
    }
#   endif
    
    /// Apply target address, if unicast enabled, and rebase it from the TX 
    /// ot_queue, which remains for the duration of the dialog.
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

    /// Perform DLLS Encryption of frame
    /// @note experimental
    /// When doing encryption, we always use the most recent key (key 0). 
    /// It is either:
    /// <LI> The matching key that came in the request </LI>
    /// <LI> The key being used with an active multi-dialog session </LI>
    /// <LI> Or the key activated by authentication API </LI>
#   if (OT_FEATURE(DLL_SECURITY))
    if (txq.getcursor[4] & M2FI_DLLS) {
        ot_u8*  nonce   = txq.getcursor + 5;
        ot_u8*  data    = txq.getcursor + 5 + 7;
        ot_uint datalen = txq.putcursor - data;
        ot_int  lendiff = auth_encrypt(nonce, data, datalen, 0, 0);     // using key_index=0 and options=0
        txq.putcursor  += lendiff;
    }
#   endif    
#   if (OT_FEATURE(NL_SECURITY))
    if (txq.getcursor[4] & M2FI_NLS) {
        ///@todo Put footer: NLS not currently supported
        //auth_userkey_encrypt(&txq.getcursor[??], &txq);
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
    //ot_queue   advq;
    //ot_u8   txadv_buffer[10];
#endif


#ifndef EXTF_m2advp_parse
OT_WEAK m2session* m2advp_parse(void) {
    ot_u8       scancode;
    ot_u8       netstate;
    ot_uni16    count;
    ot_int      slop;
    m2session*	s_next;
    ot_u16      pkt_ti;

    // Get the counter-ETA information from the inbound frame
    count.ubyte[UPPER]  = rxq.getcursor[3];
    count.ubyte[LOWER]  = rxq.getcursor[4];
    count.ushort       &= 0x7FFF;

    // stores the bg packet duration of the active channel.  We need this
    // in order to deal with timing skew.
    pkt_ti  = rm2_bgpkt_duration();

    // Account for "slop" due to clock deviation, process latency,
    // and other such things.  Thus the follow-up session is
    // either a second BG scan (if too much slop), or it is FG
    // listening for the request.
    slop = (count.ushort / OT_GPTIM_ERRDIV);
    if (slop <= pkt_ti) {
    	count.ushort   -= pkt_ti;
    	scancode        = otutils_encode_timeout(pkt_ti<<1);
        netstate    	= M2_NETSTATE_REQRX;
    }
    else {
    	count.ushort   -= slop;
        scancode        = 0x80;
        netstate        = M2_NETSTATE_REQRX | M2_NETFLAG_BG;
    }

    // ensure that count value is never negative.
    if (count.sshort < 0) {
        count.sshort = 0;
    }

    // The next session is written in-place of the current session
    s_next              = session_top();
    s_next->applet      = &dll_scan_applet;
    s_next->counter     = count.ushort;
    s_next->channel     = rxq.getcursor[2];
    s_next->netstate    = netstate;
    s_next->extra       = scancode;

    return s_next;
}
#endif



#ifndef EXTF_m2advp_open
OT_WEAK void m2advp_open(m2session* follower) {
    q_empty(&txq);
    txq.getcursor += 2;     //Bypass unused length and Link CTL bytes
    
    q_writebyte(&txq, (BG_FRAMESIZE-1));    //Dummy Length value (not actually sent)
    q_writebyte(&txq, 0);                   //Dummy Link-Control (not actually sent)
    q_writebyte(&txq, 0);                   //Dummy TX-EIRP (updated by RF driver)
    
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

