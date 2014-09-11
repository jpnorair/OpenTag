/*  Copyright (c) 2010, Texas Instruments Incorporated
  * All rights reserved.
  * 
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions are met:
  *  * Redistributions of source code must retain the above copyright notice,
  *    this list of conditions, and the following disclaimer.
  *  * Redistributions in binary form must reproduce the above copyright 
  *    notice, this list of conditions and the following disclaimer in the
  *    documentation and/or other materials provided with the distribution.
  *  * Neither the name of the organization, Texas Instruments, nor the names 
  *    of its contributors may be used to endorse or promote products derived 
  *    from this software without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  * ARE DISCLAIMED. IN NO EVENT SHALL TEXAS INSTRUMENTS BE LIABLE FOR ANY 
  * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
  * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  */
/**
  * @file       /otplatform/msp430f5/usb_cdc_driver/usb_main.c
  * @author     JP Norair
  * @version    R100
  * @date       1 Nov 2012
  * @brief      Main driver/MAC for USB  implementation
  * @ingroup    MSP430F5 USB CDC
  *
  * This module is a derivative work of TI's USB library file, usb.c.  JP has
  * optimized it to compile into less code by streamlining data alignment and 
  * other such tricks.  In addition, functionality supporting composite devices
  * is no longer supported.  The module is especially optimized to be a CDC
  * endpoint with 1 control endpoint, 1 bulk-in, and 1 bulk-out.
  * 
  * <PRE>
  * The revision history from the original TI codebase is shown below.
  * Source: usb.c, File Version 1.02 2010/06/17
  * Author: RSTO
  *
  * WHO         WHEN        WHAT                                          
  * ---         ----------  --------------------------------------------- 
  * RSTO        2008/09/03  born                                          
  * RSTO        2008/12/23  enhancements of CDC API                       
  * RSTO        2009/01/12  enhancements for USB serial number            
  * RSTO        2009/05/15  added USB_connectionState()                   
  * RSTO        2009/07/17  added __data16 qualifier for USB buffers      
  * RSTO        2009/08/04  workaround for PLL start up problem           
  * MSP,Biju    2009/10/20  Changes for composite support                 
  * RSTO        2009/10/21  updated USB_InitSerialStringDescriptor()      
  * RSTO        2009/11/05  updated USB_connectionState()                 
  * MSP,Biju    2010/07/15  Updated for MSC
  * ---         ----------  --------------------------------------------
  * JPN         2012/05/01  Integrated with OpenTag 0.3
  * JPN         2012/11/01  Integrated with OpenTag 0.4
  * </PRE>                           
  *****************************************************************************/

#include <otsys/config.h>
#include <otsys/veelite.h>    //for getting device id

#include "msp430f5_lib.h"

#include "usb_cdc_driver/usb_device.h"
#include "usb_cdc_driver/usb_types.h"    //Basic Type declarations
#include "usb_cdc_driver/defMSP430USB.h"
#include "usb_cdc_driver/usb_main.h"      //USB-specific Data Structures
#include "usb_cdc_driver/usb_cdc_backend.h"
#include "usb_cdc_driver/usb_descriptors.h"

#ifdef _CDC_

//#include "HAL_UCS.h"
//#include "HAL_TLV.h"

//#include <string.h>

/*----------------------------------------------------------------------------+
 | Internal Constant Definition                                               |
 +----------------------------------------------------------------------------*/
#define NO_MORE_DATA    0xFFFF
#define EPBCT_NAK       0x80
#define EPCNF_TOGLE     0x20

#define DIRECTION_IN    0x80
#define DIRECTION_OUT   0x00





/*----------------------------------------------------------------------------+
 | Internal Variables                                                          |
 +----------------------------------------------------------------------------*/

//@testing
//ot_u16          Ttrace[32];
//usbreq_struct   Treq[32];
//ot_int          Tactive = 0;






//interface number
//static ot_u8 bInterfaceNumber;

//Set to 1 when USB device has been configured, set to 0 when unconfigured
//static ot_u8 bConfigurationNumber;

//static ot_u8 bHostAskMoreDataThanAvailable = 0;
//static ot_u8 bRemoteWakeup;

//ot_u16 wUsbEventMask;                 //used by USB_getEnabledEvents() and USB_setEnabledEvents()

//A buffer pointer to input end point 0 Data sent back to host is copied from
//this pointed memory location
//static ot_u8* pbIEP0Buffer;

//A buffer pointer to output end point 0 Data sent from host is copied to
//this pointed memory location
//static ot_u8* pbOEP0Buffer;

//For endpoint zero transmitter only Holds count of bytes remaining to be
//transmitted by endpoint 0.  A value of 0 means that a 0-length data packet
//A value of 0xFFFF means that transfer is complete

//For endpoint zero transmitter only Holds count of bytes remaining to be
//received by endpoint 0.  A value of 0 means that a 0-length data packet
//A value of 0xFFFF means that transfer is complete.



