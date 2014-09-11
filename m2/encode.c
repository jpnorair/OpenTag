/* Copyright 2010-2014 JP Norair
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
  * @version    R102
  * @date       19 Jan 2014
  * @brief      Mode 2 SW Encoding support
  * @ingroup    Encode
  *
  ******************************************************************************
  */

#include <otstd.h>
#if (OT_FEATURE(SERVER) && OT_FEATURE(M2))

#include <m2/encode.h>
#include <m2/radio.h>

#include <otlib/crc16.h>
#include <otlib/buffers.h>

#include <platform/config.h>



em2_struct  em2;

#if !defined(EXTF_em2_encode_data)
fn_codec    em2_encode_data;
#endif

#if !defined(EXTF_em2_decode_data)
fn_codec    em2_decode_data;
#endif




/** CRC5 implementation for Low-Level Link Control Header <BR>
  * ========================================================================<BR>
  * CRC5 is used in foreground frames in order to protect the length byte and
  * low-level link control bits.
  *
  * When decoding frames, crc5 will be checked automatically as the frame
  * downloads.  You can look in em2.crc5 (which should be 0), or simply use
  * the em2_length_isvalid() function.
  *
  * When encoding frames, crc5 will be applied automatically when calling the
  * function em2_encode_newframe().
  *
  * CRC5 computation over the 10 bit field (8 bit length + 2 link-ctl bits) is
  * very fast, so even though it is wasted on background frame processing,
  * the amount of wasted energy is roughly equal to the amount of energy that
  * would be wasted through control logic that might ignore CRC5 during the
  * background frames that don't use it.
  */
ot_u8 crc0B_0t5[32] = {
    0x08, 0x03, 0x1E, 0x15, 0x0F, 0x04, 0x19, 0x12,
    0x06, 0x0D, 0x10, 0x1B, 0x01, 0x0A, 0x17, 0x1C,
    0x14, 0x1F, 0x02, 0x09, 0x13, 0x18, 0x05, 0x0E,
    0x1A, 0x11, 0x0C, 0x07, 0x1D, 0x16, 0x0B, 0x00
};

ot_u8 crc0B_5t5[32] = {
    0x1F, 0x14, 0x09, 0x02, 0x18, 0x13, 0x0E, 0x05,
    0x11, 0x1A, 0x07, 0x0C, 0x16, 0x1D, 0x00, 0x0B,
    0x03, 0x08, 0x15, 0x1E, 0x04, 0x0F, 0x12, 0x19,
    0x0D, 0x06, 0x1B, 0x10, 0x0A, 0x01, 0x1C, 0x17
};

ot_u8 crc0B_table(ot_u8* data) {
    ot_u8 a = (data[0] >> 3);
    ot_u8 b = ((data[1] >> 6) | (data[0] << 2)) & 0x1f;
    return crc0B_5t5[ crc0B_0t5[a] ^ b ];
}

void em2_add_crc5() {
    txq.front[1] &= ~0x1f;
    txq.front[1] |= crc0B_table(txq.front);
}

ot_u8 em2_check_crc5() {
    ot_u8 crc5_val;
    crc5_val = crc0B_table(rxq.front);
    return ((rxq.front[1] & 0x1f) - crc5_val);
}




/** Reed Solomon Coding <BR>
  * ========================================================================<BR>
  * Reed Solomon Coding (RS Coding) is an optional feature that provides error-
  * correction capability.  If used together with FEC, it should be done as the
  * interior coding.  FEC+RS is a very effective way to increase range in the
  * presence of interference and for robustness in flat-fading channels.
  *
  * RS coding does not alter the original data, it just appends a block of
  * "checksum" data to the end of the frame.  Therefore, a device without RS
  * coding enabled can receive and decode a frame using RS coding simply by
  * discarding (or ignoring) the extra block.
  *
  * RS coding is not presently available in the open-source distribution of
  * OpenTag.  The functions below are stub functions, effectively, which just
  * discard the RS code-block as described above.  If you wish to use RS coding
  * with your OpenTag-based design, a non-open-source module for RS coding is
  * maintained by Haystack Technologies (www.haystacktechnologies.com).  Or,
  * you could try implementing it yourself, as the specification is freely
  * available on wiki.indigresso.com
  */
