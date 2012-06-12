/* Copyright 2012 JP Norair
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
  * @file       /OTradio/CC1101/radio_CC1101.c
  * @author     JP Norair
  * @version    V1.0
  * @date       13 Oct 2011
  * @brief      Radio Driver (RF transceiver) for CC1101
  * @defgroup   Radio (Radio Module)
  * @ingroup    Radio
  *
  * The header file for this implementation is /OTlib/radio.h.  It is universal
  * for all platforms, even though the implementation (this file) can differ.
  * There is also a header file at /OTradio/CC1101/radio_CC1101.h that contains
  * macros & constants specific to this implementation.
  *
  * For DASH7 Silicon certification, there are four basic tiers of HW features:
  * 1. PHY      The HW has a buffered I/O and the basic features necessary
  * 2. PHY+     The HW can do encoding, CRC, and some packet handling
  * 3. MAC      The HW can automate some inner loops, like Adv Flood and CSMA
  * 4. MAC+     The HW has most features of the MAC integrated
  *
  * The CC1101 is not the highest performing RF Core, but it has the most
  * built-in features of currently available DASH7 transceivers.  By the rubrick
  * above, it meets PHY and PHY+ (with exception of CRC).
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

#include "CC1101_interface.h"
#include "CC1101_defaults.h"      // register definitions file




/** PHY-MAC Array declaration
  * Described in radio.h of the OTlib.
  * This driver only supports M2_PARAM_MI_CHANNELS = 1.
  */
phymac_struct   phymac[M2_PARAM_MI_CHANNELS];


/** Radio struct array declaration
  * Defined in radio_CC1101.h
  */
radio_struct radio;



/** Virtual ISR (gets supplied by real ISR from CC1101_....c)  <BR>
  * ========================================================================<BR>
  */
void cc1101_virtual_isr(ot_u8 irq) {
#ifdef RADIO_IRQ2_PIN
	switch (irq & 7) {
        case RFIV_SYNC:             rm2_rxsync_isr();   break;
        case RFIV_RXIDLE:           rm2_kill();         break;
        case RFIV_RXEND:            //rm2_rxend_isr();    break;
        case RFIV_RXFIFOTHR:        rm2_rxdata_isr();   break;
        case RFIV_TXIDLE:
        case RFIV_TXFIFOTHR:        rm2_txdata_isr();   break;
        case RFIV_CS:               //radio.flags |= RADIO_FLAG_CCAFAIL;
        case RFIV_CCA:              break;
    }
#else
	switch (irq & 3) {
	    case RFIV_SYNC: 	rm2_rxsync_isr();   break;
	    case RFIV_RX:       rm2_rxdata_isr();   break;
	    case RFIV_TX:       rm2_txdata_isr();   break;
	    case RFIV_ERR:      rm2_kill();         break; //radio.flags |= RADIO_FLAG_CCAFAIL;
	}
#endif
}




/** Local Subroutine Prototypes  <BR>
  * ========================================================================<BR>
  */
typedef enum {
    SYNC_bg = 0,
    SYNC_fg = 3
} sync_enum;


ot_bool subcc1101_test_channel(ot_u8 channel, ot_u8 netstate);
void    subcc1101_launch_rx(ot_u8 channel, ot_u8 netstate, ot_u8 mcsm2_val);
void    subcc1101_launch_tx(ot_u8 flags, ot_sig2 callback);

void    subcc1101_kill(ot_int main_err, ot_int frame_err);
void    subcc1101_killonlowrssi();
void 	subcc1101_reset_autocal();

ot_bool subcc1101_chan_scan();
ot_bool subcc1101_cca_scan();

ot_bool subcc1101_channel_lookup(ot_u8 chan_id, vlFILE* fp);
void    subcc1101_syncword_config(sync_enum sync_class);
void    subcc1101_buffer_config(ot_u8 mode, ot_u8 param);
void    subcc1101_chan_config(ot_u8 old_chan, ot_u8 old_eirp);

void    subcc1101_prep_q(Queue* q);
//ot_int  subcc1101_eta(ot_int next_int);
//ot_int  subcc1101_eta_rxi();
//ot_int  subcc1101_eta_txi();
void    subcc1101_offset_rxtimeout();




/** Radio Core Control Functions
  * ============================================================================
  * - Need to be customized per radio platform
  */

#ifndef EXTF_radio_off
void radio_off() {
   radio_sleep();   
}
#endif


#ifndef EXTF_radio_gag
void radio_gag() {
    cc1101_int_turnoff(RFI_SOURCE0 | RFI_SOURCE2);
}
#endif


