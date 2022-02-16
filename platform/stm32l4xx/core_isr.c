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
  * @file       /platform/stm32l4xx/core_isr.c
  * @author     JP Norair
  * @version    R100
  * @date       16 Dec 2019
  * @brief      ISRs abstracted by the platform module
  * @ingroup    Platform
  *
  *
  ******************************************************************************
  */


#include <otplatform.h>
#if defined(__STM32L4__)

#include <otstd.h>
#include <app/isr_config_STM32L4.h>      // Must be in your app distribution!







/** Platform-Managed ISRs   <BR>
  * ========================================================================<BR>
  * The constants of the form __ISR_XXX should be defined per-application in:
  * /apps/[YOURAPP]/code/isr_config_[YOURPLATFORM].h.  Define __ISR_XXX
  * constants based on the ISRs you want to enable.  In some cases, OpenTag
  * will need a certain ISR, and this file will declare an ISR even if it is 
  * defined by __ISR_XXX.  For example, Kernel Timer and Radio I/F interrupts
  * are almost always going to be needed.
  */

#define _USB_ON_MPIPE           (OT_FEATURE(MPIPE) && defined(MPIPE_USB))
#define _USB_ON_SOMETHINGELSE   ...

#if (MCU_CONFIG_USB && (_USB_ON_MPIPE || 0))
#   undef __ISR_USB
#   define __ISR_USB
#endif


/// ISR Entry and Exit Hooks are presently Null, but they are implemented in
/// all the ISRs, so if you have something you need to do on Entry and Exit of
/// every ISR, you can put it here
#define __ISR_ENTRY_HOOK(); 
#define __ISR_EXIT_HOOK(); 


/// LPTIM Alarm and RTC-Wakeup interrupt are required by OpenTag
#undef __ISR_RTC_WKUP
#undef __N_ISR_RTC_WKUP
#define __ISR_RTC_WKUP
#undef __ISR_LPTIM1
#undef __N_ISR_LPTIM1
#define __ISR_LPTIM1


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

#   if (MCU_CONFIG(MPIPECDC))
        ///@todo USB MPipe Interrupt settings
   
#   elif (MCU_CONFIG(MPIPEUART))
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
#       elif (MPIPE_UART_ID == 3)
//#           undef __USE_DMA1_CHAN2
//#           undef __ISR_USART3
//#           define __ISR_USART3
#       elif (MPIPE_UART_ID == 4)
//#           undef __USE_DMA2_CHAN3
//#           undef __ISR_USART4
//#           define __ISR_USART4
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
#endif





/// Enable RF Module interrupts
#if OT_FEATURE(M2)
//#   undef __ISR_RF1A
//#   define __ISR_RF1A
#endif


/// Open Window-Watchdog interrupt (todo)
#if defined(__ISR_WWDG) && !defined(__N_ISR_WWDG)
void WWDG_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_wwdg();
    __ISR_EXIT_HOOK();
}
#endif


/// Open PVD (supply monitor) Interrupt (todo)
#if defined(__ISR_PVD) && !defined(__N_ISR_PVD)
void PVD_PVM_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    
    // clear PVD (16), and PVM1-4 (35,36,37,38)
    // Use PVMO flags in PWR->SR2 for PVM detection
    // Use PVDO flag in PWR->SR2 for PVD detection
    EXTI->PR1 = (1<<16);
    EXTI->PR2 = (1<<(35-32))|(1<<(36-32))|(1<<(37-32))|(1<<(38-32));
    
    __ISR_WAKEUP_HOOK();
    platform_isr_pvdpvm();
    __ISR_EXIT_HOOK();
}
#endif


/// Open RTC_TAMP_STAMP/CSS_LSE interupt, which has 2 synthetic interrupts
#define _TAMPER     (defined(__ISR_TAMPER_STAMP) && !defined(__N_ISR_TAMPER_STAMP))
#define _CSSLSE     (defined(__ISR_CSSLSE) && !defined(__N_ISR_CSSLSE))
#if (_TAMPER || CSSLSE)
void TAMP_STAMP_IRQHandler(void) {
    ot_u32 exti_pr;
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    exti_pr     = EXTI->PR1;
    EXTI->PR1    = (1<<19);

#   if (_CSSLSE)
#       if (_TAMPER) 
        if (RCC->CIER & RCC->CIFR & RCC_CIFR_CSSLSEF)
#       endif
            platform_isr_csslse();
#   endif
#   if (_TAMPER) 
#       if (_CSSLSE)
        if (TAMP->MISR & (TAMP_MISR_TAMP2MF | TAMP_MISR_TAMP1MF))
#       endif
            // platform_isr_tamperstamp must clear its own flags in RTC->ISR
            platform_isr_tamperstamp();
#   endif
    __ISR_EXIT_HOOK();
}
#endif
#undef _TAMPER
#undef _CCSLSE


