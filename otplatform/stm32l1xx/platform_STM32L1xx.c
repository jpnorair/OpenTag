/* Copyright 2013 JP Norair
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
  * @file       /otplatform/stm32l1xx/platform_STM32L1xx.c
  * @author     JP Norair
  * @version    V1.0
  * @date       14 Feb 2013
  * @brief      Platform Library Macros and Functions for STM32L
  * @ingroup    Platform
  *
  ******************************************************************************
  */

#include "OT_utils.h"
#include "OT_types.h"
#include "OT_config.h"
#include "OT_platform.h"

// OT modules that need initialization
#include "veelite.h"
#include "veelite_core.h"
#include "buffers.h"
#include "auth.h"
#include "mpipe.h"
#include "radio.h"
#include "system.h"
#include "session.h"



//API wrappers
void otapi_poweron()    { platform_poweron(); }
void otapi_poweroff()   { platform_poweroff(); }
void otapi_init()       { platform_init_OT(); }
void otapi_exec()       { platform_ot_run(); }
void otapi_preempt()    { platform_ot_preempt(); }
void otapi_pause()      { platform_ot_pause(); }

#ifndef EXTF_otapi_led1_on
void otapi_led1_on() { platform_trig1_high(); }
#endif
#ifndef EXTF_otapi_led2_on
void otapi_led2_on() { platform_trig2_high(); }
#endif
#ifndef EXTF_otapi_led1_off
void otapi_led1_off() { platform_trig1_low(); }
#endif
#ifndef EXTF_otapi_led2_off
void otapi_led2_off() { platform_trig2_low(); }
#endif






/** Context Caching Parameters <BR>
  * =======================================================================<BR>
  * STM32L and all other Cortex M devices have a hardware mechanism to assist
  * the caching of multiple contexts and levels of interrupt preemption.
  *
  * GULP kernels should use __CM3_NVIC_GROUPS == 1, which will create a single,
  * global interrupt context.  Tasks must manage their own contexts.
  *
  * HICCULP kernels can use __CM3_NVIC_GROUPS > 1, but each extra group (each 
  * extra context) will require the allocation of extra stack.
  *
  * Usually, these configuration parameters are set in the compiler or in the 
  * build_config.h file.  If they are not set, defaults are used.
  */
#if (__CM3_NVIC_GROUPS < 2)
#   define _GROUP_PRIORITY  7
#elif (__CM3_NVIC_GROUPS < 4)
#   define _GROUP_PRIORITY  6
#elif (__CM3_NVIC_GROUPS < 8)
#   define _GROUP_PRIORITY  5
#elif (__CM3_NVIC_GROUPS < 16)
#   define _GROUP_PRIORITY  4
#else
#   error "Cortex M3 may support more than 8 NVIC Groups, but STM32L supports no more than 8"
#endif




/** Clocking Constants <BR>
  * ========================================================================<BR>
  */
  
#define POWER_1V8   0x0800
#define POWER_1V5   0x1000
#define POWER_1V2   0x1800
  
// error checks
#if (BOARD_FEATURE(HFXTAL) && BOARD_FEATURE(HFBYPASS))
#   error "BOARD_FEATURE_HFXTAL and BOARD_FEATURE_HFBYPASS cannot be both ENABLED."
#endif
#if (MCU_CONFIG(USB) && (BOARD_FEATURE(PLL) != ENABLED))
#   error "To use built-in USB, you must ENABLE the PLL"
#endif
#if (MCU_CONFIG(USB) && (PLATFORM_PLLCLOCK_OUT != 96000000))
#   error "STM32L requires PLL output to be 96 MHz when using internal USB."
#endif

// If GPTIM/KTIM uses RTC as a time source, we need to keep it open,
// and unfortunately this access tends to be in the same place as clocking.
#if (OT_GPTIM_ID == 'R')
#   define _RTC_PROTECTION  PWR_CR_DBP
#else
#   define _RTC_PROTECTION  0
#endif

// Flankspeed uses PLL, which requires 1.8V
#if (BOARD_FEATURE(FLANKSPEED) || BOARD_FEATURE(PLL))
#   define _USE_PLL
#   define _PLL_SRC             ((BOARD_FEATURE(FLANKXTAL) == ENABLED) << 16)
#   define _FLANKSPEED_VOLTAGE  POWER_1V8
#   if (BOARD_PARAM(PLLmult) == 3)
#       define _PLL_MULT    (0<<18)
#   elif (BOARD_PARAM(PLLmult) == 4)
#       define _PLL_MULT    (1<<18)
#   elif (BOARD_PARAM(PLLmult) == 6)
#       define _PLL_MULT    (2<<18)
#   elif (BOARD_PARAM(PLLmult) == 8)
#       define _PLL_MULT    (3<<18)
#   elif (BOARD_PARAM(PLLmult) == 12)
#       define _PLL_MULT    (4<<18)
#   elif (BOARD_PARAM(PLLmult) == 16)
#       define _PLL_MULT    (5<<18)
#   elif (BOARD_PARAM(PLLmult) == 24)
#       define _PLL_MULT    (6<<18)
#   elif (BOARD_PARAM(PLLmult) == 32)
#       define _PLL_MULT    (7<<18)
#   elif (BOARD_PARAM(PLLmult) == 48)
#       define _PLL_MULT    (8<<18)
#   else
#       error "PLL Multiplier from BOARD_PARAM_PLLmult is out of range"
#   endif
#   if (BOARD_PARAM(PLLdiv) == 2)
#       define _PLL_DIV    (1<<22)
#   elif (BOARD_PARAM(PLLdiv) == 3)
#       define _PLL_DIV    (2<<22)
#   elif (BOARD_PARAM(PLLdiv) == 4)
#       define _PLL_DIV    (3<<22)
#   else
#       error "PLL Divider from BOARD_PARAM_PLLdiv is out of range"
#   endif
#   if (PLATFORM_PLLCLOCK_HZ > 32000000)
#       error "PLL Flank Speed Clock must be less than 32 MHz"
#   elif (PLATFORM_PLLCLOCK_HZ > 16000000)
#       define _FLANKSPEED_FLASHWAIT ENABLED
#   else
#       define _FLANKSPEED_FLASHWAIT DISABLED
#   endif
#   if BOARD_FEATURE(FLANKXTAL)
#       define _FLANKOSC_RDYFLAG        RCC_CR_HSERDY
#       define _FLANKOSC_CLOCKBIT       3
#       if BOARD_FEATURE(HFBYPASS)
#           define _FLANKOSC_ONBIT      (RCC_CR_HSEON | RCC_CR_HSEBYP)
#           define _FLANKOSC_TIMEOUT    1000
#       else
#           define _FLANKOSC_ONBIT      RCC_CR_HSEON
#           define _FLANKOSC_TIMEOUT    3000 //HSE_STARTUP_TIMEOUT
#       endif
#   else
#       define _FLANKOSC_ONBIT      RCC_CR_HSION
#       define _FLANKOSC_RDYFLAG    RCC_CR_HSIRDY
#       define _FLANKOSC_CLOCKBIT   3
#       define _FLANKOSC_TIMEOUT    300 //HSI_STARTUP_TIMEOUT
#   endif
#else
#   define _PLL_SRC     0
#   define _PLL_MULT    0
#   define _PLL_DIV     0

#endif

// Fullspeed uses HSE or HSI without PLL
#if BOARD_FEATURE(FULLSPEED)
#   if (PLATFORM_HSCLOCK_HZ > 32000000)
#       error "High Speed Clock must be less than 32 MHz"
#   elif (PLATFORM_HSCLOCK_HZ > 16000000)
#       define _FULLSPEED_VOLTAGE   POWER_1V8
#       define _FULLSPEED_FLASHWAIT ENABLED
#   elif (PLATFORM_HSCLOCK_HZ > 8000000)
#       define _FULLSPEED_VOLTAGE   POWER_1V5
#       define _FULLSPEED_FLASHWAIT ENABLED
#   elif (PLATFORM_HSCLOCK_HZ > 4000000)
#       define _FULLSPEED_VOLTAGE   POWER_1V5
#       define _FULLSPEED_FLASHWAIT DISABLED
#   elif (PLATFORM_HSCLOCK_HZ > 2000000)
#       define _FULLSPEED_VOLTAGE   POWER_1V2
#       define _FULLSPEED_FLASHWAIT ENABLED
#   else
#       define _FULLSPEED_VOLTAGE   POWER_1V2
#       define _FULLSPEED_FLASHWAIT DISABLED
#   endif
#   if BOARD_FEATURE(FULLXTAL)
#       define _FULLOSC_RDYFLAG         RCC_CR_HSERDY
#       define _FULLOSC_CLOCKBIT        2
#       if BOARD_FEATURE(HFBYPASS)
#           define _FULLOSC_ONBIT       (RCC_CR_HSEON | RCC_CR_HSEBYP)
#           define _FULLOSC_TIMEOUT     1000
#       else
#           define _FULLOSC_ONBIT       RCC_CR_HSEON
#           define _FULLOSC_TIMEOUT     3000 //HSE_STARTUP_TIMEOUT
#       endif
#   else
#       define _FULLOSC_ONBIT      RCC_CR_HSION
#       define _FULLOSC_RDYFLAG    RCC_CR_HSIRDY
#       define _FULLOSC_CLOCKBIT   1
#       define _FULLOSC_TIMEOUT    300 //HSI_STARTUP_TIMEOUT
#   endif
#endif

