/* Copyright 2010-2012 JP Norair
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
  * @file       /apps/mpipe_bridge/app/isr_config_STM32L0.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R100
  * @date       15 September 2014
  * @brief      STM32L0 ISR Configuration
  *
  * Don't actually include this.  
  * Include <platform/config.h> or <otplatform.h> instead.
  *
  ******************************************************************************
  */

#if !defined(__ISR_CONFIG_STM32L0_H) && defined(__STM32L0__)
#define __ISR_CONFIG_STM32L_H



/// Cortex M0 NVIC specification.  For STM32L0, it should take values 1, 2, 4, 
/// or 8, or it can be defined in compiler.
#ifndef __CM0_NVIC_GROUPS
#   define __CM0_NVIC_GROUPS        1
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


/// STM32L Universal Interrupt Sources

//#define __N_ISR_WWDG                0      /* Window WatchDog Interrupt     */
//#define __N_ISR_PVD                 1      /* PVD through EXTI Line detection Interrupt          */
//#define __N_ISR_RTC_WKUP            2      /* RTC Global Interrupt */
//#define __N_ISR_RTC_Alarm           2      /* RTC Global Interrupt */
//#define __N_ISR_TAMPER_STAMP        2      /* RTC Global Interrupt */
//#define __N_ISR_CSSLSE              2      /* RTC Global Interrupt */
//#define __N_ISR_FLASH               3      /* FLASH global Interrupt        */
//#define __N_ISR_RCC                 4      /* RCC global Interrupt          */
//#define __N_ISR_CRS                 4      /* RCC global Interrupt          */
//#define __N_ISR_EXTI0               5      /* EXTI Line[1:0] Interrupt          */
//#define __N_ISR_EXTI1               5      /* EXTI Line[1:0] Interrupt          */
//#define __N_ISR_EXTI2               6      /* EXTI Line[3:2] Interrupt          */
//#define __N_ISR_EXTI3               6      /* EXTI Line[3:2] Interrupt          */
//#define __N_ISR_EXTI4               7      /* EXTI Line[15:4] Interrupt         */
//#define __N_ISR_EXTI5               7      /* External Line[15:4] Interrupts */
//#define __N_ISR_EXTI6               7      /* External Line[15:4] Interrupts */
//#define __N_ISR_EXTI7               7      /* External Line[15:4] Interrupts */
//#define __N_ISR_EXTI8               7      /* External Line[15:4] Interrupts */
//#define __N_ISR_EXTI9               7      /* External Line[15:4] Interrupts */
//#define __N_ISR_EXTI10              7      /* External Line[15:4] Interrupts */
//#define __N_ISR_EXTI11              7      /* External Line[15:4] Interrupts */
//#define __N_ISR_EXTI12              7      /* External Line[15:4] Interrupts */
//#define __N_ISR_EXTI13              7      /* External Line[15:4] Interrupts */
//#define __N_ISR_EXTI14              7      /* External Line[15:4] Interrupts */
//#define __N_ISR_EXTI15              7      /* External Line[15:4] Interrupts */
//#define __N_ISR_TSC                 8      /* TSC Interrupt */
//#define __N_ISR_DMA1_Channel1       9      /* DMA1 Channel 1 global Interrupt */
//#define __N_ISR_DMA1_Channel2       10     /* DMA1 Channel 2:3 global Interrupt */
//#define __N_ISR_DMA1_Channel3       10     /* DMA1 Channel 2:3 global Interrupt */
//#define __N_ISR_DMA1_Channel4       11     /* DMA1 Channel 4:7 global Interrupt */
//#define __N_ISR_DMA1_Channel5       11     /* DMA1 Channel 4:7 global Interrupt */
//#define __N_ISR_DMA1_Channel6       11     /* DMA1 Channel 4:7 global Interrupt */
//#define __N_ISR_DMA1_Channel7       11     /* DMA1 Channel 4:7 global Interrupt */
//#define __N_ISR_ADC1                12     /* ADC1_COMP global Interrupt         */
//#define __N_ISR_COMP                12     /* ADC1_COMP global Interrupt         */
//#define __N_ISR_LPTIM1              13     /* LPTIM1 interrupt */
//#define __N_ISR_TIM2                15     /* TIM2 global Interrupt         */
//#define __N_ISR_TIM6                17     /* TIM6_DAC global Interrupt         */
//#define __N_ISR_DAC                 17     /* TIM6_DAC global Interrupt         */
//#define __N_ISR_TIM21               20     /* TIM10 global Interrupt        */
//#define __N_ISR_TIM22               22     /* TIM11 global Interrupt        */
//#define __N_ISR_I2C1                23     /* I2C1 Global Interrupt          */
//#define __N_ISR_I2C2                24     /* I2C2 Global Interrupt          */
//#define __N_ISR_SPI1                25     /* SPI1 global Interrupt         */
//#define __N_ISR_SPI2                26     /* SPI2 global Interrupt         */
//#define __N_ISR_USART1              27     /* USART1 global Interrupt       */
//#define __N_ISR_USART2              28     /* USART2 global Interrupt       */
//#define __N_ISR_LPUART1             29     /* LPUART + AES + RNG global interrupt */
//#define __N_ISR_RNG                 29     /* LPUART + AES + RNG global interrupt */
//#define __N_ISR_AES                 29     /* LPUART + AES + RNG global interrupt */
//#define __N_ISR_USB                 31     /* USB Global Interrupt     */



