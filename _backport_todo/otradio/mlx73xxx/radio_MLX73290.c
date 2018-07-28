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
  * @file       /Platforms/MLX73290/radio_MLX73290.c
  * @author     JP Norair
  * @version    V1.0
  * @date       8 Dec 2011
  * @brief      Radio Driver (RF transceiver) for MLX73290
  * @defgroup   Radio (Radio Module)
  * @ingroup    Radio
  *
  * The header file for this implementation is /OTlib/radio.h.  It is universal
  * for all platforms, even though the implementation (this file) can differ.
  * There are also some header files at /Platforms/MLX73290/mlx73xxx_~.h that
  * apply to this driver.
  *
  * For DASH7 Silicon certification, there are four basic tiers of HW features:
  * 1. PHY      The HW has a buffered I/O and the basic features necessary
  * 2. PHY+     The HW can do encoding, CRC, and some packet handling
  * 3. MAC      The HW can automate some inner loops, like Adv Flood and CSMA
  * 4. MAC+     The HW has most features of the MAC integrated
  *
  * The MLX73290 satisfies PHY and PHY+, for the most part (no FEC), and it also
  * satisfies more MAC features than any other chip available at the time of
  * writing (RX timer, huge FIFO, multiframe support, nice state transitioning).
  ******************************************************************************
  */

#include <otsys/types.h>
#include <otsys/config.h>
#include <otlib/utils.h>
#include <otplatform.h>
#include <otlib/logger.h>
#include <m2/radio.h>
#include <m2/encode.h>
#include <otlib/crc16.h>
#include <otlib/buffers.h>
#include <otlib/queue.h>
#include <otsys/veelite.h>
#include <m2/session.h>

#include "mlx73xxx_interface.h"




/** Standard Radio Module Declarations <BR>
  * ========================================================================<BR>
  */

/** Radio Buffer Allocation constants (MLX73xxx has internal buffer) */
#define BUFFER_ALLOC         128


// Change these settings depending on your radio buffer
#define RADIO_BUFFER_TXMIN      RADIO_BUFFER_TXMAX
#define RADIO_BUFFER_RXMIN      RADIO_BUFFER_RXMAX


/** Internal Radio States   <BR>
  * b5:3        b2:0        <BR>
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

/** Internal Radio Flags (lots of room remaining) */
#define RADIO_FLAG_FLOOD        (1 << 0)
#define RADIO_FLAG_ERROR        (1 << 1)
#define RADIO_FLAG_AUTO         (1 << 3)
#define RADIO_FLAG_FRCONT       (1 << 5)
#define RADIO_FLAG_TXRECAL      (1 << 6)
#define RADIO_FLAG_RXRECAL      (1 << 7)
#define RADIO_FLAG_RECAL        (1 << 7)

//#define RADIO_FLAG_RESIZE       (1 << 4)



/** Internal Radio Interrupt Flags <BR>
  * For performance reasons, sometimes interrupt flags will be stored locally
  * and used in later conditionals.  The usage is implementation dependent.
  */
#define RADIO_INT_CCA           (1 << 3)
#define RADIO_INT_CS            (1 << 4)


/** Radio HW Timing Parameters <BR>
  * Measurements for Radio HW performance, usually corresponding to timings
  * as the radio goes from one state to the next.
  *
  * Reminder: sti is "short tick" ( 1 sti = 32 ti = 1/32768 sec = ~30.52 us )
  */
#define RADIO_COLDSTART_STI_MAX     27
#define RADIO_COLDSTART_STI_TYP     9
#define RADIO_WARMSTART_STI         3
#define RADIO_CALIBRATE_STI         24
#define RADIO_FASTHOP_STI           5
#define RADIO_KILLRX_STI            0
#define RADIO_KILLTX_STI            0
#define RADIO_TURNAROUND_STI        1


/** PHY-MAC Array declaration <BR>
  * Described in radio.h -- This driver only supports M2_PARAM_MI_CHANNELS = 1.
  */
phymac_struct   phymac[M2_PARAM_MI_CHANNELS];


/** Radio Module Data <BR>
  * Data that is useful for the internal use of this module.  The list below is
  * comprehensive, and it may not be needed in entirety for all implementations.
  * For implementations that don't use the values, comment out.
  * <LI> state:     Radio State, used with multi-call functions (RX/TX) </LI>
  * <LI> flags:     A local store for usage flags </LI>
  * <LI> txlimit:   An interrupt/event comes when tx buffer gets below this number of bytes </LI>
  * <LI> rxlimit:   An interrupt/event comes when rx buffer gets above this number of bytes </LI>
  * <LI> last_rssi: Experimental, used to buffer the last-read RSSI value </LI>
  * <LI> evtdone:   A callback that is used when RX or TX is completed (i.e. done) </LI>
  * <LI> txcursor:  holds some data about tx buffer position (MCU-based buffer only) </LI>
  * <LI> rxcursor:  holds some data about rx buffer position (MCU-based buffer only) </LI>
  * <LI> buffer[]:  buffer data.  (MCU-based buffer only) </LI>
  * <LI> last_rssi: The most recent value of the rss (not always needed) </LI>
  */
typedef struct {
    ot_u8   state;
    ot_u8   flags;
    ot_int  fifoest;
    ot_int  txlimit;
    ot_int  rxlimit;
//  ot_int  last_rssi;
    ot_sig2 evtdone;
#   if (BUFFER_ALLOC > 0)
        //ot_int  txcursor;
        //ot_int  rxcursor;
        ot_u8   buffer[BUFFER_ALLOC+1];
#   endif
} radio_struct;

radio_struct radio;