#ifndef EXTF_radio_sleep
void radio_sleep() {
/// Only go to sleep if the device is not already asleep
#ifndef BOARD_RF430USB_5509
	if ((radio.flags & RADIO_FLAG_ASLEEP) == 0) {
		radio.flags  = RADIO_FLAG_ASLEEP | RADIO_FLAG_SETPWR;
		cc1101_strobe(STROBE(SIDLE));
		cc1101_strobe(STROBE(SPWD));
	}
#else
	radio_idle();
	//cc1101_write(RFREG(IOCFG2), GDO_CLK_XOSC_D_1);
	//USB_enable();
	//USB_resume();
	//cc1101_strobe( STROBE(SFRX) );
	//cc1101_strobe( STROBE(SFTX) );
#endif
}
#endif


#ifndef EXTF_radio_idle
void radio_idle() {
/// Going from Sleep->Idle may use a different process than Active->Idle does.
#ifdef BOARD_RF430USB_5509
	if (radio.flags & RADIO_FLAG_ASLEEP) {
		radio.flags &= ~RADIO_FLAG_ASLEEP;
		//USB_suspend();
		//USB_disable();
		//cc1101_write(RFREG(IOCFG2), GDO_HIZ);
	}
#endif
	cc1101_strobe(STROBE(SIDLE));
}
#endif


#ifndef EXTF_radio_calibrate
void radio_calibrate() {
   cc1101_strobe(STROBE(SCAL));
}
#endif






/** Radio Module Control Functions
  * ============================================================================
  * - Need to be customized per radio platform
  */

#ifndef EXTF_radio_init
void radio_init( ) {
    vlFILE* fp;

    cc1101_init_bus();           //Initialize IO
    cc1101_load_defaults();     //Load default registers                  

    /// Set startup channel to a completely invalid channel ID (0x55), and run 
    /// lookup on the default channel (0x00) to kick things off.  Since the 
    /// startup channel will always be different than a real channel, the 
    /// necessary settings and calibration will always occur. 
    fp                  = ISF_open_su( ISF_ID(channel_configuration) );
    phymac[0].channel   = 0x55;
    phymac[0].tx_eirp   = 0x7F;
    radio.flags			= 0;
    radio.state         = 0;
    radio.evtdone       = &otutils_sig2_null;
    subcc1101_channel_lookup(0x00, fp);
    vl_close(fp);

    radio_sleep();
}
#endif



#ifndef EXTF_radio_check_cca
ot_bool radio_check_cca() {
/// CCA Method 2: Compare stored limit with actual, detected RSSI.
    ot_int thr  = (ot_int)phymac[0].cca_thr - 140;
    ot_int rssi = radio_rssi();
    return (ot_bool)(rssi < thr);
}
#endif



#ifndef EXTF_radio_rssi
ot_int radio_rssi() {
    return cc1101_calc_rssi( cc1101_rssi() );
}
#endif



#ifndef EXTF_radio_buffer
ot_u8 radio_buffer(ot_int index) {
/// This function is not used on the CC1101, which has a HW FIFO
    return 0;
}
#endif



#ifndef EXTF_radio_putbyte
void radio_putbyte(ot_u8 databyte) {
    cc1101_write(RFREG(TXFIFO), databyte);
}
#endif



#ifndef EXTF_radio_putfourbytes
void radio_putfourbytes(ot_u8* data) {
/// @note Eventually I would like to use an endian-independent implementation,
/// (basically, this is the big endian version) but now is not the time. Doing
/// so will involve revising the FEC encoder in the Encode Module.
#if (M2_FEATURE(FEC) == ENABLED)
#   ifdef __BIG_ENDIAN__
        data--;
        *data = RFREG(TXFIFO);
        cc1101_burstwrite(5, data);
#   else
        cc1101_write(RFREG(TXFIFO), data[3]);
        cc1101_write(RFREG(TXFIFO), data[2]);
        cc1101_write(RFREG(TXFIFO), data[1]);
        cc1101_write(RFREG(TXFIFO), data[0]);
#   endif
#endif
}
#endif



#ifndef EXTF_radio_getbyte
ot_u8 radio_getbyte() {
    return cc1101_read(RFREG(RXFIFO));
}
#endif



#ifndef EXTF_radio_getfourbytes
void radio_getfourbytes(ot_u8* data) {
///@note Radio is big endian as is datastream, so no conversion necessary
    cc1101_burstread(RFREG(RXFIFO), 4, data);
}
#endif



#ifndef EXTF_radio_flush_rx
void radio_flush_rx() {
    cc1101_strobe( STROBE(SFRX) );
}
#endif



