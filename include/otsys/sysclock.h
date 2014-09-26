/*  Copyright 2010-2014, JP Norair
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
  * @file       /include/otsys/sysclock.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Aug 2014
  * @brief      System CPU Clock manipulation
  * @ingroup    System
  * 
  ******************************************************************************
  */


#ifndef __OTSYS_SYSCLOCK_H
#define __OTSYS_SYSCLOCK_H

#include <otstd.h>
#include <otsys/syskern.h>

typedef enum {
    SPEED_Std   = -1,
    SPEED_Full  = 0,
    SPEED_Flank = 1
} SPEED_enum;

/** @brief  Request a system-speed.  Returns a handle.
  * @param  Speed   (SPEED_enum) 
  * @retval ot_int  a speed handle, or -1 if an inappropriate speed was entered
  * @ingroup System
  * @sa sysclock_dismiss()
  *
  * The way this works is that you request a speed like SPEED_Full or 
  * SPEED_Flank.  There is no point to request SPEED_Std, because that happens
  * automatically when nothing else requests a higher speed.
  *
  * If the system is already running above the requested speed, or at the 
  * requested speed, nothing will change.  However, the system will mark this
  * request and return a handle.  Until you call sysclock_dismiss(), supplying
  * this handle as the input, the System will guarantee that it will never
  * drop below your requested speed.
  */
ot_int sysclock_request(SPEED_enum Speed);


/** @brief  Dismiss a requested a system-speed.  Returns an error code
  * @param  handle  (ot_int) a speed request handle
  * @retval ot_int  an error code: -1 if a bad handle, 0 if no error
  * @ingroup System
  * @sa sysclock_request()
  *
  * This function works together with sysclock_request().  Once a requested
  * speed is dismissed, the system will check its other requests and go to the
  * lowest allowable speed.
  */
ot_int sysclock_dismiss(ot_int handle);

#endif





