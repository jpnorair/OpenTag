/* Copyright 2010-2013 JP Norair
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
  * @file       /otplatform/stm32l1xx/mpipe_usbcdc_driver.c
  * @author     JP Norair
  * @version    R101
  * @date       14 Nov 2013
  * @brief      Message Pipe (MPIPE) USB CDC ACM implementation for STM32L
  * @defgroup   MPipe (Message Pipe)
  * @ingroup    MPipe
  *
  * CDC ACM stands for "Communication Device Class Abstract Control Model."
  * Windows people call it a virtual COM port.
  *
  * This implementation utilizes a modestly upgraded, but functionally
  * identical, version of STMicro's standard USB library for STM32.  The code
  * for the USB library is stored in:
  * /otplatform/stm32_mculib/STM32_USB-FS-Device_Driver
  *
  * CDC pretents to be a TTY UART device.  All of the features of a serial DB9
  * connector (and more) are available as control commands via the ACM profile,
  * but this ACM driver doesn't use any of the control signals.  The STM32L has
  * an auto-wakeup on USB feature, so none of those are necessary.
  *
  * The UART simulation via ACM supports the following configurations:
  * Baudrates (kbps):   9.6, 28.8, 57.6, 115.2, 125, 250, 500, 1000
  * Byte structure:     8N1
  * Duplex:             Half
  * Flow control:       None
  *
  * MPipe Protocol for USB-CDC-ACM:
  * <PRE>
  * +-------+-----------+-------+-----------+----------+---------+---------+
  * | Field | Sync Word | CRC16 | P. Length | Sequence | Control | Payload |
  * | Bytes |     2     |   2   |     2     |     1    |    1    |    N    |
  * | Value |   FF55    |       |     N     |  0-255   |   RFU   |   ALP   |
  * +-------+-----------+-------+-----------+----------+---------+---------+
  * </PRE>
  *
  * The protocol includes an ACK/NACK feature, although this is only of any
  * importance if you have a lossy link between client and server.  If you are
  * using a USB->UART converter, USB has a reliable MAC implementation that
  * eliminates the need for MPipe ACKing.
  *
  * Anyway, after receiving a message, the Mpipe send an ACK/NACK.  The "YY"
  * byte is 0 for ACK and non-zero for ACK.  Presently, 0x7F is used as the YY
  * NACK value.
  * <PRE>
  * [ Seq ID ] 0xDD 0x00 0x00 0x02 0x00 0xYY  [ CRC16 ]
  * </PRE>
  *
  ******************************************************************************
  */


#include <otstd.h>
#include <otplatform.h>

#ifndef BOARD_PARAM_MPIPE_IFS
#   define BOARD_PARAM_MPIPE_IFS 1
#endif

#if (defined(__STM32L__) && OT_FEATURE(MPIPE) && (BOARD_PARAM_MPIPE_IFS == 1) && defined(MPIPE_USB))

#include <otlib/buffers.h>
#include <otsys/mpipe.h>
#include <otlib/utils.h>

#include "usb_conf[acm].h"
#include "usb_lib.h"    //part of USB library: /lib/STM32_USB_FS/inc/usb_lib.h




/** Packet structure constants for MPipe over USB-CDC-ACM  <BR>
  * ========================================================================<BR>
  */
#define MPIPE_HEADERBYTES       8
#define MPIPE_FOOTERBYTES       0
#define MPIPE_OVERHEADBYTES     (MPIPE_HEADERBYTES + MPIPE_FOOTERBYTES)




//new
/** MPIPE Interrupt Configuration  <BR>
  * ========================================================================<BR>
  * NVIC parameters are negotiated or defaulted, considering the preset config
  * in the platform section.
  */
#ifndef BUILD_NVIC_SUBGROUP_MPIPE
#   define BUILD_NVIC_SUBGROUP_MPIPE 0
#endif

#define _SUBGROUP   (BUILD_NVIC_SUBGROUP_MPIPE & (16 - (16/__CM3_NVIC_GROUPS)))
#define _IRQGROUP   ((PLATFORM_NVIC_IO_GROUP + _SUBGROUP) << 4)

#if ((BOARD_FEATURE_MPIPE_CS == ENABLED) || (BOARD_FEATURE_MPIPE_FLOWCTL == ENABLED))
#   define _CTS_IRQ
#endif
#if (BOARD_FEATURE_MPIPE_BREAK == ENABLED)
#   define _BREAK_IRQ
#endif

/** Platform Clock Configuration   <BR>
  * ========================================================================<BR>
  * MPIPE typically requires the system clock to be set to a certain speed in
  * order for the baud rate generation to work.  So, platforms that are using
  * multispeed clocking will need some extra logic in the MPIPE driver to
  * assure that the clock speed is on the right setting during MPIPE usage.
  */

#define __USB_CLKON()   (RCC->APB1ENR |= RCC_APB1ENR_USBEN)
#define __USB_CLKOFF()  (RCC->APB1ENR &= ~RCC_APB1ENR_USBEN)



