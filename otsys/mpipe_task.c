/* Copyright 2010-2014 JP Norair
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
  * @file       /otsys/mpipe_task.c
  * @author     JP Norair
  * @version    R102
  * @date       27 Aug 2014
  * @brief      Message Pipe (MPIPE) Task Interface
  * @ingroup    MPipe
  *
  * @todo       Use sysqueue or a session structure to allow truly queued
  *             communications.
  ******************************************************************************
  */


#include <otstd.h>
#if (OT_FEATURE(MPIPE) == ENABLED)

#include <otsys/mpipe.h>

//#include <otplatform.h>



mpipe_struct mpipe;




void mpipe_connect(void* port_id) {
///@todo no hard-coded input for second arg
    sys.task_MPA.latency = mpipedrv_init(port_id, MPIPE_default);
}

void mpipe_disconnect(void* port_id) {
    mpipe_close();
    mpipedrv_detach(port_id);
}


#ifndef EXTF_mpipe_status
mpipe_state mpipe_status() {
    return mpipe.state;
}
#endif


void mpipe_open() {
	sys.task_MPA.event = 0;
	mpipedrv_rx(False, MPIPE_High );
}

void mpipe_close() {
    sys.task_MPA.event = 0;
    //mpipedrv_kill();
}


void sub_mpipe_actuate(ot_u8 new_event, ot_u8 new_reserve, ot_uint new_nextevent) {
/// Kernel should be pre-empted in order to cancel the currently scheduled
/// task for MPipe and replace it with this one (if any task is scheduled).
    sys.task_MPA.event      = new_event;
    sys.task_MPA.reserve    = new_reserve;
    sys_preempt(&sys.task_MPA, new_nextevent);
}


void mpipe_send() {
///@todo A session stack could be implemented for MPipe Task.  For now, Sending (TX)
/// will just fall-through if mpipe is occupied
    //mpipedrv_unblock();
	//sub_mpipe_actuate(3, 1, (ot_uint)mpipedrv_tx(False, MPIPE_High));
    mpipe_txschedule(0);
}


void mpipe_txschedule(ot_int wait) {
    if (sys.task_MPA.event == 0) {
        sub_mpipe_actuate(2, 1, wait);
    }
}


void mpipe_rxschedule(ot_int wait) {
    if (sys.task_MPA.event == 0) {
        sub_mpipe_actuate(4, 1, wait);
    }
}


void mpipeevt_txdone(ot_int code) {
    // If driver returns 0, it closes connection itself.  Reopen in RX.
    // If driver returns >0, the task must delay the connection termination.
    ot_u8 nextevent;
    nextevent = 3 + (code==0);
    sub_mpipe_actuate(nextevent, 1, code);
    //sub_mpipe_actuate(4, 1, code);
}


void mpipeevt_rxinit(ot_int code) {
    sub_mpipe_actuate(4, 1, code);
}


void mpipeevt_rxdetect(ot_int code) {
    sub_mpipe_actuate(3, 1, (ot_uint)code);
}


void mpipeevt_rxdone(ot_int code) {
/// "32" in the array is given as the maximum time for protocol parsing.  It might need
/// to be more dynamic, depending on protocol and length of packet.  In the future, there
/// might be a "guess runtime" function in ALP that inspects these things.
#if (defined(MPIPE_USB) || (BOARD_FEATURE_USBCONVERTER == ENABLED))
	sub_mpipe_actuate(1, 32, 0);
#else
    static const ot_u8 params[] = { 1, 32, 4, 1 };
    ot_u8* task_params;

    task_params  = params;
    task_params += (code != 0) << 1;
    sub_mpipe_actuate(task_params[0], task_params[1], 0);
#endif
}



void mpipe_systask(ot_task task) {
    switch (sys.task_MPA.event) {
        //Task destructor & initializer: close mpipe
        ///@todo fix mpipe_connect(NULL)
        case 0: sys_taskinit_macro(task, mpipedrv_kill(), mpipe_connect(NULL)); 
                break;

        // RX successful: process the new frames -- note case fall through
        case 1: {
            ALP_status status;
            status      = alp_parse_message(&mpipe.alp, NULL);
            mpipedrv_clear();
            switch (status) {
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
        case 2: //mpipe_send();
                mpipedrv_unblock();
                sub_mpipe_actuate(3, 1, (ot_uint)mpipedrv_tx(False, MPIPE_High));
                break;

        // TX/RX timeout -- note case fall-through
        case 3: mpipedrv_kill();
                //mpipedrv_clear();

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


#endif