// Standard Speed uses the MSI
#if BOARD_FEATURE(STDSPEED)
#   if (   (PLATFORM_MSCLOCK_HZ != 4200000)   \
        && (PLATFORM_MSCLOCK_HZ != 2100000)   \
        && (PLATFORM_MSCLOCK_HZ != 1050000)   \
        && (PLATFORM_MSCLOCK_HZ != 524000)    \
        && (PLATFORM_MSCLOCK_HZ != 262000)    \
        && (PLATFORM_MSCLOCK_HZ != 131000)    \
        && (PLATFORM_MSCLOCK_HZ != 655000)  )
#       error "PLATFORM_MSCLOCK_HZ is not set to a value matching HW options"
#   endif
#   if (PLATFORM_MSCLOCK_HZ > 4200000)
#       error "Mid Speed Clock does not support higher than 4.2 MHz."
#   elif (PLATFORM_MSCLOCK_HZ > 4000000)
#       define _STDSPEED_VOLTAGE    POWER_1V5
#       define _STDSPEED_FLASHWAIT  DISABLED
#   elif (PLATFORM_MSCLOCK_HZ > 2000000)
#       define _STDSPEED_VOLTAGE    POWER_1V2
#       define _STDSPEED_FLASHWAIT  ENABLED
#   else 
#       define _STDSPEED_VOLTAGE    POWER_1V2
#       define _STDSPEED_FLASHWAIT  DISABLED
#   endif
#endif

// For systems with only FLANKSPEED enabled, drop APB clocks by half
#if ( BOARD_FEATURE(FLANKSPEED)             \
  && (BOARD_FEATURE(STDSPEED) != ENABLED)   \
  && (BOARD_FEATURE(FULLSPEED) != ENABLED)  )
#   undef BOARD_PARAM_AHBCLKDIV
#   define BOARD_PARAM_AHBCLKDIV 2
#   define _APB1SCALE    0
#   define _APB2SCALE    0
#else
#   define _APB1SCALE    0
#   define _APB2SCALE    0
#endif

//Validate and set AHB Divider based on board config header setting
#if (BOARD_PARAM_AHBCLKDIV == 1)
#   define _AHB_DIV     (0<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 2)
#   define _AHB_DIV     (8<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 4)
#   define _AHB_DIV     (9<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 8)
#   define _AHB_DIV     (10<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 16)
#   define _AHB_DIV     (11<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 64)
#   define _AHB_DIV     (12<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 128)
#   define _AHB_DIV     (13<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 256)
#   define _AHB_DIV     (14<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 512)
#   define _AHB_DIV     (15<<4)
#else
#   error "BOARD_PARAM_AHBCLKDIV not set to a value permitted by this HW"
#endif

//Validate and set APB1 Divider based on board config header setting
#if (BOARD_PARAM_APB1CLKDIV == 1)
#   define _APB1_DIV    ((0+_APB1SCALE)<<8)
#elif (BOARD_PARAM_APB1CLKDIV == 2)
#   define _APB1_DIV    ((4+_APB1SCALE)<<8)
#elif (BOARD_PARAM_APB1CLKDIV == 4)
#   define _APB1_DIV    ((5+_APB1SCALE)<<8)
#elif (BOARD_PARAM_APB1CLKDIV == 8)
#   define _APB1_DIV    ((6+_APB1SCALE)<<8)
#elif (BOARD_PARAM_APB1CLKDIV == 16)
#   define _APB1_DIV    (7<<8)
#else
#   error "BOARD_PARAM_APB1CLKDIV not set to a value permitted by this HW"
#endif

//Validate and set APB2 Divider based on board config header setting
#if (BOARD_PARAM_APB2CLKDIV == 1)
#   define _APB2_DIV    ((0+_APB2SCALE)<<11)
#elif (BOARD_PARAM_APB2CLKDIV == 2)
#   define _APB2_DIV    ((4+_APB2SCALE)<<11)
#elif (BOARD_PARAM_APB2CLKDIV == 4)
#   define _APB2_DIV    ((5+_APB2SCALE)<<11)
#elif (BOARD_PARAM_APB2CLKDIV == 8)
#   define _APB2_DIV    ((6+_APB2SCALE)<<11)
#elif (BOARD_PARAM_APB2CLKDIV == 16)
#   define _APB2_DIV    (7<<11)
#else
#   error "BOARD_PARAM_APB2CLKDIV not set to a value permitted by this HW"
#endif






/** Macros of Ill Repute <BR>
  * ========================================================================<BR>
  */
#define __SET_PENDSV()      (SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk)
#define __CLR_PENDSV()      (SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk)
#define __SEND_SVC(VAL)     asm volatile(" svc  %0" : : "I"(VAL) );






/** Platform Data <BR>
  * ========================================================================<BR>
  */
platform_struct     platform;
platform_ext_struct platform_ext;





/** Local Subroutines <BR>
  * ========================================================================<BR>
  */
void sub_voltage_config(ot_u16 pwr_cr_vos_x) {
/// Set Power Configuration based on Voltage Level parameters.
/// Input must be: POWER_1V2, POWER_1V5, POWER_1V8
/// Additionally, PWR_CR_DBP can be ORed in for RTC hacking
    ot_u16 scratch;
    // Power should be enabled by periphclk function, not here
    //RCC->APB1ENR   |= RCC_APB1ENR_PWREN;    
    scratch     = PWR->CR & ~(ot_u32)((3<<11) | (3<<5));
    scratch    |= pwr_cr_vos_x;
    PWR->CR     = scratch;
    
    // Wait Until the Voltage Regulator is ready
    while((PWR->CSR & PWR_CSR_VOSF) != 0) { }
}


void sub_osc_startup(ot_u16 counter, ot_u32 osc_mask) {
    ///@todo figure out a way to do this with WFE
    // Wait for Oscillator to get ready, counter goes to 0 on failure
    RCC->CR    |= osc_mask;
    osc_mask  <<= 1; 
    while ( ((RCC->CR & osc_mask) == 0) && (--counter) );
    
    /// Test if oscillator failed to startup
    if (counter == 0) {
        ///@todo Death message / Death Blinkly
    }
}


void sub_osc_setclock(ot_u32 clock_mask) {
    ot_u32 scratch;
    scratch         = RCC->CFGR & ~3;
    scratch        |= clock_mask;
    clock_mask    <<= 2; 
    RCC->CFGR       = scratch;
    while ( (RCC->CFGR & clock_mask) != clock_mask);
}


void sub_save_error(ot_u16 vector_code) {
/// Save the high-level code and low-level code in backup RAM, and then reset
    ot_u32 error_code;
    error_code  = ((ot_u32)platform.error_code << 16) | (ot_u32)vector_code;
    RTC->BKP19R = error_code;
    RTC->BKP18R = __get_PSP();
    RTC->BKP17R = __get_MSP();
    
    NVIC_SystemReset();     //see core_cm3.h in CMSIS
}















/** Extended Platform (STM32L-specific) power and PLL control<BR>
  * ========================================================================<BR>
  */
#ifndef EXTF_platform_ext_wakefromstop
void platform_ext_wakefromstop() {
    // Always start chrono when coming out of STOP
    gptim_start_chrono();
    
    // Use the lowest allowed speed coming from STOP
#   if BOARD_FEATURE(STDSPEED)
        platform_standard_speed();
#   elif BOARD_FEATURE(FULLSPEED)
        platform_full_speed();
#   elif BOARD_FEATURE(FLANKSPEED)
        platform_flank_speed();
#   endif
}
#endif


#ifndef EXTF_platform_ext_pllon
void platform_ext_pllon() {
#if BOARD_FEATURE(PLL)
    sub_voltage_config((POWER_1V8 | PWR_CR_DBP | 0x0040));
    
    BOARD_HSXTAL_ON();
    sub_osc_startup(_FLANKOSC_TIMEOUT, _FLANKOSC_ONBIT);

    RCC->CR |= RCC_CR_PLLON;
    while((RCC->CR & RCC_CR_PLLRDY) == 0) { }
#endif
}
#endif


#ifndef EXTF_platform_ext_plloff
void platform_ext_plloff() {
#if BOARD_FEATURE(PLL)
/// Don't call this function unless you know what you are doing.  STM32L will
/// not shut-off an active clock, so you won't kill your app, but worse: the
/// PLL will stay on even if you probably think it is off.
    RCC->CR &= ~RCC_CR_PLLON;
    BOARD_HSXTAL_OFF();
#endif
}
#endif


#ifndef EXTF_platform_ext_hsitrim
void platform_ext_hsitrim() {
/// Calibrate the HSI clock against LSE.
/// This feature is presently only available through Haystack Distribution of
/// OpenTag (HDO).  Contact Haystack Technologies for more information.
}
#endif



#ifndef EXTF_platform_ext_lsihz
ot_u16 platform_ext_lsihz() {
/// Calibrate the LSI clock against HSE or HSI.
/// This feature is presently only available through Haystack Distribution of
/// OpenTag (HDO).  Contact Haystack Technologies for more information.
    return 37000;
}
#endif










