/* Copyright 2010-2020 JP Norair
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
  * @file       /io/stm32wl_lora/radio_rm2.c
  * @author     JP Norair
  * @version    R103
  * @date       31 Jan 2022
  * @brief      Mode 2 Radio Layer Implementation for STM32WL
  * @ingroup    Radio
  *
  * @todo this file is a very incomplete port from SX127x to STM32WL LoRa.
  *       It is only used right now for compiling platform tests, and it
  *       will be overhauled shortly. 
  *
  * <LI> The main header file for this is: /include/m2/radio.h      </LI>
  * <LI> Other headers for SX127x are in: /include/io/stm32wl_lora/     </LI>
  * <LI> Local header "radio_rm2.h" allows patching of the functions within. </LI>
  *
  * For DASH7 Silicon certification, there are four basic tiers of HW features:
  * 1. PHY      The HW has a buffered I/O and the basic features necessary
  * 2. PHY+     The HW can do encoding, CRC, and some packet handling
  * 3. MAC      The HW can automate some inner loops, like Adv Flood and CSMA
  * 4. MAC+     The HW has most features of the MAC integrated
  *
  * The SX127x basically meets (1) and (2) criteria, although using the LoRa
  * modulation.  We choose to do software CRC encoding in order to guarantee
  * that we are using a strong polynomial, as it appears LoRa HW uses the 
  * horribly weak CCITT poly.
  ******************************************************************************
  */

#include <otstd.h>
#if (OT_FEATURE(M2) == ENABLED)

#include <otplatform.h>
#include <io/stm32wl_lora/config.h>
#include <io/stm32wl_lora/interface.h>

#include <m2/bgcrc8.h>
#include <m2/encode.h>

#include <otlib/buffers.h>
#include <otlib/crc16.h>
#include <otlib/utils.h>

// Local header for subroutines and implementation constants (supports patching)
#include "radio_rm2.h"

#if (M2_FEATURE_HSCODE)
#   include <hblib/hscode.h>
#   include <hblib/lorallr.h>
    hsc_t       hsc;
    lorallr_t   lorallr;
    radio_snr_t loralink;
#endif


#define _ENABLE_BGCAD


/// RFCORE_DEBUG can be uncommented in order to activate the CORE DUMP feature.
/// CORE DUMP will read all the SX127x registers and save them to an array.
/// Printing them out is too slow -- the registers can change too fast.
/// Instead, set breakpoints and inspect the rfcore_dump[] array via debugger HW.
//#define RFCORE_DEBUG
//
#ifdef RFCORE_DEBUG
#   define __CORE_DUMP()   wllora_coredump(rfcore_dump, 112)
    ot_u8 rfcore_dump[112];
#else
#   define __CORE_DUMP();
#endif


// For doing some quick and dirty metrics on CAD noise



/** Local Data declaration
  * Described in radio_SX127x.h
  */
rfctl_struct rfctl;




/** Local Subroutine Prototypes  <BR>
  * ========================================================================<BR>
  */
void sub_hwtimeout_isr(void);
void sub_initrx(void);
void sub_initcad(void);


/** Virtual ISR RF  <BR>
  * ========================================================================<BR>
  */

#define RFIV_LISTEN     0
#define RFIV_RXDONE     1
#define RFIV_RXTIMEOUT  2
#define RFIV_RXHEADER   4
#define RFIV_CCA        5
#define RFIV_TXDONE     6


void wllora_virtual_isr(ot_u8 code) {
///@todo this function is only a stub for the purpose of test compiling
    rm2_kill();
}


#ifndef EXTF_radio_mac_isr
OT_WEAK void radio_mac_isr(void) {
///@todo this function is only a stub for the purpose of test compiling

	// Utilized for foreground reception and also soft reception of any FEC frame
	if (radio.state == RADIO_DataRX) {
		rm2_rxdata_isr();
		return;
	}
    
    // All usages other than RX sample timer are one-shot
	systim_disable_insertion();

    // Used as CA insertion timer.
    if (radio.state == RADIO_Csma) {
    	rm2_txcsma_isr();
    	return;
    }
    
    // if RSSI is below BG Threshold, cancel BG RX
    // Note: Fall through to kill on weak rssi
    if (radio.state == RADIO_Listening) {
    	ot_u8 chan_rssi;
    	chan_rssi = wllora_read(RFREG_LR_RSSIVALUE);
    	if (chan_rssi >= phymac[0].cs_thr) {
    		return;
    	}
    }
    
    // Fall-through case, but also here for false state handling
	rm2_kill();
}
#endif





/** Mode 2 - Generic Radio Module control    <BR>
  * ========================================================================<BR>
  * By "generic" it means that it is not Mode2 specific
  */

#ifndef EXTF_radio_init
OT_WEAK void radio_init(void) {
///@todo this function is kept as-is, even though WL impl is token-only

/// Transceiver implementation dependent
    //vlFILE* fp;

    /// Set SX127x-dependent initialization defaults
    rfctl.flags     = 0;

    /// Set universal Radio module initialization defaults
    radio.state     = RADIO_Idle;
    radio.evtdone   = &otutils_sig2_null;

    /// Initialize the bus between SX127x and MCU, and load defaults.
    /// SX127x starts-up in STANDBY (READY), so we set the state and flags
    /// to match that.  Then, init the bus and send RADIO to sleep.
    /// SX127x can do SPI in Sleep.
    wllora_init_bus();
    wllora_load_defaults();

    /// Verify Register settings after defaults are loaded.
    /// For debugging purposes.  Macro resolves to nothing if RF_DEBUG is off.
    //__CORE_DUMP();

    /// Done with the radio init: wllora_load_defaults() requires that the 
    /// radio is in SLEEP already, so explicit sleep call is commented here.
    //radio_sleep();

    /// Initialize RM2 elements such as channels, link-params, etc.
    rm2_init();
}
#endif



#ifndef EXTF_radio_finish
OT_WEAK void radio_finish(ot_int main_err, ot_int frame_err) {
///@todo this function is kept as-is, even though WL impl is token-only

/// Reset radio & callback to null state, then run saved callback
    ot_sig2 callback;
    radio_gag();                            // redundant, but here for robustness
    dll_unblock();                          // also redundant and here for robustness
    
    rfctl.state     = 0;
    rfctl.flags    &= (RADIO_FLAG_XOON);    //clear all other flags
    callback        = radio.evtdone;
    radio.evtdone   = &otutils_sig2_null;
    callback(main_err, frame_err);
}
#endif



