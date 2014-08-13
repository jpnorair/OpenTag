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
  * @file       /otplatform/msp430f5/mpipe_usbcdc_MSP430F55xx.c
  * @author     JP Norair
  * @version    R104
  * @date       17 Nov 2012
  * @brief      Message Pipe (MPIPE) USB Virtual COM implementation for MSP430F55xx
  * @defgroup   MPipe (Message Pipe)
  * @ingroup    MPipe
  *
  * Implemented Mpipe Protocol: The Mpipe protocol is a simple wrapper to NDEF.
  * <PRE>
  * Legend: [ NDEF Header ] [ NDEF Payload ] [ Seq. Number ] [ CRC16 ]
  * Bytes:        6             <= 255             2             2 
  * </PRE>
  ******************************************************************************
  */

#include "OT_platform.h"

/// Do not compile if MPIPE is disabled, or MPIPE does not use USB CDC
#if (   defined(__MSP430F5__) \
     && OT_FEATURE(MPIPE) \
     && defined(MPIPE_USB) \
     && (OMG_FEATURE(USBGADGET) != ENABLED) )

#include "OTAPI.h"


#include "usb_cdc_driver/usb_descriptors.h"
#include <intrinsics.h>
#include "usb_cdc_driver/usb_device.h"
#include "usb_cdc_driver/usb_types.h"
#include "usb_cdc_driver/defMSP430USB.h"
#include "usb_cdc_driver/usb_main.h"
#include "usb_cdc_driver/usb_cdc_backend.h"
#include "usb_cdc_driver/usb_isr.h"







/** Mpipe Driver Data   <BR>
  * ========================================================================<BR>
  */
  
// Footer is 2 byte sequence ID + CRC (usually 2 bytes, but could be more)
///@todo reduce to 2, and make sure all OTlib code has no "4" magic numbers
#define MPIPE_FOOTERBYTES   4

typedef struct {
    ot_bool     standby;
    ot_uni16    seq;
} tty_struct;

tty_struct tty;






/** USB Event Handling   <BR>
  * ========================================================================<BR>
  * These functions and data (below) originally come from the TI USB library
  * C file called usbEventHandling.c.  They have been streamlined into this
  * monolithic file in order to allow better potential for future optimization.
  */

//These variables are only example, they are not needed for stack
//extern volatile ot_u8 bCDCDataReceived_event;    //data received event


/** If this function gets executed, it's a sign that the output of the USB PLL 
  * has failed.
  */
#if (1)
void usbevt_pllerror () {
}
#endif


/** If this function gets executed, it indicates that a valid voltage has just 
  * been applied to the VBUS pin.  It returns True in order to wake the CPU,
  * which is likely asleep while waiting for the USB cable to be attached.
  */
#if (1)
void usbevt_vbuson () {
/// The standard code turns-on the USB subsystem and generates a rising edge
/// on DP (D+) via reset-connect, which causes the host to enumerate this 
/// device as a 12 Mbps USB Full Speed Device.
    if (usb_enable() == kUSB_succeed){
        usb_reset();
        usb_connect();
    }
    if (platform_ext.usb_wakeup != False) {
        sys_resume();
    }
}
#endif



/** If this function gets executed, it indicates that a valid voltage has just 
  * been removed from the VBUS pin.
  */
#if (1)
void usbevt_vbusoff (){
/// Call sys_halt() first and mpipe_disconnect() last, because this guarantees 
/// that whatever happens in sys_halt() does not change the fact that we are 
/// certain MPipe must be disconnected.  sys_halt() is not a sure-thing: some 
/// devices might have batteries, and the non-MPipe part of the application 
/// might not want or need to halt.
    sys_halt(HALT_nopower);     //should set platform_ext.usb_wakeup ... or not
    mpipe_disconnect(NULL);
}
#endif


