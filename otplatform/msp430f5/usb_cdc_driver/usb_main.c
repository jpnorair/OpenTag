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

#include "OT_config.h"
#include "veelite.h"	//for getting device id

#include "msp430f5_lib.h"

#include "usb_cdc_driver/usb_device.h"
#include "usb_cdc_driver/usb_types.h"    //Basic Type declarations
#include "usb_cdc_driver/defMSP430USB.h"
#include "usb_cdc_driver/usb_main.h"      //USB-specific Data Structures
#include "usb_cdc_driver/usb_cdc_backend.h"
#include "usb_cdc_driver/usb_descriptors.h"

#if defined(_CDC_)

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

//Set to 1 when USB device has been configured, set to 0 when unconfigured
static ot_u8 bConfigurationNumber;


//interface number
static ot_u8 bInterfaceNumber;


//For endpoint zero transmitter only Holds count of bytes remaining to be
//transmitted by endpoint 0.  A value of 0 means that a 0-length data packet
//A value of 0xFFFF means that transfer is complete
ot_u16 wBytesRemainingOnIEP0;


//For endpoint zero transmitter only Holds count of bytes remaining to be
//received by endpoint 0.  A value of 0 means that a 0-length data packet
//A value of 0xFFFF means that transfer is complete.
ot_u16 wBytesRemainingOnOEP0;


//A buffer pointer to input end point 0 Data sent back to host is copied from
//this pointed memory location
static ot_u8* pbIEP0Buffer;


//A buffer pointer to output end point 0 Data sent from host is copied to
//this pointed memory location
static ot_u8* pbOEP0Buffer;



static ot_u8 bHostAskMoreDataThanAvailable = 0;

ot_u8 abUsbRequestReturnData[USB_RETURN_DATA_LENGTH];
ot_u8 abUsbRequestIncomingData[USB_RETURN_DATA_LENGTH];

__no_init ot_u8 abramSerialStringDescriptor[34];

ot_u8 bStatusAction;
ot_u8 bFunctionSuspended = FALSE;    //TRUE if function is suspended
ot_u8 bEnumerationStatus = 0;        //is 0 if not enumerated

static ot_u8 bRemoteWakeup;

//ot_u16 wUsbEventMask;                 //used by USB_getEnabledEvents() and USB_setEnabledEvents()



/*----------------------------------------------------------------------------+
 | Global Variables                                                            |
 +----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
 | Hardware Related Structure Definition                                       |
 +----------------------------------------------------------------------------*/

#if defined(__IAR_SYSTEMS_ICC__)

#pragma location = 0x2380
__no_init tDEVICE_REQUEST __data16 tSetupPacket;

#pragma location = 0x0920
__no_init tEDB0 __data16 tEndPoint0DescriptorBlock;

#pragma location = 0x23C8
__no_init tEDB __data16 tInputEndPointDescriptorBlock[7];

#pragma location = 0x2388
__no_init tEDB __data16 tOutputEndPointDescriptorBlock[7];

#pragma location = 0x2378
__no_init ot_u8 __data16 abIEP0Buffer[EP0_MAX_PACKET_SIZE];

#pragma location = 0x2370
__no_init ot_u8 __data16 abOEP0Buffer[EP0_MAX_PACKET_SIZE];

#pragma location = OEP1_X_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbXBufferAddressEp1[EP_MAX_PACKET_SIZE];

#pragma location = OEP1_Y_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbYBufferAddressEp1[EP_MAX_PACKET_SIZE];

#pragma location = IEP1_X_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbXBufferAddressEp81[EP_MAX_PACKET_SIZE];

#pragma location = IEP1_Y_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbYBufferAddressEp81[EP_MAX_PACKET_SIZE];

#pragma location = OEP2_X_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbXBufferAddressEp2[EP_MAX_PACKET_SIZE];

#pragma location = OEP2_Y_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbYBufferAddressEp2[EP_MAX_PACKET_SIZE];

#pragma location = IEP2_X_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbXBufferAddressEp82[EP_MAX_PACKET_SIZE];

#pragma location = IEP2_Y_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbYBufferAddressEp82[EP_MAX_PACKET_SIZE];

#pragma location = OEP3_X_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbXBufferAddressEp3[EP_MAX_PACKET_SIZE];

#pragma location = OEP3_Y_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbYBufferAddressEp3[EP_MAX_PACKET_SIZE];

#pragma location = IEP3_X_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbXBufferAddressEp83[EP_MAX_PACKET_SIZE];

#pragma location = IEP3_Y_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbYBufferAddressEp83[EP_MAX_PACKET_SIZE];

#pragma location = OEP4_X_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbXBufferAddressEp4[EP_MAX_PACKET_SIZE];

#pragma location = OEP4_Y_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbYBufferAddressEp4[EP_MAX_PACKET_SIZE];

#pragma location = IEP4_X_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbXBufferAddressEp84[EP_MAX_PACKET_SIZE];

#pragma location = IEP4_Y_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbYBufferAddressEp84[EP_MAX_PACKET_SIZE];

#pragma location = OEP5_X_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbXBufferAddressEp5[EP_MAX_PACKET_SIZE];

#pragma location = OEP5_Y_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbYBufferAddressEp5[EP_MAX_PACKET_SIZE];

