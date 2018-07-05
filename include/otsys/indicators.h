/* Copyright 2014 JP Norair
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
