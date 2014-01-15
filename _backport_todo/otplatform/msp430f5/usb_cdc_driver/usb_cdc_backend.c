/*  Copyright (c) 2009, Texas Instruments Incorporated
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
  *
  */
/** @file       /otplatform/msp430f5/usb_cdc_driver/usb_cdc_backend.c
  * @author     RSTO, JP Norair
  * @version    R102
  * @date       8 Nov 2012
  * @brief      CDC DLL for MSP430F5-USB, optimized by JP Norair for OpenTag
  * @ingroup    MSP430F5_USB_CDC
  *
  * This is the CDC driver adapted by JP Norair from the official TI USB stack
  * code, and targeted for OpenTag.  In this file, there is nothing especially
  * dependent on OpenTag, however there are four references:
  * <LI> platform_memcpy() is used explicitly.  It has the same interface as normal 
  *          memcpy(), except that platform_memcpy() typically uses DMA.    </LI>
  * <LI> platform_memset() is used, to similar affect as platform_memcpy()  </LI>
  * <LI> Platform Macro "PLATFORM_ENDIAN32()" is used.  On the MSP430, this
  *          maps to the intrinsic function __swap_long_bytes(u32) </LI>
  * <LI> OpenTag types are used.  They should be transparent.  </LI>
  *
  * Document History:
  * <PRE>
  * WHO         WHEN        WHAT
  * ---         ----------  ---------------------------------------------------
  * RSTO        2008/09/03  born
  * RSTO        2008/09/19  Changed usbcdc_txdata to send more then 64bytes
  * RSTO        2008/12/23  enhancements of CDC API
  * RSTO        2008/05/19  updated usbcdc_intf_status()
  * RSTO        2009/05/26  added usbcdc_bytes_in_buffer()
  * RSTO        2009/05/28  changed usbcdc_txdata()
  * RSTO        2009/07/17  updated usbcdc_bytes_in_buffer()
  * RSTO        2009/10/21  move __disable_interrupt() before
  *                             checking for suspend
  * MSP,Biju    2009/12/28  Fix for the bug "Download speed is slow"
  * ---         ----------  ---------------------------------------------------
  * JPN         2012/05/01  Code size optimizations for single-CDC interface
  * JPN         2012/05/07  Some state numeric values changed for optimization
  * JPN         2012/11/8   Major architectural and coding optimizations
  * </PRE>
  *****************************************************************************
  */

#include "usb_cdc_driver/usb_descriptors.h"
#ifdef _CDC_

#include "usb_cdc_driver/usb_types.h"                //Basic Type declarations
#include "usb_cdc_driver/usb_device.h"
#include "usb_cdc_driver/defMSP430USB.h"
#include "usb_cdc_driver/usb_main.h"                  //USB-specific Data Structures
#include "usb_cdc_driver/usb_cdc_backend.h"


#include "OT_platform.h"

#if (CDC_NUM_INTERFACES == 1) 
#   define USB_HANDLE(INTF) usb_handle[0]
#   define CDC_WRITER(INTF) cdc.writer[0]
#   define CDC_READER(INTF) cdc.reader[0]
#   define CDC_CTRLER(INTF) cdc.ctrler[0]

#elif (CDC_NUM_INTERFACES > 1)
#   define USB_HANDLE(INTF) usb_handle[INTF]
#   define CDC_WRITER(INTF) cdc.writer[(INTF-CDC0_INTFNUM)]
#   define CDC_READER(INTF) cdc.reader[(INTF-CDC0_INTFNUM)]
#   define CDC_CTRLER(INTF) cdc.ctrler[(INTF-CDC0_INTFNUM)]

#else 
#   error "CDC_NUM_INTERFACES is set to a negative value"
#endif


#if (USBEVT_MASK & USBEVT_TXCOMPLETE)
#   define USBCDCEVT_TXDONE(INTFNUM)    usbcdcevt_txdone(INTFNUM)
#else
#   define USBCDCEVT_TXDONE(INTFNUM)    while(0)
#endif

#if (USBEVT_MASK & USBEVT_RXCOMPLETE)
#   define USBCDCEVT_RXDONE(INTFNUM)    usbcdcevt_rxdone(INTFNUM)
#else
#   define USBCDCEVT_RXDONE(INTFNUM)    while(0)
#endif

