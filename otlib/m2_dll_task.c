/* Copyright 2012 JP Norair
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
  * @file       /otlib/m2_dll_task.c
  * @author     JP Norair
  * @version    R101
  * @date       30 Oct 2012
  * @brief      Data Link Layer Task for DASH7
  * @ingroup    DLL
  *
  * If you are searching for a brainfuck and you are coming up short, rest easy
  * now, you have found one.  The Data Link Layer is big and ugly.  Proceed 
  * with caution.  Working on DLL is about as esoteric as producing a ten year
  * long catalog of prog metal albums that exclusively recount topics from the
  * "Stargate" Sci-Fi franchise in the lyrics.  However, that task was also 
  * proven to be possible, by some talented Norwegians (bonus points for ID).
  *
  * <PRE>
  * " In a trance-like possession I arose from sleep
  *   Reading pictures and codes that I recall
  *   Complex technical structures appear in my mind
  *   Recess the past
  *   Drawing back in time "
  * </PRE>
  ******************************************************************************
  */


#include "OT_config.h"

// Don't compile this if m2 is not enabled
#if OT_FEATURE(M2)

#include "OT_types.h"
#include "OT_utils.h"
#include "OT_platform.h"


#include "m2_dll.h"
#include "system.h"

#include "auth.h"
#include "buffers.h"
#include "m2_network.h"
#include "m2_transport.h"
#include "external.h"
#include "queue.h"
#include "radio.h"
#include "session.h"
#include "veelite.h"




#if defined(EXTF_dll_sig_rfinit)
#   define DLL_SIG_RFINIT(CODE)                 dll_sig_rfinit(CODE)
#elif (OT_FEATURE(DLLRF_CALLBACKS) == ENABLED)
#   define DLL_SIG_RFINIT(CODE)                 dll.sig.rfinit(CODE)
#else
#   define DLL_SIG_RFINIT(CODE)                 while(0)
#endif

#if defined(EXTF_dll_sig_rfterminate)
#   define DLL_SIG_RFTERMINATE(CODE1, CODE2)    dll_sig_rfterminate(CODE1, CODE2)
#elif (OT_FEATURE(DLLRF_CALLBACKS) == ENABLED)
#   define DLL_SIG_RFTERMINATE(CODE1, CODE2)    dll.sig.rfterminate(CODE1, CODE2)
#else
#   define DLL_SIG_RFTERMINATE(CODE1, CODE2)    while(0)
#endif





m2dll_struct    dll;

void sub_dll_flush();
void sub_scan_channel(ot_u8 task_offset);

void sub_init_bscan();
void sub_init_fscan();
void sub_init_ftx();
void sub_init_btx();
void sub_timeout_scan();
void sub_dll_txcsma();

void rfevt_bscan(ot_int scode, ot_int fcode);
void rfevt_frx(ot_int pcode, ot_int fcode);
void rfevt_txcsma(ot_int pcode, ot_int tcode);
void rfevt_ftx(ot_int pcode, ot_int scratch);
void rfevt_btx(ot_int flcode, ot_int scratch);


void sub_processing();
void sub_activate();
void sub_init_rx(ot_u8 is_brx);
void sub_init_tx(ot_u8 is_btx);












/** Flow & Congestion Control Subroutines
  * ============================================================================
  */

ot_u8 sub_default_idle();

/** @brief Performs subnet filtering (per Mode 2 spec) and returns status
  * @retval ot_bool     True/False on received frame subnet passes/fails filter
  * @ingroup System
  */
ot_bool sub_mac_filter();


/** @brief Scrambles the response-channel-list in order to improve collision
  *        avoidance when multiple response channels are available.
  * @ingroup System
  */
void sub_csma_scramble();


/** @brief Initializes the flow/congestion control sequence
  * @retval ot_uint     Initial TX backoff, in ticks
  * @ingroup System
  */
CLK_UNIT sub_fcinit();


/** @brief Evaluates the TX slot usage based on the quality of the query
  * @param  query_score (ot_int) To be defined
  * @retval none        
  * @ingroup System
  * @note Optional, not implemented yet
  */
void sub_fceval(ot_int query_score);


/** @brief Processes the flow/congestion control sequence following initialization
  * @retval ot_uint     Subsequent TX backoff, in ticks
  * @ingroup System
  */
CLK_UNIT sub_fcloop();


/** @brief Initializes a RIGD sequence and determines initial TX offset
  * @retval ot_uint     Number of ticks until TX for new slot should commence
  * @ingroup System
  * @sa sub_fcinit(), sub_rigd_nextslot()
  *
  * @note JP pronounces this "rig-dee."  RIGD is one reason why DASH7 Mode 2
  * CSMA-CA shits all over 802.15.4 CSMA-CA.
  *
  * This is called by sub_fcinit() when RIGD is specified by the request.  RIGD
  * is "Random Increase Geometric Decay" which divides the active contention 
  * period (dll.comm.tca) into subslots of decaying duration and picks random
  * TX offsets within each subslot.
  */
CLK_UNIT sub_rigd_newslot();


/** @brief Continues an ongoing RIGD sequence and determines subslot TX offset
  * @retval ot_uint     Number of ticks until TX for next slot should commence
  * @ingroup System
  * @sa sub_fcloop(), sub_rigd_newslot()
  *
  * This is called by sub_fcloop() and is much like sub_rigd_newslot().
  */
ot_uint sub_rigd_nextslot();


/** @brief Picks a TX offset based on the AIND (or RAIND) method
  * @retval ot_uint     Number of ticks until TX for next slot should commence
  * @ingroup System
  * @sa sub_fcinit(), sub_fcloop()
  *
  * @note JP pronounces these "ain-dee" and "rain-dee"
  *
  * This is called by sub_fcinit() and sub_fcloop() when a RAIND or AIND process
  * is moving along.  RAIND means "Randomized Additive Increase No Decay," and
  * AIND is just without the "Randomized."  Basically, it uses a fixed subslot
  * size which can be initially offset with a random value (or not).
  */
ot_uint sub_aind_nextslot();








