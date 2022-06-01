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
  * @todo this file is an incomplete port from SX127x to STM32WL LoRa.
  *       Work is underway.
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
#include <otlib/delay.h>

// Local header for subroutines and implementation constants (supports patching)
#include "radio_rm2.h"

#if (M2_FEATURE_MPCODE)
#   include <hblib/mpcode.h>
#   include <hblib/lorallr.h>
    mpc_t       mpc;
    lorallr_t   lorallr;
    radio_snr_t loralink;
#endif


#define _ENABLE_BGCAD

/// _RFCORE_NOSLEEP can be uncommented to prevent radio driver functions from
/// putting the RF Core into sleep mode(s).  Mainly, this can be useful during
/// driver development.
#define _RFCORE_NOSLEEP

/// _RFCORE_DEBUG can be uncommented in order to activate the CORE DUMP feature.
/// CORE DUMP will read all the SX127x registers and save them to an array.
/// Printing them out is too slow -- the registers can change too fast.
/// Instead, set breakpoints and inspect the rfcore_dump[] array via debugger HW.
#define _RFCORE_DEBUG


#ifdef _RFCORE_DEBUG
#   define __CORE_DUMP()                wllora_coredump_uart1(0x000, 0xA00)
#   define __CORE_DUMP_BLOCK(LO, HI)    wllora_coredump_uart1(LO, HI)
#else
#   define __CORE_DUMP()                do {} while(0)
#   define __CORE_DUMP_BLOCK(LO, HI)    do {} while(0)
#endif

///@todo could this be static?
rfctl_struct rfctl;


/** Test Subroutines  <BR>
  * ========================================================================<BR>

static volatile ot_u16 err_dump;
static volatile ot_u16 irq_dump;
static volatile ot_u8 pktlen_dump;
static volatile ot_u8 status_dump;
static volatile lr_rxbufstatus_t bufstatus_dump;
static void test_rfreg_stepthrough(void) {
    err_dump        = wllora_geterr_cmd();
    irq_dump        = wllora_getirq_cmd();
    pktlen_dump     = wllora_rdreg(LR_RTXPLDLEN);
    status_dump     = wllora_status_cmd();
    bufstatus_dump  = wllora_rxbufstatus_cmd();
}
  */


/** Local Subroutine Prototypes  <BR>
  * ========================================================================<BR>
  */
void sub_hwtimeout_isr(void);
void sub_initrx(void);
void sub_initcad(void);

void wlloradrv_mdmconfig(MODE_enum mode, ot_u16 param);
void wlloradrv_save_linkinfo(void);



/** Virtual ISR RF  <BR>
  * ========================================================================<BR>
  */

#define RFIV_LISTEN     0
#define RFIV_RXDONE     1
#define RFIV_RXTIMEOUT  2
#define RFIV_RXHEADER   4
#define RFIV_CCA        5
#define RFIV_TXDONE     6



void wllora_virtual_isr(ot_u16 irq_mask) {
    switch (wllora.imode) {
        // Listen Mode: go into RX if CAD detected, else failure
        case MODE_Listen:
            if (irq_mask & LR_IRQ_CADDET) {
                rfctl.flags |= RADIO_FLAG_CADFOUND;
                rm2_reenter_rx(radio.evtdone);
                return;
            }
            goto KILL_ON_ERROR;

        // Available IRQs are HDRVALID and RXDONE
        case MODE_RXData:
            if (irq_mask & LR_IRQ_RXDONE) {
                // frame received
                rm2_rxend_isr();
                return;
            }
            if (irq_mask & LR_IRQ_HDRVALID) {
                // sync
                rm2_rxsync_isr();
                return;
            }
            goto KILL_ON_ERROR;

        // Set the CAD flag depending on result.
        case MODE_CSMA:
            if (irq_mask & LR_IRQ_CADDET) {
                rfctl.flags |= RADIO_FLAG_CADFOUND;
            }
            else {
                rfctl.flags &= ~RADIO_FLAG_CADFOUND;
            }
            rm2_txcsma_isr();
            return;

        case MODE_TXData:
            if (irq_mask & LR_IRQ_TXDONE) {
                rm2_txdata_isr();
                return;
            }
            goto KILL_ON_ERROR;

        // Errors
        default:
#           ifdef __DEBUG__
            __BKPT();
#           endif
            break;
    }

    KILL_ON_ERROR:
    rm2_kill();
}



