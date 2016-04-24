#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../../include/otsys/types.h"
#include "../../include/otlib/crc16_table.h"


#   ifdef __BIG_ENDIAN__
#       define UPPER    0
#       define LOWER    1
#       define B0       3
#       define B1       2
#       define B2       1
#       define B3       0
#   else
#       define UPPER    1
#       define LOWER    0
#       define B0       0
#       define B1       1
#       define B2       2
#       define B3       3
#   endif


/** @note CRCBASE, CRCPOLY
  * These make the "magic number" to use when starting up CRC, written to the
  * crc.value variable during init_crc.value.  They are not currently used,
  * having been substituted for the somewhat optimized tabular approach.  If
  * program memory becomes a problem, a bitwise CRC routine may be used in place
  * of the tabular method.
  */
#ifndef CRCBASE
#define CRCBASE    0xFFFF
#define CRCBASE_LO 0xFF
#define CRCBASE_HI 0xFF
#endif

#ifndef CRCPOLY
#define CRCPOLY    0x8005
#define CRCPOLY_LO 0x05
#define CRCPOLY_HI 0x80
#endif




typedef struct {
    ot_u16      alloc;
  //ot_u16      length;     ///@todo length field is replaced with q_length()
    ot_uni16    options;
    ot_u8*      getcursor;
    ot_u8*      putcursor;
    ot_u8*      front;
    ot_u8*      back;
} ot_queue;

void q_init(ot_queue* q, ot_u8* buffer, ot_u16 alloc);
void q_rebase(ot_queue* q, ot_u8* buffer);
void q_copy(ot_queue* q1, ot_queue* q2);
ot_int q_length(ot_queue* q);
ot_int q_span(ot_queue* q);
ot_int q_space(ot_queue* q);
void q_empty(ot_queue* q);
ot_u8* q_start(ot_queue* q, ot_uint offset, ot_u16 options);
ot_u8* q_markbyte(ot_queue* q, ot_int shift);
void q_writebyte(ot_queue* q, ot_u8 byte_in);
void q_writeshort(ot_queue* q, ot_u16 short_in);
void q_writeshort_be(ot_queue* q, ot_u16 short_in);
void q_writelong(ot_queue* q, ot_u32 long_in);
ot_u8 q_readbyte(ot_queue* q);
ot_u16 q_readshort(ot_queue* q);
ot_u16 q_readshort_be(ot_queue* q);
ot_u32 q_readlong(ot_queue* q);
void q_writestring(ot_queue* q, ot_u8* string, ot_int length);
void q_readstring(ot_queue* q, ot_u8* string, ot_int length);





typedef struct {
    ot_u8*      cursor;
    ot_int      count;
    ot_bool     writeout;
    ot_u16      val;
} crcstream_t;



void crc_init_stream(crcstream_t* stream, ot_bool writeout, ot_int size, ot_u8* data);
ot_u16 crc_block(crcstream_t* stream, ot_bool writeout, ot_int size, ot_u8* data);
void crc_calc_stream(crcstream_t* stream);
void crc_calc_nstream(crcstream_t* stream, ot_u16 n);
ot_bool crc_check(crcstream_t* stream);
ot_u16 crc_get(crcstream_t* stream);














ot_queue testq;
ot_u8 buffer[256];









/** @var crc_table
  * @ingroup Platform
  *
  */
