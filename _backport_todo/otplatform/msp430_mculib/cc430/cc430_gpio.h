/* Copyright 2009 JP Norair
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
/** @file       /Platforms/CC430/cc430_lib/cc430_gpio.h
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Dec 2009
  * @brief      Library resources for GPIO peripheral(s)
  * @ingroup    CC430 Library
  *
  * "GPIO" includes the "Digital I/O" Peripheral and "Port Mapping Controller"
  * of the CC430.
  ******************************************************************************
  */


#ifndef __CC430_LIB_GPIO_H
#define __CC430_LIB_GPIO_H

#include "cc430_types.h"
#include "cc430_map.h"

#ifdef __CC430__

typedef enum {
    GPIO_Mode_In_Floating   = 0x00,
    GPIO_Mode_In_Pullup     = 0x01,
    GPIO_Mode_In_Pulldown   = 0x02,
    GPIO_Mode_Out_Weak      = 0x03,
    GPIO_Mode_Out_Strong    = 0x04
} GPIOMode_Type;


typedef struct {
    u8              GPIO_PinMask;
    u8              GPIO_Values;
    GPIOMode_Type   GPIO_Mode;
} GPIO_InitType;



// Maskable Pin defs
// The 16 bit pins are almost certainly not implemented

#define GPIO_Pin_0                  0x01
#define GPIO_Pin_1                  0x02
#define GPIO_Pin_2                  0x04
#define GPIO_Pin_3                  0x08
#define GPIO_Pin_4                  0x10
#define GPIO_Pin_5                  0x20
#define GPIO_Pin_6                  0x40
#define GPIO_Pin_7                  0x80
#define GPIO_Pin_8                  0x0100
#define GPIO_Pin_9                  0x0200
#define GPIO_Pin_10                 0x0400
#define GPIO_Pin_11                 0x0800
#define GPIO_Pin_12                 0x1000
#define GPIO_Pin_13                 0x2000
#define GPIO_Pin_14                 0x4000
#define GPIO_Pin_15                 0x8000


// Maskable Rising / Falling Edge defs

#define GPIO_IT_RisingEdge_0        0x00
#define GPIO_IT_RisingEdge_1        0x00
#define GPIO_IT_RisingEdge_2        0x00
#define GPIO_IT_RisingEdge_3        0x00
#define GPIO_IT_RisingEdge_4        0x00
#define GPIO_IT_RisingEdge_5        0x00
#define GPIO_IT_RisingEdge_6        0x00
#define GPIO_IT_RisingEdge_7        0x00

#define GPIO_IT_FallingEdge_0       0x01
#define GPIO_IT_FallingEdge_1       0x02
#define GPIO_IT_FallingEdge_2       0x04
#define GPIO_IT_FallingEdge_3       0x08
#define GPIO_IT_FallingEdge_4       0x10
#define GPIO_IT_FallingEdge_5       0x20
#define GPIO_IT_FallingEdge_6       0x40
#define GPIO_IT_FallingEdge_7       0x80


// Port Remapping Definitions

#define GPIO_Remap_NONE             ((u8)0)
#define GPIO_Remap_CBOUT0           ((u8)1)
#define GPIO_Remap_TA0CLK           ((u8)1)
#define GPIO_Remap_CBOUT1           ((u8)2)
#define GPIO_Remap_TA1CLK           ((u8)2)
#define GPIO_Remap_ACLK             ((u8)3)
#define GPIO_Remap_MCLK             ((u8)4)
#define GPIO_Remap_SMCLK            ((u8)5)
#define GPIO_Remap_RTCCLK           ((u8)6)
#define GPIO_Remap_ADC12CLK         ((u8)7)
#define GPIO_Remap_DMAE0            ((u8)7)
#define GPIO_Remap_SVMOUT           ((u8)8)
#define GPIO_Remap_TA0CCR0A         ((u8)9)
#define GPIO_Remap_TA0CCR1A         ((u8)10)
#define GPIO_Remap_TA0CCR2A         ((u8)11)
#define GPIO_Remap_TA0CCR3A         ((u8)12)
#define GPIO_Remap_TA0CCR4A         ((u8)13)
#define GPIO_Remap_TA1CCR0A         ((u8)14)
#define GPIO_Remap_TA1CCR1A         ((u8)15)
#define GPIO_Remap_TA1CCR2A         ((u8)16)
#define GPIO_Remap_UCA0RXD          ((u8)17)
#define GPIO_Remap_UCA0SOMI         ((u8)17)
#define GPIO_Remap_UCA0TXD          ((u8)18)
#define GPIO_Remap_UCA0SIMO         ((u8)18)
#define GPIO_Remap_UCA0CLK          ((u8)19)
#define GPIO_Remap_UCB0STE          ((u8)19)
#define GPIO_Remap_UCB0SOMI         ((u8)20)
#define GPIO_Remap_UCB0SCL          ((u8)20)
#define GPIO_Remap_UCB0SIMO         ((u8)21)
#define GPIO_Remap_UCB0SDA          ((u8)21)
#define GPIO_Remap_UCB0CLK          ((u8)22)
#define GPIO_Remap_UCA0STE          ((u8)22)
#define GPIO_Remap_RFGDO0           ((u8)23)
#define GPIO_Remap_RFGDO1           ((u8)24)
#define GPIO_Remap_RFGDO2           ((u8)25)
#define GPIO_Remap_ANALOG           ((u8)31)


/*
I haven't yet determined an elegant way to do most types of GPIO operations
via a HAL, because the MSP430 core has a kind-of annoying GPIO register
organization.

void GPIO_Init(u8 GPIOx, GPIO_InitType* GPIO_InitStruct);

void GPIO_ITConfig(u8 GPIOx, u8 GPIO_PinMask, u8 GPIO_EdgeMask, FunctionalState NewState );

void GPIO_ClearITPendingBit(u8 GPIOx, u8 GPIO_PinMask);

void GPIO_ClearITStatus(u8 GPIOx, u8 GPIO_PinMask);

bool GPIO_GetITStatus(u8 GPIOx, u8 GPIO_PinMask);

void GPIO_Write(u8 GPIOx, u8 GPIO_PinMask, u8 Values);

u8 GPIO_Read(u8 GPIOx, u8 GPIO_PinMask);
*/

bool GPIO_RemapUnlock( );

void GPIO_RemapControl( FunctionalState NewState );

bool GPIO_RemapStatus();

void GPIO_Remap(u8 Port_Number, u8 Pin_Number, u8 Remap_Index);


#endif

#endif