/** Platform Interrupts <BR>
  * ========================================================================<BR>
  * GPTIM, RTC, SysTick interrupts (kernel interrupts), plus any power 
  * management interrupts that might be used.  These interrupts are important
  * to the platform.  You can move them out of the platform module, into a
  * global interrupt handler C file if you want, but it is better to leave the
  * OpenTag resources allocated to OpenTag if you can.  STM32's tend to have
  * plenty of resources, so the only one you might want to use is the RTC, but
  * you can also just refer to the 
  */

/// The NMI is triggerable by software.  It is used to manage runtime errors.
void NMI_Handler(void) {
    ot_int code;
    
#   ifdef __DEBUG__
    if (platform.error_code == 0)
        while(1);
#   endif
    
    code                = platform.error_code;
    platform.error_code = 0;
    sys_panic(code);
}


void HardFault_Handler(void) {
/// If you have traced the code here, most likely the problem is stack overrun.
/// You need to allocate a bigger stack for SSTACK or your thread, or fix some
/// other serious problem (possibly interrupt storm)
#ifdef __DEBUG__
    while (1);
    
#else
    /// Log HardFault by saving an error code to Backup RAM, then resetting.
    /// The startup routine will check this sector of backup RAM and do the
    /// actual logging.
    /// @note Backup RAM *MUST* be enabled (normally done in RTC/GPTIM init)
    /// @todo Finish this implementation, it is not tested yet
    sub_save_error(1);
#endif
}


void MemManage_Handler(void) {
/// MemManage Fault is analagous to Segmentation Fault.  OpenTag doesn't use
/// Protected memory spaces at this time, so MemManage fault should never occur.
#ifdef __DEBUG__
    while (1);
#else
    sub_save_error(11);
#endif
}


void BusFault_Handler(void) {
/// BusFault is analagous to Bus Error.
#ifdef __DEBUG__
    while (1);
#else
    sub_save_error(7);
#endif
}


void UsageFault_Handler(void) {
/// UsageFault is the "kitchen sink" error.  Mostly, it will happen on 
/// misaligned memory access, but also on divide by zero operations.
#ifdef __DEBUG__
    while (1);
#else
    sub_save_error(10);
#endif
}


void DebugMon_Handler(void) { 
/// Not really applicable
}



/** KERNEL INTERRUPTS
  * =================
  */
void SVC_Handler(void) { 
/// At present, the only protected system call is the call to the scheduler.
/// (sys_event_manager()).  Future system calls might make-use of the SVC
/// vectoring, however.
    sys_event_manager();
    
#if 0 //(OT_PARAM_SYSTHREADS != 0)
    ot_u8* task_lr;
    register ot_u32* stack;
    
    // Caller of SVC should be a task on the P-stack
    asm volatile ("   MRS  %0, PSP" : "=r"(stack) );
    
    // 6 word offset is where LR from caller stack is stored.  
    // LR will be instruction after SVC
    task_lr = (ot_u8*)(stack[6]);
    
    // The 16bit Thumb2 instruction before the LR address is the SVC.
    // The lower byte of this instruction is the immediate code
    switch (task_lr[-2]) {
    
        // Call 0: this is the call to the scheduler
        case 0: 
            SVC_Handler_eventmgr:
            break;
        
        // Call 1: Task Killer
        case 1: //sys_kill()
        
        // Other calls are process notifications
       default: //sys_notify();
                break;  //goto SVC_Handler_eventmgr;
    }
#endif
}



void PendSV_Handler(void) { 
/// PendSV ISR is issued if and only if some task or event is asynchronously 
/// pre-empting the kernel.  It is invoked __only__ through platform_ot_preempt().
/// @note SV is for "Supervisor," not "Sport Veloce"
   
    // There is an erratum that PendSV bit is not adequately cleared in HW
    __CLR_PENDSV();
    
    // halt system until RTC is known to be ready
    
    // Disable interrupts during scheduler runtime
    __SEND_SVC(0);
}






/** MAINLINE OPENTAG DRIVER INTERRUPTS
  * ==================================
  */


#ifndef EXTF_WWDG_IRQHandler
void WWDG_IRQHandler(void) {
}
#endif









/** Platform Enablers <BR>
  * ========================================================================<BR>
  */
#ifndef EXTF_platform_poweron
void platform_poweron() {
    
    ///1. Set cooperative task runtime in P-stack,
    ///   Interrupts and protected system calls run in M-stack
    __set_PSP( __get_MSP() );
    __set_CONTROL(2);
    __set_MSP( (ot_u32)&platform_ext.sstack[(OT_PARAM_SSTACK_ALLOC/4)-1] );
    
    /// 3. Board-Specific power-up configuration
    BOARD_PERIPH_INIT();
    BOARD_POWER_STARTUP();
    
    ///2. Configure GPIO
    //platform_init_gpio();
    BOARD_PORT_STARTUP();
    
    /// 3. Configure Clocks
    platform_init_busclk();
    platform_init_periphclk();
    
    /// 5. Debugging setup
#   if defined(__DEBUG__) || defined(__PROTO__)
    DBGMCU->CR     |= ( DBGMCU_CR_DBG_SLEEP \
                      | DBGMCU_CR_DBG_STOP \
                      | DBGMCU_CR_DBG_STANDBY);
    
    DBGMCU->APB1FZ |= ( DBGMCU_APB1_FZ_DBG_RTC_STOP \
                      | DBGMCU_APB1_FZ_DBG_WWDG_STOP \
                      | DBGMCU_APB1_FZ_DBG_IWDG_STOP);

    DBGMCU->APB2FZ |= ( DBGMCU_APB2_FZ_DBG_TIM9_STOP \
                      | DBGMCU_APB2_FZ_DBG_TIM10_STOP \
                      | DBGMCU_APB2_FZ_DBG_TIM11_STOP);
#   endif

    /// 6. Final initialization of OpenTag system resources
               // Set up connections on the board
    platform_init_interruptor();    // Interrupts OpenTag cares about
    platform_init_gptim(0);         // Initialize GPTIM (to 1024 Hz)
    
    /// 7. Start the chronometer if not in debug mode
#   if !defined(__DEBUG__)
    gptim_start_chrono();
#   endif
    
    /// 8. Initialize Low-Level Drivers (worm, mpipe)
    // Restore vworm (following save on shutdown)
    vworm_init();
}
#endif


#ifndef EXTF_platform_poweroff
void platform_poweroff() {
    ISF_syncmirror();
    vworm_save();
    
#   if ( OT_FEATURE(MPIPE) && MCU_CONFIG(MPIPECDC) )
    mpipe_disconnect(NULL);
#   endif
}
#endif



#ifndef EXTF_platform_init_OT
void platform_init_OT() {
    /// 1. Initialize Data sources required by basically all OT features
    ///    - Buffers module allocates the data queues, used by all I/O
    ///    - Veelite module allocates and prepares the filesystem
    buffers_init();
    vl_init();

    /// 2. Initialize the RTC, if available.
#   if (OT_FEATURE(RTC))
        platform_init_rtc(364489200);
#   endif
    
    /// 3. Look for errors, report them, and make sure to clear the error RAM.
    /// @note This is a decent place to put a breakpoint when debugging.
    if (RTC->BKP19R != 0) {
        vlFILE*     fp;
        ot_uni16    resets;
        ot_uni16    faults;
        ot_uni32    error;
        
        error.ulong     = RTC->BKP19R;
        RTC->BKP19R     = 0;
        fp              = ISF_open_su(ISF_ID(hardware_fault_status));
        resets.ushort   = vl_read(fp, 0);
        faults.ushort   = vl_read(fp, 2);
        resets.ubyte[0]+= (error.ushort[LOWER] != 0);
        resets.ubyte[1]+= (error.ushort[UPPER] != 0);
        faults.ubyte[1]+= (error.ushort[LOWER] == 11) << 1; //Memory Fault
        
        vl_write(fp, 0, resets.ushort);
        vl_write(fp, 2, faults.ushort);
        vl_close(fp);
    }    
    
    /// 4. Initialize the System (Kernel & more).  The System initializer must
    ///    initialize all modules that are built onto the kernel.  These include
    ///    the DLL and MPipe.
    sys_init();
    
    /// 5. If debugging, copy the UNIQUE ID that ST writes into the ROM into
    ///    the lower 48 bits of the Mode 2 UID (Device Settings ISF)
    ///
    /// @note the ID is inserted via Veelite, so it is abstracted from the 
    /// file memory configuration of your board and/or app. 
#   if (defined(__DEBUG__) || defined(__PROTO__))
    {
        vlFILE* fpid;
        ot_u16* hwid;
        ot_int  i;
        
        fpid    = ISF_open_su(ISF_ID(device_features));
        hwid    = (ot_u16*)(0x1FF80050);
        for (i=6; i!=0; i-=2) {
            vl_write(fpid, i, *hwid++);
        }
        vl_close(fpid);
    }
#   else
    ///@note for production (__RELEASE__) the default UID should be written to
    ///      the default file location by the manufacturer firmware upload.

#   endif

    ///5. This prevents the scheduler from getting called by a preemption
    ///   event until it officially begins.  It allows some tasks to be
    ///   used for special purposes at power-on (namely MPipe).
    platform_ext.task_exit = (void*)__get_PSP();
}
#endif