#if (OT_FEATURE(CAPI) == ENABLED)

/** OTAPI Server System Functions
  * ============================================================================
  */
  
#define OTAPI_Q     rxq

ot_u16 sub_session_handle(m2session* session) {
    return (session == NULL) ? 0 : *((ot_u16*)&session->protocol);
}



ot_u16 otapi_sysinit() {
    dll_refresh();
    return 1;
}



ot_u16 otapi_new_dialog(session_tmpl* s_tmpl, void* applet) {
#if (SYS_SESSION == ENABLED)
    return sub_session_handle( otapi_task_immediate(s_tmpl, (ot_app)applet) );
#else
    return 0;
#endif
}



ot_u16 otapi_new_advdialog(advert_tmpl* adv_tmpl, session_tmpl* s_tmpl, void* applet) {
#if (SYS_FLOOD == ENABLED)
    return sub_session_handle( otapi_task_advertise(adv_tmpl, s_tmpl, (ot_app)applet) );
#else
    return 0;
#endif
}



ot_u16 otapi_open_request(addr_type addr, routing_tmpl* routing) {
/// Set the header if the session is valid.  Also conditionally write the header
/// depending on the address type (a parameter).  
    if (session_count() >= 0) {
        m2session* session;
        session = session_top();
        
        // Load default DLL communication parameters if the applet is NULL.
        // Applet is typically NULL in sessions generated by ALP-API 
        if (session->applet == NULL) {
            dll_set_defaults(session);
        }
    
        // Unicast/Anycast support routing, so copy the supplied template
        if ((addr & 0x40) == 0) {   
            platform_memcpy((ot_u8*)&m2np.rt, (ot_u8*)routing, sizeof(routing_tmpl));
        }

        // Load the header: last argument is for NACK (0 means normal request)
        m2np_header(session, (ot_u8)addr, 0);
        return 1;
    }
    return 0;
}



ot_u16 otapi_close_request() {
/// Set the footer if the session is valid
    if (session_count() >= 0) {
        m2np_footer( /* session_top() */ );
        return 1;
    }
    return 0;
}



ot_u16 otapi_start_dialog(ot_u16 timeout) {
/// Stop any ongoing processes and seed the event for the event manager.  The
/// radio killer will work in all cases, but it is bad form to kill sessions
/// that are moving data.
    if (timeout != 0) {
        dll.comm.tc = TI2CLK(timeout);
    }
    if (radio.state != RADIO_Idle) {
    	rm2_kill();
    }
    sys.task_RFA.event = 0;
    sys_preempt(&sys.task_RFA, 0);
    return 1;
}

#endif







/** Initializers state management functions <BR>
  * ========================================================================<BR>
  */
#ifndef EXTF_dll_init
void dll_init() {
    /// Initialize Radio
    radio_init();

    /// Initialize all upper-layer elements of the protocol stack
    network_init();
    m2qp_init();
    auth_init();
    
    /// Load the Network settings from ISF 0 to the dll.netconf buffer, reset
    /// the session, and send system to idle.
    dll_refresh();
}
#endif



#ifndef EXTF_dll_refresh
void dll_refresh() {
    ot_uni16 scratch;
    vlFILE* fp;
    
    /// Open Network Features ISF and load the values from that file into the
    /// cached dll.netconf settings.
    fp = ISF_open_su( 0x00 );
    scratch.ushort          = vl_read(fp, 2);
    dll.netconf.subnet      = scratch.ubyte[0];
    dll.netconf.b_subnet    = scratch.ubyte[1];
    scratch.ushort          = vl_read(fp, 6);
    dll.netconf.dd_flags    = scratch.ubyte[0];
    dll.netconf.b_attempts  = scratch.ubyte[1];
    dll.netconf.active      = vl_read(fp, 4);
    dll.netconf.hold_limit  = vl_read(fp, 8);   ///@todo endian conversion
    vl_close(fp);

    // Reset the Scheduler (only does anything if scheduler is implemented)
    sys_refresh_scheduler();
    sub_dll_flush();
}
#endif



#ifndef EXTF_dll_change_settings
void dll_change_settings(ot_u16 new_mask, ot_u16 new_settings) {
    vlFILE* fp_active;
    vlFILE* fp_supported;

    // Get Active Settings, Get Supported Settings,
    // Mask-out unsupported settings, apply to new active settings
    ///@todo assert fp
    fp_active           = ISF_open_su( 0x00 );
    fp_supported        = ISF_open_su( 0x01 );
    new_mask           &= vl_read(fp_supported, 8);
    dll.netconf.active  = vl_read(fp_active, 4);
    new_settings       &= new_mask;
    dll.netconf.active &= ~new_mask;
    dll.netconf.active |= new_settings;

    // Write the new settings to the ISF 0
    vl_write(fp_active, 4, dll.netconf.active);
    vl_close(fp_active);
    vl_close(fp_supported);

    // Flush the System of all Sessions and Events, and restart it
    sub_dll_flush();
}
#endif



void sub_task_defaults() {


}


void sub_dll_flush() {
/// (1) Put System states into the right place and flush existing events.
/// (2) Reset sessions.
/// (3) Set scheduler ids and prepare idle time events
	ot_task task;

	if (radio.state != RADIO_Idle) {
		rm2_kill();
	}

    task = &sys.task_HSS;
    do {
        task->cursor   = 0;
        task->reserve  = 1;
        task->latency  = 255;
        task->nextevent= 0;
        task++;
	} while (task < &sys.task[BTS_INDEX+1]);

    session_init();

    dll.idle_state  = sub_default_idle();
    dll_idle();
}




#ifndef EXTF_dll_goto_off
void dll_goto_off() {
    dll.idle_state = M2_DLLIDLE_OFF;
    dll_idle();
}
#endif




