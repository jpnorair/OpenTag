/* Copyright 2010-2013 JP Norair
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
  * @file       /otplatform/stm32l1xx/platform_isr_STM32L.c
  * @author     JP Norair
  * @version    R100
  * @date       16 Jan 2013
  * @brief      ISRs abstracted by the platform module
  * @ingroup    Platform
  *
  *
  ******************************************************************************
  */


#include "OT_platform.h"
#if defined(__STM32L__)

#include "isr_config_STM32L.h"

#include "OTAPI.h"






/** Platform-Managed ISRs   <BR>
  * ========================================================================<BR>
  * The constants of the form __ISR_XXX should be defined per-application in:
  * /apps/[YOURAPP]/code/isr_config_[YOURPLATFORM].h.  Define __ISR_XXX
  * constants based on the ISRs you want to enable.  In some cases, OpenTag
  * will need a certain ISR, and this file will declare an ISR even if it is 
  * defined by __ISR_XXX.  For example, Kernel Timer and Radio I/F interrupts
  * are almost always going to be needed.
  */

#if (MCU_FEATURE_USB)
#   undef __ISR_USB_LP
#   undef __ISR_USB_FS_WKUP
#   define __ISR_USB_LP
#   define __ISR_USB_FS_WKUP
#endif

#define __ISR_ENTRY_HOOK(); 
#define __ISR_EXIT_HOOK(); 

/// RTC Alarm interrupt is required by OpenTag in non-debug modes
#undef __ISR_RTC_Alarm
#undef __N_ISR_RTC_Alarm
#define __ISR_RTC_Alarm


/// ISRs that can bring the system out of STOP mode have __ISR_WAKEUP_HOOK().
/// When coming out of STOP, clock is always MSI.  If Multispeed clocking is
/// disabled and a non-MSI clock is the clock source, it must be turned-on.
#if (BOARD_FEATURE_STDSPEED)
#   define __ISR_KTIM_WAKEUP_HOOK()     platform_standard_speed();
#elif (BOARD_FEATURE_FULLSPEED)
#   define __ISR_KTIM_WAKEUP_HOOK()     platform_full_speed();
#elif (BOARD_FEATURE_FLANKSPEED)
#   define __ISR_KTIM_WAKEUP_HOOK()     platform_flank_speed();
#endif

#if defined(__DEBUG__)
#   undef __ISR_RTC_Alarm
#   define __ISR_TIM9
#   define __ISR_WAKEUP_HOOK();

#else 
#   define __ISR_WAKEUP_HOOK()  do { gptim_start_chrono(); __ISR_KTIM_WAKEUP_HOOK(); } while(0);

#endif







/// Open SPI interrupts:



/// Enable MPIPE Interrupts:
/// DMA interrupts for UART DMA
#if (OT_FEATURE(MPIPE))
#   if (MCU_FEATURE(MPIPECDC))
        ///@todo USB MPipe Interrupt settings
   
#   elif (MCU_FEATURE(MPIPEUART))
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
#       error "MPIPE enabled in app configuration, but not enabled on this board."
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





/// Enable RF Module interrupts: 
/// In every known case, these are :
/// RTC Wakeup interrupt is required by OpenTag for advertisement flooding
#if (OT_FEATURE(M2))
#   undef __ISR_RF1A
#   define __ISR_RF1A

//#   undef __ISR_RTC_WKUP
//#   undef __N_ISR_RTC_WKUP
//#   define __ISR_RTC_WKUP
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
    __ISR_WAKEUP_HOOK();
    platform_isr_pvd();
    __ISR_EXIT_HOOK();
}
#endif




/// Open RTC interrupts

#if defined(__ISR_TAMPER_STAMP) && !defined(__N_ISR_TAMPER_STAMP)
void TAMPER_STAMP_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    RTC->ISR &= ~RTC_ISR_RSF;
    __ISR_WAKEUP_HOOK();
    platform_isr_tamper();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_RTC_WKUP) && !defined(__N_ISR_RTC_WKUP)
void RTC_WKUP_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    //RTC->ISR &= ~RTC_ISR_RSF;
    EXTI->PR    = (1<<20);
    RTC->ISR   &= ~RTC_ISR_WUTF;
    //__ISR_WAKEUP_HOOK();          //OpenTag doesn't use this INT for STOP wakeup
    platform_isr_rtcwakeup();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_RTC_Alarm) && !defined(__N_ISR_RTC_Alarm)
void RTC_Alarm_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    RTC->ISR &= ~RTC_ISR_RSF;
    __ISR_KTIM_WAKEUP_HOOK();
    platform_isr_rtcalarm();
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




/// Open RCC interrupt (todo)

