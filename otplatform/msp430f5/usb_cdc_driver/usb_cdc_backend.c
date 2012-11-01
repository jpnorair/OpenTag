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
  * @version    R100
  * @date       27 Oct 2012
  * @brief      CDC DLL for MSP430F5-USB, optimized by JP Norair for OpenTag
  * @ingroup    MSP430F5_USB_CDC
  *
  * This is the CDC driver adapted by JP Norair from the official TI USB stack
  * code, and targeted for OpenTag.  In this file, there is nothing especially
  * dependent on OpenTag, however there are four references:
  * <LI> platform_memcpy() is used.  It has the same interface as normal 
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
  * RSTO        2008/09/19  Changed USBCDC_sendData to send more then 64bytes
  * RSTO        2008/12/23  enhancements of CDC API
  * RSTO        2008/05/19  updated USBCDC_intfStatus()
  * RSTO        2009/05/26  added USBCDC_bytesInUSBBuffer()
  * RSTO        2009/05/28  changed USBCDC_sendData()
  * RSTO        2009/07/17  updated USBCDC_bytesInUSBBuffer()
  * RSTO        2009/10/21  move __disable_interrupt() before
  *                             checking for suspend
  * MSP,Biju    2009/12/28  Fix for the bug "Download speed is slow"
  * ---         ----------  ---------------------------------------------------
  * JPN         2012/05/01  Code size optimizations for single-CDC interface
  * JPN         2012/05/07  Some state numeric values changed for optimization
  * JPN         2012/10/26  System calls inserted into event callbacks, so the
  *                             logic to wake-up MCU in the ISR is removed.
  * JPN         2012/10/30  Buffer rotation optimized: pointer rotation now, 
  *                             instead of repetitive flag change + if-else
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


cdc_struct   cdc;



#if (USBEVT_MASK & USBEVT_TXCOMPLETE)
#   define USBCDC_HANDLE_TXCOMPLETE(INTFNUM)    USBCDC_handleSendCompleted(INTFNUM)
#else
#   define USBCDC_HANDLE_TXCOMPLETE(INTFNUM)    while(0)
#endif

#if (USBEVT_MASK & USBEVT_RXCOMPLETE)
#   define USBCDC_HANDLE_RXCOMPLETE(INTFNUM)    USBCDC_handleReceiveCompleted(INTFNUM)
#else
#   define USBCDC_HANDLE_RXCOMPLETE(INTFNUM)    while(0)
#endif

#if (USBEVT_MASK & USBEVT_RXBUFFERED)
#   define USBCDC_HANDLE_RXBUFFERED(INTFNUM)    USBCDC_handleDataReceived(INTFNUM)
#else
#   define USBCDC_HANDLE_RXBUFFERED(INTFNUM)    while(0)
#endif






//extern ot_u16 wUsbEventMask;


///@note memcpy has been modified to use platform_memcpy, which is very similar
///      in design to the memcpy functions implemented in USB_Common/dma.c.
///      platform_memcpy() does not return dest, but none of the usage in this
///      file actually assign the return anyway.

//function pointers
//extern void *(*USB_TX_memcpy)(void * dest, const void * source, size_t count);
//extern void *(*USB_RX_memcpy)(void * dest, const void * source, size_t count);

#include "OT_platform.h"
#define USB_TX_memcpy(DST, SRC, SIZE)   platform_memcpy((ot_u8*)DST, (ot_u8*)SRC, (ot_int)SIZE)
#define USB_RX_memcpy(DST, SRC, SIZE)   platform_memcpy((ot_u8*)DST, (ot_u8*)SRC, (ot_int)SIZE)


/*----------------------------------------------------------------------------+
 | Global Variables                                                            |
 +----------------------------------------------------------------------------*/

//extern __no_init tEDB __data16 tInputEndPointDescriptorBlock[];
//extern __no_init tEDB __data16 tOutputEndPointDescriptorBlock[];


