/* Copyright 2010
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
  * @file       /OTlib/STM32H152/platform_STM32L1xx.c
  * @author
  * @version    V1.0
  * @date       16 Feb 2012
  * @brief      ISRs and hardware services abstracted by the platform module
  * @ingroup    Platform
  *
  * ISRs implemented in this file shall use only the Platform module macro
  * abstractions, not functions or register nomenclature specific to any given
  * hardware platform.
  *
  ******************************************************************************
  */

#define _plat_  1
#include <otlib/utils.h>
#include <otsys/types.h>
#include <otsys/config.h>
#include <otplatform.h>

// OT modules that need initialization
#include <otsys/veelite.h>
#include <otsys/veelite_core.h>
#include <otlib/buffers.h>
#include <otlib/auth.h>
#include <otsys/mpipe.h>
#include <m2/radio.h>
#include <otsys/syskern.h>
#include <m2/session.h>
#ifdef RADIO_DEBUG
#   include "debug_uart.h"
#endif

DMA_InitTypeDef  UTX_DMA_Init;
#define USART3_DR_ADDRESS       0x40004804  /* FIXME: hardcoding is stupid */

static void
platform_uart_init(void)
{
    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate = 115200;    // 57600
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    /* USART configuration */
    USART_Init(USART3, &USART_InitStructure);

    {
        /* DMA Configuration -------------------------------------------------------*/
        UTX_DMA_Init.DMA_PeripheralBaseAddr = USART3_DR_ADDRESS;
        UTX_DMA_Init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        UTX_DMA_Init.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
        UTX_DMA_Init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        UTX_DMA_Init.DMA_MemoryInc = DMA_MemoryInc_Enable;
        UTX_DMA_Init.DMA_Mode = DMA_Mode_Normal;
        UTX_DMA_Init.DMA_M2M = DMA_M2M_Disable;

        UTX_DMA_Init.DMA_DIR = DMA_DIR_PeripheralDST;  // only used for UART TX
        UTX_DMA_Init.DMA_Priority = DMA_Priority_High;


        USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);  // non-blocking dma for usart3 tx
    }

    /* Enable USART */
    USART_Cmd(USART3, ENABLE);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

}

unsigned int rprime = 83;   // rand number generator
static unsigned int r = 73; // rand number generator

static void
rng_seed()
{
    ADC_InitTypeDef ADC_InitStructure;
    ot_u16 ADCdata;
    int n;

    /* Enable The HSI (16Mhz) */
    RCC_HSICmd(ENABLE); // ADC can only use HSI?


    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 1, ADC_SampleTime_4Cycles);

    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);

    /* Wait until ADC1 ON status */
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET)
    {
        asm("nop");
    }

    /* Start ADC1 Software Conversion */
    ADC_SoftwareStartConv(ADC1);

    r = 0;
    for (n = 0; n < 30; n++) {
        /* Wait until end of conversion */
        while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
            asm("nop");

        /* Read ADC conversion result */
        ADCdata = ADC_GetConversionValue(ADC1);
        r += ADCdata & 0x07;    // take lower noise bits
    }

    //debug_printf("r: %x\r\n", r);

    ADC_Cmd(ADC1, DISABLE);
    RCC_HSICmd(DISABLE); // assuming HSI not used
}

ot_u8 rand_prn8() {
    static unsigned int Y;
    // TODO: some random number from STM32L
    if (r==0 || r==1 || r==-1)
        r=rprime; /* keep from getting stuck */
    r = (9973 * ~r) + ((Y) % 701); /* the actual algorithm */
//    Y = (r>>24) % 9; /* choose upper bits and reduce */
    Y = r>>24; /* choose upper bits and reduce */
    return Y;
}

ot_u16 rand_prn16() {
    ot_u16 ret = rand_prn8();
    ret <<= 8;
    return ret + rand_prn8();
}

