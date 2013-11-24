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
/** @file       /Platforms/CC430/cc430_lib/cc430_ucs.h
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Dec 2009
  * @brief      Library resources for Unified Clock System
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#ifndef __CC430_LIB_UCS_H
#define __CC430_LIB_UCS_H

#include "cc430_lib.h"
#ifdef __CC430__

//*************************************************************************
//* MACROS
//**************************************************************************/

#define FLLSRC_XT1CLK       (0<<4)
#define FLLSRC_REFOCLK      (2<<4)
#define FLLSRC_XT2CLK       (5<<4)
#define FLLDIV_1            (0)
#define FLLDIV_2            (1)
#define FLLDIV_4            (2)
#define FLLDIV_8            (3)
#define FLLDIV_12           (4)
#define FLLDIV_16           (5)

#define ACLKSRC_XT1CLK      (0<<8)
#define ACLKSRC_VLOCLK      (1<<8)
#define ACLKSRC_REFOCLK     (2<<8)
#define ACLKSRC_DCOCLK      (3<<8)
#define ACLKSRC_DCODIVCLK   (4<<8)
#define ACLKSRC_XT2CLK      (5<<8)

#define SMCLKSRC_XT1CLK     (0<<4)
#define SMCLKSRC_VLOCLK     (1<<4)
#define SMCLKSRC_REFOCLK    (2<<4)
#define SMCLKSRC_DCOCLK     (3<<4)
#define SMCLKSRC_DCODIVCLK  (4<<4)
#define SMCLKSRC_XT2CLK     (5<<4)

#define MCLKSRC_XT1CLK      (0<<0)
#define MCLKSRC_VLOCLK      (1<<0)
#define MCLKSRC_REFOCLK     (2<<0)
#define MCLKSRC_DCOCLK      (3<<0)
#define MCLKSRC_DCODIVCLK   (4<<0)
#define MCLKSRC_XT2CLK      (5<<0)



// Select source for FLLREF  e.g. SELECT_FLLREF(SELREF__XT1CLK)
#define SELECT_FLLREF(source) st(UCS->CTL3 = (UCS->CTL3 & ~(SELREF_7)) | (source);)

// Select source for ACLK    e.g. SELECT_ACLK(SELA__XT1CLK) */
#define SELECT_ACLK(source)   st(UCS->CTL4 = (UCS->CTL4 & ~(SELA_7))   | (source);)

// Select source for MCLK    e.g. SELECT_MCLK(SELM__XT2CLK) */
#define SELECT_MCLK(source)   st(UCS->CTL4 = (UCS->CTL4 & ~(SELM_7))   | (source);)

// Select source for SMCLK   e.g. SELECT_SMCLK(SELS__XT2CLK) */
#define SELECT_SMCLK(source)  st(UCS->CTL4 = (UCS->CTL4 & ~(SELS_7))   | (source);)

// Select source for MCLK and SMCLK e.g. SELECT_MCLK_SMCLK(SELM__DCOCLK + SELS__DCOCLK) */
#define SELECT_MCLK_SMCLK(sources) st(UCS->CTL4 = (UCS->CTL4 & ~(SELM_7 + SELS_7)) | (sources);)

// set ACLK/x */
#define ACLK_DIV(x)         st(UCS->CTL5 = (UCS->CTL5 & ~(DIVA_7)) | (DIVA__##x);)

// set MCLK/x */
#define MCLK_DIV(x)         st(UCS->CTL5 = (UCS->CTL5 & ~(DIVM_7)) | (DIVM__##x);)

// set SMCLK/x */
#define SMCLK_DIV(x)        st(UCS->CTL5 = (UCS->CTL5 & ~(DIVS_7)) | (DIVS__##x);)

// Select divider for FLLREF  e.g. SELECT_FLLREFDIV(2) */
#define SELECT_FLLREFDIV(x) st(UCS->CTL3 = (UCS->CTL3 & ~(FLLREFDIV_7))|(FLLREFDIV__##x);)

//************************************************************************
// Defines
//************************************************************************

#define UCS_STATUS_OK     0
#define UCS_STATUS_ERROR  1







// CTL0, read only
#define UCS_CTL0_DCO        0x1F00
#define UCS_CTL0_MOD        0x00F8


// CTL1
#define UCS_CTL1_DCORSEL    0x0070
#define UCS_CTL1_DCORSELx 
#define UCS_CTL1_DISMOD     0x0001


// CTL2
#define UCS_CTL2_FLLD       0x7000
#define UCS_CTL2_FLLD1      0x0000
#define UCS_CTL2_FLLD2      0x1000
#define UCS_CTL2_FLLD4      0x2000
#define UCS_CTL2_FLLD8      0x3000
#define UCS_CTL2_FLLD16     0x4000
#define UCS_CTL2_FLLD32     0x5000
#define UCS_CTL2_FLLN       0x01FF
#define SET_FLLN(MULT)      do { \
                                UCS->CTL2 &= ~UCS_CTL2_FLLN; \
                                UCS->CTL2 |= MULT; \
                            } while(0)


