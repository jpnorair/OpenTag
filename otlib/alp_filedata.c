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
  * @file       /OTlib/alp_filedata.c
  * @author     JP Norair
  * @version    V1.0
  * @date       08 May 2011
  * @brief      Application Layer protocol (ALP) for Filesystem Operations
  * @ingroup    ALP's
  *
  * Works with the file system (veelite) and the general form of ALP's.  The
  * ALP's may be transmitted as LLDP (condensed over-the-air server-to-server) 
  * or NDEF (over the wire for client-server).
  *
  * File Data Subprotocol, Basic Directive Structure (LLDP).  The NDEF format is
  * somewhat different, but the information is conveyed to this module in the
  * same way (through the proc function).
  * +-----------+-----------+-----------+-----------+
  * | Directive | Directive | Directive | Directive |
  * |  Length   |    ID     |  Command  |   Data    |
  * +-----------+-----------+-----------+-----------+
  * |  2 Bytes  |  1 Byte   |  1 Byte   | N-4 Bytes |
  * +-----------+-----------+-----------+-----------+
  * |     N     |    0x60   |     —     |     —     |
  * +-----------+-----------+-----------+-----------+
  *
  * Directive Command Field:
  * b7:     Respond Bit     0 don't respond
  *                         1 Respond with directive return template
  *
  * b6-4:   File Block      001 GFB
  *                         010 ISFSB (Series)
  *                         011 ISFB
  *
  * b3-0:   Operand         0000: Read File Permissions 
  *                         0001: Return File Permissions 
  *                         0011: Write File Permissions 
  *                         0100: Read File Data
  *                         0101: Return File Data 
  *                         0111: Write File Data 
  *                         1000: Read File Headers 
  *                         1001: Return File Headers 
  *                         1010: Delete File (optional) 
  *                         1011: Create New File (optional) 
  *                         1100: Read File Header + Data 
  *                         1101: Return File Header + Data 
  *                         1110: Restore File (optional) 
  *                         1111: Return Error
  ******************************************************************************
  */


#include "alp.h"

#if (   (OT_FEATURE(ALP) == ENABLED) && \
        (OT_FEATURE(SERVER) == ENABLED) && \
        (OT_FEATURE(VEELITE) == ENABLED)    )

#include "auth.h"
#include "veelite.h"


// Processing subroutines
typedef ot_int (*sub_file)(ot_bool, alp_record*, Queue*, Queue*, id_tmpl*);


ot_int sub_return(ot_bool respond, alp_record* in_rec, 
                        Queue* in_q, Queue* out_q, id_tmpl* user_id );

ot_int sub_fileperms(ot_bool respond, alp_record* in_rec,
                        Queue* in_q, Queue* out_q, id_tmpl* user_id );
                        
ot_int sub_fileheaders(ot_bool respond, alp_record* in_rec,
                        Queue* in_q, Queue* out_q, id_tmpl* user_id );
                        
ot_int sub_filedata(ot_bool respond, alp_record* in_rec,
                        Queue* in_q, Queue* out_q, id_tmpl* user_id );
                        
ot_int sub_filedelete(ot_bool respond, alp_record* in_rec,
                        Queue* in_q, Queue* out_q, id_tmpl* user_id );
                        
ot_int sub_filecreate(ot_bool respond, alp_record* in_rec,
                        Queue* in_q, Queue* out_q, id_tmpl* user_id );   
                                  
ot_int sub_filerestore(ot_bool respond, alp_record* in_rec,
                        Queue* in_q, Queue* out_q, id_tmpl* user_id );
                        
//ot_int sub_fileerror(ot_bool respond, alp_record* in_rec, 
//                        Queue* in_q, Queue* out_q, id_tmpl* user_id );