#ifndef EXTF_radio_set_mactimer
OT_WEAK void radio_set_mactimer(ot_u16 clocks) {
/// Used for high-accuracy TX/CSMA slot insertion, and flooding.
    systim_enable_insertion();
    systim_set_insertion(clocks);
}
#endif



#ifndef EXTF_radio_activate_queue
OT_WEAK void radio_activate_queue(ot_queue* q) {
/// Put some special data in the queue options field.
/// Lower byte is encoding options (i.e. FEC)
/// Upper byte is processing options (i.e. CRC)
    q->options.ubyte[UPPER]    += 1;
    q->options.ubyte[LOWER]     = 0;
}
#endif



#ifndef EXTF_radio_buffer
OT_WEAK ot_u8 radio_buffer(ot_uint index) {
/// Transceiver implementation dependent
/// This function is not used on the SX127x
    return 0;
}
#endif








/** Radio Mode 2 Setup Functions
  * ============================================================================
  * @todo globalize the routines here which can be globalized, in radio_task.c
  */
OT_INLINE ot_u8 rm2_clip_txeirp(ot_u8 m2_txeirp) {
    return wllora_clip_txeirp(m2_txeirp & 0x7f);
}

OT_INLINE ot_u8 rm2_calc_rssithr(ot_u8 m2_rssithr) {
    return wllora_calc_rssithr(m2_rssithr);
}


#ifndef DRF_MOD1_SS
#   define DRF_MOD1_SS      DRF_MOD1_LS
#   define DRF_MOD0_SS      DRF_MOD0_LS
#   define DRF_CHFLT_SS     DRF_CHFLT_LS
#endif



/// This section below has SF and modemconfig2 settings for certain different chips
/// This should be in channel selection, not RX/TX startup, but sometimes SX127x bundle
/// data into registers in a less-than-logical way.
static const ot_u8 mdmcfg2[4] = {
    _SF_11| (DRF_LR_MODEMCONFIG2 & 0x0F),
    _SF_9 | (DRF_LR_MODEMCONFIG2 & 0x0F),
    _SF_7 | (DRF_LR_MODEMCONFIG2 & 0x0F),
    _SF_5 | (DRF_LR_MODEMCONFIG2 & 0x0F)
};
static const ot_u8 mdmcfg2_sf[4] = {
    _SF_11,
    _SF_9,
    _SF_7,
    _SF_5
};


/// sf_code must be 0, 1, 2, 3.
/// Otherwise behavior is unpredictable.
///
/// Rule: code 0 = SF11     Used in paging mode
///       code 1 = SF9
///       code 2 = SF7
///       code 3 = SF5      (only SX126x)
///

OT_INLINE ot_u8 sub_modemconfig2_baseval(ot_u8 sf_code) {
    return mdmcfg2[sf_code];
}
OT_INLINE ot_u8 sub_modemconfig2_sfval(ot_u8 sf_code) {
    return mdmcfg2_sf[sf_code];
}




// Patch function
ot_bool rm2_channel_fastcheck(ot_u8 chan_id) {
    // Check if there's a forced-refresh condition (always fail)
    if (radio.flags & RADIO_FLAG_REFRESH) {
        radio.flags ^= RADIO_FLAG_REFRESH;
        return False;
    }
    // Use Last Channel on chan_id == 0, or same explicit channel as already set
    if ((chan_id == 0) || (chan_id == phymac[0].channel)) {
        return True;
    }
    return False;
}



#ifndef EXTF_rm2_enter_channel
OT_WEAK void rm2_enter_channel(ot_u8 old_chan_id, ot_u8 old_tx_eirp) {
    // The detection matrix optimizes RX depending on the channel specification
    static const ot_u8 detection[16][2] = {
        {0xC0|0x03, 0x0A},      // 0000 - no encoding, SF11
        {0xC0|0x03, 0x0A},      // 0001 - no encoding, SF9
        {0xC0|0x04, 0x0A},      // 0010 - no encoding, SF7
        {0xC0|0x05, 0x0A},      // 0011 - no encoding, SF5
        {0xC0|0x03, 0x0A},      // 0100 -------------------------
        {0xC0|0x03, 0x0A},      // 0101 Undefined option,
        {0xC0|0x04, 0x0A},      // 0110 treat these as unencoded
        {0xC0|0x05, 0x0A},      // 0111 -------------------------
        {0xC0|0x02, 0x02},      // 1000 - encoding, SF11
        {0xC0|0x02, 0x04},      // 1001 - encoding, SF9
        {0xC0|0x03, 0x04},      // 1010 - encoding, SF7
        {0xC0|0x03, 0x04},      // 1011 - encoding, SF5
        {0xC0|0x02, 0x02},      // 1100 -------------------------
        {0xC0|0x02, 0x04},      // 1101 Undefined option,
        {0xC0|0x03, 0x04},      // 1110 treat these as encoded
        {0xC0|0x04, 0x04}       // 1111 -------------------------
    };

    /// PA reprogram
    if (old_tx_eirp != phymac[0].tx_eirp) {
        wllora_set_txpwr( phymac[0].tx_eirp );
    }
    
    // old_chan_id becomes an XOR: 0 bits mean no change.
    old_chan_id ^= phymac[0].channel;

    /// Configure Threshold Settings based on spreading factor and encoding.
    if (old_chan_id & 0xF0) {
        const ot_u8* detect_regs = detection[phymac[0].channel>>4];
        wllora_write(RFREG_LR_DETECTOPTIMIZE, detect_regs[0]);
        wllora_write(RFREG_LR_DETECTIONTHRESHOLD, detect_regs[1]);
    }

    /// Configure Channel Center Frequency
    if (old_chan_id & 0x0F) {
        wllora_configure_chan(phymac[0].flags, (phymac[0].channel & 0x0F)-1);
    }
}
#endif




#ifndef EXTF_rm2_mac_configure
OT_WEAK void rm2_mac_configure(void) {
/// Only use this when there is a hardware MAC filtering ability.  
/// STM32WL RF Core does not have this capability.
}
#endif


