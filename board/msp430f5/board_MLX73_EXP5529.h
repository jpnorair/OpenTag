/*  Copyright 2010-2011, JP Norair
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
  * @file       /board/MSP430F5/board_TIProto_EXP5529.h
  * @author     JP Norair
  * @version    V1.0
  * @date       5 February 2012
  * @brief      Board Configuration for MSP-EXP430F5529 kit, using CC1101EMK attached
  * @ingroup    Platform
  *
  * !!! UNTESTED !!! UNTESTED !!! UNTESTED !!! UNTESTED !!! UNTESTED !!!
  *
  * Do not include this file, include OT_platform.h
  *
  * The Texas Instruments MSP-EXP430F5529 is a dev kit using the MSP430F5529.
  * It includes various connectivity options, including a port that fits into
  * the CC1101EMK boards also available from TI.  So, this board configuration
  * is for the combined assembly of these two TI boards.  Additionally, it uses
  * the MSP430F5529's USB device peripheral to implement MPipe, and one of the 
  * serial ports is defined to connect to the RI-ACC-ADR2 kit (also from TI).
  * This kit is a 134 kHz reader, and it is completely optional.  It just gets
  * a serial port defined, which you can ignore if you don't have an ADR2.
  *
  * For more information on the board: 
  * http://www.indigresso.com/wiki/doku.php?id=opentag:board:tiproto_exp5529
  ******************************************************************************
  */
  

#ifndef __board_TIProto_EXP5529_H
#define __board_TIProto_EXP5529_H

#include "build_config.h"
#include "platform_MSP430F5.h"
#include "radio_CC1101.h"



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
  * The default is 433 MHz.  You could potentially change this to 866 if you
  * have 866 EMK boards, but make sure the radio module supports it.  Only 433
  * is confirmed working.
  */
#define RF_PARAM_BAND   433
#define RF_HDB_ATTEN    6       //Half dB attenuation (units = 0.5dB), used to scale TX power
#define RF_RSSI_OFFSET  3       //Offset applied to RSSI calculation




/** MCU-based Feature settings      <BR>
  * ========================================================================<BR>
  * Implemented capabilities of the MSP430F5529
  */
#define MCU_FEATURE(VAL)                MCU_FEATURE_##VAL       // FEATURE                  AVAILABILITY
#define MCU_FEATURE_CRC                 ENABLED                 // CCITT CRC16              Low
#define MCU_FEATURE_AES128              ENABLED                 // AES128 engine            Moderate
#define MCU_FEATURE_ECC                 DISABLED                // ECC engine               Low
#define MCU_FEATURE_RADIODMA            DISABLED
#define MCU_FEATURE_RADIODMA_TXBYTES    0
#define MCU_FEATURE_RADIODMA_RXBYTES    0
#define MCU_FEATURE_MAPEEPROM           DISABLED
#define MCU_FEATURE_MPIPEDMA            DISABLED //ENABLED    // Only for UART DMA
#define MCU_FEATURE_MEMCPYDMA           ENABLED     // Must be disabled if MPIPEDMA is enabled
#define MCU_FEATURE_MPIPEVCOM           ENABLED     // USB uses memcpy

#define MCU_PARAM(VAL)                  MCU_PARAM_##VAL
#define MCU_PARAM_POINTERSIZE           2

OT_INLINE_H BOARD_DMA_COMMON_INIT() {
    DMA->CTL4 = (   DMA_Options_RMWDisable | \
                    DMA_Options_RoundRobinDisable | \
                    DMA_Options_ENMIEnable  );
}



/** Board-based Feature Settings <BR>
  * ========================================================================<BR>
  */
#define BOARD_FEATURE(VAL)              BOARD_FEATURE_##VAL
#define BOARD_FEATURE_USBCONVERTER      ENABLED                 // Is UART connected via USB converter?
#define BOARD_FEATURE_LFXTAL            ENABLED                 // LF XTAL used as Clock source
#define BOARD_FEATURE_HFXTAL            ENABLED                 // HF XTAL used as Clock source

