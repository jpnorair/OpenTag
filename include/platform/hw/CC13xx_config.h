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
  * @file       /include/platform/hw/CC13xx_config.h
  * @author     JP Norair
  * @version    R102
  * @date       14 Oct 2016
  * @brief      Platform Library Macros and Functions for CC13xx
  * @ingroup    Platform
  *
  ******************************************************************************
  */
  

#ifndef __PLATFORM_CC13xx_CONFIG_H
#define __PLATFORM_CC13xx_CONFIG_H

#include <otstd.h>

//Stored in App Code folder
#include <app/board_config.h>
#include <app/isr_config_CC13xx.h>




/** Long Chip-Select Section <BR>
  * ========================================================================<BR>
  * In your project gcc defines or in the board config file you should set the
  * part number for the MCU on the board.  The preprocessing below will set up
  * other attributes based on the part number.
  */

#define __CC13xx__

#ifndef CORTEX_M3
#   define CORTEX_M3
#endif
#ifndef CC13XX
#   define CC13XX
#endif
#ifndef CC13XX_CC26XX
#   define CC13XX_CC26XX
#endif

// 4x4mm 32-Pin CC13xxFxxRSM devices have 10 GPIOs
#if (   defined(__CC1310F32RSM__) \
    ||  defined(__CC1310F64RSM__) \
    ||  defined(__CC1310F128RSM__) )
#   define MCU_PARAM_GPIO   10

// 5x5mm 32-Pin CC13xxFxxRHB devices have 15 GPIOs
#if (   defined(__CC1310F32RHB__) \
    ||  defined(__CC1310F64RHB__) \
    ||  defined(__CC1310F128RHB__) )
#   define MCU_PARAM_GPIO   15

// 7x7mm 48-Pin CC13xxFxxRGZ devices have 30 GPIOs
#if (   defined(__CC1310F32RGZ__) \
    ||  defined(__CC1310F64RGZ__) \
    ||  defined(__CC1310F128RGZ__) \
    ||  defined(__CC1350F128RGZ__) )
#   define MCU_PARAM_GPIO   30

#else
#   warning "Unknown CC13xx defined.  Defaulting to 10 GPIOs."
#   define MCU_PARAM_GPIO   10
#endif

// CC13xx supports a single port up to 31 pins
#if (MCU_PARAM_GPIO <= 31)
#   define MCU_PARAM_PORTS  1
#else
#   warning "More GPIOs declared than known supported.  Defaulting to 1 Port."
#   define MCU_PARAM_PORTS  1
#endif




// Devices with 32KB FLASH / 16KB SRAM (not including cache)
#if (   defined(__CC1310F32RSM__) \
    ||  defined(__CC1310F32RHB__) \
    ||  defined(__CC1310F32RGZ__) )
#   define SRAM_SIZE    (16*1024)    
#   define EEPROM_SIZE  (0*1024)
#   define FLASH_SIZE   (32*1024)
#   ifndef CC13XXF32
#       define CC13XXF32
#   endif

// Devices with 64KB FLASH / 16KB SRAM (not including cache)
#elif (   defined(__CC1310F64RSM__) \
    ||  defined(__CC1310F64RHB__) \
    ||  defined(__CC1310F64RGZ__) )
#   define SRAM_SIZE    (16*1024)    
#   define EEPROM_SIZE  (0*1024)
#   define FLASH_SIZE   (64*1024)
#   ifndef CC13XXF64
#       define CC13XXF64
#   endif

// Devices with 128KB FLASH / 16KB SRAM (not including cache)
#elif (   defined(__CC1310F128RSM__) \
    ||  defined(__CC1310F128RHB__) \
    ||  defined(__CC1310F128RGZ__) )
#   define SRAM_SIZE    (16*1024)    
#   define EEPROM_SIZE  (0*1024)
#   define FLASH_SIZE   (128*1024)
#   ifndef CC13XXF128
#       define CC13XXF128
#   endif

