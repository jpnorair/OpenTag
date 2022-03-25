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
  * @file       /platform/stm32wl55_m0/core_isr.c
  * @author     JP Norair
  * @version    R100
  * @date       16 Dec 2021
  * @brief      ISRs abstracted by the platform module
  * @ingroup    Platform
  *
  *
  ******************************************************************************
  */


#include <otplatform.h>
#if defined(CORE_CM0PLUS)

#include <otstd.h>

///@note isr_config_STM32WL55_M0.h Must be in your app distribution!
#include <app/isr_config_STM32WL55.h>




/** Platform-Managed ISRs   <BR>
  * ========================================================================<BR>
  * The constants of the form __ISR_XXX should be defined per-application in:
  * /apps/[YOURAPP]/code/isr_config_[YOURPLATFORM].h.  Define __ISR_XXX
  * constants based on the ISRs you want to enable.  In some cases, OpenTag
  * will need a certain ISR, and this file will declare an ISR even if it is 
  * defined by __ISR_XXX.  For example, Kernel Timer and Radio I/F interrupts
  * are almost always going to be needed.
  */


/// ISR Entry and Exit Hooks are presently Null, but they are implemented in
/// all the ISRs, so if you have something you need to do on Entry and Exit of
/// every ISR, you can put it here
#define __ISR_ENTRY_HOOK(); 
#define __ISR_EXIT_HOOK(); 


/// The following RTC ISRs are required for OpenTag STM32WL
#undef __ISR_RTC_WKUP
#undef __N_ISR_RTC_WKUP
#define __ISR_RTC_WKUP


/// ISRs that can bring the system out of STOP mode have __ISR_WAKEUP_HOOK().
/// When coming out of STOP, clock either MSI or HSI, and it needs to go back 
/// to the selected value.
#ifdef __DEBUG__
#   define __ISR_KTIM_WAKEUP_HOOK();
#   define __ISR_WAKEUP_HOOK();
#else
#   define __ISR_KTIM_WAKEUP_HOOK()    platform_ext_wakefromstop()
#   define __ISR_WAKEUP_HOOK()         platform_ext_wakefromstop()
#endif







/// Open SPI interrupts:



/// Enable MPIPE Interrupts:
/// DMA interrupts for UART DMA
#if (OT_FEATURE(MPIPE) && BOARD_FEATURE(MPIPE))
#include <otsys/mpipe.h>

#   if (MCU_CONFIG(MPIPEUART))
        // MPipe UART Driver only uses RTS/CTS if Board implements them
#       if (BOARD_FEATURE(MPIPE_FLOWCTL))
        ///@todo CTS/RTS on UART
#       endif

        // MPipe UART Driver uses TX DMA, but not interrupt
#       if (MPIPE_UART_ID == 1)
//#           undef __USE_DMA1_CHAN4
//#           undef __ISR_USART1
//#           define __ISR_USART1
#       elif (MPIPE_UART_ID == 2)
//#           undef __USE_DMA1_CHAN7
//#           undef __ISR_USART2
//#           define __ISR_USART2
#       endif

#   else
#       error "MPIPE is not enabled on a known communication interface."
#   endif

#endif




// Don't use DMA interrupt for Memcpy
#if   ((MEMCPY_DMA_CHAN_ID == 1) && !defined(__N_ISR_DMA1_Channel1))
#   define __N_ISR_DMA1_Channel1
#elif ((MEMCPY_DMA_CHAN_ID == 2) && !defined(__N_ISR_DMA1_Channel2))
#   define __N_ISR_DMA1_Channel2
#elif ((MEMCPY_DMA_CHAN_ID == 3) && !defined(__N_ISR_DMA1_Channel3))
#   define __N_ISR_DMA1_Channel3
#elif ((MEMCPY_DMA_CHAN_ID == 4) && !defined(__N_ISR_DMA1_Channel4))
#   define __N_ISR_DMA1_Channel4
#elif ((MEMCPY_DMA_CHAN_ID == 5) && !defined(__N_ISR_DMA1_Channel5))
#   define __N_ISR_DMA1_Channel5
#elif ((MEMCPY_DMA_CHAN_ID == 6) && !defined(__N_ISR_DMA1_Channel6))
#   define __N_ISR_DMA1_Channel6
#elif ((MEMCPY_DMA_CHAN_ID == 7) && !defined(__N_ISR_DMA1_Channel7))
#   define __N_ISR_DMA1_Channel7
#elif ((MEMCPY_DMA_CHAN_ID == 8) && !defined(__N_ISR_DMA2_Channel1))
#   define __N_ISR_DMA2_Channel1
#elif ((MEMCPY_DMA_CHAN_ID == 9) && !defined(__N_ISR_DMA2_Channel2))
#   define __N_ISR_DMA2_Channel2
#elif ((MEMCPY_DMA_CHAN_ID == 10) && !defined(__N_ISR_DMA2_Channel3))
#   define __N_ISR_DMA2_Channel3
#elif ((MEMCPY_DMA_CHAN_ID == 11) && !defined(__N_ISR_DMA2_Channel4))
#   define __N_ISR_DMA2_Channel4
#elif ((MEMCPY_DMA_CHAN_ID == 12) && !defined(__N_ISR_DMA2_Channel5))
#   define __N_ISR_DMA2_Channel5
#elif ((MEMCPY_DMA_CHAN_ID == 13) && !defined(__N_ISR_DMA2_Channel6))
#   define __N_ISR_DMA2_Channel6
#elif ((MEMCPY_DMA_CHAN_ID == 14) && !defined(__N_ISR_DMA2_Channel7))
#   define __N_ISR_DMA2_Channel7
#endif