const tEDB* dbselect[] = { dblock_epin, dblock_epout };
                          
usbctl_struct usbctl;




void sub_reset_epdata(tEDB* edb, ot_u16* xbuf, ot_u8 count, ot_u8 y_offset) {
    edb->bEPCNF     = EPCNF_USBIE | EPCNF_UBME | EPCNF_DBUF;
    edb->bEPBBAX    = (ot_u8)(((*xbuf - START_OF_USB_BUFFER) >> 3) & 0x00ff);
    xbuf            = (ot_u16*)((ot_u16)xbuf + y_offset);
    edb->bEPBCTX    = count;
    edb->bEPBBAY    = (ot_u8)(((*xbuf - START_OF_USB_BUFFER) >> 3) & 0x00ff);
    edb->bEPBCTY    = count;
    edb->bEPSIZXY   = MAX_PACKET_SIZE;
}












ot_u8 usb_init (void) {
/// init Serial Number on debug or proto builds.  On release builds, it should
/// be burned into the FLASH during manufacturing/assembly
    ot_u16 bGIE;

    // atomic operation - disable interrupts
    bGIE = __get_SR_register() & GIE;   //save interrupt status
    __disable_interrupt();

#   if defined(__DEBUG__) || defined(__PROTO__)
    usbdesc_set_serialnum();
#   endif

    // configuration of USB module
    // To fix USB9 enumeration issue seen by Matrox
    USBKEYPID   = 0x9628;
    USBPWRCTL   = 0;
    USBPHYCTL   = PUSEL;                //use DP + DM as USB pins (not needed with external PHY on P9)
    USBPWRCTL   = VUSBEN + SLDOAON;     //enable primary and secondary LDO (3.3 and 1.8V)
    
    //wait some time for LDOs (5ms delay)
    delay_ms(5);             
    
    USBPWRCTL   = VUSBEN + SLDOAON + VBONIE;    //enable interrupt VBUSon
    USBKEYPID   = 0x9600;                       //access to configuration registers disabled

    usbctl.status       = 0;

    //restore interrupt status
    __bis_SR_register(bGIE);
    return kUSB_succeed;
}






ot_u8 usb_enable () {
    //volatile unsigned int i;
    //volatile unsigned int j = 0;
    ot_int j = 0;
    ot_u16 pll_unsettled;

    // exit if USB Bandgap and VBUS not valid (error) --or--
    // exit if PLL is already enabled (success)
    if (!(USBPWRCTL & USBBGVBV))                    return kUSB_generalError;
    if ((USBCNF & USB_EN) && (USBPLLCTL & UPLLEN))  return kUSB_succeed;

    ///@todo Make sure that this function only gets called at power-on (seems like the case).
    ///      At power-on, OpenTag already configures XT2
    USBKEYPID   = 0x9628;
    USBPLLDIVB  = USB_XT_FREQ;                  //Settings desired frequency

#   if (USB_PLL_XT == 2)
        USBPLLCTL = UPCS0 + UPFDEN + UPLLEN;    //Select XT2 as Ref, PLL for USB, Discrim. on, enable PLL
#   else
        USBPLLCTL = UPFDEN + UPLLEN;            //Select XT1 as Ref, PLL for USB, Discrim. on, enable PLL
#   endif

    //Wait some time till PLL is settled
    // - Clear PLL IR and wait some time for settling
    // - Success if PLL IR stays 0 (no error flags go high)
    // - Try 10 times, and give-up if PLL IR will not settle after 10 tries
    do {
        USBPLLIR    = 0x0000;
#       ifdef __MSP430F6638
            delay_us(1000); //1ms
#       else
            delay_us(500);  //0.5ms
#       endif

        pll_unsettled = (USBPLLIR != 0);
    }
    while (pll_unsettled && (++j <= 10));

    USBCNF     |= (pll_unsettled == 0);   //Enable USB if PLL settled (USB_EN = 0x0001)
    USBKEYPID   = 0x9600;

    return pll_unsettled;   //kUSB_succeed = 0, kUSB_generalError = 1
}





ot_u8 usb_disable (void) {
    USBKEYPID       = 0x9628;
    USBCNF          = 0;           //disable USB module
    USBPLLCTL      &= ~UPLLEN;      //disable PLL
    USBKEYPID       = 0x9600;
    usbctl.status   = 0;         //device is not enumerated or suspended
    return (kUSB_succeed);
}





//ot_u8 usb_setevents (ot_u16 events) {
//    wUsbEventMask = events;
//    return (kUSB_succeed);
//}





//ot_u16 usb_getevents () {
//    return (wUsbEventMask);
//}