// Callable processing function
void alp_proc_filedata(alp_record* in_rec, alp_record* out_rec, \
                            Queue* in_q, Queue* out_q, id_tmpl* user_id) {
    //sub_file cmd;
    const sub_file cmd[] = {
  	  	  &sub_fileperms,
          &sub_return,
          &sub_fileperms,
          &sub_fileperms,
          &sub_filedata,
          &sub_return,
          &sub_filedata,
          &sub_filedata,
          &sub_fileheaders,
          &sub_return,
          &sub_filedelete,
          &sub_filecreate,
          &sub_filedata,
          &sub_return,
          &sub_filerestore,
          &sub_return
    };

	ot_int  data_out;
    ot_bool respond = (ot_bool)(in_rec->dir_cmd & 0x80);
    
    // Return value is the number of bytes of output the command has produced
    /*
    switch (in_rec->dir_cmd & 0x0F) {
        case 0:     cmd = &sub_fileperms;               break;
        case 1:     cmd = &sub_return;                  break;
        case 2:     //cmd = &sub_fileperms;               break;
        case 3:     cmd = &sub_fileperms;               break;
        case 4:     cmd = &sub_filedata;                break;
        case 5:     cmd = &sub_return;                  break;
        case 6:     //cmd = &sub_filedata;                break;
        case 7:     cmd = &sub_filedata;                break;
        case 8:     cmd = &sub_fileheaders;             break;
        case 9:     cmd = &sub_return;                  break;
        case 10:    cmd = &sub_filedelete;              break;
        case 11:    cmd = &sub_filecreate;              break;
        case 12:    cmd = &sub_filedata;                break;
        case 13:    cmd = &sub_return;                  break;
        case 14:    cmd = &sub_filerestore;             break;
        case 15:    cmd = &sub_return;                  break;
    }
    data_out = cmd(respond, in_rec, in_q, out_q, user_id);
    */
    data_out = cmd[in_rec->dir_cmd & 0x0F](respond, in_rec, in_q, out_q, user_id);
    
    if (respond) {        
        //Transform input cmd to error or data return variant for response
        // - for write and control funcs, error is the response
        // - for read, data return is the response
        out_rec->dir_cmd    = in_rec->dir_cmd & ~0x80;
        out_rec->dir_cmd   |= (in_rec->dir_cmd & 0x02) ? 0x0F : 0x01;   //02 is write mask
        out_rec->flags     &= ~ALP_FLAG_CF;
        out_rec->flags     |= (ot_u8)(in_rec->bookmark != NULL) << 5;
    }
    
    out_rec->payload_length = data_out;
}





// Return functions are not handled by the server (ignore)
ot_int sub_return(ot_bool respond, alp_record* in_rec,
                    Queue* in_q, Queue* out_q, id_tmpl* user_id ) {
    return 0;
}


ot_bool sub_testchunk(ot_int data_in) {
    return (ot_bool)(data_in > 0);
}



/// This is a form of overwrite protection
ot_bool sub_qnotfull(ot_bool write, ot_u8 write_size, Queue* q) {
    return (ot_bool)(((q->putcursor+write_size) < q->back) || (write == False));
}




ot_int sub_fileperms(ot_bool respond, alp_record* in_rec,
                        Queue* in_q, Queue* out_q, id_tmpl* user_id ) {
    
    ot_int  data_out    = 0;
    ot_int  data_in     = in_rec->payload_length;
    vlBLOCK file_block  = (vlBLOCK)((in_rec->dir_cmd >> 4) & 0x07);
    ot_u8   file_mod    = ((in_rec->dir_cmd & 0x02) ? VL_ACCESS_W : VL_ACCESS_R);

    /// Loop through all the listed file ids and process permissions.
    while ((data_in > 0) && sub_qnotfull(respond, 2, out_q)) {
        ot_u8   file_id         = q_readbyte(in_q);
        ot_bool allow_write     = respond;
        vaddr   header;
        
        data_in--;  // one for the file id
        
        if (file_mod == VL_ACCESS_W ) {
            /// run the chmod and return the error code (0 is no error)
            data_in--;  // two for the new mod
            file_mod = vl_chmod(file_block, file_id, q_readbyte(in_q), user_id);
        }
        else if (allow_write) {
            /// Get the header address and return mod (offset 5).  The root user
            /// (NULL) is used because this is only for reading filemod.
            /// Note: This is a hack that is extremely optimized for speed
            allow_write = (ot_bool)(vl_getheader_vaddr(&header, file_block, file_id, \
                                                    VL_ACCESS_R, NULL) == 0);
            if (allow_write) {
                Twobytes filemod;
                filemod.ushort  = vworm_read(header + 4);   //shortcut to idmod, hack-ish but fast
                file_mod        = filemod.ubyte[1];
            }
        }
        if (allow_write) {
            /// load the data onto the output, if response enabled
            q_writebyte(out_q, file_id);
            q_writebyte(out_q, file_mod);
            data_out += 2;
        }
    }
    
    /// return number of bytes put onto the output (always x2)
    in_rec->bookmark = (void*)sub_testchunk(data_in);
    return data_out;
}




