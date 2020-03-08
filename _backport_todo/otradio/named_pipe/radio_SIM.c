
#include <otsys/types.h>
#include <otsys/config.h>
#include <otlib/utils.h>
#include <otplatform.h>

#include <m2/radio.h>
#include "encode.h"
#include <otlib/buffers.h>
#include <otlib/queue.h>
#include <otsys/veelite.h>
#include <m2/session.h>
#include <otlib/crc16.h>

//#include "radio_SIM.h"      // register definitions file
#include <stdio.h>
#include <otsys/stat.h>    // for open()
#include <fcntl.h>    // for open()
#include <unistd.h>    // for write()

#include "radio_pipe.h"

#define RADIO_STATE_RXSHIFT     0
#define RADIO_STATE_RXMASK      (0xf << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXINIT      (0 << RADIO_STATE_RXSHIFT)

#define RADIO_STATE_TXSHIFT     4
#define RADIO_STATE_TXMASK      (0xf << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXCCA_INIT  (0 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXCCA_      (1 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXSTART     (2 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXDATA      (3 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXDONE      (4 << RADIO_STATE_TXSHIFT)


#define RADIO_FLAG_FRCONT       (1 << 0)
#define RADIO_FLAG_FLOOD        (1 << 1)
#define RADIO_FLAG_AUTO         (1 << 2)
#define RADIO_FLAG_RESIZE       (1 << 3)

/** PHY-MAC Array declaration
  * Described in radio.h of the OTlib.
  * This driver only supports M2_FEATURE(MI_CHANNELS) = 1.
  */
phymac_struct   phymac[M2_PARAM_MI_CHANNELS];


radio_struct radio;

void
radio_gag()
{
    // would a simulated radio do anything in sleep or standby?
}

void
radio_sleep()
{
}

ot_int rm2_default_tgd(ot_u8 chan_id) {
    //ot_int  output;

#if ((M2_FEATURE(FEC) == DISABLED) && (M2_FEATURE(TURBO) == DISABLED))
    //output = M2_TGD_55FULL;
    return M2_TGD_55FULL;

#elif ((M2_FEATURE(FEC) == DISABLED) && (M2_FEATURE(TURBO) == ENABLED))
    //output = (chan_id & 0x60) ? M2_TGD_200FULL : M2_TGD_55FULL;
    return (chan_id & 0x60) ? M2_TGD_200FULL : M2_TGD_55FULL;

#elif ((M2_FEATURE(FEC) == ENABLED) && (M2_FEATURE(TURBO) == DISABLED))
    //output = (chan_id & 0x80) ? M2_TGD_55FULL : M2_TGD_55HALF;
    return (chan_id & 0x80) ? M2_TGD_55FULL : M2_TGD_55HALF;

#elif ((M2_FEATURE(FEC) == ENABLED) && (M2_FEATURE(TURBO) == ENABLED))
    /// @note This is an incredible hack, but it is fast and compact.
    /// To understand it, you need to know the way the channel ID works.
    chan_id += 0x20;

    switch ( chan_id >> 6) {
        case 0: return M2_TGD_55FULL;
        case 1: return M2_TGD_200FULL;
        case 2: return M2_TGD_55HALF;
        case 3: return M2_TGD_200HALF;
    }

#else
    // seemingly impossible condition
    return 0;
#endif
    // I commented out this section because I have a simpler way of doing the
    // timing, in the system module.  Basically, the lag can be cancelled out
    // because it will be about the same each time.

    // return Tg minus the time it takes to bring the radio back
    // up, and make sure to set to 0 on negative.
    //output  -= (output < (RADIO_COLDSTART_STI_TYP >> 5)) ?
    //            (RADIO_WARMSTART_STI_TYP >> 5) :
    //            (RADIO_COLDSTART_STI_TYP >> 5);

//#   if ( (RADIO_COLDSTART_STI_TYP >> 5) > 2) || ((RADIO_WARMSTART_STI_TYP >> 5) > 2))
    //    output  *= (output > 0);
//#   endif

    //return output;
}

