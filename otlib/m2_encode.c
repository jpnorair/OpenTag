/* Copyright 2010-2012 JP Norair
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
  * @file       /otlib/m2_encode.c
  * @author     JP Norair
  * @version    V2.0
  * @date       31 Jul 2012
  * @brief      Mode 2 SW Encoding support
  * @ingroup    Encode
  * 
  ******************************************************************************
  */

#include "OT_config.h"
#include "OT_platform.h"

#include "crc16.h"
#include "m2_encode.h"
#include "buffers.h"
#include "radio.h"


em2_struct  em2;
fn_codec    em2_encode_data;
fn_codec    em2_decode_data;



#if ( (RF_FEATURE_CRC == ENABLED) && \
      (RF_FEATURE_PN9 == ENABLED) && \
      ((RF_FEATURE_FEC == ENABLED) || (M2_FEATURE(FEC) != ENABLED)) )
    
#   ifndef EXTF_em2_encode_data_HW
    void em2_encode_data_HW() {
        while ( (em2.bytes > 0) && (radio_txopen() == True) ) {
            radio_putbyte( q_readbyte(&txq) );
            em2.bytes--;
        }
    }
#   endif
    
#   ifndef EXTF_em2_decode_data_HW
    void em2_decode_data_HW() {
        if (em2.state == 0) {
            em2.state--;
            q_writebyte(&rxq, radio_getbyte() );
            em2.bytes = (ot_int)rxq.front[0]/* - 1*/;
        }
        while ( (em2.bytes > 0) && (radio_rxopen() == True) ) {
            q_writebyte(&rxq, radio_getbyte() );
            em2.bytes--;
        }
    }
#   endif
#endif
    
    


#if ((RF_FEATURE(PN9) == ENABLED) && (RF_FEATURE(CRC) != ENABLED))

#   ifndef EXTF_em2_encode_data_HWCRC
    void em2_encode_data_HWCRC() {
        while ( (em2.bytes > 0) && (radio_txopen() == True) ) {
            crc_calc_stream();
            radio_putbyte( q_readbyte(&txq) );
            em2.bytes--;
        }
    }
#   endif
    
#   ifndef EXTF_em2_decode_data_HWCRC
    void em2_decode_data_HWCRC() {
        if (em2.state == 0) {
            em2.state--;
            rxq.length++;
            *rxq.putcursor  = radio_getbyte();
            em2.bytes       = (ot_int)*rxq.putcursor /* - 1 */;		//new spec is non-inclusive length byte
            crc_init_stream(1+em2.bytes, rxq.putcursor++);		//new spec adds +1, as it is non-inclusive
            crc_calc_stream();
        }
        while ( (em2.bytes > 0) && (radio_rxopen() == True) ) {
            q_writebyte(&rxq, radio_getbyte() );
            crc_calc_stream();
            em2.bytes--;
        }
    }
#   endif
#endif



#if ( (RF_FEATURE(PN9) != ENABLED) || \
         ((M2_FEATURE(FEC) == ENABLED) && (RF_FEATURE_FEC != ENABLED)) )
/// Only compile these functions if the RF core does not have a built-in PN9
/// encoder/decoder.  Some radios have PN9, although it is not to the spec of
/// Mode 2.  The CC430/CC11xx have suitable HW.  There is one other chip I know
/// of that has suitable HW, but it is not yet public knowledge.

    void init_PN9() { em2.PN9_lfsr.ushort = 0x01FF; }
    ot_u8 get_PN9() { return em2.PN9_lfsr.ubyte[LOWER]; }

    void rotate_PN9() {
    /// Nibble-wise PN9 implementation.  Runs pretty fast, no table.
#       define PN9reg       em2.PN9_lfsr.ushort

        ot_u16 x;
        x           = (PN9reg << 5) ^ PN9reg;
        x          &= 0x01E0;
        PN9reg    >>= 4;
        PN9reg     |= x;
        x           = (PN9reg << 5) ^ PN9reg;
        x          &= 0x01E0;
        PN9reg    >>= 4;
        PN9reg     |= x;
    }
#endif
    
#if (RF_FEATURE(PN9) != ENABLED)
#   ifndef EXTF_em2_encode_data_PN9
    void em2_encode_data_PN9() {
        while ( (em2.bytes > 0) && (radio_txopen() == True) ) {
            crc_calc_stream();
            radio_putbyte( q_readbyte(&txq) ^ get_PN9() );
            rotate_PN9();
            em2.bytes--;
        }
    }