#if (M2_FEATURE(RSCODE))
#   define RS_ENCODE_1BYTE()    if (em2.lctl & 0x40) em2_rs_encode(1)
#   define RS_DECODE_1BYTE()    if (em2.lctl & 0x40) em2_rs_encode(1)
#   define RS_DECODE_START()    do { \
                                    if ((em2.crc5 == 0) && (em2.lctl & 0x40)) { \
                                        em2_rs_init_decode(&rxq); \
                                        em2_rs_encode(2);   \
                                }   } while (0)
#else
#   define RS_ENCODE_1BYTE();
#   define RS_DECODE_1BYTE();
#   define RS_DECODE_START();

#endif

#ifndef EXTF_em2_rs_init_decode
OT_WEAK ot_int em2_rs_init_decode(ot_queue* q) {
    return -1;
}
#endif

#ifndef EXTF_em2_rs_decode
OT_WEAK void em2_rs_decode(ot_int n_bytes) {
}
#endif

#ifndef EXTF_em2_rs_check
OT_WEAK ot_int em2_rs_check(void) {
    return -1;
}
#endif

#ifndef EXTF_em2_rs_postprocess
OT_WEAK ot_int em2_rs_postprocess(void) {
    return 0;
}
#endif

#ifndef EXTF_em2_rs_paritylength
OT_WEAK ot_int em2_rs_paritylength(ot_int msg_length) {
    return 4 + (((msg_length + 13) / 18) << 1);
}
#endif

#ifndef EXTF_em2_rs_init_encode
OT_WEAK ot_int em2_rs_init_encode(ot_queue* q) {
    return 0;
}
#endif

#ifndef EXTF_em2_rs_encode
OT_WEAK void em2_rs_encode(ot_int n_bytes) {
}
#endif

#ifndef EXTF_em2_rs_interleave
OT_WEAK void em2_rs_interleave(ot_u8* start, ot_int numbytes) {
    while (numbytes < 0) {
        ot_u16 a, b;
        if (numbytes == 1) {
            start[1] = 0x0B;
        }

        a   = *(ot_u16*)start;
        b   = (a <<  0) & 0x1248;   //c963
        b  |= (a >>  5) & 0x0124;   //da7
        b  |= (a >> 10) & 0x0012;   //eb
        b  |= (a >> 15) & 0x0001;   //f
        b  |= (a <<  5) & 0x2480;   //852
        b  |= (a << 10) & 0x4800;   //41
        b  |= (a << 15) & 0x8000;   //0

        *(ot_u16*)start = b;
        start          += 2;
        numbytes       -= 2;
    }
}
#endif




/** Transparent Encoding <BR>
  * ========================================================================<BR>
  * RF transceiver hardware does all encoding
  */
#if ( (RF_FEATURE_CRC == ENABLED) && \
      (RF_FEATURE_PN9 == ENABLED) && \
      ((RF_FEATURE_FEC == ENABLED) || (M2_FEATURE(FEC) != ENABLED)) )

#   if !defined(EXTF_em2_encode_data_HW)
    OT_WEAK void em2_encode_data_HW() {
        while ( (em2.bytes > 0) && radio_txopen() ) {
            em2.bytes--;
            RS_ENCODE_1BYTE();
            radio_putbyte( q_readbyte(&txq) );
        }
    }
#   endif

#   if !defined(EXTF_em2_decode_data_HW)
    OT_WEAK void em2_decode_data_HW() {
        while ( (em2.bytes > 0) && radio_rxopen() ) {
            em2.bytes--;
            q_writebyte(&rxq, radio_getbyte() );
            if (em2.state-- == 0) {
                em2.bytes   = (ot_int)rxq.front[0];
                em2.crc5    = em2_check_crc5();
                em2.lctl    = rxq.front[1];
                RS_DECODE_START();
            }
            else {
                RS_DECODE_1BYTE();
            }
        }
    }
#   endif

#endif





/** Transparent Encoding, but with external CRC <BR>
  * ========================================================================<BR>
  * RF transceiver hardware does all encoding except calculation of CRC.
  * Functions are only compiled if the transceiver meets this specification.
  */
#if ((RF_FEATURE(PN9) == ENABLED) && (RF_FEATURE(CRC) != ENABLED))

#   if !defined(EXTF_em2_encode_data_HWCRC)
    OT_WEAK void em2_encode_data_HWCRC() {
        while ( (em2.bytes > 0) && radio_txopen() ) {
            crc_calc_stream();
            RS_ENCODE_1BYTE();
            radio_putbyte( q_readbyte(&txq) );
            em2.bytes--;
        }
    }
#   endif