static void
sub_prep_q(ot_queue* q)
{
/// Put some special data in the queue options field.
/// Lower byte is encoding options (i.e. FEC)
/// Upper byte is processing options (i.e. CRC)
    q->options.ubyte[LOWER]    = (phymac[0].channel & 0x80);
    q->options.ubyte[UPPER]    = 1;
    /*q->options.ubyte[UPPER]    = (RF_FEATURE(PACKET) != ENABLED) || \
                                  (RF_FEATURE(CRC) != ENABLED) || \
                                  ( (RF_FEATURE(PN9) != ENABLED) && \
                                    (q->options.ubyte[LOWER] == 0)) || \
                                  ( (RF_FEATURE(FEC) != ENABLED) && \
                                    (q->options.ubyte[LOWER] == 1));
    */
}

static void
sub_set_txpower( ot_u8 eirp_code )
{
    // on real radio would set tx power
}

static void
sub_chan_config(ot_u8 old_chan, ot_u8 old_eirp)
{
    ot_u8 fc_i;

    /// Center Frequency index = lower four bits channel ID
    fc_i = (phymac[0].channel & 0x0F);

    printf("sub_chan_config(0x%x, %d) [34mfc_i=%d[0m\r\n", old_chan, old_eirp, fc_i);
    /// Reprogram the PA Table if eirp of new channel isn't the same as before
    if (old_eirp != phymac[0].tx_eirp) {
        sub_set_txpower( phymac[0].tx_eirp );
    }

    if ( (old_chan ^ phymac[0].channel) & 0x70 ) {
        printf("new bitrate %x\r\n", (phymac[0].channel >> 4) );
        switch ((phymac[0].channel >> 4) & 0x03) {
            case 0: fc_i = 7;
            case 1:     /// 55 kS/s method
                break;
            case 2:
            case 3:     /// 200 kS/s method
                break;
        }
    }

    if ( fc_i != (old_chan & 0x0F) ) {
        fc_i &= 0xf;
        printf("new freq 0x%x\r\n", fc_i);

    }

}

static ot_bool
sub_channel_lookup(ot_u8 chan_id, vlFILE* fp)
{
/// Called during channel scans.
/// Duty: (a) See if the supplied channel is supported on this device & config.
///       If yes, return true.  (b) Determine if recalibration is required
///       before changing to the new channel, and recalibrate if so.

    ot_u8       fec_id;
    ot_u8       spectrum_id;
    ot_int      i;
    ot_uni16    scratch;

    //printf("sub_channel_lookup(%x)", chan_id);
    /// Only do the channel lookup if the new channel is different than before
    if (chan_id == phymac[0].channel) {
        //printf("sub_channel_lookup(%x) same\r\n", chan_id);
        return True;
    }
    printf("sub_channel_lookup(%x) ", chan_id);

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
        printf("fec_id ");
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
            printf(" no-fec ");
            return False;
#       endif
    }

    /// 0x7F is the wildcard spectrum id.  It means use same spectrum as before.
    /// In this case, of course no recalibration is necessary.
    if (spectrum_id == 0x7F) {
        printf("wildcard spectrum ID\r\n");
        return True;
    }
    printf("\r\n");

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
            printf("sid=0x%x cca_thr=%d\r\n", spectrum_id, phymac[0].cca_thr);
            /* 0 = -140dBm, 140=0dBm, 60=80dBm */

            /// value 0-127 that is: input - 140 = threshold in dBm
            // todo: phymac[0].cs_thr

            printf("phymac[0].channel=0x%x\r\n", phymac[0].channel);
            sub_chan_config(old_chan_id, old_tx_eirp);
            return True;
        }
    }

    printf("spectrum_id %x not found ", spectrum_id);
    return False;
}

#if (SYS_RECEIVE == ENABLED)
static ot_bool sub_test_channel(ot_u8 channel, ot_u8 netstate) {
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
}
#endif

