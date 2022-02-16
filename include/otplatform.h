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
  * @file       /include/otplatform.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Mar 2014
  * @brief      Platform Library functions
  * @defgroup   Platform (Platform Module)
  * @ingroup    Platform
  *
  * OpenTag requires the implementation of certain platform functions.  Others
  * are optional (and they are marked as such).  When porting to a new platform,
  * these functions need to be implemented, as they are typically not hardware
  * agnostic.
  *
  ******************************************************************************
  */


#ifndef __OTPLATFORM_H
#define __OTPLATFORM_H

#include <otsys/types.h>
#include <otsys/config.h>
#include <platform/config.h>
#include <app/build_config.h>


/** Platform D <BR>
  * ========================================================================<BR>
  */

#if (defined(__DEBUG__) || defined(__PROTO__))
#   define __DEBUG_ERRCODE_EVAL(EVAL) do { \
		platform.error_module = 0;	\
		platform.error_code EVAL;	\
	} while(0)
		
#	define __DEBUG_ERRCODE_SET(FCODE, PCODE) do { \
		platform.error_module 	= (ot_u16)(FCODE & 0xFFFF); \
		platform.error_code		= (ot_int)(PCODE & 0xFFFF); \
	} while(0)

#else
#   define __DEBUG_ERRCODE_EVAL(EVAL);
#   define __DEBUG_ERRCODE_SET(FCODE, PCODE);

#endif



/** Platform Headers <BR>
  * ========================================================================<BR>
  */
#include <platform/capi.h>
#include <platform/data.h>
#include <platform/init.h>
#include <platform/interrupts.h>
//#include <platform/memcpy.h>
#include <platform/speed.h>
#include <platform/tasking.h>
#include <platform/timers.h>




/** Trailing includes & defines <BR>
  * ========================================================================<BR>
  * These must trail the function definitions above, so that the board config
  * header can use inline functions calling the functions defined above.
  */

#include <app/board_config.h>


/// Errors for missing critical parameters
#ifndef MCU_TYPE_PTRINT
#   error "No value for MCU_TYPE_PTRINT (should be in include/platform/hw/xxx.h)"
#endif
#ifndef MCU_TYPE_PTRUINT
#   error "No value for MCU_TYPE_PTRUINT (should be in include/platform/hw/xxx.h)"
#endif
#ifndef MCU_PARAM_ERRPTR
#   error "No value for MCU_TYPE_ERRPTR (should be in include/platform/hw/xxx.h)"
#endif


#endif
