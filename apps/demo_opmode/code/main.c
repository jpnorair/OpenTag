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
  * @file       /apps/demo_opmode/code/main.c
  * @author     JP Norair
  * @version    V1.0
  * @date       10 Oct 2012
  * @brief      Opmode Switching Demo
  *
  * Functional purpose of this demo:
  * <LI> Show differences in Gateway and Endpoint idle-time behavior </LI>
  * <LI> Switching of Operational Modes during runtime </LI>
  * <LI> Sending commands and enacting mode changes by button-press or by
  *      reception of a custom MPipe-ALP protocol that instructs OpenTag
  *      to send such commands or enact such mode-changes </LI>
  *
  * OpenTag programming examples in this demo:
  * <LI> How to put your application code into a task that gets invoked by two
  *      means: 1. Asynchronous event (button press), 2. Another task (the
  *      MPipe task, via a custom ALP command). </LI>
  * <LI> Defining a custom ALP protocol using ID=0x90, 0<=CMD<=3, and putting
  *      the custom protocol code in static callback otapi_alpext_proc() </LI>
  * <LI> Basic usage of OTAPI for logging and DASH7 dialog creation </LI>
  * <LI> Basic structuring of an OpenTag-based App & main.c </LI>
  *
  * Elements of the Demo you can customize easily (or relatively easily):
  * <LI> Easy: attach different applets to the signals by changing the applet
  *      C files in your makefile (or IDE project, in "applets" folder) </LI>
  * <LI> Pretty Easy: Change the ISF file that gets used by query or beacon,
  *      command by altering applet_send_query() or applet_send_beacon() </LI>
  * <LI> Medium: Change ISF parameters (in files 2-5) from data_default.c
  *      in order to alter the behavior of Gateway and Endpoint modes. </LI>
  *
  * This Application Recommends:
  * <LI> CC430/MSP430F5: ~24KB Flash, ~1.5KB SRAM                       </LI>
  * <LI> STM32: ~32KB Flash, ~2KB SRAM                                  </LI>
  * <LI> Minimum Two LEDs to show RX/TX activity                        </LI>
  * <LI> Two additional outputs for the application (optionally, LEDs)  </LI>
  * <LI> One input source for mode selection (optionally, a button)     </LI>
  * <LI> Two input sources for command activations                      </LI>
  *
  * Supported Boards & Platforms: <BR>
  * See /apps/demo_opmode/code/platform_config.h for board & platform support.
  * @note Different boards may support different methods of input and output.
  *
  * Stock Applets: <BR>
  * Many of the stock applets from /otlibext/stdapplets/ are used as signal
  * callbacks from the protocol layers.  You can look at extf_config.h to see
  * which ones are enabled (or, to specify new ones).
  *
  ******************************************************************************
  */

#include "OTAPI.h"
#include "OT_platform.h"




/** Data Mapping <BR>
  * ===========================================================================
  * The Opmode Demo needs a particular data mapping.  It is not unusual, but 
  * the demo may not work if the data is not set correctly.  This define below
  * uses the default data mapping (/apps/demo_opmode/code/data_default.c)
  */
#define __DATA_DEFAULT






/** Application Global Variables <BR>
  * ========================================================================<BR>
  * It is safe to either keep opmode_devicemode volatile, since it is set in an
  * interrupt service routine.
  */
#ifdef __BIG_ENDIAN__
#   define SYSMODE_OFF         0x0000
#   define SYSMODE_MASK        0x0F00
#   define SYSMODE_GATEWAY     0x0800
#   define SYSMODE_ENDPOINT    0x0200
#else
#   define SYSMODE_OFF         0x0000
#   define SYSMODE_MASK        0x000F
#   define SYSMODE_GATEWAY     0x0008
#   define SYSMODE_ENDPOINT    0x0002
#endif

volatile ot_u16 opmode_devicemode;
//ot_bool (*app_task)(void);





/** Applet Functions (platform & board independent) <BR>
  * ========================================================================
  */
// Main Application Functions
void app_init();
void app_invoke(ot_u8 call_type);

/// Communication Task Applets
void applet_send_query(m2session* session);
void applet_send_beacon(m2session* session);

/// Direct Control Applets
void opmode_goto_gateway();
void opmode_goto_endpoint();

// Applets that run due to OpenTag callbacks
//void    app_packet_routing(ot_int code, ot_int type);



/** Application local subroutines (platform & board dependent) <BR>
  * ============================================================================
  */
void sub_button_init();
void sub_trig_init();
void sub_trig3_high();
void sub_trig3_low();
void sub_trig3_toggle();
void sub_trig4_high();
void sub_trig4_low();
void sub_trig4_toggle();






