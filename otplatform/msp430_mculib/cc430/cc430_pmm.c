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

    PMMCTL0_H   = 0xA5;
    svsmh_reg   = svsmh_cfg | ((unsigned short)Von << 8) | (unsigned short)Voffon;
    svsml_reg   = SVSMLCTL & 0x070F;
    svsml_reg  |= svsmh_reg & 0x08C0;   // Always disable SVML (useless)
    PMMIFG      = 0;
    PMMRIE      = 0;
    SVSMHCTL    = svsmh_reg;
    SVSMLCTL    = svsml_reg;
    while ((PMMIFG & (SVSMLDLYIFG+SVSMHDLYIFG)) != (SVSMLDLYIFG+SVSMHDLYIFG));

    PMMIFG      = 0;
    PMMRIE      = 0x0130;               //Always enable SVSL reset, SVMH interrupt, SVS/MH Delayed interrupt
    PMMCTL0_H   = 0x00;
}



void PMM_EnterLPM0(void) {
    //WDTA->CTL = WDTPW+WDTHOLD;                  // Stop WDT
    __bis_SR_register(LPM0_bits + GIE);         // Enter LPM0
    __no_operation();                           // For debugger
}


void PMM_EnterLPM1(void) {
/// In this implementation, LPM1 shuts off the following modules:
/// <LI> MCLK & CPU </LI>
/// <LI> FLL </LI>

    //WDTA->CTL = WDTPW+WDTHOLD;                  // Stop WDT
    __bis_SR_register(LPM1_bits + GIE);         // Enter LPM1
    __no_operation();                           // For debugger
}


void PMM_EnterLPM2(void) {
/// In this implementation, LPM2 shuts off the following modules below.
/// LPM2 will wake-up in <10us if the supply monitors are off or <110us
/// if they are on.
/// <LI> MCLK & CPU </LI>
/// <LI> FLL </LI>
/// <LI> Supply Voltage Monitors </LI>
/// <LI> SMCLK </LI>

    //WDTA->CTL   = WDTPW+WDTHOLD;                // Stop WDT
    UCS->CTL6  |= SMCLKOFF;                     // Kill SMCLK

    __bis_SR_register(LPM2_bits + GIE);         // Enter LPM2
    //__no_operation();                         // For debugger

    // Resume SMCLK
    UCS->CTL6  &= ~SMCLKOFF;
}



void PMM_EnterLPM3(void) {
/// In this implementation, LPM3 shuts off the following modules below.
/// LPM3 will wake-up in <10us if the supply monitors are off or <110us
/// if they are on.  This implementation of LPM3 will also ground all
/// GPIO to save power.
/// <LI> MCLK & CPU </LI>
/// <LI> FLL </LI>
/// <LI> Supply Voltage Monitors </LI>
/// <LI> SMCLK </LI>
/// <LI> All GPIO is floored to ground </LI>

///@note This seems to work OK with JTAG connected.  I have not tested
///      it with Spy-Bi-Wire

#if ( defined(DEBUG_ON) || defined(__DEBUG__) )
#   define _DEBUG_ON_PORTJ  1
#else
#   define _DEBUG_ON_PORTJ  0
#endif

    //u8 saved_douts[(CC430_TOTAL_PORTS-_DEBUG_ON_PORTJ)];
    //u8 saved_ddirs[(CC430_TOTAL_PORTS-_DEBUG_ON_PORTJ)];
    //u8 i;

    /// HOLD Watchdog: optional, could be removed
    //WDTA->CTL = WDTPW+WDTHOLD;

    /// Save GPIO dout & ddir settings, and set pins to ground during LPM3.
    ///@todo make sure setting P5 to FF doesn't mess clock.
    //for (i=(CC430_TOTAL_PORTS-_DEBUG_ON_PORTJ); i!=0; i--) {
    //    saved_douts[i]              = *cc430_pout_list[i];
    //    *((u8*)cc430_pout_list[i])  = 0x00;
    //    saved_ddirs[i]              = *cc430_pdir_list[i];
    //    *((u8*)cc430_pdir_list[i])  = 0xFF;
    //}

    __bis_SR_register(LPM3_bits + GIE);             // Enter LPM3
    __no_operation();

    /// Restore GPIO to known saved values
    //for (i=(CC430_TOTAL_PORTS-_DEBUG_ON_PORTJ); i!=0; i--) {
    //    *((volatile u8*)cc430_pout_list[i])  = saved_douts[i];
    //    *((volatile u8*)cc430_pdir_list[i])  = saved_ddirs[i];
    //}
    
#undef _DEBUG_ON_PORTJ
}




void PMM_EnterLPM4(void) {
    __bis_SR_register(LPM4_bits + GIE);
}

/*
void PMM_EnterLPM5(void) {
    // Currently unimplemented
}
*/

