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
  * @file       /otplatform/msp430f5/usb_cdc_driver/usb_main.h
  * @author     RSTO, JP Norair
  * @version    R100
  * @date       1 Nov 2012
  * @brief      USB Device Main Header
  * @defgroup   MSP430F5 USB CDC
  * @ingroup    MSP430F5 USB CDC
  *
  * This module is a derivative work of TI's USB library file, Usb.h.  
  * JP has integrated it into OpenTag.  This file provides an interface to USB
  * driver-calls, in addition to an assortment of constants, macros, and 
  * control flags specific to the USB driver and MSP430 USB HW.
  * 
  * <PRE>
  * Author: RSTO
  * Source: Usb.h, File Version 1.04 2010/10/30  
  *
  * WHO         WHEN        WHAT                                          
  * ---         ----------  --------------------------------------------- 
  * RSTO        2008/09/03  born
  * RSTO        2008/12/23  enhancements of CDC API
  * RSTO        2009/05/15  changed USB_connectionStatus() to USB_connectionInfo()
  * RSTO        2009/05/26  remove kUSB_failedEnumEvent 
  * RSTO        2009/07/17  added __data16 qualifier for USB buffers 
  * MSP,Biju    2009/10/20  Composite support changes
  * RSTO        2009/11/05  added event ST_NOENUM_SUSPENDED
  * MSP,Biju    2009/12/28  macros DESC_TYPE_IAD added due to IAD support
  * RSTO        2010/10/30  added kUSB_allXXXEvents
  * ---         ----------  --------------------------------------------
  * JPN         2012/05/01  Integrated with OpenTag 0.3
  * JPN         2012/11/01  Integrated with OpenTag 0.4
  * </PRE>                           
  *****************************************************************************/


#ifndef _USB_H_
#define _USB_H_

#include "usb_cdc_driver/usb_types.h"


/*----------------------------------------------------------------------------+
 | Constant Definition                                                         |
 +----------------------------------------------------------------------------*/
#define USB_RETURN_DATA_LENGTH  8
#define SIZEOF_DEVICE_REQUEST   0x08

//Bit definitions for DEVICE_REQUEST.bmRequestType
//Bit 7:   Data direction
#define USB_REQ_TYPE_OUTPUT     0x00    //0 = Host sending data to device
#define USB_REQ_TYPE_INPUT      0x80    //1 = Device sending data to host

//Bit 6-5: Type
#define USB_REQ_TYPE_MASK       0x60    //Mask value for bits 6-5
#define USB_REQ_TYPE_STANDARD   0x00    //00 = Standard USB request
#define USB_REQ_TYPE_CLASS      0x20    //01 = Class specific
#define USB_REQ_TYPE_VENDOR     0x40    //10 = Vendor specific

//Bit 4-0: Recipient
#define USB_REQ_TYPE_RECIP_MASK 0x1F    //Mask value for bits 4-0
#define USB_REQ_TYPE_DEVICE     0x00    //00000 = Device
#define USB_REQ_TYPE_INTERFACE  0x01    //00001 = Interface
#define USB_REQ_TYPE_ENDPOINT   0x02    //00010 = Endpoint
#define USB_REQ_TYPE_OTHER      0x03    //00011 = Other

//Values for DEVICE_REQUEST.bRequest
//Standard Device Requests
#define USB_REQ_GET_STATUS              0
#define USB_REQ_CLEAR_FEATURE           1
#define USB_REQ_SET_FEATURE             3
#define USB_REQ_SET_ADDRESS             5
#define USB_REQ_GET_DESCRIPTOR          6
#define USB_REQ_SET_DESCRIPTOR          7
#define USB_REQ_GET_CONFIGURATION       8
#define USB_REQ_SET_CONFIGURATION       9
#define USB_REQ_GET_INTERFACE           10
#define USB_REQ_SET_INTERFACE           11
#define USB_REQ_SYNCH_FRAME             12

