//(c)2009 by Texas Instruments Incorporated, All Rights Reserved.
/*----------------------------------------------------------------------------+
 |                                                                             |
 |                              Texas Instruments                              |
 |                                                                             |
 |                          MSP430 USB-Example (CDC Driver)                    |
 |                                                                             |
 +-----------------------------------------------------------------------------+
 |  Source: UsbCdc.c, File Version 1.01 2009/12/03                             |
 |  Author: RSTO                                                               |
 |                                                                             |
 |  WHO          WHEN         WHAT                                             |
 |  ---          ----------   ------------------------------------------------ |
 |  RSTO         2008/09/03   born                                             |
 |  RSTO         2008/09/19   Changed USBCDC_sendData to send more then 64bytes|
 |  RSTO         2008/12/23   enhancements of CDC API                          |
 |  RSTO         2008/05/19   updated USBCDC_intfStatus()                      |
 |  RSTO         2009/05/26   added USBCDC_bytesInUSBBuffer()                  |
 |  RSTO         2009/05/28   changed USBCDC_sendData()                        |
 |  RSTO         2009/07/17   updated USBCDC_bytesInUSBBuffer()                |
 |  RSTO         2009/10/21   move __disable_interrupt() before                |
 |                            checking for suspend                             |
 |  MSP,Biju     2009/12/28   Fix for the bug "Download speed is slow"         |
 +----------------------------------------------------------------------------*/
#include "usb_cdc_driver/usb_descriptors.h"
#ifdef _CDC_

#include "usb_cdc_driver/usb_types.h"                //Basic Type declarations
#include "usb_cdc_driver/usb_device.h"
#include "usb_cdc_driver/defMSP430USB.h"
#include "usb_cdc_driver/usb_main.h"                  //USB-specific Data Structures
#include "usb_cdc_driver/usb_cdc_backend.h"


_CdcControl     CdcControl[CDC_NUM_INTERFACES];
_CdcWriteCtrl   CdcWriteCtrl[CDC_NUM_INTERFACES];
_CdcReadCtrl    CdcReadCtrl[CDC_NUM_INTERFACES];


#if (USBEVT_MASK & USBEVT_TXCOMPLETE)
#   define USBCDC_HANDLE_TXCOMPLETE(INTFNUM)    USBCDC_handleSendCompleted(INTFNUM)
#else
#   define USBCDC_HANDLE_TXCOMPLETE(INTFNUM)    0
#endif

#if (USBEVT_MASK & USBEVT_RXCOMPLETE)
#   define USBCDC_HANDLE_RXCOMPLETE(INTFNUM)    USBCDC_handleReceiveCompleted(INTFNUM)
#else
#   define USBCDC_HANDLE_RXCOMPLETE(INTFNUM)    0
#endif

#if (USBEVT_MASK & USBEVT_TXCOMPLETE)
#   define USBCDC_HANDLE_RXBUFFERED(INTFNUM)    USBCDC_handleDataReceived(INTFNUM)
#else
#   define USBCDC_HANDLE_RXBUFFERED(INTFNUM)    0
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

cdc_memwipe(ot_u8* addr, ot_u16 length) {
    while (length != 0) {
        *addr++ = 0;
        length--;
    }
}

void CdcResetData () {
    //indicates which buffer is used by host to transmit data via OUT endpoint3 - X buffer is first
    //CdcReadCtrl[intfIndex].bCurrentBufferXY = X_BUFFER;

    cdc_memwipe((ot_u8*)&CdcWriteCtrl, sizeof(CdcWriteCtrl));
    cdc_memwipe((ot_u8*)&CdcReadCtrl, sizeof(CdcReadCtrl));
    cdc_memwipe((ot_u8*)&CdcControl, sizeof(CdcControl));
   
    CdcControl[0].bDataBits = 8;
}

