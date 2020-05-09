/* Copyright 2017 JP Norair
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
  * @file       /apps/ht_gnssdemo/app/main.c
  * @author     JP Norair
  * @version    R100
  * @date       27 Apr 2017
  * @brief      HayTag Ping Pong with GNSS
  *
  * This is an outgrowth of PongLT, but it includes GNSS features.  To use 
  * these, of course, you need a GNSS receiver.  HayTag uses a uBlox M8.
  *
  ******************************************************************************
  */

#include <board.h>
#include <otplatform.h>
#include <otstd.h>
#include <otsys.h>

#include <otlib/delay.h>
#include <otlib/rand.h>
#include <otlib/logger.h>

#include <m2api.h>
#include <alp/asapi.h>


#if defined(__UBX_GNSS__)
#	include <io/ubx_gnss.h>
#	define gnss_connect()  	ubx_connect()
#   define gnss_config(A,B)	ubx_config(A, B)
#elif defined(__NULL_GNSS__)
#	include <io/gnss_null/interface.h>
#	define gnss_connect()  	gnssnull_connect()
#   define gnss_config(A,B)	gnssnull_config(A, B)
#endif

#include <stdio.h>
#include <math.h>

/** Data Mapping <BR>
  * ===========================================================================
  * The Opmode Demo needs a particular data mapping.  It is not unusual, but
  * the demo may not work if the data is not set correctly.  This define below
  * uses the default data mapping (/apps/demo_opmode/code/data_default.c)
  */
#define __DATA_DEFAULT

#if defined(__DATA_DEFAULT)
#   include <../_common/fs_default_startup.c>
#else
#   error "There is no data mapping specified.  Put one here."
#endif

//#define __FORCE_GATEWAY

// You can set these parameters:
// __DBM_DEFAULT:   the starting power (dBm)
// __GNSS_PERIOD_S:	period in seconds for GNSS location sampling
///@note __GNSS_PERIOD_S controls the onoff cycling period of the receiver.
///      It will yield location information slightly slower than this period,
///      due to time-to-fix.  Supplying 0 seconds will make it a one-shot.
#define __DBM_DEFAULT       20
#define __GNSS_PERIOD_S		-3

#define PING_CHANNEL        (0x28)
#define TG_CHANNEL          (0x81)


/** Main Static Variables <BR>
  * ========================================================================<BR>
  */
typedef struct {
    ot_bool is_gateway;
} setup_struct;

static setup_struct setup;




/** Ping Static Variables <BR>
  * ========================================================================<BR>
  */
typedef struct {
    ot_u16  val;
} ping_struct;

static ping_struct ping;





/** Local Functions & definitions
  * ========================================================================
  */
#define PING_TASK    (&sys.task[TASK_ping])
#define NAV_TASK     (&sys.task[TASK_gnssreporter])


// Main Application Functions
void setup_button_init();                /// board/platform dependent
void setup_init();

// Ping Functions
void ping_invoke(ot_u8 channel);
void ping_query_applet(m2session* active);

/// GNSS Functions
void gnss_callback(ot_int rx_type, ot_int msg_type);
void gnss_storedata(ot_s32* lat, ot_s32* lon, ot_u32* acc);
void gnss_broadcast_applet(m2session* active);
void gnss_telegram_applet(m2session* active);


/// Direct Control Applets
//void opmode_goto_gateway();
//void opmode_goto_endpoint();

// Applets that run due to OpenTag callbacks
//void    app_packet_routing(ot_int code, ot_int type);



/** Function for modulating power <BR>
  * ==================================================================<BR>
  */
void app_set_txpwr(ot_int dBm) {
    vlFILE* fp;
    ot_uni16 pwr_flt;

    fp = ISF_open_su(0x02);
    if (fp != NULL) {
        ot_uint offset;

        // Replace existing power levels for each of three channel groups
        // with new power spec.
        offset              = 6+2;
        pwr_flt.ushort      = vl_read(fp, offset);
        pwr_flt.ubyte[0]    = (ot_u8)( (dBm + 40) * 2 );
        vl_write(fp, offset, pwr_flt.ushort);

        offset              = 6+6+2;
        pwr_flt.ushort      = vl_read(fp, offset);
        pwr_flt.ubyte[0]    = (ot_u8)( (dBm + 40) * 2 );
        vl_write(fp, offset, pwr_flt.ushort);

        offset              = 6+6+6+2;
        pwr_flt.ushort      = vl_read(fp, offset);
        pwr_flt.ubyte[0]    = (ot_u8)( (dBm + 40) * 2 );
        vl_write(fp, offset, pwr_flt.ushort);

        vl_close(fp);

        // This block of code resets the channel status, forcing the
        // radio module to update the channel information.
        // It's a hack, but this whole app is a hack.
        //phymac[0].channel   = 0xF0;
        //phymac[0].tx_eirp   = 0x7F;
        rm2_channel_refresh();
    }

}





