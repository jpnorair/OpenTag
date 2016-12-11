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