/** MPipe Macros
  * ========================================================================<BR>
  */

/// @todo lookup actual data rate in use
#define __MPIPE_TIMEOUT(BYTES)  (10 + (BYTES >> 3))



/** Mpipe Driver Data  <BR>
  * ========================================================================<BR>
  */

typedef struct {
    ot_u8       syncFF;
    ot_u8       sync55;
    ot_u16      crc16;
    ot_u16      plen;
    ot_u8       seq;
    ot_u8       ctl;
} mpipe_header;

typedef struct {
    ot_int          packets;
    ot_u8*          pkt;
    ot_u8*          pktend;
    mpipe_header    header;
} cdcacm_struct;


cdcacm_struct cdcacm;


///@note the MANT+FRAC baud rate selection can be achieved by the div16 term
static const ot_u32 br_hssel[] = {
    9600, 28800, 57600, 115200, 125000, 250000, 500000, 1000000
};


//mpipe_struct        mpipe;
//mpipe_ext_struct    mpipe_ext;
//alp_tmpl            mpipe_alp;




/** Mpipe Main Subroutine Prototypes   <BR>
  * ========================================================================
  */
void sub_usb_loadheader();
void sub_usb_loadtx(ot_int pad_bytes);
void sub_usb_portsetup();






/** Mpipe Virtual COM Constants for STM32 implementation & driver   <BR>
  * ========================================================================<BR>
  */
//#   define CTR_CALLBACK
//#   define DOVR_CALLBACK
//#   define ERR_CALLBACK
//#   define WKUP_CALLBACK
//#   define SUSP_CALLBACK
//#   define RESET_CALLBACK
//#   define SOF_CALLBACK
//#   define ESOF_CALLBACK

#define VCOMPORT_IN_FRAME_INTERVAL  5

#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05
#define CDCACM_DATA_SIZE              64
#define CDCACM_INT_SIZE               8
#define CDCACM_SIZ_DEVICE_DESC        18
#define CDCACM_SIZ_CONFIG_DESC        67
#define CDCACM_SIZ_STRING_LANGID      4
#define CDCACM_SIZ_STRING_VENDOR      (20+2)
#define CDCACM_SIZ_STRING_PRODUCT     (32+2)
#define CDCACM_SIZ_STRING_SERIAL      (24+2)
#define STANDARD_ENDPOINT_DESC_SIZE             0x09

#define cdcacm_GetConfiguration          NOP_Process
//#define cdcacm_SetConfiguration          NOP_Process
#define cdcacm_GetInterface              NOP_Process
#define cdcacm_SetInterface              NOP_Process
#define cdcacm_GetStatus                 NOP_Process
#define cdcacm_ClearFeature              NOP_Process
#define cdcacm_SetEndPointFeature        NOP_Process
#define cdcacm_SetDeviceFeature          NOP_Process
//#define cdcacm_SetDeviceAddress          NOP_Process

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
ot_u32 STM32_PCD_OTG_ISR_Handler (void);

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
    void SOF_Callback(void);
#else
#   define SOF_Callback()   while(0)
#endif
#ifdef ESOF_CALLBACK
    void ESOF_Callback(void);
#else
#   define ESOF_Callback()   while(0)
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

void cdcacm_init(void);
void cdcacm_Reset(void);
void cdcacm_SetConfiguration(void);
void cdcacm_SetDeviceAddress(void);
void cdcacm_Status_In(void);
void cdcacm_Status_Out(void);
RESULT cdcacm_Data_Setup(ot_u8 RequestNo);
RESULT cdcacm_NoData_Setup(ot_u8 RequestNo);
ot_u8 *cdcacm_GetDeviceDescriptor(ot_u16 Length);
ot_u8 *cdcacm_GetConfigDescriptor(ot_u16 Length);
ot_u8 *cdcacm_GetStringDescriptor(ot_u16 Length);
RESULT cdcacm_Get_Interface_Setting(ot_u8 Interface, ot_u8 AlternateSetting);
ot_u8 *cdcacm_GetLineCoding(ot_u16 Length);
ot_u8 *cdcacm_SetLineCoding(ot_u16 Length);

void Handle_USBAsynchXfer (void);






/** Mpipe Virtual COM Data Elements   <BR>
  * ========================================================================<BR>
  */
//ot_u8 USB_Rx_Buffer[CDCACM_DATA_SIZE];
//ot_u8 USB_Tx_State;
//extern  ot_u8 USART_Rx_Buffer[];
//extern ot_u32 USART_Rx_ptr_out;
//extern ot_u32 USART_Rx_length;



volatile ot_u32 bDeviceState    = UNCONNECTED;  // USB device status
volatile ot_bool remotewakeupon = False;
volatile ot_bool fSuspendEnabled= False;         // true when suspend is possible


//ot_u32 wCNTR                    = 0;
volatile ot_u16 wIstr           = 0;                // ISTR register last read value
ot_int esof_counter             = 0;
volatile ot_u8  bIntPackSOF     = 0;            // SOFs received between 2 consecutive packets