#ifndef EXTF_radio_mac_isr
OT_WEAK void radio_mac_isr(void) {

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
    	chan_rssi = wllora_rssi_cmd();
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
    /// Set device-dependent init defaults (rfctl is device-dependent)
    rfctl.flags     = 0;

    /// Set universal Radio module init defaults (radio is device-independent)
    ///@note The high-level radio.state doesn't differentiate between Standby
    ///      and Sleep modes.  Both considered Idle.
    radio.state     = RADIO_Idle;
    radio.evtdone   = &otutils_sig2_null;

    /// Initialize the bus between WLLora and MCU, and load defaults.
    /// After wllora_init_bus(), WLLora will be in Standby/Idle mode
    wllora_init_bus();
    wllora_load_defaults();

    /// For Deep Debugging of RF Core, or reverse engineering purposes.
    /// This dump takes a while over the UART, because there are ~2500
    /// registers in the SX126x core.
    //__CORE_DUMP();

    /// After wllora_load_defaults(), RF core can be put to sleep
    radio_sleep();

    /// Initialize RM2 elements such as channels, link-params, etc.
    rm2_init();
}
#endif



#ifndef EXTF_radio_finish
OT_WEAK void radio_finish(ot_int main_err, ot_int frame_err) {
/// Reset radio & callback to null state, then run saved callback
    ot_sig2 callback;
    
    // Gagging and Unblocking are redundant when RF Core is behaving per
    // specification.  If it is not (can happen), these are necessary.
    radio_gag();
    dll_unblock();

    rfctl.state     = 0;

    ///@todo rfctl.flags is changed in port from sx1276.  Make sure the
    ///      zero-ing here is the right change.
    //rfctl.flags    &= (RADIO_FLAG_XOON);    // clear all other flags
    rfctl.flags     = 0;

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



/** @note On rm2_enter_channel() and sub_set_channel():
  *       SX126x / STM32WL cannot do register operations while in sleep.
  *       rm2_enter_channel() just buffers the channel entry parameters, and
  *       sub_set_channel() will apply them.  At the task level,
  *       rm2_enter_channel() should be used, and within the driver code (i.e.
  *       in this C file) sub_set_channel() before RX or TX.
  */

#ifndef EXTF_rm2_enter_channel
OT_WEAK void rm2_enter_channel(ot_u8 old_chan_id, ot_u8 old_tx_eirp) {
    wllora_ext.old_chan_id = old_chan_id;
    wllora_ext.old_tx_eirp = old_tx_eirp;
}
#endif

void sub_set_channel(ot_u8 old_chan_id, ot_u8 old_tx_eirp) {
///@todo when additional information is known about setting up all types of
///      sensitivity parameters, that configuration will go in this function.

    static const ot_u8 cadparams_lut[8][4] = {
    //      symbs | det peak  | det min |   symbol timeout
        {   3,      0x18,       0x10,       11  },   // sf5, no fec
        {   2,      0x18,       0x04,       4   },   // sf5, mpc
        {   3,      0x20,       0x10,       9   },   // sf7, no fec
        {   1,      0x20,       0x04,       3   },   // sf7, mpc
        {   2,      0x22,       0x10,       7   },   // sf9, no fec
        {   0,      0x22,       0x04,       1   },   // sf9, mpc
        {   2,      0x24,       0x10,       5   },   // sf11, no fec
        {   0,      0x24,       0x04,       0   },   // sf11, mpc
    };
    static const ot_u8 sf_lut[4] = {
        5, 7, 9, 11
    };

    /// PA reprogram
    if (old_tx_eirp != phymac[0].tx_eirp) {
        wllora_set_txpwr( phymac[0].tx_eirp );
    }

    // old_chan_id becomes an XOR: 0 bits mean no change.
    old_chan_id ^= phymac[0].channel;

    /// Configure Threshold Settings based on spreading factor and encoding.
    // 1. Sensitivity Parameters via Cad Params
    // 2. symbol timeout
    // 3. SF per channel code, BW = 500 kHz, CR = 4/4 or 4/5
    if (old_chan_id & 0xF0) {
        ot_u8 sf_index      = (phymac[0].channel >> 4) & 3;
        ot_u8 usefec        = (phymac[0].channel >> 7);
        const ot_u8* lut    = cadparams_lut[(sf_index << 1) + usefec];

        wllora_cadparams_cmd(lut[0], lut[1], lut[2], 0, 0);
        wllora_symtimeout_cmd(lut[3]);
        wllora_modparams_cmd(sf_lut[sf_index], 0x06, 1-usefec, 0);

        ///@todo Figure out if there is a pattern to the values in these
        /// registers, and set them directly rather than Read-Modify-Write.

        ///@todo need to monitor if these registers change at all, or stay the
        /// same once set.

        ///@note this is a workaround per Semtech SX1262 datasheet.  Must have
        /// bit2 set to 0 in register 0x0889 when Bandwidth is 500 kHz, and 1
        /// when Bandwidth is less than 500 kHz.
        {   ot_u8 txmod_val;
            txmod_val = wllora_rdreg(LR_TXMOD);
            wllora_wrreg(LR_TXMOD, txmod_val & ~(1<<2) );
        }

        ///@note this is a workaround per Semtech SX1262 datasheet.  Must have
        /// bit2 set to 0 in register 0x0736 when IQ is inverted, and set to 1
        /// when IQ is Normal.
        {   ot_u8 iqpol_val;
            iqpol_val = wllora_rdreg(LR_IQPOLARITY);
            wllora_wrreg(LR_IQPOLARITY, iqpol_val | (1<<2) );
        }
    }

    /// Configure Channel Center Frequency
    if (old_chan_id & 0x0F) {
        wllora_configure_chan(phymac[0].flags, (phymac[0].channel & 0x0F)-1);
    }
}







#ifndef EXTF_rm2_mac_configure
OT_WEAK void rm2_mac_configure(void) {
/// Only use this when there is a hardware MAC filtering ability.  
/// STM32WL RF Core does not have this capability.
}
#endif


#ifndef EXTF_rm2_calc_link
OT_WEAK void rm2_calc_link(void) {
	lr_pktlink_t linkinfo;
	linkinfo = wllora_pktlink_cmd();

#   if (M2_FEATURE_MPCODE)
	loralink.mean_snr   = (ot_s16)linkinfo.snr_pkt;
#   endif

    radio.last_rssi     = -(linkinfo.signal_rssi_pkt / 2);
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
    wllora_rfirq_txdata();
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

    /// For Deep Debugging of RF Core, or reverse engineering purposes.
    /// This dump takes a while over the UART, because there are ~2500
    /// registers in the SX126x core.
    //__CORE_DUMP();

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
            wlloradrv_save_linkinfo();
            radio_finish(RM2_ERR_TIMEOUT, 1 /*radio.link.sqi*/);
            return;
        }
    }
    
    /// Loop RX-Single listening mode in BG-retry or FG-RX.
    /// In FG-RX, DLL will timeout the RX process manually.
    //rm2_reenter_rx(radio.evtdone);

    ///@todo is wllora_clrirq_cmd() needed here?
    wllora_clrirq_cmd();

    wllora_set_state(RFSTATE_rx, False);
}

