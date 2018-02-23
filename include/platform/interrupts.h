/* Copyright 2013-14 JP Norair
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
  * @file       /include/platform/interrupts.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Mar 2014
  * @brief      Platform Interrupt functions
  * @ingroup    Platform
  *
  * 
  ******************************************************************************
  */


#ifndef __PLATFORM_INTERRUPTS_H
#define __PLATFORM_INTERRUPTS_H

#include <otsys/types.h>

//#include <otsys/config.h>
//#include <app/build_config.h>



/** @brief Routine for putting maskable interrupts on hold
  * @param None
  * @retval None
  * @ingroup Platform
  */
void platform_disable_interrupts(void);


/** @brief Routine for taking maskable interrupts off of hold
  * @param None
  * @retval None
  * @ingroup Platform
  */
void platform_enable_interrupts(void);



#endif
