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