#ifndef EXTF_platform_init_busclk
void platform_init_busclk() {
/// This function should be called during initialization and restart, right at
/// the top of platform_poweron().
    ot_u16 counter;
    
    ///1. RESET System Clocks
    ///@todo This may not be necessary.  These settings should be reset default settings.
    
    // Assure MSI bit is on (it should be, by default)
    RCC->CR    |= (uint32_t)0x00000100;
    
    // Configure dividers and PLL information (even if not used) and keep active clock
    // via MSI (these fields are 0)
    RCC->CFGR   = ( _PLL_SRC | _PLL_MULT | _PLL_DIV | _AHB_DIV | _APB1_DIV | _APB2_DIV );    

    // Reset HSION, HSEON, HSEBYP, CSSON and PLLON bits
    // Disable all clocker interrupts (default)
    RCC->CR    &= (uint32_t)0xEEFAFFFE;
    RCC->CIR    = 0x00000000;


    ///2. Prepare external Memory bus (not currently supported)
//#   ifdef DATA_IN_ExtSRAM
//        SystemInit_ExtMemCtl(); 
//#   endif
    
    ///3a. Begin clocking system with MSI clock at specified frequency.
    ///    <LI> Specified as PLATFORM_MSCLOCK_HZ in board support header </LI>
    ///    <LI> MSI is only used as standard clock if BOARD_FEATURE_STDSPEED
    ///           is also ENABLED in board support header.
#   if BOARD_FEATURE(STDSPEED)
        // Change MSI to required frequency
#       if (PLATFORM_MSCLOCK_HZ == 4200000)
        sub_voltage_config((POWER_1V5 | PWR_CR_DBP));
        FLASH->ACR  = FLASH_ACR_ACC64;
        FLASH->ACR |= FLASH_ACR_PRFTEN;
        RCC->ICSCR ^= 0x00006000;               //setting 110
        
#       elif (PLATFORM_MSCLOCK_HZ == 2100000)
        FLASH->ACR  = FLASH_ACR_ACC64;
        FLASH->ACR |= (FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY);
        sub_voltage_config((POWER_1V2 | PWR_CR_DBP));
        //RCC->ICSCR |= 0x00005000;                //setting 101 (default)
        
#       else
#       error "MSI speeds below 2.1 MHz not supported currently."
        
#       endif

        // Setup the Bus Dividers as specified (MSI already selected as system clock)
        sub_set_clockhz(PLATFORM_MSCLOCK_HZ);


    ///3b. Use HSE or HSI without PLL as Full-Speed clock
    ///    <LI> HSE or HSI is full speed clock if BOARD_FEATURE_STDSPEED is 
    ///           DISABLED and BOARD_FEATURE_FULLSPEED is ENABLED</LI>
    ///    <LI> HSE is used if BOARD_FEATURE_HFXTAL is ENABLED, else HSI used. </LI>
    ///    <LI> Boards using HSE can declare any value into PLATFORM_HSCLOCK_HZ.
    ///           Board using HSI may only declare 2, 4, 8, or 16 MHz</LI>
#   elif BOARD_FEATURE(FULLSPEED)
        // Basic Flash setup, then run normal routine
        FLASH->ACR |= FLASH_ACR_ACC64;
        FLASH->ACR |= FLASH_ACR_PRFTEN;
        platform_full_speed();


    ///3c. Begin clocking system with PLL driven from HSE or HSI
    ///    <LI> PLL only used if .... </LI>
    ///    <LI> Use BOARD_PARAM_PLLdiv, BOARD_PARAM_PLLmult to specify the 
    ///           particular PLL configuration.  For example, if using USB
    ///           (BOARD_PARAM_HFHz * BOARD_PARAM_PLLmult) must be 96 MHz, and
    ///           (96 MHz / BOARD_PARAM_PLLdiv) == PLATFORM_HSCLOCK_HZ. </LI>
#   elif BOARD_FEATURE(FLANKSPEED)
        // Basic Flash setup, then run normal routine
        FLASH->ACR |= FLASH_ACR_ACC64;
        FLASH->ACR |= FLASH_ACR_PRFTEN;
        platform_flank_speed();
        
#   else 
#       error "At least one of BOARD_FEATURE_STDSPEED, _FULLSPEED, or _FLANKSPEED must be ENABLED"
#   endif

    /// X. Vector Table Relocation in Internal SRAM or FLASH.
#   ifdef VECT_TAB_SRAM
#       error "Silly rabbit! SRAM is for DATA!"
        SCB->VTOR   = SRAM_BASE;  
#   else
        SCB->VTOR   = FLASH_BASE;
#   endif
}
#endif



#ifndef EXTF_platform_init_periphclk
void platform_init_periphclk() {
/// Turn-on LSE or LSI, it is used by some peripherals.  In particular,
/// OpenTag likes having a 32768Hz clock for timing purposes.  TIM9, 10, 11,
/// and the RTC are all driven by the LF clock.
   
/*    
#   define CSR_BYTE2_ADDRESS        ((uint32_t)0x40023835)
#   define RCC_LSE_OFF              ((uint8_t)0x00)
#   define RCC_LSE_ON               ((uint8_t)0x01)
#   define RCC_LSE_Bypass           ((uint8_t)0x05)
#   define RCC_LSE_RDY              ((uint8_t)0x02)
#   define RCC_LSI_OFF              ((uint8_t))
#   define RCC_LSI_ON               ((uint8_t))
#   define RCC_LSI_Bypass           ((uint8_t))
#   define RCC_LSI_RDY              ((uint8_t))
    
#define CR_OFFSET                (PWR_OFFSET + 0x00)
#define DBP_BitNumber            0x08
#define CR_DBP_BB                (PERIPH_BB_BASE + (CR_OFFSET * 32) + (DBP_BitNumber * 4))
*/
    ot_u32  pwr_cr_save = PWR->CR;
    
#   if BOARD_FEATURE(LFXTAL)
    PWR->CR     = ((1 << 11) | PWR_CR_DBP);
    RCC->CSR    = RCC_CSR_RMVF | RCC_CSR_RTCRST;
    RCC->CSR    = RCC_CSR_LSEON | RCC_CSR_RTCEN | RCC_CSR_RTCSEL_LSE;
    while ((RCC->CSR & RCC_CSR_LSERDY) == 0);
    
#   else // enable LSI
    PWR->CR     = ((1 << 11) | PWR_CR_DBP);
    RCC->CSR    = RCC_CSR_RMVF | RCC_CSR_RTCRST;
    RCC->CSR    = RCC_CSR_LSION | RCC_CSR_RTCEN | RCC_CSR_RTCSEL_LSI;
    while ((RCC->CSR & RCC_CSR_LSIRDY) == 0);
    
#   endif
   
   PWR->CR = (pwr_cr_save);
}
#endif








  
#ifndef EXTF_platform_disable_interrupts
OT_INLINE void platform_disable_interrupts() {
    __disable_irq();    // CMSIS intrinsic
}
#endif


#ifndef EXTF_platform_enable_interrupts
OT_INLINE void platform_enable_interrupts() {
    __enable_irq();     // CMSIS intrinsic
}
#endif








void sub_set_clockhz(ot_ulong cpu_clock_hz) {
/// In interest of speed and size, you need to setup your clock dividers as
/// constants in the board configuration file.
    ///@todo map these 0 shifts to derived constants
    platform_ext.clock_hz[0]    = cpu_clock_hz >> 0;    //AHB
    platform_ext.clock_hz[1]    = cpu_clock_hz >> 0;    //APB1
    platform_ext.clock_hz[2]    = cpu_clock_hz >> 0;    //APB2
}


ot_ulong platform_get_clockhz(ot_uint clock_index) {
#   if defined(__DEBUG__)
    if (clock_index > 2) {
        while(1);   //trap in debugging
    }
#   elif defined(__API__)
    if (clock_index > 2) {
        return 0;   //result for dumb APIs
    }
#   endif
    return platform_ext.clock_hz[clock_index];
}







/** Platform Speed Control <BR>
  * ========================================================================<BR>
  * These functions do not yet have a system-level call.  A function named
  * something like "sys_speedcontrol(param)" might be nice to write at some
  * point, which would manage the speed based on different active tasks that
  * request it.
  *
  * In the meantime, a task can call any of these functions, but it is only
  * recommended to call platform_full_speed and platform_flank_speed.  The
  * kernel and indeed the hardware itself manage down-speeding when going 
  * into STOP mode.
  */

#ifndef EXTF_platform_standard_speed
void platform_standard_speed() {
/// Best efficient speed.  (MSI)
/// typ config: 4.2 MHz, Power Level 2, 0 wait state.  ~1mA, 5.2 DMIPS
/// @note Going into STOP will automatically put system into Standard Speed.
#if BOARD_FEATURE(STDSPEED)
    if ((RCC->CR & RCC_CR_MSION) == 0) {
        sub_osc_startup(300, RCC_CR_MSION);
        
        // Set or clear the Flash Wait state, and set MSI as clock
#       if (_STDSPEED_FLASHWAIT == ENABLED) 
            FLASH->ACR |= FLASH_ACR_LATENCY;
            sub_osc_setclock(0);
#       else
            FLASH->ACR &= ~FLASH_ACR_LATENCY;
            sub_osc_setclock(0);
#       endif
    }

    // Turn off non-MSI clocks to save power
    RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_HSEON | RCC_CR_HSION);

    // Set new core voltage, if necessary.