/** If this function gets executed, it indicates that the USB host has issued a 
  * USB reset event to the device.  A USB reset is not a chip reset, it just
  * happens at the beginning of the enumeration process.  This ISR is generally
  * useless, and unless you are a USB master with some kind of clever idea, 
  * just comment-it-out in usb_descriptors.h
  */
#if (1)
void usbevt_reset () {
}
#endif


/** If this function gets executed, it indicates that the USB host has chosen 
  * to suspend this device after a period of active operation.
  *
  * USB Suspend mode (sleep, basically) stipulates < 500 µA loading from the 
  * host, so there is not much power to run OpenTag (maybe you are using a
  * battery in which case you can keep going!).  The decision is up to your 
  * application code.
  */
#if (1)
void usbevt_suspend () {
/// Call sys_halt() first and mpipe_close() & (mpipe.state = MPIPE_Null) last, 
/// because this guarantees that whatever happens in sys_halt() does not change 
/// the fact that we are certain MPipe must be suspended.  sys_halt() is not a 
/// sure-thing: some devices might have batteries, and the non-MPipe part of 
/// the application might not want or need to halt.
#   if (MPIPE_USB_REMWAKE)
    if (mpipe.state >= MPIPE_Idle) {
        mpipe_close();
        mpipe.state = MPIPE_Null;
        sys_halt(HALT_lowpower);        //should set platform_ext.usb_wakeup ... or not
    }
#   else
    mpipe_close();
    mpipe.state = MPIPE_Null;
#   endif
}
#endif


/** If this function gets executed, it indicates that the USB host has chosen 
  * to resume this device after a period of suspended operation.  returns True 
  * to bring CPU out of sleep after USB interrupt.
  *
  * For OpenTag, this routine calls sys_refresh() and platform_ot_preempt(), 
  * which gets everything back on-line, and in startup configuration.
  */
#if (1)
void usbevt_resume () {
#   if (MPIPE_USB_REMWAKE)
    mpipe.state = MPIPE_Idle;
	mpipeevt_rxinit(0);
	if (platform_ext.usb_wakeup != False) {
		sys_resume();
	}
#   endif
}
#endif


/** If this function gets executed, it indicates that the USB host has set an
  * address for this device -- BUT IT HAS NOT BEEN FULLY CONFIGURED.  Comms
  * cannot actually take place until configuration is complete, and that event
  * is detected by usbcdcevt_set_ctlline() or usbcdcevt_set_linecoding().
  */
#if (1)
void usbevt_enumerate () {
}
#endif


/** This event indicates that data has been received for interface intfNum, but 
  * no data receive operation is underway.  This is typically unused.
  */
#if (1)
void usbcdcevt_rxdetect (ot_u8 intfNum) {
    //return True;
}
#endif


/** This event indicates that a send operation on interface intfNum has just 
  * been completed.  returns True to keep CPU awake
  */
#if (1)
void usbcdcevt_txdone (ot_u8 intfNum) {
    mpipedrv_isr();
    //return True;
}
#endif


/** This event indicates that a receive operation on interface intfNum has just 
  * been completed.
  */
#if (1)
void usbcdcevt_rxdone (ot_u8 intfNum){
    mpipedrv_isr();
    //return True;
}
#endif



/** This event indicates that new line coding params have been received from 
  * the host.  Line coding means baud rate, considering the TTY baud rate that
  * is being simulated by the USB CDC.
  */
#if (1)
void usbcdcevt_set_linecoding (ot_u8 intfNum, ot_u32 lBaudrate) {
}
#endif


/** This event indicates that new line control state has been received from
  * the host.  Line control is very similar to basic connection.  This function
  * is somewhat more informative than usbcdcevt_set_linecoding() for signaling
  * the start of a connection.
  *
  * The parameter "lineState" can have values 0,1,2,3.  It is a 2bit bitmap.
  * Bit0 is DTR: the host is telling the device it is ready to communicate
  * Bit1 is RTS: the host is telling the device it has data to send
  */