/** Button Features : semi-platform dependent <BR>
  * ========================================================================<BR>
  * This Application can be initialized by a button-press or by an ALP command.
  * The ALP command is treated at a higher-level (see ALP callback later).
  * The button-press is unique to this application, and it is treated here.
  */

#ifdef OT_SWITCH1_ISR
void OT_SWITCH1_ISR(void) {
    // Ignore the button press if the task is in progress already
    if (PING_TASK->event == 0) {
        ping_invoke(PING_CHANNEL);              // Initialize Ping Task on channel 28
    }
    
    // Connect UBX on button press.
    // ubx_connect() has its own logic to prevent overusage
    //if (setup.is_gateway == False) {
    //    gnss_connect();
    //}

///@note this section is for testing without having to wait for GNSS fix
//    sys_task_setevent(NAV_TASK, 1);
//    sys_task_setreserve(NAV_TASK, 1);
//    sys_task_setlatency(NAV_TASK, 255);
//    sys_preempt(NAV_TASK, 0);
}
#endif


void setup_button_init() {
/// ARM Cortex M boards must prepare all EXTI line interrupts in their board
/// configuration files, but the actual line interrupt must be enabled here.
#ifdef OT_SWITCH1_PIN
    EXTI->RTSR |= OT_SWITCH1_PIN;
    EXTI->IMR  |= OT_SWITCH1_PIN;
#endif
}





/** M2 Transport Callback <BR>
  * ========================================================================<BR>
  * This is a lower-level callback than the application layer callback.  It
  * should probably be merged into ALP somehow.
  *
  * In any case, in this implementation we filter for the following
  * - announcement requests
  */
ot_bool m2qp_sig_isf(ot_u8 type, ot_u8 opcode, id_tmpl* user_id) {
#   if (OT_FEATURE(MPIPE))
    ot_bool is_ann_req  = ((opcode == M2OP_ANN_S) || (opcode == M2OP_ANN_F)) && (type == M2TT_REQNA2P);
    
    if (setup.is_gateway && is_ann_req) {
        const char* label = "ANN";

        ///@todo this block of code could be turned into a logging function
#       if 0
        logger_header(MSG_utf8, 0);
        q_writestring(mpipe.alp.outq, label, 4);
        
        mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "ID: ");
        mpipe.alp.outq->putcursor += otutils_bin2hex(mpipe.alp.outq->putcursor,
                                                        user_id->value,
                                                        user_id->length );        
        mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\nRSSI: %d\nLink: %d\n", 
                                                        radio.last_rssi, radio.last_linkloss);
        logger_direct();
#       endif
    }
#   endif

    return True;
}




/** ALP Processor Callback for Starting a Ping <BR>
  * ========================================================================<BR>
  * "ALP" is the NDEF-based set of low-level API protocols that OpenTag uses.
  * ALP messages can come-in over any communication method: wire, wireless,
  * telepathy... anything that can transfer a packet payload.
  *
  * Some ALPs are standardized. Those get handled by OTlib automatically.  ALPs
  * that are not recognized are sent to this function to be handled.  In this
  * demo, we are using a very simple ALP, shown below:
  *
  * ALP Payload Length:         0
  * ALP Protocol ID:            255 (FF)
  * ALP Protocol Commands:      0-127 (00-7F) corresponding to channel to ping
  *
  * The "user_id" parameter corresponds to the Device ID that sent this ALP.
  *
  * A quickstart guide to the ALP API is available on the Indigresso Wiki.
  * http://www.indigresso.com/wiki/doku.php?id=opentag:api:quickstart
  */

