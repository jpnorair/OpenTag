/* Copyright 2010-2020 JP Norair
  *
  * Licensed under the OpenTag License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  * http://www.indigresso.com/wiki/doku.php?idopentag:license_1_0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  */
/**
  * @file       /apps/ht_xrttest/app/isr_config_STM32L4.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R100
  * @date       15 September 2017
  * @brief      STM32L4 ISR Configuration
  *
  * Don't actually include this.  
  * Include <platform/config.h> or <otplatform.h> instead.
  *
  ******************************************************************************
  */

#if !defined(__ISR_CONFIG_STM32L4_H) && defined(__STM32L4__)
#define __ISR_CONFIG_STM32L4_H



/// Cortex M4 NVIC specification.  For STM32L4, it should take values 1, 2, 4, 
/// or 8, or it can be defined in compiler.
#ifndef __CM4_NVIC_GROUPS
#   define __CM4_NVIC_GROUPS        2
#endif



/// Cortex M0 Processor Exception Interrupt Sources
/// Uncomment to force-negate or force-enable for your app (negate takes priority)
/// The Board Support Header should enable the typical ones automatically.

//#define __N_ISR_NonMaskableInt      -14    /* 2 Non Maskable Interrupt                 */
//#define __N_ISR_SVC                 -5     /* 11 Cortex-M0+ SV Call Interrupt           */
//#define __N_ISR_PendSV              -2     /* 14 Cortex-M0+ Pend SV Interrupt           */
//#define __N_ISR_SysTick             -1     /* 15 Cortex-M0+ System Tick Interrupt       */

//#define __ISR_NonMaskableInt        -14    /* 2 Non Maskable Interrupt                 */
//#define __ISR_SVC                   -5     /* 11 Cortex-M0+ SV Call Interrupt           */
//#define __ISR_PendSV                -2     /* 14 Cortex-M0+ Pend SV Interrupt           */
//#define __ISR_SysTick               -1     /* 15 Cortex-M0+ System Tick Interrupt       */