#ifndef EXTF_rm2_calc_link
OT_WEAK void rm2_calc_link(void) {
	ot_u8 prssi_code;
	ot_s8 psnr_code;
	prssi_code			= wllora_read(RFREG_LR_PKTRSSIVALUE);
	psnr_code           = wllora_read(RFREG_LR_PKTSNRVALUE);

#   if (M2_FEATURE_HSCODE)
	loralink.mean_snr   = (ot_s16)psnr_code;
#   endif

    radio.last_rssi     = wllora_calc_rssi(prssi_code, psnr_code);
    radio.last_linkloss = (ot_int)(rxq.front[2] & 0x7F) - 80 - RF_HDB_RXATTEN;
    radio.last_linkloss = (radio.last_linkloss - radio.last_rssi*2) >> 1;
}
#endif


#ifndef EXTF_rm2_flood_getcounter
OT_WEAK ot_int rm2_flood_getcounter(void) {
/// The time value put into countdown should indicate the amount of time following
/// the end of the BG packet it is inserted into.  On SX127x, there isn't a nice
/// way to send a continuous flood, so we consider the time on air of a whole
/// BG packet (about 29 symbols, or 7 ticks).
    ot_int offset_count;
    offset_count    = (ot_int)wllora_get_counter();
    offset_count   -= 7; //rm2_scale_codec(phymac[0].channel, 29);
    return offset_count;
}
#endif


#ifndef EXTF_rm2_flood_txstop
OT_WEAK void rm2_flood_txstop(void) {
/// Stop the MAC counter used to clock advertising flood synchronization.
/// Then simply configure TX driver state machine to go to TX Done state
/// as soon as the current packet is finished transmitting.
    rfctl.state = RADIO_STATE_TXDONE;
    wllora_stop_counter();
    wllora_int_txdata();
}
#endif


#ifndef EXTF_rm2_flood_txstart
OT_WEAK void rm2_flood_txstart(void) {
    wllora_start_counter();
}
#endif


#ifndef EXTF_rm2_kill
OT_WEAK void rm2_kill(void) {
    __DEBUG_ERRCODE_EVAL(=290);

    __CORE_DUMP();

    radio_gag();
    radio_idle();
    dll_unblock();
    radio_finish(RM2_ERR_KILL, 0);
}
#endif


void sub_hwtimeout_isr(void) {
    /// In BG Listen, system is using RX-Single reception, and we try a few 
    /// times to receive a packet after having CAD validation.
    /// Also, on RX timeout, save the link information.  This is used as a baseline.
	///@todo "tries" feature can be global now (not just for BG).  Test it.
    if (rfctl.flags & RADIO_FLAG_BG) {
        if (--rfctl.tries <= 0) {
            //wlloradrv_save_linkinfo();
            radio_finish(RM2_ERR_TIMEOUT, 1 /*radio.link.sqi*/);
            return;
        }
    }
    
    /// Loop RX-Single listening mode in BG-retry or FG-RX.
    /// In FG-RX, DLL will timeout the RX process manually.
    //rm2_reenter_rx(radio.evtdone);
    wllora_write(RFREG_LR_IRQFLAGS, 0xFF);
    wllora_strobe(_OPMODE_RXSINGLE, False);
}

#ifndef EXTF_rm2_rxtimeout_isr
OT_WEAK void rm2_rxtimeout_isr(void) {
    __DEBUG_ERRCODE_EVAL(=280);

    /// Can't have an interrupt during termination/re-configuration
    radio_gag();

    /// Send timeout error back to DLL
    //wlloradrv_save_linkinfo();
    radio_finish(RM2_ERR_TIMEOUT, 1 /*radio.link.sqi*/);
}
#endif





/** Radio RX Functions
  * ============================================================================
  */
void sub_initrx(void) {
    //radio_activate_queue(&rxq);
    wllora_iocfg_rx();
    
    ///@todo why is this commented-out?  Does it matter?
    //dll_offset_rxtimeout();
    
    rm2_reenter_rx(radio.evtdone);
}

void sub_initcad(void) {
    // Set I/O for CAD detect
    wllora_write(RFREG_LR_IRQFLAGS, 0xFF);  
    wllora_iocfg_cad();
    wllora_int_listen();
    
    radio.state = RADIO_Listening;
    /// @todo Set MAC-Timer to expire when RSSI is valid during CAD.  This may
    ///       require an all-new timer to use, or at least a faster-working
    ///       MAC Timer.  RSSI only valid for 8us
    ///
    ///
    /// time for STANDBY->SLEEP is ?
    /// time for READY->CAD is ?
    /// time for SLEEP->CAD is 240us
    /// time for CAD->RSSI is dependent on SF and BW
    /// - SF7:500kHZ = 320us
    /// - SF7:125kHZ = 1280us
    //radio_set_mactimer(?);
    
    wllora_antsw_rx();
    wllora_strobe(_OPMODE_CAD, False);  //non-blocking call
}




static const ot_u8 bg1init[3] = {
    (ot_u8)MODE_bg,                 //buffer mode
    //6,                              //pktlen
    //(0x80 | RFREG_LR_MODEMCONFIG2),
    DRF_LR_MODEMCONFIG2_BG & 0x0F,  //ModemCfg2 value, with cleared spreading factor
    DRF_LR_SYMBTIMEOUTLSB_BG        //SymbTimeoutLsb value
};
static const ot_u8 bg2init[3] = {
    (ot_u8)MODE_bg,                 //buffer mode
    //6,                            //pktlen
    //(0x80 | RFREG_LR_MODEMCONFIG2),
    DRF_LR_MODEMCONFIG2_FG & 0x0F,  //ModemCfg2 value, with cleared spreading factor
    DRF_LR_SYMBTIMEOUTLSB_FG        //SymbTimeoutLsb value
};
static const ot_u8 fginit[3] = {
    (ot_u8)MODE_fg,                 //buffer mode
    //255,                          //pktlen
    //(0x80 | RFREG_LR_MODEMCONFIG2),
    DRF_LR_MODEMCONFIG2_FG & 0x0F,  //ModemCfg2 value, with cleared spreading factor
    DRF_LR_SYMBTIMEOUTLSB_FG        //SymbTimeoutLsb value
};
static const ot_u8 pginit[3] = {
    (ot_u8)MODE_pg,                 //buffer mode
    //16,
    //(0x80 | RFREG_LR_MODEMCONFIG2),
    DRF_LR_MODEMCONFIG2_FG & 0x0F,  //ModemCfg2 value, with cleared spreading factor
    DRF_LR_SYMBTIMEOUTLSB_FG        //SymbTimeoutLsb value
};