// Devices with 128KB FLASH / 20KB SRAM (not including cache)
#elif (defined(__CC1350F128RGZ__))
#   define SRAM_SIZE    (20*1024)    
#   define EEPROM_SIZE  (0*1024)
#   define FLASH_SIZE   (128*1024)
#   ifndef CC1350F128
#       define CC1350F128
#   endif

#else
#   warning "Unknown CC13xx defined.  Defaulting to CC1310F128RGZ"
#   define SRAM_SIZE    (16*1024)    
#   define EEPROM_SIZE  (0*1024)
#   define FLASH_SIZE   (128*1024)
#   ifndef CC13XXF128
#       define CC13XXF128
#   endif
#endif






/** Low-Level Library inclusion <BR>
  * ========================================================================<BR>
  * 1. A few CMSIS Cortex M3 Hardware intrinsics that OpenTag uses
  * 2. CC13xxWare driver and low-level support (lots of headers)
  */

// From lib/CMSIS/Include
#include <cm3_endian.h>
#include <cm3_byteswap.h>
#include <cm3_bitrotate.h>
#include <cm3_countleadingzeros.h>
#include <cm3_saturation.h>

// From CC13xxWare directory (it must be added do header search dirs)
#include <inc/asmdefs.h>
#include <inc/hw_adi.h>
#include <inc/hw_adi_0_rf.h>
#include <inc/hw_adi_1_synth.h>
#include <inc/hw_adi_2_refsys.h>
#include <inc/hw_adi_3_refsys.h>
#include <inc/hw_adi_4_aux.h>
#include <inc/hw_aon_batmon.h>
#include <inc/hw_aon_event.h>
#include <inc/hw_aon_ioc.h>
#include <inc/hw_aon_rtc.h>
#include <inc/hw_aon_sysctl.h>
#include <inc/hw_aon_wuc.h>
#include <inc/hw_aux_aiodio.h>
#include <inc/hw_aux_anaif.h>
#include <inc/hw_aux_evctl.h>
#include <inc/hw_aux_sce.h>
#include <inc/hw_aux_smph.h>
#include <inc/hw_aux_tdc.h>
#include <inc/hw_aux_timer.h>
#include <inc/hw_aux_wuc.h>
#include <inc/hw_ccfg.h>
#include <inc/hw_ccfg_simple_struct.h>
#include <inc/hw_chip_def.h>
#include <inc/hw_cpu_dwt.h>
#include <inc/hw_cpu_fpb.h>
#include <inc/hw_cpu_itm.h>
#include <inc/hw_cpu_rom_table.h>
#include <inc/hw_cpu_scs.h>
#include <inc/hw_cpu_tiprop.h>
#include <inc/hw_cpu_tpiu.h>
#include <inc/hw_crypto.h>
#include <inc/hw_ddi.h>
#include <inc/hw_ddi_0_osc.h>
#include <inc/hw_device.h>
#include <inc/hw_event.h>
#include <inc/hw_fcfg1.h>
#include <inc/hw_flash.h>
#include <inc/hw_gpio.h>
#include <inc/hw_gpt.h>
#include <inc/hw_i2c.h>
#include <inc/hw_i2s.h>
#include <inc/hw_ints.h>
#include <inc/hw_ioc.h>
#include <inc/hw_memmap.h>
#include <inc/hw_nvic.h>
#include <inc/hw_prcm.h>
#include <inc/hw_rfc_dbell.h>
#include <inc/hw_rfc_pwr.h>
#include <inc/hw_rfc_rat.h>
#include <inc/hw_smph.h>
#include <inc/hw_ssi.h>
#include <inc/hw_sysctl.h>
#include <inc/hw_trng.h>
#include <inc/hw_types.h>
#include <inc/hw_uart.h>
#include <inc/hw_udma.h>
#include <inc/hw_vims.h>
#include <inc/hw_wdt.h>
#include <driverlib/adi.h>
#include <driverlib/adi_doc.h>
#include <driverlib/aon_batmon.h>
#include <driverlib/aon_event.h>
#include <driverlib/aon_event_doc.h>
#include <driverlib/aon_ioc.h>
#include <driverlib/aon_ioc_doc.h>
#include <driverlib/aon_rtc.h>
#include <driverlib/aon_rtc_doc.h>
#include <driverlib/aon_wuc.h>
#include <driverlib/aux_adc.h>
#include <driverlib/aux_smph.h>
#include <driverlib/aux_tdc.h>
#include <driverlib/aux_timer.h>
#include <driverlib/aux_wuc.h>
#include <driverlib/ccfgread.h>
#include <driverlib/ccfgread_doc.h>
#include <driverlib/chipinfo.h>
#include <driverlib/cpu.h>
#include <driverlib/cpu_doc.h>
#include <driverlib/crypto.h>
#include <driverlib/ddi.h>
#include <driverlib/ddi_doc.h>
#include <driverlib/debug.h>
#include <driverlib/driverlib_release.h>
#include <driverlib/event.h>
#include <driverlib/event_doc.h>
#include <driverlib/flash.h>
#include <driverlib/gpio.h>
#include <driverlib/gpio_doc.h>
#include <driverlib/group_analog_doc.h>
#include <driverlib/group_aon_doc.h>
#include <driverlib/group_aux_doc.h>
#include <driverlib/i2c.h>
#include <driverlib/i2c_doc.h>
#include <driverlib/i2s.h>
#include <driverlib/interrupt.h>
#include <driverlib/interrupt_doc.h>
#include <driverlib/ioc.h>
#include <driverlib/ioc_doc.h>
#include <driverlib/osc.h>
#include <driverlib/prcm.h>
#include <driverlib/pwr_ctrl.h>
#include <driverlib/rf_ble_cmd.h>
#include <driverlib/rf_ble_mailbox.h>
#include <driverlib/rf_common_cmd.h>
#include <driverlib/rf_data_entry.h>
#include <driverlib/rf_hs_cmd.h>
#include <driverlib/rf_hs_mailbox.h>
#include <driverlib/rf_mailbox.h>
#include <driverlib/rf_prop_cmd.h>
#include <driverlib/rf_prop_mailbox.h>
#include <driverlib/rfc.h>
#include <driverlib/rom.h>
#include <driverlib/rom_crypto.h>
#include <driverlib/setup.h>
#include <driverlib/setup_rom.h>
#include <driverlib/smph.h>
#include <driverlib/smph_doc.h>
#include <driverlib/ssi.h>
#include <driverlib/sys_ctrl.h>
#include <driverlib/systick.h>
#include <driverlib/systick_doc.h>
#include <driverlib/timer.h>
#include <driverlib/timer_doc.h>
#include <driverlib/trng.h>
#include <driverlib/uart.h>
#include <driverlib/udma.h>
#include <driverlib/vims.h>
#include <driverlib/watchdog.h>
#include <driverlib/watchdog_doc.h>
#include <rf_patches/rf_patch_cpe_ble.h>
#include <rf_patches/rf_patch_cpe_ble_priv_1_2.h>
#include <rf_patches/rf_patch_cpe_genfsk.h>
#include <rf_patches/rf_patch_cpe_genook.h>
#include <rf_patches/rf_patch_cpe_lrm.h>
#include <rf_patches/rf_patch_mce_genook.h>
#include <rf_patches/rf_patch_mce_hsp_4mbps.h>
#include <rf_patches/rf_patch_rfe_ble.h>
#include <rf_patches/rf_patch_rfe_genfsk.h>
#include <rf_patches/rf_patch_rfe_genook.h>
#include <rf_patches/rf_patch_rfe_hsp_4mbps.h>
#include <rf_patches/rf_patch_rfe_lrm.h>





