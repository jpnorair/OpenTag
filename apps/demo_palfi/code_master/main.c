/*  Copyright 2010-2011, JP Norair
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
  */
/**
  * @file       /apps/Demo_PaLFi/Code_Master/main.c
  * @author     JP Norair
  * @version    V1.0
  * @date       16 April 2011
  * @brief      PaLFi Demo Main
  *
  * This Demonstration Intends to Show:
  * <LI> Basic DASH7 features                                           </LI>
  * <LI> Using CoAP with DASH7, inside UDP Shell transport commands     </LI>
  * <LI> Interfacing an MCU with a TI 3705                              </LI>
  * <LI> Basic PaLFi Master functions, usable via client GUI            </LI>
  *
  * Common Variations:
  *
  * This Application Requires:
  * <LI> An MSP430F5-EXP5529 board, or something similar                </LI>
  * <LI> An RI-ACC-ADR2 board, or something similar                     </LI>
  * <LI> Minimum 32KB Flash, 2KB SRAM                                   </LI>
  * <LI> Minimum Two LEDs to show RX/TX activity                        </LI>
  *
  * Currently Supported Boards:
  * <LI> MSP430F5-EXP5529 + RI-ACC-ADR2               </LI>
  *
  * @note Different boards may support different methods of input and output.
  ******************************************************************************
  */

#include "OTAPI.h"
#include "OT_platform.h"


// These should probably get integrated into OTAPI at some point
//#include "mpipe.h"
//#include "m2_transport.h"
//#include "m2_network.h"
//#include "system_native.h"
//#include "veelite.h"




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

#if (MCU_FEATURE(MPIPEVCOM) == ENABLED)
    volatile ot_u8 app_usbhold = 0;
#endif
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





#if defined(BOARD_TIProto_EXP5529)

/** Application Triggers & Button(s) <BR>
  * ========================================================================<BR>
  * The RX and TX indicators are part of the mainline platform code, using the
  * two required platform triggers.  All boards must specify at least two
  * "triggers" (i.e. output indicators, usually LEDs).
  *
  * This app requires FOUR triggers.  Therefore, triggers 3 and 4 are defined
  * below.  They are specific to this app.  Change them if you want.
  *
  * The Application button is used as an interrupt source.  For now, it also
  * changes modes.
  */
#define APP_TRIG3_PORTNUM	1
#define APP_TRIG3_PORT      GPIO1
#define APP_TRIG3_PIN       GPIO_Pin_0
#define APP_TRIG4_PORTNUM	1
#define APP_TRIG4_PORT      GPIO1
#define APP_TRIG4_PIN       GPIO_Pin_1

#define APP_BUTTON_PORTNUM  1
#define APP_BUTTON_VECTOR	PORT1_VECTOR
#define APP_BUTTON_PORT     GPIO1
#define APP_BUTTON_PIN      GPIO_Pin_7
#define APP_BUTTON_POL      0


/** LED routines, plus a process to show that initialization has succeeded <BR>
  * ===========================================================================
  * The RX and TX indicators are part of the mainline platform code.  All boards
  * must specify at least two "triggers" (i.e. output indicators, usually LEDs).
  * This app requires FOUR triggers.  Triggers 3 and 4 are defined in this app,
  */
void sub_trig3_high() {    APP_TRIG3_PORT->DOUT |= APP_TRIG3_PIN; }
void sub_trig3_low() {     APP_TRIG3_PORT->DOUT &= ~APP_TRIG3_PIN; }
void sub_trig3_toggle() {  APP_TRIG3_PORT->DOUT ^= APP_TRIG3_PIN; }
void sub_trig4_high() {    APP_TRIG4_PORT->DOUT |= APP_TRIG4_PIN; }
void sub_trig4_low() {     APP_TRIG4_PORT->DOUT &= ~APP_TRIG4_PIN; }
void sub_trig4_toggle() {  APP_TRIG4_PORT->DOUT ^= APP_TRIG4_PIN; }

void sub_trig_init() {
#if (APP_TRIG3_PORTNUM == APP_TRIG4_PORTNUM)
    APP_TRIG3_PORT->DDIR    |= (APP_TRIG3_PIN | APP_TRIG4_PIN);
    APP_TRIG3_PORT->DS      |= (APP_TRIG3_PIN | APP_TRIG4_PIN);

#else
    APP_TRIG3_PORT->DDIR    |= APP_TRIG3_PIN;
    APP_TRIG3_PORT->DS      |= APP_TRIG3_PIN;
    APP_TRIG4_PORT->DDIR    |= APP_TRIG4_PIN;
    APP_TRIG4_PORT->DS      |= APP_TRIG4_PIN;

#endif
}