//#define __ISR_WWDG                0      /* Window WatchDog Interrupt     */
//#define __ISR_PVD                 1      /* PVD through EXTI Line detection Interrupt          */
//#define __ISR_RTC_WKUP            2      /* RTC Global Interrupt */
//#define __ISR_RTC_Alarm           2      /* RTC Global Interrupt */
//#define __ISR_TAMPER_STAMP        2      /* RTC Global Interrupt */
//#define __ISR_CSSLSE              2      /* RTC Global Interrupt */
//#define __ISR_FLASH               3      /* FLASH global Interrupt        */
//#define __ISR_RCC                 4      /* RCC global Interrupt          */
//#define __ISR_CRS                 4      /* RCC global Interrupt          */
//#define __ISR_EXTI0               5      /* EXTI Line[1:0] Interrupt          */
//#define __ISR_EXTI1               5      /* EXTI Line[1:0] Interrupt          */
//#define __ISR_EXTI2               6      /* EXTI Line[3:2] Interrupt          */
//#define __ISR_EXTI3               6      /* EXTI Line[3:2] Interrupt          */
//#define __ISR_EXTI4               7      /* EXTI Line[15:4] Interrupt         */
//#define __ISR_EXTI5               7      /* External Line[15:4] Interrupts */
//#define __ISR_EXTI6               7      /* External Line[15:4] Interrupts */
//#define __ISR_EXTI7               7      /* External Line[15:4] Interrupts */
//#define __ISR_EXTI8               7      /* External Line[15:4] Interrupts */
//#define __ISR_EXTI9               7      /* External Line[15:4] Interrupts */
//#define __ISR_EXTI10              7      /* External Line[15:4] Interrupts */
//#define __ISR_EXTI11              7      /* External Line[15:4] Interrupts */
//#define __ISR_EXTI12              7      /* External Line[15:4] Interrupts */
//#define __ISR_EXTI13              7      /* External Line[15:4] Interrupts */
//#define __ISR_EXTI14              7      /* External Line[15:4] Interrupts */
//#define __ISR_EXTI15              7      /* External Line[15:4] Interrupts */
//#define __ISR_TSC                 8      /* TSC Interrupt */
//#define __ISR_DMA1_Channel1       9      /* DMA1 Channel 1 global Interrupt */
//#define __ISR_DMA1_Channel2       10     /* DMA1 Channel 2:3 global Interrupt */
//#define __ISR_DMA1_Channel3       10     /* DMA1 Channel 2:3 global Interrupt */
//#define __ISR_DMA1_Channel4       11     /* DMA1 Channel 4:7 global Interrupt */
//#define __ISR_DMA1_Channel5       11     /* DMA1 Channel 4:7 global Interrupt */
//#define __ISR_DMA1_Channel6       11     /* DMA1 Channel 4:7 global Interrupt */
//#define __ISR_DMA1_Channel7       11     /* DMA1 Channel 4:7 global Interrupt */
//#define __ISR_ADC1                12     /* ADC1_COMP global Interrupt         */
//#define __ISR_COMP                12     /* ADC1_COMP global Interrupt         */
//#define __ISR_LPTIM1              13     /* LPTIM1 interrupt */
//#define __ISR_TIM2                15     /* TIM2 global Interrupt         */
//#define __ISR_TIM6                17     /* TIM6_DAC global Interrupt         */
//#define __ISR_DAC                 17     /* TIM6_DAC global Interrupt         */
//#define __ISR_TIM21               20     /* TIM10 global Interrupt        */
//#define __ISR_TIM22               22     /* TIM11 global Interrupt        */
//#define __ISR_I2C1                23     /* I2C1 Global Interrupt          */
//#define __ISR_I2C2                24     /* I2C2 Global Interrupt          */
//#define __ISR_SPI1                25     /* SPI1 global Interrupt         */
//#define __ISR_SPI2                26     /* SPI2 global Interrupt         */
//#define __ISR_USART1              27     /* USART1 global Interrupt       */
//#define __ISR_USART2              28     /* USART2 global Interrupt       */
//#define __ISR_LPUART1             29     /* LPUART + AES + RNG global interrupt */
//#define __ISR_RNG                 29     /* LPUART + AES + RNG global interrupt */
//#define __ISR_AES                 29     /* LPUART + AES + RNG global interrupt */
//#define __ISR_USB                 31     /* USB Global Interrupt     */

#endif
