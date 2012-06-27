/* Copyright 2010-2011 JP Norair
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
  * @file       /OTlib/session.h
  * @author     JP Norair
  * @version    V1.0
  * @date       6 Mar 2011
  * @brief      ISO 18000-7.4 Session Framework
  * @defgroup   Session (Session Layer)
  * @ingroup    Session
  *
  * Session Management in the OpenTag implementation of Made 2 is, to a large 
  * extent, glue that holds together.  It includes the basic session layer
  * requirements of the spec in addition to the storage of some other, session
  * oriented parameters that need to be passed between layers.
  * 
  * The session module implements a session stack.  The stack is sorted when a
  * new session is inserted.  The implementation of the stack itself is in the
  * session.c file, and it is completely abstracted in case you want to do 
  * something more complex than the really basic stack implementation that is
  * currently used.  The current implementation is good for systems where there
  * are fewer that four simultaneous sessions permitted, which is actually more
  * than a lot of devices will ever need -- for many devices, one session is all
  * that will ever be used at any given time.
  ******************************************************************************
  */


#ifndef __SESSION_h
#define __SESSION_h

#include "OT_types.h"
#include "OT_config.h"




/** Mode 2 Session Struct
  * The session struct is a 12 byte message container that is passed between
  * many layers
  * 
  * counter     (ot_uint) A number of ti until the session is scheduled to
  *             begin.  This is, in fact, an event timeout.
  *
  * netstate    (ot_u8) A code representing what kind of session is underway,
  *             as well as a network association state.
  *
  * channel     (ot_u8) primary channel ID that the session is using.
  *
  * subnet      (ot_u8) This is the subnet value of the session.  It is loaded
  *             when a session becomes associated.
  *
  * dialog_id   (ot_u8) This is the dialog value of the session, which is loaded
  *             when a session is associated.  It is a random number that needs
  *             to be provided if instantiating the session, or loaded from the
  *             request if you are attaching to the session.
  *
  * flags       (ot_u8) Protocol Flags viable to the session.  These are aligned
  *             with the flag values from M2NP/M2QP, but they are generic enough
  *             that they could be used for other dialog-based protocols, too.
  */

/** Session Network State Flags
  * Used in the m2session.netstate container for the unassoc-synced-
  * connected-assoc state transition.  The secondary purpose is for other types 
  * of state transitions.
  */


#define M2_NETSTATE_TMASK           0x70
#define M2_NETSTATE_REQTX           0x00
#define M2_NETSTATE_REQRX           0x20            // set after valid request received
#define M2_NETSTATE_RESP            0x10
#define M2_NETSTATE_RESPTX          0x10
#define M2_NETSTATE_RESPRX          0x30
#define M2_NETFLAG_FLOOD            0x40
#define M2_NETFLAG_SCRAP            0x80

#define M2_NETSTATE_SMASK           0x03
#define M2_NETSTATE_INIT            0x08
#define M2_NETSTATE_DSDIALOG        0x04
#define M2_NETSTATE_UNASSOC         0x00
//#define M2_NETSTATE_SYNCED          0x01            // set after valid synchronizer received
#define M2_NETSTATE_CONNECTED       0x01            // set after connecting to session
#define M2_NETSTATE_ASSOCIATED      0x03            // Connected, plus uses First-RX mode
#define M2_NETFLAG_FIRSTRX          0x02            // Stops receiving after first RX found.


/** Session power configuration
  * Used in the m2session.cs_rssi, .cca_rssi, .tx_eirp containers to 
  * move power configurations between layers.
  */
#define M2_RSSI_AUTOSCALE           b10000000
#define M2_RSSI_MASK                b00011111
#define M2_RSSI_DBM(VAL)            (ot_int)(-140 + ((VAL)*3))
#define M2_EIRP_AUTOSCALE           b10000000
#define M2_EIRP_MASK                b01111111
#define M2_EIRP_DBM(VAL)            (ot_int)(-40 + ((VAL)>>1))
#define M2_EIRP_0p5DBM(VAL)         (ot_int)(-80 + (VAL))


