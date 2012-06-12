/* Copyright 2010-2011 JP Norair
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
  * @file       OTlib/queue.c
  * @author     JP Norair
  * @version    V1.0
  * @date       15 July 2011
  * @brief      A module and ADT for buffering data packets.
  * @ingroup    Queue
  *
  * The queue module intends to abstract the user from the queue implementation,
  * but it does not intend to provide programmatic safeguards.  In other words,
  * the user must do his own boundary checking, or overrun may occur.
  ******************************************************************************
  */


#include "OT_config.h"
#include "OT_platform.h"
#include "queue.h"

#include "veelite.h"



#ifndef EXTF_q_init
void q_init(Queue* q, ot_u8* buffer, ot_u16 alloc) {
    q->alloc    = alloc;
    q->front    = buffer;
    q->back     = buffer+alloc;
    q_empty(q);
}
#endif


#ifndef EXTF_q_rebase
void q_rebase(Queue *q, ot_u8* buffer) {
    q->front        = buffer;
    q->getcursor    = buffer;
    q->putcursor    = buffer;
}
#endif


#ifndef EXTF_q_copy
void q_copy(Queue* q1, Queue* q2) {
    platform_memcpy((ot_u8*)q1, (ot_u8*)q2, sizeof(Queue));
}
#endif


#ifndef EXTF_q_empty
void q_empty(Queue* q) {
    q->length           = 0;
    q->options.ushort   = 0;
    q->back             = q->front + q->alloc;
    q->putcursor        = q->front;
    q->getcursor        = q->front;
}
#endif


#ifndef EXTF_q_start
ot_u8* q_start(Queue* q, ot_uint offset, ot_u16 options) {  
    q_empty(q);

    if (offset >= q->alloc) 
        return NULL;  
    
    q->length          = offset;
    q->options.ushort  = options;
    q->putcursor      += offset;
    q->getcursor      += offset;
    return q->getcursor;
}
#endif


#ifndef EXTF_q_markbyte
ot_u8* q_markbyte(Queue* q, ot_int shift) {
    ot_u8* output;
    output          = q->getcursor;
    q->getcursor   += shift;
    return output;
}
#endif


#ifndef EXTF_q_writebyte
void q_writebyte(Queue* q, ot_u8 byte_in) {
    *(q->putcursor) = byte_in;
    q->putcursor++;
    q->length++;
}
#endif


#ifndef EXTF_q_writeshort
void q_writeshort(Queue* q, ot_uint short_in) {
    ot_u8* data;
    data = (ot_u8*)&short_in;

#   ifdef __BIG_ENDIAN__
        q->putcursor[0] = data[0];
        q->putcursor[1] = data[1];
#   else
        q->putcursor[0] = data[1];
        q->putcursor[1] = data[0];
#   endif
    
    q->putcursor  += 2;
    q->length     += 2;
}
#endif


#ifndef EXTF_q_writeshort_be
void q_writeshort_be(Queue* q, ot_uint short_in) {
#   ifdef __BIG_ENDIAN__
        q_writeshort(q, short_in);

#   else
        ot_u8* data;
        data = (ot_u8*)&short_in;
    
        q->putcursor[0] = data[0];
        q->putcursor[1] = data[1];
        
        q->putcursor  += 2;
        q->length     += 2;
#   endif    
}
#endif



#ifndef EXTF_q_writelong
void q_writelong(Queue* q, ot_ulong long_in) {
    ot_u8* data;
    data = (ot_u8*)&long_in;

#   ifdef __BIG_ENDIAN__
        q->putcursor[0] = data[0];
        q->putcursor[1] = data[1];
        q->putcursor[2] = data[2];
        q->putcursor[3] = data[3];
#   else
        q->putcursor[0] = data[3];
        q->putcursor[1] = data[2];
        q->putcursor[2] = data[1];
        q->putcursor[3] = data[0];
#   endif
    
    q->putcursor  += 4;
    q->length     += 4;
}
#endif


#ifndef EXTF_q_readbyte
ot_u8 q_readbyte(Queue* q) {
    return *(q->getcursor++);
}
#endif


#ifndef EXTF_q_readshort
ot_u16 q_readshort(Queue* q) {
    Twobytes data;

#   ifdef __BIG_ENDIAN__
        data.ubyte[0]   = q->getcursor[0];
        data.ubyte[1]   = q->getcursor[1];
#   else
        data.ubyte[1]   = q->getcursor[0];
        data.ubyte[0]   = q->getcursor[1];
#   endif
    
    q->getcursor  += 2;
    return data.ushort;
}
#endif


#ifndef EXTF_q_readshort_be
ot_u16 q_readshort_be(Queue* q) {
#   ifdef __BIG_ENDIAN__
        return q_readshort(q);
#   else
        Twobytes data;
        data.ubyte[0]   = q->getcursor[0];
        data.ubyte[1]   = q->getcursor[1];
        q->getcursor  += 2;
        return data.ushort;
#   endif
}
#endif


#ifndef EXTF_q_readlong
ot_u32 q_readlong(Queue* q)  {
    Fourbytes data;

#   ifdef __BIG_ENDIAN__
        data.ubyte[0]   = q->getcursor[0];
        data.ubyte[1]   = q->getcursor[1];
        data.ubyte[2]   = q->getcursor[2];
        data.ubyte[3]   = q->getcursor[3];
#   else
        data.ubyte[3]   = q->getcursor[0];
        data.ubyte[2]   = q->getcursor[1];
        data.ubyte[1]   = q->getcursor[2];
        data.ubyte[0]   = q->getcursor[3];
#   endif
    
    q->getcursor  += 4;
    return data.ulong;
}
#endif


#ifndef EXTF_q_writestring
void q_writestring(Queue* q, ot_u8* string, ot_int length) {
    platform_memcpy(q->putcursor, string, length);
    q->length      += length;
    q->putcursor   += length;
}
#endif


#ifndef EXTF_q_readstring
void q_readstring(Queue* q, ot_u8* string, ot_int length) {
    platform_memcpy(string, q->getcursor, length);
    q->getcursor += length;
}
#endif


