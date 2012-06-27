/* Copyright 2010 JP Norair
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
  * @file       /OTlib/platform_CC430.c
  * @author     JP Norair
  * @version    V1.0
  * @date       16 July 2011
  * @brief      ISRs and hardware services abstracted by the platform module
  * @ingroup    Platform
  *
  * ISRs implemented in this file shall use only the Platform module macro
  * abstractions, not functions or register nomenclature specific to any given
  * hardware platform.
  *
  ******************************************************************************
  */

#include "OTAPI.h"
#include "OT_platform.h"

// OT low-level modules that need initialization
#include "veelite_core.h"
#include "mpipe.h"
#include "radio.h"

//#include "auth.h"         //should be initialized via system (sys_init())
//#include "session.h"      //should be initialized via system (sys_init())


//API wrappers
void otapi_poweron()    { platform_poweron(); }
void otapi_poweroff()   { platform_poweroff(); }
void otapi_init()       { platform_init_OT(); }
void otapi_exec()       { platform_ot_run(); }
void otapi_preempt()    { platform_ot_preempt(); }
void otapi_pause()      { platform_ot_pause(); }

#ifndef EXTF_otapi_led1_on
void otapi_led1_on() {
#if (BOARD_FEATURE(INVERT_TRIG1) == ENABLED)
    platform_trig1_low();
#else
    platform_trig1_high();
#endif
}
#endif

#ifndef EXTF_otapi_led2_on
void otapi_led2_on() {
#if (BOARD_FEATURE(INVERT_TRIG2) == ENABLED)
    platform_trig2_low();
#else
    platform_trig2_high();
#endif
}
#endif

#ifndef EXTF_otapi_led1_off
void otapi_led1_off() {
#if (BOARD_FEATURE(INVERT_TRIG1) == ENABLED)
    platform_trig1_high();
#else
    platform_trig1_low();
#endif
}
#endif

#ifndef EXTF_otapi_led2_off
void otapi_led2_off() {
#if (BOARD_FEATURE(INVERT_TRIG2) == ENABLED)
    platform_trig2_high();
#else
    platform_trig2_low();
#endif
}
#endif






/** Feature Configuration Macros <BR>
  * ========================================================================<BR>
  * These should be defined in apps/.../app_config.h.  If one or more are
  * missing, use the defaults.
  */
#ifndef OT_FEATURE_RTC
#   define OT_FEATURE_RTC       DISABLED
#endif
#ifndef OT_FEATURE_MPIPE
#   define OT_FEATURE_MPIPE     DISABLED
#endif
//#ifndef OT_FEATURE_STDGPIO
//#   define OT_FEATURE_STDGPIO   DISABLED
//#endif





/** Platform Data <BR>
  * ============================================================================
  */
platform_struct platform;

#if (OT_FEATURE(RTC) == ENABLED)
#   define RTC_ALARMS       0 //(ALARM_beacon + __todo_IS_STM32L__)
#   define RTC_OVERSAMPLE   0
    // RTC_OVERSAMPLE: min 0, max 16

    typedef struct {
        ot_bool active;
        ot_u16  mask;
        ot_u16  value;
    } rtcalarm;

    typedef struct {
#       if (RTC_OVERSAMPLE != 0)
            ot_u32      utc;
#       endif
#       if (RTC_ALARMS > 0)
            rtcalarm    alarm[RTC_ALARMS]
#       endif
    } otrtc_struct;

#   if ((RTC_OVERSAMPLE != 0) || (RTC_ALARMS > 0))
        otrtc_struct otrtc;
#   endif
#endif









/** Platform Interrupt Service Routines (ISR's) <BR>
  * ========================================================================<BR>
  * GPTIM, RTC, SysTick interrupts (kernel interrupts), plus any power
  * management interrupts that might be used.  These interrupts are important
  * to the platform.  You can move them out of the platform module, into a
  * global interrupt handler C file if you want, but it is better to leave the
  * OpenTag resources allocated to OpenTag if you can.
  *
  * -- User Non-Maskable Interrupt (UNMI) ISR  <BR>
  * OpenTag uses the UNMI to deal with segmentation faults and bus errors.  The
  * Veelite Core is responsible for both, and if logging is enabled, it will
  * also shoot the error code to the log before the system goes into emergency.
  * <LI> NMIIFG (2): Used as Segmentation Fault (Code 11)</LI>
  * <LI> OFIFG (4): Oscillator Fault (not implemented in OpenTag). </LI>
  * <LI> ACCVIFG (6): Used as Flash Bus Error (Code 7).</LI>
  *
  * -- System Non-Maskable Interrupt (SYSNMI) ISR     <BR>
  * The Supply Voltage Supervisor and Monitor is generally used to provide a
  * graceful shut-down when power is getting low (via platform_power_off()). If
  * your device is line-powered, then you will want to put in a capacitor to
  * manage power disconnects.  If battery powered, the discharge curve of the
  * battery is unlikely to be steep enough to need a capacitor.
  *
  * -- Reset ISR     <BR>
  * Reset ISR is called immediately AFTER reset, and OpenTag doesn't really use
  * it for anything except debugging, where the user can set a breakpoint here
  * to see what caused the reset.  So, usually it is commented-out.
  *
  * -- Watchdog Interval Interrupt <BR>
  * @note In the current CC430 implementation, the Watchdog is used by the radio
  * driver as a "quick fix" for some known errata of the CC430 RF Core.  We hope
  * to solve it in a better way (working on it), but for now the watchdog is for
  * the radio driver only.
  *
  * -- GPTIM Interrupt (Kernel Timer) <BR>
  * (GPTIM, general purpose timer) is a simple upcounting time that is typically
  * used as the kernel timer.  When it expires, the kernel must run.
  *
  * -- RTC Interrupt <BR>
  * The RTC can be used for synchronization duties within the kernel and/or as
  * the kernel timer itself.  The RTC is implemented as a free-running clock,
  * and alarms are layered on top via software.  When the RTC reaches the time
  * of the alarm, an interrupt occurs.
  */