void sub_button_init() {
    // Pin 1.7  = "S1" button, the interrupt source
	// Apply pull-up/down resistor.  The MSP430 method to do this is strange.
	// The DOUT needs to be set to 1/0 for pull-up/pull-down.  Lines of code
	// that are startup default are commented-out for optimization.
  //APP_BUTTON_PORT->DDIR  &= ~APP_BUTTON_PIN;
	APP_BUTTON_PORT->REN   |= APP_BUTTON_PIN;
#   if (APP_BUTTON_POL == 1)
	  //APP_BUTTON_PORT->DOUT  &= ~APP_BUTTON_PIN;
#   else
	    APP_BUTTON_PORT->DOUT  |= APP_BUTTON_PIN;
#   endif

    // Check if button is hard-wired or depressed at startup
#   if (APP_BUTTON_POL == 1)
	if (APP_BUTTON_PORT->DIN & APP_BUTTON_PIN)
#   else
    if ((APP_BUTTON_PORT->DIN & APP_BUTTON_PIN) == 0)
#   endif
    {
#		if (MCU_FEATURE_MPIPEVCOM == ENABLED)
        	app_usbhold = 1;
#		endif
    }

    // Configure Button interrupt to happen when the button is released.
    // Positive/Negative polarity means Falling/Rising Edge detection.
#   if (APP_BUTTON_POL == 1)
        APP_BUTTON_PORT->IES   |= APP_BUTTON_PIN;
#   else
      //APP_BUTTON_PORT->IES   &= ~APP_BUTTON_PIN;
#   endif
    APP_BUTTON_PORT->IFG    = 0;
    APP_BUTTON_PORT->IE    |= APP_BUTTON_PIN;
}

#if (CC_SUPPORT == CL430)
#   pragma vector=APP_BUTTON_VECTOR
#elif (CC_SUPPORT == IAR_V5)
    // don't know yet
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(APP_BUTTON_VECTOR)
#endif
OT_INTERRUPT void app_buttons_isr(void) {
    APP_BUTTON_PORT->IFG	= 0;

#	if (MCU_FEATURE_MPIPEVCOM == ENABLED)
    app_usbhold 			= 1;
#	endif

    LPM4_EXIT;
}


/// USB Stick
#elif defined(BOARD_RF430USB_5509)
#include "_patch_5509stick/extf_code.c"
void sub_trig3_high()   { }
void sub_trig3_low()    { }
void sub_trig3_toggle() { }
void sub_trig4_high()   { }
void sub_trig4_low()    { }
void sub_trig4_toggle() { }

void sub_trig_init() {
/// Configure Timer0 Pin to generate small duty cycle on LED pin.  Amazingly,
/// just 1/32 duty cycle gives enough light and offers good lo/hi contrast.
	TIM0A5->CTL   = TIMA_FLG_TACLR | 0x01C0 | 0x0010;  //up mode
	TIM0A5->EX0   = 0;
	TIM0A5->CCTL1 = OUTMOD_3;
	TIM0A5->CCR1  = 31;
	TIM0A5->CCR0  = 32;
	TIM0A5->CTL  &= ~TIMA_FLG_TACLR;
}

void sub_button_init()  { }





#else
#   error "A known board is not defined in platform_config.h"

#endif





/** User Applet and Button Management Routines <BR>
  * ===========================================================================<BR>
  * The User applet is primarily activated by callbacks from the kernel.  However,
  * in this system some features are also activated by button presses.
  *
  */
void sub_led_cycle(ot_u8 i) {
}

