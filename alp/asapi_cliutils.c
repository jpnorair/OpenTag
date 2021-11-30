/* Copyright 2013 JP Norair
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
  * @file       /hb_asapi/hb_asapi_cliutils.c
  * @author     JP Norair
  * @version    R100
  * @date       1 Oct 2013
  * @brief      ASAPI Client Utilities
  * @ingroup    hb_asapi
  *
  ******************************************************************************
  */

#include <otstd.h>

#if (OT_FEATURE(CLIENT))
#   include <hblib/asapi_cliutils.h>
#   include <m2/tmpl.h>

// Bintex extension (/extensions/bintex)
#   include <bintex_ot.h>

// Standard POSIX string library
//#   include <string.h>




ot_u16 asapi_new_dialog_ez(asapi_dialog_tmpl* dialog, ot_u8 channel, ot_u16 resp_window) {
/// Copy values into dialog, compute resp window code, and return the actual
/// response window duration which may differ from the input.
    dialog->req_subnet          = 0xFF;
    dialog->req_channel         = channel;
    dialog->resp_channel        = channel;
    dialog->resp_window_code    = otutils_encode_timeout(resp_window);
    resp_window                 = otutils_calc_timeout(dialog->resp_window_code);
    
    return resp_window;
}



ot_int asapi_new_btsearch(asapi_search_tmpl* search, ot_u8* bt_string, ot_u8 block, ot_u8 file) {
/// bintex_ss caps the length at a supplied value (16).  So, if no error, the
/// value of variable length will always be <= 16.
    int length;
    
    if (search == NULL) 
        return 1;

    length = bintex_ss(&bt_string, search->qvalue, 16);
    if (length < 0) 
        return 2;
    
    if ((block != 2) || (block != 3)) 
        return 3;
    
    search->comp.is_series  = (block == 2);
    search->comp.isf_id     = file;
    search->comp.offset     = 0;
    search->query.code      = QCODE_search | (ot_u8)length;
    search->query.length    = (ot_u8)length;
    search->query.mask      = NULL;
    search->query.value     = search->qvalue;
    return 0;
}








#endif