#if (USBEVT_MASK & USBEVT_RXBUFFERED)
#   define USBCDCEVT_RXDETECT(INTFNUM)  usbcdcevt_rxdetect(INTFNUM)
#else
#   define USBCDCEVT_RXDETECT(INTFNUM)  while(0)
#endif





/** Data
  * ===================================
  */

cdc_struct   cdc;


//extern __no_init tEDB __data16 dblock_epin[];
//extern __no_init tEDB __data16 dblock_epout[];






/** Subroutines
  * ===================================
  */

void sub_update_writebuf(cdc_write_struct* writer, ot_u8 byte_count) {
    writer->xy_select  ^= 1;
    writer->txbytes    -= byte_count;
    writer->qbuf       += byte_count;  //move buffer pointer
    writer->txcnt_last  = byte_count;
}


//This function copies data from OUT endpoint into user's buffer
//Arguments:
//pEP - pointer to EP to copy from
//pCT - pointer to pCT control reg
//
void sub_copy_usbtobuf (ot_u8* pEP, ot_u8* pCT, ot_u8 x) {
    ot_u8 nCount;

    //how many bytes we can get from one endpoint buffer?
    nCount = (CDC_READER(x).rxbytes > CDC_READER(x).epbytes) ? \
                CDC_READER(x).epbytes : CDC_READER(x).rxbytes;

    //copy data from OEP3 X or Y buffer
    ///@todo this could be made into platform_memcpy2() via (nCount+1)>>1
    platform_memcpy_2(CDC_READER(x).qbuf, pEP, (nCount+1)>>1);
    
    //update buffer pointer past new bytes
    CDC_READER(x).rxbytes -= nCount;
    CDC_READER(x).qbuf    += nCount;

    // If all bytes are copied from receive buffer, switch buffer & clear EP.
    // Else, update EP (endpoint).
    if (nCount == CDC_READER(x).epbytes) {
        *pCT                       = 0;
        CDC_READER(x).epbytes      = 0;
        CDC_READER(x).xy_select   ^= 1;
    }
    else {
        CDC_READER(x).epbytes     -= nCount;
        CDC_READER(x).epcursor     = pEP + nCount;
    }
}


void sub_buffer_firedrill(ot_u8 epcnt, ot_u8 x, ot_u8* ep_out) {
    if (epcnt & EPBCNT_NAK) {
        //holds how many valid bytes in the EP buffer
        CDC_READER(x).epbytes = epcnt & ~EPBCNT_NAK;
        sub_copy_usbtobuf(ep_out, CDC_READER(x).ct1, x);

        // Read data from second buffer if the second buffer has received data
        epcnt = *CDC_READER(x).ct2;
        if ((CDC_READER(x).rxbytes > 0) && (epcnt & EPBCNT_NAK)) {
            //epcnt holds num valid bytes in EP buffer, plus NAK flag on b7
            CDC_READER(x).epbytes   = epcnt & ~EPBCNT_NAK;
            CDC_READER(x).ct1       = CDC_READER(x).ct2;
            sub_copy_usbtobuf(CDC_READER(x).ep2, CDC_READER(x).ct2, x);
        }
    }
}











void usbcdc_reset_data () {
	/// Wipe all interfaces
	platform_memset((ot_u8*)&cdc, 0, sizeof(cdc_struct));
    //platform_memset((ot_u8*)cdc.writer, 0, sizeof(cdc.writer));
    //platform_memset((ot_u8*)cdc.reader, 0, sizeof(cdc.reader));
    //platform_memset((ot_u8*)cdc.ctrler, 0, sizeof(cdc.ctrler));

    /// Set data bits to 8 for each interface
#   if ((NONCOMP_NUM_USB_INTERFACES <= 0) || (NONCOMP_NUM_USB_INTERFACES > 3))
#       error "MSP430 cannot hold more than 3 CDC interfaces (only has 7 endpoints)"
#   elif (NONCOMP_NUM_USB_INTERFACES >= 1)
        cdc.ctrler[0].bDataBits = 8;
#   endif
#   if (NONCOMP_NUM_USB_INTERFACES >= 2)
        cdc.ctrler[1].bDataBits = 8;
#   endif
#	if (NONCOMP_NUM_USB_INTERFACES == 3)
        cdc.ctrler[2].bDataBits = 8;
#   endif
}



/** Sends data over interface x, of size size and starting at address data.
  * Returns: kUSBCDC_sendStarted, kUSBCDC_sendComplete, kUSBCDC_intfBusyError
  */
