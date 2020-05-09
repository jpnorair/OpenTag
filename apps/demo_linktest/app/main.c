/* Copyright 2017 JP Norair
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
  */
/**
  * @file       /apps/ht_linktest/app/main.c
  * @author     JP Norair
  * @version    R100
  * @date       8 Oct 2017
  * @brief      HayTag API demo
  *
  * This demo uses 
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>
#include <board.h>
#include <otlib/delay.h>
#include <otlib/rand.h>
#include <otlib/logger.h>

#include <otsys.h>
#include <m2api.h>

#include <stdio.h>


/** Data Mapping <BR>
  * ===========================================================================
  * The Linktest Demo needs a particular data mapping.  It is not unusual, but
  * the demo may not work if the data is not set correctly.  This define below
  * uses the default data mapping (/apps/demo_opmode/code/data_default.c)
  */
#define __DATA_DEFAULT

#define __BEACONS			    100
#define __MIN_BEACON_PERIOD     200
#define __BASE_CHANNEL          0x88
#define __USE_BIDIRECTIONAL     0
#define __DBM_DEFAULT       	20


/** Main Static Variables <BR>
  * ========================================================================<BR>
  */
typedef struct {
    ot_bool is_gateway;
} setup_struct;

static setup_struct setup;




/** Ping Static Variables <BR>
  * ========================================================================<BR>
  */
typedef struct {
	ot_u8	channel;
	ot_u8	reserved;
	ot_u16	beacons;
	ot_u16  interval;
    ot_u16  hits;
    ot_u16  last_hits;
    ot_u16  corrections;
    ot_long	rssi;
    ot_long link;
    ot_app  applet;
} lt_struct;

static lt_struct linktest;





/** Local Functions & definitions
  * ========================================================================
  */
#define LINKTEST_TASK    (&sys.task[TASK_linktest])

// Main Application Functions
void setup_button_init();                /// board/platform dependent
void setup_init();

// Ping Functions
void linktest_invoke(ot_u8 source);
void linktest_query_applet(m2session* active);
void linktest_ping(m2session* active);
void linktest_ping_done(m2session* active);

/** Button Features : semi-platform dependent <BR>
  * ========================================================================<BR>
  * This Application can be initialized by a button-press or by an ALP command.
  * The ALP command is treated at a higher-level (see ALP callback later).
  * The button-press is unique to this application, and it is treated here.
  */

void OT_SWITCH1_ISR(void) {
/// This switch has falling edge and rising edge triggers.
    ot_u8 edge;
    
    // If switch is pressed = first edge   = do data-rate select
    // If switch is raised  = second edge  = start linktest
    if ((OT_SWITCH1_PORT->IDR & OT_SWITCH1_PIN) == (OT_SWITCH1_POLARITY << OT_SWITCH1_PIN)) {
        indicator_on(INDICATOR_3);
        edge = 0;
    }
    else {
        indicator_off(INDICATOR_3);
        edge = 1;
    }
    
    // Invoke function includes blocks to prevent repetitive calls
    linktest_invoke(edge);
}

void setup_button_init() {
/// ARM Cortex M boards must prepare all EXTI line interrupts in their board
/// configuration files, but the actual line interrupt must be enabled here.
    EXTI->RTSR |= OT_SWITCH1_PIN;
    EXTI->FTSR |= OT_SWITCH1_PIN;
    EXTI->IMR  |= OT_SWITCH1_PIN;
}




/** Beacon on, off, config functions <BR>
  * ========================================================================<BR>
  */

ot_int beacons_off() {
//	dll_silence(True);
//	return 0;

	vlFILE* fp;
	ot_int output;

	fp = ISF_open_su(6);
	if (fp != NULL) {
		vl_store(fp, 0, NULL);
		vl_close(fp);
		dll_refresh();
		output = 0;
	}
	else {
	    output = -1;
	}

	return output;
}



