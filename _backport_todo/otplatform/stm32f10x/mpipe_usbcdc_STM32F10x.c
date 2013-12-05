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
  * @file       /otplatform/stm32f10x/mpipe_usbcdc_STM32F10x.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 Jul 2011
  * @brief      Message Pipe (MPIPE) USB Virtual COM implementation for STM32F10x
  * @defgroup   MPipe (Message Pipe)
  * @ingroup    MPipe
  *
  * Baudrates supported:    9600, 115200                                        <BR>
  * Byte structure:         8N1                                                 <BR>
  * Duplex:                 Half                                                <BR>
  * Flow control:           Custom, ACK-based                                   <BR>
  * Connection:             RS-232, DTE-DTE (use a null-modem connector)        <BR><BR>
  * 
  * Implemented Mpipe Protocol:                                                 <BR>
  * The Mpipe protocol is a simple wrapper to NDEF.                             <BR>
  * Legend: [ NDEF Header ] [ NDEF Payload ] [ Seq. Number ] [ CRC16 ]          <BR>
  * Bytes:        6             <= 255             2             2              <BR><BR>
  *
  * The protocol includes an ACK/NACK feature.  After receiving a message, the  <BR>
  * Mpipe send an ACK/NACK.  The "YY" byte is 0 for ACK and non-zero for ACK.   <BR>
  * Presently, 0x7F is used as the YY NACK value.                               <BR>
  * [ Seq ID ] 0xDD 0x00 0x00 0x02 0x00 0xYY  [ CRC16 ]
  ******************************************************************************
  */


#include "OT_config.h"
#include "OT_platform.h"

/// Do not compile if MPIPE is disabled, or MPIPE does not use USB VCOM
#if ((OT_FEATURE(MPIPE) == ENABLED) && (MCU_CONFIG(MPIPECDC) == ENABLED))

#include "mpipe.h"
#include "OT_utils.h"

#include "usb_conf.h"   //local file
#include "usb_lib.h"




/** Mpipe Module Data (used by all Mpipe implementations)   <BR>
  * ========================================================================<BR>
  * At present this consumes 24 bytes of SRAM.  6 bytes could be freed by
  * removing the callbacks, which might not be used.
  */
  
// Footer is 2 byte sequence ID + CRC (usually 2 bytes, but could be more)
#define MPIPE_FOOTERBYTES   4


typedef struct {
    mpipe_state     state;
    ot_uni16        sequence;
    ot_u8*          pktbuf;
    ot_int          pktlen;
    
#   if (OT_FEATURE(MPIPE_CALLBACKS) == ENABLED)
        ot_sigv sig_rxdone;
        ot_sigv sig_txdone;
        ot_sigv sig_rxdetect;
#   endif
} mpipe_struct;


typedef struct {
    ot_int i;
} mpipe_ext_struct;


mpipe_struct        mpipe;
mpipe_ext_struct    mpipe_ext;
alp_tmpl            mpipe_alp;




/** Mpipe Main Subroutine Prototypes   <BR>
  * ========================================================================
  */
void sub_usb_loadtx();
void sub_usb_portsetup();






/** Mpipe Virtual COM Constants for STM32 implementation & driver   <BR>
  * ========================================================================<BR>
  */
#ifndef STM32F10X_CL
//#   define CTR_CALLBACK
//#   define DOVR_CALLBACK
//#   define ERR_CALLBACK
//#   define WKUP_CALLBACK
//#   define SUSP_CALLBACK
//#   define RESET_CALLBACK
//#   define SOF_CALLBACK
//#   define ESOF_CALLBACK
#endif

#ifdef STM32F10X_CL
//                              FIFO Size Configuration
//  
//  (i) Dedicated data FIFO SPRAM of 1.25 Kbytes = 1280 bytes = 320 32-bits words
//      available for the endpoints IN and OUT.
//      Device mode features:
//      -1 bidirectional CTRL EP 0
//      -3 IN EPs to support any kind of Bulk, Interrupt or Isochronous transfer
//      -3 OUT EPs to support any kind of Bulk, Interrupt or Isochronous transfer
//
//  ii) Receive data FIFO size = RAM for setup packets + 
//                   OUT endpoint control information +
//                   data OUT packets + miscellaneous
//      Space = ONE 32-bits words
//     --> RAM for setup packets = 4 * n + 6 space
//        (n is the nbr of CTRL EPs the device core supports) 
//     --> OUT EP CTRL info      = 1 space
//        (one space for status information written to the FIFO along with each 
//        received packet)
//     --> data OUT packets      = (Largest Packet Size / 4) + 1 spaces 
//        (MINIMUM to receive packets)
//     --> OR data OUT packets  = at least 2*(Largest Packet Size / 4) + 1 spaces 
//        (if high-bandwidth EP is enabled or multiple isochronous EPs)
//     --> miscellaneous = 1 space per OUT EP
//        (one space for transfer complete status information also pushed to the 
//        FIFO with each endpoint's last packet)
//
//  (iii)MINIMUM RAM space required for each IN EP Tx FIFO = MAX packet size for 
//       that particular IN EP. More space allocated in the IN EP Tx FIFO results
//       in a better performance on the USB and can hide latencies on the AHB.
//
//  (iv) TXn min size = 16 words. (n  : Transmit FIFO index)
//   (v) When a TxFIFO is not used, the Configuration should be as follows: 
//       case 1 :  n > m    and Txn is not used    (n,m  : Transmit FIFO indexes)
//       --> Txm can use the space allocated for Txn.
//       case2  :  n < m    and Txn is not used    (n,m  : Transmit FIFO indexes)
//       --> Txn should be configured with the minimum space of 16 words
//  (vi) The FIFO is used optimally when used TxFIFOs are allocated in the top 
//       of the FIFO.Ex: use EP1 and EP2 as IN instead of EP1 and EP3 as IN ones.

#   define RX_FIFO_SIZE                          128
#   define TX0_FIFO_SIZE                          64
#   define TX1_FIFO_SIZE                          64
#   define TX2_FIFO_SIZE                          16
#   define TX3_FIFO_SIZE                          16

    // OTGD-FS-DEVICE IP interrupts Enable definitions
    // Uncomment the define to enable the selected interrupt
//# define INTR_MODEMISMATCH
//#   define INTR_SOFINTR
#   define INTR_RXSTSQLVL           // Mandatory
//# define INTR_NPTXFEMPTY
//# define INTR_GINNAKEFF
//# define INTR_GOUTNAKEFF
//# define INTR_ERLYSUSPEND
#   define INTR_USBSUSPEND          // Mandatory
#   define INTR_USBRESET            // Mandatory
#   define INTR_ENUMDONE            // Mandatory
//# define INTR_ISOOUTDROP
//# define INTR_EOPFRAME
//# define INTR_EPMISMATCH
#   define INTR_INEPINTR            // Mandatory
#   define INTR_OUTEPINTR           // Mandatory
//# define INTR_INCOMPLISOIN
//# define INTR_INCOMPLISOOUT
#   define INTR_WKUPINTR            // Mandatory

    // OTGD-FS-DEVICE IP interrupts subroutines
    // Comment the define to enable the selected interrupt subroutine and replace it
    // by user code
