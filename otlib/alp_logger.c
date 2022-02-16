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
  * @file       /otlib/alp_logger.c
  * @author     JP Norair
  * @version    R101
  * @date       24 Mar 2014
  * @brief      ALP to Logger processor
  * @ingroup    ALP
  *
  * When called by ALP, Logger acts the same way "echo" does.  It echos to the
  * logger output, which by official implementation is always Mpipe.  Therefore,
  * this is one way to send a message to the client connected to a server.
  *
  ******************************************************************************
  */



#include <otstd.h>
#include <otlib/alp.h>

#if !(OT_FEATURE(SERVER) && OT_FEATURE(ALP) && OT_FEATURE(LOGGER))
OT_WEAK ot_bool alp_proc_logger(alp_tmpl* alp, const id_tmpl* user_id) {
    return True;
}

#else

#include <otlib/auth.h>
#include <otlib/queue.h>

///@todo replace INREC calls with direct access from input

OT_WEAK ot_bool alp_proc_logger(alp_tmpl* alp, const id_tmpl* user_id) {
/// Logger ALP is like ECHO.  The input is copied to the output.

    // Only root can log directly (this is an important security firewall)
    if (auth_isroot(user_id)) {
        alp->OUTREC(FLAGS)  = q_readbyte(alp->inq);
        alp->OUTREC(PLEN)   = q_readbyte(alp->inq);
        alp->inq->getcursor+= 2;

        if (alp->inq != alp->outq) {
            q_movedata(alp->outq, alp->inq, alp->OUTREC(PLEN));
        }
    }
    return True;
}

#endif


