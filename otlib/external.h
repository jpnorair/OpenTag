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
  *
  */
/**
  * @file       /otlib/external.h
  * @author     JP Norair
  * @version    V1.0
  * @date       10 Oct 2012
  * @brief      External Data & Event Wrappers
  * @defgroup   External (External Module)
  * @ingroup    External
  *
  * OpenTag assumes that you have at least one task that runs application code
  * of some type.  This is called the EXTERNAL TASK.  You could have more than
  * one of your own tasks, but the EXTERNAL TASK is the one that the system
  * module will automatically initiate at startup, and which gets to utilize 
  * the DASH7 DLL application flags.
  *
  ******************************************************************************
  */


#ifndef __EXTERNAL_H
#define __EXTERNAL_H

#include "OTAPI.h"


/** @brief  Initialize External module data elements (objects)
  * @param  None
  * @retval None
  * @ingroup External
  * @sa sys_init()
  * 
  * Called by sys_init(), which itself is called by platform_OT_init().
  *
  * To implement it, the BEST PRACTICE is to define EXTF_ext_init in your 
  * extf_config.h file, and then implement your own void ext_init() function 
  * however you want.
  */
void ext_init();


/** @brief  Returns Mode 2 Application Flags (part of Mode 2 spec)
  * @param  None
  * @retval ot_u8       The current app flag values, which are in lower 4 bits
  * @ingroup External
  *
  * This is the default implementation with no app flag support.  Unlike comm
  * protocols that religiously follow OSI, DASH7 includes a 4 bit space in the
  * Data Link Layer Header that you can load with 4 bits of your own data, 
  * therefore allowing some application data to be visible to lower-layers for
  * whatever purpose you have.
  *
  * To use it, the BEST PRACTICE is to define EXTF_ext_get_m2appflags in your
  * extf_config.h file, and then implement your own ot_u8 ext_get_m2appflags()
  * function however you want.
  */
ot_u8 ext_get_m2appflags();




/** @brief  External Task runtime function
  * @param  task        (ot_task) Task marker containing task parameters
  * @retval None
  * @ingroup External
  *
  * This function will get called whenever the external task is activated by
  * the kernel.  Like other external module functions, you should implement
  * your own version of this function in your own application code (i.e. not
  * inside external.c).  Here is how this can work:
  * <LI> Enable OT_FEATURE_SYSTASK_CALLBACKS in your app_config.h file, and
  *      assign your task to the external task: sys.task_EXT.call       </LI>
  * <LI> Define EXTF_ext_systask in your extf_config.h file, and write your own
  *      function called "void ext_systask(task_marker* task)" that implements
  *      your task.  </LI>
  * <LI> If you do both of the above, the kernel SHOULD assign your ext_systask
  *      function as the init default callback.  The Native Kernel will indeed
  *      do this.  </LI>
  * <LI> If you Enable OT_SYSTASK_CALLBACKS and do NOT define EXTF_ext_systask,
  *      this blank function will be assigned as the default external task. </LI>
  */
void ext_systask(ot_task task);

#endif





