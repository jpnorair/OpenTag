/* Copyright 2010-2011 JP Norair
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
  * @file       /otradio/null/radio_NULL.c
  * @author     JP Norair
  * @version    V1.0
  * @date       13 Oct 2011
  * @brief      Radio Driver (RF transceiver) for NULL Radio
  * @defgroup   Radio (Radio Module)
  * @ingroup    Radio
  *
  * "NULL Radio" is for testing purposes.  It does not actually map to hardware,
  * but it does all the processes that a normal driver does.
  *
  * The header file for this implementation is /otlib/radio.h.  It is universal
  * for all platforms, even though the implementation (this file) can differ.
  * There is also a header file at /otradio/null/radio_null.h that includes
  * additional settings.
  *
  ******************************************************************************
  */

#include "OT_types.h"
#include "OT_config.h"
#include "OT_utils.h"
#include "OT_platform.h"

#include "radio.h"
#include "m2_encode.h"
#include "crc16.h"
#include "buffers.h"
#include "queue.h"
#include "veelite.h"
#include "session.h"
#include "system.h"

#include "radio_NULL.h"
#include "NULL_interface.h"




/** PHY-MAC Array declaration
  * Described in radio.h of the OTlib.
  * This driver only supports M2_PARAM_MI_CHANNELS = 1.
  */
phymac_struct       phymac[M2_PARAM_MI_CHANNELS];
radio_struct        radio;
null_radio_struct   null_radio;



/** Local Subroutine Prototypes  <BR>
  * ========================================================================<BR>
  */
typedef enum {
    SYNC_bg = 0,
    SYNC_fg = 2
} sync_enum;

void    subnull_kill(ot_int main_err, ot_int frame_err);
void    subnull_killonlowrssi();
void 	subnull_reset_autocal();

ot_u8   subnull_rssithr_calc(ot_u8 input, ot_u8 offset); // Fiddling necessary
ot_bool subnull_cca_init();
ot_bool subnull_chan_scan( );
ot_bool subnull_cca_scan();

ot_bool subnull_channel_lookup(ot_u8 chan_id, vlFILE* fp);
void    subnull_syncword_config(ot_u8 sync_class);
void    subnull_buffer_config(ot_u8 mode, ot_u8 param);
void    subnull_chan_config(ot_u8 old_chan, ot_u8 old_eirp);

void    subnull_set_txpwr(ot_u8 eirp_code);
void    subnull_prep_q(Queue* q);
ot_int  subnull_eta(ot_int next_int);
ot_int  subnull_eta_rxi();
ot_int  subnull_eta_txi();
void    subnull_offset_rxtimeout();






/** Null RF Virtual Interrupt Handler  <BR>
  * ========================================================================<BR>
  */
void radio_isr(void) {
    switch (null_radio.imode) {
        case MODE_listen:   rm2_rxsync_isr();   break;
        case MODE_rxdata:   rm2_rxdata_isr();   break;
        case MODE_rxend:    rm2_rxend_isr();    break;
        case MODE_txcsma:   break;
        case MODE_txdata:   
        case MODE_txend:    rm2_txdata_isr();   break;
    }
}






/** Radio Core Control Functions
  * ============================================================================
  * - Need to be customized per radio platform
  */
  
#ifndef EXTF_radio_off
void radio_off() {
   radio_sleep();   //CC430 is SoC, so sleep is same as off
}
#endif

#ifndef EXTF_radio_gag
void radio_gag() {
}
#endif

#ifndef EXTF_radio_sleep
void radio_sleep() {
	radio.flags |= RADIO_FLAG_ASLEEP;
}
#endif

#ifndef EXTF_radio_idle
void radio_idle() {
    if (radio.flags & RADIO_FLAG_ASLEEP) {
		radio.flags ^= RADIO_FLAG_ASLEEP;
	}
}
#endif

#ifndef EXTF_radio_calibrate
void radio_calibrate() {
}
#endif






/** Radio Module Control Functions
  * ============================================================================
  * - Need to be customized per radio platform
  */
#ifndef EXTF_radio_init
void radio_init( ) {
    vlFILE* fp;

    null_radio_init();

    /// Set startup channel to a completely invalid channel ID (0x55), and run 
    /// lookup on the default channel (0x07) to kick things off.  Since the 
    /// startup channel will always be different than a real channel, the 
    /// necessary settings and calibration will always occur. 
    phymac[0].channel   = 0x55;
    phymac[0].tx_eirp   = 0x7F;
    radio.flags         = 0;
    radio.state         = 0;
    radio.evtdone       = &otutils_sig2_null;
    fp                  = ISF_open_su( ISF_ID(channel_configuration) );
    subnull_channel_lookup(0x07, fp);
    vl_close(fp);

    // radio will be in sleep mode here
}
#endif


#ifndef EXTF_radio_check_cca
ot_bool radio_check_cca() {
/// This is a random simulation.  If the random byte (0-255) is less than 250,
/// the channel is considered open.  You can change the value (default 250) to
/// change the amount of simulated network traffic.
    return (platform_prand_u8() < 250);
}
#endif


#ifndef EXTF_radio_rssi
ot_int radio_rssi() {
/// Return -49 >= RSSI >= -111
/// Change the range by changing the constants
    ot_int  ranging;
    ot_u8   rand_u8;

    rand    = platform_prand_u8();
    ranging = rand & 0x1F;
    if (rand & 0x80) {
        ranging = 0 - ranging;
    }
    
    radio.last_rssi = 80 + ranging;

    return radio.last_rssi;
}
#endif


