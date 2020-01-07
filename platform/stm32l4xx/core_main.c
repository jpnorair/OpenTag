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
  * @file       /platform/stm32l4xx/core_main.c
  * @author     JP Norair
  * @version    R100
  * @date       31 Dec 2019
  * @brief      Main, core platform implementation for STM32L4
  * @ingroup    Platform
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




///@note for some reason, the Constant "RCC_CR_HSION" from the device header
/// is possessed by Satan.  Compiler isn't happy with it.
#define _RCC_CR_HSI     RCC_CR_HSION



/** Context Caching Parameters <BR>
  * =======================================================================<BR>
  * STM32L and all other Cortex M devices have a hardware mechanism to assist
  * the caching of multiple contexts and levels of interrupt preemption.
  *
  * GULP kernels should use __CM4_NVIC_GROUPS == 1, which will create a single,
  * global interrupt context.  Tasks must manage their own contexts.
  *
  * HICCULP kernels can use __CM4_NVIC_GROUPS > 1, but each extra group (each
  * extra context) will require the allocation of extra stack.
  *
  * Usually, these configuration parameters are set in the compiler or in the
  * build_config.h file.  If they are not set, defaults are used.
  */



/** Clocking Constants <BR>
  * ========================================================================<BR>
  */

// This impl permanently uses Range 1 (1.2 V)
#define POWER_1V0   0x0400  // Range 2
#define POWER_1V2   0x0200  // Range 1


// The Board header should define these.  
// If not, these are the defaults.
#ifndef BOARD_FEATURE_HFXTAL
#   define BOARD_FEATURE_HFXTAL     DISABLED
#endif
#ifndef BOARD_FEATURE_HFBYPASS
#   define BOARD_FEATURE_HFBYPASS   DISABLED
#endif
#ifndef MCU_CONFIG_USB
#   define MCU_CONFIG_USB           DISABLED
#endif
#ifndef BOARD_FEATURE_HFCRS
#   define BOARD_FEATURE_HFCRS      DISABLED
#endif
#ifndef BOARD_FEATURE_USBPLL
#   define BOARD_FEATURE_USBPLL     DISABLED
#endif
#ifndef PLATFORM_PLLCLOCK_OUT
#   define PLATFORM_PLLCLOCK_OUT    128000000
#endif


// error checks
#if (BOARD_FEATURE(HFXTAL) && BOARD_FEATURE(HFBYPASS))
#   error "BOARD_FEATURE_HFXTAL and BOARD_FEATURE_HFBYPASS cannot be both ENABLED."
#endif
#if (MCU_CONFIG(USB) && (BOARD_FEATURE(USBPLL) != ENABLED) && (BOARD_FEATURE(HFCRS) != ENABLED))
#   error "To use built-in USB, you must ENABLE the PLL or CRS"
#endif
#if (MCU_CONFIG(USB) && BOARD_FEATURE(USBPLL) && (PLATFORM_PLLCLOCK_OUT != 96000000))
#   error "STM32L0 requires PLL output to be 96 MHz when using it for internal USB."
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
#   define _USE_PLL
#   define _PLL_SRC     (2 + ((BOARD_FEATURE(FLANKXTAL) == ENABLED))

#   if (BOARD_PARAM(PLLmult) >= 8) && (BOARD_PARAM(PLLmult) <= 86)
#       define _PLL_MULT    (BOARD_PARAM(PLLmult)<<8)
#   else
#       error "PLL Multiplier from BOARD_PARAM_PLLmult is out of range"
#   endif
#   if (BOARD_PARAM(PLLdiv) == 2)
#       define _PLL_DIV    (0<<25)
#   elif (BOARD_PARAM(PLLdiv) == 4)
#       define _PLL_DIV    (1<<25)
#   elif (BOARD_PARAM(PLLdiv) == 6)
#       define _PLL_DIV    (2<<25)
#   elif (BOARD_PARAM(PLLdiv) == 8)
#       define _PLL_DIV    (3<<25)
#   else
#       error "PLL Divider from BOARD_PARAM_PLLdiv is out of range"
#   endif
#   if (PLATFORM_PLLCLOCK_HZ > 80000000)
#       error "PLL Flank Speed Clock must be less than 80 MHz"
#   elif (PLATFORM_PLLCLOCK_HZ > 64000000)
#       define _FLANKSPEED_FLASHWAIT    4
#   elif (PLATFORM_PLLCLOCK_HZ > 48000000)
#       define _FLANKSPEED_FLASHWAIT    3
#   elif (PLATFORM_PLLCLOCK_HZ > 32000000)
#       define _FLANKSPEED_FLASHWAIT    2
#   elif (PLATFORM_PLLCLOCK_HZ > 16000000)
#       define _FLANKSPEED_FLASHWAIT    1
#   else
#       define _FLANKSPEED_FLASHWAIT    0
#   endif
#else
#   define _PLL_SRC     0
#   define _PLL_MULT    0
#   define _PLL_DIV     0
#endif

// Flankspeed must use the PLL (that's the whole point)
#if BOARD_FEATURE(FLANKSPEED)
#   if (BOARD_FEATURE(PLL) != ENABLED)
#       error "Cannot have Flank Speed without PLL"
#   endif

#   define _FLANKOSC_CLOCKBIT       3
#   define _FLANKSPEED_VOLTAGE      POWER_1V2
#   define _FLANKSPEED_HZ           PLATFORM_PLLCLOCK_HZ
#   define _DEFCLK_HZ               _FLANKSPEED_HZ
#   define _DEFCLK_MODE             SPEED_Flank

