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
  * @file       /hb_dftp/hb_dftp.c
  * @author     JP Norair
  * @version    R100
  * @date       21 Feb 2014
  * @brief      DFTP implementation for H-Builder and OpenTag/ALP
  * @defgroup   hb_dftp
  * @ingroup    hb_dftp
  * 
  * @note this is a developmental implementation of DFTP which supports only
  * the PUT request.  PUT requires DATA, ACK, and GPACK as well.  All other
  * commands (or data types if you prefer) are not tested.
  *
  * Concept of Operation
  * ====================
  *
  * Interface --> Application: 
  * The interface that calls the DFTP app (this module) does so by copying an
  * ALP record from its interface buffer to its ALP queue and then calling
  * alp_parse_message().  If the ALP record/message is meant for DFTP, it will
  * get here via alp_parse_message().
  *
  * Application --> Interface:
  * This DFTP application will write its response to alp->outq, just like any
  * normal ALP process should.  The interface that called alp_parse_message()
  * can do whatever it wants with that information.
  *
  * Special Note on usage with DASH7: 
  * DASH7's MAC implements a structured, low-level transport model utilizing
  * REQUEST and RESPONSE pairs.  Normal TFTP uses UDP/IP, and UDP/IP does not
  * offer such a model.  With DASH7, DFTP works in its normal way, but the 
  * interface caller decides what to do based on the response in alp->outq.
  * If there is no response, then it will scrap the DASH7/M2 session.
  *
  ******************************************************************************
  */

#include <alp/dftp.h>

#if (OT_FEATURE(ALP) && OT_FEATURE(SERVER))

#if (ALP(DFTP) != ENABLED)
ot_bool dftp_proc(alp_tmpl* alp, id_tmpl* user_id) {
    return True;
}

#else

ot_bool dftp_init() {
    dftp.evtdone = &otutils_sig_null;
    dftp.tid    = 0;
}


ot_int sub_datalength(ot_u8);



ot_uint dftp_state() { return dftp.state; }