#if defined(BOARD_MLX73Proto_E)
#   include "support/main_inc_MLX73Proto_E.c"

#elif (defined(BOARD_AG430DK_GW1) || defined(BOARD_AG430DK_EP1))
#   include "support/main_inc_AG430DK.c"

#elif defined(BOARD_EM430RF)
#   include "support/main_inc_EM430RF.c"

#elif defined(BOARD_eZ430Chronos)
#   include "support/main_inc_ez430chronos.c"

#elif defined(BOARD_STM32H152)
#   include "support/main_inc_STM32H152.c"

#else
#   error "You have not defined a supported board: select one in build_config.h"

#endif









/** User Applet and Button Management Routines <BR>
  * ========================================================================<BR>
  * The User applet is primarily activated by callbacks from the kernel.  
  * However, in this system some features are also activated by button presses.
  */
static const ot_sub led_on[4] = {   &sub_trig4_high, 
                                    &sub_trig3_high,
                                    &platform_trig2_high,
                                    &platform_trig1_high    };

static const ot_sub led_off[4] = {  &sub_trig4_low, 
                                    &sub_trig3_low,
                                    &platform_trig2_low,
                                    &platform_trig1_low     };

void sub_led_cycle(ot_u8 i) {
    led_on[i&3]();
    platform_block(600);	//600 sti ~= 18.3ms
    led_off[i&3]();
}



void app_init() {
    ot_u8 i;

    ///Default Startup Mode: Gateway.  The button init may also alter this.
    opmode_devicemode = SYSMODE_GATEWAY;

    ///Initialize Application Triggers (LEDs) and blink the LEDs
    ///(Go left-to-right then right-to-left, like on an old-timey Cylon helmet).
    sub_trig_init();

    i = 255;
    do { sub_led_cycle(++i); } while (i != 3);
    do { sub_led_cycle(--i); } while (i != 0);

    ///Initialize the input buttons
    sub_button_init();
    //app_trig4_high();
}



