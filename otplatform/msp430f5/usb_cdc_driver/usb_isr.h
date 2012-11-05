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
  * @file       /otplatform/msp430f5/usb_cdc_driver/usb_isr.h
  * @author     RSTO, JP Norair
  * @version    R100
  * @date       1 Nov 2012
  * @brief      USB Module ISR Interface
  * @ingroup    MSP430F5 USB CDC
  *
  * This module is a derivative work of TI's USB library file, UsbIsr.h.
  * JP has integrated it into OpenTag and heavily optimized the code for a
  * CDC-only USB system.
  * 
  * <PRE>
  * Author: RSTO
  * Source: UsbIsr.h, File Version 1.00 2009/12/03   
  *
  * WHO         WHEN        WHAT
  * ---         ----------  ------------------------------------------------
  * RSTO        2008/09/03  born
  * RSTO        2008/12/23  enhancements of CDC API
  * ---         ----------  ------------------------------------------------
  * JPN         2012/05/01  Integrated with OpenTag 0.3
  * JPN         2012/11/01  Integrated with OpenTag 0.4
  * </PRE>                           
  *****************************************************************************/

#ifndef _USB_ISR_H_
#define _USB_ISR_H_

#include "usb_cdc_driver/usb_types.h"


extern ot_u8  bFunctionSuspended;
//extern __no_init tEDB0 __data16 tEndPoint0DescriptorBlock;
//extern __no_init tEDB __data16 tInputEndPointDescriptorBlock[];
//extern __no_init tEDB __data16 tOutputEndPointDescriptorBlock[];
extern volatile ot_u8 bHostAsksUSBData;
extern volatile ot_u8 bTransferInProgress;
extern volatile ot_u8 bSecondUartTxDataCounter[];
extern volatile ot_u8* pbSecondUartTxData;
extern ot_u8 bStatusAction;
//extern ot_u16 wUsbEventMask;
//ot_bool CdcToHostFromBuffer(ot_u8);
//ot_bool CdcToBufferFromHost(ot_u8);
//ot_bool CdcIsReceiveInProgress(ot_u8);
//extern ot_u16 wUsbHidEventMask;




/** Handle incoming setup packet.
  * returns TRUE to wakeup CPU
  */
ot_u8 SetupPacketInterruptHandler(void);



/** Handle VBuss on signal.
  */
void PWRVBUSonHandler(void);



/** Handle VBus off signal.
  */
void PWRVBUSoffHandler(void);



/** Handle In-requests from control pipe.
  */
void IEP0InterruptHandler(void);



/** Handle Out-requests from control pipe.
  */
ot_u8 OEP0InterruptHandler(void);



#endif
