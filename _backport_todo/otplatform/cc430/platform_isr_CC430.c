/* Copyright 2010-2012 JP Norair
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
  * @file       /otplatform/cc430/platform_isr_CC430.c
  * @author     JP Norair
  * @version    R102
  * @date       31 Nov 2012
  * @brief      ISRs abstracted by the platform module
  * @ingroup    Platform
  *
  * This file works together with platform_CC430.c and platform_CC430.h.  It
  * has its own file so users reading through the codebase can find it easily,
  * because otherwise this code should be in platform_CC430.c.
  *
  ******************************************************************************
  */


#include <otplatform.h>
#if defined(__CC430__)

#include "OTAPI.h"


#if (1) //Using GULP

/** @note MSP430 compilers do not allow the LPM4_EXIT macro to be called
  * outside the scope of an ISR function.  Syntactically intelligent compilers
  * (GCC) can figure out that an inline function inside an interrupt is OK.
  * Syntactically stupid compilers (TI's CL430) require a macro.
  *
  * __SPECIAL_ISR_GPTIM__ is specifically for the GPTIM interrupt, which the
  * kernel uses to schedule tasks and power-down periods.  The kernel interrupt
  * MUST be able to wakeup from sleep.  Every other interrupt-driven process
  * *should* use a system call to wakeup the kernel: sys_preempt(), sys_halt(),
  * sys_synchronize(), sys_kill_active(), or sys_kill_all().  Otherwise, it can
  * just run in its context and not bother the kernel.
  */
//Kernel Timer is on case 2 ==> CCR1
//If Timer0 is being used, there is room for two more channels

#include <m2/radio.h>

#define __SPECIAL_ISR_GPTIM__	\
    switch (__even_in_range(OT_GPTIM->IV, 16)) {	\
        case 0: break;								\
        case 2: LPM4_EXIT;              break;  	\
        case 4: radio_mac_isr();        break;		\
    }


#endif //GULP







/** Platform-Managed ISRs   <BR>
  * ========================================================================<BR>
  * The constants of the form __ISR_XXX should be defined per-application in:
  * /apps/[YOURAPP]/code/isr_config_[YOURPLATFORM].h.  Define __ISR_XXX
  * constants based on the ISRs you want to enable.  In some cases, OpenTag
  * will need a certain ISR, and this file will declare an ISR even if it is 
  * defined by __ISR_XXX.  For example, Kernel Timer and Radio I/F interrupts
  * are almost always going to be needed.
  */

// Always enable RESET
//#undef  __ISR_REEST
//#define __ISR_RESET


// Always enable SYSNMI
#undef  __ISR_SYSNMI
#define __ISR_SYSNMI

// Always enable USER NMI
#undef  __ISR_USERNMI
#define __ISR_USERNMI

// Only permit LCD B if the CC430 in question actually has it
#ifndef _LCD
#undef __ISR_LCDB
#endif


#if (OT_FEATURE(TIME))
#   if (defined(_RTCA))
#       undef   __ISR_RTCA
#       undef   __N_ISR_RTCA
#       define  __ISR_RTCA   
#   elif (defined(_RTCD))
#       undef   __ISR_RTCD
#       undef   __N_ISR_RTCD
#       define  __ISR_RTCD
#   else 
#       warn "RTC not attached to a known RTC in this CC430"
#   endif
#endif


#if (OT_FEATURE(M2))
#   undef __ISR_RF1A
#   define __ISR_RF1A
#endif


#if (OT_FEATURE(MPIPE))
// DMA-driven serial MPipe: only DMA interrupt needed
#   if (MCU_FEATURE_MPIPEDMA == ENABLED)
#       undef   __ISR_DMA
#       define  __ISR_DMA

// Old Timey serial MPipe (no DMA)
#   elif ((MPIPE_UART_ID == 0xA0) || (MPIPE_SPI_ID == 0xA0))
#       undef   __ISR_USCIA0
#       undef   __N_ISR_USCIA0
#       define  __ISR_USCIA0
#   elif ((MPIPE_SPI_ID == 0xB0) || (MPIPE_I2C_ID == 0xB0))
#       undef   __ISR_USCIB0
#       undef   __N_ISR_USCIB0
#       define  __ISR_USCIB0

#   else
#       warn "MPIPE not attached to a known serial line in this MSP430."
#   endif
#endif



///If specified, this gets called after reset, during startup
#if defined(__ISR_RESET) && !defined(__ISR_RESET)
#if (CC_SUPPORT == CL430)
#pragma vector=RESET_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(RESET_VECTOR)
#endif
OT_INTERRUPT void isr_reset(void) {
    platform_isr_reset();
    LPM4_EXIT;
}
#endif



#if defined(__ISR_SYSNMI) && !defined(__N_ISR_SYSNMI)
#if (CC_SUPPORT == CL430)
#   pragma vector=SYSNMI_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(SYSNMI_VECTOR)
#endif
OT_INTERRUPT void isr_sysnmi(void) {
    platform_isr_sysnmi();
}
#endif



#if defined(__ISR_USERNMI) && !defined(__N_ISR_USERNMI)
#if (CC_SUPPORT == CL430)
#   pragma vector=UNMI_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(UNMI_VECTOR)
#endif
OT_INTERRUPT void isr_usernmi(void) {
    platform_isr_usernmi();
}
#endif