#   define  INTR_MODEMISMATCH_Callback      NOP_Process
#   define  INTR_SOFINTR_Callback           NOP_Process
#   define  INTR_RXSTSQLVL_Callback         NOP_Process
#   define  INTR_NPTXFEMPTY_Callback        NOP_Process
#   define  INTR_NPTXFEMPTY_Callback        NOP_Process
#   define  INTR_GINNAKEFF_Callback         NOP_Process
#   define  INTR_GOUTNAKEFF_Callback        NOP_Process
#   define  INTR_ERLYSUSPEND_Callback       NOP_Process
#   define  INTR_USBSUSPEND_Callback        NOP_Process
#   define  INTR_USBRESET_Callback          NOP_Process
#   define  INTR_ENUMDONE_Callback          NOP_Process
#   define  INTR_ISOOUTDROP_Callback        NOP_Process
#   define  INTR_EOPFRAME_Callback          NOP_Process
#   define  INTR_EPMISMATCH_Callback        NOP_Process
#   define  INTR_INEPINTR_Callback          NOP_Process
#   define  INTR_OUTEPINTR_Callback         NOP_Process
#   define  INTR_INCOMPLISOIN_Callback      NOP_Process
#   define  INTR_INCOMPLISOOUT_Callback     NOP_Process
#   define  INTR_WKUPINTR_Callback          NOP_Process
#   define  INTR_RXSTSQLVL_ISODU_Callback   NOP_Process  // Isochronous data update

// Isochronous transfer parameters
#   define ISOC_BUFFER_SZE  1   // Size of a single Isochronous buffer/transfer
#   define NUM_SUB_BUFFERS  2   // Number of sub-buffers, should be even

#endif 



#define VCOMPORT_IN_FRAME_INTERVAL  5

#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05
#define VIRTUAL_COM_PORT_DATA_SIZE              64
#define VIRTUAL_COM_PORT_INT_SIZE               8
#define VIRTUAL_COM_PORT_SIZ_DEVICE_DESC        18
#define VIRTUAL_COM_PORT_SIZ_CONFIG_DESC        67
#define VIRTUAL_COM_PORT_SIZ_STRING_LANGID      4
#define VIRTUAL_COM_PORT_SIZ_STRING_VENDOR      (20+2)
#define VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT     (32+2)
#define VIRTUAL_COM_PORT_SIZ_STRING_SERIAL      (24+2)
#define STANDARD_ENDPOINT_DESC_SIZE             0x09

#define Virtual_Com_Port_GetConfiguration          NOP_Process
//#define Virtual_Com_Port_SetConfiguration          NOP_Process
#define Virtual_Com_Port_GetInterface              NOP_Process
#define Virtual_Com_Port_SetInterface              NOP_Process
#define Virtual_Com_Port_GetStatus                 NOP_Process
#define Virtual_Com_Port_ClearFeature              NOP_Process
#define Virtual_Com_Port_SetEndPointFeature        NOP_Process
#define Virtual_Com_Port_SetDeviceFeature          NOP_Process
//#define Virtual_Com_Port_SetDeviceAddress          NOP_Process

#define SEND_ENCAPSULATED_COMMAND   0x00
#define GET_ENCAPSULATED_RESPONSE   0x01
#define SET_COMM_FEATURE            0x02
#define GET_COMM_FEATURE            0x03
#define CLEAR_COMM_FEATURE          0x04
#define SET_LINE_CODING             0x20
#define GET_LINE_CODING             0x21
#define SET_CONTROL_LINE_STATE      0x22
#define SEND_BREAK                  0x23













/** Mpipe Module ISR & Callbacks Prototypes   <BR>
  * ========================================================================<BR>
  */
#ifndef STM32F10X_CL
void USB_Istr(void);
u32 STM32_PCD_OTG_ISR_Handler (void);
#endif

void EP1_IN_Callback (void);
void EP3_OUT_Callback(void);


#ifdef CTR_CALLBACK
    void CTR_Callback(void);
#else
#   define CTR_Callback()   while(0)
#endif
#ifdef DOVR_CALLBACK
    void DOVR_Callbacx(void);
#else
#   define DOVR_Callback()  while(0)
#endif
#ifdef ERR_CALLBACK
    void ERR_Callback(void);
#else
#   define ERR_Callback()   while(0)
#endif
#ifdef WKUP_CALLBACK
    void WKUP_Callback(void);
#else
#   define WKUP_Callback()   while(0)
#endif
#ifdef SUSP_CALLBACK
    void SUSP_Callback(void);
#else
#   define SUSP_Callback()   while(0)
#endif
#ifdef RESET_CALLBACK
    void RESET_Callback(void);
#else
#   define RESET_Callback()   while(0)
#endif
#ifdef SOF_CALLBACK
#   ifdef STM32F10X_CL
        void INTR_SOFINTR_Callback(void);
#   else
        void SOF_Callback(void);
#   endif
#else
#   define SOF_Callback()   while(0)
#endif
#ifdef ESOF_CALLBACK
    void ESOF_Callback(void);
#else
#   define ESOF_Callback()   while(0)
#endif







#if (ISR_EMBED(MPIPE) == ENABLED)
#   ifndef STM32F10X_CL
        void USB_LP_CAN1_RX0_IRQHandler(void);
#   endif
#   ifdef STM32F10X_CL
        void OTG_FS_IRQHandler(void);
#   endif
#endif





/** Mpipe Virtual COM Local Typedefs  <BR>
  * ========================================================================<BR>
  */

typedef enum _RESUME_STATE {
    RESUME_EXTERNAL,
    RESUME_INTERNAL,
    RESUME_LATER,
    RESUME_WAIT,
    RESUME_START,
    RESUME_ON,
    RESUME_OFF,
    RESUME_ESOF
} RESUME_STATE;

typedef enum _DEVICE_STATE {
    UNCONNECTED,
    ATTACHED,
    POWERED,
    SUSPENDED,
    ADDRESSED,
    CONFIGURED
} DEVICE_STATE;

typedef struct {
    ot_u32 bitrate;
    ot_u8 format;
    ot_u8 paritytype;
    ot_u8 datatype;
} LINE_CODING;






/** Mpipe Module Subroutine Prototypes   <BR>
  * ========================================================================<BR>
  */
RESULT PowerOn(void);
RESULT PowerOff();
void Suspend(void);
void Resume_Init(void);
void Resume(RESUME_STATE eResumeSetVal);

