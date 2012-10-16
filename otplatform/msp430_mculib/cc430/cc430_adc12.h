/* Copyright 2009-2012 JP Norair
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
/** @file       /otplatform/msp430f5_mculib/cc430/cc430_adc12.h
  * @author     JP Norair
  * @version    V1.0
  * @date       27 Sept 2012
  * @brief      Library resources for ADC12 peripheral
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#ifndef __CC430_LIB_ADC12_H
#define __CC430_LIB_ADC12_H

#include "cc430_map.h"

#define ADC_CTL0_SHT1_4     (0 << 12)
#define ADC_CTL0_SHT1_8     (1 << 12)
#define ADC_CTL0_SHT1_16    (2 << 12)
#define ADC_CTL0_SHT1_32    (3 << 12)
#define ADC_CTL0_SHT1_64    (4 << 12)
#define ADC_CTL0_SHT1_96    (5 << 12)
#define ADC_CTL0_SHT1_128   (6 << 12)
#define ADC_CTL0_SHT1_192   (7 << 12)
#define ADC_CTL0_SHT1_256   (8 << 12)
#define ADC_CTL0_SHT1_384   (9 << 12)
#define ADC_CTL0_SHT1_512   (10 << 12)
#define ADC_CTL0_SHT1_768   (11 << 12)
#define ADC_CTL0_SHT1_1024  (12 << 12)
#define ADC_CTL0_SHT0_4     (0 << 8)
#define ADC_CTL0_SHT0_8     (1 << 8)
#define ADC_CTL0_SHT0_16    (2 << 8)
#define ADC_CTL0_SHT0_32    (3 << 8)
#define ADC_CTL0_SHT0_64    (4 << 8)
#define ADC_CTL0_SHT0_96    (5 << 8)
#define ADC_CTL0_SHT0_128   (6 << 8)
#define ADC_CTL0_SHT0_192   (7 << 8)
#define ADC_CTL0_SHT0_256   (8 << 8)
#define ADC_CTL0_SHT0_384   (9 << 8)
#define ADC_CTL0_SHT0_512   (10 << 8)
#define ADC_CTL0_SHT0_768   (11 << 8)
#define ADC_CTL0_SHT0_1024  (12 << 8)
#define ADC_CTL0_MSC        (1 << 7)
#define ADC_CTL0_REF_1V5    (0 << 6)
#define ADC_CTL0_REF_2V5    (1 << 6)
#define ADC_CTL0_REFON      (1 << 5)
#define ADC_CTL0_ON         (1 << 4)
#define ADC_CTL0_OVIE       (1 << 3)
#define ADC_CTL0_TOVIE      (1 << 2)
#define ADC_CTL0_ENC        (1 << 1)
#define ADC_CTL0_SC         (1 << 0)




#define ADC_CTL1_START(VAL) (VAL << 12)

#define ADC_CTL1_SHS        (3 << 10)
#define ADC_CTL1_SHS_SC     (0 << 10)
#define ADC_CTL1_SHS_SRC1   (1 << 10)
#define ADC_CTL1_SHS_SRC2   (2 << 10)
#define ADC_CTL1_SHS_SRC3   (3 << 10)

#define ADC_CTL1_SHP        (1 << 9)
#define ADC_CTL1_ISSH       (1 << 8)

#define ADC_CTL1_DIV(VAL)   ((VAL-1) << 5)
#define ADC_CTL1_DIV_1      (0 << 5)
#define ADC_CTL1_DIV_2      (1 << 5)
#define ADC_CTL1_DIV_3      (2 << 5)
#define ADC_CTL1_DIV_4      (3 << 5)
#define ADC_CTL1_DIV_5      (4 << 5)
#define ADC_CTL1_DIV_6      (5 << 5)
#define ADC_CTL1_DIV_7      (6 << 5)
#define ADC_CTL1_DIV_8      (7 << 5)

#define ADC_CTL1_SSEL       (3 << 3)
#define ADC_CTL1_SSEL_MODCLK (0 << 3)
#define ADC_CTL1_SSEL_ACLK  (1 << 3)
#define ADC_CTL1_SSEL_MCLK  (2 << 3)
#define ADC_CTL1_SSEL_SMCLK (3 << 3)

#define ADC_CTL1_CONSEQ             (3 << 3)
#define ADC_CTL1_CONSEQ_SINGLE      (0 << 1)
#define ADC_CTL1_CONSEQ_SEQUENCE    (1 << 1)
#define ADC_CTL1_CONSEQ_REPSINGLE   (2 << 1)
#define ADC_CTL1_CONSEQ_REPSEQUENCE (3 << 1)

#define ADC_CTL1_BUSY       (1 << 0)





#define ADC_CTL2_PDIV       (1 << 8)
#define ADC_CTL2_PDIV_1     (0 << 8)
#define ADC_CTL2_PDIV_4     (1 << 8)

#define ADC_CTL2_TCOFF      (1 << 7)

#define ADC_CTL2_RES        (3 << 4)
#define ADC_CTL2_RES_8      (0 << 4)
#define ADC_CTL2_RES_10     (1 << 4)
#define ADC_CTL2_RES_12     (2 << 4)
#define ADC_CTL2_RES_MAX    ADC_CTL2_RES_12

#define ADC_CTL2_DF         (1 << 3)
#define ADC_CTL2_DF_UNSIGNED (0 << 3)
#define ADC_CTL2_DF_SIGNED   (1 << 3)

#define ADC_CTL2_SR         (1 << 2)
#define ADC_CTL2_SR_200K    (0 << 2)
#define ADC_CTL2_SR_50K     (1 << 2)

#define ADC_CTL2_REFOUT     (1 << 1)
#define ADC_CTL2_REFBURST   (1 << 0)





#define ADC_MCTL_EOS        (1 << 7)

#define ADC_MCTL_SREF               (7 << 4)
#define ADC_MCTL_SREF_AVCC_AVSS     (0 << 4)
#define ADC_MCTL_SREF_VREF_AVSS     (0 << 4)
#define ADC_MCTL_SREF_VeREF_AVSS    (2 << 4)
#define ADC_MCTL_SREF_AVCC_VREF     (4 << 4)
#define ADC_MCTL_SREF_VREF_VREF     (5 << 4)
#define ADC_MCTL_SREF_VeREF_VREF    (6 << 4)

#define ADC_MCTL_INCH       (15 << 0)
#define ADC_MCTL_INCH_A0    (0 << 0)
#define ADC_MCTL_INCH_A1    (1 << 0)
#define ADC_MCTL_INCH_A2    (2 << 0)
#define ADC_MCTL_INCH_A3    (3 << 0)
#define ADC_MCTL_INCH_A4    (4 << 0)
#define ADC_MCTL_INCH_A5    (5 << 0)
#define ADC_MCTL_INCH_A6    (6 << 0)
#define ADC_MCTL_INCH_A7    (7 << 0)
#define ADC_MCTL_INCH_A8        (8 << 0)
#define ADC_MCTL_INCH_VeREFP    (8 << 0)
#define ADC_MCTL_INCH_A9        (9 << 0)
#define ADC_MCTL_INCH_VREFN     (9 << 0)
#define ADC_MCTL_INCH_A10       (10 << 0)
#define ADC_MCTL_INCH_TEMP      (10 << 0)
#define ADC_MCTL_INCH_A11       (11 << 0)
#define ADC_MCTL_INCH_HALFVCC   (11 << 0)
#define ADC_MCTL_INCH_A12       (12 << 0)
#define ADC_MCTL_INCH_A13       (13 << 0)
#define ADC_MCTL_INCH_A14       (14 << 0)
#define ADC_MCTL_INCH_A15       (15 << 0)



#define ADC_IE_15   (1 << 15)
#define ADC_IE_14   (1 << 14)
#define ADC_IE_13   (1 << 13)
#define ADC_IE_12   (1 << 12)
#define ADC_IE_11   (1 << 11)
#define ADC_IE_10   (1 << 10)
#define ADC_IE_9    (1 << 9)
#define ADC_IE_8    (1 << 8)
#define ADC_IE_7    (1 << 7)
#define ADC_IE_6    (1 << 6)
#define ADC_IE_5    (1 << 5)
#define ADC_IE_4    (1 << 4)
#define ADC_IE_3    (1 << 3)
#define ADC_IE_2    (1 << 2)
#define ADC_IE_1    (1 << 1)
#define ADC_IE_0    (1 << 0)

//ADC10
#define ADC_IE_TOV    0
#define ADC_IE_OV     0
#define ADC_IE_HI     0
#define ADC_IE_LO     0
#define ADC_IE_IN     0


#define ADC_IFG_15   (1 << 15)
#define ADC_IFG_14   (1 << 14)
#define ADC_IFG_13   (1 << 13)
#define ADC_IFG_12   (1 << 12)
#define ADC_IFG_11   (1 << 11)
#define ADC_IFG_10   (1 << 10)
#define ADC_IFG_9    (1 << 9)
#define ADC_IFG_8    (1 << 8)
#define ADC_IFG_7    (1 << 7)
#define ADC_IFG_6    (1 << 6)
#define ADC_IFG_5    (1 << 5)
#define ADC_IFG_4    (1 << 4)
#define ADC_IFG_3    (1 << 3)
#define ADC_IFG_2    (1 << 2)
#define ADC_IFG_1    (1 << 1)
#define ADC_IFG_0    (1 << 0)


//ADC10
#define ADC_IFG_TOV   0
#define ADC_IFG_OV    0
#define ADC_IFG_HI    0
#define ADC_IFG_LO    0
#define ADC_IFG_IN    0











#endif