#ifndef EXTF_radio_buffer
ot_u8 radio_buffer(ot_int index) {
/// Transceiver implementation dependent
/// This function is not used on the CC430, which has a HW FIFO
    return 0;
}
#endif


#ifndef EXTF_radio_putbyte
void radio_putbyte(ot_u8 databyte) {
/// Transceiver implementation dependent
    radio.buffer[radio.txcursor++] = databyte;
}
#endif


#ifndef EXTF_radio_putfourbytes
void radio_putfourbytes(ot_u8* data) {
/// @note Eventually I would like to use an endian-independent implementation,
/// (basically, this is the big endian version) but now is not the time. Doing
/// so will involve revising the FEC encoder in the Encode Module.
#if (M2_FEATURE(FEC) == ENABLED)
#   ifdef __BIG_ENDIAN__
        radio.buffer[radio.txcursor++] = data[0];
        radio.buffer[radio.txcursor++] = data[1];
        radio.buffer[radio.txcursor++] = data[2];
        radio.buffer[radio.txcursor++] = data[3];
#   else
        radio.buffer[radio.txcursor++] = data[3];
        radio.buffer[radio.txcursor++] = data[2];
        radio.buffer[radio.txcursor++] = data[1];
        radio.buffer[radio.txcursor++] = data[0];
#   endif
#endif
}
#endif


#ifndef EXTF_radio_getbyte
ot_u8 radio_getbyte() {
/// Transceiver implementation dependent
    return null_radio_getbyte();
}
#endif


#ifndef EXTF_radio_getfourbytes
void radio_getfourbytes(ot_u8* data) {
    data[0] = null_radio_getbyte();
    data[1] = null_radio_getbyte();
    data[2] = null_radio_getbyte();
    data[3] = null_radio_getbyte();
}
#endif


#ifndef EXTF_radio_flush_rx
void radio_flush_rx() {
    radio.rxcursor = 0;
}
#endif


#ifndef EXTF_radio_flush_tx
void radio_flush_tx() {
    radio.txcursor = 0;
}
#endif


#ifndef EXTF_radio_rxopen
ot_bool radio_rxopen() {
    return (ot_bool)(null_radio_rxbytes() > 0);
}
#endif



#ifndef EXTF_radio_rxopen_4
ot_bool radio_rxopen_4() {
    return radio_rxopen();
}
#endif


#ifndef EXTF_radio_txopen
ot_bool radio_txopen() {
    return (ot_bool)(null_radio_txbytes() < radio.txlimit);
}
#endif


#ifndef EXTF_radio_txopen_4
ot_bool radio_txopen_4() {
/// Use commented-out version, or alternatively just never set txlimit above
/// (RADIO_BUFFER_TXMAX-4)
	//ot_u8 fifo_limit = (radio.txlimit < (RADIO_BUFFER_TXMAX-4)) ? \
	//						(ot_u8)radio.txlimit : (RADIO_BUFFER_TXMAX-4);
    //return (ot_bool)(null_radio_txbytes() < fifo_limit);
    
    return radio_txopen();
}
#endif








/** Radio I/O Functions
  * ============================================================================
  * -
  */

ot_bool subnull_test_channel(ot_u8 channel, ot_u8 netstate) {
#if (SYS_RECEIVE == ENABLED)
    ot_bool test = True;

    if ((channel != phymac[0].channel) || (netstate == M2_NETSTATE_UNASSOC)) {
        vlFILE* fp;
        /// Open the Mode 2 FS Config register that contains the channel list
        /// for this host, and make sure the channel we want to use is available
        /// @todo assert fp
        fp      = ISF_open_su( ISF_ID(channel_configuration) );
        test    = subnull_channel_lookup(channel, fp);
        vl_close(fp);
    }

    return test;
#else
    return True;
#endif
}




