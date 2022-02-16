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
  * @file       /platform/stm32wl55_m0/delay_driver.c
  * @author     JP Norair
  * @version    R100
  * @date       16 Dec 2021
  * @brief      Delay OTlib Functions for STM32WL
  * @ingroup    Delay
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>
#include <otlib/delay.h>


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


#ifndef EXTF_delay_sti
void delay_sti(ot_u16 sti) {
///@todo get WFE working on STM32L
    delay_us( (sti<<5) );
    //sub_timed_wfe(sti, 0);
}
#endif


#ifndef EXTF_delay_ti
void delay_ti(ot_u16 n) {
///@todo get WFE working on STM32L
    delay_ms(n);
    //sub_timed_wfe(n, 31);
}
#endif



#ifndef EXTF_delay_ms
void delay_ms(ot_u16 n) {
    ot_long c;
    c   = (platform_ext.clock_hz[0]>>10);   // Set cycles per ms
    c  *= n;                                // Multiply by number of ms
    do {
        c -= 7;                         // 7 cycles per loop (measured)
    } while (c > 0);
}
#endif


#ifndef EXTF_delay_us
void delay_us(ot_u16 n) {
    ot_long c;
    c   = (platform_ext.clock_hz[0]>>10);   // Set cycles per ms
    c  *= n;                                // Multiply by number of us
    c >>= 10;                               // Divide into cycles per us
    do {
        c -= 7;                         // 7 cycles per loop (measured)
    } while (c > 0);
}
#endif