#   if BOARD_FEATURE(FLANKXTAL)
#       if BOARD_FEATURE(HFBYPASS)
#           define _FLANKOSC_ONBIT      (RCC_CR_HSEON | RCC_CR_HSEBYP)
#           define _FLANKOSC_TIMEOUT    1000
#       else 
#           define _FLANKOSC_ONBIT      RCC_CR_HSEON
#           define _FLANKOSC_TIMEOUT    3000 //HSE_STARTUP_TIMEOUT
#       endif
#       define _FLANKOSC_RDYFLAG        RCC_CR_HSERDY
#   else
#       define _FLANKOSC_ONBIT          _RCC_CR_HSI
#       define _FLANKOSC_RDYFLAG        RCC_CR_HSIRDY
#       define _FLANKOSC_TIMEOUT        300 //HSI_STARTUP_TIMEOUT
#   endif

#   define _FLANKSPEED_ON()     ((RCC->CFGR & 3) == _FLANKOSC_CLOCKBIT)
#   define _FLANKSPEED_OFF()    ((RCC->CFGR & 3) != _FLANKOSC_CLOCKBIT)
#   define _FLANKRQ()           (platform_ext.rqflank != 0)

#else
#   define _FLANKSPEED_ON()     0
#   define _FLANKSPEED_OFF()    1
#   define _FLANKRQ()           0

#endif

// Fullspeed uses HSE or HSI without PLL
// Support for other clocking options is pending
#if BOARD_FEATURE(FULLSPEED)
#   define _FULLSPEED_VOLTAGE   POWER_1V2
#   define _FULLSPEED_HZ        PLATFORM_HSCLOCK_HZ
#   undef _DEFCLK_HZ
#   undef _DEFCLK_MODE
#   define _DEFCLK_HZ           _FULLSPEED_HZ
#   define _DEFCLK_MODE         SPEED_Full

#   if (PLATFORM_HSCLOCK_HZ > 48000000)
#       error "High Speed Clock must be less than 48 MHz"
#   elif (PLATFORM_HSCLOCK_HZ > 32000000)
#       define _FULLSPEED_FLASHWAIT 2
#   elif (PLATFORM_HSCLOCK_HZ > 16000000)
#       define _FULLSPEED_FLASHWAIT 1
#   else
#       define _FULLSPEED_FLASHWAIT 0
#   endif
#   if (BOARD_FEATURE_FULLXTAL)
#       define _FULLOSC_RDYFLAG         RCC_CR_HSERDY
#       define _FULLOSC_CLOCKBIT        2
#       if (BOARD_FEATURE_HFBYPASS)
#           define _FULLOSC_ONBIT       (RCC_CR_HSEON | RCC_CR_HSEBYP)
#           define _FULLOSC_TIMEOUT     10
#       else
#           define _FULLOSC_ONBIT       RCC_CR_HSEON
#           define _FULLOSC_TIMEOUT     3000 //HSE_STARTUP_TIMEOUT
#       endif
#   else
#       define _FULLOSC_ONBIT           RCC_CR_HSION
#       define _FULLOSC_RDYFLAG         RCC_CR_HSIRDY
#       define _FULLOSC_CLOCKBIT        1
#       define _FULLOSC_TIMEOUT         300 //HSI_STARTUP_TIMEOUT
#   endif
#   define _FULLSPEED_ON()      ((RCC->CFGR & 3) == _FULLOSC_CLOCKBIT)
#   define _FULLSPEED_OFF()     ((RCC->CFGR & 3) != _FULLOSC_CLOCKBIT)
#   define _FULLRQ()            (platform_ext.rqfull != 0)
#else
#   define _FULLSPEED_ON()      0
#   define _FULLSPEED_OFF()     1
#   define _FULLRQ()            0
#endif

// Standard Speed uses HSI16 at voltage range 1 (1.2V)
#if BOARD_FEATURE(STDSPEED)
#   define _STDSPEED_VOLTAGE    POWER_1V2
#   define _STDSPEED_HZ         16000000
#   define _STDSPEED_FLASHWAIT  0
#   undef _DEFCLK_HZ
#   undef _DEFCLK_MODE
#   define _DEFCLK_HZ           _STDSPEED_HZ
#   define _DEFCLK_MODE         SPEED_enum
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
#   define _STDFULL_DOWNVOLT()      (_FULLSPEED_ON() && (_FULLSPEED_VOLTAGE != _STDSPEED_VOLTAGE))
#   define _STDFLANK_DOWNVOLT()     (_FLANKSPEED_ON() && (_STDSPEED_VOLTAGE != _FLANKSPEED_VOLTAGE))
#   define _STD_DOWNVOLT()          (_STDFLANK_DOWNVOLT() || _STDFULL_DOWNVOLT())
#   define _FULL_UPVOLT()           (_STDSPEED_ON() && (_FULLSPEED_VOLTAGE != _STDSPEED_VOLTAGE))
#   define _FULL_DOWNVOLT()         (_FLANKSPEED_ON() && (_FULLSPEED_VOLTAGE != _FLANKSPEED_VOLTAGE))
#   define _FLANKSTD_UPVOLT()       (_STDSPEED_ON() && (_STDSPEED_VOLTAGE != _FLANKSPEED_VOLTAGE))
#   define _FLANKFULL_UPVOLT()      (_FULLSPEED_ON() && (_FULLSPEED_VOLTAGE != _FLANKSPEED_VOLTAGE))
#   define _FLANK_UPVOLT()          (_FLANKSTD_UPVOLT() || _FLANKFULL_UPVOLT())
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
#   undef _FULLRQ
#   undef _FLANKRQ
#   define _FLANK_UPVOLT()      0
#   define _FULL_DOWNVOLT()     0
#   define _FULLRQ()            0
#   define _FLANKRQ()           1
#endif


