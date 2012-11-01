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
/** @file       /otplatform/msp430_mculib/msp430f5/msp430f5_map.h
  * @author     JP Norair
  * @version    R100
  * @date       1 Nov 2012
  * @brief      Resource & Peripheral Mappings for MSP430F5
  * @ingroup    MSP430F5 Library
  *
  ******************************************************************************
  */


#ifndef __MSP430F5_MAP_H
#define __MSP430F5_MAP_H


#include "msp430f5_conf.h"
#include "msp430f5_types.h"




// The peripheral register typedefs are in the order the peripherals are 
// presented in the MSP430F5 Users' Guide



/*------------------------ SFR ---------------------------------------------*/
typedef struct {
    vu16 IE;
    vu16 IFG;
    vu16 RPRCR;
} SFR_Type;

typedef struct {
    vu8 IE1;
    vu8 IE2;
    vu8 IFG1;
    vu8 IFG2;
    vu8 RPRCR_L;
    vu8 RPRCR_H;
} SFRb_Type;



/*------------------------ PMM -----------------------------------------------*/
typedef struct {
    vu16 CTL0;
    vu16 CTL1;
    vu16 SVSMHCTL;
    vu16 SVSMLCTL;
    vu16 reserved08;
    vu16 reserved0A;
    vu16 IFG;
    vu16 IE;
    vu16 PM5CTL0;
} PMM_Type;

typedef struct {
    vu8 CTL0_L;
    vu8 CTL0_H;
    vu8 CTL1_L;
    vu8 CTL1_H;
    vu8 SVSMHCTL_L;
    vu8 SVSMHCTL_H;
    vu8 SVSMLCTL_L;
    vu8 SVSMLCTL_H;
    vu8 Reserved08;
    vu8 Reserved09;
    vu8 Reserved0A;
    vu8 Reserved0B;
    vu8 IFG_L;
    vu8 IFG_H;
    vu8 IE_L;
    vu8 IE_H;
    vu8 PM5CTL0_L;
    vu8 PM5CTL0_H;
} PMMb_Type;




/*------------------------ FLASH ---------------------------------------------*/
typedef struct {
    vu16 CTL1;
    vu16 Reserved02;
    vu16 CTL3;
    vu16 CTL4;
} FLASH_Type;

typedef struct {
    vu8 CTL1_L;
    vu8 CTL1_H;
    vu8 Reserved02_L;
    vu8 Reserved02_H;
    vu8 CTL3_L;
    vu8 CTL3_H;
    vu8 CTL4_L;
    vu8 CTL4_H;
} FLASHb_Type;




/*------------------------ CRC -----------------------------------------------*/
typedef struct {
    vu16 DI;
    vu16 DIRB;
    vu16 INIRES;
    vu16 RESR;
} CRC_Type;

typedef struct {
    vu8 DI_L;
    vu8 DI_H;
    vu8 DIRB_L;
    vu8 DIRB_H;
    vu8 INIRES_L;
    vu8 INIRES_H;
    vu8 RESR_L;
    vu8 RESR_H;
} CRCb_Type;




/*------------------------ RAM Controller ------------------------------------*/
typedef struct {
    vu16 CTL0;
} RAM_Type;





/*------------------------ WDT_A ---------------------------------------------*/
typedef struct {
    vu16 CTL;
} WDT_Type;

typedef struct {
    vu8 CTL_L;
    vu8 CTL_H;
} WDTb_Type;





/*------------------------ UCS -----------------------------------------------*/
typedef struct {
    vu16 CTL0;
    vu16 CTL1;
    vu16 CTL2;
    vu16 CTL3;
    vu16 CTL4;
    vu16 CTL5;
    vu16 CTL6;
    vu16 CTL7;
    vu16 CTL8;
} UCS_Type;

typedef struct {
    vu8 CTL0_L;
    vu8 CTL0_H;
    vu8 CTL1_L;
    vu8 CTL1_H;
    vu8 CTL2_L;
    vu8 CTL2_H;
    vu8 CTL3_L;
    vu8 CTL3_H;
    vu8 CTL4_L;
    vu8 CTL4_H;
    vu8 CTL5_L;
    vu8 CTL5_H;
    vu8 CTL6_L;
    vu8 CTL6_H;
    vu8 CTL7_L;
    vu8 CTL7_H;
    vu8 CTL8_L;
    vu8 CTL8_H;
} UCSb_Type;