// CTL3
#define UCS_CTL3_SELREF             0x0070
#define UCS_CTL3_SELREF_XT1CLK      0x0000
#define UCS_CTL3_SELREF_REFOCLK     0x0020
#define UCS_CTL3_SELREF_XT2CLK      0x0050
#define UCS_CTL3_FLLREFDIV          0x0007
#define UCS_CTL3_FLLREFDIV1         0x0000
#define UCS_CTL3_FLLREFDIV2         0x0001
#define UCS_CTL3_FLLREFDIV4         0x0002
#define UCS_CTL3_FLLREFDIV8         0x0003
#define UCS_CTL3_FLLREFDIV12        0x0004
#define UCS_CTL3_FLLREFDIV16        0x0005


// CTL4
#define UCS_CTL4_SELA               8
#define UCS_CTL4_SELS               4
#define UCS_CTL4_SELM               0
#define UCS_SEL_XT1CLK              0x0000
#define UCS_SEL_VLOCLK              0x0001
#define UCS_SEL_REFOCLK             0x0002
#define UCS_SEL_DCOCLK              0x0003
#define UCS_SEL_DCOCLKDIV           0x0004
#define UCS_SEL_XT2CLK              0x0005


// CTL5
#define UCS_CTL5_DIVPA              12
#define UCS_CTL5_DIVA               8
#define UCS_CTL5_DIVS               4
#define UCS_CTL5_DIVM               0
#define UCS_DIV1                    0x0000
#define UCS_DIV2                    0x0001
#define UCS_DIV4                    0x0002
#define UCS_DIV8                    0x0003
#define UCS_DIV16                   0x0004
#define UCS_DIV32                   0x0005


// CTL6
#define UCS_CTL6_XT2OFF             0x0100
#define UCS_CTL6_XT1DRIVE           0x00C0
#define UCS_CTL6_XT1DRIVE_0         0x0000
#define UCS_CTL6_XT1DRIVE_1         0x0040
#define UCS_CTL6_XT1DRIVE_2         0x0080
#define UCS_CTL6_XT1DRIVE_3         0x00C0
#define UCS_CTL6_XTS                0x0020
#define UCS_CTL6_XTS_low            0x0000
#define UCS_CTL6_XTS_high           0x0000
#define UCS_CTL6_XT1BYPASS          0x0010
#define UCS_CTL6_XT1BYPASS_off      0x0000
#define UCS_CTL6_XT1BYPASS_on       0x0010
#define UCS_CTL6_XCAP               0x000C
#define UCS_CTL6_XCAP_0             0x0000
#define UCS_CTL6_XCAP_1             0x0004
#define UCS_CTL6_XCAP_2             0x0008
#define UCS_CTL6_XCAP_3             0x000C
#define UCS_CTL6_SMCLKOFF           0x0002
#define UCS_CTL6_XT1OFF             0x0001
#define SET_XCAP(VAL)               do { \
                                        UCS->CTL6 &= ~UCS_CTL6_XCAP; \
                                        UCS->CTL6 |= (VAL<<2); \
                                    } while(0)


// CTL7 (Flags)
#define UCS_FLG_XT2OFFG             0x0008
#define UCS_FLG_XT1HFOFFG           0x0004
#define UCS_FLG_XT1LFOFFG           0x0002
#define UCS_FLG_DCOFFG              0x0001


// CTL8 (Conditional Request Enable Bits)
#define UCS_CREQ_MODOSCREQEN        0x0008
#define UCS_CREQ_SMCLKREQEN         0x0004
#define UCS_CREQ_MCLKREQEN          0x0002
#define UCS_CREQ_ACTKREQEN          0x0001






typedef enum {
    clockACLK   = UCS_CTL4_SELA,
    clockSMCLK  = UCS_CTL4_SELS,
    clockMCLK   = UCS_CTL4_SELM
} ClockIndex;

typedef enum {
    range0  = 0x00,
    range1  = 0x10,
    range2  = 0x20,
    range3  = 0x30,
    range4  = 0x40,
    range5  = 0x50,
    range6  = 0x60,
    range7  = 0x70
} RangeIndex;

typedef enum {
    sourceXT1    = UCS_SEL_XT1CLK,
    sourceVLO    = UCS_SEL_VLOCLK,
    sourceREFO   = UCS_SEL_REFOCLK,
    sourceDCO    = UCS_SEL_DCOCLK,
    sourceDCODIV = UCS_SEL_DCOCLKDIV,
    sourceXT2    = UCS_SEL_XT2CLK
} SourceIndex;