#   endif
#endif
    
#if (RF_FEATURE(PN9) != ENABLED)
#   ifndef EXTF_em2_decode_data_PN9
    void em2_decode_data_PN9() {
        if (em2.state == 0) {
            em2.state--;
            rxq.length++;
            *rxq.putcursor  = (radio_getbyte() ^ get_PN9());
            em2.bytes       = (ot_int)*rxq.putcursor/* - 1*/;
            crc_init_stream(em2.bytes+1 /*rxq.front[0]*/, rxq.putcursor++);
            crc_calc_stream();
            rotate_PN9();
        }
        while ( (em2.bytes > 0) && (radio_rxopen() == True) ) {
            q_writebyte(&rxq, (radio_getbyte() ^ get_PN9()) );
            crc_calc_stream();         
            rotate_PN9();
            em2.bytes--;
        }
    }
#   endif
#endif






#if ((M2_FEATURE(FECTX) == ENABLED) && (RF_FEATURE(FEC) != ENABLED))
/// Only compile these functions if the RF core does not have a built-in FEC
/// encoder/decoder.  Some radios have FEC, although it is not to the spec of
/// Mode 2.  The CC11xx has suitable HW, although the CC430 does not.  There is 
/// one other chip I know of that has suitable HW, but it is not yet public
/// knowledge.

//#   include "m2_encode_table.h"

static const ot_u8 FECtable[16] = { 0, 3, 1, 2, 3, 0, 2, 1, 3, 0, 2, 1, 0, 3, 1, 2 }; 
    
#   ifndef EXTF_em2_encode_data_FEC
    void em2_encode_data_FEC() {
        ot_int      i, j, k;
        ot_u8       scratch;
        ot_u8       input;
        ot_u8       data_buffer[RADIO_BUFFER_TXMAX];
        Twobytes    FECoutput;
        Twobytes    FECreg;       //@todo adjust to allow multiframe packets
        Fourbytes   INToutput;
        
        k               = 0;
        FECreg.ushort   = 0;      //@todo adjust to allow multiframe packets
        
        // Encode each input byte into two output bytes, and add the trellis
        // terminator to the end of the message (0x0B).  The number of
        // pre-encoded bytes plus the trellis terminator must be even, so one 
        // or two trellis terminators are added (odd or even).  The number of 
        // post-encoded bytes is always a multiple of 4
        while ( (em2.state != 0) && (radio_txopen_4() == True) ) {
            
            if (em2.bytes == 0) {
                em2.state--;
                input = 0x0B;                   //trellis terminator
            }
            else {
                em2.bytes--;
                crc_calc_stream();
                input   = q_readbyte(&txq);
                input  ^= get_PN9();
                rotate_PN9();
            }

            FECoutput.ushort     = 0;
            FECreg.ubyte[UPPER] &= 0x07;
            FECreg.ubyte[LOWER]  = input;
            
            for (j=8; j>0; j--) {
                FECoutput.ushort = (FECoutput.ushort << 2) | FECtable[ (FECreg.ushort >> 7) ];
                FECreg.ushort    = (FECreg.ushort << 1) & 0x7FF;
            }
            data_buffer[k++] = FECoutput.ubyte[UPPER];
            data_buffer[k++] = FECoutput.ubyte[LOWER];
        }
        
        // Interleave the buffer after it is encoded
        // The interleaver operates on four bytes at a time
        for (i=0; i<k; i+=4) {
            INToutput.ulong = 0;
            
            for (j=0; j<16; j++) {
                scratch         = data_buffer[i + (~j & 0x03)];
                //INToutput.ulong = (INToutput.ulong << 2) | ((scratch >> (2 * ((j & 0x0C) >> 2))) & 0x03);
                INToutput.ulong = (INToutput.ulong << 2) | ((scratch >> (((j & 0x0C) >> 2) << 1)) & 0x03);
            }
            /*
            INToutput.ubyte[B3]  = (data_buffer[i+3] & 0x03) << 6;
            INToutput.ubyte[B3] |= (data_buffer[i+2] & 0x03) << 4;
            INToutput.ubyte[B3] |= (data_buffer[i+1] & 0x03) << 2;
            INToutput.ubyte[B3] |= (data_buffer[i+0] & 0x03);
            
            INToutput.ubyte[B2]  = (data_buffer[i+3] & 0x0C) << 4;
            INToutput.ubyte[B2] |= (data_buffer[i+2] & 0x0C) << 2;
            INToutput.ubyte[B2] |= (data_buffer[i+1] & 0x0C);
            INToutput.ubyte[B2] |= (data_buffer[i+0] & 0x0C) >> 2;
            
            INToutput.ubyte[B1]  = (data_buffer[i+3] & 0x30) << 2;
            INToutput.ubyte[B1] |= (data_buffer[i+2] & 0x30);
            INToutput.ubyte[B1] |= (data_buffer[i+1] & 0x30) >> 2;
            INToutput.ubyte[B1] |= (data_buffer[i+0] & 0x30) >> 4;
            
            INToutput.ubyte[B0]  = (data_buffer[i+3] & 0xC0);
            INToutput.ubyte[B0] |= (data_buffer[i+2] & 0xC0) >> 2;
            INToutput.ubyte[B0] |= (data_buffer[i+1] & 0xC0) >> 4;
            INToutput.ubyte[B0] |= (data_buffer[i+0] & 0xC0) >> 6;
            */
            radio_putfourbytes(&INToutput.ubyte[0]);
        }
        
    }
