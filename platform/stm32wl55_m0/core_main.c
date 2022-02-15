/* Copyright 2014 JP Norair
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
  * @file       /platform/stm32wl55_m0/core_main.c
  * @author     JP Norair
  * @version    R100
  * @date       16 Dec 2021
  * @brief      Main, core platform implementation for STM32WL M0+ Core
  * @ingroup    Platform
  *
  * STM32WL55 has one set of peripherals for both of its cores, so many parts of
  * this implementation can be copied to the M4 side, should you choose.
  *
  * Being a dual-core MCU, the STM32WL55 has a clock tree that gets shared, so
  * you must be very careful about setting-up clocking.  Unlike other OpenTag
  * ports, the STM32WL55 does not support multispeed clocking.  It's clocking
  * can be either inherited (STDSPEED) or reset (FULLSPEED).  In the reset case,
  * this will alter the system clock of the M4 as well.
  *
  * - The major exception is the NVIC initialization which is unique to M0+.
  * - The secondary exception is the 
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>

// OT modules that need initialization
#include <otlib/auth.h>
#include <otlib/buffers.h>
#include <otsys/sysclock.h>
#include <otsys/veelite.h>
#include <otsys/veelite_core.h>

#include <otsys/mpipe.h>
#include <otsys/syskern.h>
#include <otsys/time.h>

#include <m2/radio.h>
//#include <m2/session.h>


//API wrappers
void otapi_poweron()    { platform_poweron(); }
void otapi_poweroff()   { platform_poweroff(); }
void otapi_init()       { platform_init_OT(); }
void otapi_exec()       { platform_ot_run(); }
void otapi_preempt()    { platform_ot_preempt(); }
void otapi_pause()      { platform_ot_pause(); }



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
#ifndef CORE_CM0PLUS
#   if (__CM4_NVIC_GROUPS < 2)
#       define _GROUP_PRIORITY  7
#   elif (__CM4_NVIC_GROUPS < 4)
#       define _GROUP_PRIORITY  6
#   elif (__CM4_NVIC_GROUPS < 8)
#       define _GROUP_PRIORITY  5
#   elif (__CM4_NVIC_GROUPS < 16)
#       define _GROUP_PRIORITY  4
#   else
#       error "Cortex M4 may support more than 8 NVIC Groups, but STM32L supports no more than 8"
#   endif
#endif


/** Clocking Constants <BR>
  * ========================================================================<BR>
  */

// This impl permanently uses Range 1 (1.2 V)
#define POWER_1V0   0x0400  // Range 2
#define POWER_1V2   0x0200  // Range 1


// error checks
#if (BOARD_FEATURE(HFXTAL) && BOARD_FEATURE(HFBYPASS))
#   error "BOARD_FEATURE_HFXTAL and BOARD_FEATURE_HFBYPASS cannot be both ENABLED."
#endif

// If GPTIM/KTIM uses RTC as a time source, we need to keep it open,
// and unfortunately this access tends to be in the same place as clocking.
// Also, the interval-timer uses WUTE, so basically DBP is always set
#if 1 || (OT_GPTIM_ID == 'L') || (OT_GPTIM_ID == 'R')
#   define _RTC_PROTECTION  PWR_CR1_DBP
#else
#   define _RTC_PROTECTION  0
#endif

// System PLL is configured by default to 64MHz via HSI16*4
// Currently, PLL may be clocked either by HSE or HSI
// Support for other clock sources is pending
#if BOARD_FEATURE(PLL)
#   error "This platform does not implement PLL"
#else
#   define _PLL_SRC     0
#   define _PLL_MULT    0
#   define _PLL_DIV     0
#endif


#if BOARD_FEATURE(FLANKSPEED)
#   error "This platform does not support flank speed."
#endif

// Fullspeed uses MSI48
#if BOARD_FEATURE(FULLSPEED)
#   if (BOARD_FEATURE_FULLXTAL)
#       error "This platform does not implement a crystal for MCU clock"
#   endif

#   define _FULLSPEED_VOLTAGE   POWER_1V2
#   define _FULLSPEED_HZ        PLATFORM_HSCLOCK_HZ
#   undef _DEFCLK_HZ
#   undef _DEFCLK_MODE
#   define _DEFCLK_HZ           _FULLSPEED_HZ
#   define _DEFCLK_MODE         SPEED_Full

#   if (PLATFORM_HSCLOCK_HZ > 48000000)
#       error "High Speed Clock must be less than or equal to 48 MHz"
#   elif (PLATFORM_HSCLOCK_HZ > 36000000)
#       define _FULLSPEED_FLASHWAIT 2
#   elif (PLATFORM_HSCLOCK_HZ > 18000000)
#       define _FULLSPEED_FLASHWAIT 1
#   else
#       define _FULLSPEED_FLASHWAIT 0
#   endif

#   define _FULLOSC_ONBIT       RCC_CR_MSION
#   define _FULLOSC_RDYFLAG     RCC_CR_MSIRDY
#   define _FULLOSC_CLOCKBIT    0
#   define _FULLOSC_TIMEOUT     300 //MSI_STARTUP_TIMEOUT

