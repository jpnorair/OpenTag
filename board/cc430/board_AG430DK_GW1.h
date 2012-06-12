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
  * @file       /board/CC430/board_AG430DK_GW1.h
  * @author     JP Norair
  * @version    V1.0
  * @date       5 February 2012
  * @brief      Board Configuration for Agaidi CC430 Gateway
  * @ingroup    Platform
  *
  * Do not include this file, include OT_platform.h
  *
  * Specifically, this board is from the Rev1 Agaidi Dev Kit.  It is the one
  * that has the FTDI USB connector built-in.  Other than that, it is actually 
  * identical to the non-USB board that is also in the kit.
  ******************************************************************************
  */
  

#ifndef __board_AG430DK_GW1_H
#define __board_AG430DK_GW1_H

#include "build_config.h"
#include "platform_CC430.h"
#include "radio_CC430.h"



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




/** RF Front End Parameters and Settings <BR>
  * ========================================================================<BR>
  * The Agaidi Kits are built for 433 MHz.
  */
#define RF_PARAM_BAND   433




/** Platform Memory Configuration <BR>
  * ========================================================================<BR>
  * OpenTag needs to know where it can put Nonvolatile memory (file system) and
  * how much space it can allocate for filesystem.
  */
#define SRAM_START_ADDR         0x0000
#define SRAM_SIZE               (4*1024)
#define EEPROM_START_ADDR       0
#define EEPROM_SIZE             0
#define FLASH_START_ADDR        0x8000
#define FLASH_START_PAGE        0
#define FLASH_PAGE_SIZE         512
#define FLASH_NUM_PAGES         64
#define FLASH_WORD_BYTES        2
#define FLASH_WORD_BITS         (FLASH_WORD_BYTES*8)
#define FLASH_FS_ALLOC          (512*8)     //using only 8 blocks: 5 primary, 3 fallow
#define FLASH_FS_ADDR           0x8000
#define FLASH_PAGE_ADDR(VAL)    (FLASH_START_ADDR + ( (VAL) * FLASH_PAGE_SIZE) )





// MSP430 only ... Information Flash
// Info Page A usage is riddled with glitches, hence limiting to 3 pages
// instead of 4.  Page A is addresses 1980 - 19FF
#   define INFO_START_ADDR          0x1800
#   define INFO_START_PAGE          0
#   define INFO_PAGE_SIZE           128
#   define INFO_NUM_PAGES           3
#   define INFO_PAGE_ADDR(VAL)      (INFO_START_ADDR + (INFO_PAGE_SIZE*VAL))
#   define INFO_D_ADDR              INFO_PAGE_ADDR(0)
#   define INFO_C_ADDR              INFO_PAGE_ADDR(1)
#   define INFO_B_ADDR              INFO_PAGE_ADDR(2)
#   define INFO_A_ADDR              INFO_PAGE_ADDR(3)





/** MCU Feature settings      <BR>
  * ========================================================================<BR>
  * Implemented capabilities of the CC430F5137/6137 variants
  */
#define MCU_FEATURE(VAL)                 MCU_FEATURE_##VAL       // FEATURE                  AVAILABILITY
#define MCU_FEATURE_CRC                  ENABLED                 // CCITT CRC16              Low
#define MCU_FEATURE_AES128               ENABLED                 // AES128 engine            Moderate
#define MCU_FEATURE_ECC                  DISABLED                // ECC engine               Low
#define MCU_FEATURE_ALGE                 DISABLED                // Algebraic Eraser engine  Rare/None yet
#define MCU_FEATURE_RADIODMA             DISABLED
#define MCU_FEATURE_RADIODMA_TXBYTES     0
#define MCU_FEATURE_RADIODMA_RXBYTES     0
#define MCU_FEATURE_MAPEEPROM            DISABLED
#define MCU_FEATURE_MPIPEDMA             ENABLED
#define MCU_FEATURE_MEMCPYDMA            DISABLED   // Must be disabled if MPIPEDMA is enabled





