/* Copyright 2016 JP Norair
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
  */
/**
  * @file       /io/stm32wl_lora/radio_rm2_patch.c
  * @author     JP Norair
  * @version    R100
  * @date       31 Jan 2022
  * @brief      Patch functions used with STM32WL
  * @ingroup    Radio
  *
  ******************************************************************************
  */

#include <otstd.h>
#if (OT_FEATURE(M2) == ENABLED)

// Local header for supporting this patch, or other patches
#include "radio_rm2.h"
#include <m2/encode.h>
#include <otlib/memcpy.h>

#if (M2_FEATURE(RSCODE))
#   error "Reed Solomon not implemented on this interface."
#elif (M2_FEATURE(LDPC))
#   error "LDPC not implemented on this interface."
#elif (M2_FEATURE(MPCODE))
#   include <hblib/mpcode.h>
#   include <hblib/lorallr.h>

    ///@todo mpc_iobuf might be changeable to &busrx[16], 
    ///and it doesn't really need to be on heap.
    static uint8_t      mpc_iobuf[16];

    extern mpc_t        mpc;
    extern lorallr_t    lorallr;
    extern radio_snr_t  loralink;

#else
    typedef enum {
        LoraSF11    = 0,
        LoraSF9     = 1,
        LoraSF7     = 2,
        LoraSF5     = 3
    } lorasf_t;

#endif
//static volatile ot_int hits;





/** Replacement Timing and Packet Duration functions <BR>
  * ========================================================================<BR>
  * rm2_rxtimeout_floor
  * rm2_default_tgd
  * rm2_txpkt_duration
  * rm2_bgpkt_duration
  * rm2_scale_codec
  */

typedef ot_uint (*converter_fn)(ot_int, ot_int);

static ot_uint sub_bytes_to_symbols_sf5(ot_int pkt_bytes, ot_int codesize) {
    return 8 + ( ((8*pkt_bytes - 12 + 19) / 20) * codesize );
}
static ot_uint sub_bytes_to_symbols_sf7(ot_int pkt_bytes, ot_int codesize) {
    return 8 + ( ((8*pkt_bytes - 20 + 27) / 28) * codesize );
}
static ot_uint sub_bytes_to_symbols_sf9(ot_int pkt_bytes, ot_int codesize) {
    return 8 + ( ((8*pkt_bytes - 28 + 35) / 36) * codesize );
}
static ot_uint sub_bytes_to_symbols_sf11(ot_int pkt_bytes, ot_int codesize) {
    return 8 + ( ((8*pkt_bytes - 36 + 43) / 44) * codesize );
}




static ot_uint sub_symbols_to_ti(ot_int symbols, ot_uint miti_per_sym) {
    ot_ulong miti;
    miti    = symbols * miti_per_sym;
    miti   += 1023;
    return (ot_uint)(miti >> 10);
}


///@todo must account for different regimes
ot_uint rm2_rxtimeout_floor(ot_u8 chan_id) {
/// Timeout floor is the duration of one bg packet, rounded-up
    static const ot_u8 bgpkt_ti_lut[16] = {
               98, // 0000: No FEC, No extra coding, SF11
               25, // 0001: No FEC, No extra coding, SF9
                7, // 0010: No FEC, No extra coding, SF7
                2, // 0011: No FEC, No extra coding, SF5
                0, // 0100: No FEC, extra coding, SF11      (presently unspecified)
                0, // 0101: No FEC, extra coding, SF9       (presently unspecified)
                0, // 0110: No FEC, extra coding, SF7       (presently unspecified)
                0, // 0111: No FEC, extra coding, SF5       (presently unspecified)
              111, // 1000: FEC, No extra coding, SF11
               28, // 1001: FEC, No extra coding, SF9
                9, // 1010: FEC, No extra coding, SF7
                3, // 1011: FEC, No extra coding, SF5
                0, // 1100: FEC, extra coding, PAGE         (presently unspecified)
                0, // 1101: FEC, extra coding, SF9          (presently unspecified)
                0, // 1110: FEC, extra coding, SF7          (presently unspecified)
                0, // 1111: FEC, extra coding, SF5          (presently unspecified)
        };
    return (ot_uint)bgpkt_ti_lut[ (chan_id>>4) & 0x0F ];
}