/// Open RTC_WKUP interrupt
#if defined(__ISR_RTC_WKUP) && !defined(__N_ISR_RTC_WKUP)
void RTC_WKUP_IRQHandler(void) {
    ot_u32 exti_pr;
    __ISR_ENTRY_HOOK();
    EXTI->PR1 = (1<<20);
    __ISR_WAKEUP_HOOK();
    
    ///@todo put this in entry/wakeup hook?
    RTC->ICSR  &= ~RTC_ICSR_RSF;
    
    RTC->SCR   |= RTC_SCR_CWUTF;
    platform_isr_rtcwakeup();

    __ISR_EXIT_HOOK();
}
#endif


/// Open RTC_Alarm interrupt
#if defined(__ISR_RTC_Alarm) && !defined(__N_ISR_RTC_Alarm)
void RTC_Alarm_IRQHandler(void) {
    ot_u32 exti_pr;
    __ISR_ENTRY_HOOK();
    EXTI->PR1 = (1<<18);
    __ISR_WAKEUP_HOOK();
    
    ///@todo put this in entry/wakeup hook?
    RTC->ICSR  &= ~RTC_ICSR_RSF;
    
    // platform_isr_rtcalarm must clear its own flags in RTC->ISR / RTC->SCR
    platform_isr_rtcalarm();

    __ISR_EXIT_HOOK();
}
#endif

///@todo Open FLASH interrupt
#if defined(__ISR_FLASH) && !defined(__N_ISR_FLASH)
void FLASH_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_flash();
    __ISR_EXIT_HOOK();
}
#endif


/// Open RCC
#if (defined(__ISR_RCC) && !defined(__N_ISR_RCC))
void RCC_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_rcc();
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

#if (OT_FEATURE(M2))
#   define __RADIO_EXTI(NUM)    BOARD_RADIO_EXTI##NUM##_ISR()
#else
#   define __RADIO_EXTI(NUM); 
#endif
#if (BOARD_FEATURE(MPIPE) && defined(BOARD_COM_EXTI0_ISR))
#   define __MPIPE_EXTI(NUM)    BOARD_COM_EXTI##NUM##_ISR()
#elif (BOARD_FEATURE(MPIPE) && defined(BOARD_MPIPE_EXTI0_ISR))
#   define __MPIPE_EXTI(NUM)    BOARD_MPIPE_EXTI##NUM##_ISR()
#else
#   define __MPIPE_EXTI(NUM); 
#endif

#define __EXTI_MACRO_LOW(NUM);  \
    EXTI->PR1 = (1<<NUM);  \
    __RADIO_EXTI(NUM); \
    __MPIPE_EXTI(NUM); \
    APPLICATION_EXTI##NUM##_ISR();
    
#define __EXTI_MACRO(NUM);   \
    if (EXTI->PR1 & (1<<NUM)) { \
        EXTI->PR1 = (1<<NUM);  \
        __RADIO_EXTI(NUM); \
        __MPIPE_EXTI(NUM); \
        APPLICATION_EXTI##NUM##_ISR(); \
    } \
    else

// EXTI0 - EXTI4
#if ((defined(__ISR_EXTI0) || defined(__USE_EXTI0)) && !defined(__N_ISR_EXTI0))
void EXTI0_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    __EXTI_MACRO_LOW(0);
    __ISR_EXIT_HOOK();
}
#endif
#if ((defined(__ISR_EXTI1) || defined(__USE_EXTI1)) && !defined(__N_ISR_EXTI1))
void EXTI1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    __EXTI_MACRO_LOW(1);
    __ISR_EXIT_HOOK();
}
#endif
#if ((defined(__ISR_EXTI2) || defined(__USE_EXTI2)) && !defined(__N_ISR_EXTI2))
void EXTI2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    __EXTI_MACRO_LOW(2);
    __ISR_EXIT_HOOK();
}
#endif
#if ((defined(__ISR_EXTI3) || defined(__USE_EXTI3)) && !defined(__N_ISR_EXTI3))
void EXTI3_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    __EXTI_MACRO_LOW(3);
    __ISR_EXIT_HOOK();
}
#endif
#if ((defined(__ISR_EXTI4) || defined(__USE_EXTI4)) && !defined(__N_ISR_EXTI4))
void EXTI4_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    __EXTI_MACRO_LOW(4);
    __ISR_EXIT_HOOK();
}
#endif