#define PLATFORM_FEATURE_USBCONVERTER    ENABLED




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
#define MCU_PARAM_LFXTALtol             0.00002                  // Uncomment (and change) if using LF XTAL
//#define MCU_PARAM_LFOSCHz               32768                   // Uncomment (and change) if using LF OSC
//#define MCU_PARAM_LFOSCtol              0.01                    // Uncomment (and change) if using LF OSC
//#define MCU_PARAM_XTALHz                27000000                // Uncomment (and change) if using XTAL
//#define MCU_PARAM_XTALtol               0.0001                  // Uncomment (and change) if using XTAL
#define MCU_PARAM_OSCHz                 19990000                // OSC can do ~20 MHz
#define MCU_PARAM_OSCtol                0.01                    // STM32 Osc typ +/- 1%
  
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
  * OT_GPTIM:   General Purpose Timer used by OpenTag kernel                <BR>
  * OT_TRIG:    Optional test trigger usable in OpenTag apps (often LEDs)   <BR>
  * MPIPE:      UART to use for the MPipe                                   <BR>
  */


#define OT_GPTIM            TIM0A5
#define OT_GPTIM_IRQ        TIM0A5_IRQChannel
#define OT_GPTIM_VECTOR     TIMER0_A1_VECTOR
#define OT_GPTIM_CLOCK      32768
#define OT_GPTIM_RES        1024
#define TI_TO_CLK(VAL)      ((OT_GPTIM_RES/1024)*VAL)
#define CLK_TO_TI(VAL)      (VAL/(OT_GPTIM_RES/1024))

#if (OT_GPTIM_CLOCK == PLATFORM_LSCLOCK_HZ)
#   undef OT_GPTIM_CLOCK
#   define OT_GPTIM_CLOCK   TIMA_Ctl_Clock_ACLK
#   define OT_GPTIM_ERROR   PLATFORM_LSCLOCK_ERROR
#else
#   undef OT_GPTIM_CLOCK
#   define OT_GPTIM_CLOCK   TIMA_Ctl_Clock_MCLK
#   define OT_GPTIM_ERROR   PLATFORM_HSCLOCK_ERROR
#endif

#define OT_GPTIM_ERRDIV     32768 //this needs to be hard-coded, or else CCS shits in its pants

///@todo Make "PLATFORM_GPTIM" conform to new "OT_GPTIM" nomenclature
///      Then this legacy definition block can be removed.
#define PLATFORM_GPTIM_HZ        PLATFORM_LSCLOCK_HZ
#define PLATFORM_GPTIM_PS        32
#define PLATFORM_GPTIM_CLK       (PLATFORM_LSCLOCK_HZ/PLATFORM_GPTIM_PS)
#define PLATFORM_GPTIM_RES       OT_GPTIM_RES
#define PLATFORM_GPTIM_ERROR     OT_GPTIM_ERROR
#define PLATFORM_GPTIM_ERRDIV    OT_GPTIM_ERRDIV       
#define PLATFORM_GPTIM_DEV       0

///@todo Legacy -- remove 
#define PLATFORM_HSI0_FREQ       8000000
#define PLATFORM_HSI1_FREQ       16000000
#define PLATFORM_HSI2_FREQ       20000000
#define PLATFORM_HSI3_FREQ       25000000
#define PLATFORM_XTAL_FREQ       27000000
#define PLATFORM_LSE_FREQ        32768

///@todo Legacy -- remove 
#define HSCLOCK_HZ               PROC_HSI2_FREQ
#define LSCLOCK_HZ               PROC_LSE_FREQ
#define HSCLOCK_ERROR            0.01
#define HSCLOCK_ERRPCT           (HSCLOCK_ERROR*100)
#define LSCLOCK_ERROR            0


// Trigger 1 = Green LED
// Trigger 2 = Yellow LED
#define OT_TRIG1_PORTNUM    3
#define OT_TRIG1_PORT       GPIO3
#define OT_TRIG1_PIN        GPIO_Pin_3  
#define OT_TRIG1_HIDRIVE    ENABLED         // Use high-current option
#define OT_TRIG2_PORTNUM    3
#define OT_TRIG2_PORT       GPIO3
#define OT_TRIG2_PIN        GPIO_Pin_5
#define OT_TRIG2_HIDRIVE    ENABLED


// Pin that can be used for ADC-based random number (usually floating pin)
// You could also put on a low voltage, reverse-biased zener on the board
// to produce a pile of noise.  2.1V seems like a good value.
#define OT_GWNADC_PORT      GPIO2
#define OT_GWNADC_PIN       GPIO_Pin_1
#define OT_GWNADC_BITS      1
//#define OT_GWNZENER_PORT    GPIO2
//#define OT_GWNZENER_PIN     GPIO_Pin_2
//#define OT_GWNZENER_HIDRIVE DISABLED


