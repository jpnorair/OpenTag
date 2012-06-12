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
  * @file       /Platforms/CC430/radio_CC430.c
  * @author     JP Norair
  * @version    V1.0
  * @date       13 Oct 2011
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

#include "CC430_defaults.h"      // register definitions file




/** Radio Buffer Allocation constants
  * CC430 has internal buffer
  */
#define BUFFER_ALLOC         0


// Change these settings depending on your radio buffer
#define RADIO_BUFFER_TXMIN      RADIO_BUFFER_TXMAX
#define RADIO_BUFFER_RXMIN      RADIO_BUFFER_RXMAX




/** Internal Radio States
  *
  * b5:3        b2:0
  * TX States   RX States
  */
#define RADIO_STATE_RXSHIFT     0
#define RADIO_STATE_RXMASK      (3 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXINIT      (4 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXMFP       (0 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXPAGE      (1 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXAUTO      (2 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXDONE      (3 << RADIO_STATE_RXSHIFT)

#define RADIO_STATE_TXSHIFT     3
#define RADIO_STATE_TXMASK      (7 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXINIT      (1 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXCCA1      (2 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXCCA2      (3 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXSTART     (4 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXDATA      (5 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXDONE      (6 << RADIO_STATE_TXSHIFT)

/** Internal Radio Flags (lots of room remaining)
*/
#define RADIO_FLAG_FRCONT       (1 << 0)
#define RADIO_FLAG_FLOOD        (1 << 1)
#define RADIO_FLAG_AUTO         (1 << 2)
#define RADIO_FLAG_RESIZE       (1 << 3)


/** Internal Radio Interrupt Flags
  * For performance reasons, sometimes interrupt flags will be stored locally
  * and used in later conditionals.  The usage is implementation dependent.
  */
#define RADIO_INT_CCA           (1 << 3)
#define RADIO_INT_CS            (1 << 4)



/** Radio HW Timing Parameters
  * Measurements for Radio HW performance, usually corresponding to timings
  * as the radio goes from one state to the next
  *
  * Reminder: sti is "short tick"
  * 1 sti = 32 ti = 1/32768 sec = ~30.52 us
  */
#define RADIO_COLDSTART_STI_MAX     27
#define RADIO_COLDSTART_STI_TYP     9
#define RADIO_WARMSTART_STI         3
#define RADIO_CALIBRATE_STI         24
#define RADIO_FASTHOP_STI           5
#define RADIO_KILLRX_STI            0
#define RADIO_KILLTX_STI            0
#define RADIO_TURNAROUND_STI        1




/** PHY-MAC Array declaration
  * Described in radio.h of the OTlib.
  * This driver only supports M2_PARAM_MI_CHANNELS = 1.
  */
phymac_struct   phymac[M2_PARAM_MI_CHANNELS];




/** Radio Module Data
  * Data that is useful for the internal use of this module.  The list below is
  * comprehensive, and it may not be needed in entirety for all implementations.
  * For implementations that don't use the values, comment out.
  *
  * state       Radio State, used with multi-call functions (RX/TX)
  * flags       A local store for usage flags
  * txlimit     An interrupt/event comes when tx buffer gets below this number of bytes
  * rxlimit     An interrupt/event comes when rx buffer gets above this number of bytes
  * last_rssi   Experimental, used to buffer the last-read RSSI value
  * evtdone     A callback that is used when RX or TX is completed (i.e. done)
  * txcursor    holds some data about tx buffer position (MCU-based buffer only)
  * rxcursor    holds some data about rx buffer position (MCU-based buffer only)
  * buffer[]    buffer data.  (MCU-based buffer only)
  * last_rssi   The most recent value of the rss (not always needed)
  */
typedef struct {
    ot_u8   state;
    ot_u8   flags;
    ot_int  txlimit;
    ot_int  rxlimit;
//  ot_int  last_rssi;
    ot_sig2 evtdone;
#   if (BUFFER_ALLOC > 0)
        ot_int  txcursor;
        ot_int  rxcursor;
        ot_u8   buffer[BUFFER_ALLOC];
#   endif
} radio_struct;

radio_struct radio;




/** Local Subroutine Prototypes  <BR>
  * ========================================================================<BR>
  */
void    sub_kill(ot_int main_err, ot_int frame_err);
void    sub_killonlowrssi();

ot_u8   sub_rssithr_calc(ot_u8 input, ot_u8 offset); // Fiddling necessary
ot_bool sub_cca_init();
ot_bool sub_cca2();
ot_bool sub_chan_scan( ot_bool (*scan_test)() );
ot_bool sub_noscan();
ot_bool sub_cca_scan();
ot_bool sub_csma_init();
ot_bool sub_nocsma_init();

ot_bool sub_channel_lookup(ot_u8 chan_id, vlFILE* fp);
void    sub_syncword_config(ot_u8 sync_class);
void    sub_buffer_config(ot_u8 mode, ot_u8 param);
void    sub_chan_config(ot_u8 old_chan, ot_u8 old_eirp);

void    sub_set_txpower(ot_u8 eirp_code);
void    sub_prep_q(Queue* q);
ot_int  sub_eta(ot_int next_int);
ot_int  sub_eta_rxi();
ot_int  sub_eta_txi();
void    sub_offset_rxtimeout();






/** CC430 RF Core Interrupt Handler  <BR>
  * ========================================================================<BR>
  */