#ifndef EXTF_radio_flush_tx
void radio_flush_tx() {
    cc1101_strobe( STROBE(SFTX) );
}
#endif



#ifndef EXTF_radio_rxopen
ot_bool radio_rxopen() {
///@note Do not draw-out the bottom byte in the FIFO until packet is complete.
///      This is a known erratum of CC11xx.
    return (ot_bool)(cc1101_read(RFREG(RXBYTES)) > (radio.state != RADIO_STATE_RXDONE));
}
#endif



#ifndef EXTF_radio_rxopen_4
ot_bool radio_rxopen_4() {
// this function probably is not used on CC1101, which has HW FEC
    ot_int thresh;
    thresh = (radio.state != RADIO_STATE_RXDONE) << 2;
    return (ot_bool)(cc1101_read(RFREG(RXBYTES)) > thresh);
}
#endif



#ifndef EXTF_radio_txopen
ot_bool radio_txopen() {
//	volatile ot_int txbytes = cc1101_read(RFREG(TXBYTES));
    return (ot_bool)(cc1101_read(RFREG(TXBYTES)) < radio.txlimit);
}
#endif



#ifndef EXTF_radio_txopen_4
ot_bool radio_txopen_4() {
// this function probably is not used on CC1101, which has HW FEC
	ot_u8 fifo_limit = (radio.txlimit < (RADIO_BUFFER_TXMAX-4)) ? \
						(ot_u8)radio.txlimit : (RADIO_BUFFER_TXMAX-4);

    return (ot_bool)(cc1101_read(RFREG(TXBYTES)) < fifo_limit);
}
#endif








/** Radio I/O Functions
  * ============================================================================
  * -
  */

ot_bool subcc1101_test_channel(ot_u8 channel, ot_u8 netstate) {
#if (SYS_RECEIVE == ENABLED)
    ot_bool test = True;

    if ((channel != phymac[0].channel) || (netstate == M2_NETSTATE_UNASSOC)) {
        vlFILE* fp;
        /// Open the Mode 2 FS Config register that contains the channel list
        /// for this host, and make sure the channel we want to use is available
        ///@todo assert fp in debug
        fp      = ISF_open_su( ISF_ID(channel_configuration) );
        test    = subcc1101_channel_lookup(channel, fp);
        vl_close(fp);
    }

    return test;
#else
    return True;
#endif
}



#if (SYS_RECEIVE == ENABLED)
void subcc1101_launch_rx(ot_u8 channel, ot_u8 netstate, ot_u8 mcsm2_val) {
	ot_u8 		buffer_mode;
	ot_u8 		pktlen;
	sync_enum	sync_type;

	/// 1.  Prepare RX queue by flushing it
#   ifdef RADIO_IRQ2_PIN
	radio.rxlimit = 48;
#   else
	radio.rxlimit = 8;
#	endif
	q_empty(&rxq);
    subcc1101_prep_q(&rxq);

    /// 2. Go to IDLE.  CC1101 must be in IDLE before writing data.
    radio_idle();

    /// 3. 	Fetch the RX channel, exit if the specified channel is not available
    if (subcc1101_test_channel(channel, netstate) == False) {
        subcc1101_kill(RM2_ERR_BADCHANNEL, 0);
        return;
    }

    /// 4a. Setup RX for Background detection (queue config & low-RSSI termination)
    if (radio.flags & RADIO_FLAG_FLOOD) {
#       if ((M2_FEATURE(FEC) == ENABLED) && (RF_FEATURE(FEC) != ENABLED))
            pktlen = (phymac[0].channel & 0x80) ? 16 : 7;
#       else
            pktlen = 7;
#       endif

        /// Queue manipulation to fit background frame into common model
        rxq.length      = pktlen + 2;
        rxq.front[0]    = pktlen;
        rxq.front[1]    = 0;
        rxq.getcursor   = &rxq.front[2];
        rxq.putcursor   = &rxq.front[2];
        buffer_mode		= 0;
        sync_type		= SYNC_bg;
    }

    /// 4b. Setup RX for Foreground detection (normal sync & timeout)
    else {
#       if (M2_FEATURE(MULTIFRAME) == ENABLED)
    	    ot_u8 auto_flag;
            // Initial state on CC430 could be RXMFP (0), RXPAGE (1), RXAUTO (2)
            auto_flag       = ((radio.flags & RADIO_FLAG_AUTO) != 0);
            radio.state     = ((radio.flags & RADIO_FLAG_FRCONT) == 0);
            radio.state    += auto_flag;
            radio.flags    |= (auto_flag << 3);     // sets RADIO_FLAG_RESIZE
            radio.rxlimit   = (auto_flag) ? 48 : 8; ///@todo 48 is a magic-number
            buffer_mode     = 2 - auto_flag;
#       else
            // Initial state is always RXAUTO (2)
            radio.state     = RADIO_STATE_RXAUTO;
            buffer_mode     = 1;
#       endif
            pktlen 			= 255;
            sync_type		= SYNC_fg;
    }

    /// 5.  Configure CC1101 for FG or BG receiving
    subcc1101_buffer_config(buffer_mode, pktlen);
    subcc1101_syncword_config(sync_type);
    cc1101_write(RFREG(FIFOTHR), (ot_u8)((radio.rxlimit >> 2) - 1));
    cc1101_write(RFREG(AGCCTRL2), phymac[0].cs_thr);
    if (radio.flags & RADIO_FLAG_AUTOCAL) {
    	cc1101_write( RFREG(MCSM0), (DRF_MCSM0&0xCF) | _FS_AUTOCAL_FROMIDLE );
    }
    cc1101_write(RFREG(MCSM2), mcsm2_val);

    /// 6.  Prepare Decoder to receive
    em2_decode_newpacket();
    em2_decode_newframe();
    subcc1101_offset_rxtimeout();     // if timeout is 0, set it to a minimal amount

    /// 7.  Setup interrupts for Sync Detect and IDLE fallback, then Turn on RX
    radio_flush_rx();
    cc1101_iocfg_listen();
    cc1101_strobe( STROBE(SRX) );
    cc1101_int_turnon(RFI_SYNC | RFI_RXIDLE);
}
#endif



