/*  Copyright 2008-2022, JP Norair
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted provided that the following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright 
  *    notice, this list of conditions and the following disclaimer in the 
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
  * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
  * POSSIBILITY OF SUCH DAMAGE.
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
    /// L0, L4, WL can be debugged through STOP mode, but WL55-CPU2 cannot
    /// be debugged through STOP.
#   if defined(__STM32L0xx__) || defined(__STM32L4xx__) || defined(__STM32WLxx__)
#       if defined(__DEBUG__) && defined(CORE_CM0PLUS)
#           define _USE_STOP    0
#       elif defined(__DEBUG__)
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
    // - ULP option (L0, L1) only reduces power in cases where Vdd < 3V
    // - FWU option (L0, L1) is superfluous unless ULP is set
    // - In STM32L1 implementations we must kill the chrono timer before STOP.
    // - In all STM32L impls, we must clear EXTI's before stop.  In very rare cases,
    //   an EXTI might be missed, but there is nothing that can be done about this.
#   if _USE_STOP
    if (code & 2) {
        BOARD_STOP(code);
    }   
    else 
#   endif
    {   // Normal Sleeping mode (not deep sleep)
        SCB->SCR   &= ~((ot_u32)SCB_SCR_SLEEPDEEP_Msk);
        PWR->CR3    &= ~PWR_CR3_ULPEN;
        platform_enable_interrupts();
        __WFI();
    }

}
//#endif