// Timer here runs at 4096 Hz
// First group of numbers is for CR1
// Second group of numbers is for CR0
//static const ot_u8 iterinit[4] = {
////	84, 21, 6, 6
//    68, 17, 5, 5
//};

#ifndef EXTF_rm2_rxinit
OT_WEAK void rm2_rxinit(ot_u8 channel, ot_u8 psettings, ot_sig2 callback) {
    //ot_u8   netstate;
    ot_u8*  initvals;
    ot_sub  sub_init;

    __DEBUG_ERRCODE_EVAL(=200);

    /// Setup the RX engine for Foreground Frame detection and RX.  Wipe-out
    /// the lower flags (non-persistent flags)
    radio.evtdone   = callback;
    rfctl.flags    &= ~(RADIO_FLAG_CONT | RADIO_FLAG_BG | RADIO_FLAG_PG);
    rfctl.tries 	= 1;

    // LoRa HW cannot support multiframe packets, which simplifies setup a bit
    ///@todo rfctl.tries could be configurable in future
    ///@todo fix CAD detection
    ///@todo re-assign lower bits of netflags to do special format selection
    if (psettings & (M2_NETFLAG_BG)) {
        //netstate    = (M2_NETSTATE_UNASSOC | M2_NETFLAG_FIRSTRX);

        // PG Frame = BG without stream
        // this is temporary hack until lower bits of netflags are re-assigned to
        // special format selection
        if ((psettings & M2_NETFLAG_STREAM) == 0) {
            rfctl.flags |= RADIO_FLAG_PG;
        	initvals = (ot_u8*)pginit;
        	sub_init = &sub_initrx;
        }
        // BG RX Initialization: optional until CAD is perfected
#       ifdef _ENABLE_BGCAD
        else if (psettings & M2_NETSTATE_INIT) {
            rfctl.flags |= RADIO_FLAG_BG;
            rfctl.tries = 3;
		    initvals 	= (ot_u8*)bg1init;
		    sub_init 	= &sub_initcad;
		}
#       endif
        // BG RX Retry: used as main BG RX until CAD is perfected
		else {
		    rfctl.flags |= RADIO_FLAG_BG;
            initvals = (ot_u8*)bg2init;
            sub_init = &sub_initrx;
		}
    }
    else {
      //netstate    = psettings;
        initvals    = (ot_u8*)fginit;
        sub_init    = &sub_initrx;
    }
    
    /// 1. Fetch the RX channel, exit if the specified channel is not available
    if (rm2_test_channel(channel) == False) {
        radio_finish(RM2_ERR_BADCHANNEL, 0);
        return;
    }

    /// 2a. Setup RX for Background detection (if BG):
    ///     <LI> Manipulate ot_queue to fit bg frame into common model </LI>
    ///     <LI> Prepare for CAD-Detect
    ///     <LI> Following CAD-Detect positive, use RX-Single
    /// 2b. Setup RX for Foreground detection (ELSE):
    ///     <LI> Set Foreground paging and tentative packet-len to max </LI>
    wlloradrv_mdmconfig(initvals[0], initvals[1], initvals[2], 0);
    
    ///@note the iteration ticks corresponds to the duration of one interleaver block
    ///      of LoRa symbols.  "Ticks" in this case is implementation-specific to this driver
    ///      and not [necessarily] DASH7 standard ticks.  This impl has iter_ti = 0.25 Ticks.
    rfctl.iter_ti = (ot_u8)(wllora_block_miti(&phymac[0]) >> 8) + 1;

    ///@note Initvals[3] contains the Spreading Factor, which must be re-applied here.
    //initvals[3] |= sub_modemconfig2_sfval((channel>>4)&3);

    // Write to registers.  There's a optimized call commented out
    //wllora_write(RFREG_LR_MODEMCONFIG2, initvals[3]);

    // Write the final RX timeout value
    //wllora_write(RFREG_LR_SYMBTIMEOUTLSB, initvals[2]);

    /// 3.  Use CAD or RX intialization subroutine
    sub_init();
}
#endif




#ifndef EXTF_rm2_rxtest
void sub_rxtest_callback(ot_int a, ot_int b) {
/// Loop RX until timeout occurs: Could log packet here as well
    ///@todo log packet here
    sub_initrx();
}


///@todo this function needs to be completely redone
OT_WEAK void rm2_rxtest(ot_u8 channel, ot_u8 tsettings, ot_u16 timeout) {
    // Set basic operational flags
    rfctl.flags    &= ~(RADIO_FLAG_CONT | RADIO_FLAG_BG);
    //netstate        = tsettings;                    ///@todo bury this into session top?
    radio.evtdone   = &sub_rxtest_callback;
    
    // We don't care if the channel is supported, because no data is going to
    // be sent.  Just set the center frequency to where it is desired
    rm2_enter_channel((channel & 0x7f), (phymac[0].tx_eirp & 0x7f));
    
    // Configure radio for Foreground reception (nominal)
    wlloradrv_mdmconfig(fginit[0], fginit[1], fginit[2], 0);
    
    // Set MAC timer as termination timer
    radio_set_mactimer(timeout << 2);
}
#endif



#ifndef EXTF_rm2_reenter_rx
OT_WEAK void rm2_reenter_rx(ot_sig2 callback) {
/// Restart RX using the same settings that are presently in the radio core.
    //ot_u8 opmode;
    radio.evtdone   = callback;
    rfctl.state     = RADIO_STATE_RXAUTO;
    //opmode          = (rfctl.flags & RADIO_FLAG_BG) ? _OPMODE_RXSINGLE : _OPMODE_RXCONT;
    
    // Reset interrupts and FIFO for fresh RX'ing
    radio_gag();                            
    radio_flush_rx();
    
    // Pre-RX core dump to verify RX register settings
    // Core dump is only done when RFCORE_DEBUG is defined
    __CORE_DUMP();
    
    //radio_idle();
    wllora_antsw_rx();
    wllora_strobe(_OPMODE_RXSINGLE, False);  //wllora_strobe(opmode, False);   //non-blocking call
    
    // Undo packet sync on re-entry.
    // Main work here is to change Radio IRQ state and task priority.
    // Occurs when you need to listen to more packets.
    //wlloradrv_unsync_isr();
    wllora_int_rxdata();
    radio.state = RADIO_Listening;
    dll_unblock();
}
#endif


