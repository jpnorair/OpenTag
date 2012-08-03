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
  * @file       /otlibext/applets_std/sys_sig_rfainit.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2012
  * @brief      Standard RFA Init routine
  *
  * This is a callback the kernel uses when it is starting up a radio process.
  * It is used here to turn-on activity LEDs.
  */

#include "OTAPI.h"



#ifdef EXTF_sys_sig_rfainit
void sys_sig_rfainit(ot_int pcode) {
/// Assume that (1 <= code1 <= 5), which is the case in normal operation
    if (pcode < 3)  otapi_led2_on();
    else            otapi_led1_on();
}
#endif
