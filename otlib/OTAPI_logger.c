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
  * @file       /otlib/OTAPI_logger.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 Jul 2012
  * @brief      Default implementation of OTAPI logging functions
  * @ingroup    OTAPI
  * 
  * The default logger uses ALP/NDEF as a wrapping framework and MPipe as the
  * medium.  MPipe uses NDEF headers internally, so when a queue full of NDEF
  * frames is sent to MPipe, the MPipe control task (/otlib/mpipe.c) and the 
  * MPipe driver (/otplatform/xxx/mipe_xxx~.c) have all the information they 
  * need to do the logging.
  *
  * Considering the above, the function mpipe_send() is used to trigger the 
  * MPipe control task, and it does not need any arguments because all of the
  * necessary information is written onto the NDEF header in the mpipe.alp.out
  * queue.
  *
  * Check the wiki for high-level (and low level) description of the logger.
  * http://www.indigresso.com/wiki/doku.php?id=opentag:otlib:logger
  *
  * @note Ideally, these functions shouldn't be directly manipulating ALP/NDEF
  * data, but for the moment they might do so.
  *
  ******************************************************************************
  */


#include "OT_config.h"

#if ((LOG_FEATURE(ANY) == ENABLED) && (LOG_METHOD == LOG_METHOD_DEFAULT))

//#include "OT_types.h"
//#include "OT_utils.h"
#include "OTAPI.h"
//#include "alp.h"
//#include "buffers.h"
#include "OT_platform.h"


ot_bool otapi_log_header(ot_u8 id_subcode, ot_int payload_length) {
/// otapi_log_header creates the ALP/NDEF header needed by MPipe.  It is used
/// by all logging functions.  If there is not enough space in the MPipe queue
/// to hold the log data, it will return False, and nothing will be added to 
/// the MPipe queue.

    mpipe.alp.outrec.id     = 4;                //Logger ID
    mpipe.alp.outrec.cmd    = id_subcode;       //Format Type
    
    if ((mpipe.alp.outq->putcursor+payload_length) < mpipe.alp.outq->back) {
        alp_new_record(&mpipe.alp, (ALP_FLAG_MB+5), 255, payload_length);
        return True;
    }
    //mpipe_kill();
    return False;
}



#ifndef EXTF_otapi_log
void otapi_log(ot_u8 subcode, ot_int length, ot_u8* data) {
/// Log raw data that is able to fit in the MPipe output queue.

    if (otapi_log_header(subcode, length)) {
        q_writestring(mpipe.alp.outq, data, length);
        mpipe_send();
    }
}
#endif



void sub_logmsg(ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data) {
	q_writestring(mpipe.alp.outq, label, label_len);
    q_writebyte(mpipe.alp.outq, ' ');
    q_writestring(mpipe.alp.outq, data, data_len);
}



#ifndef EXTF_otapi_log_msg
void otapi_log_msg(logmsg_type logcmd, ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data) {
/// Log a "Message" if it fits in the MPipe output queue.  A "Message" is a 
/// text label, plus a space, plus a data payload.  The client should know how
/// to read-back Message data.  Check here for more information about messages:
/// http://www.indigresso.com/wiki/doku.php?id=opentag:otlib:logger

    ot_int payload_length = label_len + 1 + data_len;
    //q_empty(mpipe.alp.outq);
    
    if (otapi_log_header(logcmd, payload_length)) {
    	sub_logmsg(label_len, data_len, label, data);
        mpipe_send();
    }
}
#endif


#ifndef EXTF_otapi_log_hexmsg
void otapi_log_hexmsg(ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data) {
/// This creates a "Message" (see otapi_log_msg()) in utf-8 text that includes
/// binary data that has been converted to hex on the server side.  It is more
/// efficient to use otapi_log_msg() with a client that understands Message
/// formatting, but if you don't have one of those, this function works.

    ot_int payload_length = label_len + 1 + (data_len<<1);

    if (otapi_log_header(7, payload_length)) {
        q_writestring(mpipe.alp.outq, label, label_len);
        q_writebyte(mpipe.alp.outq, ' ');
        
        payload_length              = otutils_bin2hex(data, mpipe.alp.outq->putcursor, data_len);
        mpipe.alp.outq->putcursor  += payload_length;
        mpipe.alp.outq->length     += payload_length;
        mpipe_send();
    }
}
#endif



#ifndef EXTF_otapi_log_direct
void otapi_log_direct() {
/// Use this if you have already created a valid MPipe ALP/NDEF frame in the
/// MPipe output queue and all you want to do is log it.
    mpipe_send();
}
#endif



#ifndef EXTF_otapi_log_code
void otapi_log_code(ot_int label_len, ot_u8* label, ot_u16 code) {
/// Emergency logging, with direct access to MPipe.
/// Used for kernel panics or other such things.

    code = PLATFORM_ENDIAN16(code);

    mpipe_kill();
    q_empty(mpipe.alp.outq);
    otapi_log_header(MSG_raw, label_len+3);
    sub_logmsg(label_len, 2, label, (ot_u8*)&code);

    mpipe_txndef(False, MPIPE_High);
}
#endif



#endif

