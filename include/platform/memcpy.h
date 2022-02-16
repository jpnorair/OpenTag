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
  * @file       /include/platform/memcpy.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Mar 2014
  * @brief      Memcpy, Memset interfaces from the Platform
  * @ingroup    Platform
  *
  * The user should include <otlib/memcpy.h> which will selectively use the
  * platform implementation or the string.h implementation based on the build
  * parameters.
  *
  ******************************************************************************
  */


#ifndef __PLATFORM_MEMCPY_H
#define __PLATFORM_MEMCPY_H

#include <otsys/types.h>
#include <otsys/config.h>
#include <app/build_config.h>


/** @brief platform-specific memcpy, in some cases wraps to OS-level memcpy
  * @param  dest        (void*) destination memory address
  * @param  src         (void*) source memory address
  * @param  length      (ot_uint) number of bytes to transfer/copy
  * @retval None
  * @ingroup Platform
  * @sa ot_memcpy2()
  * @sa ot_memcpy4()
  * @sa ot_memset()
  *
  * ot_memcpy() is the generic implementation of memcpy, which must
  * handle byte-aligned memory copies.
  *
  * ot_memcpy2() and ot_memcpy4() are two and four byte aligned
  * variants of ot_memcpy()
  */
void ot_memcpy(ot_u8* dst, ot_u8* src, ot_uint length);
void ot_memcpy2(ot_u16* dst, ot_u16* src, ot_uint length);
void ot_memcpy4(ot_u32* dst, ot_u32* src, ot_uint length);



/** @brief platform-specific memset, in some cases wraps to OS-level memset
  * @param  dest        (void*) destination memory address
  * @param  value       (ot_u8) byte to put into memory (repeatedly)
  * @param  length      (ot_uint) number of bytes to set
  * @retval None
  * @ingroup Platform
  * @sa memcpy()
  */
void ot_memset(ot_u8* dst, ot_u8 value, ot_uint length);
void ot_memset2(ot_u16* dst, ot_u16 value, ot_uint length);
void ot_memset4(ot_u32* dst, ot_u32 value, ot_uint length);



#endif
