/* Copyright 2010-2012 JP Norair
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
  * @file       /otradio/cc430/radio_CC430.c
  * @author     JP Norair
  * @version    V1.0
  * @date       22 July 2012
  * @brief      Radio Driver (RF transceiver) for CC430
  * @defgroup   Radio (Radio Module)
  * @ingroup    Radio
  *
  * The header file for this implementation is /OTlib/radio.h.  It is universal
  * for all platforms, even though the implementation (this file) can differ.
  * There is also a header file at /Platforms/CC430/radio_CC430.h that contains
  * macros & constants specific to this implementation.
  *
  * For DASH7 Silicon certification, there are four basic tiers of HW features:
  * 1. PHY      The HW has a buffered I/O and the basic features necessary
  * 2. PHY+     The HW can do encoding, CRC, and some packet handling
  * 3. MAC      The HW can automate some inner loops, like Adv Flood and CSMA
  * 4. MAC+     The HW has most features of the MAC integrated
  *
  * The CC430/CC1101 is not the highest performing RF Core, but it has the most
  * built-in features of currently available DASH7 transceivers.  By the rubrick
  * above, it meets PHY and PHY+ (with the exception of FEC support), and it
  * also has some degree of built-in MAC features, notably an RX Timeout timer.
  ******************************************************************************
  */

#include <m2/radio.h>
#include "radio_CC430.h"
#include "CC430_interface.h"

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
  * Described in radio_CC430.h
  */
cc430_struct    cc430;



/** Local Subroutine Prototypes  <BR>
  * ========================================================================<BR>
  */
typedef enum {
    SYNC_bg = 0,
    SYNC_fg = 2
} sync_enum;


ot_bool subcc430_test_channel(ot_u8 channel);
void    subcc430_launch_rx(ot_u8 channel, ot_u8 netstate);

void    subcc430_finish(ot_int main_err, ot_int frame_err);
ot_bool subcc430_lowrssi_reenter();
void    subcc430_reset_autocal();

ot_bool subcc430_chanscan();
void    subcc430_ccascan();
void    subcc430_ccascan_isr();

ot_bool subcc430_channel_lookup(ot_u8 chan_id, vlFILE* fp);
void    subcc430_syncword_config(ot_u8 sync_class);
void    subcc430_buffer_config(ot_u8 mode, ot_u8 param);
void    subcc430_chan_config(ot_u8 old_chan, ot_u8 old_eirp);

void    subcc430_prep_q(ot_queue* q);
//ot_int  subcc430_eta(ot_int next_int);
//ot_int  subcc430_eta_rxi();
//ot_int  subcc430_eta_txi();
void    subcc430_offset_rxtimeout();

ot_u8   subcc430_rssithr_calc(ot_u8 input, ot_u8 offset); // Fiddling necessary
void    subcc430_set_txpwr(ot_u8 eirp_code);


//void sub_txend_isr();



/** CC430 RF Core Interrupt Handler  <BR>
  * ========================================================================<BR>
  */
OT_INLINE void platform_isr_rf1a(void) {
    u16 core_edge;
    u16 core_vector;

    /// 1.  Interface Interrupt Handler:
    ///     At the moment, OpenTag does not use these.  You might be able to use
    ///     use the non-error interrupts if you want to optimize the last 1% of
    ///     power consumption.  The error interrupts can also be helpful for
    ///     driver & system debugging.  The basic design of this block assumes
    ////    that you are using it for error debugging.
    if ((RFWord->IFIV & 0x0E) != 0) {
        // Assuming error usage: call a User NMI, which OT handles as seg fault.
        // Technically it should be bus error, but there is only one User NMI.
        SFRIFG1 |= NMIIFG;

#       if 0
            /// Here is the max functionality of the RF interface handler.  For
            /// now itis disabled by macro above.  Maybe later I will find
            /// something good to do with it.  For now, I just ignore.
            switch (__even_in_range(RFWord->IFIV, 0x0E)) {
                case 0x00:  break;

                // error traps -- avoid these by clearing above
                case 0x02:  switch (RFWord->IFERRV) {
                                case 0x02: break;     // low core voltage error
                                case 0x04: break;     // operand error
                                case 0x06: break;     // output data not available error
                                case 0x08: break;     // operand overwrite error
                            }
                            RF_ClearIFITPendingBit( RF_IFIT_ERR );
                            break;

                case 0x04:  RF_ClearIFITPendingBit( RF_IFIT_DOUT );     break;
                case 0x06:  RF_ClearIFITPendingBit( RF_IFIT_STAT );     break;
                case 0x08:  RF_ClearIFITPendingBit( RF_IFIT_DIN );      break;
                case 0x0A:  RF_ClearIFITPendingBit( RF_IFIT_INSTR );    break;
                case 0x0C:  RF_ClearIFITPendingBit( RF_IFIT_TX );       break;
                case 0x0E:  RF_ClearIFITPendingBit( RF_IFIT_RX );       break;
            }
#       endif
    }



    /// 2.  Core Interrupt Handler:
    /// The following lines are a little confusing.  The CC430 RF Core interrupt
    /// mechanism is a bit strange, as the meaning of an interrupt is dependent
    /// on the interrupt flag (obviously) but also the setting of the edge
    /// detector.  Rising and falling edges generate the same flag bit, but have
    /// different meanings.
    ///
    /// 1. Get the flag vector (it has values 0 - 0x20, evens only)     <BR>
    /// 2. Convert flag vector to edge-select mask bit                  <BR>
    /// 3. Clear the interrupt flag that is causing this interrupt      <BR>
    /// 4. Compare the flag location with the edge setting              <BR>
    /// 5. If the edge setting is "falling," extend the vector.

    core_vector     = RFWord->IV;                       //1
    core_edge       = 1 << ((core_vector-2) >> 1);          //2
    //core_edge     >>= 1;                                //2
    RFWord->IFG    &= ~core_edge;                       //3
    core_edge      &= RFWord->IES;                      //4
    core_vector    += (core_edge) ? 0x22 : 0;           //5

    switch (core_vector) {
        // Rising Edges
        case 0x00:  break;

#       if (RF_FEATURE(RXTIMER) == ENABLED)
        case 0x02:  rm2_rxtimeout_isr();    break;  //IOCFG0_ISR();
#       endif

        case 0x04:  subcc430_ccascan_isr(); break;  //IOCFG1_ISR(); (Default: RSSI_Valid)
        //case 0x06:  __no_operation();       break;  //IOCFG2_ISR();
        case 0x08:  rm2_rxdata_isr();       break;  //RXFull_ISR();
        //case 0x0A:  rm2_rxdata_isr();       break;  //RXFullOrDone_ISR();
        //case 0x0C:  __no_operation();       break;  //TXAboveThresh_ISR();
        //case 0x0E:  __no_operation();       break;  //TXFull_ISR();
        //case 0x10:  __no_operation();       break;  //RXOverflow_ISR();
        //case 0x12:  rm2_txdata_isr();       break;  //TXUnderflow_ISR();
        case 0x14:  rm2_rxsync_isr();       break;  //SyncWord_ISR()
        //case 0x16:  __no_operation();       break;  //CRCValid_ISR();
        //case 0x18:  __no_operation();       break;  //PQTReached_ISR();
        //case 0x1A:  __no_operation();       break;  //ClearChannel_ISR();
        //case 0x1C:  __no_operation();       break;  //CarrierSense2_ISR();
        //case 0x1E:  __no_operation();       break;  //WORevent0_ISR();
        //case 0x20:  __no_operation();       break;  //WORevent1_ISR();

        // Falling Edges
        //case 0x22:  break;
        //case 0x24:  __no_operation();       break;  //IOCFG0_ISR();
        //case 0x26:  __no_operation();       break;  //IOCFG1_ISR();
        //case 0x28:  __no_operation();       break;  //IOCFG2_ISR();
        //case 0x2A:  __no_operation();       break;  //RXNotFull_ISR();
        //case 0x2C:  __no_operation();       break;  //RXEmpty();
        case 0x2E:  rm2_txdata_isr();       break;  //TXBelowThresh_ISR();
        //case 0x30:  __no_operation();       break;  //TXBelowThresh2_ISR();
        //case 0x32:  __no_operation();       break;  //RXFlushed_ISR();
        //case 0x34:  __no_operation();       break;  //TXFlushed_ISR();

        case 0x36:                                  //EndState_ISR();
            if (cc430.state & RADIO_STATE_TXMASK)
                subcc430_finish(0, 0); //rm2_txdata_isr();
            else
                rm2_rxend_isr();
            break;

        //case 0x38:  __no_operation();       break;  //RXFirstByte_ISR();
        //case 0x3A:  __no_operation();       break;  //LPW_ISR();
        //case 0x3C:  __no_operation();       break;  //CarrierSense_ISR();
        //case 0x3E:  __no_operation();       break;  //RSSILow_ISR();   //(ClearChannel2)
        //case 0x40:  __no_operation();       break;  //WORevent0ACLK_ISR();
        //case 0x42:  __no_operation();       break;  //OscStable_ISR();

        // Bug trap: Underflow, Overflow, or something else bad
        default:    rm2_kill();
                    break;
    }

    //Read to RFWord->IV automatically clears the highest priority pending bit
    //RF_ClearCoreITPendingBit(RF_CoreIT_ALL);
    //LPM4_EXIT;
}




