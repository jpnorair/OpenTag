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
  * @file       /otplatform/msp430f5/omg_usbgadget_MSP430F55xx.c
  * @author     JP Norair
  * @version    R104
  * @date       17 Nov 2012
  * @brief      USB Gadget Driver for OMG: GoodFET + MPipe + TTY
  * @defgroup   USB_Gadget (USB Gadget)
  * @ingroup    USB_Gadget
  *
  * The OMG USB Gadget is a tool that combines MPipe (an official part of the
  * OpenTag Library), a TTY interface for terminal access, and GoodFET for 
  * actually programming another MSP430/CC430 connected to this one.  All of 
  * these peripherals are multiplexed as independent CDC interfaces on a single
  * USB device (this is allowed by the USB spec).
  *
  * All of the OTlib functions for MPipe are retained, and in fact they are 
  * used in a somewhat universal context for USB_Gadget.  That is, calling 
  * mpipe_init() will initialize the USB device and prepare all interfaces, so
  * OpenTag code does not really need to change much, if at all, when swapping
  * a standard MPipe module with the USB_Gadget Module.
  *
  * The GoodFET and TTY interfaces are not part of OTlib, nor are they part of
  * the mainline distribution for OpenTag.  They are part of the OMG distro,
  * which includes the OpenTag distro.
  *
  ******************************************************************************
  */

#include "OT_platform.h"

/// The Board Support Header must define OMG_FEATURE_USBGADGET as ENABLED in 
/// order for this code to compile
#if (defined(__MSP430F5__) && OMG_FEATURE(USBGADGET))

// Other OMG_FEATURE() settings that should be ENABLED or DISABLED in the B.S.H.
// MPIPE_BRIDGE     MPIPE is bridged across USB and MPIPE_UART/SPI/I2C
// GOODFET          GoodFET is enabled
// TTY              TTY is enabled


#include "OTAPI.h"

#include <intrinsics.h>
#include "usb_cdc_driver/usb_descriptors.h"
#include "usb_cdc_driver/usb_device.h"
#include "usb_cdc_driver/usb_types.h"
#include "usb_cdc_driver/defMSP430USB.h"
#include "usb_cdc_driver/usb_main.h"
#include "usb_cdc_driver/usb_cdc_backend.h"
#include "usb_cdc_driver/usb_isr.h"







/** Gadget Local Driver Data   <BR>
  * ========================================================================<BR>
  */
  
typedef struct {
    ot_bool     standby;
    ot_uni16    seq;
} mpipe_mac_struct;

typedef struct {
    
} tty_struct;

typedef struct {
    ot_u8 state;
    ot_queue    inq;
    ot_queue    outq;
} goodfet_struct;


mpipe_mac_struct    mpmac;
tty_struct          tty;
goodfet_struct      gf;




/** USB Event Handling   <BR>
  * ========================================================================<BR>
  * Interface ID for MPipe is 0:        
  * Interface ID for TTY is 0 or 1:     0 if MPipe disabled, 1 if Mpipe enabled
  * Interface ID for GoodFET is 0,1,2:  (etc)
  */
#define _MPIPE_INTF     0
#define _TTY_INTF       0
#define _GFET_INTF      0

#if (OT_FEATURE(MPIPE))
#   define MPIPE_ID     _MPIPE_INTF
#   undef _TTY_INTF
#   undef _GFET_INTF
#   define _TTY_INTF    1
#   define _GFET_INTF   1
#endif

#if (OMG_FEATURE(TTY))
#   define TTY_ID       _TTY_INTF
#   undef _GFET_INTF
#   define _GFET_INTF   (_TTY_INTF+1)
#endif

#if (OMG_FEATURE(GOODFET))
#   define GFET_ID      _GFET_INTF
#endif

#define NUM_INTF        (defined(MPIPE_ID) + defined(TTY_ID) + defined(GFET_ID))

#if (NUM_INTF == 0)
#   error "You have not enabled any Gadget interfaces... Eh?"