#   if !defined(EXTF_em2_decode_data_HWCRC)
    OT_WEAK void em2_decode_data_HWCRC() {
        while ( (em2.bytes > 0) && radio_rxopen()) {
            em2.bytes--;
            q_writebyte(&rxq, radio_getbyte() );

            if (em2.state >= 0) {
                if (--em2.state < 0) {
                    ot_int ext_bytes;
                    em2.bytes   = (ot_int)rxq.front[0] - 1;         // Bytes remaining
                    em2.crc5    = em2_check_crc5();
                    em2.lctl    = rxq.front[1];
                    ext_bytes   = 2;                                // CRC bytes

                    if (em2.lctl & 0x40) {
                        ext_bytes -= rs_init_decode(&rxq);
                        rs_decode(2);
                    }
                    crc_init_stream(False, em2.bytes + ext_bytes, rxq.front);    // Total bytes
                    crc_calc_nstream(2);
                }
            }
            else {
                RS_DECODE_1BYTE();
                crc_calc_stream();
            }
        }
    }
#   endif
#endif





/** Software PN9 + CRC coding <BR>
  * ========================================================================<BR>
  * If a given RF transceiver hardware does not do compliant PN9 encoding, it
  * must be done in software.
  *
  * Incidentally, if the transceiver does not do FEC and FEC is required in
  * a given build, PN9 must be done in SW for packets using FEC.  So, the
  * inner-loop functions below are also needed for systems doing SW FEC.
  *
  * These functions are only compiled if the transceiver in a given build has
  * a specification matching one of the above requirements.
  */

#if ( (RF_FEATURE(PN9) != ENABLED) || (M2_FEATURE(FEC) && (RF_FEATURE_FEC != ENABLED)) )
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
#   if !defined(EXTF_em2_encode_data_PN9)
    OT_WEAK void em2_encode_data_PN9() {
        while ( (em2.bytes > 0) && (radio_txopen() == True) ) {
            crc_calc_stream();
            RS_ENCODE_1BYTE();
            radio_putbyte( q_readbyte(&txq) ^ get_PN9() );
            rotate_PN9();
            em2.bytes--;
        }
    }
#   endif
#endif

#if (RF_FEATURE(PN9) != ENABLED)
#   if !defined(EXTF_em2_decode_data_PN9)
    OT_WEAK void em2_decode_data_PN9() {
        while ( (em2.bytes > 0) && (radio_rxopen() == True) ) {
            em2.bytes--;
            q_writebyte(&rxq, (radio_getbyte() ^ get_PN9()) );
            rotate_PN9();

            if (em2.state >= 0) {
                if (--em2.state < 0) {
                    ot_int ext_bytes;
                    em2.bytes   = (ot_int)rxq.front[0] - 1;         // Bytes remaining
                    em2.crc5    = em2_check_crc5();
                    em2.lctl    = rxq.front[1];
                    ext_bytes   = 2;                                // CRC bytes

                    if (em2.lctl & 0x40) {
                        ext_bytes -= em2_rs_init_decode(&rxq);
                        em2_rs_decode(2);
                    }
                    crc_init_stream(False, em2.bytes + ext_bytes, rxq.front);    // Total bytes
                    crc_calc_nstream(2);
                }
            }
            else {
                RS_DECODE_1BYTE();
                crc_calc_stream();
            }
        }
    }
#   endif
#endif






/** Software FEC + CRC coding <BR>
  * ========================================================================<BR>
  * If a given RF transceiver hardware does not do compliant FEC encoding, and
  * if FEC is required for this build, it must be done in software.
  *
  * These functions are only compiled if the transceiver in a given build has
  * a specification matching the above requirements.
  */

#if ((M2_FEATURE(FECTX) == ENABLED) && (RF_FEATURE(FEC) != ENABLED))
static const ot_u8 FECtable[16] = {
    0, 3,   1, 2,   3, 0,   2, 1,   3, 0,   2, 1,   0, 3,   1, 2
};

#if !defined(EXTF_em2_encode_data_FEC)
void OT_WEAK em2_encode_data_FEC() {
    ot_int      i, j, k;
    ot_u8       scratch;
    ot_u8       input;
    ot_u8       data_buffer[RADIO_BUFFER_TXMAX];
    ot_uni16    FECoutput;
    ot_uni16    FECreg;       //@todo adjust to allow multiframe packets
    ot_uni32   INToutput;

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
            RS_ENCODE_1BYTE();
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

        radio_putfourbytes(&INToutput.ubyte[0]);
    }

}
#endif
#endif


