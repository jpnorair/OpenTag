/* Copyright 2010-2012 JP Norair
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
  * @file       /otplatform/cc430/platform_CC430.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2012
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

//#include "auth.h"         //should be initialized via DLL (dll_init())
//#include "session.h"      //should be initialized via DLL (dll_init())


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
#define RTC_ALARMS          (1 * OT_FEATURE(RTC))       // Max=3
#define RTC_OVERSAMPLE      0                           // unsupported on CC430

typedef struct {
    ot_u8   disabled;
    ot_u8   taskid;
    ot_u16  mask;
    ot_u16  value;
} rtcalarm;

typedef struct {
    ot_u16 prand_reg;
    
#   if (RTC_OVERSAMPLE)
#   endif
#   if (RTC_ALARMS > 0)
        rtcalarm alarm[RTC_ALARMS]
#   endif

} platform_ext_struct;


platform_struct     platform;       //defined in OT_platform.h
platform_ext_struct platform_ext;














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
#   if (CC_SUPPORT == CL430)
#       pragma vector=UNMI_VECTOR
#   elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#   elif (CC_SUPPORT == GCC)
        OT_IRQPRAGMA(UNMI_VECTOR)
#   endif
OT_INTERRUPT void platform_usernmi_isr(void) {
/// Error Codes
/// 11 - Segmentation Fault (Veelite Error)
///  2 - Oscillator error
///  7 - Vacant memory access (flash error)
/// 10 - Bus error (HW bus problem, typically USB timeout)
    ot_int code = 0;
    
    switch (__even_in_range(SYS->UNIV, 6)) {
    case 0: break;
    case 2: code = 9;
    case 4: code -= 5;
    case 6: sys_panic( code+7 );
            break;
    }
}
#endif



// 2. System NMI Interrupt (bus errors and other more serious faults)
#if (ISR_EMBED(POWER) == ENABLED)
#   if (CC_SUPPORT == CL430)
#       pragma vector=SYSNMI_VECTOR
#   elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#   elif (CC_SUPPORT == GCC)
        OT_IRQPRAGMA(SYSNMI_VECTOR)
#   endif
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




/// 3.  Reset Interrupt
///     If specificed, this gets called after reset, during startup
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



// 5. Kernel Timer Interrupt: This must be the "A1" interrupt
#if (ISR_EMBED(GPTIM) == ENABLED)
#   if (CC_SUPPORT == CL430)
#       pragma vector=OT_GPTIM_VECTOR
#   elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#   elif (CC_SUPPORT == GCC)
        OT_IRQPRAGMA(OT_GPTIM_VECTOR)
#   endif
OT_INTERRUPT void platform_gptim_isr() {
    switch (__even_in_range(OT_GPTIM->IV, 16)) {
        case 0: break;
        case 2: LPM4_EXIT;              break;  //Kernel Timer
        case 4: radio_mac_isr();        break;
    }
}
#endif


// 6. RTC Interrupt
#if (OT_FEATURE(RTC) == ENABLED)

#if (RTC_OVERSAMPLE != 0)
#   error "RTC Oversampling is not supported on CC430"   
#endif


#if (RTC_ALARMS > 0)
#if (ISR_EMBED(RTC) == ENABLED)
#   if (CC_SUPPORT == CL430)
#       pragma vector=RTC_VECTOR
#   elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#   elif (CC_SUPPORT == GCC)
        OT_IRQPRAGMA(RTC_VECTOR)
#   endif
OT_INTERRUPT void platform_rtc_isr() {
/// The only supported interrupt for CC430 is the 1 second interval interrupt.
/// CC430 (and MSP430) do not have an RTC that is well-suited to some of the 
/// more advanced RTC-based MAC features of OpenTag/DASH7, so I keep the RTC
/// implementation as simple as possible.
    ot_int  i;
    
    RTC->PS1CTL &= ~RT1PSIFG;

    for (i=(RTC_ALARMS-1); i>=0; i--) {
        if (platform_ext.alarm[i].disabled == 0) {
            if ((RTC->TIM0 & platform_ext.alarm[i].mask) == platform_ext.alarm[i].value) {
                sys_synchronize(platform_ext.alarm[i].taskid);
            }
        }
    }
}
#endif  // ISR declaration stuff
#endif  // RTC_ALARM > 0


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
/// The kernel scheduler runs normally in co-operative mode, and it is invoked
/// always after a task finishes running.  The kernel scheduler must also run
/// in pre-emptive mode if the chip is asleep (otherwise it will run normally
/// in co-operative mode after the task completes).  In this function, we just
/// check if GPTIM is hot, which means the chip is asleep.  If so, it will
/// invoke sys_event_manager() pre-emptively.
    if (OT_GPTIM->CCTL1 & CCIE) {
        OT_GPTIM->CCTL1 |= (sys_event_manager() == 0) | CCIE;
    }
}

//void platform_ot_pause() {
//    platform_ot_preempt();
//    platform_flush_gptim();
//}

void platform_ot_run() {
    sys_task_manager();   //invoke USER NMI, which runs kernel scheduler
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
    platform_init_prand( *((ot_u16*)0x1A10) );  //seed using part of HW Chip ID

    /// 3. Initialize Low-Level Drivers (worm, mpipe)
    // Restore vworm (following save on shutdown)
    vworm_init();
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
    /// 1. Initialize Data sources required by basically all OT features
    ///    - Buffers module allocates the data queues, used by all I/O
    ///    - Veelite module allocates and prepares the filesystem
    buffers_init();
    vl_init();

    /// 2. Initialize the RTC, if available.  364489200 is a magic number.
    ///    RTC can be initialized outside of OTlib scope (e.g. in 
    ///    platform_poweron()), but doing it here allows filesystem to store a 
    ///    default value if you choose.
#   if (OT_FEATURE(RTC))
        platform_init_rtc(364489200);
#   endif
    
    /// 3. Initialize the System (Kernel & more).  The System initializer must
    ///    initialize all modules that are built onto the kernel.  These include
    ///    the DLL and MPipe.
    sys_init();
    

    /// 4. If debugging, find the Chip ID and use 6 out of 8 bytes of it to
    ///    yield the UID.  This ID might not be entirely unique -- technically, 
    ///    there is 1/65536 chance of non-uniqueness, but practically the 
    ///    chance is much lower, given the way chips are distributed.  For 
    ///    test/debug, this is adequately unique.
    ///
    /// @note the ID is inserted via Veelite, so it is abstracted from the 
    /// file memory configuration of your board and/or app. 
#   if (defined(__DEBUG__) || defined(DEBUG_ON))
    {
        vlFILE* fpid;
        ot_u16* hwid;
        ot_int  i;

        fpid    = ISF_open_su(1);
        hwid    = (ot_u16*)(0x1A08);

        for (i=2; i<8; i+=2) {
            ot_u16 scratch;
            scratch     = *hwid++;
            scratch     = __swap_bytes(scratch);
            scratch    ^= *hwid++;

            vl_write(fpid, i, scratch);
        }
        vl_close(fpid);
    }
#   endif
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
#       define _SMCLK_DIV   div1
#   elif (PLATFORM_SMCLK_DIV == 2)
#       define _SMCLK_DIV   div2
#   elif (PLATFORM_SMCLK_DIV == 4)
#       define _SMCLK_DIV   div4
#   elif (PLATFORM_SMCLK_DIV == 8)
#       define _SMCLK_DIV   div8
#   elif (PLATFORM_SMCLK_DIV == 16)
#       define _SMCLK_DIV   div16
#   elif (PLATFORM_SMCLK_DIV == 32)
#       define _SMCLK_DIV   div32
#   else
#       error SMCLK divider not supported, must be: 1,2,4,8,16,32
#   endif

    UCS->CTL4   = (CLKSRC_32768 << clockACLK) | \
                  (sourceDCO << clockMCLK) | \
                  (sourceDCO << clockSMCLK);

    UCS->CTL5   = (div1 << clockACLK) | \
                  (div1 << clockMCLK) | \
                  (_SMCLK_DIV << clockSMCLK);
}


void platform_init_periphclk() {
/// MSP430 Clocks are system wide.  No AHB/APB setup like on ARMs
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
    OT_TRIG1_PORT->DDIR    |= (OT_TRIG1_PIN | OT_TRIG2_PIN);
    OT_TRIG1_PORT->DS      |= ((OT_TRIG1_PIN*(OT_TRIG1_HIDRIVE == ENABLED)) | \
                               (OT_TRIG2_PIN*(OT_TRIG2_HIDRIVE == ENABLED)) );
#else
#   ifdef OT_TRIG1_PORT
        OT_TRIG1_PORT->DOUT    &= ~OT_TRIG1_PIN;
        OT_TRIG1_PORT->DDIR    |= OT_TRIG1_PIN;
        OT_TRIG1_PORT->DS      |= (OT_TRIG1_PIN*(OT_TRIG1_HIDRIVE == ENABLED));
#   endif
#   ifdef OT_TRIG2_PORT
        OT_TRIG2_PORT->DOUT    &= ~OT_TRIG2_PIN;
        OT_TRIG2_PORT->DDIR    |= OT_TRIG2_PIN;
        OT_TRIG2_PORT->DS      |= (OT_TRIG2_PIN*(OT_TRIG2_HIDRIVE == ENABLED));
#   endif
#endif

#ifdef OT_GWNZENER_PORT
    //OT_GWNZENER_PORT->DOUT &= ~OT_GWNZENER_PIN;
    OT_GWNZENER_PORT->DDIR |= OT_GWNZENER_PIN;
#endif

}



void platform_init_gptim(ot_uint prescaler) {
/// With the CC430, the timer prescaler input is a code, since the MSP430 core
/// doesn't have a conventional prescaler.  The breakdown is:  <BR>
/// bits 9:8 - Clock select (TASSEL) -  {01 = ACLK, 10 = SMCLK}           <BR>
/// bits 7:6 - Input Divider (ID) -     values {0,1,2,3} yield division by {1,2,4,8}  <BR>
/// bits 2:0 - Input Divider 2 (IDEX) - values b000-b111 yield division by 1 to 8  <BR>
    ot_u16 ctl      = (prescaler & 0x01C0);
    ot_u16 idex     = (prescaler & 0x0007);

    OT_GPTIM->CTL  |= TIMA_FLG_TACLR;   //Clear the timer before changing mode
    OT_GPTIM->CTL   = ctl;
    OT_GPTIM->EX0   = idex;
    //OT_GPTIM->CCR0  = 0;
    //OT_GPTIM->CCTL0 = 0;
    OT_GPTIM->CTL  |= 0x0020;       //continuous-up counting, no update interrupt
}



void platform_init_watchdog() {
/// Watchdog is generally unused in OpenTag, which has a kernel to administer
/// timeouts to I/O task processes.  The Watchdog is free to the user in stable
/// builds, but in Betas, it might be used by some drivers.

/// @todo Implement a more usable watchdog.  Right now it is not tested.
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
#if (OT_FEATURE(RTC) || defined(OT_GPTIM_USERTC))

#   if (BOARD_PARAM_LFHz != 32768)
#       error "Currently, the RTC must use 32768Hz"
#   endif

#   ifdef OT_GPTIM_USERTC
#       error "The MSP430 cannot elegantly support RTC as GPTIM, however, GPTIM can use same ACLK"
#   endif

    /// Set Prescalers for 1 second RTC increments (assuming 32768 Hz), but 
    /// do not activate RTC interrupts (the RTC interrupt is used by the RTC
    /// task scheduler).  When a Task is bound to the scheduler using function
    /// platform_set_rtc_alarm(), the interrupt is activated.  RTC interrupt is
    /// deactivated via platform_clear_rtc_alarms(), called to unbind Tasks.
#   if (RTC_ALARMS > 0)
#       define _1S_INC  (RT1PSDIV_6 | RT1IP_6)
#   else
#       define _1S_INC  (RT1PSDIV_6)
#   endif

    platform_clear_rtc_alarms();

    RTC->CTL01  = 0x0800;       // clear RTC interrupts & flags, RT1PS prescaled clock select
    RTC->CTL23  = 0x0000;       // ignore calibration for now, no clock pin output
    RTC->PS0CTL = RT0PSDIV_7;
    RTC->PS1CTL = RT1SSEL_2 | _1S_INC;

    // Set RTC timer value from input.
    RTC->TIM0   = ((ot_u16*)value)[0];
    RTC->TIM1   = ((ot_u16*)value)[1];

    //platform_enable_rtc();
#endif
}


void platform_init_memcpy() {
#if (MCU_FEATURE(MEMCPYDMA) == ENABLED)
    BOARD_DMA_COMMON_INIT();

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

ot_u16 platform_get_ktim() {
    return (OT_GPTIM->R - OT_GPTIM->CCR0);
}

ot_u16 platform_next_ktim() {
    return (OT_GPTIM->CCR1 - OT_GPTIM->R);
}

void platform_enable_ktim() {
    OT_GPTIM->CCTL1 |= 0x0010;  //enable interrupt for CC0
}

void platform_disable_ktim() {
    OT_GPTIM->CCTL1 &= ~0x0010; //disable interrupt for CC0
}

void platform_pend_ktim() {
    OT_GPTIM->CCTL1 |= 1; //Set CCIFG bit
}

void platform_flush_ktim() {
    OT_GPTIM->CCTL1 = 0x0000;
    OT_GPTIM->CCR0  = OT_GPTIM->R;
}

void platform_set_ktim(ot_u16 value) {
/// Input to kernel timer should never be 0
    ot_u16 timer_cnt;
    OT_GPTIM->CCTL1 = 0x0000;
    timer_cnt       = OT_GPTIM->R;
    OT_GPTIM->CCR0  = timer_cnt;
    OT_GPTIM->CCR1  = timer_cnt + value;    //add value to current timer, put in CC reg
    OT_GPTIM->CCTL1 = 0x0030;               //Put CC in SET mode, enable compare-SET interrupt
}

void platform_set_gptim2(ot_u16 value) {
/// Note that if value == 0, the counter interrupt will not go until one full
/// loop, which is not the desired behavior, so an interrupt will be forced.
    OT_GPTIM->CCTL2 = 0x0000;
    OT_GPTIM->CCR2  = OT_GPTIM->R + value;      //add value to current timer, put in CC reg
    OT_GPTIM->CCTL2 = 0x0030 | (value == 0);    //Put CC in SET mode, enable compare-SET interrupt
}



void platform_set_watchdog(ot_u16 timeout_ticks) {
    static const ot_u16 wdt_limit[5] = { 2, 16, 256, 32768, 65535 };
    ot_u8 i = 0;
    while (timeout_ticks > wdt_limit[i]) i++;

    WDTA->CTL = WDTPW | b10111000 | (7-i);
}

void platform_kill_watchdog() {
    WDTA->CTL   = WDTPW | WDTHOLD;
}

void platform_pause_watchdog() {
    ot_u16 saved_reg;
    saved_reg   = WDTA->CTL & 0x7F;
    WDTA->CTL   = WDTPW | WDTHOLD | saved_reg;
}

void platform_resume_watchdog() {
    ot_u16 saved_reg;
    saved_reg   = WDTA->CTL & 0x7F;
    WDTA->CTL   = WDTPW | saved_reg;
}





void platform_enable_rtc() {
    RTC->PS1CTL |= RT1PSIE;
}

void platform_disable_rtc() {
    RTC->PS1CTL &= ~RT1PSIE;
}

void platform_set_time(ot_u32 utc_time) {
#if (RTC_OVERSAMPLE)
#else
    RTC->TIM0   = ((ot_u16*)utc_time)[0];
    RTC->TIM1   = ((ot_u16*)utc_time)[1];
#endif
}

ot_u32 platform_get_time() {
#if (OT_FEATURE(RTC) == ENABLED)
#   if (RTC_OVERSAMPLE)
    return platform_ext.utc;
    
#   else
    ot_u32 output;
    ((ot_u16*)output)[0]    = RTC->TIM0;
    ((ot_u16*)output)[1]    = RTC->TIM1;
    return output;
#   endif

#else
    return 0;
#endif
}

void platform_set_rtc_alarm(ot_u8 alarm_id, ot_u8 task_id, ot_u16 offset) {
#if (OT_FEATURE(RTC) == ENABLED)
#   ifdef __DEBUG__
    if (alarm_id < RTC_ALARMS)
#   endif
    {
        vlFILE* fp                      = ISF_open_su( ISF_ID(real_time_scheduler) );
        platform_ext.alarm[alarm_id].disabled  = 0;
        platform_ext.alarm[alarm_id].taskid    = task_id;
        platform_ext.alarm[alarm_id].mask      = PLATFORM_ENDIAN16(ISF_read(fp, offset));
        platform_ext.alarm[alarm_id].value     = PLATFORM_ENDIAN16(ISF_read(fp, offset+2));
        vl_close(fp);
        
        platform_enable_rtc();
    }
#endif
}

void platform_clear_rtc_alarms() {
#if (OT_FEATURE(RTC) == ENABLED)
    platform_disable_rtc();

#   if (RTC_ALARMS > 0)
        platform_ext.alarm[0].disabled = 1;
#   endif
#   if (RTC_ALARMS > 1)
        platform_ext.alarm[1].disabled = 1;
#   endif
#   if (RTC_ALARMS > 2)
        platform_ext.alarm[2].disabled = 1;
#   endif
#   if (RTC_ALARMS > 3)
#       warn "Currently, only three RTC ALARMS supported"
#   endif
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
/// This random number generator is quite fast.  A 128 bit number can be
/// generated in less than 50us, typically.

#ifndef OT_GWNADC_BITS
#   define OT_GWNADC_BITS 8
#endif

#ifdef OT_GWNADC_PINNUM
#   undef OT_GWNADC_PIN
#else
#   warn "OT_GWNADC_PINNUM is not defined in the board configuration header."
#   define OT_GWNADC_PINNUM 0
#   undef OT_GWNADC_PIN
#endif

    /// Open Floating Input pin
#   define OT_GWNADC_PIN (1 << OT_GWNADC_PINNUM)
    OT_GWNADC_PORT->DDIR  &= ~OT_GWNADC_PIN;
    OT_GWNADC_PORT->REN   &= ~OT_GWNADC_PIN;

    /// 1. Set-up ADC: The trick to this implementation is to drive the sampling
    ///    timer MUCH faster than it is supposed to run, so that it just produces
    ///    garbage data.  The garbage data turns-out to be quite random, even to
    ///    the 8 LSBs of the 12 bit capture.  If you reduce the clock frequency,
    ///    you might want to change the OT_GWNADC_BITS to a lower number.
    ADC->CTL0   = 0;
    ADC->CTL0   = ADC_CTL0_SHT0_4 | ADC_CTL0_MSC | ADC_CTL0_ON;
    ADC->CTL1   = ADC_CTL1_START(0) | ADC_CTL1_SHP | ADC_CTL1_SSEL_MCLK | ADC_CTL1_CONSEQ_SINGLE;
    ADC->CTL2   = ADC_CTL2_TCOFF + ADC_CTL2_RES_MAX;
    ADC->MCTL0  = OT_GWNADC_PINNUM;
    ADC->CTL0  |= ADC_CTL0_ENC;

    /// 2. Turn-on Zener noisemaker, if enabled by board
#   ifdef OT_GWNZENER_PORT
        OT_GWNZENER_PORT->DOUT |= OT_GWNZENER_PIN;
#   endif

    /// 3. Do Conversion!  Loop until the required number of bytes are produced.
    ///    The random bytes are produced by shifting-in the least-significant
    ///    sections of each sample (exactly how many bits is board-defined).
    while (--bytes_out >= 0) {
        ot_u8 reg;
#       if (OT_GWNADC_BITS == 8)
            //Special case for direct synthesis of random bytes.
            ADC->CTL0  |= ADC_CTL0_SC;  //start conversion
            while (ADC->CTL1 & ADC_CTL1_BUSY);
            reg = (ot_u8)ADC->MEM0;

#       else
            ot_u8 shifts;
            shifts = ((8+(OT_GWNADC_BITS-1)) / OT_GWNADC_BITS);
            while (shifts-- != 0) {
                ADC->CTL0  |= ADC_CTL0_SC;  //start conversion
                while (ADC->CTL1 & ADC_CTL1_BUSY);
                reg   <<= OT_GWNADC_BITS;
                reg    |= ((1<<OT_GWNADC_BITS)-1) & ADC->MEM0;
            }
#       endif
        
        *rand_out++ = reg;
    }

    ///5. Shut down ADC, turn-off Zener (if enabled), turn-off pin (if enabled)
    ADC->CTL0              = 0;
    OT_GWNADC_PORT->DDIR  |= OT_GWNADC_PIN;
    OT_GWNADC_PORT->DOUT  &= ~OT_GWNADC_PIN;

#   ifdef OT_GWNZENER_PORT
        OT_GWNZENER_PORT->DOUT &= ~OT_GWNZENER_PIN;
#   endif
}


void platform_init_prand(ot_u16 seed) {
    platform_ext.prand_reg = seed;
}

ot_u8 platform_prand_u8() {
    return (ot_u8)platform_prand_u16();
}

ot_u16 platform_prand_u16() {
/// Run the HW CRC on the prand register stored value.  Always save the value in
/// the CRC HW and return it when the process is done.  prand_reg should be
/// initialized at startup with the device ID or serial number (or something)
    ot_u16 scratch          = CRC->INIRES;
    CRC->INIRES             = platform_ext.prand_reg;
    CRCb->DIRB_L            = UCSCTL0_L;        // contains rotating FLL modulation value
    platform_ext.prand_reg  = CRC->INIRES;
    CRC->INIRES             = scratch;

    return platform_ext.prand_reg;
}





/** Platform memcpy, memset routines <BR>
  * ========================================================================<BR>
  * Similar to standard implementation of "memcpy" and "memset"
  * Behavior is always blocking
  */