///@todo must account for different regimes
ot_uint rm2_default_tgd(ot_u8 chan_id) {
/// Tgd is the duration of 32 frame bytes (per DASH7 specification).
/// In the present DASH7+LoRa implementation, the BW is always 500kHz and the coding rate either 4/5 or 4/8
/// SF11: 2.15 kbps     : 120.0 ti   (255)  -- limited to 255
/// SF9:  5.022 kbps    : 52.18 ti   (53)
/// SF7:  15.625 kbps   : 16.77 ti   (17)
/// SF5:  ~48 kbps      : 5.46 ti    (6)    -- baudrate estimated
  
    static const ot_u8 tgdti_lut[16] = {
          120, // 0000: No FEC, No extra coding, SF11
           53, // 0001: No FEC, No extra coding, SF9	
           17, // 0010: No FEC, No extra coding, SF7
            6, // 0011: No FEC, No extra coding, SF5
            0, // 0100: No FEC, extra coding, SF11      (presently unspecified)
            0, // 0101: No FEC, extra coding, SF9       (presently unspecified)
            0, // 0110: No FEC, extra coding, SF7       (presently unspecified)
            0, // 0111: No FEC, extra coding, SF5       (presently unspecified)
          192, // 1000: FEC, No extra coding, SF11
           84, // 1001: FEC, No extra coding, SF9
           27, // 1010: FEC, No extra coding, SF7
            9, // 1011: FEC, No extra coding, SF5
            0, // 1100: FEC, extra coding, SF11         (presently unspecified)
            0, // 1101: FEC, extra coding, SF9          (presently unspecified)
            0, // 1110: FEC, extra coding, SF7          (presently unspecified)
            0, // 1111: FEC, extra coding, SF5          (presently unspecified)
    };
    
    return (ot_uint)tgdti_lut[ (chan_id>>4) & 0x0F ];
}



ot_uint rm2_txpkt_duration(ot_queue* pkt_q) {
    ///@note this is a little bit of a hack, using em2.state, but it's OK for this hacky implementation
    return rm2_scale_codec( phymac[0].channel, em2.state );
}


ot_uint rm2_bgpkt_duration() {
    return rm2_rxtimeout_floor(phymac[0].channel);
}


///@todo this entire function might be done in a direct lookup table, even if in blocks.
ot_uint rm2_scale_codec(ot_u8 channel_code, ot_uint buf_bytes) {
/// Turns a number of bytes (buf_bytes) into a number of ti units.

    static const converter_fn converter[4] = {
        &sub_bytes_to_symbols_sf11,     /// 0000 -> SF11
        &sub_bytes_to_symbols_sf9,      /// 0001 -> SF9
        &sub_bytes_to_symbols_sf7,      /// 0010 -> SF7
        &sub_bytes_to_symbols_sf5,      /// 0011 -> SF5
    };

    ot_int codesize;
    ot_uint miti_per_sym;

    codesize        = (channel_code & 0x80) ? 4 : 5;
    channel_code    = (channel_code >> 4) & 3;
    miti_per_sym    = wllora_symbol_miti(phymac[0].flags, channel_code);
    buf_bytes       = converter[channel_code](buf_bytes, codesize);
    
    // + Also add Preamble & Sync Overhead
    // + Also add Explicit Header, if enabled (always 8 symbols)
    // + Also add 4 or 5 symbols because LoRa TX-END is late-arriving by one symbol group
    
    // Original: implicit mode
    //buf_bytes  += RF_PARAM_PKT_OVERHEAD + codesize;

    // Explicit mode
    buf_bytes  += RF_PARAM_PKT_OVERHEAD + 8 + codesize;

    return sub_symbols_to_ti(buf_bytes, miti_per_sym);
}







/** Replacement Encoder functions <BR>
  * ========================================================================<BR>
  * The Encoder/Decoder module in M2 module provides some generic encode and
  * decode functions.  They work for most radios.  The SX127x implementation
  * has these optimized encode and decode functions.
  *
  */

