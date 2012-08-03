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
  * @version    V1.0
  * @date       31 July 2012
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



///@todo when more hardware is supported by mpipe, variations of this will be
///      specified.  In certain implementations, this is superfluous
typedef enum {
    MPIPE_9600bps    = 0,
    MPIPE_28800bps   = 1,
    MPIPE_57600bps   = 2, 
    MPIPE_115200bps  = 3,
    MPIPE_230400bps  = 4,
    MPIPE_460800bps  = 5
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


/** MPipe ALP allocation         <BR>
  * ========================================================================<BR>
  * This should be allocated in the MPipe driver if you are using an ALP-based
  * data wrapper for MPipe.  NDEF is an ALP-based wrapper, and it is the only
  * wrapper currently supported by Mainline MPipe implementations.
  */
extern alp_tmpl mpipe_alp;





/** @brief  Returns the number of bytes the MPipe needs for its footer
  * @param  None
  * @retval ot_u8       Number of bytes of the footer
  * @ingroup Mpipe
  *
  * All this does is return a constant.  It is here to prevent magic numbers
  * from being used, and to allow different MPipe implementations to have 
  * different footer sizes.
  */
ot_u8 mpipe_footerbytes();



/** @brief  Initializes Mpipe module (run at boot time)
  * @param  port_id     (void*) Implementation-dependent port identifier 
  * @retval ot_int      0 on success, negative on error
  * @ingroup Mpipe
  *
  * About port_id: on POSIX systems this will be a string representing a file
  * in the /dev/ directory somewhere (often /dev/tty...).  On embedded sytems,
  * it might be a pointer to a peripheral configuration register bank.  Check
  * with the Platform layer implementation -- in many cases it is unused and can
  * be ignored (set to NULL).
  */
ot_int mpipe_init(void* port_id);


/** @brief  Kills the Mpipe: not always implemented
  * @param  None 
  * @retval None
  * @ingroup Mpipe
  */
void mpipe_kill();



/** @brief  Provides manual blocking to MPIPE transfers
  * @param  None 
  * @retval None
  * @ingroup Mpipe
  *
  * Puts the MCU to sleep until the transfer is complete.  This is most useful
  * for preventing subsequent calls to MPIPE from interfering with each other.
  */
void mpipe_wait();



/** @brief  Sets the baud-rate of the mpipe.
  * @param  speed       (mpipe_speed) baud rate of the pipe
  * @retval None 
  * @ingroup Mpipe
  *
  * This function sets or resets data rate controlling attributes of the Mpipe.
  * In certain Mpipe implementations, data rate is irrelevant, and for these all
  * calls to mpipe_setspeed() will do the same thing.
  */
void mpipe_setspeed(mpipe_speed speed);



/** @brief  Returns the Mpipe state
  * @param  None
  * @retval (mpipe_state)   State enum
  * @ingroup Mpipe
  *
  * This is similar to a call to a mutex flag.  In Mpipe implementations that 
  * are blocking, this function will always return non-zero (because it can't be
  * called while Mpipe is active).  In Non-blocking implementations of Mpipe, it 
  * should be used to prevent usage of OpenTag features that depend on Mpipe 
  * (e.g. basically everything)
  */
mpipe_state mpipe_status();


#if (OT_FEATURE(MPIPE_CALLBACKS) == ENABLED)
/** @brief  Attaches a callback for TX done
  * @param  signal      (ot_sigv)  Signal handler callback
  * @retval None
  * @ingroup Mpipe
  * 
  * @note Only available when OT_FEATURE_MPIPE_CALLBACKS is ENABLED in OT_config.h
  */
void mpipe_setsig_txdone(ot_sigv signal);



/** @brief  Attaches a callback for RX done
  * @param  signal      (ot_sigv)  Signal handler callback
  * @retval None
  * @ingroup Mpipe
  *
  * @note Only available when OT_FEATURE_MPIPE_CALLBACKS is ENABLED in OT_config.h
  */
void mpipe_setsig_rxdone(ot_sigv signal);



/** @brief  Attaches a callback for RX detect
  * @param  signal      (ot_sigv)  Signal handler callback
  * @retval None
  * @ingroup Mpipe
  * 
  * @note Only available when OT_FEATURE_MPIPE_CALLBACKS is ENABLED in OT_config.h
  */
void mpipe_setsig_rxdetect(ot_sigv signal);

#else

/** @brief  Static Callback Functions
  * @param  code      	(void*) pointer to some sort of template (extensible)
  * @retval None
  * @ingroup Mpipe
  *
  * <LI>mpipe_sig_txdone(ot_int code)</LI>
  * <LI>mpipe_sig_rxdone(ot_int code)</LI>
  * <LI>mpipe_sig_rxdetect(ot_int code)</LI>
  *
  * Static callbacks are not used through function pointers (callback pointers)
  * they are used by direct implementation in the client app.  In other words,
  * you would implement these functions in your app, and mpipe will call them
  * as needed.  In order to have Mpipe call them, you need to define a constant
  * like the following (for each static callback you use):
  * #define EXTF_mpipe_sig_xxx
  *
  * "xxx" will be txdone, rxdone, or rxdetect.  Define one for each callback
  * you want to implement.
  */
void mpipe_sig_txdone(void* tmpl);
void mpipe_sig_rxdone(void* tmpl);
void mpipe_sig_rxdetect(void* tmpl);

#endif


/** @brief  Transmits an NDEF structured datastream over the MPIPE
  * @param  data        (ot_u8*) Pointer to start of NDEF stream
  * @param  blocking    (ot_bool) True/False for blocking/non-blocking call
  * @param  data_priority (mpipe_priority) Priority of the TX
  * @retval None
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
void mpipe_txndef(ot_u8* data, ot_bool blocking, mpipe_priority data_priority);





/** @brief  Receives an NDEF structed datastream over the MPIPE
  * @param  data        (ot_u8*) Byte array to place received data
  * @param  blocking    (ot_bool) True/False for blocking/non-blocking call
  * @param  data_priority (mpipe_priority) Priority of the TX
  * @retval None
  * @ingroup Mpipe
  * @sa mpipe_txndef, mpipe_status
  *
  * The blocking parameter and data_priority parameters are dealt-with in the 
  * same way as they are with mpipe_txndef()
  */
void mpipe_rxndef(ot_u8* data, ot_bool blocking, mpipe_priority data_priority);




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
void mpipe_isr();



#endif

#endif
