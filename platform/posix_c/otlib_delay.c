/* Copyright 2017 JP Norair
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
  * @file       /platform/posix_c/otlib_delay.c
  * @author     JP Norair
  * @version    R100
  * @date       26 Oct 2017
  * @brief      Delay OTlib Functions for POSIX
  * @ingroup    Delay
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>
#include <otlib/delay.h>

#include <unistd.h>


#ifndef EXTF_delay_sti
void delay_sti(ot_u16 sti) {
    delay_us( sti*31 );
}
#endif


#ifndef EXTF_delay_ti
void delay_ti(ot_u16 n) {
    delay_us(n*977);
}
#endif


#ifndef EXTF_delay_ms
void delay_ms(ot_u16 n) {
    delay_us(n*1000);
}
#endif


#ifndef EXTF_delay_us
void delay_us(ot_u16 n) {
    usleep(n);
}
#endif

