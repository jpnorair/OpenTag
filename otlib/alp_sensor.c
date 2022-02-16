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
  * @file       /otlib/alp_sensor.c
  * @author     JP Norair
  * @version    V1.0
  * @date       20 July 2011
  * @brief      ALP to Sensor protocol processor
  * @ingroup    ALP
  *
  * Placeholder for future sensor protocol ALP wrapper
  * 
  ******************************************************************************
  */


#include <otlib/alp.h>

#if (   (OT_FEATURE(SERVER) == ENABLED) \
     && (OT_FEATURE(ALP) == ENABLED) \
     && (OT_FEATURE(SENSORS) == ENABLED) )

#include <otlib/auth.h>
#include <otsys/sysext.h>
#include <otlib/queue.h>

#include <otsys/veelite.h>


typedef ot_int (*sub_sensor)(alp_tmpl*, const id_tmpl*, ot_u8, ot_u8, ot_int);



//static ot_bool sub_qnotfull(ot_u8 write, ot_u8 write_size, ot_queue* q) {
//    return (ot_bool)((write_size <= q_writespace(q)) || (write == 0));
//}

// Return functions are not handled by the server (ignore)
static ot_int sub_return(alp_tmpl* alp, const id_tmpl* user_id, ot_u8 respond, ot_u8 cmd_in, ot_int data_in) {
    return 0;
}


static ot_int sub_actuator( alp_tmpl* alp, const id_tmpl* user_id, ot_u8 respond, ot_u8 cmd_in, ot_int data_in ) {
    ot_int  data_out = 0;
    ot_int  i;
    ot_int  listsz;
    
    ///@todo have list of actuators (taken from sensors list)
    /// currently hard coded to just have actuator 0 --> file 24
    ot_u8   actuator_list[2] = {0, 24};
    listsz = 2;

    /// Loop through the list actuator IDs and turn them on/off
    while (data_in > 0) {
        ot_u8 actuator_id   = q_readbyte(alp->inq);
        ot_u8 actuator_val  = q_readbyte(alp->inq);
        ot_u8 actuator_file = 0;
        ot_u8 error_code = 255;
        vlFILE* fp;

        data_in -= 2;

        /// Check to make sure input ID is in the list of available IDs
        for (i=0; i<listsz; i++) {
            if (actuator_list[0] == actuator_id) {
                actuator_file = actuator_list[1];
            }
        }
        
        /// If input id is matched to a file (0 is a reserved file),
        /// then attempt to actuate this file callback.
        if (actuator_file != 0) {
            fp = vl_open(VL_ISF_BLOCKID, actuator_file, VL_ACCESS_X, NULL);
            if (fp != NULL) {
                error_code = vl_execute(fp, 1, &actuator_val);
                vl_close(fp);
            }
        }

        /// provide response, if requested
        if (respond && (2 <= q_writespace(alp->outq))) {
            q_writebyte(alp->outq, actuator_id);
            q_writebyte(alp->outq, error_code);
            data_out += 2;
        }
    }

    /// return number of bytes put onto the output (always x2)
    return data_out;
}



OT_WEAK ot_bool alp_proc_sensor(alp_tmpl* alp, const id_tmpl* user_id) {
    //sub_file cmd;
    static const sub_sensor cmd_fn[] = {
          &sub_actuator,
          &sub_return,
    };

    ot_int  data_in     = INREC(alp, PLEN);     //alp->INREC(PLEN);
    ot_u8   cmd_in      = INREC(alp, CMD);      //alp->INREC(CMD);

    // Return value is the number of bytes of output the command has produced
    alp->OUTREC(PLEN) = cmd_fn[cmd_in & 0x01](alp, user_id, (cmd_in & 0x80), cmd_in, data_in);

    if (cmd_in & 0x80) {
        //Transform input cmd to error or data return variant for response
        alp->OUTREC(CMD) ^= 0x80;
    }
    else {
        ///@todo find if this is even necessary.  I don't think it is.  It is
        /// here now for safety purposes.
        alp->outq->putcursor -= alp->OUTREC(PLEN);
    }

    return True;
}


#endif


