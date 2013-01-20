/* Copyright 2009-2013 JP Norair
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
  * @file       /otplatform/stm32l1xx/platform_STM32L1xx.h
  * @author     JP Norair
  * @version    R101
  * @date       17 Jan 2013
  * @brief      Platform Library Macros and Functions for STM32L
  * @ingroup    Platform
  *
  ******************************************************************************
  */
  

#ifndef __PLATFORM_STM32L1xx_H
#define __PLATFORM_STM32L1xx_H

#include "build_config.h"
#include "OT_support.h"
#include "ST/STM32L1xx/stm32l1xx.h"     //from CMSIS/CM3/DeviceSupport/

//#include "stm32l1xx_conf.h"




/** Platform Support settings      <BR>
  * ========================================================================<BR>
  * STM32 is little endian with 4 byte pointer (32 bits), and at this stage it
  * can be compiled using GCC (RIDE, most other IDE's) or IAR's proprietary
  * compiler.
  */

#define PLATFORM(VAL)           PLATFORM_##VAL
#define __STM32L__
#define __STM32L1xx__
#define PLATFORM_STM32L1xx

#ifndef __LITTLE_ENDIAN__
#   error "Endian-ness misdefined, should be __LITTLE_ENDIAN__ (check build_config.h)"
#endif
#define PLATFORM_POINTER_SIZE       4               // How many bytes is a pointer?
#define PLATFORM_ENDIAN16(VAR16)    __REV16(VAR16)  // Big-endian to Platform-endian
#define PLATFORM_ENDIAN32(VAR32)    __REV(VAR32)    // Big-endian to Platform-endian





/** Chip Settings  <BR>
  * ========================================================================<BR>
  * @todo Check if this is even needed.  ARMGCC is dominant compiler
  */