#ifndef EXTF_dll_idle
void dll_idle() {
/// Idle Routine: Disable radio and manipulate system tasks in order to go into
/// the type of idle that is configured in dll.idle_state (OFF, SLEEP, HOLD)
	static const ot_u8 scan_events[] = { 0,0, 0,5, 4,0 };
	ot_u8* scan_evt_ptr;

    /// Make sure Radio is powered-down
	radio_gag();
    radio_sleep();
    
    /// Assure all DLL tasks are in IDLE
    sys.task_RFA.event = 0;

    scan_evt_ptr        = (ot_u8*)&scan_events[dll.idle_state<<1];
    sys.task_HSS.event  = *scan_evt_ptr;
#   if (M2_FEATURE(ENDPOINT) == ENABLED)
    sys.task_SSS.event  = *(++scan_evt_ptr);
#   endif
    
#   if (M2_FEATURE(BEACONS) == ENABLED)
    sys.task_BTS.event  = ((dll.netconf.b_attempts != 0) \
    		            && (dll.idle_state != M2_DLLIDLE_OFF));
#   endif
    
}
#endif









/** DLL Systask Manager <BR>
  * ========================================================================<BR>
  */

void dll_block() {
	sys.task_RFA.latency = 0;
}


#ifndef EXTF_dll_clock
void dll_clock(ot_uint clocks) {
  //dll.comm.tca   -= clocks;
  //dll.comm.tc    -= clocks;
    clocks          = CLK2TI(clocks);
    session_refresh(clocks);
    
    if (sys.task_RFA.event != 0) {
        dll.comm.rx_timeout -= clocks;
    }
    else if (session_count() >= 0) {
        sys.task_RFA.event      = 2;
        sys.task_RFA.nextevent  = TI2CLK(session.heap[session.top].counter);
    }
}
#endif



#ifndef EXTF_dll_systask_rf
void dll_systask_rf(ot_task task) {
    switch (sys.task_RFA.event) {
        // Do Nothing
        case 0: break;
    
        // Processing
        case 1: sub_processing();       break;
    
        // Session Activation: this can activate Radio Exotasks.
        case 2: sub_activate();         break;
        
        // RX Scan Timeout Watchdog
        case 3: sub_timeout_scan();     break;
             
        // CSMA Manager (needed for archaic radios only)
        //case 4: sub_dll_txcsma();       break;
        
        // TX & CSMA Timeout Watchdog
       default: rm2_kill();             break;
    }
}
#endif




/** DLL Systask Processors <BR>
  * ========================================================================<BR>
  * Processing tasks run in co-operative mode, and they do not directly spawn
  * I/O tasks (which run in pre-emptive mode).  Instead, they create sessions,
  * and sessions will invoke the radio module task.
  */

void sub_processing() {
/// This is the task for processing a DASH7 packet which has been received.
/// If the packet is a valid request, this task will put the DLL into HOLD.
/// If the listen bit is high, additionally, this task will clone the session
/// to be re-visited at a later time.
    m2session* session;
    ot_int proc_score;
    sys.task_RFA.event      = 0;                // Only run processing once
    session                 = session_top();
    session->counter        = 0;
    proc_score              = network_route_ff(session);

    /// If the Listen flag is high, clone the session to a time in the
    /// future, when it will listen again.
    /// <LI> dll.comm.tc has just been assigned by the request.  It is the
    ///      response contention period (Tc). </LI>
    /// <LI> This device should listen again after Tc ends, so the session
    ///      is cloned & scheduled for Tc </LI>
    /// <LI> The current session is popped after response, or on next kernel
    ///      loop (immediately) if no response </LI>

    /// Response is prepared already, so setup holdstate and flow control
    if (proc_score >= 0) {
        sub_fceval(proc_score);
        sys.task_HSS.cursor = 0;
        dll.counter         = 0;
        dll.idle_state      = M2_DLLIDLE_HOLD;

        if (session->flags & M2FI_LISTEN) {
            m2session* s_clone;
            s_clone = session_new( session->applet, CLK2TI(dll.comm.tc),
                                   (M2_NETSTATE_REQRX | M2_NETSTATE_ASSOCIATED),
                                   dll.comm.rx_chanlist[0]     );

            s_clone->dialog_id  = session->dialog_id;
            s_clone->subnet     = session->subnet;
            s_clone->channel    = session->channel;
            //if (session->applet == NULL) {
            //    dll_set_defaults(session);    ///@todo put this in activate for all NULL requests
            //}
        }
    }

    // No response (or no listening for responses)
    // Plus bad score: stop the session
    else if ((session->netstate & M2_NETSTATE_RESP) == 0) {
        session->netstate |= M2_NETFLAG_SCRAP;
    }
}




void dll_systask_holdscan(ot_task task) {
/// The Hold Scan process runs as an independent systask.  It will perform the
/// scan using the same routine as the sleep scan, but it does some additional
/// checks on switching between hold and sleep modes.  Namely, it increments
/// the hold_cycle parameter, and if this is over the limit it will return into
/// sleep mode.
#if ((SYS_RECEIVE == ENABLED) && (M2_FEATURE(ENDPOINT) == ENABLED))
    dll.counter += (sys.task_HSS.cursor == 0);
    
    // Do holdscan (top) or go back into sleep mode (bottom)
    if (((dll.netconf.active & M2_SET_CLASSMASK) != M2_SET_ENDPOINT) | \
        (dll.counter >= dll.netconf.hold_limit)) {
        dll_systask_sleepscan(task);
    }
    else {
        dll.counter         = 0;
        sys.task_HSS.event  = 0;
        sys.task_HSS.cursor = 0;
        sys.task_SSS.event  = 5;
    }
    
#else
    // SLEEP state is not enabled on the device, so it will use HOLD always.
    dll_systask_sleepscan(task);
#endif
}



