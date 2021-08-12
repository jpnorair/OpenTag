/*  Copyright 2010-2012, JP Norair
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
  */
/**
  * @file       /otlibext/applets_std/sys_sig_powerdown_[msp430fast].c
  * @author     JP Norair
  * @version    V1.0
  * @date       24 Oct 2012
  * @brief      MSP430 Fast Sleep Routine
  *
  * This powerdown routine is specific to MSP430F5 and CC430 chips.  It uses
  * LPM1 for sleeping during RF instead of LPM2, but it will shut-off the SMCLK
  * during Radio sleep.
  *
  * LPM2 puts the supply voltage monitors in a low-duty state, which saves some
  * power but causes a 100us delay on wake-up.  Hence, this is the "fast" 
  * powerdown routine because LPM1 has only a 2us delay on wake-up.  The RF
  * driver should actually be able to tolerate a 100us delay, but nonetheless
  * this routine does not require that it does.  As a result, RX-TX turnaround
  * may be slightly faster.  It is also recommended for systems that must TX
  * background packet floods.
  */

#include "OTAPI.h"
#include <otplatform.h>


#ifdef EXTF_sys_sig_powerdown
void sys_sig_powerdown(ot_int code) {
    //static const ot_u8 sleep_bits[4] = { 0x18, 0x58, 0x98, 0xD8 };    //0, 1, 2, 3
    ot_u16 sleep_bits;
    
    //00D8 is LPM3 with Interrupt, 0058 is LPM1 with interrupt
    sleep_bits  = (code == 3) ? 0x00D8 : 0x0058;
    
    // SMCLK_OFF setting is bit 1 (0x0002)
    // code will be 2 on Radio sleep.
    code       &= 2;        
    UCS->CTL6  |= code;     

    // Put in LPM1 or 3
    __bis_SR_register(sleep_bits);

    // On wakeup, restore SMCLK
    UCS->CTL6  &= ~code; 
}
#endif