#if (CC_SUPPORT == CL430)
#   pragma vector=CC1101_VECTOR
#elif (CC_SUPPORT == IAR_V5)
    //unknown at this time
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(CC1101_VECTOR)
#	endif
OT_INTERRUPT void radio_isr(void) {
    u16 core_edge;
    u16 core_vector;

    /// 1.  Interface Interrupt Handler:
    ///     Primarily for Debugging, since OpenTag doesn't use any of these.
    ///     Memscope onto RFWord->IFIV and RF_IFIT_ERR to get more information if
    ///     your code segfaults here.
    if ((RFWord->IFIV & 0x0E) != 0) {
    	SFRIFG1 |= NMIIFG;	//call a segmentation fault
#       if 0
            /// Here is the max functionality of the RF interface handler.  It
            /// is commented-out for the meantime.  Maybe later I will find
            /// something good to do with it.  For now, I just ignore.
//            switch  {
//                case 0x00:  break;
//                case 0x02:  switch (RFWord->IFERRV) {     // error traps -- avoid these by clearing above
//                                case 0x02: break;     // low core voltage error
//                                case 0x04: break;     // operand error
//                                case 0x06: break;     // output data not available error
//                                case 0x08: break;     // operand overwrite error
//                            }
//                            RF_ClearIFITPendingBit( RF_IFIT_ERR );
//                            break;
//
//                case 0x04:  RF_ClearIFITPendingBit( RF_IFIT_DOUT );     break;
//                case 0x06:  RF_ClearIFITPendingBit( RF_IFIT_STAT );     break;
//                case 0x08:  RF_ClearIFITPendingBit( RF_IFIT_DIN );      break;
//                case 0x0A:  RF_ClearIFITPendingBit( RF_IFIT_INSTR );    break;
//                case 0x0C:  RF_ClearIFITPendingBit( RF_IFIT_TX );       break;
//                case 0x0E:  RF_ClearIFITPendingBit( RF_IFIT_RX );       break;
//            }

            //RF_ClearIFITPendingBit( RF_IFIT_ALL );
#       endif
    }



    /// 2.  Core Interrupt Handler:
    ///     Several Core interrupts are used by the OpenTag implementation.
    ///     - GDO2: RF_Ready, used by init routine (blocking)
    ///     - GDO1: RSSI_Valid, used by CSMA/CCA process (blocking)
    ///     - GDO0: RX Timeout, used by RX fore+background detection (non-blocking)
    ///     - Core: SyncWord, used by RX fore+background detection (non-blocking)
    ///     - Core: CarrierSense, used by RX background detection (non-blocking)
    ///     - Core: RXFullOrDone, used by RX process (non-blocking)
    ///     - Core: TXBelowThresh, used by TX process (non-blocking)
    ///     - Core: TXUnderflow, used by TX process (non-blocking)

    /* The following lines are a little confusing.  The CC430 RF Core interrupt
       mechanism is a bit strange, as the meaning of an interrupt is dependent
       on the interrupt flag (obviously) but also the setting of the edge
       detector.  Rising and falling edges generate the same flag bit, but have
       different meanings.

       1. Get the flag vector (it has values 0 - 0x20, evens only)
       2. Convert flag vector to edge-select mask bit
       3. Compare the flag location with the edge setting
       4. If the edge setting is "falling," extend the vector.
    */
    core_vector     = RFWord->IV;                           //1
    core_edge       = 1 << (core_vector >> 1);          //2
    core_edge     >>= 1;                                //2
    core_edge      &= RFWord->IES;                          //3
    core_vector    += (core_edge) ? 0x22 : 0;           //4

    switch (core_vector) {
        // Rising Edges
        //case 0x00:  break;
        case 0x02:  rm2_rxtimeout_isr();    break;  //IOCFG0_ISR();
        //case 0x04:  __no_operation();       break;  //IOCFG1_ISR();
        //case 0x06:  __no_operation();       break;  //IOCFG2_ISR();
        case 0x08:  rm2_rxdata_isr();       break;  //RXFull_ISR();
        //case 0x0A:  rm2_rxdata_isr();       break;  //RXFullOrDone_ISR();
        //case 0x0C:  __no_operation();       break;  //TXAboveThresh_ISR();
        //case 0x0E:  __no_operation();       break;  //TXFull_ISR();
        //case 0x10:  __no_operation();       break;  //RXOverflow_ISR();
        case 0x12:  rm2_txdata_isr();       break;  //TXUnderflow_ISR();
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
        //case 0x2C:  __no_operation();       break;  //RXEmpty_ISR();
        case 0x2E:  rm2_txdata_isr();       break;  //TXBelowThresh_ISR();
        //case 0x30:  __no_operation();       break;  //TXNotFull_ISR();
        //case 0x32:  __no_operation();       break;  //RXFlushed_ISR();
        //case 0x34:  __no_operation();       break;  //TXFlushed_ISR();
        case 0x36:
        	if (radio.state & RADIO_STATE_TXMASK)	rm2_txdata_isr();
        	else 									rm2_rxend_isr();
        	break;  //EndState_ISR();

        //case 0x38:  __no_operation();       break;  //RXFirstByte_ISR();
        //case 0x3A:  __no_operation();       break;  //LPW_ISR();
        //case 0x3C:  __no_operation();       break;  //CarrierSense_ISR();
        //case 0x3E:  __no_operation();       break;  //RSSILow_ISR();   //(ClearChannel2)
        //case 0x40:  __no_operation();       break;  //WORevent0ACLK_ISR();
        //case 0x42:  __no_operation();       break;  //OscStable_ISR();

#       ifdef DEBUG_ON
        default:
            // Bug trap: IRQ gone wild
            sub_kill(RM2_ERR_GENERIC, -1);
            break;
#       endif
    }

    //Read to RFWord->IV automatically clears the highest priority pending bit
    //RF_ClearCoreITPendingBit(RF_CoreIT_ALL);
    LPM4_EXIT;  // Clear All Sleep Bits
}







/** Radio Core Control Functions
  * ============================================================================
  * - Need to be customized per radio platform
  */

void radio_off() {
   radio_sleep();   //CC430 is SoC, so sleep is same as off
}


void radio_gag() {
    //RF_CoreITConfig(RF_CoreIT_ALL, DISABLE);
    RFWord->IE = 0;
}


void radio_sleep() {
    RF_CmdStrobe(RF_CoreStrobe_IDLE);
    RF_CmdStrobe(RF_CoreStrobe_PWD);
}


void radio_idle() {
    RF_CmdStrobe(RF_CoreStrobe_IDLE);
}


void radio_calibrate() {
    RF_CmdStrobe(RF_CoreStrobe_CAL);
}







/** Radio Module Control Functions
  * ============================================================================
  * - Need to be customized per radio platform
  */