void platform_memcpy(ot_u8* dest, ot_u8* src, ot_int length) {
#if (OS_FEATURE(MEMCPY) == ENABLED)
    memcpy(dest, src, length);

#elif (MCU_FEATURE(MEMCPYDMA) == ENABLED)
/// DMA driven method: CC430 DMA Block Transfer is blocking, and the CPU is
/// stopped during the data movement.  Thus the while loop is not needed.
/// If using DMA for memcpy in addition to MPipe, the throughput of MPipe is
/// limited to (in practice) about 39000 BYTES/SEC (so, 390kbaud for UART)
    MEMCPY_DMA->SA_L    = (ot_u16)src;
    MEMCPY_DMA->DA_L    = (ot_u16)dest;
    MEMCPY_DMA->SZ      = length;
    MEMCPY_DMA->CTL     = ( DMA_Mode_Block | \
                            DMA_DestinationInc_Enable | \
                            DMA_SourceInc_Enable | \
                            DMA_DestinationDataSize_Byte | \
                            DMA_SourceDataSize_Byte | \
                            DMA_TriggerLevel_RisingEdge | \
                            0x11);
    //while ((MEMCPY_DMA->CTL & DMAIFG) == 0);
    
#else
/// Uses the "Duff's Device" for loop unrolling.  If this is incredibly
/// confusing to you, check the internet for "Duff's Device."
    if (length != 0) {
        ot_int loops = (length + 3) >> 2;

        switch (length & 3) {
            case 0: do {    *dest++ = *src++;
            case 3:         *dest++ = *src++;
            case 2:         *dest++ = *src++;
            case 1:         *dest++ = *src++;
                        }
                        while (--loops > 0);
        }
    }

#endif
}