/** Protocols (Translated)
  */
#define M2_PROTOCOL_M2NP            0x0
#define M2_PROTOCOL_M2QP            0x0
#define M2_PROTOCOL_M2DP            0x2
#define M2_PROTOCOL_M2ADVP          0xF0
#define M2_PROTOCOL_M2RESP          0xF1



/** Session persistent flags
  * Used in the m2session.flags, to pass common flags between layers.
  * The flag values are consistent with the ones used for M2NP/M2QP, but they
  * can be used for other protocols, too, since they are pretty generic.
  */
#define M2_FLAG_LISTEN              b10000000
#define M2_FLAG_DLLS                b01000000
#define M2_FLAG_VID                 b00100000
#define M2_FLAG_NLS                 b00010000



typedef struct {
    ot_u16  counter;
    ot_u8   channel;
    ot_u8   dialog_id;
    ot_u8   protocol;
    ot_u8   netstate;
    ot_u8   subnet;
    ot_u8   flags;
} m2session;


typedef struct {
    m2session   heap[OT_FEATURE(SESSION_DEPTH)];
    ot_s8       top;
    ot_u8       seq_number;
} session_struct;

extern session_struct session;




/** @brief  Initializes the Session module.  Run on startup.
  * @param  none
  * @retval none
  * @ingroup Session
  */
void session_init();



/** @brief  Reduces the session counters uniformly, and alerts if a session is beginning
  * @param  elapsed_ti      (ot_uint) ti to reduce all session counters by
  * @retval ot_bool         True/False on session event timeout / no timeout
  * @ingroup Session
  */
ot_bool session_refresh(ot_uint elapsed_ti);



/** @brief  Adds a session element to the heap, pushes it to stack, and sorts
  *         the stack so the session at the top is the one happening soonest.
  * @param  new_counter     (ot_u16) new session initial counter value
  * @param  new_netstate    (ot_u8) new session netstate value
  * @param  new_channel     (ot_u8) new session channel id
  * @retval m2session*      Pointer to added session struct
  * @ingroup Session
  *
  * Additional session data not supplied as parameters to this function must be
  * loaded-in by the user, via the returned pointer.
  */
m2session* session_new(ot_u16 new_counter, ot_u8 new_netstate, ot_u8 new_channel);



/** @brief  Returns true if there is already a session scheduled on supplied channel
  * @param  chan_id         (ot_u8) channel id to check for occupancy
  * @retval none
  * @ingroup Session
  *
  * The Mode 2 Session Layer is not supposed to ever allow two sessions to be 
  * _scheduled_ on a single channel, at any given time (the reason for this, by 
  * the way, is to provide a way to avoid hitting the same background packets 
  * multiple times).  The user can call this function before running a scan, in
  * order to potentially cancel the scan before it starts.
  * 
  * Ad-hoc sessions (counter set initially to 0) are exempt from any such rules.
  */
ot_bool session_occupied(ot_u8 chan_id);



/** @brief  Pops a session off the top of the stack (deleting it).
  * @param  none
  * @retval none
  * @ingroup Session
  */
void session_pop();



/** @brief  Flushes (pops) expired sessions out of the stack.
  * @param  none
  * @retval none
  * @ingroup Session
  */
void session_flush();



/** @brief  Flushes (pops) expired sessions out of the stack, but not the top session.
  * @param  none
  * @retval none
  * @ingroup Session
  */
void session_drop();



/** @brief  Returns the number of sessions in the stack (zero indexed)
  * @param  none
  * @retval ot_int		Number of sessions in the stack.  -1 = empty
  * @ingroup Session
  */
ot_int session_count();



/** @brief  Returns the session at the top of the stack.
  * @param  none
  * @retval m2session*   Session at top of stack
  * @ingroup Session
  *
  * The stack is sorted, so the session that is pending the soonest is on top.
  */
m2session* session_top();



/** @brief  Fast session netstate function, but not safe
  * @param  none
  * @retval ot_u8    Netstate of session at the top of the stack
  * @ingroup Session
  */
ot_u8 session_netstate();

#endif

