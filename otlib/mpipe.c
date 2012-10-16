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
    sys.task_MPA.latency = mpipe_init_driver(port_id);
}


#ifndef EXTF_mpipe_status
mpipe_state mpipe_status() {
    return mpipe.state;
}
#endif



void sub_mpipe_actuate(ot_int event) {
    sys.task_MPA.event      = event;
    sys.task_MPA.reserve    = 1;
    sys.task_MPA.nextevent  = 0;
    sys_preempt();
}


void mpipe_open() { 
    sub_mpipe_actuate(6);
}

void mpipe_close() {
    sys.task_MPA.event = 0;
    mpipe_kill();
}


void mpipe_send() {
    sub_mpipe_actuate(2);
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
                case MSG_Chunking_Out:
                case MSG_End:           //goto systask_mpipe_TX;
            }
        } 
    
        // Initialize TX (wait for rest of frame) -- Note case fall-through
        case 2: 
        //systask_mpipe_TX:
        	sys.task_MPA.cursor = 32;
            mpipe_txndef(False, MPIPE_High);
        
        // (3) Frame timeout for TX once initiated, or RX once detected
        // (4) TX page-out bufferring
        case 3:
        case 4: sys.task_MPA.event      = 5;
                sys.task_MPA.nextevent  = TI2CLK(sys.task_MPA.cursor);
                break;
        
        // TX/RX timeout -- note case fall-through
        case 5: mpipe_kill();
        
        // Return to idle (passive RX)
        case 6: {
        //systask_mpipe_IDLE:
        systask_mpipe_RX:
            sys.task_MPA.event = 0;
            mpipe_rxndef(False, MPIPE_High );
        } break;
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


void mpipeevt_rxdetect(ot_int code) {
    sys.task_MPA.cursor = (ot_u8)code;
    sub_mpipe_actuate(3);
}


void mpipeevt_rxdone(ot_int code) {
/// 32 is an assumed maximum time to do the mpipe RX processing
    static const ot_u8 params[] = { 6, 1, 1, 32 };

    code                    = (code == 0);
    sys.task_MPA.event      = params[code];
    sys.task_MPA.reserve    = params[code+2];
    sys.task_MPA.nextevent  = 0;
    sys_preempt();
    
}


void mpipeevt_txdone(ot_int code) {
    sub_mpipe_actuate(6);
}

