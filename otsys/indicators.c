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

#ifdef OT_TRIG_COUNT
#   define _INDICATORS  OT_TRIG_COUNT
#else
#   define _INDICATORS  ( defined(OT_TRIG1_PIN) \
                        + defined(OT_TRIG2_PIN) \
                        + defined(OT_TRIG3_PIN) \
                        + defined(OT_TRIG4_PIN) \
                        + defined(OT_TRIG5_PIN) \
                        + defined(OT_TRIG6_PIN) \
                        + defined(OT_TRIG7_PIN) \
                        + defined(OT_TRIG8_PIN) \
                        )
#endif


void indicator_set(ot_u8 indicator_id, ot_bool onoff) {
    if (onoff)  indicator_on(indicator_id);
    else        indicator_off(indicator_id);
}



void indicator_on(ot_u8 indicator_id) {
    switch (indicator_id) {
#   ifdef OT_TRIG1_PIN
        case 1: OT_TRIG1_ON(); break;
#   endif
#   ifdef OT_TRIG2_PIN
        case 2: OT_TRIG2_ON(); break;
#   endif
#   ifdef OT_TRIG3_PIN
        case 3: OT_TRIG3_ON(); break;
#   endif
#   ifdef OT_TRIG4_PIN
        case 4: OT_TRIG4_ON(); break;
#   endif
#   ifdef OT_TRIG5_PIN
        case 5: OT_TRIG5_ON(); break;
#   endif
#   ifdef OT_TRIG6_PIN
        case 6: OT_TRIG6_ON(); break;
#   endif
#   ifdef OT_TRIG7_PIN
        case 7: OT_TRIG7_ON(); break;
#   endif
#   ifdef OT_TRIG8_PIN
        case 8: OT_TRIG8_ON(); break;
#   endif
    }
}



void indicator_off(ot_u8 indicator_id) {
    switch (indicator_id) {
#   ifdef OT_TRIG1_PIN
        case 1: OT_TRIG1_OFF(); break;
#   endif
#   ifdef OT_TRIG2_PIN
        case 2: OT_TRIG2_OFF(); break;
#   endif
#   ifdef OT_TRIG3_PIN
        case 3: OT_TRIG3_OFF(); break;
#   endif
#   ifdef OT_TRIG4_PIN
        case 4: OT_TRIG4_OFF(); break;
#   endif
#   ifdef OT_TRIG5_PIN
        case 5: OT_TRIG5_OFF(); break;
#   endif
#   ifdef OT_TRIG6_PIN
        case 6: OT_TRIG6_OFF(); break;
#   endif
#   ifdef OT_TRIG7_PIN
        case 7: OT_TRIG7_OFF(); break;
#   endif
#   ifdef OT_TRIG8_PIN
        case 8: OT_TRIG8_OFF(); break;
#   endif
    }
}



#undef _INDICATORS