ot_u8 usbcdc_txdata (const ot_u8* data, ot_u16 size, ot_u8 x) {
    ot_u16 bGIE;
    ot_u8   scratch;

    // Do not access USB memory when size is zero or device is suspended.
    // It may produce a bus error if you do.
    if (size == 0) {
        return (kUSBCDC_generalError);
    }
    if (usbctl.status != USB_STATUS_ENUMERATED) {
        return (kUSBCDC_busNotAvailable);
    }
    //the USB still sends previous data, we have to wait
    if (CDC_WRITER(x).txbytes != 0) {
        return (kUSBCDC_intfBusyError);
    }

    bGIE  = (__get_SR_register() & GIE);        //save interrupt status
    __disable_interrupt();                      //atomic operation - disable interrupts

    //The data will be sent out from interrupt USBIEPIFG
    CDC_WRITER(x).txsize    = size;
    CDC_WRITER(x).txbytes   = size;
    CDC_WRITER(x).qbuf      = (ot_u8*)data;

    scratch = USB_HANDLE(x).edb_index;

    //trigger Endpoint Interrupt - to start send operation
    USBIEPIFG |= 1 << (scratch+1);     //IEPIFGx;

    __bis_SR_register(bGIE);
    return (kUSBCDC_sendStarted);
}




#define EP_MAX_PACKET_SIZE_CDC      64


//this function is used only by USB interrupt
void usbcdc_transfer_buf2host (ot_u8 x) {
    ot_u8 byte_count;
    ot_u8 i;
    ot_u8 j;
    ot_u8* pEP[2];
    ot_u8* pCT[2];
    //ot_u8 bWakeUp = FALSE;                                                   //TRUE for wake up after interrupt
    
    i = USB_HANDLE(x).edb_index;

    /// 1. If there is nothing left to send:
    ///    send a ZLP terminator, or exit if ZLP was sent last call
    if (CDC_WRITER(x).txbytes == 0) {
        if (CDC_WRITER(x).is_zlp_sent == 0){
            CDC_WRITER(x).is_zlp_sent = TRUE;

            if (CDC_WRITER(x).txcnt_last == EP_MAX_PACKET_SIZE_CDC) {
                pCT[0]  = &dblock_epin[i].bEPBCTX;
                pCT[1]  = &dblock_epin[i].bEPBCTY;
                j       = CDC_WRITER(x).xy_select;
                if (*pCT[j] & EPBCNT_NAK) {
                    *pCT[j] = 0;
                    CDC_WRITER(x).xy_select ^= 1;
                }
            }

            CDC_WRITER(x).txsize = 0;      //nothing to send
            USBCDCEVT_TXDONE(x);  //call event callback function
            //bWakeUp |= USBCDCEVT_TXDONE(x);  //call event callback function
        }

        //return (bWakeUp);
        return;
    }


    /// 2. There is more data to send
    /// <LI> Assure ZLP-Terminator flag is false </LI>
    /// <LI> Pick the open buffer, and rotate it to the next </LI>
    CDC_WRITER(x).is_zlp_sent = FALSE;

    j       = CDC_WRITER(x).xy_select;
    pEP[j]  = (ot_u8*)USB_HANDLE(x).in_xbuf;
    pCT[j]  = &dblock_epin[i].bEPBCTX;
    j      ^= 1;
    pEP[j]  = (ot_u8*)USB_HANDLE(x).in_ybuf;
    pCT[j]  = &dblock_epin[i].bEPBCTY;

    // iii
    j = 0;
    usbcdc_transfer_buf2host_LOAD:

    if (*pCT[j] & EPBCNT_NAK) {
        //how many byte we can send over one endpoint buffer
        byte_count = (CDC_WRITER(x).txbytes > EP_MAX_PACKET_SIZE_CDC) ? \
                    EP_MAX_PACKET_SIZE_CDC : CDC_WRITER(x).txbytes;
    
        platform_memcpy(pEP[j], CDC_WRITER(x).qbuf, byte_count);
        *pCT[j] = byte_count;

        sub_update_writebuf(&CDC_WRITER(x), byte_count);

        //If the 64-byte buffer was not enough to fit the packet, preload the
        //next chunk into the second buffer.  Of course, check the buffer NAK
        //first to make sure it's not in use already.
        j++;
        if ((j!=2) && (CDC_WRITER(x).txbytes > 0)) {
            goto usbcdc_transfer_buf2host_LOAD;
        }
    }
    //return (bWakeUp);
}








