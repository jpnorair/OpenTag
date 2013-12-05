/* Copyright 2010 JP Norair
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
  * @file       /OTlib/STM32F10x/platform_STM32F10x.c
  * @author     JP Norair
  * @version    V1.0
  * @date       16 July 2011
  * @brief      ISRs and hardware services abstracted by the platform module
  * @ingroup    Platform
  *
  * ISRs implemented in this file shall use only the Platform module macro 
  * abstractions, not functions or register nomenclature specific to any given 
  * hardware platform.
  *
  ******************************************************************************
  */

#include "OT_utils.h"
#include "OT_types.h"
#include "OT_config.h"
#include "OT_platform.h"

// OT modules that need initialization
#include "veelite.h"
#include "veelite_core.h"
#include "buffers.h"
#include "auth.h"
#include "mpipe.h"
#include "radio.h"
#include "system.h"
#include "session.h"



/** A Long list of Peripheral Clock selection Constants, based on settings <BR>
  * =======================================================================<BR>
  * The conditional selections draw on platform_config_~.h and the compiler
  * identification of your part.
  */

/// Core Bus Peripheral Select
// RCC_AHBPeriph_DMA1, RCC_AHBPeriph_DMA2, RCC_AHBPeriph_SRAM, 
// RCC_AHBPeriph_FLITF, RCC_AHBPeriph_CRC, RCC_AHBPeriph_FSMC, 
// RCC_AHBPeriph_SDIO, RCC_AHBPeriph_OTG_FS, RCC_AHBPeriph_ETH_MAC, 
// RCC_AHBPeriph_ETH_MAC_Tx, RCC_AHBPeriph_ETH_MAC_Rx
#if ((RADIO_DMA_INDEX == 1) || (MPIPE_DMA_INDEX == 1) || (MEMCPY_DMA_INDEX == 1))
#   define AHBClkDMA1   RCC_AHBPeriph_DMA1
#else
#   define AHBClkDMA1   0
#endif

#if ((RADIO_DMA_INDEX == 2) || (MPIPE_DMA_INDEX == 2) || (MEMCPY_DMA_INDEX == 2))
#   define AHBClkDMA2   RCC_AHBPeriph_DMA2
#else
#   define AHBClkDMA2   0
#endif

/// so far, none of these are used in OpenTag.  You could enable them 
/// independently in a user application, though
#define AHBClkSRAM          0
#define AHBClkFLITF         0
#define AHBClkCRC           0
#define AHBClkFSMC          0
#define AHBClkSDIO          0
#define AHBClkOTG_FS        0
#define AHBClkETH_MAC       0
#define AHBClkETH_MAC_Tx    0
#define AHBClkETH_MAC_Rx    0

#define AHBClk  (AHBClkDMA1 | AHBClkDMA2 | AHBClkSRAM | AHBClkFLITF | AHBClkCRC | \
                 AHBClkFSMC | AHBClkSDIO | AHBClkOTG_FS |AHBClkETH_MAC | \
                 AHBClkETH_MAC_Tx | AHBClkETH_MAC_Rx)


/// APB1 Peripheral Select
// RCC_APB1Periph_TIM2, RCC_APB1Periph_TIM3, RCC_APB1Periph_TIM4, 
// RCC_APB1Periph_TIM5, RCC_APB1Periph_TIM6, RCC_APB1Periph_TIM7, 
// RCC_APB1Periph_TIM12, RCC_APB1Periph_TIM13, RCC_APB1Periph_TIM14, 
// RCC_APB1Periph_WWDG, RCC_APB1Periph_SPI2, RCC_APB1Periph_SPI3, 
// RCC_APB1Periph_USART2, RCC_APB1Periph_USART3, RCC_APB1Periph_UART4, 
// RCC_APB1Periph_UART5, RCC_APB1Periph_I2C1, RCC_APB1Periph_I2C2, 
// RCC_APB1Periph_USB, RCC_APB1Periph_CAN1, RCC_APB1Periph_CAN2, 
// RCC_APB1Periph_BKP, RCC_APB1Periph_PWR, RCC_APB1Periph_DAC, 
// RCC_APB1Periph_CEC