///@todo change task into logger task, and make the Pingpong part entirely
///      session driven.
ot_bool alp_ext_proc(alp_tmpl* alp, const id_tmpl* user_id) {
    ot_u8 flags      	= INREC(alp, FLAGS);
    ot_u8 payload_len 	= INREC(alp, PLEN);
    ot_u8 alp_id      	= INREC(alp, ID);
    ot_u8 alp_cmd     	= INREC(alp, CMD);
    
    /// Offset=2 is the ALP ID, which defines the protocol to use.
    /// This Project has two apps (similar to inbound ports): 
    /// - ID=255: Ping-Pong Application
    /// - ID=254: GNSS data is received from Tag
    switch (alp_id) {

    // Telegram code = 16
    case 16: {
    	ot_u8   tx_eirp;
    	ot_u8   subnet;
    	ot_u8   ptype;
    	ot_u8   acc;
    	ot_u32  token;
    	ot_uni32 lat;
    	ot_uni32 lon;

    	tx_eirp 		= q_readbyte(alp->inq);
    	subnet  		= q_readbyte(alp->inq);
    	token   		= q_readlong(alp->inq);
    	ptype   		= q_readbyte(alp->inq);
    	acc      		= q_readbyte(alp->inq);

    	lat.ubyte[B3] 	= q_readbyte(alp->inq);
    	lat.ubyte[B2] 	= q_readbyte(alp->inq);
    	lat.ubyte[B1] 	= q_readbyte(alp->inq);
    	lat.ubyte[B0] 	= 0;
    	lon.ubyte[B3] 	= q_readbyte(alp->inq);
		lon.ubyte[B2] 	= q_readbyte(alp->inq);
		lon.ubyte[B1] 	= q_readbyte(alp->inq);
		lon.ubyte[B0] 	= 0;

		lat.ulong      -= 1800000000;
		lon.ulong      -= 1800000000;

    	if (setup.is_gateway) {
#           if (OT_FEATURE(MPIPE))
    		///@todo this block of code could be turned into a logging function
    		logger_header(DATA_json, 0);
    		mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "{\"tgloc\":{");
    		mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\"token\":\"%08X\", ", token);
    		mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\"acc\":%u, ", acc);
    		mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\"rssi\":%i, ", radio.last_rssi);
    		mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\"link\":%i, ", radio.last_linkloss);
    		mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\"elat\":%i, ", lat.slong);
    		mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\"elon\":%i}}", lon.slong);
    		logger_direct();
#	        endif
    	}
    } break;

    /// GNSS data in: job is only to send it to log output.
    /// Output data format is either UTF8 manifest, JSON, or raw.
    /// Binary input format is:
    /// [ flags (2) ] 
    /// flag b0 - [ Lat (4) ] 
    /// flag b1 - [ Lon (4) ] 
    /// flag b2 - [ Acc (4) ]
    /// other flags: ignored for now
    case 254: {
#       if (OT_FEATURE(MPIPE))
        if (setup.is_gateway) {
            ot_u8 idbuf[17];
            ot_u8 len;
            ot_u16 flags;
            ot_int i;
            
            static const char legend[3][4] = {
				"lat",
				"lon",
				"acc"
            };

            /// Get id string and flags
            len         = otutils_bin2hex(idbuf, user_id->value, user_id->length);
            idbuf[len]  = 0;
            flags       = q_readshort(alp->inq);

            /// JSON output model
			/// Output: MSG_raw with "UBX" as message type specifier.
			/// Clients that understand UBX JSON will parse it as needed.
			/// Message specifier can be anything, it is of main importance to the client.
			logger_header(DATA_json, 0);

			/// Write front part of JSON, including ID
			mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor,
									"{\"ubx_gnss_nav\":{\"id\":\"%s\",",
									idbuf
									);

			/// Write remaining fields based on flags, and eat last comma
			for (i=0; i<3; i++) {
				if (flags & (1<<i)) {
					ot_long word = q_readlong(alp->inq);
					mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\"%s\":%d,", legend[i], word);
				}
			}
			mpipe.alp.outq->putcursor -= (flags != 0);

			// includes null terminator, which is optional
			q_writestring(mpipe.alp.outq, "}}\n", 4);

			logger_direct();
        }
