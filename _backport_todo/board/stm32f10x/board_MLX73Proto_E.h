/* Copyright 2009-2012 JP Norair
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
  */
/**
  * @file       /board/MLX73_Proto/board_STM3210EEVAL_MLX73Proto.h
  * @author     JP Norair
  * @version    V1.0
  * @date       17 November 2011
  * @brief      Board Configuration for STM3210E-EVAL plus MLX73 Prototype
  * @ingroup    Platform
  *
  * Do not include this file, include OT_platform.h
  ******************************************************************************
  */
  

#ifndef __board_MLX73Proto_E_H
#define __board_MLX73Proto_E_H

#include "build_config.h"
#include "platform_STM32F10x.h"
#include "radio_MLX73290.h"


#ifndef STM32F10X_XL
#   define STM32F10X_XL
#endif


/// Macro settings: ENABLED, DISABLED, NOT_AVAILABLE
#ifdef ENABLED
#   undef ENABLED
#endif
#define ENABLED  1

#ifdef DISABLED
#   undef DISABLED
#endif
#define DISABLED  0

#ifdef NOT_AVAILABLE
#   undef NOT_AVAILABLE
#endif
#define NOT_AVAILABLE   DISABLED




/** Supported [Known] Platforms for MLXGateway Application <BR>
  * ========================================================================<BR>
  * Uncomment the one you want to build with, OR define it through your compiler
  * toolchain.  In GCC, this would be: 
  */
  
#if defined (STM32F10X_MD_VL)
#   define MCU_STM32F100RBT6     // Cortex M3: 128KB / 8KB / 24MHz

#elif defined (STM32F10X_XL)
//#   define MCU_STM32F103ZET6     // Cortex M3: 512KB / 96KB / 72MHz
#   define MCU_STM32F103ZGT6     // Cortex M3: 1024KB / 96KB / 72MHz

#elif defined (STM32L15X_MD)
#   define MCU_STM32L152         // Cortex M3LP: 128KB / 16KB / 32MHz

#else
#   error "A supported MCU linker constant was not defined"

#endif




/** Platform Memory Configuration <BR>
  * ========================================================================<BR>
  * OpenTag needs to know where it can put Nonvolatile memory (file system) and
  * how much space it can allocate for filesystem.
  */
/*
#if defined (MCU_STM32F100RBT6)
#   define EEPROM_START_ADDR 0
#   define EEPROM_SIZE      0
#   define FLASH_START_ADDR 0x08000000
#   define FLASH_PAGE_SIZE  1024
#   define FLASH_NUM_PAGES  128
#   define FLASH_WORD_BYTES 2
#   define FLASH_FS_ALLOC   32768
#   define FLASH_FS_ADDR    (FLASH_START_ADDR + (FLASH_PAGE_SIZE*FLASH_NUM_PAGES) - FLASH_FS_ALLOC)
*/

/*
#elif defined (MCU_STM32F103ZET6)
#   define EEPROM_START_ADDR 0
#   define EEPROM_SIZE      0
#   define FLASH_START_ADDR 0x08000000
#   define FLASH_PAGE_SIZE  2048
#   define FLASH_NUM_PAGES  256
#   define FLASH_WORD_BYTES 2
#   define FLASH_FS_ALLOC   32768
#   define FLASH_FS_ADDR    (FLASH_START_ADDR + (FLASH_PAGE_SIZE*FLASH_NUM_PAGES) - FLASH_FS_ALLOC)
*/


//#elif defined (MCU_STM32F103ZGT6)
//On XL Density devices, you could use the 2nd Bank for FS
#   define SRAM_START_ADDR  0x20000000
#   define SRAM_SIZE        (96*1024)
#   define EEPROM_START_ADDR 0
#   define EEPROM_SIZE      0
#   define FLASH_START_ADDR 0x08000000
#   define FLASH_PAGE_SIZE  (2*1024)
#   define FLASH_NUM_PAGES  32
#   define FLASH_WORD_BYTES 2
#   define FLASH_FS_ALLOC   (10*1024)
#   define FLASH_FS_ADDR    0x0800D800
//#   define MIRROR_FS_ALLOC  256
//#   define MIRROR_FS_ADDR   (SRAM_START_ADDR + SRAM_SIZE - MIRROR_FS_ALLOC)

