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
  * @file       /hbuilder/protocols/ht_server.c
  * @author     JP Norair
  * @version    R100
  * @date       7 April 2013
  * @brief      HayTag server protocols
  *
  *
  ******************************************************************************
  */

#include "OTAPI.h"
#include "OT_platform.h"


/** ALP Processor Callback for Starting a Ping <BR>
  * ========================================================================<BR>
  * "ALP" is the NDEF-based set of low-level API protocols that OpenTag uses.
  * ALP messages can come-in over any communication method: wire, wireless, 
  * telepathy... anything that can transfer a packet payload.
  *
  * Some ALPs are standardized. Those get handled by OTlib automatically.  ALPs
  * that are not recognized are sent to this function to be handled.  In this
  * demo, we are using a very simple ALP, shown below:
  *
  * ALP Payload Length:         0
  * ALP Protocol ID:            255 (FF)
  * ALP Protocol Commands:      0-127 (00-7F) corresponding to channel to ping
  *
  * The "user_id" parameter corresponds to the Device ID that sent this ALP.
  * 
  * A quickstart guide to the ALP API is available on the Indigresso Wiki.
  * http://www.indigresso.com/wiki/doku.php?id=opentag:api:quickstart
  */ 

void otapi_alpext_proc(alp_tmpl* alp, id_tmpl* user_id) {
/// The function app_invoke() will cause the kernel to call ext_systask() as
/// soon as resources are available.

    // Start the task only if: Caller is ROOT, ALP Call is Protocol-255, Task is idle
    if (    auth_isroot(user_id)    \
        &&  (alp->inrec.id == 0xFF) \
        &&  (APP_TASK->event == 0)   )   {
        
        app_invoke(alp->inrec.cmd);     // Initialize Ping Task on supplied channel
        alp_load_retval(alp, 1);        // Write back 1 (success)
    }
}