#       endif  
    } break;
    
    // This app is completely atomic and it manages the ALP data in place rather
    // than through a subordinate app queue.
    case 255: {
        alp_cmd &= 0x7f;
        
        // Start a ping
        if (alp_cmd == 0) {
            ping_invoke(PING_CHANNEL);
        }

        // Request: copy ping val to pong
        else if (alp_cmd == 255) {
            ot_u16  pongval = q_readshort(alp->inq);
            q_writeshort(alp->outq, pongval);
        }

        // Response: Compare PING Val to PONG Val and write output to MPipe
        else if (setup.is_gateway) {
#       if (OT_FEATURE(MPIPE))
            //ot_u8   i;
            //ot_u8   scratch;

            // Prepare logging header: UTF8 (text log), dummy length is 0
            logger_header(DATA_utf8, 0);

            // Print out the three parameters for PongLT
            mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "PongID: ");
            mpipe.alp.outq->putcursor += otutils_bin2hex(mpipe.alp.outq->putcursor,
                                                            user_id->value,
                                                            user_id->length     );
            
            mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\nRSSI: %d\nLink: %d\n", 
                                                radio.last_rssi, radio.last_linkloss);
            
            logger_direct();
#       endif
        }

    } break;
    
    /// Default case is mainly in case you want to trap other ALPs
    default: {
#       if (OT_FEATURE(MPIPE))
        ot_int len;
        logger_header(MSG_raw, 0);
        q_writestring(mpipe.alp.outq, (ot_u8*)"ALP", 4);
        len = q_span(alp->inq);
        q_writestring(mpipe.alp.outq, q_markbyte(alp->inq, len), len);
        logger_direct();
#		endif
    } break;
    
    }

    return True;
}





/** PING Kernel Task <BR>
  * =======================================================================<BR>
  * This function will be activated by the kernel when the external task is
  * active and there are resources available to run the task.  This task will
  * just activate the DLL session (also a task) and then turn itself off.
  * Then, the kernel will call the DLL session task as soon as necessary
  * resources are available to run that task (by default DLL task is top
  * priority, so it should get called right away).
  */
void ping_systask(void* arg) {
    ot_task task = (ot_task)arg;
    session_tmpl    s_tmpl;

    switch (task->event) {
    
    // Constructor Destructor: do nothing.
    case 0: break;
    
    // Button is pressed
    // 1 - Tell GPS module to startup (usually takes some time to get started up)
    // 2 - Send a Ping
    case 1:
    {   //static const char legend[3][4] = {
        //    "lat",
        //    "lon",
        //    "acc"
        //};
        //ot_u8 idbuf[17];
        //ot_u8 len;
        //ot_u16 flags;
        //ot_int i;
                
        /// Output: MSG_raw with "JSON" as message type specifier.
        /// Clients that understand JSON will parse it as needed.
        /// Message specifier can be anything, it is of main importance to the client.
#		if (OT_FEATURE(MPIPE))
        logger_header(MSG_utf8, 0);

        ///@note important to include null terminator in MSG type
        q_writestring(mpipe.alp.outq, (ot_u8*)"UBX", 4);
        
        /// Write front part of JSON, including ID
        mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, 
                                "{\"ubx_gnss_nav\":{\"id\":\"0102030405060708\","); 
        
        mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\"lat\":0,");
        mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\"lon\":0");
        
        // includes null terminator, which is optional
        q_writestring(mpipe.alp.outq, "}}\n", 4);   
        
        logger_direct();
#		endif

        task->event = 0;
        break;
    }
        
//        // Invoke a blocking state to prevent the button from attempted invoke
//        task->event = 2;
//
//        // this is the same as the length of the response window,
//        // which is set in ping_query_applet(), plus a bit of extra time
//        task->nextevent = 512 + 64;
//
//        // Generate a pseudo random 16 bit number to be used as a ping check value
//        ping.val = PLATFORM_ENDIAN16(rand_prn16());
//
//        // Log a message.  It is scheduled, and the RF task has higher priority,
//        // so if you are sending a DASH7 dialog this log message will usually
//        // come-out after the dialog finishes.
//        ///@todo this seems to cause a crash when it is followed with GNSS
//#       if (OT_FEATURE(MPIPE))
//        if (setup.is_gateway) {
//            logger_msg(MSG_raw, 5, 2, (ot_u8*)"PING:", (ot_u8*)&ping.val);
//        }
//#       endif
//
//        // Load the session template: Only used for communication tasks
//        s_tmpl.channel      = task->cursor;
//        s_tmpl.flagmask     = 0;
//        s_tmpl.subnetmask   = 0;
//        m2task_immediate(&s_tmpl, &ping_query_applet);
//        //m2task_schedule(&s_tmpl, &ping_query_applet, 0);
//        break;
    
    /// There is only one usage of this app, with no init
    /// Use any non-zero, non-one state to block the button press.
    default: task->event = 0;
            break;
    }
}






