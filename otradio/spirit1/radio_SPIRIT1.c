/* Copyright 2010-2013 JP Norair
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
  * @file       /otradio/spirit1/radio_SPIRIT1.c
  * @author     JP Norair
  * @version    V1.0
  * @date       2 Feb 2013
  * @brief      Radio Driver (RF transceiver) for SPIRIT1
  * @defgroup   Radio (Radio Module)
  * @ingroup    Radio
  *
  * "   Invisible airwaves crackle with life, 
  *     Bright antenna, bristle with the energy. 
  *     Emotional feedback, on timeless wavelength, 
  *     Bearing a gift beyond price, almost free.
  *
  *     All this machinery making modern music 
  *     Can still be open-hearted. 
  *     Not so coldly charted, it's really just a question 
  *     Of your honesty.  Yeah, your honesty!   "
  *
  *     -- The SPIRIT1 of Radio (I couldn't resist):
  *
  * The header file for this implementation is /otlib/radio.h.  It is universal
  * for all platforms, even though the implementation (this file) can differ.
  * There is also a header file at /otradio/spirit1/radio_SPIRIT1.h that 
  * contains macros & constants specific to this implementation.
  *
  * For DASH7 Silicon certification, there are four basic tiers of HW features:
  * 1. PHY      The HW has a buffered I/O and the basic features necessary
  * 2. PHY+     The HW can do encoding, CRC, and some packet handling
  * 3. MAC      The HW can automate some inner loops, like Adv Flood and CSMA
  * 4. MAC+     The HW has most features of the MAC integrated
  *
  * The SPIRIT1 is a high-performing RF Core that ALSO has a lot of DASH7 
  * features implemented in HW.  Some of them are a bit roundabout and are not
  * implemented in this driver because software control has been found to work
  * better for the GENERAL CASE, but if you're a plucky innovator, there is a
  * great degree of optimization possible with the SPIRIT1 core for special 
  * case applications.
  ******************************************************************************
  */

#include "radio.h"
#include "radio_SPIRIT1.h"
#include "SPIRIT1_interface.h"

#include "OT_types.h"
#include "OT_config.h"
#include "OT_utils.h"
#include "OT_platform.h"

#include "veelite.h"
#include "session.h"
#include "m2_dll.h"
#include "crc16.h"
#include "m2_encode.h"
#include "buffers.h"
#include "queue.h"



/** Universal Data declaration
  * Described in radio.h of the OTlib.
  * This driver only supports M2_PARAM_MI_CHANNELS = 1.
  */
phymac_struct   phymac[M2_PARAM_MI_CHANNELS];
radio_struct    radio;


/** Local Data declaration
  * Described in radio_SPIRIT1.h
  */
rfctl_struct rfctl;



/** Local Subroutine Prototypes  <BR>
  * ========================================================================<BR>
  */
typedef enum {
    MODE_bg     = 0,
    MODE_fgauto = 1,
    MODE_fgpage = 2
} MODE_enum;


ot_bool subrfctl_test_channel(ot_u8 channel);
void    subrfctl_launch_rx(ot_u8 channel, ot_u8 netstate);

void    subrfctl_finish(ot_int main_err, ot_int frame_err);
//ot_bool subrfctl_lowrssi_reenter();

ot_bool subrfctl_chanscan();
//void    subrfctl_ccascan();
//void    subrfctl_ccascan_isr();
void    subrfctl_ccafail_isr();
void    subrfctl_ccapass_isr();
void    subrfctl_txend_isr();

ot_bool subrfctl_channel_lookup(ot_u8 chan_id, vlFILE* fp);
void    subrfctl_chan_config(ot_u8 old_chan, ot_u8 old_eirp);
void    subrfctl_buffer_config(MODE_enum mode, ot_u16 param);

void    subrfctl_prep_q(Queue* q);
ot_int  subrfctl_eta(ot_int next_int);
ot_int  subrfctl_eta_rxi();
ot_int  subrfctl_eta_txi();
void    subrfctl_offset_rxtimeout();



//void sub_txend_isr();



/** SPIRIT1 Virtual ISR RF  <BR>
  * ========================================================================<BR>
  */
void spirit1_virtual_isr(ot_u8 code) {
    switch (code) {
        case RFIV_RXTERM:   rm2_kill();             break;
        case RFIV_RXSYNC:   rm2_rxsync_isr();       break;
        
        case RFIV_RXEND:    rm2_rxend_isr();        break;
        case 3:                 
        case RFIV_RXFIFO:   rm2_rxdata_isr();       break;
        
        case RFIV_CCAPASS:  subrfctl_ccapass_isr(); break;
        case RFIV_CCAFAIL:  subrfctl_ccafail_isr(); break;
        
        case RFIV_TXEND:    
        case 8:             subrfctl_txend_isr();   break;
        case RFIV_TXFIFO:   rm2_txdata_isr();       break;
    }
}
  
  



void radio_mac_isr() {    
    //if (radio.state == RADIO_Csma)
    platform_disable_gptim2();
    rm2_txcsma_isr();
}
    






/** Radio Core Control Functions
  * ============================================================================
  * - Need to be customized per radio platform
  */
  
#ifndef EXTF_radio_off
void radio_off() {
   spirit1_shutdown();
}
#endif

#ifndef EXTF_radio_gag
void radio_gag() {
    spirit1_int_off();
}
#endif



#ifndef EXTF_radio_sleep
void radio_sleep() {
/// Sleep on SPIRIT1 is actually STANDBY mode.  There is also a SLEEP mode on 
/// SPIRIT1 that is STANDBY+RC_Osc, but this implementation does not use the RC
/// Oscillator at all, so STANDBY is the best choice.
#if BOARD_FEATURE_RFXTALOUT
    if (spirit1.clkreq) {
        radio_idle();
    } 
    else
#endif
    // Only go to STANDBY if it is not already the present mode.
    if (rfctl.flags & RADIO_FLAG_XOON) {
        radio_idle();
        rfctl.flags &= ~RADIO_FLAG_PWRMASK;
        spirit1_strobe(RFSTROBE_STANDBY);
    }
}
#endif

