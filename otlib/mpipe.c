/* Copyright 2010-2012 JP Norair
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
  * @file       /otlib/mpipe.c
  * @author     JP Norair
  * @version    V1.0
  * @date       30 September 2012
  * @brief      Message Pipe (MPIPE) Task Interface
  * @ingroup    MPipe
  *
  * 
  ******************************************************************************
  */


#include "mpipe.h"
#include "OTAPI.h"
#include "OT_platform.h"



mpipe_struct mpipe;



void mpipe_init(void* port_id) {
    sys.task_MPA.latency = mpipedrv_init(port_id);
}


#ifndef EXTF_mpipe_status
mpipe_state mpipe_status() {
    return mpipe.state;
}
#endif


void mpipe_open() {
	sys.task_MPA.event = 0;
	mpipedrv_rxndef(False, MPIPE_High );
}

void mpipe_close() {
    sys.task_MPA.event = 0;
    mpipedrv_kill();
}


void sub_mpipe_actuate(ot_u8 new_event, ot_u8 new_reserve, ot_uint new_nextevent) {
    sys.task_MPA.event      = new_event;
    sys.task_MPA.reserve    = new_reserve;
    sys_preempt(&sys.task_MPA, new_nextevent);
}


void mpipe_send() {
/// "32" is a magic number right now, which is probably longer than the largest packet
/// that can be TXed or RXed.
///@todo A session stack could be implemented for MPipe Task.  For now, Sending (TX)
/// will just fall-through if mpipe is occupied
	mpipedrv_txndef(False, MPIPE_High);
	sub_mpipe_actuate(3, 1, 32);
}


void mpipeevt_txdone(ot_int code) {
	sub_mpipe_actuate(4, 1, (ot_uint)code);
}


void mpipeevt_rxdetect(ot_int code) {
    sub_mpipe_actuate(3, 1, (ot_uint)code);
}


void mpipeevt_rxdone(ot_int code) {
/// "32" in the array is given as the maximum time for protocol parsing.  It might need
/// to be more dynamic, depending on protocol and length of packet.  In the future, there
/// might be a "guess runtime" function in ALP that inspects these things.
    static const ot_u8 params[] = { 4, 1, 1, 32 };
    code = (code == 0);
    sub_mpipe_actuate(params[code], params[code+2], 0);
}



void mpipe_systask(ot_task task) {
    switch (sys.task_MPA.event) {
        //Do Nothing
        case 0: break;
    
        // RX successful: process the new frames -- note case fall through
        case 1: {
            switch (alp_parse_message(&mpipe.alp, NULL)) {
                //wipe queue and go back to idle listening
                case MSG_Null:          //goto systask_mpipe_IDLE;
    
                //listen for next record/message
                case MSG_Chunking_In:   goto systask_mpipe_RX;
    
                //transmit next record/message (case fall-through)
                //case MSG_Chunking_Out:
                //case MSG_End:           //goto systask_mpipe_TX;
            }
        } 
    
        // Initialize TX: mpipe_send is used.
        case 2: mpipe_send();
                break;

        // TX/RX timeout -- note case fall-through
        case 3: mpipedrv_kill();
        
        // Return to idle (passive RX)
        case 4:
        systask_mpipe_RX:
        		mpipe_open();
        		break;
    }
}







#if defined(EXTF_mpipe_sig_txdone)
#   define SIG_TXDONE()     mpipe_sig_txdone(&sys.task_MPA)
#elif OT_FEATURE(MPIPE_CALLBACKS)
#   define SIG_TXDONE()     mpipe.sig.txdone(&sys.task_MPA)
#else
#   define SIG_TXDONE()     while(0)
#endif
#if defined(EXTF_mpipe_sig_rxdone)
#   define SIG_RXDONE()     mpipe_sig_rxdone(&sys.task_MPA)
#elif OT_FEATURE(MPIPE_CALLBACKS)
#   define SIG_RXDONE()     mpipe.sig.rxdone(&sys.task_MPA)
#else
#   define SIG_RXDONE()     while(0)
#endif
#if defined(EXTF_mpipe_sig_rxdetect)
#   define SIG_RXDETECT()   mpipe_sig_rxdetect(&sys.task_MPA)
#elif OT_FEATURE(MPIPE_CALLBACKS)
#   define SIG_RXDETECT()   mpipe.sig.rxdetect(&sys.task_MPA)
#else
#   define SIG_RXDETECT()   while(0)
#endif







