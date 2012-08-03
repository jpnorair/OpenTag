/* Copyright 2010-2011 JP Norair
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  * http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  */
/**
  * @file       /Apps/.../build_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       22 August 2011
  * @brief      Most basic list of constants needed to configure build
  *
  *
  ******************************************************************************
  */

#ifndef __BUILD_CONFIG_H
#define __BUILD_CONFIG_H

#include "OT_support.h"


// Comment-out for release build
#define DEBUG_ON


// Conditionally select one of the supported POSIX platforms
#if defined (__x86_64__)
#   define PLATFORM_TYPE    POSIX_x86_64

#elif defined (__i386__)
#   define PLATFORM_TYPE    POSIX_i386

#elif defined (__arm__)
#   define PLATFORM_TYPE    POSIX_ARMv7

#else
#   error "Platform is not officially supported for Testbed Protocol"

#endif


// Select The TestbedProtocol app
#define APP_TYPE            TestbedProtocol






#endif 
