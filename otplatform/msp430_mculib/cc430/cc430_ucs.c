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
/** @file       /Platforms/CC430/cc430_lib/cc430_ucs.c
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Dec 2009
  * @brief      Library resources for UCS
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#include "cc430_lib.h"
#ifdef __CC430__

/** Startup routine for 32kHz Cristal on LFXT1
 * \param xtdrive: Bits defining the LFXT drive mode after startup
 */
void LFXT_Start(u16 xtdrive);


/** Startup routine for 32kHz Cristal on LFXT1 with timeout counter
  * \param xtdrive: Bits defining the LFXT drive mode after startup
  * \param timeout: value for the timeout counter
  */
u16 LFXT_Start_Timeout(u16 xtdrive, u16 timeout);


/** Startup routine for XT1
  * \param xtdrive: Bits defining the XT drive mode
  */
void XT1_Start(u16 xtdrive);


/** Startup routine for XT1 with timeout counter
  * \param xtdrive: Bits defining the XT drive mode
  * \param timeout: value for the timeout counter
  */
u16 XT1_Start_Timeout(u16 xtdrive, u16 timeout);


/** Use XT1 in Bypasss mode
  */
void XT1_Bypass(void);


/** Startup routine for XT2
  * \param xtdrive: Bits defining the XT drive mode
  */
void XT2_Start(u16 xtdrive);


/** Startup routine for XT2 with timeout counter
  * \param xtdrive: Bits defining the XT drive mode
  * \param timeout: value for the timeout counter
  */
u16 XT2_Start_Timeout(u16 xtdrive, u16 timeout);


/** Use XT2 in Bypasss mode for MCLK
  */
void XT2_Bypass(void);



/** Initializes FLL of the UCS
  * \param fsystem  required system frequency (MCLK) in kHz
  * \param ratio    ratio between fsystem and FLLREFCLK
  */
void Init_FLL(u16 fsystem, u16 ratio);






// Public Functions



void UCS_FLLInit(FLLInit_Type* FLLInitStruct) {
    u16 CTL3Value;
    
    // Set Source (Reference Clock) and Reference Divider
    CTL3Value   = (u16)(FLLInitStruct->Source) << 4;
    CTL3Value  |= (u16)(FLLInitStruct->SourceDiv);
    UCS->CTL3   = CTL3Value;

    // Initialization Routine -- wait for settling
    UCS_FLL_settle(FLLInitStruct->Frequency, FLLInitStruct->Multiplier);
}

void UCS_FLLCmd(FunctionalState NewState) { 
    if (NewState != DISABLE)    UCS->CTL1 &= ~UCS_CTL1_DISMOD;
    else                        UCS->CTL1 |= UCS_CTL1_DISMOD;
}

void UCS_LFXT1Init( ) {
    //UCS->CTL6 &= ~UCS_CTL6_XT1OFF;
    LFXT_Start( UCS_CTL6_XT1DRIVE_0 );
}



void UCS_ClockInit(ClockInit_Type* ClockInitStruct) {
    u16 MaskBits;
    u16 DataBits;
    
    MaskBits    = 0x0007 << (u8)(ClockInitStruct->Clock);
    
    DataBits    = (u16)(ClockInitStruct->Source) << (u8)(ClockInitStruct->Clock);
    UCS->CTL4  &= ~MaskBits;
    UCS->CTL4  |= DataBits;
    
    DataBits    = (u16)(ClockInitStruct->Div) << (u8)(ClockInitStruct->Clock);
    UCS->CTL5  &= ~MaskBits;
    UCS->CTL5  |= DataBits;
}




void UCS_XT1Config(u16 ConfigBits ) {   
    UCS->CTL6 = ConfigBits;
}




void UCS_XT1Cmd(FunctionalState NewState) {
    if (NewState == DISABLE)    UCS->CTL6 |= UCS_CTL6_XT1OFF;
    else                        UCS->CTL6 &= ~UCS_CTL6_XT1OFF;
    
    UCS->CTL7 &= ~(DCOFFG+XT1LFOFFG+XT1HFOFFG+XT2OFFG);     // Clear OSC fault Flags
    SFRIFG1 &= ~OFIFG;                                    // Clear OFIFG fault flag
}

