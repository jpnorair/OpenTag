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
  * @file       /otlib/alp_tmpl.c
  * @author     JP Norair
  * @version    R101
  * @date       23 Mar 2014
  * @brief      ALP to template conversion functions
  * @ingroup    ALP
  *
  * Functions to convert a stream to templates (OTAPI_tmpl.h) and templates to
  * a stream.
  * 
  ******************************************************************************
  */


#include <otlib/alp.h>
#if ( (OT_FEATURE(ALP) == ENABLED) )

#include <m2/tmpl.h>



#define _PTR_TEST(X)    (X != NULL)




OT_WEAK void alp_breakdown_u8(ot_queue* in_q, void* data_type) {
    *(ot_u8*)data_type = q_readbyte(in_q); 
}

OT_WEAK void alp_breakdown_u16(ot_queue* in_q, void* data_type) {
    *(ot_u16*)data_type = q_readshort(in_q);
}

OT_WEAK void alp_breakdown_u32(ot_queue* in_q, void* data_type) {
    *(ot_u32*)data_type = q_readlong(in_q);
}

OT_WEAK void alp_stream_u8(ot_queue* out_q, void* data_type) {
    q_writebyte(out_q, *(ot_u8*)data_type);
}

OT_WEAK void alp_stream_u16(ot_queue* out_q, void* data_type) {
    q_writebyte(out_q, *(ot_u16*)data_type);
}

OT_WEAK void alp_stream_u32(ot_queue* out_q, void* data_type) {
    q_writebyte(out_q, *(ot_u32*)data_type);
}





OT_WEAK void alp_breakdown_queue(ot_queue* in_q, void* data_type) {
    ot_u16 queue_length;
    ot_u8* queue_front;
    queue_length                    = q_readshort(in_q);
    ((ot_queue*)data_type)->alloc      = queue_length;
    ((ot_queue*)data_type)->options    = in_q->options;
    queue_front                     = q_markbyte(in_q, queue_length);
    ((ot_queue*)data_type)->front      = queue_front;
    ((ot_queue*)data_type)->back       = queue_front+queue_length;
    ((ot_queue*)data_type)->getcursor  = queue_front;
    ((ot_queue*)data_type)->putcursor  = queue_front;
}

OT_WEAK void alp_stream_queue(ot_queue* out_q, void* data_type) { 
    if _PTR_TEST(data_type) {
    	ot_int length;
    	length = q_length((ot_queue*)data_type);
        q_writeshort(out_q, ((ot_queue*)data_type)->alloc);
        q_writeshort(out_q, ((ot_queue*)data_type)->options.ushort);
        q_writeshort(out_q, length);
        q_writestring(out_q, ((ot_queue*)data_type)->front, length);     
    }
}




OT_WEAK void alp_breakdown_session_tmpl(ot_queue* in_q, void* data_type) {
    q_readstring(in_q, (ot_u8*)data_type, 6);
    //((session_tmpl*)data_type)->reserved    = q_readbyte(in_q);
    //((session_tmpl*)data_type)->channel     = q_readbyte(in_q);
    //((session_tmpl*)data_type)->subnet      = q_readbyte(in_q);
    //((session_tmpl*)data_type)->subnetmask  = q_readbyte(in_q);
    //((session_tmpl*)data_type)->flags       = q_readbyte(in_q);
    //((session_tmpl*)data_type)->flagmask    = q_readbyte(in_q);
}

OT_WEAK void alp_stream_session_tmpl(ot_queue* out_q, void* data_type) {
    if _PTR_TEST(data_type) 
        q_writestring(out_q, (ot_u8*)data_type, 6);
}




OT_WEAK void alp_breakdown_advert_tmpl(ot_queue* in_q, void* data_type) {
    q_readstring(in_q, (ot_u8*)data_type, 4);
    ((advert_tmpl*)data_type)->duration     = q_readshort(in_q);
}

OT_WEAK void alp_stream_advert_tmpl(ot_queue* out_q, void* data_type) { 
    if _PTR_TEST(data_type) {
        q_writestring(out_q, (ot_u8*)data_type, 4);
        q_writeshort(out_q, ((advert_tmpl*)data_type)->duration);
    }
}




OT_WEAK void alp_breakdown_command_tmpl(ot_queue* in_q, void* data_type) {
    q_readstring(in_q, (ot_u8*)data_type, 3);
    //((command_tmpl*)data_type)->type        = q_readbyte(in_q);
    //((command_tmpl*)data_type)->opcode      = q_readbyte(in_q);
    //((command_tmpl*)data_type)->extension   = q_readbyte(in_q);
}

OT_WEAK void alp_stream_command_tmpl(ot_queue* out_q, void* data_type) {
    if _PTR_TEST(data_type)
        q_writestring(out_q, (ot_u8*)data_type, 3);
}





