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
  * @file       /include/platform/hw/STM32L1xx_config.h
  * @author     JP Norair
  * @version    R102
  * @date       14 Feb 2013
  * @brief      Platform Library Macros and Functions for STM32L
  * @ingroup    Platform
  *
  ******************************************************************************
  */
  

#ifndef __PLATFORM_STM32L1xx_CONFIG_H
#define __PLATFORM_STM32L1xx_CONFIG_H

#include <otstd.h>

//Stored in App Code folder
#include <app/board_config.h>
#include <app/isr_config_STM32L.h>




/** Long Chip-Select Section <BR>
  * ========================================================================<BR>
  * In your project gcc defines or in the board config file you should set the
  * part number for the MCU on the board.  The preprocessing below will set up
  * other attributes based on the part number.
  */

#define __STM32__
#define __STM32L__
#define __STM32L1__
#define __STM32L1xx__

// 48 Pin STM32L's have ports A, B, C   (although C is limited)
#if (   defined(__STM32L151C6__) \
    ||  defined(__STM32L151C8__) \
    ||  defined(__STM32L151CB__) \
    ||  defined(__STM32L151CC__) \
    ||  defined(__STM32L152C6__) \
    ||  defined(__STM32L152C8__) \
    ||  defined(__STM32L152CB__) \
    ||  defined(__STM32L151CC__)   )
#   define MCU_PARAM_PORTS  3

// 63 & 64 Pin STM32L's have ports A, B, C, D    (although D is limited)
#elif ( defined(__STM32L151R6__) \
    ||  defined(__STM32L151R8__) \
    ||  defined(__STM32L151RB__) \
    ||  defined(__STM32L151RC__) \
    ||  defined(__STM32L151RD__) \
    ||  defined(__STM32L151UC__) \
    ||  defined(__STM32L152R6__) \
    ||  defined(__STM32L152R8__) \
    ||  defined(__STM32L152RB__) \
    ||  defined(__STM32L151RC__) \
    ||  defined(__STM32L151RD__) \
    ||  defined(__STM32L162RC__) \
    ||  defined(__STM32L162RD__)    )
#   define MCU_PARAM_PORTS  4

// 63 & 64 Pin STM32L's have ports A, B, C, D    (although D is limited)
#elif ( defined(__STM32L151R6__) \
    ||  defined(__STM32L151R8__) \
    ||  defined(__STM32L151RB__) \
    ||  defined(__STM32L151RC__) \
    ||  defined(__STM32L151RD__) \
    ||  defined(__STM32L151UC__) \
    ||  defined(__STM32L152R6__) \
    ||  defined(__STM32L152R8__) \
    ||  defined(__STM32L152RB__) \
    ||  defined(__STM32L151RC__) \
    ||  defined(__STM32L151RD__) \
    ||  defined(__STM32L162RC__) \
    ||  defined(__STM32L162RD__)    )
#   define MCU_PARAM_PORTS  4

// 100 Pin STM32L's have ports A, B, C, D, E
#elif ( defined(__STM32L151V8__) \
    ||  defined(__STM32L151VB__) \
    ||  defined(__STM32L151VD__) \
    ||  defined(__STM32L152V8__) \
    ||  defined(__STM32L152VB__) \
    ||  defined(__STM32L152VC__) \
    ||  defined(__STM32L152VD__) \
    ||  defined(__STM32L162VC__) \
    ||  defined(__STM32L162VD__) )
#   define MCU_PARAM_PORTS  5

// 132 & 144 Pin STM32L's have ports A, B, C, D, E, F, G
#elif ( defined(__STM32L151QC__) \
    ||  defined(__STM32L151QD__) \
    ||  defined(__STM32L151ZC__) \
    ||  defined(__STM32L151ZD__) \
    ||  defined(__STM32L152QC__) \
    ||  defined(__STM32L152QD__) \
    ||  defined(__STM32L152ZC__) \
    ||  defined(__STM32L152ZD__) \
    ||  defined(__STM32L162QD__) \
    ||  defined(__STM32L162ZD__) )
