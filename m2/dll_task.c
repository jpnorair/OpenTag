/* Copyright 2013-2014 JP Norair
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
  * @version    R106
  * @date       7 May 2014
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


#include <otstd.h>
#include <platform/config.h>

#if (OT_FEATURE(M2) == ENABLED)

#include <m2/dll.h>
#include <m2/network.h>
#include <m2/radio.h>
#include <m2/session.h>
#include <m2/transport.h>

#include <otsys/syskern.h>

#include <otlib/auth.h>         ///@todo might not be necessary here
#include <otlib/buffers.h>
#include <otlib/rand.h>
#include <otlib/utils.h>
#include <otsys/veelite.h>

#include <platform/config.h>
//#include <otplatform.h>         // only needed for certain testing

#if (OT_FEATURE(CRON))
#   include "otcron.h"
#endif

#if (M2_FEATURE(BEACONS))
#endif


/** Test and Variant Configuration
 * _DLL_BLOCK_IDLE_ON_QUEUEING:
 * - Will block Sleep/Hold/Beacon while waiting for a delayed session.
 * - Temporary.  Will be replaced by session queuing heuristic.
 * - Implementation will change also when "IdleTasks" grouped into one task.
 *
 */

#define _DLL_BLOCK_IDLE_ON_QUEUEING





m2dll_struct    dll;

static void sub_dll_flush(void);





/** Flow & Congestion Control Subroutines
  * ============================================================================
  */

ot_u8 sub_default_idle(void);



/** @brief Performs subnet filtering (per Mode 2 spec) and returns status
  * @retval ot_bool     True/False on received frame subnet passes/fails filter
  * @ingroup System
  */
//moved to rm2_mac_filter()


/** @brief Scrambles the response-channel-list in order to improve collision
  *        avoidance when multiple response channels are available.
  * @ingroup System
  */
static void sub_csma_scramble(void);


/** @brief Initializes the flow/congestion control sequence
  * @retval ot_uint     Initial TX backoff, in ticks
  * @ingroup System
  */
static CLK_UNIT sub_fcinit(void);


/** @brief Evaluates the TX slot usage based on the quality of the query
  * @param  query_score (ot_int) To be defined
  * @retval none
  * @ingroup System
  * @note Optional, not implemented yet
  */
static void sub_fceval(ot_int query_score);


/** @brief Processes the flow/congestion control sequence following initialization
  * @retval ot_uint     Subsequent TX backoff, in ticks
  * @ingroup System
  */
static CLK_UNIT sub_fcloop(void);


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
static CLK_UNIT sub_rigd_newslot(void);


/** @brief Continues an ongoing RIGD sequence and determines subslot TX offset
  * @retval ot_uint     Number of ticks until TX for next slot should commence
  * @ingroup System
  * @sa sub_fcloop(), sub_rigd_newslot()
  *
  * This is called by sub_fcloop() and is much like sub_rigd_newslot().
  */
//static ot_uint sub_rigd_nextslot(void);


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
//static ot_uint sub_aind_nextslot(void);









/** Initializers state management functions <BR>
  * ========================================================================<BR>
  */

OT_WEAK void dll_block_idletasks(void) {
    sys.task_HSS.event  = 0;
#   if (M2_FEATURE(BEACONS) == ENABLED)
    sys.task_BTS.event  = 0;
#   endif
    sys.task_SSS.event  = 0;
}