#   define _FULLSPEED_ON()      ((RCC->CFGR & 3) == _FULLOSC_CLOCKBIT)
#   define _FULLSPEED_OFF()     ((RCC->CFGR & 3) != _FULLOSC_CLOCKBIT)
#   define _FULLRQ()            (platform_ext.rqfull != 0)
#else
#   define _FULLSPEED_ON()      0
#   define _FULLSPEED_OFF()     1
#   define _FULLRQ()            0
#endif

// Standard Speed uses sysclk set-up by the M4, divided by a certain amount.
#if BOARD_FEATURE(STDSPEED)
#   if (PLATFORM_STDCLOCK_HZ > 48000000)
#       error "High Speed Clock must be less than or equal to 48 MHz"
#   elif (PLATFORM_STDCLOCK_HZ > 36000000)
#       define _FULLSPEED_FLASHWAIT 2
#   elif (PLATFORM_STDCLOCK_HZ > 18000000)
#       define _FULLSPEED_FLASHWAIT 1
#   else
#       define _FULLSPEED_FLASHWAIT 0
#   endif

#   define _STDSPEED_VOLTAGE    POWER_1V2
#   define _STDSPEED_HZ         16000000
#   define _STDSPEED_FLASHWAIT  0
#   undef _DEFCLK_HZ
#   undef _DEFCLK_MODE
#   define _DEFCLK_HZ           _STDSPEED_HZ
#   define _DEFCLK_MODE         SPEED_Std
#   define _STDSPEED_ON()       ((RCC->CFGR & 3) == 1)
#   define _STDSPEED_OFF()      ((RCC->CFGR & 3) != 1)
#else
#   define _STDSPEED_VOLTAGE    POWER_1V2
#   define _STDSPEED_HZ         16000000
#   define _STDSPEED_FLASHWAIT  0
#   define _STDSPEED_ON()       0
#   define _STDSPEED_OFF()      1
#endif


#if MCU_CONFIG(MULTISPEED)
#   error "Multi-speed is not supported on this platform."
#elif BOARD_FEATURE(STDSPEED)
#   undef _FULLRQ
#   undef _FLANKRQ
#   define _STD_DOWNVOLT()      0
#   define _FULLRQ()            0
#   define _FLANKRQ()           0
#   define _FLANK_UPVOLT()      (_STDSPEED_VOLTAGE != _FLANKSPEED_VOLTAGE)
#elif BOARD_FEATURE(FULLSPEED)
#   undef _FULLRQ
#   undef _FLANKRQ
#   define _FULL_UPVOLT()       0
#   define _FULL_DOWNVOLT()     0
#   define _FULLRQ()            1
#   define _FLANKRQ()           0
#   define _FLANK_UPVOLT()      (_FULLSPEED_VOLTAGE != _FLANKSPEED_VOLTAGE)
#elif BOARD_FEATURE(FLANKSPEED)
#   error "Flank speed is not supported on this platform."
#endif


// Final derivation of clock dividers for each speed, and conversion to register values
#define _STD_AHBHZ           (_STDSPEED_HZ/BOARD_PARAM_AHBCLKDIV)
#define _STD_APB1HZ          (_STD_AHBHZ/BOARD_PARAM_APB1CLKDIV)
#define _STD_APB2HZ          (_STD_AHBHZ/BOARD_PARAM_APB2CLKDIV)
#define _STD_AHBDIV          BOARD_PARAM_AHBCLKDIV
#define _STD_APB1DIV         BOARD_PARAM_APB1CLKDIV
#define _STD_APB2DIV         BOARD_PARAM_APB2CLKDIV
#if ((_STD_AHBDIV!=1)&&(_STD_AHBDIV!=2)&&(_STD_AHBDIV!=4)&&(_STD_AHBDIV!=8)&&(_STD_AHBDIV!=16))
#    error "AHB in standard speed configuration is out of range (divider must be: 1,2,4,8,16)."
#endif 
#if ((_STD_APB1DIV!=1)&&(_STD_APB1DIV!=2)&&(_STD_APB1DIV!=4)&&(_STD_APB1DIV!=8)&&(_STD_APB1DIV!=16))
#    error "APB1 in standard speed configuration is out of range (divider must be: 1,2,4,8,16)."
#endif 
#if ((_STD_APB2DIV!=1)&&(_STD_APB2DIV!=2)&&(_STD_APB2DIV!=4)&&(_STD_APB2DIV!=8)&&(_STD_APB2DIV!=16))
#    error "APB2 in standard speed configuration is out of range (divider must be: 1,2,4,8,16)."
#endif
#if (_STD_AHBDIV == 1)
#    define _STD_AHBDIV_VAL (0<<4)
#    define _STD_AHBSHIFT   (0)
#elif (_STD_AHBDIV == 2)
#    define _STD_AHBDIV_VAL (8<<4)
#    define _STD_AHBSHIFT   (1)
#elif (_STD_AHBDIV == 4)
#    define _STD_AHBDIV_VAL (9<<4)
#    define _STD_AHBSHIFT   (2)
#elif (_STD_AHBDIV == 8)
#    define _STD_AHBDIV_VAL (10<<4)
#    define _STD_AHBSHIFT   (3)
#elif (_STD_AHBDIV == 16)
#    define _STD_AHBDIV_VAL (11<<4)
#    define _STD_AHBSHIFT   (4)
#endif
#if (_STD_APB1DIV == 1)
#    define _STD_APB1DIV_VAL (0<<8)
#    define _STD_APB1SHIFT   (0)
#elif (_STD_APB1DIV == 2)
#    define _STD_APB1DIV_VAL (4<<8)
#    define _STD_APB1SHIFT   (1)
#elif (_STD_APB1DIV == 4)
#    define _STD_APB1DIV_VAL (5<<8)
#    define _STD_APB1SHIFT   (2)
#elif (_STD_APB1DIV == 8)
#    define _STD_APB1DIV_VAL (6<<8)
#    define _STD_APB1SHIFT   (3)
#elif (_STD_APB1DIV == 16)
#    define _STD_APB1DIV_VAL (7<<8)
#    define _STD_APB1SHIFT   (4)
#endif
#if (_STD_APB2DIV == 1)
#    define _STD_APB2DIV_VAL (0<<11)
#    define _STD_APB2SHIFT   (0)
#elif (_STD_APB2DIV == 2)
#    define _STD_APB2DIV_VAL (4<<11)
#    define _STD_APB2SHIFT   (1)
#elif (_STD_APB2DIV == 4)
#    define _STD_APB2DIV_VAL (5<<11)
#    define _STD_APB2SHIFT   (2)
#elif (_STD_APB2DIV == 8)
#    define _STD_APB2DIV_VAL (6<<11)
#    define _STD_APB2SHIFT   (3)
#elif (_STD_APB2DIV == 16)
#    define _STD_APB2DIV_VAL (7<<11)
#    define _STD_APB2SHIFT   (4)
#endif