#define BOARD_PARAM(VAL)                BOARD_PARAM_##VAL
#define BOARD_PARAM_LFHz                32768
#define BOARD_PARAM_LFtol               0.00002
#define BOARD_PARAM_HFHz                4000000
#define BOARD_PARAM_HFtol               0.0001
#define BOARD_PARAM_HFmult              6                       // DCO = HFHz * HFmult
#define BOARD_PARAM_MCLKDIV             1                       // Master Clock = DCO / MCLKDIV
#define BOARD_PARAM_SMCLKDIV            4                       // Submaster Clock = DCO / SMCLKDIV (Keep < 6.5 MHz)

#if (BOARD_FEATURE(LFXTAL) == ENABLED)
#   define BOARD_LFXT_PINS      (GPIO_Pin_4 | GPIO_Pin_5)
#   define BOARD_LFXT_CONF      (XT1DRIVE_3 | UCS_CTL6_XTS_low | UCS_CTL6_XCAP_1)
#else
#   define BOARD_LFXT_PINS      0
#   define BOARD_LFXT_CONF      (XT1OFF)
#endif

#if (BOARD_FEATURE(HFXTAL) == ENABLED)
#   define BOARD_HFXT_PINS      (GPIO_Pin_2 | GPIO_Pin_3)
#   define BOARD_HFXT_CONF      (XT2DRIVE_3)
#else
#   define BOARD_HFXT_PINS      0
#   define BOARD_HFXT_CONF      (XT2OFF)
#endif


OT_INLINE_H void BOARD_PORT_STARTUP(void) {
/// Configure all ports to grounded outputs in order to minimize current
    GPIO12->DDIR    = 0xFFFF;
    GPIO34->DDIR    = 0xFFFF;
    GPIO56->DDIR    = 0xFFFF ^ (BOARD_LFXT_PINS | BOARD_HFXT_PINS);
    GPIO78->DDIR    = 0xFFFF;
    
#   if (defined(__DEBUG__))
    PJDIR = 0x00;
    PJOUT = 0x00;
#   else
    PJDIR = 0xFF;
    PJOUT = 0x00;
#   endif    
    
    GPIO12->DOUT    = 0x0000;
    GPIO34->DOUT    = 0x0000;
    GPIO56->DOUT    = 0x0000;
    GPIO78->DOUT    = 0x0000;
}

// LFXT1 Preconfiguration, using values local to the board design
// ALL MSP430F5 Boards MUST HAVE SOME VARIANT OF THIS
OT_INLINE_H void BOARD_XTAL_STARTUP(void) {
    // Turn on both crystals, XT2 for HF and XT1 for LF
    GPIO5->SEL |= BOARD_LFXT_PINS | BOARD_HFXT_PINS;
    UCS->CTL6   = BOARD_LFXT_CONF | BOARD_HFXT_CONF;
    
    // Loop until XT2, XT1, and DCO all stabilize
    do {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1&OFIFG);
  
    // Set LF and HF drive strength to minimum (depends on XTAL spec)
    UCS->CTL6  &= ~(XT2DRIVE_3 | XT1DRIVE_3);
}






/** Platform Memory Configuration <BR>
  * ========================================================================<BR>
  * OpenTag needs to know where it can put Nonvolatile memory (file system) and
  * how much space it can allocate for filesystem.  We put this into the bottom 
  * of Bank A.  Make sure you allocate the linker file appropriately.  You can
  * possibly get some performance improvement by putting code outside of Bank A.
  */
#define SRAM_START_ADDR             0x02400
#define EEPROM_START_ADDR           0
#define EEPROM_SIZE                 0
#define FLASH_START_PAGE            0
#define FLASH_PAGE_SIZE             512
#define FLASH_WORD_BYTES            2
#define FLASH_WORD_BITS             (FLASH_WORD_BYTES*8)

// Large Model: 128KB Flash, 8KB SRAM
#if defined(__LARGE_MEMORY__)
#   define SRAM_SIZE                (8*1024)
#   define FLASH_START_ADDR         0x04400
#   define FLASH_NUM_PAGES          256
#   define FLASH_FS_ALLOC           (512*24)     //allocating total of 24 blocks (12KB)
#   define FLASH_FS_ADDR            0x10000
#   define FLASH_FS_FALLOWS         6
#   define FLASH_PAGE_ADDR(VAL)     (FLASH_START_ADDR + ( (VAL) * FLASH_PAGE_SIZE) )