/// Enable RF Module interrupts
#if OT_FEATURE(M2)
#   undef __ISR_RFIRQ
#   define __ISR_RFIRQ
#endif




/// Open TrustZone Interrupts
/// This IRQ is unique to the M0 core
#if (defined(__ISR_TZ) && !defined(__N_ISR_TZ))
void TZIC_ILA_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tz();
    __ISR_EXIT_HOOK();
}
#endif



/// Open PVD (supply monitor) Interrupt (todo)
#if defined(__ISR_PVD) && !defined(__N_ISR_PVD)
void PVD_PVM_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    
    // clear PVD (16), and PVM[3] (34)
    // Use PVMO3 flags in PWR->SR2 for PVM detection
    // Use PVDO flag in PWR->SR2 for PVD detection
    EXTI->PR1 = (1<<16);
    EXTI->PR2 = (1<<(34-32));
    
    platform_isr_pvdpvm();
    __ISR_EXIT_HOOK();
}
#endif


/// There is a single IRQ for:
/// Tamper, RTC Timestamp, RTC SSRU, RTC Alarm, RTC Wakeup, and LSE CSS
#define _TAMPER     (defined(__ISR_TAMP) && !defined(__N_ISR_TAMP))
#define _RTC_STAMP  (defined(__ISR_RTC_STAMP) && !defined(__N_ISR_RTC_STAMP))
#define _RTC_SSRU   (defined(__ISR_RTC_SSRU) && !defined(__N_ISR_RTC_SSRU))
#define _RTC_ALARM  (defined(__ISR_RTC_Alarm) && !defined(__N_ISR_RTC_Alarm))
#define _RTC_WKUP   (defined(__ISR_RTC_WKUP) && !defined(__N_ISR_RTC_WKUP))
#define _LSECSS     (defined(__ISR_LSECSS) && !defined(__N_ISR_LSECSS))

#if (_TAMPER || _RTC_STAMP || _RTC_SSRU || _RTC_ALARM || _RTC_WKUP || LSECSS)

void RTC_LSECSS_IRQHandler(void) {
    ot_u32 rtc_misr;
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();

    // Unfortunately, these EXTIs are not available in EXTI-PRn register.
    // The RTC->MISR pins must be cleared in the platform_isr_...() functions.
    // EXTI17   RTC Alarm
    // EXTI18   RTC SSRU
    // EXTI19   TAMPER, RTC TIMESTAMP, LSE_CSS
    // EXTI20   RTC Wakeup
    rtc_misr = RTC->MISR;

#   if _RTC_SSRU
    if (rtc_misr & RTC_MISR_SSRUMF) {
        ///@todo not yet implemented
        platform_isr_rtcssru();
    }
#   endif

#   if _RTC_STAMP
    if (rtc_misr & _RTC_STAMP) {
        platform_isr_timestamp();
    }
#   endif

#   if _TAMPER
    if (rtc_misr & (RTC_MISR_TSOVMF | RTC_MISR_TSMF)) {
        platform_isr_tamper();
    }
#   endif

#   if _LSECSS
    if (/** @todo this check for LSE CSS */) {
        platform_isr_lsecss();
    }
#   endif

#   if _RTC_ALARM
    if (rtc_misr & (RTC_MISR_ALRAMF | RTC_MISR_ALRBMF)) {
        platform_isr_rtcalarm();
    }
#   endif

#   if _RTC_WKUP
    if (rtc_misr & RTC_MISR_WUTMF) {
        platform_isr_rtcwakeup();
    }
#   endif

    __ISR_EXIT_HOOK();
}
#endif
#undef _TAMPER
#undef _RTC_STAMP
#undef _RTC_SSRU
#undef _RTC_ALARM
#undef _RTC_WKUP
#undef _LSECSS



