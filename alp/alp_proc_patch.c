/* Copyright 2014 JP Norair
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
  * @file       /hb_asapi/hb_alp_proc.c
  * @author     JP Norair
  * @version    R100
  * @date       12 Sept 2013
  * @brief      Patch function for alp_proc(), which adds ASAPI direct support
  * @ingroup    hb_asapi
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otlib/alp.h>

#if (ALP(CONFIT))
#   include <alp/confit.h>
#endif
#if (ALP(ASAPI))
#   include <alp/asapi.h>
#endif


#define ALP_NULL        1
#define ALP_FILESYSTEM  (ALP(FILE_MGR) == ENABLED)
#define ALP_API         (ALP(LLAPI) == ENABLED)
#define ALP_SECURITY    (ALP(SECURE_MGR) == ENABLED)
#define ALP_EXT         (OT_FEATURE(ALPEXT) == ENABLED)



#define ALP_FUNCTIONS   (   ALP_NULL \
                          + ALP_FILESYSTEM \
                          + ALP_API \
                          + ALP_SECURITY \
                          + ALP_LOGGER \
                          + ALP_DASHFORTH \
                          + ALP_CONFIT \
                          + ALP_ASAPI \
                          + ALP_EXT )


typedef ot_bool (*sub_proc)(alp_tmpl*, const id_tmpl*);




#ifdef EXTF_alp_proc
ot_bool alp_proc(alp_tmpl* alp, const id_tmpl* user_id) {
    static const sub_proc proc[9] = {
        &alp_proc_null,
        &alp_proc_filedata,     // 1, File I/O
        &alp_proc_null,         // 2, &alp_llapi not used for this build
        &alp_proc_null,         // 3, &alp_proc_sec not fully implemented
        &alp_proc_logger,       // 4, Logger
        &alp_proc_null,         // 5, &dashforth_proc not implemented yet
#       if (ALP(CONFIT))
            &confit_proc,       // 6, &confit_proc
#       else
            &alp_proc_null,
#       endif
#       if (ALP(ASAPI))
            &asapi_proc,        // 7, ASAPI
#       else
            &alp_proc_null,
#       endif
        &alp_ext_proc
    };

    ot_u8 alp_handle;

    // Always flush payload length of output before any data is written
    alp->outrec.plength = 0;

    /// <LI> IDs in range 0-to-ALP_FUNCTIONS are standardized, or psuedo-standardized </LI>
    /// <LI> IDs in range 128-to-(128+ALP_API) are mapped OTAPI functions </LI>
    /// <LI> IDs outside this range get pushed to ALP_EXT </LI>
    //alp_handle = alp->inq->getcursor[2];
    alp_handle = INREC(alp, ID);

    //ASAPI section
    if (alp_handle > 7) {
        alp_handle = 8;
    }
    alp_handle = (ot_u8)proc[alp_handle](alp, user_id);

    /// If the output bookmark is non-Null, there is output chunking.  Else,
    /// the output message is complete (ended)
    ///@todo Bookmarked has been refactored, and is not currently supported... sort it.
    //alp->OUTREC(_FLAGS)   &= ~ALP_FLAG_ME;
    //alp->OUTREC(_FLAGS)   |= (alp->BOOKMARK_OUT) ? ALP_FLAG_CF : ALP_FLAG_ME;
    alp->OUTREC(FLAGS)   |= ALP_FLAG_ME;   //Temporary fix

    // Return True (atomic) or False (non-atomic)
    return (ot_bool)alp_handle;
}
#endif