//CDC CLASS Requests
#define USB_CDC_GET_LINE_CODING         0x21
#define USB_CDC_SET_LINE_CODING         0x20
#define USB_CDC_SET_CONTROL_LINE_STATE  0x22

//HID CLASS Requests
#define USB_HID_REQ                     0x81
#define USB_REQ_GET_REPORT              0x01
#define USB_REQ_GET_IDLE                0x02
#define USB_REQ_SET_REPORT              0x09
#define USB_REQ_SET_IDLE                0x0A
#define USB_REQ_SET_PROTOCOL            0x0B
#define USB_REQ_GET_PROTOCOL            0x03

//MSC CLASS Requests
#define USB_MSC_RESET_BULK              0xFF
#define USB_MSC_GET_MAX_LUN             0xFE

// PHDC CLASS Requests
#define USB_PHDC_GET_STATUS             0x00

//HID Values for HID Report Types (tSetup.bValueH)
#define USB_REQ_HID_INPUT               0x01
#define USB_REQ_HID_OUTPUT              0x02
#define USB_REQ_HID_FEATURE             0x03

#define USB_REQ_HID_BOOT_PROTOCOL       0x00
#define USB_REQ_HID_REPORT_PROTOCOL     0x01


//Descriptor Type Values
#define DESC_TYPE_DEVICE                1       //Device Descriptor (Type 1)
#define DESC_TYPE_CONFIG                2       //Configuration Descriptor (Type 2)
#define DESC_TYPE_STRING                3       //String Descriptor (Type 3)
#define DESC_TYPE_INTERFACE             4       //Interface Descriptor (Type 4)
#define DESC_TYPE_ENDPOINT              5       //Endpoint Descriptor (Type 5)
#define DESC_TYPE_DEVICE_QUALIFIER      6       //Endpoint Descriptor (Type 6)
#define DESC_TYPE_IAD                   0x0B
#define DESC_TYPE_HUB                   0x29    //Hub Descriptor (Type 6)
#define DESC_TYPE_HID                   0x21    //HID Descriptor
#define DESC_TYPE_REPORT                0x22    //Report Descriptor
#define DESC_TYPE_PHYSICAL              0x23    //Physical Descriptor

//Feature Selector Values
#define FEATURE_REMOTE_WAKEUP           1       //Remote wakeup (Type 1)
#define FEATURE_ENDPOINT_STALL          0       //Endpoint stall (Type 0)

//Device Status Values
#define DEVICE_STATUS_REMOTE_WAKEUP     0x02
#define DEVICE_STATUS_SELF_POWER        0x01

//Maximum descriptor size
#define MAX_DESC_SIZE                   256

//DEVICE_DESCRIPTOR structure
#define SIZEOF_DEVICE_DESCRIPTOR        0x12
#define OFFSET_DEVICE_DESCRIPTOR_VID_L  0x08
#define OFFSET_DEVICE_DESCRIPTOR_VID_H  0x09
#define OFFSET_DEVICE_DESCRIPTOR_PID_L  0x0A
#define OFFSET_DEVICE_DESCRIPTOR_PID_H  0x0B
#define OFFSET_CONFIG_DESCRIPTOR_POWER  0x07
#define OFFSET_CONFIG_DESCRIPTOR_CURT   0x08

//CONFIG_DESCRIPTOR structure
#define SIZEOF_CONFIG_DESCRIPTOR 0x09

//HID DESCRIPTOR structure
//#define SIZEOF_HID_DESCRIPTOR 0x09

//Bit definitions for CONFIG_DESCRIPTOR.bmAttributes
#define CFG_DESC_ATTR_SELF_POWERED  0x40    //Bit 6: If set, device is self powered
#define CFG_DESC_ATTR_BUS_POWERED   0x80    //Bit 7: If set, device is bus powered
#define CFG_DESC_ATTR_REMOTE_WAKE   0x20    //Bit 5: If set, device supports remote wakeup