/// RCC_FLASH_C1SEV_IRQHandler
#define _RCC        (defined(__ISR_RCC) && !defined(__N_ISR_RCC))
#define _FLASH      (defined(__ISR_FLASH) && !defined(__N_ISR_FLASH))
#define _C1SEV      (defined(__ISR_C1SEV) && !defined(__N_ISR_C1SEV))

#if (_FLASH || _C1SEV)
void RCC_FLASH_C1SEV_IRQHandler(void) {
    __ISR_ENTRY_HOOK();

#   if _C1SEV
    if (EXTI->PR2 & (1<<(41-32))) {
        EXTI->PR2 = (1<<(41-32));
        __ISR_WAKEUP_HOOK();
        platform_isr_c1sev();
    }
#   endif    

#   if _FLASH
    ///@todo need to check the peripheral flag, PR2 is unused in this bit position
    //if (EXTI->PR2 & (1<<(42-32))) {
    //    EXTI->PR2 = (1<<(42-32));
        __ISR_WAKEUP_HOOK();
    //if ((FLASH->CR & (FLASH_CR_RDERRIE | FLASH_CR_ERRIE | FLASH_CR_EOPIE)) &&
    //    (FLASH->SR & (FLASH_SR_RDERR | FLASH_SR_OPERR | FLASH_SR_EOP))) {
        platform_isr_flash();
    }
#   endif

#   if _RCC
    if (RCC->CIER & RCC->CIFR) {
        platform_isr_rcc();
    }
#   endif

    __ISR_EXIT_HOOK();
}
#endif


/// Open EXTI interrupts
#if defined(__ISR_EXTI0)
#   define APPLICATION_EXTI0_ISR()  platform_isr_exti0()
#else
#   define APPLICATION_EXTI0_ISR(); 
#endif
#if defined(__ISR_EXTI1)
#   define APPLICATION_EXTI1_ISR()  platform_isr_exti1()
#else
#   define APPLICATION_EXTI1_ISR(); 
#endif
#if defined(__ISR_EXTI2)
#   define APPLICATION_EXTI2_ISR()  platform_isr_exti2()
#else
#   define APPLICATION_EXTI2_ISR(); 
#endif
#if defined(__ISR_EXTI3)
#   define APPLICATION_EXTI3_ISR()  platform_isr_exti3()
#else
#   define APPLICATION_EXTI3_ISR(); 
#endif
#if defined(__ISR_EXTI4)
#   define APPLICATION_EXTI4_ISR()  platform_isr_exti4()
#else
#   define APPLICATION_EXTI4_ISR(); 
#endif
#if defined(__ISR_EXTI5)
#   define APPLICATION_EXTI5_ISR()  platform_isr_exti5()
#else
#   define APPLICATION_EXTI5_ISR(); 
#endif
#if defined(__ISR_EXTI6)
#   define APPLICATION_EXTI6_ISR()  platform_isr_exti6()
#else
#   define APPLICATION_EXTI6_ISR(); 
#endif
#if defined(__ISR_EXTI7)
#   define APPLICATION_EXTI7_ISR()  platform_isr_exti7()
#else
#   define APPLICATION_EXTI7_ISR(); 
#endif
#if defined(__ISR_EXTI8)
#   define APPLICATION_EXTI8_ISR()  platform_isr_exti8()
#else
#   define APPLICATION_EXTI8_ISR(); 
#endif
#if defined(__ISR_EXTI9)
#   define APPLICATION_EXTI9_ISR()  platform_isr_exti9()
#else
#   define APPLICATION_EXTI9_ISR(); 
#endif
#if defined(__ISR_EXTI10)
#   define APPLICATION_EXTI10_ISR()  platform_isr_exti10()
#else
#   define APPLICATION_EXTI10_ISR(); 
#endif
#if defined(__ISR_EXTI11)
#   define APPLICATION_EXTI11_ISR()  platform_isr_exti11()
#else
#   define APPLICATION_EXTI11_ISR(); 
#endif
#if defined(__ISR_EXTI12)
#   define APPLICATION_EXTI12_ISR()  platform_isr_exti12()
#else
#   define APPLICATION_EXTI12_ISR(); 
#endif
#if defined(__ISR_EXTI13)
#   define APPLICATION_EXTI13_ISR()  platform_isr_exti13()
#else
#   define APPLICATION_EXTI13_ISR(); 
#endif
#if defined(__ISR_EXTI14)
#   define APPLICATION_EXTI14_ISR()  platform_isr_exti14()
#else
#   define APPLICATION_EXTI14_ISR(); 
#endif
#if defined(__ISR_EXTI15)
#   define APPLICATION_EXTI15_ISR()  platform_isr_exti15()
#else
#   define APPLICATION_EXTI15_ISR(); 
#endif