OT_WEAK void alp_breakdown_routing_tmpl(ot_queue* in_q, void* data_type) {
    ot_u8 id_length;
    ot_u8 code_mask;
    ((routing_tmpl*)data_type)->hop_code    = q_readbyte(in_q);
    ((routing_tmpl*)data_type)->hop_ext     = q_readbyte(in_q);
    
    alp_breakdown_id_tmpl(in_q, (void*)&((routing_tmpl*)data_type)->dlog);
    
    if (((routing_tmpl*)data_type)->hop_code > 1) {
        code_mask                               = (((routing_tmpl*)data_type)->hop_ext != 0) << 7;
        id_length                               = q_readbyte(in_q);
        ((routing_tmpl*)data_type)->orig.length = id_length;
        code_mask                              |= (id_length != 0) << 6;
        code_mask                              |= (id_length == 2) << 4;
        ((routing_tmpl*)data_type)->orig.value  = q_markbyte(in_q, id_length);
        id_length                               = q_readbyte(in_q);
        ((routing_tmpl*)data_type)->dest.length = id_length;
        code_mask                              |= (id_length != 0) << 5;
        code_mask                              |= (id_length == 2) << 4;
        ((routing_tmpl*)data_type)->dest.value  = q_markbyte(in_q, id_length);
        ((routing_tmpl*)data_type)->hop_code   |= code_mask;
    }
}

OT_WEAK void alp_stream_routing_tmpl(ot_queue* out_q, void* data_type) { 
    if _PTR_TEST(data_type) {
        q_writebyte(out_q, ((routing_tmpl*)data_type)->hop_code );
        q_writebyte(out_q, ((routing_tmpl*)data_type)->hop_ext );
        
        alp_stream_id_tmpl(out_q, (void*)&((routing_tmpl*)data_type)->dlog);
        
        if ( ((routing_tmpl*)data_type)->hop_code & (1<<6) )
            alp_stream_id_tmpl(out_q, (void*)&((routing_tmpl*)data_type)->orig);
        
        if ( ((routing_tmpl*)data_type)->hop_code & (1<<5) )
            alp_stream_id_tmpl(out_q, (void*)&((routing_tmpl*)data_type)->dest);
    }
}




OT_WEAK void alp_breakdown_id_tmpl(ot_queue* in_q, void* data_type) {
    ot_int id_length;
    id_length                       = q_readbyte(in_q);
    ((id_tmpl*)data_type)->length   = id_length;
    ((id_tmpl*)data_type)->value    = (id_length==0) ? NULL : q_markbyte(in_q, id_length);
}

OT_WEAK void alp_stream_id_tmpl(ot_queue* out_q, void* data_type) { 
    if _PTR_TEST(data_type) {
        q_writebyte(out_q, ((id_tmpl*)data_type)->length);
        q_writestring(out_q, ((id_tmpl*)data_type)->value, ((id_tmpl*)data_type)->length);
    }
}




OT_WEAK void alp_breakdown_dialog_tmpl(ot_queue* in_q, void* data_type) {
    ((dialog_tmpl*)data_type)->timeout = q_readbyte(in_q);
    
    if (((dialog_tmpl*)data_type)->timeout & 0x80) {
        ((dialog_tmpl*)data_type)->channels = q_readbyte(in_q);
        ((dialog_tmpl*)data_type)->chanlist = \
            q_markbyte(in_q, ((dialog_tmpl*)data_type)->channels);
    }
}

OT_WEAK void alp_stream_dialog_tmpl(ot_queue* out_q, void* data_type) { 
    if _PTR_TEST(data_type) {
        q_writeshort(out_q, ((dialog_tmpl*)data_type)->timeout);
        q_writeshort(out_q, ((dialog_tmpl*)data_type)->channels);
        q_writestring(out_q, ((dialog_tmpl*)data_type)->chanlist, ((dialog_tmpl*)data_type)->channels); 
    }
}





OT_WEAK void alp_breakdown_query_tmpl(ot_queue* in_q, void* data_type) {
    ot_u8   query_length;
    ot_u8   query_code;
    ot_u8*  query_mask;
    
    query_code      = q_readbyte(in_q);
    query_length    = q_readbyte(in_q);
    query_mask      = NULL;
    
    if (query_code & 0x80) {
        query_mask  = q_markbyte(in_q, query_length);
    }
    
    ((query_tmpl*)data_type)->code      = query_code;
    ((query_tmpl*)data_type)->length    = query_length;
    ((query_tmpl*)data_type)->mask      = query_mask;
    ((query_tmpl*)data_type)->value     = q_markbyte(in_q, query_length);
}