void em2_encode_newpacket() {
}





void em2_encode_newframe() {
/// LoRa doesn't use the CRC5 header (it has its own), nor does it utilize the
/// length byte at rxq.front[0].  txq.getcursor should start at txq.front[2]
/// LoRa also requires encoding the entire frame/packet at once, because it 
/// does not adequately support FIFO streaming.
#   if (M2_FEATURE(MPCODE))
    static const ot_u8 frparams[8][3] = {
        {1, 0, 0},              // b000 : FG, unencoded, CRC=2, offset=0
        {0, 2, 0},              // b001 : BG, unencoded, CRC=1, offset=2
        {1, 0, (ot_u8)RATE64},  // b010 : FG, encoded, CRC=2, offset=0, rate=64
        {0, 2, (ot_u8)RATE48},  // b011 : BG, encoded, CRC=1, offset=2, rate=48
        {1, 2, 0},              // b100 : PG, unencoded, CRC=2, offset=2
        {0, 2, (ot_u8)RATE48},  // b101 : PG/BG (impossible)
        {1, 2, (ot_u8)RATE64},  // b110 : PG, encoded, CRC=2, offset=2, rate=64
        {0, 2, (ot_u8)RATE48},  // b111 : PG/BG encoded (impossible)
    };
#   else
    static const ot_u8 frparams[8][2] = {
        {1, 0},             // b000 : FG, unencoded, CRC=2, offset=0
        {0, 2},             // b001 : BG, unencoded, CRC=1, offset=2
        {1, 0},             // b010 : FG, encoded, CRC=2, offset=0, rate=64
        {0, 2},             // b011 : BG, encoded, CRC=1, offset=2, rate=48
        {1, 2},             // b100 : PG, unencoded, CRC=2, offset=2
        {0, 2},             // b101 : PG/BG (impossible)
        {1, 2},             // b110 : PG, encoded, CRC=2, offset=2, rate=64
        {0, 2},             // b111 : PG/BG encoded (impossible)
        };
#   endif
    ///@todo this retransmission case is broken since state was reassigned to bit counter
    /// Case where this is a retransmission.  Don't re-encode.
    if (txq.options.ubyte[UPPER] == 0) {
        em2.bytes = em2.state;
    }

    /// Case where this is not a retransmission.  Encode.
    else {
        ot_u8 selector;
        ot_uint bytes_to_crc;

        selector = (rfctl.flags & 5) | ((phymac[0].channel >> 6) & 2);
        ///@todo need to clean up this section for correct offsets
        ///@note txq.front[1] normally contains three things.
        /// 1. Listen bit   -- for multiframing, not supported on LoRa
        /// 2. RS Code bit  -- not supported in this impl
        /// 3. CRC5         -- used only with unencoded FG frames
        txq.options.ubyte[LOWER]= frparams[selector][0];
        txq.getcursor           = txq.front + frparams[selector][1];
        bytes_to_crc            = q_span(&txq);
        txq.front[0]            = bytes_to_crc + frparams[selector][0];
        txq.front[1]            = 0;
        txq.putcursor          += (frparams[selector][0] + 1);
        em2.bytes               = bytes_to_crc + (frparams[selector][0] + 1);

        ///@todo maybe this could be put at the end of the function?
        if ((selector & 1) == 0) {
            crc_init_stream(&em2.crc, True, bytes_to_crc, txq.getcursor);
        }

        /// Frame CRC for PG and FG frames
        /// Header CRC for unencoded FG frames
        if (selector == 0) {
            em2_add_crc5();
        }

#       if (M2_FEATURE(MPCODE))
        ///@todo This implementation doesn't yet support dynamic rates in TX
        /// MPCode supports dynamic rates in FG frames.
        /// The first block is always RATE48 (BG), or RATE64 (PG, FG).
		else if (selector & 2) {
		    if (selector & 1) {
		        txq.front[1]= frparams[selector][2];
		        em2.bytes   = 16;
		    }
		    else if (selector & 4) {
		        txq.front[1]= frparams[selector][2];
		        em2.bytes   = 32;
		    }
		    else {
		        ///@todo this line below is what would change with dynamic rate feature
		        txq.front[1]= RATE64;

		        em2.bytes   = 16;
		        em2.bytes  += mpc_octetsinframe(txq.front[1], q_span(&txq)-8);
		    }

		    mpc_init(&mpc, frparams[selector][2]);
		}
#       endif

		/// Save amount of encoded bytes for this frame (last frame) in state
		em2.state = em2.bytes;
    }
}


