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
  * ongoing MCU-driven I/O tasks.  On a STM32L with LSE on and RTC disabled, 
  * this can deliver typically about 1.5uA drain.  RTC adds about 0.8uA.
  *
  * For the RF task, STOP mode is used because the RF I/O is managed by the RF
  * chipset.  The STM32L has nothing to do except wait for pin interrupt.
  * 
  * For other I/O tasks, the SLEEP mode is used with oscillators kept alive.
  * This is required for things like MPipe, which need active peripherals 
  * during the powerdown.
  */

#include "OTAPI.h"
#include "OT_platform.h"


/// Puts system in "stop" mode.  The LSE remains active, all other clocks are
/// stopped.  GPTIM is driven directly by LSE in STM32L OpenTag (typ TIM9), so
/// it still has the ability to wake-up the MCU.  Wake-up from STOP goes into
/// MSI-clocked active mode (typ 4.2 MHz), and it takes approximately 8.2 us.
void sub_stop() {
  //ot_u32 tmpreg;
  
  //tmpreg      = PWR->CR;
  //tmpreg     &= CR_DS_MASK;       // Clear PDDS and LPDSR bits
  //tmpreg     |= PWR_CR_LPSDSR;    // Set LPDSR bit according to PWR_Regulator value
  //PWR->CR     = tmpreg;
    PWR->CR    |= PWR_CR_LPSDSR;
    SCB->SCR   |= SCB_SCR_SLEEPDEEP;
    __WFI();

    // On wakeup reset SLEEPDEEP bit of Cortex System Control Register
    SCB->SCR &= ~((ot_u32)SCB_SCR_SLEEPDEEP);
    
#   if ((MCU_FEATURE_MULTISPEED != ENABLED) && defined(BOARD_PARAM_HFHz))
    /// No Multispeed and HSI/HSE/PLL used for System Clock.  
    /// On exit from STOP, here we disable MSI and enable HSI/HSE/PLL.
    platform_full_speed();
#   endif
}


/// Put system into "sleep" mode.  Note, this is not "low-power sleep mode."
/// The CPU is off, but the clocks and peripherals remain active.  Sleep mode
/// is used during MPIPE transfers and things like these that require clocked
/// peripherals but not necessarily CPU.
void sub_sleep() {
    PWR->CR    &= (PWR_CR_PDDS | PWR_CR_LPDSR);
    SCB->SCR   &= ~((ot_u32)SCB_SCR_SLEEPDEEP);
    __WFI();
}




#ifdef EXTF_sys_sig_powerdown
void sys_sig_powerdown(ot_int code) {
/// code = 3: No active I/O Task                                (STOP)
/// code = 2: RF I/O Task active                                (STOP)
/// code = 1: MPipe or other local peripheral I/O task active   (SLEEP)
/// code = 0: Use fastest-exit powerdown mode                   (SLEEP)
    switch (code & 2) {
        case 0: sub_sleep();    break;
        case 2: sub_stop();     break;
    }
}
#endif
