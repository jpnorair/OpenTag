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
  * @file       /otlib/logger.c
  * @author     JP Norair
  * @version    R100
  * @date       31 Jul 2012
  * @brief      Default implementation of OpenTag logging functions
  * @ingroup    Logger
  *
  * The default logger uses ALP/NDEF as a wrapping framework and MPipe as the
  * medium.  MPipe uses NDEF headers internally, so when a queue full of NDEF
  * frames is sent to MPipe, the MPipe control task (/otlib/mpipe.c) and the
  * MPipe driver (/otplatform/xxx/mipe_xxx~.c) have all the information they
  * need to do the logging.
  *
  * Considering the above, the function mpipe_txschedule(0) is used to trigger 
  * the MPipe control task.  It does not need any extensive arguments because 
  * all of the necessary information is written onto the NDEF header in the
  * mpipe.alp.out queue.
  *
  * Check the wiki for high-level (and low level) description of the logger.
  * http://www.indigresso.com/wiki/doku.php?id=opentag:otlib:logger
  *
  * @note Ideally, these functions shouldn't be directly manipulating ALP/NDEF
  * data, but for the moment they might do so.
  *
  ******************************************************************************
  */


#include <otstd.h>

#if (OT_FEATURE(MPIPE))
#include <otlib/logger.h>
#include <otlib/alp.h>
#include <otlib/queue.h>
#include <otlib/utils.h>
#include <otsys/mpipe.h>

#include <string.h>


ot_bool logger_header(ot_u8 id_subcode, ot_int payload_length) {
/// logger_header creates the ALP/NDEF header needed by MPipe.  It is used
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



#ifndef EXTF_logger
void logger(ot_u8 subcode, ot_int length, const ot_u8* data) {
/// Log raw data that is able to fit in the MPipe output queue.

    if (logger_header(subcode, length)) {
        q_writestring(mpipe.alp.outq, data, length);
        mpipe_txschedule(0); //mpipe_send();
    }
}
#endif



void sub_logmsg(ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data) {
	q_writestring(mpipe.alp.outq, label, label_len);
    q_writebyte(mpipe.alp.outq, 0);
    q_writestring(mpipe.alp.outq, data, data_len);
}



#ifndef EXTF_logger_str
void logger_str(const char* data) {
    if (data != NULL) {
        logger(DATA_utf8, strlen(data), data);
    }
}
#endif



#ifndef EXTF_logger_msg
void logger_msg(logmsg_type logcmd, ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data) {
/// Log a "Message" if it fits in the MPipe output queue.  A "Message" is a
/// text label, plus a space, plus a data payload.  The client should know how
/// to read-back Message data.  Check here for more information about messages:
/// http://www.indigresso.com/wiki/doku.php?id=opentag:otlib:logger

    ot_int payload_length = label_len + 1 + data_len;
    //q_empty(mpipe.alp.outq);

    if (logger_header(logcmd, payload_length)) {
    	sub_logmsg(label_len, data_len, label, data);
    	mpipe_txschedule(0); //mpipe_send();
    }
}
#endif


#ifndef EXTF_logger_hexmsg
void logger_hexmsg(ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data) {
/// This creates a "Message" (see logger_msg()) in utf-8 text that includes
/// binary data that has been converted to hex on the server side.  It is more
/// efficient to use logger_msg() with a client that understands Message
/// formatting, but if you don't have one of those, this function works.

    ot_int payload_length = label_len + 1 + (data_len<<1);

    if (logger_header(7, payload_length)) {
        q_writestring(mpipe.alp.outq, label, label_len);
        q_writebyte(mpipe.alp.outq, ' ');

        payload_length              = otutils_bin2hex(data, mpipe.alp.outq->putcursor, data_len);
        mpipe.alp.outq->putcursor  += payload_length;
        mpipe_txschedule(0); //mpipe_send();
    }
}
#endif



#ifndef EXTF_logger_direct
void logger_direct() {
/// Use this if you have already created a valid MPipe ALP/NDEF frame in the
/// MPipe output queue and all you want to do is log it.
    mpipe.alp.outq->getcursor[1] = q_span(mpipe.alp.outq) - 4;
    mpipe_txschedule(0); //mpipe_send();
}
#endif



#ifndef EXTF_logger_code
void logger_code(ot_int label_len, ot_u8* label, ot_u16 code) {
/// Emergency logging: Used for kernel panics or other such things.
    mpipedrv_kill();

    //code = PLATFORM_ENDIAN16(code);
    logger_msg(MSG_raw, label_len, 2, label, (ot_u8*)&code);
}
#endif



#endif