ot_u8 usb_reset () {
    ot_u8 i;

    USBKEYPID = 0x9628;

    //reset should be on the bus after this!
    usbctl.status   = 0;    //device is not enumerated or suspended
    usbctl.intfnum  = 0;
    usbctl.confnum  = 0;    //device unconfigured
    usbctl.flags    = 0;
    
    //FRSTE handling:
    //Clear FRSTE in the RESRIFG interrupt service routine before re-configuring USB control registers.
    //Set FRSTE at the beginning of SUSRIFG, SETUP, IEPIFG.EP0 and OEPIFG.EP0 interrupt service routines.
    USBCTL              = 0;        //Function Reset Connection disable (FRSTE)

    usbctl.bytes_ep0in  = NO_MORE_DATA;
    usbctl.bytes_ep0out = NO_MORE_DATA;
    usbctl.action       = ACTION_nothing;

    //The address reset normally will be done automatically during bus function reset
    USBFUNADR           = 0x00;     //Address 0 is the value for "unconfigured device"

    // Set settings for EP0
    //NAK both 0 endpoints and enable endpoint 0 interrupt
    dblock_ep0.bIEPBCNT = EPBCNT_NAK;
    dblock_ep0.bOEPBCNT = EPBCNT_NAK;
    dblock_ep0.bIEPCNFG = EPCNF_USBIE | EPCNF_UBME | EPCNF_STALL;    //8 byte data packet
    dblock_ep0.bOEPCNFG = EPCNF_USBIE | EPCNF_UBME | EPCNF_STALL;    //8 byte data packet

    USBOEPIE            = USB_OUTEP_INT_EN;
    USBIEPIE            = USB_INEP_INT_EN;

    ///@todo there will need to be a loop for multi-interface impl.
    i                   = usb_handle[0].edb_index;

    // Set settings for IEPx (double bufferring)
//    dblock_epin[i].bEPCNF   = EPCNF_USBIE | EPCNF_UBME | EPCNF_DBUF;   
//    dblock_epin[i].bEPBBAX  = (ot_u8)(((usb_handle[0].in_xbuf - START_OF_USB_BUFFER) >> 3) & 0x00ff);
//    dblock_epin[i].bEPBBAY  = (ot_u8)(((usb_handle[0].in_ybuf - START_OF_USB_BUFFER) >> 3) & 0x00ff);
//    dblock_epin[i].bEPBCTX  = EPBCNT_NAK;
//    dblock_epin[i].bEPBCTY  = EPBCNT_NAK;
//    dblock_epin[i].bEPSIZXY = MAX_PACKET_SIZE;
    sub_reset_epdata(&(dblock_epin[i]), (ot_u16*)&usb_handle[0].in_xbuf, EPBCNT_NAK, 4);

    // Set settings for OEPx double buffering
//    dblock_epout[i].bEPCNF  = EPCNF_USBIE | EPCNF_UBME | EPCNF_DBUF ;
//    dblock_epout[i].bEPBBAX = (ot_u8)(((usb_handle[0].out_xbuf - START_OF_USB_BUFFER) >> 3) & 0x00ff);
//    dblock_epout[i].bEPBBAY = (ot_u8)(((usb_handle[0].out_ybuf - START_OF_USB_BUFFER) >> 3) & 0x00ff);
//    dblock_epout[i].bEPBCTX = 0x00;
//    dblock_epout[i].bEPBCTY = 0x00;
//    dblock_epout[i].bEPSIZXY= MAX_PACKET_SIZE;
    sub_reset_epdata(&(dblock_epout[i]), (ot_u16*)&usb_handle[0].out_xbuf, 0, 4);

    ///@todo This interrupt endpoint might not actually be used at all.

    // Additional Mgmt interrupt end point for CDC
    // decrement index: Mgmt endpoint is one below Data
    //if (usb_handle[0].class == CDC_CLASS) { 
//    dblock_epin[i-1].bEPCNF   = EPCNF_USBIE | EPCNF_UBME | EPCNF_DBUF;
//    dblock_epin[i-1].bEPBBAX  = (ot_u8)(((usb_handle[0].irq_xbuf - START_OF_USB_BUFFER) >> 3) & 0x00ff);
//    dblock_epin[i-1].bEPBBAY  = (ot_u8)(((usb_handle[0].irq_ybuf - START_OF_USB_BUFFER) >> 3) & 0x00ff);
//    dblock_epin[i-1].bEPBCTX  = EPBCNT_NAK;
//    dblock_epin[i-1].bEPBCTY  = EPBCNT_NAK;
//    dblock_epin[i-1].bEPSIZXY = MAX_PACKET_SIZE;
    sub_reset_epdata(&(dblock_epin[i-1]), (ot_u16*)&usb_handle[0].irq_xbuf, EPBCNT_NAK, 2);
    //}



    usbcdc_reset_data();   //reset CDC specific data structures

    USBCTL      = FEN;                          //enable function
    USBIFG      = 0;                            //make sure no interrupts are pending
    USBIE       = SETUPIE | RSTRIE | SUSRIE;    //enable USB specific interrupts (setup, reset, suspend)
    USBKEYPID   = 0x9600;

    return kUSB_succeed;
}




/* Instruct USB module to make itself available to the PC for connection, by pulling PUR high.
 */
