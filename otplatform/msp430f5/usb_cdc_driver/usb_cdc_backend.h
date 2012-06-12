//(c)2009 by Texas Instruments Incorporated, All Rights Reserved.
/*----------------------------------------------------------------------------+
 |                                                                             |
 |                              Texas Instruments                              |
 |                                                                             |
 |                          MSP430 USB-Example (CDC Driver)                    |
 |                                                                             |
 +-----------------------------------------------------------------------------+
 |  Source: UsbCdc.h, File Version 1.00 2009/12/03                             |
 |  Author: RSTO                                                               |
 |                                                                             |
 |  WHO          WHEN         WHAT                                             |
 |  ---          ----------   ------------------------------------------------ |
 |  RSTO         2008/09/03   born                                             |
 |  RSTO         2008/12/23   enhancements of CDC API                          |
 |  RSTO         2009/05/15   added param to USBCDC_rejectData()               |
 |  RSTO         2009/05/26   added USBCDC_bytesInUSBBuffer()                  |
 |  MSP,Biju     2009/12/03   file versioning started                          |
 |                                                                             |
 +----------------------------------------------------------------------------*/
#ifndef _USB_CDC_BACKEND_H_
#define _USB_CDC_BACKEND_H_

#include "usb_cdc_driver/usb_descriptors.h"
#include "usb_cdc_driver/usb_types.h"


#define kUSBCDC_sendStarted         0x00 //was 0x01
#define kUSBCDC_sendComplete        0x02
#define kUSBCDC_intfBusyError       0x03
#define kUSBCDC_receiveStarted      0x04
#define kUSBCDC_receiveCompleted    0x05
#define kUSBCDC_receiveInProgress   0x06
#define kUSBCDC_generalError        0x07
#define kUSBCDC_busNotAvailable     0x08


//Local Macros
#define INTFNUM_OFFSET(X)   (X - CDC0_INTFNUM)  //Get the CDC offset


typedef struct {
    ot_u32 lBaudrate;
    ot_u8 bDataBits;
    ot_u8 bStopBits;
    ot_u8 bParity;
} _CdcControl;

typedef struct {
    ot_u16 nCdcBytesToSend;                       //holds counter of bytes to be sent
    ot_u16 nCdcBytesToSendLeft;                   //holds counter how many bytes is still to be sent
    const ot_u8* pUsbBufferToSend;               //holds the buffer with data to be sent
    ot_u8 bCurrentBufferXY;                      //is 0 if current buffer to write data is X, or 1 if current buffer is Y
    ot_u8 bZeroPacketSent;                       //= FALSE;
    ot_u8 last_ByteSend;
} _CdcWriteCtrl;

typedef struct {
    ot_u8 *pUserBuffer;                          //holds the current position of user's receiving buffer. If NULL- no receiving
                                                //operation started
    ot_u8 *pCurrentEpPos;                        //current positon to read of received data from curent EP
    ot_u16 nBytesToReceive;                       //holds how many bytes was requested by receiveData() to receive
    ot_u16 nBytesToReceiveLeft;                   //holds how many bytes is still requested by receiveData() to receive
    ot_u8 * pCT1;                                //holds current EPBCTxx register
    ot_u8 * pCT2;                                //holds next EPBCTxx register
    ot_u8 * pEP2;                                //holds addr of the next EP buffer
    ot_u8 nBytesInEp;                            //how many received bytes still available in current EP
    ot_u8 bCurrentBufferXY;                      //indicates which buffer is used by host to transmit data via OUT endpoint3
} _CdcReadCtrl;

extern _CdcControl      CdcControl[CDC_NUM_INTERFACES];
extern _CdcWriteCtrl    CdcWriteCtrl[CDC_NUM_INTERFACES];
extern _CdcReadCtrl     CdcReadCtrl[CDC_NUM_INTERFACES];




/*----------------------------------------------------------------------------
 * These functions can be used in application
 +----------------------------------------------------------------------------*/

/*
 * Sends data over interface intfNum, of size size and starting at address data.
 * Returns:  kUSBCDC_sendStarted
 *          kUSBCDC_sendComplete
 *          kUSBCDC_intfBusyError
 */
ot_u8 USBCDC_sendData (const ot_u8* data, ot_u16 size, ot_u8 intfNum);

/*
 * Receives data over interface intfNum, of size size, into memory starting at address data.
 */
ot_u8 USBCDC_receiveData (ot_u8* data, ot_u16 size, ot_u8 intfNum);

/*
 * Aborts an active receive operation on interface intfNum.
 * size: the number of bytes that were received and transferred
 * to the data location established for this receive operation.
 */