// 1. User NMI Interrupt (segmentation faults, i.e. firmware faults)
#if (ISR_EMBED(POWER) == ENABLED)
#	if (CC_SUPPORT == CL430)
#		pragma vector=UNMI_VECTOR
#	elif (CC_SUPPORT == IAR_V5)
		//unknown at this time
#	elif (CC_SUPPORT == GCC)
		OT_IRQPRAGMA(UNMI_VECTOR)
#	endif
OT_INTERRUPT void platform_usernmi_isr(void) {
    static const ot_u8 error_code[] = {0, 11, 2, 7};
    ot_u8 sysuniv_reg;

    sysuniv_reg     = (ot_u8)SYS->UNIV;
    sysuniv_reg   >>= 1;
    SYS->UNIV       = 0;        // Clear all User NMI flags

    if (sysuniv_reg != 0) {
        sys_panic( error_code[sysuniv_reg] );
    }

    //LPM4_EXIT;          // Don't Clear All Sleep Bits
}
#endif


// 2. System NMI Interrupt (bus errors and other more serious faults)
#if (ISR_EMBED(POWER) == ENABLED)
#	if (CC_SUPPORT == CL430)
#		pragma vector=SYSNMI_VECTOR        //0xFFFC
#	elif (CC_SUPPORT == IAR_V5)
		//unknown at this time
#	elif (CC_SUPPORT == GCC)
		OT_IRQPRAGMA(SYSNMI_VECTOR)
#	endif

OT_INTERRUPT void platform_sysnmi_isr(void) {
    ot_u8 syssniv_reg = SYS->SNIV;
    SYS->SNIV = 0;        // Clear all System NMI flags

    switch (__even_in_range(syssniv_reg, 12)) {
        case 0x00:  break;

        // Supply Voltage Supervisor/Monitor level-low Interrupts
        // (SVSMHDLY is the one usually used)
        case 0x02:                          //SVML      (not recommended)
        case 0x04:                          //SVMH      (sometimes used)
        case 0x06:                          //SVSMLDLY  (not recommended)
        case 0x08:  platform_poweroff();    //SVSMHDLY  (typically used)
                    sys_panic(1);
                    break;

        // Vacant Memory Access (HW Bus Error): SFRIE1.VMAIE,
        case 0x0A:  sys_panic(4);
                    break;

        // JTAG Mailbox in/out (0C/0E): SFRIE1.JMB~IE
        case 0x0C:
        case 0x0E:  break;

        // Supply Voltage Monitor level-return interrupts (not used)
        case 0x10:                          //SVMLVLR
        case 0x12:  break;                  //SVMHVLR
    }

    //LPM4_EXIT;          // Don't Clear All Sleep Bits
}
#endif


// 3. Reset Interrupt
#ifdef DEBUG_ON
/*
#pragma vector=RESET_VECTOR
__interrupt void Reset_ISR(void) {
    ot_u8 sysrstiv_reg = SYS->RSTIV;
    SYS->RSTIV = 0;       // Clear all reset source flags

    if (sysrstiv_reg != 0) {
        // Set Breakpoint Here
        __no_operation();
    }

//    //Fully dressed reset vector implementation
//    switch (__even_in_range(sysrstiv_reg, 0x20)) {
//        case 0x00:  //No reset source condition
//        case 0x02:  //BOR Brownout
//        case 0x04:  //BOR RST/NMI
//        case 0x06:  //BOR PMMSWBOR
//        case 0x08:  //BOR wakeup from LPM5
//        case 0x0A:  //BOR Security Violation
//        case 0x0C:  //POR SVSL
//        case 0x0E:  //POR SVSH
//        case 0x10:  //POR SVML_OVP
//        case 0x12:  //POR SVMH_OVP
//        case 0x14:  //POR PMMSWPOR
//        case 0x16:  //PUC Watchdog Time out (WDT)
//        case 0x18:  //PUC Watchdog Password Violation (WDT)
//        case 0x1A:  //PUC Flash Password Violation
//        case 0x1C:  //PUC PLL Unlock
//        case 0x1E:  //PUC PERF Peripheral/configuration area fetch
//        case 0x20:  //PUC PMM passsword violation
//    }
}
*/
#endif




// 4. Watchdog Interrupt
//#pragma vector=WDT_VECTOR
//__interrupt void Watchdog_ISR(void) {
//    LPM4_EXIT;  // Clear All Sleep Bits
//}