#if (SYS_RECEIVE == ENABLED)
void subnull_launch_rx(ot_u8 channel, ot_u8 netstate) {
    ot_u8       buffer_mode = 0;
    ot_u8       mcsm2_val   = 7;
    ot_u8       pktlen      = 7;
    sync_enum   sync_type   = SYNC_bg;

    /// 1.  Prepare RX queue by flushing it
    radio.rxlimit = 48;
    q_empty(&rxq);
    subnull_prep_q(&rxq);
    
    /// 2. CC430 can be in any state (even sleep) and access registers

    /// 3. Fetch the RX channel, exit if the specified channel is not available
    if (subnull_test_channel(channel, netstate) == False) {
        subnull_kill(RM2_ERR_BADCHANNEL, 0);
        return;
    }

    /// 4a. Setup RX for Background detection (queue config & low-RSSI termination)
    if (radio.flags & RADIO_FLAG_FLOOD) {
#       if (M2_FEATURE(FEC_RX) == ENABLED)
            if (phymac[0].channel & 0x80) pktlen = 16;
#       endif

        /// Queue manipulation to fit background frame into common model
        rxq.length      = pktlen + 2;
        rxq.front[0]    = pktlen;
        rxq.front[1]    = 0;
        rxq.getcursor   = &rxq.front[2];
        rxq.putcursor   = &rxq.front[2];
        mcsm2_val      |= b00010000;
    }

    /// 4b. Setup RX for Foreground detection (normal sync & timeout)
    else {
#       if ((M2_FEATURE(MULTIFRAME) == ENABLED) || (M2_FEATURE(FEC_RX) == ENABLED))
            ot_u8 auto_flag;
            // Initial state on CC430 could be RXMFP (0), RXPAGE (1), RXAUTO (2)
            auto_flag       = ((radio.flags & RADIO_FLAG_AUTO) != 0);
            radio.state     = ((radio.flags & RADIO_FLAG_FRCONT) == 0);
            radio.state    += auto_flag;
            radio.flags    |= (auto_flag << 3);     // sets RADIO_FLAG_RESIZE
            radio.rxlimit   = (auto_flag) ? 48 : 8; ///@todo 48 is a magic-number
            buffer_mode     = 2 - auto_flag;
#       else
            // Initial state is always RXAUTO (2), because no FEC or Multiframe RX
            radio.state     = RADIO_STATE_RXAUTO;
            buffer_mode     = 1;
#       endif
            pktlen          = 255;
            sync_type       = SYNC_fg;
    }

    /// 5.  Configure CC1101 for FG or BG receiving
    subnull_buffer_config(buffer_mode, pktlen);
    subnull_syncword_config(sync_type);

    /// 6.  Prepare Decoder to receive
    em2_decode_newpacket();
    em2_decode_newframe();
    subnull_offset_rxtimeout();     // if timeout is 0, set it to a minimal amount

    /// 7.  Setup interrupts for Sync Detect and IDLE fallback, then Turn on RX
    ///     It is a known erratum that CC430 must go to IDLE and set value of
    ///     TEST0 before entering RX/TX.  This is contrary to the Users' Guide.
    radio_flush_rx();
    radio_idle();
    null_command_rx();
    null_iocfg_listen();
}
#endif



#ifndef EXTF_rm2_default_tgd
ot_int rm2_default_tgd(ot_u8 chan_id) {
#if ((M2_FEATURE(FEC) == DISABLED) && (M2_FEATURE(TURBO) == DISABLED))
    return M2_TGD_55FULL;

#elif ((M2_FEATURE(FEC) == DISABLED) && (M2_FEATURE(TURBO) == ENABLED))
    return (chan_id & 0x60) ? M2_TGD_200FULL : M2_TGD_55FULL;

#elif ((M2_FEATURE(FEC) == ENABLED) && (M2_FEATURE(TURBO) == DISABLED))
    return (chan_id & 0x80) ? M2_TGD_55FULL : M2_TGD_55HALF;

#elif ((M2_FEATURE(FEC) == ENABLED) && (M2_FEATURE(TURBO) == ENABLED))
    /// @note This is an incredible hack, but it is fast and compact.
    /// To understand it, you need to know the way the channel ID works.
    static const ot_int tgd[4] = {
        M2_TGD_55FULL,
        M2_TGD_200FULL,
        M2_TGD_55HALF,
        M2_TGD_200HALF
    };

    chan_id    += 0x20;
    chan_id   >>= 6;
    return      tgd[chan_id];

#else
#   error "Missing definitions of M2_FEATURE(FEC) and/or M2_FEATURE(TURBO)"
    return 0;
#endif
}
#endif




#ifndef EXTF_rm2_pkt_duration
ot_int rm2_pkt_duration(ot_int pkt_bytes) {
/// Wrapper function for rm2_scale_codec that adds some slop overhead
/// Slop = preamble bytes + sync bytes + ramp-up + ramp-down + padding
    pkt_bytes  += RADIO_PKT_OVERHEAD;
    pkt_bytes  += ((phymac[0].channel & 0x60) != 0) << 1;

    return rm2_scale_codec(pkt_bytes);
}
#endif



#ifndef EXTF_rm2_scale_codec
ot_int rm2_scale_codec(ot_int buf_bytes) {
/// Turns a number of bytes (buf_bytes) into a number of ti units.
/// To refresh your memory: 1 ti = ((1sec/32768) * 2^5) = 2^-10 sec = ~0.977 ms

    /// Pursuant to DASH7 Mode 2 spec, b6:4 of channel ID corresponds to kS/s.
    /// 55.555 kS/s = 144us per buffer byte
    /// 200.00 kS/s = 40us per buffer bytes
    if (buf_bytes < 0) {
        buf_bytes = 0;
    }
    buf_bytes *= (phymac[0].channel & 0x60) ? 40 : 144;

    /// Divide us into Ticks
    /// (shift right 10 = divide by 1024)
#   if ((M2_FEATURE(FEC) == ENABLED) && (RF_FEATURE(FEC) == ENABLED))
        buf_bytes >>= (10 - ((phymac[0].channel & 0x80) != 0) );
#   else
        buf_bytes >>= 10;
#   endif

    return buf_bytes;
}
#endif



#ifndef EXTF_rm2_prep_resend
void rm2_prep_resend() {
    txq.options.ubyte[UPPER] = 255;
}
#endif


#ifndef EXTF_rm2_kill
void rm2_kill() {
    subnull_kill(RM2_ERR_KILL, 0);
}
#endif



