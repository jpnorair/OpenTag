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
  * @file       /board/cc430/board_EM430RF.h
  * @author     JP Norair
  * @version    R100
  * @date       31 October 2012
  * @brief      Board Configuration for Classic TI/Amber EM430RF (CC430)
  * @ingroup    Platform
  *
  * Do not include this file, include OT_platform.h
  ******************************************************************************
  */
  

#ifndef __board_EM430RF_H
#define __board_EM430RF_H

#include "build_config.h"
#include "platform_CC430.h"

#ifdef __NULL_RADIO__
#   include "radio_null.h"
#else
#   include "radio_CC430.h"
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




/** Additional RF Front End Parameters and Settings <BR>
  * ========================================================================<BR>
  * The TI EM430 kits so far must be re-configured to 433 MHz, or set to 866.
  */
#define RF_PARAM_BAND   866
#define RF_HDB_ATTEN    6       //Half dB attenuation (units = 0.5dB), used to scale TX power
#define RF_RSSI_OFFSET  3       //Offset applied to RSSI calculation



/** MCU Feature settings      <BR>
  * ========================================================================<BR>
  * Implemented capabilities of the CC430F5137/6137 variants.
  *
  * @note On DMA: The MSP430 and CC430 do not have a sophisticated DMA, but it
  * is still good to use if you can.  However, it is very important to make sure
  * the DMA channels are assigned correctly.  Memcpy MUST have the highest
  * priority (0).  MPipe can have the second highest priority.  If you are
  * using the 3rd channel in your app, be aware that it can cause memory
  * glitching if you don't use it properly.
  */
#define MCU_FEATURE(VAL)                MCU_FEATURE_##VAL       // FEATURE                  AVAILABILITY
#define MCU_FEATURE_CRC                 ENABLED                 // CCITT CRC16              Low
#define MCU_FEATURE_AES128              ENABLED                 // AES128 engine            Moderate
#define MCU_FEATURE_ECC                 DISABLED                // ECC engine               Low
#define MCU_FEATURE_RADIODMA            DISABLED
#define MCU_FEATURE_RADIODMA_TXBYTES    0
#define MCU_FEATURE_RADIODMA_RXBYTES    0
#define MCU_FEATURE_MAPEEPROM           DISABLED
#define MCU_FEATURE_MPIPEDMA            ENABLED         // MPIPE typically requires DMA on this platform
#define MCU_FEATURE_MEMCPYDMA           ENABLED         // MEMCPY DMA should be lower priority than MPIPE DMA

#define MCU_PARAM(VAL)                  MCU_PARAM_##VAL
#define MCU_PARAM_POINTERSIZE           2

#define PLATFORM_FEATURE_USBCONVERTER    ENABLED

OT_INLINE_H BOARD_DMA_COMMON_INIT() {
    DMA->CTL4 = (   DMA_Options_RMWDisable | \
                    DMA_Options_RoundRobinDisable | \
                    DMA_Options_ENMIEnable  );
}






/** Board-based Feature Settings <BR>
  * ========================================================================<BR>
  * Notes apart from the obvious:  
  *
  * 1. There is a general purpose button attached to P1.7
  * 
  * 2. The 2 LEDs (TRIGS 1-2) are normal-biased.
  */
#define BOARD_FEATURE(VAL)              BOARD_FEATURE_##VAL
#define BOARD_FEATURE_USBCONVERTER      ENABLED                 // Is UART connected via USB converter?
#define BOARD_FEATURE_MPIPE_QMGMT       ENABLED
#define BOARD_FEATURE_LFXTAL            ENABLED                 // LF XTAL used as Clock source
#define BOARD_FEATURE_HFXTAL            DISABLED                // HF XTAL used as Clock source
#define BOARD_FEATURE_INVERT_TRIG1      DISABLED
#define BOARD_FEATURE_INVERT_TRIG2      DISABLED

#define BOARD_SW2_PORT                  GPIO1
#define BOARD_SW2_PIN                   GPIO_Pin_7
#define BOARD_SW2_POLARITY              0

#define BOARD_PARAM(VAL)                BOARD_PARAM_##VAL
#define BOARD_PARAM_LFHz                32768
#define BOARD_PARAM_LFtol               0.00002
#define BOARD_PARAM_HFHz                32768
#define BOARD_PARAM_HFmult              610                     // DCO = HFHz * HFmult
#define BOARD_PARAM_HFtol               BOARD_PARAM_LFtol
#define BOARD_PARAM_MCLKDIV             1                       // Master Clock = DCO / MCLKDIV
#define BOARD_PARAM_SMCLKDIV            8                       // Submaster Clock = DCO / SMCLKDIV (~2.5 MHz)

