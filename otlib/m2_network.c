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
  * @file       /OTlib/m2_network.c
  * @author     JP Norair
  * @version    V1.0
  * @date       2 November 2011
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

#include "m2_network.h"
#include "m2_transport.h"

#include "OT_config.h"
#include "OT_platform.h"
#include "OT_utils.h"
#include "OTAPI_tmpl.h"

#include "auth.h"
#include "buffers.h"
#include "queue.h"
#include "system.h"         //including system.h just for some constants
#include "veelite.h"


//#ifdef DEBUG_ON
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
        case (0): {
            Twobytes    scratch;
            ot_u8       netstate;
            ot_int      slop;
            
            scratch.ubyte[UPPER]    = rxq.getcursor[3];
            scratch.ubyte[LOWER]    = rxq.getcursor[4];
            netstate                = (M2_NETSTATE_REQRX | M2_NETSTATE_INIT);
            slop                    = scratch.ushort / OT_GPTIM_ERRDIV;
            slop                   += scratch.ushort / M2_ADV_ERRDIV;
            
            if (slop > M2_ADV_SLOP) {
                scratch.ushort -= slop;
                netstate       |= M2_NETFLAG_FLOOD;
            }

            return session_new( scratch.ushort, netstate, rxq.getcursor[2]);
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



#ifndef EXTF_network_route_ff
ot_int network_route_ff(m2session* session) {
    ot_int route_val;

    /// Strip CRC (-2 bytes)
    rxq.front[0] -= 2;
    
    /// Acquire Flags and Protocol from the Frame Info Field
    session->protocol   = (rxq.front[3] & M2FI_FRTYPEMASK);
    m2np.header.fr_info = rxq.front[3];
    session->flags      = rxq.front[3] & 0xC0;
    rxq.getcursor       = &rxq.front[4]; 
    
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
    /// - if unassociated or synced, connect now
    /// - if already connected, make sure the dialog IDs are equal
    if (m2np.header.fr_info & M2FI_ENADDR) {
        switch (session->netstate & 0x03) {
            case M2_NETSTATE_UNASSOC: // same handling as synced
            case M2_NETSTATE_SYNCED:
                session->netstate  |= M2_NETSTATE_CONNECTED;
                session->subnet     = rxq.front[2];
                session->dialog_id  = q_readbyte(&rxq);
                break;
                                        
            default:
                if (session->dialog_id != q_readbyte(&rxq)) 
                    return -1;
                break;
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
            if ( !m2np_idcmp(m2np.rt.dlog.length, q_markbyte(&rxq, m2np.rt.dlog.length)) ) {
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
            route_val = m2qp_parse_frame(session);   // Routing has passed!
            break;
        }
    
        case M2FI_STREAM: {
            ///@todo currently, do nothing, just assume that getcursor is where
            ///      it needs to be, at the front of the data.
            route_val           = m2dp_parse_dspkt(session);
            break;
        }
        
        case M2FI_RFU: break;
    }
    
    /// Attach footer to response, if necessary
    if (route_val >= 0) {
        m2np_footer(session);
    }

#   if (OT_FEATURE(M2NP_CALLBACKS) == ENABLED) && \
        !defined(EXTF_network_sig_route)
        m2np.signal.route(route_val, session->protocol);
#   elif defined(EXTF_network_sig_route)
        network_sig_route(route_val, session->protocol);
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

    /// Prep txq, and write Frame Info & Addr Ctrl Fields (universal)
    q_start(&txq, 0, 0);
    //q_writebyte(&txq, 0);                           // null length (placeholder only)
    //q_writebyte(&txq, 0);                           // Dummy TX EIRP setting (placeholder only)
    q_writeshort(&txq, 0x0000);
    q_writebyte(&txq, session->subnet);
    addressing             |= (session->flags & 0x3F);
    m2np.header.fr_info     = session->flags & 0xC0;
    m2np.header.fr_info    += nack;
    {
        ot_u8 frspec            = (session->protocol == 0);
        m2np.header.fr_info    |= frspec ? M2FI_ENADDR : M2FI_STREAM;
        m2np.header.addr_ctl    = frspec ? addressing : 0;
    }
    q_writebyte(&txq, m2np.header.fr_info);
    
#   if (OT_FEATURE(DLL_SECURITY))
    if (m2np.header.fr_info & M2FI_DLLS) {
        ///@todo experimental
        AES_load_static_key(ISF_ID(user_authentication_key), (ot_u32*)rxq.front);
        AES_keyschedule_enc((ot_u32*)rxq.front, (ot_u32*)(rxq.front+16));
        AES_encrypt(&txq.front[4], &txq.front[4], (ot_u32*)(rxq.front+16));
    }
#   endif
    
    /// Write M2NP Address Templates
    if (m2np.header.fr_info & M2FI_ENADDR) {
        // Add Dialog ID, Address Ctrl Field, and Source Address
        q_writebyte(&txq, session->dialog_id);
        q_writebyte(&txq, m2np.header.addr_ctl);
        m2np_put_deviceid( (ot_bool)(m2np.header.addr_ctl & M2AC_VID) );
        
        ///@todo Put NLS auth header (NLS not currently supported)
#       if (OT_FEATURE(NL_SECURITY))
        if (m2np.header.addr_ctl & M2_FLAG_NLS) {
        }
#       endif
        
        /// Unicast: add Target address, and rebase it from the TX Queue, which
        /// is non-volatile for the remaining duration of the dialog
        if ((m2np.header.addr_ctl & 0xC0) == 0) {
            q_writestring(&txq, m2np.rt.dlog.value, m2np.rt.dlog.length);
            m2np.rt.dlog.value = (txq.putcursor - m2np.rt.dlog.length);
        }
        
        // Anycast or unicast: Multi-Hopping
        if ((m2np.header.addr_ctl & 0x40) == 0) {
            q_writebyte(&txq, m2np.rt.hop_code);
            
            if ((m2np.rt.hop_code & M2HC_EXT) != 0) {
                q_writebyte(&txq, m2np.rt.hop_ext);
            }
            if ((m2np.rt.hop_code & M2HC_ORIG) != 0) {
                ot_u8* temp = txq.putcursor;
                q_writestring(&txq, m2np.rt.orig.value, m2np.rt.orig.length);
                m2np.rt.orig.value = temp;
            }
            if ((m2np.rt.hop_code & M2HC_DEST) != 0) {
                ot_u8* temp = txq.putcursor;
                q_writestring(&txq, m2np.rt.dest.value, m2np.rt.dest.length);
                m2np.rt.dest.value = temp;
            }
        }
    }
}
#endif



#ifndef EXTF_m2np_footer
void m2np_footer(m2session* session) {
#   if (OT_FEATURE(NL_SECURITY))
    if (m2np.header.addr_ctl & M2_FLAG_NLS) {
        ///@todo Put footer: NLS not currently supported
    }
#   endif

#   if (OT_FEATURE(DLL_SECURITY))
    ///DLLS footer is just padding, goes in automatically during encryption
    //if (m2np.header.fr_info & M2_FLAG_DLLS) {
    //}
#   endif
    
    /// Add payload (+txq.length), subtract length byte (-1), add CRC (+2)
    txq.front[0] = txq.length + 1;
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
    ot_bool check = True;
    ot_u8   use_uid = (length == 8);
    vlFILE* fp;
    
    //file 0=network_settings, 1=device_features
    fp = ISF_open_su( use_uid );
    check &= ( ((ot_u16*)id)[0] == vl_read(fp, 0));

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
    Queue   advq;
    ot_u8   txadv_buffer[10];
#endif


//void network_make_m2advp(m2session* session) {
#ifndef EXTF_m2advp_open
void m2advp_open(m2session* session) {
    q_start(&txq, 1, 0);
    txq.front[0] = 7;
    q_writebyte(&txq, session->subnet);
    q_writebyte(&txq, 0xF0);
    q_writebyte(&txq, session->channel);
    q_writeshort(&txq, session->counter);
}
#endif


#ifndef EXTF_m2advp_close
void m2advp_close() {
#if (SYS_FLOOD == ENABLED)
    /// Restore original TXQ
    q_copy(&txq, &advq);
#endif
}
#endif


#ifndef EXTF_m2advp_init_flood
ot_int m2advp_init_flood(m2session* session, ot_u16 schedule) {
#if (SYS_FLOOD == ENABLED) 
#   ifdef DEBUG_ON
        // Bug catcher
        if (session->counter > (32767 /* -RADIO_TURNON_LAG */ )) {
            //OT_LOGFAIL();
            return -1;
        }
#   endif

    /// Set Netstate to match advertising type
    session->netstate = (   M2_NETFLAG_FLOOD | M2_NETSTATE_REQTX | \
                            M2_NETSTATE_INIT | M2_NETSTATE_SYNCED   );

    /// Store existing TXQ (bit of a hack)
    q_copy(&advq, &txq);
    
    /// Reinit txq to the advertising buffer, and load data that will stay the
    /// same for all packets in the flood.
    q_init(&txq, txadv_buffer, 10);
    
    txq.front[0]    = session->subnet;
    txq.front[1]    = M2_PROTOCOL_M2ADVP;
    txq.front[2]    = session->channel;
    txq.front[3]    = ((ot_u8*)&schedule)[UPPER];
    txq.front[4]    = ((ot_u8*)&schedule)[LOWER];
 
    return 0;
#else
    return -1;
#endif
}
#endif





/** M2DP Network Functions
  * ============================================================================
  * - M2DP = Mode 2 Datastream Protocol
  * - Very little overhead, good for arbitrary data encapsulation
  * - Supports multi-frame packets and multi-packet streams
  * - No inherent flow control or ACK (requires a session layer to do it)
  * 
  * @note OpenTag uses a spec-legal, partial implementation of M2DP, including:
  * - Encapsulated data is restricted to supported application subprotocols
  * - Requires in-order frame delivery for datastreams that span multiple frames
  * - Usage with multiframe packets should work but is untested
  */
#ifndef EXTF_m2dp_open
void m2dp_open(ot_u8 frame_id, m2session* session) {
    q_empty(&txq);
    txq.front[0] = 0;
    q_writebyte(&txq, session->subnet);
    q_writebyte(&txq, 0x60);
    q_writebyte(&txq, (ot_u8)session->counter);    
    q_writebyte(&txq, frame_id);
}
#endif


#ifndef EXTF_m2dp_dsproc
void m2dp_dsproc() {
/// @note: The code below is basically copied from ndef_parse_record() in
///        the NDEF module.  At some point this code might be consolidated
///        and reused across both modules.
#if (OT_FEATURE(ALP) == ENABLED)
    ot_u8*      header;
    alp_record  in_rec;

    while ( ((m2dp.out_rec.flags & ALP_FLAG_ME) == 0) && \
            (rxq.getcursor > rxq.back) ) {
        /// Get ALP directive header (Damn similar to NDEF)
        in_rec.flags            = q_readbyte(&rxq);
        in_rec.payload_length   = q_readbyte(&rxq);
        in_rec.dir_id           = q_readbyte(&rxq);
        in_rec.dir_cmd          = q_readbyte(&rxq);
        
        /// Set Universal Output Record Flags
        m2dp.out_rec.flags     |= (in_rec.flags & ALP_FLAG_ME);
    
        /// Leave room for directive header data (put in later)
        header                  = txq.putcursor;
        txq.putcursor          += 4;

        /// Process The input record and yield output
        /// @todo change the AUTH_GUEST when the Authentication and security
        ///       mechanisms are actually implemented.
        alp_proc(&in_rec, &(m2dp.out_rec), &rxq, &txq, AUTH_GUEST);
    
        /// If there's no output data, rewind output queue 
        /// Else, update header with output from alp_proc()
        if (m2dp.out_rec.payload_length == 0) {
            dir_out.putcursor = header; ///@todo check if this should be txq
        }
        else {
            ///@note Line below is temporary hack for pre-release crippleware.
            ///      If a record spills over (indicates chunking), then mark the
            ///      Message End bit.  Remaining input records are still parsed,
            ///      but the output is stopped here.
            m2dp.out_rec.flags |= (m2dp.out_rec.flags & ALP_FLAG_CF) << 1;  
        
            header[0] = m2dp.out_rec.flags;
            header[1] = m2dp.out_rec.payload_length;
            header[2] = m2dp.out_rec.dir_id;
            header[3] = m2dp.out_rec.dir_cmd;
        }
    }
#endif
}
#endif


#ifndef EXTF_m2dp_parse_dspkt
ot_int m2dp_parse_dspkt(m2session* session) {
#if (OT_FEATURE(ALP) == ENABLED)
    /// Put together the beginning of the ACK request, as long as the ACK is
    /// enabled (set up by the M2QP handshaking).
    if ((m2dp.dscfg.ctl & M2DS_DISABLE_ACKREQ) == 0) {
        //ot_u8*  cmd_ptr;
        //ot_int  cmd_length;
        
        /// Step 1: Build the base of the response (everything except the frame numbers)
        /// @todo - 1a: configure necessary MAC variables for timeouts
        /// - 1b: prepare the header of the ack response (generic)
        /// - 1c: prepare the command-specific M2QP data
        
        m2np_header(session, 0, 2);                 // (1b) 0 = unicast, 2 = request
        //cmd_ptr     = txq.putcursor;
        //cmd_length  = txq.length; 
        
        if (m2dp.dscfg.ctl & M2DS_DISABLE_ACKRESP) {     // (1c)
            q_writebyte(&txq, b10101010);
            q_writebyte(&txq, b00000010);   //command extension
        }
        else {
            q_writebyte(&txq, b00101010);
        }
        
        
        /// Step 2: Figure out which M2DP frames are damaged, and write to the 
        /// response.  The current version of OpenTag supports only 1 frame per
        /// packet, so the implementation here is a shortcut.  In the future,
        /// OT might support multiple frames per packet.  There is some sample
        /// code commented-out that can ID frame errors for multiframe packets.
        /// It working in theory but hasn't been tested!
        
        /// @note the function m2dp_mark_dsframe() is called within the system.c
        /// RX process when a frame is detected bad.  The value in dmg_count is
        /// incremented on each call of m2dp_mark_dsframe().
        q_writebyte(&txq, (ot_u8)m2dp.dscfg.dmg_count);
        if (m2dp.dscfg.dmg_count != 0) {
            q_writebyte(&txq, 0);   //damaged frame always is frame 0
        }
    }
    
    /// If the packet is error free, process the ALPs inside it.
    if (m2dp.dscfg.dmg_count == 0) {
        m2dp_dsproc();
    }
        
    ///@todo I probably need to think about a way to engage the "Scrap" features
    ///      of the ACK control in the event when data integrity is a continuous
    ///      problem.  "Scrapping" is not a mandatory feature for DASH7, but it
    ///      can make transfers more efficient.
    
    // reset dscfg for next packet
    m2dp.dscfg.dmg_count = 0;
    
    
    ///@todo Need to correlate a return value to m2dp_dsproc().  It probably
    ///      only needs to be 0/-1.
    return 0;
#else
    return -1;
#endif
}
#endif


#ifndef EXTF_m2dp_mark_dsframe
void m2dp_mark_dsframe(m2session* session) {
/// Mark the frame info as FF, which will be parsed later as damaged frame, and
/// increment the damage counter.
    rxq.front[3] = 0xFF;
    m2dp.dscfg.dmg_count++;
}
#endif

























/* Method for multiframe packet marking of datastreams (for next version)

// - Write number of damaged frames to ACK request
// - Set damaged frame count back to zero, for next packet
// - Start at beginning of packet (manual rx queue addressing)
// - Determine number of frames in the packet (either max, or num frames left)
// - Damaged frames are marked during RX. ACK the damaged, marked frames.

q_writebyte(&txq, (ot_u8)m2dp.dscfg.dmg_count);
m2dp.dscfg.dmg_count    = 0;                    // reset for next packet
data_ptr                = rxq.front; 

if (m2dp.dscfg.fr_count < m2dp.dscfg.fr_per_pkt) {
    m2dp.dscfg.fr_per_pkt = m2dp.dscfg.fr_left;
}
{
    ot_u8 i;
    ot_u8 test_is_good;
    for (i=0; i<m2dp.dscfg.fr_per_pkt; i++) {
        test_is_good        = ((data_ptr[3] & 0x0F) == 0);  //byte[3] is "frame control"
        m2dp.dscfg.fr_left -= test_is_good;
        
        if (!test_is_good) {
            q_writebyte(&txq, i);
        }
        data_ptr += 255;                //255 is hard-coded max frame length
    }
}
*/









/*
void sub_ds_stitch() {
/// Go through all the frames and make the datastream contiguous
    ot_u8*  dest_ptr;
    
    rxq.front           = rxq_data;
    dest_ptr            = rxq_data;
    m2dp.dscfg.fr_left  = m2dp.dscfg.fr_count;
    
    while (m2dp.dscfg.fr_left > 0) {    
        ot_int copy_length;
        rxq.getcursor = rxq.front;

        if (network_route_ff(session) != 0) {
            //return error, scrap
            return b0100000;
        }
        rxq.getcursor++;    // bypass frame ID

        /// Remove the frame overhead and just leave the datastream
        /// 1. Following network_route_ff(), the number of overhead bytes = 
        ///    (1 + rxq.getcursor - rxq.front).  The +1 is factored in above.
        /// 2. The datastream bytes = frame length - overhead bytes
        /// 3. The frame length = rxq.front[0]
        copy_length = (rxq.front[0]-(rxq.getcursor-rxq.front));
        platform_memcpy(dest_ptr, rxq.getcursor, copy_length);
        
        /// network_route_ff() will place the putcursor after the frame data,
        /// so prepare re-align front (and getcursor) to the next frame)
        rxq.front = rxq.putcursor;
        m2dp.dscfg.fr_left--;
    }
    
    //reset queue variables for next operation
    rxq.front       = rxq_data;
    rxq.getcursor   = rxq_data;
    rxq.length      = m2dp.dscfg.data_total;
    
    return 0;
}
*/