void app_set_txpwr(ot_int dBm) {
    vlFILE* fp;
    ot_uni16 pwr_flt;

    fp = ISF_open_su(0x02);
    if (fp != NULL) {
        ot_uint offset;

        // Replace existing power levels for each of three channel groups
        // with new power spec.
        offset              = 6+2;
        pwr_flt.ushort      = vl_read(fp, offset);
        pwr_flt.ubyte[0]    = (ot_u8)( (dBm + 40) * 2 );
        vl_write(fp, offset, pwr_flt.ushort);

        offset              = 6+6+2;
        pwr_flt.ushort      = vl_read(fp, offset);
        pwr_flt.ubyte[0]    = (ot_u8)( (dBm + 40) * 2 );
        vl_write(fp, offset, pwr_flt.ushort);

        offset              = 6+6+6+2;
        pwr_flt.ushort      = vl_read(fp, offset);
        pwr_flt.ubyte[0]    = (ot_u8)( (dBm + 40) * 2 );
        vl_write(fp, offset, pwr_flt.ushort);

        vl_close(fp);

        // This block of code resets the channel status, forcing the
        // radio module to update the channel information.
        // It's a hack, but this whole app is a hack.
        //phymac[0].channel   = 0xF0;
        //phymac[0].tx_eirp   = 0x7F;
        rm2_channel_refresh();
    }

}



/** M2 Transport Callback <BR>
  * ========================================================================<BR>
  * This is a lower-level callback than the application layer callback.  It
  * should probably be merged into ALP somehow.
  *
  * In any case, in this implementation we filter for the following
  * - announcement requests
  */
//ot_bool m2qp_sig_isf(ot_u8 type, ot_u8 opcode, id_tmpl* user_id) {
//
//    return True;
//}






/** ALP Processor Callback for Starting a Ping <BR>
  * ========================================================================<BR>
  * "ALP" is the NDEF-based set of low-level API protocols that OpenTag uses.
  * ALP messages can come-in over any communication method: wire, wireless,
  * telepathy... anything that can transfer a packet payload.
  *
  * Some ALPs are standardized. Those get handled by OTlib automatically.  ALPs
  * that are not recognized are sent to this function to be handled.  In this
  * demo, we are using a very simple ALP, shown below:
  *
  * ALP Payload Length:         0
  * ALP Protocol ID:            255 (FF)
  * ALP Protocol Commands:      ignored
  *
  * The "user_id" parameter corresponds to the Device ID that sent this ALP.
  *
  * A quickstart guide to the ALP API is available on the Indigresso Wiki.
  * http://www.indigresso.com/wiki/doku.php?id=opentag:api:quickstart
  */

///@todo change task into logger task, and make the Pingpong part entirely
///      session driven.
ot_bool alp_ext_proc(alp_tmpl* alp, id_tmpl* user_id) {
    ot_u8 flags, payload_len, alp_id, alp_cmd;
    
    flags       = q_readbyte(alp->inq);
    payload_len = q_readbyte(alp->inq);
    alp_id      = q_readbyte(alp->inq);
    alp_cmd     = q_readbyte(alp->inq);
    
    /// Offset=2 is the ALP ID, which defines the protocol to use.
    /// This Project has two apps (similar to inbound ports): 
    switch (alp_id) {

		// This app is completely atomic and it manages the ALP data in place rather
		// than through a subordinate app queue.
        ///@todo this should be updated for gateway to send a command to endpoint
		case 255: {
			ot_u16 beacons;
			ot_u16 interval;
			
			///@todo Put beacons, control, and channel parameters into invocation
			beacons = q_readshort_be(alp->inq);
			interval = q_readshort_be(alp->inq);

			// cmd == 0: this will initialize a linktest from console, will be ignored otherwise
			switch (alp_cmd) {
			case 0: if (alp->inq == mpipe) {
			            break;
			        }
			case 1: linktest.beacons    = beacons;
			        linktest.interval   = interval;
			        linktest_invoke(2);
			}
			
			// Start linktest if cmd==0
			// - Endpoints will begin transmitting linktest
			// - Gateways will send a request to start communicating
			if (alp_cmd == 0) {
			    
			}

			// If cmd is non-zero (1) this is a ping.
			// update the parameters accordingly
			///@todo additional logic to compare "beacons" value to last received beacon.
			else {
				linktest.hits++;
				linktest.rssi          += radio.last_rssi;
				linktest.link          += radio.last_linkloss;
				linktest.corrections   += radio.link.corrections;
			}
			
		} break;

		/// Default case is mainly in case you want to trap other ALPs
		default: {
			if (setup.is_gateway) {
				ot_int len;
				logger_header(MSG_raw, 0);
				q_writestring(mpipe.alp.outq, (ot_u8*)"ALP", 4);
				len = q_span(alp->inq);
				q_writestring(mpipe.alp.outq, q_markbyte(alp->inq, len), len);
				logger_direct();
			}
		} break;
    
    }

    return True;
}






