/* Copyright 2013-14 JP Norair
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
  * @file       /include/otlib/triggers.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Mar 2014
  * @brief      Triggers: basically just output pins that have official OTlib support
  * @defgroup   Triggers (Triggers Module)
  * @ingroup    Triggers
  *
  * These functions must be implemented in the platform driver section
  * 
  ******************************************************************************
  */


#ifndef __OTLIB_TRIGGERS_H
#define __OTLIB_TRIGGERS_H



/** Macro function:
  * PIN can be a number, usually TR1 or TR2
  * OP can be high, low, or toggle
  */
#define trigger(PIN, OP)    trigger_##PIN_##OP()


void trigger_1_high();
void trigger_1_low();
void trigger_1_toggle();
void trigger_2_high();
void trigger_2_low();
void trigger_2_toggle();





#endif