#if (OT_GPTIM_INDEX == 2) 
#   define APB1ClkTIM2   RCC_APB1Periph_TIM2
#   define OT_GPTIM_ISR()   TIM2_IRQHandler(void)
#else
#   define APB1ClkTIM2   0
#endif

#if (OT_GPTIM_INDEX == 3)
#   define APB1ClkTIM3   RCC_APB1Periph_TIM3
#   define OT_GPTIM_ISR()   TIM3_IRQHandler(void)
#else
#   define APB1ClkTIM3   0
#endif

#if (OT_GPTIM_INDEX == 4)
#   define APB1ClkTIM4   RCC_APB1Periph_TIM4
#   define OT_GPTIM_ISR()   TIM4_IRQHandler(void)
#else
#   define APB1ClkTIM4   0
#endif

#if (OT_GPTIM_INDEX == 5)
#   define APB1ClkTIM5   RCC_APB1Periph_TIM5
#   define OT_GPTIM_ISR()   TIM5_IRQHandler(void)
#else
#   define APB1ClkTIM5   0
#endif

#if (OT_GPTIM_INDEX == 6)
#   define APB1ClkTIM3   RCC_APB1Periph_TIM6
#   define OT_GPTIM_ISR()   TIM6_IRQHandler(void)
#else
#   define APB1ClkTIM6   0
#endif

#if (OT_GPTIM_INDEX == 7)
#   define APB1ClkTIM7   RCC_APB1Periph_TIM7
#   define OT_GPTIM_ISR()   TIM7_IRQHandler(void)
#else
#   define APB1ClkTIM7   0
#endif

#if (OT_GPTIM_INDEX == 12)
#   define APB1ClkTIM12  RCC_APB1Periph_TIM12
#   define OT_GPTIM_ISR()   TIM12_IRQHandler(void)
#else
#   define APB1ClkTIM12  0
#endif

#if (OT_GPTIM_INDEX == 13)
#   define APB1ClkTIM13  RCC_APB1Periph_TIM13
#   define OT_GPTIM_ISR()   TIM13_IRQHandler(void)
#else
#   define APB1ClkTIM13  0
#endif

#if (OT_GPTIM_INDEX == 14)
#   define APB1ClkTIM14  RCC_APB1Periph_TIM14
#   define OT_GPTIM_ISR()   TIM14_IRQHandler(void)
#else
#   define APB1ClkTIM14  0
#endif

#if ((RADIO_SPI_INDEX == 2) || (MPIPE_SPI_INDEX == 2))
#   define APB1ClkSPI2   RCC_APB1Periph_SPI2
#else
#   define APB1ClkSPI2   0
#endif

#if ((RADIO_SPI_INDEX == 3) || (MPIPE_SPI_INDEX == 3))
#   define APB1ClkSPI3   RCC_APB1Periph_SPI3
#else
#   define APB1ClkSPI3   0
#endif

#define APB1ClkUSART2 0
#define APB1ClkUSART3 0
#define APB1ClkUART4  0
#define APB1ClkUART5  0
#define APB1ClkI2C1  0
#define APB1ClkI2C2  0
#define APB1ClkUSB    0
#define APB1ClkCAN1   0
#define APB1ClkCAN2    0

// Additional Peripherals (turn on some of them)
#define APB1ClkWWDG  RCC_APB1Periph_WWDG
#define APB1ClkBKP   RCC_APB1Periph_BKP
#define APB1ClkPWR   RCC_APB1Periph_PWR
#define APB1ClkDAC   0
#define APB1ClkCEC   0

#define APB1Clk     (APB1ClkTIM2 | APB1ClkTIM3 | APB1ClkTIM4 | APB1ClkTIM5 | \
                    APB1ClkTIM6 | APB1ClkTIM7 | APB1ClkTIM12 | APB1ClkTIM13 | \
                    APB1ClkTIM14 | APB1ClkWWDG | APB1ClkSPI2 | APB1ClkSPI3 | \
                    APB1ClkUSART2 | APB1ClkUSART3 | APB1ClkUART4 | APB1ClkUART5 | \
                    APB1ClkI2C1 | APB1ClkI2C2 | APB1ClkUSB | APB1ClkCAN1 | \
                    APB1ClkCAN2 | APB1ClkBKP | APB1ClkPWR | APB1ClkDAC | APB1ClkCEC)


