/* Copyright 2009-2014 JP Norair
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
  * @file       /include/platform/hw/STM32WLxx_config.h
  * @author     JP Norair
  * @version    R100
  * @date       20 Dec 2021
  * @brief      Platform Library Macros and Functions for STM32WL
  * @ingroup    Platform
  *
  ******************************************************************************
  */
  

#ifndef __PLATFORM_STM32WLxx_CONFIG_H
#define __PLATFORM_STM32WLxx_CONFIG_H

#include <otstd.h>

//Stored in App Code folder
#include <app/board_config.h>


#include <app/isr_config_STM32WL55.h>


/** Long Chip-Select Section <BR>
  * ========================================================================<BR>
  * In your project gcc defines or in the board config file you should set the
  * part number for the MCU on the board.  The preprocessing below will set up
  * other attributes based on the part number.
  */

#define __STM32__
#define __STM32WL__
#define __STM32WLxx__

#define STM32
#define STM32WL
#define STM32WLxx


// 48 Pin STM32WLs are in UFQFPN48 packages
// Port A: { 0:15 } 
// Port B: { 0:7 } 
// Port C: { 14, 15 }
#if (   defined(__STM32WLE5C8__) \
    ||  defined(__STM32WLE5CB__) \
    ||  defined(__STM32WLE5CC__) \
    ||  defined(__STM32WL55C8__) \
    ||  defined(__STM32WL55CB__) \
    ||  defined(__STM32WL55CC__) \
    )
    
#   define __STM32WLxxCx__
#   define STM32WLxxCx
#   define MCU_PARAM_PORTS  3

#   if (   defined(__STM32WLE5CC__) \
       ||  defined(__STM32WL55CC__) \
       )
#       define FLASH_AVAILABLE       (256*1024)
#       define SRAM_AVAILABLE        (64*1024)    
#       define EEPROM_AVAILABLE      (0*1024)
#       define __STM32WLxxCC__
#       define STM32WLxxCC

#   elif ( defined(__STM32WLE5CB__) \
       ||  defined(__STM32WL55CB__) \
       )
#       define FLASH_AVAILABLE       (128*1024)
#       define SRAM_AVAILABLE        (48*1024)    
#       define EEPROM_AVAILABLE      (0*1024)
#       define __STM32WLxxCB__
#       define STM32WLxxCB

#   else
#       define FLASH_AVAILABLE       (64*1024)
#       define SRAM_AVAILABLE        (20*1024)    
#       define EEPROM_AVAILABLE      (0*1024)
#       define __STM32WLxxC8__
#       define STM32WLxxC8

#	endif

// 73 Pin BGA
#elif ( defined(__STM32WLE5J8__) \
    ||  defined(__STM32WLE5JB__) \
    ||  defined(__STM32WLE5JC__) \
    ||  defined(__STM32WL55J8__) \
    ||  defined(__STM32WL55JB__) \
    ||  defined(__STM32WL55JC__) \
    )
#   define __STM32WLxxJx__
#   define STM32WLxxJx
#   define MCU_PARAM_PORTS  3

#   if (   defined(__STM32WLE5JC__) \
       ||  defined(__STM32WL55JC__) \
       )
#       define FLASH_AVAILABLE       (256*1024)
#       define SRAM_AVAILABLE        (64*1024)    
#       define EEPROM_AVAILABLE      (0*1024)
#       define __STM32WLxxJC__
#       define STM32WLxxJC

#   elif ( defined(__STM32WLE5JB__) \
       ||  defined(__STM32WL55JB__) \
       )
#       define FLASH_AVAILABLE       (128*1024)
#       define SRAM_AVAILABLE        (48*1024)    
#       define EEPROM_AVAILABLE      (0*1024)
#       define __STM32WLxxJB__
#       define STM32WLxxJB

