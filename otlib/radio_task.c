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
  * @file       /otlib/radio_task.c
  * @author     JP Norair
  * @version    R100
  * @date       30 Oct 2012
  * @brief      Generic Radio (RF transceiver) task
  * @defgroup   Radio (Radio Module)
  * @ingroup    Radio
  *
  * To be extracted from current DLL task
  *
  ******************************************************************************
  */

#include "OT_config.h"

#if OT_FEATURE(M2)
#include "radio.h"
#include "m2_dll.h"
#include "OT_platform.h"


/** Universal Data declaration
  * Described in radio.h of the OTlib.
  * This driver only supports M2_PARAM_MI_CHANNELS = 1.
  */
phymac_struct   phymac[M2_PARAM_MI_CHANNELS];
radio_struct    radio;




#ifndef EXTF_rm2_default_tgd
ot_int rm2_default_tgd(ot_u8 chan_id) {
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

    return (ot_int)tgd[chan_id>>4];

#else
#   error "Missing definitions of M2_FEATURE(FEC) and/or M2_FEATURE(TURBO)"
    return 0;
#endif
}
#endif





#ifndef EXTF_rm2_rxtimeout_floor
ot_u16 rm2_rxtimeout_floor(ot_u8 chan_id) {
/// If the rx timeout should be a minimally small amount, which relates to
/// the rounded-up duration of an M2AdvP packet: 1, 2, 3, or 4 ti.
    static const ot_u8 min_ti_lut[8] = { 3, 1, 0, 0, 4, 2, 0, 0 };

    return (ot_u16)min_ti_lut[((phymac[0].channel & 0xA0) >> 5)];
}
#endif



#ifndef EXTF_rm2_pkt_duration
ot_int rm2_pkt_duration(ot_queue* pkt_q) {
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
#endif


#ifndef EXTF_rm2_bgpkt_duration
ot_int rm2_bgpkt_duration() {
/// Fast lookup for background packets.  If you care more about code size, 
/// you can change this implementation to: rm2_pkt_duration(7).  That said,
/// it doesn't save you much.
    static const ot_u8 bg_ti[4] = 
    {  ((RF_PARAM_PKT_OVERHEAD*8*19) + (56*19)) / 1024,
       ((RF_PARAM_PKT_OVERHEAD*8*19) + (64*38)) / 1024,
       1,
       ((RF_PARAM_PKT_OVERHEAD*8*6)  + (64*11)) / 1024   };
    
    return (ot_int)bg_ti[ ((phymac[0].channel & 0x20)>>4) + (phymac[0].channel >> 7) ];
}
#endif



#ifndef EXTF_rm2_scale_codec
ot_int rm2_scale_codec(ot_int buf_bytes) {
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
#endif








#endif
