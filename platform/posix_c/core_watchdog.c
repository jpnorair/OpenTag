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




