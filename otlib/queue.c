/* Copyright 2010-14 JP Norair
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
  * @version    R103
  * @date       20 Sept 2014
  * @brief      A module and ADT for buffering data packets.
  * @ingroup    ot_queue
  *
  * The queue module intends to abstract the user from the queue implementation,
  * but it does not intend to provide programmatic safeguards.  In other words,
  * the user must do his own boundary checking, or overrun may occur.
  *
  * R103: Some improvements and getting ready for multithreading
  * 
  * R102: Updates to multibyte read and write functions including __bswap...()
  *       functions for endian conversion as well as unaligned memory access.
  *
  * R101: Updates for new ot_queue definition, which no longer has explicit
  *       "length" attribute.  Use q_length() and q_span() instead.
  *
  ******************************************************************************
  */


#include <otstd.h>
#include <platform/config.h>
#include <otlib/delay.h>
#include <otlib/queue.h>
#include <otlib/memcpy.h>

#include <otsys/veelite.h>



/** @todo Add blocking conditions to all queue operations that move cursors. 
  *       Also make succeed/fail return values for all these operations.
  */



/** Queue "Object" functions
  * ========================
  */

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







/** Queue Info functions
  * ====================
  */

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






/** Queue Threading/Blocking functions
  * ==================================
  */

OT_INLINE ot_uint q_blocktime(ot_queue* q) {
    return q->options.ushort;
}

OT_INLINE void q_blockwrite(ot_queue* q, ot_uint blocktime) {
    q->options.ushort = blocktime;
}


void q_lock(ot_queue* q) {
    while (q->options.ushort != 0) {
        //ot_thandle thandle = otthread_this_thandle();
        //if (thandle != otthread_kernel_thandle()) {
        //    otthread_set_tmask( &(q->tmask), thandle );
        //    otthread_wait();
        //    otthread_clear_tmask( &(q->tmask), thandle );
        //}
        //else 
        {   delay_ti(q->options.ushort);
        }
    }
    q->options.ushort = 1;
}

void q_unlock(ot_queue* q) {
    // otthread_release_any( &(q->tmask) );
    q->options.ushort = 0;
}





#ifndef EXTF_q_empty
void q_empty(ot_queue* q) {
    q->options.ushort   = 0;
    q->back             = q->front + q->alloc;
    q->putcursor        = q->front;
    q->getcursor        = q->front;
}
#endif


#ifndef EXTF_q_rewind
void q_rewind(ot_queue* q) {
    ot_int dist = q->getcursor - q->front;
    
    if (dist > 0) {
        ot_u8* put      = q->putcursor;
        ot_u8* get      = q->getcursor;
        q->putcursor   -= dist;
        q->getcursor    = q->front;
        
        memcpy(q->front, get, put-get);
    }
}
#endif


#ifndef EXTF_q_start
ot_u8* q_start(ot_queue* q, ot_uint offset, ot_u16 options) {
    q_empty(q);

    if (offset >= q->alloc)
        return NULL;

    q->options.ushort  = options;
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
}
#endif



#ifndef EXTF_q_writeshort
void q_writeshort(ot_queue* q, ot_uint short_in) {
#   if defined(__UNALIGNED_ACCESS__)
        *(ot_u16*)q->putcursor  = PLATFORM_ENDIAN16(short_in);
        q->putcursor           += 2;
#   else
        ot_u8* data     = (ot_u8*)&short_in;
        *q->putcursor++ = data[UPPER];
        *q->putcursor++ = data[LOWER];
#   endif
}
#endif


#ifndef EXTF_q_writeshort_be
void q_writeshort_be(ot_queue* q, ot_uint short_in) {
#   if defined(__BIG_ENDIAN__)
        q_writeshort(q, short_in);
#   elif defined(__UNALIGNED_ACCESS__)
        *(ot_u16*)q->putcursor = short_in;
        q->putcursor          += 4;
#   else
        ot_u8* data     = (ot_u8*)&short_in;
        *q->putcursor++ = data[0];
        *q->putcursor++ = data[1];
#   endif
}
#endif



#ifndef EXTF_q_writelong
void q_writelong(ot_queue* q, ot_ulong long_in) {
#   if defined(__UNALIGNED_ACCESS__)
        *(ot_u32*)q->putcursor = PLATFORM_ENDIAN32(long_in);
        q->putcursor          += 4;
#   else
        ot_u8* data;
        data = (ot_u8*)&long_in;
        *q->putcursor++ = data[B3];
        *q->putcursor++ = data[B2];
        *q->putcursor++ = data[B1];
        *q->putcursor++ = data[B0];
#   endif
}
#endif


#ifndef EXTF_q_readbyte
ot_u8 q_readbyte(ot_queue* q) {
    return *q->getcursor++;
}
#endif



#ifndef EXTF_q_readshort
ot_u16 q_readshort(ot_queue* q) {
#   if defined(__UNALIGNED_ACCESS__)
        ot_u16 data     = *(ot_u16*)q->getcursor;
        q->getcursor   += 2;
        return PLATFORM_ENDIAN16(data);
#   else
        ot_uni16 data;
        data.ubyte[UPPER] = *q->getcursor++;
        data.ubyte[LOWER] = *q->getcursor++;
        return data.ushort;
#   endif
}
#endif


#ifndef EXTF_q_readshort_be
ot_u16 q_readshort_be(ot_queue* q) {
#   if defined(__BIG_ENDIAN__)
        return q_readshort(q);
#   elif defined(__UNALIGNED_ACCESS__)
        ot_u16 data     = *(ot_u16*)q->getcursor;
        q->getcursor   += 2
        return data;
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
#   if defined(__UNALIGNED_ACCESS__)
        ot_u32 data     = *(ot_u32*)q->getcursor;
        q->getcursor   += 4;
        return PLATFORM_ENDIAN32(data);
#   else
        ot_uni32 data;
        data.ubyte[B3]  = *q->getcursor++;
        data.ubyte[B2]  = *q->getcursor++;
        data.ubyte[B1]  = *q->getcursor++;
        data.ubyte[B0]  = *q->getcursor++;
        return data.ulong;
#   endif
}
#endif


#ifndef EXTF_q_writestring
void q_writestring(ot_queue* q, ot_u8* string, ot_int length) {
    ot_int limit;
    
    limit = (q->back - q->putcursor);
    if (length > limit) {
        length = limit;
    } 
    if (length > 0) {
        memcpy(q->putcursor, string, length);
        q->putcursor   += length;
    }
}
#endif


#ifndef EXTF_q_readstring
void q_readstring(ot_queue* q, ot_u8* string, ot_int length) {
    ot_int limit;
    
    limit = (q->back - q->getcursor);
    if (length > limit) {
        length = limit;
    }
    if (length > 0) {
        memcpy(string, q->getcursor, length);
        q->getcursor += length;
    }
}
#endif



#if (defined(__STDC__) || defined (__POSIX__))
#include <stdio.h>

void q_print(ot_queue* q) {
    int length;
    int i;
    int row;
    length = q_length(q);

    printf("ot_queue Length/Alloc: %d/%d\n", length, q->alloc);
    printf("ot_queue Getcursor:    %d\n", (int)(q->getcursor-q->front));
    printf("ot_queue Putcursor:    %d\n", (int)(q->putcursor-q->front));

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

