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
  * @file       /otlib/config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R100
  * @date       19 August 2013
  * @brief      OpenTag Master Configuration Header that aggregates external 
  *             Platform & App headers.
  *
  * If the make process included some version of configuration script, it would
  * act on this file.  Therefore, this is the only file that NEEDS to be changed
  * from build-to-build, because the default setting might not match your 
  * platform or application.
  *
  ******************************************************************************
  */

#ifndef __OT_CONFIG_H
#define __OT_CONFIG_H

#include <otsys/support.h>
#include <app/app_config.h>
#include <app/extf_config.h>

/// Intra-Word Addressing: 
/// Using these addressing constants in the extended type unions ensures that
/// the code is portable across little and big endian architectures.

#   ifdef __BIG_ENDIAN__
#       define UPPER    0
#       define LOWER    1
#       define B0       3
#       define B1       2
#       define B2       1
#       define B3       0
#       define _B0      3
#       define _B1      2
#       define _B2      1
#       define _B3      0
#   else
#       define UPPER    1
#       define LOWER    0
#       define B0       0
#       define B1       1
#       define B2       2
#       define B3       3
#       define _B0      0
#       define _B1      1
#       define _B2      2
#       define _B3      3
#   endif



#endif 