// 5. Kernel Timer Interrupt
#if (ISR_EMBED(GPTIM) == ENABLED)
#	if (CC_SUPPORT == CL430)
#		pragma vector=OT_GPTIM_VECTOR
#	elif (CC_SUPPORT == IAR_V5)
		//unknown at this time
#	elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(OT_GPTIM_VECTOR)
#	endif
OT_INTERRUPT void platform_gptim_isr() {
    platform_ot_run();
	LPM4_EXIT;
}
#endif


// 6. RTC Interrupt
#if (OT_FEATURE(RTC) == ENABLED)

#if (RTC_ALARMS > 0)
void sub_juggle_rtc_alarm() {
    ot_u8  i;
    ot_u32 comp;
    ot_u32 next     = ~0;
    ot_u32 rtcval   = RTC_GetCounter();

    for (i=0; i<RTC_ALARMS; i++) {
        if (otrtc.alarm[i].active) {
            comp    = rtcval & ((ot_u32)otrtc.alarm[i].mask << RTC_OVERSAMPLE);
            comp   += (ot_u32)otrtc.alarm[i].value << RTC_OVERSAMPLE;
            next    = (comp < next) ? comp : next;
        }
    }

    RTC_SetAlarm(rtcval+next);
}
#endif


#if (ISR_EMBED(RTC) == ENABLED)
#	if (CC_SUPPORT == CL430)
#		pragma vector=RTC_VECTOR
#	elif (CC_SUPPORT == IAR_V5)
		//unknown at this time
#	elif (CC_SUPPORT == GCC)
		//unknown at this time
#	endif
OT_INTERRUPT void platform_rtc_isr() {
/// Currently Psuedocode.  RTC is normally used with ALARM interrupt.  If it
/// is oversampling, then it will also interrupt on each second, so the user
/// can increment the UTC.
#   if (RTC_OVERSAMPLE != 0)
        if (__rtc_interrupt_is_second__) {
            otrtc.utc++;
            //add clear flag
        }
#   endif
#   if (RTC_ALARMS > 0)
        if (__rtc_interrupt_is_alarm__) {
            sub_juggle_rtc_alarm();
            //add clear flag
        }
#    endif
}
#endif


#endif // End of RTC stuff








/** Platform Interrupt & Event Management Routines <BR>
  * ========================================================================<BR>
  */
void platform_disable_interrupts() {
    __bic_SR_register(GIE);
    __no_operation();
}

void platform_enable_interrupts() {
    __bis_SR_register(GIE);
    __no_operation();
}

void platform_ot_preempt() {
/// Manually kick the GPTIM interrupt flag in order to pre-empt the kernel.
/// Also, save the current value of the timer so that the kernel can subtract
/// whatever time passed since the last event.
    ot_u16 scratch = OT_GPTIM->R;
    platform_set_gptim(scratch);
    OT_GPTIM->CTL  |= 1;
}

void platform_ot_pause() {
    platform_ot_preempt();
    platform_flush_gptim();
}

void platform_ot_run() {
/// 1. Save the amount of time that just passed
/// 2. Put the timer into free-running upcounter to time kernel process
/// 3. Run System Kernel, which returns its next scheduled call
/// 4. Put the next scheduled call into the timer, and turn it back on
    ot_u16 next_event;
    ot_u16 elapsed_time;
    elapsed_time    = OT_GPTIM->CCR0;
  //elapsed_time   += platform_get_gptim();  //experimental, for use with continuous timer
    next_event      = sys_event_manager( elapsed_time );

#   if (OT_PARAM(KERNEL_LIMIT) > 0)
        if (next_event > OT_PARAM(KERNEL_LIMIT))
            next_event = OT_PARAM(KERNEL_LIMIT);
#   endif

    platform_set_gptim( next_event );
}







/** Platform Startup and Shutdown Routines <BR>
  * ========================================================================<BR>
  */

void platform_poweron() {
    /// 1. CC430-specific Initial Power-On Routine
    /// <LI> Hold Watchdog timer through the power-on routine           </LI>
    /// <LI> Set Core Voltage to 2.2V, as needed by 20 MHz mode         </LI>
    /// <LI> Enable High Side (Vcc) NMI, and low side (Vcore) Reset     </LI>
    /// <LI> Clear Power Manager Flags and reset the Voltage Monitor    </LI>
    /// <LI> Enable User NMI's (SW NMI, Osc Fault, Flash Violation)     </LI>

    WDTA->CTL = WDTPW + WDTHOLD;

    ///@note On SVSM Config Flags: (1) It is advised in all cases to include
    ///      SVSM_EventDelay.  (2) If using line-power (not battery), it is
    ///      advised to enable FullPerformance and ActiveDuringLPM.  (3) Change
    ///      The SVSM_Voffon_ parameter to one that matches your requirements.
    ///      I recommend putting it as high as you can, to give the most time
    ///      for the power-down routine to work.
    PMM_SetVCore(PMM_Vcore_22);
    PMM_SetStdSVSM( (SVM_Enable | SVSM_AutoControl | SVSM_EventDelay),
                    SVS_Von_20, SVSM_Voffon_235);

    /// 2. Initialize OpenTag platform peripherals
    platform_init_busclk();
    platform_init_interruptor();

    SFRIFG1 &= ~(NMIIFG | OFIFG | VMAIFG);
    SFRIE1   = (NMIIE | /* OFIE | */ ACCVIE);   //Oscillator fault can be glitchy

    platform_init_gptim(0x01C3);        // Initialize GPTIM (to 1024 Hz)
    platform_init_gpio();
    platform_init_memcpy();
    platform_init_prand(0xBEEF);        // BEEF is tasty

    /// 3. Initialize Low-Level Drivers (worm, mpipe)
    // Restore vworm (following save on shutdown)
    vworm_init();

    // Mpipe (message pipe) typically used for serial-line comm.
#   if (OT_FEATURE(MPIPE) == ENABLED)
        mpipe_init(NULL);
#   endif
}