/// APB2 Peripheral Select
// RCC_APB2Periph_AFIO, RCC_APB2Periph_GPIOA, RCC_APB2Periph_GPIOB, 
// RCC_APB2Periph_GPIOC, RCC_APB2Periph_GPIOD, RCC_APB2Periph_GPIOE, 
// RCC_APB2Periph_GPIOF, RCC_APB2Periph_GPIOG, RCC_APB2Periph_ADC1, 
// RCC_APB2Periph_ADC2, RCC_APB2Periph_TIM1, RCC_APB2Periph_SPI1, 
// RCC_APB2Periph_TIM8, RCC_APB2Periph_USART1, RCC_APB2Periph_ADC3, 
// RCC_APB2Periph_TIM15, RCC_APB2Periph_TIM16, RCC_APB2Periph_TIM17, 
// RCC_APB2Periph_TIM9, RCC_APB2Periph_TIM10, RCC_APB2Periph_TIM11
#if (OT_GPTIM_INDEX == 1)
#   define APB2ClkTIM1   RCC_APB2Periph_TIM1
#   define OT_GPTIM_ISR()   TIM1_IRQHandler(void)
#else
#   define APB2ClkTIM1   0
#endif

#if (OT_GPTIM_INDEX == 8)
#   define APB2ClkTIM8   RCC_APB2Periph_TIM8
#   define OT_GPTIM_ISR()   TIM8_IRQHandler(void)
#else
#   define APB2ClkTIM8   0
#endif

#if (OT_GPTIM_INDEX == 9)
#   define APB2ClkTIM9   RCC_APB2Periph_TIM9
#   define OT_GPTIM_ISR()   TIM9_IRQHandler(void)
#else
#   define APB2ClkTIM9   0
#endif

#if (OT_GPTIM_INDEX == 10)
#   define APB2ClkTIM10  RCC_APB2Periph_TIM10
#   define OT_GPTIM_ISR()   TIM10_IRQHandler(void)
#else
#   define APB2ClkTIM10  0
#endif

#if (OT_GPTIM_INDEX == 11)
#   define APB2ClkTIM11  RCC_APB2Periph_TIM11
#   define OT_GPTIM_ISR()   TIM11_IRQHandler(void)
#else
#   define APB2ClkTIM11  0
#endif

#if (OT_GPTIM_INDEX == 15)
#   define APB2ClkTIM15  RCC_APB2Periph_TIM15
#   define OT_GPTIM_ISR()   TIM15_IRQHandler(void)
#else
#   define APB2ClkTIM15  0
#endif

#if (OT_GPTIM_INDEX == 16)
#   define APB2ClkTIM16  RCC_APB2Periph_TIM16
#   define OT_GPTIM_ISR()   TIM16_IRQHandler(void)
#else
#   define APB2ClkTIM16  0
#endif

#if (OT_GPTIM_INDEX == 17)
#   define APB2ClkTIM17  RCC_APB2Periph_TIM17
#   define OT_GPTIM_ISR()   TIM17_IRQHandler(void)
#else
#   define APB2ClkTIM17  0
#endif

#if ((RADIO_SPI_INDEX == 1) || (MPIPE_SPI_INDEX == 1))
#   define APB2ClkSPI1   RCC_APB2Periph_SPI1
#else
#   define APB2ClkSPI1   0
#endif

#define APB2ClkUSART1   0
#define APB2ClkAFIO     RCC_APB2Periph_AFIO
#define APB2ClkGPIOA    RCC_APB2Periph_GPIOA
#define APB2ClkGPIOB    RCC_APB2Periph_GPIOB
#define APB2ClkGPIOC    RCC_APB2Periph_GPIOC
#define APB2ClkGPIOD    RCC_APB2Periph_GPIOD
#define APB2ClkGPIOE    RCC_APB2Periph_GPIOE
#define APB2ClkGPIOF    RCC_APB2Periph_GPIOF
#define APB2ClkGPIOG    RCC_APB2Periph_GPIOG
#define APB2ClkADC1     0 //RCC_APB2Periph_ADC1
#define APB2ClkADC2     0 //RCC_APB2Periph_ADC2
#define APB2ClkADC3     0 //RCC_APB2Periph_ADC3