/*------------------------ SYS ---------------------------------------------*/
typedef struct {
    vu16 CTL;
    vu16 BSLC;
    vu16 Reserved04;  
    vu16 JMBC;  
    vu16 JMBI0;  
    vu16 JMBI1;
    vu16 JMBO0;
    vu16 JMBO1;
    vu16 Reserved10;
    vu16 Reserved12;
    vu16 Reserved14;
    vu16 Reserved16;
    vu16 BERRIV;
    vu16 UNIV;
    vu16 SNIV;
    vu16 RSTIV;
} SYS_Type;






/*------------------------ REF -----------------------------------------------*/
typedef struct {
    vu16 CTL0;
} REF_Type;






/*------------------------ PMAP ----------------------------------------------*/
typedef struct {
    vu16 PWD;
    vu16 CTL;
} PM_Type;

typedef struct {
    vu8  MAP0;
    vu8  MAP1;
    vu8  MAP2;
    vu8  MAP3;
    vu8  MAP4;
    vu8  MAP5;
    vu8  MAP6;
    vu8  MAP7;
} PXM_Type;





/*------------------------ GPIO and PortMapping ------------------------------*/
typedef struct {
    vu16 DIN;
    vu16 DOUT;
    vu16 DDIR;
    vu16 REN;
    vu16 DS;
    vu16 SEL;
    vu16 Reserved0C;
    vu16 P1IV;
    vu16 Reserved10;
    vu16 Reserved12;
    vu16 Reserved14;
    vu16 Reserved16;
    vu16 IES;
    vu16 IE;
    vu16 IFG;
    vu16 P2IV;
} GPIO_WordAccess;

typedef struct {
    vu8  DIN;
    vu8  Reserved01;
    vu8  DOUT;
    vu8  Reserved03;
    vu8  DDIR;
    vu8  Reserved05;
    vu8  REN;
    vu8  Reserved07;
    vu8  DS;
    vu8  Reserved09;
    vu8  SEL;
    vu8  Reserved0B;
    vu16 Reserved0C;
    vu16 P1IV;
    vu16 Reserved10;
    vu16 Reserved12;
    vu16 Reserved14;
    vu16 Reserved16;
    vu8  IES;
    vu8  Reserved19;
    vu8  IE;
    vu8  Reserved1B;
    vu8  IFG;
    vu8  Reserved1D;
    vu16 P2IV;
} GPIO_EvenAccess;

typedef struct {
    vu8  Reserved00;
    vu8  DIN;
    vu8  Reserved02;
    vu8  DOUT;
    vu8  Reserved04;
    vu8  DDIR;
    vu8  Reserved06;
    vu8  REN;
    vu8  Reserved08;
    vu8  DS;
    vu8  Reserved0A;
    vu8  SEL;
    vu16 Reserved0C;
    vu16 P1IV;
    vu16 Reserved10;
    vu16 Reserved12;
    vu16 Reserved14;
    vu16 Reserved16;
    vu8  Reserved18;
    vu8  IES;
    vu8  Reserved1A;
    vu8  IE;
    vu8  Reserved1C;
    vu8  IFG;
    vu16 P2IV;
} GPIO_OddAccess;




/*------------------------ TIMA -----------------------------------------------*/
typedef struct {
    vu16 CTL;
    vu16 CCTL0;
    vu16 CCTL1;  
    vu16 CCTL2;  
    vu16 CCTL3;  
    vu16 CCTL4;
    vu16 CCTL5;
    vu16 CCTL6;
    vu16 R;
    vu16 CCR0;
    vu16 CCR1;
    vu16 CCR2;
    vu16 CCR3;
    vu16 CCR4;
    vu16 CCR5;
    vu16 CCR6;
    vu16 EX0;
    vu16 Reserved22;
    vu16 Reserved24;
    vu16 Reserved26;
    vu16 Reserved28;
    vu16 Reserved2A;
    vu16 Reserved2C;
    vu16 IV;
} TIMA_Type;




