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
  * @file       /OTlib/OTAPI_logger.c
  * @author     JP Norair
  * @version    V1.0
  * @date       1 May 2012
  * @brief      Default implementation of OTAPI logging functions
  * @ingroup    OTAPI
  * 
  * The default logger used NDEF as a wrapping protocol and MPIPE as the data
  * transfer medium (i.e. the bus).  You can develop your own method for logging
  * relatively easily by changing the LOG_METHOD attribute in OT_config.h and
  * implementing your own logger module with the same interface (from OTAPI.h).
  *
  ******************************************************************************
  */


#include "OT_config.h"

#if ((LOG_FEATURE(ANY) == ENABLED) && (LOG_METHOD == LOG_METHOD_DEFAULT))

#include "OT_types.h"
#include "OT_utils.h"
#include "OTAPI.h"
#include "alp.h"
#include "buffers.h"
#include "mpipe.h"



void otapi_log_header(ot_u8 id_subcode, ot_int payload_length) {
    ot_u8 header[]  = { 0xDD, 0x00, 0, 0x02, 0x04, 0 };
    header[2]       = (ot_u8)payload_length;
    header[5]       = id_subcode;

    q_empty(&dir_out);  // output buffer
    q_writestring(&dir_out, header, 6);
}



ot_bool sub_dirout_check(ot_int payload_length) {
///@todo Magic number 10 is the size of the logging overhead...
///      It equals 6 bytes NDEF plus 4 bytes MPipe.  It should be a constant.
    return (ot_bool)((dir_out.front+payload_length+10) < dir_out.back);
}



#ifndef EXTF_otapi_log
void otapi_log(ot_u8 subcode, ot_int length, ot_u8* data) {
    if (sub_dirout_check(length)) {
        otapi_log_header(subcode, length);
        q_writestring(&dir_out, data, length);
        mpipe_txndef(dir_out.front, False, MPIPE_Broadcast);
    }
}
#endif


#ifndef EXTF_otapi_log_msg
void otapi_log_msg(logmsg_type logcmd, ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data) {
    ot_int payload_length = label_len + 1 + data_len;
    
    if (sub_dirout_check(payload_length)) {
        otapi_log_header(logcmd, payload_length);
        q_writestring(&dir_out, label, label_len);
        q_writebyte(&dir_out, ' ');
        q_writestring(&dir_out, data, data_len);
        
        mpipe_txndef(dir_out.front, False, MPIPE_Broadcast);
    }
}
#endif


#ifndef EXTF_otapi_log_hexmsg
void otapi_log_hexmsg(ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data) {
    ot_int payload_length = label_len + 1 + (data_len<<1);

    if (sub_dirout_check(payload_length)) {
        otapi_log_header(/*2*/ 7, payload_length);
        q_writestring(&dir_out, label, label_len);
        q_writebyte(&dir_out, ' ');
        dir_out.length += otutils_bin2hex(data, dir_out.putcursor, data_len);
        
        mpipe_txndef(dir_out.front, False, MPIPE_Broadcast);
    }
}
#endif



#ifndef EXTF_otapi_log_direct
void otapi_log_direct(ot_u8* data) {
    mpipe_txndef(data, False, MPIPE_Broadcast);
}
#endif



#ifndef EXTF_otapi_log_code
void otapi_log_code(ot_int label_len, ot_u8* label, ot_u16 code) {
#   ifdef __LITTLE_ENDIAN__
        code = (code >> 8) | (code << 8);
#   endif

    otapi_log_msg(MSG_raw, label_len, 2, label, (ot_u8*)&code);
}
#endif



#endif

