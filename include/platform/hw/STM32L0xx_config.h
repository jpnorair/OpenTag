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
  * @file       /include/platform/hw/STM32L0xx_config.h
  * @author     JP Norair
  * @version    R100
  * @date       14 Sept 2014
  * @brief      Platform Library Macros and Functions for STM32L0
  * @ingroup    Platform
  *
  ******************************************************************************
  */
  

#ifndef __PLATFORM_STM32L0xx_CONFIG_H
#define __PLATFORM_STM32L0xx_CONFIG_H

#include <otstd.h>

//Stored in App Code folder
#include <app/board_config.h>
#include <app/isr_config_STM32L0.h>




/** Long Chip-Select Section <BR>
  * ========================================================================<BR>
  * In your project gcc defines or in the board config file you should set the
  * part number for the MCU on the board.  The preprocessing below will set up
  * other attributes based on the part number.
  */

#define __STM32__
#define __STM32L__
#define __STM32L0__
#define __STM32L0xx__
#define STM32
#define STM32L
#define STM32L0
#define STM32L0x


// 32 Pin STM32L0's are in LQFP32 or UFQFPN32 packages
// Port A: { 0:15 } 
// Port B: { 0:7 } 
// Port C: { 14, 15 }
#if defined(__STM32L051K6__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (32*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L051xx

#elif defined(__STM32L051K8__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (64*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L051xx

#elif defined(__STM32L052K6__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (32*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L052xx

#elif defined(__STM32L052K8__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (64*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L052xx

#elif defined(__STM32L062K8__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (64*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L062xx

#elif defined(__STM32L072KB__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (128*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L072xx

#elif defined(__STM32L072KZ__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (192*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L072xx

#elif defined(__STM32L082KB__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (128*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L082xx

#elif defined(__STM32L082KZ__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (192*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L082xx



// 36 Pin STM32L0's are in the WLCSP36 package
// Port A: { 0:15 }
// Port B: { 0:8, 10:11 }
// Port C: { 14:15 }
#elif defined(__STM32L051T6__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (32*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L051xx

#elif defined(__STM32L051T8__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (64*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L051xx

#elif defined(__STM32L052T6__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (32*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L052xx

#elif defined(__STM32L052T8__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (64*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L052xx



// 48 Pin STM32L0's come in LQFP48 packages
// Port A: { 0:15 }
// Port B: { 0:15 }
// Port C: { 13:15 }
#elif defined(__STM32L051C6__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (32*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L051xx

#elif defined(__STM32L051C8__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (64*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L051xx

#elif defined(__STM32L052C6__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (32*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L052xx

#elif defined(__STM32L052C8__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (64*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L052xx

#elif defined(__STM32L053C6__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (32*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L053xx

#elif defined(__STM32L053C8__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (64*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L053xx

#elif defined(__STM32L063C8__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (64*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L063xx

#elif defined(__STM32L073CB__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (128*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L073xx

#elif defined(__STM32L073CZ__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (192*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L073xx

#elif defined(__STM32L072CB__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (128*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L072xx

#elif defined(__STM32L072CZ__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (192*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L072xx

#elif defined(__STM32L082CB__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (128*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L082xx