void sub_dll_flush(void) {
/// (1) Put System states into the right place and flush existing events.
/// (2) Reset sessions.
/// (3) Set scheduler ids and prepare idle time events
	ot_task task;

	if (radio.state != RADIO_Idle) {
		rm2_kill();
	}
    rm2_mac_configure();

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
OT_WEAK void dll_init(void) {
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
OT_WEAK void dll_refresh(void) {
    //ot_uni16 scratch;
    vlFILE* fp;

    /// Open Network Features ISF and load the values from that file into the
    /// cached dll.netconf settings.

    fp = ISF_open_su(0);
    vl_load(fp, 10, dll.netconf.vid);
    dll.netconf.dd_flags    = 0;
    dll.netconf.hold_limit  = PLATFORM_ENDIAN16(dll.netconf.hold_limit);
    vl_close(fp);

    fp = ISF_open_su(1);
    vl_load(fp, 8, dll.netconf.uid);
    vl_close(fp);

    // Reset the Scheduler (only does anything if scheduler is implemented)
    dll_refresh_rts();
    sub_dll_flush();
}
#endif



#ifndef EXTF_dll_refresh_rts
OT_WEAK void dll_refresh_rts(void) {
#if (OT_FEATURE(CRON) && M2_FEATURE(RTC_SCHEDULER))
///@todo need to update this implementation to also delete the old tasks.

    vlFILE* fp;

    // Early exit if RTS is not active
    if (dll.netconf.active_settings & M2_SET_SCHEDMASK) == 0)
        return;

    // - open the real-time-schedule ISF
    // - make sure the file is there
    // - make sure the file has at least 12 bytes (this is how much DLL uses)
    fp = ISF_open_su(ISF_ID_real_time_scheduler);
    if (fp != NULL) {
        if (fp->length < 12) {
            static const ot_task task_fn[3] = { &dll_systask_holdscan,
                                                &dll_systask_sleepscan,
                                                &dll_systask_beacon };

            ot_u16  sched_enable    = dll.netconf.active_settings & M2_SET_SCHEDMASK;
            ot_u16  cursor          = 0;
            ot_sigv* fn             = task_fn;

            // Apply Hold, Sleep, and Beacon RTS as specified in enabler bitmask
            while (sched_enable != 0) {
                if (sched_enable & M2_SET_HOLDSCHED) {
                    ot_u32 time_evt;
                    ot_u32 time_mask;
                    time_evt    = vl_read(fp, cursor);
                    time_mask   = 0xFFFF0000 | vl_read(fp, cursor+2);
                    otcron_add(*fn, CRONTIME_absolute, time_evt, time_mask);
                }
                fn++;
                cursor         += 4;
                sched_enable  <<= 1;
            }
        }
        vl_close(fp);
    }
#endif
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


#ifndef EXTF_dll_silence
OT_WEAK void dll_silence(ot_bool onoff) {

	///@todo this is a hack to pause the idle state timers.
	///      Replace it in the near future with a set to bit 12 on .active,
	///      and have the actual sleep and beacon tasks simply ignore
	if (onoff == true) {
		sys.task_SSS.nextevent 	= INT_MAX;
		sys.task_BTS.nextevent	= INT_MAX;
	}
	else {
		sys_task_setnext(&sys.task_SSS, 500);
		sys_task_setnext(&sys.task_BTS, 2000);
	}

	///@todo I want to do it like below, but somehow this method completely
	///      crashes the scheduler.  Need to investigate why.
//	if (onoff == false) {
//		dll.netconf.active &= ~(1<<12);
//	}
//	else {
//		dll.netconf.active |= (1<<12);
//	}
}
#endif

#ifndef EXTF_dll_goto_off
OT_WEAK void dll_goto_off(void) {
    dll.idle_state = M2_DLLIDLE_OFF;
    dll_idle();
}
#endif




#ifndef EXTF_dll_idle
OT_WEAK void dll_idle(void) {
/// Idle Routine: Disable radio and manipulate system tasks in order to go into
/// the type of idle that is configured in dll.idle_state (OFF, SLEEP, HOLD)
	static const ot_u8 scan_events[] = { 0,0, 0,5, 4,0, 1,0 };
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
            dll.comm.tc -= rm2_pkt_duration(&txq);  //TI2CLK(rm2_pkt_duration(&txq));
        }
        else if (substate == M2_NETSTATE_REQRX) {
            sys.task_HSS.cursor     = 0;
            sys.task_HSS.nextevent  = TI2CLK(dll.comm.rx_timeout);
            dll.comm.rx_timeout     = rm2_default_tgd(active->channel);
        }
    }
}


