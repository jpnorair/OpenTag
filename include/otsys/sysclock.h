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
    SPEED_Std   = 0,
    SPEED_Full  = 1,
    SPEED_Flank = 2
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