void radio_init( ) {
/// Transceiver implementation dependent
    static const ot_u8 cc430_defaults[39] = {
         RFREG_IOCFG2,
         RFREG_IOCFG1,
         RFREG_IOCFG0,
         RFREG_FIFOTHR,
         RFREG_SYNC1,
         RFREG_SYNC0,
         RFREG_PKTLEN,
         RFREG_PKTCTRL1,
         RFREG_PKTCTRL0,
         RFREG_ADDR,
         RFREG_CHANNR,
         RFREG_FSCTRL1,
         RFREG_FSCTRL0,
         RFREG_FREQ2,
         RFREG_FREQ1,
         RFREG_FREQ0,
         RFREG_MDMCFG4,
         RFREG_MDMCFG3,
         RFREG_MDMCFG2,
         RFREG_MDMCFG1,
         RFREG_MDMCFG0,
         RFREG_DEVIATN,
         RFREG_MCSM2,
         RFREG_MCSM1,
         RFREG_MCSM0,
         RFREG_FOCCFG,
         RFREG_BSCFG,
         RFREG_AGCCTRL2,
         RFREG_AGCCTRL1,
         RFREG_AGCCTRL0,
         RFREG_WOREVT1,
         RFREG_WOREVT0,
         RFREG_WORCTL,
         RFREG_FREND1,
         RFREG_FREND0,
         RFREG_FSCAL3,
         RFREG_FSCAL2,
         RFREG_FSCAL1,
         RFREG_FSCAL0,
    };

    ///Disable all Interface and Core interrupts on startup init
    RFWord->IFCTL1 = 0;
    RFWord->IE     = 0;

    /// Load default register values to RF Core, starting at the first one.
    RF_WriteBurstReg(RF_CoreReg_IOCFG2, (ot_u8*)&cc430_defaults[0], 39);
    RF_WriteSingleReg(RF_CoreReg_TEST0, RFREG_TEST0);

    /// Set incumbent channel to a completely invalid channel ID and run lookup
    /// on the default channel (0x00) to kick things off.
    {
        vlFILE* fp          = ISF_open_su( ISF_ID(channel_configuration) );
        phymac[0].channel   = 0x55;         // 55=invalid, forces calibration.
        phymac[0].tx_eirp   = 0x00;         // initialized to zero
        radio.state         = 0;            // (idle)
        radio.evtdone       = &otutils_sig2_null;

        sub_channel_lookup(0x00, fp);
        vl_close(fp);
    }
}


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



ot_bool radio_check_cca() {
/// CCA Method 2: Compare stored limit with actual, detected RSSI.  On CC430,
/// this method proves to be more reliable and, amazingly, faster than Method 1.
    ot_int thr  = (ot_int)phymac[0].cca_thr - 140;
    ot_int rssi = radio_rssi();

    return (ot_bool)(rssi < thr);
}




ot_int radio_rssi() {
/// Transceiver implementation dependent
/// CC430 stores the RSSI in a special register, as a 2's complement number, of
/// offset 0.5 dBm units.  This function translates it into normal dBm units.
    ot_s8  rssi_raw;
    ot_int rssi_val;

    rssi_raw    = (ot_s8)RF_GetRSSI();      // CC430 RSSI is 0.5 dBm units, signed byte
    rssi_val    = (ot_int)rssi_raw;         // Convert to signed 16 bit (1 instr on MSP)
    rssi_val   += 128;                      // Make it positive...
    rssi_val  >>= 1;                        // ...So division to 1 dBm units can be a shift...
    rssi_val   -= (64 + RF_RSSIOffset);     // ...and then rescale it, including offset

    return rssi_val;
}




ot_u8 radio_buffer(ot_int index) {
/// Transceiver implementation dependent
/// This function is not used on the CC430, which has a HW FIFO
    return 0;
}




void radio_putbyte(ot_u8 databyte) {
/// Transceiver implementation dependent
    RF_WriteSingleReg(RF_TXFIFOWR, databyte);
}




void radio_putfourbytes(ot_u8* data) {
/// @note Eventually I would like to use an endian-independent implementation,
/// (basically, this is the big endian version) but now is not the time. Doing
/// so will involve revising the FEC encoder in the Encode Module.
#if (M2_FEATURE(FEC) == ENABLED)
#   ifdef __BIG_ENDIAN__
        RF_WriteBurstReg(RF_TXFIFOWR, data, 4);
#   else
        RF_WriteSingleReg(RF_TXFIFOWR, data[3]);
        RF_WriteSingleReg(RF_TXFIFOWR, data[2]);
        RF_WriteSingleReg(RF_TXFIFOWR, data[1]);
        RF_WriteSingleReg(RF_TXFIFOWR, data[0]);
#   endif
#endif
}




ot_u8 radio_getbyte() {
/// Transceiver implementation dependent
    return RF_ReadSingleReg(RF_RXFIFORD);
}




void radio_getfourbytes(ot_u8* data) {
/// @note because the radio is Big Endian (sensible) and the datastream is also
/// big endian (all serial streams are), no conversion is necessary (senisble).
    RF_ReadBurstReg(RF_RXFIFORD, data, 4);
}




void radio_flush_rx() {
/// Transceiver implementation dependent
    RF_CmdStrobe( RF_CoreStrobe_FRX );
}




void radio_flush_tx() {
/// Transceiver implementation dependent
    RF_CmdStrobe( RF_CoreStrobe_FTX );
}



ot_bool radio_rxopen() {
/// Transceiver implementation dependent

/// @note If you are using the CC430 or CC11xx, you will need to make sure not
///       to draw-out the bottom byte in the FIFO until the packet is complete.
///       This is a known erratum of these parts.

    return (ot_bool)(RF_GetRXBYTES() > (radio.state != RADIO_STATE_RXDONE));
}




ot_bool radio_rxopen_4() {
/// Transceiver implementation dependent, only needed with FEC
/// @note If you are using the CC430 or CC11xx, you will need to make sure not
///       to draw-out the bottom byte in the FIFO until the packet is complete.
///       This is a known erratum of these parts.
    ot_int thresh;
    thresh = (radio.state != RADIO_STATE_RXDONE) << 2;

    return (ot_bool)(RF_GetRXBYTES() > thresh);
}




ot_bool radio_txopen() {
/// Transceiver implementation dependent
    return (ot_bool)(RF_GetTXBYTES() < RADIO_BUFFER_TXMAX);
}