// Tiny Model (for testing mostly): 24KB Flash, 4KB SRAM
#elif defined (__TINY_MEMORY__)
#   define SRAM_SIZE                (4*1024)
#   define FLASH_START_ADDR         0xA000
#   define FLASH_NUM_PAGES          48
#   define FLASH_FS_ALLOC           (512*2)     //allocating total of 2 Blocks (1KB)
#   define FLASH_FS_ADDR            0xA000
#   define FLASH_FS_FALLOWS         0

// Standard Model: Small memory, 47KB Flash, 8KB SRAM
#else
#   define SRAM_SIZE                (8*1024)
#   define FLASH_START_ADDR         0x04400
#   define FLASH_NUM_PAGES          94
#   define FLASH_FS_ALLOC           (512*8)     //allocating total of 8 blocks (4KB)
#   define FLASH_FS_ADDR            0x04400
#   define FLASH_FS_FALLOWS         3
#endif

#define FLASH_PAGE_ADDR(VAL)        (FLASH_START_ADDR + ( (VAL) * FLASH_PAGE_SIZE) )


// MSP430 only ... Information Flash
// Info Blocks are not really used with OT.  You can use for your own purposes
#define INFO_START_ADDR         0x1800
#define INFO_START_PAGE         0
#define INFO_PAGE_SIZE          128
#define INFO_NUM_PAGES          4
#define INFO_PAGE_ADDR(VAL)     (INFO_START_ADDR + (INFO_PAGE_SIZE*VAL))
#define INFO_D_ADDR             INFO_PAGE_ADDR(0)
#define INFO_C_ADDR             INFO_PAGE_ADDR(1)
#define INFO_B_ADDR             INFO_PAGE_ADDR(2)
#define INFO_A_ADDR             INFO_PAGE_ADDR(3)


// MSP430 only ... Boot Strap Loader
// BSL is not presently used with OpenTag, although at some point it might be.
// In that case, we would probably reserve Bank A for the USB driver and the
// filesystem, and not overwrite Bank A.  The 2KB BSL space is not big enough
// for the USB driver, but it is big enough for UART Mpipe.
#define BSL_START_ADDR          0x1000
#define BSL_START_PAGE          0
#define BSL_PAGE_SIZE           512
#define BSL_NUM_PAGES           4
#define BSL_PAGE_ADDR(VAL)      (BSL_START_ADDR + (BSL_PAGE_SIZE*VAL))
#define BSL_D_ADDR              BSL_PAGE_ADDR(0)
#define BSL_C_ADDR              BSL_PAGE_ADDR(1)
#define BSL_B_ADDR              BSL_PAGE_ADDR(2)
#define BSL_A_ADDR              BSL_PAGE_ADDR(3)






/** Note: Clocking for the Board's MCU      <BR>
  * ========================================================================<BR>
  * The MSP430F5 can be clocked up to 25MHz.  OpenTag uses the ACLK at 1024 Hz
  * for GPTIM, so it's important to define either the internal 32768Hz osc or
  * an external LFXTAL (on XT1).  The MCLK and SMCLK can be generated by XT2,
  * a multiplied XT2, or a multiplied XT1.
  */
#define MCU_PARAM_LFXTALHz              BOARD_PARAM_LFHz
#define MCU_PARAM_LFXTALtol             BOARD_PARAM_LFtol
//#define MCU_PARAM_LFOSCHz               BOARD_PARAM_LFHz
//#define MCU_PARAM_LFOSCtol              BOARD_PARAM_LFtol
#define MCU_PARAM_XTALHz                BOARD_PARAM_HFHz
#define MCU_PARAM_XTALmult              BOARD_PARAM_HFmult
#define MCU_PARAM_XTALtol               BOARD_PARAM_HFtol
//#define MCU_PARAM_OSCHz                 BOARD_PARAM_HFHz
//#define MCU_PARAM_OSCmult               BOARD_PARAM_HFmult
//#define MCU_PARAM_OSCtol                BOARD_PARAM_HFtol


