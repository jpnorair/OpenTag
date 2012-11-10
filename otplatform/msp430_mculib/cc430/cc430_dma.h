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
/** @file       /Platforms/CC430/cc430_lib/cc430_dma.h
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Dec 2009
  * @brief      Library resources for DMA peripheral
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#ifndef __CC430_LIB_DMA_H
#define __CC430_LIB_DMA_H

#include "cc430_map.h"

#ifdef __CC430__

typedef struct {
    u16 SourceAddr;
    u16 DestinationAddr;
    u16 TransferSize;
    u16 Control;
    u16 TriggerSelect;
    u16 Options;
} DMA_InitType;



// DMA_Control Settings (in register DMAxCTL)

#define DMA_Mode_Single                 ((u16)0x0000)
#define DMA_Mode_Block                  ((u16)0x1000)
#define DMA_Mode_BurstBlock             ((u16)0x3000)
#define DMA_Mode_RepeatedSingle         ((u16)0x4000)
#define DMA_Mode_RepeatedBlock          ((u16)0x5000)
#define DMA_Mode_RepeatedBurstBlock     ((u16)0x7000)

#define DMA_DestinationInc_Enable       ((u16)0x0C00)
#define DMA_DestinationInc_Decrement    ((u16)0x0800)
#define DMA_DestinationInc_Disable      ((u16)0x0000)

#define DMA_SourceInc_Enable            ((u16)0x0300)
#define DMA_SourceInc_Decrement         ((u16)0x0200)
#define DMA_SourceInc_Disable           ((u16)0x0000)

#define DMA_DestinationDataSize_Byte    ((u16)0x0080)
#define DMA_DestinationDataSize_Word    ((u16)0x0000)
#define DMA_SourceDataSize_Byte         ((u16)0x0040)
#define DMA_SourceDataSize_Word         ((u16)0x0000)

#define DMA_TriggerLevel_RisingEdge     ((u16)0x0000)
#define DMA_TriggerLevel_High           ((u16)0x0020)



// DMA_TriggerSelect Settings (in register DMACTLn)
// These triggers are at least valid for cc430f513x and cc430f61xx

#define DMA_Trigger_DMAREQ              ((u16)0x0000)
#define DMA_Trigger_TA0CCR0             ((u16)0x0001)
#define DMA_Trigger_TA0CCR2             ((u16)0x0002)
#define DMA_Trigger_TA1CCR0             ((u16)0x0003)
#define DMA_Trigger_TA1CCR2             ((u16)0x0004)
#define DMA_Trigger_RFRXIFG             ((u16)0x000E)
#define DMA_Trigger_RFTXIFG             ((u16)0x000F)
#define DMA_Trigger_UCA0RXIFG           ((u16)0x0010)
#define DMA_Trigger_UCA0TXIFG           ((u16)0x0011)
#define DMA_Trigger_UCB0RXIFG           ((u16)0x0012)
#define DMA_Trigger_UCB0TXIFG           ((u16)0x0013)
#define DMA_Trigger_ADC12IFGx           ((u16)0x0018)
#define DMA_Trigger_MPYready            ((u16)0x001D)
#define DMA_Trigger_DMA2IFG             ((u16)0x001E)
#define DMA_Trigger_DMAE0               ((u16)0x001F)



// DMA_Options Settings (in register DMACTL4)

#define DMA_Options_RMWEnable           ((u16)0x0000)
#define DMA_Options_RMWDisable          ((u16)0x0004)
#define DMA_Options_RoundRobinDisable   ((u16)0x0000)
#define DMA_Options_RoundRobinEnable    ((u16)0x0002)
#define DMA_Options_ENMIDisable         ((u16)0x0000)
#define DMA_Options_ENMIEnable          ((u16)0x0001)



// DMA FLAGS
#define DMA_FLAG_IFG                    ((u16)0x0008)
#define DMA_FLAG_ABORT                  ((u16)0x0002)


// DMA Interrupt Enable
#define DMA_IT_IE                       ((u16)0x0004)





void DMA_Init(DMAx_Type* DMAx, DMA_InitType* DMA_InitStruct);

void DMA_Cmd(DMAx_Type* DMAx, FunctionalState NewState);

void DMA_Request(DMAx_Type* DMAx);

void DMA_ITConfig(DMAx_Type* DMAx, FunctionalState NewState);

bool DMA_GetFlagStatus(DMAx_Type* DMAx, u16 DMA_flag);

void DMA_ClearFlag(DMAx_Type* DMAx, u16 DMA_flag);


#endif

#endif