#   else
#       define FLASH_AVAILABLE       (64*1024)
#       define SRAM_AVAILABLE        (20*1024)    
#       define EEPROM_AVAILABLE      (0*1024)
#       define __STM32WLxxJ8__
#       define STM32WLxxJ8

#	endif

// 59 Pin CSP
#elif ( defined(__STM32WLE5U8__) \
    ||  defined(__STM32WLE5UB__) \
    ||  defined(__STM32WLE5UC__) \
    ||  defined(__STM32WL55U8__) \
    ||  defined(__STM32WL55UB__) \
    ||  defined(__STM32WL55UC__) \
    )
#   define __STM32WLxxUx__
#   define STM32WLxxUx
#   define MCU_PARAM_PORTS  3

#   if (   defined(__STM32WLE5UC__) \
       ||  defined(__STM32WL55UC__) \
       )
#       define FLASH_AVAILABLE       (256*1024)
#       define SRAM_AVAILABLE        (64*1024)    
#       define EEPROM_AVAILABLE      (0*1024)
#       define __STM32WLxxUC__
#       define STM32WLxxUC

#   elif ( defined(__STM32WLE5UB__) \
       ||  defined(__STM32WL55UB__) \
       )
#       define FLASH_AVAILABLE       (128*1024)
#       define SRAM_AVAILABLE        (48*1024)    
#       define EEPROM_AVAILABLE      (0*1024)
#       define __STM32WLxxUB__
#       define STM32WLxxUB

#   else
#       define FLASH_AVAILABLE       (64*1024)
#       define SRAM_AVAILABLE        (20*1024)    
#       define EEPROM_AVAILABLE      (0*1024)
#       define __STM32WLxxU8__
#       define STM32WLxxU8

#	endif

// Unknown Part, so use the lowest-common-denominator
#else
#   warning "Unknown STM32WL defined.  Defaulting to STM32WLxxC8"
#   define MCU_PARAM_PORTS  3
#   define FLASH_AVAILABLE       (64*1024)
#   define SRAM_AVAILABLE        (20*1024)    
#   define EEPROM_AVAILABLE      (0*1024)
#   define __STM32WLxxCx__
#   define STM32WLxxCx
#   define __STM32WLxxC8__
#   define STM32WLxxC8

#endif






/** Back to regularly-sheduled program <BR>
  * ========================================================================<BR>
  */

// From lib/CMSIS/Device
#include <stm32wlxx.h>

// From lib/CMSIS/Include
#ifdef CORE_CM0PLUS
#   ifndef CORTEX_M0PLUS
#       define CORTEX_M0PLUS
#   endif
#   include <cm0plus_endian.h>
#   include <cm0plus_byteswap.h>
#   include <cm0plus_bitrotate.h>
//#   include <cm0plus_countleadingzeros.h>
#   include <cm0plus_saturation.h>
#else
#   ifndef CORTEX_M4
#       define CORTEX_M4
#   endif
#   include <cm4_endian.h>
#   include <cm4_byteswap.h>
#   include <cm4_bitrotate.h>
#   include <cm4_countleadingzeros.h>
#   include <cm4_saturation.h>
#endif

// Definitions removed from recent STM32 libs
#define GPIO_MODER_INPUT            ((uint32_t)0x00000000)
#define GPIO_MODER_OUTPUT           ((uint32_t)0x00000001)
#define GPIO_MODER_ALT              ((uint32_t)0x00000002)
#define GPIO_MODER_ANALOG           ((uint32_t)0x00000003)
#define GPIO_MODER_IN               ((uint32_t)0x00000000)
#define GPIO_MODER_OUT              ((uint32_t)0x00000001)
#define GPIO_MODER_ALT              ((uint32_t)0x00000002)
#define GPIO_MODER_AN               ((uint32_t)0x00000003)
#define GPIO_OTYPER_PUSHPULL        ((uint32_t)0x00000000)
#define GPIO_OTYPER_OPENDRAIN       ((uint32_t)0x00000001)
#define GPIO_OSPEEDR_400kHz         ((uint32_t)0x00000000)
#define GPIO_OSPEEDR_2MHz           ((uint32_t)0x00000001) 
#define GPIO_OSPEEDR_10MHz          ((uint32_t)0x00000002)
#define GPIO_OSPEEDR_40MHz          ((uint32_t)0x00000003)