void radio_mac_isr() {
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
   radio_sleep();   //CC430 is SoC, so sleep is same as off
}
#endif

#ifndef EXTF_radio_gag
void radio_gag() {
    //RF_CoreITConfig(RF_CoreIT_ALL, DISABLE);
    RFWord->IE = 0;
}
#endif

#ifndef EXTF_radio_sleep
void radio_sleep() {
/// Only go to sleep if the device is not already asleep
    radio.state = RADIO_Idle;
    if ((cc430.flags & RADIO_FLAG_ASLEEP) == 0) {
        cc430.flags |= RADIO_FLAG_ASLEEP;
        RF_CmdStrobe(RF_CoreStrobe_IDLE);
        RF_CmdStrobe(RF_CoreStrobe_PWD);
    }
}
#endif

#ifndef EXTF_radio_idle
void radio_idle() {
    radio.state = RADIO_Idle;
    RF_CmdStrobe(RF_CoreStrobe_IDLE);
    if (cc430.flags & RADIO_FLAG_ASLEEP) {
        cc430.flags ^= RADIO_FLAG_ASLEEP;
        RF_WriteSingleReg(RFREG(TEST0), DRF_TEST0);
    }
}
#endif

#ifndef EXTF_radio_calibrate
void radio_calibrate() {
    RF_CmdStrobe(RF_CoreStrobe_CAL);
}
#endif


#ifndef EXTF_radio_set_mactimer
void radio_set_mactimer(ot_u16 clocks) {
/// Todo: apply board configuration
    platform_set_gptim2(clocks);
}
#endif





/** Radio Module Control Functions
  * ============================================================================
  * - Need to be customized per radio platform
  */
#ifndef EXTF_radio_init
void radio_init( ) {
/// Transceiver implementation dependent
    static const ot_u8 cc430_defaults[] = {
         DRF_IOCFG2,
         DRF_IOCFG1,
         DRF_IOCFG0,
         DRF_FIFOTHR,
         DRF_SYNC1,
         DRF_SYNC0,
         DRF_PKTLEN,
         DRF_PKTCTRL1,
         DRF_PKTCTRL0,
         DRF_ADDR,
         DRF_CHANNR,
         DRF_FSCTRL1,
         DRF_FSCTRL0,
         DRF_FREQ2,
         DRF_FREQ1,
         DRF_FREQ0,
         DRF_MDMCFG4,
         DRF_MDMCFG3,
         DRF_MDMCFG2,
         DRF_MDMCFG1,
         DRF_MDMCFG0,
         DRF_DEVIATN,
         DRF_MCSM2,
         DRF_MCSM1,
         DRF_MCSM0,
         DRF_FOCCFG,
         DRF_BSCFG,
         DRF_AGCCTRL2,
         DRF_AGCCTRL1,
         DRF_AGCCTRL0,
         DRF_WOREVT1,
         DRF_WOREVT0,
         DRF_WORCTRL,
         DRF_FREND1,
         DRF_FREND0,
         DRF_FSCAL3,
         DRF_FSCAL2,
         DRF_FSCAL1,
         DRF_FSCAL0,
    };

    vlFILE* fp;

    ///Disable all Interface and Core interrupts on startup init
    RFWord->IFCTL1 = 0;
    RFWord->IE     = 0;

    /// Load default register values to RF Core, starting at the first one.
    RF_WriteBurstReg(   RFREG(IOCFG2),
                        (ot_u8*)&cc430_defaults[0],
                        sizeof(cc430_defaults) );

    RF_WriteSingleReg(RFREG(TEST0), DRF_TEST0);

    /// Set startup channel to a completely invalid channel ID (0x55), and run
    /// lookup on the default channel (0x07) to kick things off.  Since the
    /// startup channel will always be different than a real channel, the
    /// necessary settings and calibration will always occur.
    phymac[0].channel   = 0x55;
    phymac[0].tx_eirp   = 0x7F;
    cc430.flags         = 0;
    cc430.state         = 0;
    radio.evtdone       = &otutils_sig2_null;
    fp                  = ISF_open_su( ISF_ID(channel_configuration) );
    radio.state         = RADIO_Idle;
    subcc430_channel_lookup(0x07, fp);
    vl_close(fp);

    // radio will be in sleep mode here
}
#endif

