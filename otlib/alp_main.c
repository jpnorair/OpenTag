/*  Copyright 2010-2011, JP Norair
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
  * @file       /OTlib/alp_main.c
  * @author     JP Norair
  * @version    V1.0
  * @date       08 May 2011
  * @brief      Application Layer Protocol Main Processor
  * @ingroup    ALP
  *
  * Application Layer protocols (ALP's) are directive-based protocols that
  * interface only with the application layer.  Other protocols supported by
  * DASH7 Mode 2 exist at lower levels, even though they may interact with
  * higher level data (this is done to improve size and performance).  
  *
  * Some ALP's are defined in the DASH7 Mode 2 Specification.
  * - Filesystem access (Veelite)
  * - Sensor Configuration (pending draft of ISO 21451-7)
  * - Security Configuration (pending draft of ISO 29167-7)
  *
  *
  ******************************************************************************
  */

#include "alp.h"
#include "OTAPI.h"

#if ((OT_FEATURE(ALP) == ENABLED) && (OT_FEATURE(SERVER) == ENABLED))

#define ALP_FILESYSTEM  1
#define ALP_SENSORS     (OT_FEATURE(SENSORS) == ENABLED)
#define ALP_SECURITY    ((OT_FEATURE(NL_SECURITY) == ENABLED) || (OT_FEATURE(DLL_SECURITY) == ENABLED))
#define ALP_LOGGER      (LOG_FEATURE(ANY) == ENABLED)
#define ALP_DASHFORTH   (OT_FEATURE(DASHFORTH) == ENABLED)
#define ALP_API         ((OT_FEATURE(ALPAPI) == ENABLED)*3)
#define ALP_EXT         (OT_FEATURE(ALPEXT) == ENABLED)


#define ALP_MAX         9
#define ALP_FUNCTIONS   (   ALP_FILESYSTEM \
                          + ALP_SENSORS \
                          + ALP_SECURITY \
                          + ALP_LOGGER \
                          + ALP_DASHFORTH \
                          + ALP_API \
                          + ALP_EXT     )


typedef void (*sub_proc)(alp_record*, alp_record*, Queue*, Queue*, id_tmpl*);

void sub_proc_null(alp_record* a0, alp_record* a1, Queue* a2, Queue* a3, id_tmpl* a4) {
}


void alp_load_retval(ot_bool respond, ot_u8 out_dir_cmd, ot_u16 retval, 
                     alp_record* out_rec, Queue* out_q) {
/// Write back the twobye retval integer when response is enabled
    if (respond) {
        out_rec->flags         &= ~ALP_FLAG_CF;
        out_rec->dir_cmd        = out_dir_cmd;
        out_rec->payload_length = 2;
        q_writeshort(out_q, retval);
    }
}

  
void alp_proc(alp_record* in_rec, alp_record* out_rec, \
                Queue* in_q, Queue* out_q, id_tmpl* user_id) {
    
    ot_u8       dir_i;
    sub_proc    proc = &sub_proc_null;
    
    // 0x80 directives are OTAPI, but they are compressed into the call table
    out_rec->dir_id = in_rec->dir_id;
    dir_i           = in_rec->dir_id;
    
    if (in_rec->dir_id >= 0x80) {
        dir_i -= (0x80-ALP_FILESYSTEM-ALP_SENSORS-ALP_SECURITY-ALP_LOGGER-ALP_DASHFORTH-1);
    }
    
    switch (dir_i) {
        case 0: //Null ALP would go here, if it needs to
            break;
        
        case 1: //Filesystem ALP
            proc = &alp_proc_filedata;
            break;
        
#       if (OT_FEATURE(SENSORS) == ENABLED)
        case (1+ALP_FILESYSTEM):
            break;
#       endif
        
#       if (OT_FEATURE(SECURITY) == ENABLED)
        case (1+ALP_FILESYSTEM+ALP_SENSORS):
            break;
#       endif
        
#       if (LOG_FEATURE(ANY) == ENABLED)
        case (1+ALP_FILESYSTEM+ALP_SENSORS+ALP_SECURITY):
            break;
#       endif
        
#       if (OT_FEATURE(DASHFORTH) == ENABLED)
        case (1+ALP_FILESYSTEM+ALP_SENSORS+ALP_SECURITY+ALP_LOGGER+ALP_DASHFORTH):
            break;
#       endif
        
#       if (OT_FEATURE(ALPAPI) == ENABLED)
        case (1+ALP_FILESYSTEM+ALP_SENSORS+ALP_SECURITY+ALP_LOGGER+ALP_DASHFORTH):
            proc = &alp_proc_api_session;
            break;
        
        case (2+ALP_FILESYSTEM+ALP_SENSORS+ALP_SECURITY+ALP_LOGGER+ALP_DASHFORTH):
            proc = &alp_proc_api_system;
            break;
        
        case (3+ALP_FILESYSTEM+ALP_SENSORS+ALP_SECURITY+ALP_LOGGER+ALP_DASHFORTH):
            proc = &alp_proc_api_query;
            break;
#       endif

        default:
#       if (OT_FEATURE(ALPEXT) == ENABLED)
            proc = &otapi_alpext_proc;
#       else
            proc = &sub_proc_null;
#       endif
            break;
    }
    
    proc(in_rec, out_rec, in_q, out_q, user_id);
    
    
    /*
    const sub_proc proc[ALP_FUNCTIONS] = { 
#   if (1)
        &alp_proc_filedata,                     // 0x01 -> 0
#   endif
#   if (OT_FEATURE(SENSORS) == ENABLED)
        &alp_proc_sensors,                      // 0x02 -> 1
#   endif
#   if (OT_FEATURE(SECURITY) == ENABLED)        
        &alp_proc_security,                     // 0x03 -> 2
#   endif
#   if (LOG_FEATURE(ANY) == ENABLED)
        &alp_proc_logger,                       // 0x04 -> 3
#   endif
#   if (OT_FEATURE(DASHFORTH) == ENABLED)
        &alp_proc_dashforth,                    // 0x05 -> 4
#   endif

#   if (OT_FEATURE(ALPAPI) == ENABLED)
        &alp_proc_api_session,                 // 0x80 -> 5
        &alp_proc_api_system,                  // 0x81 -> 6
        &alp_proc_api_query                    // 0x82 -> 7
#   else
        &sub_proc_null,
        &sub_proc_null,
        &sub_proc_null
#   endif

};
    
    // 0x80 directives are OTAPI, but they are compressed into the call table
    ot_u8 dir_i;
    out_rec->dir_id = in_rec->dir_id;
    dir_i          -= (in_rec->dir_id >= 0x80) ? (0x80-4) : 1;
    
    // Prevent segmentation fault
    if (dir_i < ALP_FUNCTIONS) {
        // The proc function will sort-out the rest of the out_rec attributes
        proc[dir_i](in_rec, out_rec, in_q, out_q, user_id);
    }
    */
}


#endif