void em2_encode_data(void) {
/// Only encode if this if not a retransmission
    if (txq.options.ubyte[UPPER] != 0) {

        /// Do CRC of the long frame (FG/PG) or the short frame (BG)
        if (txq.options.ubyte[LOWER] != 0) {
            crc_calc_nstream(&em2.crc, em2.crc.count);
        }
        else {
            bgcrc8_put(txq.getcursor);
        }

#       if (M2_FEATURE(MPCODE))
        if (phymac[0].channel & 0x80) {
            while (em2.bytes > 0) {
                mpc_encode(&mpc, mpc_iobuf, q_markbyte(&txq, mpc.infobytes));
                mpc_init(&mpc, txq.front[1]&7);
                wllora_burstwrite(RFREG_LR_FIFO, 16, mpc_iobuf);
                em2.bytes -= 16;
            }
        }
        else
#       endif
        {
            while (em2.bytes > 0) {

                ///@todo Saturation intrinsic here?
                ot_int fill = (em2.bytes > 16) ? 16 : em2.bytes;

                wllora_wrbuf_cmd(0, fill, q_markbyte(&txq, fill));
                em2.bytes -= fill;
            }
        }
    }
}






//ot_u8 rfcore_dump[128];

#ifdef __DECODER_DEBUG__
volatile ot_u8 rxbytes_sv[16];
volatile ot_u8 rxwb_sv[16];
volatile ot_u8 rxnb_sv[16];
volatile ot_u16 tim_sv[16];
volatile ot_u16 bits_sv[16];
static ot_uint hits_sv = 0;
#endif


void em2_decode_newpacket() {
/// Lora cannot support multiframe packets, so there is no difference between
/// packet and frame
}

void em2_decode_newframe() {
    static const ot_u8 bits_per_lorablock[4] = {
            44, 36, 28, 20
    };
    static const ot_u8 initial_bytes[8] = {
            7,  // b000 : FG, unencoded,
            5,  // b001 : BG, unencoded,
            15, // b010 : FG, encoded
            7,  // b011 : BG, encoded
            15, // b100 : PG, unencoded
            5,  // b101 : PG/BG (impossible)
            17, // b110 : PG, encoded
            17  // b111 : PG/BG encoded (impossible)
    };
    uint8_t selector;
    lorasf_t chansf = (phymac[0].channel >> 4) & 3;

#   ifdef __DECODER_DEBUG__
    hits_sv = 0;
#   endif

    /// 1. Prepare the RX Queue to receive a new frame.
    q_empty(&rxq);

    /// The selector value is
    selector = (rfctl.flags & 5) | ((phymac[0].channel >> 6) & 2);

    /// In MPCODE FEC Mode, we need to start logging RSSI and SNR in order to
    /// provide seed information for the LLR interpolator.  Also, we need to provide
    /// default decoding setup -- lower 3 bits of front[1] are used for variable
    /// rate information.  The first frame is always RATE64, unless it's a BG frame
    /// in which case it's RATE48.
#   if (M2_FEATURE(MPCODE))
    if (selector & 2) {
        ot_u8 rssi_nb   = wllora_read(RFREG_LR_RSSIVALUE);
        ot_u8 rssi_wb   = wllora_read(RFREG_LR_RSSIWIDEBAND);
    	mpcrate_t rate  = (selector & RADIO_FLAG_BG) ? RATE48 : RATE64;

#       ifdef __DECODER_DEBUG__
        tim_sv[hits_sv] = LPTIM1->CNT;
        bits_sv[hits_sv] = 0;
        rxbytes_sv[hits_sv] = 0;
        rxwb_sv[hits_sv] = rssi_wb;
        rxnb_sv[hits_sv] = rssi_nb;
        hits_sv++;
#       endif

    	// A new frame = a new sync word.  Save the RSSI and SNR from the sync
    	lorallr_init(&lorallr, rate, chansf, rssi_nb, rssi_wb);
    	mpc_init(&mpc, rate);
    	rxq.front[1] = rate; // initial encoding rate
    }
    else
#   endif
    {
        rxq.front[1] = 0;
    }

    rxq.front[0]    = initial_bytes[selector];
    em2.bytes       = 0;
    em2.state       = 0;
    em2.lctl        = bits_per_lorablock[chansf];
    em2.crc5        = 0;

    /// BG and PG frames are fixed-length, and they are shoehorned into the
    /// generic header structure via a special operation, here.
    if (selector & (RADIO_FLAG_BG | RADIO_FLAG_PG)) {
        rxq.front[1]       |= 0x20;
        rxq.options.ushort  = 2;
        rxq.putcursor       = &rxq.front[2];
        rxq.getcursor       = &rxq.front[2];
    }
}