// EXTI5-9
#if (  ((defined(__ISR_EXTI5) || defined(__USE_EXTI5)) && !defined(__N_ISR_EXTI5)) \
    || ((defined(__ISR_EXTI6) || defined(__USE_EXTI6)) && !defined(__N_ISR_EXTI6)) \
    || ((defined(__ISR_EXTI7) || defined(__USE_EXTI7)) && !defined(__N_ISR_EXTI7)) \
    || ((defined(__ISR_EXTI8) || defined(__USE_EXTI8)) && !defined(__N_ISR_EXTI8)) \
    || ((defined(__ISR_EXTI9) || defined(__USE_EXTI9)) && !defined(__N_ISR_EXTI9)) \
    )
void EXTI9_5_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    
#   if ((defined(__ISR_EXTI5) || defined(__USE_EXTI5)) && !defined(__N_ISR_EXTI5))
    __EXTI_MACRO(5);
#   endif
#   if ((defined(__ISR_EXTI6) || defined(__USE_EXTI6)) && !defined(__N_ISR_EXTI6))
    __EXTI_MACRO(6);
#   endif
#   if ((defined(__ISR_EXTI7) || defined(__USE_EXTI7)) && !defined(__N_ISR_EXTI7))
    __EXTI_MACRO(7);
#   endif
#   if ((defined(__ISR_EXTI8) || defined(__USE_EXTI8)) && !defined(__N_ISR_EXTI8))
    __EXTI_MACRO(8);
#   endif
#   if ((defined(__ISR_EXTI9) || defined(__USE_EXTI9)) && !defined(__N_ISR_EXTI9))
    __EXTI_MACRO(9);
#   endif  
    { } //to terminate "else"

    __ISR_EXIT_HOOK();
}
#endif

// EXTI10-15
#if (  ((defined(__ISR_EXTI10) || defined(__USE_EXTI10)) && !defined(__N_ISR_EXTI10)) \
    || ((defined(__ISR_EXTI11) || defined(__USE_EXTI11)) && !defined(__N_ISR_EXTI11)) \
    || ((defined(__ISR_EXTI12) || defined(__USE_EXTI12)) && !defined(__N_ISR_EXTI12)) \
    || ((defined(__ISR_EXTI13) || defined(__USE_EXTI13)) && !defined(__N_ISR_EXTI13)) \
    || ((defined(__ISR_EXTI14) || defined(__USE_EXTI14)) && !defined(__N_ISR_EXTI14)) \
    || ((defined(__ISR_EXTI15) || defined(__USE_EXTI15)) && !defined(__N_ISR_EXTI15)) )
void EXTI15_10_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
 
#   if ((defined(__ISR_EXTI10) || defined(__USE_EXTI10)) && !defined(__N_ISR_EXTI10))
    __EXTI_MACRO(10);
#   endif
#   if ((defined(__ISR_EXTI11) || defined(__USE_EXTI11)) && !defined(__N_ISR_EXTI11))
    __EXTI_MACRO(11);
#   endif
#   if ((defined(__ISR_EXTI12) || defined(__USE_EXTI12)) && !defined(__N_ISR_EXTI12))
    __EXTI_MACRO(12);
#   endif
#   if ((defined(__ISR_EXTI13) || defined(__USE_EXTI13)) && !defined(__N_ISR_EXTI13))
    __EXTI_MACRO(13);
#   endif
#   if ((defined(__ISR_EXTI14) || defined(__USE_EXTI14)) && !defined(__N_ISR_EXTI14))
    __EXTI_MACRO(14);
#   endif
#   if ((defined(__ISR_EXTI15) || defined(__USE_EXTI15)) && !defined(__N_ISR_EXTI15))
    __EXTI_MACRO(15);
#   endif
    { } //to terminate "else"

    __ISR_EXIT_HOOK();
}
#endif