#ifndef EXTF_rm2_rxinit_ff
void rm2_rxinit_ff(ot_u8 channel, ot_u8 netstate, ot_int est_frames, ot_sig2 callback) {
#if (SYS_RECEIVE == ENABLED)
    /// Setup the RX engine for Foreground Frame detection and RX.  Wipe-out
    /// the lower flags (non-persistent flags)
    radio.evtdone   = callback;
    radio.state     = RADIO_STATE_RXINIT;
    radio.flags    &= (RADIO_FLAG_SETPWR | RADIO_FLAG_AUTOCAL);

#   if (M2_FEATURE(MULTIFRAME) == ENABLED)
        radio.flags |= (est_frames > 1); //sets RADIO_FLAG_FRCONT
#   endif

    subnull_launch_rx(channel, netstate);

#else
    // BLINKER only (no RX)
    callback(RM2_ERR_GENERIC, 0);
#endif
}
#endif





#ifndef EXTF_rm2_rxinit_bf
void rm2_rxinit_bf(ot_u8 channel, ot_sig2 callback) {
#if (SYS_RECEIVE == ENABLED)
    /// 1. Open background method of RX (Burrow directly into Done state)
    radio.evtdone   = callback;
    radio.state     = RADIO_STATE_RXDONE;
    radio.flags    &= (RADIO_FLAG_SETPWR | RADIO_FLAG_AUTOCAL);
    radio.flags    |= RADIO_FLAG_FLOOD;

    subnull_launch_rx(channel, M2_NETSTATE_UNASSOC);

#else
    // BLINKER only (no RX)
    callback(RM2_ERR_GENERIC, 0);
#endif
}
#endif



#ifndef EXTF_rm2_rxsync_isr
void rm2_rxsync_isr() {
/// Reset the radio interruptor to catch the next RX FIFO interrupt, having
/// qualified the Sync Word.  rm2_rxdata_isr() will be called on that interrupt.
/// Also, re-schedule a system event as a watchdog.
    null_iocfg_rxdata();
    sys_set_mutex((ot_uint)SYS_MUTEX_RADIO_DATA);
    subnull_killonlowrssi();
}
#endif



#ifndef EXTF_rm2_rxtimeout_isr
void rm2_rxtimeout_isr() {
    subnull_kill(RM2_ERR_TIMEOUT, 0);
}
#endif




#ifndef EXTF_rm2_rxdata_isr
void rm2_rxdata_isr() {
#if (SYS_RECEIVE == ENABLED)
    subnull_killonlowrssi();

    rm2_rxdata_isr_TOP:

    /// 1. load data
    //RFGET_RXDATA();         // Only needed w/ internal DMA usage to set buffer params
    em2_decode_data();      // Contains logic to prevent over-run

    /// 2. Handle each RX type, and transitions
    switch ((radio.state >> RADIO_STATE_RXSHIFT) & (RADIO_STATE_RXMASK >> RADIO_STATE_RXSHIFT)) {

        /// RX State 0:
        /// Multiframe packets (only compiled when MFPs are supported)
#       if (M2_FEATURE(MULTIFRAME) == ENABLED)
        case (RADIO_STATE_RXMFP >> RADIO_STATE_RXSHIFT): {
            ot_int frames_left;
            rm2_rxpkt_MFP:
            frames_left = em2_remaining_frames();

            // If this is the last frame, move to single-frame packet mode
            // Else if more frames, but current frame is done, page it out.
            // Else, RX is in the middle of a frame (do nothing)
            if (frames_left == 0) {
                radio.state = RADIO_STATE_RXPAGE;
                // goes to next case statement from here
            }
            else if (em2_remaining_bytes() == 0) {
                /// @todo: I might require in the future that queue rebasing is
                ///        done in the evtdone callback (gives more flexibility).
                radio.evtdone(frames_left, (ot_int)crc_check() - 1);            // argument 2 is negative on bad Frame CRC

                // Prepare the next frame by moving the "front" pointer and
                // re-initializing the decoder engine
                q_rebase(&rxq, rxq.putcursor);
                em2_decode_newframe();

                // Clear out what's leftover in the FIFO, from the new frame,
                // and re-do boundary checks on this new frame.
                goto rm2_rxdata_isr_TOP;
            }
            else {
                break;
            }
        }
#       endif

        /// RX State 1:
        /// Continuous RX'ing of a packet that is buffered without HW control
        /// CC430 only compiles this code if multiframe packets are enabled or
        /// FEC RX is enabled (CC430 doesn't have HW FEC)
#       if ((M2_FEATURE(MULTIFRAME) == ENABLED) || (M2_FEATURE(FEC_RX) == ENABLED))
        case (RADIO_STATE_RXPAGE >> RADIO_STATE_RXSHIFT): {
            if (em2_remaining_bytes() <= radio.rxlimit) {
                // Put into DONE state
                radio.state     = RADIO_STATE_RXDONE;
                radio.flags    &= ~RADIO_FLAG_RESIZE;
                
                // kill RX full interrupt, only use packet done (prob unnecessary)
                null_iocfg_rxend();
                
                // transition to fixed length mode
                radio.rxlimit   = em2_remaining_bytes();
                subnull_buffer_config(0, radio.rxlimit);
            }
            break;
        }
#       endif

        /// RX State 2:
        /// Continuous RX'ing of a packet that is maintained by HW control
        /// On CC430, Non-FEC single frame packets should be using AUTO.
        case (RADIO_STATE_RXAUTO >> RADIO_STATE_RXSHIFT): {
            // Automatic RX should not actually need any management on CC430
            return;
        }

        /// RX State 3: RXDONE is handled by rm2_rxend_isr()
        
        /// Bug Trap
        default:
            rm2_kill();
            return;
    }


    /// 3. Change the size of the RX buffer to default, if required
#   if ((M2_FEATURE(MULTIFRAME) == ENABLED) || (M2_FEATURE(FEC_RX) == ENABLED))
        if (radio.flags & RADIO_FLAG_RESIZE) {
            radio.flags ^= RADIO_FLAG_RESIZE;
            RF_WriteSingleReg(RFREG(FIFOTHR), (ot_u8)((radio.rxlimit>>2)-1));
        }
#   endif

#endif
}
#endif