ot_u8 Request = 0;



// USB Standard Device Descriptor: Set up for Little Endian
const ot_u8 cdcacm_DeviceDescriptor[] = {
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

const ot_u8 cdcacm_ConfigDescriptor[] = {
    /*Configuation Descriptor*/
    0x09,   /* bLength: Configuation Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,      /* bDescriptorType: Configuration */
    CDCACM_SIZ_CONFIG_DESC,       /* wTotalLength:no of returned bytes */
    0x00,
    0x02,   /* bNumInterfaces: 2 interface */
    0x01,   /* bConfigurationValue: Configuration value */
    0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
    0xC0,   /* bmAttributes: self powered or bus powered */
    (ot_u8)(150>>1),   /* MaxPower 150 mA */
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
    CDCACM_INT_SIZE,      /* wMaxPacketSize: */
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
    CDCACM_DATA_SIZE,             /* wMaxPacketSize: */
    0x00,
    0x00,   /* bInterval: ignore for Bulk transfer */
    /*Endpoint 1 Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
    0x81,   /* bEndpointAddress: (IN1) */
    0x02,   /* bmAttributes: Bulk */
    CDCACM_DATA_SIZE,             /* wMaxPacketSize: */
    0x00,
    0x00    /* bInterval */
};

/* USB String Descriptors */
const ot_u8 cdcacm_StringLangID[CDCACM_SIZ_STRING_LANGID] = {
    CDCACM_SIZ_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09,
    0x04 /* LangID = 0x0409: U.S. English */
};

const ot_u8 cdcacm_StringVendor[CDCACM_SIZ_STRING_VENDOR] = {
    CDCACM_SIZ_STRING_VENDOR,     /* Size of Vendor string */
    USB_STRING_DESCRIPTOR_TYPE,             /* bDescriptorType*/
    /* Vendor: "STMicro+OT" */
    'S', 0, 'T', 0, 'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, '+', 0,
    'O', 0, 'T', 0
};

const ot_u8 cdcacm_StringProduct[CDCACM_SIZ_STRING_PRODUCT] = {
    CDCACM_SIZ_STRING_PRODUCT,          /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    /* Product name: "OpenTag MPipeUSB" */
    'O', 0, 'p', 0, 'e', 0, 'n', 0, 'T', 0, 'a', 0, 'g', 0, ' ', 0,
    'M', 0, 'P', 0, 'i', 0, 'p', 0, 'e', 0, 'U', 0, 'S', 0, 'B', 0
};

ot_u8 cdcacm_StringSerial[CDCACM_SIZ_STRING_SERIAL] = {
    CDCACM_SIZ_STRING_SERIAL,           /* bLength */
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
    cdcacm_init,
    cdcacm_Reset,
    NOP_Process,
    NOP_Process,
    cdcacm_Data_Setup,
    cdcacm_NoData_Setup,
    cdcacm_Get_Interface_Setting,
    cdcacm_GetDeviceDescriptor,
    cdcacm_GetConfigDescriptor,
    cdcacm_GetStringDescriptor,
    0,
    0x40 //MAX PACKET SIZE
};

USER_STANDARD_REQUESTS User_Standard_Requests = {
    cdcacm_GetConfiguration,
    cdcacm_SetConfiguration,
    cdcacm_GetInterface,
    cdcacm_SetInterface,
    cdcacm_GetStatus,
    cdcacm_ClearFeature,
    cdcacm_SetEndPointFeature,
    cdcacm_SetDeviceFeature,
    cdcacm_SetDeviceAddress
};

ONE_DESCRIPTOR Device_Descriptor = {
    (ot_u8*)cdcacm_DeviceDescriptor,
    CDCACM_SIZ_DEVICE_DESC
};

ONE_DESCRIPTOR Config_Descriptor = {
    (ot_u8*)cdcacm_ConfigDescriptor,
    CDCACM_SIZ_CONFIG_DESC
};

ONE_DESCRIPTOR String_Descriptor[4] = {
    {(ot_u8*)cdcacm_StringLangID, CDCACM_SIZ_STRING_LANGID},
    {(ot_u8*)cdcacm_StringVendor, CDCACM_SIZ_STRING_VENDOR},
    {(ot_u8*)cdcacm_StringProduct, CDCACM_SIZ_STRING_PRODUCT},
    {(ot_u8*)cdcacm_StringSerial, CDCACM_SIZ_STRING_SERIAL}
};









/** MPIPE Embedded ISRs  <BR>
  * ========================================================================<BR>
  * In the platform_config_~.h file, you should set the MPIPE to have embedded
  * ISRs if you want them to be included in this file.  In this case, they are
  * modularized from the rest of your code (which can be nice), but you can't
  * use the ISRs for other things.
  */



/** USB Powering Routines  <BR>
  * ========================================================================
  */
#define USB_Cable_Config(VAL)    USB_Cable_Config_##VAL
#define USB_Cable_Config_ENABLE  (SYSCFG->PMC |= (ot_u32)SYSCFG_PMC_USB_PU)
#define USB_Cable_Config_DISABLE (SYSCFG->PMC &= ~(ot_u32)SYSCFG_PMC_USB_PU)

#define Enter_LowPowerMode()    (bDeviceState = SUSPENDED)

void Leave_LowPowerMode() {
    bDeviceState = ATTACHED;
    if (Device_Info.Current_Configuration != 0) {
        bDeviceState    = CONFIGURED;
        mpipe.state     = MPIPE_Idle;   ///@todo see if this should be here
    }
}



RESULT PowerOn(void) {
    ot_u16 wRegVal;
    //USB_Cable_Config(ENABLE);   // cable plugged-in?
    //SYSCFG->PMC |= (ot_u32)SYSCFG_PMC_USB_PU;
    BOARD_USB_PORTENABLE();

    wRegVal = CNTR_FRES;        // CNTR_PWDN = 0
    _SetCNTR(wRegVal);
    wInterrupt_Mask = 0;        // CNTR_FRES = 0
    _SetCNTR(wInterrupt_Mask);
    _SetISTR(0);                // Clear pending interrupts
    wInterrupt_Mask = CNTR_RESETM | CNTR_SUSPM | CNTR_WKUPM;
    _SetCNTR(wInterrupt_Mask);
    return USB_SUCCESS;
}


RESULT PowerOff() {
    _SetCNTR(CNTR_FRES);                // disable all interrupts and force USB reset
    _SetISTR(0);                        // clear interrupt status register
    //USB_Cable_Config(DISABLE);          // Disable the Pull-Up
    //SYSCFG->PMC &= ~(ot_u32)SYSCFG_PMC_USB_PU;
    BOARD_USB_PORTDISABLE();

    _SetCNTR(CNTR_FRES + CNTR_PDWN);    // switch-off device
    return USB_SUCCESS;
}


void Suspend(void) {
    volatile ot_u16 EP[8];
	ot_u32 i =0;
    ot_u32 tmpreg = 0;
    volatile uint32_t savePWR_CR=0;
	ot_u16 wCNTR;

	//suspend preparation
	/* ... */

	wCNTR = _GetCNTR();

    // This a sequence to apply a force RESET to handle a robustness case
    //Store endpoints registers status
    for (i=0;i<8;i++) EP[i] = _GetENDPOINT(i);

	wCNTR  |=CNTR_RESETM;       _SetCNTR(wCNTR);
	wCNTR  |=CNTR_FRES;         _SetCNTR(wCNTR);
	wCNTR  &=~CNTR_FRES;        _SetCNTR(wCNTR);

	//poll for RESET flag in ISTR
	while((_GetISTR()&ISTR_RESET) == 0);

	// clear RESET flag in ISTR
	_SetISTR((uint16_t)CLR_RESET);

	//restore Enpoints
    // Then it is safe to enter macrocell in suspend mode
	for (i=0;i<8;i++) _SetENDPOINT(i, EP[i]);
	wCNTR  |= CNTR_FSUSP;       _SetCNTR(wCNTR);

	// force low-power mode in the macrocell
	wCNTR = _GetCNTR();
	wCNTR |= CNTR_LPMODE;
	_SetCNTR(wCNTR);

	// prepare entry in low power mode (STOP mode)
	// Select the regulator state in STOP mode
    mpipedrv_detach(NULL);
}


void Resume_Init(void) {
    ot_u16 wCNTR;
    wCNTR   = _GetCNTR();       // CNTR_LPMODE = 0
    wCNTR  &= (~CNTR_LPMODE);
    _SetCNTR(wCNTR);
    Leave_LowPowerMode();
    _SetCNTR(IMR_MSK);          // reset FSUSP bit
}


void Resume(RESUME_STATE eResumeSetVal) {
    ot_u16 wCNTR;
    if (eResumeSetVal != RESUME_ESOF)
        ResumeS.eState = eResumeSetVal;

    switch (ResumeS.eState) {
        case RESUME_EXTERNAL:   if (remotewakeupon == 0) {
                                    Resume_Init();
                                    ResumeS.eState = RESUME_OFF;
                                }
                                else {
                                    ResumeS.eState = RESUME_ON;
                                }
                                break;

        case RESUME_INTERNAL:   Resume_Init();
                                ResumeS.eState = RESUME_START;
                                remotewakeupon = 1;
                                break;

        case RESUME_LATER:      ResumeS.bESOFcnt = 2;
                                ResumeS.eState = RESUME_WAIT;
                                break;

        case RESUME_WAIT:       ResumeS.bESOFcnt--;
                                if (ResumeS.bESOFcnt == 0)
                                    ResumeS.eState = RESUME_START;
                                break;

        case RESUME_START:      wCNTR   = _GetCNTR();
                                wCNTR  |= CNTR_RESUME;
                                _SetCNTR(wCNTR);
                                ResumeS.eState = RESUME_ON;
                                ResumeS.bESOFcnt = 10;
                                break;

        case RESUME_ON:         ResumeS.bESOFcnt--;
                                if (ResumeS.bESOFcnt == 0) {
                                    wCNTR   = _GetCNTR();
                                    wCNTR  &= (~CNTR_RESUME);
                                    _SetCNTR(wCNTR);
                                    ResumeS.eState = RESUME_OFF;
                                    remotewakeupon = 0;
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
    Device_Serial0  = *(ot_u32*)(0x1FF80050);
    Device_Serial1  = *(ot_u32*)(0x1FF80054);
    Device_Serial2  = *(ot_u32*)(0x1FF80064);
    Device_Serial0 += Device_Serial2;

    if (Device_Serial0 != 0) {
        IntToUnicode(Device_Serial0, &cdcacm_StringSerial[2] , 8);
        IntToUnicode(Device_Serial1, &cdcacm_StringSerial[18], 4);
    }
}

void cdcacm_init(void) {
    /// Initialize VCOM CDC
    Get_SerialNum();            // Update the serial number string descriptor
    pInformation->Current_Configuration = 0;
    PowerOn();                  // Connect the device
    USB_SIL_Init();             // Perform basic device initialization operations
    bDeviceState = UNCONNECTED;
}


void cdcacm_Reset(void) {
    // - Set Virtual_Com_Port DEVICE as not configured
    // - Current Feature initialization
    // - Set Virtual_Com_Port DEVICE with the default Interface
    pInformation->Current_Configuration = 0;
    pInformation->Current_Feature       = cdcacm_ConfigDescriptor[7];
    pInformation->Current_Interface     = 0;

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
    SetEPRxCount(ENDP3, CDCACM_DATA_SIZE);
    SetEPRxStatus(ENDP3, EP_RX_VALID);
    SetEPTxStatus(ENDP3, EP_TX_DIS);

    // Set this device to response on default address
    SetDeviceAddress(0);

    bDeviceState = ATTACHED;
}


void cdcacm_SetConfiguration(void) {
    if (Device_Info.Current_Configuration != 0) {
        bDeviceState = CONFIGURED;  // Device configured
        //mpipe.state  = MPIPE_Idle;
    }
}


void cdcacm_SetDeviceAddress (void) {
    bDeviceState = ADDRESSED;
}


void cdcacm_Status_In(void) {
    if (Request == SET_LINE_CODING) Request = 0;
}

void cdcacm_Status_Out(void) {}


RESULT cdcacm_Data_Setup(ot_u8 RequestNo) {
    ot_u8 *(*CopyRoutine)(ot_u16);

    CopyRoutine = NULL;
    if (RequestNo == GET_LINE_CODING) {
        if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) {
            CopyRoutine = cdcacm_GetLineCoding;
        }
    }
    else if (RequestNo == SET_LINE_CODING) {
        if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) {
            CopyRoutine = cdcacm_SetLineCoding;
        }
        Request = SET_LINE_CODING;  ///@note was commented
    }
    if (CopyRoutine == NULL) {
        return USB_UNSUPPORT;
    }

    pInformation->Ctrl_Info.CopyData    = CopyRoutine;
    pInformation->Ctrl_Info.Usb_wOffset = 0;
    (*CopyRoutine)(0);
    return USB_SUCCESS;
}


RESULT cdcacm_NoData_Setup(ot_u8 RequestNo) {
  if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) {
    if (RequestNo == SET_COMM_FEATURE) {
      return USB_SUCCESS;
    }
    else if (RequestNo == SET_CONTROL_LINE_STATE) {
      return USB_SUCCESS;
    }
  }
  return USB_UNSUPPORT;
}


