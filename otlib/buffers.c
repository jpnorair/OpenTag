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
  * @file       /otlib/buffers.c
  * @author     JP Norair
  * @version    R100
  * @date       8 Oct 2013
  * @brief      An inline, volatile set of stream buffers used with ot_queues
  * @ingroup    Buffers
  *
  * @todo cleanup buffers allocation model, so that each buffer allocation is
  *       individually defined & declared.
  ******************************************************************************
  */

#include <otstd.h>
#include <otlib/buffers.h>


#define ALP_ENABLED (OT_FEATURE(NDEF) || OT_FEATURE(ALP) || OT_FEATURE(MPIPE))
#define TXRX_SIZE   ((M2_PARAM_MAXFRAME + (M2_PARAM_MAXFRAME & 1)) * (OT_FEATURE_SERVER == ENABLED))
#define ALP_SIZE    ((OT_PARAM_BUFFER_SIZE - (TXRX_SIZE*2))/2)

#if ((ALP_SIZE < 0) && ALP_ENABLED)
#   error "ALP ot_queues (Needed for ALP/NDEF/MPIPE) are configured with negative allocation."
#elif ((ALP_SIZE < 256) && ALP_ENABLED)
#   warn "ALP ot_queues (Needed for ALP/NDEF/MPIPE) are configured with < 256 byte allocation."
#endif


ot_u8 otbuf[OT_PARAM_BUFFER_SIZE];

#if (OT_FEATURE(SERVER) == ENABLED)
    ot_queue rxq;
    ot_queue txq;
#endif

#if (ALP_ENABLED)
    ot_queue otmpin;
    ot_queue otmpout;
#endif



#ifndef EXTF_buffers_init
void buffers_init() {
#   if (OT_FEATURE(SERVER) == ENABLED)
    q_init(&rxq,    otbuf,              TXRX_SIZE);
    q_init(&txq,    otbuf+TXRX_SIZE,    TXRX_SIZE);    
#   endif
#   if (ALP_ENABLED)
    q_init(&otmpin,     otbuf+(TXRX_SIZE*2),            ALP_SIZE );
    q_init(&otmpout,    otbuf+(TXRX_SIZE*2)+ALP_SIZE,   ALP_SIZE );
#   endif
}
#endif



/// Experimental
void buffers_swap(ot_queue* q1, ot_queue* q2) {
    ot_queue scratch;
    
    q_copy(&scratch, q1);
    q_copy(q1, q2);
    q_copy(q2, &scratch);
}




