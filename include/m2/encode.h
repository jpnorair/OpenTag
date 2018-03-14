/* Copyright 2009-2014 JP Norair
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
  * @file       /otlib/m2_encode.h
  * @author     JP Norair
  * @version    R103
  * @date       18 Sept 2014
  * @brief      Encoding and Decoding of bytewise data for the Radio
  * @defgroup   Encode (Encode Module)
  * @ingroup    Encode
  *
  * At present, the codec setup data is taken from setting in the TXQ or RXQ,
  * depending if you are encoding or decoding.  In the future, I may make the 
  * module more flexible, but right now I am more interested in it running as 
  * fast as possible.
  *
  * Mode 2 offers two encoding options: FEC and PN9.  The queue options field
  * is set up depending on how you want to encode or decode.
  * options.ubyte[UPPER]  = 0: No CRC
  * options.ubyte[UPPER] != 0: CRC
  * options.ubyte[LOWER]  = 0: PN9
  * options.ubyte[LOWER] != 0: FEC
  ******************************************************************************
  */

#ifndef __ENCODE_H
#define __ENCODE_H


#include <otsys/config.h>


#if OT_FEATURE(M2)

#include <otplatform.h>
#include <otsys/types.h>
#include <otlib/queue.h>
#include <otlib/crc16.h>


typedef struct {
    // Core Encoder State Variables
    ot_u8   lctl;
    ot_u8   crc5;
    ot_int  bytes;
    ot_int  state;              // could be changed to ot_s8
    
    // CRC Streaming object for M2 encoding module
    crcstream_t crc;

    // PN9 register needed for SW PN9 
    // You should pick a radio that has this in HW, so this is mostly relegated
    // to Software Simulation.
#   if ( (RF_FEATURE(PN9) != ENABLED) || \
         ((M2_FEATURE(FEC) == ENABLED) && (RF_FEATURE(FEC) != ENABLED)) )
        ot_uni16 PN9_lfsr;
#   endif

    // FEC registers needed for SW FEC
    // SW FEC is fairly slow and not recommended
#   if ((M2_FEATURE(FECRX) == ENABLED) && (RF_FEATURE(FEC) != ENABLED))
        ot_int  databytes;
        ot_int  path_bits;
        ot_u8   last_buffer;
        ot_u8   current_buffer;
        ot_u8   cost_matrix[2][8];
#   endif

} em2_struct;

extern em2_struct   em2;



typedef void (*fn_codec)(void);

/** @par Mode 2 Encode Data function pointer
  * The function @c em2_encode_newframe() sets this function pointer to the
  * appropriate encode function, based on the queue options field and compiled
  * in feature settings.
  *
  * @fn     em2_encode_data
  * @brief  None
  * @param  q           (ot_queue*) queue to load data for encoding
  * @retval None
  * @ingroup Encode
  */
#ifndef EXTF_em2_encode_data
extern fn_codec em2_encode_data;
#endif

/** @par Decode function pointer
  * The function @c decode_newpacket sets this function pointer to the
  * appropriate decode function, based on input to @c decode_newpacket
  *
  * @fn     decode_data
  * @brief  Decodes comm data into byte-wise data
  * @param  None
  * @retval None
  * @ingroup Encode
  */
#ifndef EXTF_em2_decode_data
extern fn_codec em2_decode_data;
#endif




/** @brief  Initializes the encoder for a new Mode 2 packet
  * @param  None
  * @retval None
  * @ingroup Encode
  */
void em2_encode_newpacket();



/** @brief  Initializes the decoder for a new Mode 2 packet
  * @param  None
  * @retval None
  * @ingroup Encode
  *
  * The decoding engine needs to be initialized at the packet level.  It is
  * conceivable that a packet may consist of multiple frames.
  */
void em2_decode_newpacket();




/** @brief  Initializes the encoder for encoding a Mode 2 Frame
  * @param  None
  * @retval None
  * @ingroup Encode
  */
void em2_encode_newframe();
//ot_int em1_encode_newframe( );


/** @brief  Initialize the frame manager for decoding a Mode 2 Frame
  * @param  None
  * @retval None
  * @ingroup Encode
  *
  * The decoding engine is at the packet level, but the calculation of CRC and
  * management of the data queue are done at the frame level.  So to manage
  * the frame, this function needs to be run before a frame comes in.
  */
void em2_decode_newframe();
//void em1_decode_newframe(ot_u32 header); 



/** @brief  Finish decoding and framing process on a fully received frame
  * @param  None
  * @retval ot_u16      0 if there are no uncorrectable errors in this frame
  * @ingroup Encode
  *
  * If the function returns 0, that is good, it means the frame is full of 
  * happy data.
  *
  * RS block coding and CRC must be removed from the frame before passing it to
  * the next layer.  If your build/app supports RS coding, it will also be 
  * processed by this function.
  */
ot_u16 em2_decode_endframe();




/** @brief  Indicates if more frames remain in the packet, after the present
  * @param none
  * @retval ot_u8       0 or non-zero, if frames remain
  * @ingroup Encode
  */
ot_u8 em2_remaining_frames();


/** @brief  Returns bytes remaining to encode or decode
  * @param none
  * @retval ot_int      value from em2.bytes
  * @ingroup Encode
  *
  * For encoding, the value returned is the number of unencoded bytes that are
  * remaining to be encoded.  For decoding, the value is the number of encoded
  * bytes that are remaining to be decoded.
  */
ot_int em2_remaining_bytes();


/** @brief  Returns True when the encoding/decoding is complete
  * @param none
  * @retval ot_bool     true when no bytes left and no frames left
  * @ingroup Encode
  */
ot_bool em2_complete();








ot_int em2_rs_init_decode(ot_queue* q);
void em2_rs_decode(ot_int n_bytes);

ot_int em2_rs_check(void);
ot_int em2_rs_postprocess(void);
ot_int em2_rs_paritylength(ot_int msg_length);
ot_int em2_rs_init_encode(ot_queue* q);
void em2_rs_encode(ot_int n_bytes);
void em2_rs_interleave(ot_u8* start, ot_int numbytes);






void em2_encode_data_HW();
void em2_decode_data_HW();
void em2_encode_data_HWCRC();
void em2_decode_data_HWCRC();
void em2_encode_data_PN9();
void em2_decode_data_PN9();
void em2_encode_data_FEC();
void em2_decode_data_FEC();
    
#endif
#endif


