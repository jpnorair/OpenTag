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
  * @file       /otradio/spirit1/radio_SPIRIT1.c
  * @author     JP Norair
  * @version    R103
  * @date       23 Jan 2013
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

#include "OT_platform.h"
#if (OT_FEATURE(M2) == ENABLED)

#include "OT_types.h"
#include "OT_config.h"
#include "OT_utils.h"

#include "radio.h"
#include "radio_SPIRIT1.h"
#include "SPIRIT1_interface.h"

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




//#define RFCORE_DEBUG
//ot_u8 dump[256];

#ifdef RFCORE_DEBUG
#   define __CORE_DUMP()   spirit1_coredump()
#else
#   define __CORE_DUMP();
#endif





/** Local Data declaration
  * Described in radio_SPIRIT1.h
  */
rfctl_struct rfctl;




/** Local Subroutine Prototypes  <BR>
  * ========================================================================<BR>
  * MODE_fg = 2 or 4, depending on length of syncword.  
  * As you might guess, this is an alignment hack.
  */
typedef enum {
    MODE_bg     = 0,
    MODE_fg     = (2 << (DRF_SYNC_BYTES > 2))
} MODE_enum;


void    subrfctl_force_ready();
void    subrfctl_smart_ready();
void    subrfctl_smart_standby();

ot_bool subrfctl_test_channel(ot_u8 channel);
void    subrfctl_launch_rx(ot_u8 channel, ot_u8 netstate);

//void    subrfctl_kill(ot_int errcode);
void    subrfctl_finish(ot_int main_err, ot_int frame_err);
//ot_bool subrfctl_lowrssi_reenter();

ot_bool subrfctl_chanscan();
void    subrfctl_unsync_isr();
//void    subrfctl_ccascan();
//void    subrfctl_ccascan_isr();
void    subrfctl_ccafail_isr();
void    subrfctl_ccapass_isr();
void    subrfctl_txend_isr();

ot_bool subrfctl_channel_fastcheck(ot_u8 chan_id);
ot_bool subrfctl_channel_lookup(ot_u8 chan_id, vlFILE* fp);
void    subrfctl_chan_config(ot_u8 old_chan, ot_u8 old_eirp);
void    subrfctl_buffer_config(MODE_enum mode, ot_u16 param);
void    subrfctl_save_linkinfo();

void    subrfctl_prep_q(ot_queue* q);
ot_bool subrfctl_mac_filter();


#if (RF_FEATURE(AUTOCAL) != ENABLED)
#   define __CALIBRATE()    subrfctl_offline_calibration()
#else
#   define __CALIBRATE();
#endif






/** SPIRIT1 Virtual ISR RF  <BR>
  * ========================================================================<BR>
  */
void spirit1_virtual_isr(ot_u8 code) {
    switch (code) {
        case RFIV_RXTERM:   rm2_kill();             break;
        case RFIV_RXSYNC:   rm2_rxsync_isr();       break;
        
        case RFIV_RXEND:    rm2_rxend_isr();        break;
        case 3:             //subrfctl_unsync_isr();  break;  //Falling Edge
        case RFIV_RXFIFO:   rm2_rxdata_isr();       break;
        
        case RFIV_CCAPASS:  subrfctl_ccapass_isr(); break;
        case RFIV_CCAFAIL:  subrfctl_ccafail_isr(); break;
        
        case RFIV_TXEND:    
        case 8:             subrfctl_txend_isr();   break;
        case RFIV_TXFIFO:   rm2_txdata_isr();       break;
    }
}
  
  



void radio_mac_isr() {
    /// Used as CA insertion timer
    //if (radio.state == RADIO_Csma) {
        platform_disable_gptim2();
        rm2_txcsma_isr();
    //}
}
    






/** Radio Core Control Functions
  * ============================================================================
  * - Need to be customized per radio platform
  */


void subrfctl_force_ready() {
/// Goes to READY without modifying states.  Use with caution.
    if ((rfctl.flags & RADIO_FLAG_XOON) == 0)  {
        rfctl.flags |= RADIO_FLAG_XOON;
        spirit1_strobe(RFSTROBE_READY);
        spirit1_waitforready();
    }
}

void subrfctl_smart_ready() {
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
    //else subrfctl_force_ready();
    subrfctl_force_ready();
}


void subrfctl_smart_standby() {
/// Sleep on SPIRIT1 is actually STANDBY mode.  There is also a SLEEP mode on 
/// SPIRIT1 that is STANDBY+RC_Osc, but this implementation does not use the RC
/// Oscillator at all, so STANDBY is the best choice.
#if BOARD_FEATURE_RFXTALOUT
    if (spirit1.clkreq) {
        subrfctl_smart_ready();
    } 
    else
#endif
    // Only go to STANDBY if it is not already the present mode.
    if (rfctl.flags & RADIO_FLAG_XOON) {
        subrfctl_smart_ready();
        rfctl.flags &= ~RADIO_FLAG_PWRMASK;
        spirit1_strobe(RFSTROBE_STANDBY);
        spirit1_waitforstandby();           // New line
    }   
}


  
#ifndef EXTF_radio_off
OT_WEAK void radio_off() {
   spirit1_shutdown();
}
#endif