#if BOARD_FEATURE(FULLSPEED)
#   define _FULL_AHBHZ          (_FULLSPEED_HZ/BOARD_PARAM_AHBCLKDIV)
#   define _FULL_AHBDIV         (BOARD_PARAM_AHBCLKDIV)
#   define _FULL_APB1DIV        (BOARD_PARAM_APB1CLKDIV*((_FULLSPEED_HZ+_DEFCLK_HZ-1)/_DEFCLK_HZ))
#   define _FULL_APB2DIV        (BOARD_PARAM_APB2CLKDIV*((_FULLSPEED_HZ+_DEFCLK_HZ-1)/_DEFCLK_HZ))
#   if ((_FULL_AHBDIV!=1)&&(_FULL_AHBDIV!=2)&&(_FULL_AHBDIV!=4)&&(_FULL_AHBDIV!=8)&&(_FULL_AHBDIV!=16))
#       error "AHB in Full speed configuration is out of range (divider must be: 1,2,4,8,16)."
#   endif
#   if ((_FULL_APB1DIV!=1)&&(_FULL_APB1DIV!=2)&&(_FULL_APB1DIV!=4)&&(_FULL_APB1DIV!=8)&&(_FULL_APB1DIV!=16))
#       error "APB1 in Full speed configuration is out of range (divider must be: 1,2,4,8,16)."
#   endif
#   if ((_FULL_APB2DIV!=1)&&(_FULL_APB2DIV!=2)&&(_FULL_APB2DIV!=4)&&(_FULL_APB2DIV!=8)&&(_FULL_APB2DIV!=16))
#       error "APB2 in Full speed configuration is out of range (divider must be: 1,2,4,8,16)."
#   endif
#   if (_FULL_AHBDIV == 1)
#       define _FULL_AHBDIV_VAL (0<<4)
#       define _FULL_AHBSHIFT   (0)
#   elif (_FULL_AHBDIV == 2)
#       define _FULL_AHBDIV_VAL (8<<4)
#       define _FULL_AHBSHIFT   (1)
#   elif (_FULL_AHBDIV == 4)
#       define _FULL_AHBDIV_VAL (9<<4)
#       define _FULL_AHBSHIFT   (2)
#   elif (_FULL_AHBDIV == 8)
#       define _FULL_AHBDIV_VAL (10<<4)
#       define _FULL_AHBSHIFT   (3)
#   elif (_FULL_AHBDIV == 16)
#       define _FULL_AHBDIV_VAL (11<<4)
#       define _FULL_AHBSHIFT   (4)
#   endif
#   if (_FULL_APB1DIV == 1)
#       define _FULL_APB1DIV_VAL (0<<8)
#       define _FULL_APB1SHIFT   (0)
#   elif (_FULL_APB1DIV == 2)
#       define _FULL_APB1DIV_VAL (4<<8)
#       define _FULL_APB1SHIFT   (1)
#   elif (_FULL_APB1DIV == 4)
#       define _FULL_APB1DIV_VAL (5<<8)
#       define _FULL_APB1SHIFT   (2)
#   elif (_FULL_APB1DIV == 8)
#       define _FULL_APB1DIV_VAL (6<<8)
#       define _FULL_APB1SHIFT   (3)
#   elif (_FULL_APB1DIV == 16)
#       define _FULL_APB1DIV_VAL (7<<8)
#       define _FULL_APB1SHIFT   (4)
#   endif
#   if (_FULL_APB2DIV == 1)
#       define _FULL_APB2DIV_VAL (0<<11)
#       define _FULL_APB2SHIFT   (0)
#   elif (_FULL_APB2DIV == 2)
#       define _FULL_APB2DIV_VAL (4<<11)
#       define _FULL_APB2SHIFT   (1)
#   elif (_FULL_APB2DIV == 4)
#       define _FULL_APB2DIV_VAL (5<<11)
#       define _FULL_APB2SHIFT   (2)
#   elif (_FULL_APB2DIV == 8)
#       define _FULL_APB2DIV_VAL (6<<11)
#       define _FULL_APB2SHIFT   (3)
#   elif (_FULL_APB2DIV == 16)
#       define _FULL_APB2DIV_VAL (7<<11)
#       define _FULL_APB2SHIFT   (4)
#   endif
#else
#   define _FULL_AHBDIV_VAL     _STD_AHBDIV_VAL
#   define _FULL_APB2DIV_VAL    _STD_APB2DIV_VAL
#   define _FULL_APB1DIV_VAL    _STD_APB1DIV_VAL
#   define _FULLOSC_CLOCKBIT    1
#   define _FULL_AHBSHIFT       _STD_AHBSHIFT
#   define _FULL_APB1SHIFT      _STD_APB1SHIFT
#   define _FULL_APB2SHIFT      _STD_APB2SHIFT
#endif




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
/// Input must be: POWER_1V0, POWER_1V2
    ot_u16 scratch;
    
    scratch     = PWR->CR1 & ~(ot_u32)(PWR_CR1_VOS_Msk);
    scratch    |= pwr_cr_vos_x;
    PWR->CR1    = scratch;

    // Wait Until the Voltage Regulator is ready
    while((PWR->SR2 & PWR_SR2_VOSF) != 0) { }
}