/** GNSS Reporter Task <BR>
  * =======================================================================<BR>
  * This function manages the data element of the GNSS.  It is instantiated
  * by the GNSS callback.
  */

void gnss_callback(ot_int rx_type, ot_int msg_type) {

	// 0 is a control message
	// 0, 0 --> error accessing the control interface to GNSS device
	// 0, 1 --> GNSS Receiver is turning-off
	// 0, 2 --> GNSS Receiver is turning-on
	// 1 indicates we have a UBX message.
	// 1, 0x0107 --> NAV-PVT message.
	switch (rx_type) {
	case 0:
		switch (msg_type) {
		//case 0: break;	//Nothing done yet to handle this.  Maybe Log a message.
		case 1:
		case 2:
			dll_silence( (ot_bool)(msg_type-1) );
			break;
		}
		break;
	case 1:
		// The event == 0 check is to prevent collision.
		// when the cron task is complete, this process will change.
		if ((msg_type == 0x0107) && (NAV_TASK->event == 0)) {
			sys_task_setevent(NAV_TASK, 1);
			sys_task_setcursor(NAV_TASK, PING_CHANNEL);
			sys_task_setreserve(NAV_TASK, 1);
			sys_task_setlatency(NAV_TASK, 255);
			sys_preempt(NAV_TASK, 250);
		}
		break;
	}
}


void gnss_storedata(ot_s32* lat, ot_s32* lon, ot_u32* acc) {
    vlFILE* fp;

    fp = ISF_open_su(ISF_ID(gnss_output));
    if (fp != NULL) {
        ((ot_u16*)lat)[0] = vl_read(fp, 28);
        ((ot_u16*)lat)[1] = vl_read(fp, 30);
        ((ot_u16*)lon)[0] = vl_read(fp, 24);
        ((ot_u16*)lon)[1] = vl_read(fp, 26);
        ((ot_u16*)acc)[0] = vl_read(fp, 40);
        ((ot_u16*)acc)[1] = vl_read(fp, 42);
    }
    else {
        *lat = 0;
        *lon = 0;
        *acc = 0;
    }
    vl_close(fp);
}


void gnssreporter_systask(void* arg) {
    ot_task task = (ot_task)arg;
    session_tmpl    s_tmpl;

    switch (task->event) {
    
    // Constructor/Destructor
    case 0: {
        // Connect UBX on task startup
        // ubx_connect() has its own logic to prevent overusage
        if (setup.is_gateway == False) {
            gnss_connect();
        }
    } break;
    
    // Data is ready
    case 1: {
#       if (OT_FEATURE(MPIPE))
        if (setup.is_gateway) {
            ot_long lat;
            ot_long lon;
            ot_long acc;
           
            gnss_storedata( (ot_s32*)&lat, (ot_s32*)&lon, (ot_u32*)&acc);
            
            logger_header(DATA_utf8, 0);
            mpipe.alp.outq->putcursor += sprintf((char*)mpipe.alp.outq->putcursor, 
                "Lat: %d\nLon: %d\nAcc: %d\n\n", lat, lon, acc);
    
            logger_direct();
        }
#       endif
        
        // Load the session template: Only used for communication tasks
        //s_tmpl.channel      = PING_CHANNEL;
        //s_tmpl.flagmask     = 0;
        //s_tmpl.subnetmask   = 0;
        //m2task_immediate(&s_tmpl, &gnss_broadcast_applet);

        s_tmpl.channel      = TG_CHANNEL;
        s_tmpl.flagmask     = 0;
        s_tmpl.subnetmask   = 0;
        m2task_telegram(&s_tmpl, &gnss_telegram_applet);
        
        /// This task is a one-shot
        task->event = 0;
    } break;
        
    // Lockout is over
    default: 
        task->event = 0;
        break;
    }
}










/** Communication Task Applets  <BR>
  * ========================================================================<BR>
  * Communication tasks in OpenTag are typically created by one of the OTAPI
  * Tasker functions in /otlib/OTAPI_tasker.h.  The task is created here, and
  * when it gets activated by the kernel, the applet that is attached to it
  * will run.  The main job of the applet is to load prepare the communication.
  * Usually, this means loading a request packet, but you could also make an
  * applet that does any manner of state-based communication routines.  The
  * Adaptive Search and CoAP demos are examples of sophisticated applets.
  */