ot_u8 *cdcacm_GetDeviceDescriptor(ot_u16 Length) {
    return Standard_GetDescriptorData(Length, &Device_Descriptor);
}


ot_u8 *cdcacm_GetConfigDescriptor(ot_u16 Length) {
    return Standard_GetDescriptorData(Length, &Config_Descriptor);
}


ot_u8 *cdcacm_GetStringDescriptor(ot_u16 Length) {
    ot_u8 wValue0 = pInformation->USBwValue0;
    return (wValue0 > 4) ? NULL : Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
}


RESULT cdcacm_Get_Interface_Setting(ot_u8 Interface, ot_u8 AlternateSetting) {
    return ((AlternateSetting > 0) || (Interface > 1)) ? USB_UNSUPPORT : USB_SUCCESS;
}


ot_u8 *cdcacm_GetLineCoding(ot_u16 Length) {
    if (Length == 0) {
        pInformation->Ctrl_Info.Usb_wLength = sizeof(linecoding);
      //mpipe.state = MPIPE_Idle;
        return NULL;
    }
    return(ot_u8 *)&linecoding;
}


ot_u8 *cdcacm_SetLineCoding(ot_u16 Length) {
    if (Length == 0) {
        pInformation->Ctrl_Info.Usb_wLength = sizeof(linecoding);
        mpipe.state = MPIPE_Idle;
        return NULL;
    }
   return(ot_u8 *)&linecoding;
}







