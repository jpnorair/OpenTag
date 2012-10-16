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
/** @file       /otplatform/msp430f5_mculib/cc430/cc430_adc10.h
  * @author     JP Norair
  * @version    V1.0
  * @date       27 Sept 2012
  * @brief      Library resources for ADC10 peripheral
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#ifndef __CC430_LIB_ADC10_H
#define __CC430_LIB_ADC10_H

#include "cc430_adc12.h"



#undef ADC_CTL0_SHT1_4     
#undef ADC_CTL0_SHT1_8     
#undef ADC_CTL0_SHT1_16    
#undef ADC_CTL0_SHT1_32    
#undef ADC_CTL0_SHT1_64    
#undef ADC_CTL0_SHT1_96    
#undef ADC_CTL0_SHT1_128   
#undef ADC_CTL0_SHT1_192   
#undef ADC_CTL0_SHT1_256   
#undef ADC_CTL0_SHT1_384   
#undef ADC_CTL0_SHT1_512   
#undef ADC_CTL0_SHT1_768   
#undef ADC_CTL0_SHT1_1024  
#undef ADC_CTL0_REF_1V5    
#undef ADC_CTL0_REF_2V5    
#undef ADC_CTL0_REFON      
#undef ADC_CTL0_OVIE       
#undef ADC_CTL0_TOVIE      
#define ADC_CTL0_SHT1_4     0
#define ADC_CTL0_SHT1_8     0
#define ADC_CTL0_SHT1_16    0
#define ADC_CTL0_SHT1_32    0
#define ADC_CTL0_SHT1_64    0
#define ADC_CTL0_SHT1_96    0
#define ADC_CTL0_SHT1_128   0
#define ADC_CTL0_SHT1_192   0
#define ADC_CTL0_SHT1_256   0
#define ADC_CTL0_SHT1_384   0
#define ADC_CTL0_SHT1_512   0
#define ADC_CTL0_SHT1_768   0
#define ADC_CTL0_SHT1_1024  0
#define ADC_CTL0_REF_1V5    0
#define ADC_CTL0_REF_2V5    0
#define ADC_CTL0_REFON      0
#define ADC_CTL0_OVIE       0
#define ADC_CTL0_TOVIE      0




#undef ADC_CTL2_PDIV
#undef ADC_CTL2_TCOFF
#undef ADC_CTL2_REFOUT
#undef ADC_CTL2_REFBURST
#undef ADC_CTL2_RES_MAX
#define ADC_CTL2_PDIV       (3 << 8)
#define ADC_CTL2_PDIV_64    (2 << 8)
#define ADC_CTL2_TCOFF      0
#define ADC_CTL2_REFOUT     0
#define ADC_CTL2_REFBURST   0
#define ADC_CTL2_RES_MAX    ADC_CTL2_RES_10


#undef ADC_MCTL_EOS
#define ADC_MCTL_EOS 0



#undef ADC_IE_TOV
#undef ADC_IE_OV
#undef ADC_IE_HI
#undef ADC_IE_LO
#undef ADC_IE_IN
#define ADC_IE_TOV    (1 << 5)
#define ADC_IE_OV     (1 << 4)
#define ADC_IE_HI     (1 << 3)
#define ADC_IE_LO     (1 << 2)
#define ADC_IE_IN     (1 << 1)




#undef ADC_IFG_TOV
#undef ADC_IFG_OV
#undef ADC_IFG_HI
#undef ADC_IFG_LO
#undef ADC_IFG_IN
#define ADC_IFG_TOV   (1 << 5)
#define ADC_IFG_OV    (1 << 4)
#define ADC_IFG_HI    (1 << 3)
#define ADC_IFG_LO    (1 << 2)
#define ADC_IFG_IN    (1 << 1)







#endif