#   if ((_STDSPEED_VOLTAGE != _FULLSPEED_VOLTAGE) && (_STDSPEED_VOLTAGE != _FLANKSPEED_VOLTAGE))
        sub_voltage_config(_STDSPEED_VOLTAGE | _RTC_PROTECTION);
#   endif

    // Update stored CPU speed
    sub_set_clockhz(PLATFORM_MSCLOCK_HZ);
#endif
}
#endif


#ifndef EXTF_platform_full_speed
void platform_full_speed() {
/// All Ahead Full.  (HSI or HSE, no PLL)
/// <LI> typ config: 16MHz, Power Level 2, 1 wait state. ~4mA, 16.5 DMIPS </LI>
/// <LI> Only enter fullspeed from MSI (stdspeed) </LI>
/// <LI> In system with attachable USB, check for flank-enable </LI>
#if BOARD_FEATURE(FULLSPEED)
    if (RCC->CR & RCC_CR_MSION) {
        // Change Voltage to FULLSPEED level, if different than STDSPEED
#       if ( _FULLSPEED_VOLTAGE != _STDSPEED_VOLTAGE)
        sub_voltage_config(_FULLSPEED_VOLTAGE | _RTC_PROTECTION);
#       endif
        
        sub_osc_startup(_FULLOSC_TIMEOUT, _FULLOSC_ONBIT);
        
        // Activate the Full-Speed Oscillator
#       if (_FULLSPEED_FLASHWAIT == ENABLED)
            FLASH->ACR |= FLASH_ACR_LATENCY;
            sub_osc_setclock(_FULLOSC_CLOCKBIT);
#       else
            sub_osc_setclock(_FULLOSC_CLOCKBIT);
            FLASH->ACR &= ~FLASH_ACR_LATENCY;
#       endif
        
        RCC->CR &= ~RCC_CR_MSION;
        sub_set_clockhz(PLATFORM_HSCLOCK_HZ);
    }
#endif
}
#endif


#ifndef EXTF_platform_flank_speed
void platform_flank_speed() {
/// Coming in hot!  (HSI or HSE + PLL)
/// typ config: 32MHz, Power Level 1, 1 wait state.  ~9mA, 33 DMIPS.
#if BOARD_FEATURE(FLANKSPEED)
#   if (BOARD_FEATURE(PLL) != ENABLED)
#       error "Cannot have Flank Speed without PLL"
#   endif
    
    // Enter Flank Speed if it is not already running
    if ((RCC->CR & RCC_CR_PLLON) == 0) {
        platform_disable_interrupts();
        platform_ext_pllon();
    
#       if (BOARD_FEATURE(STDSPEED) || BOARD_FEATURE(FULLSPEED))
            RCC->CFGR   = _PLL_SRC | _PLL_MULT | _PLL_DIV \
                        | (8<<4) | (_APB1_DIV+_APB1SCALE) | (_APB2_DIV+_APB2SCALE);  
#       endif
#       if (_FLANKSPEED_FLASHWAIT == ENABLED)
            FLASH->ACR |= FLASH_ACR_LATENCY;
            sub_osc_setclock(_FLANKOSC_CLOCKBIT);
#       else
            sub_osc_setclock(_FLANKOSC_CLOCKBIT);
            FLASH->ACR &= ~FLASH_ACR_LATENCY;
#       endif

        platform_enable_interrupts();
    }
    
    RCC->CR &= ~RCC_CR_MSION;
    sub_set_clockhz(PLATFORM_PLLCLOCK_HZ);
    
#else
    platform_full_speed();
    
#endif
}
#endif










/** OpenTag Kernel functions <BR>
  * ========================================================================<BR>
  */

#ifndef EXTF_platform_save_context
OT_INLINE void* platform_save_context(void) {
/// Save the current P-Stack context (thread) onto its stack.
    ot_u32 tsp;
    asm volatile (
    "   MRS    %0, psp\n"
    "   STMDB  %0!, {r4-r11}\n"
    "   MSR    psp, %0\n"
        : "=r" (tsp) 
    );
    return (void*)tsp;
}
#endif

#ifndef EXTF_platform_load_context
OT_INLINE void platform_load_context(void* tsp) {
/// Load the current P-Stack context (thread) from its stack.
    ot_u32 scratch;
    asm volatile (   
    "   MRS    %0, psp\n"
    "   LDMFD  %0!, {r4-r11}\n"
    "   MSR    psp, %0\n"        
        : "=r" (scratch) 
    );
}
#endif

#ifndef EXTF_platform_switch_context
OT_INLINE void platform_switch_context(void* tsp) {
}
#endif

#ifndef EXTF_platform_drop_context
void platform_drop_context(ot_uint i) {
/// This function should only be called from a kernel supervisor call (SVC).
/// In normal sytems it is used via sys_kill().  In applications using threads,
/// sys_kill() (or code by-way-of sys_kill()) is responsible for calling the 
/// task exit function and emptying the thread stack.

#   if (OT_PARAM_SYSTHREADS != 0)
        // Here would go some context clearing jazz
#   endif
    
    // Safety check!
    if (platform_ext.task_exit != NULL) {
        // Initial Task LR was placed on P-stack by NVIC HW when this interrupt
        // occurred.  By design it has a 6-word offset from PSP.  Overwrite it
        // with the saved address that sends code to RETURN_FROM_TASK.
        register ot_u32 task_lr;
        asm volatile ("MRS  %0, PSP" : "=r"(task_lr) );
        ((ot_u32*)task_lr)[6] = (ot_u32)platform_ext.task_exit;
    }
}
#endif


#ifndef EXTF_platform_ot_preempt
void platform_ot_preempt() {
/// Run the kernel scheduler by using system call.  If running a kernel task,
/// do not run the scheduler (we check this by inspecting the KTIM interrupt
/// enabler bit), as the ktask will run to completion and do the call in its 
/// own context (platform_ot_run()).

    if (platform_ext.task_exit == NULL) 
        __SET_PENDSV();
}
#endif


#ifndef EXTF_platform_pause
//void platform_ot_pause() {
//    platform_ot_preempt();
//    platform_flush_gptim();
//}
#endif


#ifndef EXTF_platform_ot_run
OT_INLINE void platform_ot_run() {
/// This function must be run in a while(1) loop from main.  It is the context
/// used by Kernel Tasks, which are co-operatively multitasked.

    /// 1. Run the Scheduler.  The scheduler will issue a PendSV if there is a 
    /// threaded task, in which case the P-stack will get changed to that 
    /// thread, and the code after this call will not run until all threads are 
    /// dormant.
    __SEND_SVC(0);
    
    
    /// 2. When the PC is here, it means that a kernel task has been scheduled 
    ///    or that no task is scheduled.  If no task is scheduled, then it is
    ///    time to go to sleep (or more likely STOP mode).  The powerdown 
    ///    routine MUST re-enable interrupts immediately before issuing the WFI
    ///    instruction.
    while (gptim.flags & GPTIM_FLAG_SLEEP) {
        platform_disable_interrupts();
        platform_enable_ktim();
        sys_powerdown();
    }    
    
    /// 3. Save the current P-stack pointer (PSP), and push the return address 
    ///    onto this position.  If the task is killed during its runtime, this
    ///    data will be used to reset the P-stack and PC.
    
    ///@note this code only works with GCC-based compilers.  The && operator
    /// ahead of the label is a label-reference, and it is a GCC feature.
    {   register ot_u32 return_from_task;
        platform_ext.task_exit  = (void*)__get_PSP();
        return_from_task        = (ot_u32)&&RETURN_FROM_TASK;
        asm volatile ("PUSH {%0}" : : "r"(return_from_task) );
    }

    /// 4. Run the Tasking Engine.  It will call the ktask or switch to the 
    /// thread, as needed based on what is scheduled.
    sys_run_task();
    
    /// 5. In any condition, retract the stack to a known, stable condition.
    /// If the task/thread exited cleanly, this changes nothing.  If killed, 
    /// this will flush the stack.
    RETURN_FROM_TASK:
    __set_PSP( (ot_u32)platform_ext.task_exit );
    platform_ext.task_exit = NULL;

}
#endif







/** OpenTag Resource Initializers <BR>
  * ========================================================================<BR>
  */

