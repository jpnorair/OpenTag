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
  * @file       /OTlib/alp_logger.c
  * @author     JP Norair
  * @version    V1.0
  * @date       1 July 2011
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

void alp_proc_logger(alp_record* in_rec, alp_record* out_rec, \
                        Queue* in_q, Queue* out_q, id_tmpl* user_id ) {
    
    out_rec->payload_length = 0;
    
    // Only root can log directly (this is an important security firewall)
    if (auth_isroot(user_id)) {    
        out_rec->flags          = in_rec->flags & ~(ALP_FLAG_MB | ALP_FLAG_ME);
        out_rec->dir_id         = 4;
        out_rec->dir_cmd        = 0;
        out_rec->payload_length = in_rec->payload_length;
        
        // Calling logger echos input queue to output queue.
        // Caller needs to decide what to do with the output queue.
        if (in_q != out_q) {
            q_writestring(out_q, in_q->getcursor, out_rec->payload_length);
        }                    
    }
}

#endif


