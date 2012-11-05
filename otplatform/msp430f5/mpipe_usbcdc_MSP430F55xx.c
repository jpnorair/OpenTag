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
  * @version    R102
  * @date       1 Nov 2012
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
#if ((OT_FEATURE(MPIPE) == ENABLED) && defined(MPIPE_USB))

#include "OTAPI.h"


#include "usb_cdc_driver/usb_descriptors.h"
#include <intrinsics.h>
//#include <string.h>

#include "usb_cdc_driver/usb_device.h"
#include "usb_cdc_driver/usb_types.h"
#include "usb_cdc_driver/defMSP430USB.h"
#include "usb_cdc_driver/usb_main.h"
//#include "F5xx_F6xx_Core_Lib/HAL_UCS.h"
#include "usb_cdc_driver/usb_cdc_backend.h"
#include "usb_cdc_driver/usb_isr.h"







/** Mpipe Driver Data   <BR>
  * ========================================================================<BR>
  */
  
// Footer is 2 byte sequence ID + CRC (usually 2 bytes, but could be more)
#define MPIPE_FOOTERBYTES   4

typedef struct {
    ot_uni16 seq;
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
void USB_handleClockEvent () {
}



/** If this function gets executed, it indicates that a valid voltage has just 
  * been applied to the VBUS pin.  It returns True in order to wake the CPU,
  * which is likely asleep while waiting for the USB cable to be attached.
  */
void USB_handleVbusOnEvent () {
/// The standard code turns-on the USB subsystem and generates a rising edge
/// on DP (D+) via reset-connect, which causes the host to enumerate this 
/// device as a 12 Mbps USB Full Speed Device.
    if (USB_enable() == kUSB_succeed){
        USB_reset();
        USB_connect();
    }
    if (mpipe.state == MPIPE_Null) {
        sys_resume();
    }
}



/** If this function gets executed, it indicates that a valid voltage has just 
  * been removed from the VBUS pin.
  */
void USB_handleVbusOffEvent (){
/// Call sys_halt() first and mpipe_disconnect() last, because this guarantees 
/// that whatever happens in sys_halt() does not change the fact that we are 
/// certain MPipe must be disconnected.  sys_halt() is not a sure-thing: some 
/// devices might have batteries, and the non-MPipe part of the application 
/// might not want or need to halt.
	//if (mpipe.state >= MPIPE_Idle) {
        sys_halt(HALT_nopower);
        mpipe_disconnect(NULL);
	//}
}



/** If this function gets executed, it indicates that the USB host has issued a 
  * USB reset event to the device.  A USB reset is not a chip reset, it just
  * happens at the beginning of the enumeration process.  This ISR is generally
  * useless, and unless you are a USB master with some kind of clever idea, 
  * just comment-it-out in usb_descriptors.h
  */
void USB_handleResetEvent () {
}



/** If this function gets executed, it indicates that the USB host has chosen 
  * to suspend this device after a period of active operation.
  *
  * USB Suspend mode (sleep, basically) stipulates < 500 µA loading from the 
  * host, so there is not much power to run OpenTag (maybe you are using a
  * battery in which case you can keep going!).  The decision is up to your 
  * application code.
  */
void USB_handleSuspendEvent () {
/// Call sys_halt() first and mpipe_close() & (mpipe.state = MPIPE_Null) last, 
/// because this guarantees that whatever happens in sys_halt() does not change 
/// the fact that we are certain MPipe must be suspended.  sys_halt() is not a 
/// sure-thing: some devices might have batteries, and the non-MPipe part of 
/// the application might not want or need to halt.
#if (MPIPE_USB_REMWAKE)
    if (mpipe.state >= MPIPE_Idle) {
        sys_halt(HALT_lowpower);
        mpipe_close();
        mpipe.state = MPIPE_Null;
    }
#else
    mpipe_close();
    mpipe.state = MPIPE_Null;
#endif
}



/** If this function gets executed, it indicates that the USB host has chosen 
  * to resume this device after a period of suspended operation.  returns True 
  * to bring CPU out of sleep after USB interrupt.
  *
  * For OpenTag, this routine calls sys_refresh() and platform_ot_preempt(), 
  * which gets everything back on-line, and in startup configuration.
  */
void USB_handleResumeEvent () {
#if (MPIPE_USB_REMWAKE)
	if (mpipe.state == MPIPE_Null) {
		mpipe.state = MPIPE_Idle;
		mpipe_open();
		sys_resume();
	}
#else
	mpipe.state = MPIPE_Idle;
#endif
}



/** If this function gets executed, it indicates that the USB host has 
  * enumerated this device.  platform_wakeup() is called in case there is some
  * blocking process that's sleeping, waiting for enumeration (typical).
  */
ot_u8 USB_handleEnumCompleteEvent () {
#if (MPIPE_USB_MANUAL_STANDBY == 0)
    mpipe.state = MPIPE_Idle;
    mpipe_open();
    return True;
#else
    return False;
#endif
}



/** This event indicates that data has been received for interface intfNum, but 
  * no data receive operation is underway.  This is typically unused.
  */
void USBCDC_handleDataReceived (ot_u8 intfNum) {
    //return True;
}



/** This event indicates that a send operation on interface intfNum has just 
  * been completed.  returns True to keep CPU awake
  */
void USBCDC_handleSendCompleted (ot_u8 intfNum) {
    mpipedrv_isr();
    //return True;
}



/** This event indicates that a receive operation on interface intfNum has just 
  * been completed.
  */
void USBCDC_handleReceiveCompleted (ot_u8 intfNum){
    mpipedrv_isr();
    //return True;
}



/** This event indicates that new line coding params have been received from 
  * the host.  Line coding means baud rate, considering the TTY baud rate that
  * is being simulated by the USB CDC.
  */
ot_u8 USBCDC_handleSetLineCoding (ot_u8 intfNum, ot_u32 lBaudrate) {
#if (MPIPE_USB_MANUAL_STANDBY == 0)
    return False;

#else
#   if (CDC_NUM_INTERFACES > 1)
    if (intfNum != 0) return False;
#   endif

    mpipe.state = MPIPE_Idle;
    mpipe_open();
    return True;
#endif
}



/** This event indicates that new line control state has been received from 
  * the host.  Line control is very similar to basic connection.  It has to do
  * with the pin voltage arrangement on connection.  It is not terribly useful.
  */
void USBCDC_handleSetControlLineState (ot_u8 intfNum, ot_u8 lineState) {
	//return False;
}









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

    tty.seq.ushort          = 0;                //not actually necessary
    mpipe.state             = (MPIPE_Null-1);	    // Disconnected

	alp_init(&mpipe.alp, &dir_in, &dir_out);
	mpipe.alp.inq->back    -= 10;
    mpipe.alp.outq->back   -= 10;
    
    USB_init();
    //USB_disconnect();	//disconnect USB first

    //See if we're already attached physically to USB, and if so, connect to it
    //Normally applications don't invoke the event handlers, but this is an exception.
    if (USBPWRCTL & USBBGVBV){
        USB_handleVbusOnEvent();
    }

    return 255;
}
#endif


