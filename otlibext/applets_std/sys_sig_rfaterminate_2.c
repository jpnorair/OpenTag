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
  * @file       /otlibext/applets_std/sys_sig_rfaterminate_2.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2011
  * @brief      Standard RFA terminate routine
  *
  * This applet implements a commonly used LED routine
  */



#include "OTAPI.h"
#include "radio.h"


#ifdef EXTF_sys_sig_rfaterminate
void sys_sig_rfaterminate(ot_int pcode, ot_int scode) {
    otapi_led2_off();   //Orange LED off
    otapi_led1_off();   //Green LED off
}
#endif