#define MPIPE_UARTNUM       0
#define MPIPE_UART_PORTNUM  1
#define MPIPE_UART_PORT     GPIO1
#define MPIPE_UART_PORTMAP  P2M
#define MPIPE_UART_RXPIN    GPIO_Pin_5
#define MPIPE_UART_TXPIN    GPIO_Pin_6
#define MPIPE_UART_PINS     (MPIPE_UART_RXPIN | MPIPE_UART_TXPIN)

#if (MPIPE_UARTNUM == 0)
#   define MPIPE_UART           UART0
#   define MPIPE_UART_RXSIG     PM_UCA0RXD
#   define MPIPE_UART_TXSIG     PM_UCA0TXD
#	define MPIPE_UART_RXTRIG	DMA_Trigger_UCA0RXIFG
#	define MPIPE_UART_TXTRIG	DMA_Trigger_UCA0TXIFG
#   define MPIPE_UART_VECTOR    USCI_A0_VECTOR
#elif (MPIPE_UARTNUM == 1)
#   define MPIPE_UART           UART1
#   define MPIPE_UART_RXSIG     PM_UCB0RXD
#   define MPIPE_UART_TXSIG     PM_UCB0TXD
#	define MPIPE_UART_RXTRIG	DMA_Trigger_UCB0RXIFG
#	define MPIPE_UART_TXTRIG	DMA_Trigger_UCB0TXIFG
#   define MPIPE_UART_VECTOR    USCI_B0_VECTOR
#else
#   error "MPIPE_UART is not defined to an available index (0-1)"
#endif

#if (MCU_FEATURE_MPIPEDMA == ENABLED)
#   define MPIPE_DMANUM    2
#   if (MPIPE_DMANUM == 0)
#       define MPIPE_DMA     DMA0
#   elif (MPIPE_DMANUM == 1)
#       define MPIPE_DMA     DMA1
#   elif (MPIPE_DMANUM == 2)
#       define MPIPE_DMA     DMA2
#   else
#       error "MPIPE_DMANUM is not defined to an available index (0-2)"
#   endif
#endif

#   if (MPIPE_UART_RXPIN == GPIO_Pin_0)
#       define MPIPE_UART_RXMAP    MPIPE_UART_PORTMAP->MAP0
#   elif (MPIPE_UART_RXPIN == GPIO_Pin_1)
#       define MPIPE_UART_RXMAP    MPIPE_UART_PORTMAP->MAP1
#   elif (MPIPE_UART_RXPIN == GPIO_Pin_2)
#       define MPIPE_UART_RXMAP    MPIPE_UART_PORTMAP->MAP2
#   elif (MPIPE_UART_RXPIN == GPIO_Pin_3)
#       define MPIPE_UART_RXMAP    MPIPE_UART_PORTMAP->MAP3
#   elif (MPIPE_UART_RXPIN == GPIO_Pin_4)
#       define MPIPE_UART_RXMAP    MPIPE_UART_PORTMAP->MAP4
#   elif (MPIPE_UART_RXPIN == GPIO_Pin_5)
#       define MPIPE_UART_RXMAP    MPIPE_UART_PORTMAP->MAP5
#   elif (MPIPE_UART_RXPIN == GPIO_Pin_6)
#       define MPIPE_UART_RXMAP    MPIPE_UART_PORTMAP->MAP6
#   elif (MPIPE_UART_RXPIN == GPIO_Pin_7)
#       define MPIPE_UART_RXMAP    MPIPE_UART_PORTMAP->MAP7
#   else
#       error "MPIPE_UART_RXPIN out of bounds"
#   endif
#   if (MPIPE_UART_TXPIN == GPIO_Pin_0)
#       define MPIPE_UART_TXMAP    MPIPE_UART_PORTMAP->MAP0
#   elif (MPIPE_UART_TXPIN == GPIO_Pin_1)
#       define MPIPE_UART_TXMAP    MPIPE_UART_PORTMAP->MAP1
#   elif (MPIPE_UART_TXPIN == GPIO_Pin_2)
#       define MPIPE_UART_TXMAP    MPIPE_UART_PORTMAP->MAP2
#   elif (MPIPE_UART_TXPIN == GPIO_Pin_3)
#       define MPIPE_UART_TXMAP    MPIPE_UART_PORTMAP->MAP3
#   elif (MPIPE_UART_TXPIN == GPIO_Pin_4)
#       define MPIPE_UART_TXMAP    MPIPE_UART_PORTMAP->MAP4
#   elif (MPIPE_UART_TXPIN == GPIO_Pin_5)
#       define MPIPE_UART_TXMAP    MPIPE_UART_PORTMAP->MAP5
#   elif (MPIPE_UART_TXPIN == GPIO_Pin_6)
#       define MPIPE_UART_TXMAP    MPIPE_UART_PORTMAP->MAP6
#   elif (MPIPE_UART_TXPIN == GPIO_Pin_7)
#       define MPIPE_UART_TXMAP    MPIPE_UART_PORTMAP->MAP7
#   else
#       error "MPIPE_UART_TXPIN out of bounds"
#   endif



