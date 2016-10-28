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
  * @file       /otplatform/stm32l1xx/delay_driver.c
  * @author     JP Norair
  * @version    R100
  * @date       26 Aug 2014
  * @brief      Delay OTlib Functions for STM32L
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
    sub_timed_wfe(n, 31);
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





