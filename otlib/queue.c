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
  * @file       /otlib/queue.c
  * @author     JP Norair
  * @version    R101
  * @date       18 Sept 2013
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
void q_init(ot_queue* q, ot_u8* buffer, ot_u16 alloc) {
    q->alloc    = alloc;
    q->front    = buffer;
    q->back     = buffer+alloc;
    q_empty(q);
}
#endif


#ifndef EXTF_q_rebase
void q_rebase(ot_queue* q, ot_u8* buffer) {
    q->front        = buffer;
    q->getcursor    = buffer;
    q->putcursor    = buffer;
    q->back         = buffer;
}
#endif


#ifndef EXTF_q_copy
void q_copy(ot_queue* q1, ot_queue* q2) {
    memcpy((ot_u8*)q1, (ot_u8*)q2, sizeof(ot_queue));
}
#endif


#ifndef EXTF_q_length
ot_int q_length(ot_queue* q) {
    return (q->putcursor - q->front);
}
#endif


#ifndef EXTF_q_span
ot_int q_span(ot_queue* q) {
    return (q->putcursor - q->getcursor);
}
#endif


#ifndef EXTF_q_space
ot_int q_space(ot_queue* q) {
    return (q->back - q->putcursor);
} 
#endif


#ifndef EXTF_q_empty
void q_empty(ot_queue* q) {
    //#q->length           = 0;
    q->options.ushort   = 0;
    q->back             = q->front + q->alloc;
    q->putcursor        = q->front;
    q->getcursor        = q->front;
}
#endif


#ifndef EXTF_q_start
ot_u8* q_start(ot_queue* q, ot_uint offset, ot_u16 options) {  
    q_empty(q);

    if (offset >= q->alloc) 
        return NULL;  
    
    q->options.ushort  = options;
    //#q->length          = offset;
    q->putcursor      += offset;
    q->getcursor      += offset;
    return q->getcursor;
}
#endif


#ifndef EXTF_q_markbyte
ot_u8* q_markbyte(ot_queue* q, ot_int shift) {
    ot_u8* output;
    output          = q->getcursor;
    q->getcursor   += shift;
    return output;
}
#endif


#ifndef EXTF_q_writebyte
void q_writebyte(ot_queue* q, ot_u8 byte_in) {
    *q->putcursor++ = byte_in;
    //#q->length++;
}
#endif


#ifndef EXTF_q_writeshort
void q_writeshort(ot_queue* q, ot_uint short_in) {
    ot_u8* data;
    data = (ot_u8*)&short_in;

#   ifdef __BIG_ENDIAN__
        *q->putcursor++ = data[0];
        *q->putcursor++ = data[1];
#   else
        *q->putcursor++ = data[1];
        *q->putcursor++ = data[0];
#   endif
    
    //#q->length     += 2;
}
#endif


#ifndef EXTF_q_writeshort_be
void q_writeshort_be(ot_queue* q, ot_uint short_in) {
#   ifdef __BIG_ENDIAN__
        q_writeshort(q, short_in);

#   else
        ot_u8* data;
        data            = (ot_u8*)&short_in;
        *q->putcursor++ = data[0];
        *q->putcursor++ = data[1];
        
        //#q->length     += 2;
#   endif    
}
#endif



#ifndef EXTF_q_writelong
void q_writelong(ot_queue* q, ot_ulong long_in) {
    ot_u8* data;
    data = (ot_u8*)&long_in;

#   ifdef __BIG_ENDIAN__
        *q->putcursor++ = data[0];
        *q->putcursor++ = data[1];
        *q->putcursor++ = data[2];
        *q->putcursor++ = data[3];
#   else
        *q->putcursor++ = data[3];
        *q->putcursor++ = data[2];
        *q->putcursor++ = data[1];
        *q->putcursor++ = data[0];
#   endif
    
    //q->putcursor  += 4;
    //#q->length     += 4;
}
#endif


#ifndef EXTF_q_readbyte
ot_u8 q_readbyte(ot_queue* q) {
    return *q->getcursor++;
}
#endif


#ifndef EXTF_q_readshort
ot_u16 q_readshort(ot_queue* q) {
    ot_uni16 data;

#   ifdef __BIG_ENDIAN__
        data.ubyte[0]   = *q->getcursor++;
        data.ubyte[1]   = *q->getcursor++;
#   else
        data.ubyte[1]   = *q->getcursor++;
        data.ubyte[0]   = *q->getcursor++;
#   endif
    
    //q->getcursor  += 2;
    return data.ushort;
}
#endif


#ifndef EXTF_q_readshort_be
ot_u16 q_readshort_be(ot_queue* q) {
#   ifdef __BIG_ENDIAN__
        return q_readshort(q);
#   else
        ot_uni16 data;
        data.ubyte[0]   = *q->getcursor++;
        data.ubyte[1]   = *q->getcursor++;

        return data.ushort;
#   endif
}
#endif


#ifndef EXTF_q_readlong
ot_u32 q_readlong(ot_queue* q)  {
    ot_uni32 data;

#   ifdef __BIG_ENDIAN__
        data.ubyte[0]   = *q->getcursor++;
        data.ubyte[1]   = *q->getcursor++;
        data.ubyte[2]   = *q->getcursor++;
        data.ubyte[3]   = *q->getcursor++;
#   else
        data.ubyte[3]   = *q->getcursor++;
        data.ubyte[2]   = *q->getcursor++;
        data.ubyte[1]   = *q->getcursor++;
        data.ubyte[0]   = *q->getcursor++;
#   endif
    
    return data.ulong;
}
#endif


#ifndef EXTF_q_writestring
void q_writestring(ot_queue* q, ot_u8* string, ot_int length) {
    memcpy(q->putcursor, string, length);
    //#q->length      += length;
    q->putcursor   += length;
}
#endif


#ifndef EXTF_q_readstring
void q_readstring(ot_queue* q, ot_u8* string, ot_int length) {
    memcpy(string, q->getcursor, length);
    q->getcursor += length;
}
#endif



#if (defined(__STDC__) || defined (__POSIX__))
#include <stdio.h>

void q_print(ot_queue* q) {
    int length;
    int i;
    int row;
    length = q_length(q);
    
    printf("Queue Length/Alloc: %d/%d\n", length, q->alloc);
    printf("Queue Getcursor:    %d\n", (int)(q->getcursor-q->front));
    printf("Queue Putcursor:    %d\n", (int)(q->putcursor-q->front));
    
    for (i=0, row=0; length>0; ) {
        length -= 8;
        row    += (length>0) ? 8 : 8+length;
        printf("%04X: ", i);
        for (; i<row; i++) {
            printf("%02X ", q->front[i]);
        }
        printf("\n");
    }
    printf("\n");
}

#endif