#ifndef EXTF_radio_gag
OT_WEAK void radio_gag() {
    spirit1_int_off();
}
#endif


#ifndef EXTF_radio_ungag
OT_WEAK void radio_ungag() {
    if (radio.state != RADIO_Idle) {
        spirit1_int_on();
    }
}
#endif



#ifndef EXTF_radio_sleep
OT_WEAK void radio_sleep() {
    subrfctl_smart_standby();
    
    //radio_idle();
    //rfctl.flags &= ~(RADIO_FLAG_XOON | RADIO_FLAG_PWRMASK);
    //spirit1_strobe(RFSTROBE_STANDBY);
    //spirit1_waitforstandby();
}
#endif


#ifndef EXTF_radio_idle
OT_WEAK void radio_idle() {
    subrfctl_smart_ready();
    
    //radio.state = RADIO_Idle;
    //rfctl.flags |= RADIO_FLAG_XOON;
    //spirit1_strobe(RFSTROBE_SABORT);
    //spirit1_strobe(RFSTROBE_READY);
    //spirit1_waitforready();
}
#endif


void spirit1_clockout_on(ot_u8 clk_param) {
/// Set the SPIRIT1 to idle, then configure the driver so it never goes into sleep 
/// or standby, and finally configure the SPIRIT1 to output the clock.
#if (BOARD_FEATURE_RFXTALOUT)
    subrfctl_smart_ready();
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
    subrfctl_smart_standby();
#endif
}


#if (RF_FEATURE(AUTOCAL) != ENABLED)
void subrfctl_offline_calibration() {
/// Make sure to only call this function when there is nothing going on.
    static const ot_u8 fc_div[6] = { RFREG(SYNT3), 0, 
        DRF_SYNT3X, DRF_SYNT2X, DRF_SYNT1X, DRF_SYNT0X };
    static const ot_u8 fc[6] = { RFREG(SYNT3), 0, 
        DRF_SYNT3, DRF_SYNT2, DRF_SYNT1, DRF_SYNT0 };
        
    ot_u8   vco_cal[4] = { RFREG(RCO_VCO_CALIBR_IN1), 0, 0, 0 };
    
    // Step 0: Check if calibration is needed
    if (--rfctl.nextcal >= 0) {
        return;
    }
    rfctl.nextcal = RF_PARAM(VCO_CAL_INTERVAL);

    // Step 1 (from erratum workaround): Set SEL_TSPLIT to 3.47 ns.
    // This is performed during startup initialization
    
    subrfctl_smart_ready();
    
    // Step 2: for 48, 50, 52 MHz crystals impls, enable _REFDIV and 
    //         change the center frequency to match.
#   if (BOARD_PARAM_RFHz > 26000000)
    spirit1_write( RFREG(SYNTH_CONFIG1), (DRF_SYNTH_CONFIG1 | _REFDIV) );
    spirit1_spibus_io(6, 0, (ot_u8*)fc_div);
#   endif
    
    // Step 3: Boost VCO current to 25 (from default)
    spirit1_write( RFREG(VCO_CONFIG), __VCO_GEN_CURR(25) );

    // Step 4: Enable Automatic Calibration (for now)
    spirit1_write( RFREG(PROTOCOL2), (DRF_PROTOCOL2|_VCO_CALIBRATION) );

    // Step 5: Calibrate TX & RX VCO values
    spirit1_strobe( STROBE(LOCKTX) );
    platform_swdelay_us(200);
    vco_cal[2] = spirit1_read( RFREG(RCO_VCO_CALIBR_OUT0) ) & 0x7f;
    spirit1_strobe(RFSTROBE_READY);
    spirit1_strobe( STROBE(LOCKRX));
    platform_swdelay_us(200);
    vco_cal[3] = spirit1_read( RFREG(RCO_VCO_CALIBR_OUT0) ) & 0x7f;
    spirit1_strobe(RFSTROBE_READY);
    
    // Step 6: write vco calibration output into SPIRIT1 input
    spirit1_spibus_io(4, 0, vco_cal);
    
    // Step 7: Disable automatic calibration
    spirit1_write( RFREG(PROTOCOL2), (DRF_PROTOCOL2) );
    
    // Step 8: Reduce VCO current to standard level
    spirit1_write( RFREG(VCO_CONFIG), __VCO_GEN_CURR(17) );
    
    // Step 9: Revert step 2
#   if (BOARD_PARAM_RFHz > 26000000)
    spirit1_write( RFREG(SYNTH_CONFIG1), DRF_SYNTH_CONFIG1 );
    spirit1_spibus_io(6, 0, (ot_u8*)fc);
#   endif
}
#endif


#ifndef EXTF_radio_calibrate
OT_WEAK void radio_calibrate() {
/// SPIRIT1 has an errata with the automatic calibrator.  It is best practice to 
/// perform the calibrations manually and offline.
#if (RF_FEATURE(AUTOCAL) != ENABLED)
    
    rfctl.nextcal = 0;
#endif
}
#endif


