/*  Copyright 2010-2012, JP Norair
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
  * @file       /apps/demo_palfi/code_master/main.c
  * @author     JP Norair
  * @version    R100
  * @date       10 October 2012
  * @brief      PaLFi Demo Main
  *
  * This Demonstration Intends to Show:
  * <LI> Basic DASH7 features                                           </LI>
  * <LI> Using CoAP with DASH7, inside UDP Shell transport commands     </LI>
  * <LI> Interfacing an MCU with a TI 3705                              </LI>
  * <LI> Basic PaLFi Master functions, usable via client GUI            </LI>
  *
  * Common Variations: (none)
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
  * Stock Applets: <BR>
  * Many of the stock applets from /otlibext/stdapplets/ are used as callbacks
  * from the protocol layers.  You can look at extf_config.h to see which ones
  * are enabled (or, to specify new ones).
  *
  ******************************************************************************
  */

#include "OTAPI.h"
#include "OT_platform.h"
//#include "radio.h"






/** Application Global Variables <BR>
  * ========================================================================<BR>
  * app_usbhold must be volatile, or else the ISR code that should set it 
  * might get optimized-out during compilation.
  */

#if (MCU_FEATURE(MPIPECDC) == ENABLED)
    volatile ot_u8 app_usbhold = 0;
#endif
//ot_bool (*app_task)(void);





/** Applet Functions (platform & board independent) <BR>
  * ========================================================================
  */
// Main Application Functions
void app_init();
void app_invoke(ot_u8 call_type);




/** Application local subroutines (platform & board dependent) <BR>
  * ============================================================================
  */
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
#define APP_TRIG3_PORTNUM   1
#define APP_TRIG3_PORT      GPIO1
#define APP_TRIG3_PIN       GPIO_Pin_0
#define APP_TRIG4_PORTNUM   1
#define APP_TRIG4_PORT      GPIO1
#define APP_TRIG4_PIN       GPIO_Pin_1

#define APP_BUTTON_PORTNUM  1
#define APP_BUTTON_VECTOR   PORT1_VECTOR
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
/// Configure Timer1 Pin to generate small duty cycle on LED pin.  Amazingly,
/// just 1/32 duty cycle gives enough light and offers good lo/hi contrast.
    TIM1A3->CTL   = TIMA_FLG_TACLR | 0x01C0 | 0x0010;  //up mode
    TIM1A3->EX0   = 0;
    TIM1A3->CCTL1 = OUTMOD_3;
    TIM1A3->CCR1  = 31;
    TIM1A3->CCR0  = 32;
    TIM1A3->CTL  &= ~TIMA_FLG_TACLR;
}

void sub_button_init()  { }



#else
#   error "A known board is not defined in platform_config.h"

#endif





/** User Applet and Button Management Routines <BR>
  * ===========================================================================<BR>
  * The User applet is primarily activated by callbacks from the kernel.  
  * However, in this system some features are also activated by button presses.
  */
static const ot_sub led_on[2] = {   &platform_trig2_high,
                                    &platform_trig1_high    };

static const ot_sub led_off[2] = {  &platform_trig2_low,
                                    &platform_trig1_low     };

void sub_led_cycle(ot_u8 i) {
    led_on[i&1]();
    platform_swdelay_ms(33);
    led_off[i&1]();
}


void app_init() {
    ot_u8 i;

    ///Initialize Application Triggers (LEDs) and blink the LEDs
    sub_trig_init();

    i = 255;
    do { sub_led_cycle(++i); } while (i != 3);
}







/** OTlib/OTAPI Callback Applets  <BR>
  * ========================================================================<BR>
  * This app uses some of the "std" applets from /otlibext/applets_std
  * The applets used are selected in extf_config.h.  They are implemented in
  * other C files, shown below.  The idea is that you pick the applet you want
  * in your makefile (or project), but they are all stored in the same folder.
  *
  * Typical Reporting Applets Used
  * sys_sig_panic()				/otlibext/applets_std/sys_sig_panic.c
  * sys_sig_rfainit()           /otlibext/applets_std/sys_sig_rfainit.c
  * sys_sig_rfaterminate()      /otlibext/applets_std/sys_sig_rfaterminate_2.c
  *
  * Typical Protocol Applets Used
  * m2qp_sig_udp()              local (below)
  */


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
  */

#ifdef EXTF_m2qp_sig_error
ot_bool m2qp_sig_error(ot_u8 code, ot_u8 subcode, id_tmpl* user_id) {
    otapi_log_msg(MSG_raw, 6, rxq.front, (ot_u8*)"RX_ERR", rxq.length);
    return False;
}
#endif


