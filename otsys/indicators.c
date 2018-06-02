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
  * @file       /otsys/indicators.c
  * @author     JP Norair
  * @version    R100
  * @date       31 Aug 2014
  * @brief      System Indicators Implementation
  * @ingroup    Indicators
  *
  ******************************************************************************
  */

#include <board.h>
#include <otstd.h>
#include <otsys/indicators.h>

#define INDICATOR_MAX   0

#if defined(OT_TRIG1_PIN)
#   define _T1  (1<<0)
#   undef  INDICATOR_MAX
#   define INDICATOR_MAX    1
#else
#   define _T1  0
#endif
#if defined(OT_TRIG2_PIN)
#   define _T2  (1<<1)
#   undef  INDICATOR_MAX
#   define INDICATOR_MAX    2
#else
#   define _T2  0
#endif
#if defined(OT_TRIG3_PIN)
#   define _T3  (1<<2)
#   undef  INDICATOR_MAX
#   define INDICATOR_MAX    3
#else
#   define _T3  0
#endif
#if defined(OT_TRIG4_PIN)
#   define _T4  (1<<3)
#   undef  INDICATOR_MAX
#   define INDICATOR_MAX    4
#else
#   define _T4  0
#endif
#if defined(OT_TRIG5_PIN)
#   define _T5  (1<<4)
#   undef  INDICATOR_MAX
#   define INDICATOR_MAX    5
#else
#   define _T5  0
#endif
#if defined(OT_TRIG6_PIN)
#   define _T6  (1<<5)
#   undef  INDICATOR_MAX
#   define INDICATOR_MAX    6
#else
#   define _T6  0
#endif
#if defined(OT_TRIG7_PIN)
#   define _T7  (1<<6)
#   undef  INDICATOR_MAX
#   define INDICATOR_MAX    7
#else
#   define _T7  0
#endif
#if defined(OT_TRIG8_PIN)
#   define _T8  (1<<7)
#   undef  INDICATOR_MAX
#   define INDICATOR_MAX    8
#else
#   define _T8  0
#endif

#define INDICATOR_MASK   (_T1 | _T2 | _T3 | _T4 | _T5 | _T6 | _T7 | _T8) 



void indicator_on(ot_uint inum) {
    switch (inum) {
#   if defined(OT_TRIG1_PIN)
        case 0: OT_TRIG1_ON();  break;
#   endif
#   if defined(OT_TRIG2_PIN) 
        case 1: OT_TRIG2_ON();  break;
#   endif
#   if defined(OT_TRIG3_PIN)
        case 2: OT_TRIG3_ON();  break;
#   endif
#   if defined(OT_TRIG4_PIN)
        case 3: OT_TRIG4_ON();  break;
#   endif
#   if defined(OT_TRIG5_PIN)
        case 4: OT_TRIG5_ON();  break;
#   endif
#   if defined(OT_TRIG6_PIN)
        case 5: OT_TRIG6_ON();  break;
#   endif
#   if defined(OT_TRIG7_PIN)
        case 6: OT_TRIG7_ON();  break;
#   endif
#   if defined(OT_TRIG8_PIN)
        case 7: OT_TRIG8_ON();  break;
#   endif
    }
}




void indicator_off(ot_uint inum) {
    switch (inum) {
#   if defined(OT_TRIG1_PIN)
        case 0: OT_TRIG1_OFF();  break;
#   endif
#   if defined(OT_TRIG2_PIN) 
        case 1: OT_TRIG2_OFF();  break;
#   endif
#   if defined(OT_TRIG3_PIN)
        case 2: OT_TRIG3_OFF();  break;
#   endif
#   if defined(OT_TRIG4_PIN)
        case 3: OT_TRIG4_OFF();  break;
#   endif
#   if defined(OT_TRIG5_PIN)
        case 4: OT_TRIG5_OFF();  break;
#   endif
#   if defined(OT_TRIG6_PIN)
        case 5: OT_TRIG6_OFF();  break;
#   endif
#   if defined(OT_TRIG7_PIN)
        case 6: OT_TRIG7_OFF();  break;
#   endif
#   if defined(OT_TRIG8_PIN)
        case 7: OT_TRIG8_OFF();  break;
#   endif
    }
}




void indicator_set(ot_uint setmask) {
    ot_int i;
    
    for (i=0; i<INDICATOR_MAX; i++) {
        if ((1<<i) & INDICATOR_MASK) {
            indicator_on(i);
        }
        else {
            indicator_off(i);
        }
    }
}