/** Cryptographic Library Setup      <BR>
  * ========================================================================<BR>
  * Needed to build AES128 or other types of Crypto
  */

  
  

/** Special Platform functions for STM32WL      <BR>
  * ========================================================================<BR>
  * These must be defined before including platform_config.h, so that they
  * can be used for inline functions.
  */
void platform_ext_pllon();
void platform_ext_plloff();
void platform_ext_wakefromstop();
void platform_ext_hsitrim();
void platform_ext_msitrim();
ot_u16 platform_ext_lsihz();







/** Platform Support settings      <BR>
  * ========================================================================<BR>
  * STM32 is little endian with 4 byte pointer (32 bits), and at this stage it
  * can be compiled using GCC (RIDE, most other IDE's) or IAR's proprietary
  * compiler.
  */

#define PLATFORM(VAL)           PLATFORM_##VAL
#define PLATFORM_STM32WLxx

#ifndef __LITTLE_ENDIAN__
#   error "Endian-ness misdefined, should be __LITTLE_ENDIAN__ (check build_config.h)"
#endif
#define PLATFORM_POINTER_SIZE           4               // How many bytes is a pointer?
#define PLATFORM_ENDIAN16(VAR16)        __REV16(VAR16)  // Big-endian to Platform-endian
#define PLATFORM_ENDIAN32(VAR32)        __REV(VAR32)    // Big-endian to Platform-endian
#define PLATFORM_ENDIAN16_C(CONST16)    (ot_u16)( (((ot_u16)CONST16) << 8) | (((ot_u16)CONST16) >> 8) )






/** STM32L family MCU settings     <BR>
  * ========================================================================<BR>
  * STM32L has a peculiar FLASH design where the erased value is 0 instead of
  * 1.  This requires some unusual setup for the Memory configuration.
  * 
  * @todo get Hardware CRC16 working.  Currently doesn't give correct calcs.
  */
#define MCU_FEATURE(VAL)        MCU_FEATURE_##VAL   // FEATURE                  NOTE
#define MCU_FEATURE_SVMONITOR   ENABLED             // Auto Low V powerdown     On many MCUs
#define MCU_FEATURE_CRC16       DISABLED            // CRC16                    Exists, but needs to be tested
#define MCU_FEATURE_CRC         MCU_FEATURE_CRC16   // Legacy definition
#define MCU_FEATURE_ECC         DISABLED            // ECC engine               Exists, but needs to be tested

// True Random Number HW
#define MCU_FEATURE_TRNG        ENABLED
// AES HW
#define MCU_FEATURE_AES128      ENABLED

#define MCU_TYPE(VAL)                   MCU_TYPE_##VAL
#define MCU_TYPE_PTRINT                 ot_s32
#define MCU_TYPE_PTRUINT                ot_u32

#define MCU_PARAM(VAL)                  MCU_PARAM_##VAL
#define MCU_PARAM_POINTERSIZE           4
#define MCU_PARAM_ERRPTR                ((ot_s32)-1)
#define MCU_PARAM_UART_9600BPS          9600
#define MCU_PARAM_UART_28800BPS         28800
#define MCU_PARAM_UART_57600BPS         57600
#define MCU_PARAM_UART_115200BPS        115200
#define MCU_PARAM_UART_250000BPS        250000
#define MCU_PARAM_UART_500000BPS        500000


/** LPTIM Manipulations      <BR>
  * ========================================================================<BR>
  * Tasking waits longer than this (in ticks) will loop through the scheduler.
  * It is pretty rare with OpenTag for waits to be longer than 2 seconds
  * (2048 ticks), so this boundary rarely occurs.
  *
  * The LPTIM used for STM32WL is oversampled beyond 1024 Hz (8192 Hz) in
  * order to workaround the 4-clock update lag of the LPTIM Compare Register.
  */
