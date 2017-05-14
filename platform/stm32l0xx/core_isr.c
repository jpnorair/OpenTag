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
  * @file       /platform/stm32l0xx/core_isr.c
  * @author     JP Norair
  * @version    R100
  * @date       16 Sept 2014
  * @brief      ISRs abstracted by the platform module
  * @ingroup    Platform
  *
  *
  ******************************************************************************
  */


#include <otplatform.h>
#if defined(__STM32L0__)

#include <otstd.h>
#include <app/isr_config_STM32L0.h>      // Must be in your app distribution!







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
//#           undef __USE_DMA1_CHAN6
//#           undef __ISR_USART2
//#           define __ISR_USART2
#       elif (MPIPE_UART_ID == 3)
//#           undef __USE_DMA1_CHAN2
//#           undef __ISR_USART3
//#           define __ISR_USART3
#       endif

#   else
#       error "MPIPE is not enabled on a known communication interface."
#   endif

#else
/*
#   if ((MPIPE_DMA_TXCHAN_ID == 1) || (MPIPE_DMA_RXCHAN_ID == 1))
#       undef __USE_DMA1_CHAN1
#   endif
#   if ((MPIPE_DMA_TXCHAN_ID == 2) || (MPIPE_DMA_RXCHAN_ID == 2))
#       undef __USE_DMA1_CHAN2
#   endif
#   if ((MPIPE_DMA_TXCHAN_ID == 3) || (MPIPE_DMA_RXCHAN_ID == 3))
#       undef __USE_DMA1_CHAN3
#   endif
#   if ((MPIPE_DMA_TXCHAN_ID == 4) || (MPIPE_DMA_RXCHAN_ID == 4))
#       undef __USE_DMA1_CHAN4
#   endif
#   if ((MPIPE_DMA_TXCHAN_ID == 5) || (MPIPE_DMA_RXCHAN_ID == 5))
#       undef __USE_DMA1_CHAN5
#   endif
#   if ((MPIPE_DMA_TXCHAN_ID == 6) || (MPIPE_DMA_RXCHAN_ID == 6))
#       undef __USE_DMA1_CHAN6
#   endif
#   if ((MPIPE_DMA_TXCHAN_ID == 7) || (MPIPE_DMA_RXCHAN_ID == 7))
#       undef __USE_DMA1_CHAN7
#   endif
*/
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
#   undef __ISR_RF1A
#   define __ISR_RF1A
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
void PVD_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    EXTI->PR = (1<<16);
    __ISR_WAKEUP_HOOK();
    platform_isr_pvd();
    __ISR_EXIT_HOOK();
}
#endif




/// Open RTC interrupt, which has 4 synthetic interrupts    
#define _RTCALARM   (defined(__ISR_RTC_Alarm) && !defined(__N_ISR_RTC_Alarm))
#define _RTCWKUP    (defined(__ISR_RTC_WKUP) && !defined(__N_ISR_RTC_WKUP)) 
#define _TAMPER     (defined(__ISR_TAMPER_STAMP) && !defined(__N_ISR_TAMPER_STAMP))
#define _CSSLSE     (defined(__ISR_CSSLSE) && !defined(__N_ISR_CSSLSE))
#if (_RTCALARM || _RTCWKUP || _TAMPER || CSSLSE)
void RTC_IRQHandler(void) {
    ot_u32 exti_pr;
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    exti_pr     = EXTI->PR;
    EXTI->PR    = (1<<17) | (1<<19) | (1<<20);
    RTC->ISR   &= ~(RTC_ISR_RSF | RTC_ISR_WUTF);
    
#   if (CSSLSE)
    if (RCC->CIER & RCC->CIFR & RCC_CIFR_CSSLSEF) {
        platform_isr_csslse();
    }
#   endif
#   if (_RTCWKUP)
    if (exti_pr & (1<<20)) 
        platform_isr_rtcwakeup();
#   endif
#   if (_RTCALARM)
    if (exti_pr & (1<<17))
        // platform_isr_rtcalarm must clear its own flags in RTC->ISR
        platform_isr_rtcalarm();
#   endif
#   if (_TAMPER) 
    if (exti_pr & (1<<19)) 
        // platform_isr_tamperstamp must clear its own flags in RTC->ISR
        platform_isr_tamperstamp();
#   endif

    __ISR_EXIT_HOOK();
}
#endif





/// Open FLASH interrupt (todo)
#if defined(__ISR_FLASH) && !defined(__N_ISR_FLASH)
void FLASH_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_flash();
    __ISR_EXIT_HOOK();
}
#endif




/// Open RCC_CRS interrupt
/// The individual RCC and CRS interrupts are synthetic
#define _RCC    (defined(__ISR_RCC) && !defined(__N_ISR_RCC))
#define _CRS    (defined(__ISR_CRS) && !defined(__N_ISR_CRS))
#if (_RCC || _CRS)
void RCC_CRS_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
#   if (_RCC)
    if (RCC->CIER & RCC->CIFR & 0x017F)
        platform_isr_rcc();