#if (BOARD_FEATURE(MPIPE) && defined(BOARD_COM_EXTI0_ISR))
#   define __MPIPE_EXTI(NUM)    BOARD_COM_EXTI##NUM##_ISR()
#elif (BOARD_FEATURE(MPIPE) && defined(BOARD_MPIPE_EXTI0_ISR))
#   define __MPIPE_EXTI(NUM)    BOARD_MPIPE_EXTI##NUM##_ISR()
#else
#   define __MPIPE_EXTI(NUM); 
#endif



#define __EXTI_MACRO_LOW(NUM)  \
    EXTI->PR1 = (1<<NUM);  \
    __MPIPE_EXTI(NUM); \
    APPLICATION_EXTI##NUM##_ISR()
    
#define __EXTI_MACRO(NUM)   \
    if (EXTI->PR1 & (1<<NUM)) { \
        EXTI->PR1 = (1<<NUM);  \
        __MPIPE_EXTI(NUM); \
        APPLICATION_EXTI##NUM##_ISR(); \
    } \
    else

#if (  ((defined(__ISR_EXTI0) || defined(__USE_EXTI0)) && !defined(__N_ISR_EXTI0)) \
    || ((defined(__ISR_EXTI1) || defined(__USE_EXTI1)) && !defined(__N_ISR_EXTI1)) )
void EXTI1_0_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    
#   if ((defined(__ISR_EXTI0) || defined(__USE_EXTI0)) && !defined(__N_ISR_EXTI0))
    __EXTI_MACRO(0)
#   endif
#   if ((defined(__ISR_EXTI1) || defined(__USE_EXTI1)) && !defined(__N_ISR_EXTI1))
    __EXTI_MACRO(1)
#   endif
    { } //to terminate "else"

    __ISR_EXIT_HOOK();
}
#endif

#if (  ((defined(__ISR_EXTI2) || defined(__USE_EXTI2)) && !defined(__N_ISR_EXTI2)) \
    || ((defined(__ISR_EXTI3) || defined(__USE_EXTI3)) && !defined(__N_ISR_EXTI3)) )
void EXTI3_2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    
#   if ((defined(__ISR_EXTI2) || defined(__USE_EXTI2)) && !defined(__N_ISR_EXTI2))
    __EXTI_MACRO(2)
#   endif
#   if ((defined(__ISR_EXTI3) || defined(__USE_EXTI3)) && !defined(__N_ISR_EXTI3))
    __EXTI_MACRO(3)
#   endif
    { } //to terminate "else"

    __ISR_EXIT_HOOK();
}
#endif

#if (  ((defined(__ISR_EXTI4) || defined(__USE_EXTI4)) && !defined(__N_ISR_EXTI4)) \
    || ((defined(__ISR_EXTI5) || defined(__USE_EXTI5)) && !defined(__N_ISR_EXTI5)) \
    || ((defined(__ISR_EXTI6) || defined(__USE_EXTI6)) && !defined(__N_ISR_EXTI6)) \
    || ((defined(__ISR_EXTI7) || defined(__USE_EXTI7)) && !defined(__N_ISR_EXTI7)) \
    || ((defined(__ISR_EXTI8) || defined(__USE_EXTI8)) && !defined(__N_ISR_EXTI8)) \
    || ((defined(__ISR_EXTI9) || defined(__USE_EXTI9)) && !defined(__N_ISR_EXTI9)) \
    || ((defined(__ISR_EXTI10) || defined(__USE_EXTI10)) && !defined(__N_ISR_EXTI10)) \
    || ((defined(__ISR_EXTI11) || defined(__USE_EXTI11)) && !defined(__N_ISR_EXTI11)) \
    || ((defined(__ISR_EXTI12) || defined(__USE_EXTI12)) && !defined(__N_ISR_EXTI12)) \
    || ((defined(__ISR_EXTI13) || defined(__USE_EXTI13)) && !defined(__N_ISR_EXTI13)) \
    || ((defined(__ISR_EXTI14) || defined(__USE_EXTI14)) && !defined(__N_ISR_EXTI14)) \
    || ((defined(__ISR_EXTI15) || defined(__USE_EXTI15)) && !defined(__N_ISR_EXTI15)) )
void EXTI15_4_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();

#   if ((defined(__ISR_EXTI4) || defined(__USE_EXTI4)) && !defined(__N_ISR_EXTI4))
    __EXTI_MACRO(4)