void dll_systask_sleepscan(ot_task task) {
/// The Sleep Scan process runs as an independent task.  It is very similar
/// to the Hold Scan process, which actually calls this same routine.  They
/// use independent task markers, however, so they behave differently.
#if ((M2_FEATURE(GATEWAY) == ENABLED) || \
     (M2_FEATURE(SUBCONTROLLER) == ENABLED) || \
     (M2_FEATURE(ENDPOINT) == ENABLED))
    ot_u8   s_channel;
    ot_u8   s_flags;
    ot_u8   new_netstate;
    ot_uni16  scratch;
    vlFILE* fp;

    if (task->event == 0) return;

    fp = ISF_open_su( task->event );
    ///@todo assert fp

    /// Pull channel ID and Scan flags
    scratch.ushort  = vl_read(fp, task->cursor);
    s_channel       = scratch.ubyte[0];
    s_flags         = scratch.ubyte[1];

    /// Set the next idle event from the two-byte Next Scan field.  
    /// The DASH7 registry is big-endian.
#   ifdef __BIG_ENDIAN__
    scratch.ushort  = TI2CLK(vl_read(fp, (task->cursor)+=2 ));
#   else
    scratch.ushort  = vl_read(fp, (task->cursor)+=2 );
    scratch.ushort  = PLATFORM_ENDIAN16(scratch.ushort);
#   endif
    sys_task_setnext(task, scratch.ushort);

    /// Advance cursor to next datum, go back to 0 if end of sequence
    /// (still works in special case where cursor = 254)
    task->cursor   += 2;
    task->cursor    = (task->cursor >= fp->length) ? 0 : task->cursor;
    vl_close(fp);
    
    /// Choosing Background-Scan or Foreground-Scan is based on flags.  If b7
    /// is set, do a Background-Scan.  At the session level, the "Flood" select
    /// uses b6, which is why there is a >> 1.
    new_netstate    = (M2_NETSTATE_REQRX | M2_NETSTATE_INIT);
    new_netstate   |= (s_flags & 0x80) >> 1;
    
    /// Perform the scan
    /// <LI> b5:0 of the scan flags is the normal scan timeout  </LI>
    /// <LI> b6 of the scan flags enables 1024x multiplier on scan timeout  </LI>
    /// <LI> b7 of the scan flags is foreground (0), background (1)  </LI>
    dll_set_defaults( session_new(NULL, 0, new_netstate, s_channel) );
    dll.comm.rx_timeout     = otutils_calc_timeout(s_flags);
    dll.comm.csmaca_params  = 0;
#endif
}




#if (M2_FEATURE(BEACONS) == ENABLED)
void dll_systask_beacon(ot_task task) {
/// The beacon rountine runs as an idependent systask.
    vlFILE*     fp;
    m2session*  session;
    ot_u8       beacon_params;
    Queue       beacon_queue;
    ot_uni32    bq_data;
    ot_uni16    scratch;

    if (task->event == 0) return;

    /// Open BTS ISF Element and read the beacon sequence.  Make sure there is
    /// a beacon file of non-zero length and that beacons are presently enabled.
    /// Otherwise, in 64 seconds (an arbitrary value) it will check again.  You 
    /// can enact a beacon at any time by setting sys.task_BTS.nextevent = 0.
    fp = ISF_open_su( ISF_ID(beacon_transmit_sequence) );
    if ((dll.netconf.b_attempts == 0) || (fp->length == 0)) {
        vl_close(fp);
        sys_task_setnext(task, 65000);  ///@todo make this an app-config parameter
        return;
    }
    
    // First 2 bytes: Chan ID, Cmd Code
    // - Setup beacon ad-hoc session, on specified channel
    //   (ad hoc sessions never return NULL)
    // - Assure cmd code is always Broadcast & Announcement
    scratch.ushort      = vl_read(fp, task->cursor);
    session             = session_new(  NULL, 0,
                                        (M2_NETSTATE_INIT | M2_NETFLAG_FIRSTRX),
                                        scratch.ubyte[0]  );
    session->subnet     = dll.netconf.b_subnet;
    beacon_params       = scratch.ubyte[1];
    session->flags      = (dll.netconf.dd_flags & ~0x30);
    session->flags     |= (beacon_params & 0x30);

    // Second & Third 2 bytes: ISF Call Template
    q_init(&beacon_queue, &bq_data.ubyte[0], 4);
    bq_data.ushort[0]   = vl_read(fp, task->cursor+=2);
    bq_data.ushort[1]   = vl_read(fp, task->cursor+=2);

    // Last 2 bytes: Next Scan ticks
#   ifdef __BIG_ENDIAN__
        scratch.ushort = TI2CLK( vl_read(fp, task->cursor+=2) );
#   else
        scratch.ushort = vl_read(fp, task->cursor+=2);
        scratch.ushort = PLATFORM_ENDIAN16(scratch.ushort);
#   endif
    //task->nextevent = scratch.ushort;
    sys_task_setnext(task, scratch.ushort);

    /// Beacon List Management:
    /// <LI> Move cursor onto next beacon period </LI>
    /// <LI> Loop cursor if it is past the length of the list </LI>
    /// <LI> In special case where cursor = 254, everything still works! </LI>
    task->cursor += 2;
    task->cursor  = (task->cursor >= fp->length) ? 0 : task->cursor;
    vl_close(fp);

    /// Start building the beacon packet:
    /// <LI> Calling m2np_header() will write most of the front of the frame </LI>
    /// <LI> Add the command byte and optional command-extension byte </LI>
    m2np_header(session, 0x40, 0);
    q_writebyte(&txq, 0x20 + (beacon_params & 1));
    if (beacon_params & 0x04) {
        q_writebyte(&txq, (beacon_params & 0x04));
    }

    /// Setup the comm parameters, if the channel is available:
    /// <LI> dll.comm values tx_eirp, cs_rssi, and cca_rssi must be set by the
    ///      radio module during the CSMA-CA process -- don't set them here.
    ///      The DASH7 spec requires it to happen in this order. </LI>
    /// <LI> Do set the CSMA-CA parameters, which are used by the radio module </LI>
    /// <LI> Do set the number of redundant TXes we would like to transmit </LI>
    /// <LI> Reset rx_timeout if this beacon uses no response </LI>
    dll_set_defaults(session);
    dll.comm.csmaca_params |= (beacon_params & 0x04);
    dll.comm.csmaca_params |= (M2_CSMACA_NA2P | M2_CSMACA_MACCA);
    dll.comm.redundants     = dll.netconf.b_attempts;  
    if (beacon_params & 0x02) {
        dll.comm.rx_timeout = 0;
    }
    q_writebyte(&txq, (ot_u8)dll.comm.rx_timeout);

    /// Finish building the beacon packet:
    /// If the beacon data is not accessible by the GUEST user, pop the session 
    /// and don't finish building the beacon.
    if (m2qp_isf_call((beacon_params & 1), &beacon_queue, AUTH_GUEST) < 0) {
        session_pop();
    }
    else {
        m2np_footer();
    }
}
#endif






