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
  * @file       /apps/demo_palfi/code_slave/main.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2011
  * @brief      PaLFi Demo Main
  *
  * This Demonstration Intends to Show:
  * <LI> Basic DASH7 features                                           </LI>
  * <LI> Using CoAP with DASH7, inside UDP Shell transport commands     </LI>
  * <LI> Interfacing OpenTag/DASH7 with a PaLFi Transponder             </LI>
  *
  * Common Variations:
  *
  * This Application Requires:
  * <LI> A TI RF430F5978 device, typically on a Fob board               </LI>
  * <LI> Minimum 32KB Flash, 2KB SRAM                                   </LI>
  * <LI> Minimum Two LEDs to show RX/TX activity                        </LI>
  * <LI> Minimum One LED to show PaLFi actions                          </LI>
  *
  * Currently Supported Boards:
  * <LI> RF430F5978 Keyfob board               </LI>
  *
  * @note
  * As there is really only one chip on the planet that can perform this demo,
  * most multi-board support code that is in other demo apps has been removed.
  * So, this would be a bad demo to use as a template unless you are developing
  * a PaLFi application, in which case it is a great template.
  ******************************************************************************
  */

#include "OTAPI.h"          // OTlib top-level stuff
#include "OT_platform.h"    // Platform level stuff
#include "palfi.h"          // palfi thread & app



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
        case 2: otapi_led2_on();   		break;
        case 3: otapi_led1_on();   		break;
    }

    platform_swdelay_ms(33);

    switch (i & 3) {
        case 0: PALFI_LED4_OFF();       break;
        case 1: PALFI_LED3_OFF();       break;
        case 2: otapi_led2_off();       break;
        case 3: otapi_led1_off();       break;
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





/** OTlib/OTAPI Callback Applets  <BR>
  * ========================================================================<BR>
  * This app uses some of the "std" applets from /otlibext/applets_std
  * The applets used are selected in extf_config.h
  */

void sys_sig_rfaterminate(ot_int pcode, ot_int scode) {
    otapi_led2_off();   //Orange LED off
    otapi_led1_off();   //Green LED off
}



/** Transport Layer Callback Applet Links  <BR>
  * =======================================================================<BR>
  * The user application naturally connects to the Transport Layer, which
  * provides the application with processed data payloads.
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

    ///3. Palfi Application Initialization
    ///   Palfi app acts as an interupt-driven thread.  An interrupt pre-empts
    ///   the kernel and seeds the thread.  After this, the kernel takes-over
    ///   the execution of the thread.  This function enables these driving
    ///   interrupts.
    palfi_init();

    ///4. Top-level application init
    ///   In this demo, the top-level application does very little.
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

