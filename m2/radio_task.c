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
  * @file       /m2/radio_task.c
  * @author     JP Norair
  * @version    R101
  * @date       14 Jul 2015
  * @brief      Generic Radio (RF transceiver) task
  * @defgroup   Radio (Radio Module)
  * @ingroup    Radio
  *
  *
  ******************************************************************************
  */

#include <otsys/config.h>

#if OT_FEATURE(M2)
#include <m2/radio.h>
#include <m2/dll.h>

#include <otlib/buffers.h>
#include <otsys/veelite.h>




#define _8SYMBOLS_US_00         (605)       //8 symbols @ 13.24 kHz
#define _8SYMBOLS_US_01         (152)       //8 symbols @ 53 kHz
#define _8SYMBOLS_US_10         (76)        //8 symbols @ 106 kHz

#define _OVERHEAD_US_00         (605)       //8 symbols @ 13.24 kHz
#define _OVERHEAD_US_01         (152)       //8 symbols @ 53 kHz
#define _OVERHEAD_US_10         (76)        //8 symbols @ 106 kHz

#ifdef RF_PARAM_PREAMBLE_BYTES
#   define _PREDATA_BYTES       (RF_PARAM_PREAMBLE_BYTES + RF_PARAM_SYNC_BYTES)
#else
#   define _PREDATA_BYTES       (8 + 4)
#endif
#define _PREDATA_US_00          (_PREDATA_BYTES*_8SYMBOLS_US_00)
#define _PREDATA_US_01          (_PREDATA_BYTES*_8SYMBOLS_US_01)
#define _PREDATA_US_10          (_PREDATA_BYTES*_8SYMBOLS_US_10)

#define _8SYMBOLS_MITI_00       (634)       //8 symbols @ 13.24 kHz
#define _8SYMBOLS_MITI_01       (159)       //8 symbols @ 53 kHz
#define _8SYMBOLS_MITI_10       (80)        //8 symbols @ 106 kHz

#define _BGPKT_TICKS_00         (((_OVERHEAD_US_00+_PREDATA_US_00+(7*_8SYMBOLS_US_00))+976) / 977)
#define _BGPKT_TICKS_01         (((_OVERHEAD_US_01+_PREDATA_US_01+(7*_8SYMBOLS_US_01))+976) / 977)
#define _BGPKT_TICKS_10         (((_OVERHEAD_US_10+_PREDATA_US_10+(7*_8SYMBOLS_US_10))+976) / 977)
#define _BGPKT_TICKS_00F        (((_OVERHEAD_US_00+_PREDATA_US_00+(16*_8SYMBOLS_US_00))+976) / 977)
#define _BGPKT_TICKS_01F        (((_OVERHEAD_US_01+_PREDATA_US_01+(7*_8SYMBOLS_US_01))+976) / 977)
#define _BGPKT_TICKS_10F        (((_OVERHEAD_US_10+_PREDATA_US_10+(16*_8SYMBOLS_US_10))+976) / 977)

#define _TGD_00                 _BGPKT_TICKS_00
#define _TGD_01                 _BGPKT_TICKS_01
#define _TGD_10                 _BGPKT_TICKS_10
#define _TGD_00F                _BGPKT_TICKS_00F
#define _TGD_01F                _BGPKT_TICKS_01F
#define _TGD_10F                _BGPKT_TICKS_10F






/** Universal Data declaration
  * Described in radio.h of the OTlib.
  * This driver only supports M2_PARAM_MI_CHANNELS = 1.
  */
phymac_struct   phymac[M2_PARAM_MI_CHANNELS];
radio_struct    radio;





/** Timing Parameters   <BR>
  * ========================================================================<BR>
  * These are used to calculate timeouts based on Data-rate, DASH7 PHY specs,
  * transceiver preferences, and encoding.
  */

static const ot_u8 preheader_ti[4] = {
    ((_OVERHEAD_US_00+_PREDATA_US_00+976)/977),    // xx00
    ((_OVERHEAD_US_01+_PREDATA_US_01+976)/977),    // xx01
    ((_OVERHEAD_US_10+_PREDATA_US_10+976)/977),    // xx10
    0                                           // xx11 (RFU)
};

