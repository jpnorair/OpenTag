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
  * @file       /hb_asapi/hb_asapi.c
  * @author     JP Norair
  * @version    R100
  * @date       12 Sept 2013
  * @brief      Abridged Session API for H-Builder
  * @ingroup    hb_asapi
  *
  * ASAPI is a simplified framework for creating and managing DASH7 sessions
  * on an OpenTag server, via a client.  It is geared towards query operations
  * via M2QP, including UDP support.
  *
  *
  *
  *
  ******************************************************************************
  */



#include <alp/asapi.h>

#if (ALP(ASAPI) != ENABLED)
ot_bool asapi_proc(alp_tmpl* alp, const id_tmpl* user_id) {
    return True;
}

#else



/** ASAPI subroutines  <BR>
  * ========================================================================<BR>
  */
static const ot_u8 base_std[32] = {
    /* 00000 */ 2 + ASAPI_UNICAST_SIZE,
    /* 00001 */ 2 + ASAPI_BROADCAST_SIZE,
    /* 00010 */ 2 + ASAPI_ANYCAST_SIZE,
    /* 00011 */ 2 + ASAPI_MULTICAST_SIZE,
    /* 00100 */ 2 + ASAPI_DLLS_SIZE + ASAPI_UNICAST_SIZE,
    /* 00101 */ 2 + ASAPI_DLLS_SIZE + ASAPI_BROADCAST_SIZE,
    /* 00110 */ 2 + ASAPI_DLLS_SIZE + ASAPI_ANYCAST_SIZE,
    /* 00111 */ 2 + ASAPI_DLLS_SIZE + ASAPI_MULTICAST_SIZE,
    /* 01000 */ 2 + ASAPI_INIT_SIZE,
    /* 01001 */ 2 + ASAPI_INIT_SIZE,
    /* 01010 */ 2 + ASAPI_INIT_SIZE,
    /* 01011 */ 2 + ASAPI_INIT_SIZE,
    /* 01100 */ 2 + ASAPI_INIT_SIZE,
    /* 01101 */ 2 + ASAPI_INIT_SIZE,
    /* 01110 */ 2 + ASAPI_INIT_SIZE,
    /* 01111 */ 2 + ASAPI_INIT_SIZE,
    /* 10000 */ 2 + ASAPI_STATUS_SIZE,
    /* 10001 */ 2 + ASAPI_STATUS_SIZE,
    /* 10010 */ 2 + ASAPI_STATUS_SIZE,
    /* 10011 */ 2 + ASAPI_STATUS_SIZE,
    /* 10100 */ 2 + ASAPI_STATUS_SIZE,
    /* 10101 */ 2 + ASAPI_STATUS_SIZE,
    /* 10110 */ 2 + ASAPI_STATUS_SIZE,
    /* 10111 */ 2 + ASAPI_STATUS_SIZE,
    /* 11000 */ 2 + ASAPI_STATUS_SIZE, // KILL
    /* 11001 */ 2 + ASAPI_STATUS_SIZE, // KILL
    /* 11010 */ 2 + ASAPI_STATUS_SIZE, // KILL
    /* 11011 */ 2 + ASAPI_STATUS_SIZE, // KILL
    /* 11100 */ 2 + ASAPI_STATUS_SIZE, // KILL
    /* 11101 */ 2 + ASAPI_STATUS_SIZE, // KILL
    /* 11110 */ 2 + ASAPI_STATUS_SIZE, // KILL
    /* 11111 */ 2 + ASAPI_STATUS_SIZE, // KILL
};


static const ot_u8 ext_std[4] = {
    /* 00 */ 0,
    /* 01 */ ASAPI_EXT_SIZE,
    /* 10 */ ASAPI_EXT_SIZE + ASAPI_FILECALL_SIZE, 
    /* 11 */ ASAPI_EXT_SIZE + ASAPI_FILECALL_SIZE,
};

  
  
