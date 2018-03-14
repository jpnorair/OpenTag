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
  * @file       /platform/posix_c/core_watchdog.c
  * @author     JP Norair
  * @version    R100
  * @date       26 Oct 2015
  * @brief      Core Watchdog implementation in POSIX-C
  * @ingroup    Platform
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>



#ifndef EXTF_WWDG_IRQHandler
void WWDG_IRQHandler(void) {
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




