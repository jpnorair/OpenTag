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
  *
  */
/**
  * @file       /OTlib/external.h
  * @author     JP Norair
  * @version    V1.0
  * @date       10 Sept 2011
  * @brief      External Data & Event Wrappers
  * @defgroup   External (External Module)
  * @ingroup    External
  *
  * The External Module is a tie-in so that your sensor management routines can
  * interact with DASH7/OpenTag features.  Namely, this is the file system, but
  * there are also tie-ins to setting MAC variables.
  ******************************************************************************
  */


#ifndef __EXTERNAL_H
#define __EXTERNAL_H

#include "OT_types.h"


/** @brief  Initialize External module data elements (objects)
  * @param  None
  * @retval None
  * @ingroup External
  * @sa sys_init()
  * 
  * Called by sys_init(), which itself is called by platform_OT_init().
  */
void ext_init();


/** @brief  Returns Mode 2 Application Flags (part of Mode 2 spec)
  * @param  None
  * @retval ot_u8       The current app flag values, which are in lower 4 bits
  * @ingroup External
  *
  * The App Flags are:
  * b3: System Fault
  * b2: Low Battery Alarm
  * b3: Sensor Alarm
  * b4: External Event Alarm
  */
ot_u8 ext_get_m2appflags();


#endif