/** Linktest Kernel Task <BR>
  * =======================================================================<BR>
  * This function will be activated by the kernel when the external task is
  * active and there are resources available to run the task.  This task will
  * just activate the DLL session (also a task) and then turn itself off.
  * Then, the kernel will call the DLL session task as soon as necessary
  * resources are available to run that task (by default DLL task is top
  * priority, so it should get called right away).
  */
void linktest_systask(void* arg) {
    ot_task task = (ot_task)arg;

    switch (task->event) {
    
    // Constructor Destructor: do nothing.
    case 0: break;
    
    // State 1: this is a wait-state for debouncing.  It falls through.
    case 1: task->event = 2;
    
    // State 2: select a data rate
    // This state will be switched externally when button is raised
    case 2: {
        ot_u8 current_cursor = task->cursor++;
        switch (current_cursor) {
        
        // One second pause
        default: task->cursor = 1;
        case 0: sys_task_setnext(task, 512);
                break;
        
        // One blink: data rate 1 (minimum)
        case 1: indicator_on(INDICATOR_4);
                sys_task_setnext(task, 128);
                break;
        case 2: indicator_off(INDICATOR_4);
                sys_task_setnext(task, 128+2048);
                break;
        
        // Two blinks: data rate 2 (medium)
        case 3: 
        case 4:
        case 5: if (current_cursor & 1) indicator_on(INDICATOR_4);
                else                    indicator_off(INDICATOR_4);
                sys_task_setnext(task, 128);
                break;
        
        case 6: indicator_off(INDICATOR_4);
                sys_task_setnext(task, 128+2048);
                break;
                
        // Two blinks: data rate 3 (maximum)
        case 7: 
        case 8:
        case 9: 
        case 10:
        case 11: if (current_cursor & 1) indicator_on(INDICATOR_4);
                else                    indicator_off(INDICATOR_4);
                sys_task_setnext(task, 128);
                break;
        
        case 12: indicator_off(INDICATOR_4);
                sys_task_setnext(task, 128+2048);
                break;
        }
        
    } break;

    
    // State 3: set link parameters based on trapped cursor value
    // - 1. universal parameters (clear old settings)
    // - 2. set the variable parameters
    // - 3. start the communication session.  **For gateways, this is just to listen.**
    ///@todo here we should change gateway behavior to send a start request
    case 3: {
        linktest.hits           = 0;
        linktest.rssi           = 0;
        linktest.link           = 0;
        linktest.corrections    = 0;
        linktest.beacons        = __BEACONS;
        linktest.interval       = __MIN_BEACON_PERIOD;  //default: set again on linktest start

        if (task->cursor > 12) {
            linktest.channel = __BASE_CHANNEL | (2<<4);
        }
        else if (task->cursor > 6) {
            linktest.channel = __BASE_CHANNEL | (1<<4);
        }
        else if (task->cursor > 2) {
            linktest.channel = __BASE_CHANNEL | (0<<4);
        }
        
        // Change listening channel
        {   vlFILE* fp;
            ot_u8 ch_scan[4] = { 0, 0x43, 0, 0 };
            ch_scan[0] = linktest.channel;
            fp = ISF_open_su(5);
            if (fp != NULL) {
                vl_store(fp, 4, ch_scan);
                vl_close(fp);
                dll_refresh();
            }
        }
        
        // If Gateway, send request that starts linktest from endpoint
        // If Tag, start linktest and go into timeout state (5) as watchdog.
        if (setup.is_gateway) {
            ///@todo spawn a session here
            task->event     = 0;
            task->cursor    = 0;
        }
        else {
            session_tmpl    s_tmpl;
            ot_u32          est_pkt_ti;
            ot_u32          timeout;
            
            est_pkt_ti = rm2_scale_codec(linktest.channel, 64);
            if (est_pkt_ti > (__MIN_BEACON_PERIOD/3)) {
                linktest.interval = est_pkt_ti*2;
            }
            
            s_tmpl.channel      = linktest.channel;
            s_tmpl.flagmask     = 0;
            s_tmpl.subnetmask   = 0;
            m2task_immediate(&s_tmpl, &linktest_query_applet);
            
            timeout         = linktest.beacons * (linktest.interval + est_pkt_ti);
            timeout        += (timeout >> 2);
            task->event     = 5;
            task->cursor    = 0;
            sys_task_setnext(task, timeout);
        }
    } break;

    /// This state is invoked on devices receiving linktest (generally gateway).
    /// It cycles each interval.  If no new pings received after 5 tries, it
    /// terminates the linktest RX.
    case 4: {
        indicator_off(INDICATOR_4);
        if (linktest.hits == linktest.last_hits) {
            task->cursor++;
        }
        else {
            indicator_on(INDICATOR_4);
            linktest.last_hits  = linktest.hits;
            task->cursor        = 0;
        }
        if (task->cursor < 5) {
            sys_task_setnext(task, linktest.interval*2);
            break;
        }
    } // fall through on timeout.

    /// This is the timeout state
    /// If gateway, print results.
    case 5: {
    	if (setup.is_gateway) {
    		// Prepare logging header: UTF8 (text log), dummy length is 0
    		logger_header(DATA_utf8, 0);

    		// Print out the linktest parameters
    		mpipe.alp.outq->putcursor += sprintf(
    				mpipe.alp.outq->putcursor,
    				"Beacons Received: %d\nMean RSSI: %d\nMean Link: %d\nMean Corrections: %d\n",
    		        linktest.hits,
					linktest.rssi/linktest.hits,
					linktest.link/linktest.hits,
					linktest.corrections/linktest.hits
				);

    		logger_direct();
    	}
    } // fall through to idle after this state
    
    /// There is only one usage of this app, with no init
    /// Use any non-zero, non-one state to block the button press.
    default: 
        task->cursor = 0; 
        task->event = 0;
        break;
    }
}










