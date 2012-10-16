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
  * @file       /otlibext/applets_std/sys_sig_powerdown.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2012
  * @brief      Standard Sleep Routine
  *
  * Standard Sleep Routine: in the platform_xxx.h file, there should be a few
  * a few macros that control the type of sleep/power-down-mode that will be 
  * entered during different states.  The normal ones are:
  * <LI> MCU_SLEEP(): Fast wakeup, core-off, RAM alive, periperals alive </LI>
  * <LI> MCU_SLEEP_WHILE_RF(): Best sleep available during RF process </LI>
  * <LI> MCU_SLEEP_WHILE_IO(): Best sleep available during wireline IO process </LI>
  * <LI> MCU_STOP(): Best sleep available where RAM, Timer interrupt are alive </LI>
  * <LI> MCU_STANDBY(): Deep sleep, basically everything powered down </LI>
  *
  * You can write your own applet with optimized sleeping behavior for your
  * desired application.  This one should serve most apps, though.
  *
  * @note MCU_STOP() typically disables the I/O ring.  MCU_SLEEP_WHILE_RF() may 
  * also disable the I/O ring on certain platforms (particularly SoCs).  When
  * the I/O ring is disabled, any LEDs, indicators -- whatever -- will not work.
  * 
  */

#include "OTAPI.h"
#include "OT_platform.h"


#ifdef EXTF_sys_sig_powerdown
void sys_sig_powerdown(ot_int code) {
/// Design Notes: This is the default sleeper routine that is also implemented
/// in the kernel itself (if you are not linking here).  For systems that are
/// powered, you can just call MCU_SLEEP() and forget the rest of this.
	static const ot_sub powerdown[] = { &MCU_SLEEP,
			                            &MCU_SLEEP_WHILE_IO,
			                            &MCU_SLEEP_WHILE_RF,
			                            &MCU_STOP
	                                   };
	powerdown[code&3]();
}
#endif