// Final derivation of clock dividers for each speed, and conversion to register values
#define _STD_AHBHZ           (_STDSPEED_HZ/BOARD_PARAM_AHBCLKDIV)
#define _STD_APB1HZ          (_STD_AHBHZ/BOARD_PARAM_APB1CLKDIV)
#define _STD_APB2HZ          (_STD_AHBHZ/BOARD_PARAM_APB2CLKDIV)
#define _STD_AHBDIV          BOARD_PARAM_AHBCLKDIV
#define _STD_APB1DIV         BOARD_PARAM_APB1CLKDIV
#define _STD_APB2DIV         BOARD_PARAM_APB2CLKDIV
#if ((_STD_AHBDIV!=1)||(_STD_AHBDIV!=2)||(_STD_AHBDIV!=4)||(_STD_AHBDIV!=8)||(_STD_AHBDIV!=16)) 
#    error "AHB in standard speed configuration is out of range (divider must be: 1,2,4,8,16)."
#endif 
#if ((_STD_APB1DIV!=1)||(_STD_APB1DIV!=2)||(_STD_APB1DIV!=4)||(_STD_APB1DIV!=8)||(_STD_APB1DIV!=16)) 
#    error "APB1 in standard speed configuration is out of range (divider must be: 1,2,4,8,16)."
#endif 
#if ((_STD_APB2DIV!=1)||(_STD_APB2DIV!=2)||(_STD_APB2DIV!=4)||(_STD_APB2DIV!=8)||(_STD_APB2DIV!=16)) 
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
#   if ((_FULL_AHBDIV!=1)||(_FULL_AHBDIV!=2)||(_FULL_AHBDIV!=4)||(_FULL_AHBDIV!=8)||(_FULL_AHBDIV!=16)) 
#       error "AHB in Full speed configuration is out of range (divider must be: 1,2,4,8,16)."
#   endif
#   if ((_FULL_APB1DIV!=1)||(_FULL_APB1DIV!=2)||(_FULL_APB1DIV!=4)||(_FULL_APB1DIV!=8)||(_FULL_APB1DIV!=16)) 
#       error "APB1 in Full speed configuration is out of range (divider must be: 1,2,4,8,16)."
#   endif
#   if ((_FULL_APB2DIV!=1)||(_FULL_APB2DIV!=2)||(_FULL_APB2DIV!=4)||(_FULL_APB2DIV!=8)||(_FULL_APB2DIV!=16)) 
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

#if BOARD_FEATURE(FLANKSPEED)
#   define _FLANK_AHBHZ         (_FLANKSPEED_HZ/BOARD_PARAM_AHBCLKDIV)
#   define _FLANK_AHBDIV        (BOARD_PARAM_AHBCLKDIV)
#   define _FLANK_APB1DIV       (BOARD_PARAM_APB1CLKDIV*((_FLANKSPEED_HZ+_DEFCLK_HZ-1)/_DEFCLK_HZ))
#   define _FLANK_APB2DIV       (BOARD_PARAM_APB2CLKDIV*((_FLANKSPEED_HZ+_DEFCLK_HZ-1)/_DEFCLK_HZ))
#   if ((_FLANK_AHBDIV!=1)||(_FLANK_AHBDIV!=2)||(_FLANK_AHBDIV!=4)||(_FLANK_AHBDIV!=8)||(_FLANK_AHBDIV!=16)) 
#       error "AHB in Flank speed configuration is out of range (divider must be: 1,2,4,8,16)."
#   endif
#   if ((_FLANK_APB1DIV!=1)||(_FLANK_APB1DIV!=2)||(_FLANK_APB1DIV!=4)||(_FLANK_APB1DIV!=8)||(_FLANK_APB1DIV!=16)) 
#       error "APB1 in Flank speed configuration is out of range (divider must be: 1,2,4,8,16)."
#   endif
#   if ((_FLANK_APB2DIV!=1)||(_FLANK_APB2DIV!=2)||(_FLANK_APB2DIV!=4)||(_FLANK_APB2DIV!=8)||(_FLANK_APB2DIV!=16)) 
#       error "APB2 in Flank speed configuration is out of range (divider must be: 1,2,4,8,16)."
#   endif
#   if (_FLANK_AHBDIV == 1)
#       define _FLANK_AHBDIV_VAL (0<<4)
#       define _FLANK_AHBSHIFT   (0)
#   elif (_FLANK_AHBDIV == 2)
#       define _FLANK_AHBDIV_VAL (8<<4)
#       define _FLANK_AHBSHIFT   (1)
#   elif (_FLANK_AHBDIV == 4)
#       define _FLANK_AHBDIV_VAL (9<<4)
#       define _FLANK_AHBSHIFT   (2)
#   elif (_FLANK_AHBDIV == 8)
#       define _FLANK_AHBDIV_VAL (10<<4)
#       define _FLANK_AHBSHIFT   (3)
#   elif (_FLANK_AHBDIV == 16)
#       define _FLANK_AHBDIV_VAL (11<<4)
#       define _FLANK_AHBSHIFT   (4)
#   endif
#   if (_FLANK_APB1DIV == 1)
#       define _FLANK_APB1DIV_VAL (0<<8)
#       define _FLANK_APB1SHIFT   (0)
#   elif (_FLANK_APB1DIV == 2)
#       define _FLANK_APB1DIV_VAL (4<<8)
#       define _FLANK_APB1SHIFT   (1)
#   elif (_FLANK_APB1DIV == 4)
#       define _FLANK_APB1DIV_VAL (5<<8)
#       define _FLANK_APB1SHIFT   (2)
#   elif (_FLANK_APB1DIV == 8)
#       define _FLANK_APB1DIV_VAL (6<<8)
#       define _FLANK_APB1SHIFT   (3)
#   elif (_FLANK_APB1DIV == 16)
#       define _FLANK_APB1DIV_VAL (7<<8)
#       define _FLANK_APB1SHIFT   (4)
#   endif
#   if (_FLANK_APB2DIV == 1)
#       define _FLANK_APB2DIV_VAL (0<<11)
#       define _FLANK_APB2SHIFT   (0)
#   elif (_FLANK_APB2DIV == 2)
#       define _FLANK_APB2DIV_VAL (4<<11)
#       define _FLANK_APB2SHIFT   (1)
#   elif (_FLANK_APB2DIV == 4)
#       define _FLANK_APB2DIV_VAL (5<<11)
#       define _FLANK_APB2SHIFT   (2)
#   elif (_FLANK_APB2DIV == 8)
#       define _FLANK_APB2DIV_VAL (6<<11)
#       define _FLANK_APB2SHIFT   (3)
#   elif (_FLANK_APB2DIV == 16)
#       define _FLANK_APB2DIV_VAL (7<<11)
#       define _FLANK_APB2SHIFT   (4)
#   endif
#else
#   define _FLANK_AHBDIV_VAL    _FULL_AHBDIV_VAL
#   define _FLANK_APB2DIV_VAL   _FULL_APB2DIV_VAL
#   define _FLANK_APB1DIV_VAL   _FULL_APB1DIV_VAL
#   define _FLANKOSC_CLOCKBIT   _FULLOSC_CLOCKBIT
#   define _FLANK_AHBSHIFT      _FULL_AHBSHIFT
#   define _FLANK_APB1SHIFT     _FULL_APB1SHIFT
#   define _FLANK_APB2SHIFT     _FULL_APB2SHIFT
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
    while((PWR->CSR2 & PWR_CSR2_VOSF) != 0) { }
}


