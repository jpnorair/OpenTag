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
  * @file       /otlib/m2_network.c
  * @author     JP Norair
  * @version    V1.0
  * @date       2 October 2012
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
#if OT_FEATURE(M2)

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
m2dp_struct m2dp;





/** Low-Level Network Functions
  * ============================================================================
  * - In some OSI models, these might be in the "LLC" layer of the MAC.  They
  *   fit more nicely and cleanly in this module, though.
  */
#ifndef EXTF_network_init
void network_init() {
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
m2session* network_parse_bf() {
/// Background Frame parsing: fill-in some stuff and treat it as foreground
    
    /// Vector to the appropriate Background protocol
    switch ( rxq.getcursor[1] & 1) {
        
        /// M2AdvP
        /// B0:     Subnet
        /// B1:     PID (F0)
        /// B2:     Channel for Wakeup
        /// B3-4:   Wakeup Duration
        /// B5-6:   CRC16
        case (0): {
            Twobytes    scratch;
            ot_int      slop;
            ot_u8       netstate;
            
            scratch.ubyte[UPPER]    = rxq.getcursor[3];
            scratch.ubyte[LOWER]    = rxq.getcursor[4];
            netstate                = (M2_NETSTATE_REQRX | M2_NETSTATE_INIT);
            
            // Wakeup needs to lock-on to a time offset in the future.  The
            // lock-on performance depends on the crystal accuracy, so if the
            // accuracy is not high, the background scan will need to happen
            // again, closer to the wakeup event.
            slop    = scratch.ushort / OT_GPTIM_ERRDIV;
            slop   += scratch.ushort / M2_ADV_ERRDIV;
            if (slop > M2_ADV_SLOP) {
                scratch.ushort -= slop;
                netstate       |= M2_NETFLAG_FLOOD;
            }
            
            return session_new(NULL, scratch.ushort, netstate, rxq.getcursor[2]);
            ///@todo need to put in session subnet?
        }
        
        /// M2ResP: Not currently supported!
        case (1): {
            break;
        }
    }
    
    return NULL;
}
#endif




#ifndef EXTF_network_mark_ff
void network_mark_ff() {
/// Mark a received frame as damaged by setting the frametype to 3 (11).
/// Typically, this is done only for frames in multiframe packets, which can
/// use this feature to provide selective acking.
    rxq.front[3] |= 3;
}
#endif




#ifndef EXTF_network_route_ff
ot_int network_route_ff(m2session* session) {
    ot_int route_val;

    /// Strip CRC (-2 bytes)
    rxq.front[0] -= 2;
    
    /// Acquire Flags and Protocol from the Frame Info Field
    rxq.getcursor       = &rxq.front[3];
    session->protocol   = (*rxq.getcursor & M2FI_FRTYPEMASK);
    session->flags      = *rxq.getcursor & 0xC0;
    m2np.header.fr_info = *rxq.getcursor++;
    
    /// Treat Non-Mode-2 Protocols
    /// @note Non-Mode-2 protocols not supported at this time
    if (m2np.header.fr_info & M2FI_NM2) {
        return -1;
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
    
    /// Address Control Header (Present in M2NP)
    /// Session Connection and Dialog Filtering:
    /// - if unassociated, connect now
    /// - if already connected, make sure the dialog IDs are equal
    if (m2np.header.fr_info & M2FI_ENADDR) {
        if (session->netstate & M2_NETSTATE_CONNECTED) {
            if (session->dialog_id != q_readbyte(&rxq)) {
                return -1;
            }
        }
        else {
            session->netstate  |= M2_NETSTATE_CONNECTED;
            session->subnet     = rxq.front[2];
            session->dialog_id  = q_readbyte(&rxq);
        }
        
        /// Grab global flags from Address Control
        m2np.header.addr_ctl    = q_readbyte(&rxq);
        session->flags         |= m2np.header.addr_ctl & 0x3F;
        
        /// Grab Source Address from this packet (dialog address), which is 
        /// converted to the target address in the response.
        m2np.rt.dlog.length = (m2np.header.addr_ctl & M2_FLAG_VID) ? 2 : 8;
        m2np.rt.dlog.value  = q_markbyte(&rxq, m2np.rt.dlog.length);
        
        /// Network Layer Security
        /// @note Network Layer Security not supported at this time
        if (m2np.header.addr_ctl & M2_FLAG_NLS) {
#       if (OT_FEATURE(NL_SECURITY))
#       else
            return -1;
#       endif
        }
        
        /// If unicasting, the next data is the target address, which will have
        /// the same length as the source address, and it needs to match this
        /// device's device ID (VID or UID)
        if ((m2np.header.addr_ctl & 0xC0) == 0) {
            session->netstate |= M2_NETFLAG_FIRSTRX;
            if ( m2np_idcmp(m2np.rt.dlog.length, q_markbyte(&rxq, m2np.rt.dlog.length)) == False ) {
                return -1;
            }
        }
    }

    /// Vector to the appropriate Network Layer Protocol Parser
    /// Most network protocols don't do anything except broadcast.  M2NP is the
    /// exception, and it manages various types of routing at the network layer.
    route_val = -1;
    switch (session->protocol & M2FI_FRTYPEMASK) {
    	case M2FI_FRDIALOG:
        case M2FI_FRNACK: {
            // Reset routing template
            m2np.rt.hop_code    = 0;
            m2np.rt.hop_ext     = 0;
            m2np.rt.orig.value  = NULL;
            m2np.rt.dest.value  = NULL;
            
            /// Unicast and Anycast Requests have a routing template
            /// (Not currently supported, so just move the cursor ahead)
            if ((m2np.header.addr_ctl & 0x40) == 0) {
                m2np.rt.hop_code    = q_readbyte(&rxq);
                m2np.rt.orig.length = ((m2np.rt.hop_code & M2HC_VID) != 0) ? 2 : 8;                       
                m2np.rt.dest.length = m2np.rt.orig.length;
                
                if ((m2np.rt.hop_code & M2HC_EXT) != 0) {
                    m2np.rt.hop_ext = q_readbyte(&rxq);
                }
                if ((m2np.rt.hop_code & M2HC_ORIG) != 0) {
                    m2np.rt.orig.value = q_markbyte(&rxq, m2np.rt.orig.length);
                }
                if ((m2np.rt.hop_code & M2HC_DEST) != 0) {
                    m2np.rt.dest.value = q_markbyte(&rxq, m2np.rt.dest.length);
                }
            }
        } // Note case fall-through
    
        case M2FI_STREAM: {
            /// M2DP gets parsed just like M2NP, but it uses the Network data
        	/// stored from the last M2NP frame
            route_val = m2qp_parse_frame(session);
            break;
        }
        
        case M2FI_RFU: break;
    }
    
    /// Attach footer to response, if necessary
    if (route_val >= 0) {
        m2np_footer(session);
    }

#   if defined(EXTF_network_sig_route)
        network_sig_route((void*)&route_val, (void*)session);
#   elif (OT_FEATURE(M2NP_CALLBACKS) == ENABLED)
        m2np.signal.route((void*)&route_val, (void*)session);
#   endif
    
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
void m2np_header(m2session* session, ot_u8 addressing, ot_u8 nack) {
/// Build an M2NP header, which gets forwarded in all cases to M2QP at the 
/// transport layer, and which has NM2=0, Frame_Type={0,1}

    /// 1. Prep txq, and write Frame Info & Addr Ctrl Fields (universal)
    q_empty(&txq);
    //q_start(&txq, 0, 0);
    
    txq.back                = txq.getcursor + 254; 
    q_writebyte(&txq, 0);                           // null length (placeholder only)
    q_writebyte(&txq, 0);                           // Dummy TX EIRP setting (placeholder only)
    //q_writeshort(&txq, 0x0000);
    q_writebyte(&txq, session->subnet);
    session->netstate      |= (addressing) ? 0 : M2_NETFLAG_FIRSTRX;    //Set FIRSTRX mode on Unicast
    m2np.header.fr_info     = (session->flags & 0xC0);
    addressing             |= (session->flags & 0x3F);
    m2np.header.addr_ctl    = addressing;
    m2np.header.fr_info    |= nack;
    m2np.header.fr_info    |= M2FI_ENADDR;
    q_writebyte(&txq, m2np.header.fr_info);
    
    /// 2. If required, enable DLLS encryption.  AES128 is the only currently
    ///    supported crypto in OpenTag.  It will be applied in m2np_footer().
    ///    If DLLS is enabled, NLS will be forced-off, because both of them
    ///    cannot be active on the same frame.
    ///    @todo Experimental!
#   if (OT_FEATURE(DLL_SECURITY))
    if (m2np.header.fr_info & M2FI_DLLS) {
        auth_setup(&txq, b00100000, 249);
        m2np.header.addr_ctl &= ~M2_FLAG_NLS;
    }
#   endif
    
    /// 3. Write Dialog, Addr Ctrl, and Source Address (always included in M2NP)
    q_writebyte(&txq, session->dialog_id);
    q_writebyte(&txq, m2np.header.addr_ctl);
    m2np_put_deviceid( (ot_bool)(m2np.header.addr_ctl & M2AC_VID) );

    /// 4. If required, enabled NLS.  The rules are basically the same as DLLS.
    ///    @todo Experimental!
#   if (OT_FEATURE(NL_SECURITY))
    if (m2np.header.addr_ctl & M2_FLAG_NLS) {
        auth_setup(&txq, b00100000, (txq.putcursor-txq.getcursor));
    }
#   endif
        
    /// 5. Apply target address, if unicast enabled, and rebase it from the TX 
    ///    Queue, which remains for the duration of the dialog
    if ((m2np.header.addr_ctl & 0xC0) == 0) {
        q_writestring(&txq, m2np.rt.dlog.value, m2np.rt.dlog.length);
        m2np.rt.dlog.value = (txq.putcursor - m2np.rt.dlog.length);
    }
        
    /// 6. Apply Multihop routing template, if unicast or anycast enabled
    if ((m2np.header.addr_ctl & 0x40) == 0) {
        ot_u8   hopmask = M2HC_ORIG;
        ot_u8   id_num  = 1;
        id_tmpl* id     = &m2np.rt.orig;
            
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
    } 
}
#endif



#ifndef EXTF_m2np_footer
void m2np_footer() {
#   if (OT_FEATURE(DLL_SECURITY))
    /// Add DLLS Padding and run encryption
    if (m2np.header.fr_info & M2_FLAG_DLLS) {
        ///@todo Implement AES128 DLLS stuff
        auth_userkey_encrypt(&txq.getcursor[4], &txq);
        goto m2np_footer_END;
    }
#   endif    
#   if (OT_FEATURE(NL_SECURITY))
    if (m2np.header.addr_ctl & M2_FLAG_NLS) {
        ///@todo Put footer: NLS not currently supported
        auth_userkey_encrypt(&txq.getcursor[6], &txq);
    }
#   endif

    m2np_footer_END:

    /// Load frame size into length byte, including -1 for length byte and +2
    /// for CRC (total, +1).
    txq.getcursor[0] = (txq.putcursor - txq.getcursor) + 1;
}
#endif



#ifndef EXTF_m2np_put_device_id
void m2np_put_deviceid(ot_bool use_vid) {
    vlFILE* fp;
    
    //file 0=network_settings, 1=device_features
    fp = ISF_open_su( (ot_u8)(use_vid == False) );
    ///@todo assert fp

    q_writeshort_be( &txq, vl_read(fp, 0) );
    if (use_vid == False) {
    	q_writeshort_be( &txq, vl_read(fp, 2) );
    	q_writeshort_be( &txq, vl_read(fp, 4) );
    	q_writeshort_be( &txq, vl_read(fp, 6) );
    }
    
    vl_close(fp);
}
#endif


#ifndef EXTF_m2np_idcmp
ot_bool m2np_idcmp(ot_int length, void* id) {
    ot_bool check   = True;
    ot_u8   use_uid = (length == 8);
    vlFILE* fp;
    
    //file 0=network_settings, 1=device_features
    fp      = ISF_open_su( use_uid );
    check  &= ( ((ot_u16*)id)[0] == vl_read(fp, 0));

    if (use_uid) {
        check &= ( ((ot_u16*)id)[1] == vl_read(fp, 2) );
        check &= ( ((ot_u16*)id)[2] == vl_read(fp, 4) );
        check &= ( ((ot_u16*)id)[3] == vl_read(fp, 6) );
    }
    
    vl_close(fp);
    return check;
}
#endif






/** M2AdvP Network Functions
  * ============================================================================
  * - M2AdvP = Advertising Protocol, i.e. flooding for group synchronization.
  */
  
#if (SYS_FLOOD == ENABLED)
    //Queue   advq;
    //ot_u8   txadv_buffer[10];
#endif


#ifndef EXTF_m2advp_open
void m2advp_open(m2session* session) {
    //q_start(&txq, 1, 0);
    //txq.front[0] = 7;
    
    q_empty(&txq);
    txq.getcursor++;
    q_writebyte(&txq, 7);
    
    /// This byte gets overwritten in the driver with EIRP value.
    /// The value 6 is needed by some sniffers during test, otherwise.
    q_writebyte(&txq, 6);
    
    /// This byte is two nibbles: Subnet specifier and AdvP ID (F)
    q_writebyte(&txq, (session->subnet | 0x0F));
    
    /// The rest is the AdvP payload
    q_writebyte(&txq, session->channel);
    q_writeshort(&txq, session->counter);
}
#endif


#ifndef EXTF_m2advp_update
void m2advp_update(ot_u16 countdown) {
    txq.getcursor       = &txq.front[1];
    txq.getcursor[3]    = ((ot_u8*)&countdown)[UPPER];
    txq.getcursor[4]    = ((ot_u8*)&countdown)[LOWER];
    //txq.putcursor       = &txq.getcursor[5];
}
#endif


#ifndef EXTF_m2advp_close
void m2advp_close() {
#if (SYS_FLOOD == ENABLED)
    /// Restore original TXQ
    //q_copy(&txq, &advq);
#endif
}
#endif


#if (0) //ifndef EXTF_m2advp_init_flood
ot_int m2advp_init_flood(m2session* session, advert_tmpl* adv_tmpl) {
#if (SYS_FLOOD == ENABLED) 
#   ifdef _NETWORK_DEBUG
        // Bug catcher
        if (adv_tmpl->duration > (32767 /* -RADIO_TURNON_LAG */ )) {
            //OT_LOGFAIL();
            return -1;
        }
#   endif

    /// Set Netstate to match advertising type
    session->netstate = (   M2_NETFLAG_FLOOD | M2_NETSTATE_REQTX | \
                            M2_NETSTATE_INIT /* | M2_NETSTATE_SYNCED */   );

    // Store existing TXQ (bit of a hack)
    //q_copy(&advq, &txq);
    
    /// Reinit txq to the advertising buffer, and load data that will stay the
    /// same for all packets in the flood.
    //q_init(&txq, txadv_buffer, 10);
    
    txq.front[0]    = session->subnet;
    txq.front[1]    = M2_PROTOCOL_M2ADVP;
    txq.front[2]    = session->channel;
    txq.front[3]    = ((ot_u8*)&adv_tmpl->duration)[UPPER];
    txq.front[4]    = ((ot_u8*)&adv_tmpl->duration)[LOWER];
 
    return 0;
#else
    return -1;
#endif
}
#endif







/** M2DP Network Functions
  * ============================================================================
  */

#ifndef EXTF_m2dp_append
void m2dp_append() {
///@note This function is experimental (untested, work-in-progress)

    // Mark LAST FRAME to continue to this one, and then move getcursor to the
    // front of this frame.
    ///@todo this method might need to change, depending on how queues end up
    ///      being managed.
    ot_u8 subnet;

    m2np.header.fr_info    &= ~(M2FI_ENADDR | M2FI_FRTYPEMASK);
    m2np.header.fr_info    |= (M2FI_STREAM);

    // M2DP will use DLLS on an existing key, such as one specified by NLS from
    // the M2NP Frame before it.
#   if (OT_FEATURE(NL_SECURITY))
    m2np.header.fr_info    |= ((m2np.header.addr_ctl & M2_FLAG_NLS) << 2);
    m2np.header.addr_ctl   &= ~M2_FLAG_NLS;
#   endif
    
    // Basic header stuff:
    subnet                  = txq.getcursor[2];
    txq.getcursor[3]       |= M2FI_FRCONT;
    txq.getcursor           = txq.putcursor;
    txq.back                = txq.putcursor + 254; 
    *txq.putcursor++        = 0;                    // length placeholder
    *txq.putcursor++        = 0;                    // TX EIRP placeholder
    *txq.putcursor++        = subnet;
    *txq.putcursor++        = m2np.header.fr_info;
    txq.length             += 4;
    
    // DLLS header information (gets adjusted on m2dp_close())
#   if (OT_FEATURE(DLL_SECURITY))
    if (m2np.header.fr_info & M2FI_DLLS) {
        auth_setup(&txq, 0, 5);
    }
#   endif

}
#endif



#ifndef EXTF_m2dp_footer
void m2dp_footer() {
/// M2DP is similar enough to M2NP that the same footer function may be used.
    m2np_footer();
}
#endif



#endif

