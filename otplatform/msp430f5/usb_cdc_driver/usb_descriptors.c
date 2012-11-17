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
  */
/**
  * @file       /otplatform/msp430f5/usb_cdc_driver/usb_descriptors.c
  * @author     auto-generation, JP Norair
  * @version    R100
  * @date       01 Nov 2012
  * @brief      This is descriptor list, generated automatically by the MSP430
  *             USB descriptor tool.  JP Norair then optimized for the revised
  *             OpenTag CDC-only driver.
  * @ingroup    MSP430F5_USB_CDC
  *
  * @todo The names of the arrays are very long.  If you are not German, they
  *       are very difficult to read :).  Shorten the names and use comments.
  *
  ******************************************************************************
  */

#include "usb_cdc_driver/defMSP430USB.h"
#include "usb_cdc_driver/usb_descriptors.h"
#include "usb_cdc_driver/usb_cdc_backend.h"
#include "usb_cdc_driver/usb_isr.h"
#include "usb_cdc_driver/usb_main.h"

#include "OT_utils.h"

#ifdef _CDC_

/** USB Descriptors   <BR>
  * ========================================================================<BR>
  * Descriptors are just data elements used by USB.  Feel free to modify them
  * below (or in descriptors.h) if you know what you are doing.
  */

//const ot_u8 abromDeviceDescriptor[SIZEOF_DEVICE_DESCRIPTOR] = {
const ot_u8 usbdesc_device[SIZEOF_DEVICE_DESCRIPTOR] = {
    SIZEOF_DEVICE_DESCRIPTOR,               // Length of this descriptor
    DESC_TYPE_DEVICE,                       // Type code of this descriptor
    0x00, 0x02,                             // USB Version (02.00)
    0x02,                                   // Class Code (02=CDC)
    0x00,                                   // Subclass Code (00=unused)
    0x00,                                   // Protocol Code (00=unused)
    EP0_PACKET_SIZE,                        // End point 0's packet size
    USB_VID_CDC&0xFF, USB_VID_CDC>>8,       // Vendor ID for device, TI=0x0451
    USB_PID_CDC&0xFF, USB_PID_CDC>>8,       // Product ID for device,
    VER_FW_L, VER_FW_H,                     // Revision level of device
    1,                                      // Index of manufacturer name string desc
    2,                                      // Index of product name string desc
    3,                                      // Index of serial number string desc
    1                                       //  Number of configurations supported
};



