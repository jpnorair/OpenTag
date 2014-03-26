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
  * @file       /otradio/null/radio_null.c
  * @author     JP Norair
  * @version    R100
  * @date       4 Feb 2014
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
  * @note Null radio simulator is not especially optimized
  * @note Null radio simulator requires System Watchdog to be enabled
  ******************************************************************************
  */

#include "OT_platform.h"
#if (OT_FEATURE(M2) == ENABLED)

#include "OT_types.h"
#include "OT_config.h"
#include "OT_utils.h"

#include "radio.h"
#include "radio_NULL.h"
//#include "NULL_interface.h"

#include "veelite.h"
#include "session.h"
#include "m2_dll.h"
#include "crc16.h"
#include "m2_encode.h"
#include "buffers.h"
#include "queue.h"


/** Some local constants, variables, macros
  */
#define _MAXPKTLEN (M2_PARAM(MAXFRAME) * M2_PARAM(MFPP))
#if (_MAXPKTLEN == 0)
#   warning "Max packet length could not be derived from app_config.h, using 256 bytes"
#   undef _MAXPKTLEN
#   define _MAXPKTLEN   256
#endif

#if (0)
#   define _DSSS
#   define _SPREAD      5
#   define _RXMAXTHR    80
#   define _RXMINTHR    (_SPREAD * 2)
#else
#   define _SPREAD      1
#   define _RXMAXTHR    64
#   define _RXMINTHR    8
#endif

#ifndef DRF_SYNC_BYTES
#   define DRF_SYNC_BYTES   4
#endif





rfctl_struct rfctl;


/** PHY-MAC Array declaration
  * Described in radio.h of the OTlib.
  * This driver only supports M2_PARAM_MI_CHANNELS = 1.
  */
phymac_struct       phymac[M2_PARAM_MI_CHANNELS];
radio_struct        radio;
//null_radio_struct   null_radio;


ot_u8   fake_data[128];
ot_int  fake_put;
ot_int  fake_get;





/** Local Subroutine Prototypes  <BR>
  * ========================================================================<BR>
  */
typedef enum {
    MODE_bg     = 0,
    MODE_fg     = (2 << (DRF_SYNC_BYTES > 2))
} MODE_enum;

//void    subrfctl_kill(ot_int main_err, ot_int frame_err);
void    subrfctl_finish(ot_int main_err, ot_int frame_err);

ot_u8   subrfctl_rssithr_calc(ot_u8 input, ot_u8 offset); // Fiddling necessary
ot_bool subrfctl_cca_init();
ot_bool subrfctl_chan_scan( );
ot_bool subrfctl_cca_scan();

ot_bool subrfctl_channel_fastcheck(ot_u8 chan_id);
ot_bool subrfctl_channel_lookup(ot_u8 chan_id, vlFILE* fp);
void    subrfctl_chan_config(ot_u8 old_chan, ot_u8 old_eirp);
void    subrfctl_buffer_config(MODE_enum mode, ot_u16 param);

ot_bool subrfctl_chanscan();
void    subrfctl_unsync_isr();

void    subrfctl_set_txpwr(ot_u8 eirp_code);
void    subrfctl_prep_q(Queue* q);
ot_int  subrfctl_eta(ot_int next_int);
ot_int  subrfctl_eta_rxi();
ot_int  subrfctl_eta_txi();
void    subrfctl_offset_rxtimeout();

ot_bool subrfctl_channel_lookup(ot_u8 chan_id, vlFILE* fp);





/** Null RF Virtual Interrupt Handler  <BR>
  * ========================================================================<BR>
  */
void radio_isr(void) {
//    switch (null_radio.imode) {
//        case MODE_listen:   rm2_rxsync_isr();   break;
//        case MODE_rxdata:   rm2_rxdata_isr();   break;
//        case MODE_rxend:    rm2_rxend_isr();    break;
//        case MODE_txcsma:   break;
//        case MODE_txdata:   
//        case MODE_txend:    rm2_txdata_isr();   break;
//    }
}



void radio_mac_isr() {    
    //if (rfctl.state == RADIO_Csma)
    
    //platform_disable_gptim2();
    rm2_txcsma_isr();
}
    





/** Radio Core Control Functions
  * ============================================================================
  * - Need to be customized per radio platform
  */
  
OT_WEAK void radio_off() {
/// Null radio doesn't really exist, so do same as sleep
   radio_sleep();
}

OT_WEAK void radio_gag() {
}

OT_WEAK void radio_sleep() {
	rfctl.flags &= ~RADIO_FLAG_XOON;
}

void sub_force_idle() {
    if ((rfctl.flags & RADIO_FLAG_XOON) == 0)  {
        rfctl.flags |= RADIO_FLAG_XOON;
    }
}

OT_WEAK void radio_idle() {
/// Only go to Idle if it is not already the present mode.
    radio_state last_state;
    last_state  = radio.state;
    radio.state = RADIO_Idle;
    
    // Active state -> Idle
    if (last_state != RADIO_Idle) {
        //spirit1_strobe(RFSTROBE_SABORT);
    }
    
    // Standby/Sleep -> Idle
    sub_force_idle();
}