void platform_poweroff() {
/// 1. Disable all NMI interrupts that can break flash writes. <BR>
/// 2. Put any mirror data into the flash <BR>
/// 3. Save the vworm mapping table
    ot_u8 old_sfrie1 = SFRIE1;
    SFRIE1 = 0;
    ISF_syncmirror();
    vworm_save();
    SFRIE1 = old_sfrie1;
}


void platform_init_OT() {
	buffers_init(); //buffers init must be first in order to do core dumps
	vl_init();      //Veelite init must be second
	radio_init();   //radio init third
	sys_init();     //system init last
}


void platform_fastinit_OT() {
    platform_init_OT();
}


void platform_init_busclk() {
    /// 1. Board Specific XTAL startup routine
    ///    (Must be defined in Board Support Header)
    BOARD_XTAL_STARTUP();

    /// 2. Select LF clock: it can be REFO or XT1 (XT1 is preferred).  If an
    ///    LF crystal is available, it will be used for XT1.
#   if defined(MCU_PARAM_LFXTALHz)
#       define CLKSRC_32768     sourceXT1
        SELECT_ACLK(ACLKSRC_XT1CLK);
#   else
#       define CLKSRC_32768     sourceREFO
        SELECT_ACLK(ACLKSRC_REFOCLK);
        UCS->CTL6  |= UCS_CTL6_XT1OFF; //Disable LFXT1 after startup
#   endif

    ///3. CONFIGURE FLL - set DCO to multiply the LS/HS clock
    ///   A good area for code optimization is UCS_FLL_settle(), which is
    ///   general purpose and bloated
#   if defined(MCU_PARAM_OSCHz)
        UCS->CTL3 = ((ot_u16)CLKSRC_32768 << 4) | (ot_u16)refdiv1;
#   elif defined(MCU_PARAM_XTALHz)
        UCS->CTL3 = ((ot_u16)sourceXT2 << 4) | (ot_u16)refdiv1;
#   else
#       error "MCLK not bound to XTAL or OSC"
#   endif
    UCS_FLL_settle((PLATFORM_HSCLOCK_HZ/1000), PLATFORM_HSCLOCK_MULT);

    ///4. Configure 32768Hz -> ACLK, DCO -> MCLK, DCO/X -> SMCLK
#   if (PLATFORM_SMCLK_DIV == 1)
#		define _SMCLK_DIV   div1
#   elif (PLATFORM_SMCLK_DIV == 2)
#		define _SMCLK_DIV   div2
#   elif (PLATFORM_SMCLK_DIV == 4)
#		define _SMCLK_DIV   div4
#   elif (PLATFORM_SMCLK_DIV == 8)
#		define _SMCLK_DIV   div8
#   elif (PLATFORM_SMCLK_DIV == 16)
#		define _SMCLK_DIV   div16
#   elif (PLATFORM_SMCLK_DIV == 32)
#		define _SMCLK_DIV   div32
#   else
#       error SMCLK divider not supported, must be: 1,2,4,8,16,32
#   endif

    UCS->CTL4   = (CLKSRC_32768 << clockACLK) | \
    		      (sourceDCO << clockMCLK) | \
    		      (sourceDCO << clockSMCLK);

    UCS->CTL5   = (div1 << clockACLK) | \
		          (div1 << clockMCLK) | \
		          (_SMCLK_DIV << clockSMCLK);

/*
    ClockInit_Type   ClockInitStruct;
    //FLLInit_Type     FLLInitStruct;

    /// 1. Select LF clock: it can be REFO or XT1 (XT1 is preferred).  If an
    ///    LF crystal is available, it will be used for XT1.
#   if defined(MCU_PARAM_LFXTALHz)
#       define CLKSRC_32768     sourceXT1
        GPIO5->SEL |= (GPIO_Pin_0 | GPIO_Pin_1);
        UCS->CTL6   = (UCS_CTL6_XT2OFF | 0xC0 | UCS_CTL6_XTS_low | UCS_CTL6_XT1BYPASS_off | UCS_CTL6_XCAP_2 );
        UCS_LFXT1Init();
        SELECT_ACLK(ACLKSRC_XT1CLK);
#   else
#       define CLKSRC_32768     sourceREFO
        SELECT_ACLK(ACLKSRC_REFOCLK);
        UCS->CTL6   = (UCS_CTL6_XT2OFF | UCS_CTL6_XT1OFF ); //Disable LFXT1
#   endif

    ///2.   Startup HFXT2 (26 MHz)
    ///     This is power-on default behavior and hence is commented-out
    //P5SEL |= BIT2+BIT3;                       // Port select XT2
    //do{
    //    status = XT2_Start_Timeout(XT2DRIVE_0, 50000);
    //} while(status == UCS_STATUS_ERROR);

    ///3. CONFIGURE FLL - set DCO to multiply the LS clock to generate the HS clock
    ///   Typically, the LS clock is 32768 Hz and HS clock is ~20 MHz (mult=610)
    UCS->CTL3 = ((ot_u16)CLKSRC_32768 << 4) | (ot_u16)refdiv1;
    UCS_FLL_settle((PLATFORM_HSCLOCK_HZ/1000), (PLATFORM_HSCLOCK_HZ/PLATFORM_LSCLOCK_HZ));

    ///4. Configure ACLK to 32768 Hz, using RTC crystal oscillator
    ///   This is power-on default behavior and may be removed if desired
    ClockInitStruct.Clock       = clockACLK;
    ClockInitStruct.Source      = CLKSRC_32768;
    ClockInitStruct.Div         = div1;             //div 1
    UCS_ClockInit(&ClockInitStruct);

    ///5. Configure MCLK to DCO
    ///   default behavior is DCODIV, which produces the same result after the FLL settings
    ClockInitStruct.Clock       = clockMCLK;
    ClockInitStruct.Source      = sourceDCO;    //default: sourceDCODIV;
    ClockInitStruct.Div         = div1;
    UCS_ClockInit(&ClockInitStruct);

    ///6. Configure SMCLK to DCO/8 (going to be ~2.5 MHz), and assure it is enabled
    ///   default behavior is DCODIV, which produces the same result after the FLL settings
    ///   The divide-by-8 is not default behavior
    ClockInitStruct.Clock       = clockSMCLK;
    ClockInitStruct.Source      = sourceDCO;    //default: sourceDCODIV;
    ClockInitStruct.Div         = div8;
    UCS_ClockInit(&ClockInitStruct);
    UCS_SMCLKCmd(ENABLE);
*/
}


