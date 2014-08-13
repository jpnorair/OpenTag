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
  * @file       /otplatform/msp430f5/usb_cdc_driver/defMSP430USB.h
  * @author     RSTO, JP Norair
  * @version    R100
  * @date       1 Nov 2012
  * @brief      USB Module HW Constants, Macros, Register Definitions
  * @ingroup    MSP430F5 USB CDC
  *
  * This module is a derivative work of TI's USB library file, defMSP430USB.h.
  * JP has integrated it into OpenTag.  
  * 
  * <PRE>
  * Author: RSTO
  * Source: defMSP430USB.h, File Version 1.00 2009/12/03  
  *
  * WHO         WHEN        WHAT
  * ---         ----------  ------------------------------------------------
  * RSTO        2008/09/03  born
  * MSP,Biju    2009/10/21  Changes for composite support
  * MSP,Biju    2009/12/03  file versioning started
  * </PRE>                           
  *****************************************************************************/

#ifndef _defMSP430USB_H
#define _defMSP430USB_H

#include "usb_cdc_driver/usb_types.h"

/*----------------------------------------------------------------------------+
 | Constant Definitions                                                        |
 +----------------------------------------------------------------------------*/
#define YES         1
#define NO          0

#ifndef TRUE
#define TRUE        True
#endif
#ifndef FALSE
#define FALSE       False
#endif

#define NOERR       0
#define ERR         1

#define NO_ERROR    0
#define ERROR       1

#ifndef DISABLE
#define DISABLE     0
#endif
#ifndef ENABLE
#define ENABLE      1
#endif


/*----------------------------------------------------------------------------+
 | USB Constants, Type Definition & Macro                                      |
 +----------------------------------------------------------------------------*/

//USB related Constant
#define MAX_ENDPOINT_NUMBER     0x07    //A maximum of 7 endpoints is available
#define EP0_MAX_PACKET_SIZE     0x08
#define EP0_PACKET_SIZE         0x08
#define EP_MAX_PACKET_SIZE      0x40

//Base addresses of transmit and receive buffers


//Endpoint 1 Buffers
#define OEP1_X_BUFFER_ADDRESS   0x1C00
#define OEP1_Y_BUFFER_ADDRESS   (OEP1_X_BUFFER_ADDRESS+MAX_IRQ_PKT)
#define IEP1_X_BUFFER_ADDRESS   (OEP1_Y_BUFFER_ADDRESS+MAX_IRQ_PKT)
#define IEP1_Y_BUFFER_ADDRESS   (IEP1_X_BUFFER_ADDRESS+MAX_IRQ_PKT)
#define EP1_BUFFER_END          (IEP1_Y_BUFFER_ADDRESS+MAX_IRQ_PKT)

//Endpoint 2 Buffers
//1D00 for 64 byte double buffer
//2000 for 256 byte double buffer (or 512 byte single buffer)
#define OEP2_X_BUFFER_ADDRESS   EP1_BUFFER_END
#define OEP2_Y_BUFFER_ADDRESS   (OEP1_X_BUFFER_ADDRESS+MAX_IRQ_PKT)
#define IEP2_X_BUFFER_ADDRESS   (OEP1_Y_BUFFER_ADDRESS+MAX_IRQ_PKT)
#define IEP2_Y_BUFFER_ADDRESS   (IEP1_X_BUFFER_ADDRESS+MAX_IRQ_PKT)
#define EP2_BUFFER_END          (IEP1_Y_BUFFER_ADDRESS+MAX_IRQ_PKT)

//Endpoint 3 Buffers
#define OEP3_X_BUFFER_ADDRESS   (EP2_BUFFER_END)
#define IEP3_X_BUFFER_ADDRESS   (OEP3_X_BUFFER_ADDRESS+MAX_DATA_PKT)
#define OEP3_Y_BUFFER_ADDRESS   (IEP3_X_BUFFER_ADDRESS+MAX_DATA_PKT)
#define IEP3_Y_BUFFER_ADDRESS   (OEP3_Y_BUFFER_ADDRESS+MAX_DATA_PKT)
#define EP3_BUFFER_END          (IEP3_Y_BUFFER_ADDRESS+MAX_DATA_PKT)

//Endpoint 4 Buffers
#define OEP4_X_BUFFER_ADDRESS   (EP3_BUFFER_END)
#define IEP4_X_BUFFER_ADDRESS   (OEP4_X_BUFFER_ADDRESS+MAX_DATA_PKT)
#define OEP4_Y_BUFFER_ADDRESS   (IEP4_X_BUFFER_ADDRESS+MAX_DATA_PKT)
#define IEP4_Y_BUFFER_ADDRESS   (OEP4_Y_BUFFER_ADDRESS+MAX_DATA_PKT)
#define EP4_BUFFER_END          (IEP4_Y_BUFFER_ADDRESS+MAX_DATA_PKT)

//Endpoint 5 Buffers
#define OEP5_X_BUFFER_ADDRESS   (EP4_BUFFER_END)
#define IEP5_X_BUFFER_ADDRESS   (OEP5_X_BUFFER_ADDRESS+MAX_DATA_PKT)
#define OEP5_Y_BUFFER_ADDRESS   (IEP5_X_BUFFER_ADDRESS+MAX_DATA_PKT)
#define IEP5_Y_BUFFER_ADDRESS   (OEP5_Y_BUFFER_ADDRESS+MAX_DATA_PKT)
#define EP5_BUFFER_END          (IEP5_Y_BUFFER_ADDRESS+MAX_DATA_PKT)

