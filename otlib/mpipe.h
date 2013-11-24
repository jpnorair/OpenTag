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
  * @file       /otlib/mpipe.h
  * @author     JP Norair
  * @version    R100
  * @date       24 Oct 2012
  * @brief      Message Pipe (MPIPE) interface
  * @defgroup   MPipe (Message Pipe)
  * @ingroup    MPipe
  *
  * The Message Pipe is used to traffic data between a server and client.  This
  * present implementation always uses NDEF as a data wrapping format, and it 
  * typically uses a wireline interface such as serial or USB CDC.
  *
  * Mpipe is a message interface, not a function-based API.  The functions in
  * the MPipe module perform MAC and I/O type features.  However, the MPipe
  * Payload is tightly coupled to NDEF, and, in OpenTag, NDEF is tightly 
  * coupled to the ALP interface.  Furthermore, the ALP interface is available
  * for use as a functional API.  So, in practical terms, MPipe's purpose is as
  * a messaging API between client (e.g. PC) and server (OpenTag DASH7 device).
  *
  * To configure MPipe, alter the board configuration header file.  These are
  * found in the /board directory.  Different boards support different MPipe
  * interfaces.
  * 
  * @note Mpipe Protocol
  * The Mpipe Protocol is completely implementation dependent.  Interfaces that
  * do not include an integrated MAC layer (addressing and data integrity) will
  * probably require some additional data to be sent as protocol overhead. 
  * These features should be transparent to the user of Mpipe.
  ******************************************************************************
  */

#ifndef __MPIPE_H
#define __MPIPE_H

#include "OT_types.h"
#include "OT_config.h"

#if (OT_FEATURE(MPIPE) == ENABLED)

#include "alp.h"
#include "system.h"



///@todo when more hardware is supported by mpipe, variations of this will be
///      specified.  In certain implementations, this is superfluous
typedef enum {
    MPIPE_9600bps    = MCU_PARAM(UART_9600BPS),
    MPIPE_28800bps   = MCU_PARAM(UART_28800BPS),
    MPIPE_57600bps   = MCU_PARAM(UART_57600BPS), 
    MPIPE_115200bps  = MCU_PARAM(UART_115200BPS),
    MPIPE_250000bps  = MCU_PARAM(UART_250000BPS),
    MPIPE_500000bps  = MCU_PARAM(UART_500000BPS)
} mpipe_speed;


///@note Priority might be altered in future implementations.
typedef enum {
    MPIPE_Low       = 0,
    MPIPE_High      = 1,
    MPIPE_Broadcast = 2,
    MPIPE_Ack       = 3
} mpipe_priority;

//Definitions to support legacy code (deprecated)
#define lo_priority ((mpipe_priority)0)
#define hi_priority ((mpipe_priority)1)


///@note States might be altered in future implementations.
typedef enum {
    MPIPE_Null          = -1,
    MPIPE_Idle          = 0,
    MPIPE_RxHeader      = 1,
    MPIPE_RxPayload     = 2,
    MPIPE_TxAck_Wait    = 3,
    MPIPE_TxAck_Done    = 4,
    MPIPE_Tx_Wait       = 5,
    MPIPE_Tx_Done       = 6,
    MPIPE_RxAck         = 7,
    MPIPE_RxAckHeader   = 8
} mpipe_state;




/** MPipe data allocation         <BR>
  * ========================================================================<BR>
  * The MPipe implementation requires an ALP template for input and output, and
  * the framing of queued IO data is managed via the ALP Module.
  *
  * MPipe also includes a 16bit sequence number that is used typically in the
  * footer of each message.
  */

typedef struct {
    volatile mpipe_state state;
    alp_tmpl    alp;
    
#if (OT_FEATURE(MPIPE_CALLBACKS) == ENABLED)
    ot_sigv sig_rxdone;
    ot_sigv sig_txdone;
    ot_sigv sig_rxdetect;
#endif
} mpipe_struct;

extern mpipe_struct mpipe;





/** Library Functions 
  * ========================================================================<BR>
  */
/** @brief  Initializes & connects Mpipe Module
  * @param  port_id     (void*) Implementation-dependent port identifier 
  * @retval None        
  * @ingroup Mpipe
  * @sa mpipe_disconnect()
  * @sa mpipedrv_init()
  *
  * The user should call this function during system initialization, and the
  * implementation must call mpipedrv_init().
  *
  * About port_id: on POSIX systems this will be a string representing a file
  * in the /dev/ directory somewhere (often /dev/tty...).  On embedded sytems,
  * it might be a pointer to a peripheral configuration register bank.  Check
  * with the Platform layer implementation -- in many cases it is unused and can
  * be ignored (set to NULL).
  */
void mpipe_connect(void* port_id);