void platform_memcpy_2(ot_u16* dest, ot_u16* src, ot_int length) {
#if (MCU_FEATURE(MEMCPYDMA) == ENABLED)
/// DMA driven method: CC430 DMA Block Transfer is blocking, and the CPU is
/// stopped during the data movement.  Thus the while loop is not needed.
/// DMA memcpy cannot be used reliably if OpenTag makes use of the DMA for some
/// other non-blocking process (e.g. MPipe).
    MEMCPY_DMA->SA_L    = (ot_u16)src;
    MEMCPY_DMA->DA_L    = (ot_u16)dest;
    MEMCPY_DMA->SZ      = length;
    MEMCPY_DMA->CTL     = ( DMA_Mode_Block | \
                            DMA_DestinationInc_Enable | \
                            DMA_SourceInc_Enable | \
                            DMA_DestinationDataSize_Word | \
                            DMA_SourceDataSize_Word | \
                            DMA_TriggerLevel_High | \
                            0x11);
    //while ((MEMCPY_DMA->CTL & DMAIFG) == 0);
    //DMA must be manually cleared on zero-length copy
    MEMCPY_DMA->CTL    &= ~(0x10);

#else
    platform_memcpy((ot_u8*)dest, (ot_u8*)src, length<<1);

#endif
}



