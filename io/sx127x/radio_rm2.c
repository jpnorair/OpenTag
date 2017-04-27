/* Copyright 2010-2016 JP Norair
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
  * @file       /otradio/sx127x/radio_rm2.c
  * @author     JP Norair
  * @version    R103
  * @date       23 Nov 2016
  * @brief      Mode 2 Radio Layer Implementation for SX127x
  * @ingroup    Radio
  *
  * <PRE>
  * "   Since you came here uninvited
  *     We all knew you'd be delighted
  *     This is not the time or place to hedge
  *     No one here would be so bold to
  *     But since you asked and no one's told you
  *     Let us take you to the cutting edge     "
  *
  *     -- "We are the Cutting Edge"  (at least in Semtech's mind)
  * </PRE>
  *
  * <LI> The main header file for this is: /include/m2/radio.h      </LI>
  * <LI> Other headers for SX127x are in: /include/io/sx127x/     </LI>
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
  * that we are using a strong polynomial, as it's not apparent that LoRa HW
  * isn't using the horribly weak CCITT poly.
  ******************************************************************************
  */

#include <otstd.h>
#if (OT_FEATURE(M2) == ENABLED)

#include <otplatform.h>
#include <io/sx127x/config.h>
#include <io/sx127x/interface.h>

#include <m2/bgcrc8.h>
#include <m2/encode.h>

#include <otlib/buffers.h>
#include <otlib/crc16.h>
#include <otlib/utils.h>

// Local header for subroutines and implementation constants (supports patching)
#include "radio_rm2.h"





//#define RFCORE_DEBUG
//ot_u8 dump[256];

#ifdef RFCORE_DEBUG
#   define __CORE_DUMP()   spirit1_coredump()
#else
#   define __CORE_DUMP();
#endif


// For doing some quick and dirty metrics on CAD noise
//volatile ot_uint cad_attempts = 0;
//volatile ot_uint cad_misses = 0;



/** Local Data declaration
  * Described in radio_SX127x.h
  */
rfctl_struct rfctl;




/** Local Subroutine Prototypes  <BR>
  * ========================================================================<BR>
  */
void sub_hwtimeout_isr();
void sub_initrx(void);
void sub_initcad(void);


/** SPIRIT1 Virtual ISR RF  <BR>
  * ========================================================================<BR>
  */
  
#define RFIV_LISTEN     0
#define RFIV_RXDONE     1
#define RFIV_RXTIMEOUT  2
#define RFIV_RXHEADER   4
#define RFIV_CCA        5
#define RFIV_TXDONE     6

void sx127x_virtual_isr(ot_u8 code) {
    
    switch (code) {
        // CAD-Done interrupt.  
        // -> BG-RX: check cadpin and go to RX entry or Kill
        case RFIV_LISTEN: {
            ot_uint test = sx127x_check_cadpin();
            if (test)   sub_initrx();
            else        rm2_kill();  
        } break;
        
        // RX BG/FG Listening and data download
        case RFIV_RXDONE:       rm2_rxend_isr();        break;
        case RFIV_RXTIMEOUT:    sub_hwtimeout_isr();    break; 
        case RFIV_RXHEADER:     rm2_rxsync_isr();       break;
        
        // TX CSMA CAD variant
        case RFIV_CCA:          rm2_txcsma_isr();       break;
        
        // TX BG/FG Data Upload
        case RFIV_TXDONE:       rm2_txdata_isr();       break;
        
        default:                rm2_kill();             break;
    }
}



/** Mode 2 - Generic Radio Module control    <BR>
  * ========================================================================<BR>
  * By "generic" it means that it is not Mode2 specific
  */

#ifndef EXTF_radio_init
OT_WEAK void radio_init( ) {
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
    sx127x_init_bus();
    sx127x_load_defaults();

    /// Done with the radio init: sx127x_load_defaults() requires that the 
    /// radio is in SLEEP already, so explicit sleep call is commented here.
    //radio_sleep();

    /// Initialize RM2 elements such as channels, link-params, etc.
    rm2_init();
}
#endif