#if (1)
void usbcdcevt_set_ctlline (ot_u8 intfNum, ot_u8 lineState) {

#   if (CDC_NUM_INTERFACES > 1)
    if (intfNum != 0) return;
#   endif

    // Only do MPipe initialization/wakeup if it is currently uninitialized
    if (mpipe.state == MPIPE_Null) {
        mpipe.state = MPIPE_Idle;
        platform_ext.usb_wakeup = True;
    }

#   if 1 //(MPIPE_MANUAL_STANDBY == ENABLED)
#   else
    mpipeevt_rxinit(10);
#   endif
}
#endif








/** Mpipe Main Public Functions  <BR>
  * ========================================================================
  */
  
#ifndef EXTF_mpipedrv_footerbytes
ot_u8 mpipedrv_footerbytes() {
    return MPIPE_FOOTERBYTES;
}
#endif



#ifndef EXTF_mpipedrv_init
ot_int mpipedrv_init(void* port_id) {
/// <LI> "port_id" is unused in this impl, and it may be NULL                   </LI>
/// <LI> Prepare the HW, which in this case is a USB CDC (i.e. Virtual COM)     </LI>
/// <LI> USB is disconnected at startup in order to clear the host USB driver,
///        which will get stuck during debugging or other sleeping processes.   </LI>

    platform_ext.usb_wakeup = False;
    tty.seq.ushort          = 0;                //not actually necessary
    mpipe.state             = MPIPE_Null;	    // Disconnected

	alp_init(&mpipe.alp, &otmpin, &otmpout);
	mpipe.alp.inq->back    -= 10;
    mpipe.alp.outq->back   -= 10;
    
    usb_init();
    //usb_disconnect();	//disconnect USB first

    //See if we're already attached physically to USB, and if so, connect to it
    //Normally applications don't invoke the event handlers, but this is an exception.
    if (USBPWRCTL & USBBGVBV){
        usbevt_vbuson();
    }

    return 255;
}
#endif


#ifndef EXTF_mpipedrv_standby
void mpipedrv_standby() {
/// Hold in LPM0 until startup complete.
    __bis_SR_register(0x18);
    __no_operation();
}
#endif


#ifndef EXTF_mpipedrv_detach
void mpipedrv_detach(void* port_id) {
    mpipe.state = MPIPE_Null;
    usb_disconnect();
}
#endif


#ifndef EXTF_mpipedrv_block
void mpipedrv_block() {
    mpipe.state = MPIPE_Null;
}
#endif


#ifndef EXTF_mpipedrv_unblock
void mpipedrv_unblock() {
    mpipe.state = MPIPE_Idle;
}
#endif


#ifndef EXTF_mpipedrv_kill
void mpipedrv_kill() {
    mpipe.state = MPIPE_Idle;
	q_empty(mpipe.alp.outq);
	mpipe.alp.outq->back -= 10;
}
#endif



#ifndef EXTF_mpipedrv_wait
void mpipedrv_wait() {
    while (mpipe.state != MPIPE_Idle);
}
#endif



#ifndef EXTF_mpipedrv_setspeed
void mpipedrv_setspeed(mpipe_speed speed) {
    //Determined by host
}
#endif