//INTERFACE_DESCRIPTOR structure
#define SIZEOF_INTERFACE_DESCRIPTOR 0x09

//ENDPOINT_DESCRIPTOR structure
#define SIZEOF_ENDPOINT_DESCRIPTOR 0x07

//Bit definitions for EndpointDescriptor.EndpointAddr
#define EP_DESC_ADDR_EP_NUM     0x0F        //Bit 3-0: Endpoint number
#define EP_DESC_ADDR_DIR_IN     0x80        //Bit 7: Direction of endpoint, 1/0 = In/Out

//Bit definitions for EndpointDescriptor.EndpointFlags
#define EP_DESC_ATTR_TYPE_MASK  0x03        //Mask value for bits 1-0
#define EP_DESC_ATTR_TYPE_CONT  0x00        //Bit 1-0: 00 = Endpoint does control transfers
#define EP_DESC_ATTR_TYPE_ISOC  0x01        //Bit 1-0: 01 = Endpoint does isochronous transfers
#define EP_DESC_ATTR_TYPE_BULK  0x02        //Bit 1-0: 10 = Endpoint does bulk transfers
#define EP_DESC_ATTR_TYPE_INT   0x03        //Bit 1-0: 11 = Endpoint does interrupt transfers

//Definition to indicate valid/invalid data
#define DATA_VALID      1
#define DATA_INVALID    0