#   endif
#   if ((defined(__ISR_EXTI5) || defined(__USE_EXTI5)) && !defined(__N_ISR_EXTI5))
    __EXTI_MACRO(5)
#   endif
#   if ((defined(__ISR_EXTI6) || defined(__USE_EXTI6)) && !defined(__N_ISR_EXTI6))
    __EXTI_MACRO(6)
#   endif
#   if ((defined(__ISR_EXTI7) || defined(__USE_EXTI7)) && !defined(__N_ISR_EXTI7))
    __EXTI_MACRO(7)
#   endif
#   if ((defined(__ISR_EXTI8) || defined(__USE_EXTI8)) && !defined(__N_ISR_EXTI8))
    __EXTI_MACRO(8)
#   endif
#   if ((defined(__ISR_EXTI9) || defined(__USE_EXTI9)) && !defined(__N_ISR_EXTI9))
    __EXTI_MACRO(9)
#   endif    
#   if ((defined(__ISR_EXTI10) || defined(__USE_EXTI10)) && !defined(__N_ISR_EXTI10))
    __EXTI_MACRO(10)
#   endif
#   if ((defined(__ISR_EXTI11) || defined(__USE_EXTI11)) && !defined(__N_ISR_EXTI11))
    __EXTI_MACRO(11)
#   endif
#   if ((defined(__ISR_EXTI12) || defined(__USE_EXTI12)) && !defined(__N_ISR_EXTI12))
    __EXTI_MACRO(12)
#   endif
#   if ((defined(__ISR_EXTI13) || defined(__USE_EXTI13)) && !defined(__N_ISR_EXTI13))
    __EXTI_MACRO(13)
#   endif
#   if ((defined(__ISR_EXTI14) || defined(__USE_EXTI14)) && !defined(__N_ISR_EXTI14))
    __EXTI_MACRO(14)
#   endif
#   if ((defined(__ISR_EXTI15) || defined(__USE_EXTI15)) && !defined(__N_ISR_EXTI15))
    __EXTI_MACRO(15)
#   endif
    { } //to terminate "else"

    __ISR_EXIT_HOOK();
}
#endif



/// Handle ADC, COMP, and DAC IRQs
#define _ADC    (defined(__ISR_ADC) && !defined(__N_ISR_ADC))
#define _COMP1  (defined(__ISR_COMP1) && !defined(__N_ISR_COMP1))
#define _COMP2  (defined(__ISR_COMP2) && !defined(__N_ISR_COMP2))
#define _DAC    (defined(__ISR_DAC) && !defined(__N_ISR_DAC))
#if (_ADC || _COMP1 || _COMP2 || _DAC)
void ADC_COMP_DAC_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    
#   if (_COMP)
    {   ot_u32 exti_pr;
        exti_pr = EXTI->PR1;
        if (exti_pr & ((1<<21)|(1<<22))) {
            EXTI->PR1 = (1<<21)|(1<<22);
            __ISR_WAKEUP_HOOK();
            
            if (exti_pr & (1<<21))  platform_isr_comp1();
            if (exti_pr & (1<<22))  platform_isr_comp2();
        }
    }
#   endif
    
#   if (_ADC)
    if (ADC->ISR & ADC->IER)
        platform_isr_adc();
#   endif

#   if (_DAC)
    if (DAC->CR & DAC->SR & DAC_SR_DMAUDR1)
        platform_isr_dac();
#   endif

    __ISR_EXIT_HOOK();
}
#endif
#undef _ADC
#undef _COMP1
#undef _COMP2
#undef _DAC




/// Open DMA interrupts:
/// If you are using a DMA, the interrupt is available unless you declare
/// __N_ISR_DMAX_ChannelY in your app.  MEMCPY uses a DMA without interrupt, so
/// you'll see below how to force-off a DMA channel interrupt.

#define _DMA1   ((defined(__USE_DMA1_CHAN1) || defined(__ISR_DMA1_Channel1)) && !defined(__N_ISR_DMA1_Channel1))
#define _DMA2   ((defined(__USE_DMA1_CHAN2) || defined(__ISR_DMA1_Channel2)) && !defined(__N_ISR_DMA1_Channel2))
#define _DMA3   ((defined(__USE_DMA1_CHAN3) || defined(__ISR_DMA1_Channel3)) && !defined(__N_ISR_DMA1_Channel3))
#define _DMA4   ((defined(__USE_DMA1_CHAN4) || defined(__ISR_DMA1_Channel4)) && !defined(__N_ISR_DMA1_Channel4))
#define _DMA5   ((defined(__USE_DMA1_CHAN5) || defined(__ISR_DMA1_Channel5)) && !defined(__N_ISR_DMA1_Channel5))
#define _DMA6   ((defined(__USE_DMA1_CHAN6) || defined(__ISR_DMA1_Channel6)) && !defined(__N_ISR_DMA1_Channel6))
#define _DMA7   ((defined(__USE_DMA1_CHAN7) || defined(__ISR_DMA1_Channel7)) && !defined(__N_ISR_DMA1_Channel7))