/*
 * Sends data over interface intfNum, of size size and starting at address data.
 * Returns: kUSBCDC_sendStarted
 *       kUSBCDC_sendComplete
 *       kUSBCDC_intfBusyError
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

    if (CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft != 0) {
        //the USB still sends previous data, we have to wait
        __bis_SR_register(bGIE);
        return (kUSBCDC_intfBusyError);
    }

    //This function generate the USB interrupt. The data will be sent out from interrupt
    CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSend = size;
    CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft = size;
    CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].pUsbBufferToSend = data;

    //trigger Endpoint Interrupt - to start send operation  ///@todo appears broken
    USBIEPIFG |= 1 << (stUsbHandle[intfNum].edb_Index + 1);     //IEPIFGx;

    __bis_SR_register(bGIE);
    return (kUSBCDC_sendStarted);
}





#define EP_MAX_PACKET_SIZE_CDC      0x40

//this function is used only by USB interrupt
BOOL CdcToHostFromBuffer (ot_u8 intfNum) {
    ot_u8 byte_count, nTmp2;
    ot_u8 * pEP1;
    ot_u8 * pEP2;
    ot_u8 * pCT1;
    ot_u8 * pCT2;
    ot_u8 bWakeUp = FALSE;                                                   //TRUE for wake up after interrupt
    ot_u8 edbIndex;

    edbIndex = stUsbHandle[intfNum].edb_Index;

    if (CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft == 0){    //do we have somtething to send?
        if (!CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].bZeroPacketSent){        //zero packet was not yet sent
            CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].bZeroPacketSent = TRUE;

            if (CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].last_ByteSend == EP_MAX_PACKET_SIZE_CDC) {
                if (CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY == X_BUFFER){
                    if (tInputEndPointDescriptorBlock[edbIndex].bEPBCTX & EPBCNT_NAK) {
                        tInputEndPointDescriptorBlock[edbIndex].bEPBCTX = 0;
                        CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY = Y_BUFFER;                                     //switch buffer
                    }
                }
                else {
                    if (tInputEndPointDescriptorBlock[edbIndex].bEPBCTY & EPBCNT_NAK) {
                        tInputEndPointDescriptorBlock[edbIndex].bEPBCTY = 0;
                        CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY = X_BUFFER;                                     //switch buffer
                    }
                }
            }

            CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSend = 0;      //nothing to send
            bWakeUp |= USBCDC_HANDLE_TXCOMPLETE(intfNum);  //call event callback function
        } //if (!bSentZeroPacket)

        return (bWakeUp);
    }

    CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].bZeroPacketSent = FALSE;          //zero packet will be not sent: we have data

    if (CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY == X_BUFFER){
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
    byte_count = (CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft > EP_MAX_PACKET_SIZE_CDC) ? \
                    EP_MAX_PACKET_SIZE_CDC : CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft;
    nTmp2 = *pCT1;

    if (nTmp2 & EPBCNT_NAK){
        //copy data into IEP3 X or Y buffer
        USB_TX_memcpy(pEP1, CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].pUsbBufferToSend, byte_count);

        *pCT1 = byte_count;      //Set counter for usb In-Transaction

        //switch buffer
        CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY = \
                (CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY + 1) & 0x01;

        CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft -= byte_count;
        CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].pUsbBufferToSend += byte_count;       //move buffer pointer
        CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].last_ByteSend = byte_count;

        //try to send data over second buffer
        nTmp2 = *pCT2;
        if ((CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft > 0) &&      //do we have more data to send?
            (nTmp2 & EPBCNT_NAK)) {                                                 //if the second buffer is free?
            //how many byte we can send over one endpoint buffer
            byte_count = (CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft > EP_MAX_PACKET_SIZE_CDC) ? \
                    EP_MAX_PACKET_SIZE_CDC : CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft;

            //copy data into IEP3 X or Y buffer
            USB_TX_memcpy(pEP2, CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].pUsbBufferToSend, byte_count);

            *pCT2 = byte_count;   //Set counter for usb In-Transaction

            //switch buffer
            CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY = \
                (CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY + 1) & 0x01;

            CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft -= byte_count;
            CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].pUsbBufferToSend += byte_count;  //move buffer pointer
            CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].last_ByteSend = byte_count;
        }
    }
    return (bWakeUp);
}




/*
 * Aborts an active send operation on interface intfNum.
 * Returns the number of bytes that were sent prior to the abort, in size.
 */