ot_int sub_fileheaders( ot_bool respond, alp_record* in_rec,
                        Queue* in_q, Queue* out_q, id_tmpl* user_id ) {
    
    ot_int  data_out    = 0;
    ot_int  data_in     = in_rec->payload_length;
    vlBLOCK file_block  = (vlBLOCK)((in_rec->dir_cmd >> 4) & 0x07);

    /// Only run if respond bit is set!
    if (respond) {
        while ((data_in > 0) && sub_qnotfull(respond, 6, out_q)) {
            vaddr   header;
            ot_bool allow_output = True;
            
            data_in--;  // one for the file id
            
            allow_output = (ot_bool)(vl_getheader_vaddr(&header, file_block, \
                                    q_readbyte(in_q), VL_ACCESS_R, NULL) == 0);
            if (allow_output) {
                q_writeshort_be(out_q, vworm_read(header + 4)); // id & mod
                q_writeshort(out_q, vworm_read(header + 0)); // length
                q_writeshort(out_q, vworm_read(header + 2)); // alloc
                data_out += 6;
            }
        }
        
        in_rec->bookmark = (void*)sub_testchunk(data_in);
    }
    
    return data_out; 
}




ot_int sub_filedata(ot_bool respond, alp_record* in_rec,
                        Queue* in_q, Queue* out_q, id_tmpl* user_id ) {
    
    vlFILE* fp;
    ot_u16  offset;
    ot_u16  span;
    ot_int  data_out    = 0;
    ot_int  data_in     = in_rec->payload_length;
    ot_bool inc_header  = (ot_bool)((in_rec->dir_cmd & 0x0F) == 0x0C);
    vlBLOCK file_block  = (vlBLOCK)((in_rec->dir_cmd >> 4) & 0x07);
    ot_u8   file_mod    = ((in_rec->dir_cmd & 0x02) ? VL_ACCESS_W : VL_ACCESS_R);
    
    while (data_in > 0) {
        vaddr   header;
        ot_u8   err_code;
        ot_u8   file_id;
        ot_u16  limit;
        
        in_rec->bookmark= in_q->getcursor;
        file_id         = q_readbyte(in_q);
        offset          = q_readshort(in_q);
        span            = q_readshort(in_q);
        limit           = offset + span;
        err_code        = vl_getheader_vaddr(&header, file_block, file_id, file_mod, user_id);
        file_mod        = ((file_mod & VL_ACCESS_W) != 0);
        fp              = NULL;
        
        // A. File error catcher Stage
        // (In this case, gotos make it more readable)
        
        /// Make sure file header was retrieved properly, or goto error
        if (err_code != 0) {
            goto sub_filedata_senderror;
        }
        
        /// Make sure file opens properly, or goto error
        fp = vl_open_file(header);
        if (fp == NULL) {
            err_code = 0xFF;
            goto sub_filedata_senderror;
        }
        
        /// Make sure offset is within file bounds, or goto error
        if (offset >= fp->alloc) {
            err_code = 0x07;
            goto sub_filedata_senderror;
        }
        
        if (limit > fp->alloc) {
        	limit 		= fp->alloc;
        	err_code	= 0x08;
        }

        // B. File Writing or Reading Stage
        // Write to file
        // 1. Process error on bad ALP parameters, but still do partial write
        // 2. offset, span are adjusted to convey leftover data
        // 3. miscellaneous write error occurs when vl_write fails
        if (file_mod) {
            for (; offset<limit; offset+=2, span-=2, data_in-=2) {
                if (in_q->getcursor >= in_q->back) {
                    goto sub_filedata_overrun;
                }
                err_code |= vl_write(fp, offset, q_readshort_be(in_q));
            }
        }
        
        // Read from File
        // 1. No error for bad read parameter, just fix the limit
        // 2. If inc_header param is set, include the file header in output
        // 3. Read out file data
        else {
            ot_u8 overhead;
            //limit       = (limit > fp->length) ? fp->length : limit;
            overhead    = 6;
            overhead   += (inc_header != 0) << 2; 
            
            if ((out_q->putcursor+overhead) >= out_q->back) {
                goto sub_filedata_overrun;
            }
            
            q_writeshort_be(out_q, vworm_read(header + 4)); // id & mod
            if (inc_header) {
                q_writeshort(out_q, vworm_read(header + 0));    // length
                q_writeshort(out_q, vworm_read(header + 2));    // alloc
                data_out += 4;
            }
            q_writeshort(out_q, offset);
            q_writeshort(out_q, span);
            data_out += 6;
            
            for (; offset<limit; offset+=2, span-=2, data_out+=2) {
                if ((out_q->putcursor+2) >= out_q->back) {
                    goto sub_filedata_overrun;
                }
                q_writeshort_be(out_q, vl_read(fp, offset));
            }
        }
        
        // C. Error Sending Stage
        sub_filedata_senderror:
        if (respond & ((err_code != 0) | file_mod)) {
            if ((out_q->putcursor+2) >= out_q->back) {
                goto sub_filedata_overrun;
            }
            q_writebyte(out_q, file_id);
            q_writebyte(out_q, err_code);
            q_markbyte(in_q, span);         // go past any leftover input data
            data_out += 2;
        }
        
        data_in -= 5;   // 5 bytes input header
        vl_close(fp);
    }
    
    
    // Total Completion:
    // Set bookmark to NULL, because the record was completely processed
    in_rec->bookmark = NULL;
    return data_out;
    
    
    // Partial or Non Completion:
    // Reconfigure last ALP operation, because it was not completely processed
    sub_filedata_overrun:
    vl_close(fp);
    {
        ot_u8* scratch;
        in_q->getcursor = (ot_u8*)in_rec->bookmark;
        scratch         = in_q->putcursor;
        in_q->putcursor = in_q->getcursor + 1;
        
        q_writeshort(in_q, offset);
        q_writeshort(in_q, span);
        
        in_q->putcursor = scratch;
    }
    
    return data_out;
}