static void
platform_init_spi(void)
{
    SPI_InitTypeDef  SPI_InitStructure;

    /* expecting platform_init_periphclk() to have already enabled SPI2 clock and GPIO clocks */

    /* SPI configuration -------------------------------------------------------*/
    SPI_I2S_DeInit(SPI2);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    //SPI_InitStructure.SPI_CRCPolynomial = 7;


    /* Initializes the SPI communication */
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_Init(SPI2, &SPI_InitStructure);

    /* Enable the Rx buffer not empty interrupt */
    SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);

    /* Enable the SPI peripheral */
    SPI_Cmd(SPI2, ENABLE);

    // platform_gpio_init(): NSS under software-gpio control
}

void systim_init(void* tim_init)
{
    TIM_TimeBaseInitTypeDef tbase_in;

    TIM_DeInit(OT_GPTIM);

/*    tbase_in.TIM_Prescaler      = 0;
    tbase_in.TIM_Period         = *(ot_u32*)tim_init;*/
    tbase_in.TIM_Prescaler      = *(ot_u32*)tim_init;
    tbase_in.TIM_Period         = 1;
    tbase_in.TIM_CounterMode    = TIM_CounterMode_Up;
    tbase_in.TIM_ClockDivision  = TIM_CKD_DIV1;
    TIM_TimeBaseInit(OT_GPTIM, &tbase_in);

    TIM_SetCompare1(OT_GPTIM, *(ot_u32*)tim_init);

    // Timers 9/10/11 have external clock wired to RTC crystal.
    // (The fact that the clock mode is called "Mode2" is a coincidence)
    // mode1: external input pin (TIx)
    // mode2: external trigger input (ETR)
    TIM_ETRClockMode2Config(OT_GPTIM,
                            TIM_ExtTRGPSC_OFF,
                            TIM_ExtTRGPolarity_NonInverted,
                            0);

}

void platform_init_interruptor() {
/// Configures the NVIC and the External interrupt.  The NVIC controllers are
/// enabled by default so that, to turn the interrupts on or off, only the
/// peripheral interrupt bits need to be set.
///
/// @note   Priority Group 2 is used.  In this method, there are 4 subpriorities
///         and 4 pre-emption priorities.  OpenTag itself sits in pre-emption
///         group 1, using the subpriorities as follows:
///              0 - SX1212 interrupts (GPIOs to IRQ0, IRQ1)
///              0 - SX1212 Pseudo-interrupts (TIM3)
///              1 - OpenTag Timer interrupt (TIM2)
///              1 - RTC interrupt
///              1 - Mpipe interrupts

    NVIC_InitTypeDef nvic_in;
    EXTI_InitTypeDef exti_in;

    exti_in.EXTI_Mode       = EXTI_Mode_Interrupt;
    exti_in.EXTI_Trigger    = EXTI_Trigger_Rising;
    exti_in.EXTI_LineCmd    = DISABLE;              // Flip this when activating RF

    // IRQ0, 1, PLL_LOCK GPIO interrupts on pins ....
#ifdef _STM32L152VBT6_ // !STM32H152:
    // PA4 SX1231-DIO0 PacketSent
    // PA5 SX1231-DIO1 FifoLevel
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4); // Connect EXTI4 Line to PA4 pin
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource5); // Connect EXTI4 Line to PA5 pin
    exti_in.EXTI_Line       = EXTI_Line4;   // PA4
    EXTI_Init(&exti_in);
    exti_in.EXTI_Line       = EXTI_Line5;   // PA5
    EXTI_Init(&exti_in);
#else   // DK7A433:
#error x
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0); // Connect EXTI0 Line to PA0 pin
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource1); // Connect EXTI1 Line to PA1 pin
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource2); // Connect EXTI2 Line to PA2 pin
    exti_in.EXTI_Line       = EXTI_Line1;   // PA1;
    EXTI_Init(&exti_in);
    exti_in.EXTI_Line       = EXTI_Line2;   // PA2;
    EXTI_Init(&exti_in);
    exti_in.EXTI_Line       = EXTI_Line0;   // PA0;
    EXTI_Init(&exti_in);