/*
#elif defined (MCU_STM32L152)
#   define EEPROM_START_ADDR 0x08080000
#   define EEPROM_SIZE      4096
#   define FLASH_START_ADDR 0x08000000
#   define FLASH_PAGE_SIZE  256
#   define FLASH_NUM_PAGES  512
#   define FLASH_WORD_BYTES 4
#   define FLASH_FS_ALLOC   32768
#   define FLASH_FS_ADDR    (FLASH_START_ADDR + (FLASH_PAGE_SIZE*FLASH_NUM_PAGES) - FLASH_FS_ALLOC)

#else 
#   error "A supported MCU ID constant was not defined.  Fix build_config.h"

#endif
*/







/** MCU Feature settings      <BR>
  * ========================================================================<BR>
  * Implemented capabilities of the STM32F103ZG
  */
#define MCU_FEATURE(VAL)                MCU_FEATURE_##VAL       // FEATURE                  AVAILABILITY
#define MCU_FEATURE_CRC                 DISABLED                // CCITT CRC16              Low
#define MCU_FEATURE_AES128              DISABLED                // AES128 engine            Moderate
#define MCU_FEATURE_ECC                 DISABLED                // ECC engine               Low
#define MCU_FEATURE_ALGE                DISABLED                // Algebraic Eraser engine  Rare/None yet
#define MCU_FEATURE_EEPROM              DISABLED                // No EEPROM on STM32F10x

#define MCU_FEATURE_RADIODMA            ENABLED
#define MCU_FEATURE_RADIODMA_TXBYTES    0
#define MCU_FEATURE_RADIODMA_RXBYTES    0

#define MCU_FEATURE_MAPEEPROM           DISABLED                
#define MCU_FEATURE_MPIPEVCOM           ENABLED
#define MCU_FEATURE_MPIPEDMA            (ENABLED && (MCU_FEATURE_MPIPEVCOM == DISABLED))

///@todo include later
#define MCU_FEATURE_MEMCPYDMA           DISABLED





/** Note: Clocking for this platform      <BR>
  * ========================================================================<BR>
  * The STM32 can be set at different clock rates.  I generally recommend using
  * the highest clock speed that can be used without activating the PLL.  There
  * is some marginal benefit as well if you keep wait states to 0.
  *
  * Flash Wait States per Clock speed:                              <BR>
  * STM32F: Vnom:   0 < clock <= 24MHz :    0 wait states           <BR>
  * STM32F: Vnom:   24 < clock <= 48MHz :   1 wait state            <BR>
  * STM32F: Vnom:   48 < clock <= 72MHz :   2 wait states           <BR>
  * STM32L: 1.2V:   0 < clock <= 2MHz :     0 wait states           <BR>
  * STM32L: 1.2V:   2 < clock <= 4MHz :     1 wait state            <BR>
  * STM32L: 1.5V:   0 < clock <= 8MHz :     0 wait states           <BR>
  * STM32L: 1.5V:   8 < clock <= 16MHz :    1 wait state            <BR>
  * STM32L: 1.8V:   0 < clock <= 16MHz :    0 wait states           <BR>
  * STM32L: 1.8V:   16 < clock <= 32MHz :   1 wait state            <BR>
  *
  */