/*
 * Aborts an active send operation on interface x.
 * Returns the number of bytes that were sent prior to the abort, in size.
 */
ot_u8 usbcdc_txabort (ot_u16* size, ot_u8 x) {
    ot_u16 bGIE;

    bGIE  = (__get_SR_register() & GIE);        //save interrupt status
    __disable_interrupt();                      //disable interrupts - atomic operation

    *size = (CDC_WRITER(x).txsize - CDC_WRITER(x).txbytes);
    CDC_WRITER(x).txsize   = 0;
    CDC_WRITER(x).txbytes  = 0;

    __bis_SR_register(bGIE);     //restore interrupt status
    return (kUSB_succeed);
}









/*
 * Receives data over interface x, of size size, into memory starting at address data.
 * Returns:
 *  kUSBCDC_receiveStarted  if the receiving process started.
 *  kUSBCDC_receiveCompleted  all requested date are received.
 *  kUSBCDC_receiveInProgress  previous receive opereation is in progress. The requested receive operation can be not started.
 *  kUSBCDC_generalError  error occurred.
 */
ot_u8 usbcdc_rxdata (ot_u8* data, ot_u16 size, ot_u8 x) {
    ot_int offset;
    ot_u16 bGIE;
    ot_u8 nTmp1;
    ot_u8 i;
    ot_u8* scratch;
    
    //Protections to prevent accessing USB with size = 0, NULL buffer, USB
    //Suspended, or USB not enumerated
    if ((size == 0) || (data == NULL)) {
        return (kUSBCDC_generalError);
    }
    if (usbctl.status != USB_STATUS_ENUMERATED){
        return (kUSBCDC_busNotAvailable);
    }
    //receive process already started
    if (CDC_READER(x).qbuf != NULL){
        return (kUSBCDC_intfBusyError);
    }

    bGIE = (__get_SR_register() & GIE);
    __disable_interrupt();          //atomic operation - disable interrupts
    
    // Get endpoint data block index
    i = USB_HANDLE(x).edb_index;
    
    // Set RX Buffer constraints
    CDC_READER(x).qbuf     = data;         //set user receive buffer
    CDC_READER(x).rxsize   = size;         //bytes to receive
    CDC_READER(x).rxbytes  = size;         //left bytes to receive

    //read rest of data from buffer, if any
    if (CDC_READER(x).epbytes > 0) {
        //copy data from pEP-endpoint into User's buffer
        sub_copy_usbtobuf(CDC_READER(x).epcursor, CDC_READER(x).ct1, x);

        if (CDC_READER(x).rxbytes == 0) { //the Receive opereation is completed
            goto usbcdc_rxdata_EXIT;
        }

        //offset should be -2 (Y-X) or 2 (X-Y)
        offset                  = 2 - ((CDC_READER(x).ct1 == &dblock_epout[i].bEPBCTY) << 2);   
        CDC_READER(x).ct1       = (&dblock_epout[i].bSPARE1 + offset);
        scratch                 = (ot_u8*)&USB_HANDLE(x).in_xbuf;
        CDC_READER(x).epcursor  = *(ot_u8**)(scratch+offset);
        
        //try read data from second buffer (it was just rotated above)
        nTmp1 = *CDC_READER(x).ct1;
        if (nTmp1 & EPBCNT_NAK){                            //if the second buffer has received data?
            nTmp1                 &= ~0x80;        //clear NAK bit
            CDC_READER(x).epbytes  = nTmp1;        //holds how many valid bytes in the EP buffer
            sub_copy_usbtobuf(CDC_READER(x).epcursor, CDC_READER(x).ct1,  x);
        }

        if (CDC_READER(x).rxbytes == 0){ //the Receive opereation is completed
            goto usbcdc_rxdata_EXIT;
        }
    } 
    
    /// Check for pending "fresh" data that was there already.
    //  offset should be -2 (Y-X) or 2 (X-Y) for host-output buffer select
    nTmp1   = 0;
    offset  = 2 - (CDC_READER(x).xy_select << 2);
    scratch = &dblock_epout[i].bSPARE1;
    if (*(scratch-offset) & EPBCNT_NAK) {
        CDC_READER(x).ct1       = (scratch-offset);
        CDC_READER(x).ct2       = (scratch+offset);
        scratch                 = (ot_u8*)&USB_HANDLE(x).in_xbuf;
        CDC_READER(x).epcursor  = *(ot_u8**)(scratch-offset);
        CDC_READER(x).ep2       = *(ot_u8**)(scratch+offset);
        nTmp1                   = 1;
    }
    
    /// Data is available
    /// Hold in a loop until data amount is written into control block.
    /// The buffer function inside will reduce CDC_READER(x).rxbytes by the
    /// number of bytes loaded.
    if (nTmp1) {
        do {
            nTmp1 = *CDC_READER(x).ct1;
        } while (nTmp1 == 0);

        sub_buffer_firedrill(nTmp1, x, CDC_READER(x).epcursor);
    }

    /// Data reception is not finished (and it may not have actually started)
    if (CDC_READER(x).rxbytes != 0) {
        __bis_SR_register(bGIE);
        return kUSBCDC_receiveStarted;
    }

    /// Receive operation complete, or killed.
    usbcdc_rxdata_EXIT:
    CDC_READER(x).qbuf = NULL;
    __bis_SR_register(bGIE);

    //USBCDCEVT_RXDONE(x);
    return kUSBCDC_receiveCompleted;
}



