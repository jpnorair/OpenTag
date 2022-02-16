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
  * @file       /platform/stm32f0xx/otlib_rand.c
  * @author     JP Norair
  * @version    R100
  * @date       27 Aug 2014
  * @brief      Random Number generators for STM32F0
  * @ingroup    Rand
  *
  ******************************************************************************
  */


#include <otstd.h>
#include <otplatform.h>
#include <otlib/rand.h>


/** Platform Random Number Generation Routines <BR>
  * ========================================================================<BR>
  * The platform must be able to compute a strong random number (via function
  * platform_rand()) and a "pseudo" random number (via rand_prn8()).
  */

void rand_stream(ot_u8* rand_out, ot_int bytes_out) {
/// Random Number generator works the best when using "slow" channels (channels
/// other than 4,5,22,23,24) and driving them as fast as possible.
///@todo Change this implementation to use the "injected conversion" method.

#ifndef OT_GWNADC_BITS
#   define OT_GWNADC_BITS 8
#endif
#ifdef OT_GWNADC_PINNUM
#   undef OT_GWNADC_PIN
#else
#   warn "OT_GWNADC_PINNUM is not defined in the board configuration header."
#   define OT_GWNADC_PINNUM 0
#   undef OT_GWNADC_PIN
#endif

/*
    /// Open Floating Input pin
#   define OT_GWNADC_PIN (1 << OT_GWNADC_PINNUM)
    OT_GWNADC_PORT->MODER  |= (3<<(OT_GWNADC_PIN<<1));  //Set to analog mode
    OT_GWNADC_PORT->PUPDR  &= ~(3<<(OT_GWNADC_PIN<<1)); //Set to floating input

    /// 1. Set-up ADC: The trick to this implementation is to drive the sampling
    ///    timer MUCH faster than it is supposed to run, so that it just produces
    ///    garbage data.  The garbage data turns-out to be quite random, even to
    ///    the 8 LSBs of the 12 bit capture.  If you reduce the clock frequency,
    ///    you might want to change the OT_GWNADC_BITS to a lower number.
    ADC->CR2    = 0;
    ADC->CR1    = 0;
    ADC->CCR    = 0;
    ADC->SQR1   = 0;
    ADC->SQR5   = OT_GWNADC_CHAN;
#   if ( <= 9)
    ADC->SMPR3 &= ~(7 << (OT_GWNADC_CHAN*3));
#   elif ( <= 19)
    ADC->SMPR2 &= ~(7 << ((OT_GWNADC_CHAN-10)*3));
#   elif ( <= 25)
    ADC->SMPR3 &= ~(7 << ((OT_GWNADC_CHAN-20)*3));
#   else
#   error "ADC channel is out of range, should be 0-25"
#   endif
    ADC->CR2   |= ADC_CR2_ADON;

    /// 2. Turn-on Zener noisemaker, if enabled by board
#   ifdef OT_GWNZENER_PORT
        OT_GWNZENER_PORT->BSRRL = OT_GWNZENER_PIN;
#   endif

    /// 3. Do Conversion!  Loop until the required number of bytes are produced.
    ///    The random bytes are produced by shifting-in the least-significant
    ///    sections of each sample (exactly how many bits is board-defined).
    while (--bytes_out >= 0) {
        ot_u8 reg;
#       if (OT_GWNADC_BITS == 8)
            //Special case for direct synthesis of random bytes.
            ADC->CR2 |= ADC_CR2_SWSTART;
            while (ADC->SR & ADC_SR_RCNR);
            reg = (ot_u8)ADC->DR;

#       else
            ot_u8 shifts;
            shifts = ((8+(OT_GWNADC_BITS-1)) / OT_GWNADC_BITS);
            while (shifts-- != 0) {
                ADC->CR2 |= ADC_CTL0_SC;  //start conversion
                while (ADC->SR & ADC_SR_RCNR);
                reg   <<= OT_GWNADC_BITS;
                reg    |= ((1<<OT_GWNADC_BITS)-1) & ADC->DR;
            }
#       endif

        *rand_out++ = reg;
    }

    ///5. Shut down ADC, turn-off Zener (if enabled), turn-off pin
    ADC->CR2                = 0;
#   ifdef OT_GWNZENER_PORT
    OT_GWNZENER_PORT->BSRRH&= OT_GWNZENER_PIN;
#   endif
    OT_GWNADC_PORT->MODER  ^= (2<<(OT_GWNADC_PIN<<1));  // go to output mode
    OT_GWNADC_PORT->BSRRH   = OT_GWNADC_PIN;            // set output=GND
*/
}




void rand_prnseed(ot_u32 seed) {
    prand_reg = (ot_u16)seed;
}



ot_u32 rand_prn32() {
///@todo Use CRC hardware
}




ot_u8 rand_prn8() {
    return (ot_u8)rand_prn16();
}



ot_u16 rand_prn16() {
    ot_u16 timer_value;
    timer_value = (ot_u16)RTC->TR;

    return crc16drv_block((ot_u8*)&timer_value, 2);
}