#if (BOARD_FEATURE(LFXTAL) == ENABLED)
#   define BOARD_LFXT_PINS      (GPIO_Pin_0 | GPIO_Pin_1)
#   define BOARD_LFXT_CONF      (UCS_CTL6_XTS_low | UCS_CTL6_XT1BYPASS_off | UCS_CTL6_XCAP_2 )
#else
#   define BOARD_LFXT_PINS      0
#   define BOARD_LFXT_CONF      (XT1OFF)
#endif

#if (BOARD_FEATURE(HFXTAL) == ENABLED)
#   define BOARD_HFXT_PINS      (GPIO_Pin_2 | GPIO_Pin_3)
#   define BOARD_HFXT_CONF      (XT2DRIVE_3)
#else
#   define BOARD_HFXT_PINS      0
#   define BOARD_HFXT_CONF      (UCS_CTL6_XT2OFF | 0xC0 )
#endif



OT_INLINE_H void BOARD_PORT_STARTUP(void) {
/// Configure all ports to grounded outputs in order to minimize current
#   if (defined(DEBUG_ON) || defined(__DEBUG__))
#   else
    PJDIR = 0xFF;
    PJOUT = 0x00;
#   endif 
    
    GPIO12->DDIR    = 0xFFFF;
    GPIO34->DDIR    = 0xFFFF;
    GPIO56->DDIR    = 0xFFFF;
    
    GPIO12->DOUT    = 0x0000;
    GPIO34->DOUT    = 0x0000;
    GPIO56->DOUT    = 0x0000;
}


OT_INLINE_H void BOARD_POWER_STARTUP(void) {
///@note On SVSM Config Flags: (1) It is advised in all cases to include
    ///      SVSM_EventDelay.  (2) If using line-power (not battery), it is
    ///      advised to enable FullPerformance and ActiveDuringLPM.  (3) Change
    ///      The SVSM_Voffon_ parameter to one that matches your requirements.
    ///      I recommend putting it as high as you can, to give the most time
    ///      for the power-down routine to work.
    PMM_SetVCore(PMM_Vcore_22);
    PMM_SetStdSVSM( (SVM_Enable | SVSM_AutoControl | SVSM_EventDelay),
                    SVS_Von_20, SVSM_Voffon_235);
}



// LFXT1 Preconfiguration, using values local to the board design
// ALL CC430 Boards MUST HAVE SOME VARIANT OF THIS
OT_INLINE_H void BOARD_XTAL_STARTUP(void) {
    // Turn on XT1 as LF crystal.  Let RF core manage 26 MHz XT2
    GPIO5->SEL |= BOARD_LFXT_PINS | BOARD_HFXT_PINS;
    UCS->CTL6   = BOARD_LFXT_CONF | BOARD_HFXT_CONF;
    
    // Loop until XT2, XT1, and DCO all stabilize
    do {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1&OFIFG);
  
    // Set LF and HF drive strength to minimum (depends on XTAL spec)
    UCS->CTL6  &= ~(XT1DRIVE_3);
}




/** Platform Memory Configuration <BR>
  * ========================================================================<BR>
  * OpenTag needs to know where it can put Nonvolatile memory (file system) and
  * how much space it can allocate for filesystem.
  */
#define SRAM_START_ADDR         0x1C00
#define SRAM_SIZE               (4*1024)
#define EEPROM_START_ADDR       0
#define EEPROM_SIZE             0
#define FLASH_START_ADDR        0x8000
#define FLASH_START_PAGE        0
#define FLASH_PAGE_SIZE         512
#define FLASH_WORD_BYTES        2
#define FLASH_WORD_BITS         (FLASH_WORD_BYTES*8)
#ifdef __LARGE_MEMORY__
#   warn "No current CC430 part has large-memory model"
#else
#   define FLASH_NUM_PAGES      64
#   define FLASH_FS_ALLOC       (512*8)     //allocating total of 8 blocks (4KB)
#   define FLASH_FS_ADDR        0x8000
#   define FLASH_FS_FALLOWS     3
#endif
#define FLASH_PAGE_ADDR(VAL)    (FLASH_START_ADDR + ( (VAL) * FLASH_PAGE_SIZE) )



// MSP430 only ... Information Flash
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
  * The CC430 MCU can be clocked up to 20MHz.  OpenTag uses the ACLK at 1024 Hz
  * for GPTIM, so it's important to define either the internal 32768Hz osc or
  * an external LFXTAL (on XT1).  The MCLK and SMCLK can be generated by XT2,
  * a multiplied XT2, or a multiplied XT1.
  */
