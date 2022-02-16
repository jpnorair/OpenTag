/*  Copyright 2008-2022, JP Norair
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted provided that the following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright 
  *    notice, this list of conditions and the following disclaimer in the 
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
  * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
  * POSSIBILITY OF SUCH DAMAGE.
  */
/**
  * @file       /io/ubxm8/ubxm8_task.c
  * @author     JP Norair
  * @version    R101
  * @date       14 Apr 2017
  * @brief      UBX Protocol manager for GNSS positioning
  * @ingroup    UBX_GNSS
  *
  *
  ******************************************************************************
  */


#include <otstd.h>
#include <board.h>

// Conditional Compliation
#if (OT_FEATURE(GNSS) == ENABLED) && (BOARD_FEATURE(UBX_GNSS) == ENABLED)

#include <otsys.h>

#include "ubx_internal.h"

#include <io/ubxm8/ubx_gnss.h>
#include <otlib.h>
#include <otsys.h>

///@todo HW needs to be better about blocking noise from radio that
/// affects GPS receiver.  Until it is, we block the DLL during GPS 
/// acquisition.
#include <m2/dll.h>

///@note set UBX_TRACKING_SETUP to NULL in order to build without tracking.
#define UBX_TRACKING_SETUP	UBX_CFG_PM2     // NULL

#define UBX_CONFIG_TIMEOUT	200		// ms, timeout for waiting ACK
#define UBX_PACKET_TIMEOUT	2		// ms, if now data during this delay assume that full update received
#define DISABLE_MSG_INTERVAL	1000000		// us, try to disable message with this interval

#define MIN(X,Y)	((X) < (Y) ? (X) : (Y))
#define SWAP16(X)	((((X) >>  8) & 0x00ff) | (((X) << 8) & 0xff00))

#define FNV1_32_INIT	((uint32_t)0x811c9dc5)	// init value for FNV1 hash algorithm
#define FNV1_32_PRIME	((uint32_t)0x01000193)	// magic prime for FNV1 hash algorithm


ot_bool         ubx_is_configured = False;
ubx_module_t    ubx;

static const uint32_t baudrate_table[] = {
        9600,       //UBX_9600bps   = 0,
        28800,    //UBX_28800bps,
        57600,    //UBX_57600bps,
        115200,     //UBX_115200bps,
        250000,     //UBX_250000bps,
        500000      //UBX_500000bps
};





/** External API functions
  * ========================================================================
  */
void ubx_config(ot_sig2 callback, ot_int onoff_interval) {
	ubx.onoff_interval	= onoff_interval;
	ubx.dataready		= (callback == NULL) ? &otutils_sig2_null : callback;
	ubx_is_configured	= True;
}

void ubx_connect(void) {
/// Do nothing if UBX module is already connected, or if callback is NULL
    if (ubx_is_configured && ((ubx.msg_stamp == 0xFFFF) || (ubx.msg_stamp == 0x4102))) {
        ubx.msg_stamp   = 0;
        ubx.task->event = (BOARD_FEATURE(GNSS_SDN)) ? UBX_TURNON : UBX_WAKEUP;
        sys_preempt(ubx.task, 0);
    }
}

void ubx_disconnect(void) {
    if (ubx_is_configured && ((ubx.msg_stamp == 0xFFFF) || (ubx.msg_stamp == 0x4102))) {
        ubx.msg_stamp   = 0xFFFF;
        ubx.task->event = UBX_SHUTDOWN;
        ubx.dataready   = &otutils_sig2_null;
        sys_preempt(ubx.task, 0);
    }
}

void ubx_agps_stamp(void) {
    ubx.ephemeris_stamp = time_uptime_secs();
}





ot_int sub_fixedpoint_sprint(ot_u8* dst, ot_s32 value, ot_s32 divider) {
/// Prints to utf-8 buffer using UBX lat/lon format (1e-7 degrees)
    ot_s32 numeral;
    ot_s32 decimal;
    ot_int written_bytes;
    
    // Get numeral part, then decimal part.
    numeral = value / divider; 
    decimal = value - (numeral * divider);
    
    if (decimal < 0) {
        decimal = -decimal;
    }
    
    written_bytes   = otutils_long2dec(dst, numeral);
    *dst++          = '.';
    written_bytes++;
    written_bytes  += otutils_long2dec(dst, decimal);
    
    return written_bytes;
}