#ifndef EXTF_rm2_rxend_isr
void rm2_rxend_isr() {
    radio.state = RADIO_STATE_RXDONE;           // Make sure in DONE State, for decoding
    em2_decode_data();                          // decode any leftover data
    subnull_kill(0, (ot_int)crc_check() - 1);
}
#endif






void subnull_launch_tx() {
    // Flush TX FIFO
	radio_flush_tx();

    /// @note Flush TX FIFO and set buffer threshold to 5 bytes: the encoder 
    /// should be at least 20% faster than the max TX data speed (1 byte per 
    /// 40 µs).  On CC430, FEC encode is possible with 20 MHz CPU.
	RF_WriteSingleReg(RFREG(FIFOTHR), DRF_FIFOTHR | _FIFO_TXTHR(5) );
    
    // Put state into TXCCA1
    radio.state = RADIO_STATE_TXCCA1;
}



#ifndef EXTF_rm2_txinit_ff
void rm2_txinit_ff(ot_int est_frames, ot_sig2 callback) {
    radio.flags    |= (ot_u8)(est_frames > 1);
    radio.evtdone   = callback;
    subnull_launch_tx();
}
#endif


#ifndef EXTF_rm2_txinit_bf
void rm2_txinit_bf(ot_sig2 callback) {
#if (SYS_FLOOD == ENABLED)
    radio.flags    |= RADIO_FLAG_FLOOD;
    radio.evtdone   = callback;
    subnull_launch_tx();
#endif
}
#endif



#ifndef EXTF_rm2_txstop_flood
void rm2_txstop_flood() {
#if (SYS_FLOOD == ENABLED)
    radio.state = RADIO_STATE_TXDONE;
    RF_CoreITConfig(RF_CoreIT_TXBelowThresh, DISABLE);  //RFCONFIG_TXFIFOLOW_INTOFF();
#endif
}
#endif




#ifndef EXTF_rm2_txcsma
ot_int rm2_txcsma() {
    ot_int retval;

    // The shifting in the switch is so that the numbers are 0, 1, 2, 3...
    // It may seem silly, but it allows the switch to be compiled better.
    switch ( (radio.state >> RADIO_STATE_TXSHIFT) & (RADIO_STATE_TXMASK >> RADIO_STATE_TXSHIFT) ) {

        /// 1. First CCA
        case (RADIO_STATE_TXCCA1 >> RADIO_STATE_TXSHIFT): {
            if (subnull_chan_scan() == False) {    //Get usable channel
                retval = RM2_ERR_BADCHANNEL;
                break;
            }
            if (dll.comm.csmaca_params & M2_CSMACA_NOCSMA) {
                goto rm2_txcsma_START;
            }
            retval = phymac[0].tg;
        }

        /// 2. Second CCA
        /// Note: case fall-through on success
        case (RADIO_STATE_TXCCA2 >> RADIO_STATE_TXSHIFT): {
            radio.state += (1 << RADIO_STATE_TXSHIFT);
            if (subnull_cca_scan() == False) {
                retval      = RM2_ERR_CCAFAIL;
                radio.state = RADIO_STATE_TXCCA1;
            }
            if (radio.state != RADIO_STATE_TXSTART) {
                break;
            }
        }

        /// 3. TX startup
        /// Now that an available channel is found, set it up to TX.  Initially 
        /// load a rather small amount of data, because after CSMA now
        /// the system is just burning energy in IDLE.
        case (RADIO_STATE_TXSTART >> RADIO_STATE_TXSHIFT): {
        rm2_txcsma_START:
            // Set TX PATABLE now that channel is known
            if (radio.flags & RADIO_FLAG_SETPWR) {
                radio.flags &= ~RADIO_FLAG_SETPWR;
                subnull_set_txpwr( phymac[0].tx_eirp );
            }

#   		if (SYS_FLOOD == ENABLED)
            {   sync_enum   sync_type   = SYNC_fg;
            	ot_u8       mcsm1       = (_CCA_MODE_ALWAYS | _RXOFF_MODE_IDLE | _TXOFF_MODE_IDLE);
            	ot_u8       buffer_mode = 1;
            	ot_u8       buffer_size = 255;

            	if (radio.flags & RADIO_FLAG_FLOOD) {
            		sync_type   = SYNC_bg;
            		mcsm1       = (_CCA_MODE_ALWAYS | _RXOFF_MODE_IDLE | _TXOFF_MODE_TX);
            		buffer_mode = 0;
#           		if (M2_FEATURE(FEC_TX) == ENABLED)
            			buffer_size = (phymac[0].channel & 0x80) ? 16 : 7;
#           	    else
            			buffer_size = 7;
#           		endif
            	}
            	subnull_buffer_config(buffer_mode, buffer_size);
            	subnull_syncword_config( sync_type );
        		RF_WriteSingleReg(RFREG(MCSM1), mcsm1);
            }
#   		else
            	subnull_buffer_config(1, 255);
            	subnull_syncword_config( SYNC_fg );
            	//null_write(RFREG(MCSM1), b00000000 );   //should be persistent default
#   		endif

            // Preload into TX FIFO a relatively small amount (8 bytes) for min
            // latency.  Amount should be multiple of 4, (> 5), and small.
            radio.txlimit   = 8;
            txq.getcursor   = txq.front;
            txq.front[1]    = phymac[0].tx_eirp;
            subnull_prep_q(&txq);
            //radio_flush_tx();
            em2_encode_newpacket();
            em2_encode_newframe();
            em2_encode_data();
        
            // Put state into TX Data, and TXlimit to maximum (after preloading)
            radio.state     = RADIO_STATE_TXDATA;
            radio.txlimit   = RADIO_BUFFER_TXMAX;
            
            // Known Erratum that device must be in IDLE before TX.
            radio_idle();
            null_command_tx();
            null_iocfg_txdata();
            return -1;
        }
    }

    /// On CSMA fail or between CCA1 and CCA2, process ends-up here
    /// There could be more intelligence put here, to determine if CC430 should
    /// go to sleep or just stay in IDLE.
    radio_sleep();
    return retval;
}
#endif