void app_init() {
    ot_u8 i;

    ///Initialize Application Triggers (LEDs) and blink the LEDs
    sub_trig_init();

    i = 4;
    while (--i != 0) {
    	otapi_led2_on();
    	platform_swdelay_ms(33);
    	otapi_led2_off();
    	otapi_led1_on();
    	platform_swdelay_ms(33);
    	otapi_led1_off();
    }

    ///Initialize the input buttons
    sub_button_init();
    //app_trig4_high();

    ///Attribute OpenTag Kernel Callbacks with App routines
    ///The idle event prestart callbacks are not used in this application.
#	if (OT_FEATURE(SYSRF_CALLBACKS) == ENABLED)
    	sys.evt.RFA.init        = &app_radio_init;
    	sys.evt.RFA.terminate   = &app_radio_term;
#	endif

    ///Atribute OpenTag Network Routing Callback
#	if (OT_FEATURE(M2NP_CALLBACKS) == ENABLED)
    	m2np.signal.route	= &app_packet_routing;
#	endif

    ///Attribute OpenTag Transport Callbacks with App routines
#	if (OT_FEATURE(M2QP_CALLBACKS) == ENABLED)
    	m2qp.signal.shell_request   = &app_udp_request;
    	m2qp.signal.error_response  = &app_error_response;
    	m2qp.signal.std_response    = &app_std_response;
    	m2qp.signal.a2p_response    = &app_a2p_response;
#	endif
    
    ///Dynamic Mpipe Callbacks: Set MPipe to go back to listen after TX.
#   if (OT_FEATURE(MPIPE_CALLBACKS) == ENABLED)
        mpipe_setsig_txdone(&otapi_ndef_idle);
        mpipe_setsig_rxdone(&otapi_ndef_proc);
        otapi_ndef_idle(0);
#   endif

    //app_task = &app_task_null;
}




///Attribute OpenTag Kernel Callbacks with App routines
///The idle event prestart callbacks are not used in this application.
#if (OT_FEATURE(SYSRF_CALLBACKS) != ENABLED)

#endif

///Atribute OpenTag Network Routing Callback
#if (OT_FEATURE(M2NP_CALLBACKS) != ENABLED)

#endif

///Attribute OpenTag Transport Callbacks with App routines
#if (OT_FEATURE(M2QP_CALLBACKS) != ENABLED)

#endif

/// Static Mpipe Callbacks: Set MPipe to go back to listen after TX.
#if (OT_FEATURE(MPIPE_CALLBACKS) != ENABLED)
void mpipe_sig_txdone(ot_int code) 	    { otapi_ndef_idle(code); }
void mpipe_sig_rxdone(ot_int code) 	    { otapi_ndef_proc(code); }
#endif







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


/*
ot_bool app_send_query() {
/// The C-API for building commands can be bypassed in favor of directly
/// putting data to the queue.  That way is more efficient, but it also requires
/// you to know more about DASH7 that just what order the templates should be.

/// The query that we build will collect sensor configuration data back from
/// all devices that support the sensor protocol.  Much more interesting queries
/// are possible.
    ot_bool output = False;

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
    otapi_start_dialog(); //don't need this, because using internal caller
    output = True;

    sys.loadapp = &sys_loadapp_null;
    return output;
}
*/












/** Kernel Callback Applet Links  <BR>
  * =======================================================================<BR>
  * The Kernel (System Module) has a few callbacks.  Most applications are
  * easier to attach to the Transport Layer, but the Kernel Callbacks can
  * be useful for certain things.
  *
  * As with all OpenTag callbacks, you have the option to compile them as
  * dynamic callbacks (function pointers that you assign during runtime) or as
  * static callbacks (specific functions).  In this demo app, static callbacks
  * are used.  Static callbacks are better, as long as you don't need to change
  * the feature attached to a callback.
  */
void sys_sig_panic(ot_int code) {
/// Kernel panic.  You could have it print something out, or blink LEDs, or
/// whatever.  The system function sys_panic() does the important shutdown
/// routines, so this callback is just for reporting purposes
	otapi_log_msg(MSG_raw, 3, 2, (ot_u8*)"WTF", (ot_u8*)&code);
}

void sys_sig_rfainit(ot_int code1) {
/// This is a callback the kernel uses when it is starting up a radio process.
/// It is used here to turn-on activity LEDs.

    // Assume that (1 <= code1 <= 5), which is the case in normal operation
    if (code1 < 3)  otapi_led2_on();
    else            otapi_led1_on();
}