#endif /* !_STM32L152VBT6_ */

    //nonexistant function NVIC_DeInit();
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00);                              //Vector Table @ 0x08000000
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);                             // Priority Group to 2 bits
    //TODO find systick priority: NVIC_SystemHandlerPriorityConfig(SystemHandler_SysTick, 0, 0);              // SysTick priority = 0,0 (highest)

    ///1. Radio interrupts (Group 0)
#ifdef _STM32L152VBT6_ // !STM32H152:
    nvic_in.NVIC_IRQChannel                     = EXTI4_IRQn;    // from PA4 SX1231-DIO0
    nvic_in.NVIC_IRQChannelPreemptionPriority   = 0;
    nvic_in.NVIC_IRQChannelCmd                  = ENABLE;
    NVIC_Init(&nvic_in);

    nvic_in.NVIC_IRQChannel                     = EXTI9_5_IRQn;    // from PA5 SX1231-DIO1
    NVIC_Init(&nvic_in);
#else   // DK7A433:
#error x
    ///   - IRQ0, IRQ1 GPIN external interrupts
    nvic_in.NVIC_IRQChannel                     = EXTI1_IRQn;    // from PA1 IRQ0
    nvic_in.NVIC_IRQChannelPreemptionPriority   = 0;
    nvic_in.NVIC_IRQChannelCmd                  = ENABLE;
    NVIC_Init(&nvic_in);

    nvic_in.NVIC_IRQChannel                     = EXTI2_IRQn;    // from PA2 IRQ1
    NVIC_Init(&nvic_in);
#endif

    // TIM3 interrupt (for RX Timeout Timer, unused presently)
    //nvic_in.NVIC_IRQChannel                     = TIM10_IRQChannel;
    //nvic_in.NVIC_IRQChannelSubPriority          = 2;
    //NVIC_Init(&nvic_in);

    ///2. OpenTag Interrupts
    ///   - Mpipe, GPTIM, RTC
    nvic_in.NVIC_IRQChannelPreemptionPriority   = 1;
    nvic_in.NVIC_IRQChannelSubPriority          = 0;
    nvic_in.NVIC_IRQChannel                     = DMA1_Channel2_IRQn; //MPIPE TX DMA Channel;
    NVIC_Init(&nvic_in);
    nvic_in.NVIC_IRQChannel                     = DMA1_Channel3_IRQn; //MPIPE RX DMA Channel;
    NVIC_Init(&nvic_in);

    nvic_in.NVIC_IRQChannelSubPriority          = 1;
    nvic_in.NVIC_IRQChannel                     = OT_GPTIM_IRQn;
    NVIC_Init(&nvic_in);

    nvic_in.NVIC_IRQChannelSubPriority          = 1;
    nvic_in.NVIC_IRQChannel                     = RXTIM_IRQn;
    NVIC_Init(&nvic_in);

    //nvic_in.NVIC_IRQChannelSubPriority          = 2;
    //nvic_in.NVIC_IRQChannel                     = RTC_IRQChannel;
    //NVIC_Init(&nvic_in);

    nvic_in.NVIC_IRQChannelSubPriority          = 2;
    nvic_in.NVIC_IRQChannel                     = SPI2_IRQn;
    NVIC_Init(&nvic_in);

    nvic_in.NVIC_IRQChannelSubPriority          = 2;
    nvic_in.NVIC_IRQChannel                     = USART3_IRQn;
    NVIC_Init(&nvic_in);

    nvic_in.NVIC_IRQChannelSubPriority          = 2;
    nvic_in.NVIC_IRQChannel                     = APPTIM_IRQn;
    NVIC_Init(&nvic_in);
}

void
platform_init_busclk()
{
    ///@todo enable RTC Clock

#   ifdef __DEBUG__
    DBGMCU_Config(DBGMCU_SLEEP     |
                  DBGMCU_STOP      |
                  DBGMCU_STANDBY   |
                  DBGMCU_IWDG_STOP |
                  DBGMCU_WWDG_STOP |
                  DBGMCU_TIM2_STOP |
                  DBGMCU_TIM3_STOP ,
                  ENABLE);
#   endif
}

