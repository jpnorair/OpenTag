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
  * @file       /otradio/cc1101/radio_CC1101.c
  * @author     JP Norair
  * @version    R100
  * @date       27 Oct 2012
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

#include <m2/radio.h>
#include "radio_CC1101.h"
#include "CC1101_interface.h"

#include <otsys/types.h>
#include <otsys/config.h>
#include <otlib/utils.h>
#include <otplatform.h>

#include <otsys/veelite.h>
#include <m2/session.h>
#include <m2/dll.h>
#include <otlib/crc16.h>
#include <m2/encode.h>
#include <otlib/buffers.h>
#include <otlib/queue.h>



/** Universal Data declaration
  * Described in radio.h of the OTlib.
  * This driver only supports M2_PARAM_MI_CHANNELS = 1.
  */
phymac_struct   phymac[M2_PARAM_MI_CHANNELS];
radio_struct    radio;


/** Local Data declaration
  * Defined in radio_CC1101.h
  */
rfctl_struct    rfctl;




/** Local Subroutine Prototypes  <BR>
  * ========================================================================<BR>
  */
typedef enum {
    SYNC_bg = 0,
    SYNC_fg = 3
} sync_enum;


ot_bool subcc1101_test_channel(ot_u8 channel);
void    subcc1101_launch_rx(ot_u8 channel, ot_u8 netstate);

void    subcc1101_finish(ot_int main_err, ot_int frame_err);
ot_bool subcc1101_lowrssi_reenter();
void    subcc1101_reset_autocal();

ot_bool subcc1101_chanscan();
ot_bool subcc1101_ccascan();

ot_bool subcc1101_channel_lookup(ot_u8 chan_id, vlFILE* fp);
void    subcc1101_syncword_config(ot_u8 sync_class);
void    subcc1101_buffer_config(ot_u8 mode, ot_u8 param);
void    subcc1101_chan_config(ot_u8 old_chan, ot_u8 old_eirp);

void    subcc1101_prep_q(ot_queue* q);
//ot_int  subcc1101_eta(ot_int next_int);
//ot_int  subcc1101_eta_rxi();
//ot_int  subcc1101_eta_txi();
void    subcc1101_offset_rxtimeout();





/** Virtual ISR (gets supplied by real ISR from CC1101_....c)  <BR>
  * ========================================================================<BR>
  */
OT_INLINE void cc1101_virtual_isr(ot_u8 irq) {
#ifdef RADIO_IRQ2_PIN
    switch (irq & 7) {
        case RFIV_SYNC:             rm2_rxsync_isr();   break;
        case RFIV_RXIDLE:           rm2_kill();         break;
        case RFIV_RXEND:            //rm2_rxend_isr();    break;
        case RFIV_RXFIFOTHR:        rm2_rxdata_isr();   break;
        case RFIV_TXIDLE:
        case RFIV_TXFIFOTHR:        rm2_txdata_isr();   break;
        case RFIV_CS:               //rfctl.flags |= RADIO_FLAG_CCAFAIL;
        case RFIV_CCA:              break;
    }
#else
    switch (irq & 3) {
        case RFIV_SYNC:     rm2_rxsync_isr();   break;
        case RFIV_RX:       rm2_rxdata_isr();   break;
        case RFIV_TX:       rm2_txdata_isr();   break;
        case RFIV_ERR:      rm2_kill();         break; //rfctl.flags |= RADIO_FLAG_CCAFAIL;
    }
#endif
}




OT_INLINE void radio_mac_isr() {
    switch (radio.state) {
        //case RADIO_Idle:
        //case RADIO_Listening:

        case RADIO_Csma:        rm2_txcsma_isr();
                                break;
        //case RADIO_DataRX:

#       if (M2_FEATURE(GATEWAY) || M2_FEATURE(SUBCONTROLLER))
        case RADIO_DataTX:      dll.counter--;
                                radio_set_mactimer(TI2CLK(1));
                                break;
#       endif
    }
}







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
    radio.state = RADIO_Idle;
    if ((rfctl.flags & RADIO_FLAG_ASLEEP) == 0) {
        rfctl.flags  = RADIO_FLAG_ASLEEP | RADIO_FLAG_SETPWR;
        cc1101_strobe(STROBE(SIDLE));
        cc1101_strobe(STROBE(SPWD));
    }