#   endif
#   if (_CRS)
    if (CRS->CR & CRS->ISR & (CRS_ESYNCIE | CRS_ERRIE | CRS_SYNCWARNIE | CRS_SYNCOKIE))
        platform_isr_crs();
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


#if (OT_FEATURE(M2))
#   define __RADIO_EXTI(NUM)    BOARD_RADIO_EXTI##NUM##_ISR()
#else
#   define __RADIO_EXTI(NUM); 
#endif
#if (BOARD_FEATURE(MPIPE) && defined(BOARD_COM_EXTI0))
#   define __MPIPE_EXTI(NUM)    BOARD_COM_EXTI##NUM##_ISR()
#elif (BOARD_FEATURE(MPIPE) && defined(BOARD_MPIPE_EXTI0))
#   define __MPIPE_EXTI(NUM)    BOARD_MPIPE_EXTI##NUM##_ISR()
#else
#   define __MPIPE_EXTI(NUM); 
#endif



#define __EXTI_MACRO_LOW(NUM);  \
    EXTI->PR = (1<<NUM);  \
    __RADIO_EXTI(NUM); \
    __MPIPE_EXTI(NUM); \
    APPLICATION_EXTI##NUM##_ISR();
    
#define __EXTI_MACRO(NUM);   \
    if (EXTI->PR & (1<<NUM)) { \
        EXTI->PR = (1<<NUM);  \
        __RADIO_EXTI(NUM); \
        __MPIPE_EXTI(NUM); \
        APPLICATION_EXTI##NUM##_ISR(); \
    } \
    else



#if (  ((defined(__ISR_EXTI0) || defined(__USE_EXTI0)) && !defined(__N_ISR_EXTI0)) \
    || ((defined(__ISR_EXTI1) || defined(__USE_EXTI1)) && !defined(__N_ISR_EXTI1)) )
void EXTI0_1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    
#   if ((defined(__ISR_EXTI0) || defined(__USE_EXTI0)) && !defined(__N_ISR_EXTI0))
    __EXTI_MACRO(0);
#   endif
#   if ((defined(__ISR_EXTI1) || defined(__USE_EXTI1)) && !defined(__N_ISR_EXTI1))
    __EXTI_MACRO(1);
#   endif
    { } //to terminate "else"

    __ISR_EXIT_HOOK();
}
#endif

#if (  ((defined(__ISR_EXTI2) || defined(__USE_EXTI2)) && !defined(__N_ISR_EXTI2)) \
    || ((defined(__ISR_EXTI3) || defined(__USE_EXTI3)) && !defined(__N_ISR_EXTI3)) )
void EXTI2_3_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    
#   if ((defined(__ISR_EXTI2) || defined(__USE_EXTI2)) && !defined(__N_ISR_EXTI2))
    __EXTI_MACRO(2);
#   endif
#   if ((defined(__ISR_EXTI3) || defined(__USE_EXTI3)) && !defined(__N_ISR_EXTI3))
    __EXTI_MACRO(3);
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
void EXTI4_15_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();

#   if ((defined(__ISR_EXTI4) || defined(__USE_EXTI4)) && !defined(__N_ISR_EXTI4))
    __EXTI_MACRO(4);
#   endif
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
#   if ((defined(__ISR_EXTI10) || defined(__USE_EXTI10)) && !defined(__N_ISR_EXTI10))
    //__EXTI_MACRO(10);
    if (EXTI->PR & (1<<10)) { \
            EXTI->PR = (1<<10);  
            ubxdrv_rxsync_isr();
        } 
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




/// Open Touch Screen Controller (TSC) Interrupt
#if defined(__ISR_TSC) && !defined(__N_ISR_TSC)
void TSC_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tsc();
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

#define _DMA2   ((defined(__USE_DMA1_CHAN2) || defined(__ISR_DMA1_Channel2)) && !defined(__N_ISR_DMA1_Channel2))
#define _DMA3   ((defined(__USE_DMA1_CHAN3) || defined(__ISR_DMA1_Channel3)) && !defined(__N_ISR_DMA1_Channel3))
#if (_DMA2 || _DMA3)
void DMA1_Channel2_3_IRQHandler(void) {
    ot_u32 chan_x;
    __ISR_ENTRY_HOOK();
#   if (_DMA2)
    chan_x = DMA->CCR2;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA->ISR & (1<<4))
        platform_isr_dma1ch2();
#   endif
#   if (_DMA3)
    chan_x = DMA->CCR3;
    if ((chan_x & 1) && (chan_x & 0x000E) && (DMA->ISR & (1<<8))
        platform_isr_dma1ch3();
#   endif
    __ISR_EXIT_HOOK();
}
#endif

#define _DMA4   ((defined(__USE_DMA1_CHAN4) || defined(__ISR_DMA1_Channel4)) && !defined(__N_ISR_DMA1_Channel4))
#define _DMA5   ((defined(__USE_DMA1_CHAN5) || defined(__ISR_DMA1_Channel5)) && !defined(__N_ISR_DMA1_Channel5))
#define _DMA6   ((defined(__USE_DMA1_CHAN6) || defined(__ISR_DMA1_Channel6)) && !defined(__N_ISR_DMA1_Channel6))
#define _DMA7   ((defined(__USE_DMA1_CHAN7) || defined(__ISR_DMA1_Channel7)) && !defined(__N_ISR_DMA1_Channel7))
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