/*
ot_bool radio_check_cca() {
/// CCA Method 1: This method uses internal CCA HW of the CC1101 core.  The
/// problem is that it is not very precise or reliable, as it depends on all
/// kinds of mysterious settings that can be changed by mistake.  It works via
/// the RFAIN register on the CC430, which contains the Core interrupt flag for
/// CCA.

    // This is the same as: (ot_bool)(RFAIN & 0x1000)
    return (ot_bool)((ot_u8)RF_GetCoreITLevel(RF_CoreIT_ClearChannel2) == 0);
}
*/


#ifndef EXTF_radio_check_cca
ot_bool radio_check_cca() {
/// CCA Method 2: Compare stored limit with actual, detected RSSI.
/// On CC430, this method is more reliable and faster than Method 1 is.
    ot_int thr  = (ot_int)phymac[0].cca_thr - 140;
    ot_int rssi = radio_rssi();
    return (ot_bool)(rssi < thr);
}
#endif


#ifndef EXTF_radio_rssi
ot_int radio_rssi() {
/// Transceiver implementation dependent
/// CC430 stores the RSSI in a special register, as a 2's complement number, of
/// offset 0.5 dBm units.  This function translates it into normal dBm units.
    ot_s8  rssi_raw;

    rssi_raw            = (ot_s8)RF_GetRSSI();      // CC430 RSSI is 0.5 dBm units, signed byte
    radio.last_rssi     = (ot_int)rssi_raw;         // Convert to signed 16 bit (1 instr on MSP)
    radio.last_rssi    += 128;                      // Make it positive...
    radio.last_rssi   >>= 1;                        // ...So division to 1 dBm units can be a shift...
    radio.last_rssi    -= (64 + RF_RSSIOffset);     // ...and then rescale it, including offset

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
    RF_WriteSingleReg(RF_TXFIFOWR, databyte);
}
#endif


#ifndef EXTF_radio_putfourbytes
void radio_putfourbytes(ot_u8* data) {
/// @note Eventually I would like to use an endian-independent implementation,
/// (basically, this is the big endian version) but now is not the time. Doing
/// so will involve revising the FEC encoder in the Encode Module.
#if (M2_FEATURE(FEC) == ENABLED)
#   ifdef __BIG_ENDIAN__
        RF_WriteBurstReg(RF_TXFIFOWR, data, 4);
#   else
        ot_u8 data_le[4];
        data_le[0] = data[3];
        data_le[1] = data[2];
        data_le[2] = data[1];
        data_le[3] = data[0];
        RF_WriteBurstReg(RF_TXFIFOWR, data_le, 4);
#   endif
#endif
}
#endif


#ifndef EXTF_radio_getbyte
ot_u8 radio_getbyte() {
/// Transceiver implementation dependent
    return RF_ReadSingleReg(RF_RXFIFORD);
}
#endif



#ifndef EXTF_radio_getfourbytes
void radio_getfourbytes(ot_u8* data) {
/// @note because the radio is Big Endian (sensible) and the datastream is also
/// big endian (all serial streams are), no conversion is necessary (senisble).
    RF_ReadBurstReg(RF_RXFIFORD, data, 4);
}
#endif



#ifndef EXTF_radio_flush_rx
void radio_flush_rx() {
/// Transceiver implementation dependent
    RF_CmdStrobe( RF_CoreStrobe_FRX );
}
#endif



#ifndef EXTF_radio_flush_tx
void radio_flush_tx() {
/// Transceiver implementation dependent
    RF_CmdStrobe( RF_CoreStrobe_FTX );
}
#endif



#ifndef EXTF_radio_rxopen
ot_bool radio_rxopen() {
/// Transceiver implementation dependent
///@note Do not draw-out the bottom byte in the FIFO until packet is complete.
///      This is a known erratum of CC430 and CC11xx.
    return (ot_bool)(RF_GetRXBYTES() > (cc430.state != RADIO_STATE_RXDONE));
}
#endif



#ifndef EXTF_radio_rxopen_4
ot_bool radio_rxopen_4() {
/// Transceiver implementation dependent, only needed with FEC
///@note Do not draw-out the bottom byte in the FIFO until packet is complete.
///      This is a known erratum of CC430 and CC11xx.
    ot_int thresh;
    thresh = (cc430.state != RADIO_STATE_RXDONE) << 2;
    return (ot_bool)(RF_GetRXBYTES() > thresh);
}
#endif


#ifndef EXTF_radio_txopen
ot_bool radio_txopen() {
/// Transceiver implementation dependent
    return (ot_bool)(RF_GetTXBYTES() < cc430.txlimit);
}
#endif


#ifndef EXTF_radio_txopen_4
ot_bool radio_txopen_4() {
/// Transceiver implementation dependent, only needed with FEC
/// Use commented-out version, or alternatively just never set txlimit above
/// (RADIO_BUFFER_TXMAX-4)
    //ot_u8 fifo_limit = (cc430.txlimit < (RADIO_BUFFER_TXMAX-4)) ? \
    //                      (ot_u8)cc430.txlimit : (RADIO_BUFFER_TXMAX-4);
    //return (ot_bool)(RF_GetTXBYTES() < fifo_limit);

    return radio_txopen();
}
#endif