/// Open DMA interrupts:
/// If you are using a DMA, the interrupt is available unless you declare
/// __N_ISR_DMAX_ChannelY in your app.  MEMCPY uses a DMA without interrupt, so
/// you'll see below how to force-off a DMA channel interrupt.
#if (defined(__USE_DMA1_CHAN1) || defined(__ISR_DMA1_Channel1)) && !defined(__N_ISR_DMA1_Channel1)
void DMA1_Channel1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma1ch1();
    __ISR_EXIT_HOOK();
}
#endif
#if (defined(__USE_DMA1_CHAN2) || defined(__ISR_DMA1_Channel2)) && !defined(__N_ISR_DMA1_Channel2)
void DMA1_Channel2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma1ch2();
    __ISR_EXIT_HOOK();
}
#endif
#if (defined(__USE_DMA1_CHAN3) || defined(__ISR_DMA1_Channel3)) && !defined(__N_ISR_DMA1_Channel3)
void DMA1_Channel2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma1ch3();
    __ISR_EXIT_HOOK();
}
#endif
#if (defined(__USE_DMA1_CHAN4) || defined(__ISR_DMA1_Channel4)) && !defined(__N_ISR_DMA1_Channel4)
void DMA1_Channel4_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma1ch4();
    __ISR_EXIT_HOOK();
}
#endif
#if (defined(__USE_DMA1_CHAN4) || defined(__ISR_DMA1_Channel4)) && !defined(__N_ISR_DMA1_Channel4)
void DMA1_Channel4_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma1ch4();
    __ISR_EXIT_HOOK();
}
#endif
#if (defined(__USE_DMA1_CHAN5) || defined(__ISR_DMA1_Channel5)) && !defined(__N_ISR_DMA1_Channel5)
void DMA1_Channel5_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma1ch5();
    __ISR_EXIT_HOOK();
}
#endif
#if (defined(__USE_DMA1_CHAN6) || defined(__ISR_DMA1_Channel6)) && !defined(__N_ISR_DMA1_Channel6)
void DMA1_Channel6_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma1ch6();
    __ISR_EXIT_HOOK();
}
#endif
#if (defined(__USE_DMA1_CHAN7) || defined(__ISR_DMA1_Channel7)) && !defined(__N_ISR_DMA1_Channel7)
void DMA1_Channel7_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma1ch7();
    __ISR_EXIT_HOOK();
}
#endif
#if (defined(__USE_DMA2_CHAN1) || defined(__ISR_DMA2_Channel1)) && !defined(__N_ISR_DMA2_Channel1)
void DMA2_Channel1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma2ch1();
    __ISR_EXIT_HOOK();
}
#endif
#if (defined(__USE_DMA2_CHAN2) || defined(__ISR_DMA2_Channel2)) && !defined(__N_ISR_DMA2_Channel2)
void DMA2_Channel2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma2ch2();
    __ISR_EXIT_HOOK();
}
#endif

#if (defined(__USE_DMA2_CHAN3) || defined(__ISR_DMA2_Channel3)) && !defined(__N_ISR_DMA2_Channel3)
void DMA2_Channel2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma2ch3();
    __ISR_EXIT_HOOK();
}
#endif
#if (defined(__USE_DMA2_CHAN4) || defined(__ISR_DMA2_Channel4)) && !defined(__N_ISR_DMA2_Channel4)
void DMA2_Channel4_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma2ch4();
    __ISR_EXIT_HOOK();
}
#endif
#if (defined(__USE_DMA2_CHAN5) || defined(__ISR_DMA2_Channel5)) && !defined(__N_ISR_DMA2_Channel5)
void DMA2_Channel5_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma2ch5();
    __ISR_EXIT_HOOK();
}
#endif
#if (defined(__USE_DMA2_CHAN6) || defined(__ISR_DMA2_Channel6)) && !defined(__N_ISR_DMA2_Channel6)
void DMA2_Channel6_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma2ch6();
    __ISR_EXIT_HOOK();
}
#endif
#if (defined(__USE_DMA2_CHAN7) || defined(__ISR_DMA2_Channel7)) && !defined(__N_ISR_DMA2_Channel7)
void DMA2_Channel7_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma2ch7();
    __ISR_EXIT_HOOK();
}
#endif


/// Open ADC Interrupts
#define _ADC1   defined(__ISR_ADC1) && !defined(__N_ISR_ADC1)
#define _ADC2   defined(__ISR_ADC2) && !defined(__N_ISR_ADC2)
#if (_ADC1 || _ADC2)
void ADC1_2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    
#   if (_ADC1)
#       if (_ADC2)
        if ((ADC1->ISR & ADC1->IER) && (ADC1->CR & 1))