//this function is used only by USB interrupt.
//It fills user queue with received data
void usbcdc_transfer_host2buf (ot_u8 x) {
    ot_u8*  pEP1;
    ot_u8*  scratch;
    ot_u8   nTmp1;
    ot_u8   i;
    ot_int  offset;
    //ot_u8 bWakeUp = FALSE;                                                   //per default we do not wake up after interrupt

    if (CDC_READER(x).rxbytes == 0){     //do we have somtething to receive?
        CDC_READER(x).qbuf = NULL;            //no more receiving pending
        return ;    //(bWakeUp);
    }

    //No data to receive...
    i = USB_HANDLE(x).edb_index;
    if (!((dblock_epout[i].bEPBCTX | dblock_epout[i].bEPBCTY) & EPBCNT_NAK)){
        return ;    //(bWakeUp);
    }
    
    //offset should be -2 (Y) or 2 (X) for host-output endpoint buffer
    offset              = 2 - (CDC_READER(x).xy_select << 2);
    scratch             = &dblock_epout[i].bSPARE1;
    CDC_READER(x).ct1   = (scratch - offset);
    CDC_READER(x).ct2   = (scratch + offset);
    scratch             = (ot_u8*)&USB_HANDLE(x).in_xbuf;
    pEP1                = *(ot_u8**)(scratch-offset);
    CDC_READER(x).ep2   = *(ot_u8**)(scratch+offset);
    
    //how many byte we can get from one endpoint buffer
    nTmp1 = *CDC_READER(x).ct1;
    sub_buffer_firedrill(nTmp1, x, pEP1);
    
    //if the Receive opereation is completed
    if (CDC_READER(x).rxbytes == 0) {         
        CDC_READER(x).qbuf = NULL; 
        USBCDCEVT_RXDONE(x);    //bWakeUp |= USBCDCEVT_RXDONE(x);

        // if read data still not available in the EP
        if (CDC_READER(x).epbytes) { 
        	USBCDCEVT_RXDETECT(x);   //bWakeUp |= USBCDCEVT_RXDETECT(x);
        }
    }
    
    //return (bWakeUp);
}




//helper for USB interrupt handler
BOOL usbcdc_rx_isactive (ot_u8 x) {
    return (CDC_READER(x).qbuf != NULL);
}




/*
 * Aborts an active receive operation on interface x.
 * Returns the number of bytes that were received and transferred
 * to the data location established for this receive operation.
 */
ot_u8 usbcdc_rxabort (ot_u16* size, ot_u8 x) {
    ot_u16 bGIE;
    bGIE  = (__get_SR_register() & GIE);
    __disable_interrupt();

    //set received bytes count to 0
    *size = 0;

    //is receive operation underway?
    //how many bytes are already received?
    if (CDC_READER(x).qbuf != NULL) {
        CDC_READER(x).qbuf      = NULL;
        *size                   = CDC_READER(x).rxsize - CDC_READER(x).rxbytes;
        CDC_READER(x).rxbytes   = 0;
        CDC_READER(x).epbytes   = 0;
    }

    //restore interrupt status
    __bis_SR_register(bGIE);
    return kUSB_succeed;
}