/** Radio I/O Functions
  * ============================================================================
  * -
  */

ot_bool subcc430_test_channel(ot_u8 channel) {
#if (SYS_RECEIVE == ENABLED)
    ot_bool test = True;

    if ((channel != phymac[0].channel)) {
        vlFILE* fp;
        /// Open the Mode 2 FS Config register that contains the channel list
        /// for this host, and make sure the channel we want to use is available
        /// @todo assert fp
        fp      = ISF_open_su( ISF_ID(channel_configuration) );
        test    = subcc430_channel_lookup(channel, fp);
        vl_close(fp);
    }

    return test;
#else
    return True;
#endif
}




#if (SYS_RECEIVE == ENABLED)
void subcc430_launch_rx(ot_u8 channel, ot_u8 netstate) {
    ot_u8       buffer_mode = 0;
    ot_u8       pktlen      = 7;
    ot_u8       mcsm21[2]   = { 7, DRF_MCSM1 };
    sync_enum   sync_type   = SYNC_bg;

    /// 1.  Prepare RX queue by flushing it
    cc430.rxlimit = 48;
    q_empty(&rxq);
    subcc430_prep_q(&rxq);

    /// 2. Fetch the RX channel, exit if the specified channel is not available
    if (subcc430_test_channel(channel) == False) {
        subcc430_finish(RM2_ERR_BADCHANNEL, 0);
        return;
    }

    /// 3. Prepare modem state-machine to do RX-Idle or RX-RX
    ///    RX-RX happens duing Response listening, unless FIRSTRX is high
    netstate &= (M2_NETFLAG_FIRSTRX | M2_NETSTATE_RESP);
    if ((netstate ^ M2_NETSTATE_RESP) == 0) {
        mcsm21[1] |= _RXOFF_MODE_RX;
    }

    /// 4a. Setup RX for Background detection (queue config & low-RSSI termination)
    if (cc430.flags & RADIO_FLAG_FLOOD) {
#       if (M2_FEATURE(FECRX) == ENABLED)
            if (phymac[0].channel & 0x80) pktlen = 16;
#       endif

        /// ot_queue manipulation to fit background frame into common model
     //#rxq.length      = pktlen + 2;
        rxq.front[0]    = pktlen;
        rxq.front[1]    = 0;
        rxq.getcursor   = &rxq.front[2];
        rxq.putcursor   = &rxq.front[2];
        mcsm21[0]      |= b00010000;
    }

    /// 4b. Setup RX for Foreground detection (normal sync & timeout)
    else {
#       if ((M2_FEATURE(MULTIFRAME) == ENABLED) || (M2_FEATURE(FECRX) == ENABLED))
            ot_u8 auto_flag;
            // Initial state on CC430 could be RXMFP (0), RXPAGE (1), RXAUTO (2)
            auto_flag       = ((cc430.flags & RADIO_FLAG_AUTO) != 0);
            cc430.state     = ((cc430.flags & RADIO_FLAG_FRCONT) == 0);
            cc430.state    += auto_flag;
            cc430.flags    |= (auto_flag << 3);     // sets RADIO_FLAG_RESIZE
            cc430.rxlimit   = (auto_flag) ? 48 : 8; ///@todo 48 is a magic-number
            buffer_mode     = 2 - auto_flag;
#       else
            // Initial state is always RXAUTO (2), because no FEC or Multiframe RX
            cc430.state     = RADIO_STATE_RXAUTO;
            buffer_mode     = 1;
#       endif
            pktlen          = 255;
            sync_type       = SYNC_fg;
    }

    /// 5.  Configure CC1101 for FG or BG receiving
    subcc430_buffer_config(buffer_mode, pktlen);
    subcc430_syncword_config(sync_type);
    RF_WriteSingleReg(RFREG(FIFOTHR), (ot_u8)((cc430.rxlimit >> 2) - 1));
    RF_WriteSingleReg(RFREG(AGCCTRL2), phymac[0].cs_thr);
    RF_WriteBurstReg(RFREG(MCSM2), mcsm21, 2);

    /// 6.  Prepare Decoder to receive, then receive
    em2_decode_newpacket();
    em2_decode_newframe();
    subcc430_offset_rxtimeout();     // if timeout is 0, set it to a minimal amount

    /// 7.  Using rm2_reenter_rx() with NULL forces entry into rx, and sets states
    rm2_reenter_rx(NULL);
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
/// are presently in the radio core.  Always do this when forced (callback == NULL).
    if (callback == NULL) {
        radio_idle();
        goto rm2_reenter_rx_PROC;
    }

    if (RF_GetMARCState() < 3) {
    rm2_reenter_rx_PROC:
        radio_flush_rx();
        cc430_iocfg_listen();
        RF_CmdStrobe( RF_CoreStrobe_RX );
        radio.state = RADIO_Listening;
        cc430.state = RADIO_STATE_RXINIT;
    }
}
#endif


#ifndef EXTF_rm2_resend
void rm2_resend(ot_sig2 callback) {
    radio.evtdone               = callback;
    cc430.state                 = RADIO_STATE_TXSTART;
    txq.options.ubyte[UPPER]    = 255;
    rm2_txcsma_isr();
}
#endif


#ifndef EXTF_rm2_kill
void rm2_kill() {
    radio_idle();
    subcc430_finish(RM2_ERR_KILL, 0);
}
#endif



#ifndef EXTF_rm2_rxinit
void rm2_rxinit(ot_u8 channel, ot_u8 psettings, ot_sig2 callback) {
#if (SYS_RECEIVE == ENABLED)
    ot_u8 netstate;

    /// Setup the RX engine for Foreground Frame detection and RX.  Wipe-out
    /// the lower flags (non-persistent flags)
    radio.evtdone   = callback;
    cc430.flags    &= (RADIO_FLAG_SETPWR | RADIO_FLAG_AUTOCAL);

    if (psettings == 0) {
        netstate    = (M2_NETSTATE_UNASSOC | M2_NETFLAG_FIRSTRX);
        cc430.flags|= RADIO_FLAG_FLOOD;
    }
    else {
        netstate    = session_netstate();
#       if (M2_FEATURE(MULTIFRAME) == ENABLED)
        cc430.flags |= ((netstate & M2_NETSTATE_DSDIALOG) != 0); //sets RADIO_FLAG_FRCONT
#       endif
    }

    subcc430_launch_rx(channel, netstate);

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
    if (subcc430_lowrssi_reenter() == 0) {
        cc430_iocfg_rxdata();
        radio.state = RADIO_DataRX;
        dll_block();
    }
}
#endif