#ifndef EXTF_rm2_rxtimeout_isr
OT_WEAK void rm2_rxtimeout_isr(void) {
    __DEBUG_ERRCODE_EVAL(=280);

    /// Can't have an interrupt during termination/re-configuration
    radio_gag();

    /// Send timeout error back to DLL
    wlloradrv_save_linkinfo();
    radio_finish(RM2_ERR_TIMEOUT, 1 /*radio.link.sqi*/);
}
#endif





/** Radio RX Functions
  * ============================================================================
  */

// Timer here runs at 4096 Hz
// First group of numbers is for CR1
// Second group of numbers is for CR0
//static const ot_u8 iterinit[4] = {
////	84, 21, 6, 6
//    68, 17, 5, 5
//};

#ifndef EXTF_rm2_rxinit
OT_WEAK void rm2_rxinit(ot_u8 channel, ot_u8 psettings, ot_sig2 callback) {
    //ot_u8     netstate;
    MODE_enum   mode;
    ot_bool     cad_else_rx;

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
            rfctl.flags|= RADIO_FLAG_PG;
        	mode        = MODE_pg;
        	cad_else_rx = False;
        }
        // BG RX Initialization: optional until CAD is perfected
#       ifdef _ENABLE_BGCAD
        else if (psettings & M2_NETSTATE_INIT) {
            rfctl.flags|= RADIO_FLAG_BG;
            rfctl.tries = 3;
		    mode 	    = MODE_bg;
		    cad_else_rx = True;
		}
