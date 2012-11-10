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
/**
  * @file       /Platforms/CC430/cc430_lib/cc430_tima.h
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Dec 2009
  * @brief      Library resources for Timer A peripherals
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#ifndef __CC430_LIB_TIMA_H
#define __CC430_LIB_TIMA_H

#include "cc430_map.h"
#ifdef __CC430__

/* TIM Time Base Init structure definition */
typedef enum {
    CCChannel0    = 0,
    CCChannel1    = 2,
    CCChannel2    = 4,
    CCChannel3    = 6,
    CCChannel4    = 8,
    CCChannel5    = 10,
    CCChannel6    = 12
} TIMA_CCChannelType;

typedef struct {
    u16 Control;   
    u16 Prescaler;
    u16 Period;
} TIMA_BaseInitType;

typedef struct {
    u16                 CCMode;   
    u16                 CCInput;
    TIMA_CCChannelType  CCChannel;
} TIMA_CCInitType;




// Control Settings (TAxCTL)
#define TIMA_Ctl_Clock_TAxCLK           ((u16)0x0000)
#define TIMA_Ctl_Clock_ACLK             ((u16)0x0100)
#define TIMA_Ctl_Clock_SMCLK            ((u16)0x0200)
#define TIMA_Ctl_Clock_nTAxCLK          ((u16)0x0300)

#define TIMA_Ctl_Divider_1              ((u16)0x0000)
#define TIMA_Ctl_Divider_2              ((u16)0x0040)
#define TIMA_Ctl_Divider_4              ((u16)0x0080)
#define TIMA_Ctl_Divider_8              ((u16)0x00C0)

#define TIMA_Ctl_Mode_Stop              ((u16)0x0000)
#define TIMA_Ctl_Mode_Up                ((u16)0x0010)
#define TIMA_Ctl_Mode_Continuous        ((u16)0x0020)
#define TIMA_Ctl_Mode_UpDown            ((u16)0x0030)


// Prescaler Settings (TAxEX0)
#define TIMA_Prescaler_1                ((u16)0x0000)
#define TIMA_Prescaler_2                ((u16)0x0001)
#define TIMA_Prescaler_3                ((u16)0x0002)
#define TIMA_Prescaler_4                ((u16)0x0003)
#define TIMA_Prescaler_5                ((u16)0x0004)
#define TIMA_Prescaler_6                ((u16)0x0005)
#define TIMA_Prescaler_7                ((u16)0x0006)
#define TIMA_Prescaler_8                ((u16)0x0007)


// Period Settings (TAxR)
// 0000 - FFFF



// CCMode Setup (TAxCCTLn)
#define TIMA_CCMode_Capture             ((u16)0x0100)
#define TIMA_CCMode_Compare             ((u16)0x0000)

#define TIMA_CCMode_Input_A             ((u16)0x0000)
#define TIMA_CCMode_Input_B             ((u16)0x1000)
#define TIMA_CCMode_Input_GND           ((u16)0x2000)
#define TIMA_CCMode_Input_Vcc           ((u16)0x3000)


// CCInput Setup (TAxCCTLn)
#define TIMA_CCInput_Edge_Off           ((u16)0x0000)
#define TIMA_CCInput_Edge_Rising        ((u16)0x4000)
#define TIMA_CCInput_Edge_Falling       ((u16)0x8000)
#define TIMA_CCInput_Edge_Double        ((u16)0xC000)

#define TIMA_CCInput_Sync_Off           ((u16)0x0000)
#define TIMA_CCInput_Sync_On            ((u16)0x0800)

// CCOutput Setup (TAxCCTLn)
#define TIMA_CCOutput_Mode_OUT          ((u16)0x0000)
#define TIMA_CCOutput_Mode_Set          ((u16)0x0020)
#define TIMA_CCOutput_Mode_TogReset     ((u16)0x0040)
#define TIMA_CCOutput_Mode_SetReset     ((u16)0x0060)
#define TIMA_CCOutput_Mode_Toggle       ((u16)0x0080)
#define TIMA_CCOutput_Mode_Reset        ((u16)0x00A0)
#define TIMA_CCOutput_Mode_ToggleSet    ((u16)0x00C0)
#define TIMA_CCOutput_Mode_ResetSet     ((u16)0x00E0)



//TIMA Flags
#define TIMA_FLG_TACLR                  ((u16)0x0004)
#define TIMA_FLG_IFG                    ((u16)0x0001)

//TIMA CC Flags
#define TIMA_FLG_CC_SCCI                ((u16)0x0200)
#define TIMA_FLG_CC_CCI                 ((u16)0x0008)
#define TIMA_FLG_CC_OUT                 ((u16)0x0004)
#define TIMA_FLG_CC_COV                 ((u16)0x0002)
#define TIMA_FLG_CC_CCIFG               ((u16)0x0001)



// TIMA Interrupt Set/Clear Bits
#define TIMA_IT_Update                  ((u16)0x0002)
#define TIMA_IT_CC                      ((u16)0x0010)
#define TIMA_IT_ALL                     ( TIMA_IT_Update | TIMA_IT_CC )





void TIMA_BaseInit(TIMA_Type *TIMAx, TIMA_BaseInitType *TIMA_InitStruct );
void TIMA_CCInit(TIMA_Type *TIMAx, TIMA_CCInitType *TIMA_InitStruct );
void TIMA_Cmd(TIMA_Type *TIMAx, u16 TIMA_Ctl_Mode);
void TIMA_ITConfig(TIMA_Type *TIMAx, FunctionalState NewState);
void TIMA_CCITConfig(TIMA_Type *TIMAx, TIMA_CCChannelType Channelx, FunctionalState NewState);
bool TIMA_GetITStatus(TIMA_Type *TIMAx);
bool TIMA_GetCCITStatus(TIMA_Type *TIMAx, TIMA_CCChannelType Channelx);
void TIMA_ClearITPendingBit(TIMA_Type *TIMAx);
void TIMA_ClearCCITPendingBit(TIMA_Type *TIMAx, TIMA_CCChannelType Channelx);
bool TIMA_GetCCFlagStatus(TIMA_Type *TIMAx, TIMA_CCChannelType Channelx, u16 TIMA_Flag);
void TIMA_ClearCCFlag(TIMA_Type *TIMAx, TIMA_CCChannelType Channelx, u16 TIMA_Flag);
void TIMA_SetCCOUT(TIMA_Type *TIMAx, TIMA_CCChannelType Channelx, u16 value);




#endif
#endif