void sub_osc_startup(ot_u16 counter) {
///@todo figure out a way to do this with WFE
///@note HSIASFS bit is not set here, because the clock system
///      only uses one clock at a time.
    //ot_u32 hsi_asfs_bit;
    ot_u16  wdog = counter;
    ot_u32  rcc_reg;
    
    // Assure MSI is ON and Ready
    if ((RCC->CR & RCC_CR_MSIRDY) == 0) {
        RCC->CR |= RCC_CR_MSION;
        while ( ((RCC->CR & RCC_CR_MSIRDY) == 0) && (--wdog) );
    }
    
    // Test if oscillator failed to startup.
    // Else, configure it to 48 MHz and as the system clock
    if (wdog == 0) {
        ///@todo Death message / Death Blinkly
        return;
    }
    
    // Assure MSI is 48 MHz
    rcc_reg = RCC->CR;
    if ((rcc_reg & 0x000000F8) != 0b10111000) {
        rcc_reg = (rcc_reg & 0x000000F8) | 0b10111000;
        RCC->CR = rcc_reg;
    }

    // Assure MSI is System Clock
    rcc_reg = RCC->CFGR;
    if ((rcc_reg & 0x0000000F) != 0b0000) {
        rcc_reg &= ~0x00000003;
        RCC->CFGR = rcc_reg;
        wdog = counter;
        while ( ((RCC->CFGR & 0x0000000C) != 0) && (--wdog) );
    }

    // Test if oscillator failed to startup.
    // Else, configure it to 48 MHz and as the system clock
    if (wdog == 0) {
        ///@todo Death message / Death Blinkly
        return;
    }
}



typedef struct {
    ot_u32 cfgr_val;
    ot_u32 ahb_hz;
    ot_u32 apb1_hz;
    ot_u32 apb2_hz;
} setclocks_t;

void sub_setclocks(SPEED_enum mode) {
///@todo this could set an MSI divider, but there's really no point for that.
}




/** Extended Platform (STM32L4-specific) power and PLL control<BR>
  * ========================================================================<BR>
  */
#ifndef EXTF_platform_ext_wakefromstop
void platform_ext_wakefromstop() {

    // Start the clocker right on wakeup.  The clock may get skewed a bit, 
    // but this is acceptable.
    systim_start_clocker();

    ///@todo this is where clock modification would go, but we're using MSI
    /// the whole time, so don't need to do anything in this platform.
}
#endif




#ifndef EXTF_platform_ext_pllon
void platform_ext_pllon() {
#if BOARD_FEATURE(PLL)
    ///@todo this is specifically to use the PLL that drives USB or other peripherals,
    /// which for this platform may not be necessary
#endif
}
#endif


#ifndef EXTF_platform_ext_plloff
void platform_ext_plloff() {
#if BOARD_FEATURE(PLL)
    ///@todo this is specifically to use the PLL that drives USB or other peripherals,
    /// which for this platform may not be necessary
#endif
}
#endif