#if (SYS_RECEIVE == ENABLED)
static void
sub_syncword_config(ot_u8 sync_class)
{
#if 0
    ot_uni16 sync_value;

#   if (M2_FEATURE(FEC) == ENABLED)
        if (phymac[0].channel & 0x80) {   //Actual= 0xF498 : 0x192F;
            sync_value.ushort = (sync_class == 0) ? 0xF498 : 0x192F;
        }
        else
#   endif
    {                                 //Actual= 0xE6D0 : 0x0B67;
        sync_value.ushort = (sync_class == 0) ? 0xE6D0 : 0x0B67;
    }

    /* sync_value is transmitted in start_tx() */

    /* sync byte order is for CC430 compatability:
     * for example, where 0x0b67 is used, 0x67 is transmitted first in time, then 0x0b.
     * And MSbit (of each byte) is sent first.
     * @todo: validate this against Dash7 specification */
    //printf("sync %02x %02x\r\n", sync_value.ubyte[LOWER], sync_value.ubyte[UPPER]);
#endif /* #if 0 */
}
#endif /* if (SYS_RECEIVE == ENABLED)  */

static void
sub_kill(ot_int main_err, ot_int frame_err)
{
    rxtim.int_enable = 0;

    radio.state = 0;
    sys_quit_rf();
    radio_gag();

    radio_pipe_close();

    radio.evtdone(main_err, frame_err);
    radio.evtdone = &otutils_sig2_null;
}

void
rx_done_isr(ot_int pcode)
{
    ot_int c;
    ot_int dBm = -140;

    // argument 2 is negative on bad Frame CRC
    c = (ot_int)crc_check();
//    radio.evtdone(0, (ot_int)crc_check() - 1);
    if (c)
        printf("[42mcrc ok");   // green
    else
        printf("[41mcrc fail"); // red

    printf("[0m rssi=%ddBm c=%d pcode=%d\r\n", dBm, c, pcode);
    //debug_printf("crc_check(): %x\r\n", c);
    sub_kill(pcode, c-1);
}

/** Initializes RX engine for "foreground" packet reception
  */
void rm2_rxinit_ff(ot_u8 channel, ot_u8 netstate, ot_int est_frames, ot_sig2 callback)
{
#if (SYS_RECEIVE == ENABLED)

#ifdef RADIO_DEBUG
    printf("rm2_rxinit_ff(0x%x, 0x%x, %d) %d\r\n", channel, netstate, est_frames, dll.comm.rx_timeout);
#endif

    if (sub_test_channel(channel, M2_NETSTATE_UNASSOC) == False) {
        printf("ff [41mFalse = sub_test_channel(%d)[0m\r\n", channel);
    }

    radio.state     = RADIO_STATE_RXINIT;
#   if (M2_FEATURE(MULTIFRAME) == ENABLED)
        radio.flags = (est_frames > 1); //sets RADIO_FLAG_FRCONT
#   else
        radio.flags = 0;
#   endif
    radio.rxlimit   = 8;
    radio.evtdone   = callback;

    q_empty(&rxq);

    sub_prep_q(&rxq);
    em2_decode_newpacket();
    em2_decode_newframe();

    sub_syncword_config(1);
    if (open_pipe_for_rx(dll.comm.rx_timeout, phymac[0].channel)) {
        callback(RM2_ERR_GENERIC, 0);
        return;
    }

    rxtim.compare = dll.comm.rx_timeout;
    //printf("setting rxtim.compare: %d\n", rxtim.compare);
    rxtim.count = 0;
    rxtim.int_enable = 1;
    rxtim.enabled = 1;

    //sx1212_print_regs();    // tmp debug remove me

#else /* (SYS_RECEIVE == ENABLED) */
    // BLINKER only (no RX)
    callback(RM2_ERR_GENERIC, 0);
#endif /* (SYS_RECEIVE == ENABLED) */
}

