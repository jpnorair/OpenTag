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
  */
/**
  * @file       /apps/test_swcodec/code/main.c
  * @author     JP Norair
  * @version    V1.0
  * @date       16 April 2011
  * @brief      PaLFi Demo Main
  *
  ******************************************************************************
  */

#include "OTAPI.h"          // OTlib top-level stuff
#include "OT_platform.h"    // Platform level stuff



/** Logging for Debugging (not available in release)<BR>
  * ========================================================================<BR>
  */
#if (MPIPE_FOR_DEBUGGING)
#	define OTAPI_LOG_MSG(TYPE, LABEL_LEN, DATA_LEN, LABEL, DATA) \
		otapi_log_msg(TYPE, LABEL_LEN, DATA_LEN, LABEL, DATA)
#else
#	define OTAPI_LOG_MSG(TYPE, LABEL_LEN, DATA_LEN, LABEL, DATA) while(0)
#endif





/** Application Global Variables <BR>
  * ========================================================================<BR>
  */

//ot_bool (*app_task)(void);





/** Applet Functions <BR>
  * ========================================================================
  */
// Main Application Functions
void    app_init();
void    app_manager();
ot_bool app_task_null();



/** Application local subroutines (platform & board dependent) <BR>
  * ============================================================================
  */
void sub_button_init();

void otapi_led1_on() {
// Used by TX encoding process measuring
    GPIO2->DOUT |= GPIO_Pin_0;
}

void otapi_led2_on() {
// Used by RX decoding process measuring
    GPIO2->DOUT |= GPIO_Pin_1;
}

void otapi_led1_off() {
    GPIO2->DOUT &= ~GPIO_Pin_0;
}

void otapi_led2_off() {
    GPIO2->DOUT &= ~GPIO_Pin_1;
}





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






void sub_button_init() {
/// Buttons are attached to the PaLFi core
}






/** User Applet and Button Management Routines <BR>
  * ===========================================================================<BR>
  * The User applet is primarily activated by callbacks from the kernel.  However,
  * in this system some features are also activated by button presses.
  *
  */
void sub_led_cycle(ot_u8 i) {
    switch (i & 3) {
        case 0: PALFI_LED4_ON();        break;
        case 1: PALFI_LED3_ON();        break;
        case 2: PALFI_LED2_ON();        break;
        case 3: PALFI_LED1_ON();        break;
    }

    platform_swdelay_ms(33);

    switch (i & 3) {
        case 0: PALFI_LED4_OFF();       break;
        case 1: PALFI_LED3_OFF();       break;
        case 2: PALFI_LED2_OFF();       break;
        case 3: PALFI_LED1_OFF();       break;
    }
}

void app_init() {
    ot_u8 i;

    /// blink the LEDs
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
    
    otapi_led1_off();
    otapi_led2_off();
    
    //app_task = &app_task_null;
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



void app_sleep() {
#if (!defined(DEBUG_ON) && !defined(__DEBUG__))
    if ((sys.evt.EXT.eventno != 0) || (sys.mutex != 0)) {
        // LPM0/1 if there's MPipe, LPM0/1/2 if not
        SLEEP_MCU();    
    }
    else {
        // You could do LPM4 if you want the system to wake-up only through
        // PalFi wake-ups or button presses.  LPM3 allows RTC and Kernel timer
        // to stay enabled.
        PMM_EnterLPM3();
    }
#else
    SLEEP_MCU();
#endif
}





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
	OTAPI_LOG_MSG(MSG_raw, 3, 2, (ot_u8*)"WTF", (ot_u8*)&code);
}


void sys_sig_rfainit(ot_int pcode) {
/// This is a static callback.   The kernel uses it when it is starting up a 
/// radio process.  It is used here to turn-on activity LEDs.

    // Assume that (1 <= code1 <= 5), which is the case in normal operation
    if (pcode < 3)  PALFI_LED4_ON();
    else            PALFI_LED3_ON();
}



void sys_sig_rfaterminate(ot_int pcode, ot_int scode) {
/// This is a static callback.   The kernel uses it when it is finishing a 
/// radio process.  It is used here to turn-off activity LEDs.
    PALFI_LED4_OFF();
    PALFI_LED3_OFF();
}





void network_sig_route(ot_int code, ot_int protocol) {
#if (MPIPE_FOR_DEBUGGING)
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
					(ot_u8*)&(labels[protocol][1]),
					rxq.front	);
#endif
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
  *
  * As with all OpenTag callbacks, you have the option to compile them as
  * dynamic callbacks (function pointers that you assign during runtime) or as
  * static callbacks (specific functions).  In this demo app, static callbacks
  * are used.  Static callbacks are better, as long as you don't need to change
  * the feature attached to a callback.
  */

ot_bool m2qp_sig_udpreq(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
    return False;
}

ot_bool m2qp_sig_errresp(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
    return False;
}

ot_bool m2qp_sig_stdresp(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
    return False;
}

ot_bool m2qp_sig_a2presp(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
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
    otapi_poweron();
    otapi_init();


    ///3. Top-level application init
    ///   In this test, the top-level application does very little.
    app_init();

    OTAPI_LOG_MSG(MSG_utf8, 6, 26, (ot_u8*)"SYS_ON", (ot_u8*)"System on and Mpipe active");

    ///5. MAIN RUNTIME (post-init)  <BR>
    ///<LI> a. Pre-empt the kernel (first run)   </LI>
    ///<LI> b. Go to sleep; OpenTag kernel will run automatically in
    ///        the background  </LI>
    ///<LI> c. The kernel has a built-in applet loader.  It is the best way
    ///        to use applets that generate requests or manipulate the system.
    ///        The applets only load during full-idle, so time slotting or
    ///        any other type of MAC activity is not affected. </LI>
    ///<LI> d. 99.99% (or more) of the time, the kernel is not actually
    ///        running.  You can run parallel, local tasks alongside OpenTag
    ///        as long as they operate above priority 1.  (I/O is usually
    ///        priority 0 and kernel is always priority 1) </LI>
    otapi_preempt();
    while(1) {
        app_manager();
        app_sleep();
    }

    ///6. Note on manually pre-empting the kernel for you own purposes:
    ///   It can be done (many internal tasks do it), but be careful.
    ///   It is recommended that you only do it when sys.mutex <= 1
    ///   (i.e. no radio data transfer underway).  One adaptation of
    ///   this demo is to have the mode-switching applets pre-empt the
    ///   kernel (it works fine, but you need to make your own loader
    ///   instead of using sys.loadapp).
}






// This is the typical default data setup.  You can change this to another
// file, but also make sure to adjust app_config.h to match
#include "data_default.c"

