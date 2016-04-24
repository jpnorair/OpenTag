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
  * @file       /otlib/trigger.c
  * @author     JP Norair
  * @version    R100
  * @date       27 Aug 2014
  * @brief      Trigger Implementation
  * @ingroup    Triggers
  * 
  * The trigger implementation is done in the board support header as a macro
  * and wrapped here as C functions.  OpenTag guarantees two triggers.
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <board.h>
#include <otlib/triggers.h>




/** Platform Debug Triggers <BR>
  * ========================================================================<BR>
  * Triggers are optional pins mostly used for debugging.  Sometimes they are
  * hooked up to LEDs, logic probes, etc.  The platform module requires that at
  * two triggers exist (if triggers are defined at all).  More triggers can be
  * defined in the application code.
  */
#ifdef OT_TRIG1_PORT
void trigger_TR1_high()      { OT_TRIG1_ON(); }
void trigger_TR1_low()       { OT_TRIG1_OFF(); }
void trigger_TR1_toggle()    { OT_TRIG1_TOG(); }
#else
void trigger_TR1_high()      { }
void trigger_TR1_low()       { }
void trigger_TR1_toggle()    { }
#endif

#ifdef OT_TRIG2_PORT
void trigger_TR2_high()      { OT_TRIG2_ON(); }
void trigger_TR2_low()       { OT_TRIG2_OFF(); }
void trigger_TR2_toggle()    { OT_TRIG2_TOG(); }
#else
void trigger_TR2_high()      { }
void trigger_TR2_low()       { }
void trigger_TR2_toggle()    { }
#endif





