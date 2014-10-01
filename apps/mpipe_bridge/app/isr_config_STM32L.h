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
  * @file       /apps/mpipe_brigde/app/isr_config_STM32L.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R100
  * @date       10 December 2012
  * @brief      STM32L ISR Configuration
  *
  * Don't actually include this.  Include OT_platform.h instead.  This file and
  * others like it are important to the preprocessor-based configuration scheme.
  *
  * You can uncomment these if you want to negate interrupts defined by the
  * board support file (via __N_ISR...) or force-on some interrupts (via
  * __ISR...).  Some interrupts are required by OpenTag, and in certain cases
  * these will be enabled regardless of the setting in this file.
  *
  * The top-level ISRs are implemented in /otplatform/stm32l1xx/isr_STM32L.c
  ******************************************************************************
  */

#if !defined(__ISR_CONFIG_STM32L_H) && defined(__STM32L__)
#define __ISR_CONFIG_STM32L_H



/// Cortex M3 NVIC specification.  For STM32L, it should take values 1, 2, 4, 
/// or 8, or it can be defined in compiler.
#ifndef __CM3_NVIC_GROUPS
#   define __CM3_NVIC_GROUPS        1
#endif



/// Cortex M3 Processor Exception Interrupt Sources
/// Uncomment to force-negate or force-enable for your app (negate takes priority)
/// The Board Support Header should enable the typical ones automatically.

//#define __N_ISR_NonMaskableInt      -14    /* 2 Non Maskable Interrupt                 */
//#define __N_ISR_MemoryManagement    -12    /* 4 Cortex-M3 Memory Management Interrupt  */
//#define __N_ISR_BusFault            -11    /* 5 Cortex-M3 Bus Fault Interrupt          */
//#define __N_ISR_UsageFault          -10    /* 6 Cortex-M3 Usage Fault Interrupt        */
//#define __N_ISR_SVC                 -5     /* 11 Cortex-M3 SV Call Interrupt           */
//#define __N_ISR_DebugMonitor        -4     /* 12 Cortex-M3 Debug Monitor Interrupt     */
//#define __N_ISR_PendSV              -2     /* 14 Cortex-M3 Pend SV Interrupt           */
//#define __N_ISR_SysTick             -1     /* 15 Cortex-M3 System Tick Interrupt       */

//#define __ISR_NonMaskableInt        -14    /* 2 Non Maskable Interrupt                 */
//#define __ISR_MemoryManagement      -12    /* 4 Cortex-M3 Memory Management Interrupt  */
//#define __ISR_BusFault              -11    /* 5 Cortex-M3 Bus Fault Interrupt          */
//#define __ISR_UsageFault            -10    /* 6 Cortex-M3 Usage Fault Interrupt        */
//#define __ISR_SVC                   -5     /* 11 Cortex-M3 SV Call Interrupt           */
//#define __ISR_DebugMonitor          -4     /* 12 Cortex-M3 Debug Monitor Interrupt     */
//#define __ISR_PendSV                -2     /* 14 Cortex-M3 Pend SV Interrupt           */
//#define __ISR_SysTick               -1     /* 15 Cortex-M3 System Tick Interrupt       */


/// STM32L Universal Interrupt Sources