void Virtual_Com_Port_init(void);
void Virtual_Com_Port_Reset(void);
void Virtual_Com_Port_SetConfiguration(void);
void Virtual_Com_Port_SetDeviceAddress(void);
void Virtual_Com_Port_Status_In(void);
void Virtual_Com_Port_Status_Out(void);
RESULT Virtual_Com_Port_Data_Setup(ot_u8 RequestNo);
RESULT Virtual_Com_Port_NoData_Setup(ot_u8 RequestNo);
ot_u8 *Virtual_Com_Port_GetDeviceDescriptor(ot_u16 Length);
ot_u8 *Virtual_Com_Port_GetConfigDescriptor(ot_u16 Length);
ot_u8 *Virtual_Com_Port_GetStringDescriptor(ot_u16 Length);
RESULT Virtual_Com_Port_Get_Interface_Setting(ot_u8 Interface, ot_u8 AlternateSetting);
ot_u8 *Virtual_Com_Port_GetLineCoding(ot_u16 Length);
ot_u8 *Virtual_Com_Port_SetLineCoding(ot_u16 Length);

void Handle_USBAsynchXfer (void);






/** Mpipe Virtual COM Data Elements   <BR>
  * ========================================================================<BR>
  */
//ot_u8 USB_Rx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
//ot_u8 USB_Tx_State;
//extern  ot_u8 USART_Rx_Buffer[];
//extern ot_u32 USART_Rx_ptr_out;
//extern ot_u32 USART_Rx_length;


__IO ot_u32 bDeviceState    = UNCONNECTED;  // USB device status
__IO bool   fSuspendEnabled = TRUE;         // true when suspend is possible
__IO ot_u16 wIstr;                          // ISTR register last read value 
__IO ot_u8  bIntPackSOF     = 0;            // SOFs received between 2 consecutive packets 

//ot_u8 Request = 0;


// USB Standard Device Descriptor: Set up for Little Endian
const u8 Virtual_Com_Port_DeviceDescriptor[] = {
    0x12,   /* bLength */
    USB_DEVICE_DESCRIPTOR_TYPE,     /* bDescriptorType */
    0x00, 0x02, /* bcdUSB = 2.00 */
    0x02,       /* bDeviceClass: CDC */
    0x00,       /* bDeviceSubClass */
    0x00,       /* bDeviceProtocol */
    0x40,       /* bMaxPacketSize0 */
    0x83, 0x04, /* idVendor = 0x0483 (STMicro) */
    0x40, 0x57, /* idProduct = 0x5740 (Virtual Com port) */
    0x00, 0x01, /* bcdDevice = 1.00 */
    1,          /* Index of string descriptor describing manufacturer */
    2,          /* Index of string descriptor describing product */
    3,          /* Index of string descriptor describing the device's serial number */
    0x01        /* bNumConfigurations */
};

const ot_u8 Virtual_Com_Port_ConfigDescriptor[] = {
    /*Configuation Descriptor*/
    0x09,   /* bLength: Configuation Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,      /* bDescriptorType: Configuration */
    VIRTUAL_COM_PORT_SIZ_CONFIG_DESC,       /* wTotalLength:no of returned bytes */
    0x00,
    0x02,   /* bNumInterfaces: 2 interface */
    0x01,   /* bConfigurationValue: Configuration value */
    0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
    0xC0,   /* bmAttributes: self powered or bus powered */
    (ot_u8)(500>>1),   /* MaxPower 500 mA */
    /*Interface Descriptor*/
    0x09,   /* bLength: Interface Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: Interface */
    /* Interface descriptor type */
    0x00,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x01,   /* bNumEndpoints: One endpoints used */
    0x02,   /* bInterfaceClass: Communication Interface Class */
    0x02,   /* bInterfaceSubClass: Abstract Control Model */
    0x01,   /* bInterfaceProtocol: Common AT commands */
    0x00,   /* iInterface: */
    /*Header Functional Descriptor*/
    0x05,   /* bLength: Endpoint Descriptor size */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x00,   /* bDescriptorSubtype: Header Func Desc */
    0x10,   /* bcdCDC: spec release number */
    0x01,
    /*Call Managment Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x01,   /* bDescriptorSubtype: Call Management Func Desc */
    0x00,   /* bmCapabilities: D0+D1 */ //@note might be 0x03
    0x01,   /* bDataInterface: 1 */
    /*ACM Functional Descriptor*/
    0x04,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
    0x02,   /* bmCapabilities */  //@note might be 0x06
    /*Union Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x06,   /* bDescriptorSubtype: Union func desc */
    0x00,   /* bMasterInterface: Communication class interface */
    0x01,   /* bSlaveInterface0: Data Class Interface */
    /*Endpoint 2 Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
    0x82,   /* bEndpointAddress: (IN2) */
    0x03,   /* bmAttributes: Interrupt */
    VIRTUAL_COM_PORT_INT_SIZE,      /* wMaxPacketSize: */
    0x00,
    0xFF,   /* bInterval: */
    /*Data class interface descriptor*/
    0x09,   /* bLength: Endpoint Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: */
    0x01,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x02,   /* bNumEndpoints: Two endpoints used */
    0x0A,   /* bInterfaceClass: CDC */
    0x00,   /* bInterfaceSubClass: */
    0x00,   /* bInterfaceProtocol: */
    0x00,   /* iInterface: */
    /*Endpoint 3 Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
    0x03,   /* bEndpointAddress: (OUT3) */
    0x02,   /* bmAttributes: Bulk */
    VIRTUAL_COM_PORT_DATA_SIZE,             /* wMaxPacketSize: */
    0x00,
    0x00,   /* bInterval: ignore for Bulk transfer */
    /*Endpoint 1 Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
    0x81,   /* bEndpointAddress: (IN1) */
    0x02,   /* bmAttributes: Bulk */
    VIRTUAL_COM_PORT_DATA_SIZE,             /* wMaxPacketSize: */
    0x00,
    0x00    /* bInterval */
};

/* USB String Descriptors */
const ot_u8 Virtual_Com_Port_StringLangID[VIRTUAL_COM_PORT_SIZ_STRING_LANGID] = {
    VIRTUAL_COM_PORT_SIZ_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09,
    0x04 /* LangID = 0x0409: U.S. English */
};

const ot_u8 Virtual_Com_Port_StringVendor[VIRTUAL_COM_PORT_SIZ_STRING_VENDOR] = {
    VIRTUAL_COM_PORT_SIZ_STRING_VENDOR,     /* Size of Vendor string */
    USB_STRING_DESCRIPTOR_TYPE,             /* bDescriptorType*/
    /* Vendor: "STMicro+OT" */
    'S', 0, 'T', 0, 'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, '+', 0,
    'O', 0, 'T', 0
};

const ot_u8 Virtual_Com_Port_StringProduct[VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT] = {
    VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT,          /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    /* Product name: "OpenTag MPipeUSB" */
    'O', 0, 'p', 0, 'e', 0, 'n', 0, 'T', 0, 'a', 0, 'g', 0, ' ', 0,
    'M', 0, 'P', 0, 'i', 0, 'p', 0, 'e', 0, 'U', 0, 'S', 0, 'B', 0
};