static const ot_u16 crc16_table[256] = { 
    CRCx00, CRCx01, CRCx02, CRCx03, CRCx04, CRCx05, CRCx06, CRCx07, 
    CRCx08, CRCx09, CRCx0A, CRCx0B, CRCx0C, CRCx0D, CRCx0E, CRCx0F, 
    CRCx10, CRCx11, CRCx12, CRCx13, CRCx14, CRCx15, CRCx16, CRCx17, 
    CRCx18, CRCx19, CRCx1A, CRCx1B, CRCx1C, CRCx1D, CRCx1E, CRCx1F, 
    CRCx20, CRCx21, CRCx22, CRCx23, CRCx24, CRCx25, CRCx26, CRCx27, 
    CRCx28, CRCx29, CRCx2A, CRCx2B, CRCx2C, CRCx2D, CRCx2E, CRCx2F, 
    CRCx30, CRCx31, CRCx32, CRCx33, CRCx34, CRCx35, CRCx36, CRCx37, 
    CRCx38, CRCx39, CRCx3A, CRCx3B, CRCx3C, CRCx3D, CRCx3E, CRCx3F, 
    CRCx40, CRCx41, CRCx42, CRCx43, CRCx44, CRCx45, CRCx46, CRCx47, 
    CRCx48, CRCx49, CRCx4A, CRCx4B, CRCx4C, CRCx4D, CRCx4E, CRCx4F, 
    CRCx50, CRCx51, CRCx52, CRCx53, CRCx54, CRCx55, CRCx56, CRCx57, 
    CRCx58, CRCx59, CRCx5A, CRCx5B, CRCx5C, CRCx5D, CRCx5E, CRCx5F, 
    CRCx60, CRCx61, CRCx62, CRCx63, CRCx64, CRCx65, CRCx66, CRCx67, 
    CRCx68, CRCx69, CRCx6A, CRCx6B, CRCx6C, CRCx6D, CRCx6E, CRCx6F, 
    CRCx70, CRCx71, CRCx72, CRCx73, CRCx74, CRCx75, CRCx76, CRCx77, 
    CRCx78, CRCx79, CRCx7A, CRCx7B, CRCx7C, CRCx7D, CRCx7E, CRCx7F, 
    CRCx80, CRCx81, CRCx82, CRCx83, CRCx84, CRCx85, CRCx86, CRCx87, 
    CRCx88, CRCx89, CRCx8A, CRCx8B, CRCx8C, CRCx8D, CRCx8E, CRCx8F, 
    CRCx90, CRCx91, CRCx92, CRCx93, CRCx94, CRCx95, CRCx96, CRCx97, 
    CRCx98, CRCx99, CRCx9A, CRCx9B, CRCx9C, CRCx9D, CRCx9E, CRCx9F, 
    CRCxA0, CRCxA1, CRCxA2, CRCxA3, CRCxA4, CRCxA5, CRCxA6, CRCxA7, 
    CRCxA8, CRCxA9, CRCxAA, CRCxAB, CRCxAC, CRCxAD, CRCxAE, CRCxAF, 
    CRCxB0, CRCxB1, CRCxB2, CRCxB3, CRCxB4, CRCxB5, CRCxB6, CRCxB7, 
    CRCxB8, CRCxB9, CRCxBA, CRCxBB, CRCxBC, CRCxBD, CRCxBE, CRCxBF, 
    CRCxC0, CRCxC1, CRCxC2, CRCxC3, CRCxC4, CRCxC5, CRCxC6, CRCxC7, 
    CRCxC8, CRCxC9, CRCxCA, CRCxCB, CRCxCC, CRCxCD, CRCxCE, CRCxCF, 
    CRCxD0, CRCxD1, CRCxD2, CRCxD3, CRCxD4, CRCxD5, CRCxD6, CRCxD7, 
    CRCxD8, CRCxD9, CRCxDA, CRCxDB, CRCxDC, CRCxDD, CRCxDE, CRCxDF, 
    CRCxE0, CRCxE1, CRCxE2, CRCxE3, CRCxE4, CRCxE5, CRCxE6, CRCxE7, 
    CRCxE8, CRCxE9, CRCxEA, CRCxEB, CRCxEC, CRCxED, CRCxEE, CRCxEF, 
    CRCxF0, CRCxF1, CRCxF2, CRCxF3, CRCxF4, CRCxF5, CRCxF6, CRCxF7, 
    CRCxF8, CRCxF9, CRCxFA, CRCxFB, CRCxFC, CRCxFD, CRCxFE, CRCxFF
}; 


ot_u16 platform_ext_crc16;


/** Platform Default CRC Routine <BR>
  * ========================================================================<BR>
  * Uses the CRC table from the CRC module because STM32L1 does not have a 
  * standalone CRC16 peripheral.
  */
ot_u16 crc16drv_init() {
    platform_ext_crc16 = CRCBASE;
    return CRCBASE;
}




ot_u16 crc16drv_block_manual(ot_u8* block_addr, ot_int block_size, ot_u16 init) {
    ot_u8 index;
    while (--block_size >= 0) {
        index   = ((ot_u8*)&init)[UPPER] ^ *block_addr++;                    //((crc_val>>8) & 0xff) ^ *block_addr++;
        init    = (init<<8) ^ crc16_table[index];
    }
    return init;
}



ot_u16 crc16drv_block(ot_u8* block_addr, ot_int block_size) {
    return crc16drv_block_manual(block_addr, block_size, CRCBASE);
}