/*------------------------ TIMB -----------------------------------------------*/
typedef struct {
    vu16 CTL;
    vu16 CCTL0;
    vu16 CCTL1;  
    vu16 CCTL2;  
    vu16 CCTL3;  
    vu16 CCTL4;
    vu16 CCTL5;
    vu16 CCTL6;
    vu16 R;
    vu16 CCR0;
    vu16 CCR1;
    vu16 CCR2;
    vu16 CCR3;
    vu16 CCR4;
    vu16 CCR5;
    vu16 CCR6;
    vu16 EX0;
    vu16 Reserved22;
    vu16 Reserved24;
    vu16 Reserved26;
    vu16 Reserved28;
    vu16 Reserved2A;
    vu16 Reserved2C;
    vu16 IV;
} TIMB_Type;




/*------------------------ RTC -----------------------------------------------*/
typedef struct {
    vu16 CTL01;
    vu16 CTL23;
    vu16 PS0CTL;
    vu16 Reserved04;
    vu16 Reserved06;
    vu16 PS1CTL;
    vu16 PS;
    vu16 IV;
    vu16 TIM0;
    vu16 TIM1;
    vu16 DATE;
    vu16 YEAR;
    vu16 AMINHR;
    vu16 ADOWDAY;
} RTC_Type;





/*------------------------ DMA -----------------------------------------------*/
typedef struct {
    vu16 CTL;
    vu16 SA_L;
    vu16 SA_H;
    vu16 DA_L;
    vu16 DA_H;
    vu16 SZ;
} DMAx_Type;

typedef struct {
    vu16 CTL0;
    vu16 CTL1;
    vu16 CTL2;
    vu16 CTL3;
    vu16 CTL4;
    vu16 Reserved0A;
    vu16 Reserved0C;
    vu16 IV;
} DMA_Type;



/*------------------------ USCI ----------------------------------------------*/
typedef struct {
    vu8 CTL1;
    vu8 CTL0;
    vu16 Reserved02;
    vu16 Reserved04;
    //vu16 BRW;
    vu8  BR0;
    vu8  BR1;
    vu8  MCTL;
    vu8  Reserved09;
    vu8  STAT;
    vu8  Reserved0B;
    vu8  RXBUF;
    vu8  Reserved0D;
    vu8  TXBUF;
    vu8  Reserved0F;
    vu8  ABCTL;
    vu8  Reserved11;
    vu8 IRTCTL;
    vu8 IRRCTL;
    vu16 Reserved14;
    vu16 Reserved16;
    vu16 Reserved18;
    vu16 Reserved1A;
    vu8 IE;
    vu8 IFG;
    vu16 IV;
} UART_Type;

typedef struct {
    vu8 CTL1;
    vu8 CTL0;
    vu16 Reserved02;
    vu16 Reserved04;
    vu16 BRW;
    vu8  MCTL;
    vu8  Reserved09;
    vu8  STAT;
    vu8  Reserved0B;
    vu8  RXBUF;
    vu8  Reserved0D;
    vu8  TXBUF;
    vu8  Reserved0F;
    vu16 Reserved10;
    vu16 Reserved12;
    vu16 Reserved14;
    vu16 Reserved16;
    vu16 Reserved18;
    vu16 Reserved1A;
    vu8  IE;
    vu8  IFG;
    vu16 IV;
} SPI_Type;

typedef struct {
    vu16 CTLW0;
    vu16 Reserved02;
    vu16 Reserved04;
    vu16 BRW;
    vu16 Reserved08;
    vu8  STAT;
    vu8  Reserved0B;
    vu8  RXBUF;
    vu8  Reserved0D;
    vu8  TXBUF;
    vu8  Reserved0F;
    vu16 I2COA;
    vu16 I2CSA;
    vu16 Reserved14;
    vu16 Reserved16;
    vu16 Reserved18;
    vu16 Reserved1A;
    vu16 ICTL;
    vu16 IV;
} I2C_Type;





