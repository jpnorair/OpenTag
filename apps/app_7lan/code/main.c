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
  * @file       /apps/app_7lan/code/main.c
  * @author     JP Norair
  * @version    R100
  * @date       7 Mar 2012
  * @brief      7LAN app for classic-style UDP networking with DASH7
  *
  * See _readme.txt in the app_7lan directory for basic information about what
  * this app does.  Also, please see the wiki for detailed information.
  *
  * http://www.indigresso.com/wiki/doku.php?id=opentag:apps:app_7lan
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
        app_invoke(7);              // Initialize Ping Task on channel 7
    }
}

#elif (defined(__STM32__) && defined(OT_SWITCH1_PINNUM) && (OT_SWITCH1_PINNUM >= 0))
#   define PLATFORM_ISR_SW  platform_isr_exti##OT_SWITCH1_PINNUM

void PLATFORM_ISR_SW(void) {
    // Ignore the button press if the task is in progress already
    if (APP_TASK->event == 0) {
        app_invoke(7);              // Initialize Ping Task on channel 7
    }
}

void sub_button_init() {
/// ARM Cortex M boards must prepare all EXTI line interrupts in their board
/// configuration files.
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

void otapi_alpext_proc(alp_tmpl* alp, id_tmpl* user_id) {
/// The function app_invoke() will cause the kernel to call ext_systask() as
/// soon as resources are available.

    // Start the task only if: Caller is ROOT, ALP Call is Protocol-255, Task is idle
    if (    auth_isroot(user_id)    \
        &&  (alp->inrec.id == 0xFF) \
        &&  (APP_TASK->event == 0)   )   {
        
        app_invoke(alp->inrec.cmd);     // Initialize Ping Task on supplied channel
        alp_load_retval(alp, 1);        // Write back 1 (success)
    }
}





/** M2QP-UDP (Transport Layer) Callback for Printing a Pong <BR>
  * =======================================================================<BR>
  * This function is called when a UDP request or response is received.  It is
  * implemented to monitor port 255, which is used for PONGs in this demo.
  */
#ifdef EXTF_m2qp_sig_udp
ot_bool m2qp_sig_udp(ot_u8 srcport, ot_u8 dstport, id_tmpl* user_id) {
    static const char* label[]  = { "PongID: ", ", RSSI: ", ", Link: " };
    ot_u16  pongval;
    ot_u8   i;
    ot_u8   scratch;

    //1. Read the PONG VAL
    pongval = q_readshort(&rxq);

    // Request: Copy PING VAL to PONG
    if (dstport == 255) {
        q_writeshort(&txq, pongval);
        return True;
    }

#   if defined(BOARD_eZ430Chronos)
    // Chronos doesn't have a normal MPipe, so print-out responses on the LCD
    
#   elif (OT_FEATURE(MPIPE))
    // Response: Compare PING Val to PONG Val and write output to MPipe
    if (dstport == 254) {
        // Prepare logging header: UTF8 (text log) is subcode 1, dummy length is 0
        otapi_log_header(1, 0);
        
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
                        
                case 2: scratch = otutils_int2dec(mpipe.alp.outq->putcursor, dll.last_nrssi);
                        break;
                        
                case 3: goto m2qp_sig_udp_PRINTDONE;
            }
            
            mpipe.alp.outq->putcursor  += scratch;
            mpipe.alp.outq->length     += scratch;
        }

        // Close the log file, send it out, return success
        m2qp_sig_udp_PRINTDONE:
        otapi_log_direct();
        return True;
    }
#   endif

    return False;
}
#endif






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
        app.pingval = platform_prand_u16();
    
        // Log a message.  It is scheduled, and the RF task has higher priority,
        // so if you are sending a DASH7 dialog this log message will usually
        // come-out after the dialog finishes.
#       if (OT_FEATURE(MPIPE))
        otapi_log_msg(MSG_raw, 5, 2, (ot_u8*)"PING:", (ot_u8*)&app.pingval);
#       endif
    
        // Load the session template: Only used for communication tasks
        s_tmpl.channel      = task->cursor;
        s_tmpl.flagmask     = 0;
        s_tmpl.subnetmask   = 0;
        otapi_task_immediate(&s_tmpl, &applet_send_query);
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

void applet_send_query(m2session* session) {
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
        otapi_open_request(ADDR_anycast, &routing);
    }
    { //use a command template for collection of single file from single file search
        command_tmpl command;
        command.opcode      = (ot_u8)CMD_udp_on_file;
        command.type        = (ot_u8)CMDTYPE_na2p_request;
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
        q_writebyte(&txq, 254);
        q_writebyte(&txq, 255);
        q_writeshort(&txq, app.pingval);
    }

    //Done building command, close the request and send the dialog
    otapi_close_request();
}





/** Application Main <BR>
  * ==================================================================<BR>
  *
  */
void app_init() {
#if defined(BOARD_eZ430Chronos)
/// Setup LCD


#else
/// 1. Blink the board LEDs to show that it is starting up.  
/// 2. Configure the board input button, which for this app will send a ping
    ot_u8 i;

    i=4;
    while (i != 0) {
        if (i&1)    otapi_led1_on();
        else        otapi_led2_on();

        platform_swdelay_ms(30);
        otapi_led2_off();
        otapi_led1_off();
        i--;
    }
#endif

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
    platform_poweron();
    platform_init_OT();

    ///3a. The device will wait (and block anything else) until you connect
    ///    it to a valid console app.
    ///3b. Load a message to show that main startup has passed
#   if (OT_FEATURE(MPIPE))
    mpipedrv_standby();
    otapi_log_msg(MSG_utf8, 6, 26, (ot_u8*)"SYS_ON", (ot_u8*)"System on and Mpipe active");
#   endif
    
    ///4. Initialize the User Applet & interrupts
    app_init();

    ///5. MAIN RUNTIME (post-init)  <BR>
    ///<LI> Use a main loop with platform_ot_run(), and nothing more. </LI>
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