#ifndef EXTF_rm2_rxtimeout_isr
void rm2_rxtimeout_isr() {
    radio_idle();
    subcc430_finish(RM2_ERR_TIMEOUT, 0);
}
#endif




#ifndef EXTF_rm2_rxdata_isr
void rm2_rxdata_isr() {
#if (SYS_RECEIVE == ENABLED)
    subcc430_lowrssi_reenter();

    rm2_rxdata_isr_TOP:

    /// 1. load data
    //RFGET_RXDATA();         // Only needed w/ internal DMA usage to set buffer params
    em2_decode_data();      // Contains logic to prevent over-run

    /// 2. Handle each RX type, and transitions
    switch ((cc430.state >> RADIO_STATE_RXSHIFT) & (RADIO_STATE_RXMASK >> RADIO_STATE_RXSHIFT)) {

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
                cc430.state = RADIO_STATE_RXPAGE;
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
#       if ((M2_FEATURE(MULTIFRAME) == ENABLED) || (M2_FEATURE(FECRX) == ENABLED))
        case (RADIO_STATE_RXPAGE >> RADIO_STATE_RXSHIFT): {
            if (em2_remaining_bytes() <= cc430.rxlimit) {
                // Put into DONE state
                cc430.state     = RADIO_STATE_RXDONE;
                cc430.flags    &= ~RADIO_FLAG_RESIZE;

                // kill RX full interrupt, only use packet done (prob unnecessary)
                cc430_iocfg_rxend();

                // transition to fixed length mode
                cc430.rxlimit   = em2_remaining_bytes();
                subcc430_buffer_config(0, cc430.rxlimit);
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
#   if (M2_FEATURE(FECRX) == ENABLED)
        if (cc430.flags & RADIO_FLAG_RESIZE) {
            cc430.flags ^= RADIO_FLAG_RESIZE;
            RF_WriteSingleReg(RFREG(FIFOTHR), (ot_u8)((cc430.rxlimit>>2)-1));
        }
#   endif

#endif
}
#endif




#ifndef EXTF_rm2_rxend_isr
void rm2_rxend_isr() {
    cc430.state = RADIO_STATE_RXDONE;           // Make sure in DONE State, for decoding
    em2_decode_data();                          // decode any leftover data
    subcc430_finish(0, (ot_int)crc_check() - 1);
}
#endif




#ifndef EXTF_rm2_txinit
void rm2_txinit(ot_u8 psettings, ot_sig2 callback) {
#if (SYS_FLOOD == ENABLED)
    cc430.flags    |= (psettings != 0) ? RADIO_FLAG_FLOOD : 0;
#endif
#if (M2_FEATURE(MULTIFRAME) == ENABLED)
    cc430.flags    |= ((session_netstate() & M2_NETSTATE_DSDIALOG) != 0); //sets RADIO_FLAG_FRCONT
#endif
    radio.evtdone   = callback;
    radio.state     = RADIO_Csma;
    cc430.state     = RADIO_STATE_TXINIT;

    /// @note Flush TX FIFO and set buffer threshold to 5 bytes: the encoder
    /// should be at least 20% faster than the max TX data speed (1 byte per
    /// 40 �s).  On CC430, FEC encode is possible with 20 MHz CPU.
    radio_flush_tx();
    RF_WriteSingleReg(RFREG(FIFOTHR), DRF_FIFOTHR | _FIFO_TXTHR(5) );

    /// CSMA-CA interrupt based and fully pre-emptive.  This is
    /// possible using CC1 on the GPTIM to clock the intervals.
    radio_set_mactimer( (ot_uint)dll.comm.tca );
}
#endif




#ifndef EXTF_rm2_txstop_flood
void rm2_txstop_flood() {
#if (SYS_FLOOD == ENABLED)
    cc430.state = RADIO_STATE_TXDONE;
    RF_CoreITConfig(RF_CoreIT_TXBelowThresh, DISABLE);  //RFCONFIG_TXFIFOLOW_INTOFF();
#endif
}
#endif




#ifndef EXTF_rm2_txcsma_isr
void rm2_txcsma_isr() {
    // The shifting in the switch is so that the numbers are 0, 1, 2, 3...
    // It may seem silly, but it allows the switch to be compiled better.
    switch ( (cc430.state >> RADIO_STATE_TXSHIFT) & (RADIO_STATE_TXMASK >> RADIO_STATE_TXSHIFT) ) {

        /// 1. First CCA: Note case fall-through on CSMA activation
        case (RADIO_STATE_TXINIT >> RADIO_STATE_TXSHIFT): {
            if (subcc430_chanscan() == False) {    //Get usable channel
                radio.evtdone(RM2_ERR_BADCHANNEL, 0);
                break;
            }
            if (dll.comm.csmaca_params & M2_CSMACA_NOCSMA) {
                goto rm2_txcsma_START;
            }
        }

        /// 2. Do First CCA check
        ///    subcc430_ccascan_isr() must establish state upon CCA result.
        ///    It can do this by doing cc430.state += (1<<RADIO_STATE_TXSHIFT),
        ///    which will put CCA from CCA1->CCA2 or CCA2->TXSTART
        case (RADIO_STATE_TXCCA1 >> RADIO_STATE_TXSHIFT): {
        //    cc430.state = RADIO_STATE_TXCCA1;
        //    subcc430_ccascan();
        //    break;
        }

        /// 3. CCA1 succeeded: do second check after waiting the guard time
        case (RADIO_STATE_TXCCA2 >> RADIO_STATE_TXSHIFT): {
        //    cc430.state = RADIO_STATE_TXCCA2;
        //    if (phymac[0].tg > 2) {
        //        radio_sleep();
        //    }
        //    radio_set_mactimer( phymac[0].tg );
        //    break;
        }

        /// 3. TX startup
        /// Now that an available channel is found, set it up to TX.  Initially
        /// load a rather small amount of data, because after CSMA now
        /// the system is just burning energy in IDLE.
        case (RADIO_STATE_TXSTART >> RADIO_STATE_TXSHIFT): {
        rm2_txcsma_START:
            // Set TX PATABLE now that channel is known
            if (cc430.flags & RADIO_FLAG_SETPWR) {
                cc430.flags &= ~RADIO_FLAG_SETPWR;
                subcc430_set_txpwr( phymac[0].tx_eirp );
            }

#           if (M2_FEATURE(SUBCONTROLLER) || M2_FEATURE(GATEWAY))
            {   sync_enum   sync_type   = SYNC_fg;
                ot_u8       mcsm1       = (_CCA_MODE_ALWAYS | _RXOFF_MODE_IDLE | _TXOFF_MODE_IDLE);
                ot_u8       buffer_mode = 1;
                ot_u8       buffer_size = 255;

                if (cc430.flags & RADIO_FLAG_FLOOD) {
                    sync_type   = SYNC_bg;
                    mcsm1       = (_CCA_MODE_ALWAYS | _RXOFF_MODE_IDLE | _TXOFF_MODE_TX);
                    buffer_mode = 0;
#                   if (M2_FEATURE(FECTX) == ENABLED)
                        buffer_size = (phymac[0].channel & 0x80) ? 16 : 7;
#                   else
                        buffer_size = 7;
#                   endif
                }
                radio.evtdone(0, buffer_mode-1);  // arg2: !0 for background, 0 for foreground
                subcc430_buffer_config(buffer_mode, buffer_size);
                subcc430_syncword_config( sync_type );
                RF_WriteSingleReg(RFREG(MCSM1), mcsm1);
            }
#           else
                radio.evtdone(0, 0);            // arg2: !0 for background, 0 for foreground
                subcc430_buffer_config(1, 255);
                subcc430_syncword_config( SYNC_fg );
                //cc430_write(RFREG(MCSM1), b00000000 );   //should be persistent default
#           endif

            // Preload into TX FIFO a relatively small amount (8 bytes) for min
            // latency.  Amount should be multiple of 4, (> 5), and small.
            cc430.txlimit   = 8;
            txq.getcursor   = txq.front;
            txq.front[1]    = phymac[0].tx_eirp;
            subcc430_prep_q(&txq);
            //radio_flush_tx();
            em2_encode_newpacket();
            em2_encode_newframe();
            em2_encode_data();

            // Put state into TX Data, and TXlimit to maximum (after preloading)
            radio.state     = RADIO_DataTX;
            cc430.state     = RADIO_STATE_TXDATA;
            cc430.txlimit   = RADIO_BUFFER_TXMAX;

            // Known Erratum that device must be in IDLE before TX.
            radio_idle();
            cc430_iocfg_txdata();
            RF_CmdStrobe( RF_CoreStrobe_TX );
            break;
        }
    }
}
#endif





#ifndef EXTF_rm2_txdata_isr
void rm2_txdata_isr() {
    /// Continues where rm2_txcsma() leaves off.
    switch ( (cc430.state >> (RADIO_STATE_TXSHIFT+1)) & (RADIO_STATE_TXMASK >> (RADIO_STATE_TXSHIFT+1)) ) {

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
            if (cc430.flags & RADIO_FLAG_FLOOD) {
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
            if (cc430.flags & RADIO_FLAG_FRCONT) {
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

            cc430.state = RADIO_STATE_TXDONE;
            RFWord->IFG = 0;
            RFWord->IE  = (ot_u16)(RF_CoreIT_EndState >> 16);
            RFWord->IES = (ot_u16)(RF_CoreIT_EndState);
            break;
        }

        /// 5. Conclude the TX process, and wipe the radio state
        //     turn off any remaining TX interrupts
        //case (RADIO_STATE_TXDONE >> (RADIO_STATE_TXSHIFT+1)):
            //radio_idle();         //should be in idle already
        //    subcc430_finish(0, 0);
        //    break;

        /// Bug trap
        default:
            rm2_kill();
            break;
    }
}
#endif


//void sub_txend_isr() {
    //radio_idle();         //should be in idle already
//  subcc430_finish(0, 0);
//}




/** Radio Subroutines
  * ============================================================================
  * - Usually some minor adjustments needed when porting to new platform
  * - See integrated notes for areas sensitive to porting
  */

void subcc430_null(ot_int arg1, ot_int arg2) { }



void subcc430_finish(ot_int main_err, ot_int frame_err) {
    ot_sig2 callback;

    /// 1. Turn-off interrupts, reset autocalibration flag
    radio_gag();
    //radio_idle();
    subcc430_reset_autocal();

    /// 2. Reset radio & callback to null state, then run saved callback
    radio.state     = RADIO_Idle;
    cc430.state     = 0;
    cc430.flags    &= RADIO_FLAG_SETPWR;    //clear all other flags
    callback        = radio.evtdone;
    radio.evtdone   = &otutils_sig2_null;
    callback(main_err, frame_err);
}


ot_bool subcc430_lowrssi_reenter() {
    //ot_int  min_rssi;
    //min_rssi = ((phymac[0].cs_thr >> 1) & 0x3F) - 40;

    //if (radio_rssi() < min_rssi) {
    //    subcc430_reset_autocal();
    //    rm2_reenter_rx(NULL);
    //    return True;
    //}
    return False;
}


void subcc430_reset_autocal() {
    if (cc430.flags & RADIO_FLAG_AUTOCAL) {
        cc430.flags ^= RADIO_FLAG_AUTOCAL;
        RF_WriteSingleReg(RFREG(MCSM0), DRF_MCSM0);
    }
}



ot_bool subcc430_chanscan( ) {
    vlFILE* fp;
    ot_int  i;

    fp = ISF_open_su( ISF_ID(channel_configuration) );
    ///@todo assert fp

    /// Go through the list of tx channels
    /// - Make sure the channel ID is valid
    /// - Make sure the transmission can fit within the contention period.
    /// - Scan it, to make sure it can be used
    for (i=0; i<dll.comm.tx_channels; i++) {
        if (subcc430_channel_lookup(dll.comm.tx_chanlist[i], fp) != False) {
            break;
        }
    }

    vl_close(fp);
    return (ot_bool)(i < dll.comm.tx_channels);
}





void subcc430_ccascan() {
/// Called indirectly by other subroutines
/// @todo this is the only blocking call.  It can cause problems, being
/// blocking, so one day I would like to make it non-blocking.

/// @note CCA scan is a blocking call to the radio core, which waits for radio
/// RSSI to be declared valid (uses CC430 RSSI_Valid interrupt).  Alternatively,
/// a bipolar setup of CS/CCA interrupts could be used if you are porting to
/// CC1xxx and don't have the RSSI_Valid interrupt, but RSSI_Valid is more
/// reliable.  Then we just compare returned RSSI's with the stored limits.

    //send radio to idle mode: It is a known erratum that CC430 requires
    //transition from IDLE->RX/TX, rather than SLEEP->RX/TX (as written in
    //Users' Guide), because TEST0 must be written in IDLE.
    radio_idle();

    /// Turn-on RX, then wait for RSSI_Valid interrupt
    /// @note: kernel will always be paused during this time
    cc430_iocfg_txcsma();
    RF_CmdStrobe( RF_CoreStrobe_RX );
}


void subcc430_ccascan_isr() {
    ot_u8 cca_status;

    /// Turn-off CSMA mode, compare RSSI value
    /// See rm2_txcsma_isr() for info why cca_status is handled as such
    RFWord->IE  = 0;
    cca_status  = radio_check_cca() << (RADIO_STATE_TXSHIFT);
    cc430.state+= cca_status;

    /// Go back to idle and remove calibration requirement, as the channel has
    /// now been calibrated by virtue of going into CSMA.
    radio_idle();
    subcc430_reset_autocal();

    /// If cca failed, use the DLL callback to establish a back-off until CCA
    /// Tries again.  It must set the radio mactimer accordingly, based on
    /// whatever CSMA algorithm it is using.
    if (cca_status == 0) {
        cc430.state = RADIO_STATE_TXCCA1;
        radio.evtdone(1, 0);
    }

    /// If cca succeeds, call TXCSMA
    else {
        rm2_txcsma_isr();
    }
}





ot_bool subcc430_channel_lookup(ot_u8 chan_id, vlFILE* fp) {
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
                i   = (cc430.state & RADIO_STATE_RXMASK);
#           endif
#           if (M2_FEATURE(FECTX) == ENABLED)
                i  |= (cc430.state & RADIO_STATE_TXMASK);
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
            phymac[0].cs_thr    = subcc430_rssithr_calc(phymac[0].cs_thr, RF_CSTHR_OFFSET);
            //phymac[0].cca_thr   = subcc430_rssithr_calc(phymac[0].cca_thr, RF_CCATHR_OFFSET);

            subcc430_chan_config(old_chan_id, old_tx_eirp);
            return True;
        }
    }

    return False;
}




void subcc430_chan_config(ot_u8 old_chan, ot_u8 old_eirp) {
/// Called by subcc430_channel_lookup()
/// Duty: perform channel setup and recalibration when moving from one channel
/// to another.
    ot_u8 mdmcfg[4] = {DRF_MDMCFG4, DRF_MDMCFG3, DRF_MDMCFG2, DRF_MDMCFG1};
    ot_u8 fc_i;

#   if (M2_FEATURE(FEC) == ENABLED)
    /// Only worry about changing encoding method if:
    /// (a) Optional FEC is enabled (otherwise only one kind of encoding)
    /// (b) The last-used channel had a different encoding type

        if ( (old_chan ^ phymac[0].channel) & 0x80 ) {
            ot_u8 pktctrl1_val = b00100000;

            if (phymac[0].channel & 0x80) {
                /// Set up transceiver for FEC
                /// set Preamble qualifier on the low side (eg PQT=4 on CC430)
                /// set Sync Word qualifier (MDMCFG2) to allow a bit error
                mdmcfg[2]      -= 1;
            }
            else {
                /// Set up transceiver for PN9 method
                /// If FEC is disabled, these settings will always be in the regs.
                ///set FEC to OFF and PN9 to ON
                ///set Preamble qualifier on the high side (eg PQT=8 on CC430)
                ///set Sync qualifier to allow no bit errors (16/16)
                pktctrl1_val   += b00100000;
                cc430.flags    |= RADIO_FLAG_AUTO;
            }

            RF_WriteSingleReg(RFREG(PKTCTRL1), pktctrl1_val);

            /// @note On CC430 register PKTCTRL0:
            /// Pktctrl0 is adjusted later, by subcc430_buffer_config, during RX/TX
            /// Pktctrl0 contains the bit that turns on CC430 PN9 HW.
        }
#   endif

    /// Reprogram the PA Table if eirp of new channel isn't the same as before
    if (old_eirp != phymac[0].tx_eirp) {
        cc430.flags |= RADIO_FLAG_SETPWR;
    }

    /// @note Settings of "mdmcfg..." are CC430 specific implementation
    /// Reprogram data rate, packet method, and modulation per upper nibble
    /// (Don't reprogram if radio is using the same channel class already)
    if ( (old_chan ^ phymac[0].channel) & 0xF0 ) {
        if (phymac[0].channel & 0x20) {
            mdmcfg[0]   = DRF_MDMCFG4_HI;
            mdmcfg[1]   = DRF_MDMCFG3_HI;
            mdmcfg[3]  |= _NUM_PREAMBLE_6B;         //6B preamble on Hi-speed
        }
        RF_WriteBurstReg(RFREG(MDMCFG4), mdmcfg, 4);
    }

    /// @note: CC430 contains a channel-offset built-in mechanism.  On chips
    /// without this, just change the center frequency.  The simplest way is to
    /// have a lookup table of center frequency settings mapped to fc_i.
    /// - Reprogram channel, and stage recalibration
    /// - But don't do these things if radio is already set on this center channel
    fc_i = (phymac[0].channel & 0x0F);          // Center Frequency index = lower four bits channel ID
    if ( fc_i != (old_chan & 0x0F) ) {
        RF_WriteSingleReg(RFREG(CHANNR), (ot_u8)fc_i); //RFCONFIG_FC(fc_i);
        RF_WriteSingleReg(RFREG(MCSM0), _FS_AUTOCAL_FROMIDLE);
        //radio_calibrate();  //Manual Calibrate
        cc430.flags |= RADIO_FLAG_AUTOCAL;
    }
}




void subcc430_syncword_config(ot_u8 sync_class) {
///@note The MSP430 core is little endian, and CC1101 core is big endian, so
/// syncwords might need to be twiddled.
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

    RF_WriteBurstReg(RFREG(SYNC1), (ot_u8*)(sync_matrix+sync_class), 2);
}




void subcc430_buffer_config(ot_u8 mode, ot_u8 param) {
/// CC430 specific implementation.  This function selects between three
/// buffering modes: FIFO buffering (2), automatic packet handling (1), or
/// fixed-length packet (0).  The "param" argument is a chip-specific option.
#   if (M2_FEATURE(FEC) == ENABLED)
    if ((phymac[0].channel & 0x80) == 0)
#   endif
        mode |= 0x40;

    RF_WriteSingleReg(RFREG(PKTLEN), param);
    RF_WriteSingleReg(RFREG(PKTCTRL0), mode);
}




void subcc430_prep_q(ot_queue* q) {
/// Put some special data in the queue options field.
/// Lower byte is encoding options (i.e. FEC)
/// Upper byte is processing options (i.e. CRC)
    q->options.ubyte[LOWER]    = (phymac[0].channel & 0x80);
    q->options.ubyte[UPPER]   += 1;
    //q->options.ubyte[UPPER]    = (RF_FEATURE(PACKET) != ENABLED) || \
                                  (RF_FEATURE(CRC) != ENABLED) || \
                                  ( (RF_FEATURE(PN9) != ENABLED) && \
                                    (q->options.ubyte[LOWER] == 0)) || \
                                  ( (RF_FEATURE(FEC) != ENABLED) && \
                                    (q->options.ubyte[LOWER] == 1));
}




ot_int subcc430_eta(ot_int next_int) {
/// Subtract by a tolerance amount, floor at 0
    ot_int eta = rm2_scale_codec(next_int) - 1;
    return (eta > 0) ? eta : 0;
}


ot_int subcc430_eta_rxi() {
    return subcc430_eta( RFGET_RXFIFO_NEXTINT() );
}

ot_int subcc430_eta_txi() {
    return subcc430_eta( RFGET_TXFIFO_NEXTINT() );
}





void subcc430_offset_rxtimeout() {
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




ot_u8 subcc430_rssithr_calc(ot_u8 input, ot_u8 offset) {
/// @note This function is CC430/CC1101-specific.  If you are not using auto CS
/// features on the CC or another chip, you can just return input.

/// This function must prepare any hardware registers needed for automated
/// CS/CCA threshold value.
/// - "input" is a value 0-127 that is: input - 140 = threshold in dBm
/// - "offset" is a value subtracted from "input" that depends on chip impl
/// - return value is chip-specific threshold value

// cs_lut table specs:
// Base dBm threshold of CCA/CS is ~-92 dBm at 200 kbps or ~-96 dBm at 55 kbps.
// Each incremented byte in the array is roughly an additional 2 dB above the base dBm threshold.
/// @todo !!!elements 8 and 11 in lut are the same, this can't be right!!!
    static const ot_u8 cs_lut[18] = {
        b00000011, b00001011, b00001011, b01000011, b01001011, b00100011, b10000011, b10001011,
        b10100011, b11000011, b11001011, b10100011, b11010011, b11011011, b11100011, b11101011,
        b11110011, b11111011
    };

    ot_int thr;
    thr     = (ot_int)input - offset;               // subtract dBm encoding offset
    thr   >>= 1;                                    // divide by two (array is 2dB increments)

    if (phymac[0].channel & 0x60)   thr -= 4;       // (1) account for turbo vs normal rate
    if (thr > 17)                   thr = 17;       // (2) make max if dBm threshold is above range
    else if (thr < 0)               thr = 0;        // (3) make 0 if dBm threshold is lower than range

    return cs_lut[thr];
}





void subcc430_set_txpwr( ot_u8 eirp_code ) {
/// Sets the tx output power.
/// "pwr_code" is a value, 0-127, that is: eirp_code/2 - 40 = TX dBm
/// i.e. eirp_code=0 => -40 dBm, eirp_code=80 => 0 dBm, etc
///
/// CC430 has an 8-value amplifier table.  TX power gets ramped-up, step by
/// step of the table.  Each value in the amp table is a power code specific
/// to the CC (from the lut).
///
/// CC430-specific power selector table (needs further validation)
/// The index value is pwr_code.  As you may notice, pwr_code does not map
/// linearly to TX dBm on CC, which is why there has to be a table.
    static const ot_u8 pa_lut[100] = {
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,     //-40 to -35.5 dBm
        0x01, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x04,     //-35 to -30.5
        0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x07,     //-30 to -25.5
        0x07, 0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0C, 0x0D,     //-25 to -20.5
        0x0E, 0x0E, 0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1C,     //-20 to -15.5
        0x1D, 0x1E, 0x1E, 0x1F, 0x24, 0x24, 0x33, 0x33, 0x25, 0x25,     //-15 to -10.5
        0x26, 0x26, 0x27, 0x27, 0x35, 0x28, 0x29, 0x36, 0x2A, 0x37,     //-10 to -5.5
        0x8F, 0x8F, 0x57, 0x56, 0x56, 0x55, 0x54, 0x53, 0x52, 0x51,     //-5 to -0.5
        0x60, 0x50, 0x50, 0x8D, 0x8C, 0x8B, 0x8A, 0x88, 0x87, 0x86,     //0 to 5
        0x85, 0x83, 0x81, 0xCA, 0xC9, 0xC7, 0xC6, 0xC4, 0xC3, 0xC1      //5 to 9.5
    };

    ot_u8   pa_table[8];
    ot_int  i;
    ot_int  eirp_val;

// Should be defined in board config file
// Depends on matching circuit, antenna, and board physics
// Described in 0.5 dB units (12 = 6dB system loss)
#   ifndef RF_HDB_ATTEN
#       define RF_HDB_ATTEN 12
#   endif

    eirp_val    = eirp_code + 12;
    eirp_val    = (eirp_val < 99) ? eirp_val : 99;

    for(i=7; (i>=0) && (eirp_val>=0); i--, eirp_val-=6) {
        pa_table[i] = pa_lut[eirp_val];
    }
    i++;

    RF_WriteBurstPATable(&pa_table[i], (ot_u8)(8-i));
    RF_WriteSingleReg( RFREG(FREND0), (DRF_FREND0 | (ot_u8)(7-i)) );
}