/** DASHCom Client-Server protocol <BR>
  * ========================================================================<BR>
  * DASHCom is a REST-like client-server messaging protocol that implements a
  * simple, command-oriented interface between DASH7 devices.  DASHCom is 
  * intended especially for querying applications.
  *
  * Data is packed as binary, but it may contain fields represented as text.
  * 
  * The Communication model is Template-based.  The user must first open a 
  * session using a Control Template.  Afterwards, more templates can be 
  * provided.  A typical Session might be templated as: C M QF RF
  *
  * Response data sent from server-to-client will contain a File Resource 
  * Template corresponding to the data source, followed by a Data Template.
  * 
  *
  * C: Control Template
  * <LI> Format is C:o|c|x </LI>
  * <LI> 'o' is for open, used to begin a command session.  'c' is for close, 
  *      used to terminate the current session in progress.  'x' is for execute
  *      used to initialize the command communication to the server dataset. </LI>
  * 
  * <LI> Format is F:a|b|g|l|t:addr:file:offset:length </LI>
  * <LI> Argument a, b, g, l, or t is the second byte.  'a' is antecedent mode,
  *      that is, it copies the address from the last-entered File template
  *      and "addr" is therefore 0-bytes. 'b' is broadcast mode, with 0-byte 
  *      address.  'g' is global mode with 8-byte address.  'l' is local mode 
  *      with 2-byte address.  't' is text mode, which is RFU.  </LI>
  * <LI> The address is an integer, which is binary mode is a binary value or
  *      in text mode is a hexadecimal string.  The loopback address is 0. </LI>
  
  * @note The server dataset is typically a group of external DASH7 devices.
  * 
  *
  * D: Data Template
  * <LI> Format is D::length:data </LI>
  * <LI> flags are ignored, but the byte is present in communication </LI>
  * <LI> length is a 2-byte integer </LI>
  * <LI> data is a binary bytestream </LI>
  *
  * K: Comparison File Template
  * A "File" is a binary data resource available on DASH7 devices.  Files are
  * used as input and output, which in a query relates to comparison and return.
  * This Comparison File Template should only follow a Query Template (Q).
  * <LI> Format is K:a|*:file:offset:length </LI>
  * <LI> Flags 'a' is antecedent mode, in which the file resource shall link to
  *      last-entered comparison file, and the remaining fields are not used.
  *      Any flag value other than 'a' requires the remaining fields. </LI>
  * <LI> The file path is a concatenation of block integer and file integer. 
  *      Block is 1-byte (0-3) and File is 1-byte (0-255).  In text mode, it is
  *      represented as hexadecimal text.  </LI>
  * <LI> Offset is a 2-byte integer describing a byte-offset to the resource
  *      for read/write, starting at byte 0.  </LI>
  * <LI> Length is a 2-byte integer describing resource byte access.  Using 0
  *      for Length is equivalent to using 65536, which will guarantee that the
  *      resource is accessed from the offset to its end. </LI>
  *
  * 
  * M: Media Template
  * <LI> Format is M:d:[flag-specific data] </LI>
  * <LI> The only supported flag at the moment is 'd' for DASH7.  This is a 
  *      media specifier for DASH7 media access. </LI>
  * 
  * DASH7 Media Data is a 8-byte binary payload
  * <PRE>
  * ^ DATA FIELD          ^ BYTES ^ VALUE        ^
  * +---------------------+-------+--------------+
  * | advertising channel |   1   | channel code |
  * | session channel     |   1   | channel code |
  * | subnet              |   1   | subnet code  |
  * | subnet mask         |   1   | subnet mask  |
  * | advertising timeout |   2   | integer ms   |
  * | session timeout     |   2   | integer ms   |
  * | retrieval timeout   |   2   | integer ms   |
  * </PRE>
  * 
  * 
  * Q: Query Template
  * Querying is the primary activity of DASH7 communications.  A Query Template
  * expects to be followed by a File Resource template.
  * <LI> Format is Q:a|o:!=|==|<|<=|>|>=|nn|sx:token:mask
  * <LI> Flags 'a'|'o' mean "and"|"or".  If you are doing a single query, the
  *      value doesn't matter.  If you are doing *batch queries* using multiple
  *      Query Templates in a single command, you can AND and OR this Query 
  *      with the next one.  Batch querying is a way to improve the speed of
  *      data retrieval and reduce energy expense, but it is necessary only for
  *      complex querying. </LI>
  * <LI> Comparison flags are 2-bytes, != == < <= > >= nn sx, and mostly self-
  *      explanatory.  The token should match part of the resource being
  *      queried using the chosen comparison method.  "nn" means "Non-null" and
  *      it succeeds if the resource exists at all. "sx" is a compound flag.
  *      's' means string search and 'x' is replaced with a 1-byte integer 
  *      (0-255).  x is the number of bytes/characters that can be different
  *      in the search.  0 is perfect match, 1 is one difference, etc. </LI>
  * <LI> Token is a length-value binary byte-string.  Length is one-byte, so
  *      the token is maximally 255 bytes long. </LI>
  * <LI> Mask is a length-value binary byte-string, similar to Token.  The Mask
  *      is logically ANDed onto the resource and Token data before comparison.
  *      If Mask-length is 0, mask isn't used.  If Mask-length is shorter than
  *      Token-length, the extra portion of the Token is unaffected by the mask.
  *      </LI>
  *
  *
  * R: Return File Template
  * Codify some information to return via the command.  Return Template is not
  * mandatory.  If missing, the return will be acknowledgements only.  The 
  * template is identical to the Comparison File Template (K) other than the 
  * leading 'R'
  * <LI> Format is R:a|n:file:offset:length </LI>
  *
  *
  * S: Security Template
  * Used for data authentication and encryption, RFU
  * 
  */





void hbs_dashcom_proc(alp_tmpl* alp, id_tmpl* user_id) {
    
    /// Available commands: g (GET), p (PUT), a (POST), d (DELETE) 
    alp->inrec.cmd
    
    switch (q_readbyte(alp->inq)) {
        case 'C': 
        case 'D':
        case 'F':
        case 'M':
        case 'Q':
        case 'R':
        
        // Not supported
        default: 
    }
    
    /// Control Templates
    /// - get/put communication parameters
    /// - post query
    
    alp_load_retval(alp, 1)
}