/** Cryptographic Library Setup      <BR>
  * ========================================================================<BR>
  * Needed to build AES128 or other types of Crypto.  OpenTag has a fully
  * software-based Crypto lib that may be used if no HW is present. However,
  * CC13xx has an internal AES coprocessor with CTR support.
  */
///@todo add library include

  
  

/** Special Platform functions for CC13xx      <BR>
  * ========================================================================<BR>
  * @todo These must be defined before including platform_config.h, so that they
  * can be used for inline functions.
  */
void platform_ext_wakefromstop();
void platform_ext_hsitrim();
ot_u16 platform_ext_lsihz();




/** GPTIM Limiting     <BR>
  * ========================================================================<BR>
  * GPTIM Limit is in TICKS, or units of 1/1024 second.  The CC13xx RTC alarms 
  * allow a 32 bit counter for GPTIM.  The counter has 1/32768 (2^-15) second 
  * resolution and is 32 bits.  Thus the maximum limit is 2^27 ticks, or 2^17
  * seconds = 131072 s = ~36.4 hrs.  Quite a long time.
  *
  * For debugging, it is better to set the limit to a short time in order to 
  * observe timing problems.  For release, it may be as long as feasible.
  */
#ifndef OT_GPTIM_LIMIT
#   if defined(__DEBUG__)
#       define OT_GPTIM_LIMIT   (16 * 1024)
#   else
#       define OT_GPTIM_LIMIT   (131000 * 1024)
#   endif
#endif  




