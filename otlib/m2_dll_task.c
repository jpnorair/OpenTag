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
  * @file       /otlib/m2_dll_task.c
  * @author     JP Norair
  * @version    R102
  * @date       26 Sept 2013
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
#if (OT_FEATURE(M2) == ENABLED)

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

#if (M2_FEATURE(BEACONS))
    //ot_queue        beacon_queue;
    ot_uni32        bq_data;
#endif

void sub_dll_flush();
void sub_scan_channel(ot_u8 task_offset);

void sub_init_bscan();
void sub_init_fscan();
void sub_init_ftx();
void sub_init_btx();
void sub_init_rx(m2session* active);
void sub_init_tx(ot_u8 is_btx);

void sub_timeout_scan();
void sub_dll_txcsma();


void sub_processing();
void sub_activate();


void rfevt_bscan(ot_int scode, ot_int fcode);
void rfevt_frx(ot_int pcode, ot_int fcode);
void rfevt_txcsma(ot_int pcode, ot_int tcode);
void rfevt_ftx(ot_int pcode, ot_int scratch);
void rfevt_btx(ot_int flcode, ot_int scratch);







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









/** Initializers state management functions <BR>
  * ========================================================================<BR>
  */

OT_WEAK void dll_block_idletasks() {
    sys.task_HSS.event  = 0;
#   if (M2_FEATURE(BEACONS) == ENABLED)
    sys.task_BTS.event  = 0;
#   endif
    sys.task_SSS.event  = 0;
}