#ifndef EXTF_radio_finish
OT_WEAK void radio_finish(ot_int main_err, ot_int frame_err) {
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


#ifndef EXTF_radio_mac_isr
OT_WEAK void radio_mac_isr() {
/// Used as CA insertion timer.
    systim_disable_insertion();
    
    if (radio.state == RADIO_Csma) {
        rm2_txcsma_isr();
    }
    
    // Just in case this ISR gets triggered strangely
    else { 
        rm2_kill();
    }
}
#endif



#ifndef EXTF_radio_activate_queue
OT_WEAK void radio_activate_queue(ot_queue* q) {
/// Put some special data in the queue options field.
/// Lower byte is encoding options (i.e. FEC)
/// Upper byte is processing options (i.e. CRC)
    q->options.ubyte[UPPER]    += 1;
    q->options.ubyte[LOWER]     = 0;
    //q->options.ubyte[LOWER]     = (phymac[0].channel & 0x80);
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
    return sx127x_clip_txeirp(m2_txeirp & 0x7f);
}

OT_INLINE ot_u8 rm2_calc_rssithr(ot_u8 m2_rssithr) {
    return sx127x_calc_rssithr(m2_rssithr);
}


#ifndef DRF_MOD1_SS
#   define DRF_MOD1_SS      DRF_MOD1_LS
#   define DRF_MOD0_SS      DRF_MOD0_LS
#   define DRF_CHFLT_SS     DRF_CHFLT_LS
#endif

#ifndef EXTF_rm2_enter_channel
OT_WEAK void rm2_enter_channel(ot_u8 old_chan_id, ot_u8 old_tx_eirp) {
/// LoRa channel changing requires changing the base frequency, as there is no 
/// channel register for doing adjustments to the base frequency.  DASH7 over
/// LoRa uses the following channel rules:
///
/// 433 MHz band: 15x 116 kHz Subchannels from 433.05 - 434.79 MHz
/// 866 MHz band: 15x 250 kHz Subchannels from 865 - 869.250 MHz
/// 915 MHz band: 15x 1500 kHz Subchannels from 902 - 924.5 MHz

#   define BASE_432M992Hz       0x6C3F81
#   define BASE_864M875Hz       0xD83800
#   define BASE_901M250Hz       0xE15000
#   define SPACE_116kHz         0x76C
#   define SPACE_250kHz         0x1000
#   define SPACE_1500kHz        0x6000
#   define __FREQ24(FC)         (0x80 | RFREG_LR_FRFMSB), (((FC)>>16)&0xFF), (((FC)>>8)&0xff), (((FC)>>0)&0xff)

    static const ot_u8 bandplan[] = {
        //433: 0-59
        __FREQ24(BASE_432M992Hz+(1*SPACE_116kHz)),
        __FREQ24(BASE_432M992Hz+(2*SPACE_116kHz)),
        __FREQ24(BASE_432M992Hz+(3*SPACE_116kHz)),
        __FREQ24(BASE_432M992Hz+(4*SPACE_116kHz)),
        __FREQ24(BASE_432M992Hz+(5*SPACE_116kHz)),
        __FREQ24(BASE_432M992Hz+(6*SPACE_116kHz)),
        __FREQ24(BASE_432M992Hz+(7*SPACE_116kHz)),
        __FREQ24(BASE_432M992Hz+(8*SPACE_116kHz)),
        __FREQ24(BASE_432M992Hz+(9*SPACE_116kHz)),
        __FREQ24(BASE_432M992Hz+(10*SPACE_116kHz)),
        __FREQ24(BASE_432M992Hz+(11*SPACE_116kHz)),
        __FREQ24(BASE_432M992Hz+(12*SPACE_116kHz)),
        __FREQ24(BASE_432M992Hz+(13*SPACE_116kHz)),
        __FREQ24(BASE_432M992Hz+(14*SPACE_116kHz)),
        __FREQ24(BASE_432M992Hz+(15*SPACE_116kHz)),
        //866: 60-119
        __FREQ24(BASE_864M875Hz+(1*SPACE_250kHz)),
        __FREQ24(BASE_864M875Hz+(2*SPACE_250kHz)),
        __FREQ24(BASE_864M875Hz+(3*SPACE_250kHz)),
        __FREQ24(BASE_864M875Hz+(4*SPACE_250kHz)),
        __FREQ24(BASE_864M875Hz+(5*SPACE_250kHz)),
        __FREQ24(BASE_864M875Hz+(6*SPACE_250kHz)),
        __FREQ24(BASE_864M875Hz+(7*SPACE_250kHz)),
        __FREQ24(BASE_864M875Hz+(8*SPACE_250kHz)),
        __FREQ24(BASE_864M875Hz+(9*SPACE_250kHz)),
        __FREQ24(BASE_864M875Hz+(10*SPACE_250kHz)),
        __FREQ24(BASE_864M875Hz+(11*SPACE_250kHz)),
        __FREQ24(BASE_864M875Hz+(12*SPACE_250kHz)),
        __FREQ24(BASE_864M875Hz+(13*SPACE_250kHz)),
        __FREQ24(BASE_864M875Hz+(14*SPACE_250kHz)),
        __FREQ24(BASE_864M875Hz+(15*SPACE_250kHz)),
        //915: 160-211
        __FREQ24(BASE_901M250Hz+(1*SPACE_1500kHz)),
        __FREQ24(BASE_901M250Hz+(2*SPACE_1500kHz)),
        __FREQ24(BASE_901M250Hz+(3*SPACE_1500kHz)),
        __FREQ24(BASE_901M250Hz+(4*SPACE_1500kHz)),
        __FREQ24(BASE_901M250Hz+(5*SPACE_1500kHz)),
        __FREQ24(BASE_901M250Hz+(6*SPACE_1500kHz)),
        __FREQ24(BASE_901M250Hz+(7*SPACE_1500kHz)),
        __FREQ24(BASE_901M250Hz+(8*SPACE_1500kHz)),
        __FREQ24(BASE_901M250Hz+(9*SPACE_1500kHz)),
        __FREQ24(BASE_901M250Hz+(10*SPACE_1500kHz)),
        __FREQ24(BASE_901M250Hz+(11*SPACE_1500kHz)),
        __FREQ24(BASE_901M250Hz+(12*SPACE_1500kHz)),
        __FREQ24(BASE_901M250Hz+(13*SPACE_1500kHz)),
        __FREQ24(BASE_901M250Hz+(14*SPACE_1500kHz)),
        __FREQ24(BASE_901M250Hz+(15*SPACE_1500kHz))     
    };
    
    ot_u8 fc_i;

    /// PA reprogram
    if (old_tx_eirp != phymac[0].tx_eirp) {
        //rfctl.flags |= RADIO_FLAG_SETPWR; // no longer necessary
        sx127x_set_txpwr( phymac[0].tx_eirp );
    }

    /// Configure data rate: only change registers if required
    /// Current LoRa impl has single data rate!
    //if ((old_chan_id ^ phymac[0].channel) & 0x30) {
    //}

    fc_i = (phymac[0].channel & 0x0F);
    if (fc_i != (old_chan_id & 0x0F)) {
        ot_uint offset;
        offset  = (phymac[0].flags > 2) ? 2 : phymac[0].flags;
        offset *= (15*4);
        offset += (fc_i-1);
        sx127x_spibus_io(4, 0, (ot_u8*)&bandplan[offset]);
    }
}
#endif




#ifndef EXTF_rm2_mac_configure
OT_WEAK void rm2_mac_configure() {
/// Only use this when there is a hardware MAC filtering ability.  
/// SX127x does not have this capability.
}
#endif


#ifndef EXTF_rm2_calc_link
OT_WEAK void rm2_calc_link() {
	ot_u8 prssi_code	= sx127x_read(RFREG_LR_PKTRSSIVALUE);
	ot_s8 psnr_code		= sx127x_read(RFREG_LR_PKTSNRVALUE);
	
    radio.last_rssi     = sx127x_calc_rssi(prssi_code, psnr_code);
    radio.last_linkloss = (ot_int)(rxq.front[2] & 0x7F) - 80 - RF_HDB_RXATTEN;
    radio.last_linkloss-= radio.last_rssi;

    // Save additional link parameters.  If OT_FEATURE(RF_LINKINFO) is not
    // enabled in the app config, nothing will happen in this function.
    sx127xdrv_save_linkinfo();
}
#endif


#ifndef EXTF_rm2_get_floodcounter
OT_WEAK ot_int rm2_get_floodcounter() {
/// The time value put into countdown should indicate the amount of time following
/// the end of the BG packet it is inserted into.  On SX127x, there isn't a nice
/// way to send a continuous flood, so we consider the time on air of a whole
/// BG packet (about 29 symbols, or 7 ticks).
    ot_int offset_count;
    offset_count    = (ot_int)sx127x_get_counter();
    offset_count   -= 7; //rm2_scale_codec(29);
    return offset_count;
}
#endif


#ifndef EXTF_rm2_kill
OT_WEAK void rm2_kill() {
    __DEBUG_ERRCODE_EVAL(=290);
    //cad_misses++;
    radio_gag();
    radio_idle();
    radio_finish(RM2_ERR_KILL, 0);
}
#endif


void sub_hwtimeout_isr() {
    /// In BG Listen, system is using RX-Single reception, and we try a few 
    /// times to receive a packet after having CAD validation.
    /// Also, on RX timeout, save the link information.  This is used as a baseline.
    if (rfctl.flags & RADIO_FLAG_BG) {
        if (--rfctl.tries == 0) {
            sx127xdrv_save_linkinfo();
            radio_finish(RM2_ERR_TIMEOUT, radio.link.sqi);
            return;
        }
    }
    
    /// Loop RX-Single listening mode in BG-retry or FG-RX.
    /// In FG-RX, DLL will timeout the RX process manually.
    //rm2_reenter_rx(radio.evtdone);
    sx127x_write(RFREG_LR_IRQFLAGS, 0xFF);
    sx127x_strobe(_OPMODE_RXSINGLE, False);
}

#ifndef EXTF_rm2_rxtimeout_isr
OT_WEAK void rm2_rxtimeout_isr() {
    __DEBUG_ERRCODE_EVAL(=280);

    /// Can't have an interrupt during termination/re-configuration
    radio_gag();

    /// Send timeout error back to DLL
    sx127xdrv_save_linkinfo();
    radio_finish(RM2_ERR_TIMEOUT, radio.link.sqi);
}
#endif





/** Radio RX Functions
  * ============================================================================
  */
void sub_initrx(void) {
    radio_activate_queue(&rxq);
    sx127x_iocfg_rx();
    //dll_offset_rxtimeout();   ///@todo why is this commented-out?  Does it matter?
    rm2_reenter_rx(radio.evtdone);
}

void sub_initcad(void) {
    sx127x_write(RFREG_LR_IRQFLAGS, 0xFF);  
    sx127x_iocfg_cad();
    sx127x_int_listen();        
    sx127x_strobe(_OPMODE_CAD, False);  //non-blocking call
}

static const ot_u8 bginit[5] = {
    (ot_u8)MODE_bg,                 //buffer mode
    6,                              //pktlen
    (0x80 | RFREG_LR_MODEMCONFIG2),
    DRF_LR_MODEMCONFIG2_BG,         //ModemCfg2 value
    DRF_LR_SYMBTIMEOUTLSB_BG        //SymbTimeoutLsb value
};
static const ot_u8 fginit[5] = {
    (ot_u8)MODE_fg,                 //buffer mode
    255,                            //pktlen
    (0x80 | RFREG_LR_MODEMCONFIG2),
    DRF_LR_MODEMCONFIG2_FG,         //ModemCfg2 value
    DRF_LR_SYMBTIMEOUTLSB_FG        //SymbTimeoutLsb value
};

#ifndef EXTF_rm2_rxinit
OT_WEAK void rm2_rxinit(ot_u8 channel, ot_u8 psettings, ot_sig2 callback) {
    //ot_u8   netstate;
    ot_u8*  initvals;
    ot_sub  sub_init;

    __DEBUG_ERRCODE_EVAL(=200);

    /// Setup the RX engine for Foreground Frame detection and RX.  Wipe-out
    /// the lower flags (non-persistent flags)
    radio.evtdone   = callback;
    rfctl.flags    &= ~(RADIO_FLAG_CONT | RADIO_FLAG_BG);

    // LoRa HW cannot support multiframe packets, which simplifies setup a bit
    ///@todo rfctl.tries could be configurable in future
    if (psettings & (M2_NETFLAG_BG)) {
        rfctl.flags    |= RADIO_FLAG_BG;
        rfctl.tries     = 3;        
      //netstate        = (M2_NETSTATE_UNASSOC | M2_NETFLAG_FIRSTRX);
        initvals        = (ot_u8*)bginit;
        sub_init        = &sub_initcad;
    }
    else {
      //netstate        = psettings;
        initvals        = (ot_u8*)fginit;
        sub_init        = &sub_initrx;
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
    sx127xdrv_buffer_config(initvals[0], initvals[1]);
    
    sx127x_write(RFREG_LR_MODEMCONFIG2, initvals[3]);
    sx127x_write(RFREG_LR_SYMBTIMEOUTLSB, initvals[4]);
    //sx127x_spibus_io(3, 0, &initvals[2]);                 //optimized version of above
    
    // Done in buffer_config
    //sx127x_write(RFREG_LR_MODEMCONFIG1, initvals[2]);

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

OT_WEAK void rm2_rxtest(ot_u8 channel, ot_u8 tsettings, ot_u16 timeout) {
    // Set basic operational flags
    rfctl.flags    &= ~(RADIO_FLAG_CONT | RADIO_FLAG_BG);
    //netstate        = tsettings;                    ///@todo bury this into session top?
    radio.evtdone   = &sub_rxtest_callback;
    
    // We don't care if the channel is supported, because no data is going to
    // be sent.  Just set the center frequency to where it is desired
    rm2_enter_channel((channel & 0x7f), (phymac[0].tx_eirp & 0x7f));
    
    // Configure radio for Foreground reception (nominal)
    sx127xdrv_buffer_config(fginit[0], fginit[1]);
    sx127x_write(RFREG_LR_MODEMCONFIG1, fginit[2]);
    
    // Set MAC timer as termination timer
    radio_set_mactimer(timeout);
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
    
    //radio_idle();
    sx127x_strobe(_OPMODE_RXSINGLE, False);  //sx127x_strobe(opmode, False);   //non-blocking call
    
    // Undo packet sync on re-entry.
    // Main work here is to change Radio IRQ state and task priority.
    // Occurs when you need to listen to more packets.
    //sx127xdrv_unsync_isr();
    sx127x_int_rxdata();
    radio.state = RADIO_Listening;
    dll_unblock();
}
#endif


#ifndef EXTF_rm2_rxsync_isr
OT_WEAK void rm2_rxsync_isr() {
/// Prepare driver for data reception, update high-level module state, and have
/// supervisor task (DLL) go into high-priority mode.

    __DEBUG_ERRCODE_EVAL(=210); 
    
    //Visual Debugging on Nucleo
    //BOARD_led3_on();

    // In LoRa, the DASH7 Header is sent neither in BG or FG modes.
    // - Multiframe packets are not possible in LoRa, so FRAME-CONT bit goes to 0
    // - RS Coding may be implemented based on channel code rather than Err-Code bit
    q_empty(&rxq);
    em2_decode_newpacket();
    em2_decode_newframe();

    radio.state = RADIO_DataRX;
    dll_block();
}
#endif



#ifndef EXTF_rm2_rxdata_isr
OT_WEAK void rm2_rxdata_isr() {
/// In SX127x, there is no intermediate RX data ISR.  Bounce to rm2_rxend_isr()
    rm2_rxend_isr();
}
#endif


#ifndef EXTF_rm2_rxend_isr
OT_WEAK void rm2_rxend_isr() {
    ot_u16 integrity;

    __DEBUG_ERRCODE_EVAL(=230);
    
    radio_gag();                                // No more interrupts!
    rfctl.state = RADIO_STATE_RXDONE;           // Make sure in DONE State, for decoding
    
    /// Background Frame: use CRC8, don't bother with Length setting
    if (rfctl.flags & RADIO_FLAG_BG) {
        rxq.putcursor += 6;
        sx127x_burstread(RFREG_LR_FIFO, 6, rxq.getcursor);
        integrity = bgcrc8_check(rxq.getcursor);
    }
    
    /// Foreground Frame: use normal encoding model, with optional RS
    else {
        em2_decode_data();                          // decode any leftover data
        integrity = em2_decode_endframe();          // Finish RS Coding or CRC, get result (should be 0)
    }
    
    rm2_calc_link();                            // Calculate relative link info
    
    //Visual Debugging on Nucleo
    //BOARD_led3_off();
    
    radio_finish(0, integrity);
}
#endif







/** Radio TX Functions  <BR>
  * ========================================================================<BR>
  */

#ifndef EXTF_rm2_txinit
OT_WEAK void rm2_txinit(ot_u8 psettings, ot_sig2 callback) {
    rfctl.flags    &= ~(RADIO_FLAG_BG | RADIO_FLAG_CONT);
    rfctl.flags    |= (psettings & (M2_NETFLAG_BG | M2_NETFLAG_STREAM)) >> 6;
    radio.evtdone   = callback;
    radio.state     = RADIO_Csma;
    rfctl.state     = RADIO_STATE_TXINIT;

    // initialize the CRC/RS disabling byte
    txq.options.ubyte[UPPER] = 0;

    /// CSMA-CA interrupt based and fully pre-emptive.  This is
    /// possible using CC1 on the GPTIM to clock the intervals.
    radio_set_mactimer( (ot_uint)dll.comm.tca );
}
#endif



#ifndef EXTF_rm2_txtest
OT_WEAK void rm2_txtest(ot_u8 channel, ot_u8 eirp_code, ot_u8 tsettings, ot_u16 timeout) {
    ot_u8 old_channel;
    ot_u8 old_eirp;

    // We don't care if the channel is supported, because no data is going to
    // be sent.  Just set the center frequency to where it is desired
    old_channel         = phymac[0].channel;
    old_eirp            = phymac[0].tx_eirp;
    phymac[0].channel   = channel;
    phymac[0].tx_eirp   = eirp_code;
    rm2_enter_channel(old_channel, old_eirp);

    // Set TX PATABLE values if different than pre-existing values
    // (Now done directly within rm2_enter_channel
    //if (rfctl.flags & RADIO_FLAG_SETPWR) {
    //    rfctl.flags &= ~RADIO_FLAG_SETPWR;
    //    sx127x_set_txpwr( phymac[0].tx_eirp );
    //}

    // This bit enables TX-Cont mode.
    ///@todo possible to have callback here that disables TX_CONT_ON
    sx127x_write(RFREG_LR_MODEMCONFIG2, DRF_LR_MODEMCONFIG2 | _TX_CONT_ON);

    // No callback
    radio.evtdone = &otutils_sig2_null;

    sx127x_int_off();
    sx127x_iocfg_tx();
    radio_flush_tx();

    radio.state = RADIO_DataTX;
    sx127x_strobe( _OPMODE_TX , False); //non-blocking call
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


#ifndef EXTF_rm2_txstop_flood
OT_WEAK void rm2_txstop_flood() {
/// Stop the MAC counter used to clock advertising flood synchronization.
/// Then simply configure TX driver state machine to go to TX Done state
/// as soon as the current packet is finished transmitting.
    rfctl.state = RADIO_STATE_TXDONE;
    sx127x_stop_counter();
    sx127x_int_txdata();
}
#endif


#ifndef EXTF_rm2_txcsma_isr
void sub_cad_csma(void) {
    radio_gag(); 
    sx127x_iocfg_cad();
    sx127x_int_csma();
    radio_idle();
    sx127x_strobe(_OPMODE_CAD, False);  //non-blocking call
    radio.state = RADIO_Csma;
}

ot_bool sub_cca_isfail(void) {
    ot_bool test;
    test = sx127x_check_cadpin();
    if (test) {
        sx127x_int_off();
        rfctl.state = RADIO_STATE_TXCAD1;
        radio.evtdone(1, 0);
    }
    return test;
}


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

            // Set other TX Buffering & Packet parameters, and also save the
            // Peristent-TX attribute for floods, which is written later
            type = (rfctl.flags & RADIO_FLAG_BG) ? MODE_bg : MODE_fg;
            sx127xdrv_buffer_config(type, em2.bytes /*q_span(&txq)*/);
            sx127x_int_off();
            
            // Bypass CSMA if MAC is disabling it
            if (dll.comm.csmaca_params & M2_CSMACA_NOCSMA) {
                goto rm2_txcsma_START;
            }
            
            // Enter CSMA via fallthrough.  
            // There are two incremental tests: CCA1 and CCA2.
            //cad_attempts++;
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
            if (sub_cca_isfail()) {
                break;
            }

        /// 6. TX startup:
        case (RADIO_STATE_TXSTART >> RADIO_STATE_TXSHIFT): {
        rm2_txcsma_START:
            // Send TX start (CSMA done) signal to DLL task
            // arg2: Non-zero for background, 0 for foreground
            rfctl.state = RADIO_STATE_TXSTART;
            radio.evtdone(0, (rfctl.flags & (RADIO_FLAG_BG | RADIO_FLAG_CONT)));

            // Clear Radio for TX
            radio_gag();
            radio_flush_tx();
            
            // Preload into TX FIFO all packet data
            // There are slightly different processes for BG and FG frames
            txq.front[2] = rm2_clip_txeirp(phymac[0].tx_eirp);  // TX EIRP value overwrite
            if (rfctl.flags & RADIO_FLAG_BG) {
                radio_idle();
                if (rfctl.flags & RADIO_FLAG_CONT) {
                    sx127x_start_counter();
                }
                radio.evtdone((rfctl.flags & RADIO_FLAG_CONT), 0);
                bgcrc8_put(&txq.front[2]);
                sx127x_burstwrite(RFREG_LR_FIFO, 6, &txq.front[2]);
            }
            else {
                em2_encode_data();
            }

            // Prepare for TX, then enter TX
            // For floods, we must activate the flood counter right before TX
            radio.state = RADIO_DataTX;
            rfctl.state = RADIO_STATE_TXDATA;
            sx127x_iocfg_tx();
            sx127x_strobe(_OPMODE_TX, False);   //non-blocking call
            sx127x_int_txdata();
            break;
        }
    }
}
#endif


#ifndef EXTF_rm2_txdata_isr
void sub_txend(void) {
    radio_gag();
    radio_idle();
    radio_finish((rfctl.flags & (RADIO_FLAG_BG | RADIO_FLAG_CONT)), 0);
}

OT_WEAK void rm2_txdata_isr() {
/// Continues where rm2_txcsma() leaves off.

    /// Packet BG flooding.  Only needed on devices that can send M2AdvP
    /// The radio.evtdone callback here should update the AdvP payload
    if ((rfctl.flags & RADIO_FLAG_BGFLOOD) == RADIO_FLAG_BGFLOOD) {
        radio.evtdone(RADIO_FLAG_CONT, 0);

        if ((rfctl.state & RADIO_STATE_TXMASK) == RADIO_STATE_TXDATA) {
            bgcrc8_put(&txq.front[2]);
            sx127x_write(RFREG_LR_IRQFLAGS, 0xFF);
            sx127x_burstwrite(RFREG_LR_FIFO, 6, &txq.front[2]);
            sx127x_strobe(_OPMODE_TX, False);   //non-blocking call
            return;
        }
    }

    /// Packet is done
    radio_gag();
    radio_idle();
    radio_finish((rfctl.flags & (RADIO_FLAG_BG | RADIO_FLAG_CONT)), 0);
}
#endif











/** General Purpose Radio Subroutines
  * ============================================================================
  * - Usually some minor adjustments needed when porting to new platform
  * - See integrated notes for areas sensitive to porting
  */

void sx127xdrv_null(ot_int arg1, ot_int arg2) { }



void sx127xdrv_buffer_config(MODE_enum mode, ot_u16 param) {
/// Background frames:
/// Sync = D7, Header=Implicit, Length=6
/// Foreground frames:
/// Sync = 28, Header=Explicit, Length=255(var)

    static const ot_u8 regs[] = { 
        (_BW_500_KHZ | _CODINGRATE_4_7 | _IMPLICITHEADER_ON | _RXPAYLOADCRC_OFF | _LOWDATARATEOPTIMIZE_OFF),
        0xD7, 
        (_BW_500_KHZ | _CODINGRATE_4_7 | _IMPLICITHEADER_OFF | _RXPAYLOADCRC_OFF | _LOWDATARATEOPTIMIZE_OFF),
        0x28, 
    };
    
    sx127x_write(RFREG_LR_MODEMCONFIG1, regs[mode+0]);
    sx127x_write(RFREG_LR_SYNCWORD, regs[mode+1]);
    sx127x_write(RFREG_LR_PAYLOADLENGTH, param);
}



void sx127xdrv_save_linkinfo() {
    // Link information: only SNR is available, which is saved as LQI
    radio.link.pqi  = 1;
    radio.link.sqi  = 1;
    radio.link.lqi  = sx127x_read(RFREG_LR_PKTRSSIVALUE);
    radio.link.agc  = 0;
}






/** General Purpose, Low-Level Radio Module control    <BR>
  * ========================================================================<BR>
  * Used for data FIFO interaction and power configuration, but not specifically
  * related to Mode 2.  These are implemented in the radio driver.
  */

#ifndef EXTF_radio_off
OT_WEAK void radio_off() {
   sx127x_reset();
   radio_sleep();
}
#endif

#ifndef EXTF_radio_gag
OT_WEAK void radio_gag() {
    sx127x_int_off();                       // Disable pin interrutps on MCU
    sx127x_write(RFREG_LR_IRQFLAGS, 0xFF);  // Clear Register IRQ Flags on SX127x
}
#endif

#ifndef EXTF_radio_ungag
OT_WEAK void radio_ungag() {
    if (radio.state != RADIO_Idle) {
        sx127x_int_on();
    }
}
#endif

#ifndef EXTF_radio_sleep
OT_WEAK void radio_sleep() {
    sx127x_strobe(_OPMODE_SLEEP, True);

/// SX127x can go into SLEEP from any other state, so the only optimization 
/// here is to check if it is already in SLEEP before dealing with GPIO & SPI.
//    ot_bool test;
//    test = (rfctl.flags & RADIO_FLAG_XOON);
//    if (test) {
//        radio.state  = RADIO_Idle;
//        rfctl.flags &= ~RADIO_FLAG_PWRMASK;
//        sx127x_strobe(_OPMODE_SLEEP);
//        sx127x_waitfor_sleep();
//    }
}
#endif

#ifndef EXTF_radio_idle
OT_WEAK void radio_idle() {
    sx127x_strobe(_OPMODE_STANDBY, True);
    
/// SX127x can go into STANDBY from any other state, so the only optimization 
/// here is to check if it is already in STANDBY before dealing with GPIO & SPI.
//    ot_bool test;
//    test = ((rfctl.flags & RADIO_FLAG_XOON) && (radio.state == RADIO_Idle));
//    if (!test) {
//        radio.state  = RADIO_Idle;
//        rfctl.flags |= RADIO_FLAG_XOON;
//        sx127xdrv_force_standby();
//    }
}
#endif

#ifndef EXTF_radio_putbyte
OT_WEAK void radio_putbyte(ot_u8 databyte) {
    sx127x_write(RFREG_LR_FIFO, databyte);
}
#endif

#ifndef EXTF_radio_putfourbytes
OT_WEAK void radio_putfourbytes(ot_u8* data) {
/// Unused in SX127x
}
#endif

#ifndef EXTF_radio_getbyte
OT_WEAK ot_u8 radio_getbyte() {
    return sx127x_read(RFREG_LR_FIFO);
}
#endif

#ifndef EXTF_radio_getfourbytes
OT_WEAK void sx127x_getfourbytes(ot_u8* data) {
/// Unused in SX127x
}
#endif

#ifndef EXTF_radio_flush_rx
OT_WEAK void radio_flush_rx() {
    sx127x_write(RFREG_LR_FIFOADDRPTR, 0);
}
#endif

#ifndef EXTF_radio_flush_tx
OT_WEAK void radio_flush_tx() {
    sx127x_write(RFREG_LR_FIFOADDRPTR, 0);
}
#endif

#ifndef EXTF_radio_rxopen
OT_WEAK ot_bool radio_rxopen() {
    return (ot_bool)sx127x_rxbytes();
}
#endif

#ifndef EXTF_radio_rxopen_4
OT_WEAK ot_bool radio_rxopen_4() {
    return (ot_bool)(sx127x_rxbytes() >= 4);
}
#endif

#ifndef EXTF_radio_txopen
OT_WEAK ot_bool radio_txopen() {
    return (ot_bool)(radio.state == RADIO_DataTX);
}
#endif

#ifndef EXTF_radio_txopen_4
OT_WEAK ot_bool radio_txopen_4() {
/// Never really needed with SX127x
    return radio_txopen();
}
#endif

#ifndef EXTF_radio_rssi
OT_WEAK ot_int radio_rssi() {
    return radio.last_rssi;
    //radio.last_rssi = sx127x_calc_rssi(sx127x_read(RFREG_LR_PKTRSSIVALUE), sx127x_read(RFREG_LR_PKTSNRVALUE));
}
#endif

#ifndef EXTF_radio_check_cca
OT_WEAK ot_bool radio_check_cca() {
/// CCA Method: Look if CS pin is high.
    return (sx127x_check_cadpin() == 0);
}
#endif

#ifndef EXTF_radio_calibrate
OT_WEAK void radio_calibrate() {
/// SX127x does mandatory automatic calibration
}
#endif


ot_u8 radio_getpwrcode() {
/// Power code: 0-3.  sx127x_getbasepwr() typically returns 3 on most platforms
    return sx127x_getbasepwr() - (radio.state > RADIO_Idle);
}


#endif