//const struct abromConfigurationDescriptorGroup abromConfigurationDescriptorGroup = {
const usbdesc_cfg_struct usbdesc_cfg = {
// Generic part
    {
        // CONFIGURATION DESCRIPTOR (9 bytes)
        SIZEOF_CONFIG_DESCRIPTOR,                           // bLength
        DESC_TYPE_CONFIG,                                   // bDescriptorType
        DESCRIPTOR_TOTAL_LENGTH, 0x00,                   // wTotalLength
        USB_NUM_INTERFACES,                                 // bNumInterfaces
        USB_CONFIG_VALUE,                                   // bConfigurationvalue
        CONFIG_STRING_INDEX,                                // iConfiguration Description offset
        USB_SUPPORT_SELF_POWERED | USB_SUPPORT_REM_WAKE,    // bmAttributes, bus power, remote wakeup
        USB_MAX_POWER                                       // Max. Power Consumption
    },

    // CDC Part
    {
        {
            //INTERFACE DESCRIPTOR (9 bytes)
            0x09,                              // bLength: Interface Descriptor size
            DESC_TYPE_INTERFACE,               // bDescriptorType: Interface
            CDC0_COMM_INTERFACE,               // bInterfaceNumber
            0x00,                              // bAlternateSetting: Alternate setting
            0x01,                              // bNumEndpoints: Three endpoints used
            0x02,                              // bInterfaceClass: Communication Interface Class
            0x02,                              // bInterfaceSubClass: Abstract Control Model
            0x01,                              // bInterfaceProtocol: Common AT commands
            INTF_STRING_INDEX + 0,             // iInterface:

            //Header Functional Descriptor
            0x05,                              // bLength: Endpoint Descriptor size
            0x24,                              // bDescriptorType: CS_INTERFACE
            0x00,                              // bDescriptorSubtype: Header Func Desc
            0x10,                              // bcdCDC: spec release number
            0x01,

            //Call Management Functional Descriptor
            0x05,                              // bFunctionLength
            0x24,                              // bDescriptorType: CS_INTERFACE
            0x01,                              // bDescriptorSubtype: Call Management Func Desc
            0x00,                              // bmCapabilities: D0+D1
            CDC0_DATA_INTERFACE,                // bDataInterface: 0

            //ACM Functional Descriptor
            0x04,                              // bFunctionLength 
            0x24,                              // bDescriptorType: CS_INTERFACE
            0x02,                              // bDescriptorSubtype: Abstract Control Management desc
            0x02,                              // bmCapabilities

            // Union Functional Descriptor
            0x05,                               // Size, in bytes
            0x24,                               // bDescriptorType: CS_INTERFACE
            0x06,                              // bDescriptorSubtype: Union Functional Desc
            CDC0_COMM_INTERFACE,                // bMasterInterface -- the controlling intf for the union
            CDC0_DATA_INTERFACE,                // bSlaveInterface -- the controlled intf for the union

            //EndPoint Descriptor for Interrupt endpoint
            SIZEOF_ENDPOINT_DESCRIPTOR,         // bLength: Endpoint Descriptor size
            DESC_TYPE_ENDPOINT,                 // bDescriptorType: Endpoint
            CDC0_INTEP_ADDR,                    // bEndpointAddress: (IN2)
            EP_DESC_ATTR_TYPE_INT,             // bmAttributes: Interrupt
            0x40, 0x00,                         // wMaxPacketSize, 64 bytes
            0xFF,                              // bInterval

            //DATA INTERFACE DESCRIPTOR (9 bytes)
            0x09,                              // bLength: Interface Descriptor size
            DESC_TYPE_INTERFACE,               // bDescriptorType: Interface
            CDC0_DATA_INTERFACE,                // bInterfaceNumber
            0x00,                               // bAlternateSetting: Alternate setting
            0x02,                               // bNumEndpoints: Three endpoints used
            0x0A,                               // bInterfaceClass: Data Interface Class
            0x00,                               // bInterfaceSubClass:
            0x00,                               // bInterfaceProtocol: No class specific protocol required
            0x00,                              // iInterface:

            //EndPoint Descriptor for Output endpoint
            SIZEOF_ENDPOINT_DESCRIPTOR,         // bLength: Endpoint Descriptor size
            DESC_TYPE_ENDPOINT,                    // bDescriptorType: Endpoint
            CDC0_OUTEP_ADDR,                    // bEndpointAddress: (OUT3)
            EP_DESC_ATTR_TYPE_BULK,                // bmAttributes: Bulk 
            0x40, 0x00,                         // wMaxPacketSize, 64 bytes
            0xFF,                              // bInterval: ignored for Bulk transfer

            //EndPoint Descriptor for Input endpoint
            SIZEOF_ENDPOINT_DESCRIPTOR,         // bLength: Endpoint Descriptor size
            DESC_TYPE_ENDPOINT,                    // bDescriptorType: Endpoint
            CDC0_INEP_ADDR,                        // bEndpointAddress: (IN3)
            EP_DESC_ATTR_TYPE_BULK,                // bmAttributes: Bulk
            0x40, 0x00,                         // wMaxPacketSize, 64 bytes
            0xFF                                // bInterval: ignored for bulk transfer
        }
    }
};

//const void *usbDeviceDescriptors[NONCOMP_NUM_USB_INTERFACES] = {&abromDeviceDescriptorCDC};
//const void *usbConfigurationDescriptors[NONCOMP_NUM_USB_INTERFACES] = {&abromConfigurationDescriptorGroupCDC};
//const BYTE usbConfigurationsSizes[NONCOMP_NUM_USB_INTERFACES] = {sizeof(abromConfigurationDescriptorGroupCDC)};