#pragma location = IEP5_X_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbXBufferAddressEp85[EP_MAX_PACKET_SIZE];

#pragma location = IEP5_Y_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbYBufferAddressEp85[EP_MAX_PACKET_SIZE];

#pragma location = OEP6_X_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbXBufferAddressEp6[EP_MAX_PACKET_SIZE];

#pragma location = OEP6_Y_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbYBufferAddressEp6[EP_MAX_PACKET_SIZE];

#pragma location = IEP6_X_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbXBufferAddressEp86[EP_MAX_PACKET_SIZE];

#pragma location = IEP6_Y_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbYBufferAddressEp86[EP_MAX_PACKET_SIZE];

#pragma location = OEP7_X_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbXBufferAddressEp7[EP_MAX_PACKET_SIZE];

#pragma location = OEP7_Y_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbYBufferAddressEp7[EP_MAX_PACKET_SIZE];

#pragma location = IEP7_X_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbXBufferAddressEp87[EP_MAX_PACKET_SIZE];

#pragma location = IEP7_Y_BUFFER_ADDRESS
__no_init ot_u8 __data16 pbYBufferAddressEp87[EP_MAX_PACKET_SIZE];



#elif 0 //defined(__TI_COMPILER_VERSION__)
extern __no_init tDEVICE_REQUEST tSetupPacket;
extern __no_init tEDB0 tEndPoint0DescriptorBlock;
extern __no_init tEDB tInputEndPointDescriptorBlock[7];
extern __no_init tEDB tOutputEndPointDescriptorBlock[7];
extern __no_init ot_u8 abIEP0Buffer[EP0_MAX_PACKET_SIZE];
extern __no_init ot_u8 abOEP0Buffer[EP0_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbXBufferAddressEp1[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbYBufferAddressEp1[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbXBufferAddressEp81[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbYBufferAddressEp81[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbXBufferAddressEp2[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbYBufferAddressEp2[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbXBufferAddressEp82[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbYBufferAddressEp82[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbXBufferAddressEp3[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbYBufferAddressEp3[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbXBufferAddressEp83[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbYBufferAddressEp83[EP_MAX_PACKET_SIZE];

extern __no_init ot_u8 pbXBufferAddressEp4[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbYBufferAddressEp4[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbXBufferAddressEp84[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbYBufferAddressEp84[EP_MAX_PACKET_SIZE];

extern __no_init ot_u8 pbXBufferAddressEp5[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbYBufferAddressEp5[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbXBufferAddressEp85[EP_MAX_PACKET_SIZE];
extern __no_init ot_u8 pbYBufferAddressEp85[EP_MAX_PACKET_SIZE];


#else
//defined in usb.h




#endif


const tEDB* dbselect[] = { tInputEndPointDescriptorBlock,
                           tOutputEndPointDescriptorBlock
                          };
                          
                          

void CdcResetData();

void USB_InitSerialStringDescriptor(void);


//Replaced with platform_memcpy(), which already has a DMA vs. Non-DMA select.
//void USB_initMemcpy (void);



ot_u8 USB_init (void) {
    ot_u16 bGIE  = __get_SR_register() & GIE;  	//save interrupt status

    //atomic operation - disable interrupts
    __disable_interrupt();

    // configuration of USB module
    // To fix USB9 enumeration issue seen by Matrox
    USBKEYPID   = 0x9628;
    USBPWRCTL	= 0;
    USBPHYCTL   = PUSEL;				//use DP + DM as USB pins (not needed with external PHY on P9)
    USBPWRCTL   = VUSBEN + SLDOAON;		//enable primary and secondary LDO (3.3 and 1.8V)
    
    //wait some time for LDOs (5ms delay)
    platform_swdelay_ms(5);				
    
    USBPWRCTL   = VUSBEN + SLDOAON + VBONIE; 	//enable interrupt VBUSon
    USBKEYPID   = 0x9600;                       //access to configuration registers disabled

    //reset events mask
    //wUsbEventMask = 0;

    //init Serial Number
#if (USB_STR_INDEX_SERNUM != 0)
    USB_InitSerialStringDescriptor();
#endif

    //USB_initMemcpy();				//using platform_memcpy instead

    __bis_SR_register(bGIE);                                                    //restore interrupt status
    return (kUSB_succeed);
}





#if (USB_STR_INDEX_SERNUM != 0)
void USB_InitSerialStringDescriptor (void) {
    //ot_u8* pbSerNum;
    //ot_u8 bBytes;
    //ot_u8 j;

    //pbSerNum = 0;
    *((ot_u16*)&abramSerialStringDescriptor[0]) = (4 | (DESC_TYPE_STRING<<8));
    *((ot_u16*)&abramSerialStringDescriptor[2]) = 0;

    //Get Hex enumeration string from lower 16bits of device ID
    {
        vlFILE* fp;
        ot_u16  uid16;
        ot_u8   enumid[4];
        
        fp      = ISF_open_su(1);
        uid16   = vl_read(fp, 6);   // Device UID takes first 8 bytes of ISF 1
        vl_close(fp);
        
        abramSerialStringDescriptor[0] = \
            2 + otutils_bin2hex((ot_u8*)&uid16, enumid, 2);
        
    }
    
    
//    TLV_Get_Info(TLV_DIERECORD, 0, (u8*)&bBytes, (u16**)&pbSerNum); //The die record used for serial number
//
//    if (bBytes != 0) {
//    	if (bBytes > 8)
//    		bBytes = 8;
//
//        for (j=1; bBytes!=0; ++pbSerNum, --bBytes) {
//        	ot_u8 hex1  = *pbSerNum & 0x0F;
//        	ot_u8 hex0	= (*pbSerNum >> 4) & 0x0F;
//            hex0	   += (hex0 >= 10) ? ('a'-10) : '0';
//            hex1	   += (hex1 >= 10) ? ('a'-10) : '0';
//
//            abramSerialStringDescriptor[++j] = hex0;
//            abramSerialStringDescriptor[++j] = 0x00;		//unicode extension
//            abramSerialStringDescriptor[++j] = hex1;
//            abramSerialStringDescriptor[++j] = 0x00;		//unicode extension
//        }
//        abramSerialStringDescriptor[0] = ++j;      	//calculate the length
//    }
}

#endif






ot_u8 USB_enable () {
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
    USBKEYPID	= 0x9628;
    USBPLLDIVB 	= USB_XT_FREQ;					//Settings desired frequency

#	if (USB_PLL_XT == 2)
        USBPLLCTL = UPCS0 + UPFDEN + UPLLEN;  	//Select XT2 as Ref, PLL for USB, Discrim. on, enable PLL
#	else
        USBPLLCTL = UPFDEN + UPLLEN;			//Select XT1 as Ref, PLL for USB, Discrim. on, enable PLL
#	endif

    //Wait some time till PLL is settled
    // - Clear PLL IR and wait some time for settling
    // - Success if PLL IR stays 0 (no error flags go high)
    // - Try 10 times, and give-up if PLL IR will not settle after 10 tries
    do {
        USBPLLIR    = 0x0000;
#		ifdef __MSP430F6638
        	platform_swdelay_us(1000);
        	//for (i = 0; i < USB_MCLK_FREQ / 1000 * 1 / 10; i++);  //1ms
#		else
        	platform_swdelay_us(500);
        	//for (i = 0; i < USB_MCLK_FREQ / 1000 * 1 / 2 / 10; i++);	//0.5ms
#		endif

        pll_unsettled = (USBPLLIR != 0);
    }
    while (pll_unsettled && (++j <= 10));

    USBCNF     |= !pll_unsettled;	//Enable USB if PLL settled (USB_EN = 0x0001)
    USBKEYPID   = 0x9600;

    return pll_unsettled;	//kUSB_succeed = 0, kUSB_generalError = 1
}




/* Disables the USB module and PLL.
 */
ot_u8 USB_disable (void) {
    USBKEYPID			= 0x9628;
    USBCNF    			= 0;  			//disable USB module
    USBPLLCTL  		   &= ~UPLLEN;      //disable PLL
    USBKEYPID 			= 0x9600;
    bEnumerationStatus 	= 0x00;         //device is not enumerated
    bFunctionSuspended 	= FALSE;        //device is not suspended
    return (kUSB_succeed);
}




/* Enables/disables various USB events.
 */
//ot_u8 USB_setEnabledEvents (ot_u16 events) {
//    wUsbEventMask = events;
//    return (kUSB_succeed);
//}




/* Returns which events are enabled and which are disabled.
 */
//ot_u16 USB_getEnabledEvents () {
//    return (wUsbEventMask);
//}




/* Reset USB-SIE and global variables.
 */
ot_u8 USB_reset () {
	ot_u8 edbIndex;

    USBKEYPID = 0x9628;

    //reset should be on the bus after this!
    bEnumerationStatus 		= 0x00;             //Device not enumerated yet
    bFunctionSuspended 		= FALSE;            //Device is not in suspend mode
    bRemoteWakeup 			= DISABLE;
    bConfigurationNumber	= 0x00;        		//device unconfigured
    bInterfaceNumber    	= 0x00;

    //FRSTE handling:
    //Clear FRSTE in the RESRIFG interrupt service routine before re-configuring USB control registers.
    //Set FRSTE at the beginning of SUSRIFG, SETUP, IEPIFG.EP0 and OEPIFG.EP0 interrupt service routines.
    USBCTL 					= 0;        //Function Reset Connection disable (FRSTE)

    wBytesRemainingOnIEP0   = NO_MORE_DATA;
    wBytesRemainingOnOEP0   = NO_MORE_DATA;
    bStatusAction           = STATUS_ACTION_NOTHING;

    //The address reset normally will be done automatically during bus function reset
    USBFUNADR   			= 0x00;     //Address 0 is the value for "unconfigured device"


    // Set settings for EP0
    //NAK both 0 endpoints and enable endpoint 0 interrupt
    tEndPoint0DescriptorBlock.bIEPBCNT = EPBCNT_NAK;
    tEndPoint0DescriptorBlock.bOEPBCNT = EPBCNT_NAK;
    tEndPoint0DescriptorBlock.bIEPCNFG = EPCNF_USBIE | EPCNF_UBME | EPCNF_STALL;    //8 byte data packet
    tEndPoint0DescriptorBlock.bOEPCNFG = EPCNF_USBIE | EPCNF_UBME | EPCNF_STALL;    //8 byte data packet

    USBOEPIE = USB_OUTEP_INT_EN;
    USBIEPIE = USB_INEP_INT_EN;

    edbIndex = stUsbHandle[0].edb_Index;

    // Set settings for IEPx
    tInputEndPointDescriptorBlock[edbIndex].bEPCNF   = EPCNF_USBIE | EPCNF_UBME | EPCNF_DBUF;   //double buffering

    tInputEndPointDescriptorBlock[edbIndex].bEPBBAX  =
        (ot_u8)(((stUsbHandle[0].iep_X_Buffer - START_OF_USB_BUFFER) >> 3) & 0x00ff);

    tInputEndPointDescriptorBlock[edbIndex].bEPBBAY  =
        (ot_u8)(((stUsbHandle[0].iep_Y_Buffer - START_OF_USB_BUFFER) >> 3) & 0x00ff);

    tInputEndPointDescriptorBlock[edbIndex].bEPBCTX  = EPBCNT_NAK;
    tInputEndPointDescriptorBlock[edbIndex].bEPBCTY  = EPBCNT_NAK;
    tInputEndPointDescriptorBlock[edbIndex].bEPSIZXY = MAX_PACKET_SIZE;

    // Set settings for OEPx
    //double buffering
    tOutputEndPointDescriptorBlock[edbIndex].bEPCNF = EPCNF_USBIE | EPCNF_UBME | EPCNF_DBUF ;
        
    tOutputEndPointDescriptorBlock[edbIndex].bEPBBAX  =
        (ot_u8)(((stUsbHandle[0].oep_X_Buffer - START_OF_USB_BUFFER) >> 3) & 0x00ff);

    tOutputEndPointDescriptorBlock[edbIndex].bEPBBAY  =
        (ot_u8)(((stUsbHandle[0].oep_Y_Buffer - START_OF_USB_BUFFER) >> 3) & 0x00ff);

    tOutputEndPointDescriptorBlock[edbIndex].bEPBCTX  = 0x00;
    tOutputEndPointDescriptorBlock[edbIndex].bEPBCTY  = 0x00;
    tOutputEndPointDescriptorBlock[edbIndex].bEPSIZXY = MAX_PACKET_SIZE;

    // Additional interrupt end point for CDC
    if (stUsbHandle[0].dev_Class == CDC_CLASS){
        //The decriptor tool always generates the managemnet endpoint before the data endpoint
        tInputEndPointDescriptorBlock[edbIndex-1].bEPCNF	= EPCNF_USBIE | EPCNF_UBME | EPCNF_DBUF;
        tInputEndPointDescriptorBlock[edbIndex-1].bEPBBAX   =
            (ot_u8)(((stUsbHandle[0].intepEP_X_Buffer - START_OF_USB_BUFFER) >> 3) & 0x00ff);

        tInputEndPointDescriptorBlock[edbIndex-1].bEPBBAY   =
            (ot_u8)(((stUsbHandle[0].intepEP_Y_Buffer - START_OF_USB_BUFFER) >> 3) & 0x00ff);

        tInputEndPointDescriptorBlock[edbIndex-1].bEPBCTX   = EPBCNT_NAK;
        tInputEndPointDescriptorBlock[edbIndex-1].bEPBCTY   = EPBCNT_NAK;
        tInputEndPointDescriptorBlock[edbIndex-1].bEPSIZXY  = MAX_PACKET_SIZE;
    }

    CdcResetData();   //reset CDC specific data structures

    USBCTL		= FEN;    						//enable function
    USBIFG 		= 0;        					//make sure no interrupts are pending
    USBIE 		= SETUPIE | RSTRIE | SUSRIE;    //enable USB specific interrupts (setup, reset, suspend)
    USBKEYPID 	= 0x9600;

    return kUSB_succeed;
}




/* Instruct USB module to make itself available to the PC for connection, by pulling PUR high.
 */
ot_u8 USB_connect () {
    USBKEYPID	= 0x9628;
    USBCNF 	   |= PUR_EN;   //Generate rising edge on DP -> the host, which
                            //enumerates our device as full speed device.
    USBPWRCTL  |= VBOFFIE;  //Enable interrupt VUSBoff
    USBKEYPID   = 0x9600;

    return (kUSB_succeed);
}




/* Force a disconnect from the PC by pulling PUR low.
 */
ot_u8 USB_disconnect () {
    USBKEYPID	        = 0x9628;
    USBCNF             &= ~PUR_EN;	//disconnect pull up resistor - logical disconnect from HOST
    USBPWRCTL          &= ~VBOFFIE; //disable interrupt VUSBoff
    USBKEYPID           = 0x9600;
    bEnumerationStatus	= 0;    	//not enumerated
    bFunctionSuspended 	= FALSE;    //device is not suspended

    return (kUSB_succeed);
}




/* Force a remote wakeup of the USB host.
 */
ot_u8 USB_forceRemoteWakeup () {
    if (bFunctionSuspended == FALSE) {   //device is not suspended
        return (kUSB_NotSuspended);
    }
    if (bRemoteWakeup == ENABLE) {
        //volatile unsigned int i;
        USBCTL |= RWUP;                 //USB - Device Remote Wakeup Request - this bit is self-cleaned
        return (kUSB_succeed);
    }
    return (kUSB_generalError);
}




/* Returns the status of the USB connection.
 */
ot_u8 USB_connectionInfo () {
    ot_u8 retval;

    //ot_u8 retval = 0;
    //if (USBPWRCTL & USBBGVBV)			retval |= kUSB_vbusPresent;
    //if (USBCNF & PUR_EN)				retval |= kUSB_purHigh;
    //if (bEnumerationStatus != 0)		retval |= kUSB_Enumerated;
    //if (bFunctionSuspended != FALSE)	retval |= kUSB_suspended;
    //else 								retval |= kUSB_NotSuspended;
    
    // JP's version below: compiles smaller, runs faster
    retval  = kUSB_suspended;                   //kUSB_suspended == 0x08
    retval<<= (bFunctionSuspended == FALSE);    //kUSB_NotSuspended == 0x10
    
    retval |= ((USBPWRCTL & USBBGVBV) != 0);    //USBBGVBV is 0x08: 1 == kUSB_vbusPresent
    retval |= (USBCNF & PUR_EN) << 5;           //PUR_EN is 0x02:  2<<5 == kUSB_purHigh
    retval |= (bEnumerationStatus != 0)	<< 5;   //1<<5 == kUSB_Enumerated

    return retval;
}




/* Returns the state of the USB connection.
 */
ot_u8 USB_connectionState () {

	// JP's version below: compiles smaller, runs faster
	static const ot_u8 retval[16] = {
		ST_USB_DISCONNECTED, ST_USB_DISCONNECTED, ST_USB_DISCONNECTED, ST_USB_DISCONNECTED,
		ST_USB_DISCONNECTED, ST_USB_DISCONNECTED, ST_USB_DISCONNECTED, ST_USB_DISCONNECTED,
		ST_USB_CONNECTED_NO_ENUM, ST_ENUM_IN_PROGRESS,
		ST_USB_CONNECTED_NO_ENUM, ST_ENUM_ACTIVE,
		ST_USB_CONNECTED_NO_ENUM, ST_ENUM_IN_PROGRESS,
		ST_NOENUM_SUSPENDED, ST_ENUM_SUSPENDED
	};

	// 8 + 4 + 2 + 1
	ot_u8 code = (USBPWRCTL & USBBGVBV) + \
				((ot_u8)bFunctionSuspended << 2) + \
				(USBCNF & PUR_EN) + \
				(bEnumerationStatus);

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
        && (bEnumerationStatus == ENUMERATION_COMPLETE)
        && (!(bFunctionSuspended == TRUE))){
        return (ST_ENUM_ACTIVE);
    }

    //If VBUS present, PUR is high, and enumeration is NOT complete, and  suspended
    if ((USBPWRCTL & USBBGVBV) && (USBCNF & PUR_EN)
        && (!(bEnumerationStatus == ENUMERATION_COMPLETE))
        && (bFunctionSuspended == TRUE)){
        return (ST_NOENUM_SUSPENDED);
    }

    //If VBUS present, PUR is high, and enumeration is complete, and  suspended
    if ((USBPWRCTL & USBBGVBV) && (USBCNF & PUR_EN)
        && (bEnumerationStatus == ENUMERATION_COMPLETE)
        && (bFunctionSuspended == TRUE)){
        return (ST_ENUM_SUSPENDED);
    }

    //If VBUS present, PUR is high, but no enumeration yet
    if ((USBPWRCTL & USBBGVBV) && (USBCNF & PUR_EN)
        && (!(bEnumerationStatus == ENUMERATION_COMPLETE))){
        return (ST_ENUM_IN_PROGRESS);
    }

    return (ST_ERROR);
    */
}




ot_u8 USB_suspend (void) {
    bFunctionSuspended	= TRUE;
    USBKEYPID		    = 0x9628;                 //set KEY and PID to 0x9628 -> access to configuration registers enabled
    USBCTL 			   |= FRSTE;                    //Function Reset Connection Enable
    USBIFG 			   &= ~SUSRIFG;                 //clear interrupt flag
    USBPLLCTL 		   &= ~UPLLEN;

#   if (USB_DISABLE_XT_SUSPEND)
#       if (USB_PLL_XT == 2)
            UCSCTL6 |= XT2OFF;        //disable XT2
#       else
            UCSCTL6 |= XT1OFF;
#       endif
#	endif

    //Disable USB specific interrupts (setup, suspend, reset), enable resume.
    //If the reset occurred during device in suspend, the resume-interrupt will
    //come, after reset interrupt
    USBIE 		= RESRIE;
    USBKEYPID	= 0x9600;

    return (kUSB_succeed);
}




ot_u8 USB_resume (void) {
    USB_enable();                       	//enable PLL

    USBIFG &= ~(RESRIFG | SUSRIFG);     	//clear interrupt flags
    USBIE	= SETUPIE | RSTRIE | SUSRIE;  	//enable USB specific interrupts (setup, reset, suspend)

    bFunctionSuspended = FALSE;
    return (kUSB_succeed);
}




void usbStallEndpoint0 (void) {
    tEndPoint0DescriptorBlock.bIEPCNFG |= EPCNF_STALL;
    tEndPoint0DescriptorBlock.bOEPCNFG |= EPCNF_STALL;
}




void usbClearOEP0ByteCount (void) {
    tEndPoint0DescriptorBlock.bOEPBCNT = 0x00;
}




void usbStallOEP0 (void) {
    //in standard USB request, there is not control write request with data stage
    //control write, stall output endpoint 0
    //wLength should be 0 in all cases
    tEndPoint0DescriptorBlock.bOEPCNFG |= EPCNF_STALL;
}




void usbSendNextPacketOnIEP0 (void) {
    ot_u8 bPacketSize, bIndex;

    //First check if there are bytes remaining to be transferred
    if (wBytesRemainingOnIEP0 != NO_MORE_DATA){
        if (wBytesRemainingOnIEP0 > EP0_PACKET_SIZE){
            //More bytes are remaining than will fit in one packet
            //there will be More IN Stage
            bPacketSize 			= EP0_PACKET_SIZE;
            wBytesRemainingOnIEP0  -= EP0_PACKET_SIZE;
            bStatusAction 			= STATUS_ACTION_DATA_IN;
        }
        else if (wBytesRemainingOnIEP0 < EP0_PACKET_SIZE){
            //The remaining data will fit in one packet.
            //This case will properly handle wBytesRemainingOnIEP0 == 0
            bPacketSize 			= (ot_u8)wBytesRemainingOnIEP0;
            wBytesRemainingOnIEP0 	= NO_MORE_DATA;   //No more data need to be Txed
            bStatusAction 			= STATUS_ACTION_NOTHING;
        }
        else {
            bPacketSize = EP0_PACKET_SIZE;
            if (bHostAskMoreDataThanAvailable == TRUE) {
                wBytesRemainingOnIEP0 	= 0;
                bStatusAction 			= STATUS_ACTION_DATA_IN;
            }
            else {
                wBytesRemainingOnIEP0 	= NO_MORE_DATA;
                bStatusAction 			= STATUS_ACTION_NOTHING;
            }
        }

        for (bIndex = 0; bIndex < bPacketSize; bIndex++) {
            abIEP0Buffer[bIndex] = *pbIEP0Buffer;
            pbIEP0Buffer++;
        }
        tEndPoint0DescriptorBlock.bIEPBCNT = bPacketSize;
    }
    else {
        bStatusAction = STATUS_ACTION_NOTHING;
    }
}




void usbSendDataPacketOnEP0 (ot_u8* pbBuffer) {
    ot_u16 wTemp;

    pbIEP0Buffer	= pbBuffer;
    wTemp 			= tSetupPacket.wLength;

    //Limit transfer size to wLength if needed
    //this prevent USB device sending 'more than require' data back to host
    bHostAskMoreDataThanAvailable = (wBytesRemainingOnIEP0 < wTemp);

    if (bHostAskMoreDataThanAvailable == FALSE){
        wBytesRemainingOnIEP0 = wTemp;
    }

    usbSendNextPacketOnIEP0();
}



void usbReceiveNextPacketOnOEP0 (void) {
    ot_u8 bIndex,bByte;

    bByte = tEndPoint0DescriptorBlock.bOEPBCNT & EPBCNT_BYTECNT_MASK;

    if (wBytesRemainingOnOEP0 >= (ot_u16)bByte){
        for (bIndex = 0; bIndex < bByte; bIndex++) {
            *pbOEP0Buffer = abOEP0Buffer[bIndex];
            pbOEP0Buffer++;
        }
        wBytesRemainingOnOEP0 -= (ot_u16)bByte;

        //clear the NAK bit for next packet
        if (wBytesRemainingOnOEP0 > 0){
            usbClearOEP0ByteCount();
            bStatusAction = STATUS_ACTION_DATA_OUT;
            return;
        }
    }

    usbStallOEP0();
    bStatusAction = STATUS_ACTION_NOTHING;
}





void usbReceiveDataPacketOnEP0 (ot_u8* pbBuffer) {
    pbOEP0Buffer 			= pbBuffer;
    wBytesRemainingOnOEP0 	= tSetupPacket.wLength;
    bStatusAction 			= STATUS_ACTION_DATA_OUT;

    usbClearOEP0ByteCount();
}




void usbSendZeroLengthPacketOnIEP0 (void) {
    tEndPoint0DescriptorBlock.bIEPBCNT 	= 0x00;
    wBytesRemainingOnIEP0 				= NO_MORE_DATA;
    bStatusAction 						= STATUS_ACTION_NOTHING;
}




ot_u8 usbClearEndpointFeature (void) {
    ot_u8 bEndpointNumber;

    //EP is from EP1 to EP7 while C language start from 0
    bEndpointNumber = (tSetupPacket.wIndex & EP_DESC_ADDR_EP_NUM);
    if (bEndpointNumber <= MAX_ENDPOINT_NUMBER) {
        if (bEndpointNumber != 0x00) {
        	tEDB* dblock;
            bEndpointNumber--;
            dblock = (tEDB*)dbselect[(tSetupPacket.wIndex&EP_DESC_ADDR_DIR_IN)!=EP_DESC_ADDR_DIR_IN];
            dblock[bEndpointNumber].bEPCNF &= ~(EPCNF_STALL | EPCNF_TOGGLE );
        }
        usbSendZeroLengthPacketOnIEP0();
    }

    return (FALSE);
}



ot_u8 sub_usbget(ot_u8 remaining_bytes, ot_u8* send_data) {
    usbClearOEP0ByteCount();                                //for status stage
    wBytesRemainingOnIEP0 = remaining_bytes;
    usbSendDataPacketOnEP0(send_data);
    return FALSE;
}


ot_u8 usbGetConfiguration (void) {
    return sub_usbget(1, (ot_u8*)&bConfigurationNumber);
}

ot_u8 usbGetDeviceDescriptor (void) {
    return sub_usbget(SIZEOF_DEVICE_DESCRIPTOR, (ot_u8*)&abromDeviceDescriptor);
}

ot_u8 usbGetConfigurationDescriptor (void) {
    return sub_usbget(sizeof(abromConfigurationDescriptorGroup), (ot_u8*)&abromConfigurationDescriptorGroup);
}




ot_u8 usbGetStringDescriptor (void) {
    ot_u16 bIndex   = 0;
    ot_u8 bVal      = (ot_u8)tSetupPacket.wValue;

#if (USB_STR_INDEX_SERNUM != 0)
    if (bVal != 0x03){
        while (bVal-- >  0x00)
        	bIndex += abromStringDescriptor[bIndex];
    }
#endif
    return sub_usbget(abromStringDescriptor[bIndex], (ot_u8*)&abromStringDescriptor[bIndex]);
}




ot_u8 usbGetInterface (void) {
    //not fully supported, return one byte, zero
    abUsbRequestReturnData[0] = 0x00;   //changed to report alternative setting byte
    abUsbRequestReturnData[1] = bInterfaceNumber;
    
    return sub_usbget(2, (ot_u8*)&abUsbRequestReturnData[0]);
}




ot_u8 usbGetDeviceStatus (void) {
    if ((abromConfigurationDescriptorGroup.abromConfigurationDescriptorGenric.mattributes &
         CFG_DESC_ATTR_SELF_POWERED) == CFG_DESC_ATTR_SELF_POWERED) {
        abUsbRequestReturnData[0] = DEVICE_STATUS_SELF_POWER;
    }

    if (bRemoteWakeup == ENABLE){
        abUsbRequestReturnData[0] |= DEVICE_STATUS_REMOTE_WAKEUP;
    }

    //Return self power status and remote wakeup status
    return sub_usbget(2, (ot_u8*)&abUsbRequestReturnData[0]);
}




ot_u8 usbGetInterfaceStatus(void) {
    //check bIndexL for index number (not supported)
    abUsbRequestReturnData[0] = 0x00;   //changed to support multiple interfaces
    abUsbRequestReturnData[1] = bInterfaceNumber;
    
    return sub_usbget(2, (ot_u8*)&abUsbRequestReturnData[0]);
}




ot_u8 usbGetEndpointStatus (void) {
    ot_u8 bEndpointNumber;

    //Endpoint number is bIndexL
    bEndpointNumber = tSetupPacket.wIndex & EP_DESC_ADDR_EP_NUM;
    if (bEndpointNumber == 0x00) {
        ot_u8* db0CNFG;
        db0CNFG                     = (tSetupPacket.wIndex&EP_DESC_ADDR_DIR_IN) ? \
                                        ((ot_u8*)&tEndPoint0DescriptorBlock)+1 : \
                                        ((ot_u8*)&tEndPoint0DescriptorBlock)+3;
        abUsbRequestReturnData[0]   = *db0CNFG & EPCNF_STALL;
    }
    
    //no response if endpoint is not supported.
    else if (bEndpointNumber <= MAX_ENDPOINT_NUMBER) {
        //EP is from EP1 to EP7 while C language start from 0
        //Firmware should NOT response if specified endpoint is not supported. (charpter 8)
    	tEDB* dblock;
    	bEndpointNumber--;
    	dblock 						= (tEDB*)dbselect[(tSetupPacket.wIndex&EP_DESC_ADDR_DIR_IN)==0];
    	abUsbRequestReturnData[0] 	= dblock[bEndpointNumber].bEPCNF & EPCNF_STALL;
    }
    
    abUsbRequestReturnData[0] >>= 3; //STALL is on bit 3
    return sub_usbget(2, (ot_u8*)&abUsbRequestReturnData[0]);
}



ot_u8 usbSetAddress (void) {
    usbStallOEP0();     //control write without data stage

    //bValueL contains device address
    if (tSetupPacket.wValue < 128){
        //hardware will update the address after status stage
        //therefore, firmware can set the address now.
        USBFUNADR = tSetupPacket.wValue;
        usbSendZeroLengthPacketOnIEP0();
    }
    else {
        usbStallEndpoint0();
    }

    return (FALSE);
}




ot_u8 usbSetConfiguration (void) {
    usbStallOEP0();        //control write without data stage

    //configuration number is in bValueL
    //change the code if more than one configuration is supported
    bConfigurationNumber = tSetupPacket.wValue;
    bEnumerationStatus = bConfigurationNumber;			//added by JP

    usbSendZeroLengthPacketOnIEP0();

    // Commented-out by JP
    //if (bConfigurationNumber == 1){
    //    bEnumerationStatus = ENUMERATION_COMPLETE;                  //set device as enumerated
    //}
    //else {
    //    bEnumerationStatus = 0;                                     //device is not configured == config # is zero
    //}

    return (FALSE);
}



//Added by JP
ot_u8 sub_EvalDeviceFeature(ot_u8 set_wakeup) {
	//bValueL contains feature selector
	if (tSetupPacket.wValue == FEATURE_REMOTE_WAKEUP){
	    bRemoteWakeup = set_wakeup;
	    usbSendZeroLengthPacketOnIEP0();
	}
    else {
	    usbStallEndpoint0();
	}
	return (FALSE);
}


ot_u8 usbClearDeviceFeature (void) {
	return sub_EvalDeviceFeature(DISABLE);
}


ot_u8 usbSetDeviceFeature (void) {
	return sub_EvalDeviceFeature(ENABLE);
}



ot_u8 usbSetEndpointFeature (void) {
    ot_u8 bEndpointNumber;

    //wValue contains feature selector
    //bIndexL contains endpoint number
    //Endpoint number is in low byte of wIndex
    if (tSetupPacket.wValue == FEATURE_ENDPOINT_STALL){
        bEndpointNumber = tSetupPacket.wIndex & EP_DESC_ADDR_EP_NUM;
        if (bEndpointNumber <= MAX_ENDPOINT_NUMBER) {
            if (bEndpointNumber != 0) {
            	tEDB* dblock;
                bEndpointNumber--;
                dblock = (tEDB*)dbselect[(tSetupPacket.wIndex&EP_DESC_ADDR_DIR_IN)==0];
                dblock[bEndpointNumber].bEPCNF |= EPCNF_STALL;
            }
            usbSendZeroLengthPacketOnIEP0();    //do nothing for endpoint 0
        }
    }
    else {
        usbStallEndpoint0();
    }

    return (FALSE);
}




ot_u8 usbSetInterface (void) {
    //bValueL contains alternative setting
    //bIndexL contains interface number
    //change code if more than one interface is supported
    usbStallOEP0();                         //control write without data stage
    bInterfaceNumber = tSetupPacket.wIndex;

    usbSendZeroLengthPacketOnIEP0();

    return (FALSE);
}




ot_u8 usbInvalidRequest (void) {
    //check if setup overwrite is set
    //if set, do nothing since we might decode it wrong
    //setup packet buffer could be modified by hardware if another setup packet
    //was sent while we are deocding setup packet
    if ((USBIFG & STPOWIFG) == 0x00){
        usbStallEndpoint0();
    }

    return (FALSE);
}



typedef ot_u8 (*tpF)(void);

ot_u8 usbDecodeAndProcessUsbRequest (void) {
    ot_u8 bMask,bResult,bTemp;
    const ot_u8* pbUsbRequestList;
    ot_u8 bWakeUp = FALSE;
    ptDEVICE_REQUEST ptSetupPacket = &tSetupPacket;
    ot_u8 bRequestType,bRequest;
    tpF lAddrOfFunction;

    //point to beginning of the matrix
    pbUsbRequestList = (ot_u8*)&tUsbRequestList[0];

    while (1) {
        bRequestType = *pbUsbRequestList++;
        bRequest     = *pbUsbRequestList++;

        if (((bRequestType == 0xff) && (bRequest == 0xff)) ||
            (tSetupPacket.bmRequestType == (USB_REQ_TYPE_INPUT | USB_REQ_TYPE_VENDOR | USB_REQ_TYPE_DEVICE)) ||
            (tSetupPacket.bmRequestType == (USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_VENDOR | USB_REQ_TYPE_DEVICE)))
        {
            pbUsbRequestList -= 2;
            break;
        }

        if ((bRequestType == tSetupPacket.bmRequestType) && (bRequest == tSetupPacket.bRequest)) {
            //compare the first two
            bResult = 0xc0;
            bMask   = 0x20;
            //first two bytes matched, compare the rest
            for (bTemp = 2; bTemp < 8; bTemp++) {
                if (*((ot_u8*)ptSetupPacket + bTemp) == *pbUsbRequestList) {
                    bResult |= bMask;
                }
                pbUsbRequestList++;
                bMask = bMask >> 1;
            }
            //now we have the result
            if ((*pbUsbRequestList & bResult) == *pbUsbRequestList){
                pbUsbRequestList -= 8;
                break;
            }
            else {
                pbUsbRequestList += (sizeof(tDEVICE_REQUEST_COMPARE) - 8);
            }
        }
        else {
            pbUsbRequestList += (sizeof(tDEVICE_REQUEST_COMPARE) - 2);
        }
    }

    //if another setup packet comes before we have the chance to process current
    //setup request, we return here without processing the request
    //this check is not necessary but still kept here to reduce response(or simulation) time

    if ((USBIFG & STPOWIFG) != 0x00){
        //return (bWakeUp);
    	return False;
    }

    //now we found the match and jump to the function accordingly.
    lAddrOfFunction = ((tDEVICE_REQUEST_COMPARE*)pbUsbRequestList)->pUsbFunction;

    //call function
    bWakeUp = (*lAddrOfFunction)();

    //perform enumeration complete event:
    //when SetAddress was called and USBADDR is not zero
    if ((lAddrOfFunction == &usbSetAddress) && (USBFUNADR != 0)){
        //bWakeUp = USB_handleEnumCompleteEvent();
    	USB_handleEnumCompleteEvent();
    }
    //return (bWakeUp);
    return False;
}




#endif

