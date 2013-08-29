/* Copyright 2010-2011 JP Norair
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
  * @file       /otlib/buffers.c
  * @author     JP Norair
  * @version    V1.0
  * @date       1 June 2011
  * @brief      An inline, volatile set of stream buffers used with Queues
  * @ingroup    Buffers
  *
  ******************************************************************************
  */

#include "buffers.h"


#define DIR_ENABLED (OT_FEATURE(NDEF) || OT_FEATURE(ALP) || OT_FEATURE(MPIPE))
#define TXRX_SIZE   ((M2_PARAM_MAXFRAME + (M2_PARAM_MAXFRAME & 1)) * (OT_FEATURE_SERVER == ENABLED))
#define DIR_SIZE    ((OT_PARAM_BUFFER_SIZE - (TXRX_SIZE*2))/2)

#if ((DIR_SIZE < 0) && DIR_ENABLED)
#   error "DIR Queues (Needed for ALP/NDEF/MPIPE) are configured with negative allocation."
#elif ((DIR_SIZE < 256) && DIR_ENABLED)
#   warn "DIR Queues (Needed for ALP/NDEF/MPIPE) are configured with < 256 byte allocation."
#endif


ot_u8 otbuf[OT_PARAM_BUFFER_SIZE];

#if (OT_FEATURE(SERVER) == ENABLED)
    Queue rxq;
    Queue txq;
#endif

#if (DIR_ENABLED)
    Queue otmpin;
    Queue otmpout;
#endif



#ifndef EXTF_buffers_init
void buffers_init() {
#   if (OT_FEATURE(SERVER) == ENABLED)
    q_init(&rxq,    otbuf,              TXRX_SIZE);
    q_init(&txq,    otbuf+TXRX_SIZE,    TXRX_SIZE);    
#   endif
#   if (DIR_ENABLED)
    q_init(&otmpin,     otbuf+(TXRX_SIZE*2),            DIR_SIZE );
    q_init(&otmpout,    otbuf+(TXRX_SIZE*2)+DIR_SIZE,   DIR_SIZE );
#   endif
}
#endif



/// Experimental
void buffers_swap(Queue* q1, Queue* q2) {
    Queue scratch;
    
    q_copy(&scratch, q1);
    q_copy(q1, q2);
    q_copy(q2, &scratch);
}




