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
  *
  */
/**
  * @file       /otlib/alp_filedata.c
  * @author     JP Norair
  * @version    V1.0
  * @date       20 July 2011
  * @brief      Application Layer protocol (ALP) for Filesystem Operations
  * @ingroup    ALP's
  *
  * Works with the file system (veelite) and the general form of ALPs.  
  * The ALPs may be "pure ALP" or translated from NDEF.
  *
  * <PRE>
  * 0             8            16       24        32
  * +------------+-------------+--------+---------+
  * | ALP Flags  | Payload Len | ALP ID | ALP CMD |
  * +------------+-------------+--------+---------+
  * |  bitfield  |      N      |   X    |    Y    |
  * +------------+-------------+--------+---------+
  * Universal ALP header (for comparison): flags are always --z10000.  The 
  * NDEF chunk flag (z) is ignored, as the values of MB and ME are sufficent to 
  * implicitly determine the value of the chunk flag.
  *
  * ALP ID Field
  * b7-0:   File ALP ID     0x01 
  *
  * ALP Command Field:
  * b7:     Respond Bit     0 don't respond
  *                         1 Respond with directive return template
  *
  * b6-4:   File Block      001 GFB
  *                         010 ISSB
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
  *</PRE>
  ******************************************************************************
  */


#include "alp.h"

#if (   (OT_FEATURE(ALP) == ENABLED) && \
        (OT_FEATURE(SERVER) == ENABLED) && \
        (OT_FEATURE(VEELITE) == ENABLED)    )

#include "auth.h"
#include "veelite.h"


// Processing subroutines
typedef ot_int (*sub_file)(alp_tmpl*, id_tmpl*, ot_bool);


ot_int sub_return(alp_tmpl* alp, id_tmpl* user_id, ot_bool respond );

ot_int sub_fileperms(alp_tmpl* alp, id_tmpl* user_id, ot_bool respond );
                        
ot_int sub_fileheaders(alp_tmpl* alp, id_tmpl* user_id, ot_bool respond );
                        
ot_int sub_filedata(alp_tmpl* alp, id_tmpl* user_id, ot_bool respond );
                        
ot_int sub_filedelete(alp_tmpl* alp, id_tmpl* user_id, ot_bool respond );
                        
ot_int sub_filecreate(alp_tmpl* alp, id_tmpl* user_id, ot_bool respond );  
                                  
ot_int sub_filerestore(alp_tmpl* alp, id_tmpl* user_id, ot_bool respond );
                        
//ot_int sub_fileerror(ot_bool respond, alp_tmpl* alp, id_tmpl* user_id );








