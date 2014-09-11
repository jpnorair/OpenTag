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
  * @file       /otplatform/stm32l1xx/core_errors.c
  * @author     JP Norair
  * @version    R100
  * @date       27 Aug 2014
  * @brief      Core Error Handlers for STM32F0, F1, L0, L1
  * @ingroup    Platform
  *
  * This implementation is universal to STM32.
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>




/** Error Interrupts <BR>
  * ========================================================================<BR>
  */
  
void sub_save_error(ot_u16 vector_code) {
/// Save the high-level code and low-level code in backup RAM, and then reset
    ot_u32 error_code;
    error_code  = ((ot_u32)platform.error_code << 16) | (ot_u32)vector_code;
    RTC->BKP2R = error_code;
    RTC->BKP1R = __get_PSP();
    RTC->BKP0R = __get_MSP();
    
    NVIC_SystemReset();     //see core_cm3.h in CMSIS
}



/// The NMI is triggerable by software.  It is used to manage runtime errors.
void NMI_Handler(void) {
    ot_int code;

#   ifdef __DEBUG__
    if (platform.error_code == 0)
        while(1);
#   endif

    code                = platform.error_code;
    platform.error_code = 0;
    sys_panic(code);
}


void HardFault_Handler(void) {
/// If you have traced the code here, most likely the problem is stack overrun.
/// You need to allocate a bigger stack for SSTACK or your thread, or fix some
/// other serious problem (possibly interrupt storm)
#ifdef __DEBUG__
    while (1);

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
    while (1);
#else
    sub_save_error(11);
#endif
}


void BusFault_Handler(void) {
/// BusFault is analagous to Bus Error.
#ifdef __DEBUG__
    while (1);
#else
    sub_save_error(7);
#endif
}


void UsageFault_Handler(void) {
/// UsageFault is the "kitchen sink" error.  Mostly, it will happen on
/// misaligned memory access, but also on divide by zero operations.
#ifdef __DEBUG__
    while (1);
#else
    sub_save_error(10);
#endif
}


void DebugMon_Handler(void) {
/// Not really applicable
}




