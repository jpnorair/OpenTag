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
/** @file       /Platforms/CC430/cc430_lib/cc430_dma.c
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Dec 2009
  * @brief      Library resources for DMA peripheral
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#include "cc430_lib.h"

#ifdef __CC430__



void DMA_Init(DMAx_Type* DMAx, DMA_InitType* DMA_InitStruct) {
    
    
    if ( (u16)DMAx == (u16)DMA0 ) {
        DMACTL0 &= 0xFFE0;
        DMACTL0 |= DMA_InitStruct->TriggerSelect;
    }
    else if ( (u16)DMAx == (u16)DMA1 ) {
        DMACTL0 &= 0xE0FF;
        DMACTL0 |= (DMA_InitStruct->TriggerSelect << 8);
    }
    else if ( (u16)DMAx == (u16)DMA2 ) {
        DMACTL1 &= 0xFFE0;
        DMACTL1 |= DMA_InitStruct->TriggerSelect;
    }
    else {
        return;
    }
    
    DMAx->CTL   = DMA_InitStruct->Control;
    DMAx->SA_L  = DMA_InitStruct->SourceAddr;
    DMAx->DA_L  = DMA_InitStruct->DestinationAddr;
    DMAx->SZ    = DMA_InitStruct->TransferSize;
    
}




void DMA_Cmd(DMAx_Type* DMAx, FunctionalState NewState) {
    
    if (NewState == ENABLE) {
        DMAx->CTL |= 0x0010;
    }
    else {
        DMAx->CTL &= ~0x0010;
    }

}



void DMA_Request(DMAx_Type* DMAx) {
    DMAx->CTL |= 0x0001;
}




void DMA_ITConfig(DMAx_Type* DMAx, FunctionalState NewState) {

    if (NewState == ENABLE) {
        DMAx->CTL |= DMA_IT_IE;
    }
    else {
        DMAx->CTL &= ~DMA_IT_IE;
    }

}




bool DMA_GetFlagStatus(DMAx_Type* DMAx, u16 DMA_flag) {
    return (bool)( DMAx->CTL & DMA_flag );
}




void DMA_ClearFlag(DMAx_Type* DMAx, u16 DMA_flag) {
    DMAx->CTL &= ~DMA_flag;
}


#endif