void CdcResetData () {
	/// Wipe all interfaces
	platform_memset((ot_u8*)&cdc, 0, sizeof(cdc_struct));
    //platform_memset((ot_u8*)cdc.writer, 0, sizeof(cdc.writer));
    //platform_memset((ot_u8*)cdc.reader, 0, sizeof(cdc.reader));
    //platform_memset((ot_u8*)cdc.ctrler, 0, sizeof(cdc.ctrler));

    /// Set data bits to 8 for each interface
#   if (NONCOMP_NUM_USB_INTERFACES == 1)
        cdc.ctrler[0].bDataBits = 8;
#   elif (NONCOMP_NUM_USB_INTERFACES == 2)
        cdc.ctrler[0].bDataBits = 8;
        cdc.ctrler[1].bDataBits = 8;
#	elif (NONCOMP_NUM_USB_INTERFACES == 3)
        cdc.ctrler[0].bDataBits = 8;
        cdc.ctrler[1].bDataBits = 8;
        cdc.ctrler[2].bDataBits = 8;
#   else
    {   ot_u8 i = NONCOMP_NUM_USB_INTERFACES;
	    while (i != 0)
            cdc.ctrler[--i].bDataBits = 8;
    }
#   endif
}



/** Sends data over interface intfNum, of size size and starting at address data.
  * Returns: kUSBCDC_sendStarted, kUSBCDC_sendComplete, kUSBCDC_intfBusyError
  */
ot_u8 USBCDC_sendData (const ot_u8* data, ot_u16 size, ot_u8 intfNum) {
    unsigned short bGIE;

    // Do not access USB memory when size is zero or device is suspended.
    // It may produce a bus error if you do.
    if (size == 0) {
        return (kUSBCDC_generalError);
    }
    if ((bFunctionSuspended) || (bEnumerationStatus != ENUMERATION_COMPLETE)) {
        return (kUSBCDC_busNotAvailable);
    }

    bGIE  = (__get_SR_register() & GIE);        //save interrupt status
    __disable_interrupt();                      //atomic operation - disable interrupts

    if (cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft != 0) {
        //the USB still sends previous data, we have to wait
        __bis_SR_register(bGIE);
        return (kUSBCDC_intfBusyError);
    }

    //This function generate the USB interrupt. The data will be sent out from interrupt
    cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSend       = size;
    cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft   = size;
    cdc.writer[INTFNUM_OFFSET(intfNum)].pUsbBufferToSend      = data;

    //trigger Endpoint Interrupt - to start send operation  ///@todo appears broken
    USBIEPIFG |= 1 << (stUsbHandle[intfNum].edb_Index + 1);     //IEPIFGx;

    __bis_SR_register(bGIE);
    return (kUSBCDC_sendStarted);
}





#define EP_MAX_PACKET_SIZE_CDC      0x40