#define MCU_PARAM(VAL)                  MCU_PARAM_##VAL
#define MCU_PARAM_LFXTALHz              32768                   // Uncomment (and change) if using LF XTAL
#define MCU_PARAM_LFXTALtol             0.0001                  // Uncomment (and change) if using LF XTAL
//#define MCU_PARAM_LFOSCHz               32768                   // Uncomment (and change) if using LF OSC
//#define MCU_PARAM_LFOSCtol              0.0001                  // Uncomment (and change) if using LF OSC
#define MCU_PARAM_XTALHz                48000000                // Uncomment (and change) if using XTAL
#define MCU_PARAM_XTALtol               0.0001                  // Uncomment (and change) if using XTAL
//#define MCU_PARAM_OSCHz                 8000000                 // OSC can do 8 MHz
//#define MCU_PARAM_OSCtol                0.02                    // STM32 Osc typ +/- 1%
  
#if defined(MCU_PARAM_LFXTALHz)
#   define PLATFORM_LSCLOCK_HZ      MCU_PARAM_LFXTALHz
#   define PLATFORM_LSCLOCK_ERROR   MCU_PARAM_LFXTALtol

#elif defined(MCU_PARAM_LFOSCHz)
#   define PLATFORM_LSCLOCK_HZ      MCU_PARAM_LFOSCHz
#   define PLATFORM_LSCLOCK_ERROR   MCU_PARAM_LFOSCtol

#endif

#if defined(MCU_PARAM_XTALHz)
#   define PLATFORM_HSCLOCK_HZ      MCU_PARAM_XTALHz
#   define PLATFORM_HSCLOCK_ERROR   MCU_PARAM_XTALtol

#elif defined(MCU_PARAM_OSCHz)
#   define PLATFORM_HSCLOCK_HZ      MCU_PARAM_OSCHz
#   define PLATFORM_HSCLOCK_ERROR   MCU_PARAM_OSCtol

#endif




/** Peripheral definitions for this platform <BR>
  * ========================================================================<BR>
  * OT_GPTIM:   General Purpose Timer used by OpenTag kernel (best is TIM9) <BR>
  * OT_TRIG:    Optional test trigger usable in OpenTag apps (often LEDs)   <BR>
  * MPIPE:      USB or UART to use for the MPipe                            <BR>
  * RADIO_SPI:  SPI used to communicate with RF core (best is SPI1)         <BR>
  * RADIO_IRQx: GPIO pin to use for radio IRQ signal (best is PA0-3)        <BR>
  * RADIO_DMA:  The SPIbus uses a DMA to automate the transfer management.
  *             There are some limits on which DMAs can be used on each SPI. <BR>
  * <PRE>
  * | RADIO_SPI | RADIO_DMA | RX Chan | TX Chan |
  * +-----------+-----------+---------+---------+
  * |   SPI1    |   DMA1    | Chan 2  | Chan 3  |
  * |   SPI2    |   DMA1    | Chan 4  | Chan 5  |
  * |   SPI3    |   DMA2    | Chan 1  | Chan 2  |
  * </PRE>
  *
  * @note It is best to set the SPI as fast as possible, but < 10 MHz because
  * MLX73 has maximum 10 MHz SPI Clock.  SPI has a minimum clock divider of 2, 
  * therefore the system clock is ideally a power-of-2 multiple of 8-10 MHz.
  */

#define OT_GPTIM_INDEX      2                           // GPTIM INDEX: TIMx, 0 for using Systick, or -1 for RTC
#define OT_GPTIM            TIM2
#define OT_GPTIM_IRQn       TIM2_IRQn
#define OT_GPTIM_CLOCK      PLATFORM_HSCLOCK_HZ         // Alternatively: PLATFORM_LSCLOCK_HZ
#define OT_GPTIM_RES        1024
#if (OT_GPTIM_CLOCK == PLATFORM_LSCLOCK_HZ)
#   define OT_GPIM_ERROR    PLATFORM_LSCLOCK_ERROR
#else
#   define OT_GPIM_ERROR    PLATFORM_HSCLOCK_ERROR
#endif
#define OT_GPTIM_ERRDIV     (1/OT_GPIM_ERROR)
#define TI_TO_CLK(VAL)      ((OT_GPTIM_RES/1024)*VAL)
#define CLK_TO_TI(VAL)      (VAL/(OT_GPTIM_RES/1024))