#else
    radio_idle();
#endif
}
#endif


#ifndef EXTF_radio_idle
void radio_idle() {
/// Going from Sleep->Idle may use a different process than Active->Idle does.
    radio.state = RADIO_Idle;
    cc1101_strobe(STROBE(SIDLE));
#   ifndef BOARD_RF430USB_5509
        if (rfctl.flags & RADIO_FLAG_ASLEEP) {
            rfctl.flags ^= RADIO_FLAG_ASLEEP;
            cc1101_write(RFREG(TEST0), DRF_TEST0);
        }
#   endif
}
#endif


#ifndef EXTF_radio_calibrate
void radio_calibrate() {
   cc1101_strobe(STROBE(SCAL));
}
#endif


#ifndef EXTF_radio_set_mactimer
void radio_set_mactimer(ot_u16 clocks) {
/// Todo: apply board configuration
    systim_set_insertion(clocks);
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
    /// lookup on the default channel (0x07) to kick things off.  Since the
    /// startup channel will always be different than a real channel, the
    /// necessary settings and calibration will always occur.
    phymac[0].channel   = 0x55;
    phymac[0].tx_eirp   = 0x7F;
    rfctl.flags         = 0;
    rfctl.state         = 0;
    radio.evtdone       = &otutils_sig2_null;
    fp                  = ISF_open_su( ISF_ID(channel_configuration) );
    subcc1101_channel_lookup(0x07, fp);
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
    radio.last_rssi = cc1101_calc_rssi( cc1101_rssi() );
    return radio.last_rssi;
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
    /// 1. Flush the Data in the RX FIFO
    cc1101_strobe( STROBE(SFRX) );

    /// 2. Flush ot_queue and set decoder
    /// ot_queue options are set by subcc1101_prep_q, which decoder uses as params
    q_empty(&rxq);
    subcc1101_prep_q(&rxq);
    em2_decode_newpacket();
    em2_decode_newframe();

    /// 3. Configure RX FIFO Limit.
#   ifdef RADIO_IRQ2_PIN
    rfctl.rxlimit = 48;
#   else
    rfctl.rxlimit   = 8;
#   endif

    cc1101_write(RFREG(FIFOTHR), (ot_u8)((rfctl.rxlimit >> 2) - 1));

#   ifndef RADIO_IRQ2_PIN
        rfctl.rxlimit  = 52;
        rfctl.flags   |= RADIO_FLAG_RESIZE;
#   endif
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
    return (ot_bool)(cc1101_read(RFREG(RXBYTES)) > (rfctl.state != RADIO_STATE_RXDONE));
}
#endif



#ifndef EXTF_radio_rxopen_4
ot_bool radio_rxopen_4() {
// this function probably is not used on CC1101, which has HW FEC
    ot_int thresh;
    thresh = (rfctl.state != RADIO_STATE_RXDONE) << 2;
    return (ot_bool)(cc1101_read(RFREG(RXBYTES)) > thresh);
}
#endif



#ifndef EXTF_radio_txopen
ot_bool radio_txopen() {
//  volatile ot_int txbytes = cc1101_read(RFREG(TXBYTES));
    return (ot_bool)(cc1101_read(RFREG(TXBYTES)) < rfctl.txlimit);
}
#endif



#ifndef EXTF_radio_txopen_4
ot_bool radio_txopen_4() {
/// Transceiver implementation dependent, only needed with FEC
/// Use commented-out version, or alternatively just never set txlimit above
/// (RADIO_BUFFER_TXMAX-4)
	//ot_u8 fifo_limit = (rfctl.txlimit < (RADIO_BUFFER_TXMAX-4)) ? \
	//						(ot_u8)rfctl.txlimit : (RADIO_BUFFER_TXMAX-4);
    //return (ot_bool)(RF_GetTXBYTES() < fifo_limit);

    return radio_txopen();
}
#endif