#ifndef EXTF_rm2_txdata_isr
void rm2_txdata_isr() {
    /// Continues where rm2_txcsma() leaves off.
    switch ( (radio.state >> (RADIO_STATE_TXSHIFT+1)) & (RADIO_STATE_TXMASK >> (RADIO_STATE_TXSHIFT+1)) ) {

        /// 4. Continuous TX'ing of a single packet data
        case (RADIO_STATE_TXDATA >> (RADIO_STATE_TXSHIFT+1)): {
        rm2_txpkt_TXDATA:
        	/// Frame is not done, so come back later to fill buffer again
        	em2_encode_data();
            if (em2_remaining_bytes() != 0) {
                break;
            }

#           if (SYS_FLOOD == ENABLED)
            /// Packet flooding.  Only needed on devices that can send M2AdvP
            if (radio.flags & RADIO_FLAG_FLOOD) {
                radio.evtdone(2, 0);
                txq.getcursor = txq.front;
                em2_encode_newframe();
                goto rm2_txpkt_TXDATA;
            }
#           endif

#           if (M2_FEATURE(MULTIFRAME) == ENABLED)
            /// If the frame is done, but more need to be sent (e.g. MFP's)
            /// queue it up.  The additional encode stage is there to fill up
            /// what's left of the buffer.
            if (radio.flags & RADIO_FLAG_FRCONT) {
                q_rebase(&txq, txq.getcursor);
                radio.evtdone(1, 0);        //callback action for next frame
                em2_encode_newframe();
                txq.front[1] = phymac[0].tx_eirp;
                goto rm2_txpkt_TXDATA;
            }

            /// If the frame is done (em2_remaining_bytes() == 0) and there are
            /// no more frames to transmit, then this interrupt is due to a low
            /// threshold, and we just need to turn-off the threshold interrupt
            /// and wait for the last bit of data to get sent.
            if (em2_remaining_frames() != 0) {
            	break;
            }
#           endif

            radio.state = RADIO_STATE_TXDONE;
            null_iocfg_txend();
            break;
        }

        /// 5. Conclude the TX process, and wipe the radio state
        //     turn off any remaining TX interrupts
        case (RADIO_STATE_TXDONE >> (RADIO_STATE_TXSHIFT+1)):
            subnull_kill(0, 0);
            break;

        /// Bug trap
        default:
            rm2_kill();
            break;
    }
}
#endif







/** Radio Subroutines
  * ============================================================================
  * - Usually some minor adjustments needed when porting to new platform
  * - See integrated notes for areas sensitive to porting
  */

void subnull_null(ot_int arg1, ot_int arg2) { return; }



void subnull_kill(ot_int main_err, ot_int frame_err) {
    /// 1. Turn-off interrupts, send to IDLE, reset autocalibration flag
    radio_gag();
    radio_idle();
    subnull_reset_autocal();
    
    /// 2. Run Callback, then reset radio & callback to null state
    radio.evtdone(main_err, frame_err);
    radio.evtdone   = &otutils_sig2_null;
    radio.state     = 0;
}


void subnull_killonlowrssi() {
    ot_int min_rssi = ((phymac[0].cs_thr >> 1) & 0x3F) - 40;
    if (radio_rssi() < min_rssi) {
    //    subnull_kill(RM2_ERR_LINK, 0);
    }
}


void subnull_reset_autocal() {
    if (radio.flags & RADIO_FLAG_AUTOCAL) {
        radio.flags ^= RADIO_FLAG_AUTOCAL;
    }
}



ot_u8 subnull_rssithr_calc(ot_u8 input, ot_u8 offset) {
/// This function must prepare any hardware registers needed for automated
/// CS/CCA threshold value.
/// - "input" is a value 0-127 that is: input - 140 = threshold in dBm
/// - "offset" is a value subtracted from "input" that depends on chip impl
/// - return value is chip-specific threshold value
    return (input - offset);
}