ot_u8 usb_connect () {
    USBKEYPID   = 0x9628;
    USBCNF     |= PUR_EN;   //Generate rising edge on DP -> the host, which
                            //enumerates our device as full speed device.
    USBPWRCTL  |= VBOFFIE;  //Enable interrupt VUSBoff
    USBKEYPID   = 0x9600;

    return (kUSB_succeed);
}




/* Force a disconnect from the PC by pulling PUR low.
 */
ot_u8 usb_disconnect () {
    USBKEYPID           = 0x9628;
    USBCNF             &= ~PUR_EN;  //disconnect pull up resistor - logical disconnect from HOST
    USBPWRCTL          &= ~VBOFFIE; //disable interrupt VUSBoff
    USBKEYPID           = 0x9600;
    usbctl.status       = 0;

    return (kUSB_succeed);
}




/* Force a remote wakeup of the USB host.
 */
ot_u8 usb_force_wakeup () {
    if ((usbctl.status & USB_STATUS_SUSPENDED) == 0) {   //device is not suspended
        return (kUSB_NotSuspended);
    }
    
    if (usbctl.flags & USB_FLAG_WAKEON) {
        //volatile unsigned int i;
        USBCTL |= RWUP;                 //USB - Device Remote Wakeup Request - this bit is self-cleaned
        return (kUSB_succeed);
    }
    
    return (kUSB_generalError);
}




/* Returns the status of the USB connection.
 */ 
ot_u8 usb_connection_info () {
    ot_u8 retval;

    //ot_u8 retval = 0;
    //if (USBPWRCTL & USBBGVBV)         retval |= kUSB_vbusPresent;
    //if (USBCNF & PUR_EN)              retval |= kUSB_purHigh;
    //if (usbctl.enum_status != 0)      retval |= kUSB_Enumerated;
    //if (usbctl.fn_suspended != FALSE) retval |= kUSB_suspended;
    //else                              retval |= kUSB_NotSuspended;
    
    // JP's version below: compiles smaller, runs faster
    retval  = (USBCNF & PUR_EN) << 1;                   //PUR_EN = 0x02, kUSB_purHigh = 0x40
    retval |= (usbctl.status & USB_STATUS_ENUMERATED);    //USB_FLAG_ENUMERATED = 2, kUSB_Enumerated = 0x20
    retval<<= 1;
    retval |= (usbctl.status & USB_STATUS_SUSPENDED)+1;   //kUSB_NotSuspended = 0x10, kUSB_NotSuspended = 0x08
    retval<<= 3;               
    retval |= ((USBPWRCTL & USBBGVBV) != 0);            //kUSB_vbusPresent = 0x01
    
    return retval;
}




/* Returns the state of the USB connection.
 */
ot_u8 usb_connection_state () {
///@todo I should fix these stupid constants to be aligned with flags, but it
/// does not appear that this function ever gets called anyway

    // JP's version below: compiles smaller, runs faster
    static const ot_u8 retval[16] = {
        // USBBGVBV not set
        ST_USB_DISCONNECTED, ST_USB_DISCONNECTED, ST_USB_DISCONNECTED, ST_USB_DISCONNECTED,
        ST_USB_DISCONNECTED, ST_USB_DISCONNECTED, ST_USB_DISCONNECTED, ST_USB_DISCONNECTED,
        
        // USBBGVBV set, PUR not set
        ST_USB_CONNECTED_NO_ENUM, ST_USB_CONNECTED_NO_ENUM,
        ST_USB_CONNECTED_NO_ENUM, ST_USB_CONNECTED_NO_ENUM,
        
        // USBBGVBV set, PUR set, enumeration not complete, not suspended
        ST_ENUM_IN_PROGRESS,
        
        // USBBGVBV set, PUR set, enumeration not complete, suspended
        ST_NOENUM_SUSPENDED,
        
        // USBBGVBV set, PUR set, enumeration complete, not suspended
        ST_ENUM_ACTIVE,
        
        // USBBGVBV set, PUR set, enumeration complete, suspended
        ST_ENUM_SUSPENDED
    };

    // 8 + 4 + (0,1,2,3)
    ot_u8 code;
    code    = (USBPWRCTL & USBBGVBV)    /*0x08*/ \
            + (USBCNF & PUR_EN)<<1      /*0x04*/ \
            + (usbctl.status);          /*0,1,2,3*/

    return retval[code];

    /*
    //If no VBUS present
    if (!(USBPWRCTL & USBBGVBV)){
        return (ST_USB_DISCONNECTED);
    }

    //If VBUS present, but PUR is low
    if ((USBPWRCTL & USBBGVBV) && (!(USBCNF & PUR_EN))){
        return (ST_USB_CONNECTED_NO_ENUM);
    }

    //If VBUS present, PUR is high, and enumeration is complete, and not suspended
    if ((USBPWRCTL & USBBGVBV) && (USBCNF & PUR_EN)
        && (usbctl.enum_status == ENUMERATION_COMPLETE)
        && (!(usbctl.fn_suspended == TRUE))){
        return (ST_ENUM_ACTIVE);
    }

    //If VBUS present, PUR is high, and enumeration is NOT complete, and  suspended
    if ((USBPWRCTL & USBBGVBV) && (USBCNF & PUR_EN)
        && (!(usbctl.enum_status == ENUMERATION_COMPLETE))
        && (usbctl.fn_suspended == TRUE)){
        return (ST_NOENUM_SUSPENDED);
    }

    //If VBUS present, PUR is high, and enumeration is complete, and  suspended
    if ((USBPWRCTL & USBBGVBV) && (USBCNF & PUR_EN)
        && (usbctl.enum_status == ENUMERATION_COMPLETE)
        && (usbctl.fn_suspended == TRUE)){
        return (ST_ENUM_SUSPENDED);
    }

    //If VBUS present, PUR is high, but no enumeration yet
    if ((USBPWRCTL & USBBGVBV) && (USBCNF & PUR_EN)
        && (!(usbctl.enum_status == ENUMERATION_COMPLETE))){
        return (ST_ENUM_IN_PROGRESS);
    }

    return (ST_ERROR);
    */
}




