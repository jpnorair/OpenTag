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
  * @file       /include/otlib/memcpy.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Mar 2014
  * @brief      Memcpy, Memset interfaces
  * @defgroup   Memcpy
  * @ingroup    Memcpy
  *
  * OpenTag defines a group of memcpy() and memset() functions.  These must be
  * implemented in the platform driver section.  Quite often they use DMAs or
  * other such optimizations.
  *
  ******************************************************************************
  */


#ifndef __OTLIB_MEMCPY_H
#define __OTLIB_MEMCPY_H

#include <otstd.h>
#include <app/build_config.h>


//#if (OS_FEATURE(MEMCPY) == ENABLED)
#if (OS_FEATURE_MEMCPY == ENABLED)
#   include <string.h>
#   define memcpy_2(DST, SRC, LEN)   memcpy(DST, SRC, (LEN)<<1)
#   define memcpy_4(DST, SRC, LEN)   memcpy(DST, SRC, (LEN)<<2)
#   define memset_2(DST, VAL, LEN)   memset(DST, VAL, (LEN)<<1)
#   define memset_4(DST, VAL, LEN)   memset(DST, VAL, (LEN)<<2)
#else
//#   include <platform/memcpy.h>
#   define memcpy       ot_memcpy
#   define memcpy_2     ot_memcpy_2
#   define memcpy_4     ot_memcpy_4
#   define memset       ot_memset
#   define memset_2     ot_memset_2
#   define memset_4     ot_memset_4
#endif


/** @brief platform-specific memcpy, in some cases wraps to OS-level memcpy
  * @param  dest        (void*) destination memory address
  * @param  src         (void*) source memory address
  * @param  length      (ot_uint) number of bytes to transfer/copy
  * @retval None
  * @ingroup Memcpy
  * @sa ot_memcpy_2()
  * @sa ot_memcpy_4()
  * @sa ot_memset()
  *
  * ot_memcpy() is the generic implementation of memcpy, which must
  * handle byte-aligned memory copies.
  *
  * ot_memcpy_2() and ot_memcpy_4() are two and four byte aligned
  * variants of ot_memcpy()
  */
///@todo consider changing to the POSIX:
///      void* memcpy(void *restrict dst, const void *restrict src, size_t n);
void ot_memcpy(void* dst, void* src, ot_uint length);
void ot_memcpy_2(void* dst, void* src, ot_uint length);
void ot_memcpy_4(void* dst, void* src, ot_uint length);



/** @brief platform-specific memset, in some cases wraps to OS-level memset
  * @param  dest        (void*) destination memory address
  * @param  value       (ot_u8) byte to put into memory (repeatedly)
  * @param  length      (ot_uint) number of bytes to set
  * @retval None
  * @ingroup Memcpy
  * @sa memcpy()
  */
void ot_memset(void* dst, ot_u8 value, ot_uint length);
void ot_memset_2(void* dst, ot_u16 value, ot_uint length);
void ot_memset_4(void* dst, ot_u32 value, ot_uint length);



#endif
