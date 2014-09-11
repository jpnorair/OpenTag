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
  * @file       /otlibext/applets_capi/beacon_discovery.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2012
  * @brief      Sends Beacon containing device discovery information
  *
  */

#include "OTAPI.h"


void applet_beacon_discovery(m2session* session) {
    { //open request for broadcast
        otapi_open_request(ADDR_broadcast, NULL);
    }
    { //use a command template for collection of single file from single file search
        ot_u8 status;
        command_tmpl command;
        command.opcode      = (ot_u8)CMD_announce_file;
        command.type        = (ot_u8)CMDTYPE_na2p_request;
        command.extension   = (ot_u8)CMDEXT_no_response;
        otapi_put_command_tmpl(&status, &command);
    }
    { //write the dialog information (timeout, channels to use)
        ot_u8 status;
        dialog_tmpl dialog;
        dialog.channels = 0;    //use same channel as request for response
        dialog.timeout  = 0;    //0 tick response timeout (1 tick = 0,977 ms)
        otapi_put_dialog_tmpl(&status, &dialog);
    }
    { //write the ISF return data (note: use isfcall template)
        ot_u8 status;
        isfcall_tmpl isfcall;
        isfcall.is_series   = False;
        isfcall.isf_id      = ISF_ID(network_settings);
        isfcall.max_return  = ISF_LEN(network_settings);
        isfcall.offset      = 0;
        otapi_put_isf_return(&status, &isfcall);
    }

    //Done building command, close the request and send the dialog
    otapi_close_request();
}