void gnss_telegram_applet(m2session* active) {
/// This is a page operation, which gets buried directly onto the queue
	ot_u32  dev_token;
	ot_s32  lat, lon;
	ot_u32  acc;
	float 	acc_x;
	ot_u8   data_id;
	ot_u8   tgram_payload[7];

	gnss_storedata( &lat, &lon, &acc);

    // Geolocation Type 0 Telegram
    data_id = 0x10 | 0x0;

	acc >>= 2;
	acc_x = (float)acc / 2000.f;
	if (acc_x == 0) {
		tgram_payload[0] = 100;
	}
	else {
		acc_x = (logf(acc_x) / logf(2000.f*100));
		if (acc_x > 1.f) {
			tgram_payload[0] = 1;
		}
		else {
			acc_x = 100.f - (99.f * acc_x);
			tgram_payload[0] = (ot_u8)acc_x;
		}
	}

	// Make lat/lon based on 0:360 degrees rather than -180:180.
	// This enables it to be unsigned and thus precision reduced by arbitrary shifting.
	lat = (lat + 1800000000);
	lon = (lon + 1800000000);

	tgram_payload[1] = ((ot_u8*)&lat)[B3];
	tgram_payload[2] = ((ot_u8*)&lat)[B2];
	tgram_payload[3] = ((ot_u8*)&lat)[B1];
	tgram_payload[4] = ((ot_u8*)&lon)[B3];
	tgram_payload[5] = ((ot_u8*)&lon)[B2];
	tgram_payload[6] = ((ot_u8*)&lon)[B1];

	/// We use the lower 32 bits of the HW-ID as the device token.
	/// This is an imperfect solution, but it's handy for demo/demokit purposes
	dev_token = *((ot_u32*)(UID_BASE + 0x14));

    otapi_new_telegram(dev_token, data_id, tgram_payload);
    otapi_start_dialog(1024);
}


void gnss_broadcast_applet(m2session* active) {
    ot_u8   status;
    ot_u8   ubx_payload[14];
    ot_s32  lat, lon;
    ot_u32  acc;
    
    /// Set flags to 0000 0000 0000 0111
    ubx_payload[0] = 0;
    ubx_payload[1] = 0x07;
    
    gnss_storedata( &lat, &lon, &acc);
    
    ubx_payload[2] = ((ot_u8*)&lat)[B3];
    ubx_payload[3] = ((ot_u8*)&lat)[B2];
    ubx_payload[4] = ((ot_u8*)&lat)[B1];
    ubx_payload[5] = ((ot_u8*)&lat)[B0];
    
    ubx_payload[6] = ((ot_u8*)&lon)[B3];
    ubx_payload[7] = ((ot_u8*)&lon)[B2];
    ubx_payload[8] = ((ot_u8*)&lon)[B1];
    ubx_payload[9] = ((ot_u8*)&lon)[B0];
    
    ubx_payload[10] = ((ot_u8*)&acc)[B3];
    ubx_payload[11] = ((ot_u8*)&acc)[B2];
    ubx_payload[12] = ((ot_u8*)&acc)[B1];
    ubx_payload[13] = ((ot_u8*)&acc)[B0];

    { //open request for single hop broadcast
        routing_tmpl routing;
        routing.hop_code = 0;
        otapi_open_request(ADDR_broadcast, &routing);
    }
    { //use a command template for collection of single file from single file search
        command_tmpl command;
        command.opcode      = (ot_u8)CMD_udp_on_file;
        command.type        = (ot_u8)CMDTYPE_bcast_request;
        command.extension   = (ot_u8)CMDEXT_no_response | (ot_u8)CMDEXT_null_file;
        otapi_put_command_tmpl(&status, &command);
    }
    { //write the dialog information (timeout, channels to use)
        dialog_tmpl dialog;
        dialog.channels = 0;    //use same channel as request for response
        dialog.timeout  = 0;    //no response slot
        otapi_put_dialog_tmpl(&status, &dialog);
    }
    { //
        // Setting udp.data to NULL causes the src file to be loaded in entirety.
        udp_tmpl udp;
        udp.data_length     = 14;
        udp.dst_port        = 254;
        udp.src_port        = ISF_ID(gnss_output);
        udp.data            = ubx_payload;
        otapi_put_udp_tmpl(&status, &udp);
    }

    //Done building command, close the request and send the dialog
    otapi_close_request();
}