ot_bool subnull_chan_scan( ) {
    vlFILE* fp;
    ot_int  i;

    fp = ISF_open_su( ISF_ID(channel_configuration) );
    ///@todo assert fp

    /// Go through the list of tx channels
    /// - Make sure the channel ID is valid
    /// - Make sure the transmission can fit within the contention period.
    /// - Scan it, to make sure it can be used
    for (i=0; i<dll.comm.tx_channels; i++) {
        if (subnull_channel_lookup(dll.comm.tx_chanlist[i], fp) != False) {
            break;
        }
    }

    vl_close(fp);
    return (ot_bool)(i < dll.comm.tx_channels);
}





ot_bool subnull_cca_scan() {
/// Called indirectly by other subroutines
/// @todo this is the only blocking call.  It can cause problems, being
/// blocking, so one day I would like to make it non-blocking.

/// @note CCA scan is a blocking call to the radio core, which waits for radio
/// RSSI to be declared valid (uses CC430 RSSI_Valid interrupt).  Alternatively,
/// a bipolar setup of CS/CCA interrupts could be used if you are porting to
/// CC1xxx and don't have the RSSI_Valid interrupt, but RSSI_Valid is more
/// reliable.  Then we just compare returned RSSI's with the stored limits.
    ot_bool cca_status;

    //send radio to idle mode: It is a known erratum that CC430 requires
    //transition from IDLE->RX/TX, rather than SLEEP->RX/TX (as written in
    //Users' Guide), because TEST0 must be written in IDLE.
    radio_idle();

    /// Turn-on RX, then wait for RSSI_Valid interrupt
    /// @note: kernel will always be paused during this time
    null_iocfg_txcsma();
    null_command_rx();

    /// Turn-off CSMA mode, compare RSSI value, and go back to IDLE
    cca_status  = radio_check_cca();
    radio_idle();                       //send radio to idle mode
    
    /// This channel has been calibrated by virtue of going into CSMA,
    /// therefore the Autocal flag (and other measures) can be reset
    subnull_reset_autocal();

    return cca_status;
}




ot_bool subnull_channel_lookup(ot_u8 chan_id, vlFILE* fp) {
/// Called during channel scans.
/// Duty: (a) See if the supplied channel is supported on this device & config.
///       If yes, return true.  (b) Determine if recalibration is required
///       before changing to the new channel, and recalibrate if so.

    ot_u8       fec_id;
    ot_u8       spectrum_id;
    ot_int      i;
    Twobytes    scratch;

    /// Only do the channel lookup if the new channel is different than before
    if (chan_id == phymac[0].channel) {
        return True;
    }

    /// pull spectrum id and encoding type out of chan_id
    fec_id      = chan_id & 0x80;
    spectrum_id = chan_id & ~0x80;


#if (0)
    /// Upper layer debugging, to make sure spectrum id is in scope
    if ((spectrum_id != RM2_CHAN_BASE) && \
        (spectrum_id != RM2_CHAN_LEGACY) && \
        (spectrum_id != RM2_CHAN_NORMAL_0) && \
        (spectrum_id != RM2_CHAN_NORMAL_2) && \
        (spectrum_id != RM2_CHAN_NORMAL_4) && \
        (spectrum_id != RM2_CHAN_NORMAL_6) && \
        (spectrum_id != RM2_CHAN_NORMAL_8) && \
        (spectrum_id != RM2_CHAN_NORMAL_A) && \
        (spectrum_id != RM2_CHAN_NORMAL_C) && \
        (spectrum_id != RM2_CHAN_NORMAL_E) && \
        (spectrum_id != RM2_CHAN_TURBO_1) && \
        (spectrum_id != RM2_CHAN_TURBO_3) && \
        (spectrum_id != RM2_CHAN_TURBO_5) && \
        (spectrum_id != RM2_CHAN_TURBO_7) && \
        (spectrum_id != RM2_CHAN_TURBO_9) && \
        (spectrum_id != RM2_CHAN_TURBO_B) && \
        (spectrum_id != RM2_CHAN_TURBO_D) && \
        (spectrum_id != RM2_CHAN_BLINK_2) && \
        (spectrum_id != RM2_CHAN_BLINK_C) && \
        (spectrum_id != RM2_CHAN_WILDCARD)) {

        /// @note good to put a break here, or a trap
        return False;
    }
#endif

    /// If FEC is requested by the new channel, but this device does not support
    /// FEC, then make sure to return False.
    if (fec_id) {
#       if (M2_FEATURE(FEC) == ENABLED)
            i = 0;

#           if (M2_FEATURE(FECRX) == ENABLED)
                i   = (radio.state & RADIO_STATE_RXMASK);
#           endif
#           if (M2_FEATURE(FECTX) == ENABLED)
                i  |= (radio.state & RADIO_STATE_TXMASK);
#           endif

            if (i == 0) {
                return False;
            }
#       else
            return False;
#       endif
    }

    /// 0x7F is the wildcard spectrum id.  It means use same spectrum as before.
    /// In this case, of course no recalibration is necessary.
    if (spectrum_id == 0x7F) {
        return True;
    }

    /// Look through the channel list to find the one with matching spectrum id.
    /// The channel list is not necessarily sorted.
#   if (M2_FEATURE(AUTOSCALE) != ENABLED)
#       define AUTOSCALE_MASK(VAL)      ((VAL) & 0x7F)
#   else
#       define AUTOSCALE_MASK(VAL)      (VAL)
#   endif

    for (i=0; i<fp->length; i+=8) {
        scratch.ushort = vl_read(fp, i);

        if (spectrum_id == scratch.ubyte[0]) {
            ot_u8 old_chan_id   = phymac[0].channel;
            ot_u8 old_tx_eirp   = phymac[0].tx_eirp;

            phymac[0].tg        = rm2_default_tgd(chan_id);
            phymac[0].channel   = chan_id;
            phymac[0].autoscale = scratch.ubyte[1];

            scratch.ushort      = vl_read(fp, i+2);
            phymac[0].tx_eirp   = AUTOSCALE_MASK(scratch.ubyte[0]);
            phymac[0].link_qual = AUTOSCALE_MASK(scratch.ubyte[1]);

            scratch.ushort      = vl_read(fp, i+4);
            phymac[0].cs_thr    = AUTOSCALE_MASK(scratch.ubyte[0]);
            phymac[0].cca_thr   = AUTOSCALE_MASK(scratch.ubyte[1]);

            /// @note This is a CC430-centric method to set cs_thr and cca_thr.
            /// Based on some internal features of the CC1101 radio core.  It
            /// works OK for CS, but for CCA I've found that direct comparison
            /// to RSSI is better (so it is commented out for CCA, and the plain
            /// RSSI value is retained).
            phymac[0].cs_thr    = subnull_rssithr_calc(phymac[0].cs_thr, RF_CSTHR_OFFSET);
            //phymac[0].cca_thr   = subnull_rssithr_calc(phymac[0].cca_thr, RF_CCATHR_OFFSET);

            subnull_chan_config(old_chan_id, old_tx_eirp);
            return True;
        }
    }

    return False;
}