#elif defined(__STM32L082CZ__)
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (192*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L082xx


// 64 Pin STM32L0's come in LQFP64 or TFBGA64 packages
// Port A: { 0:15 }
// Port B: { 0:15 }
// Port C: { 0:15 }
// Port D: { 2 }
#elif defined(__STM32L051R6__)
#   define MCU_PARAM_PORTS  4
#   define FLASH_SIZE       (32*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L051xx

#elif defined(__STM32L051R8__)
#   define MCU_PARAM_PORTS  4
#   define FLASH_SIZE       (64*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L051xx

#elif defined(__STM32L052R6__)
#   define MCU_PARAM_PORTS  4
#   define FLASH_SIZE       (32*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L052xx

#elif defined(__STM32L052R8__)
#   define MCU_PARAM_PORTS  4
#   define FLASH_SIZE       (64*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L052xx

#elif defined(__STM32L053R6__)
#   define MCU_PARAM_PORTS  4
#   define FLASH_SIZE       (32*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L053xx

#elif defined(__STM32L053R8__)
#   define MCU_PARAM_PORTS  4
#   define FLASH_SIZE       (64*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L053xx

#elif defined(__STM32L063R8__)
#   define MCU_PARAM_PORTS  4
#   define FLASH_SIZE       (64*1024)
#   define SRAM_SIZE        (8*1024)    
#   define EEPROM_SIZE      (2*1024)
#   define STM32L063xx

#elif defined(__STM32L073RB__)
#   define MCU_PARAM_PORTS  4
#   define FLASH_SIZE       (128*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L073xx

#elif defined(__STM32L073RZ__)
#   define MCU_PARAM_PORTS  4
#   define FLASH_SIZE       (192*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L073xx

#elif defined(__STM32L072RB__)
#   define MCU_PARAM_PORTS  4
#   define FLASH_SIZE       (128*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L072xx

#elif defined(__STM32L072RZ__)
#   define MCU_PARAM_PORTS  4
#   define FLASH_SIZE       (192*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L072xx

#elif defined(__STM32L082RB__)
#   define MCU_PARAM_PORTS  4
#   define FLASH_SIZE       (128*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L072xx

#elif defined(__STM32L082RZ__)
#   define MCU_PARAM_PORTS  4
#   define FLASH_SIZE       (192*1024)
#   define SRAM_SIZE        (20*1024)    
#   define EEPROM_SIZE      (6*1024)
#   define STM32L072xx


// Unknown Part, so use the lowest-common-denominator
#else
#   warning "Unknown STM32L0 defined.  Defaulting to STM32L051K6"
#   define MCU_PARAM_PORTS  3
#   define FLASH_SIZE       (32*1024)
#   define STM32L051xx

#endif






/** Back to regularly-sheduled program <BR>
  * ========================================================================<BR>
  */

// From lib/CMSIS/Device
#include <stm32l0xx.h>

// From lib/CMSIS/Include
#include <cm0plus_endian.h>
#include <cm0plus_byteswap.h>
#include <cm0plus_bitrotate.h>
#include <cm0plus_saturation.h>

// Definitions removed from recent STM32L libs
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





/** GPTIM Manipulations      <BR>
  * ========================================================================<BR>
  * Tasking waits longer than this (in ticks) will loop through the scheduler.
  * It is pretty rare with OpenTag for waits to be longer than 2 seconds
  * (2048 ticks), so this boundary rarely occurs.
  *
  * The LPTIM used for STM32L0 is oversampled in order to provide better
  * timing accuracy (it is laggy otherwise).  It is double sampled, so the
  * most it can go is 32768 ticks.  We set the limit to 30000.
  */
#ifndef OT_GPTIM_OVERSAMPLE
#   define OT_GPTIM_OVERSAMPLE  1
#endif
#ifndef OT_GPTIM_SHIFT
#   define OT_GPTIM_SHIFT       0
#endif
#define MCU_PRESCALER_SHIFT     (OT_GPTIM_OVERSAMPLE+OT_GPTIM_SHIFT)
#ifndef OT_GPTIM_LIMIT
#   define OT_GPTIM_LIMIT       (60000 >> MCU_PRESCALER_SHIFT)
#endif





/** Cryptographic Library Setup      <BR>
  * ========================================================================<BR>
  * Needed to build AES128 or other types of Crypto
  */
#ifndef CORTEX_M0
#   define CORTEX_M0
#endif
#ifndef STM32L0XX
#   define STM32L0XX
#endif

  
  

/** Special Platform functions for STM32L      <BR>
  * ========================================================================<BR>
  * These must be defined before including platform_config.h, so that they
  * can be used for inline functions.
  */
void platform_ext_usbcrson(void);
void platform_ext_usbcrsoff(void);
void platform_ext_pllon();
void platform_ext_plloff();
void platform_ext_wakefromstop();
void platform_ext_hsitrim();
ot_u16 platform_ext_lsihz();







/** Platform Support settings      <BR>
  * ========================================================================<BR>
  * STM32 is little endian with 4 byte pointer (32 bits), and at this stage it
  * can be compiled using GCC (RIDE, most other IDE's) or IAR's proprietary
  * compiler.
  */

#define PLATFORM(VAL)           PLATFORM_##VAL
#define PLATFORM_STM32L0xx

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
#define MCU_FEATURE_CRC16       DISABLED            // CRC16                    Exists on STM32L0, but HW needs to be tested
#define MCU_FEATURE_CRC         MCU_FEATURE_CRC16   // Legacy definition
#define MCU_FEATURE_ECC         DISABLED            // ECC engine 

// True Random Number HW
#define MCU_FEATURE_TRNG        (defined(STM32L052xx) || defined(STM32L053xx) \
                                || defined(STM32L062xx) || defined(STM32L063xx) \
                                || defined(STM32L072xx) || defined(STM32L073xx) \
                                || defined(STM32L082xx) || defined(STM32L083xx))
// AES HW
#define MCU_FEATURE_AES128      (defined(STM32L062xx) || defined(STM32L063xx) \
                                || defined(STM32L082xx) || defined(STM32L083xx))



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





#define SRAM_START_ADDR         0x20000000
#define EEPROM_START_ADDR       0x08080000
#define FLASH_START_ADDR        0x08000000
#define FLASH_START_PAGE        0
#define FLASH_PAGE_SIZE         128
#define FLASH_WORD_BYTES        2
#define FLASH_WORD_BITS         (FLASH_WORD_BYTES*8)
#define FLASH_PAGE_ADDR(VAL)    (FLASH_START_ADDR + ( (VAL) * FLASH_PAGE_SIZE) )