//Endpoint 6 Buffers
#define OEP6_X_BUFFER_ADDRESS   (EP5_BUFFER_END)
#define IEP6_X_BUFFER_ADDRESS   (OEP6_X_BUFFER_ADDRESS+MAX_DATA_PKT)
#define OEP6_Y_BUFFER_ADDRESS   (IEP6_X_BUFFER_ADDRESS+MAX_DATA_PKT)
#define IEP6_Y_BUFFER_ADDRESS   (OEP6_Y_BUFFER_ADDRESS+MAX_DATA_PKT)
#define EP6_BUFFER_END          (IEP6_Y_BUFFER_ADDRESS+MAX_DATA_PKT)

//Endpoint 7 Buffers
#define OEP7_X_BUFFER_ADDRESS   (EP6_BUFFER_END)
#define IEP7_X_BUFFER_ADDRESS   (OEP7_X_BUFFER_ADDRESS+MAX_DATA_PKT)
#define OEP7_Y_BUFFER_ADDRESS   (IEP7_X_BUFFER_ADDRESS+MAX_DATA_PKT)
#define IEP7_Y_BUFFER_ADDRESS   (OEP7_Y_BUFFER_ADDRESS+MAX_DATA_PKT)
#define EP7_BUFFER_END          (IEP7_Y_BUFFER_ADDRESS+MAX_DATA_PKT)

#define X_BUFFER 0
#define Y_BUFFER 1

//Macros for end point numbers
#define EP1 1
#define EP2 2
#define EP3 3
#define EP4 4
#define EP5 5
#define EP6 6
#define EP7 7

//addresses of pipes for endpoints
#define EP1_OUT_ADDR          0x01      //address for endpoint 1
#define EP2_OUT_ADDR          0x02      //address for endpoint 2
#define EP3_OUT_ADDR          0x03      //address for endpoint 3
#define EP4_OUT_ADDR          0x04      //address for endpoint 4
#define EP5_OUT_ADDR          0x05      //address for endpoint 5
#define EP6_OUT_ADDR          0x06      //address for endpoint 6
#define EP7_OUT_ADDR          0x07      //address for endpoint 7

//Input end points
#define EP1_IN_ADDR          0x81       //address for endpoint 1
#define EP2_IN_ADDR          0x82       //address for endpoint 2
#define EP3_IN_ADDR          0x83       //address for endpoint 3
#define EP4_IN_ADDR          0x84       //address for endpoint 4
#define EP5_IN_ADDR          0x85       //address for endpoint 5
#define EP6_IN_ADDR          0x86       //address for endpoint 6
#define EP7_IN_ADDR          0x87       //address for endpoint 7


//EDB Data Structure
#define EDB_X_INDEX     1
#define EDB_Y_INDEX     5
#define EDB_Y_OFFSET    4

typedef struct _tEDB {
    ot_u8 bEPCNF;                        //Endpoint Configuration
    ot_u8 bEPBBAX;                       //Endpoint X Buffer Base Address
    ot_u8 bEPBCTX;                       //Endpoint X Buffer byte Count
    ot_u8 bSPARE0;                       //not used
    ot_u8 bSPARE1;                       //not used
    ot_u8 bEPBBAY;                       //Endpoint Y Buffer Base Address
    ot_u8 bEPBCTY;                       //Endpoint Y Buffer byte Count
    ot_u8 bEPSIZXY;                      //Endpoint XY Buffer Size
} tEDB, *tpEDB;



typedef struct _tEDB0 {
    ot_u8 bIEPCNFG;                      //Input Endpoint 0 Configuration Register
    ot_u8 bIEPBCNT;                      //Input Endpoint 0 Buffer Byte Count
    ot_u8 bOEPCNFG;                      //Output Endpoint 0 Configuration Register
    ot_u8 bOEPBCNT;                      //Output Endpoint 0 Buffer Byte Count
} tEDB0, *tpEDB0;




//EndPoint Desciptor Block Bits
#define EPCNF_USBIE     0x04            //USB Interrupt on Transaction Completion. Set By MCU
                                        //0:No Interrupt, 1:Interrupt on completion
#define EPCNF_STALL     0x08            //USB Stall Condition Indication. Set by UBM
                                        //0: No Stall, 1:USB Install Condition
#define EPCNF_DBUF      0x10            //Double Buffer Enable. Set by MCU
                                        //0: Primary Buffer Only(x-buffer only), 1:Toggle Bit Selects Buffer

#define EPCNF_TOGGLE     0x20           //USB Toggle bit. This bit reflects the toggle sequence bit of DATA0 and DATA1.

#define EPCNF_UBME      0x80            //UBM Enable or Disable bit. Set or Clear by MCU.
                                        //0:UBM can't use this endpoint
                                        //1:UBM can use this endpoint
#define EPBCNT_BYTECNT_MASK 0x7F        //MASK for Buffer Byte Count
#define EPBCNT_NAK       0x80           //NAK, 0:No Valid in buffer, 1:Valid packet in buffer

//definitions for MSP430 USB-module
#define START_OF_USB_BUFFER   0x1C00

//input and output buffers for EP0
#define USBIEP0BUF 0x2378
#define USBOEP0BUF 0x2370


#endif 
