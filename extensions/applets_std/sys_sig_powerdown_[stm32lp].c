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
  * @file       /otlibext/applets_std/sys_sig_powerdown_[stm32lp].c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 Dec 2012
  * @brief      STM32 Powerdown routine optimized for low power (LP)
  *
  * This powerdown routine is specific to STM32 chips.  It works best with the
  * STM32L series chips as they have better ways of returning from STOP mode
  * than the STM32F chips do, and STOP mode is the key to getting minimum
  * power usage on all STM32 chips.
  * 
  * STOP+LP+LSE (or +LSI if your board has no LSE) is used when there are no 
  * ongoing MCU-driven I/O tasks.  On a STM32L with LSE on, RTC enabled, and
  * in normal operating circumstances, STOP uses 1.9uA.  Wakeup interval is
  * typically 9-12 us, depending on configuration.
  *
  * For the RF task, STOP mode is used because the RF I/O is managed by the RF
  * chipset.  The STM32L has nothing to do except wait for pin interrupt.
  * 
  * For other I/O tasks, the SLEEP mode is used with oscillators kept alive.
  * This is required for things like MPipe, which need active peripherals 
  * during the powerdown.  SLEEP has very fast wakeup, but it is also hungry.
  * in typical situations, it is drawing 250uA.
  */

#include <otstd.h>
#include <platform/config.h>


#if BOARD_FEATURE(FAST_WAKEUP) || OT_FEATURE(FAST_WAKEUP)
#   define _USE_STOP 0

#else
    // L0 can be debugged through STOP mode
#   if defined(__STM32L0xx__)      
#       ifdef __DEBUG__
#           define _USE_STOP    1
#       else
#           define _USE_STOP    1
#       endif

    // L1 cannot be debugged through STOP mode
#   elif defined(__STM32L1xx__)
#       ifdef __DEBUG__
#           define _USE_STOP    0
#       else
#           define _USE_STOP    1
#       endif

#   else 
#       warning "unsupported type of STM32 defined, it should work OK, but no guarantee."
#   endif

#endif



#ifndef _USE_STOP
#   define _USE_STOP    0
#endif


void sys_sig_waitforevent(void) {
    ot_u32 scbscr_save;
    
    scbscr_save = SCB->SCR;
    SCB->SCR    = scbscr_save | SCB_SCR_SEVONPEND_Msk;
    __WFE();
    SCB->SCR    = scbscr_save;
}


//#ifdef EXTF_sys_sig_powerdown
void sys_sig_powerdown(ot_int code) {
/// STOP is the primary mode used for low-power.  SLEEP is the best you
/// can do while a bus-powered peripheral is working (like UART/I2C/USB
/// as used by MPIPE.
/// code = 3: No active I/O Task                                (STOP)
/// code = 2: RF I/O Task active                                (STOP)
/// code = 1: MPipe or other local peripheral I/O task active   (SLEEP)
/// code = 0: Use fastest-exit powerdown mode                   (SLEEP)

    // STOP mode:
    // - ULP option only reduces power in cases where Vdd < 3V
    // - FWU option is superfluous unless ULP is set
    // - In STM32 implementations we must kill the chrono timer before STOP
    //     and also clear EXTI's.  In very rare cases, an EXTI might be 
    //     missed, but there is nothing that can be done about this.
#   if _USE_STOP
    if (code & 2) {
        BOARD_STOP(code);
    }   
    else 
#   endif
    {   // Normal Sleeping mode (not deep sleep)
        SCB->SCR   &= ~((ot_u32)SCB_SCR_SLEEPDEEP_Msk);
        PWR->CR    &= ~(PWR_CR_PDDS | PWR_CR_LPSDSR | PWR_CR_FWU | PWR_CR_ULP);
        platform_enable_interrupts();
        __WFI();
    }

}
//#endif
