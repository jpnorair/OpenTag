/* Copyright 2009-2011 JP Norair
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
  * @file       /OTlib/OT_support.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       1 Jan 2011
  * @brief      Indexed List of Supported Applications and Platforms, which are
  *             included into other configuration files.
  *
  ******************************************************************************
  */


#ifndef __OT_SUPPORT_H
#define __OT_SUPPORT_H


// COMPILERS Officially Supported
#define SIM_GCC     0
#define GCC         1
#define GCC_ARM     1       //legacy
#define CL430       16
#define CCSv4       CL430  //legacy
#define CCSv5       CL430  //legacy
#define IAR_V5      32


///@todo "__GCC__" and "__IARV5__" probably have other official definitions.
///      The official definitions should be appropriated ASAP.

// GCC: Preferred Compiler
#if defined(__GCC__) || defined(__GNUC__)
#   define CC_SUPPORT   GCC

// CL430: a GCC-variant used in TI CCS
#elif (defined(__CCSv4__) || defined(__CCSv5__) || defined(__CL430__))
#   define CC_SUPPORT   CL430

// IAR_V5: a proprietary compiler from IAR
#elif (defined(__IARV5__))
#   define CC_SUPPORT   IAR_V5

#else
#   error "A known, supported compiler has not been pre-defined.  Check OTlib/OT_support.h"

#endif





/** Inline Function Support  <BR>
  * ========================================================================<BR>
  * EABI linkers have nice support for inline functions.
  */

#if (CC_SUPPORT == GCC)
#   define OT_INLINE    inline
#   define OT_INLINE_H  inline

#elif (CC_SUPPORT == CL430)
#   ifdef __EABI__
#       define OT_INLINE    inline
#       define OT_INLINE_H  inline
#   else
#       define OT_INLINE
#       define OT_INLINE_H  __inline
#   endif

#elif (CC_SUPPORT == IAR_V5)
#   define OT_INLINE    __inline
#   define OT_INLINE_H  __inline

#endif






#endif 
