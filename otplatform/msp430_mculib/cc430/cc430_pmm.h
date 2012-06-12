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
  * @file       /Platforms/CC430/cc430_lib/cc430_pmm.h
  * @author     JP Norair
  * @version    V1.0
  * @date       15 Nov 2011
  * @brief      Library resources for Power Management Module
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#ifndef __CC430_LIB_PMM
#define __CC430_LIB_PMM

#include "cc430_map.h"

#define PMM_VCore   PMM_Vcore

typedef enum {
    PMM_Vcore_18 = 0,
    PMM_Vcore_20 = 1,
    PMM_Vcore_22 = 2,
    PMM_Vcore_24 = 3
} PMM_Vcore;

typedef enum {
    SVS_Von_16 = 0,
    SVS_Von_18 = 1,
    SVS_Von_20 = 2,
    SVS_Von_21 = 3
} SVS_Von;

typedef enum {
    SVSM_Voffon_17 = 0,
    SVSM_Voffon_19 = 1,
    SVSM_Voffon_21 = 2,
    SVSM_Voffon_22 = 3,
    SVSM_Voffon_235 = 4,
    SVSM_Voffon_265 = 5,
    SVSM_Voffon_30 = 6
} SVSM_Voffon;


// SVSMH Flags
#define SVM_FullPerformance     ((unsigned short)1<<15)
#define SVM_Enable              ((unsigned short)1<<14)
#define SVM_OvervoltProtect     ((unsigned short)1<<12)
#define SVS_FullPerformance     ((unsigned short)1<<11)
#define SVS_Enable              ((unsigned short)1<<10)
#define SVSM_AutoControl        ((unsigned short)1<<7)
#define SVSM_EventMasking       ((unsigned short)1<<6)
#define SVS_ActiveDuringLPM     ((unsigned short)1<<4)
#define SVSM_EventDelay         ((unsigned short)1<<3)




#define ENABLE_SVSL()        do { PMMb->CTL0_H = 0xA5; PMM->SVSMLCTL |= SVSLE; PMMb->CTL0_H = 0x00; } while(0)
#define DISABLE_SVSL()       do { PMMb->CTL0_H = 0xA5; PMM->SVSMLCTL &= ~SVSLE; PMMb->CTL0_H = 0x00; } while(0)
#define ENABLE_SVML()        do { PMMb->CTL0_H = 0xA5; PMM->SVSMLCTL |= SVMLE; PMMb->CTL0_H = 0x00; } while(0)
#define DISABLE_SVML()       do { PMMb->CTL0_H = 0xA5; PMM->SVSMLCTL &= ~SVMLE; PMMb->CTL0_H = 0x00; } while(0)
#define ENABLE_SVSH()        do { PMMb->CTL0_H = 0xA5; PMM->SVSMHCTL |= SVSHE; PMMb->CTL0_H = 0x00; } while(0)
#define DISABLE_SVSH()       do { PMMb->CTL0_H = 0xA5; PMM->SVSMHCTL &= ~SVSHE; PMMb->CTL0_H = 0x00; } while(0)
#define ENABLE_SVMH()        do { PMMb->CTL0_H = 0xA5; PMM->SVSMHCTL |= SVMHE; PMMb->CTL0_H = 0x00; } while(0)
#define DISABLE_SVMH()       do { PMMb->CTL0_H = 0xA5; PMM->SVSMHCTL &= ~SVMHE; PMMb->CTL0_H = 0x00; } while(0)
#define ENABLE_SVSL_SVML()   do { PMMb->CTL0_H = 0xA5; PMM->SVSMLCTL |= (SVSLE + SVMLE); PMMb->CTL0_H = 0x00; } while(0)
#define DISABLE_SVSL_SVML()  do { PMMb->CTL0_H = 0xA5; PMM->SVSMLCTL &= ~(SVSLE + SVMLE); PMMb->CTL0_H = 0x00; } while(0)
#define ENABLE_SVSH_SVMH()   do { PMMb->CTL0_H = 0xA5; PMM->SVSMHCTL |= (SVSHE + SVMHE); PMMb->CTL0_H = 0x00; } while(0)
#define DISABLE_SVSH_SVMH()  do { PMMb->CTL0_H = 0xA5; PMM->SVSMHCTL &= ~(SVSHE + SVMHE); PMMb->CTL0_H = 0x00; } while(0)