OT_WEAK void alp_stream_query_tmpl(ot_queue* out_q, void* data_type) { 
    if _PTR_TEST(data_type) {
        q_writebyte(out_q, ((query_tmpl*)data_type)->code);
        q_writebyte(out_q, ((query_tmpl*)data_type)->length);
        
        if (((query_tmpl*)data_type)->code & 0x80)
            q_writestring(out_q, ((query_tmpl*)data_type)->mask, ((query_tmpl*)data_type)->length);
            
        q_writestring(out_q, ((query_tmpl*)data_type)->value, ((query_tmpl*)data_type)->length);
    }
}





OT_WEAK void alp_breakdown_ack_tmpl(ot_queue* in_q, void* data_type) {
    ot_int ack_id_count;
    ot_int ack_id_length;
    ack_id_count                    = q_readbyte(in_q);
    ack_id_length                   = q_readbyte(in_q);
    ((ack_tmpl*)data_type)->count   = (ot_u8)ack_id_count;
    ((ack_tmpl*)data_type)->length  = (ot_u8)ack_id_length;
    ((ack_tmpl*)data_type)->list    = q_markbyte(in_q, ack_id_count*ack_id_length);
}

OT_WEAK void alp_stream_ack_tmpl(ot_queue* out_q, void* data_type) { 
    if _PTR_TEST(data_type) {
        q_writebyte(out_q, ((ack_tmpl*)data_type)->count);
        q_writebyte(out_q, ((ack_tmpl*)data_type)->length);
        q_writestring(out_q, ((ack_tmpl*)data_type)->list, ((ack_tmpl*)data_type)->count * ((ack_tmpl*)data_type)->length);
    }
}





OT_WEAK void alp_breakdown_error_tmpl(ot_queue* in_q, void* data_type) {
    ((error_tmpl*)data_type)->code      = q_readbyte(in_q);
    ((error_tmpl*)data_type)->subcode   = q_readbyte(in_q);
    ((error_tmpl*)data_type)->data      = in_q->getcursor;  ///@todo build a routine for code:subcode --> data length
}

OT_WEAK void alp_stream_error_tmpl(ot_queue* out_q, void* data_type) { 
    if _PTR_TEST(data_type) {
        q_writebyte(out_q, ((error_tmpl*)data_type)->code);
        q_writebyte(out_q, ((error_tmpl*)data_type)->subcode);
        ///@todo build a routine for code:subcode --> data length
    }
}





OT_WEAK void alp_breakdown_udp_tmpl(ot_queue* in_q, void* data_type) {
    ot_int udp_data_length;
    udp_data_length                       = q_readshort(in_q);
    ((udp_tmpl*)data_type)->data_length   = udp_data_length;
    ((udp_tmpl*)data_type)->dst_port      = q_readbyte(in_q);
    ((udp_tmpl*)data_type)->src_port      = q_readbyte(in_q);
    ((udp_tmpl*)data_type)->data          = q_markbyte(in_q, udp_data_length);
}

OT_WEAK void alp_stream_udp_tmpl(ot_queue* out_q, void* data_type) { 
    if _PTR_TEST(data_type) {
        q_writeshort(out_q, ((udp_tmpl*)data_type)->data_length);
        q_writebyte(out_q, ((udp_tmpl*)data_type)->dst_port);
        q_writebyte(out_q, ((udp_tmpl*)data_type)->src_port);
        q_writestring(out_q, ((udp_tmpl*)data_type)->data, ((udp_tmpl*)data_type)->data_length);
    }
}





OT_WEAK void alp_breakdown_isfcomp_tmpl(ot_queue* in_q, void* data_type) {
    ((isfcomp_tmpl*)data_type)->is_series   = q_readbyte(in_q);
    ((isfcomp_tmpl*)data_type)->isf_id      = q_readbyte(in_q);
    ((isfcomp_tmpl*)data_type)->offset      = q_readshort(in_q);
}

OT_WEAK void alp_stream_isfcomp_tmpl(ot_queue* out_q, void* data_type) { 
    if _PTR_TEST(data_type) {
        q_writebyte(out_q, ((isfcomp_tmpl*)data_type)->is_series);
        q_writebyte(out_q, ((isfcomp_tmpl*)data_type)->isf_id);
        q_writeshort(out_q, ((isfcomp_tmpl*)data_type)->offset);
    }
}





OT_WEAK void alp_breakdown_isfcall_tmpl(ot_queue* in_q, void* data_type) {
    alp_breakdown_isfcomp_tmpl(in_q, data_type);
    ((isfcall_tmpl*)data_type)->max_return  = q_readshort(in_q);
}

OT_WEAK void alp_stream_isfcall_tmpl(ot_queue* out_q, void* data_type) { 
    if _PTR_TEST(data_type) {
        alp_breakdown_isfcomp_tmpl(out_q, data_type);
        q_writeshort(out_q, ((isfcall_tmpl*)data_type)->max_return);
    }
}









#endif


