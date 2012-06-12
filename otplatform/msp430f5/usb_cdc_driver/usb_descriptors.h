// (c)2010 by Texas Instruments Incorporated, All Rights Reserved.
/*-----------------------------------------------------------------------------+
|                                                                              |
|                              Texas Instruments                               |
|                                                                              |
|    This is an automatically generated script by MSP430 USB Descriptor Tool   |
|                                                                              |
|    Descriptor Tool Version: 3.0.10                                           |
|    Date: 2012/03/08 21:59:34                                                 |
|                                                                              |
|                                Descriptor.h                                  |
|-----------------------------------------------------------------------------*/

#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

#include "OT_platform.h"

// USB_API is stored in OTplatform/~MCU_MSP430/
#include "usb_cdc_driver/usb_types.h"


// Needed for CDC inteface
#if (MCU_FEATURE_MPIPEVCOM == ENABLED)
#define _CDC_
#endif


/** USB subsystem configuration  <BR>
  * ========================================================================<BR>
  * Constants that control the way the module compiles and behaves.
  */
#define USBEVT_CLOCKFAULT    	0x0001
#define USBEVT_VBUSON           0x0002
#define USBEVT_VBUSOFF          0x0004
#define USBEVT_RESET            0x0008
#define USBEVT_SUSPEND          0x0010
#define USBEVT_RESUME           0x0020
#define USBEVT_RXBUFFERED       0x0040
#define USBEVT_TXCOMPLETE       0x0080
#define USBEVT_RXCOMPLETE       0x0100
#define USBEVT_ALL              0x01FF

///@note You should edit this line below to control which events will be
///      handled in firmware.
#define USBEVT_MASK      (   USBEVT_VBUSON        | \
                             USBEVT_VBUSOFF       | \
                             USBEVT_RESET         | \
                            /* USBEVT_SUSPEND      | */\
                            /* USBEVT_RESUME       | */\
                            /* USBEVT_RXBUFFERED   | */ \
                             USBEVT_TXCOMPLETE   | \
                             USBEVT_RXCOMPLETE   | \
                             0 )



// Configuration Constants that can change
// #define that relates to Device Descriptor
#define USB_VID_CDC             0x2047              // Vendor ID (VID)
#define USB_PID_CDC             0x1                 // Product ID (PID)
#define VER_FW_H                0x02                // Device release number, in binary-coded decimal
#define VER_FW_L                0x00                // Device release number, in binary-coded decimal

// If a serial number is to be reported, set this to the index within the string 
// descriptor of the dummy serial number string.  It will then be automatically 
// handled by the API.  If no serial number is to be reported, set this to 0.
#define USB_STR_INDEX_SERNUM    3             
#define USB_NUM_INTERFACES      2                   // Number of implemented interfaces.

#define CDC0_COMM_INTERFACE     0                   // Comm interface number of CDC0
#define CDC0_DATA_INTERFACE     1                   // Data interface number of CDC0
#define CDC0_INTEP_ADDR         0x81                // Interrupt Endpoint Address of CDC0
#define CDC0_OUTEP_ADDR         0x02                // Output Endpoint Address of CDC0
#define CDC0_INEP_ADDR          0x82                // Input Endpoint Address of CDC0

#define CDC_NUM_INTERFACES      1                   //  Total Number of CDCs implemented. should set to 0 if there are no CDCs implemented.

// Interface numbers for the implemented CDSs and HIDs, 
// This is to use in the Application(main.c) and in the interupt file(UsbIsr.c).
#define CDC0_INTFNUM            0
#define MSC_MAX_LUN_NUMBER      1           // Maximum number of LUNs supported

#define PUTot_u16(x)              ((x)&0xFF),((x)>>8)

#define USB_OUTEP_INT_EN        (BIT0 | BIT2) 
#define USB_INEP_INT_EN         (BIT0 | BIT1 | BIT2) 

#define USB_MCLK_FREQ           PLATFORM_HSCLOCK_HZ     // MCLK frequency of MCU, in Hz
#define USB_DMA_CHAN            0x00                    // Set to 0xFF if no DMA channel will be used 0..7 for selected DMA channel





// wTotalLength, This is the sum of:
//      configuration descriptor length + 
//      CDC descriptor length + 
//      HID descriptor length
#define DESCRIPTOR_TOTAL_LENGTH 67 

// Controls whether the remote wakeup feature is supported by this device.
// A value of 0x20 indicates that is it supported (this value is the mask for
// the bmAttributes field in the configuration descriptor).
// A value of zero indicates remote wakeup is not supported.
// Other values are undefined, as they will interfere with bmAttributes.
#define USB_SUPPORT_REM_WAKE    MPIPE_USB_REMWAKE