/*
#if defined(__IAR_SYSTEMS_ICC__) || (__TI_COMPILER_VERSION__)
extern __no_init tDEVICE_REQUEST __data16 tSetupPacket;
extern __no_init ot_u8 __data16 abIEP0Buffer[];
extern __no_init ot_u8 __data16 abOEP0Buffer[];
extern __no_init ot_u8 __data16 pbXBufferAddressEp1[];
extern __no_init ot_u8 __data16 pbYBufferAddressEp1[];
extern __no_init ot_u8 __data16 pbXBufferAddressEp81[];
extern __no_init ot_u8 __data16 pbYBufferAddressEp81[];
extern __no_init ot_u8 __data16 pbXBufferAddressEp2[];
extern __no_init ot_u8 __data16 pbYBufferAddressEp2[];
extern __no_init ot_u8 __data16 pbXBufferAddressEp82[];
extern __no_init ot_u8 __data16 pbYBufferAddressEp82[];

extern __no_init ot_u8 __data16 pbXBufferAddressEp3[];
extern __no_init ot_u8 __data16 pbYBufferAddressEp3[];
extern __no_init ot_u8 __data16 pbXBufferAddressEp83[];
extern __no_init ot_u8 __data16 pbYBufferAddressEp83[];

extern __no_init ot_u8 __data16 pbXBufferAddressEp4[];
extern __no_init ot_u8 __data16 pbYBufferAddressEp4[];
extern __no_init ot_u8 __data16 pbXBufferAddressEp84[];
extern __no_init ot_u8 __data16 pbYBufferAddressEp84[];

extern __no_init ot_u8 __data16 pbXBufferAddressEp5[];
extern __no_init ot_u8 __data16 pbYBufferAddressEp5[];
extern __no_init ot_u8 __data16 pbXBufferAddressEp85[];
extern __no_init ot_u8 __data16 pbYBufferAddressEp85[];


extern __no_init ot_u8 __data16 pbXBufferAddressEp6[];
extern __no_init ot_u8 __data16 pbYBufferAddressEp6[];
extern __no_init ot_u8 __data16 pbXBufferAddressEp86[];
extern __no_init ot_u8 __data16 pbYBufferAddressEp86[];

extern __no_init ot_u8 __data16 pbXBufferAddressEp7[];
extern __no_init ot_u8 __data16 pbYBufferAddressEp7[];
extern __no_init ot_u8 __data16 pbXBufferAddressEp87[];
extern __no_init ot_u8 __data16 pbYBufferAddressEp87[];

#else // More direct (and more compatible) version of the above
*/
#   define tSetupPacket                     (*((tDEVICE_REQUEST*)0x2380))
#   define tEndPoint0DescriptorBlock        (*((tEDB0*)0x0920))
#   define tInputEndPointDescriptorBlock    ((tEDB*)0x23C8)                 //Size = 7
#   define tOutputEndPointDescriptorBlock   ((tEDB*)0x2388)                 //Size = 7
#   define abIEP0Buffer                     ((u8*)0x2378)                   //Size = EP0_MAX_PACKET_SIZE
#   define abOEP0Buffer                     ((u8*)0x2370)                   //Size = EP0_MAX_PACKET_SIZE
#   define pbXBufferAddressEp1              ((u8*)OEP1_X_BUFFER_ADDRESS)    //Size = EP_MAX_PACKET_SIZE
#   define pbYBufferAddressEp1              ((u8*)OEP1_Y_BUFFER_ADDRESS)
#   define pbXBufferAddressEp81             ((u8*)IEP1_X_BUFFER_ADDRESS)
#   define pbYBufferAddressEp81             ((u8*)IEP1_Y_BUFFER_ADDRESS)
#   define pbXBufferAddressEp2              ((u8*)OEP2_X_BUFFER_ADDRESS)
#   define pbYBufferAddressEp2              ((u8*)OEP2_Y_BUFFER_ADDRESS)
#   define pbXBufferAddressEp82             ((u8*)IEP2_X_BUFFER_ADDRESS)
#   define pbYBufferAddressEp82             ((u8*)IEP2_Y_BUFFER_ADDRESS)
#   define pbXBufferAddressEp3              ((u8*)OEP3_X_BUFFER_ADDRESS)
#   define pbYBufferAddressEp3              ((u8*)OEP3_Y_BUFFER_ADDRESS)
#   define pbXBufferAddressEp83             ((u8*)IEP3_X_BUFFER_ADDRESS)
#   define pbYBufferAddressEp83             ((u8*)IEP3_Y_BUFFER_ADDRESS) 
#   define pbXBufferAddressEp4              ((u8*)OEP4_X_BUFFER_ADDRESS)
#   define pbYBufferAddressEp4              ((u8*)OEP4_Y_BUFFER_ADDRESS)
#   define pbXBufferAddressEp84             ((u8*)IEP4_X_BUFFER_ADDRESS)
#   define pbYBufferAddressEp84             ((u8*)IEP4_Y_BUFFER_ADDRESS)
#   define pbXBufferAddressEp5              ((u8*)OEP5_X_BUFFER_ADDRESS)
#   define pbYBufferAddressEp5              ((u8*)OEP5_Y_BUFFER_ADDRESS)
#   define pbXBufferAddressEp85             ((u8*)IEP5_X_BUFFER_ADDRESS)
#   define pbYBufferAddressEp85             ((u8*)IEP5_Y_BUFFER_ADDRESS)
#   define pbXBufferAddressEp6              ((u8*)OEP6_X_BUFFER_ADDRESS)
#   define pbYBufferAddressEp6              ((u8*)OEP6_Y_BUFFER_ADDRESS)
#   define pbXBufferAddressEp86             ((u8*)IEP6_X_BUFFER_ADDRESS)
#   define pbYBufferAddressEp86             ((u8*)IEP6_Y_BUFFER_ADDRESS)
#   define pbXBufferAddressEp7              ((u8*)OEP7_X_BUFFER_ADDRESS)
#   define pbYBufferAddressEp7              ((u8*)OEP7_Y_BUFFER_ADDRESS)
#   define pbXBufferAddressEp87             ((u8*)IEP7_X_BUFFER_ADDRESS)
#   define pbYBufferAddressEp87             ((u8*)IEP7_Y_BUFFER_ADDRESS)

//#endif


extern ot_u16 wBytesRemainingOnIEP0;
extern ot_u16 wBytesRemainingOnOEP0;
extern ot_u8 abUsbRequestReturnData[];
extern ot_u8 abUsbRequestIncomingData[];
extern ot_u8 bEnumerationStatus;
extern ot_u8 bFunctionSuspended;