OT_WEAK void dll_scan_applet(m2session* active) {
/// Scanning is a Request-RX operation, so Tc is not important.
    ot_long timeout;
    ot_u8 scan_code;

    dll_set_defaults(active);
    scan_code       = active->extra;
    active->extra   = 0;
    timeout         = otutils_calc_timeout(scan_code);

    if (timeout > 65535) {
        timeout = 65535;
    }

    dll.comm.rx_timeout = (ot_u16)timeout;
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

    cmd_ext     = (b_params & 0x06);                            // Normal extension bits
    cmd_ext    |= (dll.netconf.btemp[2] == 0) << 6;             // Announcement No-File bit
    cmd_code    = 0x20 | (b_params & 1) | ((cmd_ext!=0) << 7);
    q_writebyte(&txq, cmd_code);
    if (cmd_code) {
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
    dll.comm.tc             = M2_PARAM_BEACON_TCA;  //TI2CLK(M2_PARAM_BEACON_TCA);
    dll.comm.rx_timeout     = (b_params & 0x02) ? \
                                0 : rm2_default_tgd(active->channel);
    dll.comm.csmaca_params |= (b_params & 0x04) | M2_CSMACA_NA2P | M2_CSMACA_MACCA;
    dll.comm.redundants     = dll.netconf.b_attempts;

    q_writebyte(&txq, (ot_u8)dll.comm.rx_timeout);

    /// Add the announcement file data if the specified return data != 0 bytes.
    /// If the beacon data is missing or otherwise not accessible by the GUEST
    /// user, dump the session (thus killing the beacon) and go back to idle.
    if (dll.netconf.btemp[2] != 0) {
        q_init(&beacon_queue, &dll.netconf.btemp[2], 4);
        if (m2qp_isf_call((b_params & 1), &beacon_queue, AUTH_GUEST) < 0) {
            session_pop();
            dll_idle();
            return;
        }
    }

    /// Final step to any packet generation: add footer
    m2np_footer();
}






/** DLL Systask Manager <BR>
  * ========================================================================<BR>
  */

// For synchronization test only
//volatile ot_u32 sample_t0;
//volatile ot_u32 sample_t1;


#if !defined(__KERNEL_NONE__)

#ifdef OT_FEATURE_LISTEN_ALLOWANCE
#   define _REQRX_LATENCY   OT_PARAM_LISTEN_ALLOWANCE
#   define _RESPRX_LATENCY  OT_PARAM_RECEIVE_ALLOWANCE
#else
#   define _REQRX_LATENCY   40
#   define _RESPRX_LATENCY  2
#endif

OT_WEAK void dll_block(void) {
	sys.task_RFA.latency = 0;
}

OT_WEAK void dll_unblock(void) {
	sys.task_RFA.latency = _REQRX_LATENCY;
}


#ifndef EXTF_dll_clock
OT_WEAK void dll_clock(ot_uint clocks) {
/// @todo experiment giving exotasks two handles.
///       one for clocking and administrative uses, one for tasking.
    //ot_uint ticks;
    //ticks = CLK2TI(clocks);

    if (sys.task_RFA.event != 0) {
        //dll.comm.rx_timeout -= ticks;
    	dll.comm.rx_timeout -= CLK2TI(clocks);
    }
    else if (session_notempty()) {
        sys.task_RFA.event      = 2;
        sys.task_RFA.nextevent  = clocks + TI2CLK(session_getnext());

        ///@note added 18 Oct 17 for testing behavior of scheduled sessions
#		ifdef _DLL_BLOCK_IDLE_ON_QUEUEING
        dll_block_idletasks();
#		endif

        // Synchronization test
        //volatile ot_u16 next_session;
        //next_session            = session_getnext();
        //sys.task_RFA.nextevent  = next_session;
       // if (next_session != 0) {
        //    sample_t1 = 0;
        //}
    }
}
#endif



#ifndef EXTF_dll_systask_rf
OT_WEAK void dll_systask_rf(ot_task task) {
    ///Block callbacks while this runs (?)
    //radio_gag();

    //do {
        switch (sys.task_RFA.event) {
            // Refresh task
            case 0: dll_refresh();          break;

            // Processing
            case 1: dll_processing();       break;

            // Session Activation: this can activate Radio Exotasks.
            case 2: dll_activate();         break;

            // RX Scan Timeout Watchdog
            case 3: dll_scan_timeout();    break;

            // CSMA Manager (needed for archaic radios only)
            //case 4: dll_txcsma();         break;
            case 4:

            // TX & CSMA Timeout Watchdog
            case 5: rm2_kill();             break;
            
            // Power Code setting on cursor
            case 255: task->cursor = radio_getpwrcode(); break;
            
            default: break;
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

void dll_processing(void) {
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
            session_continue(active->applet, M2_NETSTATE_REQRX, 0);
        }
    }

    /// Bad score, plus session indicates no listening or sending response.
    /// Scrap the session.
    else if ((active->netstate & M2_NETSTATE_RESP) == 0) {
        goto dll_processing_SCRAP;
    }

    /// A protocol parser has scrapped the session, or possibly the above
    /// condition branched here directly.
    if (active->netstate & M2_NETSTATE_SCRAP) {
        dll_processing_SCRAP:
        session_pop();
        dll_idle();
    }
}




OT_WEAK void dll_systask_holdscan(ot_task task) {
/// The Hold Scan process runs as an independent systask.  It will perform the
/// scan using the same routine as the sleep scan, but it does some additional
/// checks on switching between hold and sleep modes.  Namely, it increments
/// the hold_cycle parameter, and if this is over the limit it will return into
/// sleep mode.

    // This is a stateless task that should probably be removed from task status
    // and replaced with a cron task or something similar.
    if (task->event == 0) return;
    
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
    ot_u8       s_code;
    ot_uni16    scratch;
    vlFILE*     fp;
    m2session*  s_new;

    /// event = 0 is the initializer, but there is no state-based operation
    /// or any heap-stored data for this task, so it does nothing
    if (task->event == 0) {
        return;
    }

    ///@todo check against session availability.

    fp = ISF_open_su( task->event );
    ///@note fp doesn't really need to be asserted unless you are mucking
    ///      with things in test builds.

    /// Pull channel ID and Scan flags
    scratch.ushort  = vl_read(fp, task->cursor);
    s_channel       = scratch.ubyte[0];
    s_code          = scratch.ubyte[1];

    /// Set the next idle event from the two-byte Next Scan field.
    /// The DASH7 registry is big-endian.
    scratch.ushort  = PLATFORM_ENDIAN16( vl_read(fp, (task->cursor)+=2 ) );
    sys_task_setnext(task, (ot_u32)scratch.ushort);

    /// Advance cursor to next datum, go back to 0 if end of sequence
    /// (still works in special case where cursor = 254)
    task->cursor   += 2;
    task->cursor    = (task->cursor >= fp->length) ? 0 : task->cursor;
    vl_close(fp);

    /// Choosing Background-Scan or Foreground-Scan is based on scan-code.
    /// If b7 is set, do a Background-Scan.  At the session level, the "Flood"
    /// select uses b6, which is why there is a >> 1.
    ///@todo the method of checking for silence seems to crash OpenTag right now.
    //if ((dll.netconf.active & (1<<12)) == 0) {
		s_new           = session_new(&dll_scan_applet, 0, s_channel,
										((M2_NETSTATE_REQRX | M2_NETSTATE_INIT) | (s_code & 0x80) >> 1)  );
		s_new->extra    = s_code;
    //}
}




#if (M2_FEATURE(BEACONS) == ENABLED)
OT_WEAK void dll_systask_beacon(ot_task task) {
/// The beacon rountine runs as an idependent systask.
    m2session*  b_session;
    ot_u16      nextbeacon;

    if ((task->event == 0) || (dll.netconf.b_attempts == 0)) {
        dll_idle();
        return;
    }

    ///@todo check against session availability.

    /// This is a retry time, in case of failure condition
    nextbeacon = 10;
    
    /// Load file-based beacon:
    /// Open BTS ISF Element and read the beacon sequence.  Make sure there is
    /// a beacon file of non-zero length and that beacons are presently enabled.
    if (dll.netconf.dd_flags == 0) {
        ot_u16  scratch;
        vlFILE* fp;

        fp = ISF_open_su( ISF_ID(beacon_transmit_sequence) );
        if (fp == NULL) {
            nextbeacon = 10;               // try again after this delay
            goto dll_systask_beacon_END;
        }
        if (fp->length == 0)    {
            vl_close(fp);
            goto dll_systask_beacon_END;
        }

        // move fp start onto the cursor in order to use vl_load() later on
        scratch     = fp->start;
        fp->start  += task->cursor;

        /// Beacon List Management:
        /// <LI> Move cursor onto next beacon period (above, +8)</LI>
        /// <LI> Loop cursor if it is past the length of the list </LI>
        /// <LI> In special case where cursor = 254, everything still works! </LI>
        task->cursor   += 8;
        task->cursor    = (task->cursor >= fp->length) ? 0 : task->cursor;

        // Load next beacon into btemp, then undo the start hack, then close
        vl_load(fp, 8, dll.netconf.btemp);
        fp->start = scratch;
        vl_close(fp);
    }

    // First 2 bytes: Chan ID, Cmd Code
    // - Setup beacon ad-hoc session, on specified channel (ad hoc sessions never return NULL)
    // - Assure cmd code is always Broadcast & Announcement
    ///@todo the method of checking for silence (b12) seems to crash OpenTag right now.
    //if ((dll.netconf.active & (1<<12)) == 0) {
		b_session           = session_new(  &dll_beacon_applet, 0, dll.netconf.btemp[0],
											(M2_NETSTATE_INIT | M2_NETSTATE_REQTX | M2_NETFLAG_FIRSTRX)  );
		b_session->subnet   = dll.netconf.b_subnet;
		b_session->extra    = dll.netconf.btemp[1];
		b_session->flags    = dll.netconf.btemp[1] & 0x78;
		//b_session->flags   |= (b_session->extra & 0x30);
    //}

    nextbeacon = (ot_u32)PLATFORM_ENDIAN16(*(ot_u16*)&dll.netconf.btemp[6]);
    
    dll_systask_beacon_END:
    
    // Last 2 bytes: Next Scan ticks
    sys_task_setnext(task, (ot_u32)nextbeacon);

    ///@note this might not be necessary or wise!
    //return;
    //dll_systask_beacon_STOP:
    //dll_idle();
}
#endif

















/** DLL Systask RF I/O routines   <BR>
  * ========================================================================<BR>
  * These routines & subroutines are called from the dll_systack() function.
  * All of these routines interact with the Radio module & driver, which is an
  * interrupt-driven pre-emptive task.
  */
OT_WEAK void dll_init_rx(m2session* active) {
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

    sys.task_RFA.event      = 3;
  //sys.task_RFA.reserved   = 10;   //un-necessary, RFA is max priority
    sys.task_RFA.latency    = (active->netstate & M2_NETSTATE_RESP) ? \
                                _RESPRX_LATENCY : _REQRX_LATENCY;

    min_timeout = rm2_rxtimeout_floor(active->channel);
    if (dll.comm.rx_timeout < min_timeout) {
        dll.comm.rx_timeout = min_timeout;
    }

	sys_task_setnext(&sys.task[TASK_radio], (ot_u32)dll.comm.rx_timeout);

    // E.g. lights a LED
    DLL_SIG_RFINIT(sys.task_RFA.event);

    //For synchronization test only
    //if ((active->netstate & M2_NETFLAG_BG) == 0) {
    //    sample_t1 = systim_chronstamp(&sample_t0);
    //    sample_t0 = 0;
    //}

    callback = (active->netstate & M2_NETFLAG_BG) ? \
                    &dll_rfevt_brx : &dll_rfevt_frx;

    rm2_rxinit(active->channel, active->netstate, callback);
}




OT_WEAK void dll_init_tx(ot_u8 is_btx) {
/// Initialize background or foreground packet TX.  Often this includes CSMA
/// initialization as well.
    //sys_task_setnext_clocks(&sys.task[TASK_radio], (ot_u32)dll.comm.tc);
    sys_task_setnext(&sys.task[TASK_radio], (ot_u32)dll.comm.tc);
    dll.comm.tca            = sub_fcinit();
    sys.task_RFA.latency    = 1;
    sys.task_RFA.event      = 4;

    DLL_SIG_RFINIT(sys.task_RFA.event);

#if (SYS_FLOOD == ENABLED)
    ///@todo this is a bit of a hack.  BG Floods should be used with a
    ///      network-layer function that decides which protocol is appropriate.
    if (is_btx == (M2_NETFLAG_BG | M2_NETFLAG_STREAM)) {
        m2advp_open( session_follower() );
    }

    rm2_txinit(is_btx, &dll_rfevt_txcsma);
#else
    rm2_txinit(0, &dll_rfevt_txcsma);
#endif
}





OT_WEAK void dll_activate(void) {
/// Do session creation
/// 1. Block DLL Idle-time tasks: they get reactivated by dll_idle()
/// 2. Get top session
/// 3. Associated Applet can construct packet, or control parameters
/// 4. Session is terminated if "SCRAP" bit is 1
/// 5. Session is processed otherwise
    m2session*  s_active;
    ot_app      s_applet;

#	ifndef _DLL_BLOCK_IDLE_ON_QUEUEING
    dll_block_idletasks();
#	endif

    //dll.idle_state      = sub_default_idle();
    s_active            = session_top();
    s_applet            = (s_active->applet == NULL) ? \
                            &dll_response_applet : s_active->applet;
    s_active->applet    = NULL;
    s_applet(s_active);

    if (s_active->netstate & M2_NETSTATE_SCRAP) {
        session_pop();
        dll_idle();
    }
    else if (s_active->netstate & M2_NETSTATE_RX) {
        dll_init_rx(s_active);
    }
    else {
        dll_init_tx(s_active->netstate & (M2_NETFLAG_BG | M2_NETFLAG_STREAM));
    }
}



OT_WEAK void dll_scan_timeout(void) {
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
    sys_task_setnext(&sys.task[TASK_radio], 10);
    sys.task[TASK_radio].event = 0;
#endif
}






/** DLL Radio Driver Task Callbacks <BR>
  * ============================================================================
  * The Radio Module & driver is implemented as a pre-emptive task, managed by
  * interrupts implemented in the radio module.  These functions will be called
  * by the radio module when a radio module task is done.  These function will
  * seed kernel level tasks, which will run as soon as the kernel is free.
  */

OT_WEAK void dll_rfevt_brx(ot_int scode, ot_int fcode) {
/// bscan reception radio-core event callback: called by radio core driver when
/// the bscan process terminates, either due to success or failure

    __DEBUG_ERRCODE_EVAL(=100);

    // CRC Failure (or init), retry
    if ((scode == -1) && (dll.comm.redundants != 0)) {
        __DEBUG_ERRCODE_EVAL(=101);
        rm2_reenter_rx(&dll_rfevt_brx);   //non-blocking
        return;
    }

    // General Error: usually a timeout
    ///@todo RM2 Error codes should be refactored.
    if (scode < 0) {
        // For synchronization test only
        //rxq.getcursor = &rxq.front[2];
        //rxq.front[3] = 0x1F;    // subnet & BPID
        //rxq.front[4] = 0x8A;    // next channel
        //rxq.front[5] = 0x01;    // countdown (upper)
        //rxq.front[6] = 0x00;    // countdown (lower)
        //if (network_parse_bf()) {
        //    sample_t0 = systim_chronstamp(NULL);
        //    goto dll_rfevt_SUCCESS;
        //}
        __DEBUG_ERRCODE_EVAL(=102);
        scode = RM2_ERR_GENERIC;
    	goto dll_rfevt_FAILURE;
    }

    // A valid packet was received:
    // - Check subnet and EIRP filters
    // - network_parse_bf() will update the session stack as needed
    if (rm2_mac_filter()) {
        __DEBUG_ERRCODE_EVAL(=103);
        if (network_parse_bf()) {
            goto dll_rfevt_SUCCESS;
        }
    }

    // A failure, due to one or more of the following reasons:
    // - Timeout
    // - BG Packet sent to different subnet
    // - Session stack is full
    // - parsing error
    dll_rfevt_FAILURE:
    session_pop();
    dll_idle();

    dll_rfevt_SUCCESS:
    DLL_SIG_RFTERMINATE(3, scode);

    sys.task_RFA.event = 0;
    sys_preempt(&sys.task_RFA, 0);
}




OT_WEAK void dll_rfevt_frx(ot_int pcode, ot_int fcode) {
/// Radio Core event callback, called by the radio driver when a frame is rx'ed
/// or if there is some type of error.
    ot_int      frx_code= 0;
    ot_bool     re_init = False;
    m2session*  active  = session_top();

    __DEBUG_ERRCODE_EVAL(=110);

    /// If pcode is less than zero, it is because of a listening timeout.
    /// Listening timeouts happen after unfulfilled request scanning, or after
    /// Response scanning window expires.  In certain cases, after a timeout,
    /// the session persists.  These cases are implemented below.
    if (pcode < 0) {
        __DEBUG_ERRCODE_EVAL(=111);
        sys.task_RFA.event  = 0;
        if (dll.comm.redundants) {
            active->netstate   = (M2_NETSTATE_REQTX | M2_NETSTATE_INIT | M2_NETFLAG_FIRSTRX);
        }
        else if (((active->netstate & M2_NETSTATE_TMASK) == M2_NETSTATE_RESPRX) \
               && (active->applet != NULL)) {
            active->netstate  &= ~M2_NETSTATE_TMASK;   // Default to Request-TX
        }
        else {
            //active->netstate   = M2_NETSTATE_SCRAP;
            session_pop();
            dll_idle();
        }
    }

    // Multiframe packet RX frame check
#   if (M2_FEATURE(M2DP) == ENABLED)
    else if (pcode > 0) {
        __DEBUG_ERRCODE_EVAL(=112);
    	if (fcode != 0) {
    		m2dp_mark_frame();
    	}
    	return;
    }
#	endif

    // pcode == 0 on last frame
    else {
        __DEBUG_ERRCODE_EVAL(=113);
        /// Handle damaged frames (bad CRC)
    	/// Run subnet filtering on frames with good CRC
    	if (fcode != 0) {
    		frx_code = -1;
    	}
        else if (rm2_mac_filter() == False) {
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

            re_init = (frx_code || rx_isresp);
            if (re_init) {
                rm2_reenter_rx(&dll_rfevt_frx);
            }
            else {
                radio_sleep();
            }
        }
    }

    /// The RX Callback Code is always "3"
    DLL_SIG_RFTERMINATE(3, frx_code);

    /// Re-initialize signal, if reinitializing
    if (re_init) {
        __DEBUG_ERRCODE_EVAL(=114);
        DLL_SIG_RFINIT(3);
    }

    /// Pre-empt the Kernel Scheduler on successful packet download or timeout
    if (frx_code == 0) {
        sys_preempt(&sys.task_RFA, 0);
    }
}




OT_WEAK void dll_rfevt_txcsma(ot_int pcode, ot_int tcode) {
    ot_uint event_ticks;

    __DEBUG_ERRCODE_EVAL(=120);

    /// ON CSMA SUCCESS: pcode == 0, tcode == 1/0 for BG/FG
    if (pcode == 0) {
        __DEBUG_ERRCODE_EVAL(=121);
        sys.task_RFA.latency    = 0;
        sys.task_RFA.event      = 5;
        
        ///@note tcode MUST have bit 0 set during BG mode and bit 1
        /// set during multiframe mode
        radio.evtdone   = (tcode & 1) ? &dll_rfevt_btx : &dll_rfevt_ftx;
        event_ticks     = (tcode & 2) ? dll.counter+20 : (ot_uint)(rm2_pkt_duration(&txq) + 4);
        radio_idle();
    }

    /// ON CSMA LOOP: calculate the next slot time and set mactimer accordingly
    /// Do not Preempt the kernel.
    /// @todo replace the "2" in the idle vs. sleep check with a per-radio constant
    else if (pcode > 0) {
        ot_uint nextcsma;
        __DEBUG_ERRCODE_EVAL(=122);

        nextcsma = (ot_uint)sub_fcloop();
        //if (nextcsma < TI2CLK(2))   radio_idle();
        if (nextcsma < 2) 
            radio_idle();
        else
            radio_sleep();

        radio_set_mactimer( nextcsma );
        return;
    }

    /// ON FAIL: Flag this session for scrap.  As with all sessions, if you
    /// have an applet attached to it, the applet can adjust the netstate and
    /// try again if it chooses.
    else {
        //m2session* active;
        __DEBUG_ERRCODE_EVAL(=123);
        DLL_SIG_RFTERMINATE(sys.task_RFA.event, pcode);

        //active              = session_top();
        //active->netstate   |= M2_NETSTATE_SCRAP;
        session_pop();
        dll_idle();

        sys.task_RFA.event  = 0;
        event_ticks         = 0;
    }

    sys_preempt(&sys.task_RFA, event_ticks);
}


#ifndef EXTF_dll_m2advp_refresh
OT_WEAK void dll_m2advp_refresh(void) {
/// <LI> Derive current value for advertising countdown and apply </LI>
/// <LI> Stop the flood if the countdown is shorter than one packet </LI>
/// <LI> The Radio Driver will flood adv packets forever, in parallel
///      with the blocked kernel, until rm2_flood_txstop() is called </LI>
    
    ot_int countdown;
    //static volatile ot_u16 _testflood[500];
    //static volatile ot_u16 _testflood_i = 0;
    
    countdown = rm2_flood_getcounter();
    //_testflood[_testflood_i++] = countdown;
    
    if (countdown < rm2_bgpkt_duration()) {
        dll.counter = (countdown < 0) ? 0 : countdown;
        m2advp_close();
        rm2_flood_txstop();
    }
    else {
        m2advp_update(countdown);
    }
}
#endif



OT_WEAK void dll_rfevt_btx(ot_int flcode, ot_int scratch) {
#if ((M2_FEATURE(SUBCONTROLLER) == ENABLED) || (M2_FEATURE(GATEWAY) == ENABLED))
    ot_bool dirty_adv_check;
    __DEBUG_ERRCODE_EVAL(=130);

    switch (flcode) {
        /// Single-issue BG packet (non-flood) is being TX'ed.
        /// There is nothing to do here because no single-issue BG protocols
        /// are developed yet, but we must skip-over termination.
        case 0: return;

        /// Single-issue BG packet finishes TX'ing.
        /// Like above, nothing to do!  Just break instead of return in order
        /// to do the necessary popping and termination.
        case 1: goto dll_rfevt_btx_KILL;

        /// BG Flood Continues:
        /// ONLY USED RIGHT NOW FOR ADVERTISING
        case 2: {
            __DEBUG_ERRCODE_EVAL(=131);
            dll_m2advp_refresh();
        } return; // skip termination section

        /// Successful exit from BG Flood transmission
        /// ONLY USED RIGHT NOW FOR ADVERTISING
        /// Flood ends & Request Begins
        /// <LI> Pop the flood session, it is no longer needed </LI>
        /// <LI> Tweak the request session to work optimally following flood </LI>
        /// <LI> Pre-empt the kernel to start on the request session.  The
        ///      kernel will clock other tasks over the flood duration.  </LI>
        case 3: {
            m2session* follower;
            // assure request hits NOW & assure it doesn't init dll.comm
            // Tweak dll.comm for request (2 ti is a token, small amount)
            __DEBUG_ERRCODE_EVAL(=132);
            follower                = session_follower();
            follower->counter       = dll.counter;
            sys.task_RFA.event      = 0;
            dll.comm.tc             = 2;    //TI2CLK(2);
            dll.comm.csmaca_params  = (M2_CSMACA_NOCSMA | M2_CSMACA_MACCA);
            session_pop();
        } break;

        /// Error: We need to pop the flooding and then also pop the following
        ///        session.
        default: {
            dll_rfevt_btx_KILL:
            __DEBUG_ERRCODE_EVAL(=133);
            dll_idle();

            // Pop BG session, and do a quick and dirty check to see if it has
            // a follower ... a better method for purging followers is needed.
            dirty_adv_check = (session_netstate() & M2_NETFLAG_STREAM);
            session_pop();// pop the BG session

            // If there is a follower, scrap it.  Scrapping is a process that
            // pops the session and runs its applet one final time.
            if (dirty_adv_check) {
                session_scrap();
            }
        } break;
    }

    DLL_SIG_RFTERMINATE(sys.task_RFA.event, flcode);

    sys.task_RFA.event = 0;
    sys_preempt(&sys.task_RFA, 0);
#endif
}




OT_WEAK void dll_rfevt_ftx(ot_int pcode, ot_int scratch) {
    m2session* active;
    __DEBUG_ERRCODE_EVAL(=140);

    /// Non-final frame TX'ed in multiframe packet
    if (pcode == 1) {
        ///@todo possibly put the queue rearrangement here
    }

    /// Packet TX is done.  Handle this event and pre-empt the kernel.
    /// <LI> Normally, go to response RX.   </LI>
    /// <LI> Allow scheduling of redundant TX on responses, or request with no response. </LI>
    /// <LI> End session if no redundant, and no listening required. </LI>
    else {
        __DEBUG_ERRCODE_EVAL(=141);
        active = session_top();
        scratch = ((active->netstate & M2_NETSTATE_RESPTX) \
        		|| (dll.comm.rx_timeout <= 0));

        /// Send redundant TX immediately, but only if no response window or if
        /// this packet is a response.
        dll.comm.redundants--;
        if ((dll.comm.redundants != 0) && scratch) {
            dll.comm.csmaca_params = (M2_CSMACA_NOCSMA | M2_CSMACA_MACCA);
            rm2_resend( (ot_sig2)&dll_rfevt_txcsma );
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
            active->netstate  &= ~M2_NETSTATE_TMASK;
            active->netstate  |= M2_NETSTATE_RESPRX;
        }

        /// Terminate Transmit state
        DLL_SIG_RFTERMINATE(sys.task_RFA.event, pcode);
        sys.task_RFA.event = 0;
        sys_preempt(&sys.task_RFA, 0);

//        else {
//        	//M2_NETSTATE_SCRAP is represented below
//            active->netstate  |= (ot_u8)(((pcode != 0) | scratch) << 2);
//            active->netstate  &= ~M2_NETSTATE_TMASK;
//            active->netstate  |= M2_NETSTATE_RESPRX;
//
//            DLL_SIG_RFTERMINATE(sys.task_RFA.event, pcode);
//            sys.task_RFA.event = 0;
//        	sys_preempt(&sys.task_RFA, 0);
//        }
    }
}



#endif // !__KERNEL_NONE__





/** Subroutines for DLL configuration and Flow & Congestion Control
  * ============================================================================
  * @todo these two functions and perhaps a few others might get put into the
  *      m2_network section, or some new section.
  */

#ifndef EXTF_dll_quit_rf
OT_WEAK void dll_quit_rf(void) {
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



ot_u8 sub_default_idle(void) {
    //return (dll.netconf.active & (1<<12)) ? M2_DLLIDLE_OFF : M2_DLLIDLE_SLEEP;
    //return ((dll.netconf.active & (1<<12)) == 0);   // compressed form of above
    
    return M2_DLLIDLE_SLEEP;
    
//            
//            
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






void sub_csma_scramble(void) {
/// Sort of optional: Go through the channel list and scramble the channel
/// entries randomly in order to improve band utilization, as multiple devices
/// will scramble the list differently.
    ot_u8 txchans = dll.comm.tx_channels - 1;

    if (txchans) {
        ot_u8       i, j, k, scratch;
        ot_uni16    rot;
        rot.ushort = rand_prn16();

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




CLK_UNIT sub_fcinit(void) {
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
        //random  = TI2CLK(rand_prn16());
        //random %= (dll.comm.tc - TI2CLK(rm2_pkt_duration(&txq)) );
        random  = rand_prn16();
        random %= dll.comm.tc - rm2_pkt_duration(&txq);
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




CLK_UNIT sub_fcloop(void) {
    /// {0,1,2,3} = {RIGD, RAIND, AIND, Default MAC CA}
    /// Default MAC CA just waits Tg before trying again
    if (dll.comm.csmaca_params & 0x20) {    //NO CA
        return phymac[0].tg; //TI2CLK(phymac[0].tg);
    }

    // AIND & RAIND Loop
    if (dll.comm.csmaca_params & 0x18) {    //RAIND, AIND
        return rm2_pkt_duration(&txq); //TI2CLK(rm2_pkt_duration(&txq));
    }

    // RIGD loop
    {   ot_long wait;
        wait    = (dll.comm.tc - dll.comm.tca);
        wait   += sub_rigd_newslot();
        return (wait < 0) ? 0 : (CLK_UNIT)wait;
    }
}



CLK_UNIT sub_rigd_newslot(void) {
/// halve tc from previous value and offset a random within that duration
    dll.comm.tc >>= 1;
    if (dll.comm.tc == 0)
        return 0;
    
    //return (TI2CLK(rand_prn16()) % (CLK_UNIT)dll.comm.tc);
    return (rand_prn16() % (CLK_UNIT)dll.comm.tc);
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

