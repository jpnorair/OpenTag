/*  Copyright 2010-2011, JP Norair
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
  * @file       /apps/Demo_PaLFi/Code_Master/~patch_USBstick/extf_code.c
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       26 May 2012
  * @brief      Extension Code for the RF430 USB stick
  *
  * The Extension code contains patched implementations of the OTAPI functions
  * used to operate the LED1 and LED2.  The USB stick only has a single LED, so
  * we have two brightness settings to show the two different operations.
  ******************************************************************************
  */

#ifndef __EXTF_CODE_C
#define __EXTF_CODE_C

#include "OTAPI.h"
#include "OT_platform.h"

ot_u8 led_mask = 0;


//Some additonal EXTFs are using the normal applets from otlibext


void otapi_led1_on() {
    led_mask               |= 1;
    OT_TRIG2_PORT->SEL     &= ~OT_TRIG2_PIN;
    OT_TRIG1_PORT->DOUT    |= OT_TRIG1_PIN;
}


void otapi_led2_on() {
    led_mask               |= 2;
    OT_TRIG2_PORT->DOUT    |= OT_TRIG2_PIN;
    OT_TRIG2_PORT->SEL     |= OT_TRIG2_PIN;
}


void otapi_led1_off() {
    led_mask &= ~1;
    if (led_mask & 2)   OT_TRIG2_PORT->SEL     |= OT_TRIG2_PIN;
    else                OT_TRIG1_PORT->DOUT    &= ~OT_TRIG1_PIN;
}


void otapi_led2_off() {
    OT_TRIG2_PORT->SEL &= ~OT_TRIG2_PIN;
    led_mask &= ~2;
    if ((led_mask & 1) == 0) {
        OT_TRIG1_PORT->DOUT &= ~OT_TRIG1_PIN;
    }
}


void sys_sig_powerdown(ot_int code) {
///LPM0 Always with USB on this device
    __bis_SR_register(0x18);
    __no_operation();
}



#endif 