void platform_init_gpio() {
/// Configures the different GPIO ports pins.
/// -------------------------------------------------------------
/// PA0            <=  RF_PLL_LOCK
/// PA1            <=  RF_IRQ0
/// PA2            <=  RF_IRQ1
/// PA5/SPI1_SCK   <=  RF_CLKOUT    (generally unused)
/// PA9             => NSS_Data
/// PB5             => RF Switch V1
/// PB8 & PB9      <=> RF_DATA      (Mode 1 only)
/// PB12/SPI2_NSS   => NSS_Config
/// PB13/SPI2_SCK   => SCK
/// PB14/SPI2_MISO <=  MISO
/// PB15/SPI2_MOSI  => MOSI
/// PC13            => RF Switch V2
/// -------------------------------------------------------------
/// PA8             => Red Bicolor LED line
/// PA10            => Green Bicolor LED line
/// PB0             => Debug LED (Yellow)
/// -------------------------------------------------------------
/// PB10/UART3_TX   => Serial UART TX to breakout board
/// PB11/UART3_RX  <=  Serial UART RX from breakout board
/// -------------------------------------------------------------
/// PB6/I2C1_SCL    => I2C sensor bus Master Clock (SCL)
/// PB7/I2C1_SDA   <=> I2C sensor bus SDA
/// -------------------------------------------------------------
/// PA11/USB_DM    <=> USB-
/// PA12/USB_DP    <=> USB+
/// -------------------------------------------------------------
/// PA3            <=  Generic Input
/// PB1            <=  Generic Input
/// -------------------------------------------------------------
    GPIO_InitTypeDef    gpio_in;

    // Assume 10MHz for everything
    // Also, all outputs in the schematic are push-pull (no drains)
    gpio_in.GPIO_Speed      = GPIO_Speed_10MHz;
    gpio_in.GPIO_OType      = GPIO_OType_PP;

    /// Generic Input
    // (not established right now)

    // analog input of unused pin(s)
    gpio_in.GPIO_Mode       = GPIO_Mode_AN;
    gpio_in.GPIO_PuPd       = GPIO_PuPd_NOPULL;
    gpio_in.GPIO_Pin        = GPIO_Pin_5;
    GPIO_Init(GPIOA, &gpio_in);

    /// LED Interface
    gpio_in.GPIO_Mode       = GPIO_Mode_OUT;    ///@todo check IPD/IPU
    gpio_in.GPIO_Pin        = GPIO_Pin_LED_RED | GPIO_Pin_LED_GREEN;
    GPIO_Init(GPIO_Port_LED_RED, &gpio_in);
#ifndef _STM32L152VBT6_ // !STM32H152:
    gpio_in.GPIO_Pin        = GPIO_Pin_LED_YELLOW;
    GPIO_Init(GPIO_Port_LED_YELLOW, &gpio_in);
#endif
    RED_LED_OFF;
    GREEN_LED_OFF;

    /// RF interface GPIO
    gpio_in.GPIO_Mode       = GPIO_Mode_IN;    ///@todo check IPD/IPU
    gpio_in.GPIO_PuPd       = GPIO_PuPd_DOWN;
#ifdef _STM32L152VBT6_ // !STM32H152:
    gpio_in.GPIO_Pin        = GPIO_Pin_RxReady | GPIO_Pin_PacketSent | GPIO_Pin_FifoLevel;
    GPIO_Init(GPIOA, &gpio_in);

    // NSS_CONFIG
    GPIO_SetBits(GPIOB, GPIO_Pin_12);    // deassert
    gpio_in.GPIO_Mode       = GPIO_Mode_OUT;
    gpio_in.GPIO_Pin        = GPIO_Pin_NSS_CONFIG;
    GPIO_Init(GPIOB, &gpio_in);

    gpio_in.GPIO_Pin        = GPIO_Pin_CON2_40; // debug output
    GPIO_Init(GPIOC, &gpio_in);
#else
    // PLL_LOCK, IRQ0, IRQ1, CLKOUT
    gpio_in.GPIO_Pin        = GPIO_Pin_PLL_LOCK | GPIO_Pin_RFIRQ0 | GPIO_Pin_RFIRQ1 | GPIO_Pin_5;
    GPIO_Init(GPIO_Port_RFIRQ, &gpio_in);
    // NSS_Data
    GPIO_SetBits(GPIOA, GPIO_Pin_9);    // deassert
    gpio_in.GPIO_Mode       = GPIO_Mode_OUT;
    gpio_in.GPIO_Pin        = GPIO_Pin_NSS_DATA | GPIO_Pin_IO1;
    GPIO_Init(GPIOA, &gpio_in);

    // RF_SwitchV1, NSS_CONFIG
    GPIO_SetBits(GPIOB, GPIO_Pin_12);    // deassert
    gpio_in.GPIO_Pin        = GPIO_Pin_RFSW_V1 | GPIO_Pin_NSS_CONFIG | GPIO_Pin_IO2;
    GPIO_Init(GPIOB, &gpio_in);
    // RF_SwitchV2
    gpio_in.GPIO_Pin        = GPIO_Pin_RFSW_V2;
    GPIO_Init(GPIO_Port_RFSW_V2, &gpio_in);
#endif /* !_STM32L152VBT6_ */
    // RF SPI+DMA Bus        (PB1=U1_PB1, PB2=U1_BOOT1)
    gpio_in.GPIO_Mode       = GPIO_Mode_AF;
    gpio_in.GPIO_Pin        = GPIO_Pin_2 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &gpio_in);
    /* GPIO_PinSource* cannot be or'd */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);    // SCK
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);    // MISO
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);    // MOSI

    /// Serial UART to breakout board
    // (have driver module set up the UART interface)
    gpio_in.GPIO_Pin        = GPIO_Pin_10 | GPIO_Pin_11;
    gpio_in.GPIO_PuPd         = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &gpio_in);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

    /// I2C Bus to sensors
    // (have driver module set up the I2C interface)
    gpio_in.GPIO_Pin        = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &gpio_in);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);

    /// USB to I/O connector
    // (have driver module set up the I2C interface)
    gpio_in.GPIO_Pin        = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_Init(GPIOA, &gpio_in);
    GPIO_PinAFConfig(GPIOA, gpio_in.GPIO_Pin, GPIO_AF_USB);

