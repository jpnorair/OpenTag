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


///@note possible 48 bit time implementation.  Just a random thought.
//typedef struct OT_PACKED {
//    ot_u32  secs;
//    ot_u16  sti;
//} ot_time;
//
//typedef union {
//    ot_time ot;
//    ot_u16  ushort[3];
//    ot_u8   ubyte[6];
//} ot_time_union;


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