void platform_init_periphclk() {
/// MSP430 Clocks are system wide.  No AHB/APB action like on ARMs
}


void platform_init_interruptor() {
/// MSP430 does not have a Nested-Vector Interrupt controller like the Cortex M
/// chips have.  To enable the interrupt controller, there is just one bit to
/// enable.  Peripheral interrupts are managed independently by the peripherals.
    platform_enable_interrupts();
}


void platform_init_gpio() {
/// Initialize ports/pins exclusively used within this platform module.
/// A. Trigger Pins
/// B. Random Number ADC pins: A Zener can be used to generate noise.

    // This must be an inline function in the board header
    BOARD_PORT_STARTUP();  

#if (defined(OT_TRIG1_PORT) && defined(OT_TRIG2_PORT) && (OT_TRIG1_PORTNUM == OT_TRIG2_PORTNUM))
  //OT_TRIG1_PORT->SEL     &= ~(OT_TRIG1_PIN | OT_TRIG2_PIN);
	OT_TRIG1_PORT->DOUT    &= ~(OT_TRIG1_PIN | OT_TRIG2_PIN);
    OT_TRIG1_PORT->REN     &= ~(OT_TRIG1_PIN | OT_TRIG2_PIN);
    OT_TRIG1_PORT->DDIR    |= (OT_TRIG1_PIN | OT_TRIG2_PIN);
    OT_TRIG1_PORT->DS      |= ((OT_TRIG1_PIN*(OT_TRIG1_HIDRIVE == ENABLED)) | \
                               (OT_TRIG2_PIN*(OT_TRIG2_HIDRIVE == ENABLED)) );
#else
#   ifdef OT_TRIG1_PORT
        OT_TRIG1_PORT->DOUT    &= ~(OT_TRIG1_PIN);
        OT_TRIG1_PORT->REN     &= ~(OT_TRIG1_PIN);
        OT_TRIG1_PORT->DDIR    |= OT_TRIG1_PIN;
        OT_TRIG1_PORT->DS      |= (OT_TRIG1_PIN*(OT_TRIG1_HIDRIVE == ENABLED));
#   endif
#   ifdef OT_TRIG2_PORT
        OT_TRIG2_PORT->DOUT    &= ~(OT_TRIG2_PIN);
        OT_TRIG2_PORT->REN     &= ~(OT_TRIG2_PIN);
        OT_TRIG2_PORT->DDIR    |= OT_TRIG2_PIN;
        OT_TRIG2_PORT->DS      |= (OT_TRIG2_PIN*(OT_TRIG2_HIDRIVE == ENABLED));
#   endif
#endif

#if (defined(OT_GWNZENER_PORT) && defined(OT_GWNADC_PORT) && (OT_GWNZENER_PORTNUM == OT_GWNADC_PORTNUM))
    OT_GWNADC_PORT->DOUT   &= ~(OT_GWNZENER_PIN);
    OT_GWNADC_PORT->REN    &= ~(OT_GWNZENER_PIN | OT_GWNADC_PIN);
    OT_GWNADC_PORT->DDIR   &= ~(OT_GWNADC_PIN);
    OT_GWNADC_PORT->DDIR   |= OT_GWNZENER_PIN;
    OT_GWNADC_PORT->DS     |= (OT_GWNZENER_PIN*(OT_GWNZENER_PIN == ENABLED));
#else
#   ifdef OT_GWNADC_PORT
        OT_GWNADC_PORT->REN    &= ~(OT_GWNADC_PIN);
        OT_GWNADC_PORT->DDIR   &= ~(OT_GWNADC_PIN);
#   endif
#   ifdef OT_GWNZENER_PORT
        OT_GWNZENER_PORT->DOUT &= ~(OT_GWNZENER_PIN);
        OT_GWNZENER_PORT->REN  &= ~(OT_GWNZENER_PIN);
        OT_GWNZENER_PORT->DDIR |= OT_GWNZENER_PIN;
        OT_GWNZENER_PORT->DS   |= (OT_GWNZENER_PIN*(OT_GWNZENER_PIN == ENABLED));
#   endif
#endif

}