typedef enum {
    div1  = UCS_DIV1,
    div2  = UCS_DIV2,
    div4  = UCS_DIV4,
    div8  = UCS_DIV8,
    div16 = UCS_DIV16,
    div32 = UCS_DIV32
} DivIndex;

typedef enum {
    refdiv1  = UCS_CTL3_FLLREFDIV1,
    refdiv2  = UCS_CTL3_FLLREFDIV2,
    refdiv4  = UCS_CTL3_FLLREFDIV4,
    refdiv8  = UCS_CTL3_FLLREFDIV8,
    refdiv12 = UCS_CTL3_FLLREFDIV12,
    refdiv16 = UCS_CTL3_FLLREFDIV16
} RefDivIndex;

typedef enum {
    MHz_4to8   = UCS_CTL6_XT1DRIVE_0,
    MHz_8to16  = UCS_CTL6_XT1DRIVE_1,
    MHz_16to24 = UCS_CTL6_XT1DRIVE_2,
    MHz_24to32 = UCS_CTL6_XT1DRIVE_3
} XT1DriveIndex;

typedef enum {
    LowFrequency  = 0,
    HighFrequency = UCS_CTL6_XTS
} XT1ModeIndex;

typedef enum {
    Internal = 0,
    External = UCS_CTL6_XT1BYPASS
} XT1BypassIndex;


typedef struct {
    ClockIndex      Clock;
    SourceIndex     Source;
    DivIndex        Div;
} ClockInit_Type;

typedef struct {
    u16         Frequency;
    u16         Multiplier;
    SourceIndex Source;
    RefDivIndex SourceDiv;
    //DivIndex  FLLDiv;
} FLLInit_Type;

typedef struct {
    XT1ModeIndex    Mode;
    XT1BypassIndex  Bypass;
    u16             CapSelect;
} XT1Init_Type;



/** @brief Initializes Frequency Locked Loop
  * @param FLLInitStruct (FLLInit_Type*): Pointer to FLL Init Data
  * @retval none
  * @ingroup CC430_lib
  * 
  * Clock Multiplier value is the rounded value of: Frequency / 32768
  *
  * UCS_FLLInit enables the FLL after running, making a call to UCS_FLLCmd
  * redundant.
  */
void UCS_FLLInit(FLLInit_Type* FLLInitStruct);

/** @brief Enables/Disables Frequency Locked Loop
  * @param NewState (FunctionalState): ENABLE or DISABLE
  * @retval none
  * @ingroup CC430_lib
  */
void UCS_FLLCmd(FunctionalState NewState);


/** Initializes FLL of the UCS and wait till settled
  * @param fsystem  required system frequency (MCLK) in kHz
  * @param ratio    ratio between fsystem and FLLREFCLK
  */
void UCS_FLL_settle(u16 fsystem, u16 ratio);


void UCS_LFXT1Init();

void UCS_ClockInit(ClockInit_Type* ClockInitStruct);


/** @brief Configure XT1 for LF, HF, etc
  * @param ConfigBits (u16): UCS_CTL6_XT1DRIVE | UCS_CTL6_XTS | UCS_CTL6_XT1BYPASS | UCS_CTL6_XCAP
  * @retval none
  * @ingroup CC430_lib
  */
void UCS_XT1Config(u16 ConfigBits);

/** @brief Enables/Disables XT1 Oscillator
  * @param NewState (FunctionalState): ENABLE or DISABLE
  * @retval none
  * @ingroup CC430_lib
  */
void UCS_XT1Cmd(FunctionalState NewState);



/** @brief Enables/Disables XT2 Oscillator
  * @param NewState (FunctionalState): ENABLE or DISABLE
  * @retval none
  * @ingroup CC430_lib
  *
  * Note, in disabled state, XT2 is still available to be used on-demand by
  * the RF component.
  */
void UCS_XT2Cmd(FunctionalState NewState);



/** @brief Enables/Disables SMCLK
  * @param NewState (FunctionalState): ENABLE or DISABLE
  * @retval none
  * @ingroup CC430_lib
  */
void UCS_SMCLKCmd(FunctionalState NewState);



/** @brief Enables/Disables Clock Requests
  * @param CREQMask (u16): Conditional Request Enable Bits
  * @param NewState (FunctionalState): ENABLE or DISABLE
  * @retval none
  * @ingroup CC430_lib
  * 
  * Conditional Request Enable Bits (defined in cc430_ucs.h)
  * UCS_CREQ_MODOSCREQEN        0x0008
  * UCS_CREQ_SMCLKREQEN         0x0004
  * UCS_CREQ_MCLKREQEN          0x0002
  * UCS_CREQ_ACTKREQEN          0x0001
  */
void UCS_RequestCmd(u16 CREQMask, FunctionalState NewState);



u16 UCS_GetFlagStatus(u16 FlagMask);

void UCS_ClearFlag(u16 FlagMask);






#endif
#endif

