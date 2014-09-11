/* Copyright 2013-2014 JP Norair
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
OT_WEAK ot_bool alp_proc_logger(alp_tmpl* alp, id_tmpl* user_id) {
    return True;
}

#else

#include <otlib/auth.h>
#include <otlib/queue.h>

///@todo replace INREC calls with direct access from input

OT_WEAK ot_bool alp_proc_logger(alp_tmpl* alp, id_tmpl* user_id) {
/// Logger ALP is like ECHO.  The input is copied to the output.

    // Only root can log directly (this is an important security firewall)
    if (auth_isroot(user_id)) {
        alp->OUTREC(FLAGS)  = *alp->inq->getcursor++;
        alp->OUTREC(PLEN)   = *alp->inq->getcursor++;
        alp->inq->getcursor+= 2;

        if (alp->inq != alp->outq) {
            q_writestring(alp->outq, alp->inq->getcursor, alp->OUTREC(PLEN));
        }
    }
    return True;
}

#endif