void sub_osc_startup(ot_u16 counter, ot_u32 osc_mask) {
///@todo figure out a way to do this with WFE
///@note HSIASFS bit is not set here, because the clock system
///      only uses one clock at a time.
    //ot_u32 hsi_asfs_bit;
    ot_int osc_shift;
    
    if (osc_mask & RCC_CR_HSION) {
        //hsi_asfs_bit = RCC_CR_HSIASFS;
        osc_shift    = 2;
    }
    else {
        //hsi_asfs_bit = 0;
        osc_shift    = 1;
    }
    
    // Wait for Oscillator to get ready, counter goes to 0 on failure
    RCC->CR    |= osc_mask /*| hsi_asfs_bit*/;
    osc_mask  <<= osc_shift;
    while ( ((RCC->CR & osc_mask) == 0) && (--counter) );

    /// Test if oscillator failed to startup
    if (counter == 0) {
        ///@todo Death message / Death Blinkly
    }
}




void sub_setclocks(SPEED_enum mode) {
/// In interest of speed and size, you need to setup your clock dividers as
/// constants in the board configuration file.    
    static const ot_u32 params[3][4] = {
        { (_STD_AHBDIV_VAL|_STD_APB2DIV_VAL|_STD_APB1DIV_VAL|1), \
            _STD_AHBSHIFT, _STD_APB1SHIFT, _STD_APB2SHIFT },
        { (_FULL_AHBDIV_VAL|_FULL_APB2DIV_VAL|_FULL_APB1DIV_VAL|_FULLOSC_CLOCKBIT), \
            _FULL_AHBSHIFT, _FULL_APB1SHIFT, _FULL_APB2SHIFT },
        { (_FLANK_AHBDIV_VAL|_FLANK_APB2DIV_VAL|_FLANK_APB1DIV_VAL|_FLANKOSC_CLOCKBIT), \
            _FLANK_AHBSHIFT, _FLANK_APB1SHIFT, _FLANK_APB2SHIFT }
    };
    ot_u32 scratch;
    ot_u32* mode_params;

    // Configure clock source and dividers
    scratch     = RCC->CFGR;
    scratch    &= ~(RCC_CFGR_HPRE_Msk|RCC_CFGR_PPRE1_Msk|RCC_CFGR_PPRE2_Msk|RCC_CFGR_SW_Msk);
    mode_params = params[mode];
    scratch    |= mode_params[0];
    RCC->CFGR   = scratch;
    
    // Validate that configuration was successful, and bail-out to hwfault if not.
    scratch     = (scratch & 3) << 2;
    while ( (RCC->CFGR & (3<<2)) != scratch) {
        ///@todo have this fail into hardware fault
    }
    
    platform_ext.clock_hz[0] = sysclock_hz >> mode_params[1];                   //AHB
    platform_ext.clock_hz[1] = platform_ext.clock_hz[0] >> mode_params[2];      //APB1
    platform_ext.clock_hz[2] = platform_ext.clock_hz[0] >> mode_params[3];      //APB2
}




/** Extended Platform (STM32L4-specific) power and PLL control<BR>
  * ========================================================================<BR>
  */
#ifndef EXTF_platform_ext_wakefromstop
void platform_ext_wakefromstop() {

    // Start the clocker right on wakeup.  The clock may get skewed a bit, 
    // but this is acceptable.
    systim_start_clocker();

    // If using Multispeed setup, we want to use the clock that was in use 
    // before going into STOP.
    // - In Standard Speed, don't do anything, HSI is already up and running
    // - In Full Speed + HSI, STOPWUCK will be set and still nothing to do 
    // - In Full Speed + HSE, we need to start-up the crystal
    // - In Flank Speed, we need to start up the PLL and optionally the crystal
#   if MCU_CONFIG(MULTISPEED)
        if ( _FLANKRQ() ) {
            platform_flank_speed();
        }
        else if ( _FULLRQ() && (_FULLOSC_CLOCKBIT != 1)) {
            platform_full_speed();
        }

    // Same basic rules as above apply to single-speed configurations.
    // MSI and HSI based clocks will be already running on wakeup.
#   elif (BOARD_FEATURE(FULLSPEED) && (_FULLOSC_CLOCKBIT != 1))       
        platform_full_speed();
#   elif BOARD_FEATURE(FLANKSPEED)
        platform_flank_speed();

#   endif
}
#endif