#ifndef EXTF_rm2_rxsync_isr
OT_WEAK void rm2_rxsync_isr(void) {
/// Prepare driver for data reception, update high-level module state, and have
/// supervisor task (DLL) go into high-priority mode (via dll_block())
/// Other notes:
/// - Multiframe packets are not possible in LoRa, so FRAME-CONT bit always 0
/// - LoRa packet reception is done with a manually-timed buffer interval,
///   via the mactimer.  LoRa's built-in packet engine is too dogmatic to be
///   useful for advanced applications (like this one)

    __DEBUG_ERRCODE_EVAL(=210); 
    
    radio.state = RADIO_DataRX;
    dll_block();

    // Manual download timing.
    // Either wait for the end of the packet (iter_ti=0: unencoded BG and PG frames)
    // or setup a timer interrupt.
    if (rfctl.iter_ti != 0) {
    	radio_set_mactimer(rfctl.iter_ti);
    }
    wllora_int_rxend();

    em2_decode_newpacket();
    em2_decode_newframe();

}
#endif


#ifndef EXTF_rm2_rxdata_isr
OT_WEAK void rm2_rxdata_isr() {

    /// 1. Decode Available Data.
    ///    em2_decode_data() must handle the header flags in FG frames (NEWHEADER, crc5)
    em2_decode_data();

    /// 2. Case 1: A-Priori Fixed Length: BG and PG.
    ///            Don't need to re-set em2.bytes or meddle with the radio buffer settings.
    // (Nothing to do here)

    /// 3. Case 2: FG frames that have dynamic length and an explicit header with frame length.
    ///            We only need to specially handle the header case.
    ///            em2_decode_data() must set NEWHEADER flag and crc5 element appropriately.
    if (em2.state < 0) {
        /// Failure case on em2.state < 0 (decoding irregularity)
        systim_disable_insertion();
        radio.state = RADIO_Idle;
        radio_gag();
        radio_idle();
        rm2_reenter_rx(radio.evtdone);
    }
    else if ((em2.crc5 == 1) && ((rfctl.flags & 7) == 0)) {
        ot_uint rx_octets;

#		if (M2_FEATURE(HSCODE))
        if (phymac[0].channel & 0x80) {
            rx_octets   = (ot_uint)hsc_octetsinframe((hscrate_t)(rxq.front[1]&7), rxq.front[0]-7);
            rx_octets  += 16;
        }
        else
#       endif
        {
            rx_octets = rxq.front[0]+1;
            systim_disable_insertion();
        }

        wllora_write(RFREG_LR_PAYLOADLENGTH, (ot_u8)rx_octets);
    }
}
#endif



#ifndef EXTF_rm2_rxend_isr
OT_WEAK void rm2_rxend_isr(void) {

    __DEBUG_ERRCODE_EVAL(=230);
    
    systim_disable_insertion();					// Here for safety purposes

    radio_gag();                                // No more Radio interrupts!
    rm2_calc_link();                            // Calculate relative link info
    rfctl.state = RADIO_STATE_RXDONE;           // Make sure in DONE State, for decoding
    
    em2_decode_data();                          // decode any leftover data
    
    // The DLL we be re-opened in this ISR via radio_finish, but only if it's not
    // already scheduled for Stage 2 decoding.
    if (sys.task_RFA.event != 6) {
        ot_u16 integrity = em2_decode_endframe();
        radio_finish(0, integrity);
    }
}
#endif



#ifndef EXTF_rm2_decode_s2
void rm2_decode_s2(void) {
#if (M2_FEATURE_HSCODE == ENABLED)
    ot_int  unprocessed_bytes;
    ot_u8   blockcrc;
    llr_t*  llrbits;

    // If decode_s2 function is called, it means there is at least one block of data to decode
    do {
        // lorallr_getblock() will generate LLR in llrbuf by de-interleaving,
        // and interpolating input data, SNR, and RSSI values
        llrbits = lorallr_getblock(&lorallr, rxq.getcursor);

        // Decode.  If the block is bad, setting em2.state --> -1 will terminate reception
        // on next interrupt.
        blockcrc = hsc_decode(&hsc, rxq.putcursor, llrbits);
        if (blockcrc != 0) {
            rxq.options.ushort  = 0;
            em2.state           = -1;
            goto rm2_decode_s2_WAITNEXT;
        }

        // Following a successful decode operation, need to pop the llr block
        lorallr_popblock(&lorallr);

        // The putcursor is advanced by the number of decoded bytes,
        // and the hsc object is reinitialized to the explicit block size for the next block
        rxq.putcursor += hsc.infobytes;
        hsc_init(&hsc, (hscrate_t)(rxq.front[1] & 7));

        // Move ahead the block decode cursor
        rxq.getcursor      += 16;
        unprocessed_bytes   = em2.bytes - (rxq.getcursor - (rxq.front + rxq.options.ushort));
    } while (unprocessed_bytes >= 16);

    // If the frame-end event is detected, do final processing
    if (rfctl.state == RADIO_STATE_RXDONE) {
        radio_finish(0, em2_decode_endframe());
        return;
    }

    // Kill the RX process if another block isn't downloaded within the time another
    // block is expected.
    ///@note The customary usage of "sys_task_setnext()" is replaced with a direct store to
    ///      nextevent, which is almost always a bad idea, but here it is the best thing to do.
    rm2_decode_s2_WAITNEXT:
    sys.task_RFA.event      = 5;
    sys.task_RFA.nextevent  = (ot_uint)wllora_hscblock_ti(&phymac[0]);
    return;
#endif
}
#endif





/** Radio TX Functions  <BR>
  * ========================================================================<BR>
  */