/** Low Power Mode Macros:
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
#define SLEEP_WHILE_UHF     MCU_SLEEP
#define SLEEP_MCU           MCU_SLEEP
#define STOP_MCU            MCU_STOP
#define STANDBY_MCU         MCU_STANDBY




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
  * /otplatform/stm32_mculib/CMSIS/CM3/DeviceSupport/ST/STM32L1xx/startup/...
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
#   define OT_PARAM_SSTACK_ALLOC    (160)
#endif
#if !defined(OT_PARAM_TSTACK_ALLOC)
#   define OT_PARAM_TSTACK_ALLOC    (OT_PARAM_SYSTHREADS*160)
#endif
#if ((OT_PARAM_TSTACK_ALLOC/OT_PARAM_SYSTHREADS) < 80)
#   error "Specified Thread Stack Size allocates less than 80 bytes per thread stack."
#endif


#ifndef __CM3_NVIC_GROUPS
//#   if (OT_PARAM_SYSTHREADS == 0)
#       define __CM3_NVIC_GROUPS    2
//#   else
//#       define __CM3_NVIC_GROUPS    4
//#   endif
#endif

#if (__CM3_NVIC_GROUPS == 1)
#   error "__CM3_NVIC_GROUPS is set to 1, but GULP Kernel not supported on this device yet."
//#   ifndef __KERNEL_GULP__
//#       define __KERNEL_GULP__
//#   endif
//#   define PLATFORM_NVIC_KERNEL_GROUP   0
//#   define PLATFORM_NVIC_RF_GROUP       0
//#   define PLATFORM_NVIC_IO_GROUP       0
//#   define PLATFORM_NVIC_TASK_GROUP     0

#elif (__CM3_NVIC_GROUPS == 2)
#   ifndef __KERNEL_HICCULP__
#       define __KERNEL_HICCULP__
#   endif
#   define PLATFORM_NVIC_KERNEL_GROUP   0
#   define PLATFORM_NVIC_RF_GROUP       0
#   define PLATFORM_NVIC_IO_GROUP       0
#   define PLATFORM_NVIC_INT_GROUP      1
#   define PLATFORM_NVIC_TASK_GROUP     1

#elif (__CM3_NVIC_GROUPS == 4)
#   ifndef __KERNEL_HICCULP__
#       define __KERNEL_HICCULP__
#   endif
#   define PLATFORM_NVIC_KERNEL_GROUP   0
#   define PLATFORM_NVIC_RF_GROUP       1
#   define PLATFORM_NVIC_IO_GROUP       1
#   define PLATFORM_NVIC_INT_GROUP      2
#   define PLATFORM_NVIC_TASK_GROUP     3

#elif (__CM3_NVIC_GROUPS == 8)
#   ifndef __KERNEL_HICCULP__
#       define __KERNEL_HICCULP__
#   endif
#   define PLATFORM_NVIC_KERNEL_GROUP   0
#   define PLATFORM_NVIC_RF_GROUP       1
#   define PLATFORM_NVIC_IO_GROUP       2
#   define PLATFORM_NVIC_SLOWIO_GROUP   3
#   define PLATFORM_NVIC_INT_GROUP      6
#   define PLATFORM_NVIC_TASK_GROUP     7

#else
#   error "__CM3_NVIC_GROUPS must be set to 1, 2, 4, or 8"
#endif


/// These Primary ISR functions should be implemented in platform_STM32l1xx.c because
/// they are mandatory (even if unused, they can get triggered by broken code).
void HardFault_Handler(void);
void NMI_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);




/// These platform_isr_... functions are called from platform_isr_STM32L1xx.c,
/// from within the primary ISRs.  They can be used in your application freely.
/// All entry and exit hooks (typically none of either) that OpenTag may need
/// to put around ISR code are managed in platform_isr_STM32L1xx.c and you, the
/// app developer, don't need to worry about those.

///@note The EXTI ISRs are slightly abstracted.  Each EXTI line has its own SW
///      ISR in this API, and the EXTI->PR bit is cleared for you upon entry.

void platform_isr_wwdg(void);
void platform_isr_pvd(void);
void platform_isr_tamper(void);
void platform_isr_rtcwkup(void);
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
void platform_isr_adc1(void);
void platform_isr_usbhp(void);
void platform_isr_usblp(void);
void platform_isr_dac(void);
void platform_isr_comp(void);
void platform_isr_exti5(void);
void platform_isr_exti6(void);
void platform_isr_exti7(void);
void platform_isr_exti8(void);
void platform_isr_exti9(void);
void platform_isr_lcd(void);
void platform_isr_tim9(void);
void platform_isr_tim10(void);
void platform_isr_tim11(void);
void platform_isr_tim2(void);
void platform_isr_tim3(void);
void platform_isr_tim4(void);
void platform_isr_i2c1ev(void);
void platform_isr_i2c1er(void);
void platform_isr_i2c2ev(void);
void platform_isr_i2c2er(void);
void platform_isr_spi1(void);
void platform_isr_spi2(void);
void platform_isr_usart1(void);
void platform_isr_usart2(void);
void platform_isr_usart3(void);
void platform_isr_exti10(void);
void platform_isr_exti11(void);
void platform_isr_exti12(void);
void platform_isr_exti13(void);
void platform_isr_exti14(void);
void platform_isr_exti15(void);
void platform_isr_rtcalarm(void);
void platform_isr_fswkup(void);
void platform_isr_tim6(void);
void platform_isr_tim7(void);


void platform_isr_tim5(void);
void platform_isr_spi3(void);
void platform_isr_dma2ch1(void);
void platform_isr_dma2ch2(void);
void platform_isr_dma2ch3(void);
void platform_isr_dma2ch4(void);
void platform_isr_dma2ch5(void);
void platform_isr_aes(void);
void platform_isr_compacq(void);


void platform_isr_sdio(void);
void platform_isr_uart4(void);
void platform_isr_uart5(void);







/** STM32L Special Platform Functions  <BR>
  * ========================================================================<BR>
  */





/** STM32L Platform Data  <BR>
  * ========================================================================<BR>
  * platform_ext stores data that is required for OpenTag to work properly on
  * the STM32L, and the data is not platform-independent.
  */
#if (OT_FEATURE_RTC == ENABLED)
#define RTC_ALARMS          1        // Max=3
#else
#define RTC_ALARMS          0
#endif

#define RTC_OVERSAMPLE      0

typedef struct {
    ot_u8   disabled;
    ot_u8   taskid;
    ot_u16  mask;
    ot_u16  value;
} rtcalarm;



typedef struct {
    // System stack alloctation: used for ISRs
    ot_u32 sstack[((OT_PARAM_SSTACK_ALLOC+3)/4)];

    // task & thread stacks allocation
#   if (OT_PARAM_SYSTHREADS != 0)
    ot_u32 tstack[((OT_PARAM_TSTACK_ALLOC+3)/4)];
#   endif

    // Real Time Clock features
#   if (RTC_ALARMS > 0)
        rtcalarm    alarm[RTC_ALARMS];
#   endif
#   if (RTC_OVERSAMPLE)
        ot_u32      utc;
#   endif

    // Not sure if this needs to stay
//#   ifdef EXTF_platform_ext_wakeup
//        ot_u16      wakeup_params;
//#   endif

    // Miscellaneous platform parameters and software registers
    ot_u16  next_task;
    ot_u16  cpu_khz;
    ot_u16  prand_reg;
    ot_u16  crc16;
    
} platform_ext_struct;


extern platform_ext_struct  platform_ext;




#endif