#define APB2Clk     (APB2ClkAFIO | APB2ClkGPIOA | APB2ClkGPIOB | APB2ClkGPIOC | \
                    APB2ClkGPIOD | APB2ClkGPIOE | APB2ClkGPIOF | APB2ClkGPIOG | \
                    APB2ClkADC1 | APB2ClkADC2 | APB2ClkTIM1 | APB2ClkSPI1 | \
                    APB2ClkTIM8 | APB2ClkUSART1 | APB2ClkADC3 | APB2ClkTIM15 | \
                    APB2ClkTIM16 | APB2ClkTIM17 | APB2ClkTIM9 | APB2ClkTIM10 | \
                    APB2ClkTIM11)




/** Platform Data <BR>
  * ============================================================================
  */
platform_struct platform;

#if (OT_FEATURE(RTC) == ENABLED)
#   define RTC_ALARMS       0 //(ALARM_beacon + __todo_IS_STM32L__)
#   define RTC_OVERSAMPLE   0
    // RTC_OVERSAMPLE: min 0, max 16

    typedef struct {
        ot_bool active;
        ot_u16  mask;
        ot_u16  value;
    } rtcalarm;

    typedef struct {
#       if (RTC_OVERSAMPLE != 0)
            ot_u32      utc;
#       endif
#       if (RTC_ALARMS > 0)
            rtcalarm    alarm[RTC_ALARMS]
#       endif
    } otrtc_struct;

#   if ((RTC_OVERSAMPLE != 0) || (RTC_ALARMS > 0))
        otrtc_struct otrtc;
#   endif
#endif










/** Platform Interrupts <BR>
  * ========================================================================<BR>
  * GPTIM, RTC, SysTick interrupts (kernel interrupts), plus any power 
  * management interrupts that might be used.  These interrupts are important
  * to the platform.  You can move them out of the platform module, into a
  * global interrupt handler C file if you want, but it is better to leave the
  * OpenTag resources allocated to OpenTag if you can.  STM32's tend to have
  * plenty of resources, so the only one you might want to use is the RTC, but
  * you can also just refer to the 
  */

/// The NMI is triggerable by software.  It is used to manage runtime errors.
void NMI_Handler(void) {
    switch (platform.error_code) {
        case 0: break;
        default: break;
    }
    while(1);
}
  

void OT_GPTIM_ISR() {
// References a TIMx_IRQHandler() function
    platform_ot_run();
}

#if (OT_FEATURE(RTC) == ENABLED)

#if (RTC_ALARMS > 0)
void sub_juggle_rtc_alarm() {
    ot_u8  i;
    ot_u32 comp;
    ot_u32 next     = ~0;
    ot_u32 rtcval   = RTC_GetCounter();
    
    for (i=0; i<RTC_ALARMS; i++) {
        if (otrtc.alarm[i].active) {
            comp    = rtcval & ((ot_u32)otrtc.alarm[i].mask << RTC_OVERSAMPLE);
            comp   += (ot_u32)otrtc.alarm[i].value << RTC_OVERSAMPLE;
            next    = (comp < next) ? comp : next;
        }
    }

    RTC_SetAlarm(rtcval+next);
}
#endif

void RTC_IRQHandler(void) {
/// Currently Psuedocode.  RTC is normally used with ALARM interrupt.  If it
/// is oversampling, then it will also interrupt on each second, so the user
/// can increment the UTC.
#if (RTC_OVERSAMPLE != 0)
    if (__rtc_interrupt_is_second__) {
        otrtc.utc++;
        //add clear flag
    }
#endif
#if (RTC_ALARMS > 0)
    if (__rtc_interrupt_is_alarm__) {
        sub_juggle_rtc_alarm();
        //add clear flag
    }
#endif
}

#endif // End of RTC stuff



#if 0
#pragma vector=ADC12_VECTOR
#endif /* #if 0 */








/** Platform Interrupt & Event Management Routines <BR>
  * ========================================================================<BR>
  */

void platform_ot_preempt() {
/// Assure interrupt is enabled and cause a SW update interrupt
    OT_GPTIM->DIER  = 0;
    OT_GPTIM->ARR   = platform_get_gptim();
    OT_GPTIM->DIER  = TIM_DIER_UIE;
    OT_GPTIM->EGR   = TIM_EGR_UG;
    __NOP();  //No-op to let forced update event (TIM_EGR_UG) stabilize
}