#ifndef _STM32L152VBT6_ // !STM32H152:
    // RF SW: select on-board antenna by default (J1->J2)
    GPIO_WriteBit(GPIO_Port_RFSW_V1, GPIO_Pin_RFSW_V1, Bit_RESET);
    GPIO_WriteBit(GPIO_Port_RFSW_V2, GPIO_Pin_RFSW_V2, Bit_SET);
#endif

}

void platform_init_periphclk() {
    /// 1. Enable AHB clocks
    /// AHB is set to whatever clock frequency the CPU is using.
    ///
    /// RCC_AHBPeriph_GPIOA (*), RCC_AHBPeriph_GPIOB (*), RCC_AHBPeriph_GPIOC (*),
    /// RCC_AHBPeriph_GPIOD, RCC_AHBPeriph_GPIOE, RCC_AHBPeriph_GPIOH,
    /// RCC_AHBPeriph_CRC, RCC_AHBPeriph_FLITF, RCC_AHBPeriph_SRAM,
    /// RCC_AHBPeriph_DMA1 (*)
    RCC_AHBPeriphClockCmd(  RCC_AHBPeriph_GPIOA     |   \
                            RCC_AHBPeriph_GPIOB     |   \
                            RCC_AHBPeriph_GPIOC     |   \
                            RCC_AHBPeriph_DMA1,         \
                            ENABLE);

#ifdef _STM32L152VBT6_ // STM32H152:
    RCC_AHBPeriphClockCmd(  RCC_AHBPeriph_GPIOE, ENABLE);
#endif

    /// Enable APB1 clocks (16 MHz)
    /// Available peripherals on APB1, (*) denotes known Platform usage:
    /// RCC_APB1Periph_TIM2, RCC_APB1Periph_TIM3, RCC_APB1Periph_TIM4,
    /// RCC_APB1Periph_TIM6, RCC_APB1Periph_TIM7, RCC_APB1Periph_LCD,
    /// RCC_APB1Periph_WWDG, RCC_APB1Periph_SPI2 (*), RCC_APB1Periph_USART2,
    /// RCC_APB1Periph_USART3 (*), RCC_APB1Periph_I2C1, RCC_APB1Periph_I2C2 (*),
    /// RCC_APB1Periph_USB (*), RCC_APB1Periph_PWR (*), RCC_APB1Periph_DAC,
    /// RCC_APB1Periph_COMP
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2     |   \
                            RCC_APB1Periph_USART3   |   \
                            RCC_APB1Periph_I2C2     |   \
                            RCC_APB1Periph_USB      |   \
                            RCC_APB1Periph_PWR      |   \
                            RCC_APB1Periph_TIM3     |   \
                            RCC_APB1Periph_TIM4,
                            ENABLE);


    /************* XXX fixme: should this be here? ********************/
    /* Allow access to the RTC */
    PWR_RTCAccessCmd(ENABLE);

    /* Reset Backup Domain */
    RCC_RTCResetCmd(ENABLE);
    RCC_RTCResetCmd(DISABLE);

    /*!< LSE Enable */
    RCC_LSEConfig(RCC_LSE_ON);

    /*!< Wait till LSE is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
        {}
    /******************************************************************/


    /// Enable APB2 clocks (1 MHz)
    /// Available peripherals on APB2, (*) denotes OpenTag usage:
    /// RCC_APB2Periph_SYSCFG, RCC_APB2ENR_SYSCFGEN, RCC_APB2Periph_TIM9 (*),
    /// RCC_APB2Periph_TIM10 (*), RCC_APB2Periph_TIM11, RCC_APB2Periph_ADC1 (*),
    /// RCC_APB2Periph_SPI1, RCC_APB2Periph_USART1
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM9     |   \
                            RCC_APB2Periph_TIM10    |   \
                            RCC_APB2Periph_TIM11    |   \
                            RCC_APB2Periph_ADC1,        \
                            ENABLE );


}

