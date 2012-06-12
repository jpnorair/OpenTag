/* Copyright 2011 JP Norair
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
  * @file       /apps/Demo_Opmode/Code/main.c
  * @author     JP Norair
  * @version    V1.0
  * @date       16 December 2011
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
  * Currently Supported Boards:
  * <LI> AG430DK variants (CC430F5137)                </LI>
  * <LI> EM430RF variants (CC430F6137)                </LI>
  * <LI> MLX73Proto board (STM32F103)                 </LI>
  *
  * @note Different boards may support different methods of input and output.
  ******************************************************************************
  */

#include "OTAPI.h"
#include "OT_platform.h"

// These should probably get integrated into OTAPI at some point
#include "m2_transport.h"
#include "m2_network.h"
#include "system_native.h"
#include "veelite.h"




/** Compile-Time Device ID configuration <BR>
  * ===========================================================================
  * If you are just using two devices for testing, they can probably use the
  * same ID's without conflict (conflict is relative, it only matters because
  * it is hard to tell which device is actually sending the response).  You can
  * also select a ID from below at compile-time (or add more) .
  */
#define __USE_ID0
//#define __USE_ID1

#if defined(__USE_ID0)
#   define __UID    0x1D, 0xAA, 0xA0, 0x1D, 0xB0, 0xB0, 0xB0, 0xB0
#   define __VID    0x1D, 0xC0
#elif defined(__USE_ID1)
#   define __UID    0x1D, 0xAA, 0xA1, 0x1D, 0xB1, 0xB1, 0xB1, 0xB1
#   define __VID    0x1D, 0xC1
#else
#   error "Device ID index is not selected"
#endif




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
void    app_radio_init(ot_int code1);
void    app_radio_term(ot_int code1, ot_int code2);
void    app_hold_init(void* adt);
void    app_sleep_init(void* adt);
void    app_beacon_init(void* adt);
void    app_packet_routing(ot_int code, ot_int type);
ot_bool app_udp_request(id_tmpl* id, ot_int payload_length, ot_u8* payload);
ot_bool app_error_response(id_tmpl* id, ot_int payload_length, ot_u8* payload);
ot_bool app_std_response(id_tmpl* id, ot_int payload_length, ot_u8* payload);
ot_bool app_a2p_response(id_tmpl* id, ot_int payload_length, ot_u8* payload);


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
#   include "main_inc_MLX73Proto_E.c"

#elif (defined(BOARD_AG430DK_GW1) || defined(BOARD_AG430DK_EP1))
#   include "main_inc_AG430DK.c"

//#elif (BOARD == EM430RF)
//#   include "main_inc_EM430RF.c"

//#elif (BOARD == eZ430Chronos)
//#   include "main_inc_eZ430Chronos.c"
#elif defined(STM32H152)
#   include "main_inc_STM32H152.c"

#else
#   error "You have not defined a supported board: select one in build_config.h"

#endif









/** User Applet and Button Management Routines <BR>
  * ===========================================================================<BR>
  * The User applet is primarily activated by callbacks from the kernel.  However,
  * in this system some features are also activated by button presses.
  *
  */
void sub_led_cycle(ot_u8 i) {
    switch (i & 3) {
        case 0: sub_trig4_high();       break;  //BLUE
        case 1: sub_trig3_high();       break;  //RED
        case 2: platform_trig2_high();  break;  //ORANGE
        case 3: platform_trig1_high();  break;  //GREEN
    }

    platform_swdelay_ms(33);

    switch (i & 3) {
        case 0: sub_trig4_low();        break;
        case 1: sub_trig3_low();        break;
        case 2: platform_trig2_low();   break;
        case 3: platform_trig1_low();   break;
    }
}