#define MCU_PARAM_LPTIM_OVERSAMPLE		3
#define MCU_PARAM_LPTIM_SHIFT			0
#define MCU_PARAM_LPTIM_PS_SHIFT        (MCU_PARAM_LPTIM_OVERSAMPLE+MCU_PARAM_LPTIM_SHIFT)
#define MCU_PARAM_LPTIM_LIMIT           ((65536 >> MCU_PARAM_LPTIM_PS_SHIFT) - 260)




#define SRAM_START_ADDR         0x20000000
#define SRAM2_START_ADDR        0x10000000
#define FLASH_START_ADDR        0x08000000
#define FLASH_START_PAGE        0
#ifndef FLASH_PAGE_SIZE
#   define FLASH_PAGE_SIZE      2048
#endif
#define FLASH_WORD_BYTES        8
#define FLASH_WORD_BITS         (FLASH_WORD_BYTES*8)
#define FLASH_PAGE_ADDR(VAL)    (FLASH_START_ADDR + ( (VAL) * FLASH_PAGE_SIZE) )

// STM32L has peculiar flash, where 0 is the erase value.  
// On every other flash, it is 1 (i.e. FFFF).
//#define NULL_vaddr              0x0000




/** Chip Settings  <BR>
  * ========================================================================<BR>
  * @todo Check if this is even needed.  ARMGCC is dominant compiler
  */



/** Data section Nomenclature  <BR>
  * ========================================================================<BR>
  * @todo Check if this is even needed.  ARMGCC is dominant compiler
  */




/** Interrupt Nomenclature  <BR>
  * ========================================================================<BR>
  * Use the "platform_isr_...()" functions in the code.  These get wrapped into
  * the official ISR function, which is defined to STM32L.  The wrapper allows
  * low power system calls (sleeping calls) to be abstracted from the user.
  *
  * For STM32L, the interrupts are defined in a startup assembly file, from:
  * /lib/CMSIS/Device/STM32L0xx/startup/...
  * They each have special names that are defined in this file.
  *
  * The board support header should define __ISR_... constants for ISRs that 
  * are suitable for the board.  The Application may define or negate ISRs as
  * it needs, also, in its isr_config_XXX.h file.
  *
  * These functions are implemented in the platform module, driver modules, or
  * in the application.
  */

#if !defined(OT_PARAM_SYSTHREADS)
#   define OT_PARAM_SYSTHREADS      0
#endif
#if !defined(OT_PARAM_SSTACK_ALLOC)
#   define OT_PARAM_SSTACK_ALLOC    (512)
#elif (OT_PARAM_SSTACK_ALLOC & 3)
#   error "OT_PARAM_SSTACK_ALLOC must be 32-bit aligned."
#endif

#if (OT_PARAM_SYSTHREADS != 0)
#   if !defined(OT_PARAM_TSTACK_ALLOC)
#       define OT_PARAM_TSTACK_ALLOC    (OT_PARAM_SYSTHREADS*160)
#   endif
#   if ((OT_PARAM_TSTACK_ALLOC/OT_PARAM_SYSTHREADS) < 80)
#       error "Specified Thread Stack Size allocates less than 80 bytes per thread stack."
#   endif
#   if (OT_PARAM_TSTACK_ALLOC & 3)
#       error "OT_PARAM_TSTACK_ALLOC must be 32-bit aligned."
#   endif
#endif


#ifdef CORE_CM0PLUS

#ifndef __CM0_NVIC_GROUPS
#   define __CM0_NVIC_GROUPS    4
#endif

