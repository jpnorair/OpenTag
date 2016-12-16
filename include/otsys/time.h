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
  * @file       /otsys/time.h
  * @author     JP Norair
  * @version    R100
  * @date       31 Aug 2014
  * @brief      System Time Interface
  * @defgroup   Time
  * @ingroup    Time
  *  
  ******************************************************************************
  */

#include <otstd.h>


#ifndef __SYS_TIME_H
#define __SYS_TIME_H


typedef struct {
    ot_u32 upper;
    ot_u32 clocks;
} ot_time;

void time_init_utc(ot_u32 utc);

void time_set_utc(ot_u32 utc);

void time_add(ot_u32 clocks);

//void time_add_ti(ot_u32 ticks);

ot_u32 time_get_utc(void);

ot_u32 time_uptime_secs(void);

ot_u32 time_uptime(void);


#endif