//this function is used only by USB interrupt
void CdcToHostFromBuffer (ot_u8 intfNum) {
    ot_u8 byte_count, nTmp2;
    ot_u8 * pEP1;
    ot_u8 * pEP2;
    ot_u8 * pCT1;
    ot_u8 * pCT2;
    //ot_u8 bWakeUp = FALSE;                                                   //TRUE for wake up after interrupt
    ot_u8 edbIndex;

    edbIndex = stUsbHandle[intfNum].edb_Index;

    if (cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft == 0){    //do we have somtething to send?
        if (!cdc.writer[INTFNUM_OFFSET(intfNum)].bZeroPacketSent){        //zero packet was not yet sent
            cdc.writer[INTFNUM_OFFSET(intfNum)].bZeroPacketSent = TRUE;

            if (cdc.writer[INTFNUM_OFFSET(intfNum)].last_ByteSend == EP_MAX_PACKET_SIZE_CDC) {
            
                ///@todo this block should be managed with a buffer pointer
                if (cdc.writer[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY == X_BUFFER){
                    if (tInputEndPointDescriptorBlock[edbIndex].bEPBCTX & EPBCNT_NAK) {
                        tInputEndPointDescriptorBlock[edbIndex].bEPBCTX = 0;
                        cdc.writer[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY = Y_BUFFER;                                     //switch buffer
                    }
                }
                else {
                    if (tInputEndPointDescriptorBlock[edbIndex].bEPBCTY & EPBCNT_NAK) {
                        tInputEndPointDescriptorBlock[edbIndex].bEPBCTY = 0;
                        cdc.writer[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY = X_BUFFER;                                     //switch buffer
                    }
                }
                
                
            }

            cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSend = 0;      //nothing to send
            USBCDC_HANDLE_TXCOMPLETE(intfNum);  //call event callback function
            //bWakeUp |= USBCDC_HANDLE_TXCOMPLETE(intfNum);  //call event callback function
        } //if (!bSentZeroPacket)

        //return (bWakeUp);
    }

    cdc.writer[INTFNUM_OFFSET(intfNum)].bZeroPacketSent = FALSE;          //zero packet will be not sent: we have data

    ///@todo this block should be managed with a buffer pointer
    if (cdc.writer[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY == X_BUFFER){
        //this is the active EP buffer
        pEP1 = (ot_u8*)stUsbHandle[intfNum].iep_X_Buffer;
        pCT1 = &tInputEndPointDescriptorBlock[edbIndex].bEPBCTX;

        //second EP buffer
        pEP2 = (ot_u8*)stUsbHandle[intfNum].iep_Y_Buffer;
        pCT2 = &tInputEndPointDescriptorBlock[edbIndex].bEPBCTY;
    }
    else {
        //this is the active EP buffer
        pEP1 = (ot_u8*)stUsbHandle[intfNum].iep_Y_Buffer;
        pCT1 = &tInputEndPointDescriptorBlock[edbIndex].bEPBCTY;

        //second EP buffer
        pEP2 = (ot_u8*)stUsbHandle[intfNum].iep_X_Buffer;
        pCT2 = &tInputEndPointDescriptorBlock[edbIndex].bEPBCTX;
    }



    //how many byte we can send over one endpoint buffer
    byte_count = (cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft > EP_MAX_PACKET_SIZE_CDC) ? \
                    EP_MAX_PACKET_SIZE_CDC : cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft;
    nTmp2 = *pCT1;

    if (nTmp2 & EPBCNT_NAK){
        //copy data into IEP3 X or Y buffer
        USB_TX_memcpy(pEP1, cdc.writer[INTFNUM_OFFSET(intfNum)].pUsbBufferToSend, byte_count);

        *pCT1 = byte_count;      //Set counter for usb In-Transaction

        //switch buffer
        cdc.writer[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY = \
                (cdc.writer[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY + 1) & 0x01;

        cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft -= byte_count;
        cdc.writer[INTFNUM_OFFSET(intfNum)].pUsbBufferToSend += byte_count;       //move buffer pointer
        cdc.writer[INTFNUM_OFFSET(intfNum)].last_ByteSend = byte_count;

        //try to send data over second buffer
        nTmp2 = *pCT2;
        if ((cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft > 0) &&      //do we have more data to send?
            (nTmp2 & EPBCNT_NAK)) {                                                 //if the second buffer is free?
            //how many byte we can send over one endpoint buffer
            byte_count = (cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft > EP_MAX_PACKET_SIZE_CDC) ? \
                    EP_MAX_PACKET_SIZE_CDC : cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft;

            //copy data into IEP3 X or Y buffer
            USB_TX_memcpy(pEP2, cdc.writer[INTFNUM_OFFSET(intfNum)].pUsbBufferToSend, byte_count);

            *pCT2 = byte_count;   //Set counter for usb In-Transaction

            //switch buffer
            cdc.writer[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY = \
                (cdc.writer[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY + 1) & 0x01;

            cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft -= byte_count;
            cdc.writer[INTFNUM_OFFSET(intfNum)].pUsbBufferToSend += byte_count;  //move buffer pointer
            cdc.writer[INTFNUM_OFFSET(intfNum)].last_ByteSend = byte_count;
        }
    }
    //return (bWakeUp);
}




/*
 * Aborts an active send operation on interface intfNum.
 * Returns the number of bytes that were sent prior to the abort, in size.
 */
ot_u8 USBCDC_abortSend (ot_u16* size, ot_u8 intfNum) {
    unsigned short bGIE;

    bGIE  = (__get_SR_register() & GIE);        //save interrupt status
    __disable_interrupt();                      //disable interrupts - atomic operation

    *size = (cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSend - \
               cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft);

    cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSend = 0;
    cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft = 0;

    __bis_SR_register(bGIE);     //restore interrupt status
    return (kUSB_succeed);
}





//This function copies data from OUT endpoint into user's buffer
//Arguments:
//pEP - pointer to EP to copy from
//pCT - pointer to pCT control reg
//
void CopyUsbToBuff (ot_u8* pEP, ot_u8* pCT, ot_u8 intfNum) {
    ot_u8 nCount;

    //how many byte we can get from one endpoint buffer
    nCount = (cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft > cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp) ? \
            cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp : cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft;

    USB_RX_memcpy(cdc.reader[INTFNUM_OFFSET(intfNum)].pUserBuffer, pEP, nCount);   //copy data from OEP3 X or Y buffer
    cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft -= nCount;
    cdc.reader[INTFNUM_OFFSET(intfNum)].pUserBuffer += nCount;                     //move buffer pointer
    //to read rest of data next time from this place

    //all bytes are copied from receive buffer?
    if (nCount == cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp) {
        //switch current buffer
        cdc.reader[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY = \
            (cdc.reader[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY + 1) & 0x01;

        cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp = 0;

        //clear NAK, EP ready to receive data
        *pCT = 0x00;
    }
    else {
        cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp -= nCount;
        cdc.reader[INTFNUM_OFFSET(intfNum)].pCurrentEpPos = pEP + nCount;
    }
}





/*
 * Receives data over interface intfNum, of size size, into memory starting at address data.
 * Returns:
 *  kUSBCDC_receiveStarted  if the receiving process started.
 *  kUSBCDC_receiveCompleted  all requested date are received.
 *  kUSBCDC_receiveInProgress  previous receive opereation is in progress. The requested receive operation can be not started.
 *  kUSBCDC_generalError  error occurred.
 */
ot_u8 USBCDC_receiveData (ot_u8* data, ot_u16 size, ot_u8 intfNum) {
    ot_u8 nTmp1;
    ot_u8 edbIndex;
    unsigned short bGIE;

    edbIndex = stUsbHandle[intfNum].edb_Index;

    //Protections to prevent accessing USB with size = 0, NULL buffer, USB
    //Suspended, or USB not enumerated
    if ((size == 0) || (data == NULL)) {
        return (kUSBCDC_generalError);
    }
    if ((bFunctionSuspended) || (bEnumerationStatus != ENUMERATION_COMPLETE)){
        return (kUSBCDC_busNotAvailable);
    }

    bGIE  = (__get_SR_register() & GIE);
    __disable_interrupt();          //atomic operation - disable interrupts


    //receive process already started
    if (cdc.reader[INTFNUM_OFFSET(intfNum)].pUserBuffer != NULL){
        __bis_SR_register(bGIE);
        return (kUSBCDC_intfBusyError);
    }

    // Set RX Buffer constraints
    cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceive = size;            //bytes to receive
    cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft = size;        //left bytes to receive
    cdc.reader[INTFNUM_OFFSET(intfNum)].pUserBuffer = data;                //set user receive buffer


    //read rest of data from buffer, if any
    if (cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp > 0) {
        //copy data from pEP-endpoint into User's buffer
        CopyUsbToBuff(cdc.reader[INTFNUM_OFFSET(intfNum)].pCurrentEpPos, cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1, intfNum);

        if (cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft == 0){ //the Receive opereation is completed
            goto USBCDC_receiveData_EXIT;
            //cdc.reader[INTFNUM_OFFSET(intfNum)].pUserBuffer = NULL;        //no more receiving pending
            //USBCDC_HANDLE_RXCOMPLETE(intfNum);                     //call event handler in interrupt context
            //__bis_SR_register(bGIE);                                        //restore interrupt status
            //return (kUSBCDC_receiveCompleted);                              //receive completed
        }

        //check other EP buffer for data - exchange pCT1 with pCT2
        ///@todo this block should be managed with a buffer pointer
        if (cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1 == &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX){
            cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY;
            cdc.reader[INTFNUM_OFFSET(intfNum)].pCurrentEpPos = (ot_u8*)stUsbHandle[intfNum].oep_Y_Buffer;
        }
        else {
            cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX;
            cdc.reader[INTFNUM_OFFSET(intfNum)].pCurrentEpPos = (ot_u8*)stUsbHandle[intfNum].oep_X_Buffer;
        }



        nTmp1 = *cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1;
        //try read data from second buffer
        if (nTmp1 & EPBCNT_NAK){                                            //if the second buffer has received data?
            nTmp1 = nTmp1 & 0x7f;                                           //clear NAK bit
            cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp = nTmp1;        //holds how many valid bytes in the EP buffer
            CopyUsbToBuff(cdc.reader[INTFNUM_OFFSET(intfNum)].pCurrentEpPos,cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1, intfNum);
        }

        if (cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft == 0){ //the Receive opereation is completed
            goto USBCDC_receiveData_EXIT;
            //cdc.reader[INTFNUM_OFFSET(intfNum)].pUserBuffer = NULL;        //no more receiving pending
            //USBCDC_HANDLE_RXCOMPLETE(intfNum);                     //call event handler in interrupt context
            //__bis_SR_register(bGIE);                                        //restore interrupt status
            //return (kUSBCDC_receiveCompleted);                              //receive completed
        }
    } //read rest of data from buffer, if any

    //read 'fresh' data, if available
    nTmp1 = 0;
    
    ///@todo this block should be managed with a buffer pointer
    if (cdc.reader[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY == X_BUFFER){ //this is current buffer
        if (tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX & EPBCNT_NAK){ //this buffer has a valid data packet
            //this is the active EP buffer
            //pEP1
            cdc.reader[INTFNUM_OFFSET(intfNum)].pCurrentEpPos = (ot_u8*)stUsbHandle[intfNum].oep_X_Buffer;
            cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX;

            //second EP buffer
            cdc.reader[INTFNUM_OFFSET(intfNum)].pEP2 = (ot_u8*)stUsbHandle[intfNum].oep_Y_Buffer;
            cdc.reader[INTFNUM_OFFSET(intfNum)].pCT2 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY;
            nTmp1 = 1;                 //indicate that data is available
        }
    }
    else {                                                                //Y_BUFFER
        if (tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY & EPBCNT_NAK){
            //this is the active EP buffer
            cdc.reader[INTFNUM_OFFSET(intfNum)].pCurrentEpPos = (ot_u8*)stUsbHandle[intfNum].oep_Y_Buffer;
            cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY;

            //second EP buffer
            cdc.reader[INTFNUM_OFFSET(intfNum)].pEP2 = (ot_u8*)stUsbHandle[intfNum].oep_X_Buffer;
            cdc.reader[INTFNUM_OFFSET(intfNum)].pCT2 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX;
            nTmp1 = 1;          //indicate that data is available
        }
    }
    
    

    if (nTmp1){
        //how many byte we can get from one endpoint buffer
        nTmp1 = *cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1;
        while (nTmp1 == 0) {
            nTmp1 = *cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1;
        }

        if (nTmp1 & EPBCNT_NAK) {
            nTmp1 = nTmp1 & 0x7f;                                           //clear NAK bit
            cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp = nTmp1;        //holds how many valid bytes in the EP buffer

            CopyUsbToBuff(cdc.reader[INTFNUM_OFFSET(intfNum)].pCurrentEpPos, cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1, intfNum);

            nTmp1 = *cdc.reader[INTFNUM_OFFSET(intfNum)].pCT2;
            //try read data from second buffer

            //do we have more data to send?  if the second buffer has received data?
            if ((cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft > 0) &&
                (nTmp1 & EPBCNT_NAK)) {
                nTmp1 = nTmp1 & 0x7f;                                       //clear NAK bit
                cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp = nTmp1;    //holds how many valid bytes in the EP buffer

                CopyUsbToBuff(cdc.reader[INTFNUM_OFFSET(intfNum)].pEP2, cdc.reader[INTFNUM_OFFSET(intfNum)].pCT2, intfNum);

                cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1 = cdc.reader[INTFNUM_OFFSET(intfNum)].pCT2;
            }
        }
    }

    if (cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft == 0){     //the Receive opereation is completed
        goto USBCDC_receiveData_EXIT;
        //cdc.reader[INTFNUM_OFFSET(intfNum)].pUserBuffer = NULL;            //no more receiving pending
        //USBCDC_HANDLE_RXCOMPLETE(intfNum);                         //call event handler in interrupt context
        //__bis_SR_register(bGIE);                                            //restore interrupt status
        //return (kUSBCDC_receiveCompleted);
    }

    //interrupts enable
    __bis_SR_register(bGIE);                                                //restore interrupt status
    return (kUSBCDC_receiveStarted);
    
    
    //Intermediate Output Stage
    USBCDC_receiveData_EXIT:
    cdc.reader[INTFNUM_OFFSET(intfNum)].pUserBuffer = NULL;    //no more receiving pending
    USBCDC_HANDLE_RXCOMPLETE(intfNum);                          //call event handler in interrupt context
    __bis_SR_register(bGIE);                                    //restore interrupt status
    return (kUSBCDC_receiveCompleted);
}




//this function is used only by USB interrupt.
//It fills user receiving buffer with received data
void CdcToBufferFromHost (ot_u8 intfNum) {
    ot_u8 * pEP1;
    ot_u8 nTmp1;
    //ot_u8 bWakeUp = FALSE;                                                   //per default we do not wake up after interrupt

    ot_u8 edbIndex;

    edbIndex = stUsbHandle[intfNum].edb_Index;

    if (cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft == 0){     //do we have somtething to receive?
        cdc.reader[INTFNUM_OFFSET(intfNum)].pUserBuffer = NULL;            //no more receiving pending
        return ;//(bWakeUp);
    }

    //No data to receive...
    if (!((tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX | tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY) & 0x80)){
        return ;//(bWakeUp);
    }


    ///@todo this block should be managed with a buffer pointer
    if (cdc.reader[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY == X_BUFFER){ //X is current buffer
        //this is the active EP buffer
        pEP1 = (ot_u8*)stUsbHandle[intfNum].oep_X_Buffer;
        cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX;

        //second EP buffer
        cdc.reader[INTFNUM_OFFSET(intfNum)].pEP2 = (ot_u8*)stUsbHandle[intfNum].oep_Y_Buffer;
        cdc.reader[INTFNUM_OFFSET(intfNum)].pCT2 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY;
    }
    else {
        //this is the active EP buffer
        pEP1 = (ot_u8*)stUsbHandle[intfNum].oep_Y_Buffer;
        cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY;

        //second EP buffer
        cdc.reader[INTFNUM_OFFSET(intfNum)].pEP2 = (ot_u8*)stUsbHandle[intfNum].oep_X_Buffer;
        cdc.reader[INTFNUM_OFFSET(intfNum)].pCT2 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX;
    }
    
    

    //how many byte we can get from one endpoint buffer
    nTmp1 = *cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1;

    if (nTmp1 & EPBCNT_NAK){
        nTmp1 = nTmp1 & 0x7f;                                                   //clear NAK bit
        cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp = nTmp1;                //holds how many valid bytes in the EP buffer

        CopyUsbToBuff(pEP1, cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1, intfNum);

        nTmp1 = *cdc.reader[INTFNUM_OFFSET(intfNum)].pCT2;
        //try read data from second buffer
        if ((cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft > 0) &&   //do we have more data to send?
            (nTmp1 & EPBCNT_NAK)){                                              //if the second buffer has received data?
            nTmp1 = nTmp1 & 0x7f;                                               //clear NAK bit
            cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp = nTmp1;            //holds how many valid bytes in the EP buffer

            CopyUsbToBuff(cdc.reader[INTFNUM_OFFSET(intfNum)].pEP2,cdc.reader[INTFNUM_OFFSET(intfNum)].pCT2, intfNum);

            cdc.reader[INTFNUM_OFFSET(intfNum)].pCT1 = cdc.reader[INTFNUM_OFFSET(intfNum)].pCT2;
        }
    }

    if (cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft == 0){         //the Receive opereation is completed
        cdc.reader[INTFNUM_OFFSET(intfNum)].pUserBuffer = NULL;                //no more receiving pending
        //bWakeUp |= USBCDC_HANDLE_RXCOMPLETE(intfNum);
        USBCDC_HANDLE_RXCOMPLETE(intfNum);

        if (cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp){                   //Is not read data still available in the EP?
            //bWakeUp |= USBCDC_HANDLE_RXBUFFERED(intfNum);
        	USBCDC_HANDLE_RXBUFFERED(intfNum);
        }
    }
    //return (bWakeUp);
}




//helper for USB interrupt handler
BOOL CdcIsReceiveInProgress (ot_u8 intfNum) {
    return (cdc.reader[INTFNUM_OFFSET(intfNum)].pUserBuffer != NULL);
}




/*
 * Aborts an active receive operation on interface intfNum.
 * Returns the number of bytes that were received and transferred
 * to the data location established for this receive operation.
 */
ot_u8 USBCDC_abortReceive (ot_u16* size, ot_u8 intfNum) {
    //interrupts disable
    unsigned short bGIE;

    bGIE  = (__get_SR_register() & GIE);                                    //save interrupt status
    //atomic operation - disable interrupts
    __disable_interrupt();                                                  //Disable global interrupts

    *size = 0;                                                              //set received bytes count to 0

    //is receive operation underway?
    if (cdc.reader[INTFNUM_OFFSET(intfNum)].pUserBuffer){
        //how many bytes are already received?
        *size = cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceive -
                cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft;

        cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp = 0;
        cdc.reader[INTFNUM_OFFSET(intfNum)].pUserBuffer = NULL;
        cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft = 0;
    }

    //restore interrupt status
    __bis_SR_register(bGIE);                                                //restore interrupt status
    return (kUSB_succeed);
}




/*
 * This function rejects payload data that has been received from the host.
 */
ot_u8 USBCDC_rejectData (ot_u8 intfNum) {
    ot_u8 edbIndex;
    unsigned short bGIE;

    edbIndex = stUsbHandle[intfNum].edb_Index;

    bGIE  = (__get_SR_register() & GIE);                                    //save interrupt status

    //atomic operation - disable interrupts
    __disable_interrupt();                                                  //Disable global interrupts

    //do not access USB memory if suspended (PLL off). It may produce BUS_ERROR
    if (bFunctionSuspended){
        __bis_SR_register(bGIE);                                            //restore interrupt status
        return (kUSBCDC_busNotAvailable);
    }

    //Is receive operation underway?
    //- do not flush buffers if any operation still active.
    if (!cdc.reader[INTFNUM_OFFSET(intfNum)].pUserBuffer){
        ot_u8 tmp1 = tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX & EPBCNT_NAK;
        ot_u8 tmp2 = tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY & EPBCNT_NAK;

        //switch current buffer if any and only ONE of buffers is full
        if (tmp1 ^ tmp2){
            //switch current buffer
            cdc.reader[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY = \
                    (cdc.reader[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY + 1) & 0x01;
        }

        tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX = 0;               //flush buffer X
        tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY = 0;               //flush buffer Y
        cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp = 0;                //indicates that no more data available in the EP
    }

    __bis_SR_register(bGIE);                                                //restore interrupt status
    return (kUSB_succeed);
}




/*
 * This function indicates the status of the itnerface intfNum.
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
ot_u8 USBCDC_intfStatus (ot_u8 intfNum, ot_u16* bytesSent, ot_u16* bytesReceived) {
    ot_u8 ret = 0;
    unsigned short bGIE;
    ot_u8 edbIndex;

    *bytesSent = 0;
    *bytesReceived = 0;

    edbIndex = stUsbHandle[intfNum].edb_Index;

    bGIE  = (__get_SR_register() & GIE);                                    //save interrupt status
    __disable_interrupt();                                                  //disable interrupts - atomic operation

    //Is send operation underway?
    if (cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft != 0){
        ret        |= kUSBCDC_waitingForSend;
        *bytesSent  = cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSend \
                    - cdc.writer[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft;
    }

    //Is receive operation underway?
    if (cdc.reader[INTFNUM_OFFSET(intfNum)].pUserBuffer != NULL){
        ret            |= kUSBCDC_waitingForReceive;
        *bytesReceived  = cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceive \
                        - cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft;
    }
    else {
        //do not access USB memory if suspended (PLL off).
        //It may produce BUS_ERROR
        if (!bFunctionSuspended){
            //any of buffers has a valid data packet
            
            ///@todo this block should be managed with a buffer pointer
            if ((tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX & EPBCNT_NAK) |
                (tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY & EPBCNT_NAK)){
                ret |= kUSBCDC_dataWaiting;
            }
        }
    }

    if ((bFunctionSuspended) || (bEnumerationStatus != ENUMERATION_COMPLETE)) {
        //if suspended or not enumerated - report no other tasks pending
        ret = kUSBCDC_busNotAvailable;
    }

    //restore interrupt status
    __bis_SR_register(bGIE);        //restore interrupt status

    __no_operation();
    return (ret);
}




/* Returns how many bytes are in the buffer are received and ready to be read.
 */
ot_u8 USBCDC_bytesInUSBBuffer (ot_u8 intfNum) {
    ot_u8 bTmp1 = 0;
    ot_u16 bGIE;
    ot_u8 edbIndex;
    ot_u8* extra_buf;
    ot_u8 num_bufs;

    edbIndex = stUsbHandle[intfNum].edb_Index;

    bGIE  = (__get_SR_register() & GIE);    //save interrupt status
    //atomic operation - disable interrupts
    __disable_interrupt();                  //Disable global interrupts

    if (!(bFunctionSuspended) && (bEnumerationStatus == ENUMERATION_COMPLETE)) {
        //If a RX operation is underway, part of data may was read of the OEP buffer
        if (cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp > 0) {
            bTmp1       = cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp;
            extra_buf   = cdc.reader[INTFNUM_OFFSET(intfNum)].pCT2;
            num_bufs    = 1;
        }
        else {
            ///@todo this block should be managed with a buffer pointer
            extra_buf   = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX;
            num_bufs    = 2;
        }
        while (num_bufs != 0) {
            num_bufs--;
            if (*extra_buf & EPBCNT_NAK) {
                bTmp1 += *extra_buf ^ EPBCNT_NAK;
            }
            extra_buf += 4;
        }
    }
    
//    if ((bFunctionSuspended) || (bEnumerationStatus != ENUMERATION_COMPLETE)) {
//    }
//
//    //If a RX operation is underway, part of data may was read of the OEP buffer
//    else if (cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp > 0) {
//        bTmp1 = cdc.reader[INTFNUM_OFFSET(intfNum)].nBytesInEp;
//
//        //the next buffer has a valid data packet
//        if (*cdc.reader[INTFNUM_OFFSET(intfNum)].pCT2 & EPBCNT_NAK) {
//            bTmp1 += *cdc.reader[INTFNUM_OFFSET(intfNum)].pCT2 & 0x7F;
//        }
//    } 
//    else {
//        //this buffer has a valid data packet
//        if (tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX & EPBCNT_NAK){
//            bTmp1 = tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX & 0x7F;
//        }
//        //this buffer has a valid data packet
//        if (tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY & EPBCNT_NAK){
//            bTmp1 += tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY & 0x7F;
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
ot_u8 usbGetLineCoding (void) {
///@todo validate that this works, and then look into combining as a single
///      memcpy call.

    platform_memcpy(&abUsbRequestReturnData[0],
                        (ot_u8*)&cdc.ctrler[INTFNUM_OFFSET(tSetupPacket.wIndex)].lBaudrate,
                        4   );
    //abUsbRequestReturnData[3] = cdc.ctrler[INTFNUM_OFFSET(tSetupPacket.wIndex)].lBaudrate >> 24;
    //abUsbRequestReturnData[2] = cdc.ctrler[INTFNUM_OFFSET(tSetupPacket.wIndex)].lBaudrate >> 16;
    //abUsbRequestReturnData[1] = cdc.ctrler[INTFNUM_OFFSET(tSetupPacket.wIndex)].lBaudrate >> 8;
    //abUsbRequestReturnData[0] = cdc.ctrler[INTFNUM_OFFSET(tSetupPacket.wIndex)].lBaudrate;

    platform_memcpy(&abUsbRequestReturnData[4], 
                    &cdc.ctrler[INTFNUM_OFFSET(tSetupPacket.wIndex)].bStopBits,
                    3   );
    //abUsbRequestReturnData[6] = cdc.ctrler[INTFNUM_OFFSET(tSetupPacket.wIndex)].bDataBits;          //Data bits = 8
    //abUsbRequestReturnData[5] = cdc.ctrler[INTFNUM_OFFSET(tSetupPacket.wIndex)].bParity;            //No Parity
    //abUsbRequestReturnData[4] = cdc.ctrler[INTFNUM_OFFSET(tSetupPacket.wIndex)].bStopBits;          //Stop bits = 1

    wBytesRemainingOnIEP0   = 0x07;               //amount of data to be send over EP0 to host
    usbSendDataPacketOnEP0((ot_u8*)&abUsbRequestReturnData[0]);              //send data to host

    return (FALSE);
}




ot_u8 usbSetLineCoding (void) {
    usbReceiveDataPacketOnEP0((ot_u8*)&abUsbRequestIncomingData);            //receive data over EP0 from Host

    return (FALSE);
}




ot_u8 usbSetControlLineState (void) {
	//ot_u8 output;
    //output = 
    USBCDC_handleSetControlLineState((ot_u8)tSetupPacket.wIndex, (ot_u8)tSetupPacket.wValue);
    
    //Send Zero Length Packet for status stage
    usbSendZeroLengthPacketOnIEP0();

    //return output;
    return False;
}




void Handler_SetLineCoding (void) {
    //ot_u8 bWakeUp;
    Fourbytes scratch;
    ot_u8* coded_value;

    //Baudrate Settings
    //cdc.ctrler[INTFNUM_OFFSET(tSetupPacket.wIndex)].lBaudrate = \
                        PLATFORM_ENDIAN32(*(ot_u32*)abUsbRequestIncomingData);

    coded_value			= (ot_u8*)&abUsbRequestIncomingData[3];
    scratch.ubyte[B3]   = *coded_value--;
    scratch.ubyte[B2]   = *coded_value--;
    scratch.ubyte[B1]   = *coded_value--;
    scratch.ubyte[B0]   = *coded_value;

    cdc.ctrler[INTFNUM_OFFSET(tSetupPacket.wIndex)].lBaudrate = scratch.ulong;

    //bWakeUp = 
    USBCDC_handleSetLineCoding(tSetupPacket.wIndex,
                    cdc.ctrler[INTFNUM_OFFSET(tSetupPacket.wIndex)].lBaudrate);

    //return (bWakeUp);
}



#endif