ot_bool dftp_proc(alp_tmpl* alp, id_tmpl* user_id) {
    ot_int err_code = -1;
    
    /// Section A: Formal ALP setup.
    /// The formal way to handle applications using ALP is to create a local
    /// application queue that attaches to the ALP stream, and then to 
    /// retrieve records and release records.  This is MANDATORY for non-atomic
    /// applications, i.e. applications that retain records in the ALP queue.
    ///
    /// For purely atomic applications, we don't actually need to do this, we
    /// can operate directly on the alp queue.  So, section A is commented-out
    /// and section B shows how to use the ALP "quick and dirty" for atomic 
    /// apps.
    
    /*
    alp_record  rec;
    ot_queue local_q;
#   define appq                 (&local_q)
#   define dftp_ctl             rec.cmd
#   define dftp_length          rec.plength
    
    ///A1. Bind the local queue to the ALP queue.  This is a regular practice
    ///    for ALP apps reading-in data from the ALP stream.  The other 
    ///    regular practice is to call alp_retrieve_record() and then 
    //     alp_release_record() after finishing.
    alp_new_appq(alp, appq);
    
    ///A2. Get the record information.  If the function returns NULL, it means
    ///    that a segmentation fault has occured between two applications on
    ///    this ALP (extremely rare, if not entirely improbable).  Nonetheless
    ///    we handle the exception by killing this app's allocation in the ALP.
    if (alp_retrieve_record(&rec, appq, DFTP_ALP_ID) == NULL) {
        alp_kill(alp, ALP_ID_DFTP);
        return False;
    }
    */
    
    
    /// Section B: Quick and Dirty way of setting up this app to an ALP stream,
    ///            which is OK as long as app uses ALP *only* atomically.
    ot_u8   dftp_ctl        = alp->INREC(CMD);
    ot_u8   dftp_length     = alp->INREC(PLEN);
#   define appq (alp->inq)
    
    
    /// Section C: Validate basic parts of DFTP
    /// This section is commented-out because these features are not presently
    /// implemented in current builds of OpenTag, or they are implemented in
    /// different ways (i.e. at deeper level)
    
    ///C1. User can be root, user, or guest, or potentially any network-level
    ///    user.  At present this is done directly at the file access level 
    ///    rather than at the system level, but this skeleton code below might
    ///    be implemented in the future.
    //if (auth_user_exists(user_id) == False) {
    //    err_code = 7;           //No such user
    //    goto dftp_proc_SENDERR;
    //}
    
    ///C2. Do any other preliminary ALP operations, as after this point the 
    ///    ALP record header is repurposed to meet the local needs of the DFTP
    ///    app.  The data is handled "atomically" (the record is removed by the
    ///    ALP controller, alp_parse_message(), upon this function returning),
    ///    so it makes no difference if we fiddle with the ALP record header.
    
    
    /// Section D: normal processing of DFTP app data
    
    ///D1. Get the DFTP Control Byte and TID, which will control the way the 
    ///    rest of the datagram is handled.  Use it to verify the structure of 
    ///    the data.
    dftp_length-= 2;
    dftp_ctl    = q_readbyte(alp->inq);
    dftp_tid    = q_readbyte(alp->inq);
    if ((dftp.state != DFTP_IDLE) && (dftp_tid != dftp.tid)) {
        err_code = 5;           //Uknown Transfer ID
        goto dftp_proc_END;
    }
      
    scratch = (dftp_ctl & 7);
    
    ///D2. If this command is a DFTP-Request (Read-1, Write-2, Get-9, Put-10):
    ///   <LI> check state of DFTP, which must be idle </LI>
    ///   <LI> check that ALP tmpl is 7 bytes </LI>
    ///   <LI> run ALP tmpl through File ALP to see if valid </LI>
    if (((scratch == DFTP_OPC_RRQ) || (scratch == DFTP_OPC_WRQ)) \
    && (dftp.state == 0) && (dftp_length != 7)) {
        // Initiate the veelite accessing process.  Veelite has its own error
        // reporting, which must be converted to DFTP/TFTP numbers.
        vl_header file_hdr;
        dftp.file.block     = q_readbyte(alp->inq);
        dftp.file.id        = q_readbyte(alp->inq);
        dftp.file.start     = q_readshort(alp->inq);
        dftp.file.xsize     = q_readshort(alp->inq);
        dftp.file.xbytes    = 0;
        err_code            = vl_getheader( &file_hdr, dftp.file.block, dtfp.file.id, 
                                            VL_ACCESS_RW, user  );
        // Writing (Put or Write)
        // - If file doesn't exist, create it if this feature is allowed.  Access 
        //       privileges will be enforced as well via vl_new().
        // - If the file does exist, open it.  Report access error.
        // - If the file is successfully opened, set the length to 0 via vl_store().
        if (dftp_ctl & 2) {
            vlFILE* fp = NULL;
            if ((err_code == 1) && (OT_FEATURE(VL_NEW)) {
                err_code = vl_new(  &fp, dftp.file.block, dftp.file.id, 
                                    VL_ACCESS_RW, dftp.file.xsize, user );
            }
            else if (err_code == 0) {
                if (file_hdr.alloc < dftp.file.xsize) {
                    err_code = 8;
                }
                else {
                    fp = vl_open(dftp.file.block, dftp.file.id, VL_ACCESS_W, user);
                    if (fp == NULL) err_code = 255;
                    else            vl_store(fp, 0, NULL);
                }
            }
            vl_close(fp);
        }
        
        // Reading (Get or Read)
        // - If the requested transfer size is 0 or larger than file size, 
        //      then set the transfer size to the length of the file.
        // - If READ ((dftp_ctl & 8) == 0), then send first part of the data.
        // - If GET, then simply send GPACK
        else if (err_code == 0) {
            if ((dftp.file.xsize == 0) || (dftp.file.xsize > file_hdr.length)) {
                dftp.file.xsize = file_hdr.length;
            }
            if ((dftp_ctl & 8) == 0)    sub_load_file();
            else                        sub_load_gpack();
        }
        
        // Convert Veelite Error to DFTP/TFTP error
        err_code = sub_convert_error(err_code);
        
        // If there's an error, set state to 0.  Else, state gets set to 1 or 2.
        dftp.state  = (err_code < 0) ? (dftp_ctl & 3) : 0;
        dftp.flags  = (dftp_ctl & DFTP_CTL_LZO);
    }
    
    /// 4. If this command is a Data payload (3):
    ///   <LI> Make sure CRC32 is valid, if CRC32 is enabled </LI>
    ///   <LI> If CRC32 is invalid or there is some other issue, SCRAP the
    ///            session.  This applies to Client or Server. </LI>
    ///   <LI> Append to the file, after doing LZO decompression if necessary </LI>
    else if ((scratch == DFTP_OPC_DAT) && (dftp.state & DFTP_WAITFOR_DAT)) {
        ot_bool crc_error = False;
        if (dftp_ctl & DFTP_CTL_CRC32) {
            ot_int total_length;
            total_length    = dftp_length;
            dftp_length    -= 4;
            crc_error       = (crc32_calc_block(alp->inq->getcursor, total_length) != 0);
        }
        if (crc_error) {
            sub_load_noresp(alp->outq);
        }
        else {
            dftp.flags |= (dftp_ctl & DFTP_CTL_LZO);
            if (dftp.flags & DFTP_CTL_LZO) {
                // LZO implementation not available right now.
            }
            else {
                vlFILE* fp;
                dftp.file.xsize -= block_size;
                fp = vl_open(dftp.file.block, dtfp.file, VL_ACCESS_W, user);
                if (fp == NULL) {
                    err_code = 0;   // "unknown" error
                }
                else {
                    vl_append(fp, block_size, q_markbyte(alp->inq, block_size));
                    vl_close(fp);
                }
            }
        }
    }
    
    /// 5. If this command is an ACK (4):
    ///   <LI> Server transmits (client receives) ACK-REQ during PUT sessions. </LI>
    ///   <LI> Server transmits (client receives) ACK-RESP during WRITE sessions. </LI>
    ///   <LI> Server receives (client transmits) ACK-REQ during READ sessions. </LI>
    ///   <LI> Server receives (client transmits) ACK-RESP during GET sessions. </LI>
    ///   <LI> When client or server receives ACK, it must prepare next DATA to send </LI>
    ///   <LI> When client or server receives ACK-RESP, it must continue the session </LI>
    
    // SERVER: whether GET or READ, the same thing occurs: produce a data payload
    else if ((scratch == DFTP_OPC_ACK) && (dftp.state & 1)) {
        sub_load_file(alp->outq);
    }
    // CLIENT:
    //else if ((scratch == DFTP_OPC_ACK) && (dftp.state & 2)) {
    //    sub_load_file(alp->outq);
    //}
    
    /// 6. If this command is an ERR (5):
    ///   <LI> Server will drop the connection by state management and noresp </LI>
    ///   <LI> Client should restart the connection if the error is recoverable </LI>
    else if (scratch == DFTP_OPC_ERR) {
        dftp.state = 0;
        sub_load_noresp(alp->outq);
    }
    
    /// 7. If this command is a GPACK (12, Client only):
    ///  <LI> Server transmits (client receives) GPACK-RESP after PUT or GET REQ </LI>
    ///  <LI> Client receiving GPACK should start listening for DATA or ACK REQ </LI>
    ///  <LI> Server transmitting GPACK should create follow-on session 
    ///           containing DATA or ACK REQ </LI>
    // CLIENT: Switch the applet pointer to listen-for-request.  UDP Adaptation
    //         Layer should declare continuation of session by default.
    //else if (((dftp_ctl & 15) == DFTP_OPC_GPACK) && (dftp.state & DFTP_WAITFOR_GPACK)) {
    //    dftp.listen_time = q_readshort(alp->inq);
    //    sub_load_listen(alp->sstack);
    //}
    
    ///8. command is invalid
    else {
        err_code = 4;   // Illegal TFTP/DFTP operation
    }
    
    /// 9. 
    if (err_code >= 0) {
        dftp.state = 0;
        sub_load_error(err_code);
    }
    
    /// No need to release record because we are doing DFTP the Quick and Dirty
    /// fully-atomic way, so there was no record formally retreived.
    //alp_release_record(&dftpq);
    return True;
}





ot_int sub_convert_error(ot_int vl_err) {
// VL Err     DFTP Err
//   0    -->  -1       (No error)
//   1    -->   1       (file not found)
//   4    -->   2       (invalid permissions / access violation)
//   5    -->   3       (length too long / disk-full or allocation exceeded)
//   6    -->   3       (disk-full or allocation exceeded)
//   8    -->   3       (disk-full or allocation exceeded)
//  255   -->   0       (unknown error / undefined error)
    static const ot_u8 err_table[8] = { 1, 0, 0, 2, 3, 3, 0, 3 };
    vl_err--;
    if (vl_err <= 0) {
        vl_err = (vl_err > 7) ? 0 : (ot_int)err_table[vl_err];
    }
    return vl_err;
}


void sub_load_file(ot_queue* outq) {
    sub_load_header(outq, ...DAT with flags...);
    
    ... open file, check file block variables in dftp datastruct, and write block.
    q_writeshort(outq, blocknum);
    
    outq->putcursor+= vl_load(fp, outq->putcursor, ... );
    
    if (dftp.flags & DFTP_CTL_CRC32) {
        ot_u32 crc32_val;
        crc32_val = crc32_calc_block(alp->inq->getcursor, total_length);
        q_writelong(outq, crc32_val);       // writelong_be ?
    }
}


void sub_load_gpack(ot_queue* outq) {
    sub_load_header(outq, ...CONTROL...);
    q_writeshort(outq, dftp.timeout );
}


void sub_load_noresp(ot_queue* outq) {
    
}
        

void sub_load_listen(void* sstack) {
///@note the hardcoding of the top-session access is temporary, and it must be
///      fixed to general-purpose once the session libary is generalized.
    if (sstack != NULL) {
        if (session_notempty()) {
            session_continue(&applet_listen, M2_NETSTATE_REQRX, 0);
        }
    }
}


void sub_load_error(ot_queue* outq, err_code) {
    sub_load_header(outq, CONTROL);
    q_writeshort(outq, err_code);
}






// Primary DFTP applet feature is just to retransmit the TXQ following a RX timeout.

/// Client listen for ACK or DATA, following reception of GPACK
void applet_listen(m2session* active) {

}



/// Server ACKREQ is an applet called during session instantiation.  It is 
/// used for PRQ sessions
void applet_ackreq(m2session* active) {
    
}


/// Server DATAREQ is an applet called during session instantiation.  It is 
/// used for GRQ sessions
void applet_datareq(m2session* active) {
    
}


#endif
#endif