/** DLL Systask RF I/O routines   <BR>
  * ========================================================================<BR>
  * These routines & subroutines are called from the dll_systack() function.
  * All of these routines interact with the Radio module & driver, which is an
  * interrupt-driven pre-emptive task.
  */
void sub_activate() {
/// Do session creation
/// 1. Block DLL Idle-time tasks: they get reactivated by dll_idle()
/// 2. Drop expired sessions (but top session is kept)
/// 3. Associated Applet can construct packet, or control parameters
/// 4. Session is terminated if "SCRAP" bit is 1
/// 5. Session is processed otherwise
    m2session* session;

    sys.task_HSS.event = 0;
#   if (M2_FEATURE(ENDPOINT) == ENABLED)
    sys.task_SSS.event = 0;
#   endif
#   if (M2_FEATURE(BEACONS) == ENABLED)
    sys.task_BTS.event = 0;
#   endif

    session_drop();
    dll.idle_state  = sub_default_idle();
    session         = session_top();

    if (session->applet != NULL) {
        ot_app this_applet;
        this_applet     = session->applet;
        session->applet = NULL;
        dll_set_defaults(session);
        this_applet(session);
    }
    if (session->netstate & M2_NETFLAG_SCRAP) {
        session_pop();
        dll_idle();
    }
    else if (session->netstate & M2_NETSTATE_REQRX) {
        sub_init_rx(session->netstate & M2_NETFLAG_FLOOD);
    }
    else {
        sub_init_tx(session->netstate & M2_NETFLAG_FLOOD);
    }
}
  
  

void sub_timeout_scan() {
/// This function just operates the timing-out of the RX.  RX is forced into
/// timeout when there is no data being received OR if the MAC is operating
/// under the A2P regime, which uses strict time-slots.

/// @note if you want to use a built-in RX termination or polling timer inside
/// the radio core (some devices have these), be very careful.  If the radio
/// timer gets out of sync with the kernel (i.e. if your core timer isn't very
/// accurate), then it is going to cause all sorts of timing glitches.  Giving
/// the kernel control over listening makes the system more stable and better
/// synchronized.  Usage of HW RX timer is mostly useful for very specific
/// applications using very custom builds of OpenTag.

#if (RF_FEATURE(RXTIMER) == DISABLED)
    // If not presently receiving, time-out the RX.
    // else if presently receiving, pad timeout by 128
    if ((radio.state != RADIO_DataRX) || (dll.comm.csmaca_params & M2_CSMACA_A2P)) 
        rm2_rxtimeout_isr();
    else 
        sys_task_setnext(&sys.task[TASK_radio], 128);

#else
    // Add a little bit of time in case the radio timer is a bit slow.
    sys_task_setnext(&sys.task[TASK_radio]->nextevent, 10);
    sys.task[TASK_radio].event = 0;
#endif
}



void sub_init_rx(ot_u8 is_brx) {
/// RX initialization is called by the session activation routine, and it is
/// often the result of an SSS or HSS invocation.  Scanning allows a 1 tick
/// latency for other tasks, although this could be potentially increased.
/// Background scans are very fast, and pending tasks will not be impacted
/// greatly by the strict latency requirement.  Foreground scans can last for
/// potentially long periods of time, however.  The latency, here, affects the
/// the request-response turnaround but not the radio RX process itself.
///
/// @todo A more adaptive method for scanning is planned, in which the latency 
/// drops to 1 only after a sync word is detected.
#ifdef OT_FEATURE_LISTEN_ALLOWANCE
#   define _RX_LATENCY  OT_FEATURE_LISTEN_ALLOWANCE
#else
#   define _RX_LATENCY  40
#endif
	sys_task_setnext(&sys.task[TASK_radio], dll.comm.rx_timeout);

    sys.task_RFA.latency    = _RX_LATENCY;
  //sys.task_RFA.reserved   = 10;    //un-necessary, RFA is max priority
    sys.task_RFA.event      = 3;
    
    DLL_SIG_RFINIT(sys.task_RFA.event);
    
#if (M2_FEATURE(GATEWAY) || M2_FEATURE(SUBCONTROLLER) || M2_FEATURE(ENDPOINT))
    {   // is_brx is inverted to 0=brx, 1=frx
    	static const ot_sig2 rfevt_cb[2] = { &rfevt_bscan, &rfevt_frx };
    	is_brx = (is_brx == 0);
    	rm2_rxinit(session.heap[session.top].channel, is_brx, rfevt_cb[is_brx]);
    }
#else
    rm2_rxinit(session.heap[session.top].channel, 1, &rfevt_frx);
#endif

#undef _RX_LATENCY
}




void sub_init_tx(ot_u8 is_btx) {
/// Initialize background or foreground packet TX.  Often this includes CSMA
/// initialization as well.
    sys_task_setnext_clocks(&sys.task[TASK_radio], dll.comm.tc);
    dll.comm.tca            = sub_fcinit();
    sys.task_RFA.latency    = 1; 
    sys.task_RFA.event      = 4;

    DLL_SIG_RFINIT(sys.task_RFA.event);
    
#if (M2_FEATURE(GATEWAY) || M2_FEATURE(SUBCONTROLLER))
    dll.counter = 0;
    if (is_btx) {
        dll.counter = session.heap[session.top-1].counter;
        m2advp_open(session_top(), dll.counter);
    }
    rm2_txinit(is_btx, &rfevt_txcsma);
#else
    dll.counter = 0;
    rm2_txinit(0, &rfevt_txcsma);
#endif
}