void em2_decode_data(void) {
/// This is the live decoding implementation of LoRa
///@note em2.bytes is used as bits.
    ot_u8 rxbytes;
    //ot_u8 rxptr;
    //lr_rxbufstatus_t rxstatus;

    int newbytes;
    int grab;
    ot_u8* data;

    /// Live encoder needs to track the predicted bit position over the air
    em2.state += em2.lctl;

    /// 1. When using MPCODE FEC, we need to take the RSSI and SNR values and log them.
    ///    It's important to do this as close to the time of reception as possible.
#	if (M2_FEATURE(MPCODE))
    if (phymac[0].channel & 0x80) {
        ot_u8 rssi_wb  = wllora_read(RFREG_LR_RSSIWIDEBAND);
        ot_u8 rssi_nb  = wllora_read(RFREG_LR_RSSIVALUE);

#       ifdef __DECODER_DEBUG__
        rxwb_sv[hits_sv] = rssi_wb;
        rxnb_sv[hits_sv] = rssi_nb;
        hits_sv++;
#       endif

        ///@todo inspect LPTIM counter between hits in order to get estimate on bits over
        ///      air since the last call.  Now just looking at lctl.
        lorallr_pushsample(&lorallr, rssi_nb, rssi_wb, em2.lctl);
    }
#   endif

    /// 2. Just download whatever fresh data is in the LoRa radio and put it
    ///    on the rx queue.  After data is on the rx queue, it will be reprocessed
    ///    if it needs to be.  The implementation here has a 32 byte SPI buffer.
    //rxbytes     = wllora_read(RFREG_LR_FIFORXBYTEADDR);
    //rxptr       = wllora_read(RFREG_LR_FIFOADDRPTR);
    //newbytes    = (int)rxbytes - (int)rxptr;

    ///@todo need to inspect some registers for intermediate buffer size, perhaps 0042
    //rxstatus = wllora_rxbufstatus_cmd();
    //newbytes = rxstatus.payload_len - em2.bytes;

    rxbytes = wllora_rdreg(LR_RXADRPTR);
    newbytes = rxbytes - em2.bytes;

#   ifdef __DECODER_DEBUG__
    rxbytes_sv[hits_sv] = rxbytes;
    tim_sv[hits_sv] = LPTIM1->CNT;
    bits_sv[hits_sv] = em2.state;
    hits_sv++;
#   endif

    if (newbytes <= 0) {
        return;
    }

    // rxbyte-addr is at the last loaded byte, not ahead of the last loaded byte,
    // so need to increment newbytes.
    data = rxq.front + rxq.options.ushort + em2.bytes;
	while (newbytes > 0) {

	    ///@todo saturation intrinsic
		grab = (newbytes > 32) ? 32 : newbytes;

		wllora_rdbuf_cmd(0, grab, data);

		newbytes   -= grab;
		em2.bytes  += grab;
		data       += grab;
	}
    
    /// 3. Raw data is in the queue.  If frame is unencoded, nothing more to do.
    ///    Encoded frames, on the other hand, require a substantial amount of
    ///    processing.  Decoding is block-based and occurs when a block is
    ///    available in the queue.  Blocks are 16 bytes.
#   if (M2_FEATURE(MPCODE))
	if (phymac[0].channel & 0x80) {
		uint8_t blockcrc;
		ot_int blcursor;

		///@todo could swap this for lorallr_getblock() and checking the NULL return
		blcursor    = rxq.getcursor - (rxq.front + rxq.options.ushort);
		em2.crc5   += (em2.bytes >= 16);

		if ((em2.bytes-blcursor) >= 16) {
		    sys.task_RFA.event = 6;
		    sys_preempt(&sys.task_RFA, 0);
		}
	}
	else
#   endif

	{
	    rxq.putcursor = rxq.getcursor + em2.bytes;

	    ///@todo Checking a header bit to see if CRC must be checked.
	    ///      Not sure this really works, but need it for testing now.
	    if (((rxq.front[1] & 0x20) == 0) && (em2.bytes >= 2)) {
            rxq.front[1] ^= 0x20;
            if (em2_check_crc5() != 0) {
                em2.state = -1;
            }
        }
	}
}



