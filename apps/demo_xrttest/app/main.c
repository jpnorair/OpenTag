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
  * @file       /apps/ht_xrttest/app/main.c
  * @author     JP Norair
  * @version    R100
  * @date       8 Oct 2017
  * @brief      HayTag API demo
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
#include <alp/asapi.h>

#include <stdio.h>

#include <m2/radio.h>


#ifndef __PAGE_PERIOD_TI__
#   define __PAGE_PERIOD_TI__   (1024)
#endif

#define __PAGE_NUMBURST__		1


// Channel power and number are currently dependent on the region
#if (RF_PARAM_BAND == 866)
#   define __DBM_DEFAULT        14
#   define TG_CHANNEL           (0x99)  //867.125
#elif (RF_PARAM_BAND == 433)
#   define __DBM_DEFAULT        10
#   define TG_CHANNEL           (0x99)  //434.036
#else //(RF_PARAM(BAND) == 915)
#   define __DBM_DEFAULT        20
#   define TG_CHANNEL           (0x89) // Could be also another channel
#endif


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







// nice to have for debugging
//#define __FORCE_GATEWAY


/** Main Static Variables <BR>
  * ========================================================================<BR>
  */
typedef struct {
    ot_bool is_gateway;
    ot_bool	page_received;
    ot_int	page_type;
    ot_int 	page_count;
} setup_struct;

static setup_struct setup = {
		.is_gateway = False,
		.page_received = False,
		.page_type = 0,
		.page_count = __PAGE_NUMBURST__
};





/** Local Functions & definitions
  * ========================================================================
  */
#define XRTTEST_TASK    (&sys.task[TASK_xrttest])

// Main Application Functions
void setup_init(void);
ot_int setup_beacons(ot_u16 interval, ot_u8 channel);
void setup_listen(bool is_gateway, ot_u8 channel);

// Ping Functions
void xrt_invoke(ot_u8 state);
void xrt_applet_fec(m2session* active);
void xrt_applet_nofec(m2session* active);




/** Button Features : semi-platform dependent <BR>
  * ========================================================================<BR>
  * This Application can be initialized by a button-press or by an ALP command.
  * The ALP command is treated at a higher-level (see ALP callback later).
  * The button-press is unique to this application, and it is treated here.
  */
#if defined(OT_SWITCH1_ISR)
void OT_SWITCH1_ISR(void) {
	if (setup.is_gateway) {
		xrt_invoke(2);
	}
}
#endif







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
extern ot_u8 rxwb_sv[16];
extern ot_u8 rxnb_sv[16];
//volatile ot_u16 tim_sv[16];
//volatile ot_u16 bits_sv[16];

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
		ot_u32  token;
		ot_uni16 dtype;
		ot_uni16 pcnt;
		radio_snr_t* snrdata = radio_getlinkinfo();

		setup.page_received = True;

		tx_eirp	= q_readbyte(alp->inq);
		subnet  = q_readbyte(alp->inq);
		token   = q_readlong(alp->inq);
		ptype   = q_readbyte(alp->inq);

		dtype.ubyte[UPPER] 	= q_readbyte(alp->inq);
		dtype.ubyte[LOWER] 	= q_readbyte(alp->inq);
		pcnt.ubyte[UPPER] 	= q_readbyte(alp->inq);
		pcnt.ubyte[LOWER] 	= q_readbyte(alp->inq);

#       if (OT_FEATURE(MPIPE))
		if (setup.is_gateway) {
			///@todo this block of code could be turned into a logging function
			logger_header(DATA_json, 0);
			mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "{\"tgtest\":{");
			mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\"token\":\"%08X\", ", token);
			mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\"rssi\":%i, ", radio.last_rssi);
			mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\"mean-snr\":%i}}", snrdata->mean_snr);

//			mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\"min-ebn0\":%i, ", snrdata->min_ebn0);
//			mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor, "\"max-ebn0\":%i}}", snrdata->max_ebn0);

			logger_direct();
		}