#       endif
        // BG RX Retry: used as main BG RX until CAD is perfected
		else {
		    rfctl.flags|= RADIO_FLAG_BG;
            mode        = MODE_bg;
            cad_else_rx = False;
		}
    }
    else {
      //netstate    = psettings;
        mode        = MODE_fg;
        cad_else_rx = False;
    }
    
    /// 1. Fetch the RX channel, exit if the specified channel is not available
    if (rm2_test_channel(channel) == False) {
        radio_finish(RM2_ERR_BADCHANNEL, 0);
        return;
    }

    /// Need to go into Idle ahead of any register access
    radio_idle();

    // Set Modem Configuration: BW, SF, sensitivity params, packet config.
    // Additionally, this function refers to phymac[] struct.
    wlloradrv_mdmconfig(mode, 0);
    
    ///@note the iteration ticks corresponds to the duration of one interleaver block
    ///      of LoRa symbols.  "Ticks" in this case is implementation-specific to this driver
    ///      and not [necessarily] DASH7 standard ticks.  This impl has iter_ti = 0.25 Ticks.

    // Set the rx iteration period
    rfctl.iter_ti = (ot_u8)(wllora_block_miti(&phymac[0]) >> 8) + 1;

    // Apply the cached channel parameters to the Radio Core
    sub_set_channel(wllora_ext.old_chan_id, wllora_ext.old_tx_eirp);

    /// 3.  Use CAD or RX initialization subroutine
    if (cad_else_rx) {
        // Set I/O for CAD detect
        wllora_rfio_cad();
        wllora_rfirq_listen();
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
        wllora_set_state(RFSTATE_cad, False);
    }
    else {
        //radio_activate_queue(&rxq);
        wllora_rfio_rx();

        ///@todo why is this commented-out?  Does it matter?
        //dll_offset_rxtimeout();

        rm2_reenter_rx(radio.evtdone);
    }
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
    sub_set_channel((channel & 0x7f), (phymac[0].tx_eirp & 0x7f));
    
    // Configure radio for Foreground reception (nominal)
    wlloradrv_mdmconfig(MODE_fg, 0);
    
    // Set MAC timer as termination timer
    radio_set_mactimer(timeout << 2);
}
#endif


//static volatile ot_u8 test_state;
#ifndef EXTF_rm2_reenter_rx
OT_WEAK void rm2_reenter_rx(ot_sig2 callback) {
/// Restart RX using the same settings that are presently in the radio core.
///@note the radio must be in standby or RX going into this function
///
    radio.evtdone   = callback;
    rfctl.state     = RADIO_STATE_RXAUTO;
    
    ///@todo might need to have an idle entry at the top of this function
    //radio_idle();

    // Reset interrupts and FIFO for fresh RX'ing
    radio_gag();                            
    radio_flush_rx();
    
    /// For Deep Debugging of RF Core, or reverse engineering purposes.
    /// This dump takes a while over the UART, because there are ~2500
    /// registers in the SX126x core.
    //__CORE_DUMP();
    
    // testing only
//    {
//        test_state = wllora_status_cmd();
//    }

    wllora_rfirq_rxdata();
    wllora_antsw_rx();
    wllora_set_state(RFSTATE_rx, False);
    
    // testing only
//    {
//        delay_ms(10);
//        test_state = wllora_status_cmd();
//    }

    // Packet Sync must be reset at this point, so another sync can be found.
    // radio_gag() above will reset all the IRQ flags.
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
    wllora_rfirq_rxend();

    em2_decode_newpacket();
    em2_decode_newframe();
}
#endif