ot_bool radio_txopen_4() {
/// Transceiver implementation dependent, only needed with FEC
    return (ot_bool)(RF_GetTXBYTES() < (RADIO_BUFFER_TXMAX-4));
}









/** Radio I/O Functions
  * ============================================================================
  * -
  */

ot_bool sub_test_channel(ot_u8 channel, ot_u8 netstate) {
#if (SYS_RECEIVE == ENABLED)
    ot_bool test = True;

    if ((channel != phymac[0].channel) || (netstate == M2_NETSTATE_UNASSOC)) {
        vlFILE* fp;

        /// Open the Mode 2 FS Config register that contains the channel list
        /// for this host, and make sure the channel we want to use is available
        fp = ISF_open_su( ISF_ID(channel_configuration) );
        ///@todo assert fp

        test = sub_channel_lookup(channel, fp);
        vl_close(fp);
    }

    return test;
#else
    return True;
#endif
}



#if (SYS_RECEIVE == ENABLED)
void subcc430_launch_rx(ot_u8 mcsm2_val, ot_u16 intr_en, ot_u16 intr_eselect ) {
    sub_prep_q(&rxq);
    em2_decode_newpacket();
    em2_decode_newframe();
    sub_offset_rxtimeout();     // if timeout is 0, set it to a minimal amount

    RF_WriteSingleReg(RF_CoreReg_FIFOTHR, (ot_u8)((radio.rxlimit >> 2) - 1));
    RF_WriteSingleReg(RF_CoreReg_AGCCTRL2, phymac[0].cs_thr);
    //CCA now uses direct RSSI, not AGCCTRL, so no reset below
    //RF_WriteSingleReg(RF_CoreReg_AGCCTRL1, b01000111);

#   if (RF_FEATURE(RXTIMER) == ENABLED)
    {
        Twobytes rxtime;
        rxtime.ushort = dll.comm.rx_timeout;
        RF_WriteSingleReg(RF_CoreReg_WOREVT1, rxtime.ubyte[UPPER]);
        RF_WriteSingleReg(RF_CoreReg_WOREVT0, rxtime.ubyte[LOWER]);
    }
#   endif

    RF_WriteSingleReg(RF_CoreReg_MCSM2, mcsm2_val);
    RF_ClearCoreITPendingBit(RF_CoreIT_ALL);
    RFWord->IE  = intr_en;
    RFWord->IES = intr_eselect;
    RF_CmdStrobe( RF_CoreStrobe_RX );
}
#endif


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

    return tgd[chan_id];

#else
#   error "Missing definitions of M2_FEATURE(FEC) and/or M2_FEATURE(TURBO)"
    return 0;
#endif
}





ot_int rm2_pkt_duration(ot_int pkt_bytes) {
/// Wrapper function for rm2_scale_codec that adds some slop overhead
/// Slop = preamble bytes + sync bytes + ramp-up + ramp-down + padding
    pkt_bytes  += RADIO_PKT_OVERHEAD;
    pkt_bytes  += ((phymac[0].channel & 0x60) != 0) << 1;

    return rm2_scale_codec(pkt_bytes);
}