OT_WEAK void radio_calibrate() {
}

OT_WEAK void radio_set_mactimer(ot_u16 clocks) {
/// Used for high-accuracy TX/CSMA slot insertion, and flooding.
    //platform_set_gptim2(clocks);
}


ot_u32 macstamp;

OT_WEAK ot_u16 radio_get_countdown() {
    ot_u16 value;
    value = dll.counter - 0; //(ot_u16)platform_get_interval(&macstamp);
    return value;
}






/** Radio Module Control Functions
  * ============================================================================
  * - Need to be customized per radio platform
  */
OT_WEAK void radio_init( ) {
    vlFILE* fp;

    //null_radio_init();
    fake_get = 0;
    fake_put = 0;


    /// Set startup channel to a completely invalid channel ID (0x55), and run 
    /// lookup on the default channel (0x07) to kick things off.  Since the 
    /// startup channel will always be different than a real channel, the 
    /// necessary settings and calibration will always occur. 
    phymac[0].channel   = 0x55;
    phymac[0].tx_eirp   = 0x7F;
    rfctl.flags         = 0;
    rfctl.state         = 0;
    radio.state         = RADIO_Idle;
    radio.evtdone       = &otutils_sig2_null;
    fp                  = ISF_open_su( ISF_ID(channel_configuration) );
    subrfctl_channel_lookup(0x07, fp);
    vl_close(fp);

    // radio will be in sleep mode here
}



#ifndef EXTF_radio_mac_configure
OT_WEAK void radio_mac_configure() {
/// Null radio does not do any HW Mac filtering.
}
#endif


#ifndef EXTF_radio_mac_filter
OT_WEAK ot_bool radio_mac_filter() {
/// Link Budget Filtering (LBF) is a normalized RSSI Qualifier.
/// Subnet Filtering is an numerical qualifier
    // TX EIRP encoded value    = (dBm + 40) * 2
    // TX EIRP dBm              = ((encoded value) / 2) - 40
    // Link Loss                = TX EIRP dBm - Detected RX dBm
    // Link Quality Filter      = (Link Loss <= Link Loss Limit)
    ot_u8 qualifier = (ot_u8)(radio.last_linkloss <= ((ot_int)phymac[0].link_qual)<<1);
    {   ot_u8 fr_subnet, dsm, specifier, mask;
        fr_subnet   = rxq.front[3];
        dsm         = dll.netconf.subnet & 0x0F;
        mask        = fr_subnet & dsm;
        specifier   = (fr_subnet ^ dll.netconf.subnet) & 0xF0;
        fr_subnet  &= 0xF0;
        qualifier  &= (ot_bool)(((fr_subnet == 0xF0) || (specifier == 0)) && (mask == dsm));
    }
    return qualifier;
}
#endif



#ifndef EXTF_radio_check_cca
OT_WEAK ot_bool radio_check_cca() {
/// This is a random simulation.  If the random byte (0-255) is less than 250,
/// the channel is considered open.  You can change the value (default 250) to
/// change the amount of simulated network traffic.
    return (ot_bool)(platform_prand_u8() < 250);
}
#endif



#ifndef EXTF_radio_rssi
OT_WEAK ot_int radio_rssi() {
/// Return -49 >= RSSI >= -111
/// Change the range by changing the constants
    ot_int  ranging;
    ot_u8   rand;

    rand    = platform_prand_u8();
    ranging = rand & 0x1F;
    if (rand & 0x80) {
        ranging = 0 - ranging;
    }
    
    radio.last_rssi = 80 + ranging;

    return radio.last_rssi;
}
#endif


#ifndef EXTF_radio_calc_link
OT_WEAK void radio_calc_link() {
    radio.last_rssi     = radio_rssi();
    radio.last_linkloss = (ot_int)(rxq.front[2] & 0x7F) - 80 + RF_HDB_RXATTEN;
    radio.last_linkloss-= radio.last_rssi;
}
#endif



#ifndef EXTF_radio_putbyte
OT_WEAK void radio_putbyte(ot_u8 databyte) {
/// Transceiver implementation dependent
    fake_data[fake_put++] = databyte;
}
#endif


#ifndef EXTF_radio_putfourbytes
OT_WEAK void radio_putfourbytes(ot_u8* data) {
/// @note Eventually I would like to use an endian-independent implementation,
/// (basically, this is the big endian version) but now is not the time. Doing
/// so will involve revising the FEC encoder in the Encode Module.
#if (M2_FEATURE(FEC) == ENABLED)
#   ifdef __BIG_ENDIAN__
        fake_data[fake_put++] = data[0];
        fake_data[fake_put++] = data[1];
        fake_data[fake_put++] = data[2];
        fake_data[fake_put++] = data[3];
#   else
        fake_data[fake_put++] = data[3];
        fake_data[fake_put++] = data[2];
        fake_data[fake_put++] = data[1];
        fake_data[fake_put++] = data[0];
#   endif
#endif
}
#endif