ot_int sub_test_stream(ot_u8* msg, ot_u8 cmd, ot_u8 ext) {
    ot_u16 test_len;
    ot_u8* msg_curs;
    
    // Derive message alignment/boundary based on command and extension values.
    // msg_curs just takes the cursor past the initial boilerplate.  It is modified
    // subsequently after each template stage.
    test_len    = base_std[cmd&31];
    test_len   += ext_std[(ext>>5) & 3];
    msg_curs    = msg + test_len;
    
    // DLLS: contains a new key if encrypt_index and user_index are both 0
    if (cmd & ASAPI_CMD_DLLS) {
        test_len    = 2 + ((msg_curs[0] == 0) << 4);
        msg_curs   += test_len;  
    }
    
    // queries: there are 0, 1, or 2.  The order doesn't matter when just counting them
    if (cmd & ASAPI_CMD_ANYCAST) {
        sub_test_stream_QUERYLEN:
        test_len    = (msg_curs[0] & 0x80) ? msg_curs[1]<<1 : msg_curs[1];
        test_len   += 2;
        msg_curs   += test_len;
        
        if ((cmd & (ASAPI_CMD_MULTICAST+ASAPI_CMD_INIT)) == (ASAPI_CMD_MULTICAST+ASAPI_CMD_INIT)) {
            cmd = 0;
            goto sub_test_stream_QUERYLEN;
        }
    }
    
    // UDP
    if (ext & ASAPI_EXT_UDP) {
        ((ot_u8*)&test_len)[UPPER]  = msg_curs[0];
        ((ot_u8*)&test_len)[LOWER]  = msg_curs[1];
        test_len                   += ASAPI_UDP_SIZE;
        msg_curs                   += test_len;
    }
    
    return (ot_int)(msg_curs - msg);
}


ot_int sub_test_tmpl(   ot_u8 cmd, ot_u8 ext, 
                        asapi_dlls_tmpl* dlls,
                        query_tmpl* gquery, 
                        query_tmpl* lquery, 
                        udp_tmpl* udp           ) {
    ot_int test_len;
    test_len = base_std[ (cmd&31) | (ext&ASAPI_EXT_FILECALL) ];
    
    if (dlls != NULL) {
        test_len   += 2;
        test_len   += (dlls->encrypt_index == 0) << 4;  // 16 byte explicit key
    }
    if (gquery != NULL) {
        test_len += 2;
        test_len += (gquery->code & 0x80) ? gquery->length<<1 : gquery->length;
    }
    if (lquery != NULL) {
        test_len += 2;
        test_len += (lquery->code & 0x80) ? lquery->length<<1 : lquery->length;
    }
    if (udp != NULL) {
        test_len += (4 + udp->data_length);
    }
    
    return test_len;
}







/** ASAPI Server functions  <BR>
  * ========================================================================<BR>
  * The Server functions are designed to run on an OpenTag server, which is
  * almost always a microcontroller running OpenTag as an RTOS.
  */
  
#if (OT_FEATURE(SERVER))
#if OT_FEATURE(STROBE) 
#   include <hblib/strobe.h>
#endif


typedef struct {
    alp_tmpl*   alp;        // alp stream associated with this server
    //ot_bool     inprogress;
    ot_u16      wait;
    ot_queue    q;          // execution queue (bound to alp->inq)
} asapi_srv_struct;


static asapi_srv_struct srv;
static ot_u8 srv_buffer[256];


static ot_int sub_start_session(alp_tmpl* alp);
static void sub_stop_srv();


#ifndef EXTF_asapi_init_srv
void asapi_init_srv() {
    srv.alp         = NULL;
    //srv.inprogress  = False;
    
    q_init(&srv.q, srv_buffer, 256);
}
#endif


#ifndef EXTF_asapi_open_srv
void asapi_open_srv(alp_tmpl* alp) {
    if (srv.alp == NULL) {
        srv.alp         = alp;
        //srv.inprogress  = False;
    }
}
#endif


#ifndef EXTF_asapi_close_srv
ot_bool asapi_close_srv() {
    ot_bool was_active = False;
    
    if (srv.alp != NULL) {
        was_active = session_app_isloaded(&asapi_applet);
        if (was_active) {
            sub_stop_srv();
        }
    }
    return was_active;
}
#endif



static void sub_stop_srv() {
    session_app_purge(&asapi_applet);
    
    //alp_kill(srv.alp, ASAPI_ALP_ID);
    asapi_init_srv();
}