/** USB ISTR events   <BR>
  * ========================================================================<BR>
  */

void platform_isr_usblp(void) {
    ot_u16 wCNTR;
    ot_u32 i    = 0;
    wIstr       = _GetISTR();

#   if (IMR_MSK & ISTR_SOF)
        if (wIstr & ISTR_SOF & wInterrupt_Mask) {
            _SetISTR((ot_u16)CLR_SOF);
            bIntPackSOF++;
#           ifdef SOF_CALLBACK
            SOF_Callback();
#           endif
        }
#   endif
#   if (IMR_MSK & ISTR_CTR)
        if (wIstr & ISTR_CTR & wInterrupt_Mask) {
            // servicing of the endpoint correct transfer interrupt
            // clear of the CTR flag into the sub
            CTR_LP();
#           ifdef CTR_CALLBACK
            CTR_Callback();
#           endif
        }
#   endif
#   if (IMR_MSK & ISTR_RESET)
        if (wIstr & ISTR_RESET & wInterrupt_Mask) {
            _SetISTR((ot_u16)CLR_RESET);
            Device_Property.Reset();
#           ifdef RESET_CALLBACK
            RESET_Callback();
#           endif
        }
#   endif
#   if (IMR_MSK & ISTR_DOVR)
        if (wIstr & ISTR_DOVR & wInterrupt_Mask) {
            _SetISTR((ot_u16)CLR_DOVR);
#           ifdef DOVR_CALLBACK
            DOVR_Callback();
#           endif
        }
#   endif
#   if (IMR_MSK & ISTR_ERR)
        if (wIstr & ISTR_ERR & wInterrupt_Mask) {
            _SetISTR((ot_u16)CLR_ERR);
#           ifdef ERR_CALLBACK
            ERR_Callback();
#           endif
        }
#   endif
#   if (IMR_MSK & ISTR_WKUP)
        if (wIstr & ISTR_WKUP & wInterrupt_Mask) {
            _SetISTR((ot_u16)CLR_WKUP);
            Resume(RESUME_EXTERNAL);
#           ifdef WKUP_CALLBACK
            WKUP_Callback();
#           endif
        }
#   endif
#   if (IMR_MSK & ISTR_SUSP)
        if (wIstr & ISTR_SUSP & wInterrupt_Mask) {
            if (fSuspendEnabled)    Suspend();
            else                    Resume(RESUME_LATER);
            // clear of the ISTR bit must be done after setting of CNTR_FSUSP
            _SetISTR((ot_u16)CLR_SUSP);
#           ifdef SUSP_CALLBACK
            SUSP_Callback();
#           endif
        }
#   endif
#   if (IMR_MSK & ISTR_ESOF)
        if (wIstr & ISTR_ESOF & wInterrupt_Mask) {
            _SetISTR((ot_u16)CLR_ESOF);   // resume handling timing is made with ESOFs
            if ((_GetFNR()&FNR_RXDP) != 0) {
                esof_counter++;
                if ((esof_counter > 3) && ((_GetCNTR() & CNTR_FSUSP) == 0)) {
                    volatile ot_u16 EP[8];
                    esof_counter = 0;

                    //Sequence to apply forced-reset
                    wCNTR = _GetCNTR();
                    for (i=0;i<8;i++) EP[i] = _GetENDPOINT(i);

                    wCNTR  |=CNTR_FRES;     _SetCNTR(wCNTR);
                    wCNTR  &=~CNTR_FRES;    _SetCNTR(wCNTR);

                    while((_GetISTR() & ISTR_RESET) == 0);
                    _SetISTR((ot_u16)CLR_RESET);

                    for (i=0;i<8;i++) _SetENDPOINT(i, EP[i]);
                }
            }
            else {
                esof_counter = 0;
            }

            Resume(RESUME_ESOF);
#           ifdef ESOF_CALLBACK
            ESOF_Callback();
#           endif
        }
#   endif
}



