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
  * @date       31 July 2012
  * @brief      Opmode Switching Demo
  *
  * This Demonstration Shows:
  * <LI> Differences in Gateway and Endpoint idle-time behavior         </LI>
  * <LI> How to switch Operational Modes during runtime                 </LI>
  * <LI> How to use the sys.loadapp feature                             </LI>
  * <LI> How to build basic commands using the OTAPI-C                  </LI>
  * <LI> How to send log messages to an MPipe Client                    </LI>
  *
  * Common Variations:
  * <LI> Changes in the ISF file the query/beacon command uses          </LI>
  * <LI> Compile-time changes to default scan & beacon cycle parameters </LI>
  *
  * This Application Requires:
  * <LI> A CC430/MSPF5: ~24KB Flash, ~1.5KB SRAM                        </LI>
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
  * Many of the stock applets from /otlibext/stdapplets/ are used as callbacks
  * from the protocol layers.  You can look at extf_config.h to see which ones
  * are enabled (or, to specify new ones).
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
  * It is safe to either keep app_devicemode volatile, since it is set in an
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

volatile ot_u16 app_devicemode;
//ot_bool (*app_task)(void);





/** Applet Functions (platform & board independent) <BR>
  * ========================================================================
  */
// Main Application Functions
void    app_init();
void    app_manager();
ot_bool app_task_null();

// Applets that run on button input (or other external input)
ot_bool app_send_query();
ot_bool app_send_beacon();
ot_bool app_goto_gateway();
ot_bool app_goto_endpoint();

// Applets that run due to OpenTag callbacks
void    app_packet_routing(ot_int code, ot_int type);



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
    platform_swdelay_ms(20);
    led_off[i&3]();
}



void app_init() {
    ot_u8 i;

    ///Default Startup Mode: Gateway.  The button init may also alter this.
    app_devicemode = SYSMODE_GATEWAY;

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



ot_bool app_task_null() {
    return False;
}



void app_manager() {
/// This is something you can play with.  I WOULD NOT recommend using it
/// with the request generation applets, but it will work fine with the
/// mode-switching applets.  (Note, if you change the conditional to
/// "if (sys.mutex == 0)" it is basically identical to sys.loadapp, in
/// which case it will also work with the request generating applets).

//  if (sys.mutex <= 1) {
//      app_task();
//      app_task = &app_task_null;
//  }
}



ot_bool app_send_query() {
/// The C-API for building commands can be bypassed in favor of directly
/// putting data to the queue.  That way is more efficient, but it also requires
/// you to know more about DASH7 that just what order the templates should be.

/// The query that we build will collect sensor configuration data back from
/// all devices that support the sensor protocol.  Much more interesting queries
/// are possible.
    ot_bool output = False;

    if (app_devicemode == SYSMODE_GATEWAY) {
    { //create a new session (it will get copied to session stack)
        session_tmpl session;
        session.channel     = 0x00;
        session.flagmask    = 0;
        session.flags       = 0;
        session.subnet      = 0;
        session.subnetmask  = 0;
        session.timeout     = 16;
        otapi_new_session(&session);
    }
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
        //otapi_start_dialog(); //don't need this, because using internal caller
        output = True;
    }
    sys.loadapp = &sys_loadapp_null;
    return output;
}



ot_bool app_send_beacon() {
    { //create a new session (it will get copied to session stack)
        session_tmpl session;
        session.channel     = 0x00;
        session.flagmask    = 0;
        session.flags       = 0;
        session.subnet      = 0;
        session.subnetmask  = 0;
        session.timeout     = 16;
        otapi_new_session(&session);
    }
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
    //otapi_start_dialog();
    sys.loadapp = &sys_loadapp_null;
    return True;
}



ot_bool app_goto_gateway() {
    if (app_devicemode != SYSMODE_GATEWAY) {
        //vlFILE* fp;
        app_devicemode = SYSMODE_GATEWAY;

        /// Change the Beacon Period to a higher value (slower)
        /// The setting is 0x1000 = 4s
        /// NOTE: the filesystem is purely big-endian, so data must be flipped
        //fp = ISF_open_su(ISF_ID(beacon_transmit_sequence);
        //vl_write(fp, 6, 0x0010);
        //vl_close(fp);

        sys_change_settings(SYSMODE_MASK, SYSMODE_GATEWAY);
        //platform_ot_preempt();
        sub_trig4_high();
    }
    sys.loadapp = &sys_loadapp_null;
    return False;   //no new session
}


ot_bool app_goto_endpoint() {
    if (app_devicemode != SYSMODE_ENDPOINT) {
        //vlFILE* fp;
        app_devicemode = SYSMODE_ENDPOINT;

        /// Change the Beacon Period to a lower value (faster)
        /// The setting is 0x0200 = 500ms
        /// NOTE: the filesystem is purely big-endian, so data must be flipped
        //fp = ISF_open_su(ISF_ID(beacon_transmit_sequence);
        //vl_write(fp, 6, 0x0002);
        //vl_close(fp);

        /// Use the built-in system function to change modes and restart
        sys_change_settings(SYSMODE_MASK, SYSMODE_ENDPOINT);
        //platform_ot_preempt();
        sub_trig4_low();
    }
    sys.loadapp = &sys_loadapp_null;
    return False;   //no new session
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

    if (alp->inrec.id == 0x90) {
        switch (alp->inrec.cmd) {
            case 0: sys.loadapp = &app_send_query;      break;
            case 1: sys.loadapp = &app_send_beacon;     break;
            case 2: sys.loadapp = &app_goto_gateway;    break;
            case 3: sys.loadapp = &app_goto_endpoint;   break;
           default: return;
        }
        
        /// Write back success (1) if respond (CMD bit 7) is enabled
        alp_load_retval(alp, 1);
    }
}












/** Application Main <BR>
  * ======================================================================
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
        while (app_devicemode == 0) {
            SLEEP_MCU();
        }
#   endif

    ///4b. Send a message to show that main startup has passed
    otapi_log_msg(MSG_utf8, 6, 26, (ot_u8*)"SYS_ON", (ot_u8*)"System on and Mpipe active");
    mpipe_wait(); //blocks until msg complete (optional)

    ///5. MAIN RUNTIME (post-init)  <BR>
    ///<LI> a. Pre-empt the kernel (first run)   </LI>
    ///<LI> b. Go to sleep; OpenTag kernel will run automatically in
    ///        the background  </LI>
    ///<LI> c. The kernel has a built-in applet loader.  It is the best way
    ///        to use applets that generate requests or manipulate the system.
    ///        The applets only load during full-stop, so time slotting or
    ///        any other type of MAC activity is not affected. </LI>
    ///<LI> d. 99.99% (or more) of the time, the kernel is not actually
    ///        running.  You can run parallel, local tasks alongside OpenTag
    ///        as long as they operate above priority 1.  (I/O is usually
    ///        priority 0 and kernel is always priority 1) </LI>
    platform_ot_preempt();
    while(1) {
        //app_manager();        //kernel pre-emptor demo
        //local_task_manager();
        SLEEP_MCU();
    }

    ///6. Note on manually pre-empting the kernel for you own purposes:
    ///   It can be done (many internal tasks do it), but be careful.
    ///   It is recommended that you only do it when sys.mutex <= 1
    ///   (i.e. no radio data transfer underway).  One adaptation of
    ///   this demo is to have the mode-switching applets pre-empt the
    ///   kernel (it works fine, but you need to make your own loader
    ///   instead of using sys.loadapp).

}




#if defined(__DATA_DEFAULT)
#   include "data_default.c"
#else
#   error "There is no data mapping specified.  Put one here."
#endif