ot_u8 USBCDC_abortSend (ot_u16* size, ot_u8 intfNum) {
    unsigned short bGIE;

    bGIE  = (__get_SR_register() & GIE);        //save interrupt status
    __disable_interrupt();                      //disable interrupts - atomic operation

    *size = (CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSend - \
               CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft);

    CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSend = 0;
    CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft = 0;

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
    nCount = (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft > CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp) ? \
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp : CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft;

    USB_RX_memcpy(CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pUserBuffer, pEP, nCount);   //copy data from OEP3 X or Y buffer
    CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft -= nCount;
    CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pUserBuffer += nCount;                     //move buffer pointer
    //to read rest of data next time from this place

    //all bytes are copied from receive buffer?
    if (nCount == CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp) {
        //switch current buffer
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY = \
            (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY + 1) & 0x01;

        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp = 0;

        //clear NAK, EP ready to receive data
        *pCT = 0x00;
    }
    else {
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp -= nCount;
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCurrentEpPos = pEP + nCount;
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
    if (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pUserBuffer != NULL){
        __bis_SR_register(bGIE);
        return (kUSBCDC_intfBusyError);
    }

    // Set RX Buffer constraints
    CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceive = size;            //bytes to receive
    CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft = size;        //left bytes to receive
    CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pUserBuffer = data;                //set user receive buffer


    //read rest of data from buffer, if any
    if (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp > 0) {
        //copy data from pEP-endpoint into User's buffer
        CopyUsbToBuff(CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCurrentEpPos, CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1, intfNum);

        if (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft == 0){ //the Receive opereation is completed
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pUserBuffer = NULL;        //no more receiving pending
            USBCDC_HANDLE_RXCOMPLETE(intfNum);                     //call event handler in interrupt context
            __bis_SR_register(bGIE);                                        //restore interrupt status
            return (kUSBCDC_receiveCompleted);                              //receive completed
        }

        //check other EP buffer for data - exchange pCT1 with pCT2
        if (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1 == &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX){
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY;
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCurrentEpPos = (ot_u8*)stUsbHandle[intfNum].oep_Y_Buffer;
        }
        else {
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX;
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCurrentEpPos = (ot_u8*)stUsbHandle[intfNum].oep_X_Buffer;
        }

        nTmp1 = *CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1;
        //try read data from second buffer
        if (nTmp1 & EPBCNT_NAK){                                            //if the second buffer has received data?
            nTmp1 = nTmp1 & 0x7f;                                           //clear NAK bit
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp = nTmp1;        //holds how many valid bytes in the EP buffer
            CopyUsbToBuff(CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCurrentEpPos,CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1, intfNum);
        }

        if (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft == 0){ //the Receive opereation is completed
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pUserBuffer = NULL;        //no more receiving pending
            USBCDC_HANDLE_RXCOMPLETE(intfNum);                     //call event handler in interrupt context
            __bis_SR_register(bGIE);                                        //restore interrupt status
            return (kUSBCDC_receiveCompleted);                              //receive completed
        }
    } //read rest of data from buffer, if any

    //read 'fresh' data, if available
    nTmp1 = 0;
    if (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY == X_BUFFER){ //this is current buffer
        if (tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX & EPBCNT_NAK){ //this buffer has a valid data packet
            //this is the active EP buffer
            //pEP1
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCurrentEpPos = (ot_u8*)stUsbHandle[intfNum].oep_X_Buffer;
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX;

            //second EP buffer
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pEP2 = (ot_u8*)stUsbHandle[intfNum].oep_Y_Buffer;
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT2 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY;
            nTmp1 = 1;                 //indicate that data is available
        }
    }
    else {                                                                //Y_BUFFER
        if (tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY & EPBCNT_NAK){
            //this is the active EP buffer
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCurrentEpPos = (ot_u8*)stUsbHandle[intfNum].oep_Y_Buffer;
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY;

            //second EP buffer
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pEP2 = (ot_u8*)stUsbHandle[intfNum].oep_X_Buffer;
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT2 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX;
            nTmp1 = 1;          //indicate that data is available
        }
    }

    if (nTmp1){
        //how many byte we can get from one endpoint buffer
        nTmp1 = *CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1;
        while (nTmp1 == 0) {
            nTmp1 = *CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1;
        }

        if (nTmp1 & EPBCNT_NAK) {
            nTmp1 = nTmp1 & 0x7f;                                           //clear NAK bit
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp = nTmp1;        //holds how many valid bytes in the EP buffer

            CopyUsbToBuff(CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCurrentEpPos, CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1, intfNum);

            nTmp1 = *CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT2;
            //try read data from second buffer

            //do we have more data to send?  if the second buffer has received data?
            if ((CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft > 0) &&
                (nTmp1 & EPBCNT_NAK)) {
                nTmp1 = nTmp1 & 0x7f;                                       //clear NAK bit
                CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp = nTmp1;    //holds how many valid bytes in the EP buffer

                CopyUsbToBuff(CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pEP2, CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT2, intfNum);

                CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1 = CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT2;
            }
        }
    }

    if (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft == 0){     //the Receive opereation is completed
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pUserBuffer = NULL;            //no more receiving pending
        USBCDC_HANDLE_RXCOMPLETE(intfNum);                         //call event handler in interrupt context
        __bis_SR_register(bGIE);                                            //restore interrupt status
        return (kUSBCDC_receiveCompleted);
    }

    //interrupts enable
    __bis_SR_register(bGIE);                                                //restore interrupt status
    return (kUSBCDC_receiveStarted);
}




//this function is used only by USB interrupt.
//It fills user receiving buffer with received data
BOOL CdcToBufferFromHost (ot_u8 intfNum)
{
    ot_u8 * pEP1;
    ot_u8 nTmp1;
    ot_u8 bWakeUp = FALSE;                                                   //per default we do not wake up after interrupt

    ot_u8 edbIndex;

    edbIndex = stUsbHandle[intfNum].edb_Index;

    if (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft == 0){     //do we have somtething to receive?
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pUserBuffer = NULL;            //no more receiving pending
        return (bWakeUp);
    }

    //No data to receive...
    if (!((tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX | tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY) & 0x80)){
        return (bWakeUp);
    }

    if (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY == X_BUFFER){ //X is current buffer
        //this is the active EP buffer
        pEP1 = (ot_u8*)stUsbHandle[intfNum].oep_X_Buffer;
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX;

        //second EP buffer
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pEP2 = (ot_u8*)stUsbHandle[intfNum].oep_Y_Buffer;
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT2 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY;
    }
    else {
        //this is the active EP buffer
        pEP1 = (ot_u8*)stUsbHandle[intfNum].oep_Y_Buffer;
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY;

        //second EP buffer
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pEP2 = (ot_u8*)stUsbHandle[intfNum].oep_X_Buffer;
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT2 = &tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX;
    }

    //how many byte we can get from one endpoint buffer
    nTmp1 = *CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1;

    if (nTmp1 & EPBCNT_NAK){
        nTmp1 = nTmp1 & 0x7f;                                                   //clear NAK bit
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp = nTmp1;                //holds how many valid bytes in the EP buffer

        CopyUsbToBuff(pEP1, CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1, intfNum);

        nTmp1 = *CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT2;
        //try read data from second buffer
        if ((CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft > 0) &&   //do we have more data to send?
            (nTmp1 & EPBCNT_NAK)){                                              //if the second buffer has received data?
            nTmp1 = nTmp1 & 0x7f;                                               //clear NAK bit
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp = nTmp1;            //holds how many valid bytes in the EP buffer

            CopyUsbToBuff(CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pEP2,CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT2, intfNum);

            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT1 = CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT2;
        }
    }

    if (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft == 0){         //the Receive opereation is completed
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pUserBuffer = NULL;                //no more receiving pending
        bWakeUp |= USBCDC_HANDLE_RXCOMPLETE(intfNum);

        if (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp){                   //Is not read data still available in the EP?
            bWakeUp |= USBCDC_HANDLE_RXBUFFERED(intfNum);
        }
    }
    return (bWakeUp);
}




//helper for USB interrupt handler
BOOL CdcIsReceiveInProgress (ot_u8 intfNum) {
    return (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pUserBuffer != NULL);
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
    if (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pUserBuffer){
        //how many bytes are already received?
        *size = CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceive -
                CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft;

        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp = 0;
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pUserBuffer = NULL;
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft = 0;
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
    if (!CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pUserBuffer){
        ot_u8 tmp1 = tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX & EPBCNT_NAK;
        ot_u8 tmp2 = tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY & EPBCNT_NAK;

        //switch current buffer if any and only ONE of buffers is full
        if (tmp1 ^ tmp2){
            //switch current buffer
            CdcReadCtrl[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY = \
                    (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].bCurrentBufferXY + 1) & 0x01;
        }

        tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX = 0;               //flush buffer X
        tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY = 0;               //flush buffer Y
        CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp = 0;                //indicates that no more data available in the EP
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
    if (CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft != 0){
        ret |= kUSBCDC_waitingForSend;
        *bytesSent = CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSend -
                     CdcWriteCtrl[INTFNUM_OFFSET(intfNum)].nCdcBytesToSendLeft;
    }

    //Is receive operation underway?
    if (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pUserBuffer != NULL){
        ret |= kUSBCDC_waitingForReceive;
        *bytesReceived = CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceive -
                         CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesToReceiveLeft;
    }
    else {
        //do not access USB memory if suspended (PLL off).
        //It may produce BUS_ERROR
        if (!bFunctionSuspended){
            //any of buffers has a valid data packet
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




/*
 * Returns how many bytes are in the buffer are received and ready to be read.
 */
ot_u8 USBCDC_bytesInUSBBuffer (ot_u8 intfNum) {
    ot_u8 bTmp1 = 0;
    ot_u16 bGIE;
    ot_u8 edbIndex;
    ot_u8* extra_buf;
    ot_u8 num_bufs;

    edbIndex = stUsbHandle[intfNum].edb_Index;

    bGIE  = (__get_SR_register() & GIE);                                    //save interrupt status
    //atomic operation - disable interrupts
    __disable_interrupt();                                                  //Disable global interrupts

    if (!(bFunctionSuspended) && (bEnumerationStatus == ENUMERATION_COMPLETE)) {
        //If a RX operation is underway, part of data may was read of the OEP buffer
        if (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp > 0) {
            bTmp1       = CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp;
            extra_buf   = CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT2;
            num_bufs    = 1;
        }
        else {
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
//    else if (CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp > 0) {
//        bTmp1 = CdcReadCtrl[INTFNUM_OFFSET(intfNum)].nBytesInEp;
//
//        //the next buffer has a valid data packet
//        if (*CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT2 & EPBCNT_NAK) {
//            bTmp1 += *CdcReadCtrl[INTFNUM_OFFSET(intfNum)].pCT2 & 0x7F;
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
    abUsbRequestReturnData[6] = CdcControl[INTFNUM_OFFSET(tSetupPacket.wIndex)].bDataBits;          //Data bits = 8
    abUsbRequestReturnData[5] = CdcControl[INTFNUM_OFFSET(tSetupPacket.wIndex)].bParity;            //No Parity
    abUsbRequestReturnData[4] = CdcControl[INTFNUM_OFFSET(tSetupPacket.wIndex)].bStopBits;          //Stop bits = 1

#   if (1)
    abUsbRequestReturnData[3] = CdcControl[INTFNUM_OFFSET(tSetupPacket.wIndex)].lBaudrate >> 24;
    abUsbRequestReturnData[2] = CdcControl[INTFNUM_OFFSET(tSetupPacket.wIndex)].lBaudrate >> 16;
    abUsbRequestReturnData[1] = CdcControl[INTFNUM_OFFSET(tSetupPacket.wIndex)].lBaudrate >> 8;
    abUsbRequestReturnData[0] = CdcControl[INTFNUM_OFFSET(tSetupPacket.wIndex)].lBaudrate;
#   elif defined(__LITTLE_ENDIAN)
        *((ot_u32*)abUsbRequestReturnData) = CdcControl[INTFNUM_OFFSET(tSetupPacket.wIndex)].lBaudrate;
#   endif

    wBytesRemainingOnIEP0   = 0x07;               //amount of data to be send over EP0 to host
    usbSendDataPacketOnEP0((ot_u8*)&abUsbRequestReturnData[0]);              //send data to host

    return (FALSE);
}






ot_u8 usbSetLineCoding (void) {
    usbReceiveDataPacketOnEP0((ot_u8*)&abUsbRequestIncomingData);            //receive data over EP0 from Host

    return (FALSE);
}






ot_u8 usbSetControlLineState (void) {
    USBCDC_handleSetControlLineState((ot_u8)tSetupPacket.wIndex, (ot_u8)tSetupPacket.wValue);
    usbSendZeroLengthPacketOnIEP0();                                        //Send ZLP for status stage

    return (FALSE);
}






ot_u8 Handler_SetLineCoding (void) {
    ot_u8 bWakeUp;
    Fourbytes scratch;

    //Baudrate Settings
#   if (1)
        scratch.ubyte[B3] = abUsbRequestIncomingData[3];
        scratch.ubyte[B2] = abUsbRequestIncomingData[2];
        scratch.ubyte[B1] = abUsbRequestIncomingData[1];
        scratch.ubyte[B0] = abUsbRequestIncomingData[0];
#   elif defined (__LITTLE_ENDIAN__)
        scratch.ulong = *(ot_u32*)abUsbRequestIncomingData;
#   endif
    
    CdcControl[INTFNUM_OFFSET(tSetupPacket.wIndex)].lBaudrate = scratch.ulong;

    bWakeUp = USBCDC_handleSetLineCoding(tSetupPacket.wIndex,
            CdcControl[INTFNUM_OFFSET(tSetupPacket.wIndex)].lBaudrate);

    return (bWakeUp);
}



#endif  //ifdef _CDC_

/*----------------------------------------------------------------------------+
 | End of source file                                                          |
 +----------------------------------------------------------------------------*/
/*------------------------ Nothing Below This Line --------------------------*/