/// EP1_IN is for USB TX (Host Input = Device TX)
void EP1_IN_Callback (void) {
    mpipedrv_isr();
}


/// EP3_OUT is for USB RX (Host Output = Device RX)
void EP3_OUT_Callback(void) {
/// Copy data from the USB HW buffer into the SW pipe, and also
/// Advance the position of the pipe cursor for the next call.

///@note Sync Word is not checked to equal FF55 because it doesn't matter for
///      USB transfer.  You could add sync word checking if you want to be
///      exhaustively compliant.  Do it in the if() block.

    ot_int  rxbytes;
    ot_int  maxbytes;
    ot_u8*  cursor;

    // Get amount of bytes in the Endpoint
    rxbytes = GetEPRxCount(ENDP3);

    // The first block contains the header, which is cropped.
    ///@todo check against max packet length (maybe in mpipedrv_isr())
    if (cdcacm.pktend == NULL) {
        rxbytes -= 8;
        PMAToUserBufferCopy(&cdcacm.header.syncFF, ENDP3_RXADDR, 8);

        // Get partial CRC and endian-ize the payload length
        cdcacm.header.crc16 = crc16drv_block((ot_u8*)&cdcacm.header.crc16, 6);
        cdcacm.header.plen  = PLATFORM_ENDIAN16(cdcacm.header.plen);

        // Protect against too-long packets (they get cropped)
        cdcacm.pktend = cdcacm.pkt + cdcacm.header.plen;
        if (cdcacm.pktend > mpipe.alp.inq->back) {
            cdcacm.pktend = mpipe.alp.inq->back;
        }
    }

    // Protect against pending overflow
    maxbytes = cdcacm.pktend - cdcacm.pkt;
    if (rxbytes > maxbytes) {
        rxbytes = maxbytes;
    }

    // Load the packet and advance cursors
    cursor      = cdcacm.pkt;
    cdcacm.pkt += rxbytes;
    PMAToUserBufferCopy(cursor, ENDP3_RXADDR, rxbytes);

    // call virtual ISR to frame the data
    mpipedrv_isr();
}