void
platform_poweron() {
	ot_u32 prescaler = 32;

    /// Hardware turn-on stuff
    SystemInit();                   // comes from STLib, does lots of startup
    platform_init_busclk();         // extra bus clock setup not in SystemInit()
    platform_init_periphclk();      // Peripherals OpenTag cares about
    platform_init_interruptor();    // Interrupts OpenTag cares about
    systim_init((void*)&prescaler);        // Initialize GPTIM (to 1024 Hz)
    platform_init_gpio();           // Set up connections on the board
    platform_init_spi();            // initialize command interface to radio

#if ( defined(RADIO_DEBUG) || (OT_FEATURE(MPIPE) == ENABLED) )
    platform_uart_init();
#endif /* RADIO_DEBUG */
#if defined(RADIO_DEBUG)
    debug_uart_init();
#endif /* RADIO_DEBUG */

    rng_seed();

    /// Restore vworm (following save on shutdown)
    vworm_init();

#   if (OT_FEATURE(MPIPE) == ENABLED)
        /// Mpipe (message pipe) typically used for serial-line comm.
        mpipe_init(NULL);
#   endif
}

void platform_init_OT() {

    buffers_init(); //buffers init must be first in order to do core dumps
    vl_init();      //Veelite init must be second

    radio_init();   //radio init third

    sys_init();     //system init last
}