/** @brief  Disconnects Mpipe Module
  * @param  port_id     (void*) Implementation-dependent port identifier 
  * @retval None        
  * @ingroup Mpipe
  * @sa mpipe_connect()
  * @sa mpipedrv_detach()
  *
  * Some drivers may be essentially connectionless -- e.g. an embedded UART.
  * USB Mpipes usually need to be disconnected, though, when in fact they are
  * physically disconnected.
  */
void mpipe_disconnect(void* port_id);



/** @brief  Returns the Mpipe state: merely a wrapper for mpipe.state
  * @param  None
  * @retval (mpipe_state)   State enum
  * @ingroup Mpipe
  *
  * This is a wrapper for folks who like that sort of thing.
  */
mpipe_state mpipe_status();


/** @brief  Opens the Mpipe for passive RX
  * @param  None
  * @retval None
  * @ingroup Mpipe
  * @sa mpipe_close()
  *
  * This function is usually called only once, by mpipe_connect().  However, if
  * mpipe is closed, then you will have to call it again to re-open mpipe.
  */
void mpipe_open();


/** @brief  Closes the Mpipe, terminating any present usage
  * @param  None
  * @retval None
  * @ingroup Mpipe
  * @sa mpipe_open()
  *
  * You may call this function to close the Mpipe.  From a tasking perspective 
  * this has the effect of blocking Mpipe.
  */
void mpipe_close();


/** @brief  Sends the contents of the mpipe output queue over the MPipe
  * @param  None
  * @retval None
  * @ingroup Mpipe
  */
void mpipe_send();



/** @brief  Mpipe Task function
  * @param  task    (task_marker*) Task pointer from the kernel
  * @retval None
  * @ingroup Mpipe
  *
  * The best policy is not to call this function, ever, unless you are working
  * on the kernel.
  */
void mpipe_systask(ot_task task);





/** Driver->Task Callbacks (Static) <BR>
  * ========================================================================<BR>
  */


/** @brief  Driver callback for RX Initialization Event
  * @param  code    (ot_int) Number of ticks before kernel calls init procedure
  * @retval None
  * @ingroup Mpipe
  */
void mpipeevt_rxinit(ot_int code);


/** @brief  Driver callback for RX Detect Event
  * @param  code    (ot_int) Ticks (typ 0-255) the task should apply as timeout
  * @retval None
  * @ingroup Mpipe
  *
  * Once an mpipe driver detects an incoming packet, it will use this callback.
  * The "code" value will be applied to a timeout measured by the task.  If the
  * packet is not finished being received by this timeout, MPipe is flushed.
  * 
  * The timeout value itself is in Ticks, and in typical implementations it is
  * just provided as a fixed value.  For example, on a 115200bps 8N1 UART, 256
  * bytes require 22.7 ticks to receive.  So, for an implementation with a 256
  * byte MPipe frame buffer, the timeout can be safely fixed to 23 ticks.
  */
void mpipeevt_rxdetect(ot_int code);



/** @brief  Driver callback for RX Done Event
  * @param  code    (ot_int) 0 if RX'ed packet is valid, non-zero otherwise
  * @retval None
  * @ingroup Mpipe
  */
void mpipeevt_rxdone(ot_int code);



/** @brief  Driver callback for TX Done Event
  * @param  code    (ot_int) Ticks (typ 0-255) the driver needs to page-out TX
  * @retval none
  * @ingroup Mpipe
  * 
  * Some MPipe driver implementations may deliver "DONE" interrupts before the 
  * TX data has completely exited from the buffer.  The code value tells the
  * MPipe Task to wait a number of ticks before re-opening the MPipe in RX.
  * Most embedded devices using DMA will have a two byte delay (20 bits), so
  * "code" will be 1 tick for baud rates 28.8kbps or higher.
  */
void mpipeevt_txdone(ot_int code);





/** Driver Interface Functions 
  * ========================================================================<BR>
  */


/** @brief  Returns the number of bytes the MPipe needs for its footer
  * @param  None
  * @retval ot_u8       Number of bytes of the footer
  * @ingroup Mpipe
  *
  * All this does is return a constant.  It is here to prevent magic numbers
  * from being used, and to allow different MPipe implementations to have 
  * different footer sizes.
  */
ot_u8 mpipedrv_footerbytes();



/** @brief  Initializes Mpipe Driver
  * @param  port_id     (void*) Implementation-dependent port identifier 
  * @param  baud_rate   (mpipe_speed) baud rate value
  * @retval ot_int      Amount of latency to attribute to this driver
  * @ingroup Mpipe
  * @sa mpipe_connect()
  * @sa mpipedrv_detach()
  *
  * @note the baud_rate input may differ on each platform.  So, the header
  * platform_xxx.h must include MCU_PARAM(UART_xxxBPS) constants.
  * 
  * This function must be implemented in the MPipe driver.  It should be called
  * from inside mpipe_connect().
  * 
  * About port_id: this value is passed-in directly from mpipe_connect().
  *
  * About returned latency: this is a value in ticks that is used by the native
  * kernel, and perhaps others, that assists the management of task timing.
  * This value should be 1-255 for the native kernel, but for other kernels it
  * may have different outputs.
  */