#ifndef EXTF_rm2_default_tgd
ot_int rm2_default_tgd(ot_u8 chan_id) {
#if ((M2_FEATURE(FEC) == DISABLED) && (M2_FEATURE(TURBO) == DISABLED))
    return M2_TGD_55FULL;

#elif ((M2_FEATURE(FEC) == DISABLED) && (M2_FEATURE(TURBO) == ENABLED))
    return (chan_id & 0x20) ? M2_TGD_200FULL : M2_TGD_55FULL;

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

    return tgd[chan_id];

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
    pkt_bytes  += RF_PARAM_PKT_OVERHEAD;
    pkt_bytes  += (phymac[0].channel >> 4) & 2;
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
    buf_bytes *= (phymac[0].channel & 0x20) ? 40 : 144;

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
	radio_idle();
    subcc1101_kill(RM2_ERR_KILL, 0);
}
#endif




#ifndef EXTF_rm2_rxinit_ff
void rm2_rxinit_ff(ot_u8 channel, ot_u8 netstate, ot_int est_frames, ot_sig2 callback) {
#if (SYS_RECEIVE == ENABLED)
    /// Setup the RX engine for Foreground Frame detection and RX.  Wipe-out
	/// the lower flags (non-persistent flags)
    radio.evtdone   = callback;
    radio.state     = RADIO_STATE_RXINIT;
    radio.flags	   &= (RADIO_FLAG_ASLEEP | RADIO_FLAG_SETPWR | RADIO_FLAG_AUTOCAL);

#   if (M2_FEATURE(MULTIFRAME) == ENABLED)
        radio.flags |= (est_frames > 1); //sets RADIO_FLAG_FRCONT
#   endif

    subcc1101_launch_rx(channel, netstate, 7);

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
    radio.flags	   &= (RADIO_FLAG_ASLEEP | RADIO_FLAG_SETPWR | RADIO_FLAG_AUTOCAL);
    radio.flags    |= RADIO_FLAG_FLOOD;

    subcc1101_launch_rx(channel, M2_NETSTATE_UNASSOC, (_RX_TIME_RSSI|7) );

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
	cc1101_int_turnoff(RFI_SOURCE0 | RFI_SOURCE2);
    cc1101_iocfg_rxdata();
    cc1101_int_turnon(RFI_SOURCE0 | RFI_SOURCE2);

    sys_set_mutex((ot_uint)SYS_MUTEX_RADIO_DATA);
    subcc1101_killonlowrssi();
}
#endif




#ifndef EXTF_rm2_rxtimeout_isr
void rm2_rxtimeout_isr() {
	radio_idle();
    subcc1101_kill(RM2_ERR_TIMEOUT, 0);
}
#endif




