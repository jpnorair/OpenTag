/*  Copyright 2008-2022, JP Norair
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted provided that the following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright 
  *    notice, this list of conditions and the following disclaimer in the 
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
  * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
  * POSSIBILITY OF SUCH DAMAGE.
  */
/**
  * @file       /otlibext/applets_std/dll_sig_rfterminate.c
  * @author     JP Norair
  * @version    V1.0
  * @date       10 October 2012
  * @brief      Informative RF terminate routine
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
#include <m2/radio.h>  //to get "phymac" data
#include <otlib/logger.h>

#ifdef EXTF_dll_sig_rfterminate
void dll_sig_rfterminate(ot_int pcode, ot_int scode) {
/// DLL Task codes
/// 1 = Packet Processing (rfterminate() not called here)
/// 2 = Session invocation (rfterminate() not called here)
/// 3 = RX termination (!)
/// 4 = TX CSMA termination (!)
/// 5 = TX termination (!)

/// Hint: RFxRX_e1 label means the received frame has bad CRC.
    ot_u8   loglabel[7] = {'R', 'F', 0, 'X', '_', 0, 0};
    ot_u8*  logdata;
    ot_int  logdata_len;

    BOARD_led2_off();   //Orange LED off
    BOARD_led1_off();   //Green LED off

    /// Error Handler:
    /// <LI> If there is an error code, put the code in the log label.     </LI>
    /// <LI> Else, the code is for TX or CCA.  Append the channel number to
    ///      the log label (in hex) </LI>
    if (scode < 0) {
        loglabel[5] = 'e';
        loglabel[6] = (ot_int)('0') - scode;
    }
    else {
    	otutils_bin2hex(&(phymac[0].channel), &loglabel[6], 1);
    }

    /// Look at the control code an form the label to reflect the type
    switch (pcode) {
        /// RX driver process termination: This can happen a lot, so a lot of
        /// times it will be commented-out.
        case 3: loglabel[2] = 'R';
                logdata_len = q_length(&rxq);
                logdata     = rxq.front;
                break;

        /// TX CSMA-CA driver process termination:
        case 4: loglabel[2] = 'C';
                loglabel[3] = 'A';
                logdata_len = 0;
                logdata     = NULL; // suppress compiler warning
                break;

        /// TX driver process termination
        case 5: loglabel[2] = 'T';
                logdata_len = q_length(&txq);
                logdata     = txq.front;
                break;

        /// Unknown code, don't log any data
        default: return;
    }

    /// Log in ASCII hex the prepared driver message
    logger_msg(MSG_raw, 7, logdata_len, loglabel, logdata);
}
#endif