/// Open ADC & COMPARATOR Interrupts
/// OpenTag Kernel uses DMA with ADC, if/when if uses ADC
#define _ADC1   defined(__ISR_ADC1) && !defined(__N_ISR_ADC1)
#define _COMP   defined(__ISR_COMP) && !defined(__N_ISR_COMP)
#if (_ADC1 || _COMP)
void ADC1_COMP_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
#   if (_COMP)
    __ISR_WAKEUP_HOOK();
    {   ot_u32 exti_pr;
        exti_pr     = EXTI->PR;
        EXTI->PR    = (1<<21) | (1<<22);
        if (exti_pr & (1<<21))  platform_isr_comp1();
        if (exti_pr & (1<<22))  platform_isr_comp2();
    }
#   endif
#   if (_ADC1)
        if ((ADC1->ISR & ADC1->IER) && (ADC1->CR & 1))
            platform_isr_adc1();
#   endif
    __ISR_EXIT_HOOK();
}
#endif



/// Open LPTIM Interrupt
#if defined(__ISR_LPTIM1) && !defined(__N_ISR_LPTIM1)
void LPTIM1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    
    ///@todo Not certain this PR clear is necessary or good
    EXTI->PR = (1<<29);         
    
    __ISR_KTIM_WAKEUP_HOOK();
    platform_isr_lptim1();
    __ISR_EXIT_HOOK();
}
#endif





/// Open LCD interface interrupts (todo)
#if defined(__ISR_LCD) && !defined(__N_ISR_LCD)
void LCD_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_lcd();
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
        if (TIM6->CR1 & TIM6->DIER & TIM6->SR)
            platform_isr_tim6()
#   endif
#   if (_DAC)
    {   ot_u32 dac_cr;
        dac_cr  = DAC->CR;
        if ((DAC->CR & (DAC_CR_DMAEN1 | DAC_CR_EN1)) && (DAC->SR & DAC->CR))
            platform_isr_dac();
    }
#   endif
    __ISR_EXIT_HOOK();
}
#endif



/// Open TIM2, TIM21, TIM22 timer interrupts
#if defined(__ISR_TIM2) && !defined(__N_ISR_TIM2)
void TIM2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim2();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_TIM21) && !defined(__N_ISR_TIM21)
void TIM21_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim21();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_TIM22) && !defined(__N_ISR_TIM22)
void TIM22_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim22();
    __ISR_EXIT_HOOK();
}
#endif








/// Open I2C interrupts 
#if defined(__USE_I2C1) || (defined(__ISR_I2C1) && !defined(__N_ISR_I2C1))
void I2C1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    EXTI->PR = (1<<23);
    __ISR_WAKEUP_HOOK();
    platform_isr_i2c1();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__USE_I2C2) || (defined(__ISR_I2C2) && !defined(__N_ISR_I2C2))
void I2C2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_i2c2();
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
    EXTI->PR = (1<<25);
    __ISR_WAKEUP_HOOK();
    platform_isr_usart1();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_USART2) && !defined(__N_ISR_USART2)
void USART2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    EXTI->PR = (1<<26);
    __ISR_WAKEUP_HOOK();
    platform_isr_usart2();
    __ISR_EXIT_HOOK();
}
#endif







/// Open AES / RNG / LPUART interrupts
#define _AES    defined(__ISR_AES) && !defined(__N_ISR_AES)
#define _RNG    defined(__ISR_RNG) && !defined(__N_ISR_RNG)
#define _LPUART defined(__ISR_LPUART) && !defined(__N_ISR_LPUART)
#if (_AES || _RNG || LPUART)
void RNG_LPUART1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
#   if (_LPUART)
    {   ot_u32 exti_pr;
        exti_pr = EXTI->PR;
        if (exti_pr & (1<<28)) {
            EXTI->PR = (1<<28);
            __ISR_WAKEUP_HOOK();
            platform_isr_lpuart();
        }
    }
#   endif
#   if (_AES)
    {   ot_u32 aes_cr;
        aes_cr = AES->CR;
        if ((aes_cr & (AES_CR_EN)) \
        &&  (aes_cr & (AES_CR_ERRIE | AES_CR_CCFIE)) \
        &&  (AES->SR != 0))
            platform_isr_aes();
    }
#   endif
#   if (_RNG)
        if ((RNG->CR == (RNG_CR_IE | RNG_CR_RNGEN)) && (RNG->SR != 0))
            platform_isr_rng();
#   endif
    __ISR_EXIT_HOOK();
}
#endif





///Open USB Interrupts (todo)
#if defined(__ISR_USB) && !defined(__N_ISR_USB)
void USB_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    EXTI->PR = (1<<18);
    __ISR_WAKEUP_HOOK();
    platform_isr_usb(); //HAL_PCD_IRQHandler(&hpcd);
    __ISR_EXIT_HOOK();
}
#endif




#endif  // if defined(__STM32L__)