#if defined(__ISR_RCC) && !defined(__N_ISR_RCC)
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
#if defined(__ISR_EXTI0)
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
#   define __EXTI_MACRO_LOW(NUM);  \
    EXTI->PR = (1<<NUM);  \
    BOARD_RADIO_EXTI##NUM##_ISR(); \
    BOARD_COM_EXTI##NUM##_ISR(); \
    APPLICATION_EXTI##NUM##_ISR();
    
#   define __EXTI_MACRO(NUM);   \
    if (EXTI->PR & (1<<NUM)) { \
        EXTI->PR = (1<<NUM);  \
        BOARD_RADIO_EXTI##NUM##_ISR(); \
        BOARD_COM_EXTI##NUM##_ISR(); \
        APPLICATION_EXTI##NUM##_ISR(); \
    } \
    else
        
#else
#   define __EXTI_MACRO_LOW(NUM);  \
    EXTI->PR = (1<<NUM);  \
    BOARD_COM_EXTI##NUM##_ISR(); \
    APPLICATION_EXTI##NUM##_ISR();
    
#   define __EXTI_MACRO(NUM);   \
    if (EXTI->PR & (1<<NUM)) { \
        EXTI->PR = (1<<NUM);  \
        BOARD_COM_EXTI##NUM##_ISR(); \
        APPLICATION_EXTI##NUM##_ISR(); \
    } \
    else
    
#endif



#if (defined(__ISR_EXTI0) || defined(__USE_EXTI0)) && !defined(__N_ISR_EXTI0)
void EXTI0_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    __EXTI_MACRO_LOW(0);
    __ISR_EXIT_HOOK();
}
#endif

#if (defined(__ISR_EXTI1) || defined(__USE_EXTI1)) && !defined(__N_ISR_EXTI1)
void EXTI1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    __EXTI_MACRO_LOW(1);
    __ISR_EXIT_HOOK();
}
#endif

#if (defined(__ISR_EXTI2) || defined(__USE_EXTI2)) && !defined(__N_ISR_EXTI2)
void EXTI2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    __EXTI_MACRO_LOW(2);
    __ISR_EXIT_HOOK();
}
#endif

#if (defined(__ISR_EXTI3) || defined(__USE_EXTI3)) && !defined(__N_ISR_EXTI3)
void EXTI3_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    __EXTI_MACRO_LOW(3);
    __ISR_EXIT_HOOK();
}
#endif

#if (defined(__ISR_EXTI4) || defined(__USE_EXTI4)) && !defined(__N_ISR_EXTI4)
void EXTI4_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    __EXTI_MACRO_LOW(4);
    __ISR_EXIT_HOOK();
}
#endif
      

#if (  ((defined(__ISR_EXTI5) || defined(__USE_EXTI5)) && !defined(__N_ISR_EXTI5)) \
    || ((defined(__ISR_EXTI6) || defined(__USE_EXTI6)) && !defined(__N_ISR_EXTI6)) \
    || ((defined(__ISR_EXTI7) || defined(__USE_EXTI7)) && !defined(__N_ISR_EXTI7)) \
    || ((defined(__ISR_EXTI8) || defined(__USE_EXTI8)) && !defined(__N_ISR_EXTI8)) \
    || ((defined(__ISR_EXTI9) || defined(__USE_EXTI9)) && !defined(__N_ISR_EXTI9)) )
void EXTI9_5_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    
#   if ((defined(__ISR_EXTI5) || defined(__USE_EXTI5)) && !defined(__N_ISR_EXTI5))
    __EXTI_MACRO(5);
#   endif
#   if ((defined(__ISR_EXTI6) || defined(__USE_EXTI6)) && !defined(__N_ISR_EXTI6))
    //__EXTI_MACRO(6);
    if (EXTI->PR & (1<<6)) { 
        EXTI->PR = (1<<6);  
        BOARD_RADIO_EXTI6_ISR(); 
        APPLICATION_EXTI6_ISR(); 
    } 
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
void DMA1_Channel3_IRQHandler(void) {
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

#if (defined(__ISR_DMA2_Channel1) && !defined(__N_ISR_DMA2_Channel1))
void DMA2_Channel1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma2ch1();
    __ISR_EXIT_HOOK();
}
#endif

#if (defined(__ISR_DMA2_Channel2) && !defined(__N_ISR_DMA2_Channel2))
void DMA2_Channel2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma2ch2();
    __ISR_EXIT_HOOK();
}
#endif

#if (defined(__ISR_DMA2_Channel3) && !defined(__N_ISR_DMA2_Channel3))
void DMA2_Channel3_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma2ch3();
    __ISR_EXIT_HOOK();
}
#endif

