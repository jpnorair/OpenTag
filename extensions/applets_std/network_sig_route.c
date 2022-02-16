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
  * @file       /otlibext/applets_std/network_sig_route.c
  * @author     JP Norair
  * @version    R102
  * @date       9 May 2017
  * @brief      Standard Network Routing Callback Routine
  *
  * Network layer uses a callback when a packet has been successfully received
  * and it is routed.  This routine will log the type of packet/frame that has
  * been received & routed, and its contents.
  */

#include <otstd.h>
#include <m2api.h>
#include <otlib/logger.h>
#include <otlib/buffers.h>

#ifdef EXTF_network_sig_route
void network_sig_route(void* route, void* session) {
#   if (OT_FEATURE(MPIPE))
    static const char* label_dialog = "M2_Dialog";
    static const char* label_nack   = "M2_Nack";
    static const char* label_stream = "M2_Stream";
    static const char* label_snack  = "M2_SNack";
    static const ot_u8 label_len[]  = { 9, 7, 9, 8 };
    
    const char* label[] = { label_dialog, label_nack, label_stream, label_snack };
    
	ot_u8 protocol;

	protocol = ((m2session*)session)->extra & 3;
	logger_msg(	MSG_raw,
					label_len[protocol], q_length(&rxq),
					label[protocol], rxq.front
					);
#   endif
}
#endif
