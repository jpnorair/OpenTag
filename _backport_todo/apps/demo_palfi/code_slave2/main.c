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
  * @file       /apps/demo_palfi/code_slave2/main.c
  * @author     JP Norair
  * @version    R101
  * @date       6 November 2011
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
#include <otplatform.h>    // Platform level stuff
#include <m2/radio.h>          // for lastrssi
#include <otlib/logger.h>


/** Logging for Debugging (not available in release)<BR>
  * ========================================================================<BR>
  */
#if (MPIPE_FOR_DEBUGGING)
#   define WAIT_FOR_MPIPE() while(0)
#	define OTAPI_LOG_MSG(TYPE, LABEL_LEN, DATA_LEN, LABEL, DATA) \
		logger_msg(TYPE, LABEL_LEN, DATA_LEN, LABEL, DATA)
#else
#   define WAIT_FOR_MPIPE() while(0)
#	define OTAPI_LOG_MSG(TYPE, LABEL_LEN, DATA_LEN, LABEL, DATA) while(0)
#endif





/** Application Global Variables <BR>
  * ========================================================================<BR>
  */

//ot_bool (*app_task)(void);

typedef struct {
    float   slope_dC;
    float   offset_dC;
}
tempmodel_struct;
tempmodel_struct tmodel;


/** Applet Functions <BR>
  * ========================================================================
  */
// Main Application Functions
void app_init();



/** Application local subroutines (platform & board dependent) <BR>
  * ============================================================================
  */
void sub_button_init();

void sub_build_uhfmsg(ot_int* buffer);
void sub_adc_measurement(ot_int* buffer);
void applet_adcpacket(m2session* session);





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




#define PALFI_TASK (&sys.task[TASK_external])

void PALFI_WAKE_ISR(void) {
/// In this demo the PALFI LF interface is not initialized.  However, there are
/// some switches (buttons) that are attached to the PaLFI core, and they use
/// the same interrupt.  Those buttons are attached.

    if (PALFI_WAKE_PORT->IFG & BOARD_SW2_PIN) {
        sys_task_setevent(PALFI_TASK, 2);
        sys_task_setreserve(PALFI_TASK, 64);
        sys_task_setlatency(PALFI_TASK, 255);
        sys_preempt(PALFI_TASK, 0);
    }
    //if (PALFI_WAKE_PORT->IFG & PALFI_WAKE_PIN) {
    //    sys_task_setevent(PALFI_TASK, 1);
    //}

    PALFI_WAKE_PORT->IFG = 0;
}

void PALFI_TIM_ISR(void) {
}



void platform_isr_usernmi(void) {
/// Reset (Actually generates PUC due to access violation)
    PMMCTL0 |= PMMSWPOR;
}


void platform_isr_sysnmi(void) {
///Reset
    PMMCTL0 |= PMMSWPOR;
}







#ifndef EXTF_ext_systask
#   error "EXTF_ext_systask must be defined in extf_config.h for this app to work"
#endif

void ext_systask(ot_task task) {
    switch (task->event) {
        case 0: break;

        case 1: // break and deactivate task if switch is *not* being held-down
                if (PALFI_WAKE_PORT->DIN & BOARD_SW2_PIN) {
                    sys_task_setevent(task, 0);
                    break;
                }

        case 2: { // Add new DASH7 comm task to kernel, using most defaults.
            session_tmpl s_tmpl;
            sys_task_setevent(task, 1);
            sys_task_setnext(task, 512);

            s_tmpl.channel      = 7;
            s_tmpl.subnetmask   = 0;
            s_tmpl.flagmask     = 0;
            m2task_immediate(&s_tmpl, &applet_adcpacket);
        } break;
    }

}