void sub_proc_control() {
    
}

void sub_proc_data() {
    // unimplemented at present time
}


void sub_get_file(dc_filetmpl* attachpoint) {
    /// Only add resource if necessary, and breakdown input template
    if (*alp->inq->getcursor++ != 'a') {
        handle                      = (dc_filetmpl*)sub_add_template(sizeof(dc_filetmple));
        handle->next                = NULL;
        handle->last                = dashcom.filelist;
        dashcom.filelist->next      = handle;
        dashcom.filelist            = handle;
        dashcom.filelist->block     = *alp->inq->getcursor++;
        dashcom.filelist->id        = *alp->inq->getcursor++;
        dashcom.filelist->offset    = q_readshort(alp->inq);
        dashcom.filelist->length    = q_readshort(alp->inq);
    }
    
    /// Attach the file, either new one or the last one
    attachpoint = dashcom.filelist;
}

void sub_proc_retfile() {   sub_get_file(dashcom.retfile);      }
void sub_proc_compfile() {  sub_get_file(dashcom.query.file);   }




void sub_proc_media() {
    if (*alp->inq->getcursor++ != 'd') {
        //error must be standard DASH7
    }
    else {
        dashcom.advert.channel      = *alp->inq->getcursor++;
        dashcom.session.channel     = *alp->inq->getcursor++;
        dashcom.session.subnet      = *alp->inq->getcursor++;
        dashcom.session.subnetmask  = *alp->inq->getcursor++;
        dashcom.advert.duration     = q_readshort(alp->inq);
        dashcom.extra.csma_timeout  = q_readshort(alp->inq);
        dashcom.extra.comm_timeout  = q_readshort(alp->inq);
    }
}



void sub_proc_query() {
    /// Add a new query to the list
    query               = (dc_querytmpl*)sub_add_template(sizeof(dc_querytmpl));
    query->last         = dashcom.query;
    dashcom.query->next = query;
    dashcom.query       = query;
    
    /// If query is sequential and ANDing, update the last query window by 
    /// setting to 0 and moving old value to this value.
    query->window       = query->last->window;
    query->last->window = 0;

    /// If query is sequential and ORing, update the last query window by
    /// dividing by two, and set this window to the same amount
    query->last->window >>= 1;
    query->window       = query->last->window;
    
    /// Copy Comparison type
    ot_u8 comp1, comp0;
    comp1           = *alp->inq->getcursor++;
    comp0           = *alp->inq->getcursor++;
    comp0           = (comp0 == '=');
    query->length   = *alp->inq->getcursor++;
    ccode           = b00100000;
    
    if (comp1 == 'n') {
        ccode = 0;  
    }
    else if (comp1 == 's') {
        comp0 = query->length - comp0;    // comp0 reset to correlation tolerance
        ccode = b0100000 | (comp0 & b00011111);
    }
    else if (comp1 == '=')  ccode = b00100001;
    else if (comp1 == '<')  ccode = b00100010 + comp0;
    else if (comp1 == '>')  ccode = b00100100 + comp0;

    /// Load Token
    if (query->length != 0) {
        query->token = (ot_u8*)sub_add_template(query->length);
        q_readstring(alp->inq, query->token, query->length);
    }
    
    /// Load Mask
    mask_length     = *alp->inq->getcursor++;
    query->mask     = NULL;
    if (mask_length != 0) {
        query->mask = (ot_u8*)sub_add_template(query->length);
        q_readstring(alp->inq, query->mask, mask_length);
        while(mask_length < query_length) {
            query->mask[mask_length] = 0xff;
            mask_length++;
        }
        ccode |= b10000000;
    }
}







/** M2QP-UDP (Transport Layer) Callback for Printing a Pong <BR>
  * =======================================================================<BR>
  * This function is called when a UDP request or response is received.  It is
  * implemented to monitor port 255, which is used for PONGs in this demo.
  */
