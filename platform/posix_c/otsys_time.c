/*  Copyright 2008-2022, JP Norair
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted provided that the following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright 
  *    notice, this list of conditions and the following disclaimer in the 
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
  * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
  * POSSIBILITY OF SUCH DAMAGE.
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