void app_init() {
    ot_u8 i;

    ///Default Startup Mode: Gateway.  The button init may also alter this.
    app_devicemode = SYSMODE_GATEWAY;

    ///Initialize Application Triggers (LEDs) and blink the LEDs
    ///(Go left-to-right then right-to-left, like on an old-timey Cylon helmet).
    sub_trig_init();

    i = 255;
    do {
        i++;
        sub_led_cycle(i);
    } while (i != 3);
    do {
        i--;
        sub_led_cycle(i);
    } while (i != 0);

    ///Initialize the input buttons
    sub_button_init();
    //app_trig4_high();

    ///Attribute OpenTag Kernel Callbacks with App routines
    ///The idle event prestart callbacks are not used in this application.
    sys.evt.RFA.init        = &app_radio_init;
    sys.evt.RFA.terminate   = &app_radio_term;

    ///Atribute OpenTag Network Routing Callback
    m2np.signal.route       = &app_packet_routing;

    ///Attribute OpenTag Transport Callbacks with App routines
    m2qp.signal.shell_request   = &app_udp_request;
    m2qp.signal.error_response  = &app_error_response;
    m2qp.signal.std_response    = &app_std_response;
    m2qp.signal.a2p_response    = &app_a2p_response;

    //app_task = &app_task_null;
    
    ///Set MPipe to go back to listen after TX.
#   if ((OT_FEATURE(MPIPE) == ENABLED) && (OT_FEATURE(NDEF) == ENABLED))
        mpipe_setsig_txdone(&otapi_ndef_idle);
        mpipe_setsig_rxdone(&otapi_ndef_proc);
        //otapi_ndef_idle(0);
#   endif
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






/** Kernel Callback Applet Links  <BR>
  * =======================================================================<BR>
  * The Kernel (System Module) has a few callbacks.  Most applications are
  * easier to attach to the Transport Layer, but the Kernel Callbacks can
  * be useful for certain things.
  */

void app_radio_init(ot_int code1) {
/// This is a callback the kernel uses when it is starting up a radio process.
/// It is used here to turn-on activity LEDs.

    // Assume that (1 <= code1 <= 5), which is the case in normal operation
    if (code1 < 3)  platform_trig2_high();
    else            platform_trig1_high();
}


void app_radio_term(ot_int code1, ot_int code2) {
/// This is a callback the kernel uses when it is ending a radio process.
/// In normal applications, Kernel & RF System callbacks are not terribly
/// useful, but they are good for test/debug.  The RX callback can also
/// be used to log all received frames, which can sometimes be useful,
/// or the TX callback to log all transmitted frames.  This app demos both
/// of these features.
/// Hint: RFxRX_e1 label means the received frame has bad CRC.

    ot_u8   loglabel[8] = {'R', 'F', 'F', 0, 'X', '_', 'o', 'k'};
    ot_u8*  logdata;
    ot_int  logdata_len;

    /// Put the error code in the label, if there is an error code
    if (code2 < 0) {
        loglabel[6] = 'e';
        loglabel[7] = (ot_int)('0') - code2;
    }

    /// Look at the control code an form the label to reflect the type
    switch (code1) {
        /// RX driver process termination:
        /// (1) background scan ended, (2) or foreground scan ended
        case 1: loglabel[2] = 'B';
        case 2: loglabel[3] = 'R';
                logdata_len = rxq.length;
                logdata     = rxq.front;
                break;

        /// TX CCA/CSMA driver process termination:
        /// (3) TX CSMA process ended
        case 3: loglabel[2] = 'C';
                loglabel[3] = 'C';
                loglabel[4] = 'A';
                logdata_len = 0;
                logdata     = NULL; // suppress compiler warning
                break;

        /// TX driver process termination
        /// Background Flood (4) or Foreground TX (5): turn-on green and log TX
        case 4: loglabel[2] = 'B';
        case 5: loglabel[3] = 'T';
                logdata_len = txq.length;
                logdata     = txq.front;
                break;

        default: goto app_radio_term_end;
    }

    /// Log in ASCII hex the prepared driver message
    otapi_log_hexmsg(8, logdata_len, loglabel, logdata);

    /// In all cases, turn-off the LEDs
    app_radio_term_end:
    platform_trig2_low();   //Orange LED off
    platform_trig1_low();   //Green LED off
}



void app_hold_init(void* adt) {
/// This is a callback the kernel uses immediately before it runs a hold
/// process cycle (channel scan).  It is not used here (Mostly for testing).
}

void app_sleep_init(void* adt) {
/// This is a callback the kernel uses immediately before it runs a sleep
/// process cycle (channel scan).  It is not used here (Mostly for testing).
}

void app_beacon_init(void* adt) {
/// This is a callback the kernel uses immediately before it runs an
/// automated beacon process.  It is not used here (Mostly for testing).
}


/** Network Layer Callback <BR>
  * =======================================================================<BR>
  * Network layer uses a callback when a packet has been successfully received
  * and it is routed.
  */
void app_packet_routing(ot_int code, ot_int protocol) {
	static const ot_u8 label_dialog[]	= { 9, 'M','2','_','D','i','a','l','o','g' };
	static const ot_u8 label_nack[]		= { 7, 'M','2','_','N','a','c','k' };
	static const ot_u8 label_stream[]	= { 9, 'M','2','_','S','t','r','e','a','m' };
	static const ot_u8 label_unknown[]	= { 4, 'P','K','T','?' };

	static const ot_u8* labels[] 		= { label_dialog,
											label_nack,
											label_stream,
											label_unknown };
	if ( (ot_u16)protocol > 2 )
		protocol = 3;

	otapi_log_msg(	MSG_raw,
					labels[protocol][0],
					rxq.length,
					&(labels[protocol][1]),
					rxq.front	);
}





/** Transport Layer Callback Applet Links  <BR>
  * =======================================================================<BR>
  * The user application naturally connects to the Transport Layer, which
  * provides the application with processed data payloads.
  *
  * Function input parameters
  * Arg 1:  (id_tmpl*) Pointer to device id from requester [->length, ->value]
  * Arg 2:  (ot_int) Length of the data payload
  * Arg 3:  (ot_u8*) The data payload
  *
  * Function output:
  * Output is always ot_bool.  For responses this doesn't matter.  For requests,
  * returning false will make the system not respond.
  *
  * The data payload is from the RX queue (a reserved buffer)  You can do
  * anything you want with the RX queue data -- by the time it gets to the
  * application layer, the lower layers don't need it anymore.  To put response
  * data, use the TX Queue, another reserved buffer: q_writebyte(&txq, BYTE)
  */
ot_bool app_udp_request(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
/// Transport Layer calls this when a UDP-class request has been received
/// Not part of the application at the moment
	return False;
}

ot_bool app_error_response(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
/// Called when an error response type is received.
    otapi_log_hexmsg(6, payload_length, (ot_u8*)"RX_ERR", payload);
    return False;
}

ot_bool app_std_response(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
/// Called when a non-arbitrated response is received by the Transport Layer.
    otapi_log_hexmsg(7, payload_length, (ot_u8*)"RX_RESP", payload);
    return False;
}

ot_bool app_a2p_response(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
/// Called when an Arbitrated Two Party (A2P) response is received.
/// Not part of the application at the moment
	return False;
}






/** User ALP Processor  <BR>
  * =======================================================================<BR>
  * If using proprietary/custom ALPs, the processor should be implemented here.
  * There is an example implementation below, which can be uncommented to match
  * the example in the API Quickstart Guide:
  * http://www.indigresso.com/wiki/doku.php?id=opentag:api:quickstart
  */ 
/*
void otapi_alpext_proc(alp_record* in_rec, alp_record* out_rec, Queue* in_q, 
                        Queue* out_q, id_tmpl* user_id) {
/// For this example, the directive ID is 0x90 and the commands are 0-3.  You
/// can change these values simply by changing the implementation of this 
/// function.

    if (in_rec->dir_id == 0x90) {
        switch (in_rec->dir_cmd) {
            case 0: sys.loadapp = &app_send_query;      break;
            case 1: sys.loadapp = &app_send_beacon;     break;
            case 2: sys.loadapp = &app_goto_gateway;    break;
            case 3: sys.loadapp = &app_goto_endpoint;   break;
           default: return;
        }
        
        /// Write back success (1) if respond (CMD bit 7) is enabled
        alp_load_retval(    (in_rec->dir_cmd & 0x80), 
                            (in_rec->dir_cmd | 0x40), 
                            1, out_rec, out_q           );
    }
}
*/











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
    otapi_log_msg(6, 26, (ot_u8*)"SYS_ON", (ot_u8*)"System on and Mpipe active");
    //platform_swdelay_ms(16);

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









/** Default File data allocations
  * ============================================================================
  * - Veelite also uses an additional 1536 bytes for wear leveling
  * - Wear leveling overhead is configurable, but fixed for all FS sizes
  * - Veelite virtual addressing allocations of key sectors below:
  *     Overhead:   0000 to 03FF        (1024 bytes alloc)
  *     ISFSB:      0400 to 049F        (160 bytes alloc)
  *     GFB:        04A0 to 089F        (1024 bytes)
  *     ISFB:       08A0 to 0FFF        (1888 bytes)
  */
#define SPLIT_SHORT(VAL)    (ot_u8)((ot_u16)(VAL) >> 8), (ot_u8)((ot_u16)(VAL) & 0x00FF)
#define SPLIT_LONG(VAL)     (ot_u8)((ot_u32)(VAL) >> 24), (ot_u8)(((ot_u32)(VAL) >> 16) & 0xFF), \
                            (ot_u8)(((ot_u32)(VAL) >> 8) & 0xFF), (ot_u8)((ot_u32)(VAL) & 0xFF)

#define SPLIT_SHORT_LE(VAL) (ot_u8)((ot_u16)(VAL) & 0x00FF), (ot_u8)((ot_u16)(VAL) >> 8)
#define SPLIT_LONG_LE(VAL)  (ot_u8)((ot_u32)(VAL) & 0xFF), (ot_u8)(((ot_u32)(VAL) >> 8) & 0xFF), \
                            (ot_u8)(((ot_u32)(VAL) >> 16) & 0xFF), (ot_u8)((ot_u32)(VAL) >> 24)


/// These overhead are the Veelite vl_header files. They are hard coded,
/// and they must be in the endian of the platform. (Little endian here)

#if (CC_SUPPORT == GCC)
__attribute__((section(".vl_ov")))
#elif (CC_SUPPORT == CL430)
#pragma DATA_SECTION(overhead_files, ".vl_ov")
#endif
const ot_u8 overhead_files[] = {
    //0x00, 0x00, 0x00, 0x01,                 /* GFB ELements 0 - 3 */
    //0x00, GFB_MOD_standard,
    //0x00, 0x14, 0xFF, 0xFF,
    //0x00, 0x00, 0x00, 0x01,
    //0x01, GFB_MOD_standard,
    //0x00, 0x15, 0xFF, 0xFF,
    //0x00, 0x00, 0x00, 0x01,
    //0x02, GFB_MOD_standard,
    //0x00, 0x16, 0xFF, 0xFF,
    //0x00, 0x00, 0x00, 0x01,
    //0x03, GFB_MOD_standard,
    //0x00, 0x17, 0xFF, 0xFF,

    ISFS_LEN(transit_data), 0x00,
    ISFS_ALLOC(transit_data), 0x00,
    ISFS_ID(transit_data),
    ISFS_MOD(transit_data),
    SPLIT_SHORT_LE(ISFS_BASE(transit_data)),
    0xFF, 0xFF,

    ISFS_LEN(capability_data), 0x00,
    ISFS_ALLOC(capability_data), 0x00,
    ISFS_ID(capability_data),
    ISFS_MOD(capability_data),
    SPLIT_SHORT_LE(ISFS_BASE(capability_data)),
    0xFF, 0xFF,

    ISFS_LEN(query_results), 0x00,
    ISFS_ALLOC(query_results), 0x00,
    ISFS_ID(query_results),
    ISFS_MOD(query_results),
    SPLIT_SHORT_LE(ISFS_BASE(query_results)),
    0xFF, 0xFF,

    ISFS_LEN(hardware_fault), 0x00,
    ISFS_ALLOC(hardware_fault), 0x00,
    ISFS_ID(hardware_fault),
    ISFS_MOD(hardware_fault),
    SPLIT_SHORT_LE(ISFS_BASE(hardware_fault)),
    0xFF, 0xFF,

    ISFS_LEN(device_discovery), 0x00,
    ISFS_ALLOC(device_discovery), 0x00,
    ISFS_ID(device_discovery),
    ISFS_MOD(device_discovery),
    SPLIT_SHORT_LE(ISFS_BASE(device_discovery)),
    0xFF, 0xFF,

    ISFS_LEN(device_capability), 0x00,
    ISFS_ALLOC(device_capability), 0x00,
    ISFS_ID(device_capability),
    ISFS_MOD(device_capability),
    SPLIT_SHORT_LE(ISFS_BASE(device_capability)),
    0xFF, 0xFF,

    ISFS_LEN(device_channel_utilization), 0x00,
    ISFS_ALLOC(device_channel_utilization), 0x00,
    ISFS_ID(device_channel_utilization),
    ISFS_MOD(device_channel_utilization),
    SPLIT_SHORT_LE(ISFS_BASE(device_channel_utilization)),
    0xFF, 0xFF,

    ISFS_LEN(location_data), 0x00,
    ISFS_ALLOC(location_data), 0x00,
    ISFS_ID(location_data),
    ISFS_MOD(location_data),
    SPLIT_SHORT_LE(ISFS_BASE(location_data)),
    0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* Mode 2 ISFs, written as little endian */
    ISF_LEN(network_settings), 0x00,                /* Length, little endian */
    SPLIT_SHORT_LE(ISF_ALLOC(network_settings)),    /* Alloc, little endian */
    ISF_ID(network_settings),                       /* ID */
    ISF_MOD(network_settings),                      /* Perms */
    SPLIT_SHORT_LE(ISF_BASE(network_settings)),
    SPLIT_SHORT_LE(ISF_MIRROR(network_settings)),

    ISF_LEN(device_features), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(device_features)),
    ISF_ID(device_features),
    ISF_MOD(device_features),
    SPLIT_SHORT_LE(ISF_BASE(device_features)),
    0xFF, 0xFF,

    ISF_LEN(channel_configuration), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(channel_configuration)),
    ISF_ID(channel_configuration),
    ISF_MOD(channel_configuration),
    SPLIT_SHORT_LE(ISF_BASE(channel_configuration)),
    0xFF, 0xFF, /*SPLIT_SHORT_LE(ISF_MIRROR(channel_configuration)), */

    ISF_LEN(real_time_scheduler), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(real_time_scheduler)),
    ISF_ID(real_time_scheduler),
    ISF_MOD(real_time_scheduler),
    SPLIT_SHORT_LE(ISF_BASE(real_time_scheduler)),
    0xFF, 0xFF,

    ISF_LEN(sleep_scan_sequence), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(sleep_scan_sequence)),
    ISF_ID(sleep_scan_sequence),
    ISF_MOD(sleep_scan_sequence),
    SPLIT_SHORT_LE(ISF_BASE(sleep_scan_sequence)),
    0xFF, 0xFF,

    ISF_LEN(hold_scan_sequence), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(hold_scan_sequence)),
    ISF_ID(hold_scan_sequence),
    ISF_MOD(hold_scan_sequence),
    SPLIT_SHORT_LE(ISF_BASE(hold_scan_sequence)),
    0xFF, 0xFF,

    ISF_LEN(beacon_transmit_sequence), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(beacon_transmit_sequence)),
    ISF_ID(beacon_transmit_sequence),
    ISF_MOD(beacon_transmit_sequence),
    SPLIT_SHORT_LE(ISF_BASE(beacon_transmit_sequence)),
    0xFF, 0xFF,

    ISF_LEN(protocol_list), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(protocol_list)),
    ISF_ID(protocol_list),
    ISF_MOD(protocol_list),
    SPLIT_SHORT_LE(ISF_BASE(protocol_list)),
    0xFF, 0xFF,

    ISF_LEN(isfs_list), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(isfs_list)),
    ISF_ID(isfs_list),
    ISF_MOD(isfs_list),
    SPLIT_SHORT_LE(ISF_BASE(isfs_list)),
    0xFF, 0xFF,

    ISF_LEN(gfb_file_list), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(gfb_file_list)),
    ISF_ID(gfb_file_list),
    ISF_MOD(gfb_file_list),
    SPLIT_SHORT_LE(ISF_BASE(gfb_file_list)),
    0xFF, 0xFF,

    ISF_LEN(location_data_list), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(location_data_list)),
    ISF_ID(location_data_list),
    ISF_MOD(location_data_list),
    SPLIT_SHORT_LE(ISF_BASE(location_data_list)),
    0xFF, 0xFF,

    ISF_LEN(ipv6_addresses), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(ipv6_addresses)),
    ISF_ID(ipv6_addresses),
    ISF_MOD(ipv6_addresses),
    SPLIT_SHORT_LE(ISF_BASE(ipv6_addresses)),
    0xFF, 0xFF,

    ISF_LEN(sensor_list), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(sensor_list)),
    ISF_ID(sensor_list),
    ISF_MOD(sensor_list),
    SPLIT_SHORT_LE(ISF_BASE(sensor_list)),
    0xFF, 0xFF,

    ISF_LEN(sensor_alarms), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(sensor_alarms)),
    ISF_ID(sensor_alarms),
    ISF_MOD(sensor_alarms),
    SPLIT_SHORT_LE(ISF_BASE(sensor_alarms)),
    0xFF, 0xFF,

    ISF_LEN(root_authentication_key), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(root_authentication_key)),
    ISF_ID(root_authentication_key),
    ISF_MOD(root_authentication_key),
    SPLIT_SHORT_LE(ISF_BASE(root_authentication_key)),
    0xFF, 0xFF,

    ISF_LEN(user_authentication_key), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(user_authentication_key)),
    ISF_ID(user_authentication_key),
    ISF_MOD(user_authentication_key),
    SPLIT_SHORT_LE(ISF_BASE(user_authentication_key)),
    0xFF, 0xFF,

    ISF_LEN(routing_code), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(routing_code)),
    ISF_ID(routing_code),
    ISF_MOD(routing_code),
    SPLIT_SHORT_LE(ISF_BASE(routing_code)),
    0xFF, 0xFF,

    ISF_LEN(user_id), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(user_id)),
    ISF_ID(user_id),
    ISF_MOD(user_id),
    SPLIT_SHORT_LE(ISF_BASE(user_id)),
    0xFF, 0xFF,

    ISF_LEN(optional_command_list), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(optional_command_list)),
    ISF_ID(optional_command_list),
    ISF_MOD(optional_command_list),
    SPLIT_SHORT_LE(ISF_BASE(optional_command_list)),
    0xFF, 0xFF,

    ISF_LEN(memory_size), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(memory_size)),
    ISF_ID(memory_size),
    ISF_MOD(memory_size),
    SPLIT_SHORT_LE(ISF_BASE(memory_size)),
    0xFF, 0xFF,

    ISF_LEN(table_query_size), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(table_query_size)),
    ISF_ID(table_query_size),
    ISF_MOD(table_query_size),
    SPLIT_SHORT_LE(ISF_BASE(table_query_size)),
    0xFF, 0xFF,

    ISF_LEN(table_query_results), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(table_query_results)),
    ISF_ID(table_query_results),
    ISF_MOD(table_query_results),
    SPLIT_SHORT_LE(ISF_BASE(table_query_results)),
    0xFF, 0xFF,

    ISF_LEN(hardware_fault_status), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(hardware_fault_status)),
    ISF_ID(hardware_fault_status),
    ISF_MOD(hardware_fault_status),
    SPLIT_SHORT_LE(ISF_BASE(hardware_fault_status)),
    0xFF, 0xFF,

    ISF_LEN(external_events_list), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(external_events_list)),
    ISF_ID(external_events_list),
    ISF_MOD(external_events_list),
    SPLIT_SHORT_LE(ISF_BASE(external_events_list)),
    0xFF, 0xFF,

    ISF_LEN(external_events_alarm_list), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(external_events_alarm_list)),
    ISF_ID(external_events_alarm_list),
    ISF_MOD(external_events_alarm_list),
    SPLIT_SHORT_LE(ISF_BASE(external_events_alarm_list)),
    0xFF, 0xFF,

    ISF_LEN(application_extension), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(application_extension)),
    ISF_ID(application_extension),
    ISF_MOD(application_extension),
    SPLIT_SHORT_LE(ISF_BASE(application_extension)),
    0xFF, 0xFF,
};