#       endif
            platform_isr_adc1();
#   endif
#   if (_ADC2)
#       if (_ADC1)
        if ((ADC2->ISR & ADC2->IER) && (ADC2->CR & 1))
#       endif
            platform_isr_adc2();
#   endif
    __ISR_EXIT_HOOK();
}
#endif
#undef _ADC1
#undef _ADC2






/// Open TIM1BRK or TIM15 interrupts
#define _TIM1BRK    defined(__ISR_TIM1BRK) && !defined(__N_ISR_TIM1BRK)
#define _TIM15      defined(__ISR_TIM15) && !defined(__N_ISR_TIM15)
#if (_TIM1BRK || _TIM15)
void TIM1_BRK_TIM15_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    
#   if (_TIM1BRK)
#       if (_TIM15)
        if ((TIM1->DIER & TIM1_DIER_BIE) && (TIM1->SR & (TIM1_SR_B2IF | TIM1_SR_BIF)) && (TIM1->CR1 & 1))
#       endif
            platform_isr_tim1brk();
#   endif
#   if (_TIM15)
#       if (_TIM1BRK)
        if ((TIM15->DIER & TIM15->SR & 0x00E7) && (TIM15->CR1 & 1))
#       endif
            platform_isr_tim15();
#   endif
    
    __ISR_EXIT_HOOK();
}
#endif
#undef _TIM1BRK
#undef _TIM15


/// Open TIM1UP or TIM16 interrupts
#define _TIM1UP     defined(__ISR_TIM1UP) && !defined(__N_ISR_TIM1UP)
#define _TIM16      defined(__ISR_TIM16) && !defined(__N_ISR_TIM16)
#if (_TIM1UP || _TIM16)
void TIM1_UP_TIM16_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    
#   if (_TIM1UP)
#       if (_TIM16)
        if ((TIM1->DIER & TIM1->SR & TIM1_DIER_UIE) && (TIM1->CR1 & 1))
#       endif
            platform_isr_tim1up();
#   endif
#   if (_TIM16)
#       if (_TIM1UP)
        if ((TIM16->DIER & TIM16->SR & 0x00E7) && (TIM16->CR1 & 1))
#       endif
            platform_isr_tim16();
#   endif
    
    __ISR_EXIT_HOOK();
}
#endif
#undef _TIM1UP
#undef _TIM16


/// Open TIM1_TRG_COM interrupt
#define _TIM1TRG    defined(__ISR_TIM1TRG) && !defined(__N_ISR_TIM1TRG)
#define _TIM1COM    defined(__ISR_TIM1COM) && !defined(__N_ISR_TIM1COM)
#if (_TIM1TRG || _TIM1COM)
void TIM1_TRG_COM_IRQHandler(void) {
    ot_u32 tim1_msk;
    
    __ISR_ENTRY_HOOK();
    
    tim1_msk = TIM1->DIER & TIM1->SR;
    
    if (tim1_msk & TIM1_DIER_COMIE)
        platform_isr_tim1com();

    if (tim1_msk & TIM1_DIER_TIE)
        platform_isr_tim1trg();
    
    __ISR_EXIT_HOOK();
}
#endif
#undef _TIM1TRG
#undef _TIM1COM


/// Open TIM1_CC interrupt
#if defined(__ISR_TIM1CC) && !defined(__N_ISR_TIM1CC)
void TIM1_CC_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim1cc();
    __ISR_EXIT_HOOK();
}
#endif


/// Open TIM2 interrupt
#if defined(__ISR_TIM2) && !defined(__N_ISR_TIM2)
void TIM2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim2();
    __ISR_EXIT_HOOK();
}
#endif


/// Open TIM3 interrupt
#if defined(__ISR_TIM3) && !defined(__N_ISR_TIM3)
void TIM3_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim3();
    __ISR_EXIT_HOOK();
}
#endif


/// Open LPTIM1 Interrupt
#if defined(__ISR_LPTIM1) && !defined(__N_ISR_LPTIM1)
void LPTIM1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    ///@todo Not certain this PR clear is necessary or good
    //EXTI->PR2 = (1<<(32-32));
    __ISR_KTIM_WAKEUP_HOOK();
    platform_isr_lptim1();
    __ISR_EXIT_HOOK();
}
#endif


