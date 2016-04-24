/* Copyright 2010-2011 JP Norair
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
  * @file       /OTlib/m2_encode_table.h
  * @author     JP Norair
  * @version    V1.0
  * @date       08 June 2010
  * @brief      This file stores the encode table constants, which are used in
  *             some versions of the encode module.
  * @ingroup    Encode
  ******************************************************************************
  */


#ifndef __ENCODE_TABLE_H
#define __ENCODE_TABLE_H

#include <otsys/types.h>
#include <otsys/config.h>


#   if (OTF_M2_ENCODE_TABLE != ENABLED)
#       error m2_encode_table.h referenced, but OTF_M2_ENCODE_TABLE is not ENABLED in OTConfig.h
#   endif


#   if (CC_SUPPORT == SIM_GCC)
        const ot_u8 FECtable[16] = { 0, 3, 1, 2, 3, 0, 2, 1, 3, 0, 2, 1, 0, 3, 1, 2 }; 
        const ot_u8 TrellisSourceState[8][2] = { 
            {0, 4},         // State {0,4} -> State 0
            {0, 4},         // State {0,4} -> State 1
            {1, 5},         // State {1,5} -> State 2
            {1, 5},         // State {1,5} -> State 3
            {2, 6},         // State {2,6} -> State 4
            {2, 6},         // State {2,6} -> State 5
            {3, 7},         // State {3,7} -> State 6
            {3, 7}          // State {3,7} -> State 7
        };
        const ot_u8 TrellisTransitionOutput[8][2] = { 
            {0, 3},         // State {0,4} -> State 0 produces {"00", "11"}
            {3, 0},         // State {0,4} -> State 1 produces {"11", "00"}
            {1, 2},         // State {1,5} -> State 2 produces {"01", "10"}
            {2, 1},         // State {1,5} -> State 3 produces {"10", "01"}
            {3, 0},         // State {2,6} -> State 4 produces {"11", "00"}
            {0, 3},         // State {2,6} -> State 5 produces {"00", "11"}
            {2, 1},         // State {3,7} -> State 6 produces {"10", "01"}
            {1, 2}          // State {3,7} -> State 7 produces {"01", "10"}
        };
        const ot_u8 TrellisTransitionInput[8] = { 0, 1, 0, 1, 0, 1, 0, 1 };

#   elif ((CC_SUPPORT == CL430) || (CC_SUPPORT == GCC))
        extern ot_u8 FECtable[16];
        extern ot_u8 TrellisSourceState[8][2];
        extern ot_u8 TrellisTransitionOutput[8][2];
        extern ot_u8 TrellisTransitionInput[8];
        
#   elif (CC_SUPPORT == IAR_V5)
        __no_init ot_u8 FECtable[16]                    @ OTF_LOOKUP_START_ADDR;
        __no_init ot_u8 TrellisSourceState[8][2]        @ (OTF_LOOKUP_START_ADDR+16);
        __no_init ot_u8 TrellisTransitionOutput[8][2]   @ (OTF_LOOKUP_START_ADDR+32);
        __no_init ot_u8 TrellisTransitionInput[8]       @ (OTF_LOOKUP_START_ADDR+48);
        


#   else
#       error decoder_table not allocated because PLATFORM(COMPILER) value is incorrect

#   endif



/// Here are some defines, in case you want to manually load into flash

#   define DECODE_FECtable_0                0
#   define DECODE_FECtable_1                3
#   define DECODE_FECtable_2                1
#   define DECODE_FECtable_3                2   
#   define DECODE_FECtable_4                3
#   define DECODE_FECtable_5                0
#   define DECODE_FECtable_6                2
#   define DECODE_FECtable_7                1
#   define DECODE_FECtable_8                3
#   define DECODE_FECtable_9                0
#   define DECODE_FECtable_10               2
#   define DECODE_FECtable_11               1
#   define DECODE_FECtable_12               0
#   define DECODE_FECtable_13               3
#   define DECODE_FECtable_14               1
#   define DECODE_FECtable_15               2

#   define DECODE_TrellisSourceState_0_0    0
#   define DECODE_TrellisSourceState_0_1    4
#   define DECODE_TrellisSourceState_1_0    0
#   define DECODE_TrellisSourceState_1_1    4
#   define DECODE_TrellisSourceState_2_0    1
#   define DECODE_TrellisSourceState_2_1    5
#   define DECODE_TrellisSourceState_3_0    1
#   define DECODE_TrellisSourceState_3_1    5
#   define DECODE_TrellisSourceState_4_0    2
#   define DECODE_TrellisSourceState_4_1    6
#   define DECODE_TrellisSourceState_5_0    2
#   define DECODE_TrellisSourceState_5_1    6
#   define DECODE_TrellisSourceState_6_0    3
#   define DECODE_TrellisSourceState_6_1    7
#   define DECODE_TrellisSourceState_7_0    3
#   define DECODE_TrellisSourceState_7_1    7

#   define DECODE_TrellisTransitionOutput_0_0    0
#   define DECODE_TrellisTransitionOutput_0_1    3
#   define DECODE_TrellisTransitionOutput_1_0    3
#   define DECODE_TrellisTransitionOutput_1_1    0
#   define DECODE_TrellisTransitionOutput_2_0    1
#   define DECODE_TrellisTransitionOutput_2_1    2
#   define DECODE_TrellisTransitionOutput_3_0    2
#   define DECODE_TrellisTransitionOutput_3_1    1
#   define DECODE_TrellisTransitionOutput_4_0    3
#   define DECODE_TrellisTransitionOutput_4_1    0
#   define DECODE_TrellisTransitionOutput_5_0    0
#   define DECODE_TrellisTransitionOutput_5_1    3
#   define DECODE_TrellisTransitionOutput_6_0    2
#   define DECODE_TrellisTransitionOutput_6_1    1
#   define DECODE_TrellisTransitionOutput_7_0    1
#   define DECODE_TrellisTransitionOutput_7_1    2

#   define DECODE_TrellisTransitionInput_0    0
#   define DECODE_TrellisTransitionInput_1    1
#   define DECODE_TrellisTransitionInput_2    0
#   define DECODE_TrellisTransitionInput_3    1
#   define DECODE_TrellisTransitionInput_4    0
#   define DECODE_TrellisTransitionInput_5    1
#   define DECODE_TrellisTransitionInput_6    0
#   define DECODE_TrellisTransitionInput_7    1


#endif