#ifndef EXTF_mpipedrv_txndef
void mpipedrv_txndef(ot_bool blocking, mpipe_priority data_priority) {
/// @note Using blocking: OpenTag currently does not implement blocking TX,
///       because it can interfere with time-critical radio processes.  You can
///       achieve a similar affect by calling "mpipedrv_wait()" after a logging
///       function call, if you need blocking on certain transmissions.

    if (mpipe.state == MPIPE_Idle) {
        ot_u8* data;
    	ot_u16 scratch;
        mpipe.state = MPIPE_Tx_Done;
    
        q_writeshort(mpipe.alp.outq, tty.seq.ushort);                // Sequence Number

        ///@todo remove CRC part, not necessary for USB.  
        /// Requires coordination with OTcom, though: need to add a checkbox
        scratch = mpipe.alp.outq->putcursor - mpipe.alp.outq->getcursor;    //data length
        scratch = platform_crc_block(mpipe.alp.outq->getcursor, scratch);   //CRC value
        q_writeshort(mpipe.alp.outq, scratch);                              //Put CRC
        
        scratch                     = mpipe.alp.outq->putcursor \
                                    - mpipe.alp.outq->getcursor;            //data length w/ CRC
        data                        = mpipe.alp.outq->getcursor;            //data start
        mpipe.alp.outq->getcursor   = mpipe.alp.outq->putcursor;            //move queue past packet
        
        usbcdc_txdata(data, scratch, CDC0_INTFNUM);

        // Wait for the USB transmission to complete (optional).
        if (blocking) {
        	mpipedrv_wait();
        }
    }
}
#endif



#ifndef EXTF_mpipedrv_rxndef
void mpipedrv_rxndef(ot_bool blocking, mpipe_priority data_priority) {
/// @note Using blocking: OpenTag currently does not implement blocking for RX.
///       However, RX typically is called automatically after TX, so the system
///       goes into RX (listening) whenever it is not in TX.
	if (blocking) {
    	mpipedrv_wait();
    }
    if (mpipe.state == MPIPE_Idle) {
        q_empty(mpipe.alp.inq);
        usbcdc_rxdata(mpipe.alp.inq->front, 6, CDC0_INTFNUM);
    }
}
#endif



#ifndef EXTF_mpipedrv_isr
void mpipedrv_isr() {
    switch (mpipe.state) {
        case MPIPE_Idle: //note, case doesn't break!
            
        case MPIPE_RxHeader: {
            ot_u8* payload_front;
            ot_int payload_len;
            mpipe.state             = MPIPE_RxPayload;
            payload_len             = mpipe.alp.inq->front[2];
            payload_front           = mpipe.alp.inq->front + 6;
            mpipe.alp.inq->back     = payload_front + payload_len;
            payload_len            += MPIPE_FOOTERBYTES;
         //#mpipe.alp.inq->length   = payload_len + 6;

            // if the packet is actually done, go to done state (don't break)
            if (usbcdc_rxdata(payload_front, payload_len, CDC0_INTFNUM) != 0) {
                mpipeevt_rxdetect(20);
                break;
            }
        }
        
        /// The reception is completely done.  Make state Idle, set sequence
        /// number to match incoming sequence, compute CRC (Good==0, Bad!=0),
        /// and provide the MPipe Task Event Callback with the CRC result.
        /// USB does CRC internally, though, so don't bother.
        case MPIPE_RxPayload: {
            ot_u8* footer;
            //ot_u16 crc_result;
            footer                  = mpipe.alp.inq->back;
            tty.seq.ubyte[UPPER]    = *footer++;
            tty.seq.ubyte[LOWER]    = *footer;

            //crc_result = platform_crc_block(mpipe.alp.inq->front, q_length(mpipe.alp.inq));
            //mpipeevt_rxdone((ot_int)crc_result);
            mpipeevt_rxdone(0);
        }   break;
        
//      case MPIPE_Tx_Wait:
        /// @note Current TI USB lib implementation generates a "complete"
        /// event when the last page has been loaded, not when the packet
        /// is actually complete.  I prefer the latter function, and I will
        /// change the lib when I get the chance.
        
        /// Transmission is done.  Increment sequence ID, make state Idle, and
        /// callback the MPipe task event.  The Task delay is derived from one
        /// page (<600 bits) @ 12 Mbps = 50 us = 0.05tick, but technically this
        /// function (and state) should only run once the final data is safely
        /// outbound, so 0 can be used in the event delay.
        case MPIPE_Tx_Done: 
            tty.seq.ushort++;
            //mpipe.state = MPIPE_Idle;
            //mpipedrv_kill();
            mpipeevt_txdone(0);
            break;
    }
}
#endif



#endif