void platform_ot_pause() {
    platform_ot_preempt();
    platform_flush_gptim();
}


void platform_ot_run() {
/// 1. Save the amount of time that just passed
/// 2. Put the timer into free-running upcounter to time kernel process
/// 3. Run System Kernel, which returns its next scheduled call
/// 4. Put the next scheduled call into the timer, and turn it back on
    ot_u16 next_event;
    ot_u16 elapsed_time;
    elapsed_time    = OT_GPTIM->ARR;
    next_event      = sys_event_manager( elapsed_time );
    
#   if (OT_PARAM(KERNEL_LIMIT) > 0)
        if (next_event > OT_PARAM(KERNEL_LIMIT))
            next_event = OT_PARAM(KERNEL_LIMIT);
#   endif

    platform_set_gptim(next_event);
}










/** Platform Power-up/down and intialization Functions <BR>
  * ============================================================================
  */
  
void platform_poweron() {
    /// Hardware turn-on stuff
    platform_init_busclk();         // extra bus clock setup not in SystemInit()
    platform_init_periphclk();      // Peripherals OpenTag cares about
    platform_init_interruptor();    // Interrupts OpenTag cares about
    platform_init_gptim(0);         // Initialize GPTIM (to 1024 Hz)
    platform_init_gpio();           // Set up connections on the board
    
    /// Restore vworm (following save on shutdown)
    vworm_init();
    
#   if (OT_FEATURE(MPIPE) == ENABLED) 
        /// Mpipe (message pipe) typically used for serial-line comm.
        mpipe_init(NULL);
#   endif
}


void platform_poweroff() {
    ISF_syncmirror();
    vworm_save();
}


void platform_init_OT() {
    buffers_init(); //buffers init must be first in order to do core dumps
    vl_init();      //Veelite init must be second
    radio_init();   //radio init third
    sys_init();     //system init last
}


void platform_fastinit_OT() {
    platform_init_OT();
}


void platform_init_busclk() { 
/// The SystemInit() function from the ST libraries configures the main
/// clocks, and normally it is called in the pre-main function startup
/// routine (startup_stm32f10x_~.s).  I've modified SystemInit() slightly,
/// to take in configuration defines from platform_config_~.h

    //SystemInit();   // called automatically on device startup

#   ifdef __DEBUG__
    // OT_DEBUG_PERIPHERALS is defined in the board_xxx.h file
    DBGMCU_Config(OT_DEBUG_PERIPHERALS, ENABLE);
#   endif
}


void platform_init_periphclk() {
    /// Core Bus Division
    /// AHB is always initialized at full speed.  To clock-down, uncomment and 
    /// change line below to 2, 4, 8, 16, 64, 128, 256, 512
    //RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1; 

    /// Peripheral Bus 1 Division (clock/4)
    /// APB1 is initialized at Core Bus speed for Core Bus < 48MHz.  To clock 
    /// down further, uncomment and change line below to 1, 2, 4, 8, 16
    //RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV4;

    /// Peripheral Bus 2 Division (clock/1)
    /// APB2 is initialized at Core Bus speed.  To clock down further, uncomment
    /// and change line below to 1, 2, 4, 8, 16
    //RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV2;

    //RCC->CFGR      |= (uint32_t)RCC_CFGR_HPRE_DIV1 | \
                      (uint32_t)RCC_CFGR_PPRE1_DIV4 | \
                      (uint32_t)RCC_CFGR_PPRE2_DIV1;
    RCC->AHBENR    |= AHBClk;
    RCC->APB1ENR   |= APB1Clk;
    RCC->APB2ENR   |= APB2Clk;
}




