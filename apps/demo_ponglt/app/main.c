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
  * @file       /apps/demo_ponglt/code/main.c
  * @author     JP Norair
  * @version    R100
  * @date       27 Nov 2012
  * @brief      Demo of Pong for Link Testing (PongLT)
  *
  * Functional purpose of this demo:
  * <LI> Show a simple way how anycast queries can be used </LI>
  * <LI> Show how a command session can be initiated by a button press </LI>
  * <LI> Provide a simple way to do link testing between a demonstration
  *      system of two or more devices </LI>
  *
  * Pong for Link Testing is a masterless application that allows one device to
  * send a query and all the others to respond.  It does not use any sort of
  * power conservation measures -- all the devices are listening all the time.
  * This is what makes it good for link testing.  It is also an incredibly
  * simple starting point for OpenTag users.
  *
  * The Ping is a UDP-with-inventory packet on port 255, which uses the
  * "User ID" ISF to store a small cookie that qualifies devices that have the
  * string "APP=PongLT" somewhere in that file.  Only these devices will fully
  * process the Ping request and produce a Pong response.
  *
  * This Application Recommends:
  * <LI> CC430/MSP430F5: ~24KB Flash, ~1.5KB SRAM                       </LI>
  * <LI> Cortex M3: ~32KB Flash, ~2KB SRAM                              </LI>
  * <LI> MPipe connection to show pong responses                        </LI>
  * <LI> Two LEDs to show RX/TX activity                                </LI>
  * <LI> One input source to send pings (a button), or sending by ALP   </LI>
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

#include <otstd.h>
#include <otplatform.h>
#include <board.h>
#include <otlib/rand.h>
#include <otlib/logger.h>

#include <otsys.h>
#include <m2api.h>



/** Data Mapping <BR>
  * ===========================================================================
  * The Opmode Demo needs a particular data mapping.  It is not unusual, but
  * the demo may not work if the data is not set correctly.  This define below
  * uses the default data mapping (/apps/demo_opmode/code/data_default.c)
  */
#define __DATA_DEFAULT






/** Application Global Variables <BR>
  * ========================================================================<BR>
  * opmode_devicemode must be volatile, since it is set in an interrupt service
  * routine.
  */
typedef struct {
    ot_u16 pingval;
} app_struct;

app_struct app;





/** Applet Functions (platform & board independent) <BR>
  * ========================================================================
  */
// Main Application Functions
void app_blink();
void app_init();
void app_invoke(ot_u8 call_type);

/// Communication Task Applets
void applet_send_query(m2session* active);
void applet_send_beacon(m2session* active);

/// Direct Control Applets
void opmode_goto_gateway();
void opmode_goto_endpoint();

// Applets that run due to OpenTag callbacks
//void    app_packet_routing(ot_int code, ot_int type);



/** Application local subroutines (platform & board dependent) <BR>
  * ========================================================================<BR>
  * sub_button_init() is a board-dependent function, as
  */
void sub_button_init();
#define APP_TASK    (&sys.task[TASK_external])





/** Application Events <BR>
  * ========================================================================<BR>
  * This Application can be initialized by a button-press or by an ALP command.
  * The ALP command is treated at a higher-level (see ALP callback later).
  * The button-press is unique to this application, and it is treated here.
  */

#if (defined(__MSP430F5__) && defined(OT_SWITCH1_PORT))
#   if (OT_SWITCH1_PORTNUM == 1)
#       define PLATFORM_ISR_SW  platform_isr_p1
#   else
#       define PLATFORM_ISR_SW  platform_isr_p2
#   endif

void sub_button_init() {
#   if (OT_SWITCH1_PULLING)
    OT_SWITCH1_PORT->REN   |= OT_SWITCH1_PIN;       //Enable Internal Pull up/down
#   endif
#   if (OT_SWITCH1_POLARITY == 0)
    OT_SWITCH1_PORT->DOUT  |= OT_SWITCH1_PIN;       //Set Pull-up (pull-down is default)
    //OT_SWITCH1_PORT->IES   &= ~OT_SWITCH1_PIN;    //falling edge is default
#   else
    //OT_SWITCH1_PORT->DOUT  &= ~OT_SWITCH1_PIN;    //Pull-down is default
    OT_SWITCH1_PORT->IES   |= OT_SWITCH1_PIN;       //Set Rising Edge
#   endif

    //Clear and enable interrupt
    OT_SWITCH1_PORT->IFG    = 0;
    OT_SWITCH1_PORT->IE    |= OT_SWITCH1_PIN;
}