ot_int ubx_latlon_sprint(ot_u8* dst, ot_s32 latlon) {
/// Prints to utf-8 buffer using UBX lat/lon format (1e-7 degrees)
    return sub_fixedpoint_sprint(dst, latlon, 10000000);
}


ot_int ubx_distance_sprint(ot_u8* dst, ot_s32 distance) {
/// All distances are in mm.  We make them meters.
    return sub_fixedpoint_sprint(dst, distance, 1000);
}





/** Internal Routines: called within module only.
  * ========================================================================
  */
ot_u32 sub_convert_baudrate(ubx_speed_t ibaud);
ot_u16 sub_send_cfgmsg(ot_u16 msg_stamp, ot_u8 rate);
void ubx_taskinit(ot_task task);
ot_bool ubx_retry_check(ot_u8 limit);
ot_int ubx_send_gonext(ot_u16 msg_stamp, ot_uint length);
ot_int ubx_send(ot_u16 msg_stamp, ot_uint length);
inline ot_u16 ubx_waitfor_ack(uint16_t msg_stamp, uint16_t timeout);
ot_u16 ubx_ackcheck(void);



ot_u32 sub_convert_baudrate(ubx_speed_t ibaud) {
    return baudrate_table[ibaud];
    //return 9600;
}

ot_u16 sub_send_cfgmsg(ot_u16 msg_stamp, ot_u8 rate) {
    ubx.buf.payload.tx_cfg_msg.msg  = msg_stamp;
    ubx.buf.payload.tx_cfg_msg.rate = rate;
    ubx_send_gonext(UBX_MSG_CFG_MSG, sizeof(ubx_payload_tx_cfg_msg_t));
    return ubx_waitfor_ack(UBX_MSG_CFG_MSG, UBX_CONFIG_TIMEOUT);
}


void ubx_taskinit(ot_task task) {
    /// Basic task stuff to do at startup or task kill, no matter what
    ubx.task        = task;
    task->event     = 0;
    task->cursor    = 0;
    task->reserve   = 1;
    task->latency   = 255;
    
    if (ubx_is_configured) {
        /// UBX protocol is half duplex we use the same buffer for RX/TX.
        ubxdrv_kill();
        //BOARD_led3_on();
        
        task->latency = ubxdrv_init(NULL);
        
        // Default time-since-ephemeris-update is two days before 0 time.
        // This is enough to force a cold-start.
        ///@todo method to seed AssistNow data into the setup.
        ubx.ephemeris_stamp = (ot_u32)(-2*24*60*60);

        // This is used to show that an app is not connected.
        // msg_stamp set to an invalid value.
        ubx.msg_stamp   = 0xFFFF;
        
        // This is the state on boot
        // UBX_CFG_UART verifies the UART baud.  Ublox struggles mightily to be reliable at this.
        //ubx.start_state = UBX_CFG_UART;
        ubx.start_state = UBX_CFG_UART2;
        
        /// Automatic Startup on Task initialization.  It applies a 50 tick
        /// delay in order to wait for startup, but probably not needed.
        /// @todo Not sure if I want this or not.
        ubx.msg_stamp    = 0;
        ubx.task->event = UBX_TURNON;
        sys_preempt(ubx.task, 50);
    }
}


ot_bool ubx_retry_check(ot_u8 limit) {

    // If retries fail past the limit, assume Hardware is messed up.
    // Do a reset and log it.
    if (++ubx.task->cursor > limit) {
    
        // Send error callback to application, and force disconnect.
        ubx.dataready(0, 0);
        ubx.msg_stamp	= 0xFFFF;
        ubx.dataready   = &otutils_sig2_null;
    
        ///@todo log fault with fault manager
        ubxdrv_turnoff();
        ubx.task->cursor    = 0;
        ubx.task->event     = 0;
        return True;
    }
    return False;
}