/*
 * This function rejects payload data that has been received from the host.
 */
ot_u8 usbcdc_rejectdata (ot_u8 x) {
    ot_u16 bGIE;

    //do not access USB memory if suspended (PLL off). It may produce BUS_ERROR
    if (usbctl.status & USB_STATUS_SUSPENDED) {
        return kUSBCDC_busNotAvailable;
    }

    bGIE = (__get_SR_register() & GIE);
    __disable_interrupt();

    //Is receive operation underway?
    //- do not flush buffers if any operation still active.
    if (CDC_READER(x).qbuf == NULL) {
        ot_u8 i, xnak, ynak;
        
        //Check naks, and switch current buffer if ONLY ONE is full
        i                           = USB_HANDLE(x).edb_index;
        xnak                        = dblock_epout[i].bEPBCTX & EPBCNT_NAK;
        ynak                        = dblock_epout[i].bEPBCTY & EPBCNT_NAK;
        CDC_READER(x).xy_select    ^= ((xnak ^ ynak) != 0);

        // Flush buffers
        dblock_epout[i].bEPBCTX     = 0;
        dblock_epout[i].bEPBCTY     = 0;
        CDC_READER(x).epbytes       = 0;
    }

    __bis_SR_register(bGIE);
    return (kUSB_succeed);
}




/*
 * This function indicates the status of the itnerface x.
 * If a send operation is active for this interface,
 * the function also returns the number of bytes that have been transmitted to the host.
 * If a receiver operation is active for this interface, the function also returns
 * the number of bytes that have been received from the host and are waiting at the assigned address.
 *
 * returns kUSBCDC_waitingForSend (indicates that a call to USBCDC_SendData()
 * has been made, for which data transfer has not been completed)
 *
 * returns kUSBCDC_waitingForReceive (indicates that a receive operation
 * has been initiated, but not all data has yet been received)
 *
 * returns kUSBCDC_dataWaiting (indicates that data has been received
 * from the host, waiting in the USB receive buffers)
 */
ot_u8 usbcdc_intf_status (ot_u8 x, ot_u16* bytesSent, ot_u16* bytesReceived) {
    ot_u16  bGIE;
    ot_u8   ret;
    ot_u8   i;

    *bytesSent      = 0;
    *bytesReceived  = 0;
    ret             = 0;

    bGIE  = (__get_SR_register() & GIE);
    __disable_interrupt();

    //Is send operation underway?
    if (CDC_WRITER(x).txbytes != 0){
        ret        |= kUSBCDC_waitingForSend;
        *bytesSent  = CDC_WRITER(x).txsize - CDC_WRITER(x).txbytes;
    }

    //Is receive operation underway?
    if (CDC_READER(x).qbuf != NULL) {
        ret            |= kUSBCDC_waitingForReceive;
        *bytesReceived  = CDC_READER(x).rxsize - CDC_READER(x).rxbytes;
    }
    
    //kUSBCDC_dataWaiting = 0x04, EPBCNT_NAK = 0x80;
    i    = USB_HANDLE(x).edb_index;
    ret |= ((dblock_epout[i].bEPBCTX | dblock_epout[i].bEPBCTY) & EPBCNT_NAK) >> 5;

    // if USB is not enumerated (or it is suspended), return kUSBCDC_busNotAvailable
    ret  = (usbctl.status == USB_STATUS_ENUMERATED) ? ret : kUSBCDC_busNotAvailable;
    
    __bis_SR_register(bGIE);
    __no_operation();
    return ret;
}




/* Returns how many bytes are in the buffer are received and ready to be read.
 */