void PLATFORM_ISR_SW() {
/// If you implement more interrupts on this port, you can make this function
/// into a switch statement using OT_SWITCH1_PIV.
    OT_SWITCH1_PORT->IFG = 0;

    // Ignore the button press if the task is in progress already
    if (APP_TASK->event == 0) {
        app_invoke(0x18);              // Initialize Ping Task on channel 18
    }
}

#elif (defined(__STM32__) && defined(OT_SWITCH1_PINNUM) && (OT_SWITCH1_PINNUM >= 0))
    void OT_SWITCH1_ISR(void) {
        // Ignore the button press if the task is in progress already
        if (APP_TASK->event == 0) {
            app_invoke(0x18);              // Initialize Ping Task on channel 18
        }
    }

    void sub_button_init() {
    /// ARM Cortex M boards must prepare all EXTI line interrupts in their board
    /// configuration files, but the actual line interrupt must be enabled here.
        EXTI->RTSR |= OT_SWITCH1_PIN;
        EXTI->IMR  |= OT_SWITCH1_PIN;
    }


#else
#   warn "You are not using a known, compatible MCU.  Demo might not work."
    void sub_button_init() {}

#endif






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
  * ALP Protocol Commands:      0-127 (00-7F) corresponding to channel to ping
  *
  * The "user_id" parameter corresponds to the Device ID that sent this ALP.
  *
  * A quickstart guide to the ALP API is available on the Indigresso Wiki.
  * http://www.indigresso.com/wiki/doku.php?id=opentag:api:quickstart
  */

///@todo change task into logger task, and make the Pingpong part entirely
///      session driven.
ot_bool alp_ext_proc(alp_tmpl* alp, id_tmpl* user_id) {
    /// Offset=2 is the ALP ID, which defines the protocol to use.
    /// This Project has only one custom app, ID=255
    switch (alp->inq->getcursor[2]) {

    // This app is completely atomic and it manages the ALP data in place rather
    // than through a subordinate app queue.
    case 255: {
        ot_u8 cmd           = alp->inq->getcursor[3] & 0x7f;
        alp->inq->getcursor+= 4;

        // Start a ping
        if (cmd == 0) {
            app_invoke(0x18);
        }

        // Request: copy ping val to pong
        else if (cmd == 255) {
            ot_u16  pongval = q_readshort(alp->inq);
            q_writeshort(alp->outq, pongval);
        }

        // Response: Compare PING Val to PONG Val and write output to MPipe
        else {
#       if (OT_FEATURE(MPIPE))
            static const char* label[]  = { "PongID: ", ", RSSI: ", ", Link: " };
            ot_u8   i;
            ot_u8   scratch;

            // Prepare logging header: UTF8 (text log) is subcode 1, dummy length is 0
            logger_header(1, 0);

            // Print out the three parameters for PongLT, one at a time.
            // If you are new to OpenTag, this is a common example of a state-
            // based code structure JP likes to use.
            i = 0;
            while (1) {
                q_writestring(mpipe.alp.outq, (ot_u8*)label[i], 8);
                switch (i++) {
                    case 0: scratch = otutils_bin2hex(  mpipe.alp.outq->putcursor,
                                                        user_id->value,
                                                        user_id->length     );
                            break;

                    case 1: scratch = otutils_int2dec(mpipe.alp.outq->putcursor, radio.last_rssi);
                            break;

                    case 2: scratch = otutils_int2dec(mpipe.alp.outq->putcursor, radio.last_linkloss);
                            break;

                    case 3: goto m2qp_sig_udp_PRINTDONE;
                }

                mpipe.alp.outq->putcursor  += scratch;
            }

            // Close the log file, send it out, return success
            m2qp_sig_udp_PRINTDONE:
            logger_direct();
#       endif
        }

    } break;
    }

    return True;
}








/** OTlib/OTAPI Callback Applets  <BR>
  * ========================================================================<BR>
  * This app uses some of the "std" applets from /otlibext/applets_std
  * The applets used are selected in extf_config.h
  */







/** PING Kernel Task <BR>
  * =======================================================================<BR>
  * This function will be activated by the kernel when the external task is
  * active and there are resources available to run the task.  This task will
  * just activate the DLL session (also a task) and then turn itself off.
  * Then, the kernel will call the DLL session task as soon as necessary
  * resources are available to run that task (by default DLL task is top
  * priority, so it should get called right away).
  */