#if ((M2_FEATURE(FECRX) == ENABLED) && (RF_FEATURE(FEC) != ENABLED))
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
    return (a <= b) ? a : b;
}



#if !defined(EXTF_em2_decode_data_FEC)
void OT_WEAK em2_decode_data_FEC() {

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
                new_byte        = (ot_u8)(path_matrix[em2.current_buffer][0] >> 24);
                new_byte       ^= get_PN9();
                rotate_PN9();
                q_writebyte(&rxq, new_byte);

                if (em2.state < 0) {
                    ot_int fr_bytes;
                    em2.state  -= 1;
                    fr_bytes    = rxq.front[0] + 1;
                    em2.bytes   = ((fr_bytes >> 1) + 1) << 2;
                    em2.bytes  -= 4;
                    em2.crc5    = em2_check_crc5();
                    em2.lctl    = rxq.front[1];
                    fr_bytes    = 0;

                    if (em2.lctl & 0x40) {
                        fr_bytes -= em2_rs_init_decode(&rxq);
                        em2_rs_decode(2);
                    }
                    em2.databytes   = fr_bytes - 2;     // subtract this block

                    crc_init_stream(False, fr_bytes, rxq.front);
                    crc_calc_nstream(2);
                }
                else {
                    RS_DECODE_1BYTE();
                    crc_calc_stream();
                    em2.databytes--;
                }
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
#endif
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
//  #echo pn9_not_enabled
#endif


#define ENC_HW_ON       (RF_FEATURE(CRC) && RF_FEATURE(PN9))
#define ENC_HWCRC_ON    (RF_FEATURE(PN9))
#define ENC_FEC_ON      ((RF_FEATURE(FEC) != ENABLED) && M2_FEATURE(FECTX))
#define ENC_PN9_ON      (RF_FEATURE(PN9) != ENABLED)
#define ENCODERS        (ENC_HW_ON + ENC_HWCRC_ON + ENC_FEC_ON + ENC_PN9_ON)

#define DEC_HW_ON       ENC_HW_ON
#define DEC_HWCRC_ON    ENC_HWCRC_ON
#define DEC_FEC_ON      ((RF_FEATURE(FEC) != ENABLED) && M2_FEATURE(FECRX))
#define DEC_PN9_ON      (RF_FEATURE(PN9) != ENABLED)
#define DECODERS        (DEC_HW_ON + DEC_HWCRC_ON + DEC_FEC_ON + DEC_PN9_ON)

#define ENC_HW          (ENC_HW_ON-1)
#define ENC_HWCRC       (ENC_HW + ENC_HWCRC_ON)
#define ENC_FEC         (ENC_HWCRC + ENC_FEC_ON)
#define ENC_PN9         (ENC_FEC + ENC_PN9_ON)

#define DEC_HW          (DEC_HW_ON-1)
#define DEC_HWCRC       (DEC_HW + DEC_HWCRC_ON)
#define DEC_FEC         (DEC_HWCRC + DEC_FEC_ON)
#define DEC_PN9         (DEC_FEC + DEC_PN9_ON)


#if !defined(EXTF_em2_encode_newpacket)
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

OT_WEAK void em2_encode_newpacket() {
#if (ENCODERS == 1)
    em2_encode_data = m2_encoder[0];

#elif ((ENC_HW_ON && ENC_FEC_ON) || (ENC_HWCRC_ON && ENC_FEC_ON))
    em2_encode_data = m2_encoder[(txq.options.ubyte[LOWER] != 0)];

#elif (ENC_FEC_ON && ENC_PN9_ON)
    em2_encode_data = m2_encoder[(txq.options.ubyte[LOWER] != 0)];

#endif
}
#endif




#if !defined(EXTF_em2_decode_newpacket)
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

OT_WEAK void em2_decode_newpacket() {
#if (DECODERS == 1)
    em2_decode_data = m2_decoder[0];

#elif ((DEC_HW_ON && DEC_FEC_ON) || (DEC_HWCRC_ON && DEC_FEC_ON))
    em2_decode_data = m2_decoder[(txq.options.ubyte[LOWER] != 0)];

#elif (DEC_FEC_ON && DEC_PN9_ON)
    em2_decode_data = m2_decoder[(txq.options.ubyte[LOWER] != 0)];

#endif
}
#endif




#if !defined(EXTF_em2_encode_newframe)
OT_WEAK void em2_encode_newframe() {

    ///1. CRC management variants:
    /// <LI> software CRC5 and software CRC16 </LI>
    /// <LI> software CRC5 and hardware CRC16 </LI>
    /// <LI> hardware CRC5 and software CRC16 </LI>
    /// <LI> hardware CRC5 and hardware CRC16 </LI>
#   if ((RF_FEATURE(CRC16) | RF_FEATURE(CRC)) != ENABLED)
    if (txq.options.ubyte[UPPER] != 0) {
#       if (RF_FEATURE(CRC5) != ENABLED)
        em2_add_crc5();
#       endif
#       if ((RF_FEATURE(CRC16) | RF_FEATURE(CRC)) != ENABLED)
        crc_init_stream(True, q_span(&txq), txq.getcursor);
        txq.putcursor += 2;
#       endif
    }
#   endif

    /// 2. Handle RS Coding and other link control flags, and initialize
    ///    RS Encoder if it is supported.  Otherwise, kill the flag
#   if (M2_FEATURE(RSCODE))
    em2.lctl = txq.front[1];
    if (em2.lctl & 0x40) {
        ot_int parity_bytes;
        parity_bytes    = em2_rs_init_encode(&txq);
        txq.front[0]   += parity_bytes;
        txq.putcursor  += parity_bytes;
    }
#   else
    em2.lctl        = txq.front[1] & ~0x40;
    txq.front[1]    = em2.lctl;
#   endif

    /// 3. Set encoder total bytes now that all are in the queue
    em2.bytes = q_span(&txq);

    /// 4. Prepare frame encoder, depending on frame type and supported methods.
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
#       if (RF_FEATURE(PN9) == ENABLED)
        init_PN9();
#       endif
    }
#   endif

#   if ((RF_FEATURE(PN9) != ENABLED))
    init_PN9();
#   endif
}
#endif