void sub_hsi48on(void) {
    ot_int limit = 3;
    RCC->CRRCR = RCC_CRRCR_HSI48ON;
    
    while (((RCC->CRRCR & RCC_CRRCR_HSI48RDY) == 0) && --limit);
    if (limit == 0) {
        __NOP();
        //HW Fault;
        //Reset
    }
}

void sub_hsi48off(void) {
    RCC->CRRCR = 0;
}


#ifndef EXTF_platform_ext_usbcrson
void platform_ext_usbcrson(void) {
    sub_hsi48on();
    
    RCC->APB1ENR   |= (RCC_APB1ENR1_USBFSEN | RCC_APB1ENR1_CRSEN);
    
    CRS->CFGR       = (0 << CRS_CFGR_SYNCPOL_Pos) \
                    | (2 << CRS_CFGR_SYNCSRC_Pos) \
                    | (0x22 << 16) /* Default FELIM */ \
                    | (0xBB7F);    /* Default RELOAD (1ms USB SOF) */

    ///@todo make sure CEN doesn't need to be set as an independent follow-up
    CRS->CR         = (32 << 8) | CRS_CR_AUTOTRIMEN | CRS_CR_CEN;
}
#endif


#ifndef EXTF_platform_ext_usbcrsoff
void platform_ext_usbcrsoff(void) {
    CRS->CR         = (32 << 8) | CRS_CR_AUTOTRIMEN | 0;
    RCC->APB1ENR1  &= ~(RCC_APB1ENR1_USBFSEN | RCC_APB1ENR1_CRSEN);
    
    sub_hsi48off();
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
#if MCU_CONFIG(MULTISPEED)
    ot_int handle;
    switch (Speed) {
#   if BOARD_FEATURE(FULLSPEED)
        case SPEED_Full:    platform_full_speed();
                            platform_ext.rqfull++;
                            handle = 0;
                            break;
#   endif
#   if BOARD_FEATURE(FLANKSPEED)
        case SPEED_Flank:   platform_flank_speed();
                            platform_ext.rqflank++;
                            handle = (BOARD_FEATURE(FULLSPEED) == ENABLED);
                            break;
#   endif
        default: return -1;
    }
    return handle;
    
#else
    return -1;
    
#endif
}


ot_int sysclock_dismiss(ot_int handle) {
#if MCU_CONFIG(MULTISPEED)
    if (handle < 0) || (handle > (BOARD_FEATURE(FLANKSPEED) == ENABLED)) {
        return -1;
    }
    platform_ext.rqspeed[handle]--;
    
    if ( _FLANKRQ() )       platform_flank_speed();
    else if ( _FULLRQ() )   platform_full_speed();
    else                    platform_standard_speed();
    return 0;
    
#else
    return -1;
    
#endif
}


#ifndef EXTF_platform_standard_speed
void platform_standard_speed() {
/// Best efficient speed. (HSI16)
/// typ: 16 MHz, 1.2V, 0 wait state

    // If Std speed is enabled and Fullspeed is the same thing...
#   if BOARD_FEATURE(STDSPEED) && (_FULLOSC_CLOCKBIT == 1)
    platform_full_speed();

    // If std speed is enabled but not the same as full speed, then
    // it is 16 MHz via HSI.
#   elif BOARD_FEATURE(STDSPEED)
    if ( _STDSPEED_OFF() /* && !_FULLRQ() && !_FLANKRQ() */ ) {
        platform_disable_interrupts();
        
        // Don't need to startup HSI, because it is always on in this impl
        //sub_osc_startup(300, RCC_CR_HSION);

        // HSI @ 16MHz and 1.2V never require flash wait states or prefetch
        sub_setclocks(SPEED_Standard);
        FLASH->ACR = (FLASH_ACR_DCEN | FLASH_ACR_ICEN);
        
        // Set new core voltage, if necessary.
        if (STD_DOWNVOLT()) {
            sub_voltage_config(_STDSPEED_VOLTAGE | _RTC_PROTECTION);
        }

        // Turn off any clocks that are not HSI
        RCC->CR &= RCC_CR_HSION | ~(RCC_CR_HSEBYP|RCC_CR_HSEON|RCC_CR_MSION|RCC_CR_PLLON);
        BOARD_HSXTAL_OFF();
        
        platform_enable_interrupts();
    }    
#endif
}
#endif




