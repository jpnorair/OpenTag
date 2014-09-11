/* Copyright 2010-2014 JP Norair
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
  * @file       /platform/capi.h
  * @author     JP Norair
  * @version    R101
  * @date       26 Aug 2014
  * @brief      Platform CAPI function
  *
  * The C API is a wrapper to certain internal platform functions.
  * 
  ******************************************************************************
  */


#ifndef __PLATFORM_CAPI_H
#define __PLATFORM_CAPI_H

#include <otstd.h>


/** Non-mapped API Functions <BR>
  * ========================================================================<BR>
  * These functions do not have equivalents in ALP.
  */

/** @brief Power-on initialization.  Implemented at the platform level.
  * @param  none
  * @retval none
  * 
  */
void otapi_poweron();

/** @brief Data-safe Power-down.  Implemented at the platform level.
  * @param  none
  * @retval none
  * 
  */
void otapi_poweroff();


/** @brief OpenTag system initialization.  Implemented at the platform level.
  * @param  none
  * @retval none
  * 
  */
void otapi_init();


/** @brief Enters the Kernel.  Implemented at the platform level.
  * @param  none
  * @retval none
  * 
  *
  * This function should only be used in timer ISRs that invoke the kernel.
  */
void otapi_exec();


/** @brief Pre-empts the OpenTag kernel.  Implemented at the platform level.
  * @param  none
  * @retval none
  * 
  */
void otapi_preempt();


/** @brief Pauses the OpenTag kernel.  Implemented in the platform level.
  * @param  none
  * @retval none
  * 
  */
void otapi_pause();




#endif