/// This array contains stock codes for isfs.  They are ordered strings.
#if (CC_SUPPORT == GCC)
__attribute__((section(".vl_isfs")))
#elif (CC_SUPPORT == CL430)
#pragma DATA_SECTION(isfs_stock_codes, ".vl_isfs")
#endif
const ot_u8 isfs_stock_codes[] = {
    0x10, 0x11, 0x18, 0xFF,
    0x12, 0x13, 0x14, 0x17, 0xFF, 0xFF,
    0x15, 0xFF,
    0x16, 0xFF,
    0x00, 0x01,
    0x01, 0x06, 0x07, 0x17,
    0x02, 0x03, 0x04, 0x05,
    0x11, 0xFF,
};


#if (GFB_TOTAL_BYTES > 0)
#if (CC_SUPPORT == GCC)
__attribute__((section(".vl_gfb")))
#elif (CC_SUPPORT == CL430)
#pragma DATA_SECTION(gfb_stock_files, ".vl_gfb")
#endif
const ot_u8 gfb_stock_files[] = {0xFF, 0xFF};
#endif




/// Firmware & Version information for ISF1 (Device Features)
/// This will look something like "OTv1  xyyyyyyy" where x is a letter and
/// yyyyyyy is a Base64 string containing a 16 bit build-id and a 32 bit mask
/// indicating the features compiled-into the build.
#include "OT_version.h"