void
rm2_rxinit_bf(ot_u8 channel, ot_sig2 callback)
{
#if (SYS_RECEIVE == ENABLED)

    printf("rm2_rxinit_bf(%d)\r\n", channel);

    if (sub_test_channel(channel, M2_NETSTATE_UNASSOC) == False) {
        printf("bf [41mFalse = sub_test_channel(%d)[0m\r\n", channel);
    }

    radio.state     = RADIO_STATE_RXINIT;
    radio.flags     = RADIO_FLAG_FLOOD;
    radio.rxlimit   = 8;
    radio.evtdone   = callback;
    radio.rx_timeout_count = 0;

    q_empty(&rxq);

    sub_prep_q(&rxq);
    em2_decode_newpacket();
    em2_decode_newframe();

    sub_syncword_config(0);

    if (open_pipe_for_rx(dll.comm.rx_timeout, phymac[0].channel)) {
        callback(RM2_ERR_GENERIC, 0);
        return;
    }

    rxtim.compare = dll.comm.rx_timeout;
    //printf("setting rxtim.compare: %d\n", rxtim.compare);
    rxtim.count = 0;
    rxtim.int_enable = 1;
    rxtim.enabled = 1;

    /* background scan requires minimum rssi (Esm) prior to sync word */
    /* terminate scan immediately if rssi below Esm */
    /* otherwise wait Tbsd for sync word (background scan detection timeout) */
#else
    // BLINKER only (no RX)
    callback(RM2_ERR_GENERIC, 0);
#endif
}

// Initializes TX engine for "foreground" packet transmission
// est_frames  (ot_int) Number of frames in packet to transmit
// callback    (ot_sig2) callback for when TX is done, on error or complete
void rm2_txinit_ff(ot_int est_frames, ot_sig2 callback)
{
#ifdef RADIO_DEBUG
    printf("txinit_ff est_frames=%d\r\n", est_frames);
#endif
    //sx1212_print_regs();    // tmp debug remove me

    // set up for CCA: rx mode
    /* for real radio would put into rx mode here for CCA */

    radio.state     = RADIO_STATE_TXCCA_INIT;
    radio.flags     = (est_frames > 1);
    radio.evtdone   = callback;
    radio.unlock_count   = 0;
    radio.rssi_sum    = 0;
    radio.rssi_count = 0;

    /// Prepare the foreground frame packet
    txq.getcursor   = txq.front;
    txq.front[1]    = phymac[0].tx_eirp;

    sub_prep_q(&txq);

}

int rf_buf_idx;
ot_u8 rf_buf[632];
//int air_fd = -1;

ot_bool
radio_txopen_4()
{
    if (rf_buf_idx < (sizeof(rf_buf)-4) )
        return True;    // air ok
    else
        return False;    // no room
}

// radio_txopen(): Checks the TX buffer to see if at least 1 more byte can fit in it
ot_bool
radio_txopen()
{
    if (rf_buf_idx < sizeof(rf_buf))
        return True;    // air ok
    else
        return False;    // no room

/*    if (air_fd == -1)
        return False;    // air not open
    else
        return True;    // air ok
        */
}

// radio_putbyte(): Puts a byte to the TX radio buffer
void radio_putbyte(ot_u8 databyte)
{
/*    int r;
    printf("write...\n");
    r = write(air_fd, &databyte, 1);
    printf("%d = write(%02x)\r\n", r, databyte);*/

    rf_buf[rf_buf_idx++] = databyte;
    //printf("%d %x\n", rf_buf_idx, databyte);
}

void
radio_putfourbytes(ot_u8* data)
{
    rf_buf[rf_buf_idx++] = data[0];
    rf_buf[rf_buf_idx++] = data[1];
    rf_buf[rf_buf_idx++] = data[2];
    rf_buf[rf_buf_idx++] = data[3];
}

// Checks the RX buffer to see if there are at least 1 more byte in it
ot_bool
radio_rxopen()
{
    if (rx_buf_in_idx != rx_buf_out_idx)
        return True;
    else
        return False;
}