ot_int sub_filedelete(  ot_bool respond, alp_record* in_rec,
                        Queue* in_q, Queue* out_q, id_tmpl* user_id ) {
                        
    ot_int  data_out    = 0;
    ot_int  data_in     = in_rec->payload_length;
    vlBLOCK file_block  = (vlBLOCK)((in_rec->dir_cmd >> 4) & 0x07);
    
    while ((data_in > 0) && sub_qnotfull(respond, 2, out_q)) {
        ot_u8   err_code;
        ot_u8   file_id;
        
        data_in--;
        file_id     = q_readbyte(in_q);
        err_code    = vl_delete(file_block, file_id, user_id);
    
        if (respond) {
            q_writebyte(out_q, file_id);
            q_writebyte(out_q, err_code);
            data_out += 2;
        }
    }    
    
    in_rec->bookmark = (void*)sub_testchunk(data_in);
    return data_out;     
}




ot_int sub_filecreate(  ot_bool respond, alp_record* in_rec,
                        Queue* in_q, Queue* out_q, id_tmpl* user_id  ) {
    
    ot_int  data_out    = 0;
    ot_int  data_in     = in_rec->payload_length;
    vlBLOCK file_block  = (vlBLOCK)((in_rec->dir_cmd >> 4) & 0x07);
    
    while ((data_in > 0) && sub_qnotfull(respond, 2, out_q)) {
        vlFILE*     fp = NULL;
        Twobytes    idmod;
        //ot_u16      length;
        ot_u16      alloc;
        ot_u8       err_code;
        
        data_in        -= 6;
        idmod.ushort    = q_readshort_be(in_q);
        /*length          =*/ q_readshort(in_q);
        alloc           = q_readshort(in_q);
        err_code        = vl_new(&fp, file_block, idmod.ubyte[0], idmod.ubyte[1], alloc, user_id);
        
        if (respond) {
            q_writebyte(out_q, idmod.ubyte[0]);
            q_writebyte(out_q, err_code);
            data_out += 2;
        }
        
        vl_close(fp);
    }    
    
    in_rec->bookmark = (void*)sub_testchunk(data_in);
    return data_out;      
}




/// Not currently supported, always returns "unrestorable" error
ot_int sub_filerestore( ot_bool respond, alp_record* in_rec,
                        Queue* in_q, Queue* out_q, id_tmpl* user_id ) {
    
    ot_int  data_out    = 0;
    ot_int  data_in     = in_rec->payload_length;
    //vlBLOCK file_block  = ((in_rec->dir_cmd >> 4) & 0x07);
    
    while ((data_in > 0) && sub_qnotfull(respond, 2, out_q)) {
        ot_u8   err_code    = 0x03;
        ot_u8   file_id     = q_readbyte(in_q);
        data_in            -= 1;
    
        if (respond) {
            q_writebyte(out_q, file_id);
            q_writebyte(out_q, err_code);
            data_out += 2;
        }
    }    
    
    in_rec->bookmark = (void*)sub_testchunk(data_in);
    return data_out;     
}



// File Error is transmitted as a response when the input directive does not
// meet specification.  This function will log received errors.
//ot_int sub_fileerror(ot_bool respond, ot_bool include_header, 
//                        vlBLOCK file_block, ot_u8 file_mod, ot_int data_in, 
//                        Queue* in_q, Queue* out_q, id_tmpl* user_id ) {
//    return otapi_log_msg(6, data_in, "ERR_VL", q_markbyte(in_q, data_in) );
//}



    
    
#endif