#ifndef EXTF_radio_set_mactimer
OT_WEAK void radio_set_mactimer(ot_u16 clocks) {
/// Used for high-accuracy TX/CSMA slot insertion, and flooding.
    platform_set_gptim2(clocks);
}
#endif


#ifndef EXTF_radio_get_countdown
OT_WEAK ot_u16 radio_get_countdown() {
    return spirit1_get_counter();
}
#endif







/** Radio Module Control Functions
  * ============================================================================
  * - Need to be customized per radio platform
  */
#ifndef EXTF_radio_init
OT_WEAK void radio_init( ) {
/// Transceiver implementation dependent    
    vlFILE* fp;
    
    /// Set SPIRIT1-dependent initialization defaults
    rfctl.flags     = RADIO_FLAG_XOON;
    rfctl.nextcal   = 0;
    
    /// Set universal Radio module initialization defaults
    radio.state     = RADIO_Idle;
    radio.evtdone   = &otutils_sig2_null;
    
    /// These Radio Link features are available on the SPIRIT1
#   if (OT_FEATURE(RF_LINKINFO))
#       if (M2_FEATURE(RSCODE))
#           define _CORRECTIONS RADIO_LINK_CORRECTIONS
#       else
#           define _CORRECTIONS 0
#       endif
    radio.link.flags= _CORRECTIONS \
                    | RADIO_LINK_PQI \
                    | RADIO_LINK_SQI \
                    | RADIO_LINK_LQI \
                    | RADIO_LINK_AGC;
#   endif
    
    /// Initialize the bus between SPIRIT1 and MCU, and load defaults.
    /// SPIRIT1 starts-up in Idle (READY), so we set the state and flags
    /// to match that.  Then, init the bus and send RADIO to sleep.
    /// SPIRIT1 can do SPI in Sleep.
    spirit1_init_bus();
    spirit1_load_defaults();
    
    /// Do this workaround (SPIRIT1 Errata DocID023165 R5, section 1.2) to fix
    /// the shutdown current issue for input voltages <= 2.6V.  For input
    /// voltages > 2.6V, it does not hurt anything.
    //spirit1_write(RFREG(PM_TEST), 0xCA);
    //spirit1_write(RFREG(TEST_SELECT), 0x04);
    //spirit1_write(RFREG(TEST_SELECT), 0x00);
    
    /// Done with the radio init
    subrfctl_smart_standby();
    
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



#ifndef EXTF_radio_mac_configure
OT_WEAK void radio_mac_configure() {
/// Only use this when there is a hardware MAC filtering ability.  The SPIRIT1
/// does technically have ability to filter on subnet, but it is not used.  
/// Subnet is the 4th byte in a foreground frame or 2nd byte in a background
/// frame.
    //spirit1_write(RFREG(PCKT_FLT_GOALS11), dll.netconf.subnet);
    //spirit1_write(RFREG(PCKT_FLT_GOALS7), (0xF0 | dll.netconf.subnet));
}
#endif


#ifndef EXTF_radio_mac_filter
OT_WEAK ot_bool radio_mac_filter() {
/// SPIRIT1 can hypothetically do this in HW or even at the point of header RX, 
/// but if RS-coding is being used, then errors might be corrected in these
/// pieces of data.
    return subrfctl_mac_filter();
}
#endif


#ifndef EXTF_radio_check_cca
OT_WEAK ot_bool radio_check_cca() {
/// CCA Method: Look if CS pin is high.
    return spirit1_check_cspin();
}
#endif


#ifndef EXTF_radio_rssi
OT_WEAK ot_int radio_rssi() {
/// @note SPIRIT1 only guarantees RSSI reading after end of packet RX
    return radio.last_rssi;
    //radio.last_rssi = spirit1_calc_rssi( spirit1_read(RFREG(RSSI_LEVEL)) );
}
#endif


#ifndef EXTF_radio_calc_link
OT_WEAK void radio_calc_link() {
    radio.last_rssi     = spirit1_calc_rssi( spirit1_read(RFREG(RSSI_LEVEL)) );
    radio.last_linkloss = (ot_int)(rxq.front[2] & 0x7F) - 80 - RF_HDB_RXATTEN;
    radio.last_linkloss-= radio.last_rssi;
    
    // Save additional link parameters.  If OT_FEATURE(RF_LINKINFO) is not
    // enabled in the app config, nothing will happen in this function.
    subrfctl_save_linkinfo();
}
#endif


#ifndef EXTF_radio_buffer
OT_WEAK ot_u8 radio_buffer(ot_int index) {
/// Transceiver implementation dependent
/// This function is not used on the SPIRIT1
    return 0;
}
#endif


#ifndef EXTF_radio_putbyte
OT_WEAK void radio_putbyte(ot_u8 databyte) {
/// Transceiver implementation dependent
    spirit1_write(RFREG(FIFO), databyte);
}
#endif


#ifndef EXTF_radio_putfourbytes
OT_WEAK void radio_putfourbytes(ot_u8* data) {
/// Unused in SPIRIT1, which has HW FECTX.
}
#endif


#ifndef EXTF_radio_getbyte
OT_WEAK ot_u8 radio_getbyte() {
/// Transceiver implementation dependent
    return spirit1_read(RFREG(FIFO));
}
#endif



#ifndef EXTF_radio_getfourbytes
OT_WEAK void radio_getfourbytes(ot_u8* data) {
/// Unused for SPIRIT1, which has HW FECRX
}
#endif



///@note this IRQ flushing is needed if you are using a driver interrupt based 
///      on the SPIRIT1 IRQ system.  Presently, all driver interrupts are using
///      direct status conditions, not this IRQ.
//static const ot_u8 read_irq[] = { 1, 0xFA };

#ifndef EXTF_radio_flush_rx
OT_WEAK void radio_flush_rx() {
    //spirit1_spibus_io(2, 4, (ot_u8*)read_irq);
    spirit1_strobe( RFSTROBE_FLUSHRXFIFO );
}
#endif

#ifndef EXTF_radio_flush_tx
OT_WEAK void radio_flush_tx() {
    //spirit1_spibus_io(2, 4, (ot_u8*)read_irq);
    spirit1_strobe( RFSTROBE_FLUSHTXFIFO );
}
#endif



#ifndef EXTF_radio_rxopen
OT_WEAK ot_bool radio_rxopen() {
    return (ot_bool)spirit1_rxbytes();
}
#endif



#ifndef EXTF_radio_rxopen_4
OT_WEAK ot_bool radio_rxopen_4() {
    return (ot_bool)(spirit1_rxbytes() >= 4);
}
#endif


#ifndef EXTF_radio_txopen
OT_WEAK ot_bool radio_txopen() {
    return (ot_bool)(spirit1_txbytes() < rfctl.txlimit);
}
#endif


#ifndef EXTF_radio_txopen_4
OT_WEAK ot_bool radio_txopen_4() {
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
    /// 1. Prepare the CRC and RS encoding, which need to be computed
    ///    when the upper options byte is set.  That is, it is non-zero
    ///    on the first packet and 0 for retransmissions.
    if (txq.options.ubyte[UPPER] != 0) {
        crc_init_stream(True, q_span(&txq), txq.getcursor);
        txq.putcursor  += 2;
        txq.front[0]   += 2;
        txq.front[1]   &= ~0x20;            // always clear this bit
        
#       if (M2_FEATURE(RSCODE))
            em2.lctl = txq.front[1];
            if (em2.lctl & 0x40) {
                ot_int parity_bytes;
                parity_bytes    = em2_rs_init_encode(&txq);
                txq.front[0]   += parity_bytes;
                txq.putcursor  += parity_bytes;
            }
#       else
            em2.lctl        = txq.front[1] & ~0x60;
            txq.front[1]    = em2.lctl;
#       endif
        
        // Only appoint CRC5 when the first ten bits of txq are settled
        em2_add_crc5(txq.front);
	}

    /// 2. set initial values of encoder control variables
    em2.bytes = q_span(&txq);
}
#endif

#ifdef EXTF_em2_decode_newframe
void em2_decode_newframe() {
    em2.state   = 0;
    em2.bytes   = 8;      // dummy length until actual length is received
}
#endif



#ifndef _DSSS
void em2_encode_data() {
    ot_int  fill;
    ot_int  load;
    ot_u8   save[2];
    ot_u8*  cmd;
    
    // Loop unrolling for FIFO loading
    load = (rfctl.txlimit - spirit1_txbytes());
    
    while (1) {
        fill = (load < em2.bytes) ? load : em2.bytes;
        if (fill <= 0) break;
        
        if (fill > 24) fill = 24;
        load       -= fill;
        em2.bytes  -= fill;
        
        if (txq.options.ubyte[UPPER] != 0) {
            crc_calc_nstream(fill);
#           if (M2_FEATURE(RSCODE))
            if (em2.lctl & 0x40) {
                em2_rs_encode(fill);
            }
#           endif
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
    //spirit1_read(RFREG(IRQ_STATUS0));
    *(ot_u16*)save  = PLATFORM_ENDIAN16_C(0x8000);
    spirit1_spibus_io(2, 0, save);
}


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
            ot_int ext_bytes;
            em2.state--;
            em2.bytes       = 1 + (ot_int)rxq.front[0];
            rxq.front[1]   &= ~0x20;                    // always clear this bit
            em2.lctl        = rxq.front[1];
            em2.crc5        = em2_check_crc5();
            if (em2.crc5 != 0) {
                return;
            }

            ext_bytes = 0;
            if (em2.lctl & 0x40) {
                ext_bytes = em2_rs_init_decode(&rxq);
            }
            crc_init_stream(False, em2.bytes-ext_bytes, rxq.getcursor);
        }
        
        crc_calc_nstream(grab);
        
        ///@todo we can optimize this also by waiting until crc is done,
        ///      and then verifying that it is not accurate.  but we need
        ///      better speed profiling before doing that.
#       if (M2_FEATURE(RSCODE))
        if (em2.lctl & 0x40) {
            em2_rs_decode(grab);
        }
#       endif

        em2.bytes -= grab;
        if (em2.bytes > 0) {
            goto em2_decode_data_TOP;
        }
    }
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



///@note maccfg[5] should be 1 for release, and higher (i.e. 10) for bgrx test
void subrfctl_launch_rx(ot_u8 channel, ot_u8 netstate) {
    ot_u8 maccfg[8] = { 0, RFREG(PROTOCOL2), 
                        (DRF_PROTOCOL2 & 0x1F) | _SQI_TIMEOUT_MASK,
                        DRF_PROTOCOL1, 
                        DRF_PROTOCOL0,
                        DRF_TIMERS5, 0,     // RX Termination timer (default off)
                        0 };                // Alignment Dummy
    MODE_enum   buffer_mode;
    ot_u16      pktlen;
    
    /// 1.  Prepare RX queue by flushing it
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

    /// 4a. Setup RX for Background detection (if FLOOD):
    ///     <LI> Manipulate Queue to fit bg frame into common model </LI>
    ///     <LI> Set SPIRIT1 RX timer to a small amount.  The minimum amount
    ///          depends on ramp-up/down settings, but ~400us is a safe bet </LI>
    ///     <LI> Set SPIRIT1 to pause RX timer on carrier sense </LI>
    /// 4b. Setup RX for Foreground detection (ELSE): 
    ///     <LI> Set Foreground paging and tentative packet-len to max </LI>
    if (rfctl.flags & RADIO_FLAG_FLOOD) {
        spirit1_write(RFREG(RSSI_TH), (ot_u8)phymac[0].cs_thr);
        rxq.getcursor       = rxq.front;
        *rxq.getcursor++    = 8;
        *rxq.getcursor++    = 2;            // 00010 is CRC5 for 0000100000
        rxq.putcursor       = rxq.getcursor;
        maccfg[2]           = (DRF_PROTOCOL2 & 0x1F) | _SQI_TIMEOUT_MASK | _CS_TIMEOUT_MASK;
        maccfg[6]           = 8;           // 1 unit = ~48us
        buffer_mode         = MODE_bg;
        pktlen              = (7*_SPREAD);
    }
    else {
        //spirit1_write(RFREG(PCKTCTRL4), DRF_PCKTCTRL4+3);
        buffer_mode = MODE_fg;
        pktlen      = (_MAXPKTLEN*_SPREAD);
    }

    /// 5.  Send Configuration data to SPIRIT1
    subrfctl_buffer_config(buffer_mode, pktlen);                // packet configuration
    spirit1_spibus_io(7, 0, maccfg);                            // MAC configuration

    /// 6.  Prepare Decoder to receive, then receive
    em2_decode_newpacket();
    em2_decode_newframe();
    //dll_offset_rxtimeout();   ///@todo why is this commented-out?
    
    /// 7. If manual calibration is used, sometimes it is done here
    __CALIBRATE();
    
    /// 8.  Using rm2_reenter_rx()
    spirit1_iocfg_rx();
    rm2_reenter_rx(radio.evtdone);
}










#ifndef rm2_reenter_rx
OT_WEAK void rm2_reenter_rx(ot_sig2 callback) {
/// Restart RX using the same settings that are presently in the radio core.
///@todo RX internal state configuration might need attention
    static const ot_u8 rxstates[4] = {
        RADIO_STATE_RXPAGE, RADIO_STATE_RXAUTO, RADIO_FLAG_CRC5, 0
    };
    
    radio.evtdone   = callback;
    rfctl.state     = rxstates[(rfctl.flags & RADIO_FLAG_FLOOD)];
    rfctl.flags    |= rxstates[2 + (rfctl.flags & RADIO_FLAG_FLOOD)];
    rfctl.rxlimit   = (96-_RXMINTHR);
    spirit1_write(RFREG(FIFO_CONFIG3), (ot_u8)rfctl.rxlimit );
    
    radio_gag();                    // This shouldn't be necessary, but there's a bug in the rxend function.
    subrfctl_smart_ready();
    radio_flush_rx();
    spirit1_strobe( RFSTROBE_RX );
    subrfctl_unsync_isr();
}
#endif




#ifndef EXTF_rm2_resend
OT_WEAK void rm2_resend(ot_sig2 callback) {
    radio.evtdone               = callback;
    radio.state                 = RADIO_Csma;
    rfctl.state                 = RADIO_STATE_TXINIT;
    txq.options.ubyte[UPPER]    = 255;
    platform_enable_gptim2();
    radio_set_mactimer(0);
}
#endif


ot_u32 total = 0;


ot_u32 bg_scans = 0;
ot_u32 bg_false_positives  = 0;

#ifndef EXTF_rm2_kill
OT_WEAK void rm2_kill() {
    radio_gag();
    subrfctl_smart_standby();
    
    ///@note this is only for lab testing of channel threshold
    //radio.last_rssi = spirit1_calc_rssi( spirit1_read(RFREG(RSSI_LEVEL)) );
    //total += (0-radio.last_rssi);
    //bg_scans++;
    
    subrfctl_finish(RM2_ERR_KILL, 0);
}
#endif



#ifndef EXTF_rm2_rxtimeout_isr
OT_WEAK void rm2_rxtimeout_isr() {
    radio_gag();
    subrfctl_smart_standby();
    
    ///@note this is only for lab testing of channel threshold
    //radio.last_rssi = spirit1_calc_rssi( spirit1_read(RFREG(RSSI_LEVEL)) );
    //total += (0-radio.last_rssi);
    //bg_scans++;
    //bg_false_positives++;
    
    subrfctl_finish(RM2_ERR_TIMEOUT, 0);
}
#endif




#ifndef EXTF_rm2_rxinit
OT_WEAK void rm2_rxinit(ot_u8 channel, ot_u8 psettings, ot_sig2 callback) {
#if (SYS_RECEIVE == ENABLED)
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
#       if (M2_FEATURE(MULTIFRAME) == ENABLED)
        //sets RADIO_FLAG_FRCONT
        rfctl.flags |= ((session_netstate() & M2_NETSTATE_DSDIALOG) >> 1); 
#       endif        
        netstate = session_netstate();
    }

    subrfctl_launch_rx(channel, netstate);
#else
    // BLINKER only (no RX)
    callback(RM2_ERR_GENERIC, 0);
#endif
}
#endif