// STM32L has peculiar flash, where 0 is the erase value.  
// On every other flash, it is 1 (i.e. FFFF).
//#define NULL_vaddr              0x0000




/** Chip Settings  <BR>
  * ========================================================================<BR>
  * @todo Check if this is even needed.  ARMGCC is dominant compiler
  */



/** Low Power Mode Macros: (Deprecated)
  * ========================================================================<BR>
  * SLEEP_MCU():        Core off, APB on, SRAM on                       (~50 uA)
  * SLEEP_WHILE_UHF():  Core off, APB on, SRAM on                       (~10 uA)
  * STOP_MCU():         Core off, RTC on, SRAM on                       (~1.5 uA)
  * STANDBY_MCU():      Core off, clocks off, SRAM off                  (~0.2 uA)
  */
#define MCU_SLEEP               __WFI
#define MCU_SLEEP_WHILE_IO      __WFI
#define MCU_SLEEP_WHILE_RF      __WFI
#define MCU_STOP()              PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI)
#define MCU_STANDBY()           PWR_EnterSTANDBYMode(void)

//Legacy
//#define SLEEP_WHILE_UHF     MCU_SLEEP
//#define SLEEP_MCU           MCU_SLEEP
//#define STOP_MCU            MCU_STOP
//#define STANDBY_MCU         MCU_STANDBY




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






/// These platform_isr_... functions are called from core_isr.c, from within
/// the primary ISRs.  They can be used in your application freely.  All entry 
/// and exit hooks that OpenTag may need to put around your ISR code are 
/// managed in core_isr.c and you, the app developer, do not need to worry 
/// about those unless you are hacking the kernel.
/// 
/// @note Some of the STM32L0 interrupts are abstracted from Hardware into 
/// "synthetic" interrupts.  We do this to simplify app development, since some
/// of the hardware interrupts are grouped together inconveniently.

void platform_isr_wwdg(void);
void platform_isr_pvd(void);
void platform_isr_flash(void);
void platform_isr_lptim1(void);
void platform_isr_tim2(void);
void platform_isr_tim21(void);
void platform_isr_tim22(void);
void platform_isr_i2c1(void);
void platform_isr_i2c2(void);
void platform_isr_spi1(void);
void platform_isr_spi2(void);
void platform_isr_usart1(void);
void platform_isr_usart2(void);
void platform_isr_usb(void);

// Synthetic ISRs, via Global RTC interrupt
void platform_isr_rtcwakeup(void);
void platform_isr_rtcalarm(void);
void platform_isr_tamper(void);
void platform_isr_csslse(void);

// Synthetic ISRs, via RCC_CRS
void platform_isr_rcc(void);
void platform_isr_crs(void);

// Synthetic ISRs, via EXTI[1:0]
void platform_isr_exti0(void);
void platform_isr_exti1(void);

// Synthetic ISRs, via EXTI[3:2]
void platform_isr_exti2(void);
void platform_isr_exti3(void);

// Synthetic ISRs, via EXTI[15:4]
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

// Maps directly via DMA1_Channel[1]
void platform_isr_dma1ch1(void);

// Synthetic ISRs, via DMA1_Channel[3:2]
void platform_isr_dma1ch2(void);
void platform_isr_dma1ch3(void);

// Synthetic ISRs, via DMA1_Channel[7:4]
void platform_isr_dma1ch4(void);
void platform_isr_dma1ch5(void);
void platform_isr_dma1ch6(void);
void platform_isr_dma1ch7(void);

// Synthetic ISRs, via ADC_COMP
void platform_isr_adc1(void);
void platform_isr_comp1(void);
void platform_isr_comp2(void);

// Synthetic ISRs, via TIM6_DAC
void platform_isr_tim6(void);
void platform_isr_dac(void);

// Synthetic ISRs, via LPUART1 + AES + RNG
void platform_isr_lpuart(void);
void platform_isr_aes(void);
void platform_isr_rng(void);





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
  */
#if (OT_FEATURE(TIME) == ENABLED)
#   define RTC_ALARMS   1       // Max=3
#else
#   define RTC_ALARMS   0
#endif

#define GPTIM_FLAG_RTCBYPASS    (1<<1)
#define GPTIM_FLAG_SLEEP        (1<<0)

typedef struct {
    ot_u8   flags;
    ot_u8   opt;
    ot_u16  stamp1;
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
    void*   task_exit;
    
    // Clock speed saves
    ot_ulong clock_hz[3];
    
    // LSI kHz divisor, only needed if LSI is actually used
//#   if (BOARD_FEATURE(LFXTAL) != ENABLED) //For unknown reasons this causes problems
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