/*------------------------ ADC12 ---------------------------------------------*/
typedef struct {
    vu16 CTL0;
    vu16 CTL1;
    vu16 CTL2;
    vu16 Reserved06;
    vu16 Reserved08;
    vu16 IFG;
    vu16 IE;
    vu16 IV;
    vu8  MCTL0;
    vu8  MCTL1;
    vu8  MCTL2;
    vu8  MCTL3;
    vu8  MCTL4;
    vu8  MCTL5;
    vu8  MCTL6;
    vu8  MCTL7;
    vu8  MCTL8;
    vu8  MCTL9;
    vu8  MCTL10;
    vu8  MCTL11;
    vu8  MCTL12;
    vu8  MCTL13;
    vu8  MCTL14;
    vu8  MCTL15;
    vu16 MEM0;
    vu16 MEM1;
    vu16 MEM2;
    vu16 MEM3;
    vu16 MEM4;
    vu16 MEM5;
    vu16 MEM6;
    vu16 MEM7;
    vu16 MEM8;
    vu16 MEM9;
    vu16 MEM10;
    vu16 MEM11;
    vu16 MEM12;
    vu16 MEM13;
    vu16 MEM14;
    vu16 MEM15;
} ADC12_Type;






/*------------------------ ADC10 ---------------------------------------------*/
typedef struct {
    vu16 CTL0;
    vu16 CTL1;
    vu16 CTL2;
    vu16 LO;
    vu16 HI;
    vu8  MCTL0;
    vu8  reserved0B;
    vu16 reserved0C;
    vu16 reserved0E;
    vu16 reserved10;
    vu16 MEM0;
    vu16 reserved14;
    vu16 reserved16;
    vu16 reserved18;
    vu16 IE;
    vu16 IFG;
    vu16 IV;
} ADC10_Type;





/*------------------------ Comparator B --------------------------------------*/
typedef struct {
    vu16 CTL0;
    vu16 CTL1;
    vu16 CTL2;
    vu16 CTL3;
    vu16 Reserved08;
    vu16 Reserved0A;
    vu16 INT;
    vu16 IV;
} CB_Type;




/*------------------------ USB Device --------------------------------------*/
typedef struct {
    vu16 KEYID;
    vu16 CNF;
    vu16 PHYCTL;
    vu16 reserved06;
    vu16 PWRCTL;
    vu16 reserved0A;
    vu16 reserved0C;
    vu16 reserved0E;
    vu16 PLLCTL;
    vu16 PLLDIV;
    vu16 PLLIR;
} USBconf_Type;

typedef struct {
    vu8  IEPCNF_0;
    vu8  IEPCNT_0;
    vu8  OEPCNF_0;
    vu8  OEPCNT_0;
    vu16 reserved04;
    vu16 reserved06;
    vu16 reserved08;
    vu16 reserved0A;
    vu16 reserved0C;
    vu8  IEPIE;
    vu8  OEPIE;
    vu8  IEPIFG;
    vu8  OEPIFG;
    vu16 IV;
    vu16 reserved14;
    vu16 MAINT;
    vu16 TSREG;
    vu16 USBFN;
    vu8  USBCTL;
    vu8  USBIE;
    vu8  USBIFG;
    vu8  FUNADR;
} USB_Type;













/******************************************************************************/
/*                         Peripheral memory map                              */
/******************************************************************************/

#define PERIPH_BASE             (0x0000)
#define SRAM_BASE               (0x1C00)


#define SFR_BASE                (0x0100)
#define PMM_BASE                (0x0120)
#define FLASH_BASE              (0x0140)
#define CRC_BASE                (0x0150)
#define RAM_BASE                (0x0158)
#define WDTA_BASE               (0x015C)
#define UCS_BASE                (0x0160)
#define SYS_BASE                (0x0180)
#define REF_BASE                (0x01B0)

#define PM_BASE                 (0x01C0)
#define P1M_BASE                (0x01C8)
#define P2M_BASE                (0x01D0)
#define P3M_BASE                (0x01D8)
#define P4M_BASE                (0x01E0)
#define GPIO1_BASE              (0x0200)
#define GPIO2_BASE              (0x0200)
#define GPIO3_BASE              (0x0220)
#define GPIO4_BASE              (0x0220)
#define GPIO5_BASE              (0x0240)
#define GPIO6_BASE              (0x0240)
#define GPIO7_BASE              (0x0260)
#define GPIO8_BASE              (0x0260)
#define GPIOJ_BASE              (0x0320)
#define GPIO12_BASE             GPIO1_BASE
#define GPIO34_BASE             GPIO3_BASE
#define GPIO56_BASE             GPIO5_BASE
#define GPIO78_BASE             GPIO7_BASE