static int sub_xsi_trim(uint32_t target_hz) {
/// Measure the HSI/MSI against the LSE and set trim value
///@note TIM16 is uniquely suited for this purpose.
    ot_int rc;
    ot_int edge     = 0;
    ot_u16 xsi_dev  = 0;
    ot_u16 target   = target_hz / BOARD_PARAM_LFHz;

    // Enable TIM16 with:
    // - LSE as measurement input capture 1
    // - No prescaler
    // - Immediate Reload
    RCC->APB2ENR   |= RCC_APB2ENR_TIM16EN;
	RCC->APB2RSTR  |= RCC_APB2RSTR_TIM16RST;
	RCC->APB2RSTR  &= ~RCC_APB2RSTR_TIM16RST;

	TIM16->ARR      = 65535;
    TIM16->CCMR1    = 1;                    // Set Input to line-1
    TIM16->CCER     = TIM_CCER_CC1E;        // Enable Input Capture
    TIM16->OR1      = b10;                  // LSE
    TIM16->CR1      = TIM_CR1_CEN;

    // Wait for rising edge 1, then count SYSCLK pulses until rising edge 2
    ///@todo WFE?
    while (edge == 0) {
        ot_u16 flag;
        do { flag = TIM16->SR; } while (flag == 0); //wait for edge

        if ((flag & TIM_SR_CC1IF) == 0) {           // something is wrong, revert to last
            rc = 0;
            break;
        }
        if (edge == 0) {                         // First Edge, must wait for 2nd
            xsi_dev = TIM16->CCR1;
        }
        else {
            xsi_dev = TIM16->CCR1 - xsi_dev;
            rc      = (int)(target - xsi_dev);
        }
    }
    
    // Turn-off TIM16
    TIM16->CR1      = 0;
    RCC->APB2ENR   &= ~RCC_APB2ENR_TIM16EN;
    return rc;
    
    
    
    // Else, set HW for next HSITRIM value
    

    

#   undef _TARGET
}


#ifndef EXTF_platform_ext_hsitrim
void platform_ext_hsitrim() {
    uint32_t rcc_icsr;
    int trim = sub_xsi_trim(16000000);  ///@todo replace 16 MHz with MACRO
    
    
    /// If trimming is already perfect, do nothing.
    /// HSITRIM center value is 64.
    /// Make sure trim value is between 0-127.
    if (trim != 0) {
        trim += 64;
        
        ///@todo this could be replaced with SAT intrinsics.
        if (trim < 0) {
            trim = 0;
        }
        else if (trim > 127) {
            trim = 127;
        }
        
        rcc_icsr    = RCC->ICSCR;
        rcc_icsr   &= ~(127 << 24);
        rcc_icsr   |= (trim << 24);
        RCC->ICSCR  = rcc_icsr;
    }
}
#endif


#ifndef EXTF_platform_ext_msitrim
void platform_ext_msitrim() {
///@todo unclear from manual if the MSITRIM value is 8bit signed twos-compliment or
///      a 7bit value with a signing bit in the MSB.
    uint32_t    rcc_icsr;
    int8_t      msitrim;
    int         trim = sub_xsi_trim(48000000);  ///@todo replace 48 MHz with MACRO
    
    /// If trimming is already perfect, do nothing.
    /// MSITRIM center value is 0.
    /// Make sure trim value is between -128 to 127.
    if (trim != 0) {
        if (trim < -128) {
            msitrim = -128;
        }
        else if (trim > 127) {
            msitrim = 127;
        }
        else {
            msitrim = (int8_t)trim;
        }
        
        rcc_icsr    = RCC->ICSCR;
        rcc_icsr   &= ~(255 << 8);
        rcc_icsr   |= ((uint32_t)msitrim << 8);
        RCC->ICSCR  = rcc_icsr;
    }
}
#endif


#ifndef EXTF_platform_ext_lsihz
ot_u16 platform_ext_lsihz() {
/// LSITRIM not available on this platform.
    return 37000;
}
#endif




/** Clock Hz retrieval function
  * ========================================================================<BR>
  */