void ping_query_applet(m2session* active) {
/// The C-API for building commands can be bypassed in favor of directly
/// putting data to the queue.  That way is more efficient, but it also requires
/// you to know more about DASH7 than just what order the templates should be.
///
/// The query that we build will collect sensor configuration data back from
/// all devices that support the sensor protocol.  Much more interesting queries
/// are possible.
    ot_u8 status;

    { //open request for single hop anycast query
        routing_tmpl routing;
        routing.hop_code = 0;
        otapi_open_request(ADDR_broadcast, &routing);
    }
    { //use a command template for collection of single file from single file search
        command_tmpl command;
        command.opcode      = (ot_u8)CMD_udp_on_file;
        command.type        = (ot_u8)CMDTYPE_acast_request;
        command.extension   = (ot_u8)CMDEXT_none;
        otapi_put_command_tmpl(&status, &command);
    }
    { //write the dialog information (timeout, channels to use)
        dialog_tmpl dialog;
        dialog.channels = 0;    //use same channel as request for response
        dialog.timeout  = 0x41; //same as otutils_encode_timeout(512) -- 512 tick response slot
        otapi_put_dialog_tmpl(&status, &dialog);
    }
    { //write the query to search for the sensor protocol id
        static const ot_u8 query_str[10] = "APP=PongLT";
        query_tmpl query;
        query.code      = M2QC_COR_SEARCH + 10; // do a 100% length=10 correlation search
        query.mask      = NULL;                 // don't do any masking (no partial matching)
        query.length    = 10;                   // query_str is 10 bytes
        query.value     = (ot_u8*)query_str;
        otapi_put_query_tmpl(&status, &query);
    }
    { //put in the information of the file to search (the user id)
        isfcomp_tmpl isfcomp;
        isfcomp.is_series   = False;
        isfcomp.isf_id      = ISF_ID(user_id);
        isfcomp.offset      = 0;
        otapi_put_isf_comp(&status, &isfcomp);
    }
    { //put in UDP ports (source=17 [0x11], dest=255) and Ping ID
        udp_tmpl udp;
        udp.data_length    = 2;
        udp.dst_port       = 255;
        udp.src_port       = ISF_ID(user_id);
        udp.data           = (ot_u8*)&ping.val;
        otapi_put_udp_tmpl(&status, &udp);
    }

    //Done building command, close the request and send the dialog
    otapi_close_request();
}








/** Main Configuration <BR>
  * ==================================================================<BR>
  * This function should run after platform_poweron() but before
  * platform_init_OT().  Here you can configure or do things before 
  * any of the kernel & system features are initialized.  It's a good
  * place to put features that control which drivers or apps get 
  * activated.
  */
  
void setup_init() {
/// 1. Determine if device is gateway based on initial value of button, at startup.
///    @todo it would be better to simply monitor the initial value of UART lines,
///    to see if they are connected, but this cannot be so easily guaranteed.
/// 2. Set UBS-GNSS nav event callback.
    
    // Default is gateway-mode (button not depressed)
#if defined(__FORCE_GATEWAY)
    setup.is_gateway = 1;
#elif defined(OT_SWITCH1_PIN)
    setup.is_gateway = (ot_bool)((OT_SWITCH1_PORT->IDR & OT_SWITCH1_PIN) != (OT_SWITCH1_POLARITY << OT_SWITCH1_PIN));
#else
    setup.is_gateway = 0;
#endif
    
    /// Blink the board LEDs to show that it is starting up.
    {   ot_u8 i;

        i=4;
        while (i != 0) {
            if (i&1)    BOARD_led1_on();
            else        BOARD_led2_on();

            delay_ti(60);
            BOARD_led2_off();
            BOARD_led1_off();
            i--;
        }
    }
    
    // BusyWait for button to be released.
#   ifdef OT_SWITCH1_PIN
    while ((OT_SWITCH1_PORT->IDR & OT_SWITCH1_PIN) == (BOARD_SW1_POLARITY << OT_SWITCH1_PIN));
#   endif
    
    if (setup.is_gateway == False) {
        gnss_config(&gnss_callback, __GNSS_PERIOD_S);
    }
    
    // Configure the button, which will now generate an interrupt when pushed.
    setup_button_init();
}