#   endif
#endif


#if ((M2_FEATURE(FECRX) == ENABLED) && (RF_FEATURE(FEC) != ENABLED))
    /// @note I am not aware if CC430 has the computational ability to decode FEC in
    ///       real time.  Thus packets might be limited to the size of the radio
    ///       buffer.  I am going to test it soon!

    static const ot_u8 TrellisSourceState[8][2] = {
        {0, 4},         // State {0,4} -> State 0
        {0, 4},         // State {0,4} -> State 1
        {1, 5},         // State {1,5} -> State 2
        {1, 5},         // State {1,5} -> State 3
        {2, 6},         // State {2,6} -> State 4
        {2, 6},         // State {2,6} -> State 5
        {3, 7},         // State {3,7} -> State 6
        {3, 7}          // State {3,7} -> State 7
    };

    static const ot_u8 TrellisTransitionOutput[8][2] = {
        {0, 3},         // State {0,4} -> State 0 produces {"00", "11"}
        {3, 0},         // State {0,4} -> State 1 produces {"11", "00"}
        {1, 2},         // State {1,5} -> State 2 produces {"01", "10"}
        {2, 1},         // State {1,5} -> State 3 produces {"10", "01"}
        {3, 0},         // State {2,6} -> State 4 produces {"11", "00"}
        {0, 3},         // State {2,6} -> State 5 produces {"00", "11"}
        {2, 1},         // State {3,7} -> State 6 produces {"10", "01"}
        {1, 2}          // State {3,7} -> State 7 produces {"01", "10"}
    };

    static const ot_u8 TrellisTransitionInput[8] = { 0, 1, 0, 1, 0, 1, 0, 1 };

    ot_u8 hamming_weight(ot_u8 a) {
        a = ((a & 0xAA) >> 1) + (a & 0x55);
        a = ((a & 0xCC) >> 2) + (a & 0x33);
        a = ((a & 0xF0) >> 4) + (a & 0x0F);
        return a;
    }

    /** @fn       min
      * @brief    Returns the minimum of two byte values
      * @param a  Value 1
      * @param b  Value 2
      * @return   Minimum of two values: Value 1 or Value 2
      */
    ot_u8 min(ot_u8 a, ot_u8 b) {
        return (a <= b ? a : b);
    }

    
    
