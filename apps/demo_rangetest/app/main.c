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
  * @file       /apps/ht_rangetest/app/main.c
  * @author     JP Norair
  * @version    R100
  * @date       27 Apr 2017
  * @brief      HayTag Range Demo
  *
  * This demo uses 
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>
#include <board.h>
#include <otlib/delay.h>
#include <otlib/rand.h>
#include <otlib/logger.h>

#include <otsys.h>
#include <m2api.h>

#include <stdio.h>


#include <m2/radio.h>


#define PING_PERIOD_TICKS	512
#define PING_CHANNEL    	(0xA8) //(0xA8)

// You can set these parameters:
// __DBM_DEFAULT:   the starting power (dBm)
#define __DBM_DEFAULT       20





/** Data Mapping <BR>
  * ===========================================================================
  * The Opmode Demo needs a particular data mapping.  It is not unusual, but
  * the demo may not work if the data is not set correctly.  This define below
  * uses the default data mapping (/apps/demo_opmode/code/data_default.c)
  */
#define __DATA_DEFAULT

// nice to have for debugging
//#define __FORCE_GATEWAY





/** Main Static Variables <BR>
  * ========================================================================<BR>
  */
typedef struct {
    ot_bool is_gateway;
} setup_struct;

static setup_struct setup;




/** Local Functions & definitions
  * ========================================================================
  */

// Main Application Functions
void setup_init();

// Reset Channel TX Power function
void app_set_txpwr(ot_int dBm);



/** Button Features : semi-platform dependent <BR>
  * ========================================================================<BR>
  * This Application can be initialized by a button-press or by an ALP command.
  * The ALP command is treated at a higher-level (see ALP callback later).
  * The button-press is unique to this application, and it is treated here.
  */
#if defined(OT_SWITCH1_ISR)
void OT_SWITCH1_ISR(void) {
    // Ignore the button press if the task is in progress already
    //if (PING_TASK->event == 0) {
    //    ping_invoke(PING_CHANNEL);              // Initialize Ping Task on channel 18
    //}
}
#endif




/** M2 Transport Callback <BR>
  * ========================================================================<BR>
  * This is a lower-level callback than the application layer callback.  It
  * should probably be merged into ALP somehow.
  *
  * In any case, in this implementation we filter for the following
  * - announcement requests
  * - inventory responses (not inventory w/ UDP)
  */
ot_bool m2qp_sig_isf(ot_u8 type, ot_u8 opcode, id_tmpl* user_id) {

#   if (OT_FEATURE(MPIPE))
	if (setup.is_gateway) {

		///@todo this block of code could be turned into a logging function
		logger_header(MSG_utf8, 0);
		q_writestring(mpipe.alp.outq, "FRX", 4);

		mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "ID: ");

		mpipe.alp.outq->putcursor += otutils_bin2hex(mpipe.alp.outq->putcursor,
														user_id->value, user_id->length );

		mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor,
										" | RSSI: %d | Link: %d\n",
		                                radio.last_rssi, radio.last_linkloss);
		logger_direct();
	}
#	endif

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
ot_bool alp_ext_proc(alp_tmpl* alp, id_tmpl* user_id) {
    ot_u8 flags, payload_len, alp_id, alp_cmd;
    
    flags       = q_readbyte(alp->inq);
    payload_len = q_readbyte(alp->inq);
    alp_id      = q_readbyte(alp->inq);
    alp_cmd     = q_readbyte(alp->inq);
    
    /// Offset=2 is the ALP ID, which defines the protocol to use.
    /// This Project has two apps (similar to inbound ports): 
    /// - ID=255: Ping-Pong Application
    /// - ID=254: GNSS data is received from Tag
    switch (alp_id) {
    
    /// Default case is mainly in case you want to trap other ALPs
    default: {
#	if (OT_FEATURE_MPIPE)
        ot_int len;
        logger_header(MSG_raw, 0);
        q_writestring(mpipe.alp.outq, (ot_u8*)"ALP", 4);
        len = q_span(alp->inq);
        q_writestring(mpipe.alp.outq, q_markbyte(alp->inq, len), len);
        logger_direct();
#	endif
    } break;
    
    }

    return True;
}






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
    
    // Set it up as gateway by default (switch open, if there is a switch at all)
#   if defined(__FORCE_GATEWAY)
    setup.is_gateway = 1;
#   elif !defined(OT_SWITCH1_ISR)
    setup.is_gateway = 0;
#   else
    setup.is_gateway = (ot_bool)((OT_SWITCH1_PORT->IDR & OT_SWITCH1_PIN) != (OT_SWITCH1_POLARITY << OT_SWITCH1_PIN));
#   endif
    
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
#   if defined(OT_SWITCH1_ISR) && !defined(__FORCE_GATEWAY)
    while ((OT_SWITCH1_PORT->IDR & OT_SWITCH1_PIN) == (BOARD_SW1_POLARITY << OT_SWITCH1_PIN));
#   endif
    
}



ot_int setup_beacons(ot_u16 interval) {
	ot_u8 beacon_list[8] = {PING_CHANNEL, 0x02, 0x20, 0x00, 0x00, 0x08, 0x00, 0x00};
    vlFILE* fp;
    ot_int 	output = -1;

    if (interval == 0) {
    	fp = ISF_open_su(0);
		if (fp != NULL) {
			output = vl_write(fp, 6, 1);
			vl_close(fp);
			dll_refresh();
		}
    }
    else {
    	beacon_list[6] = (ot_u8)(interval >> 8);
    	beacon_list[7] = (ot_u8)(interval);
    	fp = ISF_open_su(6);
		if (fp != NULL) {
			output = vl_store(fp, 8, beacon_list);
			vl_close(fp);
			dll_refresh();
		}
    }

    return output;
}


void setup_listen(bool fg_or_bg) {
    static const ot_u8 sleep_gateway[4] = { PING_CHANNEL, 0x50, 0x00, 0x00 };
    static const ot_u8 sleep_endpoint[4]= { PING_CHANNEL, 0x80, 0x02, 0x00 };
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
    ///2. Standard OpenTag Init (most stuff actually will not be used)
    platform_poweron();
    platform_init_OT();

    // 3. Preparing this application (local functions)
    setup_init();
    setup_listen( setup.is_gateway );
    setup_beacons( setup.is_gateway ? 0 : PING_PERIOD_TICKS );
    app_set_txpwr((ot_int)(__DBM_DEFAULT));
    
    //asapi_init_srv();
    
    ///4. Set the app name (PongPwr) as a cookie in the User-ID.
    ///   This is used for query filtering
    {   static const ot_u8 appstr[] = "APP=RangeTest";
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





/** Patches <BR>
  * ========================================================================<BR>
  */
#include <otlib/buffers.h>
void dll_sig_rfterminate(ot_int pcode, ot_int scode) {
    BOARD_led2_off();   //Orange LED off
    BOARD_led1_off();   //Green LED off

    // When a Frame is received and CRC is bad
#	if (OT_FEATURE_MPIPE)
    if ((pcode == 3) && (scode == -1)) {
    	char legend[16];
    	int len;
    	len = sprintf(legend, "BADFRX(%d)", radio.last_rssi);
    	logger_msg(MSG_raw, len, q_length(&rxq), (ot_u8*)legend, rxq.front);
    }
#	endif
}






#if defined(__DATA_DEFAULT)
#   include <../_common/fs_default_startup.c>
#else
#   error "There is no data mapping specified.  Put one here."
#endif