#ifndef EXTF_radio_idle
void radio_idle() {
/// Idle on SPIRIT1 is READY mode.  Only go to READY if it is not already the 
/// present mode.
    radio_state last_state;
    last_state  = radio.state;
    radio.state = RADIO_Idle;
    
    // Active state -> Idle
    if (last_state != RADIO_Idle) {
        spirit1_strobe(RFSTROBE_SABORT);
    }
    
    // Standby/Sleep -> Idle
    else if ((rfctl.flags & RADIO_FLAG_XOON) == 0)  {
        rfctl.flags |= RADIO_FLAG_XOON;
        spirit1_strobe(RFSTROBE_READY);
        spirit1_waitforready();
    }
}
#endif


void spirit1_clockout_on(ot_u8 clk_param) {
/// Set the SPIRIT1 to idle, then configure the driver so it never goes into sleep 
/// or standby, and finally configure the SPIRIT1 to output the clock.
#if (BOARD_FEATURE_RFXTALOUT)
    radio_idle();
    spirit1.clkreq = True;
    spirit1_write(RFREG(MCU_CK_CONF), clk_param);
    spirit1_write(RFREG(GPIO3_CONF), (_GPIO_SELECT(RFGPO_MCU_CLK) | _GPIO_MODE_HIDRIVE));
#endif
}


void spirit1_clockout_off() {
/// This is the reverse of spirit1_clockout_on(), described above.
#if (BOARD_FEATURE_RFXTALOUT)
    spirit1.clkreq = False;
    spirit1_write(RFREG(GPIO3_CONF), RFGPO(GND));
    spirit1_write(RFREG(MCU_CK_CONF), 0);
    radio_sleep();
#endif
}



#ifndef EXTF_radio_calibrate
void radio_calibrate() {
/// SPIRIT1 has an automatic calibration of VCO that runs very fast, so OpenTag
/// will do VCO calibration for every channel access except CSMA rescans.  
/// Instead, calling this function will simply cause the RCO to get calibrated
/// next time it is used.  It is good to calibrate RCO whenever there is a 
/// temperature change more than 10C.
    rfctl.nextcal = 0;
}
#endif


#ifndef EXTF_radio_set_mactimer
void radio_set_mactimer(ot_u16 clocks) {
/// Used for high-accuracy TX/CSMA slot insertion, and flooding.
#if (SYS_FLOOD == ENABLED)
    platform_set_gptim2(clocks);
#endif
}
#endif


#ifndef EXTF_radio_get_countdown
ot_u16 radio_get_countdown() {
    return spirit1_get_counter();
}
#endif







/** Radio Module Control Functions
  * ============================================================================
  * - Need to be customized per radio platform
  */
#ifndef EXTF_radio_init
void radio_init( ) {
/// Transceiver implementation dependent    
    vlFILE* fp;
    
    /// Initialize the bus between SPIRIT1 and MCU, and load defaults.
    /// SPIRIT1 starts-up in Idle (READY), so we set the state and flags
    /// to match that.  Then, init the bus and send RADIO to sleep.
    /// SPIRIT1 can do SPI in Sleep.
    rfctl.flags     = RADIO_FLAG_XOON;
    rfctl.nextcal   = 0;
    radio.state     = RADIO_Idle;
    radio.evtdone   = &otutils_sig2_null;
    spirit1_init_bus();
    spirit1_load_defaults();
    radio_sleep();

    /// Set startup channel to a completely invalid channel ID (0x55), and run 
    /// lookup on the default channel (0x07) to kick things off.  Since the 
    /// startup channel will always be different than a real channel, the 
    /// necessary settings and calibration will always occur. 
    phymac[0].channel   = 0x55;
    phymac[0].tx_eirp   = 0x7F;
    fp                  = ISF_open_su( ISF_ID(channel_configuration) );
    subrfctl_channel_lookup(0x07, fp);
    vl_close(fp);
}
#endif



#ifndef EXTF_radio_check_cca
ot_bool radio_check_cca() {
/// CCA Method: Look if CS pin is high.
    return spirit1_check_cspin();
}
#endif


#ifndef EXTF_radio_rssi
ot_int radio_rssi() {
/// @note SPIRIT1 only guarantees RSSI reading after end of packet RX
    radio.last_rssi = spirit1_calc_rssi( spirit1_read(RFREG(RSSI_LEVEL)) );
    return radio.last_rssi;
}
#endif


#ifndef EXTF_radio_buffer
ot_u8 radio_buffer(ot_int index) {
/// Transceiver implementation dependent
/// This function is not used on the SPIRIT1
    return 0;
}
#endif


#ifndef EXTF_radio_putbyte
void radio_putbyte(ot_u8 databyte) {
/// Transceiver implementation dependent
    spirit1_write(RFREG(FIFO), databyte);
}
#endif


#ifndef EXTF_radio_putfourbytes
void radio_putfourbytes(ot_u8* data) {
/// Unused in SPIRIT1, which has HW FECTX.
}
#endif


#ifndef EXTF_radio_getbyte
ot_u8 radio_getbyte() {
/// Transceiver implementation dependent
    return spirit1_read(RFREG(FIFO));
}
#endif



#ifndef EXTF_radio_getfourbytes
void radio_getfourbytes(ot_u8* data) {
/// Probably unused for SPIRIT1, but investigation still underway to see if 
/// SPIRIT1 HW FEC can be used for RX.
    spirit1_burstread(RFREG(FIFO), 4, data);
}
#endif



#ifndef EXTF_radio_flush_rx
void radio_flush_rx() {
    spirit1_strobe( RFSTROBE_FLUSHRXFIFO );
}
#endif



#ifndef EXTF_radio_flush_tx
void radio_flush_tx() {
    spirit1_strobe( RFSTROBE_FLUSHTXFIFO );
}
#endif



#ifndef EXTF_radio_rxopen
ot_bool radio_rxopen() {
    return (ot_bool)spirit1_rxbytes();
}
#endif