void platform_init_gptim(ot_uint prescaler) {
/// With the CC430, the timer prescaler input is a code, since the MSP430 core
/// doesn't have a conventional prescaler.  The breakdown is:  <BR>
/// bits 9:8 - Clock select (TASSEL) -  {01 = ACLK, 10 = SMCLK}           <BR>
/// bits 7:6 - Input Divider (ID) -     values {0,1,2,3} yield division by {1,2,4,8}  <BR>
/// bits 2:0 - Input Divider 2 (IDEX) - values b000-b111 yield division by 1 to 8  <BR>

	// Don't enable interrupt on init.  Call platform_ot_preempt to enable.
    ot_u16 ctl      = (prescaler & 0x01E0) /* | TIMA_IT_Update */;
    ot_u16 idex     = (prescaler & 0x0007);

    OT_GPTIM->CTL  |= TIMA_FLG_TACLR;   //Clear the timer before changing mode
    OT_GPTIM->CTL   = ctl;
    OT_GPTIM->EX0   = idex;
}


void platform_init_watchdog() {
    platform_reset_watchdog(64);
}


void platform_init_resetswitch() {
/// On the CC430, you could configure the reset switch to perform an NMI
/// interrupt instead of a hard reset.  We don't do that, because it can cause
/// JTAG debugging to fail.
}



void platform_init_systick(ot_uint period) {
/// Use the Watchdog Timer in "interval mode" as the SysTick (not required).
///
/// On MSP430 / CC430, the divider can be written as follows
///
/// 0   divide by 2^31 (2G)         typ 18 hrs
/// 1   divide by 2^27 (128M)       typ 68 mins
/// 2   divide by 2^23 (8M)         typ 4:16 mins
/// 3   divide by 2^19 (512K)       typ 16 secs
/// 4   divide by 2^15 (32K)        typ 1 sec
/// 5   divide by 2^13 (8K)         typ 250 ms
/// 6   divide by 2^9  (512)        typ 15.6 ms
/// 7   divide by 2^6  (64)         typ 1.95 ms

    WDTA->CTL = (WDTPW | WDTHOLD | WDTSSEL_1 | WDTTMSEL | WDTCNTCL) | period;

    // Set the Watchdog interval interrupt via Special Function register
    // Comment out if you don't want the SysTick to generate interrupts
    SFRIE1 |= WDTIE;
}



void platform_init_rtc(ot_u32 value) {
///@todo Untested, experimental only so far
#if (OT_FEATURE(RTC) == ENABLED)

    // Set Prescalers for 1 second RTC increments, but no interrupts.
    // (assuming 32768 Hz ACLK source)
    RTC->PS0CTL = RT0PSDIV_7;
    RTC->PS1CTL = RT1SSEL_2 + RT1PSDIV_6 + /* RT1PSIE + */ RT1IP_6;
    RTC->CTL01  = 0x0800;		// clear RTC interrupts & flags, RT1PS prescaled clock select
    RTC->CTL23  = 0x0000;       // ignore calibration for now, no clock pin output


    // Set RTC value from that stored in configuration
    RTC->TIM0   = ((ot_u16*)value)[0];
    RTC->TIM1   = ((ot_u16*)value)[1];

    platform_enable_rtc();
#endif
}


void platform_init_memcpy() {
#if (MCU_FEATURE(MEMCPYDMA) == ENABLED)
#   if (MEMCPY_DMANUM == 0)
        DMA->CTL0  |= DMA_Trigger_DMAREQ;
#   elif (MEMCPY_DMANUM == 1)
        DMA->CTL0  |= (DMA_Trigger_DMAREQ << 8);
#   elif (MEMCPY_DMA == 2)
        DMA->CTL1   = DMA_Trigger_DMAREQ;
#   else
#       error "MEMCPY_DMA is misconfigured in the board-config header"
#   endif
#endif
}








/** Platform Peripheral Access Routines <BR>
  * ========================================================================<BR>
  */

ot_u16 platform_get_gptim() {
    return OT_GPTIM->R;
}

void platform_set_gptim(ot_u16 value) {
    OT_GPTIM->CTL  |= 0x0004;   //clear & stop timer
    OT_GPTIM->CTL  &= ~0x0033;  //clear configuration
    OT_GPTIM->CCR0  = value;    //set next interval
    OT_GPTIM->CTL  |= 0x0012;   //restart in upmode, with interrupt
    ///@todo continuous-up with interrupt is experimental
}