#ifndef EXTF_radio_getbyte
OT_WEAK ot_u8 radio_getbyte() {
/// Transceiver implementation dependent
    fake_put--;
    return fake_data[fake_get++];
}
#endif


#ifndef EXTF_radio_getfourbytes
OT_WEAK void radio_getfourbytes(ot_u8* data) {
    fake_put -= 4;
    data[0] = fake_data[fake_get++];
    data[1] = fake_data[fake_get++];
    data[2] = fake_data[fake_get++];
    data[3] = fake_data[fake_get++];
}
#endif


#ifndef EXTF_radio_flush_rx
OT_WEAK void radio_flush_rx() {
    fake_get = 0;
}
#endif


#ifndef EXTF_radio_flush_tx
OT_WEAK void radio_flush_tx() {
    fake_put = 0;
}
#endif


#ifndef EXTF_radio_rxopen
OT_WEAK ot_bool radio_rxopen() {
    return (ot_bool)(fake_put > 0);
}
#endif



#ifndef EXTF_radio_rxopen_4
OT_WEAK ot_bool radio_rxopen_4() {
    return radio_rxopen();
}
#endif


#ifndef EXTF_radio_txopen
OT_WEAK ot_bool radio_txopen() {
    return (ot_bool)(fake_put < rfctl.txlimit);
}
#endif


#ifndef EXTF_radio_txopen_4
OT_WEAK ot_bool radio_txopen_4() {
/// Use commented-out version, or alternatively just never set txlimit above
/// (RADIO_BUFFER_TXMAX-4)
	//ot_u8 fifo_limit = (rfctl.txlimit < (RADIO_BUFFER_TXMAX-4)) ? \
	//						(ot_u8)rfctl.txlimit : (RADIO_BUFFER_TXMAX-4);
    //return (ot_bool)(null_radio_txbytes() < fifo_limit);
    
    return radio_txopen();
}
#endif








/** Radio I/O Functions
  * ============================================================================
  * @todo globalize the routines here which can be globalized, in radio_task.c
  */