void applet_adcpacket(m2session* session) {
/// This is an OpenTag session Applet.  It gets called by the kernel when the
/// communication task (session) that it is attached-to gets activated by the
/// kernel.  The kernel will wait until a currently-running communication task
/// is over before starting a new one.
///
/// In order to create a new communication task and bind this applet to it, use
/// m2task_immediate() or one of the other tasker functions.
///
/// This applet does two things:
/// 1. Do an ADC capture
/// 2. Build a DASH7 UDP packet that includes PaLFI data and the ADC values
///    that were just captured.  The app protocol inside UDP is a generic TLV.
///
/// @note The kernel automatically detaches the applet from the session after
/// it runs.  You can reattach in this function code by setting:
/// session->applet = &applet_adcpacket;
/// However, there is no reason to do so in this application, because the
/// communication method is not a persistent stream or query.  It is just a
/// single Push+ACK.  Retries are managed internally by the session.
///
    ot_int data_buffer[2];
    sub_adc_measurement(data_buffer);
    sub_build_uhfmsg(data_buffer);
}


void sub_adc_measurement(ot_int* buffer) {
/// This is a blocking ADC capture routine.  It should run in 50us or less.

    /// 1. Universal ADC config
    ///    <LI> Reset REFMSTR, REFVSEL_1 = 2.0V </LI>
    ///    <LI> Voltage Tsample > 1.2us, Temp Tsample > 30us, so use ADCCLK/32
    ///         for Voltage, ADCCLK/768 for Temp.</LI>
    ///    <LI> Also in ADCCTL0, use multisample mode, use REF=2.0V </LI>
    ///    <LI> Use MEM7 (Temp) & MEM8 (Volt), Use internal sampling timer, use MODCLK </LI>
    ///    <LI> Use 12 bit mode, use fast mode </LI>
    ///    <LI> MEM7 is Temp, MEM8 is Volt </LI>
    REFCTL0     = REFMSTR + REFON + REFVSEL_1;
    ADC12CTL0   = 0;
    ADC12CTL0   = ADC12SHT1_3 + ADC12SHT0_7 + ADC12MSC + ADC12REFON + ADC12ON;
    ADC12CTL1   = ADC12CSTARTADD_7 + ADC12SHP + ADC12CONSEQ_1;
    ADC12CTL2   = ADC12RES_2;
    ADC12MCTL7  = ADC12SREF_1 + ADC12INCH_10;
    ADC12MCTL8  = ADC12SREF_1 + ADC12INCH_11 + ADC12EOS;

    /// 2. Start ADC and Wait for ADC to finish.  Wait 75us for REF.
    ///    Grab the data, then kill everything
    delay_us(75);
    ADC12CTL0  |= ADC12ENC;
    ADC12CTL0  |= ADC12SC;
    while ((ADC12CTL1 & ADC12BUSY) == ADC12BUSY);

    ADC12CTL0  &= ~(ADC12ENC | ADC12SC);
    ADC12CTL0  &= ~(ADC12ON + ADC12REFON);
    REFCTL0    &= ~(REFMSTR + REFON + REFGENACT);

    /// 3. Convert Temperature:
    ///@todo Build a Fixed-Point Model instead of this heavy floating point one.
    ///
    /// This temperature conversion method pulls device-specific calibration
    /// data from the TLV space and uses it to produce a linear model to map
    /// the acquired ADC value.
    {
        float val_dC;
        val_dC      = tmodel.slope_dC*(float)ADC12MEM7 + tmodel.offset_dC;
        buffer[0]   = (ot_int)val_dC;
    }

    /// 4. Convert Voltage:
    /// Vdd is acquired as 12 bit number representing Vdd/2 in 1/4095V units.
    /// x(V) = 4095*(Vdd/2)/1.93V; x(mV) = (4095/2*1930mV)Vdd ~= Vdd
    //buffer[1]   = volt;                           // Cheap way, not accurate
    buffer[1]   = (ot_int)((float)ADC12MEM8 * (3860.f/4095.f));      // Higher accuracy method
}