// Controls whether the application is self-powered to any degree.  Should be
// set to 0x40, unless the USB device is fully supplied by the bus.
#define USB_SUPPORT_SELF_POWERED MPIPE_USB_POWERING

#define USB_PLL_XT              MPIPE_USB_XTAL          // Defines which XT is used by the PLL (1=XT1, 2=XT2)
#define USB_XT_FREQ             MPIPE_USB_XTFREQ        // Indicates the freq of the crystal on the oscillator indicated by USB_PLL_XT
#define USB_DISABLE_XT_SUSPEND  MPIPE_USB_XTSUSPEND     // If non-zero, then USB_suspend() will disable the oscillator
                                                        // that is designated by USB_PLL_XT; if zero, USB_suspend won't
                                                        // affect the oscillator
                                                        
// Controls what the device reports to the host regarding how much power it will
// consume from VBUS.  Expressed in 2mA units; that is, the number of mA
// communicated is twice the value of this field.
#define USB_MAX_POWER           MPIPE_USB_MAXPOWER




// Configuration constants that can not change ( Fixed Values)
#define CDC_CLASS               2
#define HID_CLASS               3
#define MSC_CLASS               4
#define PHDC_CLASS              5
#define MAX_PACKET_SIZE         0x40


// DESCRIPTOR CONSTANTS
#define SIZEOF_DEVICE_DESCRIPTOR  0x12
//#define SIZEOF_REPORT_DESCRIPTOR  36
//#define USBHID_REPORT_LENGTH      64  // length of whole HID report (including Report ID)
#define CONFIG_STRING_INDEX       4
#define INTF_STRING_INDEX         5
#define USB_CONFIG_VALUE          0x01



// OUTWARD DECLARATIONS
//#define NON_COMPOSITE_MULTIPLE_INTERFACES   1
#define NONCOMP_NUM_USB_INTERFACES          1
#define CDC_INTERFACE_INDEX                 0

//Calculates the endpoint descriptor block number from given address
#define EDB(addr)                           ((addr&0x07)-1)

// Structure for generic part of configuration descriptor
struct abromConfigurationDescriptorGenric {
	ot_u8 sizeof_config_descriptor;            // bLength
 	ot_u8 desc_type_config;                    // bDescriptorType: 2
	ot_u8 sizeof_configuration_descriptor1;    // wTotalLength
	ot_u8 sizeof_configuration_descriptor2;
	ot_u8 usb_num_configurations;              // bNumInterfaces
	ot_u8 bconfigurationvalue;                 // bConfigurationValue
	ot_u8 config_string_index;                 // iConfiguration Description offset
 	ot_u8 mattributes;                         // bmAttributes, bus power, remote wakeup
	ot_u8 usb_max_power;                       // Max. Power Consumption at 2mA unit
};

// CDC Descriptor
struct abromConfigurationDescriptorCdc {
    // interface descriptor (9 bytes)
    ot_u8 blength_intf;	                      // blength: interface descriptor size
    ot_u8 desc_type_interface;	              // bdescriptortype: interface
    ot_u8 interface_number_cdc;                // binterfacenumber
    ot_u8 balternatesetting;                   // balternatesetting: alternate setting
    ot_u8 bnumendpoints;                       // bnumendpoints: three endpoints used
    ot_u8 binterfaceclass;                     // binterfaceclass: communication interface class
    ot_u8 binterfacesubclass;                  // binterfacesubclass: abstract control model
    ot_u8 binterfaceprotocol;                  // binterfaceprotocol: common at commands 
    ot_u8 intf_string_index;	                  // interface:
    
    //header functional descriptor
    ot_u8 blength_header;	                  // blength: endpoint descriptor size
    ot_u8 bdescriptortype_header;	          // bdescriptortype: cs_interface
    ot_u8 bdescriptorsubtype_header;	          // bdescriptorsubtype: header func desc
    ot_u8 bcdcdc1;
    ot_u8 bcdcdc2;	                          // bcdcdc: spec release number

    //call managment functional descriptor
    ot_u8 bfunctionlength;	                  // bfunctionlength
    ot_u8 bdescriptortype_c;	                  // bdescriptortype: cs_interface
    ot_u8 bdescriptorsubtype_c;	              // bdescriptorsubtype: call management func desc
    ot_u8 bmcapabilities;	                  // bmcapabilities: d0+d1
    ot_u8 intf_number_cdc;                     // bdatainterface: 0

    //acm functional descriptor
    ot_u8 bfunctionlength_acm;	              // bfunctionlength
    ot_u8 bdescriptortype_acm;	              // bdescriptortype: cs_interface
    ot_u8 bdescriptorsubtype_acm;	          // bdescriptorsubtype: abstract control management desc
    ot_u8 bmcapabilities_acm;	              // bmcapabilities

