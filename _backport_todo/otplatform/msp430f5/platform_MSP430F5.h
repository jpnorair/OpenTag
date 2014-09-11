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
  *
  */
/**
  * @file       /otplatform/MSP430F5/platform_MSP430F5.h
  * @author     JP Norair
  * @version    R102
  * @date       4 Nov 2012
  * @brief      Platform Library Macros and Functions for MSP430F5
  * @ingroup    Platform
  *
  ******************************************************************************
  */


#ifndef __PLATFORM_MSP430F5_H
#define __PLATFORM_MSP430F5_H

#include <app/build_config.h>
#include <otsys/support.h>
#include "msp430f5_lib.h"
#include "msp430f5_conf.h"


/** Platform Support settings
  * These reference the exhaustive list of officially supported platform
  * setting options.
  */
#define __MSP430F5__
#define PLATFORM(VAL)           PLATFORM_##VAL
#define PLATFORM_MSP430F5

#ifndef __LITTLE_ENDIAN__
#   error "Endian-ness misdefined, should be __LITTLE_ENDIAN__ (check build_config.h)"
#endif

// How many bytes is a pointer?
///@note Filesystem contents MUST be in lower 64KB of memory space in order to
///      use 2 byte pointers.  If filesystem is in upper range, you must change
///      to 4 bytes.  Most F5's have at most 48KB in the lower space.
#define PLATFORM_POINTER_SIZE   2

// Big-endian to Platform-endian
#define PLATFORM_ENDIAN16(VAR16)    __swap_bytes(VAR16)
#define PLATFORM_ENDIAN32(VAR32)    __swap_long_bytes(VAR32)




/** Interrupt Nomenclature  <BR>
  * ========================================================================<BR>
  */
#if (CC_SUPPORT == GCC)
#   include <intrinsics.h>
#   define OT_IRQPRAGMA(VAL)            __attribute__((interrupt(VAL)))
#   define OT_INTERRUPT
#   define __get_SR_register            __read_status_register
#   define __even_in_range(x, y)        x
#   define __OT_INTERRUPT_VECTOR(VAL)   __attribute__((interrupt(VAL)))
#   define __OT_INTERRUPT_ISR(FN)       void ##FN (void)

#elif (CC_SUPPORT == CL430)
#	define OT_FNPRAGMA(VAL)             _Pragma(#VAL)
#   define OT_IRQPRAGMA(VAL)            _Pragma(#VAL)
#   define OT_INTERRUPT                 __interrupt
#   define __OT_INTERRUPT_VECTOR(VAL)   _Pragma(#VAL)
#   define __OT_INTERRUPT_ISR(FN)       __interrupt void ##FN (void)

#elif (CC_SUPPORT == IAR_V5)

#endif



/** Data section Nomenclature  <BR>
  * ========================================================================<BR>
  */
#if (CC_SUPPORT == GCC)
#   define OT_SECTION(VAR, SECTION)     __attribute__((section(##SECTION))

#elif (CC_SUPPORT == CL430)
#   define OT_DATAPRAGMA(VAR, SECTION)  _Pragma(DATA_SECTION(##VAR, ##SECTION))
#   define OT_SECTION                   OT_DATAPRAGMA
#elif (CC_SUPPORT == IAR_V5)
#endif






/** Low Power Mode Macros:
  * ========================================================================<BR>
  * Within OpenTag, only SLEEP_MCU is used.  The other low power modes may
  * be used outside OpenTag, especially during idle periods in the MAC sequence.
  * STANDBY is not normally useful because it shuts off the RTC.
  *
  * SLEEP_MCU():        LPM1 - Core off, SRAM on                        (~50 uA)
  * (unused):           LPM2 - Core off, SMCLK off, FLL off, SRAM on    (~10 uA)
  * STOP_MCU():         LPM3 - Core off, ACLK on (RTC), SRAM on         (~2 uA)
  * STANDBY_MCU():      LPM4 - Core off, clocks off, SRAM on            (~1 uA)
  */
#define MCU_SLEEP               PMM_EnterLPM1
#define MCU_SLEEP_WHILE_IO      PMM_EnterLPM1
#define MCU_SLEEP_WHILE_RF      PMM_EnterLPM1
#define MCU_STOP                PMM_EnterLPM3
#define MCU_STANDBY             PMM_EnterLPM4