#if (_DMA1 || _DMA2 || _DMA3)
void DMA1_Channel1_2_3_IRQHandler(void) {
    ot_u32 chan_x;
    __ISR_ENTRY_HOOK();
#   if (_DMA1)
    chan_x = DMA1_Channel1->CCR;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA1->ISR & (1<<0)))
        platform_isr_dma1ch1();
#   endif
#   if (_DMA2)
    chan_x = DMA1_Channel2->CCR;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA1->ISR & (1<<4)))
        platform_isr_dma1ch2();
#   endif
#   if (_DMA3)
    chan_x = DMA1_Channel3->CCR;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA1->ISR & (1<<8)))
        platform_isr_dma1ch3();
#   endif
    __ISR_EXIT_HOOK();
}
#endif

#if (_DMA4 || _DMA5 || _DMA6 || _DMA7)
void DMA1_Channel4_5_6_7_IRQHandler(void) {
    ot_u32 chan_x;
    __ISR_ENTRY_HOOK();
#   if (_DMA4)
    chan_x = DMA1_Channel4->CCR;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA1->ISR & (1<<12)))
        platform_isr_dma1ch4();
#   endif
#   if (_DMA5)
    chan_x = DMA1_Channel5->CCR;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA1->ISR & (1<<16)))
        platform_isr_dma1ch5();
#   endif
#   if (_DMA6)
    chan_x = DMA1_Channel6->CCR;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA1->ISR & (1<<20)))
        platform_isr_dma1ch6();
#   endif
#   if (_DMA7)
    chan_x = DMA1_Channel7->CCR;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA1->ISR & (1<<24)))
        platform_isr_dma1ch7();
#   endif
    __ISR_EXIT_HOOK();
}
#endif

#undef _DMA1
#undef _DMA2
#undef _DMA3
#undef _DMA4
#undef _DMA5
#undef _DMA6
#undef _DMA7


#define _DMA8   ((defined(__USE_DMA2_CHAN1) || defined(__ISR_DMA2_Channel1)) && !defined(__N_ISR_DMA2_Channel1))
#define _DMA9   ((defined(__USE_DMA2_CHAN2) || defined(__ISR_DMA2_Channel2)) && !defined(__N_ISR_DMA2_Channel2))
#define _DMA10  ((defined(__USE_DMA2_CHAN3) || defined(__ISR_DMA2_Channel3)) && !defined(__N_ISR_DMA2_Channel3))
#define _DMA11  ((defined(__USE_DMA2_CHAN4) || defined(__ISR_DMA2_Channel4)) && !defined(__N_ISR_DMA2_Channel4))
#define _DMA12  ((defined(__USE_DMA2_CHAN5) || defined(__ISR_DMA2_Channel5)) && !defined(__N_ISR_DMA2_Channel5))
#define _DMA13  ((defined(__USE_DMA2_CHAN6) || defined(__ISR_DMA2_Channel6)) && !defined(__N_ISR_DMA2_Channel6))
#define _DMA14  ((defined(__USE_DMA2_CHAN7) || defined(__ISR_DMA2_Channel7)) && !defined(__N_ISR_DMA2_Channel7))

#define _DMAMUX1_OVR0   (defined(__ISR_DMAMUX1_OVR0) && !defined(__N_ISR_DMAMUX1_OVR0))
#define _DMAMUX1_OVR1   (defined(__ISR_DMAMUX1_OVR1) && !defined(__N_ISR_DMAMUX1_OVR1))
#define _DMAMUX1_OVR2   (defined(__ISR_DMAMUX1_OVR2) && !defined(__N_ISR_DMAMUX1_OVR2))
#define _DMAMUX1_OVR3   (defined(__ISR_DMAMUX1_OVR3) && !defined(__N_ISR_DMAMUX1_OVR3))

#if ( _DMA8 || _DMA9 || _DMA10 || _DMA11 || _DMA12 || _DMA13 || _DMA14 \
    ||_DMAMUX1_OVR0 || _DMAMUX1_OVR1 || _DMAMUX1_OVR2 || _DMAMUX1_OVR3)