//Clock dividers from DCO
//This board uses a CC1101, whose SPI maxes-out at 6.5 MHz.  So make sure SPI
//(usually clocked by SMCLK) is less than 6.5 MHz.  Dividing by 4 will ensure
//this because max system clock is 25 MHz.
#define PLATFORM_MCLK_DIV           BOARD_PARAM_MCLKDIV
#define PLATFORM_SMCLK_DIV          BOARD_PARAM_SMCLKDIV

#define PLATFORM_LSCLOCK_PINS       BOARD_LFXT_PINS
#define PLATFORM_LSCLOCK_CONF       BOARD_LFXT_CONF
#define PLATFORM_HSCLOCK_PINS       BOARD_HFXT_PINS
#define PLATFORM_HSCLOCK_CONF       BOARD_HFXT_CONF
#define PLATFORM_LSCLOCK_HZ         BOARD_PARAM_LFHz
#define PLATFORM_LSCLOCK_ERROR      BOARD_PARAM_LFtol
#define PLATFORM_HSCLOCK_HZ         (BOARD_PARAM_HFHz*BOARD_PARAM_HFmult)
#define PLATFORM_HSCLOCK_ERROR      BOARD_PARAM_HFtol
#define PLATFORM_HSCLOCK_MULT       BOARD_PARAM_HFmult




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


// Trigger 1 = Green LED
// Trigger 2 = Yellow LED
#define OT_TRIG1_PORTNUM    8
#define OT_TRIG1_PORT       GPIO8
#define OT_TRIG1_PIN        GPIO_Pin_2  
#define OT_TRIG1_HIDRIVE    ENABLED         // Use high-current option
#define OT_TRIG2_PORTNUM    8
#define OT_TRIG2_PORT       GPIO8
#define OT_TRIG2_PIN        GPIO_Pin_1
#define OT_TRIG2_HIDRIVE    ENABLED


// Pin that can be used for ADC-based random number (usually floating pin)
// You could also put on a low voltage, reverse-biased zener on the board
// to produce a pile of noise.  2.1V seems like a good value.
#define OT_GWNADC_PORT      GPIO7
#define OT_GWNADC_PIN       GPIO_Pin_0
#define OT_GWNADC_BITS      1
//#define OT_GWNZENER_PORT    GPIO7
//#define OT_GWNZENER_PIN     GPIO_Pin_1
//#define OT_GWNZENER_HIDRIVE DISABLED






// Radio pin interface (CC1101).  
// - SPI1 is USCIB0, wired on the board as shown below
// - 2 GDOs are available as IRQs (GDO0, GDO2)
// - GDO1 is tied to the MISO pin, for asynchronous modulation (unused)
#define RADIO_SPINUM            1
#define RADIO_SPI               SPIB0
#define RADIO_SPI_HWCS          DISABLED
#define RADIO_SPI_VECTOR        USCI_B0_VECTOR

#define RADIO_SPICS_PORT        GPIO2
#define RADIO_SPICS_PIN         GPIO_Pin_6
#define RADIO_SPICS_HIGH()      (RADIO_SPICS_PORT->DOUT |= RADIO_SPICS_PIN)
#define RADIO_SPICS_LOW()       (RADIO_SPICS_PORT->DOUT &= ~RADIO_SPICS_PIN)

#define RADIO_SPISCK_PORT       GPIO3
#define RADIO_SPISCK_PIN        GPIO_Pin_2
#define RADIO_SPIMISO_PORT      GPIO3
#define RADIO_SPIMISO_PIN       GPIO_Pin_1
#define RADIO_SPIMOSI_PORT      GPIO3
#define RADIO_SPIMOSI_PIN       GPIO_Pin_0

