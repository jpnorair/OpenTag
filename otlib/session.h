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


#ifndef OT_PARAM_SESSION_DEPTH
#   define OT_PARAM_SESSION_DEPTH   4
#endif

#if (OT_PARAM_SESSION_DEPTH < 3)
#   error "OT_PARAM_SESSION_DEPTH is less than 3."
#elif (OT_PARAM_SESSION_DEPTH > 8)
#   warning "OT_PARAM_SESSION_DEPTH is greater than 8.  This can slow things down."
#endif




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
#define M2_FLAG_LISTEN              (1<<7)
#define M2_FLAG_DLLS                (1<<6)
#define M2_FLAG_NLS                 (1<<5)
#define M2_FLAG_VID                 (1<<4)
#define M2_FLAG_RSCODE              (1<<3)
#define M2_FLAG_STREAM              (1<<2)
#define M2_FLAG_UCAST               (0<<0)
#define M2_FLAG_BCAST               (1<<0)
#define M2_FLAG_ACAST               (2<<0)
#define M2_FLAG_MCAST               (3<<0)


/** "Extra" Information
  */
#define M2_EXTRA_USERAUTH           (1<<6)
#define M2_EXTRA_CRYPTO             (3<<4)
#define M2_EXTRA_EPSLEEP            (1<<0)

//#define M2_EXTRA_APPFLAGS           (15<<0)







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
    m2session*  top;
    m2session   heap[OT_PARAM(SESSION_DEPTH)];
} session_struct;

extern session_struct session;




/** @brief  Initializes the Session module.  Run on startup.
  * @param  none
  * @retval none
  * @ingroup Session
  */
void session_init();



/** @brief  Activates the next session and returns its wait time
  * @param  None
  * @retval ot_uint     Ticks until next session event
  * @ingroup Session
  *
  * @note this function is not inherently safe.  You must first get "True" from 
  *       session_notempty() in order to be safe.
  *
  * Don't use this function unless you are building a Link Layer integrated as
  * an exotask, and in that case just copy how it is used in m2_dll_task.c.
  */
ot_uint session_getnext();



/** @brief  Adds a session to the back of the list
  * @param  applet      (ot_app) applet bound to session
  * @param  wait        (ot_u16) new session wait counter value
  * @param  netstate    (ot_u8) new session netstate value
  * @param  channel     (ot_u8) new session channel id
  * @retval m2session*  Pointer to added session struct
  * @ingroup Session
  * @sa session_extend()
  *
  * Create a new session at the end of the session list/queue.  Returns NULL if
  * there is not enough room to add a new session.
  *
  * "Flags" and "Extra" session elements must be supplied by the user after
  * this function returns.  The other session elements are loaded-in by this
  * function.
  */
m2session* session_new(ot_app applet, ot_u16 wait, ot_u8 channel, ot_u8 netstate);


/** @brief  Extends an active session with another session
  * @param  applet      (ot_app) applet bound to session
  * @param  wait        (ot_u16) new session wait counter value
  * @param  netstate    (ot_u8) new session netstate value
  * @param  channel     (ot_u8) new session channel id
  * @retval m2session*  Pointer to added session struct
  * @ingroup Session
  * @sa session_new()
  *
  * Calling this function will insert a new session into the session list, in
  * between the end of the joined session sequence at the top of the list and
  * the first unjoined session sequence.  A "joined session sequence" is some
  * number of contiguous sessions that starts with M2_NETSTATE_INIT flag set,
  * and all subsequent sessions have this flag clear.  The session at the top
  * of the list is implicitly assumed to be part of a sequence, so it does not
  * need to have M2_NETSTATE_INIT set.
  *
  * "Flags" and "Extra" session elements must be supplied by the user after
  * this function returns.  The other session elements are loaded-in by this
  * function.
  */
m2session* session_extend(ot_app applet, ot_u16 wait, ot_u8 channel, ot_u8 netstate);



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


/** @brief  Returns the session at the top of the stack.
  * @param  none
  * @retval m2session*   Session at top of stack
  * @ingroup Session
  *
  * The stack is sorted, so the session that is pending the soonest is on top.
  */
m2session* session_top();


/** @brief  Returns Number of free sessions slots
  * @param  none
  * @retval ot_int  Number of free sessions slots
  * @ingroup Session
  */
ot_int session_numfree();


/** @brief  Returns True if the session list is not empty
  * @param  none
  * @retval ot_bool     True if session list is not empty
  * @ingroup Session
  */
ot_bool session_notempty();


/** @brief  Returns session following the one at the top of the stack
  * @param  none
  * @retval m2session*  Session pointer or NULL if list is less than two sessions
  * @ingroup Session
  */
m2session* session_follower();



/** @brief  Approximate arrival of session following the current one
  * @param  none
  * @retval ot_u16      Number of ticks until following session
  * @ingroup Session
  */
ot_u16 session_follower_wait();



/** @brief  Allows follower session to happen right away
  * @param  None
  * @retval None
  * @ingroup Session
  */
void session_invite_follower();



/** @brief  Postpones all non-sequential sessions by some amount
  * @param  postponement    (ot_u16) number of ticks to postpone, 0-65535
  * @retval None
  * @ingroup Session
  *
  * Note that the session wait time is clipped at 65535 ticks, so sessions
  * cannot be postponed beyond that.
  */
void session_postpone_inactives(ot_u16 postponement);



/** @brief  Returns netstate of session at top of list
  * @param  none
  * @retval ot_u8    Netstate value
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
