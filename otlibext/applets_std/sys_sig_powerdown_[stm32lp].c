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
  * ongoing I/O tasks.  On a STM32L with LSE on and RTC disabled, this can 
  * deliver typically about 1.5uA drain.  RTC adds about 0.8uA.
  *
  * For the RF task, the mode above is used if HSI or MSI is the top speed
  * system clock.  If a slow clock is the top speed system clock (HSE or PLL), 
  * then the SLEEP mode is used while the RF task is active.
  * 
  * For other I/O tasks, the SLEEP mode is used with oscillators kept alive.
  * This is required for things like MPipe, which need active peripherals 
  * during the powerdown.
  */

#include "OTAPI.h"
#include "OT_platform.h"


#ifdef EXTF_sys_sig_powerdown
void sys_sig_powerdown(ot_int code) {
/// code = 3: No active I/O Task                                (STOP)
/// code = 2: RF I/O Task active                                (STOP)
/// code = 1: MPipe or other local peripheral I/O task active   (SLEEP)
/// code = 0: Use fastest-exit powerdown mode                   (SLEEP)
    
#if ((MCU_FEATURE_TURBO != ENABLED) && defined(BOARD_PARAM_HFHz))
/// No Turbo Mode and HSI/HSE/PLL used for System Clock.  On exit from STOP,
/// MSI is automatically disabled and HSI/HSE/PLL is enabled.
    if (code & 2) {
        STOP;
        SystemBoost();
    }
    else {
        SLEEP;
    }
    
#   elif (MCU_FEATURE_TURBO != ENABLED)
/// No Turbo Mode, and MSI is the System Clock.  On exit from STOP, MSI is
/// selected by STM32L Hardware, so this is a simple implementation.
    if (code & 2)   STOP;
    else            SLEEP;
    
#   else
/// Turbo is enabled: MSI is the off-boost system clock and HSI/HSE/PLL is the 
/// on-boost system clock (note, HSI is the best one for most things).  On exit
/// from STOP, clock will be automatically boosted on code == 2, or system will
/// be optimized for MSI runtime on code == 3.
    static const ot_sub wakeup_fn[2] = { &SystemBoost, &SystemResume };

    if (code & 2) {
        STOP;
        wakeup_fn[code & 1];
    }
    else {
        SLEEP;
    }
#   endif

}
#endif
