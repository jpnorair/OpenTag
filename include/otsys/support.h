/* Copyright 2013-2014 JP Norair
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
  * @file       /otlib/<otsys/support.h>
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R101
  * @date       17 Jan 2014
  * @brief      Some compilation/configuration flags, constants, and macros
  *
  ******************************************************************************
  */


#ifndef __OT_SUPPORT_H
#define __OT_SUPPORT_H


#ifndef __OPENTAG__
#   define __OPENTAG__
#endif


// COMPILERS Officially Supported
#define SIM_GCC     0
#define GCC         1
#define GCC_ARM     1       //legacy
#define TI_C        16
#define CL430       TI_C
#define CCSv4       TI_C   //legacy
#define CCSv5       TI_C   //legacy
#define IAR_V5      32


///@todo "__GCC__" and "__IARV5__" probably have other official definitions.
///      The official definitions should be appropriated ASAP.

// GCC: Preferred Compiler
#if defined(__GCC__) || defined(__GNUC__)
#   define CC_SUPPORT   GCC

// CL430: a GCC-variant used in TI CCS
#elif defined(__TI_COMPILER_VERSION__) || defined(__TI_C__) || defined(__CL430__) || defined(__CL2000__)
#   define CC_SUPPORT   TI_C

// IAR_V5: a proprietary compiler from IAR
#elif (defined(__IARV5__))
#   define CC_SUPPORT   IAR_V5

#else
#   error "A known, supported compiler has not been pre-defined.  Check OTlib/<otsys/support.h>"

#endif





/** Inline Function Support  <BR>
  * ========================================================================<BR>
  * EABI linkers have nice support for inline functions.
  */

#if (CC_SUPPORT == GCC)
#   define OT_INLINE    inline
#   define OT_INLINE_H  inline
#   define OT_WEAK      __attribute__((weak)) 
#   define OT_PACKED    __attribute__((packed))

#elif (CC_SUPPORT == TI_C)
#   ifdef __EABI__
#       define OT_INLINE    inline
#       define OT_INLINE_H  inline
#       define OT_WEAK
#       define OT_PACKED
#   else //COFFABI
#       define OT_INLINE
#       define OT_INLINE_H  __inline
#       define OT_WEAK
#       define OT_PACKED
#   endif

#elif (CC_SUPPORT == IAR_V5)
#   define OT_INLINE    __inline
#   define OT_INLINE_H  __inline
#   define OT_WEAK
#   define OT_PACKED

#endif






#endif 