#ifndef EXTF_platform_init_interruptor
void platform_init_interruptor() { 
/// The NVIC setup can use any number of groups supported by Cortex M.  The 
/// strategy is to store I/O interrupts as the highest priority group.  The 
/// Kernel interrupts go in the next highest group.  Everything else is above.
/// Apps/Builds can get quite specific about how to set up the groups.

#define _KERNEL_GROUP   b0000

#   if (__CM3_NVIC_GROUPS == 1)
#       define _HIPRI_BASE  b0000
#       define _LOPRI_BASE  b0000
#       define _SUB_LIMIT   b1111
#   elif (__CM3_NVIC_GROUPS == 2)
#       define _HIPRI_BASE  b1000
#       define _LOPRI_BASE  b1000
#       define _SUB_LIMIT   b0111
#   elif (__CM3_NVIC_GROUPS == 4)
#       define _HIPRI_BASE  b0100
#       define _LOPRI_BASE  b1100
#       define _SUB_LIMIT   b0011
#   elif (__CM3_NVIC_GROUPS == 8)
#       define _HIPRI_BASE  b0010
#       define _LOPRI_BASE  b1110
#       define _SUB_LIMIT   b0001
#   endif

    /// 1. Set the EXTI channels using the board function.  Different boards
    ///    are connected differently, so this function must be implemented in 
    ///    the board support header.
    BOARD_EXTI_STARTUP();

    /// 2. Set main NVIC parameters: Vector Table @ 0x08000000 (offset = 0),
    ///    _GROUP_PRIORITY is a constant set above in this C file
    NVIC_SetPriorityGrouping(_GROUP_PRIORITY);
    
    /// 3. Setup Cortex-M system interrupts 
    ///    SysTick is not used by OpenTag, and it is a bit of a power hog, so
    ///    it is advisable to never use it.
//  SCB->SHP[((uint32_t)(MemoryManagement_IRQn)&0xF)-4] = (b0000 << 4);
//  SCB->SHP[((uint32_t)(BusFault_IRQn)&0xF)-4]         = (b0000 << 4);
//  SCB->SHP[((uint32_t)(UsageFault_IRQn)&0xF)-4]       = (b0000 << 4);
    SCB->SHP[((uint32_t)(SVC_IRQn)&0xF)-4]              = (b0000 << 4);
//  SCB->SHP[((uint32_t)(DebugMonitor_IRQn)&0xF)-4]     = (b0000 << 4);
    SCB->SHP[((uint32_t)(PendSV_IRQn)&0xF)-4]           = (b1111 << 4);
//  SCB->SHP[((uint32_t)(SysTick_IRQn)&0xF)-4]          = (_LOPRI_BASE << 4);  
    
    /// 4. Setup NVIC for Kernel Interrupts.  Kernel interrupts cannot interrupt
    /// each other, but there are subpriorities.  I/O interrupts should be set 
    /// in their individual I/O driver initializers.
    ///    <LI> NMI will interrupt anything.  It is used for panics.    </LI>
    ///    <LI> SVC is priority 0-0.  It runs the scheduler. </LI>
    ///    <LI> GPTIM (via RTC ALARM) is priority 0-1.  It runs the tasker.  </LI>
    ///    <LI> OT-Systick (via RTC WAKEUP) is priority 0-2. </LI>
    
#   ifndef __DEBUG__
#   if (RF_FEATURE(TXTIMER) != ENABLED)
#   define RTC_EXTI_MASK    ((1<<17) | (1<<20))
#   else
#   define RTC_EXTI_MASK    (1<<17)
#   endif
#   define OT_GPTIM_IRQn    RTC_Alarm_IRQn
#   define OT_SYSTICK_IRQn  RTC_WKUP_IRQn
#   define _OT_SUB1         (b0001)
#   define _OT_SUB2         ((_OT_SUB1+1)*(_SUB_LIMIT >= (_OT_SUB1+1)))  
#   define _OT_SUB3         ((_OT_SUB2+1)*(_SUB_LIMIT >= (_OT_SUB2+1)))  

    EXTI->PR                                    = RTC_EXTI_MASK;
    EXTI->IMR                                  |= RTC_EXTI_MASK;
    EXTI->RTSR                                 |= RTC_EXTI_MASK;
    NVIC->IP[(uint32_t)(OT_GPTIM_IRQn)]         = ((_KERNEL_GROUP+_OT_SUB1) << 4);
    NVIC->ISER[((uint32_t)(OT_GPTIM_IRQn)>>5)]  = (1 << ((uint32_t)(OT_GPTIM_IRQn) & 0x1F));

    // Systick is a power hog.  Avoid!
    //NVIC->IP[(uint32_t)(OT_SYSTICK_IRQn)]       = ((_KERNEL_GROUP+_OT_SUB2) << 4);
    //NVIC->ISER[((uint32_t)(OT_SYSTICK_IRQn)>>5)]= (1 << ((uint32_t)(OT_SYSTICK_IRQn) & 0x1F));
    
#   elif (RF_FEATURE(TXTIMER) != ENABLED)
    // Setup Wakeup timer used by radio driver
    EXTI->PR                                    = (1<<20);
    EXTI->IMR                                  |= (1<<20);
    EXTI->RTSR                                 |= (1<<20);
    NVIC->IP[(uint32_t)(RTC_WKUP_IRQn)]         = ((_KERNEL_GROUP+2) << 4);
    NVIC->ISER[((uint32_t)(RTC_WKUP_IRQn)>>5)]  = (1 << ((uint32_t)(RTC_WKUP_IRQn) & 0x1F));

#   endif
    
    
    /// 5. Setup other external interrupts
    /// @todo set these up in priority 1 (2nd priority).  Also, make sure board
    /// files set-up the __USE_EXTIX definitions
#   ifdef __USE_EXTI0
    NVIC->IP[(uint32_t)(EXTI0_IRQn)]        = (_HIPRI_BASE << 4);
    NVIC->ISER[((uint32_t)(EXTI0_IRQn)>>5)] = (1 << ((uint32_t)(EXTI0_IRQn) & 0x1F));
#   endif
#   ifdef __USE_EXTI1
    NVIC->IP[(uint32_t)(EXTI1_IRQn)]        = (_HIPRI_BASE << 4);
    NVIC->ISER[((uint32_t)(EXTI1_IRQn)>>5)] = (1 << ((uint32_t)(EXTI1_IRQn) & 0x1F));
#   endif
#   ifdef __USE_EXTI2
    NVIC->IP[(uint32_t)(EXTI2_IRQn)]        = (_HIPRI_BASE << 4);
    NVIC->ISER[((uint32_t)(EXTI2_IRQn)>>5)] = (1 << ((uint32_t)(EXTI2_IRQn) & 0x1F));
#   endif
#   ifdef __USE_EXTI3
    NVIC->IP[(uint32_t)(EXTI3_IRQn)]        = (_HIPRI_BASE << 4);
    NVIC->ISER[((uint32_t)(EXTI3_IRQn)>>5)] = (1 << ((uint32_t)(EXTI3_IRQn) & 0x1F));
#   endif
#   ifdef __USE_EXTI4
    NVIC->IP[(uint32_t)(EXTI4_IRQn)]        = (_HIPRI_BASE << 4);
    NVIC->ISER[((uint32_t)(EXTI4_IRQn)>>5)] = (1 << ((uint32_t)(EXTI4_IRQn) & 0x1F));
#   endif
#   if (  defined(__USE_EXTI5) || defined(__USE_EXTI6) || defined(__USE_EXTI7) \
       || defined(__USE_EXTI8) || defined(__USE_EXTI9) )
    NVIC->IP[(uint32_t)(EXTI9_5_IRQn)]        = (_HIPRI_BASE << 4);
    NVIC->ISER[((uint32_t)(EXTI9_5_IRQn)>>5)] = (1 << ((uint32_t)(EXTI9_5_IRQn) & 0x1F));
#   endif
#   if (  defined(__USE_EXTI15) || defined(__USE_EXTI14) || defined(__USE_EXTI13) \
       || defined(__USE_EXTI12) || defined(__USE_EXTI11) || defined(__USE_EXTI10) )
    NVIC->IP[(uint32_t)(EXTI15_10_IRQn)]        = (_HIPRI_BASE << 4);
    NVIC->ISER[((uint32_t)(EXTI15_10_IRQn)>>5)] = (1 << ((uint32_t)(EXTI15_10_IRQn) & 0x1F));
#   endif


    /// 6. Setup ADC interrupt.  This is needed only for ADC-enabled builds,
    ///    but ADC is used for true-random-number generation as well as actual
    ///    analog voltage sensing.
    ///@todo figure out why this #if isn't working
//#   if defined(__ISR_ADC1)
    NVIC->IP[(uint32_t)(ADC1_IRQn)]         = (_HIPRI_BASE << 4);
    NVIC->ISER[((uint32_t)(ADC1_IRQn)>>5)]  = (1 << ((uint32_t)(ADC1_IRQn) & 0x1F));
//#   endif
    
}
#endif








#ifndef EXTF_platform_init_gpio
void platform_init_gpio() { 
    BOARD_PORT_STARTUP();  
}
#endif



// platform_init_gptim() is implemented in platform_gptim_STM32L1xx.c



#ifndef EXTF_platform_init_watchdog
void platform_init_watchdog() {
/// The watchdog used here is the "Independent Watchdog."  OpenTag does not
/// officially use the watchdog anywhere -- it has a kernel to manage tasks.
/// You can use this for your needs.
}
#endif


#ifndef EXTF_platform_init_resetswitch
void platform_init_resetswitch() {
/// Currently unused
}
#endif


#ifndef EXTF_platform_init_systick
void platform_init_systick(ot_uint period) {
/// OpenTag does NOT use the ARM Cortex M SysTick.  Instead, it uses the RTC
/// Wakeup Timer feature to produce a 1 ti (1/1024sec) interval, which actually
/// is implemented as GPTIM for STM32L.  So, the setup for establishing this
/// interval is done in the gptim initialization (above).
}
#endif


#ifndef EXTF_platform_init_rtc
void platform_init_rtc(ot_u32 value) {
/// OpenTag on STM32L uses the RTC hardware for GPTIM purposes, so the 
/// functional RTC must be handled in software by a kernel-task.
    platform_set_rtc(value);
}
#endif


#ifndef EXTF_platform_init_memcpy
void platform_init_memcpy() {
#if (MCU_CONFIG(MEMCPYDMA) == ENABLED)
#endif
}
#endif