void DMA2_DMAMUX1_OVR_IRQHandler(void) {
    ot_u32 chan_x;
    __ISR_ENTRY_HOOK();
    
#   if (_DMA8)
    chan_x = DMA2_Channel1->CCR;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA2->ISR & (1<<0)))
        platform_isr_dma2ch1();
#   endif
#   if (_DMA9)
    chan_x = DMA2_Channel2->CCR;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA2->ISR & (1<<4)))
        platform_isr_dma2ch2();
#   endif
#   if (_DMA10)
    chan_x = DMA2_Channel3->CCR;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA2->ISR & (1<<8)))
        platform_isr_dma2ch3();
#   endif
#   if (_DMA11)
    chan_x = DMA2_Channel4->CCR;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA2->ISR & (1<<12)))
        platform_isr_dma2ch4();
#   endif
#   if (_DMA12)
    chan_x = DMA2_Channel5->CCR;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA2->ISR & (1<<16)))
        platform_isr_dma2ch5();
#   endif
#   if (_DMA13)
    chan_x = DMA2_Channel6->CCR;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA2->ISR & (1<<20)))
        platform_isr_dma2ch6();
#   endif
#   if (_DMA14)
    chan_x = DMA2_Channel7->CCR;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA2->ISR & (1<<24)))
        platform_isr_dma2ch7();
#   endif

#   if (_DMAMUX1_OVR0)
    chan_x = DMAMUX->RG0CR;
    if ((chan_x & DMAMUX_OIE) && (DMAMUX->ISR & (1<<0))
        platform_isr_dmamux_ovr0();
#   endif
#   if (_DMAMUX1_OVR1)
    chan_x = DMAMUX->RG1CR;
    if ((chan_x & DMAMUX_OIE) && (DMAMUX->ISR & (1<<1))
        platform_isr_dmamux_ovr1();
#   endif
#   if (_DMAMUX1_OVR2)
    chan_x = DMAMUX->RG2CR;
    if ((chan_x & DMAMUX_OIE) && (DMAMUX->ISR & (1<<2))
        platform_isr_dmamux_ovr2();
#   endif
#   if (_DMAMUX1_OVR3)
    chan_x = DMAMUX->RG3CR;
    if ((chan_x & DMAMUX_OIE) && (DMAMUX->ISR & (1<<3))
        platform_isr_dmamux_ovr3();
#   endif

    __ISR_EXIT_HOOK();
}
#endif

#undef _DMA8
#undef _DMA9
#undef _DMA10
#undef _DMA11
#undef _DMA12
#undef _DMA13
#undef _DMA14
#undef _DMAMUX1_OVR0
#undef _DMAMUX1_OVR1
#undef _DMAMUX1_OVR2
#undef _DMAMUX1_OVR3



/// Open LPTIM1 Interrupt
#if defined(__ISR_LPTIM1) && !defined(__N_ISR_LPTIM1)
void LPTIM1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    //__ISR_KTIM_WAKEUP_HOOK();
    platform_isr_lptim1();
    __ISR_EXIT_HOOK();
}
#endif

/// Open LPTIM2 Interrupt
#if defined(__ISR_LPTIM2) && !defined(__N_ISR_LPTIM2)
void LPTIM2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    //__ISR_KTIM_WAKEUP_HOOK();
    platform_isr_lptim2();
    __ISR_EXIT_HOOK();
}
#endif

/// Open LPTIM3 Interrupt
#if defined(__ISR_LPTIM3) && !defined(__N_ISR_LPTIM3)
void LPTIM3_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    //__ISR_KTIM_WAKEUP_HOOK();
    platform_isr_lptim3();
    __ISR_EXIT_HOOK();
}
#endif

/// Open TIM1 interrupts
#if (defined(__ISR_TIM1) && !defined(__N_ISR_TIM1))
void TIM1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim1();
    __ISR_EXIT_HOOK();
}
#endif

/// Open TIM2 interrupts
#if (defined(__ISR_TIM2) && !defined(__N_ISR_TIM2))
void TIM2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim2();
    __ISR_EXIT_HOOK();
}
#endif

/// Open TIM16 interrupts  
#if (defined(__ISR_TIM16) && !defined(__N_ISR_TIM16))
void TIM16_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim16();
    __ISR_EXIT_HOOK();
}
#endif

/// Open TIM17 interrupts  
#if (defined(__ISR_TIM17) && !defined(__N_ISR_TIM17))
void TIM17_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim17();
    __ISR_EXIT_HOOK();
}
#endif




/// Open IPCC Interrupt (CPU2 side)
#if (defined(__ISR_IPCC) && !defined(__N_ISR_IPCC))
void IPCC_C2_RX_C2_TX_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    //EXTI->PR2 = (1<<(37-32));
    platform_isr_ipcc();
    __ISR_EXIT_HOOK();
}
#endif

