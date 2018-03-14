/*  Copyright 2011-2012 JP Norair
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
  * @file       /otlib/OT_utils.h
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2012
  * @brief      Utility definitions and macros for OpenTag
  *
  ******************************************************************************
  */


#ifndef __OT_UTILS_H
#define __OT_UTILS_H

#include <otsys/types.h>
#include <otsys/config.h>


/// Null Signal calls
void otutils_null(void);
void otutils_sig_null(ot_int a);
void otutils_sig2_null(ot_int a, ot_int b);
void otutils_sigv_null(void* a);



// Exp-Mantissa expansion for common 7-bit field
ot_ulong otutils_calc_timeout(ot_u8 timeout_code);


// Exp-Mantissa expansion for common 7-bit field
ot_u8 otutils_encode_timeout(ot_u16 timeout_ticks);

// 1 byte to 2 hex chars
ot_u16 otutils_byte2hex(ot_u8 input);

// Binary data to hex-text
ot_int otutils_bin2hex(ot_u8* src, ot_u8* dst, ot_int size);


// ot_int type to decimal text
ot_int otutils_int2dec(ot_u8* dst, ot_int data);

ot_int otutils_long2dec(ot_u8* dst, ot_long data);


ot_int slistf(ot_u8* dst, const char* label, char format, ot_u8 number, ot_u8* src);


/** Big Endian converters
  * DASH7 Mode 2 is a radio system, radios stream data, and because the western
  * world (a) writes left to right and (b) invented all things computer, the
  * streaming of data is a big endian concept.  For interoperability, DASH7 
  * Mode 2 standardizes (and requires) the use of big endian in the storage of
  * data in its ISF Data Elements.
  */
#   ifdef __BIG_ENDIAN__
#       define PUT_BE_U16(DAT)      (ot_u16)DAT
#       define PUT_BE_U32(DAT)      (ot_u32)DAT
#       define GET_BE_U16(DAT)      PUT_BE_U16(DAT)
#       define GET_BE_U32(DAT)      PUT_BE_U32(DAT)
#       define FLIP_BE_U32(DAT)     (ot_u32)DAT
#       define GET_B0_U16(DAT)      ((ot_u8*)&(DAT))[0]
#       define GET_B1_U16(DAT)      ((ot_u8*)&(DAT))[1]
#       define GET_BE_LBFS(DAT)     (ot_u8)(DAT)

#       define ENDIANIZE_U16(DAT)   while(0)
#       define ENDIANIZE_U32(DAT)   while(0)

#   else
#       define PUT_BE_U16(DAT)      (ot_u16)( (((ot_u16)DAT)>>8) | (((ot_u16)DAT)<<8) )
#       define PUT_BE_U32(DAT)      ((((ot_u32)DAT)>>24) | ((((ot_u32)DAT)<<8) & 0x00FF0000) | \
                                        ((((ot_u32)DAT)>>8) & 0x0000FF00) | (((ot_u32)DAT)<<24))
#       define GET_BE_U16(DAT)      PUT_BE_U16(DAT)
#       define GET_BE_U32(DAT)      PUT_BE_U32(DAT)
#       define FLIP_BE_U32(DAT)     (ot_u32)( (((ot_u32)DAT)>>16) | (((ot_u32)DAT)<<16) )
#       define GET_B0_U16(DAT)      ((ot_u8*)&(DAT))[0]
#       define GET_B1_U16(DAT)      ((ot_u8*)&(DAT))[1]
#       define GET_BE_LBFS(DAT)     (ot_u8)(DAT)

#       define ENDIANIZE_U16(DAT)   do { DAT = GET_BE_U16(DAT); } while(0)
#       define ENDIANIZE_U32(DAT)   do { DAT = GET_BE_U32(DAT); } while(0)

#   endif



/** Generic Duff's Device Macros
  * Thanks to Chris Ladden from Liquidware for hooking me up with this!
  * 
  * DUFFLOOP_16(  count, any_action_to_unroll() );
  * 
  * If you are just copying data, you might try using memcpy()
  */
#define DUFFLOOP_16(aCount, aAction) \
                                    do { \
                                        ot_int count_ = (aCount); \
                                        ot_int times_ = (count_ + 15) >> 4; \
                                        switch (count_ & 0xF) { \
                                            case 0: do { aAction; \
                                            case 15: aAction; \
                                            case 14: aAction; \
                                            case 13: aAction; \
                                            case 12: aAction; \
                                            case 11: aAction; \
                                            case 10: aAction; \
                                            case 9: aAction; \
                                            case 8: aAction; \
                                            case 7: aAction; \
                                            case 6: aAction; \
                                            case 5: aAction; \
                                            case 4: aAction; \
                                            case 3: aAction; \
                                            case 2: aAction; \
                                            case 1: aAction; \
                                            } while (--times_ > 0); \
                                        } \
                                    } while (0)

#define DUFFLOOP_8(aCount, aAction) \
                                    do { \
                                        ot_int count_ = (aCount); \
                                        ot_int times_ = (count_ + 7) >> 3; \
                                        switch (count_ & 0x7) { \
                                            case 0: do { aAction; \
                                            case 7: aAction; \
                                            case 6: aAction; \
                                            case 5: aAction; \
                                            case 4: aAction; \
                                            case 3: aAction; \
                                            case 2: aAction; \
                                            case 1: aAction; \
                                            } while (--times_ > 0); \
                                        } \
                                    } while (0)



/** M1 Data Rate & Packet Timings
  * These are used by any routine needing to calculate the duration of a Mode 1
  * Packet.  The SYNC Constants use a synchronous, sub-sampled "SYNCBIT" as the
  * primary unit.  Some radios have the ability to transmit and/or receive the
  * M1 signal via synchronous data, and others don't.
  *
  * Unless you have a legacy Mode 1 application, I cannot recommend Mode 1 
  * because Mode 2 is vastly superior in all regards.  Besides, OpenTag doesn't
  * really support Mode 1 at the moment.
  */
#   define M1_US_PER_PREAMBLE       1296
#   define M1_US_PER_BYTE           324
#   define M1_US_PER_OVERHEAD       36
#   define M1_BITS_PER_BYTE         9

#   define M1_US_PER_SYNCBIT        6
#   define M1_US_PER_SYNCBYTE       (M1_USPER_SYNCBIT * 8)
#   define M1_SYNCBITS_PER_BYTE     54
#   define M1_SYNCBITS_PREAMBLE     216
#   define M1_SYNCBITS_OVERHEAD     6



/** M2 Data Rate & Packet Timings 
  * These are used by any routine needing to calculate the duration of an M2
  * Packet.  M2 is "Mode 2," a.k.a. ISO 18000-7.4 Mode 2.  
  *
  * Mode 2 is currently a draft spec that has been frozen by the DASH7 Alliance 
  * prior to official ratification into ISO.  OpenTag is the de-facto 
  * implementation of Mode 2.
  */
#   define M2_US_PER_NSYMBOL        18
#   define M2_US_PER_TSYMBOL        5
#   define M2_SYMBOLS_PER_FECBYTE   16
#   define M2_SYMBOLS_PER_PN9BYTE   8
#   define M2_SYMBOLS_PER_PREAMBLE  32
#   define M2_SYMBOLS_PER_SYNCWORD  16




#endif