/** Platform Support settings      <BR>
  * ========================================================================<BR>
  * CC13xx is little endian with 4 byte pointer (32 bits), and at this stage it
  * can be compiled using GCC or TI's ARM compiler
  */

#define PLATFORM(VAL)                   PLATFORM_##VAL
#define PLATFORM_CC13XX
#define PLATFORM_CC13XX_CC26XX

#ifndef __LITTLE_ENDIAN__
#   error "Endian-ness misdefined, should be __LITTLE_ENDIAN__ (check build_config.h)"
#endif
#define PLATFORM_POINTER_SIZE           4               // How many bytes is a pointer?
#define PLATFORM_ENDIAN16(VAR16)        __REV16(VAR16)  // Big-endian to Platform-endian
#define PLATFORM_ENDIAN32(VAR32)        __REV(VAR32)    // Big-endian to Platform-endian
#define PLATFORM_ENDIAN16_C(CONST16)    (ot_u16)( (((ot_u16)CONST16) << 8) | (((ot_u16)CONST16) >> 8) )






/** CC13xx family MCU settings     <BR>
  * ========================================================================<BR>
  * MCU feature settings that are important for OpenTag to know.
  */
#define MCU_FEATURE(VAL)                MCU_FEATURE_##VAL   // FEATURE                  NOTE
#define MCU_FEATURE_SVMONITOR           ENABLED             // Auto Low V powerdown     Low Batt monitor
#define MCU_FEATURE_CRC16               DISABLED            // CRC16                    On RF Core but not MCU
#define MCU_FEATURE_CRC                 MCU_FEATURE_CRC16   // Legacy definition
#define MCU_FEATURE_AES128              ENABLED             // AES128 engine            AES Engine
#define MCU_FEATURE_ECC                 DISABLED            // ECC engine               Not available

#define MCU_TYPE(VAL)                   MCU_TYPE_##VAL
#define MCU_TYPE_PTRINT                 ot_s32
#define MCU_TYPE_PTRUINT                ot_u32

#define MCU_PARAM(VAL)                  MCU_PARAM_##VAL
#define MCU_PARAM_POINTERSIZE           4
#define MCU_PARAM_ERRPTR                ((ot_s32)-1)