#ifndef EXTF_rm2_txinit
OT_WEAK void rm2_txinit(ot_u8 psettings, ot_sig2 callback) {
    ///@todo temporary method until PG mode refined
    static const ot_u8 flag_lut[4] = {
            0,
            RADIO_FLAG_PG,
            RADIO_FLAG_CONT,
            (RADIO_FLAG_BG | RADIO_FLAG_CONT)
    };

    rfctl.flags    &= ~(RADIO_FLAG_BG | RADIO_FLAG_PG | RADIO_FLAG_CONT);
    //rfctl.flags    |= (psettings & (M2_NETFLAG_BG | M2_NETFLAG_STREAM)) >> 6;

    ///@todo temporary method until PG mode refined
    rfctl.flags |= flag_lut[(psettings & (M2_NETFLAG_STREAM | M2_NETFLAG_BG)) >> 6];

    radio.evtdone   = callback;
    radio.state     = RADIO_Csma;
    rfctl.state     = RADIO_STATE_TXINIT;

    /// If 0, it's a retransmission, no re-encoding necessary
    /// This value is revised later.
    txq.options.ubyte[UPPER] = 0;

    /// CSMA-CA interrupt based and fully pre-emptive.  This is
    /// possible using CC1 on the GPTIM to clock the intervals.
    radio_set_mactimer( (ot_uint)dll.comm.tca << 3 );
}
#endif



#ifndef EXTF_rm2_txtest
///@todo this function needs to be completely re-done
OT_WEAK void rm2_txtest(ot_u8 channel, ot_u8 eirp_code, ot_u8 tsettings, ot_u16 timeout) {
    ot_u8 old_channel;
    ot_u8 old_eirp;
    ot_u8 mdmconfig2;

    // We don't care if the channel is supported, because no data is going to
    // be sent.  Just set the center frequency to where it is desired
    old_channel         = phymac[0].channel;
    old_eirp            = phymac[0].tx_eirp;
    phymac[0].channel   = channel;
    phymac[0].tx_eirp   = eirp_code;
    rm2_enter_channel(old_channel, old_eirp);

    // This bit enables TX-Cont mode.
    ///@todo possible to have callback here that disables TX_CONT_ON
    mdmconfig2 = sub_modemconfig2_baseval((phymac[0].channel>>4) & 3);
    wllora_write(RFREG_LR_MODEMCONFIG2, mdmconfig2);

    // No callback
    radio.evtdone = &otutils_sig2_null;

    wllora_int_off();
    wllora_iocfg_tx();
    radio_flush_tx();

    // Verify Register settings ahead of TX Test (continuous random TX)
    // Core dump is only done when RFCORE_DEBUG is defined
    __CORE_DUMP();
    
    radio.state = RADIO_DataTX;
    wllora_antsw_tx( (ot_bool)(BOARD_FEATURE(RF_PABOOST)) );
    wllora_strobe( _OPMODE_TX , False); //non-blocking call
}
#endif



#ifndef EXTF_rm2_resend
OT_WEAK void rm2_resend(ot_sig2 callback) {
    radio.evtdone               = callback;
    radio.state                 = RADIO_Csma;
    rfctl.state                 = RADIO_STATE_TXINIT;
    txq.options.ubyte[UPPER]    = 255;
    radio_set_mactimer(0);
}
#endif


#ifndef EXTF_rm2_txcsma_isr
void sub_cad_csma(void) {
    radio_gag(); 
    wllora_iocfg_cad();
    wllora_int_csma();
    radio_idle();
    
    wllora_antsw_rx();
    wllora_strobe(_OPMODE_CAD, False);  //non-blocking call
    radio.state = RADIO_Csma;
}

ot_bool sub_cca_isfail(void) {
    ot_bool test;
    test = wllora_check_cadpin();
    if (test) {
        wllora_int_off();
        rfctl.state = RADIO_STATE_TXCAD1;
        
        // This calls CSMA loop
        radio.evtdone(1, 0);
    }
    return test;
}

OT_WEAK void rm2_txcsma_isr(void) {
    ///@todo this is a hack to get paging features exposed
    static const ot_u8 mode_lut[8] = { 
        MODE_fg, MODE_bg, MODE_fg, MODE_bg, MODE_pg, MODE_pg, MODE_pg, MODE_pg };

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
            ot_u8 mdmconfig2;

            // Find a usable channel from the TX channel list.  If none, error.
            if (rm2_test_chanlist() == False) {
                radio.evtdone(RM2_ERR_BADCHANNEL, 0);
                break;
            }

            // Set Spreading Factor (required here) -- Now done in sub_mdmconfig()
            //mdmconfig2 = sub_modemconfig2_baseval((phymac[0].channel>>4) & 3);
            //wllora_write(RFREG_LR_MODEMCONFIG2, mdmconfig2);
            
            // Get Queue and encoder ready for transmission.  We do this here
            // in order to set em2.bytes properly, which becomes the payload
            // length on LoRa.  
            radio_activate_queue(&txq);
            em2_encode_newpacket();
            em2_encode_newframe();

            // Set other TX Buffering & Packet parameters, and also save the
            // Peristent-TX attribute for floods, which is written later
            //type = (rfctl.flags & RADIO_FLAG_BG) ? MODE_bg : MODE_fg;
            ///@note this hack is for paging
            type = mode_lut[rfctl.flags & 7];
            wlloradrv_mdmconfig(type, (DRF_LR_MODEMCONFIG2 & 0x0F), (0), em2.bytes);
            wllora_int_off();
            
            // Bypass CSMA if MAC is disabling it
            //if (dll.comm.csmaca_params & M2_CSMACA_NOCSMA) {
                goto rm2_txcsma_START;
            //}
            
            // Enter CSMA via fallthrough.  
            // There are two incremental tests: CCA1 and CCA2.
        } 

        // 2-3. First CCA: If it is valid, set MAC timer to invoke CAD2.
        // If invalid, return to CAD1 via sub_caa_isfail().
        case (RADIO_STATE_TXCAD1 >> RADIO_STATE_TXSHIFT):
            rfctl.state = RADIO_STATE_TXCCA1;
            sub_cad_csma();
            break;
            
        case (RADIO_STATE_TXCCA1 >> RADIO_STATE_TXSHIFT):
            if (sub_cca_isfail() == False) {
                radio_sleep();
                rfctl.state = RADIO_STATE_TXCAD2;

                ///@todo there's something wrong with the wakeup timer, it isn't
                ///      triggering the interrupt on the haytag.  Need to figure
                ///      out why and fix it.
                radio_set_mactimer(0 /*phymac[0].tg */);
            }
            break;
        
        // 4-5. 2nd CCA: If it is valid, fall through to TX START
        // If invalid, return to CAD1 via sub_caa_isfail().
        case (RADIO_STATE_TXCAD2 >> RADIO_STATE_TXSHIFT):
            rfctl.state = RADIO_STATE_TXCCA2;
            sub_cad_csma();
            break;

        case (RADIO_STATE_TXCCA2 >> RADIO_STATE_TXSHIFT): 
            if (sub_cca_isfail()) {
            	///@todo is it necessary to go back to CCA1 from here?
                break;
            }

        /// 6. TX startup:
        case (RADIO_STATE_TXSTART >> RADIO_STATE_TXSHIFT): {
        rm2_txcsma_START:
            // Send TX start (CSMA done) signal to DLL task
            // arg2: Non-zero for background, 0 for foreground
            rfctl.state = RADIO_STATE_TXSTART;

            ///@todo there is a race condition that can occur here between RX and TX processes,
            /// which will cause SX127x to freak-out.
            /// One issue is that the sys.task_RFA.event is getting set to 5 (TX) before an
            /// RX process is completely over (some priority inversion is happening)
            radio.evtdone(0, (rfctl.flags & (RADIO_FLAG_PG | RADIO_FLAG_CONT | RADIO_FLAG_BG)));
            radio_gag();
            
            // Preload into TX FIFO all packet data
            // There are slightly different processes for BG and FG frames
            txq.front[2] = phymac[0].tx_eirp;
            if (rfctl.flags & RADIO_FLAG_BG) {
                radio_idle();
                if (rfctl.flags & RADIO_FLAG_CONT) {
                    rm2_flood_txstart();
                }
                radio.evtdone((rfctl.flags & RADIO_FLAG_CONT), 0);
            }

            // Clear Radio for TX
            radio_flush_tx();
            em2_encode_data();

            // Prepare for TX, then enter TX
            // For floods, we must activate the flood counter right before TX
            radio.state = RADIO_DataTX;
            rfctl.state = RADIO_STATE_TXDATA;
            wllora_iocfg_tx();
            
            // Pre-TX Core Dump to verify TX Register Settings.
            // Core dump is only done when RFCORE_DEBUG is defined
            __CORE_DUMP();
            
            wllora_antsw_tx((ot_bool)BOARD_FEATURE(RF_PABOOST));
            wllora_int_txdata();
            wllora_strobe(_OPMODE_TX, False);   //non-blocking call
            break;
        }
    }
}
#endif


