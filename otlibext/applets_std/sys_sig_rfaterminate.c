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
  * @file       /otlibext/applets_std/sys_sig_rfaterminate.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2011
  * @brief      Standard RFA terminate routine
  *
  * This applet implements a commonly used LED and logging routine that is used
  * when the System RFA Terminate callback is called.
  * In normal applications, Kernel & RF System callbacks are not terribly
  * useful, but they are good for test/debug.  The RX callback can also
  * be used to log all received frames, which can sometimes be useful,
  * or the TX callback to log all transmitted frames.  This app demos both
  * of these features.
  */



#include "OTAPI.h"
#include "radio.h"


#ifdef EXTF_sys_sig_rfaterminate
void sys_sig_rfaterminate(ot_int pcode, ot_int scode) {
/// Hint: RFxRX_e1 label means the received frame has bad CRC.
    ot_u8   loglabel[8] = {'R', 'F', 'F', 0, 'X', '_', 0, 0};
    ot_u8*  logdata;
    ot_int  logdata_len;

    otapi_led2_off();   //Orange LED off
    otapi_led1_off();   //Green LED off

    /// Error Handler:
    /// <LI> If there is an error code, put the code in the log label.     </LI>
    /// <LI> Else if the code is for RX termination, just return.  You can
    ///      comment this and uncomment cases 1 & 2 from the switch block if
    ///      you want to log on RX termination.  </LI>
    /// <LI> Else, the code is for TX or CCA.  Append the channel number to 
    ///      the log label (in hex) </LI> 
    if (scode < 0) {
        loglabel[6] = 'e';
        loglabel[7] = (ot_int)('0') - scode;
    }
    else if (pcode < 3) {
    	return;
    }
    else {
    	otutils_bin2hex(&(phymac[0].channel), &loglabel[6], 1);
    }

    /// Look at the control code an form the label to reflect the type
    switch (pcode) {
        /// RX driver process termination:
        /// (1) background scan ended, (2) or foreground scan ended
        //case 1: loglabel[2] = 'B';
        //case 2: loglabel[3] = 'R';
        //        logdata_len = rxq.length;
        //        logdata     = rxq.front;
        //        break;

        /// TX CCA/CSMA driver process termination:
        /// (3) TX CSMA process ended
        case 3: loglabel[2] = 'C';
                loglabel[3] = 'C';
                loglabel[4] = 'A';
                logdata_len = 0;
                logdata     = NULL; // suppress compiler warning
                break;

        /// TX driver process termination
        /// Background Flood (4) or Foreground TX (5): turn-on green and log TX
        case 4: loglabel[2] = 'B';
        case 5: loglabel[3] = 'T';
                logdata_len = txq.length;
                logdata     = txq.front;
                break;

        /// Unknown code, don't log any data
        default: return;
    }

    /// Log in ASCII hex the prepared driver message
    otapi_log_msg(MSG_raw, 8, logdata_len, loglabel, logdata);
}
#endif