#ifndef EXTF_rm2_rxsync_isr
OT_WEAK void rm2_rxsync_isr() {
/// Prepare driver for data reception, update high-level module state, and have
/// supervisor task (DLL) go into high-priority mode.
    q_empty(&rxq);                  ///@note this is new fix!!!
    spirit1_int_rxdata();
    radio.state = RADIO_DataRX;
    dll_block();
}
#endif



void subrfctl_unsync_isr() {
/// Undo what happens in rm2_rxsync_isr().  This is used when a packet is discarded
/// by SPIRIT1 on account of having a bad header.
    spirit1_int_listen();
    radio.state = RADIO_Listening;
    dll_unblock();
}




#ifndef EXTF_rm2_rxdata_isr
OT_WEAK void rm2_rxdata_isr() {
#if (SYS_RECEIVE == ENABLED)
/// I am aware that this ISR includes a lot of "spaghetti code."  This allows it
/// to run fast, which is more important in this I/O ISR than it is for you to
/// understand it easily.  Deal with it.

    ///@todo speed-up this for background... Just check for RXAUTO, which is
    ///      only used by BG, and which needs no special cases 1 & 3
    
    /// 1. special handler for Manual RX-DONE, needed for Foreground packets
    if (rfctl.state == RADIO_STATE_RXDONE) {
        rm2_rxdata_isr_DONE:
        spirit1_strobe(RFSTROBE_SABORT);
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
            spirit1_strobe(RFSTROBE_SABORT);
            rm2_reenter_rx(radio.evtdone);
            return;
        }
    }
    
    /// 4. Handle main types of RX FIFO data
    switch ((rfctl.state >> RADIO_STATE_RXSHIFT) & (RADIO_STATE_RXMASK >> RADIO_STATE_RXSHIFT)) {
    
        /// RX State 0: Automatic Packet Control
        /// SPIRIT1 automatically handles the following packet RX:
        /// <LI> All background packets </LI>
        case (RADIO_STATE_RXAUTO >> RADIO_STATE_RXSHIFT):
            break;

        /// RX State 1: Paging Mode
        /// Paging Mode is used for Foreground packet reception.  Multiframe packets
        /// use it for the final frame.
//#       if ((M2_FEATURE(MULTIFRAME) == ENABLED) || (M2_FEATURE(FECRX) == ENABLED))
        case (RADIO_STATE_RXPAGE >> RADIO_STATE_RXSHIFT): {
            ot_int chipoctets_left;
        
            if (em2.bytes <= 0) {
                goto rm2_rxdata_isr_DONE;
            }
            
            ///@todo reduce from max-fill, so that more work can be done while
            ///      data is being read-in.
            
            chipoctets_left = (em2.bytes*_SPREAD);
            if (chipoctets_left  <= 96) {
                rfctl.rxlimit   = 96 - chipoctets_left;
                rfctl.state     = RADIO_STATE_RXDONE;
                goto rm2_rxdata_isr_RESIZE; 
            }
            if (rfctl.rxlimit != (96-_RXMAXTHR)) {
                rfctl.rxlimit   = (96-_RXMAXTHR);
                goto rm2_rxdata_isr_RESIZE; 
            }
            break;

            rm2_rxdata_isr_RESIZE:
            spirit1_write(RFREG(FIFO_CONFIG3), rfctl.rxlimit);
            break;
//#       endif
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
#endif
}
#endif





#ifndef EXTF_rm2_rxend_isr
OT_WEAK void rm2_rxend_isr() {
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
OT_WEAK void rm2_txinit(ot_u8 psettings, ot_sig2 callback) {
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
    
    // initialize the CRC/RS disabling byte
    txq.options.ubyte[UPPER] = 0;
    
    /// CSMA-CA interrupt based and fully pre-emptive.  This is
    /// possible using CC1 on the GPTIM to clock the intervals.
    platform_enable_gptim2();
    radio_set_mactimer( (ot_uint)dll.comm.tca );
}
#endif




#ifndef EXTF_rm2_txstop_flood
OT_WEAK void rm2_txstop_flood() {
/// Stop the MAC counter used to clock advertising flood synchronization.
/// Then simply configure TX driver state machine to go to TX Done state
/// as soon as the current packet is finished transmitting.
#if (SYS_FLOOD == ENABLED)
    rfctl.state = RADIO_STATE_TXDONE;
    spirit1_stop_counter();
    spirit1_int_txdone();     
#endif
}
#endif




#ifndef EXTF_rm2_txcsma_isr
OT_WEAK void rm2_txcsma_isr() {
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
            ot_u8       timcfg[8] = {   0, RFREG(PROTOCOL0), DRF_PROTOCOL0, 
                                        DRF_TIMERS5, 5, // RX Timer @ ~240us
                                        33, 1,          // LDC First Interval (default 2 ticks)
                                        0 };            // Alignment Dummy
            
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
                spirit1_set_txpwr( &phymac[0].tx_eirp );
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
                timcfg[2]      |= _PERS_TX;
                type            = MODE_bg;
            }
            else {
                type            = MODE_fg;
            }
#           endif
            
            subrfctl_buffer_config(type, (em2.bytes*_SPREAD) /*q_span(&txq)*/);
            spirit1_int_off();
            spirit1_iocfg_tx();
            
            // If manual calibration is used, it is done here
            __CALIBRATE();

            // No CSMA enabled, so jump to transmit
            if (1) {
            //if (dll.comm.csmaca_params & M2_CSMACA_NOCSMA) {
                spirit1_spibus_io(3, 0, timcfg);
                goto rm2_txcsma_START;
            }
            
            // Setup CSMA/CCA parameters for this channel and fall through
            spirit1_write(RFREG(RSSI_TH), (ot_u8)phymac[0].cca_thr );
            
            timcfg[6] = (phymac[0].tg - 1);
            spirit1_spibus_io(7, 0, timcfg);
        }
        
        /// 2. Fall through from CSMA setup.  This code bypassed for No-CSMA
        ///    case.  This code directly accessed on repeat-CCA after fail.
        ///    Also setup to calibrate LDC RCO clock every X uses of CSMA.
        case (RADIO_STATE_TXCCA1 >> RADIO_STATE_TXSHIFT): 
        case (RADIO_STATE_TXCCA2 >> RADIO_STATE_TXSHIFT): {
            ot_u8 protocol2;
            rfctl.state = RADIO_STATE_TXCCA1;
            protocol2   = (DRF_PROTOCOL2 | _LDC_MODE);
            if (--rfctl.nextcal < 0) {
                rfctl.nextcal   = RF_PARAM(RCO_CAL_INTERVAL);
                protocol2      |= _RCO_CALIBRATION;
            }
            spirit1_write(RFREG(PROTOCOL2), protocol2);
            spirit1_int_csma();
            subrfctl_force_ready();
            radio.state = RADIO_Csma;
            spirit1_strobe(STROBE(RX));
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
            //txq.front[2] = 6;     //hack for packet sniff debugging on Chipcon kits
            
            subrfctl_force_ready(); 
            radio_flush_tx();
            em2_encode_data();
            
            // Prepare for TX, then enter TX
            // For floods, we must activate the flood counter right before TX
            radio.state     = RADIO_DataTX;
            rfctl.state     = RADIO_STATE_TXDATA;
            rfctl.txlimit   = RADIO_BUFFER_TXMAX;   // Change TXlimit to max buffer
            spirit1_strobe( RFSTROBE_TX );
            spirit1_int_txdata();
            
            if (rfctl.flags & RADIO_FLAG_FLOOD) {
                spirit1_start_counter();
            }
            break;
        }
    }
}
#endif