#ifndef EXTF_radio_rxopen_4
ot_bool radio_rxopen_4() {
    return (ot_bool)(spirit1_rxbytes() >= 4);
}
#endif


#ifndef EXTF_radio_txopen
ot_bool radio_txopen() {
    return (ot_bool)(spirit1_txbytes() < rfctl.txlimit);
}
#endif


#ifndef EXTF_radio_txopen_4
ot_bool radio_txopen_4() {
/// Never needed with SPIRIT1, which has HW FECTX
    return radio_txopen();
}
#endif




/** Replacement Encoder functions <BR>
  * ========================================================================<BR>
  * The Encoder/Decoder module in OTlib provides some generic encode and 
  * decode functions.  They work for most radios.  The SPIRIT1 implementation
  * has these optimized encode and decode functions.
  *
  */ 

//ot_sig crc_stream_fn;

#ifdef EXTF_em2_encode_newpacket
void em2_encode_newpacket() {
}
#endif

#ifdef EXTF_em2_decode_newpacket
void em2_decode_newpacket() {
}
#endif


#ifdef EXTF_em2_encode_newframe
void em2_encode_newframe() {
    /// 1. Prepare the CRC, also adding 2 bytes to the frame length
    if (txq.options.ubyte[UPPER] != 0) {
        crc_init_stream(txq.length, txq.getcursor);
        txq.length += 2;
	}

    /// 2. Align encoder control variables with tx frame
    em2.bytes   = txq.length;
    em2.fr_info = &txq.getcursor[3];
}
#endif

#ifdef EXTF_em2_decode_newframe
void em2_decode_newframe() {
    em2.fr_info = &rxq.front[3];
    em2.state   = 0;
    em2.bytes   = 8;      // dummy length until actual length is received
}
#endif



#ifdef EXTF_em2_encode_data
void em2_encode_data() {
    ot_int  fill;
    ot_int  load;
    ot_u8   save[2];
    ot_u8*  cmd;
    
    // Loop unrolling for FIFO loading
    load = (rfctl.txlimit - spirit1_txbytes());
    
    while ((load > 0) && (em2.bytes > 0)) {
        fill = load;
        if (fill > 24) fill = 24;
        load       -= fill;
        em2.bytes  -= fill;
        
        if (txq.options.ubyte[UPPER] != 0) {
            crc_calc_nstream(fill);
        }
        cmd     = txq.getcursor;
        save[0] = *(--cmd);
        *cmd    = 0xff;
        save[1] = *(--cmd);
        *cmd    = 0x00;
        
        txq.getcursor += fill;
        spirit1_spibus_io(fill+2, 0, cmd);
        *cmd++  = save[1];
        *cmd    = save[0];
    }
    
    /// dummy SPI access to complete fill
    *(ot_u16*)save  = 0x0080;
    spirit1_spibus_io(2, 0, save);
}
#endif




#ifdef EXTF_em2_decode_data
void em2_decode_data() {
    static const ot_u8 cmd[] = { 0x01, 0xFF };
    ot_u16 grab;
    
    em2_decode_data_TOP:

    grab = spirit1_rxbytes();
    if (grab != 0) {
        if (grab > 24)  grab = 24;
        
        spirit1_spibus_io(2, grab, (ot_u8*)cmd);
        q_writestring(&rxq, spirit1.busrx, grab);
        
        if (em2.state == 0) {
            em2.state--;
            em2.bytes = (ot_int)*rxq.getcursor;
            crc_init_stream(1+em2.bytes, rxq.getcursor);
        }
        
        em2.bytes -= grab;
        crc_calc_nstream(grab);
        if (em2.bytes > 0) {
            goto em2_decode_data_TOP;
        }
    }
}
#endif








/** Radio I/O Functions
  * ============================================================================
  * -
  */

ot_bool subrfctl_test_channel(ot_u8 channel) {
    ot_bool test = True;

    if ((channel != phymac[0].channel)) {
        vlFILE* fp;
        /// Open the Mode 2 FS Config register that contains the channel list
        /// for this host, and make sure the channel we want to use is available
        /// @todo assert fp
        fp      = ISF_open_su( ISF_ID(channel_configuration) );
        test    = subrfctl_channel_lookup(channel, fp);
        vl_close(fp);
    }

    return test;
}