ot_u8 USBCDC_abortReceive (ot_u16* size, ot_u8 intfNum);


#define kUSBCDC_noDataWaiting 1 //returned by USBCDC_rejectData() if no data pending

/*
 * This function rejects payload data that has been received from the host.
 */
ot_u8 USBCDC_rejectData (ot_u8 intfNum);

/*
 * Aborts an active send operation on interface intfNum.  Returns the number of bytes that were sent prior to the abort, in size.
 */
ot_u8 USBCDC_abortSend (ot_u16* size, ot_u8 intfNum);


#define kUSBCDC_waitingForSend      0x01
#define kUSBCDC_waitingForReceive   0x02
#define kUSBCDC_dataWaiting         0x04
#define kUSBCDC_busNotAvailable     0x08
#define kUSB_allCdcEvents           0xFF

/*
 * This function indicates the status of the interface intfNum.
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
ot_u8 USBCDC_intfStatus (ot_u8 intfNum, ot_u16* bytesSent, ot_u16* bytesReceived);

/*
 * Returns how many bytes are in the buffer are received and ready to be read.
 */
ot_u8 USBCDC_bytesInUSBBuffer (ot_u8 intfNum);

/*----------------------------------------------------------------------------
 * Event-Handling routines
 +----------------------------------------------------------------------------*/

/*
 * This event indicates that data has been received for interface intfNum, but no data receive operation is underway.
 * returns TRUE to keep CPU awake
 */
ot_u8 USBCDC_handleDataReceived (ot_u8 intfNum);

/*
 * This event indicates that a send operation on interface intfNum has just been completed.
 * returns TRUE to keep CPU awake
 */
ot_u8 USBCDC_handleSendCompleted (ot_u8 intfNum);

/*
 * This event indicates that a receive operation on interface intfNum has just been completed.
 * returns TRUE to keep CPU awake
 */
ot_u8 USBCDC_handleReceiveCompleted (ot_u8 intfNum);

/*
 * Toggle state variable for CTS in USB Stack
 */
void USBCDC_setCTS(ot_u8 state);

/*
 * This event indicates that a SetLineCoding request was received from the host and new values
 * for line coding paramters are available.
 *
 */
ot_u8 USBCDC_handleSetLineCoding (ot_u8 intfNum, ot_u32 lBaudrate);

/*
 * This event indicates that a SetControlLineState request was received from the host. 
 * Basically new RTS and DTR states have been sent. Bit 0 of lineState is DTR and Bit 1 is RTS.
 *
 */
ot_u8 USBCDC_handleSetControlLineState (ot_u8 intfNum, ot_u8 lineState);

/*----------------------------------------------------------------------------
 * These functions is to be used ONLY by USB stack, and not by application
 +----------------------------------------------------------------------------*/

/**
 * Send a packet with the settings of the second uart back to the usb host
 */
ot_u8 usbGetLineCoding(void);

/**
 * Prepare EP0 to receive a packet with the settings for the second uart
 */
ot_u8 usbSetLineCoding(void);

/**
 * Function set or reset RTS
 */
ot_u8 usbSetControlLineState(void);

/**
 * Readout the settings (send from usb host) for the second uart
 */
ot_u8 Handler_SetLineCoding(void);

/**
 * sets up dma for CDC interface used as usb to uart bridge
 */
ot_u8 USBCDC_setupDMA_Bridge();
/**
 * sends data alread present in endpoint buffer
 */
ot_u8 USBCDC_Bridge_sendData (ot_u16 size, ot_u8 intfNum);
/**
 * gets address of input endpoint X
 */
ot_u8 *USBCDC_Bridge_getInEndpointBufferXAddr(ot_u8 intfNum);
/**
 * gets address of input endpoint Y
 */
ot_u8 *USBCDC_Bridge_getInEndpointBufferYAddr(ot_u8 intfNum);
/**
 * gets address of output endpoint X
 */
ot_u8 *USBCDC_Bridge_getOutEndpointBufferXAddr(ot_u8 intfNum);
/**
 * gets address of output endpoint Y
 */
ot_u8 *USBCDC_Bridge_getOutEndpointBufferYAddr(ot_u8 intfNum);
/**
 * completes receive operation once data from endpoint has been transmitted thru uart
 */
ot_u8 USBCDC_Bridge_completeReceiveData(ot_u8 intfNum);







BOOL CdcToHostFromBuffer (ot_u8 intfNum);
BOOL CdcIsReceiveInProgress (ot_u8 intfNum);
BOOL CdcToBufferFromHost (ot_u8 intfNum);





#endif  //_UsbCdc_H_