#   define MCU_PARAM_PORTS  7

#else
#   warning "Unknown STM32L defined.  Defaulting to 3 Ports (A, B, C)"
#   define MCU_PARAM_PORTS  3
#endif



// Devices with 32KB
#if (   defined(__STM32L151C6__) \
    ||  defined(__STM32L151R6__) \
    ||  defined(__STM32L152C6__) \
    ||  defined(__STM32L152R6__) )
#   define SRAM_SIZE    (10*1024)    
#   define EEPROM_SIZE  (4*1024)
#   define FLASH_SIZE   (32*1024)
#   ifndef STM32L1XX_MD
#       define STM32L1XX_MD
#   endif

#elif ( defined(__STM32L151C8__) \
    ||  defined(__STM32L152C8__) \
    ||  defined(__STM32L151R8__) \
    ||  defined(__STM32L152R8__) \
    ||  defined(__STM32L151V8__) \
    ||  defined(__STM32L152V8__) )
#   define SRAM_SIZE    (10*1024)    
#   define EEPROM_SIZE  (4*1024)
#   define FLASH_SIZE   (64*1024)
#   ifndef STM32L1XX_MD
#       define STM32L1XX_MD
#   endif

#elif ( defined(__STM32L151CB__) \
    ||  defined(__STM32L152CB__) \
    ||  defined(__STM32L151RB__) \
    ||  defined(__STM32L152RB__) \
    ||  defined(__STM32L151VB__) \
    ||  defined(__STM32L152VB__) )
#   define SRAM_SIZE    (16*1024)    
#   define EEPROM_SIZE  (4*1024)
#   define FLASH_SIZE   (128*1024)
#   ifndef STM32L1XX_MD
#       define STM32L1XX_MD
#   endif
    
#elif ( defined(__STM32L151CC__) \
    ||  defined(__STM32L151RC__) \
    ||  defined(__STM32L151UC__) \
    ||  defined(__STM32L162RC__) \
    ||  defined(__STM32L152VC__) \
    ||  defined(__STM32L162VC__) \
    ||  defined(__STM32L151QC__) \
    ||  defined(__STM32L151ZC__) \
    ||  defined(__STM32L152QC__) \
    ||  defined(__STM32L152ZC__) )
#   define SRAM_SIZE    (32*1024)    
#   define EEPROM_SIZE  (8*1024)
#   define FLASH_SIZE   (256*1024)
#   ifndef STM32L1XX_MDP
#       define STM32L1XX_MDP
#   endif

#elif ( defined(__STM32L151RD__) \
    ||  defined(__STM32L162RD__) \
    ||  defined(__STM32L151VD__) \
    ||  defined(__STM32L152VD__) \
    ||  defined(__STM32L162VD__) \
    ||  defined(__STM32L151QD__) \
    ||  defined(__STM32L151ZD__) \
    ||  defined(__STM32L152QD__) \
    ||  defined(__STM32L152ZD__) \
    ||  defined(__STM32L162QD__) \
    ||  defined(__STM32L162ZD__) )
#   define SRAM_SIZE    (48*1024)    
#   define EEPROM_SIZE  (12*1024)
#   define FLASH_SIZE   (384*1024)
#   ifndef STM32L1XX_MDP
#       define STM32L1XX_MDP
#   endif

#elif ( defined(__STM32L151RE__) \
    ||  defined(__STM32L162RE__) \
    ||  defined(__STM32L151VE__) \
    ||  defined(__STM32L152VE__) \
    ||  defined(__STM32L162VE__) \
    ||  defined(__STM32L151QE__) \
    ||  defined(__STM32L151ZE__) \
    ||  defined(__STM32L152QE__) \
    ||  defined(__STM32L152ZE__) \
    ||  defined(__STM32L162QE__) \
    ||  defined(__STM32L162ZE__) )
#   define SRAM_SIZE    (80*1024)    
#   define EEPROM_SIZE  (16*1024)
#   define FLASH_SIZE   (512*1024)
#   ifndef STM32L1XX_HD
#       define STM32L1XX_HD
#   endif

