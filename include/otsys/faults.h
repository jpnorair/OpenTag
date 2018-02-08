/* Copyright 2018 JP Norair
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
  * @file       /include/otsys/faults.h
  * @author     JP Norair
  * @version    R100
  * @date       31 Jan 2018
  * @brief      System Fault Handling Implementation
  * @defgroup    Faults
  *
  * Faults are saved in a veelite file that must be known during compile-time.
  * The standardized file for tracking faults is ...
  *
  ******************************************************************************
  */

#include <otstd.h>


/// We do some checking to make sure that the system is set up in a way that
/// Fault handling is supported.
#if OT_FEATURE(FAULTS)
#   if (OT_FEATURE(VLACTIONS) != ENABLED)
#       error "System-level fault handling requires VL Actions to be enabled."
#   endif

#endif


typedef enum {
    FAULTTYPE_virtual       = 0,
    FAULTTYPE_recoverable   = 1,
    FAULTTYPE_reboot        = 2,
    FAULTTYPE_terminal      = 3,
    FAULTTYPE_MAX
} ot_faulttype;


typedef enum {
    FAULTGROUP_unknown  = 0,
    FAULTGROUP_user     = 1,
    FAULTGROUP_otlib    = 2,
    FAULTGROUP_m2       = 3,
    FAULTGROUP_otsys    = 4,
    FAULTGROUP_io       = 5,
    FAULTGROUP_platform = 6,
    FAULTGROUP_MAX
} ot_faultgroup;



/*

ot_int faults_init(vlACTION* onfault, ) {
#if OT_FEATURE(FAULTS)
    

#else
    return 0;
#endif
}
*/