#   ifndef __KERNEL_HICCULP__
#       define __KERNEL_HICCULP__
#   endif
#   define PLATFORM_NVIC_KERNEL_GROUP   b00
#   define PLATFORM_NVIC_RF_GROUP       b01
#   define PLATFORM_NVIC_IO_GROUP       b01
#   define PLATFORM_NVIC_INT_GROUP      b10
#   define PLATFORM_NVIC_TASK_GROUP     b11

///@todo these platform_isr...() functions are from STM32L4

/// These platform_isr_... functions are called from core_isr.c, from within
/// the primary ISRs.  They can be used in your application freely.  All entry 
/// and exit hooks that OpenTag may need to put around your ISR code are 
/// managed in core_isr.c and you, the app developer, do not need to worry 
/// about those unless you are hacking the kernel.
/// 
/// @note Some of the STM32WL interrupts are abstracted from Hardware into 
/// "synthetic" interrupts.  We do this to simplify app development, since some
/// of the hardware interrupts are grouped together inconveniently.

// Trust Zone Illegal Access has its own IRQ
void platform_isr_tz(void);

// PVD/PVM has a shared IRQ, but only a single ISR as well
void platform_isr_pvdpvm(void);

// RTC, Tamper, and LSE-Clock-Security have a shared IRQ
void platform_isr_rtcalarm(void);
void platform_isr_rtcwakeup(void);
void platform_isr_rtcssru(void);
void platform_isr_tamper(void);
void platform_isr_timestamp(void);
void platform_isr_lsecss(void);

// CPU1-SEV, Flash, and RCC have a shared IRQ
void platform_isr_c1sev(void);
void platform_isr_flash(void);
void platform_isr_rcc(void);

// EXTI0-1 have a shared IRQ
void platform_isr_exti0(void);
void platform_isr_exti1(void);

// EXTI2-3 have a shared IRQ
void platform_isr_exti2(void);
void platform_isr_exti3(void);

// EXTI4-15 have a shared IRQ
void platform_isr_exti4(void);
void platform_isr_exti5(void);
void platform_isr_exti6(void);
void platform_isr_exti7(void);
void platform_isr_exti8(void);
void platform_isr_exti9(void);
void platform_isr_exti10(void);
void platform_isr_exti11(void);
void platform_isr_exti12(void);
void platform_isr_exti13(void);
void platform_isr_exti14(void);
void platform_isr_exti15(void);

// COMP1, COMP2, ADC, and DAC have a shared IRQ
void platform_isr_comp1(void);
void platform_isr_comp2(void);
void platform_isr_adc(void);
void platform_isr_dac(void);

// DMA1 channels have a shared IRQ
void platform_isr_dma1ch1(void);
void platform_isr_dma1ch2(void);
void platform_isr_dma1ch3(void);
void platform_isr_dma1ch4(void);
void platform_isr_dma1ch5(void);
void platform_isr_dma1ch6(void);
void platform_isr_dma1ch7(void);

// DMA2 channels and DMAMUX have a shared IRQ
void platform_isr_dma2ch1(void);
void platform_isr_dma2ch2(void);
void platform_isr_dma2ch3(void);
void platform_isr_dma2ch4(void);
void platform_isr_dma2ch5(void);
void platform_isr_dma2ch6(void);
void platform_isr_dma2ch7(void);
void platform_isr_dmamux_ovr0(void);
void platform_isr_dmamux_ovr1(void);
void platform_isr_dmamux_ovr2(void);
void platform_isr_dmamux_ovr3(void);

// LPTIM1 has its own IRQ
void platform_isr_lptim1(void);

// LPTIM2 has its own IRQ
void platform_isr_lptim2(void);

// LPTIM3 has its own IRQ
void platform_isr_lptim3(void);

// TIM1 has its own IRQ
void platform_isr_tim1(void);

// TIM2 has its own IRQ
void platform_isr_tim2(void);

// TIM16 has its own IRQ
void platform_isr_tim16(void);

// TIM17 has its own IRQ
void platform_isr_tim17(void);