#define OT_TRIG1_PORTNUM    'F'
#define OT_TRIG1_PORT       GPIOF
#define OT_TRIG1_PIN        GPIO_Pin_6
#define OT_TRIG1_HIDRIVE    ENABLED         // Use high-current option
#define OT_TRIG2_PORTNUM    'F'
#define OT_TRIG2_PORT       GPIOF
#define OT_TRIG2_PIN        GPIO_Pin_7
#define OT_TRIG2_HIDRIVE    ENABLED

#define RADIO_SPI_INDEX     1
#define RADIO_SPI           SPI1
#define RADIO_SPI_IRQn      SPI1_IRQn

#define RADIO_SPI_GPIO      GPIO_AF_SPI1                // must be same as RADIO_SPI
#define RADIO_SPI_REMAP     DISABLED                    // puts port on "Remap" pins
#define RADIO_SPICS_PORT    GPIOA                       // HW NSS not used, but we use same pin
#define RADIO_SPICS_PIN     GPIO_Pin_4
#define RADIO_SPISCK_PORT   GPIOA
#define RADIO_SPISCK_PIN    GPIO_Pin_5
#define RADIO_SPIMISO_PORT  GPIOA
#define RADIO_SPIMISO_PIN   GPIO_Pin_6
#define RADIO_SPIMOSI_PORT  GPIOA
#define RADIO_SPIMOSI_PIN   GPIO_Pin_7

#define RADIO_SPI_CLKSRC    (PLATFORM_HSCLOCK_HZ / (1 + ((RADIO_SPI_INDEX != 1)*3)))
#if (RADIO_SPI_CLKSRC < 20000000)
#	define RADIO_SPI_PRESCALER SPI_BaudRatePrescaler_2
#elif (RADIO_SPI_CLKSRC < 40000000)
#	define RADIO_SPI_PRESCALER SPI_BaudRatePrescaler_4
#elif (RADIO_SPI_CLKSRC < 80000000)
#	define RADIO_SPI_PRESCALER SPI_BaudRatePrescaler_8
#else //no STM32 goes beyond 160MHz, even the psycho F4 models
#	define RADIO_SPI_PRESCALER SPI_BaudRatePrescaler_16
#endif



#define RADIO_DMA_INDEX     1
#define RADIO_DMA_RXINDEX   12
#define RADIO_DMA_TXINDEX   13
#define RADIO_DMA           0 //disabled, otherwise DMA1
#define RADIO_DMA_RXCHAN    DMA1_Channel2				//For SPI1
#define RADIO_DMA_RXIRQn    DMA1_Channel2_IRQn
#define RADIO_DMA_RXINT  	DMA1_IT_TC2
#define RADIO_DMA_TXCHAN    DMA1_Channel3				//For SPI1
#define RADIO_DMA_TXIRQn    DMA1_Channel3_IRQn
#define RADIO_DMA_TXINT  	DMA1_IT_TC3

#define RADIO_IRQ0_PORT     GPIOB           // ideally should also be wakeup pin
#define RADIO_IRQ0_PIN      GPIO_Pin_0
#define RADIO_IRQ0_SRCPORT  GPIO_PortSourceGPIOB
#define RADIO_IRQ0_SRCPIN   GPIO_PinSource0
#define RADIO_IRQ0_SRCLINE  0
#define RADIO_IRQ0_IRQn     EXTI0_IRQn

#define RADIO_IRQ1_PORT     GPIOA
#define RADIO_IRQ1_PIN      GPIO_Pin_1
#define RADIO_IRQ1_SRCPORT  GPIO_PortSourceGPIOA
#define RADIO_IRQ1_SRCPIN   GPIO_PinSource1
#define RADIO_IRQ1_SRCLINE  1
#define RADIO_IRQ1_IRQn     EXTI1_IRQn