///@note maccfg[5] should be 1 for release, and higher (i.e. 10) for bgrx test
void subrfctl_launch_rx(ot_u8 channel, ot_u8 netstate) {
    ot_u8 maccfg[7] = { 0, RFREG(PROTOCOL2), 
                        ((DRF_PROTOCOL2 & 0x1F) | _SQI_TIMEOUT_MASK),
                        DRF_PROTOCOL1, 
                        DRF_PROTOCOL0,
                        1,
                        0 };
    MODE_enum   buffer_mode;
    ot_u16      pktlen;
    
    /// 1.  Prepare RX queue by flushing it
    q_empty(&rxq);
    subrfctl_prep_q(&rxq);
    
    /// 2. Fetch the RX channel, exit if the specified channel is not available
    if (subrfctl_test_channel(channel) == False) {
        subrfctl_finish(RM2_ERR_BADCHANNEL, 0);
        return;
    }
    
    /// 3. Prepare modem state-machine to do RX-Idle or RX-RX
    ///    RX-RX happens duing Response listening, unless FIRSTRX is high
    netstate &= (M2_NETFLAG_FIRSTRX | M2_NETSTATE_RESP);
    if ((netstate ^ M2_NETSTATE_RESP) == 0) {
        maccfg[4] = _PERS_RX;
    }

    /// 4a. Setup RX for Background detection:
    ///     <LI> Manipulate Queue to fit bg frame into common model </LI>
    ///     <LI> Set SPIRIT1 RX timer to a small amount (~250 us) </LI>
    ///     <LI> Set SPIRIT1 to pause RX timer on carrier sense </LI>
    if (rfctl.flags & RADIO_FLAG_FLOOD) {
        rxq.length          = 9;
        *(ot_u16*)rxq.front = PLATFORM_ENDIAN16_C(0x0700);
        rxq.getcursor       = &rxq.front[2];
        rxq.putcursor       = &rxq.front[2];
        rfctl.state         = RADIO_STATE_RXAUTO;
        rfctl.rxlimit       = 64;
        pktlen              = 7;
        buffer_mode         = MODE_bg;
        maccfg[2]           = ((DRF_PROTOCOL2 & 0x1F) | _CS_TIMEOUT_MASK);
        maccfg[6]           = (5 << (BOARD_PARAM_RFHz > 26000000)); 
    }

    /// 4b. Setup RX for Foreground detection:
    ///     
    else {
#   if ((M2_FEATURE(MULTIFRAME) == ENABLED) || (M2_FEATURE(FECRX) == ENABLED))
        rfctl.state     = rfctl.flags & 3;
        ///@todo 0x1000 should be MAXPKTLEN
        pktlen          = (rfctl.flags & RADIO_FLAG_FRCONT) ? 0x1000 : 0x0100;
        buffer_mode     = MODE_fgauto + (rfctl.state != RADIO_STATE_RXAUTO);
#   else
        // Initial state is always RXAUTO (2), because no FEC or Multiframe RX
        rfctl.state     = RADIO_STATE_RXAUTO;
        rfctl.rxlimit   = 64;
        pktlen          = 0x0100;
        buffer_mode     = MODE_fgauto;
#   endif
    }

    /// 5.  Send Configuration data to SPIRIT1
    spirit1_spibus_io(7, 0, maccfg);                            // MAC configuration
    subrfctl_buffer_config(buffer_mode, pktlen);                // packet configuration
    spirit1_write(RFREG(RSSI_TH), (ot_u8)phymac[0].cs_thr);     // RX CS threshold

    /// 6.  Prepare Decoder to receive, then receive
    em2_decode_newpacket();
    em2_decode_newframe();
    subrfctl_offset_rxtimeout();
    
    /// 7.  Using rm2_reenter_rx() with NULL forces entry into rx, and sets states
    spirit1_iocfg_rx();
    rm2_reenter_rx(NULL);
}



#ifndef EXTF_rm2_default_tgd
ot_int rm2_default_tgd(ot_u8 chan_id) {
#if ((M2_FEATURE(FEC) == DISABLED) && (M2_FEATURE(TURBO) == DISABLED))
    return M2_TGD_55FULL;

#elif ((M2_FEATURE(FEC) == DISABLED) && (M2_FEATURE(TURBO) == ENABLED))
    return (chan_id & 0x20) ? M2_TGD_200FULL : M2_TGD_55FULL;

#elif ((M2_FEATURE(FEC) == ENABLED) && (M2_FEATURE(TURBO) == DISABLED))
    return (chan_id & 0x80) ? M2_TGD_55HALF : M2_TGD_55FULL;

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
    pkt_bytes  += (phymac[0].channel & 0x20) ? \
                    RF_PARAM_PKT_OVERHEAD+6 : RF_PARAM_PKT_OVERHEAD+4;
    return rm2_scale_codec(pkt_bytes);
}
#endif



#ifndef EXTF_rm2_scale_codec
ot_int rm2_scale_codec(ot_int buf_bytes) {
/// Turns a number of bytes (buf_bytes) into a number of ti units.
/// To refresh your memory: 
/// 1 ti    = ((1sec/32768) * 2^5) = 2^-10 sec = ~0.977 ms
/// 1 miti   = 1/1024 ti = 2^-20 sec = ~0.954us

    /// Low-Speed + Non-FEC = 18.88 miti/bit (55.55 kbps), 
    /// Low-Speed + FEC     = 37.75 miti/bit (27.77 kbps), 
    /// Hi-Speed + Non-FEC  = 5.24 miti/bit  (200 kbps), 
    /// Hi-Speed + FEC      = 10.49 miti/bit (100 kbps), 
    static const ot_u8 bit_us[4] = { 19, 38, 6, 11 };
    ot_u8 index;
    
    index       = ((phymac[0].channel & 0x20)>>4) + (rfctl.flags & RADIO_FLAG_FEC);
    buf_bytes  *= bit_us[index];
    // buf_bytes = (buf_bytes * 8 / 1024) = (buf_bytes >> 7)
    buf_bytes >>= 7;

    return buf_bytes;
}
#endif



#ifndef rm2_reenter_rx
void rm2_reenter_rx(ot_sig2 callback) {
/// If radio is in an inactive state, restart RX using the same settings that
/// are presently in the radio core.  Always do this when forced (callback == NULL).
    if (callback == NULL) {
        radio_idle();
        goto rm2_reenter_rx_PROC;
    }
        
    if ( (spirit1_read(RFREG(MC_STATE0)) & 0x22) == 0x22 ) {
    rm2_reenter_rx_PROC:
        radio_flush_rx();
        rfctl.rxlimit = (rfctl.state == RADIO_STATE_RXAUTO) ? 64 : 8;
        spirit1_write(RFREG(FIFO_CONFIG3), (ot_u8)rfctl.rxlimit);
        spirit1_strobe( RFSTROBE_RX );
        spirit1_int_listen();
        radio.state = RADIO_Listening;
        rfctl.state = RADIO_STATE_RXINIT;
    }
}
#endif


#ifndef EXTF_rm2_resend
void rm2_resend(ot_sig2 callback) {
    radio.evtdone               = callback;
    radio.state                 = RADIO_Csma;
    rfctl.state                 = RADIO_STATE_TXINIT;
    txq.options.ubyte[UPPER]    = 255;
    platform_enable_gptim2();
    radio_set_mactimer(0);
}
#endif


#ifndef EXTF_rm2_kill
void rm2_kill() {
    radio_idle();
    subrfctl_finish(RM2_ERR_KILL, 0);
}
#endif