#define MCU_PARAM_LFXTALHz              BOARD_PARAM_LFHz
#define MCU_PARAM_LFXTALtol             BOARD_PARAM_LFtol
//#define MCU_PARAM_LFOSCHz               BOARD_PARAM_LFHz
//#define MCU_PARAM_LFOSCtol              BOARD_PARAM_LFtol
//#define MCU_PARAM_XTALHz                BOARD_PARAM_HFHz
//#define MCU_PARAM_XTALmult              BOARD_PARAM_HFmult
//#define MCU_PARAM_XTALtol               BOARD_PARAM_HFtol
#define MCU_PARAM_OSCHz                 (BOARD_PARAM_HFHz*BOARD_PARAM_HFmult)
#define MCU_PARAM_OSCmult               BOARD_PARAM_HFmult
#define MCU_PARAM_OSCtol                BOARD_PARAM_HFtol

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
  * <LI> OT_GPTIM:  General Purpose Timer used by OpenTag kernel and some other
  *                   internal stuff (like Radio MAC timer). </LI>
  * <LI> OT_TRIG:   Optional test trigger(s) usable in OpenTag apps.  Often the 
  *                   triggers are implemented on LED pins </LI>   
  * <LI> MPIPE:     The wireline interface, which on CC430 is often a UART </LI>
  */
#define OT_GPTIM            TIM1A3
#define OT_GPTIM_ISR_ID     __ISR_T1A1_ID
#ifdef __ISR_T1A1
#   error "ISR T1A1 is already allocated.  It must be used for GPTIM."
#else
#   define __ISR_T1A1
#endif
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

#define OT_GPTIM_ERRDIV     32768 //this needs to be hard-coded, or else CCS has problems







/** Basic GPIO Setup <BR>
  * ========================================================================<BR>
  * <LI> MPipe is a UART-based serial interface.  It needs to be specified with
  *      a UART peripheral number and RX/TX port & pin configurations. 
  * </LI>
  * <LI> OT Triggers are test outputs, usually LEDs.  They should be configured
  *      to ports & pins where there are LED or other trigger connections.
  * </LI>
  * <LI> The GWN feature is part of a true-random-number generator.  It is
  *      optional.  It needs a port & pin for the ADC input (GWNADC) and also,
  *      optionally, one for a Zener driving output port & pin.  On the CC430,
  *      the ADC can be hacked to produce the benefits of a Zener without 
  *      actually having one, so don't worry about the Zener. 
  * </LI>
  */
#define MPIPE_DMANUM        0
#define MPIPE_UART_PORTNUM  1
#define MPIPE_UART_PORT     GPIO1
#define MPIPE_UART_PORTMAP  P1M
#define MPIPE_UART_RXPIN    GPIO_Pin_5
#define MPIPE_UART_TXPIN    GPIO_Pin_6

#define OT_TRIG1_PORT       GPIO1
#define OT_TRIG1_PIN        GPIO_Pin_0
#define OT_TRIG1_HIDRIVE    ENABLED
#define OT_TRIG1_PORT       GPIO3
#define OT_TRIG2_PIN        GPIO_Pin_6
#define OT_TRIG2_HIDRIVE    ENABLED


// Pin that can be used for ADC-based random number (usually floating pin)
// You could also put on a low voltage, reverse-biased zener on the board
// to produce a pile of noise.  2.1V seems like a good value.
#define OT_GWNADC_PORTNUM   2
#define OT_GWNADC_PINNUM    1
#define OT_GWNADC_BITS      8
//#define OT_GWNZENER_PORT    GPIO2
//#define OT_GWNZENER_PIN     GPIO_Pin_2
//#define OT_GWNZENER_HIDRIVE DISABLED

#ifdef OT_GWNADC_PORTNUM
#   if (OT_GWNADC_PORTNUM == 2)
#       define OT_GWNADC_PORT      GPIO2
#   else
#       error "GWNADC (White Noise ADC Input Pin) must be on port 2"
#   endif
#endif


#define MPIPE_UART_PINS     (MPIPE_UART_RXPIN | MPIPE_UART_TXPIN)
#define MPIPE_UART_ID       0xA0
#define MPIPE_UART          UARTA0
#define MPIPE_UART_RXSIG    PM_UCA0RXD
#define MPIPE_UART_TXSIG    PM_UCA0TXD
#define MPIPE_UART_RXTRIG   DMA_Trigger_UCA0RXIFG
#define MPIPE_UART_TXTRIG   DMA_Trigger_UCA0TXIFG

#if (MCU_FEATURE_MPIPEDMA == ENABLED)
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
#   define MEMCPY_DMANUM      (MPIPE_DMANUM + (MCU_FEATURE_MPIPEDMA == ENABLED))
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