ot_ulong platform_get_clockhz(ot_uint clock_index) {
#   if defined(__DEBUG__)
    if (clock_index > 2) {
        while(1);   //trap in debugging
    }
#   endif
    return (clock_index > 2) ? 0 : platform_ext.clock_hz[clock_index];
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
  
ot_int sysclock_request(SPEED_enum Speed) {
    return -1;
}


ot_int sysclock_dismiss(ot_int handle) {
    return -1;
}


#ifndef EXTF_platform_standard_speed
void platform_standard_speed() {
/// Best efficient speed. (MSI)
/// typ: 16 MHz, 1.2V, 0 wait state

    ///@todo assure 48 MHz MSI is divided less than 18 MHz
}
#endif




#ifndef EXTF_platform_full_speed
void platform_full_speed() {
/// All Ahead Full.  MSI
/// typ config: 48MHz MSI, 1.2V, 0 wait states
/// 
/// Only go through the process of entering Full Speed if it is not activated
/// already.  In multispeed systems, don't call this function directly unless you know
/// exactly what you're doing.  Use sysclock_request() instead.

    ///@todo assure 48 MHz MSI with divider = 1
}
#endif


#ifndef EXTF_platform_flank_speed
void platform_flank_speed() {
/// Same as FullSpeed on this platform
    platform_full_speed();
}
#endif








/** Platform Interrupts <BR>
  * ========================================================================<BR>
  */

#ifndef EXTF_platform_disable_interrupts
OT_INLINE void platform_disable_interrupts(void) {
    __disable_irq();    // CMSIS intrinsic
}
#endif


#ifndef EXTF_platform_enable_interrupts
OT_INLINE void platform_enable_interrupts(void) {
    __enable_irq();     // CMSIS intrinsic
}
#endif







/** Platform Initialization & Control <BR>
  * ========================================================================<BR>
  */
#ifndef EXTF_platform_poweron
void platform_poweron() {

    /// 1. Set cooperative task runtime in P-stack,
    ///    Interrupts and protected system calls run in M-stack
    __set_PSP( __get_MSP() );
    __set_CONTROL(2);
    __set_MSP( (ot_u32)&platform_ext.sstack[(OT_PARAM_SSTACK_ALLOC/4)-1] );

    /// 2. Platform-Specific power-up configuration
    ///    - Can be overriden by Board-Specific power-up, below.
    ///    - Debug Configuration must be done via CPU1!
    ///

    RCC->C2AHB1ENR      = RCC_C2AHB1ENR_CRCEN
                        | RCC_C2AHB1ENR_DMAMUX1EN
                        | RCC_C2AHB1ENR_DMA2EN;

    RCC->C2AHB1SMENR    = RCC_C2AHB1SMENR_CRCSMEN
                        | 0x07;

#   if 0 //defined(__DEBUG__)
    DBGMCU->CR          = DBGMCU_CR_DBG_SLEEP
                        | DBGMCU_CR_DBG_STOP
                        | DBGMCU_CR_DBG_STANDBY;

    DBGMCU->C2APB1FZR1  = DBGMCU_C2APB1FZR1_DBG_RTC_STOP;

//    DBGMCU->C2APB1FZR1  = DBGMCU_C2APB1FZR1_DBG_LPTIM1_STOP
//                        | DBGMCU_C2APB1FZR1_DBG_I2C3_STOP
//                        | DBGMCU_C2APB1FZR1_DBG_I2C2_STOP
//                        | DBGMCU_C2APB1FZR1_DBG_I2C1_STOP
//                        | DBGMCU_C2APB1FZR1_DBG_IWDG_STOP
//                        | DBGMCU_C2APB1FZR1_DBG_RTC_STOP
//                        | DBGMCU_C2APB1FZR1_DBG_TIM2_STOP;
//
//    DBGMCU->C2APB1FZR2  = DBGMCU_C2APB1FZR1_DBG_LPTIM2_STOP
//                        | DBGMCU_C2APB1FZR1_DBG_LPTIM3_STOP;
//
//    DBGMCU->C2APB2FZR   = DBGMCU_C2APB1FZR1_DBG_TIM17_STOP
//                        | DBGMCU_C2APB1FZR1_DBG_TIM16_STOP
//                        | DBGMCU_C2APB1FZR1_DBG_TIM1_STOP;
#   endif

    // STOP2 Mode as Deep Sleep
    PWR->C2CR1 = 4;

    // C2CR3 should be set-up in the BOARD_POWER_STARTUP()
    //PWR->C2CR3 = 0;

#   if MCU_PARAM(CPU2PWR)
    sub_voltage_config(POWER_1V2);

    // 0 = reset default for CR2.
    // Set up PWR->CR2 in BOARD_POWER_STARTUP() if you want to use it.
    //PWR->CR2    = 0;

    // 0x8000 = reset default for CR3.
    // Set up PWR->CR3 in BOARD_POWER_STARTUP() if you want to use it.
    //PWR->CR3    = 0x8000;

#   endif

    /// 3. Board-Specific power-up configuration
    BOARD_POWER_STARTUP();
    BOARD_PERIPH_INIT();
    BOARD_PORT_STARTUP();

    /// 4. Configure Clocks
    platform_init_periphclk();
    platform_init_busclk();
#   if (BOARD_FEATURE(HFXTAL) != ENABLED)
    //platform_ext_hsitrim();
    platform_ext_msitrim();
#   endif

    /// 5. Final initialization of OpenTag system resources
    ///    - NVIC configuration & kernel interrupts
    ///    - Initialize systim, which provides the kernel timer.
    platform_init_interruptor();
    systim_init(NULL);

    /// 6. Restore the vworm-backed files (following save on shutdown)
    vworm_init();

    /// 7. This prevents the scheduler from getting called by a preemption
    ///    event until it officially begins.  It allows some tasks to be
    ///    used for special purposes at power-on (namely MPipe).
    platform_ext.task_exit = (void*)__get_PSP();
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

    /// 2. Initialize the System Time.  If the build does not support time,
    ///    then this will do nothing.
    time_set_utc(364489200);

    /// 3. Look for errors, report them, and make sure to clear the error RAM.
    /// @note This is a decent place to put a breakpoint when debugging.
    if (TAMP->BKP2R != 0) {
        vlFILE*     fp;
        ot_uni16    resets;
        ot_uni16    faults;
        ot_uni32    error;

        error.ulong     = TAMP->BKP2R;
        TAMP->BKP2R     = 0;
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

    /// 4. If debugging, copy the UID64 that ST writes into the ROM into
    ///    the Mode 2 UID (Device Settings ISF)
    ///
    /// @note the ID is inserted via Veelite, so it is abstracted from the
    /// file memory configuration of your board and/or app.
    ///
    /// @note for production (__RELEASE__) the default UID should be written to
    ///      the default file location by the manufacturer firmware upload.
#   if (defined(__DEBUG__) || defined(__PROTO__))
    {   vlFILE* fpid;
        union {
            ot_u32 word[2];
            ot_u16 halfw[4];
        } uid;
        ot_u16* hwid;
        ot_int  i;

        // 96 bits with ASCII
        uid.word[1] = *((ot_u32*)(0x1FFF7584));
        uid.word[0] = *((ot_u32*)(0x1FFF7580));
        
        ///@todo compression of the ID.  There are many gaps in the ID
        fpid    = ISF_open_su(ISF_ID(device_features));
        hwid    = &(uid.halfw[0]);
        for (i=0; i<8; i+=2) {
            vl_write(fpid, i, *hwid++);
        }
        vl_close(fpid);
    }
#   else

#   endif
    
    /// 5. Initialize the System (Kernel & more).  The System initializer must
    ///    initialize all modules that are built onto the kernel.  These include
    ///    the DLL and MPipe.
    sys_init();
}
#endif



#ifndef EXTF_platform_init_busclk
void platform_init_busclk() {
/// This function should be called during initialization and restart, right at
/// the top of platform_poweron().

    // The system clock must be MSI @ 48MHz
    sub_osc_startup(300);
    sub_setclocks(SPEED_Std);
    
    
    FLASH->ACR = (FLASH_ACR_DCEN | FLASH_ACR_ICEN);
    
    /// The code below will disable all the clocks that are not used.
    /// It is commented-out because CPU1 is expected to control the clocking system.
    //RCC->CR    &= ~(RCC_CR_HSEON|RCC_CR_HSEBYP|RCC_CR_CSSON|RCC_CR_PLLON|RCC_CR_MSION);
    //RCC->CIER   = 0;

    ///3a. If Standard Speed is enabled, it is already underway (HSI16), so do nothing
#   if BOARD_FEATURE(STDSPEED)

    ///3b. Use HSE or HSI16 without PLL as Full-Speed clock
    ///    <LI> If HSI16, that's already active, so do nothing. </LI>
    ///    <LI> If HSE, switch to it.</LI>
#   elif BOARD_FEATURE(FULLSPEED) && (_FULLOSC_CLOCKBIT != 1)
        platform_full_speed();
    
    {   ot_u32 rcc_ccipr;
        rcc_ccipr   = RCC->CCIPR & 0x0FFFFFFF;
        RCC->CCIPR  = rcc_ccipr
                    | (b11 << RCC_CCIPR_RNGSEL_Pos)             /* MSI for RNG */
                    | (b11 << RCC_CCIPR_ADCSEL_Pos);            /* MSI for ADC */
    }
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


///@todo have a setting that looks at the backup domain to make sure the clocking
///      for it is not already what it should be.
#ifndef EXTF_platform_init_periphclk
void platform_init_periphclk() {
/// Turn-on LSE or LSI, it is used by some peripherals.  In particular,
/// OpenTag likes having a 32768Hz clock for timing purposes.
/// LPTIM1, 2, and the RTC are all driven by the LF clock.

    
    
    PWR->CR1 |= PWR_CR1_DBP;
    //RCC->BDCR |= RCC_BDCR_RTCRST;
    
#   if BOARD_FEATURE(LFXTAL)
        RCC->CSR    = RCC_CSR_RMVF;
        RCC->BDCR  = (1<<RCC_BDCR_LSCOSEL_Pos) \
                    | (b01<<RCC_BDCR_RTCSEL_Pos) \
                    | RCC_BDCR_LSECSSD \
                    | (b00<<RCC_BDCR_LSEDRV_Pos) \
                    | RCC_BDCR_LSEON;
                
        while ((RCC->BDCR & RCC_BDCR_LSERDY) == 0);
    
        RCC->BDCR |= RCC_BDCR_LSCOEN | RCC_BDCR_RTCEN | RCC_BDCR_LSECSSON;   

#   else // enable LSI
        PWR->CR     = ((1<<9) | PWR_CR_DBP);
        //RCC->BDCR |= RCC_BDCR_RTCRST;
        RCC->CSR    = RCC_CSR_RMVF | RCC_CSR_LSION;
        while ((RCC->CSR & RCC_CSR_LSIRDY) == 0);
    
        RCC->BDCR  = (0<<RCC_BDCR_LSCOSEL_Pos) | RCC_BDCR_LSCOEN \
                    | (b10<<RCC_BDCR_RTCSEL_Pos) | RCC_BDCR_RTCEN;
#   endif

}
#endif










/** OpenTag Resource Initializers <BR>
  * ========================================================================<BR>
  */

///@todo requires additional porting to STM32WL EXTI system.
#ifndef EXTF_platform_init_interruptor
void platform_init_interruptor() {
/// The NVIC setup can use any number of groups supported by Cortex M.  The
/// strategy is to store I/O interrupts as the highest priority group.  The
/// Kernel interrupts go in the next highest group.  Everything else is above.
/// Apps/Builds can get quite specific about how to set up the groups.

#   define _SVC_GROUP       b00
#   define _KERNEL_GROUP    b01
#   define _HIPRI_GROUP     b10
#   define _LOPRI_GROUP     b11
#   define _SUB_LIMIT       b11

    /// 1. Set the EXTI channels using the board function.  Different boards
    ///    are connected differently, so this function must be implemented in
    ///    the board support header.
    BOARD_EXTI_STARTUP();

    /// 2. Cortex M0 doesn't have NVIC priority grouping, so nothing to do here
    //NVIC_SetPriorityGrouping(_GROUP_PRIORITY);

    /// 3. Setup Cortex-M system interrupts
    /// <LI> Fault IRQs (Mem management, bus-fault, usage-fault) can be enabled
    ///         if you want more clarity of the fault than just Hard-Fault </LI>
    /// <LI> SVC IRQ is for supervisor-call.  The kernel needs it. </LI>
    /// <LI> Pend SV is for supervisor-call-pending.  The kernel needs it. </LI>
    /// <LI> Debug-Monitor is not used </LI>
    /// <LI> Systick is not used and it is inadvisable to use because it is a
    ///         power hog and because it is mostly useless with OpenTag. </LI>
//  SCB->SHP[_SHP_IDX(MemoryManagement_IRQn)  = (b00 << 4);
//  SCB->SHP[_SHP_IDX(BusFault_IRQn)]         = (b00 << 4);
//  SCB->SHP[_SHP_IDX(UsageFault_IRQn)]       = (b00 << 4);
//  SCB->SHP[_SHP_IDX(SVC_IRQn)]            = (b00 << 4);
//  SCB->SHP[_SHP_IDX(PendSV_IRQn)]         = (b11 << 4);
//  SCB->SHP[_SHP_IDX(DebugMonitor_IRQn)]     = (b00 << 4);
    
    NVIC_SetPriority(SVC_IRQn, _SVC_GROUP);
    NVIC_SetPriority(PendSV_IRQn, _LOPRI_GROUP);
    
    // Systick needs SCB and NVIC to be enabled in order to run.
#   if defined(SYSTICK_IS_HIGHLY_DISCOURAGED)
    NVIC_SetPriority(SysTick_IRQn, _LOPRI_GROUP);
    NVIC_EnableIRQ(SysTick_IRQn);
#   endif

    /// 4. Setup NVIC for Kernel Interrupts.  Kernel interrupts cannot interrupt
    /// each other, but there are subpriorities.  I/O interrupts should be set
    /// in their individual I/O driver initializers.
    ///    <LI> NMI will interrupt anything.  It is used for panics.    </LI>
    ///    <LI> SVC is priority 0.  It runs the scheduler. </LI>
    ///    <LI> LPTIM is priority 1.  It runs the tasker.  </LI>
    ///    <LI> If Mode 2 is enabled, RTC-Wakeup is the MAC-insertion timer and
    ///         is priority 0-1.  If not, RTC-Wakeup is low-priority and it is
    ///         only used for the interval timer (watchdog/systick) </LI>

    // From Reference Manual RM0453 Rev 2, page 507:
    // Line 16: PVD
    // Line 17: RTC Alaram
    // Line 18: RTC SSRU
    // Line 19: RTC-Tamper/Timestamp/LSE_CSS
    // Line 20: RTC-Wakeup
    // Line 21: COMP1-out
    // Line 22: COMP2-out
    // Line 23: I2C1
    // Line 24: I2C2
    // Line 25: I2C3
    // Line 26: USART1
    // Line 27: USART2
    // Line 29: LPTIM1
    // Line 30: LPTIM2
    // Line 31: LPTIM3
    // ...
    // Line 37: IPCC-CPU2
    // Line 39: HSEM1 (for CPU2)
    // Line 41: CPU1-SEV (for CPU2)
    // Line 44: Radio IRQs
    // Line 45: Radio Busy

    EXTI->PR1       = (1<<20);
    EXTI->IMR1     |= (1<<20);
    EXTI->RTSR1    |= (1<<20);

    NVIC_SetPriority(RTC_LSECSS_IRQn, _KERNEL_GROUP);
    NVIC_EnableIRQ(RTC_LSECSS_IRQn);

    /// 5. Setup other external interrupts
    /// @note Make sure board files use the __USE_EXTI(N) definitions
#   if defined(__USE_EXTI0) || defined(__USE_EXTI1)
    NVIC_SetPriority(EXTI1_0_IRQn, _LOPRI_GROUP);
    NVIC_EnableIRQ(EXTI1_0_IRQn);
#   endif
#   if defined(__USE_EXTI2) || defined(__USE_EXTI3)
    NVIC_SetPriority(EXTI3_2_IRQn, _LOPRI_GROUP);
    NVIC_EnableIRQ(EXTI3_2_IRQn);
#   endif
#   if( defined(__USE_EXTI4)  || defined(__USE_EXTI5)  || defined(__USE_EXTI6) \
    ||  defined(__USE_EXTI7)  || defined(__USE_EXTI8)  || defined(__USE_EXTI9) \
    ||  defined(__USE_EXTI10) || defined(__USE_EXTI11) || defined(__USE_EXTI12) \
    ||  defined(__USE_EXTI13) || defined(__USE_EXTI14) || defined(__USE_EXTI15) )
    NVIC_SetPriority(EXTI15_4_IRQn, _LOPRI_GROUP);
    NVIC_EnableIRQ(EXTI15_4_IRQn);
#   endif


    /// 6. Setup ADC interrupt.  This is needed only for ADC-enabled builds,
    ///    but ADC is frequently used, so it is enabled by default
#   if defined(__USE_ADC1)
    NVIC_SetPriority(ADC1_COMP_IRQn, _HIPRI_GROUP);
    NVIC_EnableIRQ(ADC1_COMP_IRQn);
#   endif
}
#endif





#ifndef EXTF_platform_init_gpio
void platform_init_gpio() {
    BOARD_PORT_STARTUP();
}
#endif



// systim_init() is implemented in platform_systim_STM32L1xx.c



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



#ifndef EXTF_platform_init_memcpy
void platform_init_memcpy() {
#if (MCU_CONFIG(MEMCPYDMA) == ENABLED)
#endif
}
#endif