ot_int rm2_scale_codec(ot_int buf_bytes) {
/// Turns a number of bytes (buf_bytes) into a number of ti units.
/// To refresh your memory: 1 ti = ((1sec/32768) * 2^5) = 2^-10 sec = ~0.977 ms

    /// Pursuant to DASH7 Mode 2 spec, b6:4 of channel ID corresponds to kS/s.
    /// 55.555 kS/s = 144us per buffer byte
    /// 200.00 kS/s = 40us per buffer bytes
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



void rm2_prep_resend() {
	txq.options.ubyte[UPPER] = 255;
}


void rm2_kill() {
    sub_kill(RM2_ERR_KILL, 0);
}



void rm2_rxinit_ff(ot_u8 channel, ot_u8 netstate, ot_int est_frames, ot_sig2 callback) {
#if (SYS_RECEIVE == ENABLED)
    /// Use the data from the supplied session to open a channel for RX.
    /// If the channel is already open (even if it is not being used), then
    /// we will skip the opening process
    ///
    /// We use a small FIFO to start with in order to minimize lag time for
    /// packet detection, because a long FIFO can waste energy.  Theoretical
    /// minimum length of a foreground frame is 7 bytes, but compliant min
    /// length is 9 bytes.  So for this driver the initial FIFO is set to 8.
    /// You could set it to 9 if your platform supports odd FIFOs.
    radio.evtdone   = callback;
    radio.state     = RADIO_STATE_RXINIT;
#   if (M2_FEATURE(MULTIFRAME) == ENABLED)
        radio.flags = (est_frames > 1); //sets RADIO_FLAG_FRCONT
#   else
        radio.flags = 0;
#   endif

    /// Make sure channel is supported.  If not, stop now.
    /// sub_test_channel will set RADIO_FLAG_AUTO according to radio abilities
    if (sub_test_channel(channel, netstate) == False) {
        radio.evtdone(RM2_ERR_BADCHANNEL, 0);
    }
    else {
        ot_u8 buffer_mode;
        ot_u8 auto_flag;

        q_empty(&rxq);
        radio_flush_rx();

#       if ((M2_FEATURE(MULTIFRAME) == ENABLED) || (M2_FEATURE(FEC) == ENABLED))
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
            radio.rxlimit   = 48;
            buffer_mode     = 1;
#       endif

        sub_buffer_config(buffer_mode, 0);
        sub_syncword_config(1);

        // RX startup routine specific to CC430
        subcc430_launch_rx( b00000000,
                            (ot_u16)((RF_CoreIT_IOCFG0 | RF_CoreIT_SyncWord ) >> 16),
                            (ot_u16)(RF_CoreIT_IOCFG0 | RF_CoreIT_SyncWord )  );
    }

#else
    // BLINKER only (no RX)
    callback(RM2_ERR_GENERIC, 0);
#endif
}





void rm2_rxinit_bf(ot_u8 channel, ot_sig2 callback) {
#if (SYS_RECEIVE == ENABLED)
    /// 1. Open background method of RX (Burrow directly into Done state)
    radio.state     = RADIO_STATE_RXDONE;
    radio.flags     = RADIO_FLAG_FLOOD;
    radio.evtdone   = callback;
    radio.rxlimit   = 48;   // set rxlimit as too big for RXFull interrupt

    /// Make sure channel is supported.  If not, stop now.
    if (sub_test_channel(channel, M2_NETSTATE_UNASSOC) == False) {
        radio.evtdone(RM2_ERR_BADCHANNEL, 0);
    }
    else {
        ot_u8 pktlen;
#       if ((M2_FEATURE(FEC) == ENABLED) && (RF_FEATURE(FEC) != ENABLED))
            pktlen = (phymac[0].channel & 0x80) ? 16 : 7;
#       else
            pktlen = 7;
#       endif

        /// Queue manipulation to fit background frame into common model
        q_empty(&rxq);
        rxq.length      = pktlen + 2;
        rxq.front[0]    = pktlen;
        rxq.front[1]    = 0;
        rxq.getcursor   = &rxq.front[2];
        rxq.putcursor   = &rxq.front[2];

        radio_flush_rx();
        sub_buffer_config(0, pktlen);

        /// Start Decoder
        /// - Enable direct RX termination based on RSSI value, which is not
        ///   done for Foreground RX (this is b4 in MCSM2)
        /// - Enable RX Timeout interrupt IOCFG0, which will also occur if
        ///   RSSI-based termination occurs.
        subcc430_launch_rx( \
            b00010000, \
            (ot_u16)((RF_CoreIT_IOCFG0 | RF_CoreIT_SyncWord) >> 16), \
            (ot_u16)(RF_CoreIT_IOCFG0 | RF_CoreIT_SyncWord) \
        );
    }

#else
    // BLINKER only (no RX)
    callback(RM2_ERR_GENERIC, 0);
#endif
}




void rm2_rxsync_isr() {
/// Reset the radio interruptor to catch the next RX FIFO interrupt, having
/// qualified the Sync Word.  rm2_rxdata_isr() will be called on that interrupt.
/// Also, re-schedule a system event as a watchdog.
    RFWord->IE          = (ot_u16)((RF_CoreIT_RXFull | RF_CoreIT_EndState) >> 16);
    RFWord->IES         = (ot_u16)(RF_CoreIT_RXFull | RF_CoreIT_EndState);
    sys_set_mutex((ot_uint)SYS_MUTEX_RADIO_DATA);
    sub_killonlowrssi();
}



void rm2_rxtimeout_isr() {
    sub_kill(RM2_ERR_TIMEOUT, 0);
}




void rm2_rxdata_isr() {
#if (SYS_RECEIVE == ENABLED)
    sub_killonlowrssi();

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
#       endif

        /// RX State 1:
        /// Continuous RX'ing of a packet that is buffered without HW control
        /// CC430 only compiles this code if multiframe packets are enabled or
        /// FEC RX is enabled (CC430 doesn't have HW FEC)
#       if ((M2_FEATURE(MULTIFRAME) == ENABLED) || (M2_FEATURE(FEC_RX) == ENABLED))
        case (RADIO_STATE_RXPAGE >> RADIO_STATE_RXSHIFT): {
            ot_uint remaining_bytes = em2_remaining_bytes();
            if (remaining_bytes <= radio.rxlimit) {
               if (remaining_bytes <= RF_GetRXBYTES()) {    //New
                    RFWord->IFG |= (ot_u16)(RF_CoreIT_EndState >> 16);
                }
                else {
                    radio.rxlimit = remaining_bytes;
                    sub_buffer_config(0, remaining_bytes);
                }
                // kill RX full interrupt, only use packet done
                RFWord->IE      = (ot_u16)(RF_CoreIT_EndState >> 16);
                RFWord->IES     = (ot_u16)(RF_CoreIT_EndState);
                radio.flags    &= ~RADIO_FLAG_RESIZE;
                radio.state     = RADIO_STATE_RXDONE;
            }
            break;
        }
#       endif

        /// RX State 2:
        /// Continuous RX'ing of a packet that is maintained by HW control
        /// On CC430, Non-FEC single frame packets should be using AUTO.
        case (RADIO_STATE_RXAUTO >> RADIO_STATE_RXSHIFT): {
            // Automatic RX should not actually need any management

            //ot_uint remaining_bytes = em2_remaining_bytes();
            //if (remaining_bytes <= radio.rxlimit) {
                //radio.rxlimit   = remaining_bytes;
                //radio.state     = RADIO_STATE_RXDONE;
                // It shouldn't need 0 remaining bytes condition, or interrupt
                // modification here, because in auto mode the RF core knows
                // when the packet ends
            //}
            break;
        }

        /// RX State 3: RXDONE is handled by rm2_rxend_isr()

        /// Bug Trap
        default:
            sub_kill(RM2_ERR_GENERIC, 0);
            break;
    }


    /// 3. Change the size of the RX buffer to default, if required
#   if ((M2_FEATURE(MULTIFRAME) == ENABLED) || (M2_FEATURE(FEC_RX) == ENABLED))
        if (radio.flags & RADIO_FLAG_RESIZE) {
            radio.flags ^= RADIO_FLAG_RESIZE;
            RF_WriteSingleReg(RF_CoreReg_FIFOTHR, (ot_u8)((radio.rxlimit >> 2) - 1));
        }
#   endif

#endif
}



void rm2_rxend_isr() {
#if ((M2_FEATURE(MULTIFRAME) == ENABLED) || (M2_FEATURE(FEC_RX) == ENABLED))
    em2_decode_data();  // New: decode any leftover data
#endif
    sub_kill(0, (ot_int)crc_check() - 1);
}




void rm2_txinit_ff(ot_int est_frames, ot_sig2 callback) {
    radio.state     = RADIO_STATE_TXCCA1;
    radio.flags     = (est_frames > 1);
    radio.evtdone   = callback;

    /// Flush the buffer.  This may be unnecessary on some HW.
    radio_flush_tx();
    RFCONFIG_TXINIT();

    /// Prepare the foreground frame packet
    txq.getcursor   = txq.front;
    txq.front[1]    = phymac[0].tx_eirp;

    sub_prep_q(&txq);
    em2_encode_newpacket();
    em2_encode_newframe();
    em2_encode_data();
}