    // Union Functional Descriptor
    ot_u8 bLength_ufd;                         // Size, in bytes
    ot_u8 bdescriptortype_ufd;                 // bDescriptorType: CS_INTERFACE
    ot_u8 bdescriptorsubtype_ufd;              // bDescriptorSubtype: Union Functional Desc
    ot_u8 bmasterinterface_ufd;                // bMasterInterface -- the controlling intf for the union
    ot_u8 bslaveinterface_ufd;                 // bSlaveInterface -- the controlled intf for the union

    //Interrupt end point related fields
    ot_u8 sizeof_epintep_descriptor;           // blength: endpoint descriptor size
    ot_u8 desc_type_epintep;	                  // bdescriptortype: endpoint
    ot_u8 cdc_intep_addr;	                  // bendpointaddress: (in2)
    ot_u8 epintep_desc_attr_type_int;	      // bmattributes: interrupt
    ot_u8 epintep_wmaxpacketsize1;
    ot_u8 epintep_wmaxpacketsize;   		      // wmaxpacketsize, 64 bytes
    ot_u8 epintep_binterval;                   // binterval

    // Data interface descriptor (9 bytes)
    ot_u8 blength_slaveintf;	                  // blength: interface descriptor size
    ot_u8 desc_type_slaveinterface;	          // bdescriptortype: interface
    ot_u8 interface_number_slavecdc;           // binterfacenumber
    ot_u8 balternatesetting_slave;             // balternatesetting: alternate setting
    ot_u8 bnumendpoints_slave;                 // bnumendpoints: three endpoints used
    ot_u8 binterfaceclass_slave;               // binterfaceclass: data interface class
    ot_u8 binterfacesubclass_slave;            // binterfacesubclass: abstract control model
    ot_u8 binterfaceprotocol_slave;            // binterfaceprotocol: common at commands
    ot_u8 intf_string_index_slave;	          // interface:

    // Bulk out end point related fields
    ot_u8 sizeof_outep_descriptor;             // blength: endpoint descriptor size
    ot_u8 desc_type_outep;	                  // bdescriptortype: endpoint
    ot_u8 cdc_outep_addr;	                  // bendpointaddress: (out3)
    ot_u8 outep_desc_attr_type_bulk;	          // bmattributes: bulk
    ot_u8 outep_wmaxpacketsize1;
    ot_u8 outep_wmaxpacketsize2;               // wmaxpacketsize, 64 bytes
    ot_u8 outep_binterval; 	                  // binterval: ignored for bulk transfer

    // Bulk in related fields
    ot_u8 sizeof_inep_descriptor;              // blength: endpoint descriptor size
    ot_u8 desc_type_inep;	                  // bdescriptortype: endpoint
    ot_u8 cdc_inep_addr;	                      // bendpointaddress: (in3)
    ot_u8 inep_desc_attr_type_bulk;	          // bmattributes: bulk
    ot_u8 inep_wmaxpacketsize1;
    ot_u8 inep_wmaxpacketsize2;  		      // wmaxpacketsize, 64 bytes
    ot_u8 inep_binterval;	                  // binterval: ignored for bulk transfer
};


//HID descriptor structure 
struct abromConfigurationDescriptorHid {
    //INTERFACE DESCRIPTOR (9 bytes)
    ot_u8 sizeof_interface_descriptor;        // Desc Length
    ot_u8 desc_type_interface;                // DescriptorType
    ot_u8 interface_number_hid;               // Interface number
    ot_u8 balternatesetting;                  // Any alternate settings if supported
    ot_u8 bnumendpoints;                      // Number of end points required
    ot_u8 binterfaceclass;                    // Class ID
    ot_u8 binterfacesubclass;                 // Sub class ID
    ot_u8 binterfaceprotocol;                 // Protocol
    ot_u8 intf_string_index;                  // String Index

    //hid descriptor (9 bytes)
    ot_u8 blength_hid_descriptor;             // HID Desc length
    ot_u8 hid_descriptor_type;                // HID Desc Type
    ot_u8 hidrevno1;                          // Rev no 
    ot_u8 hidrevno2;                          // Rev no - 2nd part
    ot_u8 tcountry;	    	                  // Country code 
    ot_u8 numhidclasses;                      // Number of HID classes to follow	
    ot_u8 report_descriptor_type;             // Report desc type 
    ot_u8 tlength;                            // Total length of report descriptor
    ot_u8 size_rep_desc;

