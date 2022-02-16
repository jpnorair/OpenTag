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