#	    endif
	} break;

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





/** Communication Task Applets  <BR>
  * ========================================================================<BR>
  */

void xrt_applet_fec(m2session* active) {
/// This is a page operation, which gets buried directly onto the queue
	ot_u8 	payload[7];
	ot_u32  dev_token = ((ot_u32*)UID_BASE)[0];

	payload[0]	= 0;
	payload[1]	= 0x81;
	payload[2]	= 0;
	payload[3]	= (ot_u8)setup.page_count;
	payload[4]	= 0;
	payload[5]	= 0;
	payload[6]	= 0;

    otapi_new_telegram(dev_token, 2, payload);
    otapi_start_dialog(1024);
}

void xrt_applet_nofec(m2session* active) {
/// This is a page operation, which gets buried directly onto the queue
	ot_u8 	payload[7];
	ot_u32 	dev_token = ((ot_u32*)UID_BASE)[0];

	payload[0]	= 0;
	payload[1]	= 0x01;
	payload[2]	= 0;
	payload[3]	= (ot_u8)setup.page_count;
	payload[4]	= 0;
	payload[5]	= 0;
	payload[6]	= 0;

    otapi_new_telegram(dev_token, 2, payload);

    // Dirty hack: make payload 32 bytes
    //txq.putcursor += 16;

    otapi_start_dialog(1024);
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

void xrt_invoke(ot_u8 state) {
    sys_task_setevent(XRTTEST_TASK, state);
    sys_task_setreserve(XRTTEST_TASK, 1);
    sys_task_setlatency(XRTTEST_TASK, 255);
    sys_preempt(XRTTEST_TASK, 0);
}





void xrttest_systask(void* arg) {
    ot_task         task; 
    session_tmpl    s_tmpl;
    ot_long			nextevent_ti;
    ot_app		applet;

    task = (ot_task)arg;
    
    switch (task->event) {
    
    // Constructor Destructor: do nothing.
    case 0: {
    	setup.page_count = __PAGE_NUMBURST__;
    	setup.page_received = False;
    	setup.page_type = 0;
    	break;
    }
    
    // Operational Routine
    case 1: {
    	xrttest_systask_OPERATIONAL:
    	if (setup.is_gateway) {
    	    // Check the status each __PAGE_PERIOD_TI__ * 1.5
            // and set the Blue LED if OK
#           if defined(OT_TRIG4_PIN)
            if (setup.page_received)    BOARD_led4_on();
            else                        BOARD_led4_off();
#			elif defined(OT_TRIG3_PIN)
    		if (setup.page_received)    BOARD_led3_on();
    		else                        BOARD_led3_off();
#			endif
    		setup.page_received	= False;
    		nextevent_ti		= __PAGE_PERIOD_TI__*3;

    		//logger_msg(MSG_utf8, 2, 9, (ot_u8*)"RX", (ot_u8*)"Test Msg\n");
    	}
    	else {
			// Prepare a Page each __PAGE_PERIOD_TI__
			//if (setup.page_type == 0) {
				s_tmpl.channel 	= (TG_CHANNEL);
				applet 			= &xrt_applet_fec;
			//}
			//else {
			//	s_tmpl.channel 	= ((~0x80)&TG_CHANNEL);
			//	applet 			= &xrt_applet_nofec;
			//}
			s_tmpl.flagmask     = 0;
			s_tmpl.subnetmask   = 0;
			m2task_telegram(&s_tmpl, applet);
			// First job, change the encoding back and forth
			setup.page_type 	= (setup.page_type == 0);
			nextevent_ti = __PAGE_PERIOD_TI__;
		}
    	task->event = 1;
    	sys_task_setnext(task, nextevent_ti);
    } break;

    // Reset routine (for Gateway only)
    case 2: {
    	if (setup.is_gateway) {
    		ot_u8 chan = (setup.page_count == 0) ? (0x80|TG_CHANNEL) : ((~0x80)&TG_CHANNEL);
    		setup.page_count = (setup.page_count == 0);
    		setup_listen(setup.is_gateway, chan);
    	}
    	goto xrttest_systask_OPERATIONAL;
    } break;

    default: 
        task->event = 0;
        break;
    }

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

    // Setup Button for interrupt
#	ifdef OT_SWITCH1_PIN
    EXTI->RTSR1 |= OT_SWITCH1_PIN;
    EXTI->IMR1  |= OT_SWITCH1_PIN;
#	endif
}




ot_int setup_beacons(ot_u16 interval, ot_u8 channel) {
	ot_u8 beacon_list[8] = {0x00, 0x00, TG_CHANNEL, 0x02, 0x20, 0x00, 0x00, 0x08};
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
    	beacon_list[2] = channel;
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


void setup_listen(bool is_gateway, ot_u8 channel) {
	ot_u8 sleep_gateway[4] = { SPLIT_TIME16(2,0,0), TG_CHANNEL, 0x50 };
	ot_u8 sleep_endpoint[4]= { SPLIT_TIME16(0,2,3), TG_CHANNEL, 0x80 };
    vlFILE* fp;
    ot_u8* data;

    fp = ISF_open_su(5);
    if (fp != NULL) {
        data = is_gateway ? (ot_u8*)sleep_gateway : (ot_u8*)sleep_endpoint;
        data[2] = channel;
        vl_store(fp, 4, data);
        vl_close(fp);
        dll_refresh();
    }
}

//#define HSCODE_UNITTEST
#ifdef HSCODE_UNITTEST
#include <hblib/hscode.h>
volatile ot_int testreg;
ot_int test_hscode(ot_int trials) {
	static const uint8_t encoder_in[1][8] = {
		{0xE0, 0xD3, 0xCF, 0x66, 0x5F, 0xAD, 0x70, 0x12}
	};
	static const uint8_t encoder_out[1][16] = {
		{0xB8, 0x8D, 0xFC, 0xC9, 0x3C, 0x09, 0xE1, 0x24, 0x74, 0xD8, 0xC0, 0x50, 0x96, 0x09, 0x11, 0xBD}
	};
	static const int32_t llr_6dB[1][128] = {
		{ 	-3228,3491,-1001,-1340,-3734,3157,1463,2714,
			-2092,1985,1311,998,-2460,-2247,958,-146,
			-2711,-3385,-2409,-3267,314,-381,1922,2931,
			-3204,-2094,1844,1133,-3427,364,1569,1107,
			335,1632,-3393,-580,-2760,-3932,966,1470,
			1530,3231,1089,2968,-2190,1097,681,-2265,
			-3195,-2246,-3944,1683,2593,2322,1947,-4498,
			3545,1898,-2286,2347,3448,-1692,2218,1325,
			1099,-596,-2099,-4136,1782,-1240,1070,4028,
			-589,-1880,4040,144,-1690,3252,1817,255,
			-2810,-3276,2406,1903,1704,2053,1568,-455,
			1640,-1069,1733,-2197,3271,3503,2477,-308,
			-2059,1597,1070,-1568,1654,-3383,-2148,2945,
			1874,2183,2354,1284,-3075,2195,2342,-3097,
			2420,2861,2637,-1626,866,2778,932,-1330,
			-1962,1834,-2063,-1243,-473,-2192,2476,-1214
		}
	};
	uint8_t output[16] = {0};
	hsc_t hsc;
	uint8_t crcreg;

	hsc_init(&hsc, RATE64);
	hsc_encode(&hsc, output, &encoder_in[0]);

	crcreg = hsc_decode(&hsc, output, &llr_6dB[0]);

	return (ot_int)crcreg;
}
#endif


void main(void) {
    ///1. Standard Power-on routine (Clocks, Timers, IRQ's, etc)
    ///2. Standard OpenTag Init (most stuff actually will not be used)
    platform_poweron();
    platform_init_OT();

    // 3. Preparing this application (local functions)
    setup_init();
    app_set_txpwr((ot_int)(__DBM_DEFAULT));
    setup_listen(setup.is_gateway, TG_CHANNEL);
    setup_beacons(0, TG_CHANNEL);

    if (setup.is_gateway) {
    	asapi_init_srv();
    }

    xrt_invoke(1);
    
    ///4. Set the app name (PageTest) as a cookie in the User-ID.
    ///   This is used for query filtering
    {   static const ot_u8 appstr[] = "APP=PageTest";
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
    //if (setup.is_gateway) {
        mpipedrv_standby();
        logger_msg(MSG_utf8, 6, 27, (ot_u8*)"SYS_ON", (ot_u8*)"System on and Mpipe active\n");
    //}
#   endif


#	ifdef HSCODE_UNITTEST
    testreg = test_hscode(1);
#	endif

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






/** Patches
  * ---------------------------------------------------------------------------
  */
/*
#include <../io/sx127x/radio_rm2.h>
#include <m2/network.h>
#include <m2/dll.h>


static m2session* sub_xrt_parse(void) {
/// XR Telegram Format
// ========================================================================
/// General Background frame design
/// <PRE>   +---------+--------+-------+-------+---------+--------+
///         | TX EIRP | Subnet | Token | PType | Payload | CRC16  |
///         |   B0    |   B1   | B2:5  | B6    | B7:13   | B14:15 |
///         +---------+--------+-------+-------+---------+--------+
/// </PRE>
// ========================================================================

	ot_u8   tx_eirp;
	ot_u8   subnet;
	ot_u32  token;
	ot_u8   ptype;
	ot_u8   extra;
	ot_u32  lat, lon;

	tx_eirp = q_readbyte(&rxq);
	subnet  = q_readbyte(&rxq);
	token   = q_readlong(&rxq);
	ptype   = q_readbyte(&rxq);
	extra   = q_readbyte(&rxq);

	q_readstring(&rxq, (ot_u8*)&lat, 3);
	q_readstring(&rxq, (ot_u8*)&lon, 3);

	if (setup.is_gateway) {
		page_received = True;
#       if (OT_FEATURE(MPIPE))
		///@todo this block of code could be turned into a logging function
		logger_header(MSG_utf8, 0);
		q_writestring(mpipe.alp.outq, "PRX", 4);
		mpipe.alp.outq->putcursor += sprintf(mpipe.alp.outq->putcursor,
		                                "{\"token\":\"%08X\", \"elat\":%u, \"elon\":%u, \"rssi\":%d, \"link\":%d}\n",
		                                token, lat, lon, radio.last_rssi, radio.last_linkloss );
		logger_direct();
#	    endif
	}

	///@todo check if this NULL return value is OK
	return NULL;
}
*/

/*
#ifdef EXTF_network_parse_bf
m2session* network_parse_bf() {
/// Background Frame parsing: fill-in some stuff and treat it as foreground
// ========================================================================
/// General Background frame design
/// <PRE>   +---------+--------+---------+------+
///         | TX EIRP | Subnet | Payload | CRC8 |
///         |   B0    |   B1   |  B2:4   | B5   |
///         +---------+--------+---------+------+   </PRE>
// ========================================================================
    m2session*  s_next;

    /// Load default attributes
    s_next  = NULL;

    /// Advertising Protocol has subnet =  0xYF, where "Y" is any four bits
    switch (rxq.getcursor[1] & 15) {
    case 15:    s_next = m2advp_parse();        break;
    //case 7:     break;
    //case 3:	         break;
    case 1:     s_next = sub_xrt_parse();       break;
    default:    break;
    }

    return s_next;
}
#endif
*/