//legacy deprecated
#define SLEEP_MCU()             MCU_SLEEP()
#define SLEEP_WHILE_UHF()       MCU_SLEEP_WHILE_RF()
#define STOP_MCU()              MCU_STOP()
#define STANDBY_MCU()           MCU_STANDBY()



/** #### DMA Macros
  * - How many bytes/transfers are left for the DMA
  */
#define OT_RADIODMA_RX_BYTE     (ot_u16)(OT_RADIODMA_RX->SZ)
#define OT_RADIODMA_TX_BYTE     (ot_u16)(OT_RADIODMA_TX->SZ)







/** MSP430 Platform Data  <BR>
  * ========================================================================<BR>
  * platform_ext stores data that is required for OpenTag to work properly on
  * the MSP430, and the data is not platform-independent.
  */
#if (OT_FEATURE_TIME == ENABLED)
#define RTC_ALARMS          1        // Max=3
#else
#define RTC_ALARMS          0
#endif

#define RTC_OVERSAMPLE      0                           // unsupported on MSP430

typedef struct {
    u8   disabled;
    u8   taskid;
    u16  mask;
    u16  value;
} rtcalarm;

typedef struct {
    u16* task_entry;
    u16  prand_reg;
    
#   if 1
    u8 usb_wakeup;
#   endif
#   if (RTC_OVERSAMPLE)
#   endif
#   if (RTC_ALARMS > 0)
        rtcalarm alarm[RTC_ALARMS];
#   endif

} platform_ext_struct;


extern platform_ext_struct  platform_ext;



/**
  * The COFFABI and CL430 compiler used by CCS have serious problems with
  * inline function definitions.  Inline functions are necessary for any ISRs
  * that must bring the MCU out of sleep (via LPM4_EXIT macro).
  *
  * The only interrupt in OpenTag that normally uses LPM4_EXIT, on MSP430, is
  * the kernel timer interrupt.  It will be either Tim0A1 or Tim1A1.  CL430's
  * preprocessor is below the GCC standard, so it takes some wrangling to get
  * board configuration to dynamically inline the correct timer.
  * get
  */

#define __ISR_VECTOR(ID)	(0xFF80 + (ID*2))

// This list works for all known F55xx devices
#define __ISR_RESET_ID		63
#define __ISR_SYSNMI_ID    	62
#define __ISR_USERNMI_ID   	61
#define __ISR_CB_ID         60
#define __ISR_T0B0_ID       59
#define __ISR_T0B1_ID       58
#define __ISR_WDTI_ID       57
#define __ISR_USCIA0_ID     56
#define __ISR_USCIB0_ID     55

#if defined(_ADC10)
#   define __ISR_ADC10A_ID  54
#elif defined(_ADC12)
#   define __ISR_ADC12A_ID  54
#endif

#define __ISR_T0A0_ID       53
#define __ISR_T0A1_ID       52
#define __ISR_USB_ID        51
#define __ISR_DMA_ID        50
#define __ISR_T1A0_ID       49
#define __ISR_T1A1_ID       48
#define __ISR_P1_ID         47
#define __ISR_USCIA1_ID     46
#define __ISR_USCIB1_ID     45
#define __ISR_T2A0_ID       44
#define __ISR_T2A1_ID       43
#define __ISR_P2_ID         42
#define __ISR_RTCA_ID       41






void platform_isr_reset(void);

void platform_isr_sysnmi(void);

void platform_isr_usernmi(void);

void platform_isr_cb(void);

void platform_isr_tim0b0(void);
void platform_isr_tim0b1(void);

void platform_isr_wdti(void);

void platform_isr_uscia0(void);
void platform_isr_uscib0(void);

#if defined(_ADC10)
void platform_isr_adc10a(void);
#elif defined(_ADC12)
void platform_isr_adc12a(void);
#endif

void platform_isr_tim0a0(void);
void platform_isr_tim0a1(void);

#ifdef _USB
// Returns 0/1 to cause wakeup from LPM
void platform_isr_usb(void);
#endif

void platform_isr_dma(void);

void platform_isr_tim1a0(void);
void platform_isr_tim1a1(void);

void platform_isr_p1(void);

void platform_isr_uscia1(void);
void platform_isr_uscib1(void);

void platform_isr_tim2a0(void);
void platform_isr_tim2a1(void);

void platform_isr_p2(void);

void platform_isr_rtca(void);




#ifdef _LCD
void platform_isr_lcdb(void);
#endif

#ifdef _AES
void platform_isr_aes(void);
#endif







#endif