static const ot_u16 byte_miti[4] = {
    _8SYMBOLS_MITI_00,
    _8SYMBOLS_MITI_01,
    _8SYMBOLS_MITI_10,
    0
};

static const ot_u8 bgpkt_ti[16] = {
    _BGPKT_TICKS_00,  // 0000
    _BGPKT_TICKS_01,  // 0001
    _BGPKT_TICKS_10,  // 0010
    0,                  // 0011 (RFU)
    0,                  // 0100 (RFU)
    0,                  // 0101 (RFU)
    0,                  // 0110 (RFU)
    0,                  // 0111 (RFU)
    _BGPKT_TICKS_00F,   // 1000
    _BGPKT_TICKS_01F,   // 1001
    _BGPKT_TICKS_10F,   // 1010
    0,                  // 1011 (RFU)
    0,                  // 1100 (RFU)
    0,                  // 1101 (RFU)
    0,                  // 1110 (RFU)
    0                   // 1111 (RFU)
};

static const ot_u8 tgd_ti[16] = {
    _TGD_00,            // 0000
    _TGD_01,            // 0001
    _TGD_10,            // 0010
    0,                  // 0011 (RFU)
    0,                  // 0100 (RFU)
    0,                  // 0101 (RFU)
    0,                  // 0110 (RFU)
    0,                  // 0111 (RFU)
    _TGD_00F,           // 1000
    _TGD_01F,           // 1001
    _TGD_10F,           // 1010
    0,                  // 1011 (RFU)
    0,                  // 1100 (RFU)
    0,                  // 1101 (RFU)
    0,                  // 1110 (RFU)
    0                   // 1111 (RFU)
};





/** Radio-Agnostic Mode 2 Library Functions    <BR>
  * ========================================================================<BR>
  * Mode 2 specific virtual PHYMAC functionality, implemented in radio_task.c 
  */

#ifndef EXTF_rm2_init
OT_WEAK void rm2_init(void) {
    vlFILE* fp;
    
    /// Set universal Radio module initialization defaults
    radio.state     = RADIO_Idle;
    radio.flags     = RADIO_FLAG_REFRESH;
    radio.evtdone   = &otutils_sig2_null;

    /// These Radio Link features are available on the SPIRIT1
    ///@todo see if this and the "Set startup channel" part of the init can
    ///      get bundled into a common initialization function in radio_task.c
#   if (OT_FEATURE(RF_LINKINFO))
#       if (M2_FEATURE(RSCODE))
#           define _CORRECTIONS RADIO_LINK_CORRECTIONS
#       else
#           define _CORRECTIONS 0
#       endif
    radio.link.flags        = _CORRECTIONS | RADIO_LINK_PQI | RADIO_LINK_SQI \
                            | RADIO_LINK_LQI | RADIO_LINK_AGC;
#   endif
    radio.link.offset_thr   = 0;
    radio.link.raw_thr      = 0;
    
    /// Set startup channel to an always invalid channel ID (0xF0), and run 
    /// lookup on the default channel (0x18) to kick things off.  Since the 
    /// startup channel will always be different than a real channel, the 
    /// necessary settings and calibration will always occur. 
    phymac[0].channel   = 0xF0;
    phymac[0].tx_eirp   = 0x7F;
    fp                  = ISF_open_su( ISF_ID(channel_configuration) );
    rm2_channel_lookup(0x18, fp);
    vl_close(fp);
}
#endif