#define TIM0A5_BASE             (0x0340)
#define TIM1A3_BASE             (0x0380)
#define TIM0B7_BASE             (0x03C0)
#define TIM2A3_BASE             (0x0400)
#define RTC_BASE                (0x04A0)
#define MULT_BASE               (0x04C0)
#define DMACTL_BASE             (0x0500)
#define DMA0_BASE               (0x0510)
#define DMA1_BASE               (0x0520)
#define DMA2_BASE               (0x0530)
#define USCIA0_BASE             (0x05C0)
#define USCIB0_BASE             (0x05E0)
#define USCIA1_BASE             (0x0600)
#define USCIB1_BASE             (0x0620)

#define ADC12_BASE              (0x0700)
#define ADC10_BASE              (0x0740)
#define CB_BASE                 (0x08C0)
#define USBCONF_BASE            (0x0900)
#define USB_BASE                (0x0920)
//#define AES_BASE                (0x09C0)
//#define LCD_BASE                (0x0A00)
//#define RF_BASE                 (0x0F00)




/******************************************************************************/
/*                         Peripheral declaration                             */
/******************************************************************************/


// Special Function Registers (SFR)
#ifdef _SFR
//#   define SFR              ((SFR_Type*)SFR_BASE)
//#   define SFRb             ((SFRb_Type*)SFR_BASE)
#endif



// System Configuration Registers (SYS)
//#ifdef _SYS
#   define SYS              ((SYS_Type*)SYS_BASE)
//#endif



// Watchdog Timer(s) (WDT)
//#ifdef _WDT
#   ifdef _WDTA
#       define WDTA         ((WDT_Type*)WDTA_BASE)
#       define WDTAb        ((WDTb_Type*)WDTA_BASE)
#   endif
//#endif



// Universal Clock System (UCS)
//#ifdef _UCS
#   define UCS              ((UCS_Type*)UCS_BASE)
#   define UCSb             ((UCSb_Type*)UCS_BASE)
//#endif



// Power Management Module (PMM)
//#ifdef _PMM
#   define PMM              ((PMM_Type*)PMM_BASE)
#   define PMMb             ((PMMb_Type*)PMM_BASE)
//#endif



// Flash Controller (FLASH)
//#ifdef _FLASH
#   define FLASH            ((FLASH_Type*)FLASH_BASE)
#   define FLASHb           ((FLASHb_Type*)FLASH_BASE)
//#endif



// RAM Controller (RAM)
//#ifdef _RAM
#   define RAM              ((RAM_Type*)RAM_BASE)
//#endif




// Port Management (PM)
//#ifdef _PM
#   define PM               ((PM_Type*)PM_BASE)
#   define P1M              ((PXM_Type*)P1M_BASE)
#   define P2M              ((PXM_Type*)P2M_BASE)
#   define P3M              ((PXM_Type*)P3M_BASE)
#   define P4M              ((PXM_Type*)P4M_BASE)

//#endif




// GPIO peripherals (GPIO)
//#ifdef _GPIO

#   ifdef _GPIO1
#       define GPIO1        ((GPIO_EvenAccess*)GPIO1_BASE)
#   endif

#   ifdef _GPIO2
#       define GPIO2        ((GPIO_OddAccess*)GPIO2_BASE)
#   endif

#   ifdef _GPIO3
#       define GPIO3        ((GPIO_EvenAccess*)GPIO3_BASE)
#   endif

#   ifdef _GPIO4
#       define GPIO4        ((GPIO_OddAccess*)GPIO4_BASE)
#   endif

#   ifdef _GPIO5
#       define GPIO5        ((GPIO_EvenAccess*)GPIO5_BASE)
#   endif

#   ifdef _GPIO6
#       define GPIO6        ((GPIO_OddAccess*)GPIO6_BASE)
#   endif

#   ifdef _GPIO7
#       define GPIO7        ((GPIO_EvenAccess*)GPIO7_BASE)
#   endif

#   ifdef _GPIO8
#       define GPIO8        ((GPIO_OddAccess*)GPIO8_BASE)
#   endif

#   ifdef _GPIOJ
#       define GPIOJ        ((GPIO_EvenAccess*)GPIOJ_BASE)
#   endif