#ifndef EXTF_rm2_txdata_isr
OT_WEAK void rm2_txdata_isr(void) {
/// Continues where rm2_txcsma() leaves off.

    /// Packet BG flooding.  Only needed on devices that can send M2AdvP
    /// The radio.evtdone callback here should update the AdvP payload
    if ((rfctl.flags & RADIO_FLAG_BGFLOOD) == RADIO_FLAG_BGFLOOD) {
        radio.evtdone(RADIO_FLAG_CONT, 0);

        if ((rfctl.state & RADIO_STATE_TXMASK) == RADIO_STATE_TXDATA) {
            bgcrc8_put(&txq.front[2]);
            wllora_write(RFREG_LR_IRQFLAGS, 0xFF);
            wllora_burstwrite(RFREG_LR_FIFO, 6, &txq.front[2]);
            
            ///@todo implement this synchronization stub function
            //rm2_flood_txwait();
            
            //wllora_antsw_tx((ot_bool)(BOARD_FEATURE(RF_PABOOST)));             ///@todo make sure this doesn't need to be here
            wllora_strobe(_OPMODE_TX, False);   //non-blocking call
            return;
        }
    }

    /// Packet is done
    radio_gag();
    radio_idle();
    radio_finish((rfctl.flags & (RADIO_FLAG_PG | RADIO_FLAG_BG | RADIO_FLAG_CONT)), 0);
}
#endif











/** General Purpose Radio Subroutines
  * ============================================================================
  * - Usually some minor adjustments needed when porting to new platform
  * - See integrated notes for areas sensitive to porting
  */

void wlloradrv_null(ot_int arg1, ot_int arg2) { }


#ifndef EXTF_wlloradrv_mdmconfig
OT_WEAK void wlloradrv_mdmconfig(MODE_enum mode, ot_u8 mdmcfg2_val, ot_u8 symtimeout_def, ot_u16 param) {
	ot_u8 offset;
	ot_u8 usefec;

	static const ot_u8 syncword[6] = {
	    0xD7,0x28,
	    0xEB,0x14,
	    0x8D,0x72
	};
	static const ot_u8 preamblelen[6] = {
        (DRF_LR_PREAMBLELSB-4), (DRF_LR_PREAMBLELSB-4),
        DRF_LR_PREAMBLELSB,     DRF_LR_PREAMBLELSB,
        DRF_LR_PREAMBLELSB,     DRF_LR_PREAMBLELSB,
	};
	static const ot_u8 deflength[6] = {
        6,  16,
        16, 32,
        16, 32
    };
    static const ot_u8 mdmcfg1_lut[2] = {
        (0 | _CODINGRATE_4_5 | _IMPLICITHEADER_ON | _RXPAYLOADCRC_OFF | _LOWDATARATEOPTIMIZE_OFF),
        (0 | _CODINGRATE_4_4 | _IMPLICITHEADER_ON | _RXPAYLOADCRC_OFF | _LOWDATARATEOPTIMIZE_OFF)
    };

    // spibus command buffer
    ot_u8 mdmregs[7] = {
        (0x80 | RFREG_LR_MODEMCONFIG1),     // Write Command Address
        0,                                  // MDMCFG1
        0,                                  // MDMCFG2
        0,                                  // SymTimeout
        0,                                  // Preamble MSB
        0,                                  // Preamble LSB
        0,                                  // Packet Length
    };

    usefec      = (phymac[0].channel >> 7);
    offset      = (ot_u8)mode + usefec;

    // Prepare the modem configuration registers (mdmcfg1, mdmcfg2)
    mdmregs[1]  = wllora_get_bw(phymac[0].flags) | mdmcfg1_lut[usefec];
    mdmregs[2]  = mdmcfg2_sf[(phymac[0].channel>>4)&3] | mdmcfg2_val;

    // Prepare the symbol timeout value (only relevant on RX)
    mdmregs[3]  = symtimeout_def;

    // Prepare the preamble length, which is only done via the LSB
    mdmregs[5]  = preamblelen[offset];

    // Prepare the packet length, which in TX is known and in RX is the size of an initial header chunk
    ///@todo Param manipulation must take into account the variable coding rate
    mdmregs[6]  = (param == 0) ? deflength[offset] : param;

    // Store in a single SPI write operation
    wllora_spibus_io(7, 0, mdmregs);

    // Sync word is too far away to be done in a single write
    wllora_write(RFREG_LR_SYNCWORD, syncword[offset]);

}
#endif