void sys_sig_rfaterminate(ot_int code1, ot_int code2) {
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
    otapi_log_msg(MSG_raw, 8, logdata_len, loglabel, logdata);

    /// In all cases, turn-off the LEDs
    app_radio_term_end:
    otapi_led2_off();   //Orange LED off
    otapi_led1_off();   //Green LED off
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
void network_sig_route(ot_int code, ot_int type) {

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
/// Transport Layer calls this when a UDP-class request has been received.
    ot_u8* payload_end;

    // Check Source Port (our application uses 0x71)
    if (payload[0] != 0x71) {
        return False;
    }

    otapi_log_header(/*2*/ 1, 0);   //UTF8 (text log) is subcode 1, dummy length is 0
    dir_out.putcursor      += slistf(dir_out.putcursor, "ID:", 'x', id->length, id->value);
    *dir_out.putcursor++    = '\n';
    payload                += 2;
    payload_length         -= 2;
    payload_end             = payload + payload_length;

    while (payload < payload_end) {
        switch (*payload++) {
        // PaLFi data load: 8 bytes
        case 'D':   dir_out.putcursor  += slistf(dir_out.putcursor, "Data:", 'x', 8, payload);
                    payload            += 8;
                    break;

        // Event-type information (1 byte)
        case 'E':   dir_out.putcursor  += slistf(dir_out.putcursor, "Event:", 't', 1, payload);
                    payload++;
                    break;

        // PaLFi RSSI bytes (3 bytes)
        case 'R':   dir_out.putcursor  += slistf(dir_out.putcursor, "RSSI:", 'b', 3, payload);
                    payload            += 3;
                    break;

        // Temperature (2 bytes)
        case 'T':   dir_out.putcursor  += slistf(dir_out.putcursor, "Temp:", 's', 1, payload);
                    payload            += 2; 
                    break;

        case 'V':   dir_out.putcursor  += slistf(dir_out.putcursor, "Volt:", 's', 1, payload);
                    payload            += 2;
                    break;

        default:    continue;
        }
        *dir_out.putcursor++   = '\n';
    }

    dir_out.front[2] = dir_out.putcursor - dir_out.front - 6;
    otapi_log_direct(dir_out.front);

    return True;
}


ot_bool app_error_response(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
/// Called when an error response type is received.
    otapi_log_msg(MSG_raw, 6, payload_length, (ot_u8*)"RX_ERR", payload);
    return False;
}

ot_bool app_std_response(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
/// Called when a non-arbitrated response is received by the Transport Layer.
    otapi_log_msg(MSG_raw, 7, payload_length, (ot_u8*)"RX_RESP", payload);
    return False;
}

ot_bool app_a2p_response(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
/// Called when an Arbitrated Two Party (A2P) response is received.
/// Not part of the application at the moment
	return False;
}









#ifdef EXTF_m2qp_sig_errresp
ot_bool m2qp_sig_errresp(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
	return app_error_response(id, payload_length, payload);
}
#endif

#ifdef EXTF_m2qp_sig_stdresp
ot_bool m2qp_sig_stdresp(id_tmpl* id, ot_int payload_length, ot_u8* payload){
	return app_std_response(id, payload_length, payload);
}
#endif

#ifdef EXTF_m2qp_sig_a2presp
ot_bool m2qp_sig_a2presp(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
	return app_a2p_response(id, payload_length, payload);
}
#endif

#ifdef EXTF_m2qp_sig_dsresp
ot_bool m2qp_sig_dsresp(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
	return app_ds_response(id, payload_length, payload);
}
#endif

#ifdef EXTF_m2qp_sig_dsack
ot_bool m2qp_sig_dsack(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
	return app_ds_ack(id, payload_length, payload);
}
#endif

#ifdef EXTF_m2qp_sig_udpreq
ot_bool m2qp_sig_udpreq(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
	return app_udp_request(id, payload_length, payload);
}
#endif











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

    
#   if ((MCU_FEATURE_MPIPEVCOM) && !defined(BOARD_RF430USB_5509))
        ///4a. If USB is used for MPipe, we need to wait for the driver on
        ///    the host to be ready.  Push the "Key" to exit sleep & resume.
        ///    Also, the kernel is officially unattached during the process.
        platform_flush_gptim();
        while (app_usbhold == 0) {
            SLEEP_MCU();
        }

#   elif defined(BOARD_RF430USB_5509)
        ///4a. USB must be used for MPipe, and there is no easily accessible
        ///    button, so just wait for a few seconds and expect the client to
        ///    settle the USB driver loading by that time.
        platform_swdelay_ms(5000);
#   endif


    ///4b. Send a message to show that main startup has passed.  You can use a
    ///    transmission like this to start-up the Mpipe NDEF console, or you
    ///    can alternatively call otapi_ndef_idle(0)
    otapi_log_msg(MSG_utf8, 6, 26, (ot_u8*)"SYS_ON", (ot_u8*)"System on and Mpipe active");
    platform_swdelay_ms(5);

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


#if defined(BOARD_RF430USB_5509)
#   include "_patch_5509stick/data_5509.c"

#else
//#   include "data.c"
#   include "_patch_5509stick/data_5509.c"

#endif