ot_u8 usb_suspend (void) {
    usbctl.status  |= USB_STATUS_SUSPENDED;
    USBKEYPID       = 0x9628;                 //set KEY and PID to 0x9628 -> access to configuration registers enabled
    USBCTL         |= FRSTE;                    //Function Reset Connection Enable
    USBIFG         &= ~SUSRIFG;                 //clear interrupt flag
    USBPLLCTL      &= ~UPLLEN;

#   if (USB_DISABLE_XT_SUSPEND)
#       if (USB_PLL_XT == 2)
            UCSCTL6 |= XT2OFF;        //disable XT2
#       else
            UCSCTL6 |= XT1OFF;
#       endif
#   endif

    //Disable USB specific interrupts (setup, suspend, reset), enable resume.
    //If the reset occurred during device in suspend, the resume-interrupt will
    //come, after reset interrupt
    USBIE       = RESRIE;
    USBKEYPID   = 0x9600;

    return (kUSB_succeed);
}




ot_u8 usb_resume (void) {
    usb_enable();                           //enable PLL

    USBIFG         &= ~(RESRIFG | SUSRIFG);         //clear interrupt flags
    USBIE           = SETUPIE | RSTRIE | SUSRIE;    //enable USB specific interrupts (setup, reset, suspend)
    usbctl.status  &= ~USB_STATUS_SUSPENDED;
    
    return (kUSB_succeed);
}







void usbcmd_stall_ep0 (void) {
    dblock_ep0.bIEPCNFG |= EPCNF_STALL;
    dblock_ep0.bOEPCNFG |= EPCNF_STALL;
}


void usbcmd_stall_ep0out (void) {
    //in standard USB request, there is not control write request with data stage
    //control write, stall output endpoint 0
    //wLength should be 0 in all cases
    dblock_ep0.bOEPCNFG |= EPCNF_STALL;
}



void usbcmd_clear_ep0 (void) {
    dblock_ep0.bOEPBCNT = 0x00;
}



void usbcmd_txnext_ep0 (void) {
    ot_int pkt_size;

    // Set Control Action to Nothing (0) or Data_in (1) depending on if there
    // will be a packet after this one (or any packet at all).
    usbctl.action = ((ot_int)usbctl.bytes_ep0in >= (ot_int)EP0_PACKET_SIZE);

    // Prepare to load 8 bytes into the EP0_IN buffer.  If there are less than
    // 8 bytes to TX, adjust the packet size to that smaller amount.
    if ((ot_int)usbctl.bytes_ep0in >= 0){
        pkt_size         = EP0_PACKET_SIZE;
        usbctl.bytes_ep0in -= EP0_PACKET_SIZE;
        
        if (usbctl.bytes_ep0in == 0) {
            //On Overpull condition, bytes_ep0in will stay == 0.  On no data
            //left to TX and no overpull, bytes_ep0in is set to < 0.
            usbctl.bytes_ep0in -= USB_FLAG_OVERPULL;
            usbctl.bytes_ep0in += (usbctl.flags & USB_FLAG_OVERPULL);
        }
        else if ((ot_int)usbctl.bytes_ep0in < 0) {
            //packet size is adjusted to 1 < pkt_size < 7
            pkt_size += (ot_int)usbctl.bytes_ep0in;
        }

        ot_memcpy(abuf_ep0in, usbctl.pbuf_ep0in, pkt_size);
        usbctl.pbuf_ep0in  += pkt_size;
        dblock_ep0.bIEPBCNT = pkt_size;
    }
}



