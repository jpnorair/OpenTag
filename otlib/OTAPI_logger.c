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
  * The default logger uses ALP as a wrapping framework.  MPipe is expected to
  * know how to deal with ALP.  The default MPipe implementation converts ALP
  * into NDEF (this is a transparent conversion).
  *
  * @note This implementation is not entirely to my liking, but right now it's
  * not the most important thing in the project to have a maximally elegant
  * logger and MPipe.  So, it may change, along with MPipe (slightly).  In
  * particular, these functions shouldn't be directly manipulating ALP/NDEF
  * data.
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





ot_bool otapi_log_header(ot_u8 id_subcode, ot_int payload_length) {
	if (mpipe_status() == MPIPE_Idle) {
        mpipe_alp.outrec.id     = 4;                //Logger ID
        mpipe_alp.outrec.cmd    = id_subcode;       //Format Type
	
	    if ((mpipe_alp.outq->putcursor+payload_length) < mpipe_alp.outq->back) {
		    alp_new_record(&mpipe_alp, (ALP_FLAG_MB+5), 255, payload_length);
		    return True;
	    }
	    mpipe_kill();
	}
	return False;
}



#ifndef EXTF_otapi_log
void otapi_log(ot_u8 subcode, ot_int length, ot_u8* data) {
    if (otapi_log_header(subcode, length)) {
        q_writestring(mpipe_alp.outq, data, length);
        mpipe_txndef(mpipe_alp.outq->getcursor, False, MPIPE_Broadcast);
    }
}
#endif




#ifndef EXTF_otapi_log_msg
void otapi_log_msg(logmsg_type logcmd, ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data) {
    ot_int payload_length = label_len + 1 + data_len;
    //q_empty(mpipe_alp.outq);
    
    if (otapi_log_header(logcmd, payload_length)) {
    	q_writestring(mpipe_alp.outq, label, label_len);
    	q_writebyte(mpipe_alp.outq, ' ');
    	q_writestring(mpipe_alp.outq, data, data_len);
    	mpipe_txndef(mpipe_alp.outq->getcursor, False, MPIPE_Broadcast);
    }
}
#endif


#ifndef EXTF_otapi_log_hexmsg
void otapi_log_hexmsg(ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data) {
/// Rarely used, but sometimes helpful if you aren't using OTcom or another
/// client that knows how to process binary as hex.

    ot_int payload_length = label_len + 1 + (data_len<<1);

    if (otapi_log_header(7, payload_length)) {
        q_writestring(mpipe_alp.outq, label, label_len);
        q_writebyte(mpipe_alp.outq, ' ');
        
        payload_length              = otutils_bin2hex(data, mpipe_alp.outq->putcursor, data_len);
        mpipe_alp.outq->putcursor  += payload_length;
        mpipe_alp.outq->length     += payload_length;

        mpipe_txndef(mpipe_alp.outq->getcursor, False, MPIPE_Broadcast);
    }
}
#endif



#ifndef EXTF_otapi_log_direct
void otapi_log_direct() {
    mpipe_txndef(mpipe_alp.outq->getcursor, False, MPIPE_Broadcast);
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