ot_u8 usbcdc_bytes_in_buffer (ot_u8 x) {
    ot_u16  bGIE;
    ot_u8*  extra_buf;
    ot_u8   bTmp1;
    ot_u8   num_bufs;

    bGIE  = (__get_SR_register() & GIE);
    __disable_interrupt();
    
    bTmp1 = 0;
    if (usbctl.status == USB_STATUS_ENUMERATED) {
        //If a RX operation is underway, part of data may was read of the OEP buffer
        if (CDC_READER(x).epbytes > 0) {
            bTmp1       = CDC_READER(x).epbytes;
            extra_buf   = CDC_READER(x).ct2;
            num_bufs    = 1;
        }
        else {
            ot_u8 i;
            i           = USB_HANDLE(x).edb_index;
            extra_buf   = &dblock_epout[i].bEPBCTX;
            num_bufs    = 2;
        }
        
        do {
            if (*extra_buf & EPBCNT_NAK) {
                bTmp1 += *extra_buf & ~EPBCNT_NAK;
            }
            extra_buf += 4;
        } while (--num_bufs != 0);
    }
    
//    if (usbctl.fn_suspended || (usbctl.enum_status != ENUMERATION_COMPLETE)) {
//    }
//
//    //If a RX operation is underway, part of data may was read of the OEP buffer
//    else if (CDC_READER(x).epbytes > 0) {
//        bTmp1 = CDC_READER(x).epbytes;
//
//        //the next buffer has a valid data packet
//        if (*CDC_READER(x).ct2 & EPBCNT_NAK) {
//            bTmp1 += *CDC_READER(x).ct2 & 0x7F;
//        }
//    } 
//    else {
//        //this buffer has a valid data packet
//        if (dblock_epout[i].bEPBCTX & EPBCNT_NAK){
//            bTmp1 = dblock_epout[i].bEPBCTX & 0x7F;
//        }
//        //this buffer has a valid data packet
//        if (dblock_epout[i].bEPBCTY & EPBCNT_NAK){
//            bTmp1 += dblock_epout[i].bEPBCTY & 0x7F;
//        }
//    }

    __bis_SR_register(bGIE);
    return (bTmp1);
}






//----------------------------------------------------------------------------
//Line Coding Structure
//dwDTERate     | 4 | Data terminal rate, in bits per second
//bCharFormat   | 1 | Stop bits, 0 = 1 Stop bit, 1 = 1,5 Stop bits, 2 = 2 Stop bits
//bParityType   | 1 | Parity, 0 = None, 1 = Odd, 2 = Even, 3= Mark, 4 = Space
//bDataBits     | 1 | Data bits (5,6,7,8,16)
//----------------------------------------------------------------------------
CMD_RETURN usbcdccmd_get_linecoding (void) {
/// Copy the seven bytes from the CDC Control Field to the Return Data Buffer.
/// The CDC Control field is aligned to match the USB spec, here.

	platform_memcpy(usbctl.response, (ot_u8*)&CDC_CTRLER(dblock_setup.wIndex), 7);

    //usbctl.response[0] = CdcControl[CDCINTF(dblock_setup.wIndex)].lBaudrate;
    //usbctl.response[1] = CdcControl[CDCINTF(dblock_setup.wIndex)].lBaudrate >> 8;
    //usbctl.response[2] = CdcControl[CDCINTF(dblock_setup.wIndex)].lBaudrate >> 16;
    //usbctl.response[3] = CdcControl[CDCINTF(dblock_setup.wIndex)].lBaudrate >> 24;
    //usbctl.response[4] = cdc.ctrler[CDCINTF(dblock_setup.wIndex)].bStopBits;          //Stop bits = 1
    //usbctl.response[5] = cdc.ctrler[CDCINTF(dblock_setup.wIndex)].bParity;            //No Parity
    //usbctl.response[6] = cdc.ctrler[CDCINTF(dblock_setup.wIndex)].bDataBits;          //Data bits = 8

    usbctl.bytes_ep0in   = 7;               //amount of data to be send over EP0 to host
    usbcmd_tx_ep0((ot_u8*)&usbctl.response[0]);              //send data to host

    //return (FALSE);
}




CMD_RETURN usbcdccmd_set_linecoding (void) {
    usbcmd_rx_ep0((ot_u8*)&usbctl.request);            //receive data over EP0 from Host
    //return (FALSE);
}




CMD_RETURN usbcdccmd_set_ctlline (void) {
    usbcdcevt_set_ctlline((ot_u8)dblock_setup.wIndex, (ot_u8)dblock_setup.wValue);
    usbcdc_activate_linecoding();  //(ot_u8)dblock_setup.wIndex, (ot_u8)dblock_setup.wValue);
    
    //Send Zero Length Packet for status stage
    usbcmd_txzlp_ep0();
}




void usbcdc_activate_linecoding (void) {
    CDC_CTRLER(dblock_setup.wIndex).lBaudrate = *(ot_u32*)usbctl.request;

    usbcdcevt_set_linecoding(dblock_setup.wIndex,
                                    CDC_CTRLER(dblock_setup.wIndex).lBaudrate);
}



#endif