#ifdef EXTF_m2qp_sig_udp
ot_bool m2qp_sig_udp(ot_u8 srcport, ot_u8 dstport, id_tmpl* user_id) {
    static const char* label[]  = { "PongID: ", ", RSSI: ", ", Link: " };
    ot_u16  pongval;
    ot_u8   i;
    ot_u8   scratch;

    //1. Read the PONG VAL
    pongval = q_readshort(&rxq);

    // Request: Copy PING VAL to PONG
    if (dstport == 255) {
        q_writeshort(&txq, pongval);
        return True;
    }

#   if defined(BOARD_eZ430Chronos)
    // Chronos doesn't have a normal MPipe, so print-out responses on the LCD
    
#   elif (OT_FEATURE(MPIPE))
    // Response: Compare PING Val to PONG Val and write output to MPipe
    if (dstport == 254) {
        // Prepare logging header: UTF8 (text log) is subcode 1, dummy length is 0
        otapi_log_header(1, 0);
        
        // Print out the three parameters for PongLT, one at a time.
        // If you are new to OpenTag, this is a common example of a state-
        // based code structure JP likes to use.
        i = 0;
        while (1) {
            q_writestring(mpipe.alp.outq, (ot_u8*)label[i], 8);
            switch (i++) {
                case 0: scratch = otutils_bin2hex(  mpipe.alp.outq->putcursor, 
                                                    user_id->value,
                                                    user_id->length     );
                        break;
                
                case 1: scratch = otutils_int2dec(mpipe.alp.outq->putcursor, radio.last_rssi);
                        break;
                        
                case 2: scratch = otutils_int2dec(mpipe.alp.outq->putcursor, dll.last_nrssi);
                        break;
                        
                case 3: goto m2qp_sig_udp_PRINTDONE;
            }
            
            mpipe.alp.outq->putcursor  += scratch;
            mpipe.alp.outq->length     += scratch;
        }

        // Close the log file, send it out, return success
        m2qp_sig_udp_PRINTDONE:
        otapi_log_direct();
        return True;
    }
#   endif

    return False;
}
#endif






/** OTlib/OTAPI Callback Applets  <BR>
  * ========================================================================<BR>
  * This app uses some of the "std" applets from /otlibext/applets_std
  * The applets used are selected in extf_config.h
  */













/** PING Kernel Task <BR>
  * =======================================================================<BR>
  * This function will be activated by the kernel when the external task is
  * active and there are resources available to run the task.  This task will
  * just activate the DLL session (also a task) and then turn itself off. 
  * Then, the kernel will call the DLL session task as soon as necessary 
  * resources are available to run that task (by default DLL task is top 
  * priority, so it should get called right away).
  */

void ext_systask(ot_task task) {
    session_tmpl    s_tmpl;

    if (task->event == 1) {
        task->event = 0;
        
        // this is the same as the length of the response window,
        // which is set in applet_send_query()
        task->nextevent = 512;  
    
        // Generate a pseudo random 16 bit number to be used as a ping check value
        app.pingval = platform_prand_u16();
    
        // Log a message.  It is scheduled, and the RF task has higher priority,
        // so if you are sending a DASH7 dialog this log message will usually
        // come-out after the dialog finishes.
#       if (OT_FEATURE(MPIPE))
        otapi_log_msg(MSG_raw, 5, 2, (ot_u8*)"PING:", (ot_u8*)&app.pingval);
#       endif
    
        // Load the session template: Only used for communication tasks
        s_tmpl.channel      = task->cursor;
        s_tmpl.flagmask     = 0;
        s_tmpl.subnetmask   = 0;
        otapi_task_immediate(&s_tmpl, &applet_send_query);
    }
    
    // Turn off the task after 512 ticks (what is set above)
    // Note that this task will not be activated by the button press or ALP
    // when event != 0, because those routines have conditionals in them.
    //else {
    //    task->event = 0;
    //}
}