ot_u8 Virtual_Com_Port_StringSerial[VIRTUAL_COM_PORT_SIZ_STRING_SERIAL] = {
    VIRTUAL_COM_PORT_SIZ_STRING_SERIAL,           /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    'M', 0, 'P', 0, 'i', 0, 'p', 0, 'e', 0, 'U', 0, 'S', 0, 'B', 0,
    '0', 0, '0', 0, '0', 0, '0', 0
};



void (*pEpInt_IN[7])(void) = {
    EP1_IN_Callback,
    EP2_IN_Callback,
    EP3_IN_Callback,
    EP4_IN_Callback,
    EP5_IN_Callback,
    EP6_IN_Callback,
    EP7_IN_Callback,
};

void (*pEpInt_OUT[7])(void) = {
    EP1_OUT_Callback,
    EP2_OUT_Callback,
    EP3_OUT_Callback,
    EP4_OUT_Callback,
    EP5_OUT_Callback,
    EP6_OUT_Callback,
    EP7_OUT_Callback,
};

struct {
    __IO RESUME_STATE   eState;
    __IO ot_u8          bESOFcnt;
} ResumeS;


LINE_CODING linecoding = {
    115200, // baud rate
    0x00,   // stop bits-1
    0x00,   // parity - none
    0x08    // no. of bits 8
};

DEVICE Device_Table = {
    EP_NUM,
    1
};

DEVICE_PROP Device_Property = {
    Virtual_Com_Port_init,
    Virtual_Com_Port_Reset,
    NOP_Process,
    NOP_Process,
    Virtual_Com_Port_Data_Setup,
    Virtual_Com_Port_NoData_Setup,
    Virtual_Com_Port_Get_Interface_Setting,
    Virtual_Com_Port_GetDeviceDescriptor,
    Virtual_Com_Port_GetConfigDescriptor,
    Virtual_Com_Port_GetStringDescriptor,
    0,
    0x40 //MAX PACKET SIZE
};

USER_STANDARD_REQUESTS User_Standard_Requests = {
    Virtual_Com_Port_GetConfiguration,
    Virtual_Com_Port_SetConfiguration,
    Virtual_Com_Port_GetInterface,
    Virtual_Com_Port_SetInterface,
    Virtual_Com_Port_GetStatus,
    Virtual_Com_Port_ClearFeature,
    Virtual_Com_Port_SetEndPointFeature,
    Virtual_Com_Port_SetDeviceFeature,
    Virtual_Com_Port_SetDeviceAddress
};

ONE_DESCRIPTOR Device_Descriptor = {
    (ot_u8*)Virtual_Com_Port_DeviceDescriptor,
    VIRTUAL_COM_PORT_SIZ_DEVICE_DESC
};

ONE_DESCRIPTOR Config_Descriptor = {
    (ot_u8*)Virtual_Com_Port_ConfigDescriptor,
    VIRTUAL_COM_PORT_SIZ_CONFIG_DESC
};

ONE_DESCRIPTOR String_Descriptor[4] = {
    {(ot_u8*)Virtual_Com_Port_StringLangID, VIRTUAL_COM_PORT_SIZ_STRING_LANGID},
    {(ot_u8*)Virtual_Com_Port_StringVendor, VIRTUAL_COM_PORT_SIZ_STRING_VENDOR},
    {(ot_u8*)Virtual_Com_Port_StringProduct, VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT},
    {(ot_u8*)Virtual_Com_Port_StringSerial, VIRTUAL_COM_PORT_SIZ_STRING_SERIAL}
};









/** MPIPE Embedded ISRs  <BR>
  * ========================================================================<BR>
  * In the platform_config_~.h file, you should set the MPIPE to have embedded
  * ISRs if you want them to be included in this file.  In this case, they are
  * modularized from the rest of your code (which can be nice), but you can't
  * use the ISRs for other things.
  */

#if (ISR_EMBED(MPIPE) == ENABLED)
#   ifndef STM32F10X_CL
        void USB_LP_CAN1_RX0_IRQHandler(void) {
            USB_Istr(); //this is the one primarily used
        }
#   endif
#   ifdef STM32F10X_CL
        void OTG_FS_IRQHandler(void) {
            STM32_PCD_OTG_ISR_Handler();
        }
#   endif
#endif


    
    
    


/** USB Powering Routines  <BR>
  * ========================================================================
  */
#define USB_Cable_Config(VAL)       USB_Cable_Config_##VAL
#ifdef STM32F10X_CL 
#   define USB_Cable_Config_ENABLE  USB_DevConnect()
#   define USB_Cable_Config_DISABLE USB_DevDisconnect()
#else
#   define USB_Cable_Config_ENABLE  GPIO_ResetBits(MPIPE_USBDC_PORT, MPIPE_USBDC_PIN)
#   define USB_Cable_Config_DISABLE GPIO_SetBits(MPIPE_USBDC_PORT, MPIPE_USBDC_PIN)
#endif

#define Enter_LowPowerMode()    (bDeviceState = SUSPENDED)
#define Leave_LowPowerMode()    (bDeviceState = (Device_Info.Current_Configuration != 0) ? CONFIGURED : ATTACHED)


  
RESULT PowerOn(void) {
#ifndef STM32F10X_CL
    ot_u16 wRegVal;
    USB_Cable_Config(ENABLE);   // cable plugged-in?
    wRegVal = CNTR_FRES;        // CNTR_PWDN = 0
    _SetCNTR(wRegVal);
    wInterrupt_Mask = 0;        // CNTR_FRES = 0
    _SetCNTR(wInterrupt_Mask);
    _SetISTR(0);                // Clear pending interrupts 
    wInterrupt_Mask = CNTR_RESETM | CNTR_SUSPM | CNTR_WKUPM;
    _SetCNTR(wInterrupt_Mask);
#endif
    return USB_SUCCESS;
}


RESULT PowerOff() {
#ifndef STM32F10X_CL
    _SetCNTR(CNTR_FRES);                // disable all interrupts and force USB reset
    _SetISTR(0);                        // clear interrupt status register 
    USB_Cable_Config(DISABLE);          // Disable the Pull-Up  
    _SetCNTR(CNTR_FRES + CNTR_PDWN);    // switch-off device
#endif
    return USB_SUCCESS;
}


void Suspend(void) {
#ifndef STM32F10X_CL
    ot_u16 wCNTR;
    wCNTR   = _GetCNTR();     // macrocell enters suspend mode
    wCNTR  |= CNTR_FSUSP;
    _SetCNTR(wCNTR);
#endif
#ifndef STM32F10X_CL
    wCNTR   = _GetCNTR();     // force low-power mode in the macrocell 
    wCNTR  |= CNTR_LPMODE;
    _SetCNTR(wCNTR);
#endif
    Enter_LowPowerMode();
}


