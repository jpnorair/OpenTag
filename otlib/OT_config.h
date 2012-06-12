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
  * @file       /OTlib/OT_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       22 August 2011
  * @brief      OpenTag Master Configuration Header that aggregates external 
  *             Platform & App headers.
  *
  * If the make process included some version of configuration script, it would
  * act on this file.  Therefore, this is the only file that NEEDS to be changed
  * from build-to-build, because the default setting might not match your 
  * platform or application.
  *
  ******************************************************************************
  */

#ifndef __OT_CONFIG_H
#define __OT_CONFIG_H

#include "OT_support.h"
#include "app_config.h"
#include "extf_config.h"


/// Intra-Word Addressing: 
/// Using these addressing constants in the extended type unions ensures that
/// the code is portable across little and big endian architectures.

#   ifdef __BIG_ENDIAN__
#       define UPPER    0
#       define LOWER    1
#       define B0       3
#       define B1       2
#       define B2       1
#       define B3       0
#   else
#       define UPPER    1
#       define LOWER    0
#       define B0       0
#       define B1       1
#       define B2       2
#       define B3       3
#   endif



#endif 
