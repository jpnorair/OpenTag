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
  * @file       /OTlib/alp.h
  * @author     JP Norair
  * @version    V1.0
  * @date       08 May 2011
  * @brief      Application Layer Subprotocol header
  * @defgroup   ALP
  * @ingroup    ALP
  *
  * Application Layer Subprotocols (ALP's) are directive-based protocols that
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

#ifndef __ALP_H_
#define __ALP_H_

#include "OT_types.h"
#include "OT_config.h"

#include "OTAPI_tmpl.h"
#include "queue.h"


/** ALP Record Header
  * ALP Records can be used for NDEF and LLDP.  LLDP is basically just a reduced
  * version of NDEF that satisfies the minimum needs of ALP.  For NDEF, certain
  * data is assumed and/or thrown away.
  */
  
#define ALP_FLAG_MB     0x80    // Message Start bit
#define ALP_FLAG_ME     0x40    // Message End bit
#define ALP_FLAG_CF     0x20    // Chunk Flag
  
typedef struct {
    ot_u8       flags;              // just message end and chunk flag flags
    ot_u8       payload_length;
    ot_u8       dir_id;
    ot_u8       dir_cmd;
    void*       bookmark;           // Internal use only (private)
} alp_record;





#if ((OT_FEATURE(SERVER) == ENABLED) && (OT_FEATURE(ALP) == ENABLED))


/** @brief  Common function for responding to ALP with a simple 16-bit return
  * @param  respond     (ot_bool) True/False to respond or not
  * @param  out_dir_cmd (ot_u8) output dir cmd
  * @param  retval      (ot_u16) 16 bit return value
  * @param  out_rec     (alp_record*) Header of output ALP record, due to processing
  * @param  out_q       (Queue*) output queue for [optional] record response
  * @retval None
  * @ingroup ALP
  */
void alp_load_retval(ot_bool respond, ot_u8 out_dir_cmd, ot_u16 retval, 
                     alp_record* out_rec, Queue* out_q);



/** @note User IDs for ALP's
  * Most ALP commands require some form of user authentication to run.  This is
  * analagous for a user having to log-in to a POSIX shell in order to run the
  * applications on the system.  The User ID needs to be authenticated to a 
  * device ID via the auth system if the ALP's are being transmitted over the 
  * air (via DASH7).
  */


/** @brief  Process a received ALP record (vectors to all supported ALP's)
  * @param  in_rec      (alp_record*) Header of input ALP record, to be processed
  * @param  out_rec     (alp_record*) Header of output ALP record, due to processing
  * @param  in_q        (Queue*) input queue containing record
  * @param  out_q       (Queue*) output queue for [optional] record response
  * @param  user_id     (id_tmpl*) user id for performing the record 
  * @retval None
  * @ingroup ALP
  *
  * ID values (from spec)
  * 0x00:       Null subprotocol
  * 0x01:       Filesystem subprotocol
  * 0x02:       Sensor Configuration subprotocol (pending)
  * 0x11-14:    Security subprotocols (pending)
  */
void alp_proc(alp_record* in_rec, alp_record* out_rec, Queue* in_q, Queue* out_q, id_tmpl* user_id);





/** @note Subprotocol processing functions
  * The functions below are exposed, but they never should be called unless you
  * are writing some sort of hack or optimized version.  Instead, call the
  * alp_proc() function with the ID parameter set appropriately.
  */

/** @brief  Process a received filesystem ALP record
  * @param  in_rec      (alp_record*) Header of input ALP record, to be processed
  * @param  out_rec     (alp_record*) Header of output ALP record, due to processing
  * @param  in_q        (Queue*) input queue containing record
  * @param  out_q       (Queue*) output queue for [optional] record response
  * @param  user_id     (id_tmpl*) user id for performing the record 
  * @retval None
  * @ingroup ALP
  */
void alp_proc_filedata(alp_record* in_rec, alp_record* out_rec, Queue* in_q, Queue* out_q, id_tmpl* user_id);




#if (OT_FEATURE(SENSORS) == ENABLED)
/* @brief  Process a received sensor configurator ALP record (not implemented)
  * @param  in_rec      (alp_record*) Header of input ALP record, to be processed
  * @param  out_rec     (alp_record*) Header of output ALP record, due to processing
  * @param  in_q        (Queue*) input queue containing record
  * @param  out_q       (Queue*) output queue for [optional] record response
  * @param  user_id     (id_tmpl*) user id for performing the record 
  * @retval None
  * @ingroup ALP
  */
void alp_proc_sensor(alp_record* in_rec, alp_record* out_rec, Queue* in_q, Queue* out_q, id_tmpl* user_id);
#endif





#if (OT_FEATURE(DASHFORTH) == ENABLED)
#   if (OT_FEATURE(CAPI) != ENABLED)
#       error For DASHForth to work, C-API must be ENABLED (it is not: check OT_config.h).
#   endif
/** @brief  Process a received DASHFORTH ALP record
  * @param  in_rec      (alp_record*) Header of input ALP record, to be processed
  * @param  out_rec     (alp_record*) Header of output ALP record, due to processing
  * @param  in_q        (Queue*) input queue containing record
  * @param  out_q       (Queue*) output queue for [optional] record response
  * @param  user_id     (id_tmpl*) user id for performing the record 
  * @retval None
  * @ingroup ALP
  */
void alp_proc_dashforth(alp_record* in_rec, alp_record* out_rec, Queue* in_q, Queue* out_q, id_tmpl* user_id);
#endif






#if (LOG_FEATURE(ANY) == ENABLED)
/** @brief  Process a received logger ALP record (typically client only)
  * @param  in_rec      (alp_record*) Header of input ALP record, to be processed
  * @param  out_rec     (alp_record*) Header of output ALP record, due to processing
  * @param  in_q        (Queue*) input queue containing record
  * @param  out_q       (Queue*) output queue for [optional] record response
  * @param  user_id     (id_tmpl*) user id for performing the record 
  * @retval None
  * @ingroup ALP
  */
void alp_proc_logger(alp_record* in_rec, alp_record* out_rec, Queue* in_q, Queue* out_q, id_tmpl* user_id);
#endif







#if (OT_FEATURE(ALPAPI) == ENABLED)
#   if (OT_FEATURE(CAPI) != ENABLED)
#       error For ALP-API to work, C-API must be ENABLED (it is not: check OT_config.h).
#   endif
/** @brief  Process a received Session API record
  * @param  in_rec      (alp_record*) Header of input ALP record, to be processed
  * @param  out_rec     (alp_record*) Header of output ALP record, due to processing
  * @param  in_q        (Queue*) input queue containing record
  * @param  out_q       (Queue*) output queue for [optional] record response
  * @param  user_id     (id_tmpl*) user id for performing the record 
  * @retval None
  * @ingroup ALP
  */
void alp_proc_api_session(alp_record* in_rec, alp_record* out_rec, Queue* in_q, Queue* out_q, id_tmpl* user_id);

/** @brief  Process a received System API record
  * @param  in_rec      (alp_record*) Header of input ALP record, to be processed
  * @param  out_rec     (alp_record*) Header of output ALP record, due to processing
  * @param  in_q        (Queue*) input queue containing record
  * @param  out_q       (Queue*) output queue for [optional] record response
  * @param  user_id     (id_tmpl*) user id for performing the record 
  * @retval None
  * @ingroup ALP
  */
void alp_proc_api_system(alp_record* in_rec, alp_record* out_rec, Queue* in_q, Queue* out_q, id_tmpl* user_id);

/** @brief  Process a received Query API record
  * @param  in_rec      (alp_record*) Header of input ALP record, to be processed
  * @param  out_rec     (alp_record*) Header of output ALP record, due to processing
  * @param  in_q        (Queue*) input queue containing record
  * @param  out_q       (Queue*) output queue for [optional] record response
  * @param  user_id     (id_tmpl*) user id for performing the record 
  * @retval None
  * @ingroup ALP
  */
void alp_proc_api_query(alp_record* in_rec, alp_record* out_rec, Queue* in_q, Queue* out_q, id_tmpl* user_id);
#endif








#if (OT_FEATURE(SECURITY) == ENABLED)
/** @brief  Process a received Security/Auth ALP record (not implemented)
  * @param  in_rec      (alp_record*) Header of input ALP record, to be processed
  * @param  out_rec     (alp_record*) Header of output ALP record, due to processing
  * @param  in_q        (Queue*) input queue containing record
  * @param  out_q       (Queue*) output queue for [optional] record response
  * @param  user_id     (id_tmpl*) user id for performing the record 
  * @retval None
  * @ingroup ALP
  */
void alp_proc_sec_example(alp_record* in_rec, alp_record* out_rec, Queue* in_q, Queue* out_q, id_tmpl* user_id);
#endif





#endif  // if (OT_FEATURE(SERVER) == ENABLED) && (OT_FEATURE(ALP) == ENABLED)
#endif  // ifdef __ALP_H