static void sub_q_purge(ot_queue* q) {
    if (q_length(q) <= 0) {
        q_init(&srv.q, srv_buffer, 256);
    }
}


static void sub_release_record(ot_queue* appq) {
    appq->front    += appq->front[1] + 4;
    appq->getcursor = appq->front;
}


static ot_u8* sub_retrieve_record(alp_record* apprec, ot_queue* appq) {
    memcpy((ot_u8*)&apprec->flags, appq->getcursor, 4);
    appq->getcursor    += 4;
    return appq->front;
}

static ot_u8 sub_check_more_records(ot_queue* appq) {
    return (q_span(appq) > 0);
}








ot_bool asapi_proc(alp_tmpl* alp, const id_tmpl* user_id) {
/// Don't actually process the messages, just load them into Server execution queue.
    ot_u8*      payload;
    ot_u8       ext_value;
    ot_u8       errcode     = 0;
    ot_u8       inrec_plen;
    ot_u8       inrec_cmd;
    ot_int      len_check;
    ot_uni16    tid;

    /// 1. if server is not set to this ALP stream, it is either not-open
    ///    (so open it), or it is open by another interface (send error).
    if (srv.alp != alp) {
        if (srv.alp == NULL) {
            asapi_open_srv(alp);
        }
        else {
            errcode = ASAPI_ERR_SERVERBUSY;
            goto asapi_proc_END;
        }
    }

    /// 2. "User" and "Root" are allowed to call this function, but not Guest.
//    if (auth_isuser(user_id) == False) {
//        errcode = ASAPI_ERR_PRIVILEGES;
//        goto asapi_proc_END;
//    }
    
    /// 3. We care about payload length and command header bytes for upcoming
    ///    validations.
    inrec_plen  = INREC(alp, PLEN);
    inrec_cmd   = INREC(alp, CMD);

    /// 4. Special commands are available if the STATUS bit is set on requests
    ///    from the client.  Notably, KILL.
    if (inrec_cmd & ASAPI_CMD_STATUS) {
        if (inrec_cmd & ASAPI_CMD_INIT) {  // KILL
            dll_refresh();
            sub_stop_srv();
        }
        errcode = ASAPI_ERR_FLUSH;
        goto asapi_proc_END;
    }

    /// 4. Do a packet integrity check by validating that the payload length is
    ///    as it should be.  Payload starts after 4 byte header.
    payload             = &alp->inq->getcursor[0];
    tid.ubyte[UPPER]    = payload[0];
    tid.ubyte[LOWER]    = payload[1];
    ext_value           = (inrec_cmd & ASAPI_CMD_EXT) ? payload[2] : 0;
    len_check           = sub_test_stream(payload, inrec_cmd, ext_value);
    len_check          -= (ot_int)inrec_plen;
    len_check           = (len_check < 0) - (len_check > 0);
    if (len_check) {
        errcode = ASAPI_ERR_BADDATA + len_check;
        goto asapi_proc_END;
    }

    /// 5. There are no busy, privilege, or framing errors.
    ///    Copy ALP into server execution queue
    sub_q_purge(&srv.q);
    len_check = 4 + inrec_plen;
    if (q_writespace(&srv.q) < len_check) {
        errcode = ASAPI_ERR_STACKFULL;
        goto asapi_proc_END;
    }
    q_writestring(&srv.q, &alp->inq->getcursor[-4], len_check);

    /// 6. Start the session engine if the hold-open bit is clear AND there is
    ///    not already a session in progress.
    if ((inrec_cmd & ASAPI_CMD_HOLDOPEN) == 0) {
        if (session_app_isloaded(&asapi_applet) == False) {
            errcode = sub_start_session(alp);
        }
    }
    //else {} ///@todo some sort of non-blocking stuff

    /// 7. End, send response (or not). 
    ///@todo the inrec/outrec feature of alp is going to be reworked soon.
    asapi_proc_END:
    if (inrec_cmd & ASAPI_CMD_RESPONSE) {
        alp->OUTREC(CMD) |= ASAPI_CMD_STATUS;
        alp->OUTREC(PLEN) = 6;
        q_writeshort(alp->outq, tid.ushort);                // Write transaction ID
        q_writebyte(alp->outq, 0);                          // RFU byte
        q_writebyte(alp->outq, errcode);                    // Write Error code
        q_writeshort(alp->outq, q_writespace(&srv.q));      // Space remaining in ALP queue
    }
    
    // Current variant is always atomic
    return True;
}





