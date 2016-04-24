/*  Copyright 2010-2014, JP Norair
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
  * @file       /include/otsys/sysext.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Aug 2014
  * @brief      External Task Interface
  * @ingroup    System-Kernel
  *
  * Depending on the kernel design, it may be possible to add many tasks to the
  * System-Kernel, but OpenTag requires that the kernel can have least one task 
  * added.  This is an interface to that task.
  * 
  * The "External" task will get built into to the kernel as a kernel-task,
  * meaning that it runs in the kernel's thread/context and has all other 
  * attributes that kernel tasks have.
  * 
  ******************************************************************************
  */


#ifndef __OTSYS_SYSEXT_H
#define __OTSYS_SYSEXT_H

#include <otstd.h>
#include <otsys/syskern.h>

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