#ifndef EXTF_rm2_default_tgd
/*
OT_WEAK ot_uint rm2_default_tgd(ot_u8 chan_id) {
#if ((M2_FEATURE(FEC) == DISABLED) && (M2_FEATURE(TURBO) == DISABLED))
    return M2_TGD_55FULL;

#elif ((M2_FEATURE(FEC) == DISABLED) && (M2_FEATURE(TURBO) == ENABLED))
    return (chan_id & 0x20) ? M2_TGD_200FULL : M2_TGD_55FULL;

#elif ((M2_FEATURE(FEC) == ENABLED) && (M2_FEATURE(TURBO) == DISABLED))
    return (chan_id & 0x80) ? M2_TGD_55HALF : M2_TGD_55FULL;

#elif ((M2_FEATURE(FEC) == ENABLED) && (M2_FEATURE(TURBO) == ENABLED))
    // To understand this, you need to know the way the channel ID works.
    // The "0" elements are RFU.
    static const ot_u8 tgd[16] = {
        M2_TGD_55FULL, M2_TGD_55FULL, M2_TGD_200FULL, 0,
        0, 0, 0, 0,
        M2_TGD_55HALF, M2_TGD_55HALF, M2_TGD_200HALF, 0,
        0, 0, 0, 0
    };

    return (ot_uint)tgd[chan_id>>4];

#else
#   error "Missing definitions of M2_FEATURE(FEC) and/or M2_FEATURE(TURBO)"
    return 0;
#endif
}
*/
OT_WEAK ot_uint rm2_default_tgd(ot_u8 chan_id) {
    return (ot_int)tgd_ti[chan_id>>4];
}
#endif





#ifndef EXTF_rm2_rxtimeout_floor
/*
OT_WEAK ot_u16 rm2_rxtimeout_floor(ot_u8 chan_id) {
/// If the rx timeout should be a minimally small amount, which relates to
/// the rounded-up duration of an M2AdvP packet: 1, 2, 3, or 4 ti.
    static const ot_u8 min_ti_lut[8] = { 3, 1, 0, 0, 4, 2, 0, 0 };

    return (ot_u16)min_ti_lut[((phymac[0].channel & 0xA0) >> 5)];
}
*/
OT_WEAK ot_uint rm2_rxtimeout_floor(ot_u8 chan_id) {
    return (ot_u16)(bgpkt_ti[chan_id>>4] + 1);
}
#endif



#ifndef EXTF_rm2_pkt_duration
/*
OT_WEAK ot_uint rm2_pkt_duration(ot_queue* pkt_q) {
/// Wrapper function for rm2_scale_codec that adds some slop overhead
/// Slop = preamble bytes + sync bytes + ramp-up + ramp-down + padding
    ot_uint pkt_bytes;
    
    pkt_bytes   = q_length(pkt_q);
    pkt_bytes  += RF_PARAM_PKT_OVERHEAD;
    
    // If packet is using RS coding, adjust by the nominal rate (4/5).
    if (pkt_q->front[1] & 0x40) {
        pkt_bytes += (pkt_bytes+3)>>2;
    }
    
    return rm2_scale_codec(pkt_bytes);
}
*/
OT_WEAK ot_uint rm2_pkt_duration(ot_queue* pkt_q) {
    ot_uint pkt_bytes;
    ot_uint pkt_duration;

    pkt_bytes = q_length(pkt_q);
    if (pkt_q->front[1] & 0x40) {
        // If packet is using RS coding, adjust by the nominal rate (+25%).
        pkt_bytes += (pkt_bytes+3)>>2;
    }

    pkt_duration    = rm2_scale_codec(phymac[0].channel, pkt_bytes);
    pkt_duration   += preheader_ti[(phymac[0].channel>>4) & 3];
    return pkt_duration;
}
#endif


#ifndef EXTF_rm2_bgpkt_duration
/*
OT_WEAK ot_uint rm2_bgpkt_duration() {
/// Fast lookup for background packets.  If you care more about code size, 
/// you can change this implementation to: rm2_pkt_duration(7).  That said,
/// it doesn't save you much.
    static const ot_u8 bg_ti[4] = 
    {  ((RF_PARAM_PKT_OVERHEAD*8*19) + (56*19)) / 1024,
       ((RF_PARAM_PKT_OVERHEAD*8*19) + (64*38)) / 1024 ,
       1,
       ((RF_PARAM_PKT_OVERHEAD*8*6)  + (64*11)) / 1024   };
    
    return (ot_uint)bg_ti[ ((phymac[0].channel & 0x20)>>4) + (phymac[0].channel >> 7) ];
}
*/
OT_WEAK ot_uint rm2_bgpkt_duration() {
    return (ot_uint)bgpkt_ti[phymac[0].channel>>4];
}
#endif



