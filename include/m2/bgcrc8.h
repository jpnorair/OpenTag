/* Copyright 2016 JP Norair
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
  * @file       /include/m2/bgcrc.h
  * @author     JP Norair
  * @version    R103
  * @date       18 Nov 2016
  * @brief      Background Frame CRC8 compuation
  * @defgroup   Encode (Encode Module)
  * @ingroup    Encode
  *
  * Background-frame CRC-8 computation is part of the LoRa implementation, 
  * although in the future it might extend to other PHYs, too.  With LoRa, 
  * there is a big penalty for adding a 7th byte to the background frame, and
  * the Baicheva Polynomial used with this CRC8 implementation equals the
  * performance of CRC-16 up to 119 bits -- BG frame is only 40 bits.
  *
  * Refer to the LoRa/SX127x radio_rm2.c file for examples how to use this with
  * the normal encoder workflow.
  ******************************************************************************
  */


#include <otsys/config.h>
#if !defined(__BGCRC8_H) && OT_FEATURE(M2)
#define __BGCRC8_H

#include <otsys/types.h>

ot_u8 bgcrc8_check(ot_u8* block_addr);

void bgcrc8_put(ot_u8* block_addr);
    
#endif