// IPCC has its own IRQ
void platform_isr_ipcc(void);

// HSEM has its own IRQ
void platform_isr_hsem(void);

// RNG has its own IRQ
void platform_isr_rng(void);

// AES & PKA have a shared IRQ
void platform_isr_aes(void);
void platform_isr_pka(void);

// I2C1 has its own IRQ
void platform_isr_i2c1(void);

// I2C2 has its own IRQ
void platform_isr_i2c2(void);

// I2C3 has its own IRQ
void platform_isr_i2c3(void);

// SPI1 has its own IRQ
void platform_isr_spi1(void);

// SPI2 has its own IRQ
void platform_isr_spi2(void);

// UART1 has its own IRQ
void platform_isr_usart1(void);

// UART2 has its own IRQ
void platform_isr_usart2(void);

// UART3 has its own IRQ
void platform_isr_lpuart1(void);

/// SPI to the Radio has its own IRQ
void platform_isr_rfspi(void);

/// Radio-Specific signals have a shared IRQ
void platform_isr_rfirq(void);
void platform_isr_rfbusy(void);


#else

#ifndef __CM4_NVIC_GROUPS
#   define __CM4_NVIC_GROUPS    2
#endif

#if (__CM4_NVIC_GROUPS == 1)
#   error "__CM4_NVIC_GROUPS is set to 1, but GULP Kernel not supported on this device yet."
//#   ifndef __KERNEL_GULP__
//#       define __KERNEL_GULP__
//#   endif
//#   define PLATFORM_NVIC_KERNEL_GROUP   0
//#   define PLATFORM_NVIC_RF_GROUP       0
//#   define PLATFORM_NVIC_IO_GROUP       0
//#   define PLATFORM_NVIC_TASK_GROUP     0

#elif (__CM4_NVIC_GROUPS == 2)
#   ifndef __KERNEL_HICCULP__
#       define __KERNEL_HICCULP__
#   endif
#   define PLATFORM_NVIC_KERNEL_GROUP   b0000
#   define PLATFORM_NVIC_RF_GROUP       b0100
#   define PLATFORM_NVIC_IO_GROUP       b1000
#   define PLATFORM_NVIC_INT_GROUP      b1000
#   define PLATFORM_NVIC_TASK_GROUP     b1000 

#elif (__CM4_NVIC_GROUPS == 4)
#   ifndef __KERNEL_HICCULP__
#       define __KERNEL_HICCULP__
#   endif
#   define PLATFORM_NVIC_KERNEL_GROUP   b0000
#   define PLATFORM_NVIC_RF_GROUP       b0100
#   define PLATFORM_NVIC_IO_GROUP       b0100
#   define PLATFORM_NVIC_INT_GROUP      b1000
#   define PLATFORM_NVIC_TASK_GROUP     b1100

#elif (__CM4_NVIC_GROUPS == 8)
#   ifndef __KERNEL_HICCULP__
#       define __KERNEL_HICCULP__
#   endif
#   define PLATFORM_NVIC_KERNEL_GROUP   b0000
#   define PLATFORM_NVIC_RF_GROUP       b0010
#   define PLATFORM_NVIC_IO_GROUP       b0100
#   define PLATFORM_NVIC_SLOWIO_GROUP   b0110
#   define PLATFORM_NVIC_INT_GROUP      b1100
#   define PLATFORM_NVIC_TASK_GROUP     b1110

#else
#   error "__CM4_NVIC_GROUPS must be set to 1, 2, 4, or 8"
#endif


///@todo these platform_isr...() functions are from STM32L4

/// These platform_isr_... functions are called from core_isr.c, from within
/// the primary ISRs.  They can be used in your application freely.  All entry 
/// and exit hooks that OpenTag may need to put around your ISR code are 
/// managed in core_isr.c and you, the app developer, do not need to worry 
/// about those unless you are hacking the kernel.
/// 
/// @note Some of the STM32WL interrupts are abstracted from Hardware into 
/// "synthetic" interrupts.  We do this to simplify app development, since some
/// of the hardware interrupts are grouped together inconveniently.