void Resume_Init(void) {
#ifndef STM32F10X_CL
    ot_u16 wCNTR;
    wCNTR   = _GetCNTR();       // CNTR_LPMODE = 0 
    wCNTR  &= (~CNTR_LPMODE);
    _SetCNTR(wCNTR);
#endif
    Leave_LowPowerMode();
#ifndef STM32F10X_CL
    _SetCNTR(IMR_MSK);          // reset FSUSP bit
#endif
}


void Resume(RESUME_STATE eResumeSetVal) {
#ifndef STM32F10X_CL
    ot_u16 wCNTR;
#endif
    if (eResumeSetVal != RESUME_ESOF)
        ResumeS.eState = eResumeSetVal;

    switch (ResumeS.eState) {
        case RESUME_EXTERNAL:   Resume_Init();
                                ResumeS.eState = RESUME_OFF;
                                break;
                                
        case RESUME_INTERNAL:   Resume_Init();
                                ResumeS.eState = RESUME_START;
                                break;
      
        case RESUME_LATER:      ResumeS.bESOFcnt = 2;
                                ResumeS.eState = RESUME_WAIT;
                                break;
                                
        case RESUME_WAIT:       ResumeS.bESOFcnt--;
                                if (ResumeS.bESOFcnt == 0)
                                    ResumeS.eState = RESUME_START;
                                break;
      
        case RESUME_START:      
#                           ifdef STM32F10X_CL
                                OTGD_FS_SetRemoteWakeup();
#                           else 
                                wCNTR   = _GetCNTR();
                                wCNTR  |= CNTR_RESUME;
                                _SetCNTR(wCNTR);
#                           endif 
                                ResumeS.eState = RESUME_ON;
                                ResumeS.bESOFcnt = 10;
                                break;
    
        case RESUME_ON:
#                           ifndef STM32F10X_CL      
                                ResumeS.bESOFcnt--;
                                if (ResumeS.bESOFcnt == 0)
#                           endif 
                                {
#                           ifdef STM32F10X_CL
                                    OTGD_FS_ResetRemoteWakeup();
#                           else
                                    wCNTR   = _GetCNTR();
                                    wCNTR  &= (~CNTR_RESUME);
                                    _SetCNTR(wCNTR);
#                           endif 
                                    ResumeS.eState = RESUME_OFF;
                                }
                                break;
        case RESUME_OFF:
        case RESUME_ESOF:
        default:                ResumeS.eState = RESUME_OFF;
                                break;
    }
}







/** USB Processing Routines  <BR>
  * ========================================================================
  */
static void IntToUnicode (ot_u32 value , ot_u8 *pbuf , ot_u8 len) {
    ot_u8 idx;
    len <<= 1;
  
    for(idx=0; idx<len; idx+=2) {
        pbuf[idx]   = (value >> 28);
        pbuf[idx]  += (pbuf[idx] < 0xA) ? '0' : ('A' - 10);
        pbuf[idx+1] = 0;
        value     <<= 4;
    }
}

void Get_SerialNum(void) {
    ot_u32 Device_Serial0, Device_Serial1, Device_Serial2;
    Device_Serial0  = *(__IO ot_u32*)(0x1FFFF7E8);
    Device_Serial1  = *(__IO ot_u32*)(0x1FFFF7EC);
    Device_Serial2  = *(__IO ot_u32*)(0x1FFFF7F0);
    Device_Serial0 += Device_Serial2;

    if (Device_Serial0 != 0) {
        IntToUnicode(Device_Serial0, &Virtual_Com_Port_StringSerial[2] , 8);
        IntToUnicode(Device_Serial1, &Virtual_Com_Port_StringSerial[18], 4);
    }
}

void Virtual_Com_Port_init(void) {
    /// Initialize VCOM CDC
    Get_SerialNum();            // Update the serial number string descriptor
    pInformation->Current_Configuration = 0;
    PowerOn();                  // Connect the device
    USB_SIL_Init();             // Perform basic device initialization operations
    //USART_Config_Default();     // configure the USART to the default settings
    bDeviceState = UNCONNECTED;
}


void Virtual_Com_Port_Reset(void) {
    // - Set Virtual_Com_Port DEVICE as not configured
    // - Current Feature initialization
    // - Set Virtual_Com_Port DEVICE with the default Interface
    pInformation->Current_Configuration = 0;  
    pInformation->Current_Feature       = Virtual_Com_Port_ConfigDescriptor[7];
    pInformation->Current_Interface     = 0;

#   ifdef STM32F10X_CL     
        // EP0 is already configured by USB_SIL_Init() function  
        // - Init EP1 IN as Bulk endpoint
        // - Init EP2 IN as Interrupt endpoint
        // - Init EP3 OUT as Bulk endpoint
        OTG_DEV_EP_Init(EP1_IN, OTG_DEV_EP_TYPE_BULK, VIRTUAL_COM_PORT_DATA_SIZE);
        OTG_DEV_EP_Init(EP2_IN, OTG_DEV_EP_TYPE_INT, VIRTUAL_COM_PORT_INT_SIZE);
        OTG_DEV_EP_Init(EP3_OUT, OTG_DEV_EP_TYPE_BULK, VIRTUAL_COM_PORT_DATA_SIZE);  
#   else 

    SetBTABLE(BTABLE_ADDRESS);

    // Initialize Endpoint 0
    SetEPType(ENDP0, EP_CONTROL);
    SetEPTxStatus(ENDP0, EP_TX_STALL);
    SetEPRxAddr(ENDP0, ENDP0_RXADDR);
    SetEPTxAddr(ENDP0, ENDP0_TXADDR);
    Clear_Status_Out(ENDP0);
    SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
    SetEPRxValid(ENDP0);

    // Initialize Endpoint 1 
    SetEPType(ENDP1, EP_BULK);
    SetEPTxAddr(ENDP1, ENDP1_TXADDR);
    SetEPTxStatus(ENDP1, EP_TX_NAK);
    SetEPRxStatus(ENDP1, EP_RX_DIS);

    // Initialize Endpoint 2 
    SetEPType(ENDP2, EP_INTERRUPT);
    SetEPTxAddr(ENDP2, ENDP2_TXADDR);
    SetEPRxStatus(ENDP2, EP_RX_DIS);
    SetEPTxStatus(ENDP2, EP_TX_NAK);

    // Initialize Endpoint 3
    SetEPType(ENDP3, EP_BULK);
    SetEPRxAddr(ENDP3, ENDP3_RXADDR);
    SetEPRxCount(ENDP3, VIRTUAL_COM_PORT_DATA_SIZE);
    SetEPRxStatus(ENDP3, EP_RX_VALID);
    SetEPTxStatus(ENDP3, EP_TX_DIS);

    // Set this device to response on default address 
    SetDeviceAddress(0);
#   endif

    bDeviceState = ATTACHED;
}

  
void Virtual_Com_Port_SetConfiguration(void) {
    DEVICE_INFO *pInfo = &Device_Info;
    if (pInfo->Current_Configuration != 0) {
        bDeviceState = CONFIGURED;  // Device configured
    }
}
  

