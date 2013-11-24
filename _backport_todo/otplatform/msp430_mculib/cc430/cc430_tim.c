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
  * @file       /Platforms/CC430/cc430_lib/cc430_tim.c
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Dec 2009
  * @brief      Library resources for Timer A/B peripherals
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#include "cc430_lib.h"
#ifdef __CC430__



void TIMA_BaseInit(TIMA_Type *TIMAx, TIMA_BaseInitType *TIMA_InitStruct ) {
    
    // Reset Control
    TIMAx->CTL  = (TIMA_InitStruct->Control | TIMA_FLG_TACLR);
    
    // Prescaler (Input Divider)
    TIMAx->EX0  = TIMA_InitStruct->Prescaler;
    
    // Set Period
    TIMAx->R    = TIMA_InitStruct->Period;
}




void TIMA_CCInit(TIMA_Type *TIMAx, TIMA_CCInitType *TIMA_InitStruct ) {
    u16* CCTLn;
    
    CCTLn = (u16*)( (u16)&(TIMAx->CCTL0) + (u16)(TIMA_InitStruct->CCChannel) );
    
    // Clear & Set
    *CCTLn &= ~0xF9F0;
    *CCTLn = TIMA_InitStruct->CCMode | TIMA_InitStruct->CCInput;
}




void TIMA_Cmd(TIMA_Type *TIMAx, u16 TIMA_Ctl_Mode) {

    // Clear & Set
    TIMAx->CTL  &= ~0x0030;
    TIMAx->CTL  |= (TIMA_Ctl_Mode | TIMA_FLG_TACLR);
}




void TIMA_ITConfig(TIMA_Type *TIMAx, FunctionalState NewState) {

    // Update Interrupt
    if (NewState == DISABLE) {
        TIMAx->CTL &= ~(TIMA_FLG_IFG | TIMA_IT_Update);
    }
    else {
        TIMAx->CTL |= TIMA_IT_Update;
    }
    
}


void TIMA_CCITConfig(TIMA_Type *TIMAx, TIMA_CCChannelType Channelx, FunctionalState NewState) {
    u16* CCTLn;
    CCTLn = (u16*)( (u16)&(TIMAx->CCTL0) + (u16)Channelx );
    
    // Capture Compare Interrupt
    if (NewState == DISABLE) {
        *CCTLn &= ~TIMA_IT_CC;
    }
    else {
        *CCTLn |= TIMA_IT_CC;
    }
    
}





bool TIMA_GetITStatus(TIMA_Type *TIMAx) {

    // Update Interrupt
    return (bool)( TIMAx->CTL & TIMA_FLG_IFG );
}




bool TIMA_GetCCITStatus(TIMA_Type *TIMAx, TIMA_CCChannelType Channelx) {
    u16* CCTLn;
    CCTLn = (u16*)( (u16)&(TIMAx->CCTL0) + (u16)Channelx );
    
    // Capture Compare Interupt
    return (bool)( *CCTLn & TIMA_FLG_CC_CCIFG );
}




void TIMA_ClearITPendingBit(TIMA_Type *TIMAx) {

    // Update Interrupt
    TIMAx->CTL &= ~TIMA_FLG_IFG;
}



void TIMA_ClearCCITPendingBit(TIMA_Type *TIMAx, TIMA_CCChannelType Channelx) {
    u16* CCTLn;
    CCTLn = (u16*)( (u16)&(TIMAx->CCTL0) + (u16)Channelx );
    
    // Capture Compare Interupt
    *CCTLn &= ~TIMA_FLG_CC_CCIFG;
}




bool TIMA_GetCCFlagStatus(TIMA_Type *TIMAx, TIMA_CCChannelType Channelx, u16 TIMA_Flag) {
    u16* CCTLn;
    CCTLn = (u16*)( (u16)&(TIMAx->CCTL0) + (u16)Channelx );

    return (bool)( *CCTLn & TIMA_Flag );
}





void TIMA_ClearCCFlag(TIMA_Type *TIMAx, TIMA_CCChannelType Channelx, u16 TIMA_Flag) {
    u16* CCTLn;
    CCTLn = (u16*)( (u16)&(TIMAx->CCTL0) + (u16)Channelx );
    
    *CCTLn &= ~TIMA_Flag; 
}




// value is 0 or 1 (non zero)
void TIMA_SetCCOUT(TIMA_Type *TIMAx, TIMA_CCChannelType Channelx, u16 value) {
    u16* CCTLn;
    CCTLn = (u16*)( (u16)&(TIMAx->CCTL0) + (u16)Channelx );

    if ( value == 0 ) {
        *CCTLn &= ~TIMA_FLG_CC_OUT;
    }
    else {
        *CCTLn |= TIMA_FLG_CC_OUT;
    }
}

#endif


