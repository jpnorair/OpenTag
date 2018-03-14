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

void indicator_set(ot_u8 indicator_id, ot_bool onoff);

void indicator_on(ot_u8 indicator_id);

void indicator_off(ot_u8 indicator_id);

#endif
