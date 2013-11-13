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
/// Mark a received frame as damaged by using the reserved position in the
/// Link Control bits.  This will never be transmitted over the air.
    rxq.front[1] |= M2LC_DMGMARK;
}
#endif




#ifndef EXTF_network_cont_dialog
m2session* network_cont_dialog(ot_app applet, ot_uint wait) {
    m2session* next;
    m2session* active;
    active  = session_top();
    next    = session_new(  applet, 
                            CLK2TI(dll.comm.tc) + wait,
                            (M2_NETSTATE_REQRX | M2_NETSTATE_ASSOCIATED),
                            active->channel     );
    
    next->extra     = active->extra;
    next->dialog_id = active->dialog_id++;
    next->subnet    = active->subnet;
    next->flags     = active->flags;
}
#endif



#ifndef EXTF_network_route_ff
ot_int network_route_ff(m2session* active) {
    ot_int route_val;

    /// Strip CRC bytes from the end of the message.  RS bytes are stripped by
    /// the decoder, if RS is enabled/used.
    rxq.front[0]       -= 2;
    
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
        if (active->netstate & M2_NETSTATE_CONNECTED) {
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
        m2np_footer( /* active */);
    }

#   if defined(EXTF_network_sig_route)
        network_sig_route((void*)&route_val, (void*)active);
#   elif (OT_FEATURE(M2NP_CALLBACKS) == ENABLED)
        m2np.signal.route((void*)&route_val, (void*)active);
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
void m2np_header(m2session* active, ot_u8 addressing, ot_u8 nack) {
/// Build an M2NP header, which gets forwarded in all cases to M2QP at the 
/// transport layer, and which has NM2=0, Frame_Type={0,1}

    /// Prep txq 
    q_empty(&txq);
    //q_start(&txq, 0, 0);
    
    /// If blockcoding not enabled, don't let it get used.
    /// max frame with blockcoding is 192 data bytes, max with CRC is 254.
    {   ot_int maxframe;
#       if (M2_FEATURE(RSCODE))
        maxframe        = (active->flags & M2_FLAG_RSCODE) ? 192 : 254;
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
    q_writelong(&txq, (ot_u32)active->subnet);
    
    /// Set FIRSTRX mode on Unicast
    active->netstate       |= (addressing) ? 0 : M2_NETFLAG_FIRSTRX;
    
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
        auth_setup(&txq, b00100000, (txq.putcursor-txq.getcursor));
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
#endif



#ifndef EXTF_m2np_footer
void m2np_footer() {
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
    /// will be added to the frame length field
    m2np_footer_END:
    txq.getcursor[0] = q_span(&txq);
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
void m2advp_open(m2session* active, ot_u16 duration) {
    //q_start(&txq, 1, 0);
    //txq.front[0] = 7;
    
    q_empty(&txq);
    txq.getcursor++;
    q_writebyte(&txq, 7);
    
    /// This byte gets overwritten in the driver with EIRP value.
    /// The value 6 is needed by some sniffers during test, otherwise.
    q_writebyte(&txq, 6);
    
    /// This byte is two nibbles: Subnet specifier and AdvP ID (F)
    q_writebyte(&txq, (active->subnet | 0x0F));
    
    /// The rest is the AdvP payload
    q_writebyte(&txq, active->channel);
    q_writeshort(&txq, duration);
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
ot_int m2advp_init_flood(m2session* active, advert_tmpl* adv_tmpl) {
#if (SYS_FLOOD == ENABLED) 
#   ifdef _NETWORK_DEBUG
        // Bug catcher
        if (adv_tmpl->duration > (32767 /* -RADIO_TURNON_LAG */ )) {
            //OT_LOGFAIL();
            return -1;
        }
#   endif

    /// Set Netstate to match advertising type
    active->netstate = (   M2_NETFLAG_FLOOD | M2_NETSTATE_REQTX | \
                            M2_NETSTATE_INIT /* | M2_NETSTATE_SYNCED */   );

    // Store existing TXQ (bit of a hack)
    //q_copy(&advq, &txq);
    
    /// Reinit txq to the advertising buffer, and load data that will stay the
    /// same for all packets in the flood.
    //q_init(&txq, txadv_buffer, 10);
    
    txq.front[0]    = active->subnet;
    txq.front[1]    = M2_PROTOCOL_M2ADVP;
    txq.front[2]    = active->channel;
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

    m2np.header.fr_info |= M2FI_STREAM;

    // M2DP will use DLLS on an existing key, such as one specified by NLS from
    // the M2NP Frame before it.
    
#   if (OT_FEATURE(NL_SECURITY))
    if (m2np.header.addr_ctl & M2_FLAG_NLS) {
        m2np.header.addr_ctl &= ~M2_FLAG_NLS;
        m2np.header.fr_info  |= M2FI_DLLS;
    }
#   endif
    
    ///@todo sort out FRCONT stuff
    
    // Basic header stuff:
    subnet              = txq.getcursor[3];
    txq.getcursor[1]   |= (1<<7);           // FRCONT as presently defined
    txq.getcursor       = txq.putcursor;
    
    /// @todo need to account for blockcoding
    txq.back            = txq.putcursor + (M2_PARAM_MAXFRAME-2);    // save 2 bytes for CRC16
    
    *txq.putcursor++    = 0;                                        // length placeholder
    *txq.putcursor++    = 0;                                        // TX EIRP placeholder
    *txq.putcursor++    = subnet;
    *txq.putcursor++    = m2np.header.fr_info;
 //#txq.length         += 4;
    
    ///@todo not sure how much of this stuff below could be subroutined
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