/** Communication Task Applets  <BR>
  * ========================================================================<BR>
  * Communication tasks in OpenTag are typically created by one of the OTAPI
  * Tasker functions in /otlib/OTAPI_tasker.h.  The task is created here, and
  * when it gets activated by the kernel, the applet that is attached to it
  * will run.  The main job of the applet is to load prepare the communication.
  * Usually, this means loading a request packet, but you could also make an
  * applet that does any manner of state-based communication routines.  The
  * Adaptive Search and CoAP demos are examples of sophisticated applets.
  */


void linktest_build_command(ot_u8 lt_cmd, ot_u16 pings_remaining, ot_u16 response_duration) {
/// The C-API for building commands can be bypassed in favor of directly
/// putting data to the queue.  That way is more efficient, but it also requires
/// you to know more about DASH7 than just what order the templates should be.
///
/// The query that we build will collect sensor configuration data back from
/// all devices that support the sensor protocol.  Much more interesting queries
/// are possible.
	ot_u8 status;

	{ //open request for single hop anycast query
	        routing_tmpl routing;
	        routing.hop_code = 0;
	        otapi_open_request(ADDR_broadcast, &routing);
	    }

	{ //use a command template for collection of single file from single file search
		command_tmpl command;
		command.opcode      = (ot_u8)CMD_udp_on_file;
		command.type        = (ot_u8)CMDTYPE_acast_request;
		//command.extension   = (ot_u8)CMDEXT_none;
		command.extension   = (ot_u8)CMDEXT_no_response;
		//command.extension   = (lt_cmd == 0) ? (ot_u8)CMDEXT_no_response : (ot_u8)CMDEXT_none;
		otapi_put_command_tmpl(&status, &command);
	}

    {   // write the dialog information (timeout, channels to use)
        // 0x23 for timeout code = 64 ticks = 60 ms
        dialog_tmpl dialog;
        dialog.channels = 0;    //use same channel as request for response
        //dialog.timeout  = 0x23;  
        dialog.timeout = otutils_encode_timeout(response_duration);
        otapi_put_dialog_tmpl(&status, &dialog);
    }

	{ //write the query to search for the sensor protocol id
		static const ot_u8 query_str[12] = "APP=Linktest";
		query_tmpl query;
		query.code      = M2QC_COR_SEARCH + 12; // do a 100% length=12 correlation search
		query.mask      = NULL;                 // don't do any masking (no partial matching)
		query.length    = 12;                   // query_str is 12 bytes
		query.value     = (ot_u8*)query_str;
		otapi_put_query_tmpl(&status, &query);
	}
	{ //put in the information of the file to search (the user id)
		isfcomp_tmpl isfcomp;
		isfcomp.is_series   = False;
		isfcomp.isf_id      = ISF_ID(user_id);
		isfcomp.offset      = 0;
		otapi_put_isf_comp(&status, &isfcomp);
	}
	{ //UDP data is number of beacons to run (e.g. 100)
		udp_tmpl udp;
		ot_uni32 data;
		data.ushort[0]     = pings_remaining;
		data.ushort[1]     = response_duration;
		udp.data_length    = 4;
		udp.dst_port       = 255;
		udp.src_port       = lt_cmd;
		udp.data           = &data.ubyte[0];

		otapi_put_udp_tmpl(&status, &udp);
	}

	//Done building command, close the request and send the dialog
	///@todo this should return an ID or hash of some kind
	otapi_close_request();
	
	/// Provides the CSMA limit
	otapi_start_dialog(1024);
	
	///@note - this session is failing on CSMA stage
	///      - never gets past CCA1.
	///      - make sure interrupts are working.
}