static ot_int sub_start_session(alp_tmpl* alp) {
/// A subroutine used by asapi_proc(), when an ASAPI session is first started.
    ot_u8           type_code;
    m2session*      active;
    session_tmpl    s_tmpl;
    alp_record      rec;
    advert_tmpl     adv;

    /// 1. Start with a blank session creation template.  It will get updated
    ///    later, either in the advertising setup or the applet itself.
    memset((ot_u8*)&s_tmpl, 0, sizeof(session_tmpl));
    
    ///@todo Fix Otter/HBCC/ASAPI in order to send an init message on session
    ///      init.  This may require altering HBCC API slightly from where it
    ///      is now.  Init needs to know if there is a "naked request" or a
    ///      advertising request that follows.
    
    /// 2. The first record must be an initialization record.  Advertising can
    ///    be used with init.type_code = 1.  With init.type_code = 0, no 
    ///    advertising is performed, simply a following request is processed.
    sub_retrieve_record(&rec, &srv.q);
    if ((rec.cmd & ASAPI_CMD_INIT) == 0) {
        goto sub_start_session_SESSION_ERR;
    }
    
    /// 3. If the message contains INIT, there are no further templates in the
    ///    message (by design).  Load the INIT template and then release this
    ///    ASAPI record.
    srv.q.getcursor+= (rec.cmd & ASAPI_CMD_EXT) ? 3 : 2;    //Ext byte & transid
    type_code       = q_readbyte(&srv.q);
    s_tmpl.flags    = q_readbyte(&srv.q);       // link_flags byte
    s_tmpl.flagmask = (1<<3) | (1<<4);          // RS coding flag & VID flag    ///@todo VID usage might need extra attention
    adv.duty_off    = q_readbyte(&srv.q);
    adv.duty_on     = q_readbyte(&srv.q);
    adv.subnet      = q_readbyte(&srv.q);
    adv.channel     = q_readbyte(&srv.q);
    s_tmpl.channel  = adv.channel;
    adv.duration    = q_readshort(&srv.q);
    sub_release_record(&srv.q);                 // We are done with init record
    
    /// 4. The initialization type_code indicates what sort of initialization 
    ///    should be performed.
    ///    <LI> 0: no explicity connection initialization, just a "naked 
    ///             request" that is specified in the next record. </LI>
    ///    <LI> 1: Standard advertising followed by request in next record</LI>
    ///    <LI> 255: Strobing, no follow-on request required. </LI>
    if (type_code > 1) {
        ///@todo reconcile session_app_isloaded(&asapi_applet) usage with strobe call
        ///      My guess is that it works fine, as asapi_applet is not loaded by a strobe call.
#       if OT_FEATURE(STROBE)
        // strobing: technically type_code should be 255
        // Parameters hacked from INIT template: channel, strobe-index, subnet, duration, interval
        hb_run_strobe(adv.channel, s_tmpl.flags, adv.subnet, adv.duration, adv.duty_on);
#       endif
        //srv.inprogress = False;
        return -1;
    }
    else if (sub_check_more_records(&srv.q) == 0) {
        goto sub_start_session_SESSION_ERR;
    }
    else if (type_code == 0) {  
        // Naked Request
        ///@todo this doesn't seem to work
        active = m2task_immediate(&s_tmpl, &asapi_applet);
    }
    else {
        // Classic Advertising, type_code == 1
        active = m2task_advertise(&adv, &s_tmpl, &asapi_applet);
    }
    
    /// 5. Report error if the session stack was too full to initialize the new
    ///    session.
    if (active == NULL) {
        return ASAPI_ERR_STACKFULL;
    }
    
    ///@note removing this flag, replaced with session_app_isloaded() calls, which
    ///      are direct checks to the underlying session stack, adding reliability.
    //srv.inprogress  = True;
    return 0;
    
    sub_start_session_SESSION_ERR:
    sub_stop_srv();
    return ASAPI_ERR_NOSESSION;
}