#   ifndef EXTF_em2_decode_data_FEC
    void em2_decode_data_FEC() {

        // Two sets of buffers (last, current) for each destination state for holding: 
        ot_u32  path_matrix[2][8];      // Encoder input data (32b window)
    
        // Decoding
        ot_u8   input;
        ot_u8   min_cost;
    
        // Deinterleaving
        ot_u8*  data_in;
        ot_u8   deint_data[4];
    
        // Variables used to hold # Viterbi iterations to run, # bytes output,
        // minimum cost for any destination state, bit index of input symbol
        ot_int  i, j;
        ot_int  bit_shift = 6;
        
        while ( (em2.bytes > 0) && (radio_rxopen_4() == True) ) {
            
            /// De-interleave a 4 byte block
            {
                ot_u8   int_data[4];
                radio_getfourbytes(int_data);
                em2.bytes -= 4;
            
                for (i=0; i<4; i++) {
                    input = 0;
                    for (j=3; j>=0; j--) {
                        input  <<= 2;
                        input   |= (int_data[j] >> (i<<1)) & 0x03;
                    }
                    deint_data[i] = input;
                }
                data_in = deint_data;
            }
            
            
            // DECODE the deinterleaved data
            // Process up to 4 bytes of de-interleaved input data, 
            // processing one encoder symbol (2b) at a time
            for (i=16; i>0; i--) {
                ot_u8   symbol;
                
                min_cost    = 0xFF;
                symbol      = ((*data_in) >> bit_shift) & 0x03;
                bit_shift  -= 2;
                
                if (bit_shift < 0) {       // if bit shifting is all done (byte finished)
                    bit_shift = 6;              // reset bit_shift
                    data_in++;                  // Update pointer to the next byte of received data
                }
                
                
                // For each destination state in the trellis, 
                // calculate hamming costs for both possible paths into state, 
                // and select the one with lowest cost. 
                for (j=0; j<8; j++) {
                    ot_u8   state0;
                    ot_u8   state1;
                    ot_u8   cost0;
                    ot_u8   cost1;
                
                    input = TrellisTransitionInput[j];
        
                    // Calculate cost of transition from each of the 2 source states
                    //    (cost is Hamming difference between received 2b symbol 
                    //     and expected symbol for transition) 
                    state0  = TrellisSourceState[j][0];
                    cost0   = em2.cost_matrix[em2.last_buffer][state0];
                    cost0  += hamming_weight(symbol ^ TrellisTransitionOutput[j][0]);
                    
                    state1  = TrellisSourceState[j][1];
                    cost1   = em2.cost_matrix[em2.last_buffer][state1];
                    cost1  += hamming_weight(symbol ^ TrellisTransitionOutput[j][1]);
                    
                    // Select transition that gives lowest cost in destination state, 
                    // copy that source state's path and add new decoded bit 
                    if (cost0 <= cost1) {
                        em2.cost_matrix[em2.current_buffer][j]  = cost0;
                        path_matrix[em2.current_buffer][j]      = path_matrix[em2.last_buffer][state0] << 1; 
                        path_matrix[em2.current_buffer][j]     |= input;
                        min_cost                                = min(min_cost, cost0);
                    }
                    else {
                        em2.cost_matrix[em2.current_buffer][j]  = cost1;
                        path_matrix[em2.current_buffer][j]      = path_matrix[em2.last_buffer][state1] << 1;
                        path_matrix[em2.current_buffer][j]     |= input;
                        min_cost                                = min(min_cost, cost1);
                    }
                }
                em2.path_bits++;
                
                // If trellis history is sufficiently long, 
                // output a byte of decoded data
                if (em2.path_bits == 32) {
                    ot_u8 new_byte;
                    
                    em2.path_bits  -= 8;
                    //em2.path_bits   = 24;
                    new_byte        = (ot_u8)(path_matrix[em2.current_buffer][0] >> 24);
                    new_byte       ^= get_PN9();
                    rotate_PN9();
                    q_writebyte(&rxq, new_byte);
                    
                    if (em2.state == 0) {
                    	//otapi_led2_off();
                    	new_byte++;            			// added to meet new spec
                        em2.databytes   = new_byte;		// frame length is the first byte... always.
                        em2.bytes       = ((em2.databytes >> 1) + 1) << 2;
                        em2.bytes      -= 4;
                        em2.state--;
                        crc_init_stream(new_byte, rxq.front);
                    }
                    em2.databytes--;
                    crc_calc_stream();
                }
                
                // After having processed 3-symbol trellis terminator, 
                // flush out remaining data (always end-of-frame)
                if ( (em2.databytes <= 3)  && (em2.path_bits == ((em2.databytes<<3) + 3)) ) { 
                    while (em2.path_bits >= 8) {
                        ot_u8 new_byte;
                        
                        em2.databytes--;
                        new_byte        = (ot_u8)(path_matrix[em2.current_buffer][0] >> em2.path_bits);
                        em2.path_bits  -= 8;
                        new_byte       ^= get_PN9();
                        rotate_PN9();
                        q_writebyte(&rxq, new_byte);
                        crc_calc_stream();
                    }
                    return;
                }
                
                // Swap current and last buffers for next iteration
                em2.last_buffer      = (em2.last_buffer+1) & 1;
                em2.current_buffer   = (em2.current_buffer+1) & 1;
            }
            
            // Normalize costs so that minimum cost becomes 0
            for (j=0; j<8; j++) {
                em2.cost_matrix[em2.last_buffer][j] -= min_cost;
            }
        }
        
    }