void linktest_query_applet(m2session* active) {
	linktest_build_command(0, linktest.beacons, linktest.interval);
	
	if (setup.is_gateway == False) {
		session_new(&linktest_ping, 0, linktest.channel, M2_NETSTATE_INIT|M2_NETSTATE_REQTX);
	}
}


void linktest_ping(m2session* active) {
/// Build a linktest ping message.  Continue pinging as long as there are pings
/// left to send.
	linktest_build_command(1, --linktest.beacons, linktest.interval);

	if (linktest.beacons == 0) {
	    sys_task_setevent(LINKTEST_TASK, 5);
	    sys_preempt(LINKTEST_TASK, 0);
	}
	else if (setup.is_gateway == False) {
	    session_new(&linktest_ping, 0, linktest.channel, M2_NETSTATE_INIT|M2_NETSTATE_REQTX);
	}
}






/** Main Configuration <BR>
  * ==================================================================<BR>
  * This function should run after platform_poweron() but before
  * platform_init_OT().  Here you can configure or do things before 
  * any of the kernel & system features are initialized.  It's a good
  * place to put features that control which drivers or apps get 
  * activated.
  */
  
void setup_init() {
/// 1. Determine if device is gateway based on initial value of button, at startup.
///    @todo it would be better to simply monitor the initial value of UART lines,
///    to see if they are connected, but this cannot be so easily guaranteed.
    
    // Set it up as gateway
    setup.is_gateway = 1;
    setup.is_gateway = (ot_bool)((OT_SWITCH1_PORT->IDR & OT_SWITCH1_PIN) == (OT_SWITCH1_POLARITY << OT_SWITCH1_PIN));
    
    /// Blink the board LEDs to show that it is starting up.
    {   ot_u8 i;

        i=4;
        while (i != 0) {
            if (i&1)    BOARD_led1_on();
            else        BOARD_led2_on();

            delay_ti(60);
            BOARD_led2_off();
            BOARD_led1_off();
            i--;
        }
    }
    
    // BusyWait for button to be released.
    while ((OT_SWITCH1_PORT->IDR & OT_SWITCH1_PIN) == (BOARD_SW1_POLARITY << OT_SWITCH1_PIN));
    
    // Configure the button, which will now generate an interrupt when pushed.
    setup_button_init();
}