// Callable processing function
ot_bool alp_proc_filedata(alp_tmpl* alp, id_tmpl* user_id) {
    //sub_file cmd;
    static const sub_file cmd_fn[] = {
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
    ot_bool respond = (ot_bool)(alp->inrec.cmd & 0x80);
    
    // Return value is the number of bytes of output the command has produced
    data_out = cmd_fn[alp->inrec.cmd & 0x0F](alp, user_id, respond);
    
    if (respond) {        
        //Transform input cmd to error or data return variant for response
        // - for write and control funcs, error is the only type of response
        // - for read, data return is the response
        // - 02 is the write-cmd mask, 03 is the return-cmd mask, 0F is the error cmd
        alp->outrec.cmd    &= ~0x80;
        alp->outrec.cmd    |= (alp->inrec.cmd & 0x02) ? 0x0F : 0x01;
    }
    
    alp->outrec.plength = data_out;
    
    return (ot_bool)(data_out);
}





// Return functions are not handled by the server (ignore)
ot_int sub_return(alp_tmpl* alp, id_tmpl* user_id, ot_bool respond) {
    return 0;
}


ot_bool sub_testchunk(ot_int data_in) {
    return (ot_bool)(data_in > 0);
}



/// This is a form of overwrite protection
ot_bool sub_qnotfull(ot_bool write, ot_u8 write_size, Queue* q) {
    return (ot_bool)(((q->putcursor+write_size) < q->back) || (write == False));
}




ot_int sub_fileperms( alp_tmpl* alp, id_tmpl* user_id, ot_bool respond ) {
    ot_int  data_out    = 0;
    ot_int  data_in     = alp->inrec.plength;
    vlBLOCK file_block  = (vlBLOCK)((alp->inrec.cmd >> 4) & 0x07);
    ot_u8   file_mod    = ((alp->inrec.cmd & 0x02) ? VL_ACCESS_W : VL_ACCESS_R);

    /// Loop through all the listed file ids and process permissions.
    while ((data_in > 0) && sub_qnotfull(respond, 2, alp->outq)) {
        ot_u8   file_id         = q_readbyte(alp->inq);
        ot_bool allow_write     = respond;
        vaddr   header;
        
        data_in--;  // one for the file id
        
        if (file_mod == VL_ACCESS_W ) {
            /// run the chmod and return the error code (0 is no error)
            data_in--;  // two for the new mod
            file_mod = vl_chmod(file_block, file_id, q_readbyte(alp->inq), user_id);
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
            q_writebyte(alp->outq, file_id);
            q_writebyte(alp->outq, file_mod);
            data_out += 2;
        }
    }
    
    /// return number of bytes put onto the output (always x2)
    alp->inrec.bookmark = (void*)sub_testchunk(data_in);
    return data_out;
}




ot_int sub_fileheaders( alp_tmpl* alp, id_tmpl* user_id, ot_bool respond ) {
    ot_int  data_out    = 0;
    ot_int  data_in     = alp->inrec.plength;
    vlBLOCK file_block  = (vlBLOCK)((alp->inrec.cmd >> 4) & 0x07);

    /// Only run if respond bit is set!
    if (respond) {
        while ((data_in > 0) && sub_qnotfull(respond, 6, alp->outq)) {
            vaddr   header;
            ot_bool allow_output = True;
            
            data_in--;  // one for the file id
            
            allow_output = (ot_bool)(vl_getheader_vaddr(&header, file_block, \
                                    q_readbyte(alp->inq), VL_ACCESS_R, NULL) == 0);
            if (allow_output) {
                q_writeshort_be(alp->outq, vworm_read(header + 4)); // id & mod
                q_writeshort(alp->outq, vworm_read(header + 0)); // length
                q_writeshort(alp->outq, vworm_read(header + 2)); // alloc
                data_out += 6;
            }
        }
        
        alp->inrec.bookmark = (void*)sub_testchunk(data_in);
    }
    
    return data_out; 
}




ot_int sub_filedata( alp_tmpl* alp, id_tmpl* user_id, ot_bool respond ) {
    vlFILE* fp;
    ot_u16  offset;
    ot_u16  span;
    ot_int  data_out    = 0;
    ot_int  data_in     = alp->inrec.plength;
    ot_bool inc_header  = (ot_bool)((alp->inrec.cmd & 0x0F) == 0x0C);
    vlBLOCK file_block  = (vlBLOCK)((alp->inrec.cmd >> 4) & 0x07);
    ot_u8   file_mod    = ((alp->inrec.cmd & 0x02) ? VL_ACCESS_W : VL_ACCESS_R);
    Queue*  inq         = alp->inq;
    Queue*  outq        = alp->outq;
    
    while (data_in > 0) {
        vaddr   header;
        ot_u8   err_code;
        ot_u8   file_id;
        ot_u16  limit;
        
        alp->inrec.bookmark     = inq->getcursor;
        alp->outrec.bookmark    = NULL;
        
        file_id     = q_readbyte(inq);
        offset      = q_readshort(inq);
        span        = q_readshort(inq);
        limit       = offset + span;
        err_code    = vl_getheader_vaddr(&header, file_block, file_id, file_mod, user_id);
        file_mod    = ((file_mod & VL_ACCESS_W) != 0);
        //fp          = NULL;
        
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
            limit       = fp->alloc;
            err_code    = 0x08;
        }

        // B. File Writing or Reading Stage
        // Write to file
        // 1. Process error on bad ALP parameters, but still do partial write
        // 2. offset, span are adjusted to convey leftover data
        // 3. miscellaneous write error occurs when vl_write fails
        if (file_mod) {
            for (; offset<limit; offset+=2, span-=2, data_in-=2) {
                if (inq->getcursor >= inq->back) {
                    goto sub_filedata_overrun;
                }
                err_code |= vl_write(fp, offset, q_readshort_be(inq));
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
            
            if ((outq->putcursor+overhead) >= outq->back) {
                goto sub_filedata_overrun;
            }
            
            q_writeshort_be(outq, vworm_read(header + 4)); // id & mod
            if (inc_header) {
                q_writeshort(outq, vworm_read(header + 0));    // length
                q_writeshort(outq, vworm_read(header + 2));    // alloc
                data_out += 4;
            }
            q_writeshort(outq, offset);
            q_writeshort(outq, span);
            data_out += 6;
            
            for (; offset<limit; offset+=2, span-=2, data_out+=2) {
                if ((outq->putcursor+2) >= outq->back) {
                    goto sub_filedata_overrun;
                }
                q_writeshort_be(outq, vl_read(fp, offset));
            }
        }
        
        // C. Error Sending Stage
        sub_filedata_senderror:
        if (respond && (err_code | file_mod)) {
            if ((outq->putcursor+2) >= outq->back) {
                goto sub_filedata_overrun;
            }
            q_writebyte(outq, file_id);
            q_writebyte(outq, err_code);
            q_markbyte(inq, span);         // go past any leftover input data
            data_out += 2;
        }
        
        data_in -= 5;   // 5 bytes input header
        vl_close(fp);
    }
    
    
    // Total Completion:
    // Set bookmark to NULL, because the record was completely processed
    alp->inrec.bookmark = NULL;
    return data_out;
    
    
    // Partial or Non Completion:
    // Reconfigure last ALP operation, because it was not completely processed
    sub_filedata_overrun:
    vl_close(fp);
    {
        ot_u8* scratch;
        inq->getcursor  = (ot_u8*)alp->inrec.bookmark;
        scratch         = inq->getcursor + 1;
        *scratch++      = ((ot_u8*)&offset)[UPPER];
        *scratch++      = ((ot_u8*)&offset)[LOWER];
        *scratch++      = ((ot_u8*)&span)[UPPER];
        *scratch        = ((ot_u8*)&span)[LOWER];
    }
    
    return data_out;
}






ot_int sub_filedelete( alp_tmpl* alp, id_tmpl* user_id, ot_bool respond ) {          
    ot_int  data_out    = 0;
    ot_int  data_in     = alp->inrec.plength;
    vlBLOCK file_block  = (vlBLOCK)((alp->inrec.cmd >> 4) & 0x07);
    
    while ((data_in > 0) && sub_qnotfull(respond, 2, alp->outq)) {
        ot_u8   err_code;
        ot_u8   file_id;
        
        data_in--;
        file_id     = q_readbyte(alp->inq);
        err_code    = vl_delete(file_block, file_id, user_id);
    
        if (respond) {
            q_writebyte(alp->outq, file_id);
            q_writebyte(alp->outq, err_code);
            data_out += 2;
        }
    }    
    
    alp->inrec.bookmark = (void*)sub_testchunk(data_in);
    return data_out;     
}




ot_int sub_filecreate(alp_tmpl* alp, id_tmpl* user_id, ot_bool respond) {
    ot_int  data_out    = 0;
    ot_int  data_in     = alp->inrec.plength;
    vlBLOCK file_block  = (vlBLOCK)((alp->inrec.cmd >> 4) & 0x07);
    
    while ((data_in > 0) && sub_qnotfull(respond, 2, alp->outq)) {
        vlFILE*     fp = NULL;
        ot_u8       id;
        ot_u8       mod;
        ot_u16      alloc;
        ot_u8       err_code;
        
        data_in            -= 6;
        id                  = *alp->inq->getcursor++;
        mod                 = *alp->inq->getcursor;
        alp->inq->getcursor+= 3;                        // cursor goes past mod+length (length ignored)
        alloc               = q_readshort(alp->inq);
        err_code            = vl_new(&fp, file_block, id, mod, alloc, user_id);
        
        if (respond) {
            q_writebyte(alp->outq, id);
            q_writebyte(alp->outq, err_code);
            data_out += 2;
        }
        
        vl_close(fp);
    }
    
    alp->inrec.bookmark = (void*)sub_testchunk(data_in);
    return data_out;      
}




/// Not currently supported, always returns "unrestorable" error
ot_int sub_filerestore(alp_tmpl* alp, id_tmpl* user_id, ot_bool respond ) {
    ot_int  data_out    = 0;
    ot_int  data_in     = alp->inrec.plength;
    //vlBLOCK file_block  = ((alp->inrec.cmd >> 4) & 0x07);
    
    while ((data_in > 0) && sub_qnotfull(respond, 2, alp->outq)) {
        ot_u8   err_code    = 0x03;
        ot_u8   file_id     = q_readbyte(alp->inq);
        data_in            -= 1;
    
        if (respond) {
            q_writebyte(alp->outq, file_id);
            q_writebyte(alp->outq, err_code);
            data_out += 2;
        }
    }    
    
    alp->inrec.bookmark = (void*)sub_testchunk(data_in);
    return data_out;     
}



// File Error is transmitted as a response when the input directive does not
// meet specification.  This function will log received errors.
//ot_int sub_fileerror(ot_bool respond, ot_bool include_header, 
//                        vlBLOCK file_block, ot_u8 file_mod, ot_int data_in, 
//                        Queue* alp->inq, Queue* alp->outq., id_tmpl* user_id ) {
//    return otapi_log_msg(6, data_in, "ERR_VL", q_markbyte(alp->inq, data_in) );
//}



    
    
#endif
