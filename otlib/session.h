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
  * @file       /otlib/session.h
  * @author     JP Norair
  * @version    R102
  * @date       7 Oct 2013
  * @brief      Mode 2 Session Framework
  * @defgroup   Session (Session Layer)
  * @ingroup    Session
  *
  * The DASH7 Mode 2 specification describes a Session Layer, and this layer is
  * implemented partially by this module.  This module also implements some 
  * additional features.
  * 
  * If the application has not enabled DASH7 Mode 2 features, the session 
  * module is still part of the active compilation because any task can use the
  * session stack for various tasking purposes -- especially communication 
  * interface tasks.
  * 
  * The session module implements a session stack.  The stack is sorted when a
  * new session is inserted.  The implementation of the stack itself is in the
  * session.c file, and it is completely abstracted in case you want to do 
  * something more complex than the really basic stack implementation that is
  * currently used.  The current implementation is good for systems where there
  * are fewer that four simultaneous sessions permitted, which is actually more
  * than a lot of devices will ever need -- for many devices, one session is all
  * that will ever be used at any given time.
  * 
  ******************************************************************************
  */


#ifndef __SESSION_h
#define __SESSION_h

#include "OT_types.h"
#include "OT_config.h"






/** Session Network State Flags
  * Used in the m2session.netstate container for the unassoc-synced-
  * connected-assoc state transition.  The secondary purpose is for other types 
  * of state transitions.
  */


#define M2_NETSTATE_TMASK           0x70
#define M2_NETSTATE_REQTX           0x00
#define M2_NETSTATE_REQRX           0x20            // set after valid request received
#define M2_NETSTATE_RX              0x20
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


/** Session power configuration (deprecated)
  * Used in .cs_rssi, .cca_rssi, .tx_eirp containers to 
  * move power configurations between layers.
  */
#define M2_RSSI_AUTOSCALE           b10000000
#define M2_RSSI_MASK                b00011111
#define M2_RSSI_DBM(VAL)            (ot_int)(-140 + ((VAL)*3))
#define M2_EIRP_AUTOSCALE           b10000000
#define M2_EIRP_MASK                b01111111
#define M2_EIRP_DBM(VAL)            (ot_int)(-40 + ((VAL)>>1))
#define M2_EIRP_0p5DBM(VAL)         (ot_int)(-80 + (VAL))


/** Session persistent flags
  * Used in the m2session.flags, to pass common flags between layers.
  * The flag values are consistent with the ones used for M2NP/M2QP, but they
  * can be used for other protocols, too, since they are pretty generic.
  */
#define M2_FLAG_LISTEN              b10000000
#define M2_FLAG_DLLS                b01000000
#define M2_FLAG_VID                 b00100000
#define M2_FLAG_NLS                 b00010000


/** "Extra" Information
  */
#define M2_EXTRA_USERFLAGS          0x0F
#define M2_EXTRA_RFU                0xF0






/** @typedef m2session
  * @brief The "session" is a Mode 2 dialog sequence container.
  *
  * applet      (ot_app) Applet associated with a session.  The Applet is run
  *             from the kernel immediately before the session is activated, so
  *             it is the customary place where the user constructs a request.
  *             The user MUST set to NULL when done with the applet.
  * 
  * counter     (ot_uint) A number of ticks (ti) indicating session schedule.
  *             When counter == 0, the kernel should activate the session.
  *             The kernel can use session_refresh() (or other session module 
  *             functions) to manage the counter.
  *
  * channel     (ot_u8) The Channel ID that the session is using for requests.
  *
  * netstate    (ot_u8) Control code for Mode 2 network behavior.  Applets can
  *             usually ignore this, except for setting SCRAP to kill a session.
  *
  * extra       (ot_u8) Extra information about the session.  Most commonly it
  *             is used to specify a method of encryption, when encryption is
  *             enabled in the session flags.
  *
  * dialog_id   (ot_u8) Each session has a one-byte Dialog ID used for basic
  *             matching of request & response in a dialog sequence.  It should
  *             be assigned different values from one session to the next.  The
  *             Session Module uses an incrementer to do this.
  *
  * subnet      (ot_u8) For network-generated sessions, this gets set in the
  *             Network Module.  For system & user-generated sessions, this 
  *             takes the value of the Device Subnet (normal request) or Beacon
  *             Subnet (beacon request) field from Network Settings ISF 0.
  *
  * flags       (ot_u8) Certain Mode 2 protocol flags that get propagated.
  */
typedef struct m2session {
	void    (*applet)(struct m2session*);
    ot_u16  counter;
    ot_u8   channel;
    ot_u8   netstate;
    ot_u8   extra;      ///@todo see if there is benefit in re-aligning this element
    ot_u8   dialog_id;
    ot_u8   subnet;
    ot_u8   flags;
} m2session;



/** @typedef ot_app
  * @brief Applet callback that is associated with Mode 2 session
  *
  * When using the native kernel, the applet function is used directly as an
  * application callback.  If using other RTOS types, it could be a function
  * wrapper for a threading system, or something else.
  *
  * @note The user function may need to manipulate the session variables of the
  * input in order to control kernel processes.
  */
typedef void (*ot_app)(m2session*);




typedef struct {
    ot_int      top;
    ot_u8       reserved;
    ot_u8       seq_number;
    m2session   heap[OT_PARAM(SESSION_DEPTH)];
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
  * @retval ot_bool         True when a session is ready
  * @ingroup Session
  */
ot_bool session_refresh(ot_uint elapsed_ti);



/** @brief  Adds a session element to the heap, pushes it to stack, and sorts
  *         the stack so the session at the top is the one happening soonest.
  * @param  applet          (ot_app) applet bound to session
  * @param  new_counter     (ot_u16) new session initial counter value
  * @param  new_netstate    (ot_u8) new session netstate value
  * @param  new_channel     (ot_u8) new session channel id
  * @retval m2session*      Pointer to added session struct
  * @ingroup Session
  *
  * Additional session data not supplied as parameters to this function must be
  * loaded-in by the user, via the returned pointer.
  */
m2session* session_new(ot_app applet, ot_u16 new_counter, ot_u8 new_netstate, ot_u8 new_channel);



/** @brief  Returns true if there is a session scheduled on supplied channel
  * @param  chan_id         (ot_u8) channel id to check for occupancy
  * @retval none
  * @ingroup Session
  *
  * This function is sort-of deprecated.
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



/** @brief  Pops sessions out of the stack that are below a scheduling threshold.
  * @param  threshold       (ot_u16) scheduling threshold
  * @retval none
  * @ingroup Session
  */
void session_crop(ot_u16 threshold);



/** @brief  Drop expired sessions and move to the ready session
  * @param  none
  * @retval m2session*  The new top session pointer
  * @ingroup Session
  *
  * This function should only be used in a session activation routine or in
  * session applets, as in these cases it is implictly known that the top
  * session is either ready or expired.
  */
m2session* session_drop();



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




#if (defined(__STDC__) || defined (__POSIX__))

/** @brief  Test function to print the session stack to stdout (POSIX/STD-C only)
  * @param  none
  * @retval None
  * @ingroup Session
  */
void session_print();

#endif

#endif