#ifndef EXTF_platform_full_speed
void platform_full_speed() {
/// All Ahead Full.  (HSI or HSE, no PLL)
/// <LI> typ config: 16MHz HSI, 1.2V, 0 wait states</LI>
/// <LI> In system with attachable USB, check for flank-enable or CRS </LI>
/// 
/// Only go through the process of entering Full Speed if it is not activated
/// already.  In multispeed systems, don't call this function directly unless you know
/// exactly what you're doing.  Use sysclock_request() instead.
///
#   if BOARD_FEATURE(FULLSPEED)
    if (_FULLSPEED_OFF()) {
        platform_disable_interrupts();
        
        // Increase the voltage if necessary (must happen before configuration).
        // Platform impl has universal 1.2V, but code is left in case that ever changes.
        if (_FULL_UPVOLT()) {
            sub_voltage_config(_FULLSPEED_VOLTAGE);
        }
        
        // HSI16 is always active, so don't start the fullspeed clock unless it's
        // a different clock than HSI16.  After it's ready, set it as system clock.
#       if (_FULLOSC_CLOCKBIT != 1)
            sub_osc_startup(_FULLOSC_TIMEOUT, _FULLOSC_ONBIT);
#       endif
        sub_setclocks(SPEED_Full);

        // Set the Flash Wait States, bus caches, and prefetch feature as needed.
        // For Flash latency 0, prefetch and wait states are off.
#       if (_FULLSPEED_FLASHWAIT > 0)
            FLASH->ACR = (FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_ACR_PRFTEN | _FULLSPEED_FLASHWAIT);
#       else
            ///@todo have this as default setting in platform_busclk_init()
            FLASH->ACR = (FLASH_ACR_DCEN | FLASH_ACR_ICEN | 0);
#       endif

        // Turn off Clocks not used by Fullspeed, although HSI16 must stay on.
        RCC->CR &= _FULLOSC_ONBIT | ~(RCC_CR_HSEBYP|RCC_CR_HSEON|RCC_CR_PLLON|RCC_CR_MSION);
#       if ((_FULLOSC_ONBIT & (RCC_CR_HSEON | RCC_CR_HSEBYP)) == 0)
        BOARD_HSXTAL_OFF();
#       endif
        
        // Decrease the voltage if necessary (must happen after configuration).
        // Platform impl has universal 1.2V, but code is left in case that ever changes.
        if (_FULL_DOWNVOLT()) {
            sub_voltage_config(_FULLSPEED_VOLTAGE);
        }

        platform_enable_interrupts();
    }
#   endif
}
#endif