/** Communication Task Applets  <BR>
  * ========================================================================<BR>
  * Communication tasks in OpenTag are typically created by one of the OTAPI
  * Tasker functions in /otlib/OTAPI_tasker.h.  The task is created here, and
  * when it gets activated by the kernel, the applet that is attached to it
  * will run.  The main job of the applet is to load prepare the communication.
  * Usually, this means loading a request packet, but you could also make an
  * applet that does any manner of state-based communication routines.  The
  * Adaptive Search and CoAP demos are examples of sophisticated applets.
  */

void applet_send_query(m2session* session) {
/// The C-API for building commands can be bypassed in favor of directly
/// putting data to the queue.  That way is more efficient, but it also requires
/// you to know more about DASH7 than just what order the templates should be.
///
/// The query that we build will collect sensor configuration data back from
/// all devices that support the sensor protocol.  Much more interesting queries
/// are possible.
    ot_u8 status;

    { //open request for single hop anycast query
        routing_tmpl routing;
        routing.hop_code = 0;
        otapi_open_request(ADDR_anycast, &routing);
    }
    { //use a command template for collection of single file from single file search
        command_tmpl command;
        command.opcode      = (ot_u8)CMD_udp_on_file;
        command.type        = (ot_u8)CMDTYPE_na2p_request;
        command.extension   = (ot_u8)CMDEXT_none;
        otapi_put_command_tmpl(&status, &command);
    }
    { //write the dialog information (timeout, channels to use)
        dialog_tmpl dialog;
        dialog.channels = 0;    //use same channel as request for response
        dialog.timeout  = 0x41; //same as otutils_encode_timeout(512) -- 512 tick response slot
        otapi_put_dialog_tmpl(&status, &dialog);
    }
    { //write the query to search for the sensor protocol id
        static const ot_u8 query_str[10] = "APP=PongLT";
        query_tmpl query;
        query.code      = M2QC_COR_SEARCH + 10; // do a 100% length=10 correlation search
        query.mask      = NULL;                 // don't do any masking (no partial matching)
        query.length    = 10;                   // query_str is 10 bytes
        query.value     = (ot_u8*)query_str;
        otapi_put_query_tmpl(&status, &query);
    }
    { //put in the information of the file to search (the user id)
        isfcomp_tmpl isfcomp;
        isfcomp.is_series   = False;
        isfcomp.isf_id      = ISF_ID(user_id);
        isfcomp.offset      = 0;
        otapi_put_isf_comp(&status, &isfcomp);
    }
    { //put in UDP ports (from 254 to 255) and Ping ID
        q_writebyte(&txq, 254);
        q_writebyte(&txq, 255);
        q_writeshort(&txq, app.pingval);
    }

    //Done building command, close the request and send the dialog
    otapi_close_request();
}






void app_invoke(ot_u8 channel) {
/// The "External Task" is the place where the kernel runs the main user app.
/// Our app has 4 functions (call types).
/// <LI> The task event state is set to 1.  Event 0 is always Task-off, but
///      otherwise each task manages its own event numbers </LI>
/// <LI> We store the channel in the cursor, which is not being used otherwise.
///      The Task "cursor" can be used for additional state control, beyond the
///      event number. </LI>
/// <LI> We give it a runtime reservation of 1 tick (it runs pretty fast).
///      This is also short enough to pre-empt RX listening, but not RX data.
///      Try changing to a higher number, and observing how the kernel
///      manages this task. </LI>
/// <LI> We give it a latency of 255.  Latency is unimportant for run-once
///      tasks, so giving it the max latency will prevent it from blocking
///      any other tasks. </LI>
/// <LI> We tell it to start ASAP (preemption delay parameter = 0) </LI>
///
/// @note The latency parameter is mostly useful for protocol management,
/// for which you probably want to enforce a request-response turnaround time.
/// for processing and for iterative tasks it is not important: set to 255.
///
    sys_task_setevent(APP_TASK, 1);
    sys_task_setcursor(APP_TASK, channel);
    sys_task_setreserve(APP_TASK, 1);
    sys_task_setlatency(APP_TASK, 255);
    sys_preempt(APP_TASK, 0);
}


  