/** DLL Radio Driver Task Callbacks <BR>
  * ============================================================================
  * The Radio Module & driver is implemented as a pre-emptive task, managed by
  * interrupts implemented in the radio module.  These functions will be called
  * by the radio module when a radio module task is done.  These function will
  * seed kernel level tasks, which will run as soon as the kernel is free.
  */

void rfevt_bscan(ot_int scode, ot_int fcode) {
/// bscan reception radio-core event callback: called by radio core driver when
/// the bscan process terminates, either due to success or failure

    // CRC Failure (or init), retry
    if ((scode == -1) && (dll.comm.redundants != 0)) {
        rm2_rxinit(dll.comm.rx_chanlist[0], 0, &rfevt_bscan);    //non-blocking
        return;
    }
    
    // A valid packet was received:
    // network_parse_bf() must update the top session
    if (scode >= 0) {
    	scode = sub_mac_filter() - 1;
    	if (scode == 0) {
    		network_parse_bf();
    	}
    }

    // Either a timeout or a background packet sent to a different subnet
    if (scode < 0) {
    	session_pop();
    	dll_idle();
    }

    DLL_SIG_RFTERMINATE(3, scode);

    sys.task_RFA.event = 0;
    sys_preempt(&sys.task_RFA, 0);
}




void rfevt_frx(ot_int pcode, ot_int fcode) {
/// Radio Core event callback, called by the radio driver when a frame is rx'ed
/// or if there is some type of error.
    ot_int      frx_code = 0;
    m2session*  session  = session_top();
    
    /// If pcode is less than zero, it is because of a listening timeout.
    /// Listening timeouts happen after unfulfilled request scanning, or after
    /// Response scanning window expires.  In certain cases, after a timeout,
    /// the session persists.  These cases are implemented below.
    if (pcode < 0) {
        sys.task_RFA.event  = 0;
      //frx_code            = -1;
        if (dll.comm.redundants) {
            session->netstate   = (M2_NETSTATE_REQTX | M2_NETSTATE_INIT | M2_NETFLAG_FIRSTRX);
        }
        else if (dll.comm.csmaca_params & M2_CSMACA_A2P) {
            session->netstate  ^= 0x30;  // Converts RESPRX->REQTX, REQRX->RESPTX
        }
        else {
            session->netstate   = M2_NETFLAG_SCRAP;
        }
    }
    
    // Multiframe packet RX frame check
#   if (M2_FEATURE(M2DP) == ENABLED)
    else if (pcode > 0) {
    	if (fcode != 0) {
    		m2dp_mark_frame();
    	}
    	return;
    }
#	endif

    // pcode == 0 on last frame
    else {
        /// Handle damaged frames (bad CRC)
    	/// Run subnet filtering on frames with good CRC
    	if (fcode != 0) {
    		frx_code--;
    	}
        else if (sub_mac_filter() == False) {
            frx_code = -4;
        }
        
        /// A complete packet has been received (errors or not).
    	/// <LI> When packet is good (frx_code == 0), always process it. </LI>
    	/// <LI> When request is bad or when response is any form, retry
        ///      listening until window times-out </LI>
        /// <LI> Don't return to kernel for bad frames </LI>
        /// <LI> After receiving good request, turn-off radio subsystem </LI>
        if (pcode == 0) {
            if (frx_code == 0) {
                sys.task_RFA.reserve = 20;  ///@todo Could have quick evaluator here
                sys.task_RFA.event   = 1;   ///Process the packet!!!
            }
            if ((frx_code != 0) || (session->netstate & M2_NETSTATE_RESP)) {
                rm2_reenter_rx(&rfevt_frx);
            }
            else {
                radio_sleep();
            }
            if (frx_code != 0) {
                return;
            }
        }
    }

    /// The RX Termination Callback always uses code = "3"
    DLL_SIG_RFTERMINATE(3, frx_code);

    /// Pre-empt the Kernel Scheduler on successful packet download or timeout
    sys_preempt(&sys.task_RFA, 0);
}




void rfevt_txcsma(ot_int pcode, ot_int tcode) {
    ot_uint event_ticks;

    /// ON CSMA SUCCESS: pcode == 0, tcode == 0/1 for BG/FG
    if (pcode == 0) {
        sys.task_RFA.latency    = 0;
        sys.task_RFA.event      = 5;
#       if (SYS_FLOOD == ENABLED)
        if (tcode != 0) {
            radio.evtdone   = &rfevt_btx;
            event_ticks     = dll.counter+20;
        }
        else
#       endif
        {
            radio.evtdone   = &rfevt_ftx;
            event_ticks     = (ot_uint)(rm2_pkt_duration(txq.length) + 4);
        }
    }

    /// ON CSMA LOOP: calculate the next slot time and set mactimer accordingly
    /// Do not Preempt the kernel.
    /// @todo replace the "2" in the idle vs. sleep check with a per-radio constant
    else if (pcode > 0) {
        ot_uint nextcsma;
        nextcsma                    = (ot_uint)sub_fcloop();
        if (nextcsma < TI2CLK(2))   radio_idle();
        else                        radio_sleep();
    
        radio_set_mactimer( nextcsma );
        return;
    }
    
    /// ON FAIL: Flag this session for scrap.  As with all sessions, if you 
    /// have an applet attached to it, the applet can adjust the netstate and 
    /// try again if it chooses.
    else {
        DLL_SIG_RFTERMINATE(sys.task_RFA.event, pcode);
        
        session.heap[session.top].netstate |= M2_NETFLAG_SCRAP;
        sys.task_RFA.event                  = 0;
        event_ticks                         = 0;
    }
    
    sys_preempt(&sys.task_RFA, event_ticks);
}