/** USB Wakeup interrupt   <BR>
  * ========================================================================<BR>
  * This would do something for setups with attachable USB
  */
void platform_isr_fswkup() {

}








/** Mpipe Main Subroutines   <BR>
  * ========================================================================<BR>
  */
void sub_txopen();

void sub_usb_loadheader() {
    UserToPMABufferCopy( &cdcacm.header.syncFF, ENDP1_TXADDR, 8 );
}

void sub_usb_loadtx(ot_int pad_bytes) {
    ot_int transfer_size;
    ot_int remaining_bytes;
    ot_u8* transfer_start;

    transfer_start  = cdcacm.pkt;
    remaining_bytes = cdcacm.pktend - cdcacm.pkt;
    transfer_size   = CDCACM_DATA_SIZE - pad_bytes;
    if (remaining_bytes < transfer_size) {
        transfer_size = remaining_bytes;
    }
    cdcacm.pkt += transfer_size;

    UserToPMABufferCopy(transfer_start, ENDP1_TXADDR, transfer_size);
    SetEPTxCount(ENDP1, transfer_size);
    SetEPTxValid(ENDP1);
}

void sub_gen_mpipecrc() {
    cdcacm.header.crc16 = crc16drv_block_manual((ot_u8*)&cdcacm.header.plen,
                                                    4,
                                                    0xFFFF  );

    cdcacm.header.crc16 = crc16drv_block_manual((ot_u8*)mpipe.alp.outq->getcursor,
                                                    cdcacm.header.plen,
                                                    cdcacm.header.crc16 );

}





/** Mpipe Main Public Functions  <BR>
  * ========================================================================
  */
#ifndef EXTF_mpipedrv_setspeed

void mpipedrv_setspeed(mpipe_speed speed) {

    linecoding.bitrate = br_hssel[speed];

}

#endif




#ifndef EXTF_mpipedrv_footerbytes
ot_u8 mpipedrv_footerbytes() {
    return MPIPE_FOOTERBYTES;
}
#endif


#ifndef EXTF_mpipedrv_init
ot_int mpipedrv_init(void* port_id, mpipe_speed baud_rate) {
/// <LI> "port_id" is unused in this impl, and it may be NULL                   </LI>
/// <LI> Prepare the HW, which in this case is a USB CDC (i.e. Virtual COM)     </LI>
/// <LI> USB is disconnected at startup in order to clear the host USB driver,
///        which will get stuck during debugging or other sleeping processes.   </LI>

    alp_init(&mpipe.alp, &otmpin, &otmpout);
    mpipe.alp.inq->back    -= MPIPE_OVERHEADBYTES;
    mpipe.alp.outq->back   -= MPIPE_OVERHEADBYTES;
    cdcacm.pkt              = mpipe.alp.outq->front;
    cdcacm.header.seq       = 0;                        // not actually necessary
    mpipe.state             = MPIPE_Null;               // Disconnected

    mpipedrv_setspeed(MPIPE_115200bps);     //default baud rate

    /// Enable USB Clocks & Peripheral
    platform_flank_speed();
    __USB_CLKON();
    //BOARD_USB_PORT->MODER  |= (GPIO_MODER_ALT << (BOARD_USB_DMPINNUM*2)) \
                            | (GPIO_MODER_ALT << (BOARD_USB_DPPINNUM*2));

    /// Set USB-Wakeup interrupt on Line 18: optional
    /// @todo get this working properly
    //EXTI->PR                                   &= ~(1<<18);
    //EXTI->RTSR                                 |= (1<<18);
    //EXTI->IMR                                  |= (1<<18);
    //NVIC->IP[(ot_u32)USB_FS_WKUP_IRQn]          = _IRQGROUP;
    //NVIC->ISER[(ot_u32)(USB_FS_WKUP_IRQn>>5)]   = (1 << (USB_FS_WKUP_IRQn & 0x1F));

    /// Set USB Low-Priority interrupt
    /// USB High-Priority interrupt is not used in this implementation
    NVIC->IP[(ot_u32)USB_LP_IRQn]           = _IRQGROUP;
    NVIC->ISER[(ot_u32)(USB_LP_IRQn>>5)]    = (1 << (USB_LP_IRQn & 0x1F));

    /// Enable the USB connection, which configures Pullups
    BOARD_USB_PORTENABLE();

    /// Initialize USB (using ST library function)
    USB_Init();

#   if defined(__DEBUG__) || defined(__PROTO__)
    //cdcacm_Reset();
#   endif

    return 255;
}
#endif


