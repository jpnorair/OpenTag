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
  * @file       /platform/posix_c/otlib_delay.c
  * @author     JP Norair
  * @version    R100
  * @date       26 Oct 2017
  * @brief      Delay OTlib Functions for POSIX
  * @ingroup    Delay
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>
#include <otlib/delay.h>

#include <unistd.h>


#ifndef EXTF_delay_sti
void delay_sti(ot_u16 sti) {
    delay_us( sti*31 );
}
#endif


#ifndef EXTF_delay_ti
void delay_ti(ot_u16 n) {
    delay_us(n*977);
}
#endif


#ifndef EXTF_delay_ms
void delay_ms(ot_u16 n) {
    delay_us(n*1000);
}
#endif


#ifndef EXTF_delay_us
void delay_us(ot_u16 n) {
    usleep(n);
}
#endif