#ifndef EXTF_rm2_rxinit
void rm2_rxinit(ot_u8 channel, ot_u8 psettings, ot_sig2 callback) {
#if (SYS_RECEIVE == ENABLED)
    ot_u8 netstate;
    
    /// Setup the RX engine for Foreground Frame detection and RX.  Wipe-out
    /// the lower flags (non-persistent flags)
    radio.evtdone   = callback;
    rfctl.flags    &= (RADIO_FLAG_SETPWR);

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

    subrfctl_launch_rx(channel, netstate);

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
    spirit1_int_rxdata();
    radio.state = RADIO_DataRX;
    
    // This is a special hook to postpone other tasks during active RX, which
    // requires low latency.  Also, dll_block() may not do anything, depending
    // on the kernel and CPU architecture you are running.
    dll_block();
}
#endif



#ifndef EXTF_rm2_rxtimeout_isr
void rm2_rxtimeout_isr() {
    radio_idle();
    subrfctl_finish(RM2_ERR_TIMEOUT, 0);
}
#endif




#ifndef EXTF_rm2_rxdata_isr
void rm2_rxdata_isr() {
#if (SYS_RECEIVE == ENABLED)
    ot_int frames_left;
    
    /// 1. load data
    rm2_rxdata_isr_TOP:
    em2_decode_data();      // Contains logic to prevent over-run

    /// 2. Handle each RX type, and transitions
    switch ((rfctl.state >> RADIO_STATE_RXSHIFT) & (RADIO_STATE_RXMASK >> RADIO_STATE_RXSHIFT)) {
    
        /// RX State 0: Automatic Packet Control
        /// SPIRIT1 can automatically handle the following packet RX:
        /// <LI> All background packets </LI>
        /// <LI> Non-FEC, single-frame foreground packets </LI>
        case (RADIO_STATE_RXAUTO >> RADIO_STATE_RXSHIFT): {
            break;
        }

        /// RX State 1: Paging Mode
        /// Paging Mode is used when the automatic packet handler can't work.
        /// The Codec (Encode Module) knows when the packet is going to end 
        /// even if RF Core does not, so the technique here is to use Fixed-Len
        /// mode with a large size, and decrease it to the actual packet length
        /// once this is known (after first decoder pass).
#       if ((M2_FEATURE(MULTIFRAME) == ENABLED) || (M2_FEATURE(FECRX) == ENABLED))
        case (RADIO_STATE_RXPAGE >> RADIO_STATE_RXSHIFT): {
            rfctl.rxlimit = 64;
            spirit1_write(RFREG(FIFO_CONFIG3), (ot_u8)rfctl.rxlimit);
                
            // I know this is "spaghetti code," deal with it.
            // This section updates the PCKTLEN with known packet length;
            rm2_rxdata_isr_RXLASTFRAME: {
                ot_u16 pktlen;
                ot_u8  cmd[4];
                rfctl.flags    &= ~(RADIO_FLAG_RESIZE | RADIO_FLAG_FEC);
                pktlen          = rxq.length + em2_remaining_bytes();
                cmd[0]          = 0;
                cmd[1]          = RFREG(PCKTLEN1);
                cmd[2]          = ((ot_u8*)&pktlen)[UPPER];
                cmd[3]          = ((ot_u8*)&pktlen)[LOWER];
                spirit1_spibus_io(4, 0, cmd);
            }
            
            rfctl.state = RADIO_STATE_RXAUTO;
        } break;
#       endif

        /// RX State 2 & 3:
        /// Multiframe packets (only compiled when MFPs are supported)
#       if (M2_FEATURE(MULTIFRAME) == ENABLED)
        case (RADIO_STATE_RXMFP >> RADIO_STATE_RXSLOT):
        case (RADIO_STATE_RXMFP >> RADIO_STATE_RXSHIFT): {
            frames_left = em2_remaining_frames();

            // If this is the last frame, move to single-frame packet mode
            // Else if more frames, but current frame is done, page it out.
            // Else, RX is in the middle of a frame (do nothing)
            if (frames_left == 0) {
                rfctl.state = RADIO_STATE_RXPAGE;
                goto rm2_rxdata_isr_RXLASTFRAME;
            }
            if (em2_remaining_bytes() == 0) {
                /// @todo: I might require in the future that queue rebasing is
                ///        done in the evtdone callback (gives more flexibility).
                // argument 2 is negative on bad Frame CRC
                radio.evtdone(frames_left, (ot_int)crc_check() - 1);            

                // Prepare the next frame by moving the "front" pointer and
                // re-initializing the decoder engine
                q_rebase(&rxq, rxq.putcursor);
                em2_decode_newframe();

                // Clear out what's leftover in the FIFO, from the new frame,
                // and re-do boundary checks on this new frame.
                goto rm2_rxdata_isr_TOP;
            }
            break;
        }
#       endif
        
        /// Bug Trap
        default:
            rm2_kill();
            break;
    }
#endif
}
#endif




#ifndef EXTF_rm2_rxend_isr
void rm2_rxend_isr() {
    rfctl.state = RADIO_STATE_RXDONE;           // Make sure in DONE State, for decoding
    em2_decode_data();                          // decode any leftover data
    subrfctl_finish(0, (ot_int)crc_check() - 1);
}
#endif




#ifndef EXTF_rm2_txinit
void rm2_txinit(ot_u8 psettings, ot_sig2 callback) {
#if (SYS_FLOOD == ENABLED)
    rfctl.flags    |= (psettings != 0) ? RADIO_FLAG_FLOOD : 0;
#endif
#if (M2_FEATURE(MULTIFRAME) == ENABLED)
    rfctl.flags    |= ((session_netstate() & M2_NETSTATE_DSDIALOG) >> 1); //sets RADIO_FLAG_FRCONT
#endif
    radio.evtdone   = callback;
    radio.state     = RADIO_Csma;
    rfctl.state     = RADIO_STATE_TXINIT;
    
    /// CSMA-CA interrupt based and fully pre-emptive.  This is
    /// possible using CC1 on the GPTIM to clock the intervals.
    platform_enable_gptim2();
    radio_set_mactimer( (ot_uint)dll.comm.tca );
}
#endif




