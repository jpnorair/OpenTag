/* BSD License
  */
/**
  * @file       /OTplatform/~MCU_MSP430/CC430/cc430_pmm.c
  * @author     JP Norair
  * @version    V1.0
  * @date       1 May 2012
  * @brief      Library resources for Power Management Module
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */

#include "cc430_pmm.h"
#include "cc430_lib.h"


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
///@note it is probably best not to use this with JTAG connected (i.e. in 
/// debugging builds) but it should actually work OK.

    static const u8* dout_table[] = {
        (u8*)0x0202,     //Port 1 DOUT
        (u8*)0x0223,     //Port 2 DOUT
        (u8*)0x0222,     //Port 3 DOUT
        (u8*)0x0223,     //Port 4 DOUT
        (u8*)0x0242,     //Port 5 DOUT
        (u8*)0x0322      //Port J DOUT
    };
    
    static const u8* ddir_table[] = {
    	(u8*)0x0204,     //Port 1 DOUT
    	(u8*)0x0225,     //Port 2 DOUT
    	(u8*)0x0224,     //Port 3 DOUT
    	(u8*)0x0225,     //Port 4 DOUT
    	(u8*)0x0244,     //Port 5 DOUT
    	(u8*)0x0324      //Port J DOUT
    };
    
    static const u8 ddir_set[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF };
    
    u8 saved_douts[6];
    u8 saved_ddirs[6];
    s16 i;

    WDTA->CTL   = WDTPW+WDTHOLD;            // Stop WDT
    GPIO5->SEL |= BIT0 + BIT1;              // Assure Selection of XT1
    UCS->CTL6  |= XCAP_3;                   // Internal load cap

    // Loop until XT1,XT2 & DCO stabilizes
    do {
        UCS->CTL7 &= ~(XT1LFOFFG + DCOFFG); // Clear LFXT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                  // Clear fault flags
    } 
    while (SFRIFG1 & OFIFG);                // Test oscillator fault flag
    UCSCTL6 &= ~(XT1DRIVE_3);               // Xtal is now stable, reduce drive strength

    /// Save GPIO dout & ddir settings, and set to low-power running.
    /// GPIO5 is specials, because it drives the oscillators
    for (i=5; i>0; i--) {
        saved_douts[i]              = *dout_table[i];
        saved_ddirs[i]              = *ddir_table[i];
        *((u8*)dout_table[i])    = 0x00;
        *((u8*)ddir_table[i])    = ddir_set[i];
    }
    
    // Turn off SVSH, SVSM
//    PMMCTL0_H = 0xA5;
//    PMMSVSMHCTL = 0;
//    PMMSVSMLCTL = 0;
//    PMMCTL0_H = 0x00;

    __bis_SR_register(LPM3_bits);             // Enter LPM3
    __no_operation();
    
    /// wait for 3 Reference Clock cycles per erratum UCS7
    __delay_cycles(3);

    /// Restore GPIO
    for (i=5; i>0; i--) {
        *((u8*)dout_table[i])    = saved_douts[i];
        *((u8*)ddir_table[i])    = saved_ddirs[i];
    }
}




void PMM_EnterLPM4(void) {
    __bis_SR_register(LPM4_bits + GIE);
}

/*
void PMM_EnterLPM5(void) {
    // Currently unimplemented
}
*/