ot_bool subrfctl_test_channel(ot_u8 channel) {
///@todo this can be globalized
    ot_bool test;

    test = subrfctl_channel_fastcheck(channel);
    if (test == False) {
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



void subrfctl_launch_rx(ot_u8 channel, ot_u8 netstate) {
    MODE_enum   buffer_mode;
    ot_u16      pktlen;
    
    /// 1.  Prepare RX queue by flushing it
    //rfctl.rxlimit = 256;
    q_empty(&rxq);
    subrfctl_prep_q(&rxq);
    
    /// 2. Fetch the RX channel, exit if the specified channel is not available
    if (subrfctl_test_channel(channel) == False) {
        subrfctl_finish(RM2_ERR_BADCHANNEL, 0);
        return;
    }
    
    /// 3. Prepare modem state-machine to do RX-Idle or RX-RX
    ///    RX-RX happens during Response listening, unless FIRSTRX is high
    //netstate &= (M2_NETFLAG_FIRSTRX | M2_NETSTATE_RESP);
    //if ((netstate ^ M2_NETSTATE_RESP) == 0) {
    //    maccfg[4] = _PERS_RX | _NACK_TX;
    //}

    /// 4a. Setup RX for Background detection (if FLOOD), else Foreground reception
    if (rfctl.flags & RADIO_FLAG_FLOOD) {
        rxq.getcursor       = rxq.front;
        *rxq.getcursor++    = 8;
        *rxq.getcursor++    = 2;            // 00010 is CRC5 for 0000100000
        rxq.putcursor       = rxq.getcursor;
        buffer_mode         = MODE_bg;
        pktlen              = 7;
    }
    else {
        buffer_mode = MODE_fg;
        pktlen      = _MAXPKTLEN;
    }

    /// 5. Configure packet handler for selected channel
    subrfctl_buffer_config(buffer_mode, pktlen);

    /// 6.  Prepare Decoder to receive, then receive
    em2_decode_newpacket();
    em2_decode_newframe();
    //dll_offset_rxtimeout();
    
    /// 8.  Using rm2_reenter_rx()
    rm2_reenter_rx(radio.evtdone);
}



#ifndef rm2_reenter_rx
void rm2_reenter_rx(ot_sig2 callback) {
/// Restart RX using the same settings that are presently in the radio core.
    static const ot_u8 rxstates[4] = {
        RADIO_STATE_RXPAGE, RADIO_STATE_RXAUTO, RADIO_FLAG_CRC5, 0
    };
    
    radio.evtdone   = callback;
    rfctl.state     = rxstates[(rfctl.flags & RADIO_FLAG_FLOOD)];
    rfctl.flags    |= rxstates[2 + (rfctl.flags & RADIO_FLAG_FLOOD)];
    rfctl.rxlimit   = 8;
    
    radio_gag();
    radio_idle();
    radio_flush_rx();
    subrfctl_unsync_isr();
}
#endif



#ifndef EXTF_rm2_resend
void rm2_resend(ot_sig2 callback) {
    radio.evtdone               = callback;
    radio.state                 = RADIO_Csma;
    rfctl.state                 = RADIO_STATE_TXINIT;
    txq.options.ubyte[UPPER]    = 255;
    //platform_enable_gptim2();
    radio_set_mactimer(0);
}
#endif


#ifndef EXTF_rm2_kill
void rm2_kill() {
    radio_gag();
    radio_sleep();
    subrfctl_finish(RM2_ERR_KILL, 0);
}
#endif



#ifndef EXTF_rm2_rxinit
OT_WEAK void rm2_rxinit(ot_u8 channel, ot_u8 psettings, ot_sig2 callback) {
    ot_u8 netstate;
    
    /// Setup the RX engine for Foreground Frame detection and RX.  Wipe-out
    /// the lower flags (non-persistent flags)
    radio.evtdone   = callback;
    rfctl.flags    &= ~(  RADIO_FLAG_RESIZE \
                        | RADIO_FLAG_FRCONT \
                        | RADIO_FLAG_FLOOD  \
                        | RADIO_FLAG_RESIZE );
    
    if (psettings != 0) {
        rfctl.flags|= RADIO_FLAG_FLOOD;
        netstate    = (M2_NETSTATE_UNASSOC | M2_NETFLAG_FIRSTRX);
    }
    else {
//#       if (M2_FEATURE(MULTIFRAME) == ENABLED)
//        //sets RADIO_FLAG_FRCONT
//        rfctl.flags |= ((session_netstate() & M2_NETSTATE_DSDIALOG) >> 1); 
//#       endif     
        netstate = session_netstate();
    }

    subrfctl_launch_rx(channel, netstate);
}
#endif



#ifndef EXTF_rm2_rxsync_isr
void rm2_rxsync_isr() {
/// Prepare driver for data reception, update high-level module state, and have
/// supervisor task (DLL) go into high-priority mode.
    q_empty(&rxq);
    radio.state = RADIO_DataRX;
    ///@todo when kernel is properly emulated: dll_block();
}
#endif



void subrfctl_unsync_isr() {
/// Undo what happens in rm2_rxsync_isr().  This is used when a packet is
/// discarded account of having a bad header.
    radio.state = RADIO_Listening;
    ///@todo when kernel is properly emulated: dll_unblock();
}



#ifndef EXTF_rm2_rxtimeout_isr
void rm2_rxtimeout_isr() {
    radio_gag();
    radio_sleep();
    subrfctl_finish(RM2_ERR_TIMEOUT, 0);
}
#endif




#ifndef EXTF_rm2_rxdata_isr
void rm2_rxdata_isr() {
    ///@todo speed-up this for background... Just check for RXAUTO, which is
    ///      only used by BG, and which needs no special cases 1 & 3
    
    /// 1. special handler for Manual RX-DONE, needed for Foreground packets
    if (rfctl.state == RADIO_STATE_RXDONE) {
        rm2_rxdata_isr_DONE:
        rm2_rxend_isr();
        return;
    }
    
    /// 2. load data
    rm2_rxdata_isr_DECODE:
    em2_decode_data();      // Contains logic to prevent over-run

    /// 3. Software-based FIFO resizing and CRC5 filtering
    if (rfctl.flags & RADIO_FLAG_CRC5) {
        rfctl.flags ^= RADIO_FLAG_CRC5;
        if (em2.crc5 != 0) {
            rm2_reenter_rx(radio.evtdone);
            return;
        }
    }
    
    /// 4. Handle main types of RX FIFO data
    switch ((rfctl.state >> RADIO_STATE_RXSHIFT) & (RADIO_STATE_RXMASK >> RADIO_STATE_RXSHIFT)) {
    
        /// RX State 0: Automatic Packet Control: used by BG packets
        case (RADIO_STATE_RXAUTO >> RADIO_STATE_RXSHIFT):
            break;

        /// RX State 1: Paging Mode
        /// Paging Mode is used for Foreground packet reception.  Multiframe packets
        /// use it for the final frame.
        case (RADIO_STATE_RXPAGE >> RADIO_STATE_RXSHIFT): {
            ot_int chipoctets_left;
        
            if (em2.bytes <= 0) {
                goto rm2_rxdata_isr_DONE;
            }
            
            chipoctets_left = (em2.bytes*_SPREAD);
            if (chipoctets_left  <= 96) {
                rfctl.rxlimit   = chipoctets_left;
                rfctl.state     = RADIO_STATE_RXDONE;
                break;
            }
            
            if (rfctl.rxlimit != _RXMAXTHR) {
                rfctl.rxlimit   = _RXMAXTHR;
            }
            break;
        }

        /// RX State 2 & 3:
        /// Multiframe packets (only compiled when MFPs are supported)
        ///@todo Experimental, not presently supported
#       if (M2_FEATURE(MULTIFRAME) == ENABLED)
#       error "Multiframe Not presently supported"
        case (RADIO_STATE_RXMFP >> RADIO_STATE_RXSLOT):
        case (RADIO_STATE_RXMFP >> RADIO_STATE_RXSHIFT): {
            ot_int  frames_left = em2_remaining_frames();
            if (em2.bytes == 0) {
                if (frames_left == 0) {
                    goto rm2_rxdata_isr_DONE;
                }
                radio.evtdone(frames_left, em2_decode_endframe());
                q_rebase(&rxq, rxq.putcursor);
                em2_decode_newframe();
                rfctl.flags |= RADIO_FLAG_CRC5;
                goto rm2_rxdata_isr_DECODE;
            }
            else if (frames_left == 0) {
            }
        } break;
#       endif
        
        /// Bug Trap
       default: rm2_kill();
                break;
    }
}
#endif




#ifndef EXTF_rm2_rxend_isr
void rm2_rxend_isr() {
    ot_u16 integrity;
    radio_gag();                                // No more interrupts!
    rfctl.state = RADIO_STATE_RXDONE;           // Make sure in DONE State, for decoding
    em2_decode_data();                          // decode any leftover data
    integrity = em2_decode_endframe();          // Finish RS Coding or CRC, get result (should be 0)
    radio_calc_link();                          // Calculate relative link info
    subrfctl_finish(0, integrity);
}
#endif



#ifndef EXTF_rm2_txinit
void rm2_txinit(ot_u8 psettings, ot_sig2 callback) {
    rfctl.flags    &= ~(    RADIO_FLAG_RESIZE   \
                          | RADIO_FLAG_FLOOD    \
                          | RADIO_FLAG_FRCONT   \
                          | RADIO_FLAG_CRC5     );
#   if (SYS_FLOOD == ENABLED)
    rfctl.flags    |= (psettings != 0);   //sets RADIO_FLAG_FLOOD
#   endif
#   if (M2_FEATURE(MULTIFRAME) == ENABLED)
    rfctl.flags    |= ((session_netstate() & M2_NETSTATE_DSDIALOG) >> 1); //sets RADIO_FLAG_FRCONT
#   endif
    radio.evtdone   = callback;
    radio.state     = RADIO_Csma;
    rfctl.state     = RADIO_STATE_TXINIT;
    
    // initialize the CRC/RS disabling byte
    txq.options.ubyte[UPPER] = 0;
    
    /// CSMA-CA interrupt based and fully pre-emptive.  This is
    /// possible using CC1 on the GPTIM to clock the intervals.
    //platform_enable_gptim2();
    radio_set_mactimer( (ot_uint)dll.comm.tca );
}
#endif





#ifndef EXTF_rm2_txstop_flood
void rm2_txstop_flood() {
/// Stop the MAC counter used to clock advertising flood synchronization.
/// Then simply configure TX driver state machine to go to TX Done state
/// as soon as the current packet is finished transmitting.
#if (SYS_FLOOD == ENABLED)
    rfctl.state = RADIO_STATE_TXDONE; 
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
            MODE_enum   type;
            ot_u16      pktlen;
            //ot_u8       timcfg[8] = {   0, RFREG(PROTOCOL0), DRF_PROTOCOL0, 
            //                            DRF_TIMERS5, 5, // RX Timer @ ~240us
            //                            33, 1,          // LDC First Interval (default 2 ticks)
            //                            0 };            // Alignment Dummy
            ot_u8   timcfg[8] = { 0, 0, 0, 0, 5, 33, 1, 0 };
            
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
                //spirit1_set_txpwr( &phymac[0].tx_eirp );
            }
            
            // Set other TX Buffering & Packet parameters, and also save the
            // Peristent-TX attribute for floods, which is written later
#           ifdef _DSSS
            if (rfctl.flags & RADIO_FLAG_FLOOD) {
                timcfg[2]      |= _PERS_TX;
                type            = MODE_bg;
                rfctl.txlimit   = (7*_SPREAD);
            }
            else {
                type            = MODE_fg;
                rfctl.txlimit   = (2*_SPREAD);
            }
#           else
            rfctl.txlimit   = 7;
            if (rfctl.flags & RADIO_FLAG_FLOOD) {
                timcfg[2]      |= 0;    //_PERS_TX;
                type            = MODE_bg;
            }
            else {
                type            = MODE_fg;
            }
#           endif
            subrfctl_buffer_config(type, em2.bytes /*q_span(&txq)*/);
            //spirit1_int_off();
            //spirit1_iocfg_tx();
            
            // If manual calibration is used, it is done here
            //__CALIBRATE();

            // No CSMA enabled, so jump to transmit
            if (1) {
            //if (dll.comm.csmaca_params & M2_CSMACA_NOCSMA) {
                //spirit1_spibus_io(3, 0, timcfg);
                goto rm2_txcsma_START;
            }
            
            // Setup CSMA/CCA parameters for this channel and fall through
            //spirit1_write(RFREG(RSSI_TH), (ot_u8)phymac[0].cca_thr );
            
            timcfg[6] = (phymac[0].tg - 1);
            //spirit1_spibus_io(7, 0, timcfg);
        }
        
        /// 2. Fall through from CSMA setup.  This code bypassed for No-CSMA
        ///    case.  This code directly accessed on repeat-CCA after fail.
        ///    Also setup to calibrate LDC RCO clock every X uses of CSMA.
        case (RADIO_STATE_TXCCA1 >> RADIO_STATE_TXSHIFT): 
        case (RADIO_STATE_TXCCA2 >> RADIO_STATE_TXSHIFT): {
            //ot_u8 protocol2;
            rfctl.state = RADIO_STATE_TXCCA1;
            //protocol2   = (DRF_PROTOCOL2 | _LDC_MODE);
            if (--rfctl.nextcal < 0) {
                rfctl.nextcal   = 100; ///@todo RF_PARAM(RCO_CAL_INTERVAL);
                //protocol2      |= _RCO_CALIBRATION;
            }
            //spirit1_write(RFREG(PROTOCOL2), protocol2);
            //spirit1_int_csma();
            sub_force_idle();
            //spirit1_strobe(STROBE(RX));
            break;
        }

        /// 3. TX startup: 'nuff said
        case (RADIO_STATE_TXSTART >> RADIO_STATE_TXSHIFT): {
        rm2_txcsma_START:
            // Send TX start (CSMA done) signal to DLL task
            // arg2: Non-zero for background, 0 for foreground
            radio.evtdone(0, (rfctl.flags & RADIO_FLAG_FLOOD));  
            
            // Preload into TX FIFO a small amount of data (up to 8 bytes)
            // This is small-enough that the TX state machine doesn't need
            // special conditions, and less initial data = less latency.
            txq.front[2]    = (phymac[0].tx_eirp & 0x7f);
            
            sub_force_idle(); 
            radio_flush_tx();
            em2_encode_data();
            
            // Prepare for TX, then enter TX
            // For floods, we must activate the flood counter right before TX
            radio.state     = RADIO_DataTX;
            rfctl.state     = RADIO_STATE_TXDATA;
            rfctl.txlimit   = RADIO_BUFFER_TXMAX;   // Change TXlimit to max buffer
            //spirit1_strobe( RFSTROBE_TX );
            //spirit1_int_txdata();
            
            if (rfctl.flags & RADIO_FLAG_FLOOD) {
                //spirit1_start_counter();
            }
            break;
        }
    }
}
#endif