/** Local Radio Module Subroutine Prototypes  <BR>
  * ========================================================================<BR>
  */
typedef enum {
    SYNC_Class_0    = 0,
    SYNC_Class_1    = 2
} Sync_Class;

void    sub_kill(ot_int main_err, ot_int frame_err);
ot_bool sub_killonlowrssi();

ot_u8   sub_rssithr_calc(ot_u8 input, ot_u8 offset); // Fiddling necessary
ot_bool sub_cca_init();
ot_bool sub_cca2();
ot_bool sub_chan_scan( ot_bool (*scan_test)() );
ot_bool sub_noscan();
ot_bool sub_cca_scan();
ot_bool sub_csma_init();
ot_bool sub_nocsma_init();

ot_bool sub_channel_lookup(ot_u8 chan_id, vlFILE* fp);
void    sub_syncword_config(Sync_Class sync_class);
void    sub_buffer_config(ot_u8 mode, ot_u8 param);
void    sub_chan_config(ot_u8 old_chan, ot_u8 old_eirp);

ot_int  sub_check_crc();
void    sub_set_txpower(ot_u8 eirp_code);
void    sub_prep_q(ot_queue* q);
ot_int  sub_eta(ot_int next_int);
ot_int  sub_eta_rxi();
ot_int  sub_eta_txi();
void    sub_offset_rxtimeout();





/// @todo need to really figure out what to do with sub_kill()
void radio_isr(ot_u8 ivector) {
    switch (ivector & 0xf) {
        // Universal imode interrupt (0): RF Stopped / went to idle
        case RFIV_XTALRDY:      break;

        // Listening imode interrupts (1-3)
        case RFIV_TMRFLAG:      rm2_rxtimeout_isr();    break;
        case RFIV_PAYLOADRX:    rm2_rxsync_isr();       break;
        case RFIV_RSSICS:       /* reset timeout */     break;

        // CSMA/CCA imode interrupts (4-6): (CSMA is blocking, pre-empts caller)
        case RFIV_CCAEXP:
        case RFIV_CCAERR:
        case RFIV_CCAFAIL:      break;

        // Data RX imode interrupts (7-9)
        case RFIV_RXPKTERR:     radio.flags |= RADIO_FLAG_ERROR;
        case RFIV_RXPKTDONE:    rm2_rxend_isr();        break;
        case RFIV_RXFIFOTHR:    rm2_rxdata_isr();       break;

        // Data TX imode interrupts (10-12)
        case RFIV_TXPKTDONE:
        case RFIV_TXFIFOTHR:    rm2_txdata_isr();       break;
        case RFIV_TXFIFOERR:    /*sub_kill(0,0);*/             break;

        // Currently used only for testing (13-15)
        case 13:
        case 14:
        case 15: break;
    }
}






/** Radio Core Control Functions <BR>
  * ========================================================================<BR>
  * - Need to be customized per radio platform
  */

void radio_off() {
    radio_sleep();
}


void radio_gag() {
/// Disable interrupts (turn off interrupt bits for NVIC or EXTI pins)
    mlx73_int_turnoff(RFI_ALL);
}


void radio_sleep() {
/// Flushes FIFOs, Stops RX/TX, kills XTAL + Vdig + PLL for lowest power sleep
    mlx73_write(RFREG(STATUS1), 0);
}


void radio_idle() {
/// Flushes FIFOs, Stops RX/TX, keeps alive XTAL + Vdig
    mlx73_write(RFREG(STATUS1), (RFMODE_STOPPEDTX | (6<<2) ));  //0x18 / (6<<2)
}


void radio_calibrate() {
/// @note MLX73xxx has (according to datasheet) different calibrations for TX or
/// RX.  I think they both just calibrate the VCO, but, in any case, you should
/// use the mlx73_rfcalibrate_...() functions instead of this one.
    mlx73_rfcalibrate_rx();
}





/** Generic Radio Module Control Functions
  * ============================================================================
  * - Need to be customized per radio platform
  */