#ifndef EXTF_rm2_rxdata_isr
void rm2_rxdata_isr() {
#if (SYS_RECEIVE == ENABLED)
    subcc1101_killonlowrssi();

    /// 1. load data
    //RFGET_RXDATA();         // Only needed w/ internal DMA usage to set buffer params
    em2_decode_data();      // Contains logic to prevent over-run

    /// 2. Handle each RX type, and transitions
    switch ((radio.state >> RADIO_STATE_RXSHIFT) & (RADIO_STATE_RXMASK >> RADIO_STATE_RXSHIFT)) {

        /// RX State 0:
        /// Multiframe packets (only compiled when MFPs are supported)
#       if (M2_FEATURE(MULTIFRAME) == ENABLED)
        case (RADIO_STATE_RXMFP >> RADIO_STATE_RXSHIFT): {
        rm2_rxpkt_MFP:
            ot_int frames_left = em2_remaining_frames();

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
                em2_decode_data();
                goto rm2_rxpkt_MFP;
            }
            else {
                break;
            }
        }

        /// RX State 1:
        /// Continuous RX'ing of a packet that is buffered without HW control
        /// The code is only compiled if multiframe packets are enabled
        /// @note re-checking live FIFO bytes accounts for any processing lag
        case (RADIO_STATE_RXPAGE >> RADIO_STATE_RXSHIFT): {
            ot_uint remaining_bytes = em2_remaining_bytes();
            if (remaining_bytes <= radio.rxlimit) {
               if (remaining_bytes <= cc1101_rxbytes()) {
                    goto rm2_rxpkt_DONE;
                }
                else {
                    radio.rxlimit = remaining_bytes;
                    subcc1101_buffer_config(0, remaining_bytes);
                }
                // kill RX full interrupt, only use packet done
                radio.flags    &= ~RADIO_FLAG_RESIZE;
                radio.state     = RADIO_STATE_RXDONE;
            }
            break;
        }
#       endif

        /// RX State 2:
        /// Continuous RX'ing of a packet that is maintained by HW control
        case (RADIO_STATE_RXAUTO >> RADIO_STATE_RXSHIFT): {
            ot_int remaining_bytes = em2_remaining_bytes();
            if (remaining_bytes <= radio.rxlimit) {
            	if (remaining_bytes == 0) {
            		goto rm2_rxpkt_DONE;
            	}
                radio.state = RADIO_STATE_RXDONE;
#               ifdef RADIO_IRQ2_PIN
                cc1101_int_turnoff(RFI_RXFIFOTHR);
#				else
                cc1101_iocfg_rxend();
#               endif
            }
#           ifndef RADIO_IRQ2_PIN
            else if (radio.rxlimit <= 8) {
            	radio.rxlimit = (remaining_bytes < 48) ? remaining_bytes : 48;
            	cc1101_write(RFREG(FIFOTHR), (ot_u8)((radio.rxlimit >> 2) - 1));
            }
#           endif
            break;
        }

        /// RX State 3: RXDONE
        case (RADIO_STATE_RXDONE >> RADIO_STATE_RXSHIFT): {
        rm2_rxpkt_DONE:
            subcc1101_kill(0, (ot_int)crc_check() - 1);
            return;
        }

        /// Bug Trap
        default:
            rm2_kill();
            return;
    }

    /// 3. Change the size of the RX buffer to default, if required
#   if (M2_FEATURE(MULTIFRAME) == ENABLED)
        if (radio.flags & RADIO_FLAG_RESIZE) {
            radio.flags &= ~RADIO_FLAG_RESIZE;
            cc1101_write(RFREG(FIFOTHR), (ot_u8)((radio.rxlimit >> 2) - 1) );
        }
#   endif

#endif
}
#endif




#ifndef EXTF_rm2_rxend_isr
void rm2_rxend_isr() {
}
#endif





void subcc1101_launch_tx(ot_u8 flags, ot_sig2 callback) {
	radio.state     = RADIO_STATE_TXCCA1;
    radio.flags    |= flags;
    radio.evtdone   = callback;
}




#ifndef EXTF_rm2_txinit_ff
void rm2_txinit_ff(ot_int est_frames, ot_sig2 callback) {
	radio.state     = RADIO_STATE_TXCCA1;
	radio.flags    |= (ot_u8)(est_frames > 1);
	radio.evtdone   = callback;
}
#endif




#ifndef EXTF_rm2_txinit_bf
void rm2_txinit_bf(ot_sig2 callback) {
#if (SYS_FLOOD == ENABLED)
	radio.state     = RADIO_STATE_TXCCA1;
	radio.flags    |= RADIO_FLAG_FLOOD;
	radio.evtdone   = callback;
	//subcc1101_launch_tx(RADIO_FLAG_FLOOD, callback);
#endif
}
#endif




