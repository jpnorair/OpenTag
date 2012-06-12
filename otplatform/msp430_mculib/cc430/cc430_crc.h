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
/** @file       /Platforms/CC430/cc430_lib/cc430_crc.h
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Dec 2009
  * @brief      Library resources for CRC peripheral
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#ifndef __CC430_LIB_CRC_H
#define __CC430_LIB_CRC_H

#include "cc430_map.h"



#define CRC_DIR_FORWARD         ((u16)0x0000)
#define CRC_DIR_REVERSE         ((u16)0xFFFF)



void CRC_Init( u16 init_word );

void CRC_Calc( u16 word, u16 direction );

u16 CRC_Result( u16 direction );


#endif

