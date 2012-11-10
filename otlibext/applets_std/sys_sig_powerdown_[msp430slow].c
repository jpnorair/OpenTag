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
  * @file       /otlibext/applets_std/sys_sig_powerdown_[msp430slow].c
  * @author     JP Norair
  * @version    V1.0
  * @date       24 Oct 2012
  * @brief      MSP430 Slow Sleep Routine
  *
  * This powerdown routine is specific to MSP430F5 and CC430 chips.  It uses
  * LPM2 for sleeping during RF, LPM1 for sleeping during MPipe, and LPM3 for
  * sleeping whenever those things are not on.
  *
  * LPM2 puts the supply voltage monitors in a low-duty state, which saves some
  * power but causes a 100us delay on wake-up.  Hence, this is the "slow" 
  * powerdown routine because LPM2 incurs this delay.  It is not recommended 
  * for systems that must TX background packet floods or for systems that are
  * doing software-based FEC decoding.
  */

#include "OTAPI.h"
#include "OT_platform.h"


#ifdef EXTF_sys_sig_powerdown
void sys_sig_powerdown(ot_int code) {
    static const ot_u8 sleep_index[4] = { 0x18, 0x58, 0x98, 0xD8 };    //0, 1, 2, 3
    ot_u16 sleep_bits;
    
    code       &= 3;
    sleep_bits  = sleep_index[code];
    
    // SMCLK_OFF setting is bit 1 (0x0002)
    // code will be 2 on Radio sleep.
    code       &= 2;
    UCS->CTL6  |= code;

    // Put in LPM1 or 3
    __bis_SR_register(sleep_bits);
    __no_operation();

    // On wakeup, restore SMCLK
    UCS->CTL6  &= ~code;
}
#endif