/// Open HW Semaphore Interrupt (CPU2 side)
#if (defined(__ISR_HSEM) && !defined(__N_ISR_HSEM))
void HSEM_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    //EXTI->PR2 = (1<<(39-32));
    platform_isr_hsem();
    __ISR_EXIT_HOOK();
}
#endif




/// Open RNG Interrupt
#if defined(__ISR_RNG) && !defined(__N_ISR_RNG)
void RNG_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_rng();
    __ISR_EXIT_HOOK();
}
#endif



/// Open AES and PKA Interrupts
#define _AES    (defined(__ISR_AES) && !defined(__N_ISR_AES))
#define _PKA    (defined(__ISR_PKA) && !defined(__N_ISR_PKA))
#if (_AES || _PKA)
void AES_PKA_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_aes();

#   if (_AES)
    {   uint32_t aes_cr = AES->CR;
        uint32_t aes_sr = AES->SR
        if (((aes_cr & AES_CR_ERRIE) && (aes_sr & (AES_SR_WRERR | AES_SR_RDERR)) \
        ||  ((aes_cr & CCFIE) && (aes_sr & AES_SR_CCF)) {
            platform_isr_aes();
        }
    }
#   endif

#   if (_PKA)
    if ((PKA->SR & PKA->CR & (PKA_SR_ADDRERRF | PKA_SR_RAMERRF | PKA_SR_PROCENDF))
        platform_isr_pka();
    }
#   endif
    
    __ISR_EXIT_HOOK();
}
#endif
#undef _AES
#undef _PKA



/// Open I2C1 interrupts 
#if defined(__USE_I2C1) || (defined(__ISR_I2C1) && !defined(__N_ISR_I2C1))
void I2C1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    //EXTI->PR1 = (1<<23);
    platform_isr_i2c1();
    __ISR_EXIT_HOOK();
}
#endif

/// Open I2C2 interrupts 
#if defined(__USE_I2C2) || (defined(__ISR_I2C2) && !defined(__N_ISR_I2C2))
void I2C2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    //EXTI->PR1 = (1<<24);
    platform_isr_i2c2();
    __ISR_EXIT_HOOK();
}
#endif

/// Open I2C3 interrupts 
#if defined(__USE_I2C3) || (defined(__ISR_I2C3) && !defined(__N_ISR_I2C3))
void I2C3_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    //EXTI->PR1 = (1<<25);
    platform_isr_i2c3();
    __ISR_EXIT_HOOK();
}
#endif



/// Open SPI interrupts 
#if defined(__ISR_SPI1) && !defined(__N_ISR_SPI1)
void SPI1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_spi1();
    __ISR_EXIT_HOOK();
}
#endif
#if defined(__ISR_SPI2) && !defined(__N_ISR_SPI2)
void SPI2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_spi2();
    __ISR_EXIT_HOOK();
}
#endif



/// Open USART interrupts
#if defined(__ISR_USART1) && !defined(__N_ISR_USART1)
void USART1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    //EXTI->PR1 = (1<<26);
    platform_isr_usart1();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_USART2) && !defined(__N_ISR_USART2)
void USART2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    //EXTI->PR1 = (1<<27);
    platform_isr_usart2();
    __ISR_EXIT_HOOK();
}
#endif

/// Open LPUART interrupt
#if defined(__ISR_LPUART1) && !defined(__N_ISR_LPUART1)
void RNG_LPUART1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    //EXTI->PR1 = (1<<28);
    platform_isr_lpuart1();
    __ISR_EXIT_HOOK();
}
#endif





/// Open RF SPI interrupt
#if defined(__ISR_RFSPI) && !defined(__N_ISR_RFSPI)
void SUBGHZSPI_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_rfspi();
    __ISR_EXIT_HOOK();
}
#endif


#define _RFIRQ      (defined(__ISR_RFIRQ) && !defined(__N_ISR_RFIRQ))
#define _RFBUSY     (defined(__ISR_RFBUSY) && !defined(__N_ISR_RFBUSY))
#if (_RFIRQ || _RFBUSY)
void SUBGHZ_Radio_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    ///@note there is not a bit to detect if the IRQ is from the RFIRQ[2:0].

#   if _RFBUSY
    if (EXTI->PR2 & (1<<(45-32))) {
        EXTI->PR2 = (1<<(45-32));
        platform_isr_rfbusy();
    }
#   endif
#   if _RFIRQ
    platform_isr_rfirq();
#   endif

    __ISR_EXIT_HOOK();
}
#endif


#endif  // if defined(__STM32WL55_M0__)