void platform_flush_gptim() {
    OT_GPTIM->CTL  |= 0x0004;   //clear & stop timer
    OT_GPTIM->CTL  &= ~0x0033;  //clear configuration
    OT_GPTIM->CCR0  = 65535;    //workaround to a known CC430 erratum
    OT_GPTIM->CTL  |= 0x0020;   //restart in continuous-up, without interrupt
}

void platform_run_watchdog() {
    ot_u16 saved_reg;
    saved_reg   = WDTA->CTL & 0x7F;
    WDTA->CTL   = WDTPW | saved_reg;
}

void platform_reset_watchdog(ot_u16 reset) {
    static const ot_u16 wdt_limit[5] = { 2, 16, 256, 32768, 65535 };
    ot_u8 i = 0;
    while (reset > wdt_limit[i]) i++;

    WDTA->CTL = WDTPW | b10111000 | (7-i);
}

void platform_enable_rtc() {
    RTC->PS1CTL |= RT1PSIE;
}

void platform_disable_rtc() {
    RTC->PS1CTL &= ~RT1PSIE;
}

ot_u32 platform_get_time() {
#if (OT_FEATURE(RTC) == ENABLED)
#   if (RTC_OVERSAMPLE == 0)
        return ///@todo find time register, if it exists
#   else
        return otrtc.utc;
#   endif
#else
    return 0;
#endif
}

void platform_set_time(ot_u32 utc_time) {
#if (RTC_OVERSAMPLE != 0)
    otrtc.utc   = utc_time;
#else
    ///@todo find time register, if it exists
#endif
}

void platform_set_rtc_alarm(ot_u8 alarm_i, ot_u16 mask, ot_u16 value) {
#if (OT_FEATURE(RTC) == ENABLED)
    otrtc.alarm[alarm_i].mask     = mask;
    otrtc.alarm[alarm_i].value    = value;
#endif
}

void platform_enable_rtc_alarm(ot_u8 alarm_id, ot_bool enable) {
#if (OT_FEATURE(RTC) == ENABLED)
    otrtc.alarm[alarm_i].active   = enable;
#endif
}






/** Platform Debug Triggers <BR>
  * ========================================================================<BR>
  * Triggers are optional pins mostly used for debugging.  Sometimes they are
  * hooked up to LEDs, logic probes, etc.  The platform module requires that at
  * two triggers exist (if triggers are defined at all).  More triggers can be
  * defined in the application code.
  */

///@todo find a way to inline these
#ifdef OT_TRIG1_PORT
void platform_trig1_high() {    OT_TRIG1_PORT->DOUT |= OT_TRIG1_PIN; }
void platform_trig1_low() {     OT_TRIG1_PORT->DOUT &= ~OT_TRIG1_PIN; }
void platform_trig1_toggle() {  OT_TRIG1_PORT->DOUT ^= OT_TRIG1_PIN; }
#else
void platform_trig1_high() { }
void platform_trig1_low() { }
void platform_trig1_toggle() { }
#endif

#ifdef OT_TRIG2_PORT
void platform_trig2_high() {    OT_TRIG2_PORT->DOUT |= OT_TRIG2_PIN; }
void platform_trig2_low() {     OT_TRIG2_PORT->DOUT &= ~OT_TRIG2_PIN; }
void platform_trig2_toggle() {  OT_TRIG2_PORT->DOUT ^= OT_TRIG2_PIN; }
#else
void platform_trig2_high() { }
void platform_trig2_low() { }
void platform_trig2_toggle() { }
#endif






/** Platform Default CRC Routine <BR>
  * ========================================================================<BR>
  * CC430 has a compliant, internal CRC16 engine.  This setup allows the ASCII
  * string "123456789" produce 0x29B1.
  */

ot_u16 platform_crc_init() {
	CRC->INIRES = 0xFFFF;
    return CRC->INIRES;
}

ot_u16 platform_crc_block(ot_u8* block_addr, ot_int block_size) {
    ot_u8* data = block_addr;
    CRC->INIRES = 0xFFFF;

    for (; block_size > 0; block_size--) {
        CRCb->DIRB_L = *data++;
    }

    return CRC->INIRES;
}

void platform_crc_byte(ot_u8 databyte) {
	CRCb->DIRB_L = databyte;
}

ot_u16 platform_crc_result() {
    return CRC->INIRES;
}





/** Platform Random Number Generation Routines <BR>
  * ========================================================================<BR>
  * The platform must be able to compute a strong random number (via function
  * platform_rand()) and a "pseudo" random number (via platform_prand_u8()).
  */