#define RADIO_IRQ2_PORT     GPIOA
#define RADIO_IRQ2_PIN      GPIO_Pin_2
#define RADIO_IRQ2_SRCPORT  GPIO_PortSourceGPIOA
#define RADIO_IRQ2_SRCPIN   GPIO_PinSource2
#define RADIO_IRQ2_SRCLINE  2
#define RADIO_IRQ2_IRQn     EXTI2_IRQn

#define RADIO_IRQ3_PORT     GPIOA
#define RADIO_IRQ3_PIN      GPIO_Pin_3
#define RADIO_IRQ3_SRCPORT  GPIO_PortSourceGPIOA
#define RADIO_IRQ3_SRCPIN   GPIO_PinSource3
#define RADIO_IRQ3_SRCLINE  3
#define RADIO_IRQ3_IRQn     EXTI3_IRQn



// Uncomment if using USB for MPIPE
#if defined(STM32F10X_CL) 
#   define MPIPE_USB_IRQn       ((ot_u32)OTG_FS_IRQn)
#   define MPIPE_USBCLK(VAL)    do {\
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA+RCC_APB2Periph_GPIOB, VAL); \
        RCC_OTGFSCLKConfig(RCC_OTGFSCLKSource_PLLVCO_Div3); \
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, VAL); \
        } while(0)
#else
#   if (MCU_PARAM_XTALHz == 48000000)
#       define MPIPE_USBCLK_DIV RCC_USBCLKSource_PLLCLK_Div1
#   elif (MCU_PARAM_XTALHz == 72000000)
#       define MPIPE_USBCLK_DIV RCC_USBCLKSource_PLLCLK_1Div5
#   else
#       error "To use USB, the system clock MUST be configured to 48 or 72 MHz via the HSE."
#   endif
#   define MPIPE_USB_IRQn       ((ot_u32)USB_LP_CAN1_RX0_IRQn)
#   define MPIPE_USBCLK(VAL)    do { \
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA+RCC_APB2Periph_GPIOB, VAL); \
        RCC_USBCLKConfig(MPIPE_USBCLK_DIV); \
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, VAL); \
        } while(0)
#endif
#define MPIPE_USB_INDEX     0
#define MPIPE_USB           USB0
#define MPIPE_REMAP         DISABLED
#define MPIPE_REMAP_TYPE    0           //USB generally is not remappable
#define MPIPE_USBDC_PORT    GPIOB
#define MPIPE_USBDC_PIN     GPIO_Pin_14
#define MPIPE_USBDP_PORT    GPIOA
#define MPIPE_USBDP_PIN     GPIO_Pin_12
#define MPIPE_USBDM_PORT    GPIOA
#define MPIPE_USBDM_PIN     GPIO_Pin_11


    
// Uncomment if using UART for MPIPE
/* 
#define MPIPE_UART_INDEX 1
#define MPIPE_UART       USART1
#define MPIPE_UART_IRQn  USART1_IRQn
#define MPIPE_REMAP      DISABLED                 // ENABLED or DISABLED (very platform dependent)
#define MPIPE_REMAP_TYPE GPIO_Remap_USART1       // Check stm32f10x_gpio.c line 492
#define MPIPE_TXD_PORT   GPIOA
#define MPIPE_TXD_PIN    GPIO_Pin_9      //B6 on remap
#define MPIPE_RXD_PORT   GPIOA
#define MPIPE_RXD_PIN    GPIO_Pin_10     //B7 on remap

#define MPIPE_DMA_INDEX  1
#define MPIPE_DMA_RXINDEX 15
#define MPIPE_DMA_TXINDEX 14
#define MPIPE_DMA        DMA1
#define MPIPE_DMA_RXCHAN DMA1_Channel5       // Ch7 on USART2
#define MPIPE_DMA_RXIRQn DMA1_Channel5_IRQn
#define MPIPE_DMA_RXINT  DMA1_IT_GL5
#define MPIPE_DMA_TXCHAN DMA1_Channel4       // Ch6 on USART2
#define MPIPE_DMA_TXIRQn DMA1_Channel4_IRQn
#define MPIPE_DMA_TXINT  DMA1_IT_GL4
*/