OT_INLINE_H void BOARD_RADIO_SPI_PORTCONF() {
    //RADIO_SPICS_PORT->DDIR     |= RADIO_SPICS_PIN;                            //init-time default
    //RADIO_SPICS_PORT->DS       &= ~RADIO_SPICS_PIN;                           //power-on default
    //RADIO_SPICS_PORT->SEL      &= ~RADIO_SPICS_PIN;                           //power-on default
    //RADIO_SPIMISO_PORT->REN    &= ~RADIO_SPIMISO_PIN;                         //power-on default
    //RADIO_SPIMISO_PORT->DDIR   &= ~RADIO_SPIMISO_PIN;                         //power-on default
    //RADIO_SPISCK_PORT->DDIR    |= (RADIO_SPIMOSI_PIN | RADIO_SPISCK_PIN); 
    //RADIO_SPISCK_PORT->DS      &= ~(RADIO_SPIMOSI_PIN | RADIO_SPISCK_PIN);    //power-on default
    RADIO_SPISCK_PORT->SEL     |= (RADIO_SPIMOSI_PIN | RADIO_SPIMISO_PIN | RADIO_SPISCK_PIN);
}


    
#define RADIO_SPI_CLKSRC    (PLATFORM_HSCLOCK_HZ / PLATFORM_SMCLK_DIV)
#if (RADIO_SPI_CLKSRC > 10000)
#	warn "Radio clock might be too fast. MLX73 can do only 10 MHz"
#endif

// DMA is generally deprecated with MSP430, because it is so limited, it is
// better deployed for other purposes, and because most radios have their own
// FIFO's these days.
//#define RADIO_DMA_INDEX     
//#define RADIO_DMA_RXINDEX   
//#define RADIO_DMA_TXINDEX           
//#define RADIO_DMA_RXCHAN    
//#define RADIO_DMA_RXVECTOR  
//#define RADIO_DMA_RXINT  	
//#define RADIO_DMA_TXCHAN    
//#define RADIO_DMA_TXVECTOR  
//#define RADIO_DMA_TXINT  	

#define RADIO_IRQ_PORT      GPIO2 
#define RADIO_IRQ_SRC()     GPIO2->P2IV
#define RADIO_IRQ_VECTOR    PORT2_VECTOR
#define RADIO_IRQ0_SRCLINE  0
#define RADIO_IRQ0_PIN      (1 << RADIO_IRQ0_SRCLINE)
#define RADIO_IRQ1_SRCLINE  1
#define RADIO_IRQ1_PIN      (1 << RADIO_IRQ2_SRCLINE)
#define RADIO_IRQ2_SRCLINE  3
#define RADIO_IRQ2_PIN      (1 << RADIO_IRQ0_SRCLINE)
#define RADIO_IRQ3_SRCLINE  4
#define RADIO_IRQ3_PIN      (1 << RADIO_IRQ2_SRCLINE)









// If using the USB for MPipe, it must be on the specialized PORT U.  There is
// no other mapping option.
#if (MCU_FEATURE_MPIPEVCOM == ENABLED)
#   define MPIPE_USBNUM         0
#   define MPIPE_USBDP_PIN      GPIO_Pin_0
#   define MPIPE_USBDM_PIN      GPIO_Pin_1
#   define MPIPE_USB_VECTOR     USB_UBM_VECTOR

#   define MPIPE_USB_REMWAKE    0x00        // Set to 0x20 to enable remote wakeup (not generally supported by MPipe)
#   define MPIPE_USB_POWERING   0x80        // Set to 0x40 to enable self-powering, or 0x80 for bus-powering
#   define MPIPE_USB_MAXPOWER   100         // Max mA that can be sourced from the bus (up to 500)
#   define MPIPE_USB_XTSUSPEND  0           // Set to non-zero to disable USB XTAL when usb is suspended
#   define MPIPE_USB_XTAL       2           // Almost always XTAL2 (2)
#   define MPIPE_USB_XTFREQ     USBPLL_SETCLK_4_0

#endif



// If using the normal UART, it is wired to {rx,tx} = {4.4,4.5}.  CTS/RTS could
// hypothetically be implemented on 4.6/4.7, which are unused.  All other USCIs
// on the EXP430F5529 board are utilized by other features.
#if (MCU_FEATURE_MPIPEVCOM != ENABLED)
#   define MPIPE_UARTNUM        2
#   define MPIPE_UART_PORTNUM   4
#   define MPIPE_UART_PORT      GPIO4
#   define MPIPE_UART_PORTMAP   NULL //P4M
#   define MPIPE_UART_RXPIN     GPIO_Pin_5
#   define MPIPE_UART_TXPIN     GPIO_Pin_4
#   define MPIPE_UART_PINS      (MPIPE_UART_RXPIN | MPIPE_UART_TXPIN)
//#   define MPIPE_RTS_PORT       GPIO2
//#   define MPIPE_CTS_PORT       GPIO2
//#   define MPIPE_RTS_PIN        GPIO_Pin_6
//#   define MPIPE_CTS_PIN        GPIO_Pin_7