#if (defined(__ISR_DMA2_Channel4) && !defined(__N_ISR_DMA2_Channel4))
void DMA2_Channel4_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma2ch4();
    __ISR_EXIT_HOOK();
}
#endif

#if (defined(__ISR_DMA2_Channel5) && !defined(__N_ISR_DMA2_Channel5))
void DMA2_Channel5_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dma2ch5();
    __ISR_EXIT_HOOK();
}
#endif






///Open ADC Interrupts (todo)

#if defined(__ISR_ADC1) && !defined(__N_ISR_ADC1)
void ADC1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_adc1();
    __ISR_EXIT_HOOK();
}
#endif




///Open USB Interrupts (todo)

#if defined(__ISR_USB_HP) && !defined(__N_ISR_USB_HP)
void USB_HP_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_usbhp();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_USB_LP) && !defined(__N_ISR_USB_LP)
void USB_LP_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_usblp();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_USB_FS_WKUP) && !defined(__N_ISR_USB_FS_WKUP)
void USB_FS_WKUP_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    EXTI->PR = (1<<18);     //clear pending bit
    platform_isr_fswkup();
    __ISR_EXIT_HOOK();
}
#endif




///Open DAC Interrupts (todo)

#if defined(__ISR_DAC) && !defined(__N_ISR_DAC)
void DAC_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_dac();
    __ISR_EXIT_HOOK();
}
#endif



///Open COMP[arator] Interrupts (todo)

#if defined(__ISR_COMP) && !defined(__N_ISR_COMP)
void COMP_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    platform_isr_comp();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_COMP_ACQ) && !defined(__N_ISR_COMP_ACQ)
void COMP_ACQ_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    __ISR_WAKEUP_HOOK();
    platform_isr_compacq();
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




/// Open TIM (timer) interrupts (todo)

#if defined(__ISR_TIM2) && !defined(__N_ISR_TIM2)
void TIM2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim2();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_TIM3) && !defined(__N_ISR_TIM3)
void TIM3_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim3();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_TIM4) && !defined(__N_ISR_TIM4)
void TIM4_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim4();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_TIM5) && !defined(__N_ISR_TIM5)
void TIM5_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim5();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_TIM6) && !defined(__N_ISR_TIM6)
void TIM6_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim6();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_TIM7) && !defined(__N_ISR_TIM7)
void TIM7_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim7();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_TIM9) && !defined(__N_ISR_TIM9)
void TIM9_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim9();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_TIM10) && !defined(__N_ISR_TIM10)
void TIM10_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim10();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_TIM11) && !defined(__N_ISR_TIM11)
void TIM11_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_tim11();
    __ISR_EXIT_HOOK();
}
#endif






/// Open I2C interrupts (todo)

#if defined(__USE_I2C1) || (defined(__ISR_I2C1_EV) && !defined(__N_ISR_I2C1_EV))
void I2C1_EV_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_i2c1ev();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__USE_I2C1) || (defined(__ISR_I2C1_ER) && !defined(__N_ISR_I2C1_ER))
void I2C1_ER_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_i2c1er();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__USE_I2C2) || (defined(__ISR_I2C2_EV) && !defined(__N_ISR_I2C2_EV))
void I2C2_EV_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_i2c2ev();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__USE_I2C2) || (defined(__ISR_I2C2_ER) && !defined(__N_ISR_I2C2_ER))
void I2C2_ER_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_i2c2er();
    __ISR_EXIT_HOOK();
}
#endif





/// Open SPI interrupts (todo)

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




/// Open USART & UART interrupts (todo)

#if defined(__ISR_USART1) && !defined(__N_ISR_USART1)
void USART1_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_usart1();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_USART2) && !defined(__N_ISR_USART2)
void USART2_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_usart2();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_USART3) && !defined(__N_ISR_USART3)
void USART3_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_usart3();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_UART4) && !defined(__N_ISR_UART4)
void UART4_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_uart4();
    __ISR_EXIT_HOOK();
}
#endif

#if defined(__ISR_UART5) && !defined(__N_ISR_UART5)
void UART5_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_uart5();
    __ISR_EXIT_HOOK();
}
#endif








/// Open AES engine interrupts (todo)

#if defined(__ISR_AES) && !defined(__N_ISR_AES)
void AES_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_aes();
    __ISR_EXIT_HOOK();
}
#endif





/// Open SD interface interrupts (todo)

#if defined(__ISR_SDIO) && !defined(__N_ISR_SDIO)
void SDIO_IRQHandler(void) {
    __ISR_ENTRY_HOOK();
    platform_isr_sdio();
    __ISR_EXIT_HOOK();
}
#endif







#endif  // if defined(__STM32L__)






