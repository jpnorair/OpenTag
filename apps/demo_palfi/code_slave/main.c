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
  * @version    R100
  * @date       10 October 2011
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
#   define WAIT_FOR_MPIPE() while(0)
#	define OTAPI_LOG_MSG(TYPE, LABEL_LEN, DATA_LEN, LABEL, DATA) \
		otapi_log_msg(TYPE, LABEL_LEN, DATA_LEN, LABEL, DATA)
#else
#   define WAIT_FOR_MPIPE() while(0)
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
void app_init();



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
	i &= 3;
	switch (i) {
	case 0: PALFI_LED4_ON();	break;
	case 1: PALFI_LED3_ON();	break;
	case 2: otapi_led2_on();	break;
	case 3: otapi_led1_on();	break;
	}

    platform_swdelay_ms(33);

    switch (i) {
    case 0: PALFI_LED4_OFF();	break;
    case 1: PALFI_LED3_OFF();	break;
    case 2: otapi_led2_off();	break;
    case 3: otapi_led1_off();	break;
    }
}


void app_init() {
    ot_u8 i;

    /// blink the LEDs
    i = 255;
    do { sub_led_cycle(++i); } while (i != 3);
    do { sub_led_cycle(--i); } while (i != 0);

    ///Initialize the input buttons
    sub_button_init();
}








/** OTlib/OTAPI Callback Applets  <BR>
  * ========================================================================<BR>
  * This app uses some of the "std" applets from /otlibext/applets_std
  * The applets used are selected in extf_config.h.  They are implemented in
  * other C files, shown below.  The idea is that you pick the applet you want
  * in your makefile (or project), but they are all stored in the same folder.
  */




/** Transport Layer Callback Applet Links  <BR>
  * =======================================================================<BR>
  * The user application naturally connects to the Transport Layer, which
  * provides the application with processed data payloads.
  *
  * At the moment, only the default DASH7 implementation is used.  A callback
  * to CoAP (over UDP) will be included in a later version.
  */







/** User ALP Processor  <BR>
  * =======================================================================<BR>
  * If using proprietary/custom ALPs, the processor should be implemented here.
  * There is an example implementation below, which can be uncommented to match
  * the example in the API Quickstart Guide:
  * http://www.indigresso.com/wiki/doku.php?id=opentag:api:quickstart
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

    WAIT_FOR_MPIPE();

    ///4b. Load a message to show that main startup has passed
    OTAPI_LOG_MSG(MSG_utf8, 6, 26, (ot_u8*)"SYS_ON", (ot_u8*)"System on and Mpipe active");

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






// This is the typical default data setup.  You can change this to another
// file, but also make sure to adjust app_config.h to match
#include "data_default.c"

