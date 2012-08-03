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
  * @file       /otlib/alp_logger.c
  * @author     JP Norair
  * @version    V1.0
  * @date       20 July 2012
  * @brief      ALP to Logger processor
  * @ingroup    ALP
  *
  * When called by ALP, Logger acts the same way "echo" does.  It echos to the
  * logger output, which by official implementation is always Mpipe.  Therefore,
  * this is one way to send a message to the client connected to a server.
  * 
  ******************************************************************************
  */


#include "alp.h"

#if (   (OT_FEATURE(SERVER) == ENABLED) \
     && (OT_FEATURE(ALP) == ENABLED) \
     && (LOG_FEATURE(ANY) == ENABLED) )

#include "auth.h"
#include "queue.h"

ot_bool alp_proc_logger(alp_tmpl* alp, id_tmpl* user_id) {
/// Logger ALP is like ECHO.  The input is copied to the output.
    
    // Only root can log directly (this is an important security firewall)
    if (auth_isroot(user_id) == False)
        return False;
        
    alp->outrec.flags   = alp->inrec.flags;
    alp->outrec.plength = alp->inrec.plength;
    
    if (alp->inq != alp->outq) {
        q_writestring(alp->outq, alp->inq->getcursor, alp->inrec.plength);
    }
    
    return True;
}

#endif