static void
sub_systim_reattach(ot_u16 next_event)
{
    // Flush GPTIM and switch it back to up-counting interrupt mode
    TIM_Cmd(OT_GPTIM, DISABLE);    // TI: MC=00b stop mode
    TIM_SetCompare1(OT_GPTIM, next_event);
    TIM_SetAutoreload(OT_GPTIM, next_event);    // TI: "up" timer mode (count to compare)
    TIM_SetCounter(OT_GPTIM, 0);    // TI: TACLR
    OT_GPTIM->SR = 0;   // TI: TAIFG clear
    TIM_ITConfig(OT_GPTIM, TIM_IT_CC1, ENABLE);    // TI: TAIE off
    TIM_Cmd(OT_GPTIM, ENABLE);

}

void
platform_ot_preempt()
{
    // Manually kick the GPTIM interrupt flag.
    // this function is used particularly by radio ISRs to tell
    // sys_event_manager() run again when the radio I/O finishes.
    if ( (OT_GPTIM->ARR == 0xffff) || !(OT_GPTIM->CR1 & TIM_CR1_CEN) ) {
        /* timer was "unattached" (i.e. in "continuous" mode with interrupts off) */
        sub_systim_reattach(1);
        /* XXX ?? valididity/importance of next elapsed_time passed to sys_event_manager() ?? XXX */
    }

    TIM_GenerateEvent(OT_GPTIM, TIM_EventSource_CC1);
}

void ot_memcpy(ot_u8* dest, ot_u8* src, ot_int length) {
/// Behavior is always blocking.

#if (OS_FEATURE(MEMCPY) == ENABLED)
    memcpy(dest, src, length);

#elif (MCU_CONFIG(MEMCPYDMA) == ENABLED)
    MEMCPY_DMA->IFCR        = MEMCPY_DMA_INT;
    MEMCPY_DMA_CHAN->CPAR   = (ot_u32)dest;
    MEMCPY_DMA_CHAN->CMAR   = (ot_u32)src;
    MEMCPY_DMA_CHAN->CNDTR  = length;
    MEMCPY_DMA_CHAN->CCR    = DMA_DIR_PeripheralDST       | \
                              DMA_Mode_Normal             | \
                              DMA_PeripheralInc_Enable    | \
                              DMA_MemoryInc_Enable        | \
                              DMA_PeripheralDataSize_Byte | \
                              DMA_MemoryDataSize_Byte     | \
                              DMA_Priority_VeryHigh       | \
                              DMA_M2M_Enable              | \
                              DMA_CCR1_EN;
    while((MEMCPY_DMA->ISR & MEMCPY_DMA_INT) == 0);

#else
    /// Uses the "Duff's Device" for loop unrolling.  If this is incredibly
    /// confusing to you, check the internet for "Duff's Device."
    if (length > 0) {
        ot_int loops = (length + 7) >> 3;

        switch (length & 0x7) {
            case 0: do {    *dest++ = *src++;
            case 7:         *dest++ = *src++;
            case 6:         *dest++ = *src++;
            case 5:         *dest++ = *src++;
            case 4:         *dest++ = *src++;
            case 3:         *dest++ = *src++;
            case 2:         *dest++ = *src++;
            case 1:         *dest++ = *src++;
                        }
                        while (--loops > 0);
        }
    }

#endif
}

/** Platform Debug Triggers <BR>
  * ========================================================================<BR>
  * Triggers are optional pins used for debugging.  Sometimes they are
  * hooked up to LEDs, logic probes, etc.  There are 2 triggers defined
  * by default, but you could add more.
  */
#ifdef __DEBUG__
void trigger(TR1, high) {    }      //Green LED
void trigger(TR1, low) {     }
void trigger(TR1, toggle) {  }
void trigger(TR2, high) {    }      // Red LED
void trigger(TR2, low) {     }
void trigger(TR2, toggle) {  }
#else
void trigger(TR1, high) { }
void trigger(TR1, low) { }
void trigger(TR1, toggle) { }
void trigger(TR2, high) { }
void trigger(TR2, low) { }
void trigger(TR2, toggle) { }
#endif