void rm2_txinit_bf(ot_sig2 callback) {
#if (SYS_FLOOD == ENABLED)
    radio.state     = RADIO_STATE_TXCCA1;
    radio.flags     = RADIO_FLAG_FLOOD;
    radio.evtdone   = callback;

    /// Flush the buffer.  This may be unnecessary on some HW.
    radio_flush_tx();
    RFCONFIG_TXINIT();

    /// Prepare the background frame packet
    /// Note usage of sub_buffer_config():
    /// (packet bytes depend on radio HW.  Encoder knows.)
    txq.getcursor   = txq.front;

    sub_prep_q(&txq);
    em2_encode_newpacket();
    em2_encode_newframe();
    sub_buffer_config(0, em2_remaining_bytes() );
    em2_encode_data();
#endif
}





void rm2_txstop_flood() {
#if (SYS_FLOOD == ENABLED)
    radio.state = RADIO_STATE_TXDONE;
    RF_CoreITConfig(RF_CoreIT_TXBelowThresh, DISABLE);  //RFCONFIG_TXFIFOLOW_INTOFF();
#endif
}





ot_int rm2_txcsma() {
    // The shifting in the switch is so that the numbers are 0, 1, 2, 3...
    // It may seem silly, but it allows the switch to be compiled better.
    switch ( (radio.state >> RADIO_STATE_TXSHIFT) & (RADIO_STATE_TXMASK >> RADIO_STATE_TXSHIFT) ) {

        /// 1. First CCA
        case (RADIO_STATE_TXCCA1 >> RADIO_STATE_TXSHIFT): {
            if (dll.comm.csmaca_params & M2_CSMACA_NOCSMA) {
                if (sub_nocsma_init()) {
                    radio.state = RADIO_STATE_TXSTART;
                    return 0;
                }
                return RM2_ERR_BADCHANNEL;
            }
            if (sub_csma_init() == False){
                return RM2_ERR_CCAFAIL;
            }

            /// Dynamically put the radio to sleep if it is advantageous to
            /// do so, otherwise don't put radio to sleep (stays in idle).
            //if (phymac[0].tg > (RADIO_COLDSTART_STI_TYP >> 5))
                radio_sleep();

            radio.state = RADIO_STATE_TXCCA2;
            return phymac[0].tg;
        }

        /// 2. Second CCA
        case (RADIO_STATE_TXCCA2 >> RADIO_STATE_TXSHIFT): {
            ot_int test;
            test            = (ot_int)sub_cca2();
            radio.state     = test ? RADIO_STATE_TXSTART : RADIO_STATE_TXCCA1;
            return            test ? 0 : RM2_ERR_CCAFAIL;
        }

        /// 3. TX startup
        case (RADIO_STATE_TXSTART >> RADIO_STATE_TXSHIFT): {
            radio.state     = RADIO_STATE_TXDATA;
            radio.txlimit   = 5;
            RFCONFIG_TXBUFFER(5);

#           if (SYS_FLOOD == ENABLED)
            /// Packet flooding.  Only needed on devices that can send M2AdvP
            if (radio.flags & RADIO_FLAG_FLOOD) {
                sub_syncword_config(0);     // use syncword class 0
                RFCONFIG_TXTOTX();          //stay in tx after tx
                //radio.txlimit = 5;
                //RFCONFIG_TXBUFFER(5);
                RFCONFIG_TXFIFO_INTON();
            }
            else
#           endif
            {
                sub_syncword_config(1);     // use syncword class 1
                RFCONFIG_TXTOIDLE();        // go to idle after tx
                //radio.txlimit = 5;          // min possible packet is 6 bytes
                //RFCONFIG_TXBUFFER(5);
                RFCONFIG_TXFIFO_INTON();
            }

            RFCTRL_TX();                    //go to TX
            //return sub_eta_txi();
            return -1;
        }
    }

    /// Some bug has occurred
    radio.state = 0;
    return RM2_ERR_GENERIC;
}






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

            /// Packet flooding.  Only needed on devices that can send M2AdvP
#           if (SYS_FLOOD == ENABLED)
            if (radio.flags & RADIO_FLAG_FLOOD) {
                radio.evtdone(2, 0);
                txq.getcursor = txq.front;
                em2_encode_newframe();
                goto rm2_txpkt_TXDATA;
            }
#           endif

            /// If the frame is done (em2_remaining_bytes() == 0) and there are
            /// no more frames to transmit, then this interrupt is due to a low
            /// threshold, and we just need to turn-off the threshold interrupt
            /// and wait for the last bit of data to get sent.
            if (em2_remaining_frames() == 0) {
                radio.state = RADIO_STATE_TXDONE;
                RFCONFIG_TXFIFOLOW_INTOFF();
                break;
            }

            /// If the frame is done, but more need to be sent (e.g. MFP's)
            /// queue it up.  The additional encode stage is there to fill up
            /// what's left of the buffer.
#           if (M2_FEATURE(MULTIFRAME) == ENABLED)
            if (radio.flags & RADIO_FLAG_FRCONT) {
                q_rebase(&txq, txq.getcursor);
                radio.evtdone(1, 0);        //callback action for next frame
                em2_encode_newframe();
                txq.front[1] = phymac[0].tx_eirp;
                em2_encode_data();
                goto rm2_txpkt_TXDATA;
            }
#           endif
        }

        /// 5. Conclude the TX process, and wipe the radio state
        //     turn off any remaining TX interrupts
        case (RADIO_STATE_TXDONE >> RADIO_STATE_TXSHIFT):
            sub_kill(0, 0);
            break;

        /// Bug trap
        default:
            sub_kill(RM2_ERR_GENERIC, 0);
            break;
    }
}








/** Radio Subroutines
  * ============================================================================
  * - Usually some minor adjustments needed when porting to new platform
  * - See integrated notes for areas sensitive to porting
  */

void sub_null(ot_int arg1, ot_int arg2) { return; }



void sub_kill(ot_int main_err, ot_int frame_err) {
    radio_gag();
    radio_idle();
    radio.evtdone(main_err, frame_err);
    radio.evtdone   = &otutils_sig2_null;
    radio.state     = 0;
}