#ifndef EXTF_wlloradrv_save_linkinfo
OT_WEAK void wlloradrv_save_linkinfo(void) {
//    // Link information: only SNR is available, which is saved as LQI
//    radio.link.pqi  = 1;
//    radio.link.sqi  = 1;
//    radio.link.lqi  = wllora_read(RFREG_LR_PKTSNRVALUE);
//    radio.link.agc  = 0;
}
#endif





/** General Purpose, Low-Level Radio Module control    <BR>
  * ========================================================================<BR>
  * Used for data FIFO interaction and power configuration, but not specifically
  * related to Mode 2.  These are implemented in the radio driver.
  */

#ifndef EXTF_radio_off
OT_WEAK void radio_off(void) {
   wllora_reset();
   radio_sleep();
}
#endif

#ifndef EXTF_radio_gag
OT_WEAK void radio_gag(void) {
    wllora_int_off();                       // Disable pin interrutps on MCU
    wllora_write(RFREG_LR_IRQFLAGS, 0xFF);  // Clear Register IRQ Flags on SX127x
}
#endif

#ifndef EXTF_radio_ungag
OT_WEAK void radio_ungag(void) {
    if (radio.state != RADIO_Idle) {
        wllora_int_on();
    }
}
#endif

#ifndef EXTF_radio_sleep
OT_WEAK void radio_sleep(void) {
    wllora_antsw_off();
    wllora_strobe(_OPMODE_SLEEP, True);
    
/// SX127x can go into SLEEP from any other state, so the only optimization 
/// here is to check if it is already in SLEEP before dealing with GPIO & SPI.
//    ot_bool test;
//    test = (rfctl.flags & RADIO_FLAG_XOON);
//    if (test) {
//        radio.state  = RADIO_Idle;
//        rfctl.flags &= ~RADIO_FLAG_PWRMASK;
//        wllora_strobe(_OPMODE_SLEEP);
//        wllora_waitfor_sleep();
//    }
}
#endif

#ifndef EXTF_radio_idle
OT_WEAK void radio_idle(void) {
    wllora_antsw_on();                    ///@todo see if this matters
    wllora_strobe(_OPMODE_STANDBY, True);
    
/// SX127x can go into STANDBY from any other state, so the only optimization 
/// here is to check if it is already in STANDBY before dealing with GPIO & SPI.
//    ot_bool test;
//    test = ((rfctl.flags & RADIO_FLAG_XOON) && (radio.state == RADIO_Idle));
//    if (!test) {
//        radio.state  = RADIO_Idle;
//        rfctl.flags |= RADIO_FLAG_XOON;
//        wlloradrv_force_standby();
//    }
}
#endif

#ifndef EXTF_radio_putbyte
OT_WEAK void radio_putbyte(ot_u8 databyte) {
    wllora_write(RFREG_LR_FIFO, databyte);
}
#endif

#ifndef EXTF_radio_putfourbytes
OT_WEAK void radio_putfourbytes(ot_u8* data) {
/// Unused in SX127x
}
#endif

#ifndef EXTF_radio_getbyte
OT_WEAK ot_u8 radio_getbyte(void) {
    return wllora_read(RFREG_LR_FIFO);
}
#endif

#ifndef EXTF_radio_getfourbytes
OT_WEAK void wllora_getfourbytes(ot_u8* data) {
/// Unused in SX127x
}
#endif

#ifndef EXTF_radio_flush_rx
OT_WEAK void radio_flush_rx(void) {
    wllora_write(RFREG_LR_FIFOADDRPTR, 0);
}
#endif

#ifndef EXTF_radio_flush_tx
OT_WEAK void radio_flush_tx(void) {
    wllora_write(RFREG_LR_FIFOADDRPTR, 0);
}
#endif

#ifndef EXTF_radio_rxopen
OT_WEAK ot_bool radio_rxopen(void) {
    return (ot_bool)wllora_rxbytes();
}
#endif

#ifndef EXTF_radio_rxopen_4
OT_WEAK ot_bool radio_rxopen_4(void) {
    return (ot_bool)(wllora_rxbytes() >= 4);
}
#endif

#ifndef EXTF_radio_txopen
OT_WEAK ot_bool radio_txopen(void) {
    return (ot_bool)(radio.state == RADIO_DataTX);
}
#endif

#ifndef EXTF_radio_txopen_4
OT_WEAK ot_bool radio_txopen_4(void) {
/// Never really needed with SX127x
    return radio_txopen();
}
#endif

#ifndef EXTF_radio_rssi
OT_WEAK ot_int radio_rssi(void) {
    return radio.last_rssi;
    //radio.last_rssi = wllora_calc_rssi(wllora_read(RFREG_LR_PKTRSSIVALUE), wllora_read(RFREG_LR_PKTSNRVALUE));
}
#endif

#ifndef EXTF_radio_check_cca
OT_WEAK ot_bool radio_check_cca(void) {
/// CCA Method: Look if CS pin is high.
    return (wllora_check_cadpin() == 0);
}
#endif

#ifndef EXTF_radio_calibrate
OT_WEAK void radio_calibrate(void) {
/// SX127x does mandatory automatic calibration
}
#endif

#ifndef EXTF_radio_getlinkinfo
OT_WEAK void* radio_getlinkinfo(void) {
/// Returns pointer to link information.
/// Information is specific to HW (in this case SX127x).  User will need to cast appropriately.
#   if (M2_FEATURE_HSCODE)
    return &loralink;
#   else
    return NULL;
#   endif
}
#endif

#ifndef EXTF_radio_getpwrcode
OT_WEAK ot_u8 radio_getpwrcode(void) {
/// Power code: 0-3.  wllora_getbasepwr() typically returns 3 on most platforms
    return wllora_getbasepwr() - (radio.state > RADIO_Idle);
}
#endif

#endif