void subrfctl_ccafail_isr() {
/// CCA has been pre-empted by a Sync-detect or a CS interrupt, which of course
/// means that the CCA scan has failed.
    spirit1_int_off();
    spirit1_write(RFREG(PROTOCOL2), 0);     //Turn-off LDC, RCO-Cal, VCO-Cal
    subrfctl_smart_standby();
    
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
        spirit1_write(RFREG(PROTOCOL2), _LDC_MODE);     //Turn-off RCO-Cal, VCO-Cal
        spirit1_strobe(STROBE(LDC_RELOAD));
    }
}





#ifndef EXTF_rm2_txdata_isr
OT_WEAK void rm2_txdata_isr() {
/// Continues where rm2_txcsma() leaves off.
    
    /// This is a safety-check that can be removed in more mature builds
#   if (0) //defined(__DEBUG__)
    if ((rfctl.state & RADIO_STATE_TXMASK) != RADIO_STATE_TXDATA) {
        rm2_kill();
        return;
    }
#   endif
    
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
        spirit1_int_txdone();
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
    subrfctl_smart_ready();
    subrfctl_finish(0, 0);
}












/** Radio Subroutines
  * ============================================================================
  * - Usually some minor adjustments needed when porting to new platform
  * - See integrated notes for areas sensitive to porting
  */