// 22bit field: DIVINT is shifted up 6 bits, and DIVFRAC is in the lower 6 bits
// Intended for 24MHz peripheral clock, which is the standard we use on CC13xx.
#define MCU_PARAM_UART_9600BPS          ((156<<6) | (16))
#define MCU_PARAM_UART_28800BPS         ((52<<6) | (5))
#define MCU_PARAM_UART_57600BPS         ((26<<6) | (3))
#define MCU_PARAM_UART_115200BPS        ((13<<6) | (1))
#define MCU_PARAM_UART_250000BPS        ((6<<6) | (0))
#define MCU_PARAM_UART_500000BPS        ((3<<6) | (0))

#define SRAM_START_ADDR                 0x20000000
#define RFRAM_START_ADDR                0x21000000
#define AUXRAM_START_ADDR               0x400E0000
#define FLASH_START_ADDR                0x00000000
#define FLASH_START_PAGE                0
#define FLASH_PAGE_SIZE                 4096
#define FLASH_WORD_BYTES                4
#define FLASH_WORD_BITS                 (FLASH_WORD_BYTES*8)
#define FLASH_PAGE_ADDR(VAL)            (FLASH_START_ADDR + ( (VAL) * FLASH_PAGE_SIZE) )





/** Chip Settings  <BR>
  * ========================================================================<BR>
  * We support GCC and TI ARM compilers.
  */



/* Low Power Mode Macros: (Deprecated)
  * ========================================================================<BR>
  * SLEEP_MCU():        Core off, APB on, SRAM on                       (~50 uA)
  * SLEEP_WHILE_UHF():  Core off, APB on, SRAM on                       (~10 uA)
  * STOP_MCU():         Core off, RTC on, SRAM on                       (~1.5 uA)
  * STANDBY_MCU():      Core off, clocks off, SRAM off                  (~0.2 uA)
  */
//#define MCU_SLEEP               __WFI
//#define MCU_SLEEP_WHILE_IO      __WFI
//#define MCU_SLEEP_WHILE_RF      __WFI
//#define MCU_STOP()              error
//#define MCU_STANDBY()           error





/** Data section Nomenclature  <BR>
  * ========================================================================<BR>
  * We support GCC and TI ARM compilers.
  * Their methods for handling data sections are different enough that two sets
  * of files are needed for startup and filesystem default data placement.
  */




/** Platformized Peripheral Interrupt Handlers <BR>
  * ========================================================================<BR>
  * Use these "platform_isr_...()" functions in your code to service peripheral
  * interrupts.  To use them you must implement them in your code.  OpenTag 
  * implements some of them in its drivers (the ones it needs).  You can see 
  * how they are called from the low-level interrupts in the file:
  * platform/cc13xx_cc26xx/core_isr.c.
  *
  * Some Usage Notes on GPIO interrupts:
  * <li>"platform_isr_gpio()" is a hardware-analogous ISR that will be called
  *     when *any* GPIO that's configured on the MCU (GPIO can be configured to
  *     send interrupt to MCU, AON, or both), has an event.</li>
  * <li>"platform_isr_gpioX()" is a synthetic ISR that will be called when
  *     an event happens on a specific GPIO pin that is tied to MCU</li>
  * <li>"platform_isr_aon()" and "platform_isr_aonrtc()" are both hardware-
  *     analogous interrupts that are used with the "AON event fabric."  
  *     Either can be configured to select GPIO interrupts or actually a bunch
  *     of other systems (see Technical Reference Manual)</li>
  * <li>"platform_isr_aon_...()" ISR functions are synthetic, and they are
  *     called when the AON event hits a configured peripheral</li>
  *
  * Some Usage Notes on DMA interrupts:
  * CC13XX uses the ARM uDMA, which has channels that are for the most part
  * reserved for peripherals.  There are 4 channels for general purpose SW
  * utilization (one of which goes to memcpy).  The DMA interrupts are all
  * synthetic interrupts apart from the global DMA interrupts, which are:
  * "platform_isr_udma()" and "platform_isr_udmaerr()" 
  *
  */