//#define __N_ISR_WWDG                0      /* Window WatchDog Interrupt     */
//#define __N_ISR_PVD                 1      /* PVD through EXTI Line detection Interrupt          */
//#define __N_ISR_TAMPER_STAMP        2      /* Tamper and Time Stamp through EXTI Line Interrupts */
//#define __N_ISR_RTC_WKUP            3      /* RTC Wakeup Timer through EXTI Line Interrupt       */
//#define __N_ISR_FLASH               4      /* FLASH global Interrupt        */
//#define __N_ISR_RCC                 5      /* RCC global Interrupt          */
//#define __N_ISR_EXTI0               6      /* EXTI Line0 Interrupt          */
//#define __N_ISR_EXTI1               7      /* EXTI Line1 Interrupt          */
//#define __N_ISR_EXTI2               8      /* EXTI Line2 Interrupt          */
//#define __N_ISR_EXTI3               9      /* EXTI Line3 Interrupt          */
//#define __N_ISR_EXTI4               10     /* EXTI Line4 Interrupt          */
//#define __N_ISR_DMA1_Channel1       11     /* DMA1 Channel 1 global Interrupt */
//#define __N_ISR_DMA1_Channel2       12     /* DMA1 Channel 2 global Interrupt */
//#define __N_ISR_DMA1_Channel3       13     /* DMA1 Channel 3 global Interrupt */
//#define __N_ISR_DMA1_Channel4       14     /* DMA1 Channel 4 global Interrupt */
//#define __N_ISR_DMA1_Channel5       15     /* DMA1 Channel 5 global Interrupt */
//#define __N_ISR_DMA1_Channel6       16     /* DMA1 Channel 6 global Interrupt */
//#define __N_ISR_DMA1_Channel7       17     /* DMA1 Channel 7 global Interrupt */
//#define __N_ISR_ADC1                18     /* ADC1 global Interrupt         */
//#define __N_ISR_USB_HP              19     /* USB High Priority Interrupt     */
//#define __N_ISR_USB_LP              20     /* USB Low Priority Interrupt      */
//#define __N_ISR_DAC                 21     /* DAC Interrupt                 */
//#define __N_ISR_COMP                22     /* Comparator through EXTI Line Interrupt */
//#define __N_ISR_EXTI5               23     /* External Line[9:5] Interrupts */
//#define __N_ISR_EXTI6               23     /* External Line[9:5] Interrupts */
//#define __N_ISR_EXTI7               23     /* External Line[9:5] Interrupts */
//#define __N_ISR_EXTI8               23     /* External Line[9:5] Interrupts */
//#define __N_ISR_EXTI9               23     /* External Line[9:5] Interrupts */
//#define __N_ISR_LCD                 24     /* LCD Interrupt                 */
//#define __N_ISR_TIM9                25     /* TIM9 global Interrupt         */
//#define __N_ISR_TIM10               26     /* TIM10 global Interrupt        */
//#define __N_ISR_TIM11               27     /* TIM11 global Interrupt        */
//#define __N_ISR_TIM2                28     /* TIM2 global Interrupt         */
//#define __N_ISR_TIM3                29     /* TIM3 global Interrupt         */
//#define __N_ISR_TIM4                30     /* TIM4 global Interrupt         */
//#define __N_ISR_I2C1_EV             31     /* I2C1 Event Interrupt          */
//#define __N_ISR_I2C1_ER             32     /* I2C1 Error Interrupt          */
//#define __N_ISR_I2C2_EV             33     /* I2C2 Event Interrupt          */
//#define __N_ISR_I2C2_ER             34     /* I2C2 Error Interrupt          */
//#define __N_ISR_SPI1                35     /* SPI1 global Interrupt         */
//#define __N_ISR_SPI2                36     /* SPI2 global Interrupt         */
//#define __N_ISR_USART1              37     /* USART1 global Interrupt       */
//#define __N_ISR_USART2              38     /* USART2 global Interrupt       */
//#define __N_ISR_USART3              39     /* USART3 global Interrupt       */
//#define __N_ISR_EXTI10              40     /* External Line[15:10] Interrupts */
//#define __N_ISR_EXTI11              40     /* External Line[15:10] Interrupts */
//#define __N_ISR_EXTI12              40     /* External Line[15:10] Interrupts */
//#define __N_ISR_EXTI13              40     /* External Line[15:10] Interrupts */
//#define __N_ISR_EXTI14              40     /* External Line[15:10] Interrupts */
//#define __N_ISR_EXTI15              40     /* External Line[15:10] Interrupts */
//#define __N_ISR_RTC_Alarm           41     /* RTC Alarm through EXTI Line Interrupt  */
//#define __N_ISR_USB_FS_WKUP         42     /* USB FS WakeUp from suspend through EXTI Line Interrupt  */
//#define __N_ISR_TIM6                43     /* TIM6 global Interrupt         */
//#define __N_ISR_TIM7                44     /* TIM7 global Interrupt         */

