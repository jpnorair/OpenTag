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
  * @file       /otsys/indicators.h
  * @author     JP Norair
  * @version    R100
  * @date       31 Aug 2014
  * @brief      System Indicator Interface
  * @defgroup   Indicators
  * @ingroup    Indicators
  *  
  ******************************************************************************
  */

#include <otstd.h>


#ifndef __SYS_INDICATORS_H
#define __SYS_INDICATORS_H

#if defined(OT_TRIG_COUNT)
#   define INDICATORS_COUNT OT_TRIG_COUNT
#elif defined(BOARD_PARAM_TRIGS)
#   define INDICATORS_COUNT BOARD_PARAM_TRIGS
#else
#   define INDICATORS_COUNT ( defined(OT_TRIG1_PIN) \
                        + defined(OT_TRIG2_PIN) \
                        + defined(OT_TRIG3_PIN) \
                        + defined(OT_TRIG4_PIN) \
                        + defined(OT_TRIG5_PIN) \
                        + defined(OT_TRIG6_PIN) \
                        + defined(OT_TRIG7_PIN) \
                        + defined(OT_TRIG8_PIN) \
                        )
#endif


typedef enum {
#ifdef OT_TRIG1_PIN
    INDICATOR_1 = 0,
#endif
#ifdef OT_TRIG2_PIN
    INDICATOR_2,
#endif
#ifdef OT_TRIG3_PIN
    INDICATOR_3,
#endif
#ifdef OT_TRIG4_PIN
    INDICATOR_4,
#endif
#ifdef OT_TRIG5_PIN
    INDICATOR_5,
#endif
#ifdef OT_TRIG6_PIN
    INDICATOR_6,
#endif
#ifdef OT_TRIG7_PIN
    INDICATOR_7,
#endif
#ifdef OT_TRIG8_PIN
    INDICATOR_8,
#endif
    INDICATOR_MAX
} INDICATOR_enum;



void indicator_on(ot_uint inum);
void indicator_off(ot_uint inum);

void indicator_set(ot_uint setmask);




#endif