/// Open LPTIM2 Interrupt
#if defined(__ISR_LPTIM2) && !defined(__N_ISR_LPTIM2)
void LPTIM2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    ///@todo Not certain this PR clear is necessary or good
    //EXTI->PR2 = (1<<(33-32));
    __ISR_KTIM_WAKEUP_HOOK();
    platform_isr_lptim2();
    __ISR_EXIT_HOOK();
}
#endif


///Open TIM6 & DAC Interrupts
#define _TIM6   defined(__ISR_TIM6) && !defined(__N_ISR_TIM6)
#define _DAC    defined(__ISR_DAC) && !defined(__N_ISR_DAC)
#if (_TIM6 || _DAC)
void TIM6_DAC_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
#   if (_TIM6)
#       if (_DAC)
        if ((TIM6->CR1 & TIM6->DIER & TIM6->SR) && (TIM6->CR1 & TIM6_CR1_CEN))
#       endif
            platform_isr_tim6();
#   endif
#   if (_DAC)
    {   
#       if (_TIM6)
        bool dac1_en    = DAC->CR & DAC_CR_EN1;
        bool dac2_en    = DAC->CR & DAC_CR_EN2;
        ot_u32 dac1_if  = DAC->CR & DAC->SR & DAC_CR_DMAUDRIE1;
        ot_u32 dac1_if  = DAC->CR & DAC->SR & DAC_CR_DMAUDRIE2;
        if ((dac1_en && dac1_if) || (dac2_en && dac2_if))
#       endif
            platform_isr_dac();
    }
#   endif
    __ISR_EXIT_HOOK();
}
#endif


///Open TIM7 Interrupts
#if defined(__ISR_TIM7) && !defined(__N_ISR_TIM7)
void TIM7_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim7();
    __ISR_EXIT_HOOK();
}
#endif


/// Open I2C1 interrupts 
#if defined(__USE_I2C1EV) || (defined(__ISR_I2C1EV) && !defined(__N_ISR_I2C1EV))
void I2C1_EV_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    EXTI->PR1 = (1<<23);
    __ISR_WAKEUP_HOOK();
    platform_isr_i2c1ev();
    __ISR_EXIT_HOOK();
}
#endif
#if defined(__USE_I2C1ER) || (defined(__ISR_I2C1ER) && !defined(__N_ISR_I2C1ER))
void I2C1_ER_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    platform_isr_i2c1er();
    __ISR_EXIT_HOOK();
}
#endif

/// Open I2C2 interrupts 
#if defined(__USE_I2C2EV) || (defined(__ISR_I2C2EV) && !defined(__N_ISR_I2C2EV))
void I2C2_EV_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    EXTI->PR1 = (1<<24);
    __ISR_WAKEUP_HOOK();
    platform_isr_i2c2ev();
    __ISR_EXIT_HOOK();
}
#endif
#if defined(__USE_I2C2ER) || (defined(__ISR_I2C2ER) && !defined(__N_ISR_I2C2ER))
void I2C2_ER_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    platform_isr_i2c2er();
    __ISR_EXIT_HOOK();
}
#endif

/// Open I2C3 interrupts 
#if defined(__USE_I2C3EV) || (defined(__ISR_I2C3EV) && !defined(__N_ISR_I2C3EV))
void I2C3_EV_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    EXTI->PR1 = (1<<25);
    __ISR_WAKEUP_HOOK();
    platform_isr_i2c3ev();
    __ISR_EXIT_HOOK();
}
#endif
#if defined(__USE_I2C3ER) || (defined(__ISR_I2C3ER) && !defined(__N_ISR_I2C3ER))
void I2C3_ER_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    platform_isr_i2c3er();
    __ISR_EXIT_HOOK();
}
#endif

/// Open I2C4 interrupts 
#if defined(__USE_I2C4EV) || (defined(__ISR_I2C4EV) && !defined(__N_ISR_I2C4EV))
void I2C4_EV_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    EXTI->PR2 = (1<<(40-32));
    __ISR_WAKEUP_HOOK();
    platform_isr_i2c4ev();
    __ISR_EXIT_HOOK();
}
#endif
#if defined(__USE_I2C4ER) || (defined(__ISR_I2C4ER) && !defined(__N_ISR_I2C4ER))
void I2C4_ER_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    platform_isr_i2c4er();
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
#if defined(__ISR_SPI3) && !defined(__N_ISR_SPI3)
void SPI3_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_spi3();
    __ISR_EXIT_HOOK();
}
#endif