void sub_killonlowrssi() {
    ot_int min_rssi = ((phymac[0].cs_thr >> 1) & 0x3F) - 40;
    if (radio_rssi() < min_rssi) {
        sub_kill(RM2_ERR_LINK, 0);
    }
}


ot_u8 sub_rssithr_calc(ot_u8 input, ot_u8 offset) {
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




ot_bool sub_cca2() {
/// Do second part of CSMA double check.
/// On different chips, CCA can be optimized via different implementations, and
/// optimizng might also affect the rm2_txpkt() function
    ot_bool cca_status;

    //radio_idle();                   //Assure Radio is in Idle
    cca_status = sub_cca_scan();
    if ( cca_status == False ) {    //Optimizers may remove this if() for
        radio_sleep();              //certain implementations
    }

    return cca_status;
}




ot_bool sub_chan_scan( ot_bool (*scan_test)() ) {
    vlFILE* fp;
    ot_int  i;

    fp = ISF_open_su( ISF_ID(channel_configuration) );
    ///@todo assert fp

    /// Go through the list of tx channels
    /// - Make sure the channel ID is valid
    /// - Make sure the transmission can fit within the contention period.
    /// - Scan it, to make sure it can be used
    for (i=0; i<dll.comm.tx_channels; i++) {
        if (sub_channel_lookup(dll.comm.tx_chanlist[i], fp) != False) {
            if (scan_test()) {
                break;
            }
        }
    }

    vl_close(fp);
    return (ot_bool)(i < dll.comm.tx_channels);
}





ot_bool sub_cca_scan() {
/// Called indirectly by other subroutines
/// @todo this is the only blocking call.  It can cause problems, being
/// blocking, so one day I would like to make it non-blocking.

/// @note CCA scan is a blocking call to the radio core, which waits for radio
/// RSSI to be declared valid (uses CC430 RSSI_Valid interrupt).  Alternatively,
/// a bipolar setup of CS/CCA interrupts could be used if you are porting to
/// CC1xxx and don't have the RSSI_Valid interrupt, but RSSI_Valid is more
/// reliable.  Then we just compare returned RSSI's with the stored limits.
    ot_bool cca_status = True;

    RFCONFIG_CCA();
    RFCONFIG_CSMA_INTON();              //enable CSMA/CCA Interrupt(s)
    radio_idle();                       //send radio to idle mode

    RFCTRL_RX();                        //turn on RX
    platform_enable_interrupts();       //enable blocking interrupts just for this scan (fast)
    MCU_SLEEP_WHILE_RF();               //wait for RSSI_valid interrupt
    platform_disable_interrupts();
    RFCONFIG_CSMA_INTOFF();             //disable CSMA/CCA Interrupt(s)

    cca_status = radio_check_cca();     //Compare radio RSSI with limits
    radio_idle();                       //send radio to idle mode

    return cca_status;
}


ot_bool sub_noscan() {
    return True;
}







ot_bool sub_csma_init() {
/// Called directly by TX radio function(s)
/// Duty: Initialize csma process, and run the first scan (of two).
    ot_bool cca1_status;

    /// @note CC430 specific
    /// Disable RX Timeout (MCSM2) and enable CS+CCA differential interrupts.
    /// One of the two (CS vs. CCA) will always happen.
    RF_WriteSingleReg(RF_CoreReg_MCSM2, 0x07);

    /// Setup channel, scan it, and power down RF on scan fail
    cca1_status = sub_chan_scan( &sub_cca_scan );
    if (cca1_status == False) {         //Optimizers may remove this if() for
        radio_sleep();                  //certain implementations
    }

    return cca1_status;
}





ot_bool sub_nocsma_init() {
/// Called directly by TX radio function(s) when CSMA is disabled via System
/// Duty: bypass CSMA scan
    return sub_chan_scan( &sub_noscan );
}




ot_bool sub_channel_lookup(ot_u8 chan_id, vlFILE* fp) {
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
            phymac[0].cs_thr    = sub_rssithr_calc(phymac[0].cs_thr, RF_CSTHR_OFFSET);
            //phymac[0].cca_thr   = sub_rssithr_calc(phymac[0].cca_thr, RF_CCATHR_OFFSET);

            sub_chan_config(old_chan_id, old_tx_eirp);
            return True;
        }
    }

    return False;
}