void Virtual_Com_Port_SetDeviceAddress (void) {
    bDeviceState = ADDRESSED;
}


RESULT Virtual_Com_Port_Data_Setup(ot_u8 RequestNo) {
    ot_u8 *(*CopyRoutine)(ot_u16);

    CopyRoutine = NULL;
    if (RequestNo == GET_LINE_CODING) {
        if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) {
            CopyRoutine = Virtual_Com_Port_GetLineCoding;
        }
    }
    else if (RequestNo == SET_LINE_CODING) {
        if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) {
            CopyRoutine = Virtual_Com_Port_SetLineCoding;
        }
        //Request = SET_LINE_CODING;
    }
    if (CopyRoutine == NULL) {
        return USB_UNSUPPORT;
    }

    pInformation->Ctrl_Info.CopyData    = CopyRoutine;
    pInformation->Ctrl_Info.Usb_wOffset = 0;
    (*CopyRoutine)(0);
    return USB_SUCCESS;
}


RESULT Virtual_Com_Port_NoData_Setup(ot_u8 RequestNo) {
    return ((Type_Recipient == (CLASS_REQUEST|INTERFACE_RECIPIENT)) && \
            ((RequestNo == SET_COMM_FEATURE) || (RequestNo == SET_CONTROL_LINE_STATE))) ? \
            USB_SUCCESS : USB_UNSUPPORT;
}


ot_u8 *Virtual_Com_Port_GetDeviceDescriptor(ot_u16 Length) {
    return Standard_GetDescriptorData(Length, &Device_Descriptor);
}


ot_u8 *Virtual_Com_Port_GetConfigDescriptor(ot_u16 Length) {
    return Standard_GetDescriptorData(Length, &Config_Descriptor);
}


ot_u8 *Virtual_Com_Port_GetStringDescriptor(ot_u16 Length) {
    ot_u8 wValue0 = pInformation->USBwValue0;
    return (wValue0 > 4) ? NULL : Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
}


RESULT Virtual_Com_Port_Get_Interface_Setting(ot_u8 Interface, ot_u8 AlternateSetting) {
    return ((AlternateSetting > 0) || (Interface > 1)) ? USB_UNSUPPORT : USB_SUCCESS;
}


ot_u8 *Virtual_Com_Port_GetLineCoding(ot_u16 Length) {
    if (Length == 0) {
        pInformation->Ctrl_Info.Usb_wLength = sizeof(linecoding);
        return NULL;
    }
    return(ot_u8 *)&linecoding;
}


ot_u8 *Virtual_Com_Port_SetLineCoding(ot_u16 Length) {
    if (Length == 0) {
        pInformation->Ctrl_Info.Usb_wLength = sizeof(linecoding);
        return NULL;
    }
   return(ot_u8 *)&linecoding;
}







/** USB ISTR events   <BR>
  * ========================================================================
  */
#ifndef STM32F10X_CL

void USB_Istr(void) {
    wIstr = _GetISTR();
#   if (IMR_MSK & ISTR_SOF)
        if (wIstr & ISTR_SOF & wInterrupt_Mask) {
            _SetISTR((ot_u16)CLR_SOF);
            bIntPackSOF++;
            SOF_Callback();
        }
#   endif
#   if (IMR_MSK & ISTR_CTR)
        if (wIstr & ISTR_CTR & wInterrupt_Mask) {
            // servicing of the endpoint correct transfer interrupt
            // clear of the CTR flag into the sub 
            CTR_LP();
            CTR_Callback();
        }
#   endif
#   if (IMR_MSK & ISTR_RESET)
        if (wIstr & ISTR_RESET & wInterrupt_Mask) {
            _SetISTR((ot_u16)CLR_RESET);
            Device_Property.Reset();
            RESET_Callback();
        }
#   endif
#   if (IMR_MSK & ISTR_DOVR)
        if (wIstr & ISTR_DOVR & wInterrupt_Mask) {
            _SetISTR((ot_u16)CLR_DOVR);
            DOVR_Callback();
        }
#   endif
#   if (IMR_MSK & ISTR_ERR)
        if (wIstr & ISTR_ERR & wInterrupt_Mask) {
            _SetISTR((ot_u16)CLR_ERR);
            ERR_Callback();
        }
#   endif
#   if (IMR_MSK & ISTR_WKUP)
        if (wIstr & ISTR_WKUP & wInterrupt_Mask) {
            _SetISTR((ot_u16)CLR_WKUP);
            Resume(RESUME_EXTERNAL);
            WKUP_Callback();
        }
#   endif
#   if (IMR_MSK & ISTR_SUSP)
        if (wIstr & ISTR_SUSP & wInterrupt_Mask) {
            // clear of the ISTR bit must be done after setting of CNTR_FSUSP
            if (fSuspendEnabled)    Suspend();
            else                    Resume(RESUME_LATER);

            _SetISTR((ot_u16)CLR_SUSP);
            SUSP_Callback();
        }
#   endif
#   if (IMR_MSK & ISTR_ESOF)
        if (wIstr & ISTR_ESOF & wInterrupt_Mask) {
            _SetISTR((ot_u16)CLR_ESOF);   // resume handling timing is made with ESOFs 
            Resume(RESUME_ESOF);            // request without change of the machine state 
            ESOF_Callback();
        }
#   endif
}

#else 