void crc16drv_byte(ot_u8 databyte) {
    databyte           ^= ((ot_u8*)&platform_ext_crc16)[UPPER];                 //((platform_ext_crc16>>8) & 0xff);
    platform_ext_crc16  = (platform_ext_crc16<<8) ^ crc16_table[databyte];
}



ot_u16 crc16drv_result() {
    return platform_ext_crc16;
}





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





void crc_init_stream(crcstream_t* stream, ot_bool writeout, ot_int size, ot_u8* data) {
    stream->writeout    = writeout;
    stream->cursor      = data;
    stream->count       = size;
    stream->val         = crc16drv_init();
}


void crc_calc_stream(crcstream_t* stream) {
    crc_calc_nstream(stream, 1);
}


void crc_calc_nstream(crcstream_t* stream, ot_u16 n) {
    if (stream->count > 0) {
        ot_u8* data;
        if (n > stream->count) {
            n = stream->count;
        }
        stream->count  -= n;
        data            = stream->cursor;
        stream->cursor += n;
        stream->val     = crc16drv_block_manual(data, n, stream->val);
    }
    if ((stream->count == 0) && (stream->writeout)) {
        stream->writeout    = False;
        *stream->cursor++   = (ot_u8)(stream->val >> 8);
        *stream->cursor++   = (ot_u8)(stream->val);
    }
}



ot_bool crc_check(crcstream_t* stream) {
///@todo deprecate this function in OT, in favor of crc_get(), and checking with 0.
    return (stream->val == 0);
    //return (crc16drv_result() == 0);
}


ot_u16 crc_get(crcstream_t* stream) {
    return stream->val;
    //return crc16drv_result();
}




















ot_int sub_load_rand(ot_queue* q, ot_int max) {
    ot_int  bytes;
    ot_u8   randbyte;
    
    if (max > 254)
        max = 254;
    
    bytes   = rand() % max;
    max     = bytes;
    
    while (--bytes >= 0) {
        randbyte = rand() & 255;
        q_writebyte(q, randbyte);
    }
    
    return max;
}


#define _ITERATIONS 1

int main(void) {
    const char refstring[] = "123456789";
    int i;
    crcstream_t crcstream;
    
    srand(time(NULL));
    
    q_init(&testq, buffer, 256);
    
    {   ot_u16 crc_result;
        crc_result = crc16drv_block((ot_u8*)refstring, 9);
        printf("CRC16 reference = \"%04X\"\n\n", crc_result);
    }
    
    for (i=0; i<_ITERATIONS; i++) {
        ot_int numbytes;
        ot_u16 crc_result;
    
        sub_load_rand(&testq, 254);
        numbytes = q_span(&testq);
        
        // compute CRC16 using iterative method
        crc_init_stream(&crcstream, True, numbytes, testq.getcursor);
        crc_calc_nstream(&crcstream, numbytes);
        
        // compute CRC16 using block method
        crc_result = crc16drv_block(testq.getcursor, numbytes);
        
        // Begin test
        printf("Testing CRC on %d bytes\n", numbytes);
        printf("Stream: %02X %02X, Block: %02X %02X\n", 
                    testq.getcursor[numbytes], testq.getcursor[numbytes+1], 
                    (ot_u8)(crc_result>>8), (ot_u8)crc_result   );
        
        printf("block  %04X\n", crc_result);
        printf("stream %04X\n", crc_get(&crcstream)); 
        
        // Compare Encoding results
        if (crc_result == crc_get(&crcstream)) 
            printf("    PASS: Block CRC Calc == Iterative CRC Calc\n");
        else                         
            printf("    FAIL: Block CRC Calc != Iterative CRC Calc\n");
        
        
        // Now do decoding
        numbytes += 2;
        
        crc_init_stream(&crcstream, False, numbytes, testq.getcursor);
        crc_calc_nstream(&crcstream, numbytes);
        
        crc_result = crc16drv_block(testq.getcursor, numbytes);
        
        if (crc_get(&crcstream) == 0) 
            printf("    PASS: Iterative CRC Calc Decoding == 0\n");
        else                
            printf("    FAIL: Iterative CRC Calc Decoding != 0\n");
        
        if (crc_result == 0) 
            printf("    PASS: Block CRC Calc Decoding == 0\n");
        else                 
            printf("    FAIL: Block CRC Calc Decoding != 0\n");
    }
        
    
    return 0;
}