#ifndef EXTF_platform_flank_speed
void platform_flank_speed() {
/// Coming in hot!  (HSI or HSE + PLL)
/// typ config: 64MHz, 1.2V, 3 wait states
#   if BOARD_FEATURE(FLANKSPEED)
    if ( _FLANKSPEED_OFF() ) {
        platform_disable_interrupts();
        
        // Flank Speed, by definition, uses the PLL.
        // platform_busclk_init() will pre-set all the PLL specifications at startup.
        if ((RCC->CR & RCC_CR_PLLON) == 0) {
            if ( _FLANK_UPVOLT() ) {
                sub_voltage_config(_FLANKSPEED_VOLTAGE);
            }
    
            // Startup the oscillator used by Flank Speed
#           if (_FLANKOSC_ONBIT & (RCC_CR_HSEON | RCC_CR_HSEBYP))
            BOARD_HSXTAL_ON();
#           elif (_FLANKOSC_ONBIT & RCC_CR_HSION)
            sub_osc_startup(_FLANKOSC_TIMEOUT, _FLANKOSC_ONBIT);
#           endif

            // Startup PLL and wait for ready
            ///@todo have a counter to wait here and go to hwfault
            RCC->CR |= RCC_CR_PLLON;
            while((RCC->CR & RCC_CR_PLLRDY) == 0) { }
        }

        // Set the Flank-speed oscillator (PLL) as the system clock
        sub_setclocks(SPEED_Flank);

        // Set the Flash Wait States, bus caches, and prefetch feature as needed.
        // For Flash latency 0, prefetch and wait states are off.
#       if (_FLANKSPEED_FLASHWAIT > 0)
            FLASH->ACR = (FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_ACR_PRFTEN | _FULLSPEED_FLASHWAIT);
#       else
            ///@todo have this as default setting in platform_busclk_init()
            FLASH->ACR = (FLASH_ACR_DCEN | FLASH_ACR_ICEN | 0);
#       endif

        // Flank speed leaves all clocks on.
        //RCC->CR &= _FLANKOSC_ONBIT | ~(RCC_CR_HSEBYP|RCC_CR_HSEON|RCC_CR_MSION|RCC_CR_PLLON|RCC_CR_MSION);
        
        platform_enable_interrupts();
    }

#else
    platform_full_speed();

#endif
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

    ///1. Set cooperative task runtime in P-stack,
    ///   Interrupts and protected system calls run in M-stack
    __set_PSP( __get_MSP() );
    __set_CONTROL(2);
    __set_MSP( (ot_u32)&platform_ext.sstack[(OT_PARAM_SSTACK_ALLOC/4)-1] );

    /// 3. Board-Specific power-up configuration
    /// 3b. Platform impl demands that voltage is at 1.2V
    BOARD_PERIPH_INIT();
    BOARD_POWER_STARTUP();
    sub_voltage_config(POWER_1V2);

    ///2. Configure GPIO
    //platform_init_gpio();
    BOARD_PORT_STARTUP();

    /// 3. Configure Clocks
    platform_init_periphclk();
    platform_init_busclk();
#   if (BOARD_FEATURE(HFXTAL) != ENABLED)
    //platform_ext_hsitrim();
#   endif

    /// 5. Debugging setup: apply to all peripherals
#   if defined(__DEBUG__)
    DBGMCU->CR         |= ( DBGMCU_CR_DBG_SLEEP | DBGMCU_CR_DBG_STOP | DBGMCU_CR_DBG_STANDBY);

    DBGMCU->APB1FZR1   |=DBGMCU_APB1FZR1_DBG_TIM2_STOP \
                       | DBGMCU_APB1FZR1_DBG_TIM6_STOP \
                       | DBGMCU_APB1FZR1_DBG_RTC_STOP \
                       | DBGMCU_APB1FZR1_DBG_WWDG_STOP \
                       | DBGMCU_APB1FZR1_DBG_IWDG_STOP \
                       | DBGMCU_APB1FZR1_DBG_I2C1_STOP \
                       | DBGMCU_APB1FZR1_DBG_I2C2_STOP \
                       | DBGMCU_APB1FZR1_DBG_I2C3_STOP \
                       | DBGMCU_APB1FZR1_DBG_LPTIM1_STOP;
                       
    DBGMCU->APB1FZR2   |= DBGMCU_APB1FZR2_DBG_LPTIM2_STOP;

    DBGMCU->APB2FZ     |=DBGMCU_APB2FZ_DBG_TIM1_STOP \
                       | DBGMCU_APB2FZ_DBG_TIM15_STOP \
                       | DBGMCU_APB2FZ_DBG_TIM16_STOP;
#   endif

    /// 6. Final initialization of OpenTag system resources
               // Set up connections on the board
    platform_init_interruptor();    // Interrupts OpenTag cares about
    systim_init(NULL);         // Initialize GPTIM (to 1024 Hz)

    /// 7. Initialize Low-Level Drivers (worm, mpipe)
    // Restore vworm (following save on shutdown)
    vworm_init();

    /// 8. This prevents the scheduler from getting called by a preemption
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
    if (RTC->BKP2R != 0) {
        vlFILE*     fp;
        ot_uni16    resets;
        ot_uni16    faults;
        ot_uni32    error;

        error.ulong     = RTC->BKP2R;
        RTC->BKP2R      = 0;
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

    /// 4. If debugging, copy the UNIQUE ID that ST writes into the ROM into
    ///    the lower 48 bits of the Mode 2 UID (Device Settings ISF)
    ///
    /// @note the ID is inserted via Veelite, so it is abstracted from the
    /// file memory configuration of your board and/or app.
    ///
    /// @note for production (__RELEASE__) the default UID should be written to
    ///      the default file location by the manufacturer firmware upload.
#   if (defined(__DEBUG__) || defined(__PROTO__))
    {   vlFILE* fpid;
        union {
            ot_u32 word[3];
            ot_u16 halfw[6];
        } generated_id;
        ot_u16* hwid;
        ot_int  i;

        // 96 bits with ASCII
        chip_id.word[2] = *((ot_u32*)(0x1FFF7590));     //X/Y coords on wafer
        chip_id.word[1] = *((ot_u32*)(0x1FFF7594));     //Lower Lot number (ASCII, b31:8) and wafer number (b7:0)
        chip_id.word[0] = *((ot_u32*)(0x1FFF7598));     //Upper Lot number (ASCII)
        
        ///@todo compression of the ID.  There are many gaps in the ID
        
        fpid    = ISF_open_su(ISF_ID(device_features));
        hwid    = &generated_id.halfw[3];
        for (i=6; i!=0; i-=2) {
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

    // Load Default Clock Configuration
    // On the STM32L4 platform, OpenTag uses HSI16 for a number of peripherals.
    // HSI16 MUST be available, and thus it's OK to have STOPWUCK set permanently.
    RCC->CFGR |= RCC_CFGR_STOPWUCK;

    // Load Default PLL specification.
    // This specification cannot be changed at runtime.
    RCC->PLLCFGR = _PLL_DIV | _PLL_MULT | (0<<4) | _PLL_SRC;
    
    // MSI is the startup clock.  HSI, however, is to be our default clock via using 
    // the STOPWUCK bit.  Several peripherals will use HSI16 directly, so it will be
    // always on.
    // HSI @ 16MHz and 1.2V never require flash wait states or prefetch
    sub_osc_startup(300, RCC_CR_HSION);
    sub_setclocks(SPEED_Standard);
    FLASH->ACR = (FLASH_ACR_DCEN | FLASH_ACR_ICEN);
    
    // Disable all clocks that aren't HSI16, and disable all clock interrupts
    RCC->CR    &= ~(RCC_CR_HSEON|RCC_CR_HSEBYP|RCC_CR_CSSON|RCC_CR_PLLON|RCC_CR_MSION);
    RCC->CIER   = 0;

    ///2. Prepare external Memory bus (not currently supported)
//#   ifdef DATA_IN_ExtSRAM
//        SystemInit_ExtMemCtl();
//#   endif

    ///3a. If Standard Speed is enabled, it is already underway (HSI16), so do nothing
#   if BOARD_FEATURE(STDSPEED)

    ///3b. Use HSE or HSI16 without PLL as Full-Speed clock
    ///    <LI> If HSI16, that's already active, so do nothing. </LI>
    ///    <LI> If HSE, switch to it.</LI>
#   elif BOARD_FEATURE(FULLSPEED) && (_FULLOSC_CLOCKBIT != 1)
        platform_full_speed();

    ///3c. Begin clocking system with PLL driven from HSE or HSI
#   elif BOARD_FEATURE(FLANKSPEED)
        platform_flank_speed();

#   else
#       error "At least one of BOARD_FEATURE_STDSPEED, _FULLSPEED, or _FLANKSPEED must be ENABLED"
#   endif
    
    RCC->CCIPR  = (0) \
                | (0 << RCC_CCIPR_CLK48SEL_Pos) \
                | (BOARD_FEATURE(LFXTAL) << 21) | (1 << 20) /* LSI/LSE for LPTIM2 */ \
                | (BOARD_FEATURE(LFXTAL) << 19) | (1 << 18) /* LSI/LSE for LPTIM1 */ \
                | ((MCU_CONFIG(MULTISPEED)*2) << 16)        /* APB/HSI16 for I2C3 */ \
                | ((MCU_CONFIG(MULTISPEED)*2) << 14)        /* APB/HSI16 for I2C2 */
                | ((MCU_CONFIG(MULTISPEED)*2) << 12)        /* APB/HSI16 for I2C1 */ \
                | ((BOARD_FEATURE(LFXTAL)*3) << 10)         /* APB/LSE for LPUART1 */ \
                | ((MCU_CONFIG(MULTISPEED)*2) << 6)         /* APB/HSI16 for USART4 */ \
                | ((MCU_CONFIG(MULTISPEED)*2) << 4)         /* APB/HSI16 for USART3 */ \
                | ((MCU_CONFIG(MULTISPEED)*2) << 2)         /* APB/HSI16 for USART2 */ \
                | ((MCU_CONFIG(MULTISPEED)*2) << 0);        /* APB/HSI16 for USART1 */
                
    RCC->CCIPR2 = ((MCU_CONFIG(MULTISPEED)*2) << 0);        /* APB/HSI16 for I2C4 */

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
/// OpenTag likes having a 32768Hz clock for timing purposes.
/// LPTIM1, 2, and the RTC are all driven by the LF clock.

    ///@todo have a setting that looks at the backup domain to make sure the clocking
    ///      for it is not already what it should be.
    
    PWR->CR1 |= PWR_CR_DBP;
    //RCC->BDCSR |= RCC_BDCSR_RTCRST;
    
#   if BOARD_FEATURE(LFXTAL)
        RCC->CSR    = RCC_CSR_RMVF;
        RCC->BDCSR  = (1<<RCC_BDCR_LSCOSEL_Pos) \
                    | (b01<<RCC_BDCR_RTCSEL_Pos) \
                    | RCC_BDCR_LSECSSD \
                    | (b00<<RCC_BDCR_LSEDRV_Pos) \
                    | RCC_BDCR_LSEON;
                
        while ((RCC->CSR & RCC_CSR_LSERDY) == 0);
    
        RCC->BDCSR |= RCC_BDCR_LSCOEN | RCC_BDCR_RTCEN | RCC_BDCR_LSECSSON;   

#   else // enable LSI
        PWR->CR     = ((1<<9) | PWR_CR_DBP);
        //RCC->BDCSR |= RCC_BDCSR_RTCRST;
        RCC->CSR    = RCC_CSR_RMVF | RCC_CSR_LSION;
        while ((RCC->CSR & RCC_CSR_LSIRDY) == 0);
    
        RCC->BDCSR  = (0<<RCC_BDCR_LSCOSEL_Pos) | RCC_BDCR_LSCOEN \
                    | (b10<<RCC_BDCR_RTCSEL_Pos) | RCC_BDCR_RTCEN;
#   endif

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

    // From Reference Manual RM0394 Rev 4, page 328:
    // Line 16: PVD
    // Line 17: USB FS Wakeup Event
    // Line 18: RTC Alarms
    // Line 19: RTC-Tamper/Timestamp/CSS_LSE
    // Line 20: RTC-Wakeup
    // Line 21: COMP1-out
    // Line 22: COMP2-out
    // Line 23: I2C1-wakeup
    // Line 24: I2C2-wakeup
    // Line 25: I2C3-wakeup
    // Line 26: USART1-wakeup
    // Line 27: USART2-wakeup
    // Line 28: USART3-wakeup
    // Line 29: UART4-wakeup
    // Line 31: LPUART1-wakeup
    // Line 32: LPTIM1
    // Line 33: LPTIM2
    // Line 34: SWPMI1
    // Line 35: PVM1
    // Line 37: PVM3
    // Line 38: PVM4
    // Line 39: LCD wakeup
    // Line 40: I2C4-wakeup

    EXTI->PR    = (1<<20) | (1<<32);
    EXTI->IMR  |= (1<<20) | (1<<32);
    EXTI->RTSR |= (1<<20) | (1<<32);

#   if OT_FEATURE(M2)
        NVIC_SetPriority(RTC_IRQn, _KERNEL_GROUP);
        NVIC_EnableIRQ(RTC_IRQn);
#   else
        NVIC_SetPriority(RTC_IRQn, _LOPRI_GROUP);
        NVIC_EnableIRQ(RTC_IRQn);
#   endif

    NVIC_SetPriority(LPTIM1_IRQn, _KERNEL_GROUP);
    NVIC_EnableIRQ(LPTIM1_IRQn);


    /// 5. Setup other external interrupts
    /// @note Make sure board files use the __USE_EXTI(N) definitions
#   if defined(__USE_EXTI0)
    NVIC_SetPriority(EXTI0_IRQn, _KERNEL_GROUP);
    NVIC_EnableIRQ(EXTI0_IRQn);
#   endif
#   if defined(__USE_EXTI1)
    NVIC_SetPriority(EXTI1_IRQn, _KERNEL_GROUP);
    NVIC_EnableIRQ(EXTI1_IRQn);
#   endif
#   if defined(__USE_EXTI2)
    NVIC_SetPriority(EXTI2_IRQn, _KERNEL_GROUP);
    NVIC_EnableIRQ(EXTI2_IRQn);
#   endif
#   if defined(__USE_EXTI3)
    NVIC_SetPriority(EXTI3_IRQn, _KERNEL_GROUP);
    NVIC_EnableIRQ(EXTI3_IRQn);
#   endif
#   if defined(__USE_EXTI4)
    NVIC_SetPriority(EXTI4_IRQn, _KERNEL_GROUP);
    NVIC_EnableIRQ(EXTI4_IRQn);
#   endif
#   if defined(__USE_EXTI5) || defined(__USE_EXTI6) || defined(__USE_EXTI7) || defined(__USE_EXTI8) || defined(__USE_EXTI9)
    NVIC_SetPriority(EXTI9_5_IRQn, _KERNEL_GROUP);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
#   endif
#   if( defined(__USE_EXTI10) || defined(__USE_EXTI11) || defined(__USE_EXTI12) \
    ||  defined(__USE_EXTI13) || defined(__USE_EXTI14) || defined(__USE_EXTI15) )
    NVIC_SetPriority(EXTI15_10_IRQn, _KERNEL_GROUP);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
#   endif


    /// 6. Setup ADC interrupt.  This is needed only for ADC-enabled builds,
    ///    but ADC is frequently used, so it is enabled by default
//#   if defined(__USE_ADC1)
    NVIC_SetPriority(ADC1_2_IRQHandler, _HIPRI_GROUP);
    NVIC_EnableIRQ(ADC1_2_IRQn);
//#   endif

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