#ifdef EXTF_m2qp_sig_udp
ot_bool m2qp_sig_udp(ot_u8 srcport, ot_u8 dstport, id_tmpl* user_id) {
/// Transport Layer calls this when a UDP-class request has been received.
    static const char* label[] = { "ID:", "Data:", "Event:", "RSSI:", "Temp:", "Volt:" };
    static const char type[]   = {  'x' ,   'x'  ,   't'   ,   'b'  ,   's'  ,   's'   };
    static const ot_int len[]  = {   8  ,    8   ,    1    ,    3   ,    2   ,    2    };
    ot_int scratch;
    ot_u8  index;
    ot_u8  input_len;
    ot_u8* input_data;

    // Check Source & Destination Ports of UDP message.
    // In this example, source == FE and DST == FF.  Only requests are logged 
    // (responses will have source == FF and DST == FE).
    if ((dstport != 0xFF) || (srcport != 0xFE)) {
        return False;
    }

    // Prepare logging header: UTF8 (text log) is subcode 1, dummy length is 0
    otapi_log_header(1, 0);
    
    // Grab application protocol Type-Length-Value block and convert it into
    // human readable data, which added to the log.
    index       = 0;
    input_len   = user_id->length;
    input_data  = user_id->value;

    while (rxq.getcursor <= rxq.back) {
    	scratch                     = slistf(   mpipe.alp.outq->putcursor,
                                                label[index],
                                                type[index],
                                                input_len,
                                                input_data      );
        mpipe.alp.outq->putcursor  += scratch;
        mpipe.alp.outq->length     += scratch;
        q_writebyte(mpipe.alp.outq, '\n');

        index = q_readbyte(&rxq);               // Get next index

        if      (index == 'D')  index = 1;      // PaLFi data load: 8 bytes
        else if (index == 'E')  index = 2;      // Event-type information (1 byte)
        else if (index == 'R')  index = 3;      // PaLFi RSSI bytes (3 bytes)
        else if (index == 'T')  index = 4;      // Temperature (2 bytes)
        else if (index == 'V')  index = 5;      // Voltage (2 bytes)
        else    break;
        
        input_len	    = len[index];
        input_data      = rxq.getcursor;
        rxq.getcursor  += input_len;
    }
    
    // Close the log file, send it out, return success
    otapi_log_direct();
    return True;
}
#endif






/** User ALP Processor & User Task <BR>
  * =======================================================================<BR>
  * If using proprietary/custom ALPs, the processor should be implemented here.
  * There is an example implementation below, which can be uncommented to match
  * the example in the API Quickstart Guide:
  * http://www.indigresso.com/wiki/doku.php?id=opentag:api:quickstart
  */ 

//void app_invoke(ot_u8 call_type) {
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
//	sys_task_setevent(TASK_external, call_type);
//    sys_task_setreserve(TASK_external, 1);
//    sys_task_setlatency(TASK_external, 255);
//    sys_preempt(&sys.task[TASK_external], 0);
//}


//void ext_systask(ot_task task) {
//Do application stuff in here
//    switch (task->event) {
//        case 0: break;  //empty process, also used for destructor
//        
//        case 1: //Task state 1 
//                break;
//        
//        case 2: //Task state 2
//                break;
//    }
//}


//void otapi_alpext_proc(alp_tmpl* alp, id_tmpl* user_id) {
/// For this example, the directive ID is 0x90 and the commands are 0-3.  You
/// can change these values simply by changing the implementation of this 
/// function.
/// Alternatively, instead of using the User Task (extprocess) you can use the
/// sys.loadapi link to an app function: sys.loadapi = &opmode_goto_gateway;
/// This is a simpler approach, but it can be blocked eternally in some setups.
/// Using the User Task guarantees that your app/applet will run at some point
/// in the future, as soon as the kernel is done with high-priority I/O tasks.
//
//    if (alp->inrec.id == 0x90) {
//        ot_u8 task_cmd = (alp->inrec.cmd & 0x7F);
//
//        // Enable our command processing user task (task 0) to run the command
//        // routine, but only if the cmd is known (0<=cmd<=3)
//        if (task_cmd > 3)   task_cmd = 0;
//        else                app_invoke(++task_cmd);
//
//        // Write back success (non-zero).
//        alp_load_retval(alp, task_cmd);
//    }
//}











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
    
    ///4a. The device will wait (and block anything else) until you connect
    ///    it to a valid console that will "enumerate" this USB device
#   if (MCU_FEATURE_MPIPECDC)
    mpipedrv_wait();
#   endif

    ///4b. Load a message to show that main startup has passed
    otapi_log_msg(MSG_utf8, 6, 26, (ot_u8*)"SYS_ON", (ot_u8*)"System on and Mpipe active");

    ///5. MAIN RUNTIME (post-init)  <BR>
    ///<LI> Use a main loop with platform_ot_run(), and nothing more. </LI>
    ///<LI> The kernel actually runs at the bottom of this loop.</LI>
    ///<LI> You could put code before or after platform_ot_run(), which will
    ///     run before or after the (task + kernel).  If you do, keep the code
    ///     very short or else you are risking timing glitches.</LI>
    ///<LI> To run any significant amount of user code, use tasks. </LI>
    while(1) {
    	platform_ot_run();
    }
}


#if defined(BOARD_RF430USB_5509)
#   include "_patch_5509stick/data_5509.c"

#else
//#   include "data.c"
#   include "_patch_5509stick/data_5509.c"

#endif