void UCS_XT2Cmd(FunctionalState NewState) {
    if (NewState == DISABLE)    UCS->CTL6 |= UCS_CTL6_XT2OFF;
    else                        UCS->CTL6 &= ~UCS_CTL6_XT2OFF;
    
    UCS->CTL7 &= ~(DCOFFG+XT1LFOFFG+XT1HFOFFG+XT2OFFG);     // Clear OSC fault Flags
    SFRIFG1 &= ~OFIFG;                                    // Clear OFIFG fault flag
}

void UCS_SMCLKCmd(FunctionalState NewState) {
    if (NewState == DISABLE)    UCS->CTL6 |= UCS_CTL6_SMCLKOFF;
    else                        UCS->CTL6 &= ~UCS_CTL6_SMCLKOFF;
}

void UCS_RequestCmd(u16 CREQMask, FunctionalState NewState) { 
    if (NewState != DISABLE)    UCS->CTL8 |= CREQMask;
    else                        UCS->CTL8 &= ~CREQMask;
}




u16 UCS_GetFlagStatus(u16 FlagMask) {
    return (u16)(UCS->CTL7 & FlagMask);
}

void UCS_ClearFlag(u16 FlagMask) {
    UCS->CTL7 &= ~FlagMask;
}









// Private Functions

//************************************************************************
// Check and define required Defines
//************************************************************************

#ifndef XT1LFOFFG               // Defines if not available in header file
#define XT1LFOFFG 0
#endif
#ifndef XT1HFOFFG               // Defines if not available in header file
#define XT1HFOFFG 0
#endif
#ifndef XT2OFFG                 // Defines if not available in header file
#define XT2OFFG 0
#endif




void LFXT_Start(u16 xtdrive) {
/// Put LFXT into max drive for startup, then wait for stabilization, then
/// put it into the specified drive mode after stabilization.

	UCS->CTL6 |= 0xC0;

    while (UCSCTL7 & XT1LFOFFG) {
        UCSCTL7  &= ~(XT1LFOFFG);
        SFRIFG1 &= ~OFIFG;
    }
  
    UCS->CTL6 = (UCS->CTL6 & ~(XT1DRIVE_3)) | (xtdrive); // set Drive mode
}




u16 LFXT_Start_Timeout(u16 xtdrive, u16 timeout) {
/// Similar to LFXT_Start(), except including a timeout.

    UCSb->CTL6_L |= 0xC0; // Highest drive setting for XT1 startup
    while ((UCS->CTL7 & XT1LFOFFG) && timeout--) {   // check OFIFG fault flag
        UCS->CTL7  &= ~(XT1LFOFFG);
        SFRIFG1 &= ~OFIFG;
    }
  
    UCS->CTL6 = (UCS->CTL6 & ~(XT1DRIVE_3)) |(xtdrive); // set Drive mode
  
    return (timeout) ? UCS_STATUS_OK : UCS_STATUS_ERROR;
}



void XT1_Start(u16 xtdrive) {
    UCS->CTL6 &= ~(XT1OFF & XT1DRIVE_3);  // enable XT1
    UCS->CTL6 |= (XTS & xtdrive);         // enable XT1 and set XT1Drive

    while (UCS->CTL7 & (XT1LFOFFG+XT1HFOFFG)) {
        UCS->CTL7  &= ~(XT1LFOFFG+XT1HFOFFG);
        SFRIFG1 &= ~OFIFG;
    }
}



u16 XT1_Start_Timeout(u16 xtdrive, u16 timeout) {
    UCS->CTL6 &= ~(XT1OFF & XT1DRIVE_3);  // enable XT1
    UCS->CTL6 |= (XTS & xtdrive);         // enable XT1 and set XT1Drive

    while (UCS->CTL7 & (XT1LFOFFG+XT1HFOFFG)) {
            UCS->CTL7  &= ~(XT1LFOFFG+XT1HFOFFG);
            SFRIFG1 &= ~OFIFG;
    }

    return (timeout) ? UCS_STATUS_OK : UCS_STATUS_ERROR;
}




void XT1_Bypass(void) {
    UCS->CTL6 |= XT1BYPASS;

    while (UCS->CTL7 & (XT1LFOFFG+XT1HFOFFG)) {   // check OFIFG fault flag
        UCS->CTL7  &= ~(XT1LFOFFG + XT1HFOFFG); // Clear OSC flaut Flags
        SFRIFG1 &= ~OFIFG;        // Clear OFIFG fault flag
    }
}




