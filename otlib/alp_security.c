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
  * @file       /otlib/alp_security.c
  * @author     JP Norair
  * @version    R101
  * @date       24 Mar 2014
  * @brief      ALP to Security protocol processor
  * @ingroup    ALP
  *
  * Presently, the security ALP has only the functionality to read and write
  * cryptographic keys for Data-Link-Layer security (DLLS).
  * <LI> ID = ersxxx00: Delete DLLS Key </LI>
  * <LI> ID = ersxxx01: Read DLLS Key </LI>
  * <LI> ID = ersxxx10: Update DLLS Key </LI>
  * <LI> ID = ersxxx11: Create New DLLS Key </LI>
  *
  * 'e' from above is the "response-Enable" bit, which the client must set to 1 
  * if it wants a response.  This is universal to all ALP commands.
  *
  * 'r' from above is the "Return data" bit, which should be 0 for all requests
  * and 1 for all responses.
  * 
  * 's' from above is the "ID Specifier": 0 means the Key Index (2 bytes) is
  * used to specify the Key to be created/read/written/deleted.  1 means the
  * the DASH7 id template is used (3 to 9 bytes)
  *
  * 'xxx' from above are RFU bits which should be 000.  This implementation 
  * will return an unsupported error if not 00.
  * 
  * The format of the payload for *requests* is shown below.  Some fields are
  * optional, and are only present when the command-byte value includes the
  * appropriate bit value(s).  Key Type will always be 0 (AES128) until further 
  * notice.  Likewise, Key Len will always be 16 (128 bits).
  * <PRE>
  * ---------+-----------+-----------+----------+----------+---------+---------+
  * Field    | Key Index | ID Length | ID value | Key Type | Key Len | Key Val |
  * Option   |  's'=0    |        's'=1         |    Cmd = update or create    |
  * Bytes    |     2     |    1      |  2 or 8  |    1     |    1    |   [16]  |
  * ---------+-----------+-----------+----------+----------+---------+---------+
  * </PRE>
  *
  * The format of the payload for *response* is shown below.  The format is the
  * same as for request, except that a status field is included as well.  The
  * Status Field will report an error.  If Status is 0, there is no error.
  * <PRE>
  * ---------+--------+-------------------------------+
  * Field    | Status |     [All request fields]      |
  * Option   |        | Status=0 & Cmd=Read or Create |
  * Bytes    |   1    |          21 to 29             |
  * ---------+--------+-------------------------------+
  * </PRE>
  *
  * Status Codes:                                                   <BR>
  * 0: No error                                                     <BR>
  * 1: No key found for this specifier                              <BR>
  * 2: Can't create new key                                         <BR>
  * 3: Can't delete key, it is a required key                       <BR>
  * 4: Insufficient permissions to access this protocol             <BR>
  * 5: Can't create new key, supplied length doesn't match spec     <BR>
  * 7: Can't update key, supplied length doesn't match spec         <BR>
  * all other codes: unknown error (feature unsupported)            <BR> 
  * 
  * 
  ******************************************************************************
  */

///@todo this is untested code.  The Auth module is incomplete and must be 
///      augmented to include functions & data elements referenced in this code.

#include <otlib/alp.h>

#if ((OT_FEATURE(ALP) == ENABLED) && (OT_FEATURE(SECURITY) == ENABLED))


#include <otlib/auth.h> 


#define ALP_SEC_RESPOND     (1<<7)
#define ALP_SEC_STATUS      (1<<6)
#define ALP_SEC_IDTMPL      (1<<5)
#define ALP_SEC_RFUMASK     (7<<3)
#define ALP_SEC_OPMASK      (3<<0)
#define ALP_SEC_DELETE      (0<<0)
#define ALP_SEC_READ        (1<<0)
#define ALP_SEC_UPDATE      (2<<0)
#define ALP_SEC_CREATE      (3<<0)



#ifndef EXTF_alp_proc_sec
OT_WEAK ot_bool alp_proc_sec(alp_tmpl* alp, const id_tmpl* user_id) {
/// At present, only DLLS features are supported, no NLS
    key_tmpl    key_data;
    id_tmpl     id_data;
    id_tmpl*    id_ptr;
    ot_int      key_index;
    ot_u8       errcode = 0;
    ot_u8       cmd_in  = q_getcursor_val(alp->inq, 0);
    
    alp->inq->getcursor+= 4;

#   if 0 || (OT_FEATURE(CLIENT))
    /// Put your client code in here, which handles the response.  Response
    /// is noted by having the Status bit set.
    if (cmd_in & ALP_SEC_STATUS) {
        alp_push_sec(alp, user_id);
    }
#   endif

#   if 1 || (OT_FEATURE(SERVER))
    // User must be root in order to use this protocol
    if (auth_isroot(user_id) == False) {
        errcode = 4;
        goto alp_proc_sec_RESPONSE;
    }
    
    // Only support RFUMASK = 000
    if (cmd_in & ALP_SEC_RFUMASK) {
        errcode = 255;
        goto alp_proc_sec_RESPONSE;
    }
    
    // If ID is used, breakdown the ID tmpl and use it to find the Key index.
    // else, just use the damn key index.
    if (cmd_in & ALP_SEC_IDTMPL) {
        alp_breakdown_id_tmpl(alp->inq, (void*)&id_data);
        id_ptr      = &id_data;
        key_index   = auth_find_keyindex(&id_data);
    }
    else {
        id_ptr      = NULL;
        key_index   = q_readshort(alp->inq);
    }
    
    // There are four commands: Delete, Read, Update, Create.
    // Update (10) and Create (11) include a key in the request.  
    // Read (01) and Create (11) include a key in the response.
    
    // Create/Update a key.  When creating a new key, key_index is replaced.
    if (cmd_in & 2) {
        alp_breakdown_key_tmpl(alp->inq, (void*)&key_data);
    
        if (cmd_in & 1) {
            errcode = auth_create_key(&key_index, &key_data, id_ptr);
        }
        else {
            errcode = auth_update_key(key_index, &key_data);
        }
    }
    
    // Delete a key
    else if ((cmd_in & 3) == 0) {
        errcode = auth_delete_key(key_index);
    }
    
    // Do read operation if no errors and command is read or create
    if ((cmd_in & 1) && (errcode == 0)) {
        errcode = alp_read_key(key_index, &key_data);
    }

    // Handle response
    // Always include status.  
    // Always include key index when status==0
    // Include id and key data conditionally
    alp_proc_sec_RESPONSE:
    if (cmd_in & ALP_SEC_RESPOND) {
        alp->OUTREC(CMD)    &= 0x3F;
        alp->OUTREC(CMD)    |= ALP_SEC_STATUS;
        q_writebyte(alp->outq, errcode);
        
        if (errcode == 0) {
            q_writeshort(key_index);
            if (cmd_in & ALP_SEC_IDTMPL)    alp_stream_id_tmpl(alp->outq, &auth_data);
            if (cmd_in & 1)                 alp_stream_key_tmpl(alp->outq, &key_data);
        }
    }
    
    return True;
#   endif
}
#endif



#if (OT_FEATURE(CLIENT) && !defined(EXTF_alp_push_sec))
OT_WEAK void alp_push_sec(alp_tmpl* alp, const id_tmpl* user_id) {
/// Client function, simply pushes the response data to some upper layer.
/// You'll want to write this yourself.  This one here is an empty function
/// that does nothing, and it only exists to present this warning and prevent
/// compilations from failing.

#   warning "You are using an empty function for alp_push_sec()"

}
#endif

#endif