//Function return values
#define kUSB_succeed        0x00
#define kUSB_generalError   0x01
#define kUSB_notEnabled     0x02
//#define kUSB_VbusNotPresent 0x03

//return values USB_connectionInfo(), USB_connect()
#define kUSB_vbusPresent    0x01
#define kUSB_busActive      0x02    //frame sync packets are being received
#define kUSB_ConnectNoVBUS  0x04
#define kUSB_suspended      0x08
#define kUSB_NotSuspended   0x10
#define kUSB_Enumerated     0x20
#define kUSB_purHigh        0x40

//Parameters for function USB_setEnabledEvents()
#define kUSB_clockFaultEvent        0x0001
#define kUSB_VbusOnEvent            0x0002
#define kUSB_VbusOffEvent           0x0004
#define kUSB_UsbResetEvent          0x0008
#define kUSB_UsbSuspendEvent        0x0010
#define kUSB_UsbResumeEvent         0x0020
#define kUSB_dataReceivedEvent      0x0040
#define kUSB_sendCompletedEvent     0x0080
#define kUSB_receiveCompletedEvent  0x0100
#define kUSB_allUsbEvents           0x01FF

//USB connection states
#define ST_USB_DISCONNECTED         0x80
#define ST_USB_CONNECTED_NO_ENUM    0x81
#define ST_ENUM_IN_PROGRESS         0x82
#define ST_ENUM_ACTIVE              0x83
#define ST_ENUM_SUSPENDED           0x84
//#define ST_FAILED_ENUM              0x85
#define ST_ERROR                    0x86
#define ST_NOENUM_SUSPENDED         0x87

void usbStallInEndpoint(ot_u8);
void usbStallOutEndpoint(ot_u8);
void usbStallEndpoint(ot_u8);
void usbClearOEPByteCount(ot_u8);


/*----------------------------------------------------------------------------
 * These functions can be used in application
 +----------------------------------------------------------------------------*/

/*
 * MSP430 USB Module Management functions
 */

/**
 * Init the USB HW interface.
 */
ot_u8 USB_init(void);

/**
 * Init and start the USB PLL.
 */
ot_u8 USB_enable ();

/**
 * Disables the USB module and PLL.
 */
ot_u8 USB_disable(void);

/*
 * Enables/disables various USB events.
 */
ot_u8 USB_setEnabledEvents (ot_u16 events);

/*
 * Returns which events are enabled and which are disabled.
 */
ot_u16 USB_getEnabledEvents ();

/*
 * Instruct USB module to make itself available to the PC for connection, by pulling PUR high.
 */
ot_u8 USB_connect ();

/*
 * Force a disconnect from the PC by pulling PUR low.
 */
ot_u8 USB_disconnect ();

/**
 * Reset USB-SIE and global variables.
 */
ot_u8 USB_reset ();

/**
 * Suspend USB.
 */
ot_u8 USB_suspend(void);

/**
 * Resume USB.
 */
ot_u8 USB_resume(void);

/*
 * Force a remote wakeup of the USB host.
 *     This method can be generated only if device supports
 *     remote wake-up feature in some of its configurations.
 *     The method wakes-up the USB bus only if wake-up feature is enabled by the host.
 */
ot_u8 USB_forceRemoteWakeup ();

/*
 * Returns the status of the USB connection.
 */
//ot_u8 USB_connectionInfo ();

/*
 * Returns the state of the USB connection.
 */
ot_u8 USB_connectionState ();

#ifdef NON_COMPOSITE_MULTIPLE_INTERFACES
/*
 * Switch to a different USB configuration. Used only for non-composite devices with multiple configuratons.
 */
ot_u8 USB_switchInterface(ot_u8 interfaceIndex);

#endif

/*
 * Event-Handling routines
 */


#define HANDLE_RETURN	void