#define CNT1us    (PLATFORM_HSCLOCK_HZ/(1000000 * 5))
#define CNT1ms    (CNT1us*1000)

void delay_ms(ot_u16 n) {
	ot_u32 c;

	c = n * CNT1ms;
	for (; c>0; c--);
}


void delay_us(ot_u16 n) {
	ot_u32 c;

	c = n * CNT1us;
	for (; c>0; c--);
}

#if (MCU_FEATURE(CRC) == ENABLED)
#endif /* (MCU_FEATURE(CRC) == ENABLED) */

volatile ot_bool call_platform_ot_run = False;
#if (SYS_FLOOD == ENABLED)
volatile ot_bool call_start_tx = False;  // when tx flooding, repeat tx
#endif  /* SYS_FLOOD */

void
TIM9_IRQHandler(void)
{
    /* OT_GPTIM */
    if (TIM_GetITStatus(OT_GPTIM, TIM_IT_CC1) != RESET) {
        TIM_ClearITPendingBit(OT_GPTIM, TIM_IT_CC1);

        /* fixme: investigate premature interrupt occuring at CNT ~= 4 */
        if (OT_GPTIM->CNT >= OT_GPTIM->CCR1)    // if not spurious interrupt
            call_platform_ot_run = 1;    // to be called from main loop context
    }
#ifdef RADIO_DEBUG
    else {
        // some other interrupt i have not enabled
        debug_printf("TIM9\r\n");
        for (;;)
            asm("nop");
    }
#endif /* RADIO_DEBUG */

    if (TIM_GetITStatus(OT_GPTIM, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(OT_GPTIM, TIM_IT_Update);
        // @todo: find out who enabled the update interrupt
    }
    //lr = LR_TIM9;
}

ot_u16
platform_get_systim()
{
    return OT_GPTIM->CNT;
}

static void
sub_systim_unattach()
{
    // Stop GPTIM, resume in continuous mode with interrupt off.
    TIM_Cmd(OT_GPTIM, DISABLE);    // TI: MC=00b stop mode
    TIM_SetCounter(OT_GPTIM, 0);    // TI: TACLR
    TIM_SetAutoreload(OT_GPTIM, 0xffff);    // TI: "continuous" timer mode
    TIM_ITConfig(OT_GPTIM, TIM_IT_CC1, DISABLE);    // TI: TAIE off
    TIM_Cmd(OT_GPTIM, ENABLE);

}


void
platform_flush_systim()
{
    // Zeros GPTIM, turns off interrupt, and puts into free-running
    sub_systim_unattach();
}

void
platform_ot_run()
{
    ot_u16 next_event;
    ot_u16 elapsed_time;

    elapsed_time = OT_GPTIM->CCR1;
    //debug_printf("otrun: %d:%d\r\n", OT_GPTIM->CNT, OT_GPTIM->CCR1);
    if (OT_GPTIM->CNT != OT_GPTIM->CCR1) {
        asm("nop");
    }

    /// Clear and disable the timer interrupt while in the ISR
    /// (Also stop it, and restart it in continuous mode).
    sub_systim_unattach();

    next_event = sys_event_manager( elapsed_time );
    /* next_event will be 1 if nextevent occured during radio i/o */

    /// Flush GPTIM and switch it back to up-counting interrupt mode
    sub_systim_reattach(next_event);
}

void
sleep_mcu(void)
{
    PWR_EnterSleepMode(PWR_Regulator_LowPower, PWR_SLEEPEntry_WFI);

#ifdef RADIO_DEBUG
    console_service();
#endif
    if (call_platform_ot_run != 0) {
        call_platform_ot_run = 0;
           platform_ot_run();
    }
#if (SYS_FLOOD == ENABLED)
    else if (call_start_tx != 0) {
        call_start_tx = 0;
        start_tx(False);    // tx flood only, repeat tx
    }
#endif  /* SYS_FLOOD */

}