//#ifdef __DECODER_BERANALYSIS__
//static const ot_u8 knowndata_sv[32] = {
//    0x44, 0xD2, 0x74, 0x1D, 0x11, 0xB4, 0xDE, 0x8B, 0x44, 0x2D, 0xB8, 0x2E, 0x11, 0x87, 0x12, 0x74,
//    0xCC, 0xA0, 0x2E, 0x42, 0x65, 0x9F, 0xDD, 0xD7, 0x27, 0xDD, 0x9F, 0x95, 0x17, 0xB7, 0x5F, 0xFF
//};
//static ot_u8 rawdata_sv[32];
//#endif

ot_u16 em2_decode_endframe() {
/// Perform block-code error correction if available, strip blockcoding if its
/// there (after processing), and strip CRC
    ot_u16 crc_invalid;

    /// Decoder state errors (state < 1)
    if (em2.state < 1) {
        crc_invalid = 1;
    }
    else {
        /// In FEC mode, need to clean-up rxq after successful packet received
#       if (M2_FEATURE(MPCODE))
        if (phymac[0].channel & 0x80) {
            rxq.getcursor   = rxq.front + rxq.options.ushort;
            rxq.putcursor   = rxq.front + rxq.front[0] + 1;
        }
        else
#       endif
        {
        }

        /// BG Frame has CRC8
        if (rfctl.flags & RADIO_FLAG_BG) {
            crc_invalid = bgcrc8_check(rxq.getcursor);
        }
        else {
            /// PG, FG frames have CRC16
            crc_invalid     = crc_block(&em2.crc, False, q_span(&rxq), rxq.getcursor);
            rxq.front[0]   -= 2;
            rxq.putcursor  -= 2;
        }
        rxq.back = rxq.putcursor;

        ///@note marker that shows packet is successfully read into driver.
        rxq.front[1] |= 0x20;
    }

//#   ifdef __DECODER_BERANALYSIS__
//    loralink.max_ebn0 = 0;
//    for (int i=0; i<32; i++) {
//        ot_u8 diffbits  = knowndata_sv[i] ^ rawdata_sv[i];
//        ot_u8 errs;
//        errs    = ((diffbits & 0x80) != 0);
//        errs   += ((diffbits & 0x40) != 0);
//        errs   += ((diffbits & 0x20) != 0);
//        errs   += ((diffbits & 0x10) != 0);
//        errs   += ((diffbits & 0x08) != 0);
//        errs   += ((diffbits & 0x04) != 0);
//        errs   += ((diffbits & 0x02) != 0);
//        errs   += ((diffbits & 0x01) != 0);
//        loralink.max_ebn0 += errs;
//    }
//#   endif

    rxq.options.ushort = 0;
    return crc_invalid;
}




#endif  //if (OT_FEATURE(M2) == ENABLED)