void platform_isr_wwdg(void);
void platform_isr_pvdpvm(void);

// Synthetic ISRs for RTC_tamp_stamp/css_lse interrupt
void platform_isr_rtctamper(void);
void platform_isr_csslse(void);

void platform_isr_rtcwakeup(void);

void platform_isr_flash(void);

void platform_isr_rcc(void);

void platform_isr_exti0(void);
void platform_isr_exti1(void);
void platform_isr_exti2(void);
void platform_isr_exti3(void);
void platform_isr_exti4(void);

void platform_isr_dma1ch1(void);
void platform_isr_dma1ch2(void);
void platform_isr_dma1ch3(void);
void platform_isr_dma1ch4(void);
void platform_isr_dma1ch5(void);
void platform_isr_dma1ch6(void);
void platform_isr_dma1ch7(void);

// Synthetic ISRs, via ADC1_2
void platform_isr_adc1(void);
void platform_isr_adc2(void);

// CAN interrupts not available on L41, L42 devices
void platform_isr_can1tx(void);
void platform_isr_can1rx0(void);
void platform_isr_can1rx1(void);
void platform_isr_can1sce(void);

// Synthetic ISRs, via EXTI9_5
void platform_isr_exti5(void);
void platform_isr_exti6(void);
void platform_isr_exti7(void);
void platform_isr_exti8(void);
void platform_isr_exti9(void);

// Synthetic ISRs, via TIM1_BRK/TIM15 Interrupt
void platform_isr_tim1brk(void);
void platform_isr_tim15(void);

// Synthetic ISRs, via TIM1_UP/TIM16 Interrupt
void platform_isr_tim1up(void);
void platform_isr_tim16(void);

// Synthetic ISRs, via TIM1_TRG_COM Interrupt
void platform_isr_tim1trg(void);
void platform_isr_tim1com(void);

void platform_isr_tim1cc(void);

void platform_isr_tim2(void);
void platform_isr_tim3(void);       // Only on 45xxx, 46xxx

void platform_isr_i2c1ev(void);
void platform_isr_i2c1er(void);
void platform_isr_i2c2ev(void);     // Not on 432xx, 442xx
void platform_isr_i2c2er(void);     // Not on 432xx, 442xx
void platform_isr_i2c3ev(void);
void platform_isr_i2c3er(void);
void platform_isr_i2c4ev(void);     // Only on 45xxx, 46xxx
void platform_isr_i2c4er(void);     // Only on 45xxx, 46xxx

void platform_isr_spi1(void);
void platform_isr_spi2(void);       // Not on 432xx, 442xx
void platform_isr_spi3(void);

void platform_isr_usart1(void);
void platform_isr_usart2(void);
void platform_isr_usart3(void);     // Not on 432xx, 442xx
void platform_isr_usart4(void);     // Only on 45xxx, 46xxx

// Synthetic ISRs, via EXTI15_10
void platform_isr_exti10(void);
void platform_isr_exti11(void);
void platform_isr_exti12(void);
void platform_isr_exti13(void);
void platform_isr_exti14(void);
void platform_isr_exti15(void);

void platform_isr_rtcalarm(void);

void platform_isr_sdmmc(void);      // Not on 41xxx, 42xxx, 432xx, 442xx, 

// Synthetic ISRs, via TIM6_DACUNDER
void platform_isr_tim6(void);
void platform_isr_dacunder(void);

void platform_isr_tim7(void);       // Only on 43xxx, 44xxx

void platform_isr_dma2ch1(void);
void platform_isr_dma2ch2(void);
void platform_isr_dma2ch3(void);
void platform_isr_dma2ch4(void);
void platform_isr_dma2ch5(void);
void platform_isr_dma2ch6(void);
void platform_isr_dma2ch7(void);

