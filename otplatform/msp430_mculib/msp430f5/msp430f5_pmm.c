/*
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  * http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  */
/**
  * @file       /OTplatform/~MCU_MSP430/MSP430F5/msp430f5_pmm.c
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Apr 2012
  * @brief      Library resources for Power Management Module
  * @ingroup    MSP430F5 Library
  *
  ******************************************************************************
  */

#include "msp430f5_lib.h"


void SetVCoreUp (unsigned char level)        // Note: change level by one step only
{
  PMMCTL0_H = 0xA5;                         // Open PMM module registers for write access

  SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;     // Set SVS/M high side to new level

  SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;     // Set SVM new Level
  while ((PMMIFG & SVSMLDLYIFG) == 0);      // Wait till SVM is settled (Delay)
  PMMCTL0_L = PMMCOREV0 * level;            // Set VCore to x
  PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);        // Clear already set flags
  if ((PMMIFG & SVMLIFG))
    while ((PMMIFG & SVMLVLRIFG) == 0);     // Wait till level is reached

  SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;     // Set SVS/M Low side to new level
  PMMCTL0_H = 0x00;                         // Lock PMM module registers for write access
}



void SetVCoreDown (unsigned char level)
{
  PMMCTL0_H = 0xA5;                         // Open PMM module registers for write access
  SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;     // Set SVS/M Low side to new level
  while ((PMM->IFG & SVSMLDLYIFG) == 0);      // Wait till SVM is settled (Delay)
  PMMCTL0_L = (level * PMMCOREV0);          // Set VCore to 1.85 V for Max Speed.
  PMMCTL0_H = 0x00;                         // Lock PMM module registers for write access
}



void PMM_SetVCore (PMM_Vcore level) {
    unsigned char actLevel;

    // Note: change level by one step only
    do {
        actLevel = PMMCTL0_L & PMMCOREV_3;
        if (actLevel < level)
            SetVCoreUp(++actLevel);               // Set VCore (step by step)
        if (actLevel > level)
            SetVCoreDown(--actLevel);             // Set VCore (step by step)
    } while (actLevel != level);
}



void PMM_SetStdSVSM(unsigned short svsmh_cfg, SVS_Von Von, SVSM_Voffon Voffon) {
    unsigned short svsmh_reg;
    unsigned short svsml_reg;

    PMMCTL0_H    = 0xA5;
    svsmh_reg       = svsmh_cfg | ((unsigned short)Von << 8) | (unsigned short)Voffon;
    svsml_reg       = SVSMLCTL & 0x070F;
    svsml_reg      |= svsmh_reg & 0x08C0;   // Always disable SVML (useless)
    PMMIFG        = 0;
    PMMRIE        = 0;
    SVSMHCTL   = svsmh_reg;
    SVSMLCTL   = svsml_reg;
    while ((PMMIFG & (SVSMLDLYIFG+SVSMHDLYIFG)) != (SVSMLDLYIFG+SVSMHDLYIFG));

    PMMIFG        = 0;
    PMMRIE        = 0x0130;               //Always enable SVSL reset, SVMH interrupt, SVS/MH Delayed interrupt
    PMMCTL0_H    = 0x00;
}




void PMM_EnterLPM0(void) {

    WDTA->CTL = WDTPW+WDTHOLD;                   // Stop WDT

    __bis_SR_register(LPM0_bits);             // Enter LPM1
    __no_operation();                         // For debugger
}




void PMM_EnterLPM1(void) {
    // Currently unimplemented
}


void PMM_EnterLPM2(void) {
    // Currently unimplemented
}



void PMM_EnterLPM3(void) {

    WDTA->CTL   = WDTPW+WDTHOLD;                   // Stop WDT
    GPIO5->SEL |= BIT0 + BIT1;                     // Select XT1
    UCS->CTL6  |= XCAP_3;                          // Internal load cap

    // Loop until XT1,XT2 & DCO stabilizes
    do {
        UCS->CTL7 &= ~(XT1LFOFFG + DCOFFG);     // Clear LFXT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                      // Clear fault flags
    } while (SFRIFG1 & OFIFG);                  // Test oscillator fault flag

    UCSCTL6 &= ~(XT1DRIVE_3);                   // Xtal is now stable, reduce drive strength

    GPIO1->DOUT = 0x00;
    GPIO2->DOUT = 0x00;
    GPIO3->DOUT = 0x00;
    GPIO4->DOUT = 0x00;
    GPIO5->DOUT = 0x00;

    GPIO1->DDIR = 0xFF;
    GPIO2->DDIR = 0xFF;
    GPIO3->DDIR = 0xFF;
    GPIO4->DDIR = 0xFF;
    GPIO5->DDIR = 0xFC;
    GPIOJ->DOUT = 0x00;
    GPIOJ->DDIR = 0xFF;

    // Turn off SVSH, SVSM
//    PMMCTL0_H = 0xA5;
//    PMMSVSMHCTL = 0;
//    PMMSVSMLCTL = 0;
//    PMMCTL0_H = 0x00;

    __bis_SR_register(LPM3_bits);             // Enter LPM3
    __no_operation();                         // For debugger

    ///@todo put in part that waits for 3 Reference Clock cycles per erratum UCS7
}




void PMM_EnterLPM4(void) {
    __bis_SR_register(LPM4_bits + GIE);
}

/*
void PMM_EnterLPM5(void) {
    // Currently unimplemented
}
*/