#   endif 
#endif




#if ((RF_FEATURE(FEC) == ENABLED) || (RF_FEATURE(PN9) == ENABLED))
#   define SET_ENCODER_HW()         (em2_encode_data = &em2_encode_data_HW)
#   define SET_DECODER_HW()         (em2_decode_data = &em2_decode_data_HW)
#   define SET_ENCODER_HW_CRC()     (em2_encode_data = &em2_encode_data_HW_CRC)
#   define SET_DECODER_HW_CRC()     (em2_decode_data = &em2_decode_data_HW_CRC)
#else
#   define SET_ENCODER_HW();         
#   define SET_DECODER_HW();        
#   define SET_ENCODER_HW_CRC();    
#   define SET_DECODER_HW_CRC();    
#endif


#if (RF_FEATURE(PN9) != ENABLED)
//	#echo pn9_not_enabled
#endif


#define ENC_HW_ON		(RF_FEATURE(CRC) && RF_FEATURE(PN9))
#define ENC_HWCRC_ON	(RF_FEATURE(PN9))
#define ENC_FEC_ON     	((RF_FEATURE(FEC) != ENABLED) && M2_FEATURE(FECTX))
#define ENC_PN9_ON		(RF_FEATURE(PN9) != ENABLED)
#define ENCODERS		(ENC_HW_ON + ENC_HWCRC_ON + ENC_FEC_ON + ENC_PN9_ON)

#define DEC_HW_ON		ENC_HW_ON
#define DEC_HWCRC_ON	ENC_HWCRC_ON
#define DEC_FEC_ON     	((RF_FEATURE(FEC) != ENABLED) && M2_FEATURE(FECRX))
#define DEC_PN9_ON		(RF_FEATURE(PN9) != ENABLED)
#define DECODERS		(DEC_HW_ON + DEC_HWCRC_ON + DEC_FEC_ON + DEC_PN9_ON)

#define ENC_HW			(ENC_HW_ON-1)
#define ENC_HWCRC		(ENC_HW + ENC_HWCRC_ON)
#define ENC_FEC			(ENC_HWCRC + ENC_FEC_ON)
#define ENC_PN9			(ENC_FEC + ENC_PN9_ON)

#define DEC_HW			(DEC_HW_ON-1)
#define DEC_HWCRC		(DEC_HW + DEC_HWCRC_ON)
#define DEC_FEC			(DEC_HWCRC + DEC_FEC_ON)
#define DEC_PN9			(DEC_FEC + DEC_PN9_ON)


static const fn_codec m2_encoder[ENCODERS] = {
#if (ENC_HW_ON)
	&em2_encode_data_HW,
#endif
#if (ENC_HWCRC_ON)
	&em2_encode_data_HWCRC,
#endif
#if (ENC_PN9_ON)
	&em2_encode_data_PN9,
#endif
#if (ENC_FEC_ON)
	&em2_encode_data_FEC,
#endif
};

static const fn_codec m2_decoder[DECODERS] = {
#if (DEC_HW_ON)
	&em2_decode_data_HW,
#endif
#if (DEC_HWCRC_ON)
	&em2_decode_data_HWCRC,
#endif
#if (DEC_PN9_ON)
	&em2_decode_data_PN9,
#endif
#if (DEC_FEC_ON)
	&em2_decode_data_FEC,
#endif
};



#ifndef EXTF_em2_encode_newpacket
void em2_encode_newpacket() {
#if (ENCODERS == 1)
	em2_encode_data = m2_encoder[0];

#elif ((ENC_HW_ON && ENC_FEC_ON) || (ENC_HWCRC_ON && ENC_FEC_ON))
	em2_encode_data = m2_encoder[(txq.options.ubyte[LOWER] != 0)];

#elif (ENC_FEC_ON && ENC_PN9_ON)
	em2_encode_data = m2_encoder[(txq.options.ubyte[LOWER] != 0)];

#endif
}
#endif