void rfevt_ftx(ot_int pcode, ot_int scratch) {
    m2session* session;

    /// Non-final frame TX'ed in multiframe packet
    if (pcode == 1) {
        ///@todo possibly put the queue rearrangement here
    }

    /// Packet TX is done.  Handle this event and pre-empt the kernel.
    /// <LI> Normally, go to response RX.   </LI>
    /// <LI> Allow scheduling of redundant TX on responses, or request with no response. </LI>
    /// <LI> End session if no redundant, and no listening required. </LI>
    else {
        session = session_top();
        scratch = ((session->netstate & M2_NETSTATE_RESPTX) \
        		|| ((ot_int)dll.comm.rx_timeout <= 0));
        
        /// Send redundant TX immediately, but only if no response window or if
        /// this packet is a response.
        dll.comm.redundants--;
        if ((dll.comm.redundants != 0) && scratch) {
            dll.comm.csmaca_params = (M2_CSMACA_NOCSMA | M2_CSMACA_MACCA);
            rm2_resend( (ot_sig2)&rfevt_txcsma );
        }
        
        /// Scrap (End) Session if:
        /// <LI> There is no redundant TX to send (implicit, via else)</LI>
        /// <LI> AND (No response window to listen OR This TX itself is a response)</LI>
        /// <LI> OR Some sort of error occurred (pcode != 0) </LI>
        ///
        /// Otherwise, continue session as listening for response.
        else {
        	//M2_NETFLAG_SCRAP is represented below
            session->netstate  |= (ot_u8)(((pcode != 0) | scratch) << 7);
            session->netstate  &= ~M2_NETSTATE_TMASK;
            session->netstate  |= M2_NETSTATE_RESPRX;

            DLL_SIG_RFTERMINATE(sys.task_RFA.event, pcode);
            sys.task_RFA.event = 0;
        	sys_preempt(&sys.task_RFA, 0);
        }
    }
}



void rfevt_btx(ot_int flcode, ot_int scratch) {
#if ((M2_FEATURE(SUBCONTROLLER) == ENABLED) || (M2_FEATURE(GATEWAY) == ENABLED))
    switch (flcode) {
        /// Flood ends & Request Begins 
        /// <LI> Pop the flood session, it is no longer needed </LI>
        /// <LI> Tweak the request session to work optimally following flood </LI>
        /// <LI> Pre-empt the kernel to start on the request session.  The 
        ///      kernel will clock other tasks over the flood duration.  </LI>
        case 0: {
            // assure request hits NOW & assure it doesn't init dll.comm
            // Tweak dll.comm for request (2 ti is a token, small amount)
            sys.task_RFA.event                      = 0;
            session.heap[session.top-1].counter     = 0;    
            session.heap[session.top-1].netstate   &= ~M2_NETSTATE_INIT;
            dll.comm.tc                             = TI2CLK(2);
            dll.comm.csmaca_params                  = (M2_CSMACA_NOCSMA | M2_CSMACA_MACCA);
        } break;
        
        /// Begin Flood:
        /// <LI> This flood implementation is optimized for kernels with low
        ///      timing resolution (i.e. 1 ti).  kernels with higher resolution
        ///      (i.e. 1 sti) can implement the flooding task as a real task </LI>
        /// <LI> To maintain the best, purest timing, the low-resolution kernel 
        ///      is blocked during floods and the kernel timer is free-running </LI>
        /// <LI> An alternative method is to use a 2nd timer here </LI>
        /// <LI> The advertising time is reduced by the CSMA-CA process, but
        ///      whatever that was, the request session counter has adjusted </LI>
        /// <LI> Write the first packet to the queue (via fall-though)  </LI>
        //case 1: {
        //    m2advp_open(session_top());
        //}
        
        /// Flood Continues:
        /// <LI> Derive current value for advertising countdown and apply </LI>
        /// <LI> Stop the flood if the countdown is shorter than one packet </LI>
        /// <LI> The Radio Driver will flood adv packets forever, in parallel
        ///      with the blocked kernel, until rm2_txstop_flood() is called </LI>
        case 2: {
            ot_int countdown;
            countdown = (ot_int)radio_get_countdown();
            ///@todo make faster function for bg packet duration lookup
            if (countdown < rm2_pkt_duration(7)) {
                m2advp_close();
                rm2_txstop_flood();
            }
            else {
                m2advp_update(countdown);
            }
        } return; // skip termination section
    
        /// Error: kill everything, pop the flood (here) and the request (below)
        default: {
            dll_idle();
            session_pop();
            break;
        }
    }
        
    /// Termination on error or on flood-over: 
    /// Always pop the flood session when the flood is over.  On error, 
    /// the flood will already be popped (switch-default above), so this will
    /// pop the following request.  That is exactly the behavior required.
    session_pop();

    DLL_SIG_RFTERMINATE(sys.task_RFA.event, flcode);

    sys.task_RFA.event = 0;
    sys_preempt(&sys.task_RFA, 0);
#endif
}









/** Subroutines for DLL configuration and Flow & Congestion Control
  * ============================================================================
  */

#ifndef EXTF_dll_default_csma
ot_u8 dll_default_csma(ot_u8 chan_id) {
    chan_id &= 0x30;
    return (((chan_id == 0x00) || (chan_id == 0x30)) << 2);
}
#endif



#ifndef EXTF_dll_quit_rf
void dll_quit_rf() {
    sys.task_RFA.event = 0;
}
#endif



ot_u8 sub_default_idle() {
#if (M2_FEATURE(ENDPOINT) == ENABLED)
#ifdef __BIG_ENDIAN__
#   define _SETTING_SHIFT   9
#else
#   define _SETTING_SHIFT   1
#endif
    ot_u16 setting;
    setting = (dll.netconf.active & M2_SET_CLASSMASK) >> _SETTING_SHIFT;
    if (setting > 1) {
        setting = M2_DLLIDLE_HOLD;
    }    
    return (ot_u8)setting;
#else
    
    return (dll.netconf.active & M2_SET_CLASSMASK) ? M2_DLLIDLE_HOLD : M2_DLLIDLE_OFF;
#endif
}