ot_int ubx_send(ot_u16 msg_stamp, ot_uint length) {
	ubx.msg_stamp           = msg_stamp;
	ubx.buf.header.length   = length;
	ubx.buf.header.msg      = msg_stamp;
	/// Returns number of ticks that transmission will require on the wire
	return ubxdrv_tx();
}


ot_int ubx_send_gonext(ot_u16 msg_stamp, ot_uint length) {
	/// Advance state counter
    ubx.task->event++;
    return ubx_send(msg_stamp, length);
}


inline ot_u16 ubx_waitfor_ack(uint16_t msg_stamp, uint16_t timeout) {
    ubx.msg_stamp = msg_stamp;
    return timeout;
}


ot_u16 ubx_ackcheck(void) {
    if ((ubx.buf.header.msg == UBX_MSG_ACK_ACK) && (ubx.buf.payload.rx_ack_ack.msg == ubx.msg_stamp)) {
        ubx.task->cursor = 0;
        ubx.task->event++;
        return 0;
    }
    ubx.task->event--;
    return 50;          ///wait period before retry (ticks)
}


ot_int sub_load_assistnow(void) {
///@todo redo this function in order to work with assisted data model, 
///      which is not yet implemented.
    
    vlFILE* fp = ISF_open_su(ISF_ID_agps_input);
    if (fp != NULL) {
        if (fp->length != 0) {
            vl_load(fp, sizeof(ubx_payload_tx_mga_ano_t), (uint8_t*)&ubx.buf.payload);
            vl_close(fp);
            
            return 0;
        }
    }
    vl_close(fp);
    
    return -1;
}







/** Callbacks from driver
  * ========================================================================
  */
ot_bool sub_checkmsg_for_file(ot_u16 msg);



ot_bool sub_checkmsg_for_file(ot_u16 msg) {
    return (   (msg == UBX_MSG_NAV_PVT) \
            || (msg == UBX_MSG_NAV_AOPSTATUS) \
            || (0) \
           );
}



void ubxevt_rxdone(ot_int code) {
    /// Handle fletcher errors as non acknowledged packets by wiping the msg id
    if (code != 0) {
        ubx.buf.header.msg = 0;
    }
    
    /// 1. Handle inbound packets received during an active state.
    ///    We care (right now) about the following packets:
    ///    -- NAV-PVT: data received.
    ///    -- NAV-AOPSTATUS: AssistNow Autonomous status.
    if (ubx.task->event == UBX_TIMEOUT) {
        switch (ubx.buf.header.msg) {
        
        // NAV-PVT: GNSS navigation data.  
        // - Ignore lousy fixes.
        // - Don't pre-empt until hot-start is possible
        case UBX_MSG_NAV_PVT: {
            vlFILE* fp;
            ot_int fix = ubx.buf.payload.rx_nav_pvt.fixType;
            if ((fix < 2) || (fix > 4)) {
                goto ubxevt_rxdone_IGNORE;
            }
            
            fp = ISF_open_su(ISF_ID_gnss_output);
            if (fp != NULL) {
                vl_store(fp, sizeof(ubx_payload_rx_nav_pvt_t), (uint8_t*)&ubx.buf.payload);
                vl_close(fp);
                ubx.dataready(1, 0x0107);
            }
            
        } break;
        
        // NAV-AOPSTATUS: Tells us if Ephemeris is loaded.
        // We use this information to set a state variable, but it doesn't pre-empt.
        case UBX_MSG_NAV_AOPSTATUS:
            if ((ubx.buf.payload.rx_nav_aopstatus.aopCfg != 0)
            &&  (ubx.buf.payload.rx_nav_aopstatus.status == 0) 
            /*&&  (ubx.start_mode < UBX_hot_start)       */         ) {
                ubx.start_mode      = UBX_hot_start;
                ubx.ephemeris_stamp = time_uptime_secs();
            }
            goto ubxevt_rxdone_IGNORE;
        
        // Some other packet.  Ignore.
        default:
            goto ubxevt_rxdone_IGNORE;
        }
    }
    
    /// 1. Handle inbound packets that are received during a non-active state.
    ///    UBX_TIMEOUT is the state that indicates we are actively waiting for
    ///    Navigation results from the receiver.
    ///    CURRENTLY NOTHING TO HANDLE HERE.
    
    /// Fall through behavior is to pre-empt the UBX TASK, which is probably
    /// waiting on a message.
    ubxevt_rxdone_PREEMPT:
    sys_preempt(ubx.task, 0);
    
    ubxevt_rxdone_IGNORE:
    return;
}