#if (MCU_FEATURE_MEMCPYDMA == ENABLED)
#   define MEMCPY_DMANUM    1
#   if (MEMCPY_DMANUM == 0)
#       define MEMCPY_DMA     DMA0
#   elif (MEMCPY_DMANUM == 1)
#       define MEMCPY_DMA     DMA1
#   elif (MEMCPY_DMANUM == 2)
#       define MEMCPY_DMA     DMA2
#   else
#       error "MEMCPY_DMANUM is not defined to an available index (0-2)"
#   endif
#endif



/******* ALL SHIT BELOW HERE IS SUBJECT TO REDEFINITION **********/

/// Deprecated configuration stuff
#ifdef __VEELITE_NOINIT
#   define PLATFORM_INIT_VEELITE   DISABLED 
#else
#   define PLATFORM_INIT_VEELITE   ENABLED
#endif

#ifdef __SYSTEM_NOINIT
#   define PLATFORM_INIT_SYSTEM    DISABLED 
#else
#   define PLATFORM_INIT_SYSTEM    ENABLED
#endif

#ifdef __SESSION_NOINIT
#   define PLATFORM_INIT_SESSION   DISABLED 
#else
#   define PLATFORM_INIT_SESSION   ENABLED
#endif

#ifdef __AUTH_NOINIT
#   define PLATFORM_INIT_AUTH      DISABLED 
#else
#   define PLATFORM_INIT_AUTH      ENABLED
#endif 

#ifdef __BUFFERS_NOINIT
#   define PLATFORM_INIT_BUFFERS   DISABLED 
#else
#   define PLATFORM_INIT_BUFFERS   ENABLED
#endif 

#ifdef __MPIPE_NOINIT
#   define PLATFORM_INIT_MPIPE     DISABLED 
#else
#   define PLATFORM_INIT_MPIPE     ENABLED
#endif 

#ifdef __RADIO_NOINIT
#   define PLATFORM_INIT_RADIO     DISABLED 
#else
#   define PLATFORM_INIT_RADIO     ENABLED
#endif 



/** Deprecated Timer Macros
  */
#   define GPTIM_HZ_TI          1024
#   define GPTIM_HZ_STI         32768
#   define INIT_GPTIM_TI()      do { \
                                    OT_GPTIM->CTL   = 0x01C6; \
                                    OT_GPTIM->EX0   = 0x0003; \
                                } while(0)
                                    
#   define INIT_GPTIM_STI()     do { \
                                    OT_GPTIM->CTL   = 0x0106; \
                                    OT_GPTIM->EX0   = 0x0000; \
                                } while(0)
                                
#   define STOP_GPTIM()         OT_GPTIM->CTL  |= (u16)0x0004

#   define RUN_GPTIM(TICKS)     do { \
                                    STOP_GPTIM(); \
                                    OT_GPTIM->CCR0  = (u16)TICKS; \
                                    OT_GPTIM->CTL  |= (u16)0x0020; \
                                } while(0)
#   define RUN_GPTIM_TI(TICKS)  do { \
                                    INIT_GPTIM_LO(); \
                                    OT_GPTIM->CCR0  = (u16)TICKS; \
                                    OT_GPTIM->CTL  |= (u16)0x0020; \
                                } while(0)
#   define RUN_GPTIM_STI(TICKS) do { \
                                    INIT_GPTIM_HI(); \
                                    OT_GPTIM->CCR0  = (u16)TICKS; \
                                    OT_GPTIM->CTL  |= (u16)0x0020; \
                                } while(0)









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

#define OTF_CRC_TABLE           DISABLED
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