#ifndef EXTF_mpipedrv_standby
void mpipedrv_standby() {
/// Put into SLEEP Mode 1 until there's a wakeup
    //sys_sig_powerdown(1);
    while (mpipe.state != MPIPE_Idle);
}
#endif


#ifndef EXTF_mpipedrv_detach
void mpipedrv_detach(void* port_id) {
    mpipe.state = MPIPE_Null;
    //usb_disconnect();
    __USB_CLKOFF();
}
#endif


#ifndef EXTF_mpipedrv_clear
void mpipedrv_clear() {
    //__CLR_MPIPE();
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
	cdcacm.pkt = mpipe.alp.outq->front;
}
#endif



#ifndef EXTF_mpipedrv_wait
void mpipedrv_wait() {
    while (mpipe.state != MPIPE_Idle) {
        MCU_SLEEP();
    }
}
#endif



#ifndef EXTF_mpipedrv_tx
ot_uint mpipedrv_tx(ot_bool blocking, mpipe_priority data_priority) {
    // Adding a packet is an atomic operation
    ///@todo disable USB interrupt(s)

    // Move getcursor to end of packet, to allow another packet to be added
    cdcacm.pkt                  = mpipe.alp.outq->getcursor;
    mpipe.alp.outq->getcursor   = mpipe.alp.outq->putcursor;
    cdcacm.packets++;

    // If MPipe is not doing something else now, initiate the TX
    if (mpipe.state == MPIPE_Idle) {
        sub_txopen();
        ///@todo enable USB interrupt(s)

        if (blocking) {
            mpipedrv_wait();
        }
    }

    ///@todo enable USB interrupt(s)

    return (10 + (q_length(mpipe.alp.outq) >> 8));
}
#endif

void sub_txopen() {
    ot_u16 length;

    mpipe.state     = MPIPE_Tx_Wait;
    cdcacm.packets -= 1;

    // Setup Header information
    length                  = mpipe.alp.outq->getcursor[1] + 4;
    cdcacm.header.syncFF    = 0xff;
    cdcacm.header.sync55    = 0x55;
    cdcacm.header.seq      += 1;
    cdcacm.header.ctl       = 0;
    cdcacm.header.plen      = PLATFORM_ENDIAN16(length);
    sub_gen_mpipecrc();

    sub_usb_loadheader();
    sub_usb_loadtx(8);
}





#ifndef EXTF_mpipedrv_rx
void mpipedrv_rx(ot_bool blocking, mpipe_priority data_priority) {
    if (blocking) {
        mpipedrv_wait();
    }
    if (mpipe.state == MPIPE_Idle) {
        //mpipe.state = MPIPE_Idle;
        cdcacm.pkt      = mpipe.alp.inq->front;
        cdcacm.pktend   = NULL;
    }
}
#endif






#ifndef EXTF_mpipedrv_isr
void mpipedrv_isr() {
    switch (mpipe.state) {
        case MPIPE_Idle: //note, case doesn't break!

        /// The first page is received, so change state.  Fall through if there
        /// is an empty payload
        case MPIPE_RxHeader:
            if (cdcacm.header.plen != 0) {
                mpipe.state = MPIPE_RxPayload;
                mpipeevt_rxdetect(10);
                break;
            }

        /// The reception is completely done.  Move the putcursor to account for
        /// new data, while cropping the footer.  Go to Idle and do callback.
        case MPIPE_RxPayload:
            if (cdcacm.pkt == cdcacm.pktend) {
                if (crc16drv_block_manual(cdcacm.pkt, cdcacm.header.plen, cdcacm.header.crc16) == 0) {
                    mpipe.alp.inq->putcursor    = cdcacm.pktend;
                    mpipe.state                 = MPIPE_Idle;
                    mpipeevt_rxdone(0);
                }
                mpipedrv_rx(False, 0);
            }
            break;

        /// Transmit another page of the outbound packet
        case MPIPE_Tx_Wait:
            sub_usb_loadtx(0);
            mpipe.state += (cdcacm.pktend == cdcacm.pkt);
            break;

        /// The last transmission is done.  If there are additional packets
        /// in the queue, send them without returning to the kernel.
        case MPIPE_Tx_Done: {
            if (cdcacm.packets > 0) {
                sub_txopen();
            }
            else {
                mpipedrv_rx(False, 0);
                mpipeevt_txdone(0);
            }
        } break;
    }
}
#endif



#endif

