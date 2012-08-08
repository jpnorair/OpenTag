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
  * @file       /otlib/ndef.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 Jul 2012
  * @brief      NDEF interface
  * @ingroup    NDEF (NDEF Module)
  *
  * The NDEF interface implementation contains only OTAPI callbacks, which are
  * associated to the NDEF interface.  At present, the only interface that 
  * supports NDEF is MPipe, which provides a 1:1 persistent connection.  MPipe
  * is typically implemented as a serial port or USB CDC.
  ******************************************************************************
  */

#include "OT_config.h"

#if (OT_FEATURE(NDEF) == ENABLED)
#include "OTAPI.h"
#include "buffers.h"
#include "ndef.h"
#include "mpipe.h"
#include "auth.h"



/** OTAPI functions within NDEF module      <BR>
  * ========================================================================<BR>
  * Use the MPipe to move NDEF packets in and out.  These functions should be
  * attached to the MPipe TXDONE callback (otapi_ndef_idle) and the RXDONE 
  * callback (otapi_ndef_proc) if you are building an NDEF server command pipe.
  */

#ifndef EXTF_otapi_ndef_idle
void otapi_ndef_idle(void* tmpl) {
/// This implementation of otapi_ndef_idle assumes that NDEF is coming from
/// Mpipe.  tmpl input parameter can be NULL.  Otherwise, tmpl should be a 
/// pointer to an alp_tmpl used for the NDEF/ALP interface.
    
    // MPipe-ALP variant (input param can be NULL)
#ifndef BOARD_FEATURE_MPIPE_QMGMT
    q_empty(mpipe_alp.inq);
    q_empty(mpipe_alp.outq);
#endif
    //out_q->back -= mpipe_footerbytes();
    mpipe_rxndef( mpipe_alp.inq->front, False, MPIPE_Low );
}
#endif






#ifndef EXTF_otapi_ndef_proc
void otapi_ndef_proc(void* tmpl) {
/// This implementation of otapi_ndef_proc assumes that NDEF is coming from
/// Mpipe.  tmpl input parameter can be NULL.  Otherwise, tmpl should be a 
/// pointer to an alp_tmpl used for the NDEF/ALP interface.

    switch (alp_parse_message(&mpipe_alp, NULL)) {
        //wipe queue and go back to idle listening
        case MSG_Null:
            otapi_ndef_idle(0);
            break;
        
        //listen for next record/message
        case MSG_Chunking_In:   
            mpipe_rxndef( mpipe_alp.inq->front, False, MPIPE_High );
            break;
        
        //transmit next record/message
        case MSG_Chunking_Out:
        case MSG_End:
            mpipe_txndef( mpipe_alp.outq->front, False, MPIPE_High );
            break;
    }
}
#endif




#endif