void sub_dll_flush() {
/// (1) Put System states into the right place and flush existing events.
/// (2) Reset sessions.
/// (3) Set scheduler ids and prepare idle time events
	ot_task task;

	if (radio.state != RADIO_Idle) {
		rm2_kill();
	}
    radio_mac_configure();

#   ifndef __KERNEL_NONE__
    task = &sys.task_HSS;
    do {
        task->cursor   = 0;
        task->reserve  = 1;
        task->latency  = 255;
        task->nextevent= 0;
        task++;
	} while (task < &sys.task[SSS_INDEX+1]);
#   endif

    session_init();

    dll.idle_state  = sub_default_idle();
    dll_idle();
}


  
#ifndef EXTF_dll_init
OT_WEAK void dll_init() {
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
OT_WEAK void dll_refresh() {
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
    dll.netconf.hold_limit  = PLATFORM_ENDIAN16(vl_read(fp, 8));
    vl_close(fp);

    // Reset the Scheduler (only does anything if scheduler is implemented)
#   ifndef __KERNEL_NONE__
    sys_refresh_scheduler();
#   endif
    sub_dll_flush();
}
#endif



#ifndef EXTF_dll_change_settings
OT_WEAK void dll_change_settings(ot_u16 new_mask, ot_u16 new_settings) {
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



#ifndef EXTF_dll_goto_off
OT_WEAK void dll_goto_off() {
    dll.idle_state = M2_DLLIDLE_OFF;
    dll_idle();
}
#endif




#ifndef EXTF_dll_idle
OT_WEAK void dll_idle() {
/// Idle Routine: Disable radio and manipulate system tasks in order to go into
/// the type of idle that is configured in dll.idle_state (OFF, SLEEP, HOLD)
	static const ot_u8 scan_events[] = { 0,0, 0,5, 4,0 };
	ot_u8* scan_evt_ptr;

    /// Make sure Radio is powered-down
	radio_gag();
    radio_sleep();
    
    /// Assure all DLL tasks are in IDLE
#   ifndef __KERNEL_NONE__
    sys.task_RFA.event  = 0;
    scan_evt_ptr        = (ot_u8*)&scan_events[dll.idle_state<<1];
    sys.task_HSS.event  = *scan_evt_ptr;
    sys.task_SSS.event  = *(++scan_evt_ptr);
    
#   if (M2_FEATURE(BEACONS) == ENABLED)
    sys.task_BTS.event  = ((dll.netconf.b_attempts != 0) \
    		            && (dll.idle_state != M2_DLLIDLE_OFF));
#   endif
#   endif
 
    //LATENCY TEST
    //GPIOA->BSRRL = 3<<0;
}
#endif









/** DLL Systask Manager <BR>
  * ========================================================================<BR>
  */
#ifndef __KERNEL_NONE__
  
#ifdef OT_FEATURE_LISTEN_ALLOWANCE
#   define _REQRX_LATENCY   OT_PARAM_LISTEN_ALLOWANCE
#   define _RESPRX_LATENCY  OT_PARAM_RECEIVE_ALLOWANCE
#else
#   define _REQRX_LATENCY   40
#   define _RESPRX_LATENCY  2
#endif

OT_WEAK void dll_block() {
	sys.task_RFA.latency = 0;
}

OT_WEAK void dll_unblock() {
	sys.task_RFA.latency = _REQRX_LATENCY;
}


#ifndef EXTF_dll_clock
OT_WEAK void dll_clock(ot_uint clocks) {
  //dll.comm.tca   -= clocks;
  //dll.comm.tc    -= clocks;
    clocks          = CLK2TI(clocks);
    
    if (sys.task_RFA.event != 0) {
        dll.comm.rx_timeout -= clocks;
    }
    else if (session_notempty()) {
        sys.task_RFA.event      = 2;
        sys.task_RFA.nextevent  = session_getnext();
    }
}
#endif



#ifndef EXTF_dll_systask_rf
OT_WEAK void dll_systask_rf(ot_task task) {
    ///Block callbacks while this runs (?)
    //radio_gag();
    
    //do {
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
    //} while ((task->event != 0) && (task->nextevent <= 0));
        
    //radio_ungag();
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
    m2session* active;
    ot_int proc_score;
    sys.task_RFA.event  = 0;                // Only run processing once
    active              = session_top();
    active->counter     = 0;
    proc_score          = network_route_ff(active);

    /// Response is prepared already, so setup holdstate and flow control.
    /// proc_score is always negative after parsing a response.
    if (proc_score >= 0) {
        sub_fceval(proc_score);
        sys.task_HSS.cursor = 0;
        dll.counter         = dll.netconf.hold_limit;
        dll.idle_state      = M2_DLLIDLE_HOLD;

        /// If the Listen flag is high, clone the session to a time in the
        /// future, when it will listen again.
        /// <LI> dll.comm.tc has just been assigned by the request.  It is the
        ///      response contention period (Tc). </LI>
        /// <LI> This device should listen again after Tc ends, so the session
        ///      is cloned & scheduled for Tc </LI>
        /// <LI> The current session is popped after response, or on next kernel
        ///      loop (immediately) if no response </LI>
        if (active->flags & M2_FLAG_LISTEN) {
            network_cont_session(active->applet, M2_NETSTATE_REQRX, 0);
        }
    }

    // No response (or no listening for responses)
    // Plus bad score: stop the session
    else if ((active->netstate & M2_NETSTATE_RESP) == 0) {
        //active->netstate |= M2_NETFLAG_SCRAP;
        session_pop();
        dll_idle();
    }
    
    // Listening for responses (keep listening)
    //else {        
    //}
}




OT_WEAK void dll_systask_holdscan(ot_task task) {
/// The Hold Scan process runs as an independent systask.  It will perform the
/// scan using the same routine as the sleep scan, but it does some additional
/// checks on switching between hold and sleep modes.  Namely, it increments
/// the hold_cycle parameter, and if this is over the limit it will return into
/// sleep mode.
    
    // Do holdscan (top) or go back into sleep mode (bottom)
    if (dll.counter != 0) {
        dll.counter -= (sys.task_HSS.cursor == 0);
        dll_systask_sleepscan(task);
    }
    else {
        dll.idle_state      = sub_default_idle();
        sys.task_HSS.event  = 0;
        sys.task_HSS.cursor = 0;
        sys.task_SSS.event  = 5;
    }
    
    ///@todo see if it is possible to do this in two tasks, simply disable
    /// beacon task during hold.
}





OT_WEAK void dll_systask_sleepscan(ot_task task) {
/// The Sleep Scan process runs as an independent task.  It is very similar
/// to the Hold Scan process, which actually calls this same routine.  They
/// use independent task markers, however, so they behave differently.
    ot_u8       s_channel;
    ot_u8       s_flags;
    ot_uni16    scratch;
    vlFILE*     fp;
    m2session*  s_new;

    if (task->event == 0) return;

    fp = ISF_open_su( task->event );
    ///@note fp doesn't really need to be asserted unless you are mucking 
    ///      with things in test builds.

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
    s_new           = session_new(&dll_scan_applet, 0, s_channel,
                                    ((M2_NETSTATE_REQRX | M2_NETSTATE_INIT) | (s_flags & 0x80) >> 1)  );
    s_new->extra    = s_flags;
}




#if (M2_FEATURE(BEACONS) == ENABLED)
OT_WEAK void dll_systask_beacon(ot_task task) {
/// The beacon rountine runs as an idependent systask.
    vlFILE*     fp;
    m2session*  b_session;
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
    b_session           = session_new(  &dll_beacon_applet, 0, scratch.ubyte[0],
                                        (M2_NETSTATE_INIT | M2_NETSTATE_REQTX | M2_NETFLAG_FIRSTRX)  );
    b_session->subnet   = dll.netconf.b_subnet;
    b_session->extra    = scratch.ubyte[1];
    b_session->flags    = scratch.ubyte[1] & 0x78;
    //]b_session->flags   |= (b_session->extra & 0x30);

    // Second & Third 2 bytes: ISF Call Template
    bq_data.ushort[0]   = vl_read(fp, task->cursor+=2);
    bq_data.ushort[1]   = vl_read(fp, task->cursor+=2);

    // Last 2 bytes: Next Scan ticks
#   ifdef __BIG_ENDIAN__
        scratch.ushort = TI2CLK( vl_read(fp, task->cursor+=2) );
#   else
        scratch.ushort = vl_read(fp, task->cursor+=2);
        scratch.ushort = PLATFORM_ENDIAN16(scratch.ushort);
#   endif
    sys_task_setnext(task, scratch.ushort);

    /// Beacon List Management:
    /// <LI> Move cursor onto next beacon period </LI>
    /// <LI> Loop cursor if it is past the length of the list </LI>
    /// <LI> In special case where cursor = 254, everything still works! </LI>
    task->cursor += 2;
    task->cursor  = (task->cursor >= fp->length) ? 0 : task->cursor;
    vl_close(fp);
}
#endif









/** External DLL applets <BR>
  * ========================================================================<BR>
  */
OT_WEAK void dll_default_applet(m2session* active) {
    dll_set_defaults(active);
}


/** Internal DLL applets <BR>
  * ========================================================================<BR>
  */

OT_WEAK void dll_response_applet(m2session* active) {
/// If this is a response transmission of a session with "Listen" active, it
/// means the contention period (Tc) is followed immediately with a subsequent
/// request.  We must not overlap that request with the tail-end of our own
/// response.  Therefore, we subtract from Tc the duration of this response.
    if (active->flags & M2_FLAG_LISTEN) {
        ot_u8 substate = active->netstate & M2_NETSTATE_TMASK;
        
        if (substate == M2_NETSTATE_RESPTX) {
            dll.comm.tc -= TI2CLK(rm2_pkt_duration(&txq));
        }
        else if (substate == M2_NETSTATE_REQRX) {
            sys.task_HSS.cursor     = 0;
            sys.task_HSS.nextevent  = dll.comm.rx_timeout;
            dll.comm.rx_timeout     = rm2_default_tgd(active->channel);
        }
    }
}


OT_WEAK void dll_scan_applet(m2session* active) {
/// Scanning is a Request-RX operation, so Tc is not important.
    ot_u8 scan_code;
    dll_set_defaults(active);
    
    scan_code               = active->extra;
    active->extra           = 0;
    dll.comm.rx_timeout     = otutils_calc_timeout(scan_code);
    //dll.comm.csmaca_params  = 0;
}


OT_WEAK void dll_beacon_applet(m2session* active) {
/// Beaconing is a Request-TX operation, and the value for Tc is the amount of
/// time to spend in CSMA before quitting the beacon.
    ot_queue beacon_queue;
    ot_u8 b_params;
    ot_u8 cmd_ext;
    ot_u8 cmd_code;
    
    b_params        = active->extra;
    active->extra   = 0;

    /// Start building the beacon packet:
    /// <LI> Calling m2np_header() will write most of the front of the frame </LI>
    /// <LI> Add the command byte and optional command-extension byte </LI>
    m2np_header(active, M2RT_BROADCAST, M2FI_FRDIALOG);
    
    cmd_ext     = (b_params & 0x06);
    cmd_code    = 0x20 | (b_params & 1) | ((cmd_ext!=0) << 7);
    q_writebyte(&txq, cmd_code);
    if (cmd_ext) {
        q_writebyte(&txq, cmd_ext);
    }

    /// Setup the comm parameters, if the channel is available:
    /// <LI> dll.comm values tx_eirp, cs_rssi, and cca_rssi must be set by the
    ///      radio module during the CSMA-CA process -- don't set them here.
    ///      The DASH7 spec requires it to happen in this order. </LI>
    /// <LI> Set CSMA-CA parameters, which are used by the radio module </LI>
    /// <LI> Set number of redundant TX's we would like to transmit </LI>
    /// <LI> Set rx_timeout for Default-Tg or 0, if beacon has no response </LI>
    dll_set_defaults(active);
    dll.comm.tc             = TI2CLK(M2_PARAM_BEACON_TCA);
    dll.comm.rx_timeout     = (b_params & 0x02) ? \
                                0 : rm2_default_tgd(active->channel);
    dll.comm.csmaca_params |= (b_params & 0x04) | M2_CSMACA_NA2P | M2_CSMACA_MACCA;
    dll.comm.redundants     = dll.netconf.b_attempts;  

    q_writebyte(&txq, (ot_u8)dll.comm.rx_timeout);

    /// If the beacon data is missing or otherwise not accessible by the 
    /// GUEST user, scrap this session.  Else, finish the M2NP frame.
    q_init(&beacon_queue, &bq_data.ubyte[0], 4);
    if (m2qp_isf_call((b_params & 1), &beacon_queue, AUTH_GUEST) < 0) {
        //active->netstate = M2_NETFLAG_SCRAP;
        session_pop();
        dll_idle();
    }
    else {
        m2np_footer();
    }
}





/** DLL Systask RF I/O routines   <BR>
  * ========================================================================<BR>
  * These routines & subroutines are called from the dll_systack() function.
  * All of these routines interact with the Radio module & driver, which is an
  * interrupt-driven pre-emptive task.
  */
void sub_activate() {
/// Do session creation
/// 1. Block DLL Idle-time tasks: they get reactivated by dll_idle()
/// 2. Get top session
/// 3. Associated Applet can construct packet, or control parameters
/// 4. Session is terminated if "SCRAP" bit is 1
/// 5. Session is processed otherwise
    m2session*  s_active;
    ot_app      s_applet;
    
    dll_block_idletasks();
    
  //dll.idle_state      = sub_default_idle();
    s_active            = session_top();
    s_applet            = (s_active->applet == NULL) ? \
                            &dll_response_applet : s_active->applet;
    s_active->applet    = NULL;
    s_applet(s_active);
    
    if (s_active->netstate & M2_NETFLAG_SCRAP) {
        session_pop();
        dll_idle();
    }
    else if (s_active->netstate & M2_NETSTATE_RX) {
        sub_init_rx(s_active);
    }
    else { 
        sub_init_tx(s_active->netstate & M2_NETFLAG_FLOOD);
    }
}
  
  

void sub_timeout_scan() {
/// This function just operates the timing-out of the RX.  RX is forced into
/// timeout when there is no data being received OR if the MAC is operating
/// under the Arbitrated regime, which uses strict time-slots.

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
    if ((radio.state != RADIO_DataRX) || (dll.comm.csmaca_params & M2_CSMACA_A2P)) {    ///@todo change to LISTEN
        rm2_rxtimeout_isr();
    }
    else {
        sys.task[TASK_radio].event = 5;
        sys_task_setnext(&sys.task[TASK_radio], 128);
    }

#else
    // Add a little bit of time in case the radio timer is a bit slow.
    sys_task_setnext(&sys.task[TASK_radio]->nextevent, 10);
    sys.task[TASK_radio].event = 0;
#endif
}



void sub_init_rx(m2session* active) {
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
    ot_sig2 callback;
    ot_u16  min_timeout;
    ot_u8   is_brx;
    
    sys.task_RFA.event      = 3;
  //sys.task_RFA.reserved   = 10;   //un-necessary, RFA is max priority
    sys.task_RFA.latency    = (active->netstate & M2_NETSTATE_RESP) ? \
                                _RESPRX_LATENCY : _REQRX_LATENCY;
    
    min_timeout = rm2_rxtimeout_floor(active->channel);
    if (dll.comm.rx_timeout < min_timeout) {
        dll.comm.rx_timeout = min_timeout;
    }
    
	sys_task_setnext(&sys.task[TASK_radio], (ot_u16)dll.comm.rx_timeout);

    // E.g. lights a LED
    DLL_SIG_RFINIT(sys.task_RFA.event);
    
    is_brx      = (active->netstate & M2_NETFLAG_FLOOD);   
    //if (is_brx == 0) GPIOA->BSRRH = 1<<0;     //LATENCY TEST
    callback    = is_brx ? &rfevt_bscan : &rfevt_frx;
    
    rm2_rxinit(active->channel, is_brx, callback);
}




void sub_init_tx(ot_u8 is_btx) {
/// Initialize background or foreground packet TX.  Often this includes CSMA
/// initialization as well.
    sys_task_setnext_clocks(&sys.task[TASK_radio], dll.comm.tc);
    dll.comm.tca            = sub_fcinit();
    sys.task_RFA.latency    = 1; 
    sys.task_RFA.event      = 4;
    
    DLL_SIG_RFINIT(sys.task_RFA.event);
    
#if (SYS_FLOOD == ENABLED)
    if (is_btx) {
        m2advp_open( session_follower() );
    }
    
    //LATENCY TEST
    //else GPIOA->BSRRH = 1<<1;
    
    rm2_txinit(is_btx, &rfevt_txcsma);
#else
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
        //rm2_rxinit(dll.comm.rx_chanlist[0], 0, &rfevt_bscan);    //non-blocking
        rm2_reenter_rx(&rfevt_bscan);   //non-blocking
        return;
    }
    
    // General Error: usually a timeout
    if (scode < 0) {
    	goto rfevt_FAILURE;
    }
    
    // A valid packet was received:
    // - Check subnet and EIRP filters
    // - network_parse_bf() will update the session stack as needed
    if (radio_mac_filter()) {
        if (network_parse_bf()) {
            goto rfevt_SUCCESS;
        }
    }

    // A failure, due to one or more of the following reasons:
    // - Timeout
    // - BG Packet sent to different subnet
    // - Session stack is full
    // - parsing error
    rfevt_FAILURE:
    session_pop();
    dll_idle();

    rfevt_SUCCESS:
    DLL_SIG_RFTERMINATE(3, scode);

    sys.task_RFA.event = 0;
    sys_preempt(&sys.task_RFA, 0);
}




void rfevt_frx(ot_int pcode, ot_int fcode) {
/// Radio Core event callback, called by the radio driver when a frame is rx'ed
/// or if there is some type of error.
    ot_int      frx_code= 0;
    m2session*  active  = session_top();
    
    /// If pcode is less than zero, it is because of a listening timeout.
    /// Listening timeouts happen after unfulfilled request scanning, or after
    /// Response scanning window expires.  In certain cases, after a timeout,
    /// the session persists.  These cases are implemented below.
    if (pcode < 0) {
        sys.task_RFA.event  = 0;
      //frx_code            = -1;
        if (dll.comm.redundants) {
            active->netstate   = (M2_NETSTATE_REQTX | M2_NETSTATE_INIT | M2_NETFLAG_FIRSTRX);
        }
        else if (((active->netstate & M2_NETSTATE_TMASK) == M2_NETSTATE_RESPRX) \
               && (active->applet != NULL)) {
            active->netstate  &= ~M2_NETSTATE_TMASK;   // Default to Request-TX   
        }
        else {
            //active->netstate   = M2_NETFLAG_SCRAP;
            session_pop();
            dll_idle();
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
        else if (radio_mac_filter() == False) {
            frx_code = -4;
        }
        
        /// A complete packet has been received (errors or not).
    	/// <LI> When packet is good (frx_code == 0), always process it. </LI>
    	/// <LI> When request is bad or when response is any form, retry
        ///      listening until window times-out </LI>
        /// <LI> Don't return to kernel for bad frames </LI>
        /// <LI> After receiving good request, turn-off radio subsystem </LI>
        if (pcode == 0) {
            ot_bool rx_isresp = (active->netstate & M2_NETSTATE_RESP);
            
            if (frx_code == 0) {
                sys.task_RFA.reserve = 20;  ///@todo Could have quick evaluator here
                sys.task_RFA.event   = 1;   ///Process the packet!!!
            }
            if (rx_isresp) {
                dll.counter     = dll.netconf.hold_limit;
                dll.idle_state  = M2_DLLIDLE_HOLD;
            }
            if (frx_code || rx_isresp) {
                rm2_reenter_rx(&rfevt_frx);
            }
            else {
                radio_sleep();
            }
            
            // This should follow the above operations, because callback from
            // DLL_SIG_RFTERMINATE might take a relatively long time.
            if (frx_code != 0) {
                DLL_SIG_RFTERMINATE(3, frx_code);
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

    /// ON CSMA SUCCESS: pcode == 0, tcode == 1/0 for BG/FG
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
            event_ticks     = (ot_uint)(rm2_pkt_duration(&txq) + 4);
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
        //m2session* active;
        DLL_SIG_RFTERMINATE(sys.task_RFA.event, pcode);
        
        //active              = session_top();
        //active->netstate   |= M2_NETFLAG_SCRAP;
        session_pop();
        dll_idle();
        
        sys.task_RFA.event  = 0;
        event_ticks         = 0;
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
        		|| (dll.comm.rx_timeout <= 0));
        
        /// Send redundant TX immediately, but only if no response window or if
        /// this packet is a response.
        dll.comm.redundants--;
        if ((dll.comm.redundants != 0) && scratch) {
            dll.comm.csmaca_params = (M2_CSMACA_NOCSMA | M2_CSMACA_MACCA);
            rm2_resend( (ot_sig2)&rfevt_txcsma );
            return;
        }
        
        /// Scrap (End) Session if:
        /// <LI> There is no redundant TX to send (implicit)</LI>
        /// <LI> AND (No response window to listen OR This TX itself is a response)</LI>
        /// <LI> OR Some sort of error occurred (pcode != 0) </LI>
        /// Otherwise, continue session as listening for response.
        if (scratch || pcode) {
            session_pop();
            dll_idle();
        }
        
        /// Don't scrap the session, instead go for response listening
        else {
            session->netstate  &= ~M2_NETSTATE_TMASK;
            session->netstate  |= M2_NETSTATE_RESPRX;
        }
        
        /// Terminate Transmit state
        DLL_SIG_RFTERMINATE(sys.task_RFA.event, pcode);
        sys.task_RFA.event = 0;
        sys_preempt(&sys.task_RFA, 0);
        
//        else {
//        	//M2_NETFLAG_SCRAP is represented below
//            session->netstate  |= (ot_u8)(((pcode != 0) | scratch) << 7);
//            session->netstate  &= ~M2_NETSTATE_TMASK;
//            session->netstate  |= M2_NETSTATE_RESPRX;
//
//            DLL_SIG_RFTERMINATE(sys.task_RFA.event, pcode);
//            sys.task_RFA.event = 0;
//        	sys_preempt(&sys.task_RFA, 0);
//        }
    }
}



void rfevt_btx(ot_int flcode, ot_int scratch) {
#if ((M2_FEATURE(SUBCONTROLLER) == ENABLED) || (M2_FEATURE(GATEWAY) == ENABLED))
    ot_int countdown;
    countdown = (ot_int)radio_get_countdown();
    if (countdown < 0) {
        countdown = 0;
    }
    
    switch (flcode) {
        /// Flood ends & Request Begins 
        /// <LI> Pop the flood session, it is no longer needed </LI>
        /// <LI> Tweak the request session to work optimally following flood </LI>
        /// <LI> Pre-empt the kernel to start on the request session.  The 
        ///      kernel will clock other tasks over the flood duration.  </LI>
        case 0: {
            m2session* follower;
            // assure request hits NOW & assure it doesn't init dll.comm
            // Tweak dll.comm for request (2 ti is a token, small amount)
            //session_invite_follower();
            follower                = session_follower();
            follower->counter       = countdown;
            sys.task_RFA.event      = 0;
            dll.comm.tc             = TI2CLK(2);
            dll.comm.csmaca_params  = (M2_CSMACA_NOCSMA | M2_CSMACA_MACCA);
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
            ///@todo make faster function for bg packet duration lookup
            if (countdown < rm2_bgpkt_duration()) {
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



#endif // !__KERNEL_NONE__





/** Subroutines for DLL configuration and Flow & Congestion Control
  * ============================================================================
  * @todo these two functions and perhaps a few others might get put into the
  *      m2_network section, or some new section.
  */

#ifndef EXTF_dll_quit_rf
OT_WEAK void dll_quit_rf() {
#   ifndef __KERNEL_NONE__
    sys.task_RFA.event = 0;
#   endif
}
#endif


#ifndef EXTF_dll_default_csma
OT_WEAK ot_u8 dll_default_csma(ot_u8 chan_id) {
    ///@todo revisit if there's merit to having unguarded channels like this
    //chan_id &= 0x30;
    //return (((chan_id == 0x00) || (chan_id == 0x30)) << 2);
    return 0;
}
#endif


#ifndef EXTF_dll_set_defaults
OT_WEAK void dll_set_defaults(m2session* s_active) {
/// Set some DLL parameters that are important as general purpose
    ot_u16 follower         = session_follower_wait();
    dll.comm.tc             = follower >> 3;
    dll.comm.rx_timeout     = follower;
    //dll.comm.csmaca_params  = dll_default_csma(s_active->channel);
    dll.comm.csmaca_params  = M2_CSMACA_MACCA;
    dll.comm.redundants     = ((s_active->netstate & M2_NETSTATE_RX) == 0);
    dll.comm.tx_channels    = 1;
    dll.comm.rx_channels    = 1;
    dll.comm.tx_chanlist    = &dll.comm.scratch[0];
    dll.comm.rx_chanlist    = &dll.comm.scratch[1];
    dll.comm.scratch[0]     = s_active->channel;
    dll.comm.scratch[1]     = s_active->channel;
}
#endif




ot_u8 sub_default_idle() {
    return M2_DLLIDLE_SLEEP;

//#if (M2_FEATURE(ENDPOINT) == ENABLED)
//    ot_u16 setting;
//    setting = (dll.netconf.active & M2_SET_CLASSMASK) >> _SETTING_SHIFT;
//    if (setting > 1) {
//        setting = M2_DLLIDLE_HOLD;
//    }    
//    return (ot_u8)setting;
//#else
//    return (dll.netconf.active & M2_SET_CLASSMASK) ? M2_DLLIDLE_HOLD : M2_DLLIDLE_OFF;
//#endif
}




ot_bool sub_mac_filter() {
    return radio_mac_filter();
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
        random %= (dll.comm.tc - TI2CLK(rm2_pkt_duration(&txq)) );
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
        return TI2CLK(rm2_pkt_duration(&txq));
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
    return TI2CLK(rm2_pkt_duration(&txq));
}
*/



#undef _REQRX_LATENCY
#undef _RESPRX_LATENCY

    
#endif  // OT_FEATURE(M2)