void sub_chan_config(ot_u8 old_chan, ot_u8 old_eirp) {
/// Called by sub_channel_lookup()
/// Duty: perform channel setup and recalibration when moving from one channel
/// to another.

    ot_u8 fc_i;

#   if (M2_FEATURE(FEC) == ENABLED)
    /// Only worry about changing encoding method if:
    /// (a) Optional FEC is enabled (otherwise only one kind of encoding)
    /// (b) The last-used channel had a different encoding type

        if ( (old_chan ^ phymac[0].channel) & 0x80 ) {
            ot_u8 pktctrl1_val;
            ot_u8 mdmcfg2_val;

            if (phymac[0].channel & 0x80) {
                /// Set up transceiver for FEC
                /// set Preamble qualifier on the low side (eg PQT=4 on CC430)
                /// set Sync Word qualifier to allow up to two bit errors
                //RFCONFIG_FECQUAL();
                pktctrl1_val    = b00100000;
                mdmcfg2_val     = b10010101;

            }
            else {
                /// Set up transceiver for PN9 method
                /// If FEC is disabled, these settings will always be in the regs.
                ///set FEC to OFF and PN9 to ON
                ///set Preamble qualifier on the high side (eg PQT=8 on CC430)
                ///set Sync qualifier to allow no bit errors (16/16)
                RFCONFIG_PN9HW();
                RFCONFIG_PN9QUAL();
                pktctrl1_val    = b01000000;
                mdmcfg2_val     = b10010010;
                radio.flags    |= RADIO_FLAG_AUTO;
            }

            RF_WriteSingleReg(RF_CoreReg_PKTCTRL1, pktctrl1_val);
            RF_WriteSingleReg(RF_CoreReg_MDMCFG2, mdmcfg2_val);

            /// @note On CC430 register PKTCTRL0:
            /// Pktctrl0 is adjusted later, by sub_buffer_config, during RX/TX
            /// Pktctrl0 contains the bit that turns on CC430 PN9 HW.
        }
#   endif

    /// Reprogram the PA Table if eirp of new channel isn't the same as before
    if (old_eirp != phymac[0].tx_eirp) {
        sub_set_txpower( phymac[0].tx_eirp );
    }

    /// Center Frequency index = lower four bits channel ID
    fc_i = (phymac[0].channel & 0x0F);

    /// @note Settings of "mdmcfg..." are CC430 specific implementation
    /// Reprogram data rate, packet method, and modulation per upper nibble
    /// (Don't reprogram if radio is using the same channel class already)
    if ( (old_chan ^ phymac[0].channel) & 0x70 ) {
        ot_u8 mdmcfg4_val;
        ot_u8 mdmcfg3_val;
        ot_u8 mdmcfg1_val;

        switch ((phymac[0].channel >> 4) & 0x03) {
            case 0: fc_i = 7;
            case 1: //RFCONFIG_FSK18();       /// 55 kS/s method
                    mdmcfg4_val = (RADIO_FILTER_NORMAL | RADIO_DRATE_NORMAL_E);
                    mdmcfg3_val = RADIO_DRATE_NORMAL_M;
                    mdmcfg1_val = (RADIO_PREAMBLE_NORMAL | RADIO_CHANNEL_SPC_E);
                    break;
            case 2:
            case 3: //RFCONFIG_FSK05();       /// 200 kS/s method
                    mdmcfg4_val = (RADIO_FILTER_TURBO | RADIO_DRATE_TURBO_E);
                    mdmcfg3_val = RADIO_DRATE_TURBO_M;
                    mdmcfg1_val = (RADIO_PREAMBLE_TURBO | RADIO_CHANNEL_SPC_E);
                    break;
        }

        RF_WriteSingleReg(RF_CoreReg_MDMCFG4, mdmcfg4_val);
        RF_WriteSingleReg(RF_CoreReg_MDMCFG3, mdmcfg3_val);
        RF_WriteSingleReg(RF_CoreReg_MDMCFG1, mdmcfg1_val);
    }


    /// @note: CC430 contains a channel-offset built-in mechanism.  On chips
    /// without this, just change the center frequency.  The simplest way is to
    /// have a lookup table of center frequency settings mapped to fc_i.
    /// - Reprogram channel, and stage recalibration
    /// - But don't do these things if radio is already set on this center channel
    if ( fc_i != (old_chan & 0x0F) ) {
        RF_WriteSingleReg(RF_CoreReg_CHANNR, (ot_u8)fc_i); //RFCONFIG_FC(fc_i);
        radio_calibrate();  //Manual Calibrate
    }
}




void sub_syncword_config(ot_u8 sync_class) {
///@note The MSP430 core is little endian, and CC1101 core is big endian, so
/// syncwords might need to be twiddled.
    Twobytes sync_value;

#   if (M2_FEATURE(FEC) == ENABLED)
        if (phymac[0].channel & 0x80) {   //Actual= 0xF498 : 0x192F;
            sync_value.ushort = (sync_class == 0) ? 0xF498 : 0x192F;
        }
        else
#   endif
    {                                 //Actual= 0xE6D0 : 0x0B67;
        sync_value.ushort = (sync_class == 0) ? 0xE6D0 : 0x0B67;
    }

    ///@note Depending on how the RF core interface is designed, you may want to
    ///      redefine this macro to take-in the 16 bit value in a different way.
    //RFCONFIG_SYNCWORD( &sync_value.ubyte[0] );
    RF_WriteBurstReg(RF_CoreReg_SYNC1, &sync_value.ubyte[0], 2);
}




void sub_buffer_config(ot_u8 mode, ot_u8 param) {
/// CC430 specific implementation.  This function selects between three
/// buffering modes: FIFO buffering (2), automatic packet handling (1), or
/// fixed-length packet (0).  The "param" argument is a chip-specific option.
    if (mode == 0) {
        RF_WriteSingleReg(RF_CoreReg_PKTLEN, param);
    }

#   if (M2_FEATURE(FEC) == ENABLED)
        /// Enable PN9 encoder if FEC is not being used
        mode |= ((phymac[0].channel & 0x80) >> 1);
#   else
        /// Always enable PN9 encoder (FEC disabled)
        mode |= b01000000;
#   endif

    RF_WriteSingleReg(RF_CoreReg_PKTCTRL0, mode);
}




void sub_prep_q(Queue* q) {
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




ot_int sub_eta(ot_int next_int) {
/// Subtract by a tolerance amount, floor at 0
    ot_int eta = rm2_scale_codec(next_int) - 1;
    return (eta > 0) ? eta : 0;
}


ot_int sub_eta_rxi() {
    return sub_eta( RFGET_RXFIFO_NEXTINT() );
}

ot_int sub_eta_txi() {
    return sub_eta( RFGET_TXFIFO_NEXTINT() );
}





void sub_offset_rxtimeout() {
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




void sub_set_txpower( ot_u8 eirp_code ) {
///@note CC430-specific

/// Sets the tx output power.
/// "eirp_code" is a value, 0-127, that is: eirp_code/2 - 40 = TX dBm
/// i.e. eirp_code=0 => -40 dBm, eirp_code=80 => 0 dBm, etc

/// CC430 has an 8-value amplifier table.  TX power gets ramped-up, step by
/// step of the table.  Each value in the amp table is a power code specific
/// to the CC (from the lut).

/// CC430-specific power selector table
/// The index value is eirp_code.  As you may notice, eirp_code does not map
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

    ///@note 6dB (12/2) is a guess at loss in matching
    ///@todo make magic-number "12" a configuration constant
    eirp_val    = eirp_code + 12;
    eirp_val    = (eirp_val < 99) ? eirp_val : 99;

    for(i=7; (i>=0) && (eirp_val>=0); i--, eirp_val-=6) {
        pa_table[i] = pa_lut[eirp_val];
    }
    i++;

    RF_WriteBurstPATable(&pa_table[i], (ot_u8)(8-i));
    RF_WriteSingleReg( RF_CoreReg_FREND0, (RFREG_FREND0 | (ot_u8)(7-i)) );
}