void platform_isr_comp(void);

void platform_isr_lptim1(void);
void platform_isr_lptim2(void);

void platform_isr_usbfs(void);      // Only on 4x2xx, 4x3xx

void platform_isr_lpuart1(void);

void platform_isr_quadspi(void);

void platform_isr_sai1(void);       // Not on 41xxx, 42xxx

void platform_isr_swpmi1(void);     // Only on 43xxx, 44xxx

void platform_isr_tsc(void);

void platform_isr_lcd(void);        // Only on 43xxx

void platform_isr_aes(void);        // Only on 42xx, 44xxx, 46xxx

void platform_isr_rng(void);






#endif

/// These Primary ISR functions are typically implemented in core_errors.c
/// or core_main.c rather than core_isr.c, because they are required by the
/// kernel and not exposed to users.
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);











/** STM32L Cryptography Include <BR>
  * ========================================================================<BR>
  * Alias OTEAX Data Type into generic type for OpenTag
  * 
  */
#if OT_FEATURE(DLL_SECURITY) || OT_FEATURE(NL_SECURITY) || OT_FEATURE(VL_SECURITY)
#   include <oteax.h>
//#   define EAXdrv_t eax_ctx
    typedef eax_ctx EAXdrv_t;
#else
//#   define EAXdrv_t ot_uint
    typedef ot_uint EAXdrv_t;
#endif





/** STM32L Special Platform Functions & data for timing & clocking <BR>
  * ========================================================================<BR>
  *
  */

// These are the default GPTIM settings for this MCU
#define __ISR_RTC_Alarm
#define OT_GPTIM_ID         'R'
#define OT_GPTIM            RTC
#define OT_GPTIM_CLOCK      32768
#define OT_GPTIM_SHIFT      0
#define OT_GPTIM_OVERSAMPLE 5
#define OT_GPTIM_RES        (1024 << OT_GPTIM_SHIFT)    //1024
#define OT_GPTIM_LIMIT      (120 * 1024)
#define TI_TO_CLK(VAL)      ((OT_GPTIM_RES/1024)*VAL)
#define CLK_TO_TI(VAL)      (VAL/(OT_GPTIM_RES/1024))

#if (OT_FEATURE(TIME) == ENABLED)
#   define RTC_ALARMS   1       // Max=3
#else
#   define RTC_ALARMS   0
#endif

#define GPTIM_FLAG_SLEEP        (1<<0)


typedef struct {
    ot_u16  flags;
    ot_u32  k_stamp;
    ot_u32  clk_stamp;
} systim_struct;

typedef struct {
    ot_u8   disabled;
    ot_u8   taskid;
    ot_u16  mask;
    ot_u16  value;
} rtcalarm;

//typedef struct {
//    ot_u32      utc;
//    rtcalarm    alarm[RTC_ALARMS];
//} rtc_struct;


//extern rtc_struct rtc;
extern systim_struct systim;





/** STM32L Platform Data  <BR>
  * ========================================================================<BR>
  * platform_ext stores data that is required for OpenTag to work properly on
  * the STM32L, and the data is not platform-independent.
  */

typedef struct {
    // Tasking parameters
    void* task_exit;
    
    // Clock speed saves: AHB1, APB1, APB2
    ot_ulong clock_hz[3];
    
    // LSI kHz divisor, only needed if LSI is actually used
#   if (BOARD_FEATURE_LFXTAL != ENABLED)
    ot_u16 lsi_khz;
    ot_u16 lsi_remhz;
#   endif
    
    // System stack alloctation: used for ISRs
    ot_u32 sstack[OT_PARAM_SSTACK_ALLOC/4];

    // task & thread stacks allocation
#   if (OT_PARAM_SYSTHREADS)
    ot_u32 tstack[OT_PARAM_TSTACK_ALLOC/4];
#   endif
} 
platform_ext_struct;


extern platform_ext_struct  platform_ext;




#endif