void platform_set_watchdog(ot_u16 timeout_ticks) {
/// This platform implementation is of the Independent Watchdog (IWDG).  Maybe
/// it will change to Windowed Watchdog (or maybe you can change it).  OpenTag
/// does not use any of the system watchdogs, so they are all yours. 
///
/// @note On STM32L, the IWDG can be only started once.  After that, you are a
///       slave to the watchdog, so be careful.
/*
    ot_u32  wdt_clks;
    ot_u16  wdt_pre;
    
    if (IWDG->SR == 0) {                    //Can only set IWDG if it is not running
        wdt_clks    = timeout_ticks * 9;    //1.024kHz to 9.25kHz = 9.033
        wdt_pre     = 0;                    //prescaler_0 = 4 (yields 9.25kHz)
        
        // Loop until we can derive the best-fit 12bit timer resolution
        for ( ; wdt_clks>=4096; wdt_clks>>=1, wdt_pre++); 
        IWGD->KR    = 0x5555;       //Write access enable
        IWDG->PR    = wdt_pre;
        IWDG->RLR   = wdt_clks;
        IWGD->KR    = 0x0000;       //Write access disable
        IWGD->KR    = 0xCCCC;       //Start it up
    }
    */
}

void platform_kill_watchdog() {

}

void platform_pause_watchdog() {

}

void platform_resume_watchdog() {
/// For STM32L IWDG impl, once you set the IWDG, it cannot be disabled unless
/// by reset.  You need to "resume" it before it expires.
/*
    IWGD->KR    = 0xAAAA;
*/
}




#ifndef EXTF_platform_enable_rtc
void platform_enable_rtc() {
/// OpenTag's RTC feature on STM32L is entirely managed in software by a
/// kernel task of specific design.  It is always running, so killing it
/// actually just resets the task.
#if 0 //(OT_FEATURE(RTC) == ENABLED)
    sys_kill(TASK_RTC);
#endif
}
#endif


#ifndef EXTF_platform_disable_rtc
void platform_disable_rtc() {
/// OpenTag's RTC feature on STM32L is entirely managed in software by a 
/// kernel task of specific design.  The task is always running, so it is
/// "disabled" by setting the update interval to a slow amount (256 sec).
/// This is done by killing the task!
#if 0 //(OT_FEATURE(RTC) == ENABLED)
    TASK_RTC->cursor = 255;
    sys_kill(TASK_RTC);
#endif
}
#endif


#ifndef EXTF_platform_set_time
void platform_set_time(ot_u32 utc_time) {
/// OpenTag's RTC feature on STM32L is entirely managed in software.
#if OT_FEATURE(RTC)
    // set to backup register
#endif
}
#endif


#ifndef EXTF_platform_get_time
ot_u32 platform_get_time() {
/// UTC time is always maintained
#if OT_FEATURE(RTC)
    // pull from backup register
#endif
}
#endif


#ifndef EXTF_platform_set_rtc_alarm
void platform_set_rtc_alarm(ot_u8 alarm_id, ot_u8 task_id, ot_u16 offset) {
///@todo STM32L OpenTag alarming must be done as a task.
#if 0 //(OT_FEATURE(RTC) == ENABLED)
#   if defined(__DEBUG__) || defined(__PROTO__)
    if (alarm_id < RTC_ALARMS)
#   endif
    {
        ot_u16 mask_lsb;
        ot_u16 best_limit;
        vlFILE* fp; 
                                     
        fp                          = ISF_open_su( ISF_ID(real_time_scheduler) );
        rtc.alarm[alarm_id].disabled= 0;
        rtc.alarm[alarm_id].taskid  = task_id;
        rtc.alarm[alarm_id].mask    = PLATFORM_ENDIAN16(ISF_read(fp, offset));
        rtc.alarm[alarm_id].value   = PLATFORM_ENDIAN16(ISF_read(fp, offset+2));
        vl_close(fp);
        
        // Determine largest allowable wakeup interval based on alarm criteria,
        // and set it.  (Do checks so not to exceed any other alarm's criteria)
        mask_lsb    = 1;
        best_limit  = /* RTC->WUTR */ + 1;
        while ( (mask_lsb != 0) \
            && ((mask_lsb & rtc.alarm[alarm_id].mask) == 0) \
            && ((mask_lsb & best_limit) == 0) ) {
            mask_lsb <<= 1;
        }
    }
#endif
}
#endif






/** Platform Debug Triggers <BR>
  * ========================================================================<BR>
  * Triggers are optional pins mostly used for debugging.  Sometimes they are
  * hooked up to LEDs, logic probes, etc.  The platform module requires that at
  * two triggers exist (if triggers are defined at all).  More triggers can be
  * defined in the application code.
  */
#ifdef OT_TRIG1_PORT
void platform_trig1_high()      { OT_TRIG1_ON(); }
void platform_trig1_low()       { OT_TRIG1_OFF(); }
void platform_trig1_toggle()    { OT_TRIG1_TOG(); }
#else
void platform_trig1_high()      { }
void platform_trig1_low()       { }
void platform_trig1_toggle()    { }
#endif

#ifdef OT_TRIG2_PORT
void platform_trig2_high()      { OT_TRIG2_ON(); }
void platform_trig2_low()       { OT_TRIG2_OFF(); }
void platform_trig2_toggle()    { OT_TRIG2_TOG(); }
#else
void platform_trig2_high()      { }
void platform_trig2_low()       { }
void platform_trig2_toggle()    { }
#endif






/** OpenTag OS Utility Functions <BR>
  * ========================================================================<BR>
  */


/** Platform Random Number Generation Routines <BR>
  * ========================================================================<BR>
  * The platform must be able to compute a strong random number (via function
  * platform_rand()) and a "pseudo" random number (via platform_prand_u8()).
  */
#ifndef EXTF_platform_rand
void platform_rand(ot_u8* rand_out, ot_int bytes_out) {
/// Random Number generator works the best when using "slow" channels (channels
/// other than 4,5,22,23,24) and driving them as fast as possible.
///@todo Change this implementation to use the "injected conversion" method.

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

/*
    /// Open Floating Input pin
#   define OT_GWNADC_PIN (1 << OT_GWNADC_PINNUM)
    OT_GWNADC_PORT->MODER  |= (3<<(OT_GWNADC_PIN<<1));  //Set to analog mode
    OT_GWNADC_PORT->PUPDR  &= ~(3<<(OT_GWNADC_PIN<<1)); //Set to floating input

    /// 1. Set-up ADC: The trick to this implementation is to drive the sampling
    ///    timer MUCH faster than it is supposed to run, so that it just produces
    ///    garbage data.  The garbage data turns-out to be quite random, even to
    ///    the 8 LSBs of the 12 bit capture.  If you reduce the clock frequency,
    ///    you might want to change the OT_GWNADC_BITS to a lower number.
    ADC->CR2    = 0;
    ADC->CR1    = 0;
    ADC->CCR    = 0;
    ADC->SQR1   = 0;
    ADC->SQR5   = OT_GWNADC_CHAN;
#   if ( <= 9)
    ADC->SMPR3 &= ~(7 << (OT_GWNADC_CHAN*3));
#   elif ( <= 19)
    ADC->SMPR2 &= ~(7 << ((OT_GWNADC_CHAN-10)*3));
#   elif ( <= 25)
    ADC->SMPR3 &= ~(7 << ((OT_GWNADC_CHAN-20)*3));
#   else
#   error "ADC channel is out of range, should be 0-25"
#   endif
    ADC->CR2   |= ADC_CR2_ADON;

    /// 2. Turn-on Zener noisemaker, if enabled by board
#   ifdef OT_GWNZENER_PORT
        OT_GWNZENER_PORT->BSRRL = OT_GWNZENER_PIN;
#   endif

    /// 3. Do Conversion!  Loop until the required number of bytes are produced.
    ///    The random bytes are produced by shifting-in the least-significant
    ///    sections of each sample (exactly how many bits is board-defined).
    while (--bytes_out >= 0) {
        ot_u8 reg;
#       if (OT_GWNADC_BITS == 8)
            //Special case for direct synthesis of random bytes.
            ADC->CR2 |= ADC_CR2_SWSTART;
            while (ADC->SR & ADC_SR_RCNR);
            reg = (ot_u8)ADC->DR;

#       else
            ot_u8 shifts;
            shifts = ((8+(OT_GWNADC_BITS-1)) / OT_GWNADC_BITS);
            while (shifts-- != 0) {
                ADC->CR2 |= ADC_CTL0_SC;  //start conversion
                while (ADC->SR & ADC_SR_RCNR);
                reg   <<= OT_GWNADC_BITS;
                reg    |= ((1<<OT_GWNADC_BITS)-1) & ADC->DR;
            }
#       endif
        
        *rand_out++ = reg;
    }

    ///5. Shut down ADC, turn-off Zener (if enabled), turn-off pin
    ADC->CR2                = 0;
#   ifdef OT_GWNZENER_PORT
    OT_GWNZENER_PORT->BSRRH&= OT_GWNZENER_PIN;
#   endif
    OT_GWNADC_PORT->MODER  ^= (2<<(OT_GWNADC_PIN<<1));  // go to output mode
    OT_GWNADC_PORT->BSRRH   = OT_GWNADC_PIN;            // set output=GND
*/
}
#endif


#ifndef EXTF_platform_init_prand
void platform_init_prand(ot_u16 seed) {
    platform_ext.prand_reg = seed;
}
#endif