void platform_init_interruptor() { 
/// This function enables all used NVIC controllers, so only the peripheral 
/// interrupt bits need to be set to turn the interrupts on or off.
///
/// @note Look in the core_cm3.h file in the CMSIS distribution to learn more
/// about some functions you can use for setting up interrupts.
///
/// Below is the NVIC priority system used by OpenTag.  It uses 2 bits for the
/// priority group and 2 bits for subpriority.  Groups 0-1 are for kernel events 
/// and 2-3 for user/application interrupts.                                    <BR>
/// <LI> Priority 0: Primary I/O (Radio interrupts, MPipe)                      </LI>
/// <LI> Priority 1: OpenTag Kernel Pre-emption sources (GPTim, RTC, Systick)   </LI>
/// <LI> Priority 2: Nested User/Application interrupts                         </LI>
/// <LI> Priority 3: General User/Application interrupts                        </LI>


    /// Vector Table @ 0x08000000 (offset = 0)
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);
    
    /// Group 2 = 2 bits for Priority, 2 bits for subpriority
    NVIC_SetPriorityGrouping(5);    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    /// Priority Group 0: Radio interrupts and Mpipe: 
    /// These are initialized in their respective modules (radio, mpipe, etc)
    
    /// Priority Group 1: OpenTag Kernel Pre-emptors
    //NVIC->IP[(uint32_t)(RTC_IRQn)]              = b0100 << (8 - __NVIC_PRIO_BITS);
    NVIC->IP[(uint32_t)(OT_GPTIM_IRQn)]         = b0101 << (8 - __NVIC_PRIO_BITS);
    //NVIC->ISER[((uint32_t)(RTC_IRQn) >> 5)]     = (1 << ((uint32_t)(RTC_IRQn) & 0x1F));
    NVIC->ISER[((uint32_t)(OT_GPTIM_IRQn) >> 5)]= (1 << ((uint32_t)(OT_GPTIM_IRQn) & 0x1F));
    
    // Systick: uncomment if you want to use the systick at priority (1,2)
    //SCB->SHP[((uint32_t)(SysTick_IRQn) & 0xF)-4]    = b0110 << (8 - __NVIC_PRIO_BITS);
    //NVIC->ISER[((uint32_t)(SysTick_IRQn) >> 5)]     = (1 << ((uint32_t)(SysTick_IRQn) & 0x1F));
    
    /// Priority Group 2: Nested User/Application interrupts  <BR>
    /// none in the core - you may declare them in the code modules like above,
    /// but use priority = b10xx.
    
    /// Priority Group 3: Nested User/Application interrupts  <BR>
    /// none in the core - you may declare them in the code modules like above,
    /// but use priority = b11xx.
}




void platform_init_gpio() { 
/// The default GPIO is OT Test triggers only, for debugging
#ifdef __DEBUG__
    GPIO_InitTypeDef GPIOinit;
    
    GPIOinit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIOinit.GPIO_Mode  = GPIO_Mode_Out_PP;
	
    GPIOinit.GPIO_Pin   = OT_TRIG1_PIN;
	GPIO_Init(OT_TRIG1_PORT, &GPIOinit);
    
    GPIOinit.GPIO_Pin   = OT_TRIG2_PIN;
	GPIO_Init(OT_TRIG2_PORT, &GPIOinit);
#endif
}




void platform_init_gptim(ot_uint prescaler) {
/// Right now, prescaler input is ignored.
/// GPTIM is put into a normal upcounting mode, but in "one pulse mode" so that
/// the counter is halted after reaching the limit
    OT_GPTIM->CR1   = 0;
    OT_GPTIM->CR2   = 0;
    OT_GPTIM->SMCR  = 0;
    OT_GPTIM->ARR   = 65535;
    OT_GPTIM->PSC   = ((OT_GPTIM_CLOCK/2) / OT_GPTIM_RES);  // Should be always 1024 Hz
    OT_GPTIM->EGR   = TIM_PSCReloadMode_Immediate;     // generate update to lock-in prescaler
    OT_GPTIM->SR    = 0;        // clear update flag
    OT_GPTIM->DIER  = TIM_DIER_UIE;   // Update Interrupt
    OT_GPTIM->CR1  |= TIM_CR1_CEN;    // enable      
}


void platform_init_watchdog() {
    platform_reset_watchdog(64);
}


void platform_init_resetswitch() {
}


void platform_init_systick(ot_uint period) {
    if (period <= SysTick_LOAD_RELOAD_Msk) {            //Cancel if period is too large
        SysTick->VAL   = 0;                             //Load the SysTick Counter Value
        SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | \
                         SysTick_CTRL_TICKINT_Msk   | \
                         SysTick_CTRL_ENABLE_Msk;       //Enable SysTick IRQ and Timer 
    }
}