#ifndef EXTF_rm2_scale_codec
/*
OT_WEAK ot_uint rm2_scale_codec(ot_uint buf_bytes) {
/// Turns a number of bytes (buf_bytes) into a number of ti units.
/// To refresh your memory: 
/// 1 ti    = ((1sec/32768) * 2^5) = 2^-10 sec = ~0.977 ms
/// 1 miti   = 1/1024 ti = 2^-20 sec = ~0.954us
///
/// Implementation follows derivation below:
/// pkt_duration(ti) = ( (buf_bytes*bit_us[X]/(1024/8) )
///
/// Supported Data rates below:
/// Low-Speed + Non-FEC = 18.88 miti/bit (55.55 kbps)
/// Low-Speed + FEC     = 37.75 miti/bit (27.77 kbps)
/// Hi-Speed + Non-FEC  = 5.24 miti/bit  (200 kbps)
/// Hi-Speed + FEC      = 10.49 miti/bit (100 kbps)
    
    static const ot_u8 bit_us[4] = { 19, 38, 6, 11 };   
    
    ot_u8 index = ((phymac[0].channel & 0x20)>>4) + (phymac[0].channel >> 7);
    buf_bytes  *= bit_us[index];
    //buf_bytes >>= 7;              ///@note revert this
    //buf_bytes >>= 2;
    buf_bytes >>= 4;

    return buf_bytes;
}
*/
OT_WEAK ot_uint rm2_scale_codec(ot_u8 channel_code, ot_uint buf_bytes) {
/// Turns a number of bytes (buf_bytes) into a number of ti units.
/// To refresh your memory:
/// 1 ti    = ((1sec/32768) * 2^5) = 2^-10 sec = ~0.977 ms
/// 1 miti   = 1/1024 ti = 2^-20 sec = ~0.954us
///
/// Implementation follows derivation below:
/// pkt_duration(ti) = ( (buf_bytes*bit_us[X]/(1024/8) )
///
/// Supported Data rates below:
/// Low-Speed + Non-FEC = 18.88 miti/bit (55.55 kbps)
/// Low-Speed + FEC     = 37.75 miti/bit (27.77 kbps)
/// Hi-Speed + Non-FEC  = 5.24 miti/bit  (200 kbps)
/// Hi-Speed + FEC      = 10.49 miti/bit (100 kbps)

    ot_ulong bytes_miti;
    ot_u8 encoding;

    encoding = (channel_code >> 4);

    // If channel is FEC'ed
    if (encoding & 0x08) {
        buf_bytes  &= ~1;   // make even
        buf_bytes  += 2;    // add trellis terminator with interleaving
        buf_bytes <<= 1;    // make half rate
    }

    // If channel is spread (presently unused)
    //if (encoding & 0x04) {
    //    buf_bytes *= 7;
    //}

    bytes_miti  = buf_bytes * byte_miti[encoding & 3];
    bytes_miti += 1023;

    return (ot_uint)(bytes_miti >> 10);
}
#endif