// Checks the RX buffer to see if there are at least 4 more bytes in it
ot_bool
radio_rxopen_4()
{
    int n;

    if (rx_buf_in_idx > rx_buf_out_idx)
        n = rx_buf_in_idx - rx_buf_out_idx;
    else {
        int i = rx_buf_in_idx + sizeof(rx_buf);
        n = i - rx_buf_out_idx;
    }

    if (n >= 4)
        return True;
    else
        return False;
}

// Gets 4 bytes from the RX radio buffer
void
radio_getfourbytes(ot_u8* data)
{
    int i;

    for (i = 0; i < 4; i++) {
        data[i] = rx_buf[rx_buf_out_idx];
        if (++rx_buf_out_idx == sizeof(rx_buf))
            rx_buf_out_idx = 0;
    }
}

ot_u8
radio_getbyte()
{
    ot_u8 ret;

    ret = rx_buf[rx_buf_out_idx];
    //printf("%02x = rx_buf[%d] getbyte (in=%d)\n", ret, rx_buf_out_idx, rx_buf_in_idx);
    if (++rx_buf_out_idx == sizeof(rx_buf))
        rx_buf_out_idx = 0;

    return ret;
}

void
tx_done_isr()
{
    radio.evtdone(0, 0);
    radio.state = RADIO_STATE_TXDONE;
}

ot_bool sub_chan_scan( void ) {
    vlFILE* fp;
    ot_int  i;

    fp = ISF_open_su( ISF_ID(channel_configuration) );
    ///@todo assert fp

    /// Go through the list of tx channels
    /// - Make sure the channel ID is valid
    /// - Make sure the transmission can fit within the contention period.
    /// - Scan it, to make sure it can be used
    for (i=0; i<dll.comm.tx_channels; i++) {
        //printf("scan lookup %d:%02x: ", i, dll.comm.tx_chanlist[i]);
        if (sub_channel_lookup(dll.comm.tx_chanlist[i], fp) != False) {
            // rm2_txcsma() is measuring rssi for energy detection
            break;
        }
        //printf("\r\n");
    }

    vl_close(fp);
    return (ot_bool)(i < dll.comm.tx_channels);
}

static ot_bool
sub_nocsma_init()
{
/// Called directly by TX radio function(s) when CSMA is disabled via System
/// Duty: bypass CSMA scan
    return sub_chan_scan( );
}

static ot_bool
sub_csma_init() {
/// Called directly by TX radio function(s)
/// Duty: Initialize csma process, and run scan.
    ot_bool cca1_status;

    /// Setup channel, scan it, and power down RF on scan fail
    cca1_status = sub_chan_scan( );
    if (cca1_status == False) {         //Optimizers may remove this if() for
        radio_sleep();                  //certain implementations
    }

    return cca1_status;
}

extern char tx_done;

ot_int
rm2_txcsma()
{
    // retval -1: channel is clear
    // retval non negative: approx callback time guard time
    // retval -2: rm2 cca failed
    if (radio.state == RADIO_STATE_TXCCA_INIT) {
        if (dll.comm.csmaca_params & M2_CSMACA_NOCSMA) {
            if (sub_nocsma_init()) {
                radio.state = RADIO_STATE_TXSTART;
                return 0;
            }
            return RM2_ERR_BADCHANNEL;
        }

        if (sub_csma_init() == False) {
            return RM2_ERR_CCAFAIL;
        }

        radio.state = RADIO_STATE_TXCCA_;
        //printf("rm2_txcsma() CCA_INIT\n");
        return 1;
    } else if (radio.state == RADIO_STATE_TXCCA_) {
        radio.state = RADIO_STATE_TXSTART;
        //printf("rm2_txcsma() CCA\n");
        return 1;    // channel is clear
    } else if (radio.state == RADIO_STATE_TXSTART) {
        //printf("rm2_txcsma() TX\n");

        em2_encode_newpacket();    // move to txinit_ff
        em2_encode_newframe();    // move to txinit_ff
        if (em2_encode_data == NULL) {
            /* fail in a predictable way */
            radio.evtdone(RM2_ERR_GENERIC, 0);
            return -1;
        }

        rf_buf_idx = 0;

        em2_encode_data();  // this encodes *entire* message, calling radio_putbyte() for each
        //printf("em2_encode_data() returned\n");
        if (open_pipe_for_tx(rf_buf, rf_buf_idx, phymac[0].channel)) {
            return -2;
        }
        tx_done = 1;    // cause tx_done_isr to be called

        return -1;    // channel is clear
    } else
        return -2;    // cca failed: unknown state

}