#ifndef EXTF_rm2_rxdata_isr
OT_WEAK void rm2_rxdata_isr() {

    /// 1. Decode Available Data.
    ///    em2_decode_data() must handle the header flags in FG frames (NEWHEADER, crc5)
    em2_decode_data();

    /// 2. Case 1:
    /// A-Priori Fixed Length: BG and PG.
    /// Don't need to re-set em2.bytes or meddle with the radio buffer settings.
    // (Nothing to do here)

    /// 3. Case 2:
    /// FG frames that have dynamic length and an explicit header with frame
    /// length.  We only need to specially handle the header case.
    /// em2_decode_data() must set NEWHEADER flag and crc5 element appropriately.

    // Failure case on em2.state < 0 (decoding irregularity)
    if (em2.state < 0) {
        systim_disable_insertion();
        radio_gag();
        radio_idle();
        rm2_reenter_rx(radio.evtdone);
    }
    // decoder has successfully unpacked the header
    else if ((em2.crc5 == 1) && ((rfctl.flags & 7) == 0)) {
        ot_uint rx_octets;

#		if (M2_FEATURE(MPCODE))
        if (phymac[0].channel & 0x80) {
            rx_octets   = (ot_uint)mpc_octetsinframe((mpcrate_t)(rxq.front[1]&7), rxq.front[0]-7);
            rx_octets  += 16;
        }
        else
#       endif
        {
            rx_octets = rxq.front[0]+1;
            systim_disable_insertion();
        }

        // This driver implementation updates the RX payload length, live.
        wllora_wrreg(LR_RTXPLDLEN, (ot_u8)rx_octets);
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
#if (M2_FEATURE_MPCODE == ENABLED)
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
        blockcrc = mpc_decode(&mpc, rxq.putcursor, llrbits);
        if (blockcrc != 0) {
            rxq.options.ushort  = 0;
            em2.state           = -1;
            goto rm2_decode_s2_WAITNEXT;
        }

        // Following a successful decode operation, need to pop the llr block
        lorallr_popblock(&lorallr);

        // The putcursor is advanced by the number of decoded bytes,
        // and the mpc object is reinitialized to the explicit block size for the next block
        rxq.putcursor += mpc.infobytes;
        mpc_init(&mpc, (mpcrate_t)(rxq.front[1] & 7));

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
    sys.task_RFA.nextevent  = (ot_uint)wllora_mpcblock_ti(&phymac[0]);
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
OT_WEAK void rm2_txtest(ot_u8 channel, ot_u8 eirp_code, ot_u8 tsettings, ot_u16 timeout) {
    ot_u8 old_channel;
    ot_u8 old_eirp;

    // This seemingly circular operation is a hack to force a channel to be
    // entered even if it is unsupported.  We don't care if the channel is
    // supported, because no data is going to be sent.  Just set the center
    // frequency to where it is desired
    old_channel         = phymac[0].channel;
    old_eirp            = phymac[0].tx_eirp;
    phymac[0].channel   = channel;
    phymac[0].tx_eirp   = eirp_code;
    sub_set_channel(old_channel, old_eirp);

    // No callback
    radio.evtdone = &otutils_sig2_null;

    // Need to put into Standby in order to operate on the registers
    wllora_set_state(RFSTATE_RCstandby, True);

    wllora_mcuirq_off();
    wllora_rfio_tx();
    radio_flush_tx();

    /// For Deep Debugging of RF Core, or reverse engineering purposes.
    /// This dump takes a while over the UART, because there are ~2500
    /// registers in the SX126x core.
    __CORE_DUMP();
    
    radio.state = RADIO_DataTX;
    wllora_antsw_tx();

    // Go into TX-Cont mode.  This is a special operation.
    wllora.state = RFSTATE_tx;
    //wllora_txcontwave_cmd();
    wllora_txcontpreamble_cmd();
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
    wllora_rfio_cad();
    wllora_rfirq_cad();
    wllora_antsw_rx();
    wllora_set_state(RFSTATE_cad, False);
    radio.state = RADIO_Csma;
}

ot_bool sub_cca_test(void) {
    if ((rfctl.flags & RADIO_FLAG_CADFOUND) == 0) {
        rfctl.state = RADIO_STATE_TXCAD1;
        
        // This calls CSMA loop
        radio.evtdone(1, 0);

        return False;
    }
    return True;
}

OT_WEAK void rm2_txcsma_isr(void) {
    ///@todo this is a hack to get paging features exposed
    static const ot_u8 mode_lut[8] = { 
        MODE_fg, MODE_bg, MODE_fg, MODE_bg, MODE_pg, MODE_pg, MODE_pg, MODE_pg };

    /// When this function is entered, the Radio State is not known, but it may
    /// be in SLEEP.  We need to put it in Standby in order to change the
    /// registers, and we use HSE standby to minimize switching time among
    /// active modes (Standby -> CAD -> Standby -> CAD -> Standby -> TX)
    {   WLLora_State next_state;
        next_state = (wllora.state > RFSTATE_RCstandby) ?
                RFSTATE_HSEstandby : RFSTATE_RCstandby;
        wllora_set_state(next_state, True);
    }
    radio_gag();

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
            WLLora_State next_state;

            // Find a usable channel from the TX channel list.  If none, error.
            if (rm2_test_chanlist() == False) {
                radio.evtdone(RM2_ERR_BADCHANNEL, 0);
                break;
            }
            
            // Get Queue and encoder ready for transmission.  We do this here
            // in order to set em2.bytes properly, which becomes the payload
            // length on LoRa.  
            radio_activate_queue(&txq);
            em2_encode_newpacket();
            em2_encode_newframe();

            // Expected entry State is Sleep, here.
            //next_state = (wllora.state <= RFSTATE_RCstandby) ?
            //                RFSTATE_RCstandby : RFSTATE_HSEstandby;
            //wllora_set_state(next_state, True);

            // Apply the cached channel parameters to the Radio Core.
            // rm2_test_chanlist() will set-up the channel into wllora_ext cache.
            sub_set_channel(wllora_ext.old_chan_id, wllora_ext.old_tx_eirp);

            // Set other TX Buffering & Packet parameters, and also save the
            // Peristent-TX attribute for floods, which is written later
            ///@note this LUT supports BG, FG, and PG
            type = mode_lut[rfctl.flags & 7];
            wlloradrv_mdmconfig(type, em2.bytes);
            
            // Bypass CSMA if MAC is disabling it
            ///@todo CSMA/CCA is disabled for now while Radio is being developed
            //if (dll.comm.csmaca_params & M2_CSMACA_NOCSMA) {
                goto rm2_txcsma_START;
            //}
            
            // Enter CSMA via fallthrough.  
            // There are two incremental tests: CCA1 and CCA2.
        } 

        // 2-3. First CCA: If it is valid, set MAC timer to invoke CAD2.
        // If invalid, return to CAD1 via sub_cca_test().
        case (RADIO_STATE_TXCAD1 >> RADIO_STATE_TXSHIFT):
            rfctl.state = RADIO_STATE_TXCCA1;
            sub_cad_csma();
            break;
            
        case (RADIO_STATE_TXCCA1 >> RADIO_STATE_TXSHIFT):
            if (sub_cca_test()) {
                radio_sleep();
                rfctl.state = RADIO_STATE_TXCAD2;
                ///@todo Make sure this interrupt is working, there had been some past issues with it.
                radio_set_mactimer(phymac[0].tg);
            }
            break;
        
        // 4-5. 2nd CCA: If it is valid, fall through to TX START
        // If invalid, return to CAD1 via sub_caa_isfail().
        case (RADIO_STATE_TXCAD2 >> RADIO_STATE_TXSHIFT):
            rfctl.state = RADIO_STATE_TXCCA2;
            sub_cad_csma();
            break;

        case (RADIO_STATE_TXCCA2 >> RADIO_STATE_TXSHIFT): 
            if (sub_cca_test() == False) {
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
/// given that the code below can be too long to put in an ISR.  Ideally there is
/// a better way to handle this via the radio.evtdone() callback.
            radio.evtdone(0, (rfctl.flags & (RADIO_FLAG_PG | RADIO_FLAG_CONT | RADIO_FLAG_BG)));
            
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
            wllora_rfio_tx();
            wllora_rfirq_txdata();

            /// For Deep Debugging of RF Core, or reverse engineering purposes.
            /// This dump takes a while over the UART, because there are ~2500
            /// registers in the SX126x core.
            //__CORE_DUMP();

            wllora_antsw_tx();
            wllora_set_state(RFSTATE_tx, False);
            break;
        }
    }
}
#endif


