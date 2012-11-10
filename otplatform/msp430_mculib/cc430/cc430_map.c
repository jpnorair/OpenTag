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
/** @file       /OTplatform/~MSP430/CC430/cc430_map.c
  * @author     JP Norair
  * @version    V1.0
  * @date       20 Oct 2012
  * @brief      Mapping Lists
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#include "cc430_map.h"

#ifdef __CC430__

/******************************************************************************/
/*                 Port Lists for Init and Port Grounding                     */
/******************************************************************************/

const u8* cc430_pout_list[CC430_TOTAL_PORTS] = {
    (u8*)0x0202,     //Port 1 DOUT
    (u8*)0x0223,     //Port 2 DOUT
    (u8*)0x0222,     //Port 3 DOUT
#   ifdef _GPIO4
    (u8*)0x0223,     //Port 4 DOUT
#   endif
    (u8*)0x0242,     //Port 5 DOUT
    (u8*)0x0322      //Port J DOUT
};

const u8* cc430_pdir_list[CC430_TOTAL_PORTS] = {
    (u8*)0x0204,     //Port 1 DDIR
    (u8*)0x0225,     //Port 2 DDIR
    (u8*)0x0224,     //Port 3 DDIR
#   ifdef _GPIO4
    (u8*)0x0225,     //Port 4 DDIR
#   endif
    (u8*)0x0244,     //Port 5 DDIR
    (u8*)0x0324      //Port J DDIR
};

#endif