#define ENABLE_SVSL_RESET()       do { PMMb->CTL0_H = 0xA5; PMM->RIE |= SVSLPE; PMMb->CTL0_H = 0x00; } while(0)
#define DISABLE_SVSL_RESET()      do { PMMb->CTL0_H = 0xA5; PMM->RIE &= ~SVSLPE; PMMb->CTL0_H = 0x00; } while(0)
#define ENABLE_SVML_INTERRUPT()   do { PMMb->CTL0_H = 0xA5; PMM->RIE |= SVMLIE; PMMb->CTL0_H = 0x00; } while(0)
#define DISABLE_SVML_INTERRUPT()  do { PMMb->CTL0_H = 0xA5; PMM->RIE &= ~SVMLIE; PMMb->CTL0_H = 0x00; } while(0)
#define ENABLE_SVSH_RESET()       do { PMMb->CTL0_H = 0xA5; PMM->RIE |= SVSHPE; PMMb->CTL0_H = 0x00; } while(0)
#define DISABLE_SVSH_RESET()      do { PMMb->CTL0_H = 0xA5; PMM->RIE &= ~SVSHPE; PMMb->CTL0_H = 0x00; } while(0)
#define ENABLE_SVMH_INTERRUPT()   do { PMMb->CTL0_H = 0xA5; PMM->RIE |= SVMHIE; PMMb->CTL0_H = 0x00; } while(0)
#define DISABLE_SVMH_INTERRUPT()  do { PMMb->CTL0_H = 0xA5; PMM->RIE &= ~SVMHIE; PMMb->CTL0_H = 0x00; } while(0)
#define CLEAR_PMM_IFGS()          do { PMMb->CTL0_H = 0xA5; PMM->IFG = 0; PMMb->CTL0_H = 0x00; } while(0)



/** @brief Sets the Core Voltage to a specified level
  * @author Stefan Schauer (Texas Instruments)
  * @date 15 Nov 2011
  * @param level        (PMM_Vcore) Corresponds to 0,1,2,3 via enum
  * @retval None
  * @ingroup CC430_lib
  *
  * The CC430 documentation shows that levels 0, 1, 2, 3, correspond to core
  * voltages of 1.8V, 2.0V, 2.2V, 2.4V, and max clock of 8, 16, 20, 25 MHz.
  *
  * @note This function should be run at power-on, before any manipulation of 
  * the SVSM.  It should NOT be used during runtime, as it will clobber the
  * settings of the SVSM.  There is a __large__ amount of errata for the PMM,
  * system so it is better to just leave it alone once it is set.
  */
void PMM_SetVCore(PMM_VCore level);


/** @brief Sets up the SVSM in a standard way.  Call after PMMSetVCore().
  * @author JP Norair
  * @date 15 Nov 2011
  * @param svsmh_cfg    (unsigned short) Flags to apply to PMM->SVSMHCTL register
  * @param Von          (SVS_Von) Voltage where SVS turns-on
  * @param Voff         (SVSM_Voffon) Voltage where SVS turns-off, and SVM turns-on
  * @retval None
  * @ingroup CC430_lib
  *
  * The "svsmh_cfg" parameter can be used with the SVSMH Flag Macros defined in
  * this file, for ease of use.
  * 
  * @note This function should be run immediately after using PMM_SetVCore() at
  * power-on.  As mentioned, there is a __large__ amount of errata for the PMM,
  * so just set it and leave it alone.
  */
void PMM_SetStdSVSM(unsigned short svsmh_cfg, SVS_Von Von, SVSM_Voffon Voffon);


/** @brief Sets up the CC430 for LPM0
  * @param None
  * @retval None
  * @ingroup CC430_lib
  */
void PMM_EnterLPM0(void);


/** @brief Sets up the CC430 for LPM1
  * @param None
  * @retval None
  * @ingroup CC430_lib
  */
void PMM_EnterLPM1(void);



/** @brief Sets up the CC430 for LPM2
  * @param None
  * @retval None
  * @ingroup CC430_lib
  */
void PMM_EnterLPM2(void);



/** @brief Sets up CC430 for LPM3, using LFXT1 to drive the ACLK
  * @author M. Morales, Texas Instruments
  * @date April 2009
  * @param None
  * @retval None
  * @ingroup CC430_lib
  *
  * Description:    Configure ACLK = LFXT1 and enters LPM3. Measure current.
  * Note:           SVS(H,L) & SVM(H,L) are disabled
  *                 ACLK = LFXT1 = 32kHz, MCLK = SMCLK = default DCO
  *
  *          CC430x613x
  *      -----------------
  *  /|\ |            XIN|-
  *   |  |               | 32kHz
  *   ---|RST        XOUT|-
  *      |               |
  *
  */
void PMM_EnterLPM3(void);




/** @brief Sets up the CC430 for LPM4
  * @param None
  * @retval None
  * @ingroup CC430_lib
  */
void PMM_EnterLPM4(void);



#endif /* __PMM */
