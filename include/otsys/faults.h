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