#ifndef EXTF_rm2_txstop_flood
void rm2_txstop_flood() {
#if (SYS_FLOOD == ENABLED)
    rfctl.state = RADIO_STATE_TXDONE;
    spirit1_stop_counter();
    spirit1_int_turnoff(RFI_TXFIFO);                    //Disable TXFIFO interrupt
    spirit1_write(RFREG(PROTOCOL0), DRF_PROTOCOL0);     //Disable Persistent TX
#endif
}
#endif




#ifndef EXTF_rm2_txcsma_isr
void rm2_txcsma_isr() {
    // The shifting in the switch is so that the numbers are 0, 1, 2, 3...
    // It may seem silly, but it allows the switch to be compiled better.
    switch ( (rfctl.state >> RADIO_STATE_TXSHIFT) & (RADIO_STATE_TXMASK >> RADIO_STATE_TXSHIFT) ) {
        
        /// 1. First CCA: make sure channel is available.  Bypass if NOCSMA.
        /// <LI> Send error if device does not support one of the response channels </LI>
        /// <LI> Now that channel is known, prepare data for TX </LI>
        /// <LI> If NOCSMA is net, bypass CSMA process </LI>
        /// <LI> Fall-through to CCA1 if CSMA is enabled </LI>
        case (RADIO_STATE_TXINIT >> RADIO_STATE_TXSHIFT): {
            static const ot_u8 timcfg_flood[9] = {
                0, RFREG(PROTOCOL0),
                (DRF_PROTOCOL0 | _PERS_TX), // _PERS_TX used on FLOODS
                1, 5,                       // RX Timer @ ~240us
                1, 4,                       // LDC Entry timer @ ~115us
                33, 0                       // LDC Interval timer @ 1tick resolution
            };
            static const ot_u8 timcfg_normal[9] = {
                0, RFREG(PROTOCOL0), (DRF_PROTOCOL0), 1, 5, 1, 4, 33, 0 
            };
            ot_u8*      timcfg;
            MODE_enum   type;
            ot_u16      pktlen;
            
            // Find a usable channel from the TX channel list.  If none, error.
            if (subrfctl_chanscan() == False) {
                radio.evtdone(RM2_ERR_BADCHANNEL, 0);
                break;
            }
            
            // Configure encoder.  On SPIRIT1 TX, this needs to be done before
            // calling subrfctl_buffer_config().
            subrfctl_prep_q(&txq);
            em2_encode_newpacket();
            em2_encode_newframe();
            
            // Set TX PATABLE values if different than pre-existing values
            if (rfctl.flags & RADIO_FLAG_SETPWR) {
                rfctl.flags &= ~RADIO_FLAG_SETPWR;
                spirit1_set_txpwr( phymac[0].tx_eirp );
            }
            
            // Set other TX Buffering & Packet parameters, and also save the
            // Peristent-TX attribute for floods, which is written later
            if (rfctl.flags & RADIO_FLAG_FLOOD) {
                pktlen  = 7;
                type    = MODE_bg;
                timcfg  = (ot_u8*)timcfg_flood;
            }
            else {
                pktlen  = txq.length;
                type    = (rfctl.flags & 3) ? MODE_fgpage : MODE_fgauto;
                timcfg  = (ot_u8*)timcfg_normal;
            }
            subrfctl_buffer_config(type, pktlen);
            spirit1_int_off();
            spirit1_iocfg_tx();
            
            // START CSMA if required, using LDC mode
            if ((dll.comm.csmaca_params & M2_CSMACA_NOCSMA) == 0) {
                ot_u8 protocol2;
                spirit1_spibus_io(9, 0, timcfg);
                radio_idle();
                
                // Calibrate LDC RCO clock every X uses of CSMA.
                protocol2 = (_VCO_CALIBRATION | _LDC_MODE);
                if (rfctl.nextcal == 0) {
                    rfctl.nextcal   = RF_PARAM(RCO_CAL_INTERVAL);
                    protocol2      |= _RCO_CALIBRATION;
                }
                spirit1_write(RFREG(PROTOCOL2), protocol2);
                spirit1_strobe(RFSTROBE_SLEEP);
                spirit1_int_csma();
                break;
            }
            
            // START TX if CSMA disabled for this packet.
            // Note: fall through
            spirit1_spibus_io(3, 0, timcfg);
        }

        /// 3. TX startup: 'nuff said
        case (RADIO_STATE_TXSTART >> RADIO_STATE_TXSHIFT): {
        rm2_txcsma_START:
            // Send TX start (CSMA done) signal to DLL task
            // arg2: 0 for background, 1 for foreground
            radio.evtdone(0, (rfctl.flags & RADIO_FLAG_FLOOD));  
            
            // Preload into TX FIFO a small amount of data (up to 8 bytes)
            // This is small-enough that the TX state machine doesn't need
            // special conditions, and less initial data = less latency.
            rfctl.txlimit   = 8;
            txq.getcursor   = txq.front;
            txq.front[1]    = phymac[0].tx_eirp;
            
            radio_idle(); 
            radio_flush_tx();
            em2_encode_data();
            
            // Prepare for TX, then enter TX
            // For floods, we must activate the flood counter right before TX
            radio.state     = RADIO_DataTX;
            rfctl.state     = RADIO_STATE_TXDATA;
            rfctl.txlimit   = RADIO_BUFFER_TXMAX;   // Change TXlimit to max buffer
            if (rfctl.flags & RADIO_FLAG_FLOOD) {
                spirit1_start_counter();
            }
            spirit1_strobe( RFSTROBE_TX );
            spirit1_int_txdata();
            break;
        }
    }
}
#endif



void subrfctl_ccafail_isr() {
/// CCA has been pre-empted by a Sync-detect or a CS interrupt, which of course
/// means that the CCA scan has failed.
    spirit1_int_off();
    radio_idle();
    spirit1_write(RFREG(PROTOCOL2), 0);     //Turn-off LDC, RCO-Cal, VCO-Cal
    radio_sleep();
    
    rfctl.state = RADIO_STATE_TXCCA1;
    radio.evtdone(1, 0);
}