void XT2_Start(u16 xtdrive) {
    UCS->CTL6 &= ~(XT2OFF+XT1DRIVE_3);    // enable XT2 & wipe drive value
    UCS->CTL6 |= (xtdrive);               // Set XT2Drive


    while (UCS->CTL7 & XT2OFFG) {
        UCS->CTL7 &= ~(XT2OFFG);
        __delay_cycles(5000);
        SFRIFG1 &= ~OFIFG;        // Clear OFIFG fault flag
    }
}



u16 XT2_Start_Timeout(u16 xtdrive, u16 timeout) {
	UCS->CTL6 &= ~(XT2OFF+XT1DRIVE_3);    // enable XT2 & wipe drive value
	UCS->CTL6 |= (xtdrive);               // Set XT2Drive

	while ((UCS->CTL7 & XT2OFFG) && timeout--) {
	    UCS->CTL7 &= ~(XT2OFFG);
	    __delay_cycles(5000);
	    SFRIFG1 &= ~OFIFG;
	}

	return (timeout) ? UCS_STATUS_OK : UCS_STATUS_ERROR;
}




void XT2_Bypass(void) {
#ifdef XT2BYPASS              // on devices without XT2 this function will be empty
    UCS->CTL6 |= XT2BYPASS;

    while (UCS->CTL7 & XT2OFFG) {   // check OFIFG fault flag
       UCS->CTL7 &= ~XT2OFFG; // Clear OSC flaut Flags
       SFRIFG1 &= ~OFIFG;        // Clear OFIFG fault flag
    }
#endif
}




void UCS_FLL_settle(u16 fsystem, u16 ratio) {
    volatile u16 x = ratio * 32;
  
    Init_FLL(fsystem, ratio);
    while(x--) {
        __delay_cycles(30);
    }
}




void Init_FLL(u16 fsystem, u16 ratio) {

    // save actual state of FLL loop control
    // Have at least a divider of 2
    u16 srRegisterState = __get_SR_register() & SCG0;
    u16 d               = ratio;
    u16 dco_div_bits    = FLLD__2;
    //u16 mode            = 0;

    if (fsystem > 16000){
    	d >>= 1;
        //mode = 1;
    }
    else {
    	fsystem <<= 1;
    }

    while (d > 512) {
        dco_div_bits    = dco_div_bits + FLLD0;
        d             >>= 1;
    }

    __bis_SR_register(SCG0);                // Disable FLL
    UCS->CTL0   = 0x000;                    // Set DCO to lowest Tap
    UCS->CTL2  &= ~(0x3FF);                 // Reset FN bits
    UCS->CTL2   = dco_div_bits | (d-1);

    ///@todo Could easily optimize this by using an enum for fsystem
    if (fsystem <= 630)         UCS->CTL1= DCORSEL_0;
    else if (fsystem <  1250)   UCS->CTL1= DCORSEL_1;
    else if (fsystem <  2500)   UCS->CTL1= DCORSEL_2;
    else if (fsystem <  5000)   UCS->CTL1= DCORSEL_3;
    else if (fsystem <  10000)  UCS->CTL1= DCORSEL_4;
    else if (fsystem <  20000)  UCS->CTL1= DCORSEL_5;
    else if (fsystem <  40000)  UCS->CTL1= DCORSEL_6;
    else                        UCS->CTL1= DCORSEL_7;

    __bic_SR_register(SCG0);                // Re-enable FLL

    while (UCS->CTL7 & DCOFFG) {
        UCS->CTL7 &= ~DCOFFG;
        SFRIFG1 &= ~OFIFG;
    }

    __bis_SR_register(srRegisterState);     // Restore previous SCG0

    //if (mode == 1)                           		  // fsystem > 16000
    //    SELECT_MCLK_SMCLK(SELM__DCOCLK + SELS__DCOCLK);       // select DCOCLK
    //else
    //    SELECT_MCLK_SMCLK(SELM__DCOCLKDIV + SELS__DCOCLKDIV); // selcet DCODIVCLK


} // End of fll_init()

#endif