    //input end point descriptor (7 bytes)
    ot_u8 size_inp_endpoint_descriptor;       // End point desc size
    ot_u8 desc_type_inp_endpoint;             // Desc type
    ot_u8 hid_inep_addr;                      // Input end point address
    ot_u8 ep_desc_attr_type_inp_int;          // Type of end point
    ot_u8 inp_wmaxpacketsize1;               // Max packet size
    ot_u8 inp_wmaxpacketsize2;
    ot_u8 inp_binterval;                      // bInterval in ms

    // Output end point descriptor; (7 bytes)
    ot_u8 size_out_endpoint_descriptor;       // Output endpoint desc size
    ot_u8 desc_type_out_endpoint;             // Desc type
    ot_u8 hid_outep_addr;                     // Output end point address
    ot_u8 ep_desc_attr_type_out_int;          // End point type
    ot_u8 out_wmaxpacketsize1;                // Max packet size
    ot_u8 out_wmaxpacketsize2;
    ot_u8 out_binterval;                      // bInterval in ms
};

//MSC descriptor structure 
struct abromConfigurationDescriptorMsc {
    // INTERFACE DESCRIPTOR (9 bytes)
    ot_u8 sizeof_interface_descriptor;         // Desc Length
    ot_u8 desc_type_interface;                 // DescriptorType
    ot_u8 interface_number_hid;                // Interface number
    ot_u8 balternatesetting;                   // Any alternate settings if supported
    ot_u8 bnumendpoints;                       // Number of end points required
    ot_u8 binterfaceclass;                     // Class ID
    ot_u8 binterfacesubclass;                  // Sub class ID
    ot_u8 binterfaceprotocol;                  // Protocol
    ot_u8 intf_string_index;                   // String Index

    // input end point descriptor (7 bytes)
    ot_u8 size_inp_endpoint_descriptor;        // End point desc size
    ot_u8 desc_type_inp_endpoint;              // Desc type
    ot_u8 hid_inep_addr;                       // Input end point address
    ot_u8 ep_desc_attr_type_inp_int;           // Type of end point
    ot_u8 inp_wmaxpacketsize1;                // Max packet size
    ot_u8 inp_wmaxpacketsize2;
    ot_u8 inp_binterval;                       // bInterval in ms

    // Output end point descriptor; (7 bytes)
    ot_u8 size_out_endpoint_descriptor;        // Output endpoint desc size
    ot_u8 desc_type_out_endpoint;              // Desc type
    ot_u8 hid_outep_addr;                      // Output end point address
    ot_u8 ep_desc_attr_type_out_int;           // End point type
    ot_u8 out_wmaxpacketsize1;                 // Max packet size
    ot_u8 out_wmaxpacketsize2;
    ot_u8 out_binterval;                       // bInterval in ms
};

//struct  abromConfigurationDescriptorGroupCDC {
    // Generic part of config descriptor
//    const struct abromConfigurationDescriptorGenric abromConfigurationDescriptorGenricCDC;
//    const struct abromConfigurationDescriptorCdc    stCdc[CDC_NUM_INTERFACES];
//};

// Global structure having Generic,CDC,HID, MSC structures 
struct  abromConfigurationDescriptorGroup {
    // Generic part of config descriptor
    const struct abromConfigurationDescriptorGenric abromConfigurationDescriptorGenric;
    const struct abromConfigurationDescriptorCdc    stCdc[CDC_NUM_INTERFACES];
};

extern const struct abromConfigurationDescriptorGroup abromConfigurationDescriptorGroup;
extern const ot_u8 abromDeviceDescriptor[SIZEOF_DEVICE_DESCRIPTOR];
extern const ot_u8 abromStringDescriptor[];
//extern const ot_u8 abromReportDescriptor[SIZEOF_REPORT_DESCRIPTOR];

// Handle Structure - Will be populated in descriptors.c based on number of CDC,HID interfaces 
struct tUsbHandle {
    ot_u8 ep_In_Addr;               // Input EP Addr 
    ot_u8 ep_Out_Addr;              // Output EP Addr 
    ot_u8 edb_Index;                // The EDB index 
    ot_u8 dev_Class;                // Device Class- 2 for CDC, 3 for HID 
    ot_u16 intepEP_X_Buffer;         // Interupt X Buffer Addr 
    ot_u16 intepEP_Y_Buffer;         // Interupt Y Buffer Addr 
    ot_u16 oep_X_Buffer;             // Output X buffer Addr 
    ot_u16 oep_Y_Buffer;             // Output Y buffer Addr 
    ot_u16 iep_X_Buffer;             // Input X Buffer Addr 
    ot_u16 iep_Y_Buffer;             // Input  Y Buffer Addr 
};

extern const struct tUsbHandle stUsbHandle[CDC_NUM_INTERFACES];
extern const tDEVICE_REQUEST_COMPARE tUsbRequestList[];


#endif