#ifndef EXTF_platform_prand_u32
ot_u32 platform_prand_u32() {
///@todo Use CRC hardware
}
#endif


#ifndef EXTF_platform_prand_u8
ot_u8 platform_prand_u8() {
    return (ot_u8)platform_prand_u16();
}
#endif

#ifndef EXTF_platform_prand_u16
ot_u16 platform_prand_u16() {
    ot_u16 timer_value;
    timer_value = (ot_u16)RTC->TR;
    
    return platform_crc_block((ot_u8*)&timer_value, 1);
}
#endif





/** Platform memcpy Routine <BR>
  * ========================================================================<BR>
  * Similar to standard implementation of "memcpy"
  * Behavior is always blocking.  These functions either use the DMA or, if not
  * configured this way in the board support header, they use Duff's Device for
  * loop unrolling.
  */

#define DUFF_DEVICE_8(DST_OP, SRC_OP, LEN)      \
    if (LEN > 0) {                              \
        ot_int loops = (LEN + 7) >> 3;          \
        switch (LEN & 0x7) {                    \
            case 0: do {    DST_OP = SRC_OP;    \
            case 7:         DST_OP = SRC_OP;    \
            case 6:         DST_OP = SRC_OP;    \
            case 5:         DST_OP = SRC_OP;    \
            case 4:         DST_OP = SRC_OP;    \
            case 3:         DST_OP = SRC_OP;    \
            case 2:         DST_OP = SRC_OP;    \
            case 1:         DST_OP = SRC_OP;    \
                    } while (--loops > 0);      \
        }                                       \
    }

    

#if MCU_CONFIG(MEMCPYDMA)
#   define MEMCPY_DMA_INT  (1 << ((MEMCPY_DMA_CHAN_ID-1)*4))

void sub_memcpy_dma(ot_u8* dest, ot_u8* src, ot_uint length) {
/// Use 8, 16, or 32 bit chunks based on detected alignment
    static const ot_u16 ccr[4]      = { 0x4AD1, 0x40D1, 0x45D1, 0x40D1 };
    static const ot_u16 len_div[4]  = { 2, 0, 1, 0 };
    ot_int align;
    
    MEMCPY_DMACHAN->CCR     = 0;
    MEMCPY_DMA->IFCR        = MEMCPY_DMA_INT;
    MEMCPY_DMACHAN->CPAR    = (ot_u32)dest;
    MEMCPY_DMACHAN->CMAR    = (ot_u32)src;
    align                   = ((ot_u32)dest | (ot_u32)src | (ot_u32)length) & 3;
    length                >>= len_div[align];
    MEMCPY_DMACHAN->CNDTR   = length;
    MEMCPY_DMACHAN->CCR     = ccr[align];
    
    while((MEMCPY_DMA->ISR & MEMCPY_DMA_INT) == 0);
}

void sub_memcpy2_dma(ot_u16* dest, ot_u16* src, ot_uint length) {
/// Use 16 or 32 bit chunks based on detected alignment
    ot_u16 ccr_val = 0x45D1;

    // Set to 32 bit chunks if alignment allows it
    if ((((ot_u32)dest | (ot_u32)src | (ot_u32)length) & 3) == 0) {
        length >>= 1;
        ccr_val += 0x0500;
    }
    MEMCPY_DMACHAN->CCR     = 0;
    MEMCPY_DMA->IFCR        = MEMCPY_DMA_INT;
    MEMCPY_DMACHAN->CPAR    = (ot_u32)dest;
    MEMCPY_DMACHAN->CMAR    = (ot_u32)src;
    MEMCPY_DMACHAN->CNDTR   = length;
    MEMCPY_DMACHAN->CCR     = ccr_val;
    
    while((MEMCPY_DMA->ISR & MEMCPY_DMA_INT) == 0);
}

void sub_memcpy4_dma(ot_u32* dest, ot_u32* src, ot_uint length) {
/// 32 bit chunks based on detected alignment
    MEMCPY_DMACHAN->CCR     = 0;
    MEMCPY_DMA->IFCR        = MEMCPY_DMA_INT;
    MEMCPY_DMACHAN->CPAR    = (ot_u32)dest;
    MEMCPY_DMACHAN->CMAR    = (ot_u32)src;
    MEMCPY_DMACHAN->CNDTR   = length;
    MEMCPY_DMACHAN->CCR     = 0x4AD1;
    
    while((MEMCPY_DMA->ISR & MEMCPY_DMA_INT) == 0);
}

void sub_memset_dma(ot_u8* dest, ot_u8* src, ot_uint length) {
    /// Use 8, 16, or 32 bit chunks based on detected alignment
    static const ot_u16 ccr[4]      = { 0x4AD1, 0x40D1, 0x45D1, 0x40D1 };
    static const ot_u16 len_div[4]  = { 2, 0, 1, 0 };
    ot_int align;
    
    MEMCPY_DMACHAN->CCR     = 0;
    MEMCPY_DMA->IFCR        = MEMCPY_DMA_INT;
    MEMCPY_DMACHAN->CPAR    = (ot_u32)dest;
    MEMCPY_DMACHAN->CMAR    = (ot_u32)src;
    align                   = ((ot_u32)dest | (ot_u32)src | (ot_u32)length) & 3;
    length                >>= len_div[align];
    MEMCPY_DMACHAN->CNDTR   = length;
    MEMCPY_DMACHAN->CCR     = ccr[align];
    
    while((MEMCPY_DMA->ISR & MEMCPY_DMA_INT) == 0);
}

#endif  //MCU_CONFIG(MEMCPYDMA)




void platform_memcpy(ot_u8* dst, ot_u8* src, ot_uint length) {
#if MCU_CONFIG(MEMCPYDMA)
    sub_memcpy_dma(dst, src, length);
#else
    DUFF_DEVICE_8(*dst++, *src++, length);
#endif
}

void platform_memcpy_2(ot_u16* dst, ot_u16* src, ot_uint length) {
#if MCU_CONFIG(MEMCPYDMA)
    sub_memcpy2_dma( dst, src, length);
#else
    platform_memcpy((ot_u8*)dst, (ot_u8*)src, length<<1);
#endif
}

void platform_memcpy_4(ot_u32* dst, ot_u32* src, ot_uint length) {
#if MCU_CONFIG(MEMCPYDMA)
    sub_memcpy4_dma( dst, src, length);
#else
    platform_memcpy((ot_u8*)dst, (ot_u8*)src, length<<2);
#endif
}



void platform_memset(ot_u8* dst, ot_u8 value, ot_uint length) {
#if MCU_CONFIG(MEMCPYDMA)
    MEMCPY_DMACHAN->CCR     = 0;
    MEMCPY_DMA->IFCR        = MEMCPY_DMA_INT;       ///@todo see if this can be globalized
    MEMCPY_DMACHAN->CPAR    = (ot_u32)dst;
    MEMCPY_DMACHAN->CMAR    = (ot_u32)&value;
    MEMCPY_DMACHAN->CNDTR   = length;
    MEMCPY_DMACHAN->CCR     = DMA_CCR1_DIR      | DMA_CCR1_PINC     | \
                              DMA_CCR1_PL_LOW   | DMA_CCR1_MEM2MEM  | \
                              DMA_CCR1_EN;
    while((MEMCPY_DMA->ISR & MEMCPY_DMA_INT) == 0);

#else
    DUFF_DEVICE_8(*dst++, value, length);
#endif
}

void platform_memset_2(ot_u16* dst, ot_u16 value, ot_uint length) {
    platform_memset( (ot_u8*)dst, (ot_u8)value, length<<1 );
}

void platform_memset_4(ot_u32* dst, ot_u32 value, ot_uint length) {
    platform_memset( (ot_u8*)dst, (ot_u8)value, length<<2 );
}



void sub_timed_wfe(ot_u16 count, ot_u16 prescaler) {
/// Low power blocking function: uses Wait For Event (WFE) and a TIM update event.
    
    // Enable TIMx
    // load prescaler
    // load 0-count
    // trigger it
    //while ((TIMx->SR & TIM_SR_UIF) == 0) {
    //    __WFE();
    //}
    // disable TIMx
}


#ifndef EXTF_platform_block
void platform_block(ot_u16 sti) {
///@todo get WFE working on STM32L
    platform_swdelay_us( (sti<<5) );
    //sub_timed_wfe(sti, 0);
}
#endif


#ifndef EXTF_platform_delay
void platform_delay(ot_u16 n) {
    sub_timed_wfe(n, 31);
}
#endif



#ifndef EXTF_platform_swdelay_ms
void platform_swdelay_ms(ot_u16 n) {
    ot_long c;
    c   = (platform_ext.clock_hz[0]>>10);   // Set cycles per ms
    c  *= n;                                // Multiply by number of ms                   
    do { 
        c -= 7;                         // 7 cycles per loop (measured)
    } while (c > 0);
}
#endif


#ifndef EXTF_platform_swdelay_us
void platform_swdelay_us(ot_u16 n) {
    ot_long c;
    c   = (platform_ext.clock_hz[0]>>10);   // Set cycles per ms
    c  *= n;                                // Multiply by number of us
    c >>= 10;                               // Divide into cycles per us
    do { 
        c -= 7;                         // 7 cycles per loop (measured)
    } while (c > 0);
}
#endif