void radio_init( ) {
    mlx73_init_bus();
    mlx73_reset();
    //mlx73_iqcorrection(); //not doing IQ Cal at the moment

    //uncomment if you want to debug reset values
    //mlx73_coredump(0);
    //mlx73_coredump(-1);

    mlx73_load_defaults();

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



ot_bool radio_check_cca() {
#ifdef MLX_PROTOTYPE
    //MLX PROTOTYPE does not have functional RSSI
    return True;
#else
    ot_int thr  = (ot_int)phymac[0].cca_thr - 140;
    ot_int rssi = mlx73_offsetrssi( mlx73_read(RF_RSSI) );
    return (ot_bool)(rssi < thr);
#endif
}



ot_int radio_rssi() {
/// this function returns the RSSI value that's saved after header detection, a
/// somewhat unique feature of the MLX73xxx.  Certain other functions use the
/// momentary RSSI feature, which is very similar.
    return mlx73_offsetrssi( mlx73_read(RFREG(RSSIHDR)) );
}



ot_u8 radio_buffer(ot_int index) {
/// MLX73xxx has HW FIFO, so this function is irrelevant
    return 0;
}

void radio_putbyte(ot_u8 databyte) {
    mlx73_write( RFREG(TXFIFOWR), databyte );
    radio.fifoest++;
    //radio.buffer[radio.fifoest] = databyte;
}

void radio_putfourbytes(ot_u8* data) {
    ot_u8 tx_buf[5];
    tx_buf[0]   = RFREG(TXFIFOWR);
    tx_buf[1]   = data[B3];
    tx_buf[2]   = data[B2];
    tx_buf[3]   = data[B1];
    tx_buf[4]   = data[B0];

    mlx73_spibus_io(0, 5, 0, tx_buf, NULL);
    radio.fifoest += 4;
}

ot_u8 radio_getbyte() {
    radio.fifoest--;
    return mlx73_read( RFREG(RXFIFORD) );
}

void radio_getfourbytes(ot_u8* data) {
    ot_u8 addr = RFREG(RXFIFORD);
    mlx73_spibus_io(0, 1, 4, &addr, data);
    radio.fifoest -= 4;
}



void radio_flush_rx() {
    radio.fifoest = 0;
    mlx73_write(RFREG(STATUS1), 0x00);
}

void radio_flush_tx() {
    radio_flush_rx();
    //radio.fifoest = 0;
    //mlx73_write(RFREG(STATUS1), 0x18);
}

ot_bool radio_rxopen() {
    ot_bool output = (radio.fifoest > 0);
    return output;
    //ot_u8 status1;
    //status1 = mlx73_read( RFREG(STATUS1) );
    //return (ot_bool)((status1 & RXFIFO_STATUS_INUSE) != 0);
}

ot_bool radio_rxopen_4() {
    ot_int thresh;
    ot_bool output;
    thresh          = (radio.state != RADIO_STATE_RXDONE) << 2;
    output          = (radio.fifoest >= thresh);
    return output;
    //return radio_rxopen();
}

//ot_bool sub_txopen(ot_u8 limit) {
//    ot_u8 addr = RFREG(STATUS1);
//    ot_u8 txstatus[2];
//    mlx73_spibus_io(0, 1, 2, &addr, txstatus);
//    return (ot_bool)( /*(txstatus[0] & TXFIFO_STATUS) && */ (txstatus[1] < limit));
//}

ot_bool radio_txopen() {
    ot_bool output = (radio.fifoest < RADIO_BUFFER_TXMAX);
    return output;
}

ot_bool radio_txopen_4() {
    ot_bool output = (radio.fifoest <= (RADIO_BUFFER_TXMAX-4));
    return output;
}










/** Radio I/O Functions  <BR>
  * ========================================================================<BR>
  */

/// Local Static lookups for conditional RX configuration
#if ((M2_FEATURE(FEC) == ENABLED) && (SYS_RECEIVE == ENABLED))
    static const ot_u8 st_sel[] = {RADIO_STATE_RXAUTO, RADIO_STATE_RXPAGE};
    static const ot_u8 bf_sel[] = {0, PACKET_LEN_FIXED};
    static const ot_u8 pl_sel[] = {7, 16};
#endif

ot_bool sub_test_channel(ot_u8 channel, ot_u8 netstate);
void submlx_launch_rx(ot_u8 mcsm0_rxterm);



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
void submlx_launch_rx(ot_u8 mcsm0_rxterm) {
/// Valid inputs for mcsm0_rxterm are: RXTERM_COND_OFF, RXTERM_COND_ON,
/// RXTERM_COND_NOCS, RXTERM_COND_NORX
    sub_prep_q(&rxq);
    em2_decode_newpacket();
    em2_decode_newframe();
    sub_offset_rxtimeout();     // if timeout is 0, set it to a minimal amount

    { // Write CS threshold, RX Timeout, and rx termination spec
        ot_u8 data[3];
        data[0] = RFREG(CSCFG);
        data[1] = 0; //sub_rssithr_calc(phymac[0].cs_thr, RF_CSTHR_OFFSET);
        data[2] = 0; //mlx73_calc_rxtimeout(dll.comm.rx_timeout);
        data[3] = (RFDEF(MCSM0) & ~RXTERM_COND) | mcsm0_rxterm;
        mlx73_spibus_io(0, 4, 0, data, NULL);
    }

    // Reconfigure IO for listening,
    // Then put into RX (using pre-calibration if necessary)
    // Then enable listening interrupts
    mlx73_iocfg_listen();
    {
        //ot_u8 rx_config = (radio.flags&RADIO_FLAG_RXRECAL) | RFMODE_RX | 0x18;
        ot_u8 rx_config = RADIO_FLAG_RXRECAL | RFMODE_RX | (3<<2);  //0x1C / (7<<2), (6<<2), (1<<2)
        radio.flags    &= ~RADIO_FLAG_RXRECAL;
        mlx73_write(RFREG(STATUS1), rx_config);
    }

    //mlx73_coredump(-1);
    //mlx73_coredump(0);
    //sub_freqdump();

    mlx73_intcfg_listen();
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
    static const ot_int tgd[4] = \
        { M2_TGD_55FULL, M2_TGD_200FULL, M2_TGD_55HALF, M2_TGD_200HALF };
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
    pkt_bytes  += RADIO_PKT_OVERHEAD + 2;
    pkt_bytes  += ((phymac[0].channel & 0x60) != 0) << 1;
    return rm2_scale_codec(pkt_bytes);
}

ot_int rm2_scale_codec(ot_int buf_bytes) {
/// Turns a number of bytes (buf_bytes) into a number of tick (ti) units.
/// To refresh your memory: 1 ti = ((1sec/32768) * 2^5) = 2^-10 sec = ~0.977 ms.
/// Pursuant to DASH7 Mode 2 spec, b6:4 of channel ID corresponds to kS/s.
/// 55.555 kS/s = 144us per buffer byte
/// 200.00 kS/s = 40us per buffer bytes
    buf_bytes *= (phymac[0].channel & 0x60) ? 40 : 144;

    /// Divide us into Ticks (shift right 10 = divide by 1024)
#   if ((M2_FEATURE(FEC) == ENABLED) && (RF_FEATURE(FEC) == ENABLED))
        buf_bytes >>= (10 - ((phymac[0].channel & 0x80) != 0) );
#   else
        buf_bytes >>= 10;
#   endif

    return buf_bytes;
}



void rm2_kill() {
    sub_kill(RM2_ERR_KILL, 0);
}


void sub_freqdump() {
///debugging function to dump-out register values of RF core
    ot_u8 addr = RFREG(FREQ0);
    ot_u8 output[8];

    mlx73_spibus_io(0, 1, 4, &addr, &output[4]);

    output[0] = output[7];
    output[1] = output[6];
    output[2] = output[5];
    output[3] = output[4];

    logger_hexmsg(4, 4, "DUMP", output);
    delay_ms(5);
}

void rm2_rxinit_ff(ot_u8 channel, ot_u8 netstate, ot_int est_frames, ot_sig2 callback) {
/// Use the data from the supplied session to open a channel for RX.  If the
/// channel is already open (even if it is not being used), then we will skip
/// the opening process.

/// The MLX73xxx has two, preset RX FIFO thresholds: 1 byte in and 64 bytes in.
/// If using FEC, the length byte (first byte) is spread-out over the first 32
/// symbols (encoded bits) of the message, so FEC messages below 32 bytes will
/// have some RX lag until I come up with a workaround.

#if (SYS_RECEIVE == ENABLED)
    radio.evtdone   = callback;
    radio.state     = RADIO_STATE_RXINIT;
    //radio.rxlimit   = 64;
#   if (M2_FEATURE(MULTIFRAME) == ENABLED)
        radio.flags = (est_frames > 1) << 5;    //sets RADIO_FLAG_FRCONT
#   else
        radio.flags = 0;
#   endif

    /// Make sure channel is supported.  If not, stop now.  sub_test_channel()
    /// will set RADIO_FLAG_AUTO according to radio abilities
    if (sub_test_channel(channel, netstate) == False) {
        radio.evtdone(RM2_ERR_BADCHANNEL, 0);
    }
    else {
        ///@todo the MLX73 has some feature for HW Multiframe, but I'm not sure
        /// how it works.  So the Multiframe usage is still treated manually.
        ot_u8 buffer_mode;
#       if ((M2_FEATURE(FEC) == ENABLED) && (M2_FEATURE(MULTIFRAME) == ENABLED))
        {
            ot_u8 type;
            type        = phymac[0].channel >> 7;
            radio.state = (est_frames > 1) ? 0 : st_sel[type];
            buffer_mode = bf_sel[type];
        }
#       elif (M2_FEATURE(FEC) == ENABLED)
        {
            ot_u8 type;
            type        = phymac[0].channel >> 7;
            radio.state = st_sel[type];
            buffer_mode = bf_sel[type];
        }
#       elif (M2_FEATURE(MULTIFRAME) == ENABLED)
            radio.state = (est_frames > 1) ? 0 : RADIO_STATE_RXAUTO;
            buffer_mode = 0;
#       else
            radio.state = RADIO_STATE_RXAUTO;
            buffer_mode = 0;
#       endif

        q_empty(&rxq);
        sub_buffer_config(buffer_mode, 0);
        sub_syncword_config(SYNC_Class_1);
        submlx_launch_rx( 0 /*RXTERM_COND_NORX*/);

        //sub_freqdump();
    }

#else   // BLINKER only (no RX)
    callback(RM2_ERR_GENERIC, 0);
#endif
}





void rm2_rxinit_bf(ot_u8 channel, ot_sig2 callback) {
#if (SYS_RECEIVE == ENABLED)

    /// 1. Open background method of RX
    radio.state     = RADIO_STATE_RXDONE;
    radio.evtdone   = callback;
    radio.flags     = RADIO_FLAG_FLOOD;

    /// Make sure channel is supported.  If not, stop now.
    if (sub_test_channel(channel, M2_NETSTATE_UNASSOC) == False) {
        radio.evtdone(RM2_ERR_BADCHANNEL, 0);
    }
    else {
        ot_u8 pktlen;
#       if (M2_FEATURE(FEC) == ENABLED)
            pktlen = pl_sel[(phymac[0].channel >> 7)];
#       else
            pktlen = 7;
#       endif

        /// ot_queue manipulation to fit background frame into common model
        q_empty(&rxq);
     //#rxq.length      = pktlen + 2;
        rxq.front[0]    = pktlen;
        rxq.front[1]    = 0;
        rxq.getcursor   = &rxq.front[2];
        rxq.putcursor   = &rxq.front[2];
        //radio.rxlimit   = pktlen;
        sub_buffer_config(64, pktlen);

        /// Start Decoder
        /// - Enable direct RX termination based on RSSI value, which is not
        ///   done for Foreground RX (this is b4 in MCSM2)
        /// - Enable RX Timeout interrupt IOCFG0, which will also occur if
        ///   RSSI-based termination occurs.
        submlx_launch_rx(RXTERM_COND_NOCS);
        mlx73_int_turnon(RFI_RSSICS);
    }

#else
    // BLINKER only (no RX)
    callback(RM2_ERR_GENERIC, 0);
#endif
}




void rm2_rxsync_isr() {
/// Reset the radio interruptor to catch the next RX FIFO interrupt, having
/// qualified the Sync Word.  rm2_rxdata_isr() will be called on that interrupt.
    if (sub_killonlowrssi() == False) {
        sys_set_mutex(SYS_MUTEX_RADIO_DATA);
        //sys_quit_rf();
        mlx73_iocfg_rxdata();
        mlx73_intcfg_rxdata();
    }
}



void rm2_rxtimeout_isr() {
    sub_kill(RM2_ERR_TIMEOUT, 0);
}




void rm2_rxdata_isr() {
#if (SYS_RECEIVE == ENABLED)
    if ( sub_killonlowrssi() )
        return;

    radio.fifoest = mlx73_read(RFREG(RXFIFOCNT));
    em2_decode_data(); // Loads from FIFO & Contains logic to prevent over-run

    switch ((radio.state >> RADIO_STATE_RXSHIFT) & (RADIO_STATE_RXMASK >> RADIO_STATE_RXSHIFT)) {

        /// RX State 0: Multiframe packets
#       if (M2_FEATURE(MULTIFRAME) == ENABLED)
        case (RADIO_STATE_RXMFP >> RADIO_STATE_RXSHIFT): {
            rm2_rxpkt_MFP:
            ot_int frames_left = em2_remaining_frames();

            // If this is the last frame, move to single-frame packet mode
            // Else if more frames, but current frame is done, page it out.
            // Else, RX is in the middle of a frame (do nothing)
            if (frames_left == 0) {
                radio.state = RADIO_STATE_RXPAGE;   // (does not break-out of switch)
            }
            else if (em2_remaining_bytes() == 0) {
                /// @todo: I might require in the future that queue rebasing is
                ///        done in the evtdone callback (gives more flexibility).
                radio.evtdone(frames_left, (ot_int)sub_check_crc() );            // arg 2 is negative on bad Frame CRC

                // Prepare the next frame by moving the "front" pointer and
                // re-initializing the decoder engine
                q_rebase(&rxq, rxq.putcursor);
                em2_decode_newframe();

                // Clear out what's leftover in the FIFO, from the new frame,
                // and re-do boundary checks on this new frame.
                radio.fifoest = mlx73_read(RFREG(RXFIFOCNT));
                em2_decode_data();
                goto rm2_rxpkt_MFP;
            }
            else {
                break;
            }
        }
#       endif

        /// RX State 1: RX'ing a packet that is buffered without HW control
#       if ((M2_FEATURE(MULTIFRAME) == ENABLED) || (M2_FEATURE(FEC_RX) == ENABLED))
        case (RADIO_STATE_RXPAGE >> RADIO_STATE_RXSHIFT): {
            ot_uint remaining_bytes = em2_remaining_bytes();
            if (remaining_bytes <= (255-64) ) {
                if (remaining_bytes <= mlx73_read(RFREG(RXFIFOCNT))) {
                    mlx73_int_force(RF_PKTDONE);
                }
                else {
                    sub_buffer_config(64, remaining_bytes);
                    mlx73_int_turnoff(RFINT(RXFIFOTHR));    // kill RX-Full int
                }
                radio.state = RADIO_STATE_RXDONE;
            }
            break;
        }
#       endif

        /// RX State 2: RX'ing of a packet that is maintained by HW control
        case (RADIO_STATE_RXAUTO >> RADIO_STATE_RXSHIFT): {
            // Automatic RX should not actually need any management
            break;
        }

        /// Bug Trap
        default:
            sub_kill(RM2_ERR_GENERIC, 0);
            break;
    }

#endif
}



void rm2_rxend_isr() {
    ot_int kill_status;
    radio.fifoest = mlx73_read(RFREG(RXFIFOCNT));
    em2_decode_data(); // Loads from FIFO & Contains logic to prevent over-run

    if (radio.flags & RADIO_FLAG_ERROR) {
        kill_status = 1;
    }
    else {
        kill_status = sub_check_crc();
    }
    sub_kill(0, kill_status );
}




void rm2_txinit_ff(ot_int est_frames, ot_sig2 callback) {
    radio.state     = RADIO_STATE_TXCCA1;
#   if (M2_FEATURE(MULTIFRAME) == ENABLED)
        radio.flags = (est_frames > 1) << 5;    //sets RADIO_FLAG_FRCONT
#   else
        radio.flags = 0;
#   endif
    radio.evtdone   = callback;
    txq.getcursor   = txq.front;
    txq.front[1]    = phymac[0].tx_eirp;
    sub_prep_q(&txq);
    radio_flush_tx();
    em2_encode_newpacket();
    em2_encode_newframe();
}




void rm2_txinit_bf(ot_sig2 callback) {
#if (SYS_FLOOD == ENABLED)
    radio.state     = RADIO_STATE_TXCCA1;
    radio.flags     = RADIO_FLAG_FLOOD;
    radio.evtdone   = callback;
    txq.getcursor   = txq.front;
    sub_prep_q(&txq);
    radio_flush_tx();
    em2_encode_newpacket();
    em2_encode_newframe();
    sub_buffer_config(PACKET_LEN_FIXED, em2_remaining_bytes());
    em2_encode_data();
    // Note usage of sub_buffer_config():
    // (packet bytes depend on radio HW and encoding method.  Encoder knows.)
#endif
}





void rm2_flood_txstop() {
#if (SYS_FLOOD == ENABLED)
    radio.flags = 0;
    radio.state = RADIO_STATE_TXDONE;
#endif
}





ot_int rm2_txcsma() {
    // The shifting in the switch is so that the numbers are 0, 1, 2, 3...
    // It may seem silly, but it allows the switch to be compiled better.
    switch ( (radio.state >> RADIO_STATE_TXSHIFT) & (RADIO_STATE_TXMASK >> RADIO_STATE_TXSHIFT) ) {

        /// 1. First CCA
        /// - Calibrate TX if necessary
        /// - If CSMA-CA is disabled, then just make sure channel is allowed.
        /// - Else, begin CSMA process with first CCA
        case (RADIO_STATE_TXCCA1 >> RADIO_STATE_TXSHIFT): {
            mlx73_int_turnoff(RFI_ALL);

            if (dll.comm.csmaca_params & M2_CSMACA_NOCSMA) {
                if (sub_nocsma_init() == False) {
                    return RM2_ERR_BADCHANNEL;
                }
                radio.state = RADIO_STATE_TXSTART;
                radio_idle();   // Turn on Radio XTAL & Vdig for faster TX entry
                return 0;
            }

            mlx73_iocfg_txcsma();           //program IO for CSMA mode
            if (sub_csma_init() == False) {
                radio_sleep();
                return RM2_ERR_CCAFAIL;
            }
            radio.state = RADIO_STATE_TXCCA2;
            return phymac[0].tg;
        }

        /// 2. Second CCA
        case (RADIO_STATE_TXCCA2 >> RADIO_STATE_TXSHIFT): {
            if (sub_cca2() == False) {
                radio_sleep();
                radio.state = RADIO_STATE_TXCCA1;
                return RM2_ERR_CCAFAIL;
            }
            radio.state = RADIO_STATE_TXSTART;
            return 0;
        }

        /// 3. TX startup
        case (RADIO_STATE_TXSTART >> RADIO_STATE_TXSHIFT): {
            mlx73_iocfg_txdata();
#           if (SYS_FLOOD == ENABLED)
            {   /// Conditional packet flooding config (only needed on Gateways, subcontrollers)
                Sync_Class sync_class;
                sync_class  = (radio.flags&RADIO_FLAG_FLOOD) ? SYNC_Class_0 : SYNC_Class_1;
                sub_syncword_config(sync_class);
                mlx73_write( RFREG(MCSM1), \
                        (RFDEF(MCSM1)&~TXOK_ACT)|(radio.flags&RADIO_FLAG_FLOOD) );
            }
#           else
                sub_syncword_config(SYNC_Class_1);
                mlx73_write(RFREG(MCSM1), (RFDEF(MCSM1) & ~TXOK_ACT));
#           endif
#           ifdef MLX_PROTOTYPE
                mlx73_write_bank(BANK_0, (0x11<<1), 0x60);  // Force-ON PA and Preamp
                mlx73_write_bank(BANK_0, (0x12<<1), 0x10);  // Force-ON Quaddiv
#           endif

            /// Wait for Crystal to be ready, then dump a page to the FIFO.
            /// If there are any remaining bytes, go into data-paging mode.
            while((RADIO_IRQ0_PORT->IDR & RADIO_IRQ0_PIN) == 0);

            ///@todo In the future, this could be just a few bytes to start things.
            em2_encode_data();
            radio.state = em2_remaining_bytes() ? RADIO_STATE_TXDATA : RADIO_STATE_TXDONE;

            {   /// Initiate TX by updating STATUS1 register
                ot_u8 tx_config = (RFMODE_TX | 0x80 | 0 | TXFIFO_STATUS_INUSE);   //0x18 / (3<<2), (6<<2)
                ///@todo need to get TXRECAL figured-out, probably need 2nd flags byte.
                //if (radio.flags & RADIO_FLAG_TXRECAL) {
                //    radio.flags ^= RADIO_FLAG_TXRECAL;
                    //tx_config   |= 0x80;
                //}
                mlx73_write(RFREG(STATUS0), tx_config); //Note: Write to 0x2F
            }
            mlx73_intcfg_txdata();
            //{
            //    ot_u8 message[3];
            //    message[0] = mlx73_read(0x2E<<1);
            //    message[1] = mlx73_read_bank(BANK_0, 0x38<<1);
            //    message[2] = mlx73_read_bank(BANK_0, 0x39<<1);
            //    logger_hexmsg(4, 3, "TEST", message);
            //}

            //mlx73_coredump(-1);
            //mlx73_coredump(0);
            //sub_freqdump();

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
                radio.fifoest = mlx73_read(RFREG(TXFIFOCNT));
                em2_encode_data();
                break;
            }

            /// If the frame is done (em2_remaining_bytes() == 0) and there are
            /// no more frames to transmit, then this interrupt is due to a low
            /// threshold, and we just need to turn-off the threshold interrupt
            /// and wait for the last bit of data to get sent.
            if (em2_remaining_frames() == 0) {
                radio.state     = RADIO_STATE_TXDONE;
                //radio.txlimit   = 0;
                mlx73_int_turnoff(RFINT(TXFIFOTHR));
                break;
            }

            /// If the frame is done, but more need to be sent (e.g. MFP's)
            /// queue it up.  The additional encode stage is there to fill up
            /// what's left of the buffer.
#       if (M2_FEATURE(MULTIFRAME) == ENABLED)
            if (radio.flags & RADIO_FLAG_FRCONT) {
                q_rebase(&txq, txq.getcursor);
                radio.evtdone(1, 0);        //callback action for next frame
                em2_encode_newframe();
                txq.front[1] = phymac[0].tx_eirp;
                radio.fifoest = mlx73_read(RFREG(TXFIFOCNT));
                em2_encode_data();
                goto rm2_txpkt_TXDATA;
            }
#       endif
        }

        /// 5. Conclude the TX process, and wipe the radio state
        //     turn off any remaining TX interrupts
        case (RADIO_STATE_TXDONE >> RADIO_STATE_TXSHIFT): {
#       if (SYS_FLOOD == ENABLED)
            /// Packet flooding.  Only needed on devices that can send M2AdvP
            if (radio.flags & RADIO_FLAG_FLOOD) {
                radio.evtdone(2, 0);
                txq.getcursor = txq.front;
                em2_encode_newpacket();
                em2_encode_newframe();
                em2_encode_data();
            }
            else
#       endif
            {
                sub_kill(0, 0);
                break;
            }
        }

        default: // Bug trap
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
    radio.state = 0;
    sys_quit_rf();
    radio_gag();

#   ifndef MLX_PROTOTYPE
        radio_idle();
#   else
        radio_sleep();
        mlx73_write_bank(BANK_0, 0x11, 0x00);   // Force-OFF PA and Preamp
        mlx73_write_bank(BANK_0, 0x12, 0x00);   // Force-OFF Quaddiv
#   endif

    radio.evtdone(main_err, frame_err);
    radio.evtdone = &otutils_sig2_null;
}


ot_bool sub_killonlowrssi() {
#ifdef MLX_PROTOTYPE
    //MLX PROTOTYPE does not have functional RSSI
    return False;
#else
    ot_int min_rssi         = ((phymac[0].cs_thr >> 1) & 0x3F) - 40;
    ot_int detected_rssi    = mlx73_offsetrssi(mlx73_read(RFREG(RSSI)));

    if (detected_rssi < min_rssi) {
        sub_kill(RM2_ERR_LINK, 0);
        return True;
    }
    return False;
#endif
}


ot_u8 sub_rssithr_calc(ot_u8 input, ot_u8 offset) {
///@todo For now, I always report 0, because the spec for MLX73xxx isn't quite complete.
    return 0;
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

    /// Go through the list of tx channels:
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
#ifdef MLX_PROTOTYPE
    //MLX PROTOTYPE does not really have functional CS/RSSI
    radio_idle();

#else
    //radio_idle();                         //send radio to idle mode (already done)
    mlx73_write(RFREG(TMRSTATUS), 0);       //Clear Timer Flag

    { //turn on RX, and then make sure chip goes to sleep afterwards
        ot_u8 rx_config = (radio.flags&RADIO_FLAG_RXRECAL) | (RFMODE_RX)
        radio.flags    &= ~RADIO_FLAG_RXRECAL;
        mlx73_write(RFREG(STATUS1), rx_config);
    }
    mlx73_intcfg_txcsma();
    MCU_SLEEP_WHILE_RF();               //wait for RSSI_valid interrupt (block)
    mlx73_int_turnoff(RFI_ALL);         //turn off all csma interrupts
#endif

    return radio_check_cca();           //Check value of RFRX_RSSICS pin (pin 3)
}


ot_bool sub_noscan() {
    return True;
}




ot_bool sub_csma_init() {
/// Called directly by TX radio function(s)
/// Duty: Initialize csma process, and run the first scan (of two).
    { // Write CS threshold, RX Timeout, and rx termination spec
        ot_u8 data[3];
        data[0] = RFREG(CSCFG);
        data[1] = mlx73_calc_csthr(phymac[0].cs_thr);
        data[2] = mlx73_calc_rxtimeout(1);
        data[3] = (RFDEF(MCSM0) & 0x07) | RXTERM_COND_ON;
        mlx73_spibus_io(0, 4, 0, data, NULL);
    }

    /// Setup channel, scan it, and power down RF on scan fail
    return sub_chan_scan( &sub_cca_scan );
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
    ot_uni16    scratch;

    ///@note there is a possible Fc corruption bug, consider commenting-out
    /// Only do the channel lookup if the new channel is different than before
    //if (chan_id == phymac[0].channel) {
    //    return True;
    //}

    /// If FEC is requested by the new channel, but this device does not support
    /// FEC, then make sure to return False.
    fec_id = chan_id & 0x80;
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
    /// In this case, no reconfiguration is necessary.
    spectrum_id = chan_id & 0x7F;
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

    //ot_u8 fc_i;

#   if (M2_FEATURE(FEC) == ENABLED)
    /// Only worry about changing encoding method if:
    /// (a) Optional FEC is enabled (otherwise only one kind of encoding)
    /// (b) The last-used channel had a different encoding type

        if ( (old_chan ^ phymac[0].channel) & 0x80 ) {
            ot_u8 pktcfg0_val;
            ot_u8 datacfg_val;

            if (phymac[0].channel & 0x80) {
                /// Set up transceiver for FEC
                /// set Sync Word qualifier to allow up to two bit errors
                /// clear data whitening, auto-packet, & crc options
                pktcfg0_val = (RFDEF(PKTCFG0) & 0x20) | PATTERN_WORD_LEN_24 | PATTERN_MAX_ERR_2;
                datacfg_val = (RFDEF(DATACFG) & 0xC8) | EN_PREAMBLE;
            }
            else {
                /// Set up transceiver for PN9 method
                /// If FEC is disabled, these settings will always be in the regs.
                /// set FEC to OFF and PN9 to ON
                /// set Sync qualifier to allow no bit errors (16/16)
                pktcfg0_val     = (RFDEF(PKTCFG0) & 0x20) | PATTERN_WORD_LEN_24;
                datacfg_val     = (RFDEF(DATACFG) & 0xC8) | EN_DATAWHITE | \
                                    EN_PACKETRX | EN_PREAMBLE | EN_CRC;
                radio.flags    |= RADIO_FLAG_AUTO;
            }

            mlx73_write(RFREG(PKTCFG0), pktcfg0_val);
            mlx73_write(RFREG(DATACFG), datacfg_val);
        }
#   endif

    /// Reprogram the PA Table if eirp of new channel isn't the same as before
    if (old_eirp != phymac[0].tx_eirp) {
        sub_set_txpower( phymac[0].tx_eirp );
    }

    /// All x0000xxx channels should be x0000111
    phymac[0].channel |= (phymac[0].channel & 0x70) ? 0 : 7;

    /// (Don't reprogram if radio is using the same channel class already)
    if ( (old_chan ^ phymac[0].channel) & 0x70 ) {
        static const ot_u8 regs[5]  = { RFREG(RXDSPCFG),
                                        RFREG(MDMCFG1),
                                        RFREG(MDMCFG2),
                                        RFREG(PREAMBLE),
                                        RFREG(DEMODCFG)
                                      };
        static const ot_u8 turbo[5] = { (RFDEF(RXDSPCFG) & 0x0F) | EN_CORRECT_ISI | EN_FINE_RECOV,
                                        (RFDEF(MDMCFG1) & 0xF8) | DR_E_TURBO,
                                        DR_M_TURBO,
                                        6,
                                        (RFDEF(DEMODCFG) & 0xF0) | CHANBW_300kHz
                                      };
        static const ot_u8 normal[5] = {(RFDEF(RXDSPCFG) & 0x0F) | EN_ROUGH_RECOV,
                                        (RFDEF(MDMCFG1) & 0xF8) | DR_E_NORMAL,
                                        DR_M_NORMAL,
                                        4,
                                        (RFDEF(DEMODCFG) & 0xF0) | CHANBW_200kHz
                                       };
        ot_u8 i;
        ot_u8* lookup;
        lookup = (phymac[0].channel & 0x20) ? (ot_u8*)turbo : (ot_u8*)normal;

        for (i=0; i<5; i++) {
            mlx73_write(regs[i], lookup[i]);
        }
    }

    ///@note there is a possible Fc corruption bug, consider commenting-out
    /// Reprogram channel frequency and stage recalibration, but don't do these
    /// things if radio is already set on this channel center frequency
    //if ((phymac[0].channel ^ old_chan) & 0x0F) {
        mlx73_set_fc( phymac[0].channel & 0x0F );
        radio.flags |= (RADIO_FLAG_TXRECAL | RADIO_FLAG_RXRECAL);
    //}
}




void sub_syncword_config(Sync_Class sync_class) {
/// Class 0: E6D0 (no fec), F498 (fec)
/// Class 1: 0B67 (no fec), 192F (fec)
    ot_u8 data[4];
    ot_u8 index;

#   if (M2_FEATURE(FEC) == ENABLED)
        static const ot_u8 syncword[8] = { 0xD0, 0xE6, 0x67, 0x0B, 0x98, 0xF4, 0x2F, 0x19 };
        index = ((phymac[0].channel >> 5) & 0x04) + (ot_u8)sync_class;
#   else
        static const ot_u8 syncword[4] = { 0xD0, 0xE6, 0x67, 0x0B };
        //static const ot_u8 syncword[4] = { 0x0B, 0x67, 0xE6, 0xD0 }; //lsb first, little endian
        index = (ot_u8)sync_class;
#   endif

    data[0] = RFREG(PATTERN0);
    data[1] = syncword[index];
    data[2] = syncword[index+1];
    data[3] = 0xAA;

    mlx73_spibus_io(0, 4, 0, data, NULL);
}




void sub_buffer_config(ot_u8 mode, ot_u8 param) {
/// mode MUST be 0 or 64: 0 = variable/infinite length packet, 64 = fixed length.
/// param: sets packet length on mode == 64

/// The MLX73xxx has some features that simplify buffer configuration.  (1) It
/// has a huge FIFO.  (2) It has multiframe packet support.  (3) Buffer config
/// is mostly just dependent on if you are using FEC or not, and that is done
/// in sub_chan_config (right when channel is being selected).

    ot_u8 data[3];
    data[0] = RFREG(PKTCFG1);               // address (0x19)
    data[1] = RFDEF(PKTCFG1) | mode;        // variable (0) or fixed (0x40)
    data[2] = (mode != 0) ? param : 255;    // PKT Length

    mlx73_spibus_io(0, 3, 0, data, NULL);
}




void sub_prep_q(ot_queue* q) {
/// Put some special data in the queue options field.
/// Lower byte is encoding options (i.e. FEC)
/// Upper byte is processing options (i.e. CRC)
#if (M2_FEATURE(FEC) == ENABLED)
    q->options.ubyte[LOWER] = (phymac[0].channel & 0x80);
    q->options.ubyte[UPPER] = 1;
#endif
}




ot_int sub_check_crc() {
///Returns 0 when CRC passes
///MLX73 does not support FEC, so CRC on FEC packets needs to be done in SW.
#if ((M2_FEATURE(FEC_RX) == ENABLED) && (RF_FEATURE(CRC) == ENABLED))
    if (rxq.options.ubyte[LOWER]) {
        return crc_get(&em2.crc);
    }
    return (ot_int)mlx73_check_crc();

#elif (RF_FEATURE(CRC) == ENABLED)
    return (ot_int)mlx73_check_crc();

#else
    return (ot_int)crc_get(&em2.crc);

#endif
}


/*
ot_int sub_eta(ot_int next_int) {
/// Subtract by a tolerance amount, floor at 0
    ot_int eta;
    eta     = rm2_scale_codec(next_int) - 1;
    eta    *= (eta > 0);
    return eta;
}


ot_int sub_eta_rxi() {
    return sub_eta( radio.rxlimit - (ot_int)mlx73_read(RFREG(RXFIFOCNT)) );
}

ot_int sub_eta_txi() {
    return sub_eta( (ot_int)mlx73_read(RFREG(TXFIFOCNT)) - radio.txlimit );
}
*/




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
///@note the power settings are not yet in the MLX spec.  The function below
/// always puts in 0x3F
    mlx73_set_txpwr(eirp_code);
}