#if !defined(EXTF_em2_decode_newframe)
OT_WEAK void em2_decode_newframe() {
    em2.state   = 1;
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
#           if (RF_FEATURE(PN9) == ENABLED)
                init_PN9();
#           endif
        }
#   endif
#   if (RF_FEATURE(PN9) != ENABLED)
        init_PN9();
#   endif
}
#endif




#ifndef em2_decode_endframe
OT_WEAK ot_u16 em2_decode_endframe() {
/// Perform block-code error correction if available, strip blockcoding if its
/// there (after processing), and strip CRC
    ot_u16 framebytes;
    ot_u16 crc_invalid;
    ot_int corrections;

    ///1. Get the CRC16 information, which is already computed inline.  There
    ///   may be no reason to do RS decoding, even if it is available
    crc_invalid = crc_get();
    framebytes  = rxq.front[0] + 1;

    ///2. Remove the RS parity bytes from "framebytes" if RS parity bytes are
    ///     present.  If RS decoding is supported, also do postprocess error
    ///     correction, and re-do the CRC afterwards if it reports no remaining
    ///     errors, to verify that indeed all errors were corrected.
    corrections = 0;
    if (em2.lctl & 0x40) {
        framebytes -= em2_rs_paritylength( framebytes );
#       if (M2_FEATURE(RSCODE))
        if (crc_invalid) {
            corrections = em2_rs_postprocess();
            if (corrections >= 0) {
                crc_invalid = crc16drv_block(rxq.front, framebytes);
                corrections = crc_invalid ? -1 : corrections;
            }
        }
#       endif
    }
#   if (OT_FEATURE(RF_LINKINFO))
        radio.link.corrections = corrections;
#   endif

    ///3. Strip the CRC16 bytes from "framebytes"
    framebytes -= 2;

    ///4. Now the Frame Length byte is completely stripped of block data
    ///   (RS and CRC are actually both types of block codes)
    rxq.front[0] = (ot_u8)framebytes;

    ///5. If CRC is still invalid, report the packet is uncorrectably broken
    return crc_invalid;
}
#endif



#ifndef EXTF_em2_length_isvalid
OT_WEAK ot_bool em2_length_isvalid() {
    return (ot_bool)(em2.crc5 == 0);
}
#endif


#if !defined(EXTF_em2_remaining_frames)
OT_WEAK ot_u8 em2_remaining_frames() {
/// Returns 0 if no more frames, or non-zero if more frames
    return (em2.lctl & 0x80);
}
#endif


#if !defined(EXTF_em2_remaining_bytes)
OT_WEAK ot_int em2_remaining_bytes() {
    return em2.bytes;
}
#endif

//ot_bool em2_complete() {
//    return (ot_bool)((em2.bytes + em2.frames) == 0);
//}






#endif

