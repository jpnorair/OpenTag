/* Copyright 2010-2011 JP Norair
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
  * @file       /otlib/alp_dashforth.c
  * @author     JP Norair
  * @version    R100
  * @date       1 July 2011
  * @brief      ALP to DASHForth applet extractor
  * @ingroup    ALP
  *
  * Placeholder for future dashforth module ALP wrapper
  * 
  ******************************************************************************
  */


#include <otlib/alp.h>

#if (   (OT_FEATURE(SERVER) == ENABLED) \
     && (OT_FEATURE(ALP) == ENABLED) \
     && (OT_FEATURE(DASHFORTH) == ENABLED) )

#include "dashforth.h"

// DASHForth APL processor would go here.  It does not have to do much, just
// push the applet to the VM.  I'm not sure if the directive cmd has any usage
// for DASHForth aside from the response bit.

// DASHForth manages its own user system, pusuant to the auth module, but
// independently configurable.

#endif