u32 STM32_PCD_OTG_ISR_Handler (void) {
    USB_OTG_GINTSTS_TypeDef gintr_status;
    u32 retval = 0;

    if (USBD_FS_IsDeviceMode()) { // ensure that we are in device mode
        gintr_status.d32 = OTGD_FS_ReadCoreItr();
        
        // If there is no interrupt pending exit the interrupt routine 
        if (!gintr_status.d32) {
            return 0;
        }

#       ifdef INTR_ERLYSUSPEND
            // Early Suspend interrupt
            if (gintr_status.b.erlysuspend) {
                retval |= OTGD_FS_Handle_EarlySuspend_ISR();
            }
#       endif
#       ifdef INTR_EOPFRAME    
            // End of Periodic Frame interrupt
            if (gintr_status.b.eopframe) {
                retval |= OTGD_FS_Handle_EOPF_ISR();
            }
#       endif 
#       ifdef INTR_NPTXFEMPTY    
            // Non Periodic Tx FIFO Empty interrupt
            if (gintr_status.b.nptxfempty) {
                retval |= OTGD_FS_Handle_NPTxFE_ISR();
            }
#       endif 
#       ifdef INTR_WKUPINTR    
            // Wakeup or RemoteWakeup interrupt
            if (gintr_status.b.wkupintr) {   
                retval |= OTGD_FS_Handle_Wakeup_ISR();
            }
#       endif 
#       ifdef INTR_USBSUSPEND
            // Suspend interrupt 
            if (gintr_status.b.usbsuspend) { 
                // check if SUSPEND is possible 
                if (fSuspendEnabled) {
                    Suspend();
                }
                else {
                    // If not possible then resume after xx ms 
                    // This case shouldn't happen in OTG Device mode because 
                    // there's no ESOF interrupt to increment the ResumeS.bESOFcnt 
                    // in the Resume state machine
                    Resume(RESUME_LATER); 
                }
                retval |= OTGD_FS_Handle_USBSuspend_ISR();
            }
#       endif  
#       ifdef INTR_SOFINTR
            // Start of Frame interrupt
            if (gintr_status.b.sofintr) {
                // Update the frame number variable
                bIntPackSOF++;
                retval |= OTGD_FS_Handle_Sof_ISR();
            }
#       endif 
#       ifdef INTR_RXSTSQLVL
            // Receive FIFO Queue Status Level interrupt 
            if (gintr_status.b.rxstsqlvl) {
                retval |= OTGD_FS_Handle_RxStatusQueueLevel_ISR();
            }
#       endif 
#       ifdef INTR_ENUMDONE
            // Enumeration Done interrupt
            if (gintr_status.b.enumdone) {
                retval |= OTGD_FS_Handle_EnumDone_ISR();
            }
#       endif 
#       ifdef INTR_USBRESET
            // Reset interrupt
            if (gintr_status.b.usbreset) {
                retval |= OTGD_FS_Handle_UsbReset_ISR();
            }    
#       endif 
#       ifdef INTR_INEPINTR
            // IN Endpoint interrupt
            if (gintr_status.b.inepint) {
                retval |= OTGD_FS_Handle_InEP_ISR();
            }
#       endif 
#       ifdef INTR_OUTEPINTR
            // OUT Endpoint interrupt
            if (gintr_status.b.outepintr) {
                retval |= OTGD_FS_Handle_OutEP_ISR();
            }
#       endif  
#       ifdef INTR_MODEMISMATCH
            // Mode Mismatch interrupt
            if (gintr_status.b.modemismatch) {
                retval |= OTGD_FS_Handle_ModeMismatch_ISR();
            }
#       endif 
#       ifdef INTR_GINNAKEFF
            // Global IN Endpoints NAK Effective interrupt
            if (gintr_status.b.ginnakeff) {
                retval |= OTGD_FS_Handle_GInNakEff_ISR();
            }
#       endif 
#       ifdef INTR_GOUTNAKEFF
            // Global OUT Endpoints NAK effective interrupt
            if (gintr_status.b.goutnakeff) {
                retval |= OTGD_FS_Handle_GOutNakEff_ISR();
            }
#       endif 
#       ifdef INTR_ISOOUTDROP
            // Isochronous Out packet Dropped interrupt
            if (gintr_status.b.isooutdrop) {
                retval |= OTGD_FS_Handle_IsoOutDrop_ISR();
            }
#       endif 
#       ifdef INTR_EPMISMATCH
            // Endpoint Mismatch error interrupt
            if (gintr_status.b.epmismatch) {
                retval |= OTGD_FS_Handle_EPMismatch_ISR();
            }
#       endif
#       ifdef INTR_INCOMPLISOIN
            // Incomplete Isochronous IN transfer error interrupt
            if (gintr_status.b.incomplisoin) {
                retval |= OTGD_FS_Handle_IncomplIsoIn_ISR();
            }
#       endif
#       ifdef INTR_INCOMPLISOOUT
            // Incomplete Isochronous OUT transfer error interrupt
            if (gintr_status.b.outepintr) {
                retval |= OTGD_FS_Handle_IncomplIsoOut_ISR();
            }
#       endif
    }
    return retval;
}

#endif



/// EP1_IN is for USB TX (Host Input = Device TX)
void EP1_IN_Callback (void) { 
    mpipe_isr();
}


/// EP3_OUT is for USB RX (Host Output = Device RX)
void EP3_OUT_Callback(void) {
/// Copy data from the USB HW buffer into the SW pipe, and also
/// Advance the position of the pipe cursor for the next call.

    ///@todo Find how to get the USB HW buffer size before loading the
    ///      data, in order to avoid pipe overflow.
    mpipe_ext.i += USB_SIL_Read(EP3_OUT, &mpipe.pktbuf[mpipe_ext.i]);

#   ifndef STM32F10X_CL
    // Enable the receive of data on EP3
        SetEPRxValid(ENDP3);
#   endif
    
    mpipe_isr();
}


/*
#ifdef STM32F10X_CL
void INTR_SOFINTR_Callback(void) {
#else
void SOF_Callback(void) {
#endif
    static ot_u32 FrameCount = 0;
  
    if(bDeviceState == CONFIGURED) {
        if (FrameCount++ == VCOMPORT_IN_FRAME_INTERVAL) {
            // Reset the frame counter,
            // Check the data to be sent through IN pipe
            FrameCount = 0;
            if (mpipe.state == MPIPE_Tx_Wait) {
                sub_usb_loadtx();
            }
        }
    }
}



void Handle_USBAsynchXfer (void) {  
}
*/






/** Mpipe Main Subroutines   <BR>
  * ========================================================================
  */
void sub_usb_loadtx() {
    ot_u16 transfer_size;
    ot_u16 transfer_start;
        
    transfer_size   = (mpipe.pktlen > VIRTUAL_COM_PORT_DATA_SIZE) ? \
                            VIRTUAL_COM_PORT_DATA_SIZE : mpipe.pktlen;
    transfer_start  = mpipe_ext.i;
    mpipe_ext.i    += transfer_size;
    mpipe.pktlen   -= transfer_size;

#   ifdef USE_STM3210C_EVAL
        USB_SIL_Write(EP1_IN, &mpipe.pktbuf[transfer_start], transfer_size);  
#   else
        UserToPMABufferCopy(&mpipe.pktbuf[transfer_start], ENDP1_TXADDR, transfer_size);
        SetEPTxCount(ENDP1, transfer_size);
        SetEPTxValid(ENDP1); 
#   endif  
}