#   ifdef _GPIO12
#       define GPIO12       ((GPIO_WordAccess*)GPIO12_BASE)
#   endif

#   ifdef _GPIO34
#       define GPIO34       ((GPIO_WordAccess*)GPIO34_BASE)
#   endif

#   ifdef _GPIO56
#       define GPIO56       ((GPIO_WordAccess*)GPIO56_BASE)
#   endif

#   ifdef _GPIO78
#       define GPIO78       ((GPIO_WordAccess*)GPIO78_BASE)
#   endif

//#endif




// DMA Controller and Peripherals (DMA)
#ifdef _DMA

#   define DMA              ((DMA_Type*)DMACTL_BASE)
    
#   ifdef _DMA0
#       define DMA0         ((DMAx_Type*)DMA0_BASE)
#   endif

#   ifdef _DMA1
#       define DMA1         ((DMAx_Type*)DMA1_BASE)
#   endif

#   ifdef _DMA2
#       define DMA2         ((DMAx_Type*)DMA2_BASE)
#   endif

#endif




// CRC16 Calculation Peripheral (CRC)
#ifdef _CRC
#   define CRC              ((CRC_Type*)CRC_BASE)
#   define CRCb             ((CRCb_Type*)CRC_BASE)
#endif



// AES128 Calculation Peripheral (AES)
//#ifdef _AES
//#   define AES              ((AES_Type*)AES_BASE)
//#endif




// Timer A peripherals
#ifdef _TIMA
#   ifdef _TIM0A5
#       define TIM0A5       ((TIMA_Type*) TIM0A5_BASE)
#   endif
#   ifdef _TIM1A3
#       define TIM1A3       ((TIMA_Type*) TIM1A3_BASE)
#   endif
#   ifdef _TIM2A3
#       define TIM2A3       ((TIMA_Type*) TIM2A3_BASE)
#   endif
#endif

#ifdef _TIMB
#   ifdef _TIM0B7
#       define TIM0B7       ((TIMA_Type*) TIM0B7_BASE)
#   endif
#endif


// Real Time Clock (RTC)
#ifdef _RTC
#   define RTC              ((RTC_Type *) RTC_BASE)
#endif /*_RTC */



// Universal Serial Controller Interface (USCI)
#ifdef _USCI

#   ifdef _UARTA0
#       define UARTA0        ((UART_Type*)USCIA0_BASE)
#   endif
#   ifdef _SPIA0
#       define SPIA0         ((SPI_Type*)USCIA0_BASE)
#   endif

#   ifdef _SPIB0
#       define SPIB0         ((SPI_Type*)USCIB0_BASE)
#   endif
#   ifdef _I2CB0
#       define I2CB0         ((I2C_Type*)USCIB0_BASE)
#   endif

#   ifdef _UARTA1
#       define UARTA1        ((UART_Type*)USCIA1_BASE)
#   endif
#   ifdef _SPIA1
#       define SPIA1         ((SPI_Type*)USCIA1_BASE)
#   endif

#   ifdef _SPIB1
#       define SPIB1         ((SPI_Type*)USCIB1_BASE)
#   endif
#   ifdef _I2CB1
#       define I2CB1         ((I2C_Type*)USCIB1_BASE)
#   endif

#endif




// CC1101 Radio (RF)
//#ifdef _RF
//#   define RFByte           ((RF_ByteAccess*)RF_BASE)
//#   define RFWord           ((RF_WordAccess*)RF_BASE)
//#endif




// Reference Module (REF)
#ifdef _REF
#   define REF              ((REF_Type*)REF_BASE)
#endif




// Comparator B (CB)
#ifdef _CB
#   define CB               ((CB_Type*)CB_BASE)
#endif



// 10 or 12 bit Analog to Digital Converter (ADC10, ADC12)
#if defined(_ADC12)
#   define ADC12            ((ADC12_Type*)ADC12_BASE)
#   define ADC              ADC12
#elif defined(_ADC10)
#   define ADC10            ((ADC10_Type*)ADC10_BASE)
#   define ADC              ADC10
#endif




// Liquid Crystal Display Bus Controller (LCD)
//#ifdef _LCD
//#   define LCD              ((LCD_Type*)LCD_BASE)
//#endif



#endif