#   if (MPIPE_UARTNUM == 2)
#       define MPIPE_UART           UARTA1
#       define MPIPE_UART_RXSIG     PM_UCA1RXD
#       define MPIPE_UART_TXSIG     PM_UCA1TXD
#	    define MPIPE_UART_RXTRIG    20 //DMA_Trigger_UCA1RXIFG
#	    define MPIPE_UART_TXTRIG    21 //DMA_Trigger_UCA1TXIFG
#       define MPIPE_UART_VECTOR    USCI_A1_VECTOR
#   else
#       error "MPIPE_UART is not defined to an available index (2, i.e. USCIA1)"
#   endif

#   if (MCU_FEATURE_MPIPEDMA == ENABLED)
#       define MPIPE_DMANUM    2
#       if (MPIPE_DMANUM == 0)
#           define MPIPE_DMA     DMA0
#       elif (MPIPE_DMANUM == 1)
#           define MPIPE_DMA     DMA1
#       elif (MPIPE_DMANUM == 2)
#           define MPIPE_DMA     DMA2
#       else
#           error "MPIPE_DMANUM is not defined to an available index (0-2)"
#       endif
#   endif

#   if (MPIPE_UART_RXPIN == GPIO_Pin_5)
#       define MPIPE_UART_RXMAP    MPIPE_UART_PORTMAP->MAP5
#   else
#       error "MPIPE_UART_RXPIN out of bounds"
#   endif

#   if (MPIPE_UART_TXPIN == GPIO_Pin_4)
#       define MPIPE_UART_TXMAP    MPIPE_UART_PORTMAP->MAP4
#   else
#       error "MPIPE_UART_TXPIN out of bounds"
#   endif

#endif


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







/** PaLFi Master with TMS3705 interface<BR>
  * ========================================================================<BR>
  * Due to the schematic of the MSP430F5-EXP5529, the connection to a PalFi
  * module must use the setup listed below.  There is not any other way, unless
  * the driver is modified to work differently.
  */
#define PALFI_TIM           TIM0B7
#define PALFI_UART          UARTA1

#define PALFI_Px            P4M
#define PALFI_UARTRX_MAP    PM_UCA1RXD
#define PALFI_UARTTX_MAP    PM_UCA1TXD
#define PALFI_TIMOC_MAP     PM_TB0CCR1A

#define PALFI_PORT          GPIO4
#define PALFI_TXCT_PIN      GPIO_Pin_5
#define PALFI_SCIO_PIN      GPIO_Pin_4
#define PALFI_PINS          (PALFI_TXCT_PIN | PALFI_SCIO_PIN)

#define PALFI_TIMER_VECTOR  TIMER0_B0_VECTOR
#define PALFI_UART_VECTOR   UCA1_VECTOR











/******* ALL SHIT BELOW HERE IS SUBJECT TO REDEFINITION **********/


/** Flash Memory Setup: 
  * "OTF" means "Open Tag Flash," but if flash is not used, it just means 
  * storage memory.  Unfortunately this does not begin with F.
  */
#define OTF_VWORM_PAGES         (FLASH_FS_ALLOC/FLASH_PAGE_SIZE)
#define OTF_VWORM_FALLOW_PAGES  FLASH_FS_FALLOWS
#define OTF_VWORM_PAGESIZE      FLASH_PAGE_SIZE
#define OTF_VWORM_WORD_BYTES    FLASH_WORD_BYTES
#define OTF_VWORM_WORD_BITS     FLASH_WORD_BITS
#define OTF_VWORM_SIZE          (OTF_VWORM_PAGES * OTF_VWORM_PAGESIZE)
#define OTF_VWORM_START_PAGE    ((FLASH_FS_ADDR-FLASH_START_ADDR)/FLASH_PAGE_SIZE)
#define OTF_VWORM_START_ADDR    FLASH_FS_ADDR

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