void subnull_chan_config(ot_u8 old_chan, ot_u8 old_eirp) {
/// Called by subnull_channel_lookup()
/// Duty: perform channel setup and recalibration when moving from one channel
/// to another.
    ot_u8 fc_i;

    /// Reprogram the PA Table if eirp of new channel isn't the same as before
    if (old_eirp != phymac[0].tx_eirp) {
        radio.flags |= RADIO_FLAG_SETPWR;
    }

    /// Reprogram data rate, packet method, and modulation per upper nibble
    /// (Don't reprogram if radio is using the same channel class already)
    if ( (old_chan ^ phymac[0].channel) & 0xF0 ) {
        null_radio.mod   = phymac[0].channel & 0x70;
    }

    /// - Reprogram channel, and stage recalibration
    /// - But don't do these things if radio is already set on this center channel
    fc_i = (phymac[0].channel & 0x0F);          // Center Frequency index = lower four bits channel ID
    if ( fc_i != (old_chan & 0x0F) ) {
        null_radio.fc    = fc_i;
        radio.flags    |= RADIO_FLAG_AUTOCAL;
    }
}




void subnull_syncword_config(ot_u8 sync_class) {
///@todo Have some settings for writing the preamble & sync word in software to
///      simulation buffer.
#   if (M2_FEATURE(FEC) != ENABLED)
    static const ot_u8 sync_matrix[] = { 0xE6, 0xD0,
                                         0x0B, 0x67 };
#   else
    static const ot_u8 sync_matrix[] = { 0xE6, 0xD0,
                                         0x0B, 0x67,
                                         0xF4, 0x98,
                                         0x19, 0x2F };
    if (phymac[0].channel & 0x80) {
        sync_class += 4;
    }
#   endif
    
    null_radio.syncword = (ot_u8*)(sync_matrix+sync_class);
}




void subnull_buffer_config(ot_u8 mode, ot_u8 param) {
///@todo Have some settings for packet control in software that derive settings
///      from this function.
}




void subnull_prep_q(Queue* q) {
/// Put some special data in the queue options field.
/// Lower byte is encoding options (i.e. FEC)
/// Upper byte is processing options (i.e. CRC)
    q->options.ubyte[LOWER]    = (phymac[0].channel & 0x80);
    q->options.ubyte[UPPER]   += 1;
}




ot_int subnull_eta(ot_int next_int) {
/// Subtract by a tolerance amount, floor at 0
    ot_int eta = rm2_scale_codec(next_int) - 1;
    return (eta > 0) ? eta : 0;
}


ot_int subnull_eta_rxi() {
    return subnull_eta( RFGET_RXFIFO_NEXTINT() );
}

ot_int subnull_eta_txi() {
    return subnull_eta( RFGET_TXFIFO_NEXTINT() );
}





void subnull_offset_rxtimeout() {
/// If the rx timeout is 0, set it to a minimally small amount, which relates to
/// the rounded-up duration of an M2AdvP packet: 1, 2, 3, 4, or 6 ti.

    if (dll.comm.rx_timeout == 0) {
#       if (M2_FEATURE(TURBO) == ENABLED)
            dll.comm.rx_timeout  += 1;
            dll.comm.rx_timeout  += (((phymac[0].channel & 0x60) == 0) << 1);
#       else
            dll.comm.rx_timeout   = 3;
#       endif
#       if (M2_FEATURE(FEC) == ENABLED)
            dll.comm.rx_timeout <<= ((phymac[0].channel & 0x80) != 0);
#       endif
    }
}




void subnull_set_txpwr( ot_u8 eirp_code ) {
/// Sets the tx output power.
}