#elif (NUM_INTF > 1)
    #define GADGET_ISR(INTF)    gadget_isr[INTF]

    static const ot_sub gadget_isr[NUM_INTF] = {
#   ifdef MPIPE_ID
        &mpipedrv_isr,
#   endif
#   ifdef TTY_ID
        &ttydrv_isr,
#   endif
#   ifdef MPIPE_ID
        &gfdrv_isr
#   endif
    };

#elif defined(MPIPE_ID)
#   define GADGET_ISR(INTF)    mpipedrv_isr()
    
#elif defined(TTY_ID)
#   define GADGET_ISR(INTF)    ttydrv_isr()
    
#elif defined(GFET_ID)
#   define GADGET_ISR(INTF)    gfetdrv_isr() 
    
#else
#   error "This should never occur unless the code got mangled."

#endif







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
#ifdef __DEBUG__
    if (intfNum < NUM_INTF) 
#endif
        GADGET_ISR(intfNum);
}
#endif


/** This event indicates that a receive operation on interface intfNum has just 
  * been completed.
  */
#if (1)
void usbcdcevt_rxdone (ot_u8 intfNum){
#ifdef __DEBUG__
    if (intfNum < NUM_INTF) 
#endif
        GADGET_ISR(intfNum);
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




/** GoodFET USB functions  <BR>
  * ========================================================================<BR>
  * GoodFET data is sent over USB as a packet based serial protocol, so it is
  * treated in a similar way to MPipe.  The packet protocol is quite similar
  * to that used by the MSP430 BSL.
  *
  * GoodFET is purely half-duplex, and it operates in a stateful manner on a
  * single queue for both input and output.  GoodFET requests come-in on a USB
  * Bulk Output Endpoint and responses are sent on the corresponding Bulk Input
  * Endpoint (USB directions are from the Host perspective).  GoodFET will 
  * never load responses without requests, and each request must uniquely have
  * one response.  Because of these restrictions, we can implement it in a much
  * simpler way than MPipe, or ALP in general.
  *
  * GoodFET additionally provides a Debugging (or warning) message channel. 
  * We implement this over MPipe.
  *
  * <PRE>
  * +-------+-------+-------+-------+---------------+
  * |   0   |   1   |   2   |   3   |   4 ... 516   |
  * +-------+-------+-------+-------+---------------+
  * |  App  | Verb  | L.Endian Len  |  Packet Data  |
  * +-------+-------+-------+-------+---------------+
  * </PRE>
  * 
  */

ot_u8 gfdrv_init() {
    return 255;
}


void gfdrv_tx() {
    if (gf.state == 0) {
        usbcdc_txdata( &gf.buf.ubyte[0], gf.buf.ushort[1]+4,  GF_CDC );
    }
}


void gfdrv_rx() {
    if (gf.state == 0) {
        gf.buf.ushort[0] = 0;
        gf.buf.ushort[1] = 0;
        usbcdc_rxdata(&gf.buf.ubyte[0], 4, GF_CDC);
    }
}


void gfdrv_isr() {
    switch (gf.state) {
        // Idle Listening for Header
        case 0: {
            ot_u16 gf_len;
            
            /// Normal GoodFET has a reset command.  We don't use that here, 
            /// because there are other ways to reset the OMG.
//            if (gf.inq.front[0] == GF_RESET (0x80)) {
//                if (++gf.reset_count > 4) {
//                    sys_halt(GF_RESET_CODE);
//                    // code will never reach this line
//                }
//                gf_abort_rx();
//                gf_restart_rx();
//                break;
//            }
//            gf.reset_count = 0;     // Non-Reset Header received: 

            // Find length.  If it's too long for the implemented buffer, 
            // abort rx and log an error (goes over MPipe or TTY)
            gf_len = gf.buf.ushort[1];  //grab 16bit LE length

            if (gf_len > (GF_BUF_ALLOC-4)) {
                gf_abort_rx();
                GF_DEBUG_MSG("GF_ERR Buffer length exceeded");
                break;
            }
            
            // Length is OK, and no errors, so get the rest of the packet
            // Only send the Detect signal and break if the entire packet is 
            // NOT in the buffer now.
            gf.state = GF_RxPayload;
            if (usbcdc_rxdata(&gf.buf.ubyte[4], gf_len, GF_CDC) != 0) {
                gfevt_rxdetect(20);
                break;
            }
        }
        
        // The reception is completely done.
        case 1: gfevt_rxdone(0);
                gf.state = GF_Idle;     //do when packet processed
                break;
        
        // The transmission is completely done.
        case 2: gfevt_txdone(0);
                break;
    }
}












/** TTY  <BR>
  * ========================================================================<BR>
  * TTY is a character-based transfer method with basically no packetization or
  * MAC to speak of.  The TTY interface must maintain independent queues 
  * because the user interfaces with it as a full-duplex system, even though in 
  * implementation it is half-duplex (USB is not full duplex, nor is a 
  * single-core computer).
  *
  * TTY is not elegantly implementable via USB, because in most situations the 
  * effective "packet size" is just 1 byte.  However, arbitrary sizes need to
  * be accomodated.  The TTY driver, therefore, is always listening for input.
  * if it gets a single character (nobody can type so fast) it is assumed to be
  * a character input.  If it gets multiple characters starting with an ESC
  * character, it assumes there is a "chord" input (such as arrow keys).  If it
  * gets multiple characters not starting with ESC, it treats it as bulk input.
  * 
  * 
  * There are several "escape" characters that should send
  * singals to the controlling task when received:
  *
  * <PRE>
  * Char   Val  Key     Nomenclature                    Signal Mapping
  *   \0    0   ^@      Null (NUL)
  *  N/A    1   ^A      Start of heading (SOH)          unused
  *  N/A    2   ^B      Start of text (STX)             unused?
  *  N/A    3   ^C      End of text (ETX)               ttyevt_sigkill
  *  N/A    4   ^D      End of transmission (EOT)       [use to place EOF?]
  *  N/A    5   ^E      Enquiry (ENQ)                   unused?
  *  N/A    6   ^F      Acknowledge (ACK)               unused?
  *   \b    7   ^G      Bell (BEL)                      ttyevt_bell
  *  N/A    8   ^H      Backspace (BS)                  [managed in driver]
  *   \t    9   tab     Horizontal Tab (TAB)            ttyevt_tab
  *   \n   10   ent.    New Line Feed (LF)              ttyevt_enter
  *  N/A   11   ^K      Vertical Tab (VT)               unused?
  *  N/A   12   ^L      Form Feed/New Page (FF)         unused?
  *   \r   13   ret.    Carriage Return (CR)            [ignored by driver]
  *  N/A   14   ^N      Shift Out (SO)                   
  *  N/A   15   ^O      Shift In (SI)
  *  N/A   16   ^P      Data Link Escape (DLE)
  *  N/A   17   ^Q      Device Control 1 (DC1)          [restart output?]
  *  N/A   18   ^R      Device Control 2 (DC2)
  *  N/A   19   ^S      Device Control 3 (DC3)          [stop output?]
  *  N/A   20   ^T      Device Control 4 (DC4)
  *  N/A   21   ^U      Negative ACK (NAK)
  *  N/A   22   ^V      Synchronous Idle (SYN)          ignored (no control)
  *  N/A   23   ^W      End of TX Block (ETB)           
  *  N/A   24   ^X      Cancel (CAN)
  *  N/A   25   ^Y      End of Medium (EM)
  *  N/A   26   ^Z      Substitute (SUB)                ttyevt_sigkill
  *  N/A   27   ^[      Escape (ESC)
  *  N/A   28   ^]
  *  N/A   29   ^
  *  N/A   30  
  *  N/A   31  
  *
  * </PRE>
  * 
  * Multicharacter inputs:
  * Most importantly, the arrow keys give multicharacter inputs.  If the TTY
  * driver receives any of these, it will create the associated signal.
  * ESC + [ + A
  * ESC + [ + B
  * ESC + [ + C
  * ESC + [ + D
      
  *
  * 
  
  * TX is easy enough -- we know how much data to send, and we send it.
  * 
  * RX must occur on boundaries of 1 buffer-size, so if the host has only one
  * byte to send, it will pad the rest of the buffer.  We use a 64 byte double 
  * buffering mechanism, which is overkill for byte-by-byte TTY but it does
  * allow better implementation of things like uucp.
  *
  * @note We could use a low-speed device, which requires an 8 byte nom buffer.
  * @todo Find if it's possible to use such a low-speed device on this HW.
  */

ot_u8 gfdrv_init() {
    return 255;
}


void gfdrv_tx() {
    if (gf.state == 0) {
        usbcdc_txdata( &gf.buf.ubyte[0], gf.buf.ushort[1]+4,  GF_CDC );
    }
}


void gfdrv_rx() {
    if (gf.state == 0) {
        gf.buf.ushort[0] = 0;
        gf.buf.ushort[1] = 0;
        usbcdc_rxdata(&gf.buf.ubyte[0], 4, GF_CDC);
    }
}


void gfdrv_isr() {
    switch (gf.state) {
        // Idle Listening for Header
        case 0: {
            ot_u16 gf_len;
            
            /// Normal GoodFET has a reset command.  We don't use that here, 
            /// because there are other ways to reset the OMG.
//            if (gf.inq.front[0] == GF_RESET (0x80)) {
//                if (++gf.reset_count > 4) {
//                    sys_halt(GF_RESET_CODE);
//                    // code will never reach this line
//                }
//                gf_abort_rx();
//                gf_restart_rx();
//                break;
//            }
//            gf.reset_count = 0;     // Non-Reset Header received: 

            // Find length.  If it's too long for the implemented buffer, 
            // abort rx and log an error (goes over MPipe or TTY)
            gf_len = gf.buf.ushort[1];  //grab 16bit LE length

            if (gf_len > (GF_BUF_ALLOC-4)) {
                gf_abort_rx();
                GF_DEBUG_MSG("GF_ERR Buffer length exceeded");
                break;
            }
            
            // Length is OK, and no errors, so get the rest of the packet
            // Only send the Detect signal and break if the entire packet is 
            // NOT in the buffer now.
            gf.state = GF_RxPayload;
            if (usbcdc_rxdata(&gf.buf.ubyte[4], gf_len, GF_CDC) != 0) {
                gfevt_rxdetect(20);
                break;
            }
        }
        
        // The reception is completely done.
        case 1: gfevt_rxdone(0);
                gf.state = GF_Idle;     //do when packet processed
                break;
        
        // The transmission is completely done.
        case 2: gfevt_txdone(0);
                break;
    }
}







/** Mpipe Main Public Functions  <BR>
  * ========================================================================<BR>
  * MPipe uses a packet-based MAC and a subset of the NDEF binary protocol.
  * It uses a 6-byte NDEF header and a 4-byte MAC footer (counter and CRC).
  * For USB systems, we can technically elide the CRC, as USB provides this for
  * us via its block data transfer specification.
  */
  
// Footer is 2 byte sequence ID.  CRC16 is elided.
#define MPIPE_FOOTERBYTES   2


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
    mpmac.seq.ushort        = 0;                //not actually necessary
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
    
        q_writeshort(mpipe.alp.outq, mpmac.seq.ushort);                // Sequence Number

        //scratch = mpipe.alp.outq->putcursor - mpipe.alp.outq->getcursor;    //data length
        //scratch = platform_crc_block(mpipe.alp.outq->getcursor, scratch);   //CRC value
        //q_writeshort(mpipe.alp.outq, scratch);                              //Put CRC
        
        scratch                     = mpipe.alp.outq->putcursor \
                                    - mpipe.alp.outq->getcursor;            //data length w/ Footer
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
            mpipe.alp.inq->length   = payload_len + 6;

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
            mpmac.seq.ubyte[UPPER]  = *footer++;
            mpmac.seq.ubyte[LOWER]  = *footer;

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
        /// page (<600 bits) @ 12 Mbps = 50 us = 0.05tick, but technically this
        /// function (and state) should only run once the final data is safely
        /// outbound, so 0 can be used in the event delay.
        case MPIPE_Tx_Done: 
            mpmac.seq.ushort++;
            //mpipe.state = MPIPE_Idle;
            //mpipedrv_kill();
            mpipeevt_txdone(0);
            break;
    }
}
#endif



#endif