void subrfctl_ccapass_isr() {
/// CCA scan has passed.  SPIRIT1 is in SLEEP-state
    rfctl.state += (1<<RADIO_STATE_TXSHIFT);
    
    // CSMA process is done
    if (rfctl.state == RADIO_STATE_TXSTART) {
        spirit1_write(RFREG(PROTOCOL2), 0);     //Turn-off LDC, RCO-Cal, VCO-Cal
        rm2_txcsma_isr();
    }
    
    // CSMA process has another pass: reload LDC with guard time & exit
    else {
        spirit1_strobe(RFSTROBE_LDC_RELOAD);
        spirit1_write(RFREG(PROTOCOL2), _LDC_MODE);     //Turn-off VCO, RCO-Cal, VCO-Cal
    }
}






#ifndef EXTF_rm2_txdata_isr
void rm2_txdata_isr() {
    
    /// Continues where rm2_txcsma() leaves off.
    if ((rfctl.state & RADIO_STATE_TXMASK) == RADIO_STATE_TXDATA) {
        rm2_txpkt_TXDATA:
        /// Frame is not done, so come back later to fill buffer again
        em2_encode_data();
        if (em2_remaining_bytes() != 0) {
            return;
        }

#       if (SYS_FLOOD == ENABLED)
        /// Packet flooding.  Only needed on devices that can send M2AdvP
        if (rfctl.flags & RADIO_FLAG_FLOOD) {
            radio.evtdone(2, 0);
            txq.getcursor = txq.front;
            em2_encode_newframe();
            goto rm2_txpkt_TXDATA;
        }
#       endif

#       if (M2_FEATURE(MULTIFRAME) == ENABLED)
        /// If the frame is done, but more need to be sent (e.g. MFP's)
        /// queue it up.  The additional encode stage is there to fill up
        /// what's left of the buffer.
        if (em2_remaining_frames() != 0) {
            q_rebase(&txq, txq.getcursor);
            radio.evtdone(1, 0);        //callback action for next frame
            em2_encode_newframe();
            txq.front[1] = phymac[0].tx_eirp;
            goto rm2_txpkt_TXDATA;
        }
#       endif
    
        /// TX is finishing: disable Threshold interrupt and wait for READY
        rfctl.state = RADIO_STATE_TXDONE;
        spirit1_int_turnoff(RFI_TXFIFO);
    }

    /// Bug trap
    else {
        rm2_kill();
    }
}
#endif


void subrfctl_txend_isr() {
///@todo could put (rfctl.state != RADIO_STATE_TXDONE) as an argument, or 
///      something that resolves to an appropriate non-zero,as an arg in order 
///      to signal an error
    subrfctl_finish(0, 0);
}




/** Radio Subroutines
  * ============================================================================
  * - Usually some minor adjustments needed when porting to new platform
  * - See integrated notes for areas sensitive to porting
  */

void subrfctl_null(ot_int arg1, ot_int arg2) { }



void subrfctl_finish(ot_int main_err, ot_int frame_err) {
    ot_sig2 callback;

    /// 1. Turn-off radio interrupts @ goto idle
    radio_gag();
    
    /// 2. Reset radio & callback to null state, then run saved callback
    radio.state     = RADIO_Idle;
    rfctl.state     = 0;
    rfctl.flags    &= RADIO_FLAG_SETPWR;    //clear all other flags
    callback        = radio.evtdone;
    radio.evtdone   = &otutils_sig2_null;
    callback(main_err, frame_err);
}


//ot_bool subrfctl_lowrssi_reenter() {
///@todo SPIRIT1 has features that make this function unnecessary
//    return False;
//}


ot_bool subrfctl_chanscan( ) {
    vlFILE* fp;
    ot_int  i;

    fp = ISF_open_su( ISF_ID(channel_configuration) );
    ///@todo assert fp

    /// Go through the list of tx channels
    /// <LI> Make sure the channel ID is valid. </LI>
    /// <LI> Make sure the transmission can fit within the contention period. </LI>
    /// <LI> Scan it, to make sure it can be used. </LI>
    for (i=0; i<dll.comm.tx_channels; i++) {
        if (subrfctl_channel_lookup(dll.comm.tx_chanlist[i], fp) != False) {
            break;
        }
    }

    vl_close(fp);
    return (ot_bool)(i < dll.comm.tx_channels);
}



ot_bool subrfctl_channel_lookup(ot_u8 chan_id, vlFILE* fp) {
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

    /// If FEC is requested by the new channel, but this device does not support
    /// FEC, then make sure to return False.    
    /// @todo why is this code necessary?
    if (fec_id) {
#       if (M2_FEATURE(FEC) == ENABLED)
            i = 0;
#           if (M2_FEATURE(FECRX) == ENABLED)
                i   = (rfctl.state & RADIO_STATE_RXMASK);
#           endif
#           if (M2_FEATURE(FECTX) == ENABLED)
                i  |= (rfctl.state & RADIO_STATE_TXMASK);
#           endif
            if (i == 0) return False;
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

            /// SPIRIT1 has different encoding for CS & CCA than DASH7 does.
            /// It's actually quite similar, but it isn't identical, so this
            /// function will convert the thresholds from DASH7 to SPIRIT1.
            phymac[0].cs_thr    = spirit1_calc_rssithr(phymac[0].cs_thr);
            phymac[0].cca_thr   = spirit1_calc_rssithr(phymac[0].cca_thr);

            subrfctl_chan_config(old_chan_id, old_tx_eirp);
            return True;
        }
    }

    return False;
}