/** Main Task Function
  * ========================================================================
  */

void ubx_systask(ot_task task) {
/// It is optional to use a loop in a task function, but for this task it helps
/// reduce some load on the scheduler.
    ot_u32  nextevent = 0;

    do {
        switch (task->event) {
        // Task destructor: close iap & iapdrv
        ///@todo update this for modern form with rebooter
        case 0: sys_taskinit_macro(task, ubx_taskinit(task), otutils_null()); 
                return;

#   if (0) // If TEST          
        case 255: task->cursor = ubxdrv_getpwrcode();
                  return;
                  
        default: {
            static const ot_u8 mock_data[] = {
                0x6F, 0x24, 0x85, 0x16, 0x27, 0xA8, 0xFF, 0xFF, 0x37, 0x1C, 0x00, 0x00, 0x29, 0x2C, 0x09, 0x00, 0x1E, 0xEA, 0x00, 0x00,
                0xA5, 0xFB, 0xFF, 0xFF, 0xFA, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x06, 0x27, 0x00, 0x00, 0xC6, 0x90, 0x62, 0x00, 0x18, 0x04, 0x00, 0x00, 0xE0, 0x4A, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            };
            
            memcpy( (ot_u8*)&ubx.buf.payload, mock_data, sizeof(ubx_payload_rx_nav_pvt_t) );
            
            ubx.task->event = UBX_TIMEOUT;
            ubx.buf.header.msg = UBX_MSG_NAV_PVT;
            ubx.buf.payload.rx_nav_pvt.fixType = 3;  
            
            ubxevt_rxdone(0);
            
            task->event = 0;
            nextevent   = 0;
            ubx.msg_stamp = 0x4102;
            
            } break;
        }
               
#   else     
        /// Device configuration States
        // ------------------------------------------------------------------
        case UBX_TURNON: 
            nextevent   = ubxdrv_turnon();
            task->event = UBX_WAKEUP;
            task->cursor= 0;
            break;
            
        case UBX_WAKEUP: 
        	ubxdrv_extwakeup();
            nextevent   = (ot_u32)ubxdrv_txsync();
            task->event = ubx.start_state;
            task->cursor= 0;
            break;
            
        /// Step 1: is to set-up the UART.
        /// There are several stages to this.

        /// Step 1a: Loop through different baudrates to find the right one.
        /// Testing shows it defaults to 9600.  We want 115200.
        /// This code might be best to remove in final versions.
        case UBX_CFG_UART: {
            uint8_t i = task->cursor;
            if (ubx_retry_check(UBX_BAUDRATE_MAX)) return;
            
            ubx.baudrate = sub_convert_baudrate(i);
            ubxdrv_setbaudrate(ubx.baudrate);
            ubx.buf.payload.tx_cfg_prt.portID       = UBX_TX_CFG_PRT_PORTID;
            ubx.buf.payload.tx_cfg_prt.mode         = UBX_TX_CFG_PRT_MODE;
            ubx.buf.payload.tx_cfg_prt.baudRate     = ubx.baudrate;
            ubx.buf.payload.tx_cfg_prt.inProtoMask  = UBX_TX_CFG_PRT_INPROTOMASK_GPS;
            ubx.buf.payload.tx_cfg_prt.outProtoMask = UBX_TX_CFG_PRT_OUTPROTOMASK_GPS;
            
            ubx_send_gonext(UBX_MSG_CFG_PRT, sizeof(ubx_payload_tx_cfg_prt_t));
            nextevent = (ot_u32)ubx_waitfor_ack(UBX_MSG_CFG_PRT, UBX_CONFIG_TIMEOUT);
        } break;
        case UBX_CFG_UART_ACK: 
            nextevent = (ot_u32)ubx_ackcheck();
            break;

        /// Step 1b: Set-up Port with default settings (including data rate)
        /// Use the last baudrate from previous states, which worked.
        /// Wait for an ACK -- none is expected (wrong baudrate), but wait
        /// anyway.  Then set local UART to this baudrate.
        ///@note through testing, it has been found that a delay slot after
        ///      setting the port is necessary.
        case UBX_CFG_UART2: {
            ubx.buf.payload.tx_cfg_prt.portID       = UBX_TX_CFG_PRT_PORTID;
            ubx.buf.payload.tx_cfg_prt.mode         = UBX_TX_CFG_PRT_MODE;
            ubx.buf.payload.tx_cfg_prt.baudRate     = UBX_TX_CFG_PRT_BAUDRATE;
            ubx.buf.payload.tx_cfg_prt.inProtoMask  = UBX_TX_CFG_PRT_INPROTOMASK_GPS;
            ubx.buf.payload.tx_cfg_prt.outProtoMask = UBX_TX_CFG_PRT_OUTPROTOMASK_GPS;
            
            ubx_send_gonext(UBX_MSG_CFG_PRT, sizeof(ubx_payload_tx_cfg_prt_t));
            nextevent = (ot_u32)ubx_waitfor_ack(UBX_MSG_CFG_PRT, UBX_CONFIG_TIMEOUT);
        } break;
        case UBX_CFG_UART_ACK2: 
            ubx.baudrate = UBX_TX_CFG_PRT_BAUDRATE;
            ubxdrv_setbaudrate(ubx.baudrate);
            task->event++;
            task->cursor = 0;
            nextevent = 500;
            break;

        /// Step 2: Configure the update rate of the Receiver
        case UBX_CFG_RATE: {
        	ot_u16 measRate_val;

            if (ubx_retry_check(UBX_RETRY_MAX)) return;
            memset(&ubx.buf.payload.tx_cfg_rate, 0, sizeof(ubx.buf.payload.tx_cfg_rate));

            if (ubx.onoff_interval < 0) {
            	measRate_val = ((-ubx.onoff_interval) * 1000) / 3;  //3: UBX_TX_CFG_RATE_NAVRATE
            }
            else {
                measRate_val = 1000;//UBX_TX_CFG_RATE_MEASINTERVAL
            }
            ubx.buf.payload.tx_cfg_rate.measRate   = measRate_val;
            ubx.buf.payload.tx_cfg_rate.navRate    = 3; //UBX_TX_CFG_RATE_NAVRATE;
            ubx.buf.payload.tx_cfg_rate.timeRef    = UBX_TX_CFG_RATE_TIMEREF;
            
            ubx_send_gonext(UBX_MSG_CFG_RATE, sizeof(ubx_payload_tx_cfg_rate_t));
            nextevent = (ot_u32)ubx_waitfor_ack(UBX_MSG_CFG_RATE, UBX_CONFIG_TIMEOUT);
        } break;
        case UBX_CFG_RATE_ACK:
            nextevent = (ot_u32)ubx_ackcheck();
            break;

        // Step 4: SBAS -- may not be used
#       ifdef UBX_CONFIGURE_SBAS
        case UBX_CFG_SBAS:
            memset(&ubx.buf.payload.tx_cfg_sbas, 0, sizeof(ubx.buf.payload.tx_cfg_sbas));
	        ubx.buf.payload.tx_cfg_sbas.mode = UBX_TX_CFG_SBAS_MODE;
	        ubx_send_gonext(UBX_MSG_CFG_SBAS, sizeof(ubx_payload_tx_cfg_sbas_t));
            nextevent = (ot_u32)ubx_waitfor_ack(UBX_MSG_CFG_SBAS, UBX_CONFIG_TIMEOUT);
            break;

        case UBX_CFG_SBAS_ACK:
            nextevent = (ot_u32)ubx_ackcheck();
            break;
#       endif

		// Step 5: Set rate for NAV-PVT
#		ifdef UBX_CONFIGURE_NAVX5
		case UBX_CFG_NAVX5:
			if (ubx_retry_check(UBX_RETRY_MAX)) return;
			memset(&ubx.buf.payload.tx_cfg_navx5, 0, sizeof(ubx_payload_tx_cfg_navx5_t));
			ubx.buf.payload.tx_cfg_navx5.version			= 2;
			//ubx.buf.payload.tx_cfg_navx5.mask1				= (1<<2)|(1<<3)|(1<<6)|(1<<9)|(1<<10)|(1<<13)|(1<<14);
			ubx.buf.payload.tx_cfg_navx5.mask1				= (1<<14);
			ubx.buf.payload.tx_cfg_navx5.mask2				= 0;
			ubx.buf.payload.tx_cfg_navx5.minSVs				= 4;
			ubx.buf.payload.tx_cfg_navx5.maxSVs				= 32;
			ubx.buf.payload.tx_cfg_navx5.minCNO				= 6;
			ubx.buf.payload.tx_cfg_navx5.iniFix3D			= 0;
			ubx.buf.payload.tx_cfg_navx5.ackAiding			= 0;
			ubx.buf.payload.tx_cfg_navx5.wknrollover		= 1867;
			ubx.buf.payload.tx_cfg_navx5.sigAttnCompMode	= 0;
			ubx.buf.payload.tx_cfg_navx5.usePPP				= 1;
			ubx.buf.payload.tx_cfg_navx5.aopCfg				= 1;		//AssistNow Autonomous feature.
			ubx.buf.payload.tx_cfg_navx5.aopOrbMaxErr		= 100;
			ubx.buf.payload.tx_cfg_navx5.useAdr				= 0;
			ubx_send_gonext(UBX_MSG_CFG_NAVX5, sizeof(ubx_payload_tx_cfg_navx5_t));
			nextevent = ubx_waitfor_ack(UBX_MSG_CFG_NAVX5, UBX_CONFIG_TIMEOUT);
			break;

		case UBX_CFG_NAVX5_ACK:
			nextevent = (ot_u32)ubx_ackcheck();
			break;


#		endif

        // Step 5: Set rate for NAV-PVT
        case UBX_CFG_NAV_PVT:
            if (ubx_retry_check(UBX_RETRY_MAX))
                return;

            nextevent = (ot_u32)sub_send_cfgmsg(UBX_MSG_NAV_PVT, 1);
            break;

        case UBX_CFG_NAV_PVT_ACK:
            nextevent = (ot_u32)ubx_ackcheck();
            break;
        
#       ifdef UBX_CONFIGURE_NAV_DOP
        // Step 6: Set rate for NAV-DOP
        case UBX_CFG_NAV_DOP:
            if (ubx_retry_check(UBX_RETRY_MAX)) return;
            nextevent = (ot_u32)sub_send_cfgmsg(UBX_MSG_NAV_DOP, 1);
            break;

        case UBX_CFG_NAV_DOP_ACK:
            nextevent = (ot_u32)ubx_ackcheck();
            break;
#		endif

#       ifdef UBX_CONFIGURE_NAV_SVINFO
        // Step 7: Set rate for NAV-SVINFO
        case UBX_CFG_NAV_SVINFO:
            if (ubx_retry_check(UBX_RETRY_MAX)) return;
            nextevent = (ot_u32)sub_send_cfgmsg(UBX_MSG_NAV_SVINFO, 1);
            break;

        case UBX_CFG_NAV_SVINFO_ACK:
            nextevent = (ot_u32)ubx_ackcheck();
            break;
#		endif

#       if UBX_CONFIGURE_MON
        // Step 8: Set rate for MON-HW
        case UBX_CFG_MON_HW:
            if (ubx_retry_check(UBX_RETRY_MAX)) return;
            nextevent = (ot_u32)sub_send_cfgmsg(UBX_MSG_MON_HW, 1);
            break;

        case UBX_CFG_MON_HW_ACK:
            nextevent = (ot_u32)ubx_ackcheck();
            break;
        
        // Step 9: Get MON-VER information.
        //         Just send empty MON-VER message, ubx device will send a response.
        case UBX_CFG_MON_VER:
            ubx_send_gonext(UBX_MSG_MON_VER, 0);
            break;

        case UBX_CFG_MON_VER_ACK:
            ubx_respcheck(UBX_MSG_MON_VER);
            break;
#       endif

        // Step 10: Write configuration into non volatile memory
        // Presently, this is not enabled because it's not clear which uBlox
        // devices have flashable non-volatile memory (if any)
        //case UBX_CFG_CFG:
        //case UBX_CFG_CFG_ACK:
        
        // ------------------------------------------------------------------
        // This is the end of configuration section.
        // Below lies the normal operational runtime.
        // ------------------------------------------------------------------
        
        // Some logic that runs on start of main process.
        // If the last ephemeris download was more than two hours ago, we do
        // a warm start.  If there is no ephemeris, indicated by more than 
        // 14 days inactivity (or stamp = 0), we do a cold start.
        case UBX_MAIN_PROC: {
            // start_mode = 0: cold start
            // start_mode = 1: warm start, threshold is 1 day
            // start_mode = 2: hot start, threshold is 2 hours
            // hot/assisted start threshold: 2 hours
            // warm start threshold: 1 day
            // else, cold start
            {   ot_u32  since_ephemeris;
                since_ephemeris = time_uptime_secs() - ubx.ephemeris_stamp;
                ubx.start_mode  = (since_ephemeris < (3600 * 2)); 		//hot start + 1
                ubx.start_mode += (since_ephemeris < (3600 * 24));	    //warm start + 1
            }
            
            // If there is assist-now data, load it in.
            if (sub_load_assistnow() == 0) {
                task->event = UBX_MGA_ANO;
                goto UBX_MGA_ANO_ROUTINE;
            }
            
            // There's no assist now data, so just go to start.
            task->event = UBX_MODE_ENTRY;
            nextevent   = 0;
        } break;
            
        // Send AssistNow Data: this data comes from the Haystack network, and
        // it is saved in a DASH7 file.  It is stored to buffer above.
        case UBX_MGA_ANO: 
        UBX_MGA_ANO_ROUTINE: 
            if (ubx_retry_check(UBX_RETRY_MAX))
            	return;
            ubx_send_gonext(UBX_MSG_MGA_ANO, sizeof(ubx_payload_tx_mga_ano_t));
            nextevent = (ot_u32)ubx_waitfor_ack(UBX_MSG_MGA_ANO, UBX_CONFIG_TIMEOUT);
            break;

        case UBX_MGA_ANO_ACK:
            nextevent = (ot_u32)ubx_ackcheck();
            break;
        
        // Entry of GNSS per operating Mode
        // - Normal:        Use RST
        // - Persistent:    Use PM2 (On/Off Powersave mode)
        case UBX_MODE_ENTRY: {
            static const ot_u16 start_code[]        = { 0xFFFF, 0x0100, 0x0000 };
            ubx.dataready(0, 2);	// Startup message
            ubx.buf.payload.tx_cfg_rst.navBbrMask   = start_code[ubx.start_mode];
            ubx.buf.payload.tx_cfg_rst.resetMode    = 0x09;
            ubx.buf.payload.tx_cfg_rst.reserved1	= 0;
            nextevent   = (ot_u32)ubx_send_gonext(UBX_MSG_CFG_RST, sizeof(ubx_payload_tx_cfg_rst_t));
            nextevent  += UBX_CONFIG_TIMEOUT;
            task->event = UBX_RXDATA;
        } break;
        case UBX_MODE_ENTRY_ACK:
            nextevent = (ot_u32)ubx_ackcheck();
            break;
            
        /// Receive some NAV PTR's.
        /// Set timeout seconds depending on start mode (cold, warm, hot).
        /// The reception routine will get stopped after the first NAV-PTR is
        /// received, or if timeout occurs.
        case UBX_RXDATA: {
            static const ot_u16 timeout_secs[] = {
                    600,    // cold 
                    50,     // warm
                    50      // hot
            }; 
            nextevent   	= (ot_u32)timeout_secs[ubx.start_mode] * 1024;
            task->event 	= UBX_TIMEOUT;
            task->cursor	= 0;
        } break;
        
        /// UBX device will run for a set period of time collecting NAV PTR's
        /// It will stop, and then shutdown.
        /// - Timeout will delay going to shutdown until ephemeris is hot.
        /// - Hot ephemeris will be downloaded either by AGPS or directly from
        ///   satellites via AOPSTATUS.
        /// - When Timeout is successful, GPS Receiver is turned off.
        case UBX_TIMEOUT:
        	task->cursor++;

        	if (ubx.onoff_interval < 0) {
                // This is continuous polling mode.
                ///@todo Inside here we could do some logging.
                nextevent = 30 * 1024;
                ubx_send(UBX_MSG_NAV_AOPSTATUS, 0);

            }
            else if ((task->cursor < 10) && (ubx.start_mode < UBX_hot_start)) {
            	// Wait for AOPSTATUS.  This needs a lot of testing.
            	nextevent = 1024;
            }
            else {
            	ubx.start_mode      = UBX_warm_start;
            	//ubx.ephemeris_stamp = time_uptime_secs();
            	task->cursor        = 0;
            	ubx.dataready(0, 1);    // Receiver-off message to client

            	// Yank power, OR do controlled shutdown.
            	if ((BOARD_FEATURE(GNSS_SDN) == ENABLED) && (ubx.onoff_interval == 0)) {
            		goto ubx_systask_POWEROFF;
            	}

//            	ubx.buf.payload.tx_cfg_rst.navBbrMask   = 0;
//				ubx.buf.payload.tx_cfg_rst.resetMode    = 0x08;
//				ubx.buf.payload.tx_cfg_rst.reserved1    = 0;
//				nextevent   = (ot_u32)ubx_send(UBX_MSG_CFG_RST, sizeof(ubx_payload_tx_cfg_rst_t));
//				nextevent  += UBX_CONFIG_TIMEOUT;
				task->event = UBX_SHUTDOWN;
				nextevent = 0;
            }
            break;

        case UBX_SHUTDOWN:
            // Use PMREQ to engage Backup Mode.
            // This message is not acked.  Wakeup-Pulse needed to resume communication (UBX_WAKEUP)
#           if 0
            memset(&ubx.buf.payload.tx_rxm_pmreq, 0, sizeof(ubx_payload_tx_rxm_pmreq_t));
            ubx.buf.payload.tx_rxm_pmreq.duration   = 0;		// ms to go down.  0 is indefinite
            ubx.buf.payload.tx_rxm_pmreq.flags      = (1<<1);	// 0: Don't do Backup, 1: Do Backup
            nextevent 		= ubx_send(UBX_MSG_RXM_PMREQ, sizeof(ubx_payload_tx_rxm_pmreq_t));
            task->event  	= UBX_SHUTDOWN2;
            break;
#           endif

        case UBX_SHUTDOWN2:
            // Kill driver, and come back up in (ubx.onoff_interval) seconds.
            ubxdrv_kill();
            ubx.start_state = UBX_MAIN_PROC;
            task->event     = UBX_WAKEUP;
            nextevent       = ubx.onoff_interval * 1024;
        	break;
			
        // Return the power-code state of the driver, 0-3, without changing task states
        case 255: task->cursor = ubxdrv_getpwrcode();
                  return;
                  
        default: 
        ubx_systask_POWEROFF:
            ubxdrv_turnoff();
            task->event    = 0;
            task->cursor   = 0;
            break;
        }
#   endif // From #if (TEST)
        
    } while ((nextevent <= TI2CLK(0)) && (task->event != 0));


    /// The task is done for now.  Instruct the Scheduler when to reinvoke this task.
    sys_task_setnext(task, nextevent);
}






//#endif

#endif