void ext_systask(ot_task task) {
    session_tmpl    s_tmpl;

    if (task->event == 1) {
        task->event = 0;

        // this is the same as the length of the response window,
        // which is set in applet_send_query()
        task->nextevent = 512;

        // Generate a pseudo random 16 bit number to be used as a ping check value
        app.pingval = PLATFORM_ENDIAN16(rand_prn16());

        // Log a message.  It is scheduled, and the RF task has higher priority,
        // so if you are sending a DASH7 dialog this log message will usually
        // come-out after the dialog finishes.
#       if (OT_FEATURE(MPIPE))
        logger_msg(MSG_raw, 5, 2, (ot_u8*)"PING:", (ot_u8*)&app.pingval);
#       endif

        // Load the session template: Only used for communication tasks
        s_tmpl.channel      = task->cursor;
        s_tmpl.flagmask     = 0;
        s_tmpl.subnetmask   = 0;
        m2task_immediate(&s_tmpl, &applet_send_query);
    }

    // Turn off the task after 512 ticks (what is set above)
    // Note that this task will not be activated by the button press or ALP
    // when event != 0, because those routines have conditionals in them.
    //else {
    //    task->event = 0;
    //}
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

void applet_send_query(m2session* active) {
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
        command.extension   = (ot_u8)CMDEXT_none;
        otapi_put_command_tmpl(&status, &command);
    }
    { //write the dialog information (timeout, channels to use)
        dialog_tmpl dialog;
        dialog.channels = 0;    //use same channel as request for response
        dialog.timeout  = 0x41; //same as otutils_encode_timeout(512) -- 512 tick response slot
        otapi_put_dialog_tmpl(&status, &dialog);
    }
    { //write the query to search for the sensor protocol id
        static const ot_u8 query_str[10] = "APP=PongLT";
        query_tmpl query;
        query.code      = M2QC_COR_SEARCH + 10; // do a 100% length=10 correlation search
        query.mask      = NULL;                 // don't do any masking (no partial matching)
        query.length    = 10;                   // query_str is 10 bytes
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
    { //put in UDP ports (from 254 to 255) and Ping ID
        udp_tmpl udp;
        udp.data_length    = 2;
        udp.dst_port       = 255;
        udp.src_port       = 254;
        udp.data           = (ot_u8*)&app.pingval;
        otapi_put_udp_tmpl(&status, &udp);
    }

    //Done building command, close the request and send the dialog
    otapi_close_request();
}





/** Application Main <BR>
  * ==================================================================<BR>
  *
  */
  
void app_blink() {
#if defined(BOARD_eZ430Chronos)
/// Setup LCD

#else
/// 1. Blink the board LEDs to show that it is starting up.
/// 2. Configure the board input button, which for this app will send a ping
    ot_u8 i;

    i=4;
    while (i != 0) {
        if (i&1)    BOARD_led1_on();
        else        BOARD_led2_on();

        delay_ms(30);
        BOARD_led2_off();
        BOARD_led1_off();
        i--;
    }
#endif
}
  
void app_init() {
    sub_button_init();
}





void app_invoke(ot_u8 channel) {
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
    sys_task_setevent(APP_TASK, 1);
    sys_task_setcursor(APP_TASK, channel);
    sys_task_setreserve(APP_TASK, 1);
    sys_task_setlatency(APP_TASK, 255);
    sys_preempt(APP_TASK, 0);
}




void main(void) {
    ///1. Standard Power-on routine (Clocks, Timers, IRQ's, etc)
    ///2. Standard OpenTag Init (most stuff actually will not be used)
    ///3. Indicate the app is alive
    platform_poweron();
    platform_init_OT();
    app_blink();

    ///4. Set the app name (PongLT) as a cookie in the User-ID.
    ///   This is used for query filtering
    {   static const ot_u8 appstr[] = "APP=PongLT";
        vlFILE* fp;
        fp = ISF_open_su(ISF_ID(user_id));
        if (fp != NULL) {
            vl_store(fp, sizeof(appstr), appstr);
        }
        vl_close(fp);
    }

    ///5a. The device will wait (and block anything else) until you connect
    ///    it to a valid console app.
    ///5b. Load a message to show that main startup has passed
#   if (OT_FEATURE(MPIPE))
    mpipedrv_standby();
    logger_msg(MSG_utf8, 6, 26, (ot_u8*)"SYS_ON", (ot_u8*)"System on and Mpipe active");
#   endif

    ///6. MAIN RUNTIME (post-init)  <BR>
    ///<LI> Use a main loop with platform_ot_run(), and nothing more. </LI>
    ///<LI> You could put code before or after sys_runtime_manager, which will
    ///     run before or after the (task + kernel).  If you do, keep the code
    ///     very short or else you are risking timing glitches.</LI>
    ///<LI> To run any significant amount of user code, use tasks. </LI>
    app_init();
    
    while(1) {
        platform_ot_run();
    }
}




#if defined(__DATA_DEFAULT)
#   include <../_common/fs_default_startup.c>
#else
#   error "There is no data mapping specified.  Put one here."
#endif