#else
#   warning "Unknown STM32L defined.  Defaulting to 151CB"
#   define SRAM_SIZE    (16*1024)    
#   define EEPROM_SIZE  (4*1024)
#   define FLASH_SIZE   (128*1024)
#   ifndef STM32L1XX_MD
#       define STM32L1XX_MD
#   endif
#endif






/** Back to regularly-sheduled program <BR>
  * ========================================================================<BR>
  */

// From lib/CMSIS/Device
#include <stm32l1xx.h>

// From lib/CMSIS/Include
#include <cm3_endian.h>
#include <cm3_byteswap.h>
#include <cm3_bitrotate.h>
#include <cm3_countleadingzeros.h>
#include <cm3_saturation.h>



/** Cryptographic Library Setup      <BR>
  * ========================================================================<BR>
  * Needed to build AES128 or other types of Crypto
  */
#ifndef CORTEX_M3
#   define CORTEX_M3
#endif
#ifndef STM32L1XX
#   define STM32L1XX
#endif

  
  

/** Special Platform functions for STM32L      <BR>
  * ========================================================================<BR>
  * These must be defined before including platform_config.h, so that they
  * can be used for inline functions.
  */
void platform_ext_pllon();
void platform_ext_plloff();
void platform_ext_wakefromstop();
void platform_ext_hsitrim();
ot_u16 platform_ext_lsihz();





#undef OT_GPTIM_LIMIT
#ifdef __DEBUG__
#   define OT_GPTIM_LIMIT   15000
#else
#   define OT_GPTIM_LIMIT   60000
#endif



/** Platform Support settings      <BR>
  * ========================================================================<BR>
  * STM32 is little endian with 4 byte pointer (32 bits), and at this stage it
  * can be compiled using GCC (RIDE, most other IDE's) or IAR's proprietary
  * compiler.
  */

#define PLATFORM(VAL)           PLATFORM_##VAL
#define PLATFORM_STM32L1xx

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
  */
#define MCU_FEATURE(VAL)                MCU_FEATURE_##VAL   // FEATURE                  NOTE
#define MCU_FEATURE_SVMONITOR           DISABLED            // Auto Low V powerdown     On many MCUs
#define MCU_FEATURE_CRC16               DISABLED            // CRC16                    On some MCUs
#define MCU_FEATURE_CRC                 MCU_FEATURE_CRC16   // Legacy definition
#define MCU_FEATURE_AES128              DISABLED            // AES128 engine            On some MCUs
#define MCU_FEATURE_ECC                 DISABLED            // ECC engine               Rare

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
#define FLASH_PAGE_SIZE         256
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


#ifndef __CM3_NVIC_GROUPS
#   define __CM3_NVIC_GROUPS    2
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
#   define PLATFORM_NVIC_KERNEL_GROUP   b0000
#   define PLATFORM_NVIC_RF_GROUP       b0100
#   define PLATFORM_NVIC_IO_GROUP       b1000
#   define PLATFORM_NVIC_INT_GROUP      b1000
#   define PLATFORM_NVIC_TASK_GROUP     b1000 

#elif (__CM3_NVIC_GROUPS == 4)
#   ifndef __KERNEL_HICCULP__
#       define __KERNEL_HICCULP__
#   endif
#   define PLATFORM_NVIC_KERNEL_GROUP   b0000
#   define PLATFORM_NVIC_RF_GROUP       b0100
#   define PLATFORM_NVIC_IO_GROUP       b0100
#   define PLATFORM_NVIC_INT_GROUP      b1000
#   define PLATFORM_NVIC_TASK_GROUP     b1100

#elif (__CM3_NVIC_GROUPS == 8)
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
    ot_u16  flags;
#   if defined(__DEBUG__)
    ot_u16  stamp1;
    ot_u16  stamp2;
#   else
    ot_u16  chron_stamp;
    ot_u32  evt_stamp;
    ot_u32  evt_span;
#   endif
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