/*
void usbcmd_txnext_ep0 (void) {
    ot_u8 bPacketSize;
    ot_u8 bIndex;

    usbctl.action = ((ot_int)usbctl.bytes_ep0in >= (ot_int)EP0_PACKET_SIZE);

    //First check if there are bytes remaining to be transferred
    if (usbctl.bytes_ep0in != NO_MORE_DATA){

        if (usbctl.bytes_ep0in > EP0_PACKET_SIZE){
            //More bytes are remaining than will fit in one packet
            //there will be More IN Stage
            bPacketSize         = EP0_PACKET_SIZE;
            usbctl.bytes_ep0in -= EP0_PACKET_SIZE;
            usbctl.action       = ACTION_data_in;
        }
        else if (usbctl.bytes_ep0in < EP0_PACKET_SIZE){
            //The remaining data will fit in one packet.
            //This case will properly handle wBytesRemainingOnIEP0 == 0
            bPacketSize         = (ot_u8)usbctl.bytes_ep0in;
            usbctl.bytes_ep0in  = NO_MORE_DATA;   //No more data need to be Txed
            usbctl.action       = ACTION_nothing;
        }
        else {
            bPacketSize         = EP0_PACKET_SIZE;
            usbctl.bytes_ep0in  = 0 - (ot_int)(usbctl.flags & USB_FLAG_OVERPULL);
            usbctl.action       = ACTION_data_in;

            //if (usbctl.flags & USB_FLAG_OVERPULL){
                //usbctl.bytes_ep0in  = 0;
                //usbctl.action       = ACTION_data_in;
            //}
            //else {
                //usbctl.bytes_ep0in  = NO_MORE_DATA;
                //usbctl.action       = ACTION_data_in;
            //}
        }

        ot_memcpy(abuf_ep0in, usbctl.pbuf_ep0in, bPacketSize);
        usbctl.pbuf_ep0in  += bPacketSize;
        dblock_ep0.bIEPBCNT = bPacketSize;
    }
    else {
        usbctl.action = ACTION_nothing;
    }
}
*/



void usbcmd_tx_ep0 (ot_u8* pbBuffer) {
    ot_u16 scratch;
    usbctl.pbuf_ep0in   = pbBuffer;
    scratch             = dblock_setup.wLength;

    // Consider default as an overpull condition, that requested packet length
    // is greater than bytes remaining to send back to host
    usbctl.flags       |= USB_FLAG_OVERPULL;

    // If the opposite is true, limit remaining bytes in input, and cancel the
    // overpull condition.
    if (usbctl.bytes_ep0in >= scratch) {
        usbctl.bytes_ep0in  = scratch;
        usbctl.flags       &= ~USB_FLAG_OVERPULL;
    }

    usbcmd_txnext_ep0();
}



void usbcmd_rxnext_ep0 (void) {
    ot_u8 bByte;

    bByte = dblock_ep0.bOEPBCNT & EPBCNT_BYTECNT_MASK;

    if (usbctl.bytes_ep0out >= (ot_u16)bByte) {
        ot_memcpy(usbctl.pbuf_ep0out, abuf_ep0out, bByte);

        //clear the NAK bit for next packet
        if (usbctl.bytes_ep0out > 0){
            usbctl.action = ACTION_data_out;
            usbcmd_clear_ep0();
            return;
        }
    }

    usbctl.action = ACTION_nothing;
    usbcmd_stall_ep0out();
}





void usbcmd_rx_ep0 (ot_u8* pbBuffer) {
    usbctl.pbuf_ep0out  = pbBuffer;
    usbctl.bytes_ep0out = dblock_setup.wLength;
    usbctl.action       = ACTION_data_out;
    usbcmd_clear_ep0();
}




void usbcmd_txzlp_ep0 (void) {
    dblock_ep0.bIEPBCNT = 0x00;
    usbctl.action       = ACTION_nothing;
    usbctl.bytes_ep0in  = NO_MORE_DATA;
}




inline CMD_RETURN sub_usbget(ot_u8 remaining_bytes, ot_u8* send_data) {
    usbcmd_clear_ep0();                                //for status stage
    usbctl.bytes_ep0in = remaining_bytes;
    usbcmd_tx_ep0(send_data);
    //return False;
}


CMD_RETURN usbcmd_get_cfg (void) {
    sub_usbget(1, (ot_u8*)&usbctl.confnum);
}

CMD_RETURN usbcmd_get_devdesc (void) {
///@note Will need to select appropriate device descriptor when/if multiple
/// non-composite interfaces are supported
    sub_usbget(SIZEOF_DEVICE_DESCRIPTOR, (ot_u8*)usbdesc_device);
}

CMD_RETURN usbcmd_get_cfgdesc (void) {
///@note Will need to select appropriate device descriptor when/if multiple
/// non-composite interfaces are supported
    sub_usbget(sizeof(usbdesc_cfg), (ot_u8*)&usbdesc_cfg);
}