void subrfctl_null(ot_int arg1, ot_int arg2) { }


//void subrfctl_kill(ot_int errcode) {
//    radio_gag();
//    subrfctl_smart_ready();
//    subrfctl_finish(errcode, 0);
//}



void subrfctl_finish(ot_int main_err, ot_int frame_err) {
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
        ot_u8 next_channel = dll.comm.tx_chanlist[i];
        if (subrfctl_channel_fastcheck(next_channel))   break;
        if (subrfctl_channel_lookup(next_channel, fp))  break;
    }

    vl_close(fp);
    return (ot_bool)(i < dll.comm.tx_channels);
}



ot_bool subrfctl_channel_fastcheck(ot_u8 chan_id) {
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
            phymac[0].tx_eirp   = scratch.ubyte[0] & 0x80;
            phymac[0].tx_eirp  |= spirit1_clip_txeirp(scratch.ubyte[0] & 0x7f);
            phymac[0].link_qual = scratch.ubyte[1];

            ///@todo Try this: *(ot_u16*)&phymac[0].cs_thr = vl_read(fp, i+4);  
            ///it will need some rearrangement in phymac struct
            scratch.ushort      = vl_read(fp, i+4);
            phymac[0].cs_thr    = scratch.ubyte[0];
            phymac[0].cca_thr   = scratch.ubyte[1];

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

///@note this flag no longer used
//#   if (M2_FEATURE(FEC) == ENABLED)
//    rfctl.flags |= phymac[0].channel >> 7;
//#   endif

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
    buf_cfg[1]  = RFREG(PCKTCTRL2);                                
    //is_hs       = (phymac[0].channel & 0x20) >> 1;
    //buf_cfg[2]  = DRF_PCKTCTRL2_LSBG + is_hs;                                  
    //buf_cfg[2] |= ((ot_u8)mode & 1);
    buf_cfg[2]  = DRF_PCKTCTRL2_LSBG;
                             
#   if (DRF_SYNC_BYTES == 2)                                         
    is_fec      = (phymac[0].channel & 0x80) >> 5;      // 0 when no fec, 4 when FEC                              
    mode       += is_fec;                              
    buf_cfg[3]  = _WHIT_EN | (is_fec >> 2);
    buf_cfg[4]  = ((ot_u8*)&param)[UPPER];
    buf_cfg[5]  = ((ot_u8*)&param)[LOWER];
    buf_cfg[6]  = 0;
    buf_cfg[7]  = 0;
    buf_cfg[8]  = sync_matrix[mode];
    buf_cfg[9]  = sync_matrix[mode+1];
    
#   elif (DRF_SYNC_BYTES == 3) || (DRF_SYNC_BYTES == 4)
    is_fec      = (phymac[0].channel & 0x80) >> 4;      // 0 when no fec, 8 when FEC
    mode       += is_fec;
    buf_cfg[3]  = _WHIT_EN | (is_fec >> 3);                    
    buf_cfg[4]  = ((ot_u8*)&param)[UPPER];
    buf_cfg[5]  = ((ot_u8*)&param)[LOWER];
    buf_cfg[6]  = sync_matrix[mode];
    buf_cfg[7]  = sync_matrix[mode+1];
    buf_cfg[8]  = sync_matrix[mode+2];
    buf_cfg[9]  = sync_matrix[mode+3];
#   endif
    
    spirit1_spibus_io(10, 0, buf_cfg);
}




void subrfctl_prep_q(ot_queue* q) {
/// Put some special data in the queue options field.
/// Lower byte is encoding options (i.e. FEC)
/// Upper byte is processing options (i.e. CRC)
    q->options.ubyte[UPPER]    += 1;
    q->options.ubyte[LOWER]     = 0;
    //q->options.ubyte[LOWER]     = (phymac[0].channel & 0x80);
}






void subrfctl_save_linkinfo() {
#if (OT_FEATURE(RF_LINKINFO))
    static const ot_u8 cmd[2] = { 0x01, RFREG(LINK_QUALIF2) };
    
    // Do Read command on LINK_QUALIF[2:0] and store results in link structure
    spirit1_spibus_io(2, 3, (ot_u8*)cmd);
    
    // Convert 3 byte SPIRIT1 output into 4 byte data structure
    radio.link.pqi  = spirit1.busrx[0];
    radio.link.sqi  = spirit1.busrx[1] & ~0x80;
    radio.link.lqi  = spirit1.busrx[2] >> 4;
    radio.link.agc  = spirit1.busrx[2] & 0x0f;  ///@note this is basically useless
#endif
}




ot_bool subrfctl_mac_filter() {
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