/// Open USART interrupts
#if defined(__ISR_USART1) && !defined(__N_ISR_USART1)
void USART1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    EXTI->PR1 = (1<<26);
    __ISR_WAKEUP_HOOK();
    platform_isr_usart1();
    __ISR_EXIT_HOOK();
}
#endif
#if defined(__ISR_USART2) && !defined(__N_ISR_USART2)
void USART2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    EXTI->PR1 = (1<<27);
    __ISR_WAKEUP_HOOK();
    platform_isr_usart2();
    __ISR_EXIT_HOOK();
}
#endif
#if defined(__ISR_USART3) && !defined(__N_ISR_USART3)
void USART3_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    EXTI->PR1 = (1<<28);
    __ISR_WAKEUP_HOOK();
    platform_isr_usart3();
    __ISR_EXIT_HOOK();
}
#endif
#if defined(__ISR_UART4) && !defined(__N_ISR_UART4)
void UART4_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    EXTI->PR1 = (1<<29);
    __ISR_WAKEUP_HOOK();
    platform_isr_uart4();
    __ISR_EXIT_HOOK();
}
#endif


/// Open SD-MMC Interrupts: Only available on some parts
#if defined(__ISR_SDMMC) && !defined(__N_ISR_SDMMC)
void SDMMC_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_sdmmc();
    __ISR_EXIT_HOOK();
}
#endif


/// Open COMPARATOR Interrupts
#if defined(__ISR_COMP) && !defined(__N_ISR_COMP)
void COMP_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    {   ot_u32 exti_pr;
        exti_pr     = EXTI->PR1;
        EXTI->PR1    = (1<<21) | (1<<22);
        if (exti_pr & (1<<21))  platform_isr_comp1();
        if (exti_pr & (1<<22))  platform_isr_comp2();
    }
    __ISR_EXIT_HOOK();
}
#endif


/// Open LPUART interrupt
#if defined(__ISR_LPUART1) && !defined(__N_ISR_LPUART1)
void RNG_LPUART1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    EXTI->PR1 = (1<<31);
    __ISR_WAKEUP_HOOK();
    platform_isr_lpuart1();
    __ISR_EXIT_HOOK();
}
#endif


///Open USB Interrupts
#if defined(__ISR_USB) && !defined(__N_ISR_USB)
void USBFS_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    EXTI->PR1 = (1<<17);
    __ISR_WAKEUP_HOOK();
    platform_isr_usbfs();
    __ISR_EXIT_HOOK();
}
#endif


///Open Quad-SPI Interrupts
#if defined(__ISR_QUADSPI) && !defined(__N_ISR_QUADSPI)
void QUADSPI_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_quadspi();
    __ISR_EXIT_HOOK();
}
#endif


///Open SAI1 Interrupts
#if defined(__ISR_SAI1) && !defined(__N_ISR_SAI1)
void SAI1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_sai1();
    __ISR_EXIT_HOOK();
}
#endif


///Open SWPMI1 Interrupts
#if defined(__ISR_SWPMI1) && !defined(__N_ISR_SWPMI1)
void SWPMI1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_swpmi1();
    __ISR_EXIT_HOOK();
}
#endif


/// Open Touch Screen Controller (TSC) Interrupt
#if defined(__ISR_TSC) && !defined(__N_ISR_TSC)
void TSC_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tsc();
    __ISR_EXIT_HOOK();
}
#endif


/// Open LCD interface interrupt
#if defined(__ISR_LCD) && !defined(__N_ISR_LCD)
void LCD_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    EXTI->PR = (1<<39);
    __ISR_WAKEUP_HOOK();
    platform_isr_lcd();
    __ISR_EXIT_HOOK();
}
#endif


/// Open AES Interrupt
#if defined(__ISR_AES) && !defined(__N_ISR_AES)
void AES_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_aes();
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


/// Open FPU Interrupt
#if defined(__ISR_FPU) && !defined(__N_ISR_FPU)
void FPU_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_fpu();
    __ISR_EXIT_HOOK();
}
#endif


/// Open CRS Interrupt
#if defined(__ISR_CRS) && !defined(__N_ISR_CRS)
void CRS_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_crs();
    __ISR_EXIT_HOOK();
}
#endif


#endif  // if defined(__STM32L__)






