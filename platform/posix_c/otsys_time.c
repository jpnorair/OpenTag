/* Copyright 2014 JP Norair
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
  * @file       /otsys/time.c
  * @author     JP Norair
  * @version    R100
  * @date       31 Aug 2014
  * @brief      System Time Implementation
  * @ingroup    Time
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otsys/time.h>
#include <platform/config.h>
#include <platform/timers.h>

#if OT_FEATURE(TIME)

#include <time.h>
#include <math.h>

#if defined(OT_GPTIM_SHIFT)
#   define _SHIFT           OT_GPTIM_SHIFT
#   define _UPPER_SHIFT     (22-OT_GPTIM_SHIFT)
#   define _LOWER_SHIFT     (10+OT_GPTIM_SHIFT)
#else
#   warning "OT_GPTIM_SHIFT not defined, using clock = 1tick."
#   define _SHIFT           0
#   define _UPPER_SHIFT     (22)
#   define _LOWER_SHIFT     (10)
#endif


extern ot_time  time_sys;
extern ot_time  time_start;



/** Driver functions 
  * The following functions are sometimes implemented in the platform driver.
  * They do the low-level work for the system time module.
  */


void time_load_now(ot_time* now) {   
    struct timespec tspec;
    ot_u32          s;
    ot_u32          ti;
    
    if (now != NULL) {
        clock_gettime(CLOCK_REALTIME, &tspec);
        s   = (ot_u32)tspec.tv_sec;
        ti  = (ot_u32)round(tspec.tv_nsec / 976562.5);
        if (ti > 1023) {
            ti = 0;
            s++;
        }
        
        now->upper      = (s >> _UPPER_SHIFT);
        now->clocks     = (s << _LOWER_SHIFT);
        now->clocks    |= ti;
    }
}


#endif