/*
 * If this function gets executed, it's a sign that the output of the USB PLL has failed.
 * returns TRUE to keep CPU awake
 */
HANDLE_RETURN USB_handleClockEvent ();

/*
 * If this function gets executed, it indicates that a valid voltage has just been applied to the VBUS pin.
 * returns TRUE to keep CPU awake
 */
HANDLE_RETURN USB_handleVbusOnEvent ();

/*
 * If this function gets executed, it indicates that a valid voltage has just been removed from the VBUS pin.
 * returns TRUE to keep CPU awake
 */
HANDLE_RETURN USB_handleVbusOffEvent ();

/*
 * If this function gets executed, it indicates that the USB host has issued a USB reset event to the device.
 * returns TRUE to keep CPU awake
 */
HANDLE_RETURN USB_handleResetEvent ();

/*
 * If this function gets executed, it indicates that the USB host has chosen to suspend this device after a period of active
 * operation.
 * returns TRUE to keep CPU awake
 */
HANDLE_RETURN USB_handleSuspendEvent ();

/*
 * If this function gets executed, it indicates that the USB host has chosen to resume this device after a period of suspended
 * operation.
 * returns TRUE to keep CPU awake
 */
HANDLE_RETURN USB_handleResumeEvent ();

/*
 * If this function gets executed, it indicates that the USB host has enumerated this device :
 * after host assigned the address to the device.
 * returns TRUE to keep CPU awake
 */
ot_u8 USB_handleEnumCompleteEvent ();

/**
 * Send stall handshake for in- and out-endpoint0 (control pipe)
 */
void usbStallEndpoint0(void);

/**
 * Clear byte counter for endpoint0 (control pipe)
 */
void usbClearOEP0ByteCount(void);

/**
 * Send stall handshake for out-endpoint0 (control pipe)
 */
void usbStallOEP0(void);

/**
 * Send further data over control pipe if needed.
 *     Function is called from control-in IRQ. Do not call from user application
 */
void usbSendNextPacketOnIEP0(void);

/**
 * Send data over control pipe to host.
 *     Number of bytes to transmit should be set with
 *     global varible "wBytesRemainingOnIEP0" before function is called.
 */
void usbSendDataPacketOnEP0 (ot_u8* pbBuffer);

/**
 * Receive further data from control pipe if needed.
 *     Function is called from control-out IRQ. Do not call from user application
 */
void usbReceiveNextPacketOnOEP0(void);

/**
 * Receive data from control pipe.
 *     Number of bytes to receive should be set with
 *     global varible "wBytesRemainingOnOEP0" before function is called.
 */
void usbReceiveDataPacketOnEP0 (ot_u8* pbBuffer);

/**
 * Send zero length packet on control pipe.
 */
void usbSendZeroLengthPacketOnIEP0(void);

/*Send data to host.*/
ot_u8 MscSendData (const ot_u8* data, ot_u16 size);

/**
 * Decode incoming usb setup packet and call corresponding function
 *     usbDecodeAndProcessUsbRequest is called from IRQ. Do not call from user application
 */
ot_u8 usbDecodeAndProcessUsbRequest(void);
ot_u8 usbClearEndpointFeature(void);
ot_u8 usbGetConfiguration(void);
ot_u8 usbGetDeviceDescriptor(void);
ot_u8 usbGetConfigurationDescriptor(void);
ot_u8 usbGetStringDescriptor(void);
ot_u8 usbGetInterface(void);
ot_u8 usbGetDeviceStatus(void);
ot_u8 usbGetEndpointStatus(void);
ot_u8 usbGetInterfaceStatus(void);
ot_u8 usbSetAddress(void);
ot_u8 usbSetConfiguration(void);
ot_u8 usbClearDeviceFeature(void);
ot_u8 usbSetDeviceFeature(void);
ot_u8 usbSetEndpointFeature(void);
ot_u8 usbSetInterface(void);
ot_u8 usbInvalidRequest(void);


#define ENUMERATION_COMPLETE 0x01


#endif