void app_invoke(ot_u8 call_type) {
/// The "External Task" is the place where the kernel runs the main user app.
/// Our app has 4 functions (call types).
/// <LI> We give it a runtime reservation of 1 tick (it runs pretty fast).
///      This is also short enough to pre-empt RX listening, but not RX data.
///      Try changing to a higher number, and observing how the kernel
///      manages this task. </LI>
/// <LI> We give it a latency of 255.  Latency is unimportant for run-once
///      tasks, so giving it the max latency will prevent it from blocking
///      any other tasks. </LI>
/// <LI> We tell it to start ASAP (next = 0) </LI>
///
/// @note The latency parameter is mostly useful for protocol management,
/// for which you probably want to enforce a request-response turnaround time.
/// for processing and basic, iterative tasks it is not important: set to 255.
///
	sys_task_setevent(TASK_external, call_type);
    sys_task_setreserve(TASK_external, 1);
    sys_task_setlatency(TASK_external, 255);
    sys_preempt(&sys.task[TASK_external], 0);
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

void applet_send_query(m2session* session) {
/// The C-API for building commands can be bypassed in favor of directly
/// putting data to the queue.  That way is more efficient, but it also requires
/// you to know more about DASH7 that just what order the templates should be.

/// The query that we build will collect sensor configuration data back from
/// all devices that support the sensor protocol.  Much more interesting queries
/// are possible.

    { //open request for single hop anycast query
        routing_tmpl routing;
        routing.hop_code = 0;
        otapi_open_request(ADDR_anycast, &routing);
    }
    { //use a command template for collection of single file from single file search
        ot_u8 status;
        command_tmpl command;
        command.opcode      = (ot_u8)CMD_collect_file_on_file;
        command.type        = (ot_u8)CMDTYPE_na2p_request;
        command.extension   = (ot_u8)CMDEXT_none;
        otapi_put_command_tmpl(&status, &command);
    }
    { //write the dialog information (timeout, channels to use)
        ot_u8 status;
        dialog_tmpl dialog;
        dialog.channels = 0;    //use same channel as request for response
        dialog.timeout  = 128;  //128 tick response timeout (1 tick = 0,977 ms)
        otapi_put_dialog_tmpl(&status, &dialog);
    }
    { //write the query to search for the sensor protocol id
        query_tmpl query;
        ot_u8 status;
        ot_u8 protocol_id;
        protocol_id     = 0x02;                 // sensor protocol id = 0x02
        query.code      = M2QC_COR_SEARCH | 1;  // do a 100% length=1 correlation search
        query.mask      = NULL;                 // don't do any masking (no partial matching)
        query.length    = 1;                    // look for one byte (0x02)
        query.value     = &protocol_id;         // (query.value is a string)
        otapi_put_query_tmpl(&status, &query);
    }
    { //put in the information of the file to search (the protocol list)
        ot_u8 status;
        isfcomp_tmpl isfcomp;
        isfcomp.is_series   = False;
        isfcomp.isf_id      = ISF_ID(protocol_list);
        isfcomp.offset      = 0;
        otapi_put_isf_comp(&status, &isfcomp);
    }
    { //put in the information of the file to return (the sensor list)
        ot_u8 status;
        isfcall_tmpl isfcall;
        isfcall.is_series   = False;
        isfcall.isf_id      = ISF_ID(sensor_list);
        isfcall.max_return  = 32;
        isfcall.offset      = 0;
        otapi_put_isf_call(&status, &isfcall);
    }

    //Done building command, close the request and send the dialog
    otapi_close_request();
}



void applet_send_beacon(m2session* session) {
    { //open request for broadcast
        otapi_open_request(ADDR_broadcast, NULL);
    }
    { //use a command template for collection of single file from single file search
        ot_u8 status;
        command_tmpl command;
        command.opcode      = (ot_u8)CMD_announce_file;
        command.type        = (ot_u8)CMDTYPE_na2p_request;
        command.extension   = (ot_u8)CMDEXT_no_response;
        otapi_put_command_tmpl(&status, &command);
    }
    { //write the dialog information (timeout, channels to use)
        ot_u8 status;
        dialog_tmpl dialog;
        dialog.channels = 0;    //use same channel as request for response
        dialog.timeout  = 0;    //0 tick response timeout (1 tick = 0,977 ms)
        otapi_put_dialog_tmpl(&status, &dialog);
    }
    { //write the ISF return data (note: use isfcall template)
        ot_u8 status;
        isfcall_tmpl isfcall;
        isfcall.is_series   = False;
        isfcall.isf_id      = ISF_ID(network_settings);
        isfcall.max_return  = ISF_LEN(network_settings);
        isfcall.offset      = 0;
        otapi_put_isf_return(&status, &isfcall);
    }

    //Done building command, close the request and send the dialog
    otapi_close_request();
}






/** Direct Control Applets  <BR>
  * ========================================================================<BR>
  * The applets below do not do any communication.  They can run directly from
  * the User Task without creating a communication task (session).
  */

void opmode_goto_gateway() {
    if (opmode_devicemode != SYSMODE_GATEWAY) {
        //vlFILE* fp;
    	opmode_devicemode = SYSMODE_GATEWAY;

        /// Change the Beacon Period to a higher value (slower)
        /// The setting is 0x1000 = 4s
        /// NOTE: the filesystem is purely big-endian, so data must be flipped
        //fp = ISF_open_su(ISF_ID(beacon_transmit_sequence);
        //vl_write(fp, 6, 0x0010);
        //vl_close(fp);

        dll_change_settings(SYSMODE_MASK, SYSMODE_GATEWAY);
        sub_trig4_high();
    }
}


void opmode_goto_endpoint() {
    if (opmode_devicemode != SYSMODE_ENDPOINT) {
        //vlFILE* fp;
    	opmode_devicemode = SYSMODE_ENDPOINT;

        /// Change the Beacon Period to a lower value (faster)
        /// The setting is 0x0200 = 500ms
        /// NOTE: the filesystem is purely big-endian, so data must be flipped
        //fp = ISF_open_su(ISF_ID(beacon_transmit_sequence);
        //vl_write(fp, 6, 0x0002);
        //vl_close(fp);

        /// Use the built-in system function to change modes and restart
        dll_change_settings(SYSMODE_MASK, SYSMODE_ENDPOINT);
        sub_trig4_low();
    }
}





/** OTlib/OTAPI Callback Applets  <BR>
  * ========================================================================<BR>
  * This app uses some of the "std" applets from /otlibext/applets_std
  * The applets used are selected in extf_config.h
  */






/** User ALP Processor  <BR>
  * =======================================================================<BR>
  * If using proprietary/custom ALPs, the processor should be implemented here.
  * There is an example implementation below, which can be uncommented to match
  * the example in the API Quickstart Guide:
  * http://www.indigresso.com/wiki/doku.php?id=opentag:api:quickstart
  */ 

void otapi_alpext_proc(alp_tmpl* alp, id_tmpl* user_id) {
/// For this example, the directive ID is 0x90 and the commands are 0-3.  You
/// can change these values simply by changing the implementation of this 
/// function.
///
/// Alternatively, instead of using the User Task (extprocess) you can use the
/// sys.loadapi link to an app function: sys.loadapi = &opmode_goto_gateway;
/// This is a simpler approach, but it can be blocked eternally in some setups.
/// Using the User Task guarantees that your app/applet will run at some point
/// in the future, as soon as the kernel is done with high-priority I/O tasks.

    if (alp->inrec.id == 0x90) {
    	ot_u8 task_cmd = (alp->inrec.cmd & 0x7F);

    	// Enable our command processing user task (task 0) to run the command
    	// routine, but only if the cmd is known (0<=cmd<=3)
    	if (task_cmd > 3)   task_cmd = 0;
    	else                app_invoke(++task_cmd);

        // Write back success (non-zero).
        alp_load_retval(alp, task_cmd);
    }
}





/** User Task for ALP command processing  <BR>
  * =======================================================================<BR>
  * You can create a user task that does various different things -- basically
  * it is just a function that does whatever you want.  This task is enabled
  * when a valid "Opmode" ALP is received (see otapi_alpext_proc() above),
  * and it runs the command routine specified by the ALP CMD value that was
  * received and parsed by otapi_alpext_proc().
  *
  * It is important to use a task to do the work in cases where "the work"
  * is involving the OpenTag core.  Otherwise, you are in danger of
  * interrupting critical processes managed by the kernel.  In order to keep
  * OpenTag lightweight, there are not formal mutexes that protect system
  * resources.
  *
  * If "the work" has nothing to do with the OpenTag core, then you don't
  * need a task.  For example, you could just run the code from
  * otapi_alpext_proc().  The Built-in Filedata ALP is an example of an ALP
  * that doesn't impact the core, and therefore is runs directly from ALP.
  */

void ext_systask(ot_task task) {
    static const char msg0[] = "Sending Beacon";
    static const char msg1[] = "Sending Query ";
    static const char msg2[] = "Go to Gateway ";
    static const char msg3[] = "Go to Endpoint";
    static const char *msglist[] = { msg0, msg1, msg2, msg3 };

	ot_int app_select;
	session_tmpl s_tmpl;

	// Disable this task after running, by setting event to 0
	app_select  = task->event - 1;
	task->event = 0;

	// Log a message.  It is scheduled, and the RF task has higher priority,
	// so if you are sending a DASH7 dialog this log message will usually
	// come-out after the dialog finishes.
	otapi_log_msg(MSG_utf8, 3, 14, (ot_u8*)"CMD", (ot_u8*)msglist[app_select]);

	// Load the session template: Only used for communication tasks
	s_tmpl.channel      = 0x00;
	s_tmpl.flagmask     = 0;
	s_tmpl.subnetmask   = 0;

	switch (app_select) {
	// Communication Task Commands
	///@todo Change Query command to use otapi_task_immediate_advertise()
	///@note You can actually call otapi_task... functions directly from
	/// the alp_extproc function without breaking anything.
	case 0: otapi_task_immediate(&s_tmpl, &applet_send_beacon);   break;
	case 1: otapi_task_immediate(&s_tmpl, &applet_send_query);   break;

	// Direct System Control Commands
	case 2: opmode_goto_gateway();  break;
	case 3: opmode_goto_endpoint(); break;
	}
}










/** Application Main <BR>
  * ==================================================================<BR>
  * Hint: you don't usually need to do anything to main.
  *
  */
void main(void) {
    ///1. Standard Power-on routine (Clocks, Timers, IRQ's, etc)
    ///2. Standard OpenTag Init (most stuff actually will not be used)
    platform_poweron();
    platform_init_OT();

    ///3. Initialize the User Applet & interrupts
    app_init();

    ///4a. If USB is used for MPipe, we need to wait for the driver on
    ///    the host to be ready.  Push the "Key" to exit sleep & resume.
    ///    Also, the kernel is officially unattached during the process.
#   if (MCU_FEATURE_MPIPEVCOM)
        platform_flush_gptim();
        while (opmode_devicemode == 0) {
            SLEEP_MCU();
        }
#   endif

    ///4b. Load a message to show that main startup has passed
    otapi_log_msg(MSG_utf8, 6, 26, (ot_u8*)"SYS_ON", (ot_u8*)"System on and Mpipe active");

    ///5. MAIN RUNTIME (post-init)  <BR>
    ///<LI> Use a main loop with platform_ot_run(), and nothing more. </LI>
    ///<LI> The kernel actually runs at the bottom of this loop.</LI>
    ///<LI> You could put code before or after sys_runtime_manager, which will
    ///     run before or after the (task + kernel).  If you do, keep the code
    ///     very short or else you are risking timing glitches.</LI>
    ///<LI> To run any significant amount of user code, use tasks. </LI>
    while(1) {
    	platform_ot_run();
    }
}




#if defined(__DATA_DEFAULT)
#   include "data_default.c"
#else
#   error "There is no data mapping specified.  Put one here."
#endif