#if defined(__ISR_CB) && !defined(__N_ISR_CB)
#if (CC_SUPPORT == CL430)
#   pragma vector=COMP_B_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(COMP_B_VECTOR)
#endif
OT_INTERRUPT void isr_cb(void) {
    platform_isr_cb();
}
#endif




#if defined(__ISR_WDTI) && !defined(__N_ISR_WDTI)
#if (CC_SUPPORT == CL430)
#   pragma vector=WDT_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(WDT_VECTOR)
#endif
OT_INTERRUPT void isr_wdti(void) {
    platform_isr_wdti();
}
#endif




#if defined(__ISR_USCIA0) && !defined(__N_ISR_USCIA0)
#if (CC_SUPPORT == CL430)
#   pragma vector=USCI_A0_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(USCI_A0_VECTOR)
#endif
OT_INTERRUPT void isr_uscia0(void) {
    platform_isr_uscia0();
}
#endif



#if defined(__ISR_USCIB0) && !defined(__N_ISR_USCIB0)
#if (CC_SUPPORT == CL430)
#   pragma vector=USCI_B0_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(USCI_B0_VECTOR)
#endif
OT_INTERRUPT void isr_uscib0(void) {
    platform_isr_uscib0();
}
#endif




#if defined(__ISR_ADC12A) && !defined(__N_ISR_ADC12A)
#if (CC_SUPPORT == CL430)
#   pragma vector=ADC12_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(ADC12_VECTOR)
#endif
OT_INTERRUPT void isr_adc12a(void) {
    platform_isr_adc12a();
}
#endif




#if defined(__ISR_T0A0) && !defined(__N_ISR_T0A0)
#if (CC_SUPPORT == CL430)
#   pragma vector=TIMER0_A0_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(TIMER0_A0_VECTOR)
#endif
OT_INTERRUPT void isr_tim0a0(void) {
    platform_isr_tim0a0();
}
#endif



#if defined(__ISR_T0A1) && !defined(__N_ISR_T0A1)
#if (CC_SUPPORT == CL430)
#   pragma vector=TIMER0_A1_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(TIMER0_A1_VECTOR)
#endif
OT_INTERRUPT void isr_tim0a1(void) {
#   if (OT_GPTIM_ISR_ID == __ISR_T0A1_ID)
    __SPECIAL_ISR_GPTIM__
#	else
    platform_isr_tim0a1();
#   endif
}
#endif



#if defined(__ISR_RF1A) && !defined(__N_ISR_RF1A)
#if (CC_SUPPORT == CL430)
#   pragma vector=CC1101_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(CC1101_VECTOR)
#endif
OT_INTERRUPT void isr_rf1a(void) {
    platform_isr_rf1a();
}
#endif



#if defined(__ISR_DMA) && !defined(__N_ISR_DMA)
#if (CC_SUPPORT == CL430)
#   pragma vector=DMA_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(DMA_VECTOR)
#endif
OT_INTERRUPT void isr_dma(void) {
    platform_isr_dma();
}
#endif


#if defined(__ISR_T1A0) && !defined(__N_ISR_T1A0)
#if (CC_SUPPORT == CL430)
#   pragma vector=TIMER1_A0_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(TIMER1_A0_VECTOR)
#endif
OT_INTERRUPT void isr_tim1a0(void) {
    platform_isr_tim1a0();
}
#endif



#if defined(__ISR_T1A1) && !defined(__N_ISR_T1A1)
#if (CC_SUPPORT == CL430)
#   pragma vector=TIMER1_A1_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(TIMER1_A1_VECTOR)
#endif
OT_INTERRUPT void isr_tim1a1(void) {
#   if (OT_GPTIM_ISR_ID == __ISR_T1A1_ID)
    __SPECIAL_ISR_GPTIM__
#	else
    platform_isr_tim1a1();
#   endif
}
#endif



#if defined(__ISR_P1) && !defined(__N_ISR_P1)
#if (CC_SUPPORT == CL430)
#   pragma vector=PORT1_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(PORT1_VECTOR)
#endif
OT_INTERRUPT void isr_p1(void) {
    platform_isr_p1();
}
#endif


#if defined(__ISR_P2) && !defined(__N_ISR_P2)
#if (CC_SUPPORT == CL430)
#   pragma vector=PORT2_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(PORT2_VECTOR)
#endif
OT_INTERRUPT void isr_p2(void) {
    platform_isr_p2();
}
#endif



#if defined(__ISR_LCDB) && !defined(__N_ISR_LCDB)
#if (CC_SUPPORT == CL430)
#   pragma vector=LCD_B_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(LCD_B_VECTOR)
#endif
OT_INTERRUPT void isr_lcdb(void) {
    platform_isr_lcdb();
}
#endif



#ifdef __ISR_RTCA
#if (CC_SUPPORT == CL430)
#   pragma vector=RTC_A_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(RTC_A_VECTOR)
#endif
OT_INTERRUPT void isr_rtca(void) {
    platform_isr_rtca();
}
#endif



#ifdef __ISR_AES
#if (CC_SUPPORT == CL430)
#   pragma vector=AES_VECTOR
#elif (CC_SUPPORT == IAR_V5)
        //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(AES_VECTOR)
#endif
OT_INTERRUPT void isr_aes(void) {
    platform_isr_aes();
}
#endif


#endif