void subrfctl_ccafail_isr() {
/// CCA has been pre-empted by a Sync-detect or a CS interrupt, which of course
/// means that the CCA scan has failed.
    //spirit1_int_off();
    //spirit1_write(RFREG(PROTOCOL2), 0);     //Turn-off LDC, RCO-Cal, VCO-Cal
    radio_sleep();
    
    rfctl.state = RADIO_STATE_TXCCA1;
    radio.evtdone(1, 0);
}


void subrfctl_ccapass_isr() {
/// CCA scan has passed. 
    rfctl.state += (1<<RADIO_STATE_TXSHIFT);
    
    // CSMA process is done
    if (rfctl.state == RADIO_STATE_TXSTART) {
        //spirit1_write(RFREG(PROTOCOL2), 0);     //Turn-off LDC, RCO-Cal, VCO-Cal
        rm2_txcsma_isr();
    }
    
    // CSMA process has another pass: reload LDC with guard time & exit
    else {
        //spirit1_write(RFREG(PROTOCOL2), _LDC_MODE);     //Turn-off RCO-Cal, VCO-Cal
        //spirit1_strobe(STROBE(LDC_RELOAD));
    }
}





#ifndef EXTF_rm2_txdata_isr
void rm2_txdata_isr() {
/// Continues where rm2_txcsma() leaves off.
   
#   if (SYS_FLOOD == ENABLED)
    /// Packet flooding.  Only needed on devices that can send M2AdvP
    /// The radio.evtdone callback here should update the AdvP payload
    if (rfctl.flags & RADIO_FLAG_FLOOD) {
        radio.evtdone(2, 0);
        
        if ((rfctl.state & RADIO_STATE_TXMASK) == RADIO_STATE_TXDATA) {
            crc_init_stream(True, 5, txq.getcursor);
            em2.bytes = 7;
            em2_encode_data();
        }
        return;
    }
#   endif
        
    /// Packet is not done, so fill buffer with more data and check.
    /// Disable FIFO interrupt and this state when the packet is fully-encoded
    rm2_txpkt_TXDATA:
    em2_encode_data();
    if (em2_remaining_bytes() == 0) {
        rfctl.state = RADIO_STATE_TXDONE;
        //spirit1_int_txdone();
    }
#   if (M2_FEATURE(MULTIFRAME) == ENABLED)
    else {
        /// If the frame is done, but more need to be sent (e.g. MFP's)
        /// queue it up.  The additional encode stage is there to fill up
        /// what's left of the buffer.
        if (em2_remaining_frames() != 0) {
            q_rebase(&txq, txq.getcursor);
            radio.evtdone(1, 0);        //callback action for next frame
            em2_encode_newframe();
            txq.front[2] = (phymac[0].tx_eirp & 0x7f);
            goto rm2_txpkt_TXDATA;
        }
    }
#   endif
}
#endif