void dll_set_defaults(m2session* session) {
    if (session->flags & M2FI_LISTEN) {
        // Reduce effective Tc by the duration of a packet Tx so that listening 
        // can occur precisely following actual Tc
        dll.comm.tc -= TI2CLK(rm2_pkt_duration(txq.length));
    }
    else if (session->netstate & M2_NETSTATE_INIT) {
        // Default for new session is beacon request.  These parameters often
        // get altered afterward, in the session applet
        dll.comm.tc             = TI2CLK(M2_PARAM_BEACON_TCA);
        dll.comm.rx_timeout     = rm2_default_tgd(session->channel);
        dll.comm.csmaca_params  = dll_default_csma(session->channel);
    }
    
    // Default comm settings for any kind of comms.  As above, these parameters
    // may be altered in the session applet.
    dll.comm.redundants     = ((session->netstate & M2_NETSTATE_RX) == 0);
    dll.comm.tx_channels    = 1;
    dll.comm.rx_channels    = 1;
    dll.comm.tx_chanlist    = &dll.comm.scratch[0];
    dll.comm.rx_chanlist    = &dll.comm.scratch[1];
    dll.comm.scratch[0]     = session->channel;
    dll.comm.scratch[1]     = session->channel;
}




ot_bool sub_mac_filter() {
/// Link Budget Filtering (LBF) is a normalized RSSI Qualifier.
/// Subnet Filtering is an numerical qualifier
    ot_bool qualifier;
    {
        // TX EIRP encoded value    = (dBm + 40) * 2
        // TX EIRP dBm              = ((encoded value) / 2) - 40
        // Link Loss                = TX EIRP dBm - Detected RX dBm
        // Link Quality Filter      = (Link Loss <= Link Loss Limit)
        dll.last_nrssi  = ((ot_int)((rxq.front[1] >> 1) & 0x3F) - 40) - radio_rssi(); 
        qualifier       = (ot_bool)(dll.last_nrssi <= (ot_int)phymac[0].link_qual);
    }
    {
        ot_u8 fr_subnet, dsm, specifier, mask;
        
        fr_subnet   = rxq.front[2];
        dsm         = dll.netconf.subnet & 0x0F;
        mask        = fr_subnet & dsm;
        specifier   = (fr_subnet ^ dll.netconf.subnet) & 0xF0;
        fr_subnet  &= 0xF0;
        qualifier  &= (ot_bool)(((fr_subnet == 0xF0) || (specifier == 0)) && (mask == dsm));
    }
    return qualifier;
}




void sub_csma_scramble() {
/// Sort of optional: Go through the channel list and scramble the channel
/// entries randomly in order to improve band utilization, as multiple devices
/// will scramble the list differently.
    ot_u8 txchans = dll.comm.tx_channels - 1;

    if (txchans) {
        ot_u8       i, j, k, scratch;
        ot_uni16    rot;
        rot.ushort = platform_prand_u16();
        
        for (i=0; i<txchans; i++) {
            j = i + ((rot.ubyte[0] & 1) != 0);
            k = txchans * ((rot.ubyte[1] & 1) != 0);
            
            scratch                 = dll.comm.tx_chanlist[i];
            dll.comm.tx_chanlist[i] = dll.comm.tx_chanlist[k];
            dll.comm.tx_chanlist[k] = scratch;
            scratch                 = dll.comm.tx_chanlist[i];
            dll.comm.tx_chanlist[i] = dll.comm.tx_chanlist[j];
            dll.comm.tx_chanlist[j] = scratch;
            
            rot.ushort     &= ~0x0101;   //Shift each byte right 1 bit
            rot.ushort    >>= 1;
        }
    }
}




CLK_UNIT sub_fcinit() {
/// Pick a time offset to begin the first transmission attempt, and setup
/// flow-congestion loop parameters.
    
    // Scramble the CSMA channel list
    sub_csma_scramble();
    
    // Pick a slot offset: currently only RIGD and RAIND need a random slot.
    // {0,1,2,3} = {RIGD, RAIND, AIND, Default MAC CA} 
    if (dll.comm.csmaca_params & M2_CSMACA_AIND) {
        return 0;
    }
    
    if (dll.comm.csmaca_params & M2_CSMACA_RAIND) {
        CLK_UNIT random;
        random  = TI2CLK(platform_prand_u16());
        random %= (dll.comm.tc - TI2CLK(rm2_pkt_duration(txq.front[0])) );
        return random;
    }
    
    return sub_rigd_newslot();
}




void sub_fceval(ot_int query_score) {
/// @todo When M2QP returns zero, the query has succeeded with no priorities.  
/// Some queries have priority scores.  In these cases, higher scores may be 
/// used to affect CSMA-CA reply ordering (higher is better).  This feature is 
/// not yet implemented.
}




CLK_UNIT sub_fcloop() {
    /// {0,1,2,3} = {RIGD, RAIND, AIND, Default MAC CA} 
    /// Default MAC CA just waits Tg before trying again
    if (dll.comm.csmaca_params & 0x20) {    //NO CA
        return TI2CLK(phymac[0].tg);
    }
    
    // AIND & RAIND Loop
    if (dll.comm.csmaca_params & 0x18) {    //RAIND, AIND
        return TI2CLK(rm2_pkt_duration(txq.front[0]));
    }
    
    // RIGD loop
    {   
        ot_long wait;
        wait    = (dll.comm.tc - dll.comm.tca);
        wait   += sub_rigd_newslot();
        
        return (wait < 0) ? 0 : (CLK_UNIT)wait;
    }
}



CLK_UNIT sub_rigd_newslot() {
/// halve tc from previous value and offset a random within that duration
    dll.comm.tc >>= 1;
    if (dll.comm.tc == 0)   return 0;

    return (TI2CLK(platform_prand_u16()) % (CLK_UNIT)dll.comm.tc);
}



/*
CLK_UNIT sub_rigd_nextslot() {
    ot_long wait;
    wait = (dll.comm.tc - dll.comm.tca);
    
    return (wait < 0) ? 0 : (CLK_UNIT)wait;
}




CLK_UNIT sub_aind_nextslot() {
/// Works for RAIND or AIND next slot
    return TI2CLK(rm2_pkt_duration(txq.front[0]));
}
*/

#endif