//#define __ISR_WWDG                  0      /* Window WatchDog Interrupt     */
//#define __ISR_PVD                   1      /* PVD through EXTI Line detection Interrupt          */
//#define __ISR_TAMPER_STAMP          2      /* Tamper and Time Stamp through EXTI Line Interrupts */
//#define __ISR_RTC_WKUP              3      /* RTC Wakeup Timer through EXTI Line Interrupt       */
//#define __ISR_FLASH                 4      /* FLASH global Interrupt        */
//#define __ISR_RCC                   5      /* RCC global Interrupt          */
//#define __ISR_EXTI0                 6      /* EXTI Line0 Interrupt          */
//#define __ISR_EXTI1                 7      /* EXTI Line1 Interrupt          */
//#define __ISR_EXTI2                 8      /* EXTI Line2 Interrupt          */
//#define __ISR_EXTI3                 9      /* EXTI Line3 Interrupt          */
//#define __ISR_EXTI4                 10     /* EXTI Line4 Interrupt          */
//#define __ISR_DMA1_Channel1         11     /* DMA1 Channel 1 global Interrupt */
//#define __ISR_DMA1_Channel2         12     /* DMA1 Channel 2 global Interrupt */
//#define __ISR_DMA1_Channel3         13     /* DMA1 Channel 3 global Interrupt */
//#define __ISR_DMA1_Channel4         14     /* DMA1 Channel 4 global Interrupt */
//#define __ISR_DMA1_Channel5         15     /* DMA1 Channel 5 global Interrupt */
//#define __ISR_DMA1_Channel6         16     /* DMA1 Channel 6 global Interrupt */
//#define __ISR_DMA1_Channel7         17     /* DMA1 Channel 7 global Interrupt */
//#define __ISR_ADC1                  18     /* ADC1 global Interrupt         */
//#define __ISR_USB_HP                19     /* USB High Priority Interrupt     */
//#define __ISR_USB_LP                20     /* USB Low Priority Interrupt      */
//#define __ISR_DAC                   21     /* DAC Interrupt                 */
//#define __ISR_COMP                  22     /* Comparator through EXTI Line Interrupt */
//#define __ISR_EXTI5                 23     /* External Line[9:5] Interrupts */
//#define __ISR_EXTI6                 23     /* External Line[9:5] Interrupts */
//#define __ISR_EXTI7                 23     /* External Line[9:5] Interrupts */
//#define __ISR_EXTI8                 23     /* External Line[9:5] Interrupts */
//#define __ISR_EXTI9                 23     /* External Line[9:5] Interrupts */
//#define __ISR_LCD                   24     /* LCD Interrupt                 */
//#define __ISR_TIM9                  25     /* TIM9 global Interrupt         */
//#define __ISR_TIM10                 26     /* TIM10 global Interrupt        */
//#define __ISR_TIM11                 27     /* TIM11 global Interrupt        */
//#define __ISR_TIM2                  28     /* TIM2 global Interrupt         */
//#define __ISR_TIM3                  29     /* TIM3 global Interrupt         */
//#define __ISR_TIM4                  30     /* TIM4 global Interrupt         */
//#define __ISR_I2C1_EV               31     /* I2C1 Event Interrupt          */
//#define __ISR_I2C1_ER               32     /* I2C1 Error Interrupt          */
//#define __ISR_I2C2_EV               33     /* I2C2 Event Interrupt          */
//#define __ISR_I2C2_ER               34     /* I2C2 Error Interrupt          */
//#define __ISR_SPI1                  35     /* SPI1 global Interrupt         */
//#define __ISR_SPI2                  36     /* SPI2 global Interrupt         */
//#define __ISR_USART1                37     /* USART1 global Interrupt       */
//#define __ISR_USART2                38     /* USART2 global Interrupt       */
//#define __ISR_USART3                39     /* USART3 global Interrupt       */
//#define __ISR_EXTI10                40     /* External Line[15:10] Interrupts */
//#define __ISR_EXTI11                40     /* External Line[15:10] Interrupts */
//#define __ISR_EXTI12                40     /* External Line[15:10] Interrupts */
//#define __ISR_EXTI13                40     /* External Line[15:10] Interrupts */
//#define __ISR_EXTI14                40     /* External Line[15:10] Interrupts */
//#define __ISR_EXTI15                40     /* External Line[15:10] Interrupts */
//#define __ISR_RTC_Alarm             41     /* RTC Alarm through EXTI Line Interrupt  */
//#define __ISR_USB_FS_WKUP           42     /* USB FS WakeUp from suspend through EXTI Line Interrupt  */
//#define __ISR_TIM6                  43     /* TIM6 global Interrupt         */
//#define __ISR_TIM7                  44     /* TIM7 global Interrupt         */



/// Additional Interrupt Sources Used in MDP and HD Parts

//#define __N_ISR_TIM5                46     /* TIM5 global Interrupt           */
//#define __N_ISR_SPI3                47     /* SPI3 global Interrupt           */
//#define __N_ISR_DMA2_Channel1       50     /* DMA2 Channel 1 global Interrupt */
//#define __N_ISR_DMA2_Channel2       51     /* DMA2 Channel 2 global Interrupt */
//#define __N_ISR_DMA2_Channel3       52     /* DMA2 Channel 3 global Interrupt */
//#define __N_ISR_DMA2_Channel4       53     /* DMA2 Channel 4 global Interrupt */
//#define __N_ISR_DMA2_Channel5       54     /* DMA2 Channel 5 global Interrupt */
//#define __N_ISR_AES                 55     /* AES global Interrupt            */
//#define __N_ISR_COMP_ACQ            56     /* Comparator Channel Acquisition global Interrupt */

//#define __ISR_TIM5                  46     /* TIM5 global Interrupt           */
//#define __ISR_SPI3                  47     /* SPI3 global Interrupt           */
//#define __ISR_DMA2_Channel1         50     /* DMA2 Channel 1 global Interrupt */
//#define __ISR_DMA2_Channel2         51     /* DMA2 Channel 2 global Interrupt */
//#define __ISR_DMA2_Channel3         52     /* DMA2 Channel 3 global Interrupt */
//#define __ISR_DMA2_Channel4         53     /* DMA2 Channel 4 global Interrupt */
//#define __ISR_DMA2_Channel5         54     /* DMA2 Channel 5 global Interrupt */
//#define __ISR_AES                   55     /* AES global Interrupt            */
//#define __ISR_COMP_ACQ              56     /* Comparator Channel Acquisition global Interrupt */



/// Additional Interrupt Sources Used in HD Parts
//#define __N_ISR_SDIO                45     /* SDIO global Interrupt  */
//#define __N_ISR_UART4               48     /* UART4 global Interrupt */
//#define __N_ISR_UART5               49     /* UART5 global Interrupt */

//#define __ISR_SDIO                  45     /* SDIO global Interrupt  */
//#define __ISR_UART4                 48     /* UART4 global Interrupt */
//#define __ISR_UART5                 49     /* UART5 global Interrupt */


#endif
