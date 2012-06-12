//(c)2009 by Texas Instruments Incorporated, All Rights Reserved.
/*----------------------------------------------------------------------------+
 |                                                                             |
 |                              Texas Instruments                              |
 |                                                                             |
 |                          MSP430 USB-Example (CDC/HID Driver)                |
 |                                                                             |
 +-----------------------------------------------------------------------------+
 |  Source: UsbIsr.h, File Version 1.00 2009/12/03                             |
 |  Author: RSTO                                                               |
 |                                                                             |
 |  WHO          WHEN         WHAT                                             |
 |  ---          ----------   ------------------------------------------------ |
 |  RSTO         2008/09/03   born                                             |
 |  RSTO         2008/12/23   enhancements of CDC API                          |
 +----------------------------------------------------------------------------*/

#ifndef _ISR_H_
#define _ISR_H_

#include "usb_cdc_driver/usb_types.h"

/**
 * Handle incoming setup packet.
 * returns TRUE to keep CPU awake
 */
ot_u8 SetupPacketInterruptHandler(void);

/**
 * Handle VBuss on signal.
 */
void PWRVBUSonHandler(void);

/**
 * Handle VBuss off signal.
 */
void PWRVBUSoffHandler(void);

/**
 * Handle In-requests from control pipe.
 */
void IEP0InterruptHandler(void);

/**
 * Handle Out-requests from control pipe.
 */
ot_u8 OEP0InterruptHandler(void);

/*----------------------------------------------------------------------------+
 | End of header file                                                          |
 +----------------------------------------------------------------------------*/

#endif  /* _ISR_H_ */

/*------------------------ Nothing Below This Line --------------------------*/