void ping_invoke(ot_u8 channel) {
/// The "External Task" is the place where the kernel runs the main user app.
/// Our app has 4 functions (call types).
/// <LI> The task event state is set to 1.  Event 0 is always Task-off, but
///      otherwise each task manages its own event numbers </LI>
/// <LI> We store the channel in the cursor, which is not being used otherwise.
///      The Task "cursor" can be used for additional state control, beyond the
///      event number. </LI>
/// <LI> We give it a runtime reservation of 1 tick (it runs pretty fast).
///      This is also short enough to pre-empt RX listening, but not RX data.
///      Try changing to a higher number, and observing how the kernel
///      manages this task. </LI>
/// <LI> We give it a latency of 255.  Latency is unimportant for run -once
///      tasks, so giving it the max latency will prevent it from blocking
///      any other tasks. </LI>
/// <LI> We tell it to start ASAP (preemption delay parameter = 0) </LI>
///
/// @note The latency parameter is mostly useful for protocol management,
/// for which you probably want to enforce a request-response turnaround time.
/// for processing and for iterative tasks it is not important: set to 255.
///
    sys_task_setevent(PING_TASK, 1);
    sys_task_setcursor(PING_TASK, channel);
    sys_task_setreserve(PING_TASK, 1);
    sys_task_setlatency(PING_TASK, 255);
    sys_preempt(PING_TASK, 0);
}



ot_int beacons_off(void) {
    vlFILE* fp;
    ot_int output = -1;

    fp = ISF_open_su(0);
    if (fp != NULL) {
        output = vl_write(fp, 6, 0);
        vl_close(fp);

        dll_refresh();
    }

    return output;
}

ot_int sleepyrx_on(void) {
    const ot_u8 scanlist[] = \
          { 0x0C, 0x01, PING_CHANNEL, 0x80 };
    
    vlFILE* fp;
    ot_int output = -1;

    fp = ISF_open_su(5);
    if (fp != NULL) {
        vl_store(fp, 4, scanlist);
        vl_close(fp);
        dll_refresh();
    }

    return output;
}

void setup_tgramlisten(bool fg_or_bg) {
    static const ot_u8 sleep_gateway[4] = { SPLIT_TIME16(2,0,0), TG_CHANNEL, 0x50 };
    static const ot_u8 sleep_endpoint[4]= { SPLIT_TIME16(0,2,3), TG_CHANNEL, 0x80 };
    vlFILE* fp;
    ot_u8* data;

    fp = ISF_open_su(5);
    if (fp != NULL) {
        data = fg_or_bg ? (ot_u8*)sleep_gateway : (ot_u8*)sleep_endpoint;
        vl_store(fp, 4, data);
        vl_close(fp);
        dll_refresh();
    }
}


void main(void) {
    ///1. Standard Power-on routine (Clocks, Timers, IRQ's, etc)
    ///2. Pre-OT initialization
    ///3. Standard OpenTag Init (most stuff actually will not be used)
    platform_poweron();
    setup_init();
    platform_init_OT();

    /// Optional modifications from defaults
    app_set_txpwr((ot_int)(__DBM_DEFAULT));
    setup_tgramlisten( setup.is_gateway );

    if (setup.is_gateway) {
        beacons_off();
        asapi_init_srv();
    }
    else {
    	beacons_off();
        //sleepyrx_on();
    }



    ///4. Set the app name (PongLT) as a cookie in the User-ID.
    ///   This is used for query filtering
    {   static const ot_u8 appstr[] = "APP=PongLT";
        vlFILE* fp;
        fp = ISF_open_su(ISF_ID(user_id));
        if (fp != NULL) {
            vl_store(fp, sizeof(appstr), appstr);
        }
        vl_close(fp);
    }

    ///5a. The device will wait (and block anything else) until you connect
    ///    it to a valid console app.
    ///5b. Load a message to show that main startup has passed
#   if (OT_FEATURE(MPIPE))
    if (setup.is_gateway) {
        mpipedrv_standby();
        logger_msg(MSG_utf8, 6, 27, (ot_u8*)"SYS_ON", (ot_u8*)"System on and Mpipe active\n");
    }
#   endif

    ///6. MAIN RUNTIME (post-init)  <BR>
    ///<LI> Use a main loop with platform_ot_run(), and nothing more. </LI>
    ///<LI> You could put code before or after platform_ot_run, which will
    ///     run before or after the (task + kernel).  If you do, keep the code
    ///     very short or else you are risking timing glitches.</LI>
    ///<LI> To run any significant amount of user code, use tasks. </LI>
    while(1) {
        platform_ot_run();
    }
}


