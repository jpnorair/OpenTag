/* Copyright 2012 JP Norair
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
/** @file       /OTplatform/~MSP430/CC430/cc430_map.h
  * @author     JP Norair
  * @version    V1.0
  * @date       1 May 2012
  * @brief      Resource & Peripheral Mappings for CC430
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#ifndef __CC430_MAP_H
#define __CC430_MAP_H


#include "cc430_conf.h"
#include "cc430_types.h"




// The peripheral register typedefs are in the order the peripherals are 
// presented in the CC430 Users' Guide



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




/*------------------------ PMM -----------------------------------------------*/
typedef struct {
    vu16 CTL0;
    vu16 CTL1;
    vu16 SVSMHCTL;
    vu16 SVSMLCTL;
    vu16 SVSMIO;
    vu16 IFG;
    vu16 Reserved0C;
    vu16 RIE;
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
    vu8 SVSMIO_L;
    vu8 SVSMIO_H;
    vu8 IFG_L;
    vu8 IFG_H;
    vu8 Reserved0C;
    vu8 Reserved0D;
    vu8 RIE_L;
    vu8 RIE_H;
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




/*------------------------ RAM Controller ------------------------------------*/
typedef struct {
    vu16 CTL0;
} RAM_Type;





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




/*------------------------ AES -----------------------------------------------*/
typedef struct {
    vu16 CTL0;
    vu16 Reserved02;
    vu16 STAT;
    vu16 KEY;
    vu16 DIN;
    vu16 DOUT;
} AES_Type;





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




/*------------------------ RTC -----------------------------------------------*/
typedef struct {
    vu16 CTL01;
    vu16 CTL23;
    vu16 Reserved04;
    vu16 Reserved06;
    vu16 PS0CTL;
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





/*------------------------ RF1A ----------------------------------------------*/
typedef struct {
    vu16 IFCTL0;        // 00
    vu16 IFCTL1;        // 02
    vu16 IFCTL2;        // 04
    vu16 IFERR;         // 06
    vu16 Reserved08;    // 08
    vu16 Reserved0A;    // 0A
    vu16 IFERRV;        // 0C
    vu16 IFIV;          // 0E
    vu16 INSTRW;        // 10
    vu16 INSTR1W;       // 12
    vu16 INSTR2W;       // 14
    vu16 DINW;          // 16
    vu16 Reserved18;    // 18
    vu16 Reserved1A;    // 1A
    vu16 Reserved1C;    // 1C
    vu16 Reserved1E;    // 1E
    vu16 STATW;         // 20
    vu16 STAT1W;        // 22
    vu16 STAT2W;        // 24
    vu16 Reserved26;    // 26
    vu16 DOUTW;         // 28
    vu16 DOUT1W;        // 2A
    vu16 DOUT2W;        // 2C
    vu16 Reserved2E;    // 2E
    vu16 IN;            // 30
    vu16 IFG;           // 32
    vu16 IES;           // 34
    vu16 IE;            // 36
    vu16 IV;            // 38
    vu16 Reserved3A;    // 3A
    vu16 Reserved3C;    // 3C
    vu16 Reserved3E;    // 3E
} RF_WordAccess;

typedef struct {
    vu16 Reserved00;    // 00
    vu8  IFIFG;         // 02
    vu8  IFIE;          // 03
    vu16 Reserved04;    // 04
    vu16 Reserved06;    // 06
    vu16 Reserved08;    // 08
    vu16 Reserved0A;    // 0A
    vu16 Reserved0C;    // 0C
    vu16 Reserved0E;    // 0E
    vu8  DINB;          // 10
    vu8  INSTRB;        // 11
    vu8  Reserved12;    // 12
    vu8  INSTR1B;       // 13
    vu8  Reserved14;    // 14
    vu8  INSTR2B;       // 15
    vu16 Reserved16;    // 16
    vu16 Reserved18;    // 18
    vu16 Reserved1A;    // 1A
    vu16 Reserved1C;    // 1C
    vu16 Reserved1E;    // 1E
    vu8  DOUTB;         // 20
    vu8  STATB;         // 21
    vu8  DOUT1B;        // 22
    vu8  STAT1B;        // 23
    vu8  DOUT2B;        // 24
    vu8  STAT2B;        // 25
    vu16 Reserved26;    // 26
    vu16 Reserved28;    // 28
    vu16 Reserved2A;    // 2A
    vu16 Reserved2C;    // 2C
    vu16 Reserved2E;    // 2E
    vu16 Reserved30;    // 30
    vu16 Reserved32;    // 32
    vu16 Reserved34;    // 34
    vu16 Reserved36;    // 36
    vu16 Reserved38;    // 38
    vu16 Reserved3A;    // 3A
    vu8  FIFORX;        // 3C   //Not sure if this still exists
    vu8  Reserved3D;    // 3D
    vu8  FIFOTX;        // 3E   //Not sure if this still exists
    vu8  Reserved3F;    // 3F
} RF_ByteAccess;




/*------------------------ REF -----------------------------------------------*/
typedef struct {
    vu16 CTL0;
} REF_Type;




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
    vu8  MCTL1;
    vu8  MCTL0;
    vu8  MCTL3;
    vu8  MCTL2;
    vu8  MCTL5;
    vu8  MCTL4;
    vu8  MCTL7;
    vu8  MCTL6;
    vu8  MCTL9;
    vu8  MCTL8;
    vu8  MCTL11;
    vu8  MCTL10;
    vu8  MCTL13;
    vu8  MCTL12;
    vu8  MCTL15;
    vu8  MCTL14;
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




/*------------------------ LCD ---------------------------------------------*/
typedef struct {
    vu16 CTL0;
    vu16 CTL1;
    vu16 BLKCTL;
    vu16 MEMCTL;
    vu16 VCTL;
    vu16 PCTL0;
    vu16 PCTL1;
    vu16 PCTL2;
    vu16 PCTL3;
    vu16 CPCTL;
    vu16 Reserved14;
    vu16 Reserved16;
    vu16 Reserved18;
    vu16 Reserved1A;
    vu16 Reserved1C;
    vu16 IV;
} LCD_Type;








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
#define GPIO1_BASE              (0x0200)
#define GPIO2_BASE              (0x0200)
#define GPIO3_BASE              (0x0220)
#define GPIO4_BASE              (0x0220)
#define GPIO5_BASE              (0x0240)
#define GPIOJ_BASE              (0x0320)
#define GPIO12_BASE             (0x0200)
#define GPIO34_BASE             (0x0220)

#define TIM0A5_BASE             (0x0340)
#define TIM1A3_BASE             (0x0380)
#define RTC_BASE                (0x04A0)
#define MULT_BASE               (0x04C0)
#define DMACTL_BASE             (0x0500)
#define DMA0_BASE               (0x0510)
#define DMA1_BASE               (0x0520)
#define DMA2_BASE               (0x0530)
#define USCIA0_BASE             (0x05C0)
#define USCIB0_BASE             (0x05E0)
#define ADC12_BASE              (0x0700)
#define CB_BASE                 (0x08C0)
#define AES_BASE                (0x09C0)
#define LCD_BASE                (0x0A00)
#define RF_BASE                 (0x0F00)




/******************************************************************************/
/*                         Peripheral declaration                             */
/******************************************************************************/


// Special Function Registers (SFR)
//#ifdef _SFR
//#   define SFR              ((SFR_Type*)SFR_BASE)
//#   define SFRb             ((SFRb_Type*)SFR_BASE)
//#endif



// System Configuration Registers (SYS)
//#ifdef _SYS
#   define SYS              ((SYS_Type*)SYS_BASE)
//#endif



// Watchdog Timer(s) (WDT)
//#ifdef _WDT
//#   ifdef _WDTA
#       define WDTA         ((WDT_Type*)WDTA_BASE)
#       define WDTAb        ((WDTb_Type*)WDTA_BASE)
//#   endif
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
//#endif




// GPIO peripherals (GPIO)
//#ifdef _GPIO_
#   define GPIO1        ((GPIO_EvenAccess*)GPIO1_BASE)
#   define GPIO2        ((GPIO_OddAccess*)GPIO2_BASE)
#   define GPIO3        ((GPIO_EvenAccess*)GPIO3_BASE)
#   define GPIO4        ((GPIO_OddAccess*)GPIO4_BASE)
#   define GPIO5        ((GPIO_EvenAccess*)GPIO5_BASE)
#   define GPIOJ        ((GPIO_EvenAccess*)GPIOJ_BASE)
#   define GPIO12       ((GPIO_WordAccess*)GPIO12_BASE)
#   define GPIO34       ((GPIO_WordAccess*)GPIO34_BASE)
//#endif




// DMA Controller and Peripherals (DMA)
//#ifdef _DMA
#   define DMA          ((DMA_Type*)DMACTL_BASE)
#   define DMA0         ((DMAx_Type*)DMA0_BASE)
#   define DMA1         ((DMAx_Type*)DMA1_BASE)
#   define DMA2         ((DMAx_Type*)DMA2_BASE)
//#endif




// CRC16 Calculation Peripheral (CRC)
//#ifdef _CRC
#   define CRC              ((CRC_Type*)CRC_BASE)
#   define CRCb             ((CRCb_Type*)CRC_BASE)
//#endif



// AES128 Calculation Peripheral (AES)
//#ifdef _AES
#   define AES              ((AES_Type*)AES_BASE)
//#endif




// Timer A peripherals
//#ifdef _TIMA
#   define TIM0A5       ((TIMA_Type*) TIM0A5_BASE)
#   define TIM1A3       ((TIMA_Type*) TIM1A3_BASE)
//#endif



// Real Time Clock (RTC)
//#ifdef _RTC
#   define RTC              ((RTC_Type *) RTC_BASE)
//#endif /*_RTC */



// Universal Serial Controller Interface (USCI)
//#ifdef _USCI
#   define UARTA0       ((UART_Type*)USCIA0_BASE)
#   define SPIA0        ((SPI_Type*)USCIA0_BASE)
#   define I2CA0        ((I2C_Type*)USCIA0_BASE)
#   define UARTB0       ((UART_Type*)USCIB0_BASE)
#   define SPIB0        ((SPI_Type*)USCIB0_BASE)
#   define I2CB0        ((I2C_Type*)USCIB0_BASE)
//#endif




// CC1101 Radio (RF)
//#ifdef _RF
#   define RFByte           ((RF_ByteAccess*)RF_BASE)
#   define RFWord           ((RF_WordAccess*)RF_BASE)
//#endif




// Reference Module (REF)
//#ifdef _REF
#   define REF              ((REF_Type*)REF_BASE)
//#endif




// Comparator B (CB)
//#ifdef _CB
#   define CB               ((CB_Type*)CB_BASE)
//#endif



// 12 bit Analog to Digital Converter (ADC12)
//#ifdef _ADC12
#   define ADC12            ((ADC12_Type*)ADC12_BASE)
//#endif



// Liquid Crystal Display Bus Controller (LCD)
//#ifdef _LCD
#   define LCD              ((LCD_Type*)LCD_BASE)
//#endif



#endif

