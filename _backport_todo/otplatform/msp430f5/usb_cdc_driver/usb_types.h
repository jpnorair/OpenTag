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
  * @file       /otplatform/msp430f5/usb_cdc_driver/usb_types.h
  * @author     RSTO, JP Norair
  * @version    R100
  * @date       1 Nov 2012
  * @brief      USB Driver Types
  * @ingroup    MSP430F5 USB CDC
  *
  * This module is a derivative work of TI's USB library file, types.h.
  * JP has integrated it into OpenTag.
  *
  * <PRE>
  * Author: RSTO
  * Source: types.h, File Version 1.00 2009/12/03
  *
  * WHO         WHEN        WHAT
  * ---         ----------  ---------------------------------------------
  * RSTO        2008/09/03  born
  * RSTO        2009/07/17  Define __data16 for CCS
  * ---         ----------  ---------------------------------------------
  * JPN         2012/05/01  Integrated with OpenTag 0.3
  * JPN         2012/11/01  Integrated with OpenTag 0.4
  * </PRE>
  *****************************************************************************/

#ifndef _TYPES_H_
#define _TYPES_H_

#if (defined(__CCSv4__) || defined(__CCSv5__) || defined(__TI_COMPILER_VERSION__))
#define __no_init
#define __data16
#endif

#include <otsys/types.h>
#include <otplatform.h>

/*----------------------------------------------------------------------------+
 | Include files                                                               |
 +----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
 | Function Prototype                                                          |
 +----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
 | Type Definition & Macro                                                     |
 +----------------------------------------------------------------------------*/
typedef char CHAR;
typedef unsigned char UCHAR;
typedef int INT;
typedef unsigned int UINT;
typedef short SHORT;
typedef unsigned short USHORT;
typedef long LONG;
typedef unsigned long ULONG;
typedef void VOID;
typedef unsigned long HANDLE;
typedef char *          PSTR;
typedef int BOOL;
typedef double DOUBLE;
typedef unsigned char BYTE;
typedef unsigned char*  PBYTE;
typedef unsigned int WORD;
typedef unsigned long DWORD;
typedef unsigned long*  PDWORD;

#define SUCCESS 0
#define FAILURE 1
#define VOID void

//DEVICE_REQUEST Structure
typedef struct _tDEVICE_REQUEST {
    ot_u8 bmRequestType;         //See bit definitions below
    ot_u8 bRequest;              //See value definitions below
    ot_u16 wValue;                //Meaning varies with request type
    ot_u16 wIndex;                //Meaning varies with request type
    ot_u16 wLength;               //Number of bytes of data to transfer
} usbreq_struct;    //tDEVICE_REQUEST, *ptDEVICE_REQUEST;

typedef struct _tDEVICE_REQUEST_COMPARE {
    ot_u8 bmRequestType;         //See bit definitions below
    ot_u8 bRequest;              //See value definitions below
    ot_u8 bValueL;               //Meaning varies with request type
    ot_u8 bValueH;               //Meaning varies with request type
    ot_u8 bIndexL;               //Meaning varies with request type
    ot_u8 bIndexH;               //Meaning varies with request type
    ot_u8 bLengthL;              //Number of bytes of data to transfer (LSByte)
    ot_u8 bLengthH;              //Number of bytes of data to transfer (MSByte)
    ot_u8 bCompareMask;          //MSB is bRequest, if set 1, bRequest should be matched
    void  (*pUsbFunction)(VOID); //function pointer
} usbcmd_struct;
//tDEVICE_REQUEST_COMPARE, *ptDEVICE_REQUEST_COMPARE;

//----------------------------------------------------------------------------
typedef enum {
    ACTION_nothing  = 0,
    ACTION_data_in  = 1,
    ACTION_data_out = 2
} ACTION_index;


#define tSTATUS_EN_DISABLED		FunctionalState

#define CMD_RETURN  void

#endif