void platform_isr_gpio(void);           //hits if *any* GPIO pin in MCU domain has event
void platform_isr_i2c(void);
void platform_isr_rfccpe1(void);
void platform_isr_aonspi(void);
void platform_isr_aonrtc(void);
void platform_isr_uart0(void);
void platform_isr_auxswevent0(void);
void platform_isr_ssi0(void);
void platform_isr_ssi1(void);
void platform_isr_rfccpe0(void);
void platform_isr_rfchw(void);
void platform_isr_rfccmdack(void);
void platform_isr_i2s(void);
void platform_isr_auxswevent1(void);
void platform_isr_wdog(void);
void platform_isr_tim0a(void);
void platform_isr_tim0b(void);
void platform_isr_tim1a(void);
void platform_isr_tim1b(void);
void platform_isr_tim2a(void);
void platform_isr_tim2b(void);
void platform_isr_tim3a(void);
void platform_isr_tim3b(void);
void platform_isr_crypto(void);
void platform_isr_udmasw(void);
void platform_isr_udmaerr(void);
void platform_isr_flash(void);
void platform_isr_swevent0(void);
void platform_isr_auxcombevent(void);
void platform_isr_aonprog(void);
void platform_isr_dynprog(void);
void platform_isr_auxcompa(void);
void platform_isr_auxadc(void);
void platform_isr_trng(void);


/// Synthetic GPIO interrupts
/// There are two options:
/// 1. gpiox : interrupt occurs when the GPIO is configured in the MCU domain
/// 2. aoniox : interrupt occurs when the GPIO is configured for the AON domain
///
/// If for some reason you have configured a pin for both MCU and AON domains, 
/// both interrupts will occur.
///
void platform_isr_gpio0(void);
void platform_isr_gpio1(void);
void platform_isr_gpio2(void);
void platform_isr_gpio3(void);
void platform_isr_gpio4(void);
void platform_isr_gpio5(void);
void platform_isr_gpio6(void);
void platform_isr_gpio7(void);
void platform_isr_gpio8(void);
void platform_isr_gpio9(void);
#if (MCU_PARAM_GPIO > 10)
void platform_isr_gpio10(void);
void platform_isr_gpio11(void);
void platform_isr_gpio12(void);
void platform_isr_gpio13(void);
void platform_isr_gpio14(void);
#endif
#if (MCU_PARAM_GPIO > 15)
void platform_isr_gpio15(void);
void platform_isr_gpio16(void);
void platform_isr_gpio17(void);
void platform_isr_gpio18(void);
void platform_isr_gpio19(void);
void platform_isr_gpio20(void);
void platform_isr_gpio21(void);
void platform_isr_gpio22(void);
void platform_isr_gpio23(void);
void platform_isr_gpio24(void);
void platform_isr_gpio25(void);
void platform_isr_gpio26(void);
void platform_isr_gpio27(void);
void platform_isr_gpio28(void);
void platform_isr_gpio29(void);
#endif

/// Synthetic uDMA interrupts
/// Listed in order of descending priority (top is highest)
void platform_isr_udma_sw0(void);
void platform_isr_udma_uart0rx(void);
void platform_isr_udma_uart0tx(void);
void platform_isr_udma_ssp0rx(void);
void platform_isr_udma_ssp0tx(void);
void platform_isr_udma_spisrx(void);
void platform_isr_udma_spistx(void);
void platform_isr_udma_auxadc(void);
void platform_isr_udma_auxsw(void);
void platform_isr_udma_gpt0a(void);
void platform_isr_udma_gpt0b(void);
void platform_isr_udma_gpt1a(void);
void platform_isr_udma_gpt1b(void);
void platform_isr_udma_aonprog2(void);
void platform_isr_udma_dmaprog(void);
void platform_isr_udma_aonrtc(void);
void platform_isr_udma_ssp1rx(void);
void platform_isr_udma_ssp1tx(void);
void platform_isr_udma_sw1(void);
void platform_isr_udma_sw2(void);
void platform_isr_udma_sw3(void);