void platform_rand(ot_u8* rand_out, ot_int bytes_out) {
/*
/// The operation here is to pull 32 bits for the least-significant bit of
/// 32 12 bit samples of the temperature sensor.  The CC430 ADC works fairly
/// fast, so this function can be done in, ideally, less than 200 us.
/// One thing you can do is buffer random numbers when the device is idle.
    ot_uint adclsr;
    ot_uint *adcmemory;
    ot_u8   *adcmctl;

#   ifdef OT_GWNZENER_PORT
        OT_GWNZENER_PORT |= OT_GWNZENER_PIN;
#   endif

    ///1. Initialize ADC12_A
    //    - default sampling time = minimum (better for random numbers anyway)
    //    - use multi-sample-converter
    ADC12CTL0 = ADC12ON + ADC12MSC;

    // use temperature sensor input source
    // use internal sampling timer
    // use repeat-single-channel mode
    // use other defaults (start at MEM0, MODOSC, no clock divide, etc)
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_2;

    // use 12 bit precision
    // use other defaults, including temp sensor ON
    ADC12CTL2 = ADC12RES_2;

#   if (OT_GWNADC_BITS == 8)
    //Special case for direct synthesis of random bytes.

#   else

#   endif

    ///3. Shut down ADC
    ADC12CTL0 = 0;

#   ifdef OT_GWNZENER_PORT
        OT_GWNZENER_PORT &= ~(OT_GWNZENER_PIN);
#   endif
*/
}


ot_u16 prand_reg;

void platform_init_prand(ot_u16 seed) {
    prand_reg = seed;
}

ot_u8 platform_prand_u8() {
    return (ot_u8)platform_prand_u16();
}

ot_u16 platform_prand_u16() {
/// Run the HW CRC on the prand register stored value.  Always save the value in
/// the CRC HW and return it when the process is done.  prand_reg should be
/// initialized at startup with the device ID or serial number (or something)
    ot_u16 scratch  = CRC->INIRES;
    CRC->INIRES     = prand_reg;
    CRCb->DIRB_L    = 0;
    prand_reg       = CRC->INIRES;
    CRC->INIRES     = scratch;

    return prand_reg;
}





/** Platform memcpy Routine <BR>
  * ========================================================================<BR>
  * Similar to standard implementation of "memcpy"
  */

void platform_memcpy(ot_u8* dest, ot_u8* src, ot_int length) {
/// Behavior is always blocking.

#if (OS_FEATURE(MEMCPY) == ENABLED)
    memcpy(dest, src, length);

#elif (MCU_FEATURE(MEMCPYDMA) == ENABLED)
/// DMA driven method: CC430 DMA Block Transfer is blocking, and the CPU is
/// stopped during the data movement.  Thus the while loop is not needed.
/// DMA memcpy cannot be used reliably if OpenTag makes use of the DMA for some
/// other non-blocking process (e.g. MPipe).
    MEMCPY_DMA->SA_L    = (ot_u16)src;
    MEMCPY_DMA->DA_L    = (ot_u16)dest;
    MEMCPY_DMA->SZ      = length;
    //DMA->CTL4           = ( DMA_Options_RMWDisable | \
                            DMA_Options_RoundRobinDisable | \
                            DMA_Options_ENMIDisable      );
    MEMCPY_DMA->CTL     = ( DMA_Mode_Block | \
                            DMA_DestinationInc_Enable | \
                            DMA_SourceInc_Enable | \
                            DMA_DestinationDataSize_Byte | \
                            DMA_SourceDataSize_Byte | \
                            DMA_TriggerLevel_RisingEdge | \
                            0x11);
    //MEMCPY_DMA->CTL    |= 0x0001;

    //while ((MEMCPY_DMA->CTL & DMAIFG) == 0);

#else
/// Uses the "Duff's Device" for loop unrolling.  If this is incredibly
/// confusing to you, check the internet for "Duff's Device."
    if (length > 0) {
        ot_int loops = (length + 7) >> 3;

        switch (length & 0x7) {
            case 0: do {    *dest++ = *src++;
            case 7:         *dest++ = *src++;
            case 6:         *dest++ = *src++;
            case 5:         *dest++ = *src++;
            case 4:         *dest++ = *src++;
            case 3:         *dest++ = *src++;
            case 2:         *dest++ = *src++;
            case 1:         *dest++ = *src++;
                        }
                        while (--loops > 0);
        }
    }

#endif
}








/** Platform Utility Functions <BR>
  * ========================================================================<BR>
  * Random crap
  */

void platform_delay(ot_u16 n) {
    ot_u16 reg;

    reg         = 0x007F & WDTA->CTL;              // clear hold bit and passkey
    WDTA->CTL   = (WDTPW | WDTCNTCL | reg);     // reset with passkey and clear timer

    for (; n > 0; n--) {          // run nCount iterations
        PMM_EnterLPM3();                        // sleep until interrupt (change LPM if needed)
    }

    WDTA->CTL   = (WDTPW | WDTHOLD | reg);      // hold WDT after completion
}




/** @note
  * The multiplier values used in Proc_SwDelay...() are hard-coded into the
  * constants below.  At the moment, it is hardcoded to work with a roughly
  * 20 MHz (± 1%) instruction clock.
  *
  * @todo
  * Test constants with logic probe to verify accuracy
  * Might need to code in assembly due to 16 bit integer limit of CC430
  */
#define CNT1us_20MHz    (20/2)
#define CNT1ms_20MHz    (CNT1us_20MHz*1000)

void platform_swdelay_ms(ot_u16 n) {
	for (; n>0; n--) {
	    __delay_cycles(20000);
	}
}


void platform_swdelay_us(ot_u16 n) {
	for (; n>0; n--) {
        __delay_cycles(20);
	}
}