ot_int mpipedrv_init(void* port_id, mpipe_speed baud_rate);



/** @brief  Holds the runtime until MPipe is up and running
  * @param  None
  * @retval None
  * @ingroup Mpipe
  *
  * This function is useful only for MPipes that require a connection.  USB is
  * an example of one of these.  Basic serial comms have no imperative
  * connection, so for UART, SPI, I2C, etc this function can be empty.
  *
  * This function should be called only when the MPipe connection is required
  * before proceeding.  For example, its intended usage is in the power-on
  * section of the main function (before the loop), where a USB connection
  * needs some time to be detected and enumerated.
  */
void mpipedrv_standby();



/** @brief  De-Initializes Mpipe Driver upon disconnection
  * @param  port_id     (void*) Implementation-dependent port identifier 
  * @retval void
  * @ingroup Mpipe
  * @sa mpipe_disconnect()
  * @sa mpipedrv_init()
  *
  * This function must be implemented in the MPipe driver.  It should be called
  * from inside mpipe_disconnect().
  * 
  * About port_id: this value is passed-in directly from mpipe_disconnect().
  */
void mpipedrv_detach(void* port_id);



/** @brief  Kills the Mpipe connection
  * @param  None 
  * @retval None
  * @ingroup Mpipe
  */
void mpipedrv_kill();


/** @brief  Clears the MPipe Driver state machine (typically sends to Idle)
  * @param  None
  * @retval None
  * @ingroup Mpipe
  */
void mpipedrv_clear();


/** @brief  Blocks MPipe from operating
  * @param  None
  * @retval None
  * @ingroup Mpipe
  * @sa mpipe_unblock()
  */
void mpipedrv_block();



/** @brief  Un-blocks MPipe, allowing operation again
  * @param  None
  * @retval None
  * @ingroup Mpipe
  * @sa mpipe_block()
  */
void mpipedrv_unblock();



/** @brief  Provides driver-level blocking to MPIPE transfers
  * @param  None 
  * @retval None
  * @ingroup Mpipe
  */
void mpipedrv_wait();




/** @brief  Transmits an NDEF structured datastream over the MPIPE
  * @param  blocking    (ot_bool) True/False for blocking/non-blocking call
  * @param  data_priority (mpipe_priority) Priority of the TX
  * @retval ot_uint     Number of ticks before TX stream timeout
  * @ingroup Mpipe
  * @sa mpipe_rxndef, mpipe_status
  *
  * The blocking parameter is sometimes unused or irrelevant, so check with the
  * documentation for the specific implementation.  When used, the blocking
  * parameter will guarantee data integrity by blocking resource use of any kind
  * during the data transfer.
  *
  * The data_priority parameter is also partially implementation dependent.  The
  * priority of an in-progress tx/rx transfer is saved in the module, so if a TX
  * is invoked while another transfer is underway, it will cancel the transfer
  * underway if its own priority is higher.
  *
  */
ot_uint mpipedrv_txndef(ot_bool blocking, mpipe_priority data_priority);





/** @brief  Receives an NDEF structed datastream over the MPIPE
  * @param  blocking    (ot_bool) True/False for blocking/non-blocking call
  * @param  data_priority (mpipe_priority) Priority of the TX
  * @retval None
  * @ingroup Mpipe
  * @sa mpipe_txndef, mpipe_status
  *
  * The blocking parameter and data_priority parameters are dealt-with in the 
  * same way as they are with mpipe_txndef()
  */
void mpipedrv_rxndef(ot_bool blocking, mpipe_priority data_priority);




/** @brief  ISR Mpipe subroutine for Mpipe inclusion into global ISR function
  * @param  None
  * @retval None
  * @ingroup Mpipe
  *
  * Check the Mpipe driver implementation for usage notes.  In cases where the
  * Mpipe resources are not included into the user's application, the Mpipe 
  * driver should manage everything related to this ISR transparently.  In cases
  * where the user app includes Mpipe resources for its own purposes, you will
  * need to call this ISR subroutine somewhere in that resource's ISR.
  *
  * Example: if the Mpipe is DMA-driven, and you use the DMA for more than just
  * the Mpipe, your outsourced DMA ISR should call this function as part of the
  * servicing of the DMA channel chosen for Mpipe.
  */
void mpipedrv_isr();



#endif

#endif