CMD_RETURN usbcmd_get_strdesc (void) {
/// To understand how this works, look at usbdesc_string[] in usb_descriptors.c
/// There are multiple strings in this descriptor, and they are organized by
/// LTV (length type value).  The cursor goes past each LTV item until it 
/// gets to the index specified from the host command request.
    ot_u8*  cursor;
    ot_int  str_i;
    str_i   = (ot_u8)dblock_setup.wValue;   //important to get low byte here
    cursor  = (ot_u8*)usbdesc_string;

    if (str_i == 3) {
        __no_operation();
    }

    while (str_i-- > 0) {
        cursor += *cursor;
    }

    sub_usbget(*cursor, cursor);
}




CMD_RETURN usbcmd_get_intf (void) {
    //not fully supported, return one byte, zero
    usbctl.response[0] = 0x00;   //changed to report alternative setting byte
    usbctl.response[1] = usbctl.intfnum;
    
    sub_usbget(2, (ot_u8*)&usbctl.response[0]);
}




CMD_RETURN usbcmd_get_devstatus (void) {
///@note Will need to select appropriate descriptor when/if multiple
/// non-composite interfaces are supported

    //if ((usbdesc_cfg.generic.attrs &
    //     CFG_DESC_ATTR_SELF_POWERED) == CFG_DESC_ATTR_SELF_POWERED) {
    //    usbctl.response[0] = DEVICE_STATUS_SELF_POWER;
    //}
    usbctl.response[0] = ((usbdesc_cfg.generic.attrs & CFG_DESC_ATTR_SELF_POWERED) != 0);

    //if (usbctl.flags & USB_FLAG_WAKEON){
    //    usbctl.response[0] |= DEVICE_STATUS_REMOTE_WAKEUP;
    //}
    usbctl.response[0] |= (usbctl.flags & USB_FLAG_WAKEON);

    //Return self power status and remote wakeup status
    sub_usbget(2, (ot_u8*)&usbctl.response[0]);
}




CMD_RETURN usbcmd_get_intfstatus(void) {
    //check bIndexL for index number (not supported)
    usbctl.response[0] = 0x00;   //changed to support multiple interfaces
    usbctl.response[1] = usbctl.intfnum;
    
    sub_usbget(2, (ot_u8*)&usbctl.response[0]);
}




CMD_RETURN usbcmd_get_epstatus (void) {
    ot_u8 ep_n;
    ot_u8 is_epin;

    //Endpoint number is bIndexL
    ep_n    = dblock_setup.wIndex & EP_DESC_ADDR_EP_NUM;
    is_epin = (dblock_setup.wIndex & EP_DESC_ADDR_DIR_IN) >> 6; //make 0x80 into 2 or 0

    if (ep_n == 0x00) {
        ot_u8* db0CNFG;
        // db0CNFG points to dblock_ep0.bOEPCNFG or dblock_ep0.bIEPCNFG (-2)
        db0CNFG             = ((ot_u8*)&dblock_ep0.bOEPCNFG);
        db0CNFG            -= is_epin;
        usbctl.response[0]  = *db0CNFG & EPCNF_STALL;
    }
    
    //no response if endpoint is not supported.
    else if (ep_n <= MAX_ENDPOINT_NUMBER) {
        //EP is from EP1 to EP7 while C language start from 0
        //Firmware should NOT respond if specified endpoint is not supported. (chapter 8)
        tEDB* dblock;
        dblock  = (is_epin) ? dblock_epin : dblock_epout;
      //dblock  = (tEDB*)dbselect[(dblock_setup.wIndex&EP_DESC_ADDR_DIR_IN)==0];

        ep_n--;
        usbctl.response[0]  = dblock[ep_n].bEPCNF & EPCNF_STALL;
    }
    
    usbctl.response[0] >>= 3; //STALL is on b3
    sub_usbget(2, (ot_u8*)&usbctl.response[0]);
}



CMD_RETURN usbcmd_set_address (void) {
    usbcmd_stall_ep0out();     //control write without data stage

    //bValueL contains device address
    if (dblock_setup.wValue < 128){
        //hardware will update the address after status stage
        //therefore, firmware can set the address now.
        USBFUNADR = dblock_setup.wValue;
        usbcmd_txzlp_ep0();
    }
    else {
        usbcmd_stall_ep0();
    }

    //return False;
}




CMD_RETURN usbcmd_set_cfg (void) {
    usbcmd_stall_ep0out();        //control write without data stage

    //configuration number is in bValueL
    usbctl.confnum  = (ot_u8)dblock_setup.wValue;
    usbcmd_txzlp_ep0();

    //Set Enumeration (or not)
    usbctl.status  |= (usbctl.confnum != 0) << 1;
    //return False;
}



//Added by JP
void sub_eval_devfeature(ot_u8 enable_wakeon) {
    if (dblock_setup.wValue == FEATURE_REMOTE_WAKEUP) {
        usbctl.flags &= ~USB_FLAG_WAKEON;
        usbctl.flags |= enable_wakeon;
        usbcmd_txzlp_ep0();
    }
    else {
        usbcmd_stall_ep0();
    }
    //return False;
}


CMD_RETURN usbcmd_clear_devfeature (void) {
    sub_eval_devfeature(0);
}