#define BV0     (ot_u8)(OT_VERSION_MAJOR + 48)
#define BT0     (ot_u8)(OT_BUILDTYPE)
#define BC0     OT_BUILDCODE0
#define BC1     OT_BUILDCODE1
#define BC2     OT_BUILDCODE2
#define BC3     OT_BUILDCODE3
#define BC4     OT_BUILDCODE4
#define BC5     OT_BUILDCODE5
#define BC6     OT_BUILDCODE6
#define BC7     OT_BUILDCODE7

/// This array contains the stock ISF data.  ISF data must be big endian!
#if (CC_SUPPORT == GCC)
__attribute__((section(".vl_isf")))
#elif (CC_SUPPORT == CL430)
#pragma DATA_SECTION(isf_stock_files, ".vl_isf")
#endif
const ot_u8 isf_stock_files[] = {
    /* network settings: id=0x00, len=8, alloc=8 */
    __VID,                                              /* VID */
    0x11,                                               /* Device Subnet */
    0x11,                                               /* Beacon Subnet */
    SPLIT_SHORT(OT_ACTIVE_SETTINGS),                    /* Active Setting */
    0x00,                                               /* Default Device Flags */
    3,                                                  /* Beacon Attempts */
    SPLIT_SHORT(2),                                     /* Hold Scan Sequence Cycles */

    /* device features: id=0x01, len=46, alloc=46 */
    __UID,                                              /* UID: 8 bytes*/
    SPLIT_SHORT(OT_SUPPORTED_SETTINGS),                 /* Supported Setting */
    M2_PARAM(MAXFRAME),                                 /* Max Frame Length */
    1,                                                  /* Max Frames per Packet */
    SPLIT_SHORT(0),                                     /* DLLS Methods */
    SPLIT_SHORT(0),                                     /* NLS Methods */
    SPLIT_SHORT(ISF_TOTAL_BYTES),                       /* ISFB Total Memory */
    SPLIT_SHORT(ISF_TOTAL_BYTES-ISF_HEAP_BYTES),        /* ISFB Available Memory */
    SPLIT_SHORT(ISFS_TOTAL_BYTES),                      /* ISFSB Total Memory */
    SPLIT_SHORT(ISFS_TOTAL_BYTES-ISFS_HEAP_BYTES),      /* ISFSB Available Memory */
    SPLIT_SHORT(GFB_TOTAL_BYTES),                       /* GFB Total Memory */
    SPLIT_SHORT(GFB_TOTAL_BYTES-GFB_HEAP_BYTES),        /* GFB Available Memory */
    SPLIT_SHORT(GFB_FILE_BYTES),                        /* GFB File Size */
    0,                                                  /* RFU */
    OT_FEATURE(SESSION_DEPTH),                          /* Session Stack Depth */
    'O','T','v',BV0,' ',' ',
    BT0,BC0,BC1,BC2,BC3,BC4,BC5,BC6,BC7, 0,             /* Firmware & Version as C-string */

    /* channel configuration: id=0x02, len=32, alloc=64 */
    0x00,                                               /* Channel Spectrum ID */
    0x00,                                               /* Channel Parameters */
    (ot_u8)(( (-15) + 40 )*2),                          /* Channel TX Power Limit */
    (ot_u8)( 100 ),                                     /* Channel Link Quality Filter Level */
    (ot_u8)( (-85) + 140 ),                             /* CS RSSI Threshold */
    (ot_u8)( (-92) + 140 ),                             /* CCA RSSI Threshold*/
    0x00,                                               /* Regulatory Code */
    0x01,                                               /* Duty Cycle (100%) */

    0x10,                                               /* Channel Spectrum ID */
    0x00,                                               /* Channel Parameters */
    (ot_u8)(( (-15) + 40 )*2),                          /* Channel TX Power Limit */
    (ot_u8)( 100 ),                                     /* Channel Link Quality Filter Level */
    (ot_u8)( (-85) + 140 ),                             /* CS RSSI Threshold */
    (ot_u8)( (-92) + 140 ),                             /* CCA RSSI Threshold*/
    0x00,                                               /* Regulatory Code */
    0x01,                                               /* Duty Cycle (100%) */

    0x12,                                               /* Channel Spectrum ID */
    0x00,                                               /* Channel Parameters */
    (ot_u8)(( (-15) + 40 )*2),                          /* Channel TX Power Limit */
    (ot_u8)( 100 ),                                     /* Channel Link Quality Filter Level */
    (ot_u8)( (-85) + 140 ),                             /* CS RSSI Threshold */
    (ot_u8)( (-92) + 140 ),                             /* CCA RSSI Threshold*/
    0x00,                                               /* Regulatory Code */
    0x01,                                               /* Duty Cycle (100%) */

    0x2D,                                               /* Channel Spectrum ID */
    0x00,                                               /* Channel Parameters */
    (ot_u8)(( (-15) + 40 )*2),                          /* Channel TX Power Limit */
    (ot_u8)( 100 ),                                     /* Channel Link Quality Filter Level */
    (ot_u8)( (-80) + 140 ),                             /* CS RSSI Threshold */
    (ot_u8)( (-90) + 140 ),                             /* CCA RSSI Threshold*/
    0x00,                                               /* Regulatory Code */
    0x01,                                               /* Duty Cycle (100%) */

    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,


    /* real time scheduler: id=0x03, len=12, alloc=12 */
    0x00, 0x0F,                                         /* SSS Sync Mask */
    0x00, 0x08,                                         /* SSS Sync Value */
    0x00, 0x03,                                         /* HSS Sync Mask */
    0x00, 0x02,                                         /* HSS Sync Value */
    0x00, 0x03,                                         /* BTS Sync Mask */
    0x00, 0x02,                                         /* BTS Sync Value */

    /* sleep scan periods: id=0x04, len=12, alloc=32 */
    /* Period data format in Section X.9.4.5 of Mode 2 spec */
    0x10, 0x51, 0x0C, 0x00,                             /* Channel X scan, Scan Code, Next Scan ms */
    0xFF, 0xFF, 0xFF, 0xFF,                             /* NOTE: Scan Code should be less than     */
    0xFF, 0xFF, 0xFF, 0xFF,                             /*       Next Scan, or else you will be    */
    0xFF, 0xFF, 0xFF, 0xFF,                             /*       doing nothing except scanning!    */
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,

    /* hold scan periods: id=0x05, len=12, alloc=32 */
    /* Period data format in Section X.9.4.5 of Mode 2 spec */
    0x10, 0x52, 0x00, 0x01,                             /* Channel X scan, Scan Code, Next Scan ms */
    0x10, 0x23, 0x00, 0xA0,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,

    /* beacon transmit periods: id=0x06, len=12, alloc=24 */
    /* Period data format in Section X.9.4.7 of Mode 2 spec */ //0x0240
    0x10, 0x06, 0x20, 0x00, 0x00, 0x08, 0x00, 0x20,     /* Channel X beacon, Beacon ISF File, Next Beacon ms */
    0x10, 0x06, 0x20, 0x00, 0x00, 0x08, 0x00, 0x20,
    0x10, 0x06, 0x20, 0x00, 0x00, 0x08, 0x0B, 0x00,

    /* App Protocol List: id=0x07, len=4, alloc=16 */
    0x00, 0x01, 0x02, 0x04, 0xFF, 0xFF, 0xFF, 0xFF,     /* List of Protocols supported (Tentative)*/
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* ISFS list: id=0x08, len=12, alloc=24 */
    0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x18,
    0x80, 0x81, 0x82, 0x83, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* GFB File List: id=0x09, len=4, alloc=8 */
    0x00, 0x01, 0x02, 0x03, 0xFF, 0xFF, 0xFF, 0xFF,

    /* Location Data List: id=0x0A, len=0, alloc=96 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* IPv6 Addresses: id=0x0B, len=0, alloc=48 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* Sensor List: id=0x0C, len=16, alloc=16 (just dummy values right now) */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x00,

    /* Sensor Alarms: id=0x0D, len=2, alloc=2 (just dummy values right now) */
    0x00, 0x00,

    /* root auth key:       id=0x0E, not used in this build */
    /* Admin auth key:      id=0x0F, not used in this build */

    /* Routing Code: id=0x10, len=0, alloc=50 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF,

    /* User ID: id=0x11, len=0, alloc=60 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,

    /* Mode 1 Optional Command list: id=0x12, len=7, alloc=8 */
    0x13, 0x93, 0x0C, 0x0E, 0x60, 0xE0, 0x8E, 0xFF,

    /* Mode 1 Memory Size: id=0x13, len=12, alloc=12 */
    0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    /* Mode 1 Table Query Size: id=0x14, len=1, alloc=2 */
    0x00, 0xFF,

    /* Mode 1 Table Query Results: id=0x15, len=7, alloc=8 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,

    /* HW Fault Status: id=0x16, len=3, alloc=4 */
    0x00, 0x00, 0x00, 0xFF,

    /* Ext Services List:   id=0x17, not used in this build */
    /* Ext Services Alarms: id=0x18, not used in this build */

    /* Application Extension: id=0xFF, len=0, alloc=16 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};



//__attribute__((section(".vl_fallow")))
//const ot_u8 vl_fallow_space[ (FLASH_PAGE_SIZE*OTF_VWORM_FALLOW_PAGES) ];