void linktest_invoke(ot_u8 source) {
/// The "External Task" is the place where the kernel runs the main user app.
/// Our app has 4 functions (call types).
/// <LI> The task event state is set to 1.  Event 0 is always Task-off, but
///      otherwise each task manages its own event numbers </LI>
/// <LI> We store the channel in the cursor, which is not being used otherwise.
///      The Task "cursor" can be used for additional state control, beyond the
///      event number. </LI>
/// <LI> We give it a runtime reservation of 1 tick (it runs pretty fast).
///      This is also short enough to pre-empt RX listening, but not RX data.
///      Try changing to a higher number, and observing how the kernel
///      manages this task. </LI>
/// <LI> We give it a latency of 255.  Latency is unimportant for run-once
///      tasks, so giving it the max latency will prevent it from blocking
///      any other tasks. </LI>
/// <LI> We tell it to start ASAP (preemption delay parameter = 0) </LI>
///
/// @note The latency parameter is mostly useful for protocol management,
/// for which you probably want to enforce a request-response turnaround time.
/// for processing and for iterative tasks it is not important: set to 255.
///
    ot_u8 nextevent;
    
    // source should be 0, 1, 2.
	if (source > 2) {
		source = 2;
	}

	// if source is 0 (setup) the task must be idle in order to do anything
	switch(source) {
	case 0: if ((LINKTEST_TASK->event == 0) || setup.is_gateway) {
                sys_task_setcursor(LINKTEST_TASK, 0);
                nextevent = 1;
                break;
	        } return;
    
	case 1: if (LINKTEST_TASK->event == 2) {
	            nextevent = 3;
	            break;
	        } return;
	        
	case 2: if (LINKTEST_TASK->event == 0) {
                nextevent = 4;
                break;
	        }
	
	default: return;
	}

    sys_task_setevent(LINKTEST_TASK, nextevent);
    sys_task_setreserve(LINKTEST_TASK, 1);
    sys_task_setlatency(LINKTEST_TASK, 255);
    sys_preempt(LINKTEST_TASK, 0);
}





void main(void) {
    ///1. Standard Power-on routine (Clocks, Timers, IRQ's, etc)
    ///2. Pre-OT initialization
    ///3. Standard OpenTag Init (most stuff actually will not be used)
    platform_poweron();
    setup_init();
    platform_init_OT();

    ///4. Set the app name (ht_linktest) as a cookie in the User-ID.
	///   This is used for query filtering
	{   static const ot_u8 appstr[] = "APP=Linktest";
		vlFILE* fp;
		fp = ISF_open_su(ISF_ID(user_id));
		if (fp != NULL) {
			vl_store(fp, sizeof(appstr), appstr);
		}
		vl_close(fp);
	}

    ///5. Default state: no beacons!
    beacons_off();
    app_set_txpwr(__DBM_DEFAULT);

    ///6a. The device will wait (and block anything else) until you connect
    ///    it to a valid console app.
    ///6b. Load a message to show that main startup has passed
#   if (OT_FEATURE(MPIPE))
    if (setup.is_gateway) {
        mpipedrv_standby();
        logger_msg(MSG_utf8, 6, 27, (ot_u8*)"SYS_ON", (ot_u8*)"System on and Mpipe active\n");
    }
#   endif

    ///7. MAIN RUNTIME (post-init)  <BR>
    ///<LI> Use a main loop with platform_ot_run(), and nothing more. </LI>
    ///<LI> You could put code before or after platform_ot_run, which will
    ///     run before or after the (task + kernel).  If you do, keep the code
    ///     very short or else you are risking timing glitches.</LI>
    ///<LI> To run any significant amount of user code, use tasks. </LI>
    while(1) {
        platform_ot_run();
    }
}




#if defined(__DATA_DEFAULT)
#   include <../_common/fs_default_startup.c>
#else
#   error "There is no data mapping specified.  Put one here."
#endif