#ifndef EXTF_rm2_mac_filter
OT_WEAK ot_bool rm2_mac_filter() {
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


#ifndef EXTF_rm2_channel_refresh
OT_WEAK void rm2_channel_refresh(void) {
    radio.flags |= RADIO_FLAG_REFRESH;
}
#endif


#ifndef EXTF_rm2_test_channel
OT_WEAK ot_bool rm2_test_channel(ot_u8 channel) {
    ot_bool test;
    
    test = rm2_channel_fastcheck(channel);
    if (test == False) {
        vlFILE* fp;
        /// Open the Mode 2 FS Config register that contains the channel list
        /// for this host, and make sure the channel we want to use is available
        /// @todo assert fp
        fp      = ISF_open_su( ISF_ID(channel_configuration) );
        test    = rm2_channel_lookup(channel, fp);
        vl_close(fp);
    }

    return test;
}
#endif


#ifndef EXTF_rm2_test_chanlist
OT_WEAK ot_bool rm2_test_chanlist() {
    vlFILE* fp;
    ot_int  i;
    ot_bool test;
    ot_u8	next_channel;

    fp = ISF_open_su( ISF_ID(channel_configuration) );
    ///@todo assert fp

    /// Go through the list of tx channels
    /// <LI> Make sure the channel ID is valid. </LI>
    /// <LI> Make sure the transmission can fit within the contention period. </LI>
    /// <LI> Scan it, to make sure it can be used. </LI>
    for (i=0, test=False; i<dll.comm.tx_channels; i++) {
        next_channel = dll.comm.tx_chanlist[i];
        if (rm2_channel_fastcheck(next_channel)) {
        	test = True;
        	break;
        }
        if (rm2_channel_lookup(next_channel, fp)) {
        	test = True;
			break;
        }
    }
    vl_close(fp);

    return test;
}
#endif


#ifndef EXTF_rm2_channel_fastcheck
OT_WEAK ot_bool rm2_channel_fastcheck(ot_u8 chan_id) {
    
    // Check if there's a forced-refresh condition (always fail)
    if (radio.flags & RADIO_FLAG_REFRESH) {
        radio.flags ^= RADIO_FLAG_REFRESH;
        return False;
    }
    
    // Use Last Channel on chan_id == 0
    if (chan_id == 0) {
    	return True;
    }
    
    // If lower bits are the same, only change the encoding (radio settings remain the same)
    if ((chan_id & 0x7F) == (phymac[0].channel & 0x7F)) {
    	phymac[0].channel = chan_id;
    	return True;
    }

    return False;
}
#endif


#ifndef EXTF_rm2_channel_lookup
OT_WEAK ot_bool rm2_channel_lookup(ot_u8 chan_id, vlFILE* fp) {
/// Called during channel scans.
/// Duty: (a) See if the supplied channel is supported on this device & config.
///       If yes, return true.  (b) Determine if recalibration is required
///       before changing to the new channel, and recalibrate if so.
    ot_u8       spectrum_id;
    ot_int      i;
    volatile ot_uni16    scratch;

    // Strip the FEC & Spread bits
    spectrum_id = chan_id & 0x3F;

    /// Populate the phymac flags: these are not frequently used
    /// @todo I might want to do this in radio_init() instead
    scratch.ushort  = vl_read(fp, 2);
    phymac[0].flags = scratch.ubyte[0];

    /// Look through the channel list to find the one with matching spectrum id.
    /// The channel list is not necessarily sorted.
    /// @todo might do this with vl_get() in the future
    for (i=6; i<fp->length; i+=6) {
        scratch.ushort = vl_read(fp, i);

        if ((spectrum_id == scratch.ubyte[0]) \
        || ((spectrum_id & 0xF0) == scratch.ubyte[0])) {
            ot_u8 old_chan_id   = phymac[0].channel;
            ot_u8 old_tx_eirp   = (phymac[0].tx_eirp & 0x7f);

            phymac[0].tg        = rm2_default_tgd(chan_id);
            phymac[0].channel   = chan_id;
          //phymac[0].autoscale = scratch.ubyte[1];

            scratch.ushort      = vl_read(fp, i+2);
            phymac[0].tx_eirp   = scratch.ubyte[0] & 0x80;
            phymac[0].tx_eirp  |= rm2_clip_txeirp(scratch.ubyte[0]);
            phymac[0].link_qual = scratch.ubyte[1];

            ///@todo Try this: *(ot_u16*)&phymac[0].cs_thr = vl_read(fp, i+4);  
            ///it will need some rearrangement in phymac struct
            /// Convert thresholds from DASH7 numeric encoding to native encoding
            scratch.ushort      = vl_read(fp, i+4);
            //phymac[0].cs_thr    = __THR(scratch.ubyte[0]);
            //phymac[0].cca_thr   = scratch.ubyte[1];
            //phymac[0].cs_thr    = rm2_calc_rssithr(phymac[0].cs_thr);
            //phymac[0].cca_thr   = rm2_calc_rssithr(phymac[0].cca_thr);
            radio.link.raw_thr  = scratch.ubyte[0];
            phymac[0].cs_thr    = rm2_calc_rssithr( (ot_u8)(radio.link.raw_thr + radio.link.offset_thr) );
            phymac[0].cca_thr   = rm2_calc_rssithr( scratch.ubyte[1] );
            
            rm2_enter_channel(old_chan_id, old_tx_eirp);
            return True;
        }
    }
    return False;
}
#endif



#endif