void platform_memset(ot_u8* dest, ot_u8 value, ot_int length) {
#if (OS_FEATURE(MEMCPY) == ENABLED)
    memset(dest, value, length);

#elif (MCU_FEATURE(MEMCPYDMA) == ENABLED)
/// DMA driven method: CC430 DMA Block Transfer is blocking, and the CPU is
/// stopped during the data movement.  Thus the while loop is not needed.
/// DMA memcpy cannot be used reliably if OpenTag makes use of the DMA for some
/// other non-blocking process (e.g. MPipe).
    MEMCPY_DMA->SA_L    = (ot_u16)&value;
    MEMCPY_DMA->DA_L    = (ot_u16)dest;
    MEMCPY_DMA->SZ      = length;
    MEMCPY_DMA->CTL     = ( DMA_Mode_Block | \
                            DMA_DestinationInc_Enable | \
                            DMA_SourceInc_Disable | \
                            DMA_DestinationDataSize_Byte | \
                            DMA_SourceDataSize_Byte | \
                            DMA_TriggerLevel_High | \
                            0x11);
    //while ((MEMCPY_DMA->CTL & DMAIFG) == 0);
    //DMA must be manually cleared on zero-length copy
    MEMCPY_DMA->CTL    &= ~(0x10);

#else
/// Uses the "Duff's Device" for loop unrolling.  If this is incredibly
/// confusing to you, check the internet for "Duff's Device."
    if (length != 0) {
        ot_int loops = (length + 3) >> 2;

        switch (length & 3) {
            case 0: do {    *dest++ = value;
            case 3:         *dest++ = value;
            case 2:         *dest++ = value;
            case 1:         *dest++ = value;
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

void platform_block(ot_u16 sti) {
/// Low power blocking function.
/// Drop clock to a very low amount, then do a pointless DMA transfer.  The DMA
/// transfer is well known as 2 clocks per transfer.  The input sti should be
/// 10 or larger in order for it to be accurate (~300us).
#if 0 // (MCU_FEATURE(MEMCPYDMA) == ENABLED)
    ot_u16 a, b;
    ot_int cycles;
    
    cycles              = (ot_int)((sti>>1)-5);
    MEMCPY_DMA->SZ      = (cycles < 0) ? 0 : cycles;
    MEMCPY_DMA->SA_L    = (ot_u16)&a;
    MEMCPY_DMA->DA_L    = (ot_u16)&b;
    
    //drop CPU clock to 32768Hz
    UCS->CTL4   = (CLKSRC_32768 << clockACLK) | \
                  (CLKSRC_32768 << clockMCLK) | \
                  (sourceDCO << clockSMCLK);
    
    // Do meaningless, repetitive copy between "a" and "b"
    MEMCPY_DMA->CTL     = ( DMA_Mode_Block | \
                            DMA_DestinationInc_Disable | \
                            DMA_SourceInc_Disable | \
                            DMA_DestinationDataSize_Word | \
                            DMA_SourceDataSize_Word | \
                            DMA_TriggerLevel_High | \
                            0x11);

    //bring back clock to 20 MHz (or whatever)
    UCS->CTL4   = (CLKSRC_32768 << clockACLK) | \
                  (sourceDCO << clockMCLK) | \
                  (sourceDCO << clockSMCLK);

    //DMA must be manually cleared on zero-length copy
    MEMCPY_DMA->CTL &= ~(0x10);

#else
    while(sti-- != 0) {
        __delay_cycles(600);
    }
#endif
}


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
    while (n-- != 0) {
        __delay_cycles(20000);
    }
}


void platform_swdelay_us(ot_u16 n) {
    while (n-- != 0) {
        __delay_cycles(20);
    }
}