/** Cortex M3 Universal Interrupts & Handler Functions  <BR>
  * ========================================================================<BR>
  * These interrupts are on all CM3 chips, and we use the STMicro nomenclature
  * because they were first.  The TI nomenclature for their GCC builds is very
  * similar, you can find it in cc13xxware.../startup_files/startup_gcc.c
  *
  * These ISRs are implemented in:
  * - platform/cc13xx_cc26xx/core_errors.c      (Faults, NMI, DebugMon, MemManage)
  * - platform/cc13xx_cc26xx/core_tasking.c     (SVC, PendSV, SysTick)
  *
  * Some are used, others are not.  In particular, OpenTag does not use the
  * SysTick and in fact bars you from using it at all.
  */
void HardFault_Handler(void);
void NMI_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);




/** Cortex M3 NVIC configuration <BR>
  * ========================================================================<BR>
  * Nested Vector Interrupt Controller on the CC13xx has the standard CM3
  * features and registers.  OpenTag likes to use the 2-group NVIC model, one
  * group for the kernel and bios stuff, and one for everything higher level.
  * 4-group model is OK too, but 2-group has much less risk of blowing-up the
  * ISR stack.
  */

#ifndef __CM3_NVIC_GROUPS
#   define __CM3_NVIC_GROUPS    2
#endif

#if (__CM3_NVIC_GROUPS == 1)
#   error "__CM3_NVIC_GROUPS is set to 1, but GULP Kernel not supported on this device."

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






/** Default OpenTag Threading and Stack Parameters  <BR>
  * ========================================================================<BR>
  * These are the default parameters for the OpenTag RTOS's handling of stacks.
  * OpenTag has three stacks:
  *
  * P-stack: (P is an ARM CM term) The kernel stack, i.e. the main stack, 
  *          which is usually defined in linker configuration.
  *
  * S-stack: (S is an ARM CM term) Used by ISRs.  OpenTag puts critical I/O 
  *          tasks in the ISRs, such as Radio and MPipe.  512 bytes is found 
  *          to be sufficient with extra room for safety.
  * 
  * T-stack: Total allocation of stack for threads (apps).  This will be 
  *          added to the heap if memory allocation is enabled.  If threads
  *          are disabled, it will be 0.  When OpenTag is configured as a
  *          BIOS for a child OS (e.g. NuttX), this parameter is passed to
  *          the child OS configuration.
  *
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





/** CC13XX Cryptography Include <BR>
  * ========================================================================<BR>
  * Alias OTEAX Data Type into generic type for OpenTag
  * 
  * @todo there may be some work here to enable CC13XX HW
  */
#if OT_FEATURE(DLL_SECURITY) || OT_FEATURE(NL_SECURITY) || OT_FEATURE(VL_SECURITY)
#   include <oteax.h>
//#   define EAXdrv_t eax_ctx
    typedef eax_ctx EAXdrv_t;
#else
//#   define EAXdrv_t ot_uint
    typedef ot_uint EAXdrv_t;
#endif





/** CC13XX Special Platform Functions & data for timing & clocking <BR>
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
//#   if defined(__DEBUG__)
//    ot_u16  stamp1;
//    ot_u16  stamp2;
//#   else
    ot_u16  chron_stamp;
    ot_u32  evt_stamp;
    ot_u32  evt_span;
//#   endif
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





/** CC13xx Platform Data  <BR>
  * ========================================================================<BR>
  * platform_ext stores data that is required for OpenTag to work properly on
  * the CC13xx.  The data is not platform-independent, but it's pretty similar
  * across ARM CM3's.
  */

typedef struct {
    // Tasking parameters: similar across CM3's
    void*   task_exit;
    
    // Clock speed saves: AHB, APB1, APB2
    ///@todo check if this is necessary on CC13xx
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
