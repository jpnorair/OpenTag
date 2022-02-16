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
void q_init(ot_queue* q, void* buffer, ot_u16 alloc) {
    q->alloc    = alloc;
    q->front    = buffer;
    q->back     = buffer+alloc;
    q_empty(q);
}
#endif


#ifndef EXTF_q_rebase
void q_rebase(ot_queue* q, void* buffer) {
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




/** Queue "Intrinsics"
  * ==================
  */

#ifndef EXTF_q_intrinsics
OT_INLINE ot_u8 q_getcursor_val(ot_queue* q, ot_int offset) {
    return q->getcursor[offset];
}

OT_INLINE void q_getcursor_insert(ot_queue* q, ot_int offset, ot_u8 val) {
    q->getcursor[offset] = val;
}

OT_INLINE ot_u8 q_putcursor_val(ot_queue* q, ot_int offset) {
    return q->putcursor[offset];
}

OT_INLINE void q_putcursor_insert(ot_queue* q, ot_int offset, ot_u8 val) {
    q->putcursor[offset] = val;
}

OT_INLINE ot_qcur q_offset(ot_queue* q, ot_int offset) {
    return (ot_qcur)&((ot_u8*)q->front)[offset];
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


#ifndef EXTF_q_writespace
ot_int q_writespace(ot_queue* q) {
    return (q->back - q->putcursor);
}
#endif


#ifndef EXTF_q_readspace
ot_int q_readspace(ot_queue* q) {
    return (q->back - q->getcursor);
}
#endif





/** Queue Threading/Blocking functions
  * ==================================
  */

#ifndef EXTF_q_blocktime
OT_INLINE ot_uint q_blocktime(ot_queue* q) {
    return q->options.ushort;
}
#endif

#ifndef EXTF_q_blockwrite
OT_INLINE void q_blockwrite(ot_queue* q, ot_uint blocktime) {
    q->options.ushort = blocktime;
}
#endif

#ifndef EXTF_q_lock
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
#endif

#ifndef EXTF_q_unlock
void q_unlock(ot_queue* q) {
    // otthread_release_any( &(q->tmask) );
    q->options.ushort = 0;
}
#endif




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
void* q_start(ot_queue* q, ot_uint offset, ot_u16 options) {
    q_empty(q);

    if (offset >= q->alloc)
        return NULL;

    q->options.ushort  = options;
    q->putcursor      += offset;
    q->getcursor      += offset;
    
    return (void*)q->getcursor;
}
#endif


#ifndef EXTF_q_markbyte
ot_qcur q_markbyte(ot_queue* q, ot_int shift) {
    ot_qcur output;
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
        *q->putcursor++ = data[_B3];
        *q->putcursor++ = data[_B2];
        *q->putcursor++ = data[_B1];
        *q->putcursor++ = data[_B0];
#   endif
}
#endif


#ifndef EXTF_q_writelong_be
void q_writelong_be(ot_queue* q, ot_ulong long_in) {
#   if defined(__UNALIGNED_ACCESS__)
        *(ot_u32*)q->putcursor = long_in;
        q->putcursor          += 4;
#   else
        ot_u8* data;
        data = (ot_u8*)&long_in;
        *q->putcursor++ = data[_B0];
        *q->putcursor++ = data[_B1];
        *q->putcursor++ = data[_B2];
        *q->putcursor++ = data[_B3];
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
        data.ubyte[_B3]  = *q->getcursor++;
        data.ubyte[_B2]  = *q->getcursor++;
        data.ubyte[_B1]  = *q->getcursor++;
        data.ubyte[_B0]  = *q->getcursor++;
        return data.ulong;
#   endif
}
#endif

#ifndef EXTF_q_readlong_be
ot_u32 q_readlong_be(ot_queue* q)  {
#   if defined(__UNALIGNED_ACCESS__)
        ot_u32 data     = *(ot_u32*)q->getcursor;
        q->getcursor   += 4;
        return data;
#   else
        ot_uni32 data;
        data.ubyte[_B0]  = *q->getcursor++;
        data.ubyte[_B1]  = *q->getcursor++;
        data.ubyte[_B2]  = *q->getcursor++;
        data.ubyte[_B3]  = *q->getcursor++;
        return data.ulong;
#   endif
}
#endif


#ifndef EXTF_q_writestring
ot_int q_writestring(ot_queue* q, ot_u8* string, ot_int length) {
    ot_int limit;
    
    if (length <= 0) {
        return 0;
    }
    
    limit = q_writespace(q);
    if (limit < length) {
        length = limit;
    } 

    ot_memcpy(q->putcursor, string, length);
    q->putcursor += length;
    
    return length;
}
#endif


#ifndef EXTF_q_readstring
ot_int q_readstring(ot_queue* q, ot_u8* string, ot_int length) {
    ot_int limit;
    
    if (length <= 0) {
        return 0;
    }
    
    limit = q_readspace(q);
    if (limit < length) {
        length = limit;
    } 
    
    ot_memcpy(string, q->getcursor, length);
    q->getcursor += length;

    return length;
}
#endif



#ifndef EXTF_q_movedata
ot_int q_movedata(ot_queue* qdst, ot_queue* qsrc, ot_int length) {
    ot_int writespace, readspace, limit;
    ot_u8* dst;
    ot_u8* src;

    if (length <= 0) {
        return 0;
    }
    
    writespace  = q_writespace(qdst);
    readspace   = q_readspace(qsrc);
    limit       = (writespace < readspace) ? writespace : readspace;
    if (limit < length) {
        return length - limit;
    }
    
    dst                 = qdst->putcursor;
    src                 = qsrc->getcursor;
    qdst->putcursor    += length;
    qsrc->getcursor    += length;
    ot_memcpy(dst, src, length);
    
    return length;
}
#endif



#if (!defined(__C2000__) && (defined(__STDC__) || defined (__POSIX__)) )
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