/** Radio I/O Functions
  * ============================================================================
  * -
  */

ot_bool subcc1101_test_channel(ot_u8 channel) {
#if (SYS_RECEIVE == ENABLED)
    ot_bool test = True;

    if ((channel != phymac[0].channel)) {
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
void subcc1101_launch_rx(ot_u8 channel, ot_u8 netstate) {
    ot_u8       buffer_mode = 0;
    ot_u8       pktlen      = 7;
    ot_u8       mcsm210[4];
    sync_enum   sync_type;

    /// 1. Prepare MCSM settings
    /// -  Prepare modem state-machine to do RX-Idle or RX-RX
    ///    RX-RX happens during Response listening, unless FIRSTRX is high
    netstate   &= (M2_NETFLAG_FIRSTRX | M2_NETSTATE_RESP);
    mcsm210[0]  = (RFREG(MCSM2) | 0x40);
    mcsm210[1]  = 7;
    mcsm210[2]  = (netstate ^ M2_NETSTATE_RESP) ? DRF_MCSM1 : (DRF_MCSM1 | _RXOFF_MODE_RX);
    mcsm210[3]  = (rfctl.flags & RADIO_FLAG_AUTOCAL) ? DRF_MCSM0 : (DRF_MCSM0&0xCF) | _FS_AUTOCAL_FROMIDLE;

    /// 2. Go to IDLE.  CC1101 must be in IDLE before writing data.
    radio_idle();

    /// 4.  Fetch the RX channel, exit if the specified channel is not available
    if (subcc1101_test_channel(channel) == False) {
        subcc1101_finish(RM2_ERR_BADCHANNEL, 0);
        return;
    }

    /// 5a. Setup RX for Background detection (queue config & low-RSSI termination)
    if (rfctl.flags & RADIO_FLAG_FLOOD) {
        //pktlen = 7;                                       //CC1101 has HW FEC
        //pktlen = (phymac[0].channel & 0x80) ? 16 : 7;     //This is for SW FEC

        /// ot_queue manipulation to fit background frame into common model
     //#rxq.length      = pktlen + 2;
        rxq.front[0]    = pktlen;
        rxq.front[1]    = 0;
        rxq.getcursor   = &rxq.front[2];
        rxq.putcursor   = &rxq.front[2];
        mcsm210[0]     |= b00010000;
        sync_type       = SYNC_bg;
    }

    /// 5b. Setup RX for Foreground detection (normal sync & timeout)
    else {
#       if (M2_FEATURE(MULTIFRAME) == ENABLED)
            ot_u8 auto_flag;
            // Initial state on CC430 could be RXMFP (0), RXPAGE (1), RXAUTO (2)
            auto_flag       = ((rfctl.flags & RADIO_FLAG_AUTO) != 0);
            rfctl.state     = ((rfctl.flags & RADIO_FLAG_FRCONT) == 0);
            rfctl.state    += auto_flag;
            rfctl.flags    |= (auto_flag << 3);     // sets RADIO_FLAG_RESIZE
            rfctl.rxlimit   = (auto_flag) ? 48 : 8; ///@todo 48 is a magic-number
            buffer_mode     = 2 - auto_flag;
#       else
            // Initial state is always RXAUTO (2), because no multiframe RX
            rfctl.state     = RADIO_STATE_RXAUTO;
            buffer_mode     = 1;
#       endif
            pktlen          = 255;
            sync_type       = SYNC_fg;
    }

    /// 6.  Configure CC1101 for FG or BG receiving.
    ///     For single GDO device, setup buffer resizing after first RX page
    subcc1101_buffer_config(buffer_mode, pktlen);
    subcc1101_syncword_config(sync_type);

    /// 7.  Configure CS threshold and MCSM settings
    cc1101_write(RFREG(AGCCTRL2), phymac[0].cs_thr);
    cc1101_spibus_io(4, 0, mcsm210, NULL);

    subcc1101_offset_rxtimeout();     // if timeout is 0, set it to a minimal amount

    /// 9.  Using rm2_reenter_rx() with NULL forces entry into rx, and sets states
    rm2_reenter_rx(NULL);
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
    return      tgd[chan_id];

#else
#   error "Missing definitions of M2_FEATURE(FEC) and/or M2_FEATURE(TURBO)"
    return 0;
#endif
}
#endif




#ifndef EXTF_rm2_txpkt_duration
ot_int rm2_txpkt_duration(ot_int pkt_bytes) {
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

    return (buf_bytes+1);
}
#endif



#ifndef rm2_reenter_rx
void rm2_reenter_rx(ot_sig2 callback) {
/// If radio is in an inactive state, restart RX using the same settings that
/// are presently in the radio core.  Always do this when forced.
    if (callback == NULL) {
        goto rm2_reenter_rx_PROC;
    }

    if (cc1101_read(RFREG(MARCSTATE)) < 3) {
    rm2_reenter_rx_PROC:
        radio_flush_rx();
        cc1101_iocfg_listen();
        cc1101_strobe( STROBE(SRX) );
        cc1101_int_turnon(RFI_SYNC | RFI_RXIDLE);
        radio.state = RADIO_Listening;
        rfctl.state &= RADIO_STATE_RXMFP; //wipe all states except MFP
    }
}
#endif



#ifndef EXTF_rm2_resend
void rm2_resend(ot_sig2 callback) {
    radio.evtdone               = callback;
    rfctl.state                 = RADIO_STATE_TXSTART;
    txq.options.ubyte[UPPER]    = 255;
    rm2_txcsma_isr();
}
#endif



#ifndef EXTF_rm2_kill
void rm2_kill() {
    radio_idle();
    subcc1101_finish(RM2_ERR_KILL, 0);
}
#endif




#ifndef EXTF_rm2_rxinit
void rm2_rxinit(ot_u8 channel, ot_u8 psettings, ot_sig2 callback) {
#if (SYS_RECEIVE == ENABLED)
    ot_u8 netstate;

    /// Setup the RX engine for Foreground Frame detection and RX.  Wipe-out
    /// the lower flags (non-persistent flags)
    radio.evtdone   = callback;
    rfctl.flags    &= (RADIO_FLAG_ASLEEP | RADIO_FLAG_SETPWR | RADIO_FLAG_AUTOCAL);

    if (psettings == 0) {
        netstate    = (M2_NETSTATE_UNASSOC | M2_NETFLAG_FIRSTRX);
        rfctl.flags|= RADIO_FLAG_FLOOD;
    }
    else {
        netstate    = session_netstate();
#       if (M2_FEATURE(MULTIFRAME) == ENABLED)
        rfctl.flags |= ((netstate & M2_NETSTATE_DSDIALOG) != 0); //sets RADIO_FLAG_FRCONT
#       endif
    }

    subcc1101_launch_rx(channel, netstate);

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
    if (subcc1101_lowrssi_reenter() == 0) {
        cc1101_int_turnoff(RFI_SOURCE0 | RFI_SOURCE2);
        cc1101_iocfg_rxdata();
        cc1101_int_turnon(RFI_SOURCE0 | RFI_SOURCE2);
        radio.state = RADIO_DataRX;
        dll_block();
    }
}
#endif




#ifndef EXTF_rm2_rxtimeout_isr
void rm2_rxtimeout_isr() {
    radio_idle();
    subcc1101_finish(RM2_ERR_TIMEOUT, 0);
}
#endif




#ifndef EXTF_rm2_rxdata_isr
void rm2_rxdata_isr() {
#if (SYS_RECEIVE == ENABLED)
    subcc1101_lowrssi_reenter();

    rm2_rxdata_isr_TOP:

    /// 1. load data
    //RFGET_RXDATA();         // Only needed w/ internal DMA usage to set buffer params
    em2_decode_data();      // Contains logic to prevent over-run

    if (rfctl.state & RADIO_STATE_RXDONE) {
    rm2_rxpkt_DONE:
        subcc1101_finish(0, (ot_int)crc_check(&em2.crc) - 1);
        return;
    }

#   if (M2_FEATURE(MULTIFRAME) == ENABLED)
    if (rfctl.state & RADIO_STATE_PAGE) {
    rm2_rxpkt_PAGE:
        if (em2_remaining_bytes() <= rfctl.rxlimit) {
            // Put into DONE state
            rfctl.state    |= RADIO_STATE_RXDONE;
            rfctl.flags    &= ~RADIO_FLAG_RESIZE;

            // transition to fixed length mode
            rfctl.rxlimit   = em2_remaining_bytes();
            subcc1101_buffer_config(0, rfctl.rxlimit);

            // Final byte cannot be read unless in DONE mode.  It might be
            // in the buffer now, though, so try to get it
            //if (em2_remaining_bytes() == 1) {
            //    goto rm2_rxdata_isr_TOP;
            //}
        }
    }

    else if (rfctl.state & RADIO_STATE_MFP) {
        ot_int frames_left;
        rm2_rxpkt_MFP:
        frames_left = em2_remaining_frames();

        // If this is the last frame, move to single-frame packet mode
        // Else if more frames, but current frame is done, page it out.
        // Else, RX is in the middle of a frame (do nothing)
        if (frames_left == 0) {
            rfctl.state |= RADIO_STATE_RXPAGE;
            goto rm2_rxpkt_PAGE;
        }
        else if (em2_remaining_bytes() == 0) {
            /// @todo: I might require in the future that queue rebasing is
            ///        done in the evtdone callback (gives more flexibility).
            radio.evtdone(frames_left, (ot_int)crc_check(&em2.crc) - 1);            // argument 2 is negative on bad Frame CRC
            // Prepare the next frame by moving the "front" pointer and
            // re-initializing the decoder engine
            q_rebase(&rxq, rxq.putcursor);
            em2_decode_newframe();

            // Clear out what's leftover in the FIFO, from the new frame,
            // and re-do boundary checks on this new frame.
            goto rm2_rxdata_isr_TOP;
        }
        else {
        }
    }
#   endif

    /// AUTO
    else {
        ot_int remaining_bytes;
        remaining_bytes = em2_remaining_bytes();
        if (remaining_bytes <= rfctl.rxlimit) {
#           ifdef RADIO_IRQ2_PIN
            if (remaining_bytes == 0) {
                goto rm2_rxpkt_DONE;
            }
            cc1101_int_turnoff(RFI_RXFIFOTHR);
#           else
            cc1101_iocfg_rxend();
#           endif
            rfctl.state |= RADIO_STATE_RXDONE;
        }
    }


    /// 3. Change the size of the RX buffer to default, if required
#   if ((M2_FEATURE(MULTIFRAME) == ENABLED) || !defined(RADIO_IRQ2_PIN))
        if (rfctl.flags & RADIO_FLAG_RESIZE) {
            rfctl.flags &= ~RADIO_FLAG_RESIZE;
            cc1101_write(RFREG(FIFOTHR), (ot_u8)((rfctl.rxlimit >> 2) - 1) );
        }
#   endif

#endif
}
#endif




#ifndef EXTF_rm2_rxend_isr
void rm2_rxend_isr() {
}
#endif






#ifndef EXTF_rm2_txinit
void rm2_txinit(ot_u8 psettings, ot_sig2 callback) {
#if (M2_FEATURE(GATEWAY) || M2_FEATURE(SUBCONTROLLER))
    rfctl.flags    |= (psettings != 0) ? RADIO_FLAG_FLOOD : 0;
#endif
#if (M2_FEATURE(MULTIFRAME) == ENABLED)
    rfctl.flags    |= ((session_netstate() & M2_NETSTATE_DSDIALOG) != 0); //sets RADIO_FLAG_FRCONT
#endif
    radio.evtdone   = callback;
    radio.state     = RADIO_Csma;
    rfctl.state     = RADIO_STATE_TXINIT;

    /// @note Flush TX FIFO and set buffer threshold to 5 bytes: the encoder
    /// should be at least 20% faster than the max TX data speed (1 byte per
    /// 40 µs).  On CC430, FEC encode is possible with 20 MHz CPU.
    radio_flush_tx();
    cc1101_write(RFREG(FIFOTHR), DRF_FIFOTHR | _FIFO_TXTHR(5));

    /// CSMA-CA interrupt based and fully pre-emptive.  This is
    /// possible using CC1 on the GPTIM to clock the intervals.
    radio_set_mactimer( (ot_uint)dll.comm.tca );
}
#endif




#ifndef EXTF_rm2_txinit_ff
void rm2_txinit_ff(ot_u8 psettings, ot_sig2 callback) {
#   if (SYS_FLOOD == ENABLED)
    rfctl.flags    |= psettings;
#   endif
#   if (M2_FEATURE(MULTIFRAME) == ENABLED)
    rfctl.flags    |= ((session_netstate() & M2_NETSTATE_DSDIALOG) != 0); //sets RADIO_FLAG_FRCONT
#   endif
    radio.evtdone   = callback;
    rfctl.state     = RADIO_STATE_TXCCA1;
}
#endif





#ifndef EXTF_rm2_flood_txstop
void rm2_flood_txstop() {
#if (M2_FEATURE(GATEWAY) || M2_FEATURE(SUBCONTROLLER))
    rfctl.state = RADIO_STATE_TXDONE;
#   ifdef RADIO_IRQ2_PIN
        cc1101_int_turnoff(RFI_TXFIFOTHR);  //Disable FIFO-low interrupt
#   else
        cc1101_iocfg_txend();
#   endif
    cc1101_write(RFREG(MCSM1), 0);      //Set-up state machine to do TX->IDLE
#endif
}
#endif




#ifndef EXTF_rm2_txcsma_isr
void rm2_txcsma_isr() {
    /// 0. Go to IDLE, because register manipulations don't work otherwise.
    radio_idle();

    // The shifting in the switch is so that the numbers are 0, 1, 2, 3...
    // It may seem silly, but it allows the switch to be compiled better.
    switch ( (rfctl.state >> RADIO_STATE_TXSHIFT) & (RADIO_STATE_TXMASK >> RADIO_STATE_TXSHIFT) ) {

        /// 1. First CCA: Find a usable channel, then do 1st CCA
        /// Note: case fall-through on success
        case (RADIO_STATE_TXINIT >> RADIO_STATE_TXSHIFT): {
            if (subcc1101_chanscan() == False) {   //Get usable channel
                radio.evtdone(RM2_ERR_BADCHANNEL, 0);
                break;
            }
            if (dll.comm.csmaca_params & M2_CSMACA_NOCSMA) {
                goto rm2_txcsma_START;
            }
            /////retval = phymac[0].tg;
        }

        /// 2. CCA1: Set state and fall-through to CCA check state below
        case (RADIO_STATE_TXCCA1 >> RADIO_STATE_TXSHIFT): {
            rfctl.state = RADIO_STATE_TXCCA1;
        }

        /// 3. CCA check state (also CCA2).
        ///    If CCA1, failure->CCA1 (callback), success->CCA2 (mactimer wait)
        ///    If CCA2, failure->CCA1 (callback), success->TXSTART (fall through)
        case (RADIO_STATE_TXCCA2 >> RADIO_STATE_TXSHIFT): {
            ot_u8 result;

            result = subcc1101_ccascan();
            if (result == 0) {
                rfctl.state = RADIO_STATE_TXCCA1;
                radio.evtdone(1, 0);
                break;
            }

            rfctl.state += (result << RADIO_STATE_TXSHIFT);
            if (rfctl.state != RADIO_STATE_TXSTART) {
                if (phymac[0].tg > 2) {
                    radio_sleep();
                }
                radio_set_mactimer( phymac[0].tg );
                break;
            }
        }

        /// 3. TX startup
        /// Initially load a rather small amount of data, because right
        ///now the system is just burning energy in IDLE
        case (RADIO_STATE_TXSTART >> RADIO_STATE_TXSHIFT): {
        rm2_txcsma_START:
            if (rfctl.flags & RADIO_FLAG_SETPWR) {
                rfctl.flags &= ~RADIO_FLAG_SETPWR;
                cc1101_set_txpwr( phymac[0].tx_eirp );
            }

#           if (M2_FEATURE(SUBCONTROLLER) || M2_FEATURE(GATEWAY))
            {   sync_enum   sync_type   = SYNC_fg;
                ot_u8       mcsm1       = b00000000;
                ot_u8       buffer_mode = 1;
                ot_u8       buffer_size = 255;

                if (rfctl.flags & RADIO_FLAG_FLOOD) {
                    sync_type   = SYNC_bg;
                    mcsm1       = b00000010;
                    buffer_mode = 0;
                    buffer_size = em2_remaining_bytes();
                }
                radio.evtdone(0, buffer_mode-1);  // arg2: !0 for background, 0 for foreground
                subcc1101_buffer_config(buffer_mode, buffer_size);
                subcc1101_syncword_config( sync_type );
                cc1101_write(RFREG(MCSM1), mcsm1 );
            }
#           else
                radio.evtdone(0, 0);            // arg2: !0 for background, 0 for foreground
                subcc430_buffer_config(1, 255);
                subcc1101_syncword_config( SYNC_fg );
                //cc1101_write(RFREG(MCSM1), b00000000 );   //should be persistent default
#           endif

            // Preload into TX FIFO a relatively small amount (8 bytes) for min
            // latency.  Amount should be multiple of 4, (> 5), and small.
            cc1101_write(RFREG(FIFOTHR), DRF_FIFOTHR | _FIFO_TXTHR(5));
            rfctl.txlimit   = 8;
            txq.getcursor   = txq.front;
            txq.front[1]    = phymac[0].tx_eirp;
            subcc1101_prep_q(&txq);
            radio_flush_tx();
            em2_encode_newpacket();
            em2_encode_newframe();
            em2_encode_data();

            // Put state into TX Data, and TXlimit to maximum (after preloading)
            radio.state     = RADIO_DataTX;
            rfctl.state     = RADIO_STATE_TXDATA;
            rfctl.txlimit   = RADIO_BUFFER_TXMAX;

            cc1101_iocfg_txdata();
            cc1101_int_turnon(RFI_SOURCE0 | RFI_SOURCE2);
            cc1101_strobe(STROBE(STX));
            break;
        }
    }
}
#endif





#ifndef EXTF_rm2_txdata_isr
void rm2_txdata_isr() {
    /// Continues where rm2_txcsma() leaves off.
    switch ((rfctl.state >> RADIO_STATE_TXSHIFT) & 3 ) {

        /// 4. Continuous TX'ing of a single packet data
        case ((RADIO_STATE_TXDATA >> RADIO_STATE_TXSHIFT) & 3): {
        rm2_txpkt_TXDATA:
            /// Buffer needs filling, frame is not done
            em2_encode_data();
            if (em2_remaining_bytes() != 0) {
                break;
            }

#           if (SYS_FLOOD == ENABLED)
            /// Packet flooding.  Only needed on devices that can send M2AdvP
            if (rfctl.flags & RADIO_FLAG_FLOOD) {
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
            if (rfctl.flags & RADIO_FLAG_FRCONT) {
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

            /// The last part of the frame needs to exit the FIFO
            //rfctl.txlimit = cc1101_read(RFREG(TXBYTES));
            rfctl.state = RADIO_STATE_TXDONE;
#           ifdef RADIO_IRQ2_PIN
            cc1101_int_turnoff(RFI_TXFIFOTHR);
#           else
            cc1101_iocfg_txend();
#           endif
            break;
        }

        /// 5. Conclude the TX process, and wipe the radio state
        //     turn off any remaining TX interrupts
        case ((RADIO_STATE_TXDONE >> RADIO_STATE_TXSHIFT) & 3):
            subcc1101_finish(0, 0);
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



void subcc1101_finish(ot_int main_err, ot_int frame_err) {
    ot_sig2 callback;

    /// 1.  Turn-off interrupts & put CC1101 into IDLE
    radio_gag();
    //radio_idle();         //should already be in idle, or going to idle
    subcc1101_reset_autocal();

    /// 2.  Run callback, then reset callback and reset radio state
    radio.state     = RADIO_Idle;
    rfctl.state     = 0;
    rfctl.flags    &= RADIO_FLAG_SETPWR;    //clear all other flags
    callback        = radio.evtdone;
    radio.evtdone   = &otutils_sig2_null;
    callback(main_err, frame_err);
}




ot_bool subcc1101_lowrssi_reenter() {
    ot_int  min_rssi;
    min_rssi = ((phymac[0].cs_thr >> 1) & 0x3F) - 40;

    //if (radio_rssi() < min_rssi) {
    //    subcc1101_reset_autocal();
    //    radio_idle();               //for CC1101 version, idle must be called ahead of rm2_reenter_rx(NULL)
    //    rm2_reenter_rx(NULL);
    //    return True;
    //}
    return False;
}


void subcc1101_reset_autocal() {
/// Turn off entry-into-RX/TX calibration stage.
    if (rfctl.flags & RADIO_FLAG_AUTOCAL) {
        rfctl.flags ^= RADIO_FLAG_AUTOCAL;
        cc1101_write(RFREG(MCSM0), DRF_MCSM0);
    }
}


ot_bool subcc1101_chanscan( ) {
    vlFILE* fp;
    ot_int  i;

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





ot_bool subcc1101_ccascan() {
/// This is a blocking call, and it runs pretty fast (less than 1ms).  It uses
/// direct RSSI evaluation instead of the built-in CCA mechanism, which is not
/// reliable.  Direct RSSI evaluation is faster and more precise.  400us is a
/// safe amount of time for the RSSI to stabilize after starting RX.
    ot_bool check_cca;
    ot_u16  wait_period;

    cc1101_int_turnoff(RFI_SOURCE0 | RFI_SOURCE2);
    //radio_idle();  //should be in idle already
    cc1101_strobe( STROBE(SRX) );

    // CC1101 takes about 350us to get RSSI stabilized, and 799 to calibrate
    wait_period = (rfctl.flags & RADIO_FLAG_AUTOCAL) ? (350+799) : (350);

    // Wait for RSSI, check it against threshold, go back to idle, stop autocal
    delay_us(wait_period);
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
    ot_uni16    scratch;

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
                i   = (rfctl.state & RADIO_STATE_RXMASK);
#           endif
#           if (M2_FEATURE(FECTX) == ENABLED)
                i  |= (rfctl.state & RADIO_STATE_TXMASK);
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
        rfctl.flags |= RADIO_FLAG_SETPWR;
    }


    /// @note Settings of "mdmcfg..." are CC430 specific implementation
    /// Reprogram data rate, packet method, and modulation per upper nibble
    /// (Don't reprogram if radio is using the same channel class already)
    if ( (old_chan ^ phymac[0].channel) & 0xF0 ) {
        mdmcfg[4] |= phymac[0].channel & 0x80; //FEC enable (or not)
        if (phymac[0].channel & 0x20) {
            mdmcfg[1]   = DRF_MDMCFG4_HI;
            mdmcfg[2]   = DRF_MDMCFG3_HI;
            mdmcfg[4]   = DRF_MDMCFG1_HI; //6B preamble on Hi-speed
        }
        cc1101_spibus_io(5, 0, mdmcfg, NULL);
    }

    /// If channel frequency is different than before, move to the new channel,
    /// and tell driver to stage automatic calibration upon TX/RX entry.
    /// @note: CC1101 contains a channel-offset built-in mechanism.
    fc_i = (phymac[0].channel & 0x0F);   // Center Frequency index = lower four bits channel ID
    if ( fc_i != (old_chan & 0x0F) ) {
        cc1101_write(RFREG(CHANNR), (ot_u8)fc_i);
        rfctl.flags |= RADIO_FLAG_AUTOCAL;
    }
}




void subcc1101_syncword_config(ot_u8 sync_class) {
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




void subcc1101_prep_q(ot_queue* q) {
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
    return subcc1101_eta( rfctl.rxlimit - cc1101_rxbytes() );
}

ot_int subcc1101_eta_txi() {
    return subcc1101_eta( cc1101_txbytes() - rfctl.txlimit );
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