CMD_RETURN usbcmd_set_devfeature (void) {
    sub_eval_devfeature(USB_FLAG_WAKEON);
}



//Added by JP
CMD_RETURN sub_prep_epfeature(ot_u8 clear_mask, ot_u8 set_mask) {
    ot_u8 ep_n;

    //bIndexL contains endpoint number (Endpoint number is in low byte of wIndex)
    ep_n = (dblock_setup.wIndex & EP_DESC_ADDR_EP_NUM);
    
    // Do mask operation on the requested input or output endpoint
    // Ignore requests for EP0 or any non-implemented EP
    if (ep_n <= MAX_ENDPOINT_NUMBER) {
        if (ep_n != 0) {
            tEDB* dblock;
            dblock  = (dblock_setup.wIndex & EP_DESC_ADDR_DIR_IN) ? \
                        dblock_epin : dblock_epout;
            ep_n--;
            dblock[ep_n].bEPCNF &= clear_mask;
            dblock[ep_n].bEPCNF |= set_mask;
        }
    
        usbcmd_txzlp_ep0();
    }
    //return False;
}


CMD_RETURN usbcmd_clear_epfeature (void) {
    sub_prep_epfeature(~(EPCNF_STALL | EPCNF_TOGGLE), 0);
}



CMD_RETURN usbcmd_set_epfeature (void) {
    //wValue contains feature selector
    ///@todo see if this can be done via pre-processor
    if (dblock_setup.wValue == FEATURE_ENDPOINT_STALL){
        sub_prep_epfeature(~0, EPCNF_STALL);
    }
    else {
        usbcmd_stall_ep0();
    }
    //return False;
}




CMD_RETURN usbcmd_set_intf (void) {
    //bValueL contains alternative setting
    //bIndexL contains interface number
    //change code if more than one interface is supported
    usbcmd_stall_ep0out();                         //control write without data stage
    usbctl.intfnum = dblock_setup.wIndex;
    usbcmd_txzlp_ep0();

    //return False;
}




CMD_RETURN usbcmd_invalid_request (void) {
    //check if setup overwrite is set
    //if set, do nothing since we might decode it wrong
    //setup packet buffer could be modified by hardware if another setup packet
    //was sent while we are deocding setup packet
    if ((USBIFG & STPOWIFG) == 0x00){
        usbcmd_stall_ep0();
    }

    //return False;
}




typedef void (*tpF)(void);

void usbproc_parse_request (void) {
    ot_u8*  pb_reqlist;
    tpF     parse_fn;
    
    //point to beginning of the matrix
    pb_reqlist = (ot_u8*)usbcmd_list;

    while (1) {
        ot_u8*  list_cursor;
        ot_u8   req_type;
        ot_u8   req_code;
        
        list_cursor = pb_reqlist;
        req_type    = *list_cursor++;
        req_code    = *list_cursor++;

        // No additional parsing for Input or Output requests of these types.
        if (((req_type & req_code) == 0xff) ||
            ((dblock_setup.bmRequestType & ~0x80) == (USB_REQ_TYPE_VENDOR | USB_REQ_TYPE_DEVICE))) {
            break;
        }

        if ((req_type == dblock_setup.bmRequestType) && (req_code == dblock_setup.bRequest)) {
            ot_u8*  setup_cursor    = (ot_u8*)&dblock_setup + 2;
            ot_u8   result          = 0x03;

            // first two bytes matched, compare the rest, 
            // break loop if byte 9 matches "checksum"
            while ((result & 0x80) == 0) {
                result<<=1;
                result |= (*setup_cursor++ == *list_cursor++);
            }
            if ((*list_cursor & result) == *list_cursor) {
                break;
            }
        }
        
        // Increment request list to next request
        pb_reqlist += sizeof(usbcmd_struct);
    }

    //if another setup packet comes before we have the chance to process current
    //setup request, we return here without processing the request
    //this check is not necessary but still kept here to reduce response(or simulation) time
    if ((USBIFG & STPOWIFG) != 0x00){
        //return False;
        return;
    }

    //now we found the match and jump to the function accordingly.
    parse_fn = ((usbcmd_struct*)pb_reqlist)->pUsbFunction;

    //@testing
    //if (Tactive < 32) {
    //    Ttrace[Tactive] = (ot_u16)parse_fn;
    //    ot_memcpy((ot_u8*)&Treq[Tactive], (ot_u8*)&dblock_setup, sizeof(usbreq_struct));
    //    Tactive++;
    //}

    //call function
    //bWakeUp = (*parse_fn)();
    (*parse_fn)();

    //perform enumeration complete event:
    //when SetAddress was called and USBADDR is not zero
    ///@todo This does not actually appear to be the end of enumeration, which
    /// takes place within the usbcmd_set_cfg() command... ?
    if ((parse_fn == &usbcmd_set_address) && (USBFUNADR != 0)){
        //bWakeUp = usbevt_enumerate();
        usbevt_enumerate();
    }

    //return bWakeUp;
}




#endif