void subrfctl_chan_config(ot_u8 old_chan, ot_u8 old_eirp) {
/// Called by subrfctl_channel_lookup()
/// Duty: perform channel setup and recalibration when moving from one channel
/// to another.
    static const ot_u8 drate_matrix[14] = { 
        0, RFREG(MOD1), DRF_MOD1_LS, DRF_MOD0_LS, DRF_FDEV0, DRF_CHFLT_LS, 0, 0, 
        0, RFREG(MOD1), DRF_MOD1_HS, DRF_MOD0_HS, DRF_FDEV0, DRF_CHFLT_HS
    };

    ot_u8 fc_i;

#   if (M2_FEATURE(FEC) == ENABLED)
    rfctl.flags |= (phymac[0].channel & 0x80) != 0;
#   endif

    /// Flag PA table reprogram (done before TX): only flag if power is different
    if (old_eirp != phymac[0].tx_eirp) {
        rfctl.flags |= RADIO_FLAG_SETPWR;
    }

    /// Configure data rate: only change registers if required
    if ( (old_chan ^ phymac[0].channel) & 0x70 ) {
        spirit1_spibus_io( 6, 0, (ot_u8*)&drate_matrix[(phymac[0].channel & 0x20) >> 2] );
    }

    /// Configure Channel: only change registers if required
    fc_i = (phymac[0].channel & 0x0F);
    if ( fc_i != (old_chan & 0x0F) ) {
        spirit1_write(RFREG(CHNUM), fc_i);
    }
}




void subrfctl_buffer_config(MODE_enum mode, ot_u16 param) {
/// SPIRIT1 buffer config: 
/// Mode | MF | FR | FEC | DIR || PKTCTRL2 | PKTCTRL1 | PKTLEN1:0
/// -----+----+----+-----+-----++----------+----------+-----------
///  00  |  N | BG |  N  |  X  || 00ccc010 | ddd10000 |  0x0007
///  00  |  N | BG |  Y  |  X  || 00ccc010 | ddd10001 |  0x0007
///  01  |  N | FG |  N  |  X  || 00ccc011 | ddd10000 |  0x0100
///  02  |  N | FG |  Y  |  T  || 00ccc010 | ddd10001 |   plen
///  02  |  N | FG |  Y  |  R  || 00ccc010 | ddd10001 |  0x0100 (adjusted in transfer)
///  02  |  Y | FG |  X  |  T  || 00ccc010 | ddd1000X |   plen
///  02  |  Y | FG |  X  |  R  || 00ccc010 | ddd1000X |  MAXPKT (adjusted in transfer)
///
/// ccc: channel dependent: 011 for LowSpeed, 101 for HiSpeed
/// ddd: based on defaults.  CRC isn't fully tested yet, so 000 for now
/// plen: length of the packet 

//    static const ot_u8 sync_matrix[] = { 0xE6, 0xD0,    //bg non-fec
//                                         0x0B, 0x67,    //fg non-fec
//                                         0x0B, 0x67,    //fg non-fec
//                                         0x00, 0x00,    //null
//                                         0xF4, 0x98,    //bg fec
//                                         0x19, 0x2F,    //fg fec
//                                         0x19, 0x2F     //fg fec
//                                         };
    static const ot_u8 sync_matrix[] = { 0xD0, 0xE6,    //bg non-fec
                                         0x67, 0x0B,    //fg non-fec
                                         0x67, 0x0B,    //fg non-fec
                                         0x00, 0x00,    //null
                                         0x98, 0xF4,    //bg fec
                                         0x2F, 0x19,    //fg fec
                                         0x2F, 0x19     //fg fec
                                         };
    ot_u8 buf_cfg[10];
    ot_u8 is_fec;
    ot_u8 is_hs;
    
    ///@todo optimize memory storage
    buf_cfg[0]  = 0;
    buf_cfg[1]  = RFREG(PCKTCTRL2);
    is_hs       = (phymac[0].channel & 0x20) >> 1;
    buf_cfg[2]  = DRF_PCKTCTRL2_LSBG + is_hs;
    buf_cfg[2] |= (mode & 1);
    is_fec      = (phymac[0].channel & 0x80) >> 4;
    mode      <<= 1;
    mode       += is_fec;                              
    buf_cfg[3]  = /*_CRC_MODE_1021 |*/ _WHIT_EN | (is_fec >> 3);
    buf_cfg[4]  = ((ot_u8*)&param)[UPPER];
    buf_cfg[5]  = ((ot_u8*)&param)[LOWER];
    buf_cfg[6]  = 0;
    buf_cfg[7]  = sync_matrix[mode];
    buf_cfg[8]  = sync_matrix[mode+1];
    buf_cfg[9]  = 0xAA;
    
    spirit1_spibus_io(10, 0, buf_cfg);
}




void subrfctl_prep_q(Queue* q) {
/// Put some special data in the queue options field.
/// Lower byte is encoding options (i.e. FEC)
/// Upper byte is processing options (i.e. CRC)
    q->options.ubyte[UPPER]    += 1;
    q->options.ubyte[LOWER]     = 0;
    //q->options.ubyte[LOWER]     = (phymac[0].channel & 0x80);
}




ot_int subrfctl_eta(ot_int next_int) {
/// Subtract by a tolerance amount, floor at 0
    ot_int eta = rm2_scale_codec(next_int) - 1;
    return (eta > 0) ? eta : 0;
}


ot_int subrfctl_eta_rxi() {
    return subrfctl_eta( RFGET_RXFIFO_NEXTINT() );
}

ot_int subrfctl_eta_txi() {
    return subrfctl_eta( RFGET_TXFIFO_NEXTINT() );
}





void subrfctl_offset_rxtimeout() {
/// If the rx timeout is 0, set it to a minimally small amount, which relates to
/// the rounded-up duration of an M2AdvP packet: 1, 2, 3, or 4 ti.
///@note this is a safety function that could be removed for certain builds
    static const ot_u8 min_ti_lut[8] = { 3, 1, 0, 0, 4, 2, 0, 0 };
    ot_u8 min_ti;
    min_ti = min_ti_lut[((phymac[0].channel & 0xA0) >> 5)];
    
    ///@todo check if TI2CLK is needed
    //if ( CLK2TI(dll.comm.rx_timeout) < min_ti)
    //    dll.comm.rx_timeout = TI2CLK(min_ti);
    if (dll.comm.rx_timeout < min_ti)
        dll.comm.rx_timeout = min_ti;
}