ot_int
rm2_scale_codec(ot_int buf_bytes)
{
/// Turns a number of bytes (buf_bytes) into a number of ti units.
/// To refresh your memory: 1 ti = ((1sec/32768) * 2^5) = 2^-10 sec = ~0.977 ms

    /// Pursuant to DASH7 Mode 2 spec, b6:4 of channel ID corresponds to kS/s.
    /// 55.555 kS/s = 144us per buffer byte
    /// 200.00 kS/s = 40us per buffer byte
    buf_bytes *= ((phymac[0].channel & 0x70) < 0x20) ? 144 : 40;

    /// Divide us into Ticks
    /// (shift right 10 = divide by 1024)
#   if ((M2_FEATURE(FEC) == ENABLED) && (RF_FEATURE(FEC) == ENABLED))
        buf_bytes >>= (10 - ((phymac[0].channel & 0x80) != 0) );
#   else
        buf_bytes >>= 10;
#   endif

    /// Subtract by a tolerance amount, floor at 0
    buf_bytes  -= 1;
    buf_bytes  *= (buf_bytes > 0);

    return buf_bytes;
}

ot_int rm2_txpkt_duration(ot_int pkt_bytes) {
    return rm2_scale_codec(pkt_bytes + 8);
}

//ot_u8 fake_rssi = 0;

ot_int radio_rssi()
{
    return ( (-60) + 140 );
}





void
radio_init(void)
{
    //int r;

#if 0
    // windows named pipes: \\.\pipe\name
    pipe_init();

    air_fd = open("../../../air", O_WRONLY); //O_RDWR
    if (air_fd == -1)
        perror("air");
    else {
        printf("air_fd = %d\n", air_fd);
        // make it behave like radio channel
        r = fcntl(air_fd, F_SETFL, O_NONBLOCK);
        printf("%d = fcntl()\n", r);
        if (r == -1) {
            perror("fcntl");
        }
    }
#endif /* #if 0 */

    {
        vlFILE* fp          = ISF_open_su( ISF_ID(channel_configuration) );
        phymac[0].channel   = 0x55;         // 55=invalid, forces calibration.
        phymac[0].tx_eirp   = 0x00;
        radio.state         = 0;            // (idle)
        radio.evtdone       = &otutils_sig2_null;

        sub_channel_lookup(0x00, fp);
        vl_close(fp);
    }
}

void rm2_flood_txstop() {
#if (SYS_FLOOD == ENABLED)
    radio.flags &= ~RADIO_FLAG_FLOOD;
#endif
}

// Initializes TX engine for "background" packet flooding
void
rm2_txinit_bf(ot_sig2 callback)
{
#if (SYS_FLOOD == ENABLED)

    printf("txinit_bf %x\r\n", radio.state);

    // set up for CCA: rx mode
    // on real radio, would go to RX mode here

    radio.state     = RADIO_STATE_TXCCA_INIT;
    radio.flags     = RADIO_FLAG_FLOOD;
    radio.evtdone   = callback;
    radio.unlock_count   = 0;
    radio.rssi_sum    = 0;
    radio.rssi_count = 0;

    /// Prepare the foreground frame packet
    txq.getcursor   = txq.front;
    txq.front[1]    = phymac[0].tx_eirp;

    sub_prep_q(&txq);
#endif /* ...SYS_FLOOD == ENABLED */
}


void rm2_kill() {
    sub_kill(RM2_ERR_KILL, 0);
}

void rm2_rxtimeout_isr() {
    sub_kill(RM2_ERR_TIMEOUT, 0);
}