///@todo make sure that hold periods start getting clocked immedately after
/// valid REQUEST is processed.  This way, the delay period shall be front- 
/// chained from the point following the request transmission.

void asapi_applet(m2session* active) {
    static const ot_u8 opcode_lut[32] = {
        /* 0 00 00 */ M2OP_INV_F,
        /* 0 00 01 */ M2OP_INV_F,   //invalid
        /* 0 00 10 */ M2OP_ANN_F,
        /* 0 00 11 */ M2OP_ANN_S,
        /* 0 01 00 */ M2OP_INV_F,   //invalid
        /* 0 01 01 */ M2OP_INV_F,   //invalid
        /* 0 01 10 */ M2OP_ANN_F,   //invalid
        /* 0 01 11 */ M2OP_ANN_S,   //invalid
        /* 0 10 00 */ M2OP_INV_F,
        /* 0 10 01 */ M2OP_INV_F,   //invalid
        /* 0 10 10 */ M2OP_COL_FF,
        /* 0 10 11 */ M2OP_COL_FS,
        /* 0 11 00 */ M2OP_INV_S,
        /* 0 11 01 */ M2OP_INV_S,   //invalid
        /* 0 11 10 */ M2OP_COL_SF,
        /* 0 11 11 */ M2OP_COL_SS,
        /* 1 00 00 */ M2OP_UDP_F,
        /* 1 00 01 */ M2OP_UDP_F,   //invalid
        /* 1 00 10 */ M2OP_ANN_F,
        /* 1 00 11 */ M2OP_ANN_S,
        /* 1 01 00 */ M2OP_UDP_F,   //invalid
        /* 1 01 01 */ M2OP_UDP_F,   //invalid
        /* 1 01 10 */ M2OP_ANN_F,   //invalid
        /* 1 01 11 */ M2OP_ANN_S,   //invalid
        /* 1 10 00 */ M2OP_UDP_F,
        /* 1 10 01 */ M2OP_UDP_F,   //invalid
        /* 1 10 10 */ M2OP_COL_FF,
        /* 1 10 11 */ M2OP_COL_FS,
        /* 1 11 00 */ M2OP_UDP_S,
        /* 1 11 01 */ M2OP_UDP_S,   //invalid
        /* 1 11 10 */ M2OP_COL_SF,
        /* 1 11 11 */ M2OP_COL_SS
    };

    dialog_tmpl     dialog;
    isfcomp_tmpl    comp;
    isfcall_tmpl    call;
    alp_record      rec;
    ot_uint         next_wait;
    ot_u8           ext;
    ot_u8           status;
    ot_u8           local_status;
    ot_u8           scratch;

    /// 1. Preliminary operations
    ///@note check to session_app_isloaded() is redundant, because asapi_applet()
    ///      is only called from the session stack.
    if ((srv.alp == NULL) /*|| (session_app_isloaded(&asapi_applet) == False) */ ) {
        goto asapi_applet_CLOSED;
    }

    /// 2. Move to the next record that matches this ID.  If it returns False,
    ///    there are no more records of this type, so ASAPI is no longer in
    ///    in progress.
    ///    Else, store the command and move past the header
    if (sub_check_more_records(&srv.q) == 0) {
        sub_stop_srv();
        goto asapi_applet_CLOSED;
    }
    
    /// 3. alp_goto_next moved us to the right place, so formally retrive the
    /// command without worrying about validation.
    sub_retrieve_record(&rec, &srv.q);
    
    /// 4. If the netstate has SCRAP set, then this record should be released
    ///    before it even starts.
    if (active->netstate & M2_NETSTATE_SCRAP) {
        goto asapi_applet_END;
    }
    
    /// 5. Bypass Transaction ID, which is not relevant here.  It is important
    ///    during the message loading state, not the processing.  Also, save
    ///    the EXT byte if there is one.
    srv.q.getcursor    += ASAPI_TRANSID_SIZE;
    ext                 = (rec.cmd & ASAPI_CMD_EXT) ? q_readbyte(&srv.q) : 0;
    
    /// @todo is it true that INIT must be advertising?
    /// 6. If there is an init field here, it must be an advertising-type of
    ///    initialization.  Convert the active session to an advertising 
    ///    flooding session, then add an extension session for the follow-up 
    ///    request. Duty Cycling not supported for intra-session advertising.
    if (rec.cmd & ASAPI_CMD_INIT) {
        //rec.cmd |= ASAPI_CMD_HOLDOPEN;
        if (q_readbyte(&srv.q) != 1) { 
            active->netstate = M2_NETSTATE_SCRAP;
        }
        else {
            ot_uint adv_duration;
            active->netstate    = (M2_NETFLAG_BG | M2_NETFLAG_STREAM | M2_NETSTATE_REQTX | M2_NETSTATE_INIT);
            active->flags      |= (3<<3) & q_readbyte(&srv.q);      // lc flags
            srv.q.getcursor    += 2;                                // go past duty-cycle flags (not supported)
            active->subnet      = q_readbyte(&srv.q);
            active->channel     = q_readbyte(&srv.q);
            adv_duration        = q_readshort(&srv.q);

            ///@todo change to otapi_open_flood, or something
            dll_set_defaults(active);
        
            // Create a new session for the request following advertising.
            // This will be mostly overwritten. 
            active = session_extend(&asapi_applet, adv_duration, active->channel, M2_NETSTATE_REQTX);
        }
        
        sub_release_record(&srv.q);
        return;
    }

    /// 7. The message is not an init, so process the request data, update the
    ///    session variables, and build the request on-the-fly.  The otapi...()
    ///    calls will also set DLL variables to the appropriate values.

    // The delay/wait value describes the ticks (0.977ms) following this 
    // dialog that the session should pause before resuming.  It must be
    // zero for connection-oriented sessions (i.e. single advertising, 
    // multiple dialogs).  If 0, listen bit is set in session variables.
    
    // Dialog Template is always needed, always present
    active->subnet  = q_readbyte(&srv.q);
    active->channel = q_readbyte(&srv.q);   // req channel
    dialog.chanlist = srv.q.getcursor;
    scratch         = q_readbyte(&srv.q);   // resp channel
    dialog.channels = (scratch != active->channel);
    dialog.timeout  = q_readbyte(&srv.q);   // timeout code
    next_wait       = q_readshort(&srv.q);
    active->flags  |= (next_wait == 0) << 7;          // M2_FLAG_LISTEN
    
    // DLL Security Template can be included, set session vars for DLLS
    if (rec.cmd & ASAPI_CMD_DLLS) {
        active->flags  |= M2_FLAG_DLLS;
        active->extra   = q_readbyte(&srv.q);           // Key-Index to use for encryption
        active->extra  |= (q_readbyte(&srv.q) << 7);    // User=1, Root=0
    }
    
    // Routing Template is included when Addressing is Unicast, otherwise it is
    // implicit.  It must be used to open the request, though.
    ///@todo Routing is going to get updated with new DLL/network control
    ///      structure, but ASAPI will not support multihopping in 1.0 anyway.
    {   routing_tmpl routing;
        ot_u8 addressing;
        addressing   = (rec.cmd & ASAPI_CMD_ADDRMASK);
        if (addressing == ASAPI_CMD_UNICAST) {
            routing.hop_code    = q_readbyte(&srv.q);
            routing.hop_ext     = 0;
            routing.dlog.length = q_readbyte(&srv.q);
            routing.dlog.value  = q_markbyte(&srv.q, routing.dlog.length);
            addressing          = ADDR_unicast;                 // Unicast with UID Target (2)
            addressing         |= (routing.dlog.length == 2);   // Unicast with VID Target (3)
        }
        else {
            addressing          = ADDR_broadcast;
        }
        routing.hop_code    = 0;    //override hop code value
        otapi_open_request((ADDR_Type)addressing, &routing);
    }
    
    // The presence of these file tmpls and UDP tmpl dictate the command opcode
    scratch = 0;
    if (rec.cmd & ASAPI_CMD_FILECOMP) {
        alp_breakdown_isfcomp_tmpl(&srv.q, (void*)&comp);
        scratch  = (2 | (comp.is_series & 1) << 2);
    }
    if (ext & ASAPI_EXT_FILECALL) {
        alp_breakdown_isfcall_tmpl(&srv.q, (void*)&call);
        scratch |= (2 | (call.is_series & 1));
    }
    else if (ext & ASAPI_EXT_UDP) {
        scratch |= (1<<4);
    }

    // Command Tmpl Building based on existing information
    // Plus command & Dialog Tmpl streaming
    {   ///@todo make sure this works with multicast, I don't think it really does.
        static const ot_u8 m2tt_lut[4] = { M2TT_REQ_UB, M2TT_REQ_UB, M2TT_REQ_A, M2TT_REQ_M_INIT };
        command_tmpl cmd;
        cmd.type        = m2tt_lut[(rec.cmd & ASAPI_CMD_ADDRMASK)];
        cmd.opcode      = opcode_lut[scratch];
        cmd.extension   = 0;
        otapi_put_command_tmpl(&local_status, &cmd);
        otapi_put_dialog_tmpl(&status, &dialog);
        status &= local_status;
    }

    // Load Queries, if enabled
    if ((rec.cmd & ASAPI_CMD_ADDRMASK) == ASAPI_CMD_MULTICAST) {
        query_tmpl query;
        alp_breakdown_query_tmpl(&srv.q, (void*)&query);
        otapi_put_query_tmpl(&local_status, &query);
        status &= local_status;
    }
    if (rec.cmd & ASAPI_CMD_LQUERY) {
        query_tmpl query;
        alp_breakdown_query_tmpl(&srv.q, (void*)&query);
        otapi_put_query_tmpl(&local_status, &query);
        status &= local_status;
    }

    // call & comp
    if (rec.cmd & ASAPI_CMD_FILECOMP) {
        otapi_put_isf_comp(&local_status, &comp);
        status &= local_status;
    }
    if (ext & ASAPI_EXT_FILECALL) {
        otapi_put_isf_call(&local_status, &call);
    }
    else if (ext & ASAPI_EXT_UDP) {
        udp_tmpl udp;
        alp_breakdown_udp_tmpl(&srv.q, (void*)&udp);
        otapi_put_udp_tmpl(&local_status, &udp);
    }
    
    /// Status message checking.  It should be non-zero.  If not, 
    /// scrap the session and don't bother closing the request
    /// (because it's going to get cancelled anyway).
    status &= local_status;
    if (status == 0) {
        active->netstate = M2_NETSTATE_SCRAP;
    }
    else {
        // Close request, which adds footers and encryption
        otapi_close_request();
    }
    
    
    /// 8. The message building is done.  This ASAPI ALP record must be released
    ///    to allow the next one to get used.  If there is an error, the session
    ///    flag "M2_NETSTATE_SCRAP" will be set.  
    ///    <LI> On error, ASAPI will kill itself, flushing all ASAPI data from
    ///         the input stream of the current ALP. </LI>
    ///    <LI> If this is not final command of an ASAPI sequence (HOLDOPEN is
    ///         set), then we need to create a dummy session which will recall
    ///         asapi_applet() when it begins.  </LI>
    ///    <LI> On the other hand, if HOLDOPEN is clear, we need to turn-off the
    ///         "inprogress" flag to allow new ASAPI sequences to be entered. </LI>
    asapi_applet_END:
    sub_release_record(&srv.q);

    if (active->netstate & M2_NETSTATE_SCRAP) {
        sub_stop_srv();
    }
    else if (rec.cmd & ASAPI_CMD_HOLDOPEN) {
        session_continue(active->applet, M2_NETSTATE_REQTX, next_wait);
    }
    ///@note session_app_isloaded() usage allows us to remove the inprogress flag
    //else {
    //    srv.inprogress = False;
    //}
    return;

    /// 9. the server is closed, yet this applet was invoked.  Scrap it,
    ///    which will cause no communication to occur.
    asapi_applet_CLOSED:
    active->netstate = M2_NETSTATE_SCRAP;
}



#endif



#endif