void subrfctl_txend_isr() {
///@todo could put (rfctl.state != RADIO_STATE_TXDONE) as an argument, or 
///      something that resolves to an appropriate non-zero,as an arg in order 
///      to signal an error
    radio_gag();
    radio_idle();
    subrfctl_finish(0, 0);
}







/** Radio Subroutines
  * ============================================================================
  * - Usually some minor adjustments needed when porting to new platform
  * - See integrated notes for areas sensitive to porting
  */

void subrfctl_null(ot_int arg1, ot_int arg2) { }



void subrfctl_finish(ot_int main_err, ot_int frame_err) {
///@todo globalize
/// Reset radio & callback to null state, then run saved callback
    ot_sig2 callback;
    radio_gag();                            //redundant,
    radio.state     = RADIO_Idle;
    rfctl.state     = 0;
    rfctl.flags    &= RADIO_FLAG_SETPWR;    //clear all other flags
    callback        = radio.evtdone;
    radio.evtdone   = &otutils_sig2_null;
    callback(main_err, frame_err);
}


ot_bool subrfctl_chanscan( ) {
///@todo globalize
    vlFILE* fp;
    ot_int  i;

    fp = ISF_open_su( ISF_ID(channel_configuration) );
    ///@todo assert fp

    /// Go through the list of tx channels
    /// <LI> Make sure the channel ID is valid. </LI>
    /// <LI> Make sure the transmission can fit within the contention period. </LI>
    /// <LI> Scan it, to make sure it can be used. </LI>
    for (i=0; i<dll.comm.tx_channels; i++) {
        ot_u8 next_channel = dll.comm.tx_chanlist[i];
        if (subrfctl_channel_fastcheck(next_channel))   break;
        if (subrfctl_channel_lookup(next_channel, fp))  break;
    }

    vl_close(fp);
    return (ot_bool)(i < dll.comm.tx_channels);
}