//abromStringDescriptor
const ot_u8 usbdesc_string[] = {
	// String index0, language support
	4,		// Length of language descriptor ID
	3,		// LANGID tag
	0x09, 0x04,	// 0x0409 for English

	// String index1, Manufacturer
	18,		// Length of this string descriptor
	3,		// bDescriptorType
	'H',0, 'a',0, 'y',0, 's',0, 't',0, 'a',0, 'c',0, 'k',0,

	// String index2, Product
	18,		// Length of this string descriptor
	3,		// bDescriptorType
	'M',0, 'P',0, 'i',0, 'p',0, 'e',0, 'U',0, 'S',0, 'B',0,

	// String index3, Serial Number
	34,		// Length of this string descriptor
	3,		// bDescriptorType
	//0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	//0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0x46,0, 0x32,0, 0x42,0, 0x46,0, 0x39,0, 0x33,0, 0x34,0, 0x36,0,
	0x32,0, 0x46,0, 0x30,0, 0x30,0, 0x32,0, 0x32,0, 0x30,0, 0x30,0,

	// String index4, Configuration String: Needed for windows INF
	12,		// Length of this string descriptor
	3,		// bDescriptorType
	'm',0, 'p',0, 'i',0, 'p',0, 'e',0,

	// String index5, Interface String: Needed for windows INF
	22,		// Length of this string descriptor
	3,		// bDescriptorType
	'C',0, 'D',0, 'C',0, ' ',0, '(',0, 'V',0, 'C',0, 'O',0, 'M',0, ')',0
};


const usb_handle_struct usb_handle[CDC_NUM_INTERFACES] = {
///@todo this will need to be duplicated appropriately for multi-cdc devices
    {
        CDC0_INEP_ADDR, 
        CDC0_OUTEP_ADDR,
        1,                      //Interface ID (indexes to EP2)
        CDC_CLASS,
        IEP1_X_BUFFER_ADDRESS,  //CDC0 Interrupt buffer X
        IEP1_Y_BUFFER_ADDRESS,  //CDC0 Interrupt buffer Y
        OEP2_X_BUFFER_ADDRESS,  //CDC0 Host-Data-out (device-in) buffer X
        IEP2_X_BUFFER_ADDRESS,  //CDC0 Host-Data-in (device-out) buffer X
        OEP2_Y_BUFFER_ADDRESS,  //CDC0 Host-Data-out (device-in) buffer Y
        IEP2_Y_BUFFER_ADDRESS   //CDC0 Host-Data-in (device-out) buffer Y
    }
};