#ifndef EXTF_rm2_txstop_flood
void rm2_txstop_flood() {
#if (SYS_FLOOD == ENABLED)
    radio.state = RADIO_STATE_TXDONE;
#	ifdef RADIO_IRQ2_PIN
    	cc1101_int_turnoff(RFI_TXFIFOTHR);	//Disable FIFO-low interrupt
#	else
    	cc1101_iocfg_txend();
#	endif
    cc1101_write(RFREG(MCSM1), 0);		//Set-up state machine to do TX->IDLE
#endif
}
#endif




#ifndef EXTF_rm2_txcsma
ot_int rm2_txcsma() {
	ot_int retval;

	/// 0. Go to IDLE, because register manipulations don't work otherwise.
	radio_idle();

    // The shifting in the switch is so that the numbers are 0, 1, 2, 3...
    // It may seem silly, but it allows the switch to be compiled better.
    switch ( (radio.state >> RADIO_STATE_TXSHIFT) & (RADIO_STATE_TXMASK >> RADIO_STATE_TXSHIFT) ) {

        /// 1. First CCA: Find a usable channel, then do 1st CCA
        /// Note: case fall-through on success
        case (RADIO_STATE_TXCCA1 >> RADIO_STATE_TXSHIFT): {
            if (subcc1101_chan_scan() == False) {	//Get usable channel
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
            if (subcc1101_cca_scan() == False) {
            	retval      = RM2_ERR_CCAFAIL;
            	radio.state = RADIO_STATE_TXCCA1;
            }
            if (radio.state != RADIO_STATE_TXSTART) {
            	break;
            }
        }

        /// 3. TX startup
        /// Initially load a rather small amount of data, because right
        ///now the system is just burning energy in IDLE
        case (RADIO_STATE_TXSTART >> RADIO_STATE_TXSHIFT): {
        rm2_txcsma_START:
        	if (radio.flags & RADIO_FLAG_SETPWR) {
        		radio.flags &= ~RADIO_FLAG_SETPWR;
        		cc1101_set_txpwr( phymac[0].tx_eirp );
        	}
        	radio.state		= RADIO_STATE_TXDATA;
        	radio.txlimit   = 8;
        	txq.getcursor   = txq.front;
        	txq.front[1]    = phymac[0].tx_eirp;
        	subcc1101_prep_q(&txq);
        	radio_flush_tx();
        	em2_encode_newpacket();
        	em2_encode_newframe();
        	em2_encode_data();
        	//radio.txlimit   = cc1101_read(RFREG(TXBYTES));

#       if (SYS_FLOOD == ENABLED)
        {   sync_enum	sync_type   = SYNC_fg;
        	ot_u8		mcsm1       = b00000000;
        	ot_u8		buffer_mode = 1;
        	ot_u8		buffer_size = 255;

        	if (radio.flags & RADIO_FLAG_FLOOD) {
        		sync_type   = SYNC_bg;
        		mcsm1       = b00000010;
        		buffer_mode = 0;
        		buffer_size = em2_remaining_bytes();
        	}
            subcc1101_syncword_config( sync_type );
            cc1101_write(RFREG(MCSM1), mcsm1 );
            subcc1101_buffer_config(buffer_mode, buffer_size);
        }
#       else
            subcc1101_syncword_config( SYNC_fg );
            //cc1101_write(RFREG(MCSM1), b00000000 );   //should be persistent default
#       endif

            radio.txlimit = RADIO_BUFFER_TXMAX;
            cc1101_write(RFREG(FIFOTHR), DRF_FIFOTHR | _FIFO_TXTHR(5));
            cc1101_iocfg_txdata();
            cc1101_int_turnon(RFI_SOURCE0 | RFI_SOURCE2);
            cc1101_strobe(STROBE(STX));
            return -1;
        }
    }

    /// On CSMA fail or between CCA1 and CCA2, process ends-up here
    radio_sleep();
    return retval;
}
#endif





#ifndef EXTF_rm2_txdata_isr
void rm2_txdata_isr() {
    /// Continues where rm2_txcsma() leaves off.
    switch ( (radio.state >> RADIO_STATE_TXSHIFT) & (RADIO_STATE_TXMASK >> RADIO_STATE_TXSHIFT) ) {

        /// 4. Continuous TX'ing of a single packet data
        case (RADIO_STATE_TXDATA >> RADIO_STATE_TXSHIFT): {
        rm2_txpkt_TXDATA:
            /// Buffer needs filling, frame is not done
            if (em2_remaining_bytes() != 0) {
                em2_encode_data();
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

#			if (M2_FEATURE(MULTIFRAME) == ENABLED)
            /// If the frame is done, but more need to be sent (e.g. MFP's)
            /// queue it up.  The additional encode stage is there to fill up
            /// what's left of the buffer.
            if (radio.flags & RADIO_FLAG_FRCONT) {
                q_rebase(&txq, txq.getcursor);
                radio.evtdone(1, 0);        //callback action for next frame
                em2_encode_newframe();
                txq.front[1] = phymac[0].tx_eirp;
                em2_encode_data();
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

            /// The last part of the frame needs to exit the FIFO
            //radio.txlimit = cc1101_read(RFREG(TXBYTES));
            radio.state = RADIO_STATE_TXDONE;
#           ifdef RADIO_IRQ2_PIN
            cc1101_int_turnoff(RFI_TXFIFOTHR);
#			else
            cc1101_iocfg_txend();
# 			endif
            break;
        }

        /// 5. Conclude the TX process, and wipe the radio state
        //     turn off any remaining TX interrupts
        case (RADIO_STATE_TXDONE >> RADIO_STATE_TXSHIFT):
            subcc1101_kill(0, 0);
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

void subcc1101_null(ot_int arg1, ot_int arg2) { }



void subcc1101_kill(ot_int main_err, ot_int frame_err) {
	/// 1.  Turn-off interrupts & put CC1101 into IDLE
    radio_gag();
    //radio_idle();			//should already be in idle, or going to idle
    subcc1101_reset_autocal();

    /// 2.  Run callback, then reset callback and reset radio state
    radio.evtdone(main_err, frame_err);
    radio.evtdone   = &otutils_sig2_null;
    radio.flags    &= RADIO_FLAG_SETPWR;	//clear all other flags
    radio.state     = 0;
}




void subcc1101_killonlowrssi() {
    /// need to inspect the CS bit
}


void subcc1101_reset_autocal() {
/// Turn off entry-into-RX/TX calibration stage.
	if (radio.flags & RADIO_FLAG_AUTOCAL) {
		radio.flags &= ~RADIO_FLAG_AUTOCAL;
	    cc1101_write(RFREG(MCSM0), DRF_MCSM0);
	}
}


ot_bool subcc1101_chan_scan( ) {
    vlFILE* fp;
    ot_int  i;
    
    //radio.flags &= ~RADIO_FLAG_CCAFAIL;	//this flag presently unused

    fp = ISF_open_su( ISF_ID(channel_configuration) );
    ///@todo assert fp

    /// Go through the list of tx channels
    /// - Make sure the channel ID is valid
    /// - Make sure the transmission can fit within the contention period.
    /// - Scan it, to make sure it can be used
    for (i=0; i<dll.comm.tx_channels; i++) {
        if (subcc1101_channel_lookup(dll.comm.tx_chanlist[i], fp) != False) {
            break;
        }
    }

    vl_close(fp);
    return (ot_bool)(i < dll.comm.tx_channels);
}





ot_bool subcc1101_cca_scan() {
/// This is a blocking call, and it runs pretty fast (less than 1ms).  It uses
/// direct RSSI evaluation instead of the built-in CCA mechanism, which is not
/// reliable.  Direct RSSI evaluation is faster and more precise.  400us is a 
/// safe amount of time for the RSSI to stabilize after starting RX.
    ot_bool check_cca;
    ot_u16	wait_period;

    cc1101_int_turnoff(RFI_SOURCE0 | RFI_SOURCE2);
    //radio_idle();  //should be in idle already
    cc1101_strobe( STROBE(SRX) );
    
    // CC1101 takes about 350us to get RSSI stabilized, and 799 to calibrate
    wait_period	= (radio.flags & RADIO_FLAG_AUTOCAL) ? (350+799) : (350);

    // Wait for RSSI, check it against threshold, go back to idle, stop autocal
    platform_swdelay_us(wait_period);
    check_cca = radio_check_cca();
    radio_idle();
    subcc1101_reset_autocal();
    
    return check_cca;
}




ot_bool subcc1101_channel_lookup(ot_u8 chan_id, vlFILE* fp) {
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

            /// @note This is experimental
            scratch.ubyte[0]    = (phymac[0].channel >> 4) & 2;
            phymac[0].cs_thr    = cc1101_calc_rssithr(phymac[0].cs_thr, scratch.ubyte[0]);
            phymac[0].cca_thr   = cc1101_calc_rssithr(phymac[0].cca_thr, scratch.ubyte[0]);

            subcc1101_chan_config(old_chan_id, old_tx_eirp);
            return True;
        }
    }

    return False;
}




void subcc1101_chan_config(ot_u8 old_chan, ot_u8 old_eirp) {
/// Called by subcc1101_channel_lookup()
/// Duty: perform channel setup and recalibration when moving from one channel
/// to another.
    ot_u8 fc_i;
    ot_u8 mdmcfg[5] = {0x40|RFREG(MDMCFG4) , DRF_MDMCFG4, DRF_MDMCFG3, DRF_MDMCFG2, DRF_MDMCFG1};

    /// CC1101 power table is lost during sleep, so it is programmed always
    /// when going into TX after sleeping or whenever a new power is selected
    if (old_eirp != phymac[0].tx_eirp) {
    	radio.flags |= RADIO_FLAG_SETPWR;
    }

    /// Center Frequency index = lower four bits channel ID
    fc_i = (phymac[0].channel & 0x0F);

    /// @note Settings of "mdmcfg..." are CC430 specific implementation
    /// Reprogram data rate, packet method, and modulation per upper nibble
    /// (Don't reprogram if radio is using the same channel class already)
    if ( (old_chan ^ phymac[0].channel) & 0xF0 ) {
        mdmcfg[4] |= phymac[0].channel & 0x80; //FEC enable (or not)
        if (phymac[0].channel & 0x20) {
        	mdmcfg[1]   = DRF_MDMCFG4_HI;
            mdmcfg[2]   = DRF_MDMCFG3_HI;
            mdmcfg[4]  |= _NUM_PREAMBLE_6B;	//6B preamble on Hi-speed
        }
        cc1101_spibus_io(5, 0, mdmcfg, NULL);
    }

    /// If channel frequency is different than before, move to the new channel,
    /// and tell driver to stage automatic calibration upon TX/RX entry.
    /// @note: CC1101 contains a channel-offset built-in mechanism. 
    if ( fc_i != (old_chan & 0x0F) ) {
        cc1101_write(RFREG(CHANNR), (ot_u8)fc_i);
        radio.flags |= RADIO_FLAG_AUTOCAL;
    }
}




void subcc1101_syncword_config(sync_enum sync_class) {
#   if (M2_FEATURE(FEC) != ENABLED)
    static const ot_u8 sync_matrix[] = { RFREG(SYNC1)|0x40, 0xE6, 0xD0, 
                                         RFREG(SYNC1)|0x40, 0x0B, 0x67 };
#   else
    static const ot_u8 sync_matrix[] = { RFREG(SYNC1)|0x40, 0xE6, 0xD0, 
                                         RFREG(SYNC1)|0x40, 0x0B, 0x67,
                                         RFREG(SYNC1)|0x40, 0xF4, 0x98,
                                         RFREG(SYNC1)|0x40, 0x19, 0x2F };
    
    if (phymac[0].channel & 0x80) {
        (ot_u8)sync_class += 6;
    }
#   endif
    
    cc1101_spibus_io(3, 0, (ot_u8*)(sync_matrix+sync_class), NULL);
}




void subcc1101_buffer_config(ot_u8 mode, ot_u8 param) {
/// CC430 specific implementation.  This function selects between three
/// buffering modes: FIFO buffering (2), automatic packet handling (1), or
/// fixed-length packet (0).  The "param" argument is a chip-specific option.
    cc1101_write( RFREG(PKTLEN), param );
    cc1101_write( RFREG(PKTCTRL0), DRF_PKTCTRL0 | mode);
}




void subcc1101_prep_q(Queue* q) {
/// Put some special data in the queue options field.
/// Lower byte is encoding options (i.e. FEC)
/// Upper byte is processing options (i.e. CRC)
    //q->options.ubyte[LOWER]    = (phymac[0].channel & 0x80);
    q->options.ubyte[UPPER]   += 1;
}




ot_int subcc1101_eta(ot_int next_int) {
/// Subtract by a tolerance amount, floor at 0
    ot_int eta = rm2_scale_codec(next_int) - 1;
    return (eta > 0) ? eta : 0;
}


ot_int subcc1101_eta_rxi() {
    return subcc1101_eta( radio.rxlimit - cc1101_rxbytes() );
}

ot_int subcc1101_eta_txi() {
    return subcc1101_eta( cc1101_txbytes() - radio.txlimit );
}





void subcc1101_offset_rxtimeout() {
/// If the rx timeout is 0, set it to a minimally small amount, which relates to
/// the rounded-up duration of an M2AdvP packet: 1, 2, 3, 4, or 6 ti.

    if (dll.comm.rx_timeout == 0) {
#       if (M2_FEATURE(TURBO) == ENABLED)
            dll.comm.rx_timeout  += 1;
            dll.comm.rx_timeout  += (((phymac[0].channel & 0x20) == 0) << 1);
#       else
            dll.comm.rx_timeout   = 3;
#       endif
#       if (M2_FEATURE(FEC) == ENABLED)
            dll.comm.rx_timeout <<= ((phymac[0].channel & 0x80) != 0);
#       endif
    }
}




