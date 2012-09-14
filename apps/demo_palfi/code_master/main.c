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
  * @version    V1.0
  * @date       31 July 2012
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
#include "radio.h"






/** Application Global Variables <BR>
  * ========================================================================<BR>
  * It is safe to either keep app_devicemode volatile, since it is set in an
  * interrupt service routine.
  */

#if (MCU_FEATURE(MPIPECDC) == ENABLED)
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
#       if (MCU_FEATURE_MPIPECDC == ENABLED)
            app_usbhold = 1;
#       endif
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
    APP_BUTTON_PORT->IFG    = 0;

#   if (MCU_FEATURE_MPIPECDC == ENABLED)
    app_usbhold             = 1;
#   endif

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

   
    ///Dynamic Mpipe Callbacks: Set MPipe to go back to listen after TX.
#   if (OT_FEATURE(MPIPE_CALLBACKS) == ENABLED)
        mpipe_setsig_txdone(&otapi_ndef_idle);
        mpipe_setsig_rxdone(&otapi_ndef_proc);
        otapi_ndef_idle(0);
#   endif

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
    	scratch                     = slistf(   mpipe_alp.outq->putcursor,
                                                label[index],
                                                type[index],
                                                input_len,
                                                input_data      );
        mpipe_alp.outq->putcursor  += scratch;
        mpipe_alp.outq->length     += scratch;
        q_writebyte(mpipe_alp.outq, '\n');

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
    platform_poweron();
    platform_init_OT();

    ///3. Initialize the User Applet & interrupts
    app_init();

    
#   if (MCU_FEATURE_MPIPECDC)
        ///4a. The device will wait (and block anything else) until you connect
        ///    it to a valid console app.
        mpipe_wait();
#   endif


    ///4b. Send a message to show that main startup has passed.  You can use a
    ///    transmission like this to start-up the Mpipe NDEF console, or you
    ///    can alternatively call otapi_ndef_idle(0)
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


#if defined(BOARD_RF430USB_5509)
#   include "_patch_5509stick/data_5509.c"

#else
//#   include "data.c"
#   include "_patch_5509stick/data_5509.c"

#endif
