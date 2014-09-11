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
  * @file       /apps/demo_sniffer/code/main.c
  * @author     JP Norair
  * @version    R100
  * @date       27 Nov 2012
  * @brief      Demo of Sniffer
  *
  * This demo is mostly for the purpose of testing.  It provides an RX-only
  * link.  Any of the exemplary features of this demo are made better examples
  * of in other demos.
  *
  * The default channel is 07.  You can change the listening channel using ALP
  * over MPipe.
  *
  * This Application Recommends:
  * <LI> CC430/MSP430F5: ~24KB Flash, ~1.5KB SRAM                       </LI>
  * <LI> Cortex M3: ~32KB Flash, ~2KB SRAM                              </LI>
  * <LI> MPipe connection to show pong responses                        </LI>
  * <LI> Two LEDs to show RX/TX activity                                </LI>
  * <LI> One input source to send pings (a button), or sending by ALP   </LI>
  *
  * Supported Boards & Platforms: <BR>
  * See /apps/demo_sniffer/code/platform_config.h for board & platform support.
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
#include <otplatform.h>
#include <otlib/logger.h>



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
  */


#if (   defined(BOARD_OMG_anubis)   \
    ||  defined(BOARD_EM430RF)      \
    ||  defined(BOARD_eZ430Chronos) \
    ||  defined(BOARD_RF430USB_5509)    )
#   define _MSP430F5_CORE

#else
#   error "You have not defined a currently supported board: select one in build_config.h"

#endif


#define APP_TASK    &sys.task[TASK_external]





/** Application Events <BR>
  * ========================================================================<BR>
  * This Application can be initialized by a button-press or by an ALP command.
  * The ALP command is treated at a higher-level (see ALP callback later).
  * The button-press is unique to this application, and it is treated here.
  */








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
  * ALP Protocol Commands:      0-127 (00-7F) corresponding to channel to sniff
  *
  * The "user_id" parameter corresponds to the Device ID that sent this ALP.
  *
  * A quickstart guide to the ALP API is available on the Indigresso Wiki.
  * http://www.indigresso.com/wiki/doku.php?id=opentag:api:quickstart
  */
void alp_ext_proc(alp_tmpl* alp, id_tmpl* user_id) {
/// The function app_invoke() will cause the kernel to call ext_systask() as
/// soon as resources are available.
    vlFILE*     fp;
    ot_uni16    scratch;
    ot_u8       channel;
    ot_u8       retval;

    // Start the task only if: Caller is ROOT, ALP Call is Protocol-255, Task is idle
    if (    auth_isroot(user_id)    \
        &&  (alp->inrec.id == 0xFF) \
        &&  (APP_TASK.event == 0)   )   {

        /// Make sure channel is spec-legal.  If so, set the channel of the
        /// first (and only) in the channel list to the specified one, and also
        /// set the channel of the hold scan accordingly.  By default, the scan
        /// is updated every second.  The next scan will have these settings.
        retval  = 0;
        channel = alp->inrec.cmd & 0x7F;
        if (((channel & 0xF0) <= 0x20) && ((channel & 0x0F) <= 0x0E)) {
            fp                  = ISF_open_su(ISF_ID(channel_configuration));
            scratch.ushort      = vl_read(fp, 0);
            scratch.ubyte[0]    = channel;
            vl_write(fp, 0, scratch.ushort);
            vl_close(fp);

            fp                  = ISF_open_su(ISF_ID(hold_scan_sequence));
            scratch.ushort      = vl_read(fp, 0);
            scratch.ubyte[0]    = channel;
            vl_write(fp, 0, scratch.ushort);
            vl_close(fp);

            retval = 1; //success
        }

        alp_load_retval(alp, retval);
    }
}





/** Routing Callback <BR>
  * =======================================================================<BR>
  */
#ifdef EXTF_network_sig_route
void network_sig_route(void* route, void* session) {
/// network_sig_route() will have route >= 0 on successful request processing.
/// "route" will always be set to -1, which will cause the Data Link Layer to
/// ignore the rest of the session and keep listening (sniffing).
    static const ot_u8 unknown_rx[10]   = "UNKNOWN_RX";
    static const ot_u8 opentag_rx[10]   = "OPENTAG_RX";
    ot_u8* label;

    // A valid packet has been received.
    // You can look at the response in the TX ot_queue (and log it if you want).
    // We just log the request and mark it as known or unknown.
    if (*(ot_int*)route >= 0) {
        label = (ot_u8*)opentag_rx;
    }
    else {
        label = (ot_u8*)unknown_rx;
    }

    // Log the received packet data (with 10 character label)
    logger_msg(MSG_raw, 10, q_length(&rxq), label, rxq.front);

    // Turn on Green LED to indicate received packet
    // Activate the app task, which will turn off the LED in 30 ticks
    BOARD_led1_on();
    sys_task_setevent(APP_TASK, 1);
    sys_preempt(APP_TASK, 30);

    // Tell DLL to disregard session, no matter what.
    *(ot_int*)route = -1;
}

#endif






/** OTlib/OTAPI Callback Applets  <BR>
  * ========================================================================<BR>
  * This app uses some of the "std" applets from /otlibext/applets_std
  * The applets used are selected in extf_config.h
  */






/** Kernel Task to turn-off the packet-received LED <BR>
  * =======================================================================<BR>
  */
void ext_systask(ot_task task) {

    // Turn-off the Green LED
    if (task->event != 0) {
        BOARD_led1_off();
    }

    // Turn-off the task
    task->event = 0;
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






/** Application Main <BR>
  * ==================================================================<BR>
  *
  */
void app_init() {
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

    //App Task parameters that stay the same throughout the runtime
    sys_task_setreserve(APP_TASK, 1);
    sys_task_setlatency(APP_TASK, 10);
}




void main(void) {
    ///1. Standard Power-on routine (Clocks, Timers, IRQ's, etc)
    ///2. Standard OpenTag Init (most stuff actually will not be used)
    platform_poweron();
    platform_init_OT();

    ///3. Initialize the User Applet & interrupts
    app_init();

    ///4a. The device will wait (and block anything else) until you connect
    ///    it to a valid console app.
    mpipedrv_standby();

    ///4b. Load a message to show that main startup has passed
    logger_msg(MSG_utf8, 6, 26, (ot_u8*)"SYS_ON", (ot_u8*)"System on and Mpipe active");

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