void platform_init_rtc(ot_u32 value) {
#if (OT_FEATURE(RTC) == ENABLED)
    // Prescaler is fixed to yield 1 Hz
    // Counter is set for input value (don't use 0, it's not 1/1/1990 anyway)
    // Default Alarm is set to 0
#   if (RTC_OVERSAMPLE != 0)
        otrtc.utc   = value;
        value     <<= RTC_OVERSAMPLE;
#   endif

    RTC_EnterConfigMode();
    RTC->PRLH   = 0; //(0 & 0x000F0000) >> 16;
    RTC->PRLL   = (PLATFORM_LSCLOCK_HZ-1) >> RTC_OVERSAMPLE;
    RTC->CNTH   = value >> 16;
    RTC->CNTL   = (value & 0x0000FFFF);
    RTC->ALRH   = 0;
    RTC->ALRL   = 0;
    RTC_ExitConfigMode();

    platform_enable_rtc();
#endif
}


void platform_init_memcpy() {
#if (MCU_CONFIG(MEMCPYDMA) == ENABLED)
#endif
}




/** Platform Peripheral Access Routines <BR>
  * ========================================================================<BR>
  */

ot_u16 platform_get_gptim() {
/// Return Present value of the counter
    return OT_GPTIM->CNT;
}

void platform_set_gptim(ot_u16 value) {
/// Flush GPTIM and assure it is in up-counting interrupt mode
    OT_GPTIM->DIER  = 0;
    OT_GPTIM->ARR   = value;
    OT_GPTIM->EGR   = TIM_EGR_UG;
    //__NOP();    // NOP needed for update to stabilize
    OT_GPTIM->SR    = 0;
	OT_GPTIM->DIER  = TIM_DIER_UIE;
}

void platform_flush_gptim() { 
/// Flush GPTIM, resume in continuous mode with interrupt off.
    OT_GPTIM->DIER  = 0;
    OT_GPTIM->ARR   = 65535;
    OT_GPTIM->EGR   = TIM_EGR_UG;
}

void platform_run_watchdog() {

}

void platform_reset_watchdog(ot_u16 reset) {

}

void platform_enable_rtc() {
#if (OT_FEATURE(RTC) == ENABLED)
#   if (RTC_OVERSAMPLE != 0)
        RTC->CRH = RTC_IT_ALR | RTC_IT_SEC;
#   elif (RTC_ALARMS > 0)
        RTC->CRH = RTC_IT_ALR;
#   else
        platform_disable_rtc();
#   endif
#endif
}

void platform_disable_rtc() {
#if (OT_FEATURE(RTC) == ENABLED)
    RTC->CRH    = 0;
#endif
}

ot_u32 platform_get_time() {
#if (OT_FEATURE(RTC) == ENABLED)
#   if (RTC_OVERSAMPLE == 0)
        return RTC_GetCounter();
#   else
        return otrtc.utc;
#   endif
#endif
}

void platform_set_time(ot_u32 utc_time) {
#if (RTC_OVERSAMPLE != 0)
    otrtc.utc   = utc_time;
#else
    RTC_SetCounter(utc_time);
#endif
}






/** Platform Debug Triggers <BR>
  * ========================================================================<BR>
  * Triggers are optional pins used for debugging.  Sometimes they are 
  * hooked up to LEDs, logic probes, etc.  There are 2 triggers defined
  * by default, but you could add more.
  * 
  * If you decide to hook triggers to LEDs, if at all possible TRIG1 should be
  * green and TRIG2 should be orange or amber/yellow.
  */
#ifdef OT_TRIG1_PORT
void platform_trig1_high() {    OT_TRIG1_PORT->ODR |= OT_TRIG1_PIN; }
void platform_trig1_low() {     OT_TRIG1_PORT->ODR &= ~OT_TRIG1_PIN; }
void platform_trig1_toggle() {  OT_TRIG1_PORT->ODR ^= OT_TRIG1_PIN; }
#else
void platform_trig1_high() { }
void platform_trig1_low() { }
void platform_trig1_toggle() { }
#endif

#ifdef OT_TRIG2_PORT
void platform_trig2_high() {    OT_TRIG2_PORT->ODR |= OT_TRIG2_PIN; }
void platform_trig2_low() {     OT_TRIG2_PORT->ODR &= ~OT_TRIG2_PIN; }
void platform_trig2_toggle() {  OT_TRIG2_PORT->ODR ^= OT_TRIG2_PIN; }
#else
void platform_trig2_high() { }
void platform_trig2_low() { }
void platform_trig2_toggle() { }
#endif