ot_bool subrfctl_channel_fastcheck(ot_u8 chan_id) {
///@todo globalize
    ot_u8 old_chan_id;
    old_chan_id = phymac[0].channel & 0x7F;
    chan_id    &= 0x7F;

    return (ot_bool)((chan_id == 0x7F) || (chan_id == old_chan_id));
}


ot_bool subrfctl_channel_lookup(ot_u8 chan_id, vlFILE* fp) {
/// Called during channel scans.
/// Duty: (a) See if the supplied channel is supported on this device & config.
///       If yes, return true.  (b) Determine if recalibration is required
///       before changing to the new channel, and recalibrate if so.

    //ot_u8       fec_id;
    ot_u8       spectrum_id;
    ot_int      i;
    Twobytes    scratch;

    // Only do the channel lookup if the new channel is different than before
    // (done now in subrfctl_channel_fastcheck())
    //if (chan_id == phymac[0].channel&0x7F) {
    //    return True;
    //}

    // pull spectrum id and encoding type out of chan_id
    // (SPIRIT1 driver does this already in input to chan_id)
    //fec_id      = chan_id & 0x80;
    //spectrum_id = chan_id & ~0x80;

    // FEC selection is done in subrfctl_buffer_config() for
    // each packet in SPIRIT1 driver

    // 0x7F is the wildcard spectrum id.  It means use same spectrum as before.
    // In this case, of course no recalibration is necessary.
    // (done now in subrfctl_channel_fastcheck())
    //if (spectrum_id == 0x7F) {
    //    return True;
    //}

    spectrum_id = chan_id & 0x7F;

    /// Look through the channel list to find the one with matching spectrum id.
    /// The channel list is not necessarily sorted.
    /// @todo might do this with vl_get() in the future
    for (i=6; i<fp->length; i+=6) {
        scratch.ushort = vl_read(fp, i);

        if ((spectrum_id == scratch.ubyte[0]) \
        || ((spectrum_id | 0x0F) == scratch.ubyte[0])) {
            ot_u8 old_chan_id   = phymac[0].channel;
            ot_u8 old_tx_eirp   = (phymac[0].tx_eirp & 0x7f);

            phymac[0].tg        = rm2_default_tgd(chan_id);
            phymac[0].channel   = chan_id;
          //phymac[0].autoscale = scratch.ubyte[1];

            scratch.ushort      = vl_read(fp, i+2);
            phymac[0].tx_eirp   = scratch.ubyte[0];
            phymac[0].link_qual = scratch.ubyte[1];

            ///@todo Try this: *(ot_u16*)&phymac[0].cs_thr = vl_read(fp, i+4);  
            ///it will need some rearrangement in phymac struct
            scratch.ushort      = vl_read(fp, i+4);
            phymac[0].cs_thr    = scratch.ubyte[0];
            phymac[0].cca_thr   = scratch.ubyte[1];

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
    //static const ot_u8 drate_matrix[14] = { 
    //    0, RFREG(MOD1), DRF_MOD1_LS, DRF_MOD0_LS, DRF_FDEV0, DRF_CHFLT_LS, 0, 0, 
    //    0, RFREG(MOD1), DRF_MOD1_HS, DRF_MOD0_HS, DRF_FDEV0, DRF_CHFLT_HS
    //};

    ot_u8 fc_i;

    /// Flag PA table reprogram (done before TX): only flag if power is different
    if (old_eirp != phymac[0].tx_eirp) {
        rfctl.flags |= RADIO_FLAG_SETPWR;
    }

    /// Configure data rate: only change registers if required
    if ( (old_chan ^ phymac[0].channel) & 0x70 ) {
        //spirit1_spibus_io( 6, 0, (ot_u8*)&drate_matrix[(phymac[0].channel & 0x20) >> 2] );
    }

    /// Configure Channel: only change registers if required
    fc_i = (phymac[0].channel & 0x0F);
    if ( fc_i != (old_chan & 0x0F) ) {
        //spirit1_write(RFREG(CHNUM), fc_i);
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
    
#if (DRF_SYNC_BYTES == 2)
    // 2 byte sync using: E6D0 / 0B67 / F498 / 192F
    static const ot_u8 sync_matrix[] = { 0xD0, 0xE6,    //bg non-fec
                                         0x67, 0x0B,    //fg non-fec
                                         0x98, 0xF4,    //bg fec
                                         0x2F, 0x19,    //fg fec
                                         };
#elif (DRF_SYNC_BYTES == 3)
    // 3 byte sync using:  8C497E / A5330F / F85299 / 83D699
    static const ot_u8 sync_matrix[] = { 0, 0x7E, 0x49, 0x8C,    //bg non-fec
                                         0, 0x0F, 0x33, 0xA5,    //fg non-fec
                                         0, 0x99, 0x52, 0xF8,    //bg fec
                                         0, 0x99, 0xD6, 0x83     //fg fec
                                         };
#else
    // 4 byte sync using ... 92DDC8F1 / A444B78F / EE415A3C / AD79C06C
    static const ot_u8 sync_matrix[] = { 0xF1, 0xC8, 0xDD, 0x92,     //bg non-fec
                                         0x8F, 0xB7, 0x44, 0xA4,     //fg non-fec
                                         0x3C, 0x5A, 0x41, 0xEE,     //bg fec
                                         0x6C, 0xC0, 0x79, 0xAD      //fg fec
                                         };
#endif
                                         
    ot_u8 buf_cfg[10];
    ot_u8 is_fec;
    //ot_u8 is_hs;
    
    buf_cfg[0]  = 0;
    buf_cfg[1]  = 0;    //RFREG(PCKTCTRL2);
    buf_cfg[2]  = 0;    //DRF_PCKTCTRL2_LSBG;
                             
#   if (DRF_SYNC_BYTES == 2)                                         
    is_fec      = (phymac[0].channel & 0x80) >> 5;      // 0 when no fec, 4 when FEC                              
    mode       += is_fec;                              
    buf_cfg[3]  = (is_fec >> 2);
    buf_cfg[4]  = ((ot_u8*)&param)[UPPER];
    buf_cfg[5]  = ((ot_u8*)&param)[LOWER];
    buf_cfg[6]  = 0;
    buf_cfg[7]  = 0;
    buf_cfg[8]  = sync_matrix[mode];
    buf_cfg[9]  = sync_matrix[mode+1];
    
#   elif (DRF_SYNC_BYTES == 3) || (DRF_SYNC_BYTES == 4)
    is_fec      = (phymac[0].channel & 0x80) >> 4;      // 0 when no fec, 8 when FEC
    mode       += is_fec;
    buf_cfg[3]  = (is_fec >> 3);                    
    buf_cfg[4]  = ((ot_u8*)&param)[UPPER];
    buf_cfg[5]  = ((ot_u8*)&param)[LOWER];
    buf_cfg[6]  = sync_matrix[mode];
    buf_cfg[7]  = sync_matrix[mode+1];
    buf_cfg[8]  = sync_matrix[mode+2];
    buf_cfg[9]  = sync_matrix[mode+3];
#   endif
    
    //spirit1_spibus_io(10, 0, buf_cfg);
}




void subrfctl_prep_q(ot_queue* q) {
/// Put some special data in the queue options field.
/// Lower byte is encoding options (i.e. FEC)
/// Upper byte is processing options (i.e. CRC)
    q->options.ubyte[UPPER]    += 1;
    q->options.ubyte[LOWER]     = 0;
    //q->options.ubyte[LOWER]     = (phymac[0].channel & 0x80);
}



#endif

