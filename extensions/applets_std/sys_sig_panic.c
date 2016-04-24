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
  * @file       /otlibext/applets_std/sys_sig_panic.c
  * @author     JP Norair
  * @version    R100
  * @date       29 Aug 2013
  * @brief      Standard Panic Routine
  *
  * Kernel panic.  You could have it print something out, or blink LEDs, or
  * whatever.  The system function sys_panic() does the important shutdown
  * routines, so this callback is just for reporting purposes
  */

#include <otstd.h>
#include <otsys/syskern.h>
#include <otlib/logger.h>


#ifdef EXTF_sys_sig_panic
void sys_sig_panic(ot_int code) {
#   if (OT_FEATURE(MPIPE))
    logger_code(3, (ot_u8*)"WTF", (ot_u16)code);
#   endif
}
#endif
