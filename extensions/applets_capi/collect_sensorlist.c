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
  * @file       /otlibext/applets_capi/collect_sensorlist.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2012
  * @brief      Runs anycast query on devices that support sensors and collects 
  *             the list of supported sensors from these devices.
  *
  */

#include "OTAPI.h"



void applet_collect_sensorlist(m2session* session) {
/// The C-API for building commands can be bypassed in favor of directly
/// putting data to the queue.  That way is more efficient, but it also requires
/// you to know more about DASH7 that just what order the templates should be.

/// The query that we build will collect sensor configuration data back from
/// all devices that support the sensor protocol.  Much more interesting queries
/// are possible.

    { //open request for single hop anycast query
        routing_tmpl routing;
        routing.hop_code = 0;
        otapi_open_request(ADDR_anycast, &routing);
    }
    { //use a command template for collection of single file from single file search
        ot_u8 status;
        command_tmpl command;
        command.opcode      = (ot_u8)CMD_collect_file_on_file;
        command.type        = (ot_u8)CMDTYPE_na2p_request;
        command.extension   = (ot_u8)CMDEXT_none;
        otapi_put_command_tmpl(&status, &command);
    }
    { //write the dialog information (timeout, channels to use)
        ot_u8 status;
        dialog_tmpl dialog;
        dialog.channels = 0;    //use same channel as request for response
        dialog.timeout  = 128;  //128 tick response timeout (1 tick = 0,977 ms)
        otapi_put_dialog_tmpl(&status, &dialog);
    }
    { //write the query to search for the sensor protocol id
        query_tmpl query;
        ot_u8 status;
        ot_u8 protocol_id;
        protocol_id     = 0x02;                 // sensor protocol id = 0x02
        query.code      = M2QC_COR_SEARCH | 1;  // do a 100% length=1 correlation search
        query.mask      = NULL;                 // don't do any masking (no partial matching)
        query.length    = 1;                    // look for one byte (0x02)
        query.value     = &protocol_id;         // (query.value is a string)
        otapi_put_query_tmpl(&status, &query);
    }
    { //put in the information of the file to search (the protocol list)
        ot_u8 status;
        isfcomp_tmpl isfcomp;
        isfcomp.is_series   = False;
        isfcomp.isf_id      = ISF_ID(protocol_list);
        isfcomp.offset      = 0;
        otapi_put_isf_comp(&status, &isfcomp);
    }
    { //put in the information of the file to return (the sensor list)
        ot_u8 status;
        isfcall_tmpl isfcall;
        isfcall.is_series   = False;
        isfcall.isf_id      = ISF_ID(sensor_list);
        isfcall.max_return  = 32;
        isfcall.offset      = 0;
        otapi_put_isf_call(&status, &isfcall);
    }

    //Done building command, close the request and send the dialog
    otapi_close_request();
}