void sub_usb_portsetup() {

    ///Enable USB_DISCONNECT GPIO clock, Configure USB pull-up pin
#   if !defined(STM32F10X_CL)
    {   
        GPIO_InitTypeDef GPIO_in;
        GPIO_in.GPIO_Pin    = MPIPE_USBDC_PIN;
        GPIO_in.GPIO_Speed  = GPIO_Speed_50MHz;
        GPIO_in.GPIO_Mode   = GPIO_Mode_Out_OD;
        GPIO_Init(MPIPE_USBDC_PORT, &GPIO_in);
    
        //GPIO_in.GPIO_Pin    = MPIPE_USBDM_PIN | MPIPE_USBDP_PIN;
        //GPIO_in.GPIO_Mode   = GPIO_Mode_AF_PP;
        //GPIO_Init(MPIPE_USBDM_PORT, &GPIO_in);
    }
#   endif

    /// Enable USB Clocks
    MPIPE_USBCLK(ENABLE);

    /// Set USB interrupt (Priority 0, Sub-priority 3)
    NVIC->IP[MPIPE_USB_IRQn]        = b0011 << (8 - __NVIC_PRIO_BITS);
    NVIC->ISER[MPIPE_USB_IRQn>>5]   = (1 << (MPIPE_USB_IRQn & 0x1F));

    ///Initialize USB (using ST library function)
    USB_Init();
    //Suspend();
}






/** Mpipe Callback Configurators   <BR>
  * ========================================================================
  */

#if (OT_FEATURE(MPIPE_CALLBACKS) == ENABLED)
void mpipe_setsig_txdone(ot_sigv signal) {
    mpipe.sig_txdone = signal;
}

void mpipe_setsig_rxdone(ot_sigv signal) {
    mpipe.sig_rxdone = signal;
}

void mpipe_setsig_rxdetect(ot_sigv signal) {
    mpipe.sig_rxdetect = signal;
}
#endif




/** Mpipe Main Public Functions  <BR>
  * ========================================================================
  */

#ifndef EXT_mpipe_footerbytes
ot_u8 mpipe_footerbytes() {
    return MPIPE_FOOTERBYTES;
}
#endif


#ifndef EXT_mpipe_init
ot_int mpipe_init(void* port_id) {
/// 0. "port_id" is unused in this impl, and it may be NULL
/// 1. Set all signal callbacks to NULL, and initialize other variables.
/// 2. Prepare the HW, which in this case is a USB Virtual TTY
/// 3. Set default speed, which in this case is 115200 bps

#if (OT_FEATURE(MPIPE_CALLBACKS) == ENABLED)
    mpipe.sig_rxdone    = &otutils_sigv_null;
    mpipe.sig_txdone    = &otutils_sigv_null;
    mpipe.sig_rxdetect  = &otutils_sigv_null;
#endif

    mpipe.sequence.ushort   = 0;          //not actually necessary
    mpipe.state             = MPIPE_Idle;
    mpipe_alp.inq           = &otmpin;
    mpipe_alp.outq          = &otmpout;
    
    mpipe_setspeed(MPIPE_115200bps);     //default baud rate
    sub_usb_portsetup();
    
    return 0;
}
#endif



#ifndef EXTF_mpipe_kill
void mpipe_kill() {
/// Figure out what to put in here
}
#endif



#ifndef EXT_mpipe_wait
void mpipe_wait() {
    while (mpipe.state != MPIPE_Idle) {
        SLEEP_MCU();
    }
}
#endif



#ifndef EXT_mpipe_setspeed
void mpipe_setspeed(mpipe_speed speed) {
    linecoding.bitrate = 115200; ///@todo make dynamic (fixed for now)
}
#endif



#ifndef EXT_mpipe_status
mpipe_state mpipe_status() {
    return mpipe.state;
}
#endif



#ifndef EXT_mpipe_txndef
void mpipe_txndef(ot_u8* data, ot_bool blocking, mpipe_priority data_priority) {
    if (mpipe.state == MPIPE_Idle) {
        ot_uni16 crcval;
        mpipe.pktbuf    = data;
        mpipe.pktlen    = data[2] + 6;
        mpipe.state     = MPIPE_Tx_Wait;
        
        // add sequence id & crc to end of the datastream
        data[mpipe.pktlen++] = mpipe.sequence.ubyte[UPPER];
        data[mpipe.pktlen++] = mpipe.sequence.ubyte[LOWER];
        crcval.ushort        = platform_crc_block(data, mpipe.pktlen);
        data[mpipe.pktlen++] = crcval.ubyte[UPPER];
        data[mpipe.pktlen++] = crcval.ubyte[LOWER];
        
        //USB adaptation
        mpipe_ext.i = 0;
        //Resume(RESUME_INTERNAL);
        sub_usb_loadtx();
        
        //End of USB adaptation
        
        if (blocking == True) {
            mpipe_wait();   
        }
    }
}
#endif



#ifndef EXT_mpipe_rxndef
void mpipe_rxndef(ot_u8* data, ot_bool blocking, mpipe_priority data_priority) {
    if (blocking) {
        mpipe_wait();
    }
    if (mpipe.state == MPIPE_Idle) {
        mpipe.pktbuf    = data;
        mpipe.pktlen    = 6;
        mpipe.state     = MPIPE_Idle;
    
        //USB Adaptation
        mpipe_ext.i = 0;
    
        //Resume(RESUME_INTERNAL);
        //End of USB Adaptation
    }
}
#endif



#ifndef EXT_mpipe_isr
void mpipe_isr() {
    switch (mpipe.state) {
        case MPIPE_Idle: //note, case doesn't break!
#           if ((OT_FEATURE(MPIPE_CALLBACKS) == ENABLED) && !defined(EXTF_mpipe_sig_rxdetect))
                mpipe.sig_rxdetect(0);  
#           elif defined(EXTF_mpipe_sig_rxdetect)
                mpipe_sig_rxdetect(0);
#           endif
        
        case MPIPE_RxHeader: {
            mpipe.state     = MPIPE_RxPayload;
            mpipe.pktlen   += mpipe.pktbuf[2] + MPIPE_FOOTERBYTES;
            return;
        }
        
        case MPIPE_RxPayload: 
            if (mpipe_ext.i >= mpipe.pktlen) {
                /// The reception is completely done.  Go to Idle and do callback.
                //Suspend();
                mpipe.state     = MPIPE_Idle;
#               if ((OT_FEATURE(MPIPE_CALLBACKS) == ENABLED) && !defined(EXTF_mpipe_sig_rxdone))
                    mpipe.sig_rxdone(0);
#               elif defined(EXTF_mpipe_sig_rxdone)
                    mpipe_sig_rxdone(0);
#               endif
            }
            break;
        
        case MPIPE_Tx_Wait:
            sub_usb_loadtx();
            mpipe.state += (mpipe_ext.i >= mpipe.pktlen);
            break;
        
        case MPIPE_Tx_Done: 
            /// Called when the transmission is completely done.  Increment the sequence ID
            /// and change state back to Idle.  Do the callback if necessary
            //Suspend();
            mpipe.state     = MPIPE_Idle;
            mpipe.sequence.ushort++;
#           if ((OT_FEATURE(MPIPE_CALLBACKS) == ENABLED) && !defined(EXTF_mpipe_sig_txdone))
                mpipe.sig_txdone(0);
#           elif defined(EXTF_mpipe_sig_txdone)
                mpipe_sig_txdone(0);
#           endif
            break;
    }
}
#endif



#endif