/** Platform Default CRC Routine <BR>
  * ========================================================================<BR>
  * Uses the CRC table from the CRC module because STM32F1 does not have a 
  * standalone CRC peripheral
  */
#include "crc16.h"

ot_u16 platform_crc_init() {
/// No HW CRC
}

ot_u16 platform_crc_block(ot_u8* block_addr, ot_int block_size) {
    ot_u16 crc_val;

    while (block_size > 0) {
        crc_val = (crc_val << 8) ^ crc_table[ ((crc_val >> 8) & 0xff) ^ *block_addr++ ];
        block_size--;
    }
}

void platform_crc_byte(ot_u8 databyte) {
/// No HW CRC
}

ot_u16 platform_crc_result() {
/// No HW CRC
}




/** Platform Random Number Generation Routines <BR>
  * ========================================================================<BR>
  * The platform must be able to compute a strong random number (via function
  * platform_rand()) and a "pseudo" random number (via platform_prand_u8()).
  */
void platform_rand(ot_u8* rand_out, ot_int bytes_out) {
}


ot_u32 rprime;
static ot_u32 r;


void platform_init_prand(ot_u16 seed) {
    rprime  = 83;
    r       = 73;
}

sub_rotate_prand() {
    static ot_u32 Y;
    r   = ((r+1) < 3) ? rprime : r;
    r   = (9973 * ~r) + ((Y) % 701);
    Y   = r;
}

ot_u8 platform_prand_u8() {
	sub_rotate_prand();
    return (ot_u8)(r>>24);
}

ot_u16 platform_prand_u16() {
    sub_rotate_prand();
    return (ot_u8)(r>>24);
}







/** Platform memcpy Routine <BR>
  * ========================================================================<BR>
  * Similar to standard implementation of "memcpy"
  */

void platform_memcpy(ot_u8* dest, ot_u8* src, ot_int length) {
/// Behavior is always blocking.

#if (OS_FEATURE(MEMCPY) == ENABLED)
    memcpy(dest, src, length);

#elif (MCU_CONFIG(MEMCPYDMA) == ENABLED)
    MEMCPY_DMA->IFCR        = MEMCPY_DMA_INT;
    MEMCPY_DMA_CHAN->CPAR   = (ot_u32)dest;
    MEMCPY_DMA_CHAN->CMAR   = (ot_u32)src;
    MEMCPY_DMA_CHAN->CNDTR  = length;
    MEMCPY_DMA_CHAN->CCR    = DMA_DIR_PeripheralDST       | \
                              DMA_Mode_Normal             | \
                              DMA_PeripheralInc_Enable    | \
                              DMA_MemoryInc_Enable        | \
                              DMA_PeripheralDataSize_Byte | \
                              DMA_MemoryDataSize_Byte     | \
                              DMA_Priority_VeryHigh       | \
                              DMA_M2M_Enable              | \
                              DMA_CCR1_EN;
    while((MEMCPY_DMA->ISR & MEMCPY_DMA_INT) == 0);

#else
    /// Uses the "Duff's Device" for loop unrolling.  If this is incredibly 
    /// confusing to you, check the internet for "Duff's Device."
    if (length > 0) {
        ot_int loops = (length + 7) >> 3;
        
        switch (length & 0x7) {
            case 0: do {    *dest++ = *src++;
            case 7:         *dest++ = *src++;
            case 6:         *dest++ = *src++;
            case 5:         *dest++ = *src++;
            case 4:         *dest++ = *src++;
            case 3:         *dest++ = *src++;
            case 2:         *dest++ = *src++;
            case 1:         *dest++ = *src++;
                        } 
                        while (--loops > 0);
        }
    }
    
#endif
}





#define CNT1us    (PLATFORM_HSCLOCK_HZ/(1000000 * 5))
#define CNT1ms    (CNT1us*1000)

void platform_swdelay_ms(ot_u16 n) {
	ot_u32 c;

	c = n * CNT1ms;
	for (; c>0; c--);
}


void platform_swdelay_us(ot_u16 n) {
	ot_u32 c;

	c = n * CNT1us;
	for (; c>0; c--);
}



