#ifndef EXTF_em2_decode_newpacket
void em2_decode_newpacket() {
#if (DECODERS == 1)
	em2_decode_data = m2_decoder[0];

#elif ((DEC_HW_ON && DEC_FEC_ON) || (DEC_HWCRC_ON && DEC_FEC_ON))
	em2_decode_data = m2_decoder[(txq.options.ubyte[LOWER] != 0)];

#elif (DEC_FEC_ON && DEC_PN9_ON)
	em2_decode_data = m2_decoder[(txq.options.ubyte[LOWER] != 0)];

#endif
}
#endif




#ifndef EXTF_em2_encode_newframe
void em2_encode_newframe() {
    /// 1. Prepare the CRC, also adding 2 bytes to the frame length
#   if (RF_FEATURE(CRC) != ENABLED)
		if (txq.options.ubyte[UPPER] != 0) {
			crc_init_stream(txq.length, txq.getcursor);
			//txq.putcursor   += 2;
        	txq.length      += 2;
		}
#   endif

    /// 2. Align encoder control variables with tx frame
    em2.bytes   = txq.length;
    em2.fr_info = &txq.getcursor[3];
    
    /// 3. Prepare frame encoder, depending on frame type and supported methods.
    ///    (0) HW Encoder: do nothing. 
    ///    (1) SW PN9 Encoder: init PN9 LFSR -- also used in FEC. 
    ///    (2) SW FEC Encoder: init FEC state machine and data. 
#   if ((M2_FEATURE(FECTX) == ENABLED) && (RF_FEATURE(FEC) != ENABLED))
        if (txq.options.ubyte[LOWER]) {
            // state is 1 if odd amount of data, 2 if even
        	/// Amount of FEC bytes over the air is always a multiple of 4:
            ///  (   (Bytewise Data)       )
            ///  ( ------------------- + 1 ) * 4
            ///  (          2              )
            ///  = ((em2.bytes >> 1) + 1) << 2;

            em2.state   = ((em2.bytes & 1) == 0);
            em2.state  += 1;
#			if (RF_FEATURE(PN9) == ENABLED)
            	init_PN9();
#			endif
        }
#   endif
#   if ((RF_FEATURE(PN9) != ENABLED))
    	init_PN9();
#	endif
}
#endif



#ifndef EXTF_em2_decode_newframe
void em2_decode_newframe() {
    em2.fr_info = &rxq.front[3];
    em2.state   = 0;
    em2.bytes   = 8;      // dummy length until actual length is received
    
    /// Prepare SW FEC Decoders, and if necessary PN9 decoder
#   if ((M2_FEATURE(FECRX) == ENABLED) && (RF_FEATURE(FEC) != ENABLED))
        if (rxq.options.ubyte[LOWER]) {
#           ifdef __BIG_ENDIAN__
                ///@todo: make endian agnostic
#           else
                *((ot_u32*)(&em2.cost_matrix[0][0])) = 0x64646400;
                *((ot_u32*)(&em2.cost_matrix[0][4])) = 0x64646464;
                *((ot_u32*)(&em2.cost_matrix[1][0])) = 0x00000000;
                *((ot_u32*)(&em2.cost_matrix[1][4])) = 0x00000000;
                ///@todo Verify: seemed like an error on the last line ([0][4]), which I corrected to [1][4]
#           endif

            em2.path_bits       = 0;
            em2.last_buffer     = 0;
            em2.current_buffer  = 1;
#			if (RF_FEATURE(PN9) == ENABLED)
            	init_PN9();
#			endif
        }
#   endif
#   if (RF_FEATURE(PN9) != ENABLED)
        init_PN9();
#   endif
}
#endif



#ifndef EXTF_em2_remaining_frames
ot_int em2_remaining_frames() {
/// Returns 0 if no more frames, or non-zero if more frames
    return (ot_int)(*em2.fr_info & 0x10);
}
#endif


#ifndef EXTF_em2_remaining_bytes
ot_int em2_remaining_bytes() {
    return em2.bytes;
}
#endif

//ot_bool em2_complete() {
//    return (ot_bool)((em2.bytes + em2.frames) == 0);
//}