#ifndef EXTF_mpipedrv_standby
void mpipedrv_standby() {
/// Hold in LPM0 until startup complete.
	mpipe.state = (MPIPE_Null-1);
	__bis_SR_register(0x18);
	__no_operation();
}
#endif


#ifndef EXTF_mpipedrv_detach
void mpipedrv_detach(void* port_id) {
    mpipe.state = MPIPE_Null;
    USB_disconnect();
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

        scratch = mpipe.alp.outq->putcursor - mpipe.alp.outq->getcursor;    //data length
        scratch = platform_crc_block(mpipe.alp.outq->getcursor, scratch);   //CRC value
        q_writeshort(mpipe.alp.outq, scratch);                              //Put CRC
        
        scratch                     = mpipe.alp.outq->putcursor \
                                    - mpipe.alp.outq->getcursor;            //data length w/ CRC
        data                        = mpipe.alp.outq->getcursor;            //data start
        mpipe.alp.outq->getcursor   = mpipe.alp.outq->putcursor;            //move queue past packet
        
        USBCDC_sendData(data, scratch, CDC0_INTFNUM);

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
        USBCDC_receiveData(mpipe.alp.inq->front, 6, CDC0_INTFNUM);
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
            mpipe.alp.inq->length   = payload_len + 6;
            
            USBCDC_receiveData(payload_front, payload_len, CDC0_INTFNUM);
            mpipeevt_rxdetect(20);      // USB is fast: 20 ticks way more than enough
        }   break;
        
        /// The reception is completely done.  Make state Idle, set sequence
        /// number to match incoming sequence, compute CRC (Good==0, Bad!=0),
        /// and provide the MPipe Task Event Callback with the CRC result.
        /// USB does CRC internally, though, so don't bother.
        case MPIPE_RxPayload: {
            ot_u8* footer;
            //ot_u16 crc_result;
            mpipe.state             = MPIPE_Idle;
            footer                  = mpipe.alp.inq->back;
            tty.seq.ubyte[UPPER]    = *footer++;
            tty.seq.ubyte[LOWER]    = *footer;

            //crc_result = platform_crc_block(mpipe.alp.inq->front, mpipe.alp.inq->length);
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
        /// page (<600 bits) @ 12 Mbps = 50 us = 0.05tick, but that rounds up 
        /// to 1 tick.
        ///
        /// @note You could experiment with 0 tick delay, given than the kernel
        /// pre-emption and scheduler runtime probably takes more than 50us
        /// on an MSP430F5 @ 24 MHz.
        case MPIPE_Tx_Done: 
            tty.seq.ushort++;
            mpipe.state = MPIPE_Idle;
            mpipeevt_txdone(1);
            break;
    }
}
#endif



#endif

