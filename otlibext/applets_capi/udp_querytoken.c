/*  Copyright 2010-2012, JP Norair
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
  */
/**
  * @file       /otlibext/applets_capi/udp_querytoken.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2012
  * @brief      Runs anycast query on supplied file, checking for a token match.
  *             Also includes UDP application payload.
  *
  */

#include "OTAPI.h"




void applet_udp_querytoken( session_tmpl* session,
                                udp_tmpl* udp,
                                ot_u8 file, ot_u8 toklength, ot_u8* token   ) {
/// The C-API for building commands can be bypassed in favor of directly
/// putting data to the queue.  That way is more efficient, but it also requires
/// you to know more about DASH7 that just what order the templates should be.

/// The query that we build will collect sensor configuration data back from
/// all devices that support the sensor protocol.  Much more interesting queries
/// are possible.
    ot_u8 status;

    { //open request for single hop anycast query
        routing_tmpl routing;
        routing.hop_code = 0;
        otapi_open_request(ADDR_anycast, &routing);
    }
    
    { //use a command template for UDP INVENTORY of single file from single file search
        command_tmpl command;
        command.opcode      = (ot_u8)CMD_udp_on_file;
        command.type        = (ot_u8)CMDTYPE_na2p_request;
        command.extension   = (ot_u8)CMDEXT_none;
        otapi_put_command_tmpl(&status, &command);
    }   //if (status) return False;
    
    { //write the dialog information (timeout, channels to use)
        dialog_tmpl dialog;
        dialog.channels = 0;    //use same channel as request for response
        dialog.timeout  = 128;  //128 tick response timeout (1 tick = 0,977 ms)
        otapi_put_dialog_tmpl(&status, &dialog);
    }   //if (status) return False;
    
    { //write the query to search for the sensor protocol id
        query_tmpl query;
        query.code      = M2QC_COR_SEARCH | toklength;  // do a 100% correlation search
        query.mask      = NULL;                         // don't do any masking (no partial matching)
        query.length    = toklength;                    // look for one byte (0x02)
        query.value     = token;                        // (query.value is a string)
        otapi_put_query_tmpl(&status, &query);
    }   if (status) return;
    
    { //put in the information of the file to search (the protocol list)
        isfcomp_tmpl isfcomp;
        isfcomp.is_series   = False;
        isfcomp.isf_id      = file;
        isfcomp.offset      = 0;
        otapi_put_isf_comp(&status, &isfcomp);
    }   //if (status) return False;
    
    { //put in the information of the file to search (the protocol list)
        otapi_put_udp_tmpl(&status, udp);
    }   if (status) return;

    //Done building command, close the request and send the dialog
    otapi_close_request();
}