const usbcmd_struct usbcmd_list[] = {
    //---- CDC 0 Class Requests -----//
    // GET LINE CODING (false)
        USB_REQ_TYPE_INPUT | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
        USB_CDC_GET_LINE_CODING,
        0x00, 0x00,                             // always zero
        CDC0_COMM_INTERFACE, 0x00,              // CDC interface is 0
        0x07, 0x00,                             // Size of Structure (data length)
        0xff, &usbcdccmd_get_linecoding,

    // SET LINE CODING (false)
        USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
        USB_CDC_SET_LINE_CODING,
        0x00, 0x00,                                 // always zero
        CDC0_COMM_INTERFACE, 0x00,                  // CDC interface is 0
        0x07, 0x00,                                 // Size of Structure (data length)
        0xff, &usbcdccmd_set_linecoding,

    // SET CONTROL LINE STATE (false)
        USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
        USB_CDC_SET_CONTROL_LINE_STATE,
        0xff,0xff,                                 // Contains data
        CDC0_COMM_INTERFACE,0x00,                 // CDC interface is 0
        0x00,0x00,                                 // No further data
        0xcf,&usbcdccmd_set_ctlline,

    //---- USB Standard Requests -----//
    // clear device feature (false)
        USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
        USB_REQ_CLEAR_FEATURE,
        FEATURE_REMOTE_WAKEUP,0x00,         // feature selector
        0x00,0x00,
        0x00,0x00,
        0xff,&usbcmd_clear_devfeature,

    // clear endpoint feature (false)
        USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_ENDPOINT,
        USB_REQ_CLEAR_FEATURE,
        FEATURE_ENDPOINT_STALL,0x00,
        0xff,0x00,
        0x00,0x00,
        0xf7,&usbcmd_clear_epfeature,

    // get configuration (false)
        USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
        USB_REQ_GET_CONFIGURATION,
        0x00,0x00, 
        0x00,0x00, 
        0x01,0x00,
        0xff,&usbcmd_get_cfg,

    // get device descriptor (false)
        USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
        USB_REQ_GET_DESCRIPTOR,
        0xff,DESC_TYPE_DEVICE,              // bValueL is index and bValueH is type
        0xff,0xff,
        0xff,0xff,
        0xd0,&usbcmd_get_devdesc,

    // get configuration descriptor (false)
        USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
        USB_REQ_GET_DESCRIPTOR,
        0xff,DESC_TYPE_CONFIG,              // bValueL is index and bValueH is type
        0xff,0xff,
        0xff,0xff,
        0xd0,&usbcmd_get_cfgdesc,

    // get string descriptor (false)
        USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
        USB_REQ_GET_DESCRIPTOR,
        0xff,DESC_TYPE_STRING,              // bValueL is index and bValueH is type
        0xff,0xff,
        0xff,0xff,
        0xd0,&usbcmd_get_strdesc,

    // get interface (false)
        USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
        USB_REQ_GET_INTERFACE,
        0x00,0x00,
        0xff,0xff,
        0x01,0x00,
        0xf3,&usbcmd_get_intf,

    // get device status (false)
        USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
        USB_REQ_GET_STATUS,
        0x00,0x00,
        0x00,0x00,
        0x02,0x00,
        0xff,&usbcmd_get_devstatus,
    
    // get interface status (false)
        USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
        USB_REQ_GET_STATUS,
        0x00,0x00,
        0xff,0x00,
        0x02,0x00,
        0xf7,&usbcmd_get_intfstatus,
    
    // 	get endpoint status (false)
        USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_ENDPOINT,
        USB_REQ_GET_STATUS,
        0x00,0x00,
        0xff,0x00,
        0x02,0x00,
        0xf7,&usbcmd_get_epstatus,

    // set address (false)
        USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
        USB_REQ_SET_ADDRESS,
        0xff,0x00,
        0x00,0x00,
        0x00,0x00,
        0xdf,&usbcmd_set_address,

    // set configuration (false)
        USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
        USB_REQ_SET_CONFIGURATION,
        0xff,0x00,
        0x00,0x00,
        0x00,0x00,
        0xdf,&usbcmd_set_cfg,

    // set device feature (false)
        USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
        USB_REQ_SET_FEATURE,
        0xff,0x00,                      // feature selector
        0x00,0x00,
        0x00,0x00,
        0xdf,&usbcmd_set_devfeature,

    // set endpoint feature (false)
        USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_ENDPOINT,
        USB_REQ_SET_FEATURE,
        0xff,0x00,                      // feature selector
        0xff,0x00,                      // endpoint number <= 127
        0x00,0x00,
        0xd7,&usbcmd_set_epfeature,

    // set interface (false)
        USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
        USB_REQ_SET_INTERFACE,
        0xff,0x00,                      // feature selector
        0xff,0x00,                      // interface number
        0x00,0x00,
        0xd7,&usbcmd_set_intf,
    
    //---- USB Invalid Request -----//
    // this one will be matched to any USB request since bCompareMask is 0x00.
        0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 
        0x00, &usbcmd_invalid_request     // end of list
};




void usbdesc_set_serialnum() {
///@note You must arrange the starting offset with the LTV structure of the 
///usbdesc_string array
#   define _OFFSET  (4+36+34)
/*
    ot_u8   drec[8];
    ot_u8*  tis_compiler_can_kiss_my_ass;
    ot_u16* sn_cursor;
    
    //load in 8 byte ID from die record
    otutils_bin2hex((ot_u8*)0x1A0A, drec, 8);
    sn_cursor   = (ot_u16*)(usbdesc_string+_OFFSET+2);
    tis_compiler_can_kiss_my_ass = drec;
    
    ///2. Load the lower nibble from each one of four words in the die record
    while (sn_cursor != (ot_u16*)(usbdesc_string+_OFFSET+34)) {
        FLASH_WriteShort(sn_cursor++, (ot_u16)*tis_compiler_can_kiss_my_ass++);
    }
*/
#   undef _OFFSET

}






#endif

