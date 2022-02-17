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
  * @file       /platform/stm32wl55_m0/core_errors.c
  * @author     JP Norair
  * @version    R100
  * @date       16 Dec 2021
  * @brief      Core Error Handlers for STM32WL M0+ core
  * @ingroup    Platform
  *
  * This implementation is universal to STM32.
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>
#include <otsys.h>
#include <otlib/delay.h>


/** Error Interrupts <BR>
  * ========================================================================<BR>
  */
  
void sub_save_error(ot_u16 vector_code) {
/// Save the high-level code and low-level code in backup RAM, and then reset
    ot_u32 error_code;
    error_code  = ((ot_u32)platform.error_code << 16) | (ot_u32)vector_code;
    TAMP->BKP2R = error_code;
    TAMP->BKP1R = __get_PSP();
    TAMP->BKP0R = __get_MSP();
    
    ///@todo put breakpoint instruction here?
    
    NVIC_SystemReset();     //see core_cmX.h in CMSIS
}



/// The NMI is triggerable by software.  It is used to manage runtime errors.
void NMI_Handler(void) {
    ot_int code;

#   ifdef __DEBUG__
    __BKPT(0);
#   endif

    code                = platform.error_code;
    platform.error_code = 0;
    sys_panic(code);
}


void HardFault_Handler(void) {
/// If you have traced the code here, most likely the problem is stack overrun.
/// You need to allocate a bigger stack for SSTACK or your thread, or fix some
/// other serious problem (possibly interrupt storm)
#if defined(__DEBUG__)
    __BKPT(1);
#elif defined(__PROTO__)
    while (1) {
        BOARD_led1_on();
        BOARD_led2_off();
        delay_ti(100);
        BOARD_led1_off();
        BOARD_led2_on();
        delay_ti(100);
    }
    
#else
    /// Log HardFault by saving an error code to Backup RAM, then resetting.
    /// The startup routine will check this sector of backup RAM and do the
    /// actual logging.
    /// @note Backup RAM *MUST* be enabled (normally done in RTC/GPTIM init)
    /// @todo Finish this implementation, it is not tested yet
    sub_save_error(1);
    
#endif
}


void MemManage_Handler(void) {
/// MemManage Fault is analagous to Segmentation Fault.  OpenTag doesn't use
/// Protected memory spaces at this time, so MemManage fault should never occur.
#ifdef __DEBUG__
    __BKPT(11);
#else
    sub_save_error(11);
#endif
}


void BusFault_Handler(void) {
/// BusFault is analagous to Bus Error.
#ifdef __DEBUG__
    __BKPT(7);
#else
    sub_save_error(7);
#endif
}


void UsageFault_Handler(void) {
/// UsageFault is the "kitchen sink" error.  Mostly, it will happen on
/// misaligned memory access, but also on divide by zero operations.
#ifdef __DEBUG__
    __BKPT(10);
#else
    sub_save_error(10);
#endif
}


void DebugMon_Handler(void) {
/// Not really applicable
}