// #define __ISR_WWDG              7
// #define __N_ISR_WWDG            7
// 
// #define __ISR_PVD               8
// #define __N_ISR_PVD             8
// 
// #define __ISR_TAMPER_STAMP      9
// #define __N_ISR_TAMPER_STAMP    9
// #define __ISR_CSSLSE            9
// #define __N_ISR_CSSLSE          9
// 
// #define __ISR_RTCWKUP           10
// #define __N_ISR_RTCWKUP         10
// 
// #define __ISR_RTC_Alarm         48
// #define __N_ISR_RTC_Alarm       48
// 
// #define __ISR_FLASH             11
// #define __N_ISR_FLASH           11
// 
// #define __ISR_RCC               12
// #define __N_ISR_RCC             12
// 
// #define __ISR_EXTI0             13
// #define __N_ISR_EXTI0           13
// 
// #define __ISR_EXTI1             14
// #define __N_ISR_EXTI1           14
// 
// #define __ISR_EXTI2             15
// #define __N_ISR_EXTI2           15
// 
// #define __ISR_EXTI3             16
// #define __N_ISR_EXTI3           16
// 
// #define __ISR_EXTI4             17
// #define __N_ISR_EXTI4           17
// 
// #define __ISR_EXTI5             30
// #define __N_ISR_EXTI5           30
// #define __ISR_EXTI6             30
// #define __N_ISR_EXTI6           30
// #define __ISR_EXTI7             30
// #define __N_ISR_EXTI7           30
// #define __ISR_EXTI8             30
// #define __N_ISR_EXTI8           30
// #define __ISR_EXTI9             30
// #define __N_ISR_EXTI9           30
// 
// #define __ISR_EXTI10            47
// #define __N_ISR_EXTI10          47
// #define __ISR_EXTI11            47
// #define __N_ISR_EXTI11          47
// #define __ISR_EXTI12            47
// #define __N_ISR_EXTI12          47
// #define __ISR_EXTI13            47
// #define __N_ISR_EXTI13          47
// #define __ISR_EXTI14            47
// #define __N_ISR_EXTI14          47
// #define __ISR_EXTI15            47
// #define __N_ISR_EXTI15          47
// 
// 
// #define __ISR_DMA1_Channel1     18
// #define __N_ISR_DMA1_Channel1   18
// 
// #define __ISR_DMA1_Channel2     19
// #define __N_ISR_DMA1_Channel2   19
// 
// #define __ISR_DMA1_Channel3     20
// #define __N_ISR_DMA1_Channel3   20
// 
// #define __ISR_DMA1_Channel4     21
// #define __N_ISR_DMA1_Channel4   21
// 
// #define __ISR_DMA1_Channel4     22
// #define __N_ISR_DMA1_Channel4   22
// 
// #define __ISR_DMA1_Channel5     23
// #define __N_ISR_DMA1_Channel5   23
// 
// #define __ISR_DMA1_Channel6     24
// #define __N_ISR_DMA1_Channel6   24
// 
// #define __ISR_DMA1_Channel7     25
// #define __N_ISR_DMA1_Channel7   25
// 
// #define __ISR_DMA2_Channel1     63
// #define __N_ISR_DMA2_Channel1   63
// 
// #define __ISR_DMA2_Channel2     64
// #define __N_ISR_DMA2_Channel2   64
// 
// #define __ISR_DMA2_Channel3     65
// #define __N_ISR_DMA2_Channel3   65
// 
// #define __ISR_DMA2_Channel4     66
// #define __N_ISR_DMA2_Channel4   66
// 
// #define __ISR_DMA2_Channel5     67
// #define __N_ISR_DMA2_Channel5   67
// 
// #define __ISR_DMA2_Channel6     75
// #define __N_ISR_DMA2_Channel6   75
// 
// #define __ISR_DMA2_Channel7     76
// #define __N_ISR_DMA2_Channel7   76
// 
// #define __ISR_ADC1              25
// #define __N_ISR_ADC1            25
// #define __ISR_ADC2              25
// #define __N_ISR_ADC2            25
// 
// #define __ISR_CAN1TX            26
// #define __N_ISR_CAN1TX          26
// 
// #define __ISR_CAN1RX0           27
// #define __N_ISR_CAN1RX0         27
// 
// #define __ISR_CAN1RX1           28
// #define __N_ISR_CAN1RX1         28
// 
// #define __ISR_CAN1SCE           29
// #define __N_ISR_CAN1SCE         29
// 
// #define __ISR_TIM1BRK           31
// #define __N_ISR_TIM1BRK         31
// #define __ISR_TIM15             31
// #define __N_ISR_TIM15           31
// 
// #define __ISR_TIM1UP            32
// #define __N_ISR_TIM1UP          32
// #define __ISR_TIM16             32
// #define __N_ISR_TIM16           32
// 
// #define __ISR_TIM1TRG           33
// #define __N_ISR_TIM1TRG         33
// #define __ISR_TIM1COM           33
// #define __N_ISR_TIM1COM         33
// 
// #define __ISR_TIM1CC            34
// #define __N_ISR_TIM1CC          34
// 
// #define __ISR_TIM2              35
// #define __N_ISR_TIM2            35
// 
// #define __ISR_TIM3              36
// #define __N_ISR_TIM3            36
// 
// #define __ISR_LPTIM1            72
// #define __N_ISR_LPTIM1          72
// 
// #define __ISR_LPTIM2            73
// #define __N_ISR_LPTIM2          73
// 
// #define __ISR_TIM6              61
// #define __N_ISR_TIM6            61
// #define __ISR_DAC               61
// #define __N_ISR_DAC             61
// 
// #define __ISR_TIM7              62
// #define __N_ISR_TIM7            62
// 
// #define __ISR_I2C1EV            38
// #define __N_ISR_I2C1EV          38
// 
// #define __ISR_I2C1ER            39
// #define __N_ISR_I2C1ER          39
// 
// #define __ISR_I2C2EV            40
// #define __N_ISR_I2C2EV          40
// 
// #define __ISR_I2C2ER            41
// #define __N_ISR_I2C2ER          41
// 
// #define __ISR_I2C3EV            79
// #define __N_ISR_I2C3EV          79
// 
// #define __ISR_I2C3ER            80
// #define __N_ISR_I2C3ER          80
// 
// #define __ISR_I2C4EV            90
// #define __N_ISR_I2C4EV          90
// 
// #define __ISR_I2C4ER            91
// #define __N_ISR_I2C4ER          91
// 
// #define __ISR_SPI1              42
// #define __N_ISR_SPI1            42
// 
// #define __ISR_SPI2              43
// #define __N_ISR_SPI2            43
// 
// #define __ISR_SPI3              58
// #define __N_ISR_SPI3            58
// 
// #define __ISR_USART1            44
// #define __N_ISR_USART1          44
// 
// #define __ISR_USART2            45
// #define __N_ISR_USART2          45
// 
// #define __ISR_USART3            46
// #define __N_ISR_USART3          46
// 
// #define __ISR_UART4             59
// #define __N_ISR_UART4           59
// 
// #define __ISR_SDMMC             56
// #define __N_ISR_SDMMC           56
// 
// #define __ISR_COMP              71
// #define __N_ISR_COMP            71
// 
// #define __ISR_LPUART1           77
// #define __N_ISR_LPUART1         77
// 
// #define __ISR_USB               74
// #define __N_ISR_USB             74
// 
// #define __ISR_QUADSPI           78
// #define __N_ISR_QUADSPI         78
// 
// #define __ISR_SAI1              81
// #define __N_ISR_SAI1            81
// 
// #define __ISR_SWPMI1            75
// #define __N_ISR_SWPMI1          75
// 
// #define __ISR_TSC               76
// #define __N_ISR_TSC             76
// 
// #define __ISR_LCD               77
// #define __N_ISR_LCD             77
// 
// #define __ISR_AES               78
// #define __N_ISR_AES             78
// 
// #define __ISR_RNG               79
// #define __N_ISR_RNG             79
// 
// #define __ISR_FPU               88
// #define __N_ISR_FPU             88
// 
// #define __ISR_CRS               89
// #define __N_ISR_CRS             89



#endif