#ifndef EXTF_rm2_txdata_isr
OT_WEAK void rm2_txdata_isr(void) {
/// Continues where rm2_txcsma() leaves off.
/// This function assumes the radio state is in standby or TX.  It will finish
/// paging out the frame and handle the frame-done event.

    // Packet BG flooding.  Only needed on devices that can send M2AdvP
    // The radio.evtdone callback here should update the AdvP payload
    if ((rfctl.flags & RADIO_FLAG_BGFLOOD) == RADIO_FLAG_BGFLOOD) {
        radio.evtdone(RADIO_FLAG_CONT, 0);

        if ((rfctl.state & RADIO_STATE_TXMASK) == RADIO_STATE_TXDATA) {
            bgcrc8_put(&txq.front[2]);
            wllora_clrirq_cmd();
            wllora_wrbuf_cmd(0, 6, &txq.front[2]);
            
///@todo implement this synchronization stub function
//rm2_flood_txwait();
            
            ///@todo make sure wllora_antsw_tx() doesn't need to be here
            //wllora_antsw_tx();
            wllora_set_state(RFSTATE_tx, False);
            return;
        }
    }

    // Packet is done with TX
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
OT_WEAK void wlloradrv_mdmconfig(MODE_enum mode, ot_u16 param) {
/// wlloradrv_mdmconfig() must configure the following attributes:
/// - syncword
/// - preamble length
/// - packet attrs: implicit mode, CRC off, ldro off, frame length
/// - modulation attrs: spreading factor, bandwidth,
/// - sensitivity parameters: symbol sense, det peak, det min
	ot_u8 payload_len;
	ot_u8 lut_index;

	static const ot_u8 syncword[6][2] = {
	    { 0x14, 0xD7 },
	    { 0x14, 0x28 },
	    { 0x14, 0xEB },
	    { 0x14, 0x14 },
	    { 0x14, 0x8D },
	    { 0x14, 0x72 }
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

    // Preamble length, initial payload length
    lut_index   = (ot_u8)mode + (phymac[0].channel >> 7);
    payload_len = (param == 0) ? deflength[lut_index] : param;
    wllora_pktparams_cmd((ot_u16)preamblelen[lut_index], 1, payload_len, 0, 0);

    // Sync Word
    wllora_wrburst(LR_LSYNCRH, 2, syncword[lut_index]);
}
#endif


#ifndef EXTF_wlloradrv_save_linkinfo
OT_WEAK void wlloradrv_save_linkinfo(void) {
#   if (OT_FEATURE(RF_LINKINFO) || OT_FEATURE(RF_ADAPTIVE))
    lr_pktlink_t linkinfo;
    linkinfo = wllora_pktlink_cmd();

    // Link information: only SNR is available, which is saved as LQI
    radio.link.pqi  = 1;
    radio.link.sqi  = 1;
    radio.link.lqi  = linkinfo.snr_pkt;
    radio.link.agc  = 0;
    #endif
}
#endif





/** General Purpose, Low-Level Radio Module control    <BR>
  * ========================================================================<BR>
  * Used for data FIFO interaction and power configuration, but not specifically
  * related to Mode 2.  These are implemented in the radio driver.
  */

#ifndef EXTF_radio_off
OT_WEAK void radio_off(void) {
   // Put into cold sleep
   wllora_antsw_off();
   wllora_set_state(RFSTATE_coldsleep, True);
   radio.state = RADIO_Idle;
}
#endif

#ifndef EXTF_radio_gag
OT_WEAK void radio_gag(void) {
    // Disable pin interrutps on MCU
    wllora_mcuirq_off();
    wllora_clrirq_cmd();
}
#endif

#ifndef EXTF_radio_ungag
OT_WEAK void radio_ungag(void) {
    if (radio.state != RADIO_Idle) {
        wllora_mcuirq_on();
    }
}
#endif

#ifndef EXTF_radio_sleep
OT_WEAK void radio_sleep(void) {
    wllora_antsw_off();
#   ifdef _RFCORE_NOSLEEP
    wllora_set_state(RFSTATE_RCstandby, True);
#   else
    wllora_set_state(RFSTATE_warmsleep, True);
#   endif
    radio.state = RADIO_Idle;
}
#endif

#ifndef EXTF_radio_idle
OT_WEAK void radio_idle(void) {
    ///@todo Does wllora_antsw_on() need to be here?
    wllora_antsw_on();
    wllora_set_state(RFSTATE_RCstandby, True);
    radio.state = RADIO_Idle;
}
#endif

#ifndef EXTF_radio_putbyte
OT_WEAK void radio_putbyte(ot_u8 databyte) {
///@note not supported with SX126x or STM32WL
    __BKPT();
}
#endif

#ifndef EXTF_radio_putfourbytes
OT_WEAK void radio_putfourbytes(ot_u8* data) {
///@note not supported with SX126x or STM32WL
    __BKPT();
}
#endif

#ifndef EXTF_radio_getbyte
OT_WEAK ot_u8 radio_getbyte(void) {
///@note not supported with SX126x or STM32WL
    __BKPT();
    return 0;
}
#endif

#ifndef EXTF_radio_getfourbytes
OT_WEAK void wllora_getfourbytes(ot_u8* data) {
///@note not supported with SX126x or STM32WL
    __BKPT();
}
#endif

#ifndef EXTF_radio_flush_rx
OT_WEAK void radio_flush_rx(void) {
    wllora_bufbase_cmd(0, 0);
}
#endif

#ifndef EXTF_radio_flush_tx
OT_WEAK void radio_flush_tx(void) {
    wllora_bufbase_cmd(0, 0);
}
#endif

#ifndef EXTF_radio_rxopen
OT_WEAK ot_bool radio_rxopen(void) {
///@note not supported with SX126x or STM32WL
    __BKPT();
    return False;
}
#endif

#ifndef EXTF_radio_rxopen_4
OT_WEAK ot_bool radio_rxopen_4(void) {
///@note not supported with SX126x or STM32WL
    __BKPT();
    return False;
}
#endif

#ifndef EXTF_radio_txopen
OT_WEAK ot_bool radio_txopen(void) {
    return (ot_bool)(radio.state == RADIO_DataTX);
}
#endif

#ifndef EXTF_radio_txopen_4
OT_WEAK ot_bool radio_txopen_4(void) {
/// Never really needed with SX126x or STM32WL
    return radio_txopen();
}
#endif

#ifndef EXTF_radio_rssi
OT_WEAK ot_int radio_rssi(void) {
    return radio.last_rssi;
}
#endif

#ifndef EXTF_radio_check_cca
OT_WEAK ot_bool radio_check_cca(void) {
/// CCA Method: return true if there's no CAD
    return ((rfctl.flags & RADIO_FLAG_CADFOUND) == 0);
}
#endif

#ifndef EXTF_radio_calibrate
OT_WEAK ot_u16 radio_calibrate(ot_bool blocking) {
/// Runs the calibration non-blocking, returns number of ticks until expected
/// done.  Should be called from time to time by the kernel.
    return wllora_set_state(RFSTATE_calibration, blocking);
}
#endif

#ifndef EXTF_radio_getlinkinfo
OT_WEAK void* radio_getlinkinfo(void) {
/// Returns pointer to link information.
/// Information is specific to HW.  User will need to cast appropriately.
#   if (M2_FEATURE_MPCODE)
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