void sub_build_uhfmsg(ot_int* buffer) {
/// This is the routine that builds the DASH7 UDP generic protocol message.
/// The protocol has data elements marked by a letter (T, V, R, E, D) that
/// signify Temperature, Voltage, RSSI (LF), PaLFi wake Event, and RX Data.
/// The elements are fixed/known length.
    command_tmpl    c_tmpl;
    ot_u8*          data_start;
    ot_u8           status;

    // Broadcast request (takes no 2nd argument)
    otapi_open_request(ADDR_broadcast, NULL);

    // Insert Transport-Layer headers
    c_tmpl.type     = CMDTYPE_na2p_request;
    c_tmpl.opcode   = CMD_udp_on_file;
    c_tmpl.extension= CMDEXT_no_response;
    otapi_put_command_tmpl(&status, &c_tmpl);
    otapi_put_dialog_tmpl(&status, NULL);       // NULL = defaults

    // UDP Header
    q_writebyte(&txq, 255);        // Source Port: 255 (custom application port)
    q_writebyte(&txq, 255);        // Destination Port (same value)

    data_start = txq.putcursor;

    // Place temperature data
    q_writebyte(&txq, 'T');
    q_writeshort(&txq, buffer[0]);

    // Place Voltage data
    q_writebyte(&txq, 'V');
    q_writeshort(&txq, buffer[1]);

    // Place RSSI data
    q_writebyte(&txq, 'R');
    q_writeshort(&txq, radio.last_rssi);


    // Store this information into the Port 255 file for continuous, automated
    // reporting by DASH7/OpenTag until it is updated next time.  The length of
    // this information is always 6 bytes.
    {
        vlFILE* fp;
        fp = ISF_open_su(255);
        if (fp != NULL) {
            vl_store(fp, 6, data_start);
            vl_close(fp);
        }
    }

    // Finish Message
    otapi_close_request();
}






void sub_button_init() {
/// Buttons are attached to the PaLFi core
    {
        float test_30C2V;
        float test_85C2V;
        test_30C2V          = (float)(*((ot_u16*)0x1A1E));
        test_85C2V          = (float)(*((ot_u16*)0x1A20));
        tmodel.slope_dC     = (850.f - 300.f) / (test_85C2V - test_30C2V);
        tmodel.offset_dC    = 300.f - (tmodel.slope_dC*test_30C2V);
    }

    // init wakeup port (should be P1.0)
    PALFI_WAKE_PORT->DDIR  &= ~(PALFI_WAKE_PIN | BOARD_SW2_PIN);
    PALFI_WAKE_PORT->IFG   &= ~(PALFI_WAKE_PIN | BOARD_SW2_PIN);
    PALFI_WAKE_PORT->IES    = BOARD_SW2_PIN;
    PALFI_WAKE_PORT->IE     = BOARD_SW2_PIN; //PALFI_WAKE_PIN;
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
	case 2: BOARD_led2_on();	break;
	case 3: BOARD_led1_on();	break;
	}

    delay_ms(33);

    switch (i) {
    case 0: PALFI_LED4_OFF();	break;
    case 1: PALFI_LED3_OFF();	break;
    case 2: BOARD_led2_off();	break;
    case 3: BOARD_led1_off();	break;
    }
}


void app_init() {
    ot_u8 i;

    ///Initialize the input buttons
    sub_button_init();

    /// blink the LEDs
    i = 255;
    do { sub_led_cycle(++i); } while (i != 3);
    do { sub_led_cycle(--i); } while (i != 0);
}






#ifdef EXTF_sys_sig_powerdown
void sys_sig_powerdown(ot_int code) {
///go into LPM1 and don't disable SMCLK: known safe
    __bis_SR_register(0x58);
    __no_operation();
}

#else
#   warn "The early RF430 prototype chips have problems going into LPM2, 3, or 4."
#endif





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

    ///3. Top-level application init
    app_init();

    ///4. Load a message to show that main startup has passed
    WAIT_FOR_MPIPE();
    OTAPI_LOG_MSG(MSG_utf8, 6, 26, (ot_u8*)"SYS_ON", (ot_u8*)"System on and Mpipe active");

    // LPM3/LPM4 not properly working on old CC430 silicon
    // LDO LPM causes RF module to fail
    // setting bit PMMHPMRE in PMMCTL0 register
    // global high power module request is enabled
#   ifndef __DEBUG__
    //PMMCTL0 = 0xA580;      // <-- disable for debugging
#   endif

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

