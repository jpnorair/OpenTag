#include <stdio.h>
#include <string.h>

#include "OT_types.h"
#include "OTAPI_tmpl.h"
#include "queue.h"


Queue otmpout;
ot_u8 buffer[256];




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
    memcpy(q1, q2, sizeof(Queue));
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
    memcpy(q->putcursor, string, length);
    q->length      += length;
    q->putcursor   += length;
}
#endif


#ifndef EXTF_q_readstring
void q_readstring(Queue* q, ot_u8* string, ot_int length) {
    memcpy(string, q->getcursor, length);
    q->getcursor += length;
}
#endif








void otapi_log_header(ot_u8 id_subcode, ot_int payload_length) {
    ot_u8 header[]  = { 0xDD, 0x00, 0, 0x02, 0x04, 0 };
    header[2]       = (ot_u8)payload_length;
    header[5]       = id_subcode;

    q_empty(&otmpout);  // output buffer
    q_writestring(&otmpout, header, 6);
}

void otapi_log_direct(ot_u8* data) {
    int length;
    length = data[2];
    data += 6;
    while (length != 0) {
        length--;
        putc(*data++, stdout);
    }
}




// Binary data to hex-text
#ifndef EXTF_otutils_bin2hex
ot_int otutils_bin2hex(ot_u8* src, ot_u8* dst, ot_int size) {
    ot_u8* src_end;
    ot_u8* dst_start;
    src_end     = src + size;
    dst_start   = dst;
    
    while (src != src_end) {
        ot_u8 scratch;
        *dst++      = ' ';
        scratch     = *src >> 4;
        scratch    += (scratch >= 10) ? ('A'-10) : '0';
        *dst++      = scratch;
        scratch     = *src++ & 0x0F;
        scratch    += (scratch >= 10) ? ('A'-10) : '0';
        *dst++      = scratch;
    }
    
    return (ot_int)(dst - dst_start);
}
#endif



#ifndef EXTF_otutils_int2dec
ot_int otutils_int2dec(ot_u8* dst, ot_int data) {
    ot_u8*  dst_start;
    ot_bool force;
    ot_int  divider;
    ot_int  digit;

    dst_start = dst;

    *dst++  = ' ';  //delimiter

    if (data < 0) {
        data    = 0 - data;
        *dst++  = '-';
    }

    for (divider=10000, force=False; divider!=0; divider/=10) {
        digit = (data/divider);
        if (digit | force) {
            force   = True;
            *dst++  = digit + '0';
            data   -= digit*divider;
        }
    }

    return (ot_int)(dst - dst_start);
}
#endif


ot_int slistf(ot_u8* dst, const char* label, char format, ot_u8 number, ot_u8* src) {
    ot_u8* scratch;
    ot_u8* dst_start;
    ot_int inc;

    dst_start   = dst;
    scratch     = (ot_u8*)label;
    while (*scratch != 0) {
        *dst++ = *scratch++;
    }

    if (format == 'x') {
        dst += otutils_bin2hex((ot_u8*)src, dst, number);
        goto slistf_END;
    }

    inc = (format == 'b') - (format == 's');

    if (inc != 0) {
        Twobytes value;
        while (number > 0) {
            number--;
            if (inc > 0) {
                value.sshort = (ot_int)*((ot_s8*)src);
            }
            else {
                value.ubyte[1] = *src++;
                value.ubyte[0] = *src;
            }
            src++;
            dst += otutils_int2dec(dst, value.sshort);
        }
    }

    else {
        *dst++ = ' ';
        memcpy(dst, src, number);
        dst += number;
    }

    slistf_END:
    return (ot_int)(dst - dst_start);
}





ot_bool app_udp_request(id_tmpl* id, ot_int payload_length, ot_u8* payload) {
/// Transport Layer calls this when a UDP-class request has been received.
    ot_u8* payload_end;

    // Check Source Port (our application uses 0x71)
    if (payload[0] != 0x71) {
        return False;
    }

    otapi_log_header(1, 0);   //Text is subcode 1, dummy length is 0
    otmpout.putcursor      += slistf(otmpout.putcursor, "ID:", 'x', id->length, id->value);
    *otmpout.putcursor++    = '\n';
    payload                += 2;
    payload_length         -= 2;
    payload_end             = payload + payload_length;

    while (payload < payload_end) {
        switch (*payload++) {
        // PaLFi data load: 8 bytes
        case 'D':   otmpout.putcursor  += slistf(otmpout.putcursor, "Data:", 'x', 8, payload);
                    payload            += 8;
                    break;

        // Event-type information (1 byte)
        case 'E':   otmpout.putcursor  += slistf(otmpout.putcursor, "Event:", 'b', 1, payload);
                    payload++;
                    break;

        // PaLFi RSSI bytes (3 bytes)
        case 'R':   otmpout.putcursor  += slistf(otmpout.putcursor, "RSSI:", 'b', 3, payload);
                    payload            += 3
                    break;

        // Temperature (2 bytes)
        case 'T':   otmpout.putcursor  += slistf(otmpout.putcursor, "Temp:", 's', 1, payload);
                    payload            += 2; 
                    break;

        case 'V':   otmpout.putcursor  += slistf(otmpout.putcursor, "Volt:", 's', 1, payload);
                    payload            += 2;
                    break;

        default:    continue;
        }
        *otmpout.putcursor++   = '\n';
    }

    otmpout.front[2] = otmpout.putcursor - otmpout.front - 6;
    otapi_log_direct(otmpout.front);

    return True;
}




void test_app_udp_request() {
    const ot_u8 data[] = { 0x71, 0x71,
                    'D', 1, 2, 3, 4, 5, 8, 7, 8,
                    'R', 0x50, 0xFE, 0x88,
                    'E', 0x01,
                    'V', 0x11, 0x11 };
    
    ot_u8 idvalue[8] = { 10, 11, 12, 13, 14, 15, 16, 17 };
    
    id_tmpl id;
    id.length = 8;
    id.value = idvalue;

    app_udp_request(&id, sizeof(data), (ot_u8*)data);
}





int main(void) {


    q_init(&otmpout, buffer, 256);
    
    test_app_udp_request();
    

    return 0;
}