#ifndef MPIPE_USB
#   define MPIPE_USB    0
#endif
#ifndef MPIPE_UART
#   define MPIPE_UART   0
#endif
#ifndef MPIPE_SPI
#   define MPIPE_SPI    0
#endif
#ifndef MPIPE_I2C
#   define MPIPE_I2C    0
#endif
#ifndef MPIPE_CAN
#   define MPIPE_CAN    0
#endif





#define OT_DEBUG_PERIPHERALS    (DBGMCU_SLEEP    | \
                                DBGMCU_STOP      | \
                                DBGMCU_STANDBY   | \
                                DBGMCU_IWDG_STOP | \
                                DBGMCU_WWDG_STOP | \
                                DBGMCU_TIM2_STOP)






/******* ALL SHIT BELOW HERE IS SUBJECT TO REDEFINITION **********/


/** Flash Setup Constants     <BR>
  * ========================================================================<BR>
  * The veelite module (filesystem) needs to know some things.  Mainly, this
  * information comes from build_config.h
  */
#define FLASH_START_PAGE         ((FLASH_FS_ADDR - FLASH_START_ADDR)/FLASH_PAGE_SIZE)
#define FLASH_WORD_BITS          (FLASH_WORD_BYTES*8)
#define FLASH_PAGE_ADDR(VAL)     (FLASH_START_ADDR + ( (VAL) * FLASH_PAGE_SIZE) )
#define EEPROM_END_ADDR          (EEPROM_START_ADDR + (EEPROM_SIZE-1))




/** Flash Memory Setup: 
  * "OTF" means "Open Tag Flash," but if flash is not used, it just means 
  * storage memory.  Unfortunately this does not begin with F.
  */
#define OTF_VWORM_PAGES         (FLASH_FS_ALLOC/FLASH_PAGE_SIZE)
#define OTF_VWORM_FALLOW_PAGES  3
#define OTF_VWORM_PAGESIZE      FLASH_PAGE_SIZE
#define OTF_VWORM_WORD_BYTES    FLASH_WORD_BYTES
#define OTF_VWORM_WORD_BITS     FLASH_WORD_BITS
#define OTF_VWORM_SIZE          (OTF_VWORM_PAGES * OTF_VWORM_PAGESIZE)
#define OTF_VWORM_START_PAGE    ((FLASH_FS_ADDR-FLASH_START_ADDR)/FLASH_PAGE_SIZE)
#define OTF_VWORM_START_ADDR    FLASH_FS_ADDR

#define OTF_CRC_TABLE           ENABLED //toggle
#define OTF_UHF_TABLE           DISABLED
#define OTF_UHF_TABLESIZE       0
#define OTF_M1_ENCODE_TABLE     DISABLED
#define OTF_M2_ENCODE_TABLE     ENABLED

// Total number of pages taken from program memory
#define OTF_TOTAL_PAGES         (OTF_VWORM_PAGES)




/** Abstracted Flash Organization: 
  * OpenTag uses Flash to store 2 kinds of data.  The default setup puts 
  * Filesystem memory in the back.
  * 1. Program code (obviously)
  * 2. Filesystem Memory
  *
  * FLASH_xxx constants are defined in the platform_config_xxx.h file.  
  */
#define OTF_TOTAL_SIZE          FLASH_FS_ALLOC
#define OTF_START_PAGE          OTF_VWORM_START_PAGE
#define OTF_START_ADDR          FLASH_FS_ADDR

#define OTF_VWORM_LAST_PAGE     (OTF_VWORM_START_PAGE + OTF_VWORM_PAGES - 1)
#define OTF_VWORM_END_ADDR      (FLASH_FS_ADDR + FLASH_FS_ALLOC - 1)







#endif
