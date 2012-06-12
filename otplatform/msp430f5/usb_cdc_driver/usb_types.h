//(c)2009 by Texas Instruments Incorporated, All Rights Reserved.
/*----------------------------------------------------------------------------+
 |                                                                             |
 |                              Texas Instruments                              |
 |                                                                             |
 |                          MSP430 USB-Example (CDC/HID Driver)                |
 |                                                                             |
 +-----------------------------------------------------------------------------+
 |  Source: types.h, File Version 1.00 2009/12/03                              |
 |  Author: RSTO                                                               |
 |                                                                             |
 |  WHO          WHEN         WHAT                                             |
 |  ---          ----------   ------------------------------------------------ |
 |  RSTO         2008/09/03   born                                             |
 |  RSTO         2009/07/17   Define __data16 for CCS                          |
 |                                                                             |
 +----------------------------------------------------------------------------*/
#ifndef _TYPES_H_
#define _TYPES_H_

#if (defined(__CCSv4__) || defined(__CCSv5__) || defined(__TI_COMPILER_VERSION__))
#define __no_init
#define __data16
#endif

#include "OT_types.h"
#include "OT_platform.h"

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
} tDEVICE_REQUEST, *ptDEVICE_REQUEST;

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
    ot_u8 (*pUsbFunction)(VOID); //function pointer
} 
tDEVICE_REQUEST_COMPARE, *ptDEVICE_REQUEST_COMPARE;

//----------------------------------------------------------------------------
typedef enum {
    STATUS_ACTION_NOTHING,
    STATUS_ACTION_DATA_IN,
    STATUS_ACTION_DATA_OUT
} tSTATUS_ACTION_LIST;


#define tSTATUS_EN_DISABLED		FunctionalState


#endif
