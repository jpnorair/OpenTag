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
  * @file       /otlib/OTAPI_c.h
  * @author     JP Norair
  * @version    V1.0
  * @date       26 May 2011
  * @brief      OpenTag C API
  * @ingroup    OTAPI
  *
  * The C API is a wrapper to certain internal library functions.  The function
  * input (parameters) vary depending on what the function needs to do, but the
  * return value is always an unsigned 16 bit value.
  *
  * The functions declared in this .h file are implemented across many different
  * library modules.  The specific implementation files are mentioned during the
  * function declarations in this file.
  ******************************************************************************
  */


#ifndef __OTAPI_C_H
#define __OTAPI_C_H

#include "OT_types.h"
#include "OT_config.h"





/** Non-mapped API Functions <BR>
  * ========================================================================<BR>
  * These functions do not have equivalents in ALP.
  */

/** @brief Power-on initialization.  Implemented at the platform level.
  * @param  none
  * @retval none
  * @ingroup OTAPI_c
  */
void otapi_poweron();

/** @brief Data-safe Power-down.  Implemented at the platform level.
  * @param  none
  * @retval none
  * @ingroup OTAPI_c
  */
void otapi_poweroff();


/** @brief OpenTag system initialization.  Implemented at the platform level.
  * @param  none
  * @retval none
  * @ingroup OTAPI_c
  */
void otapi_init();


/** @brief Enters the Kernel.  Implemented at the platform level.
  * @param  none
  * @retval none
  * @ingroup OTAPI_c
  *
  * This function should only be used in timer ISRs that invoke the kernel.
  */
void otapi_exec();


/** @brief Pre-empts the OpenTag kernel.  Implemented at the platform level.
  * @param  none
  * @retval none
  * @ingroup OTAPI_c
  */
void otapi_preempt();


/** @brief Pauses the OpenTag kernel.  Implemented in the platform level.
  * @param  none
  * @retval none
  * @ingroup OTAPI_c
  */
void otapi_pause();



void otapi_led1_on();
void otapi_led2_on();
void otapi_led1_off();
void otapi_led2_off();




#if (OT_FEATURE(CAPI) == ENABLED)

#include "OTAPI_tmpl.h"
#include "OTAPI.h"

/*
#if (M2_FEATURESET == ENABLED)

#   if (M2FEATURE(ENDPOINT) == ENABLED )

#   endif

#   if (M2FEATURE(SUBCONTROLLER) == ENABLED)

#   endif

#   if (M2FEATURE(GATEWAY) == ENABLED)

#   endif

#endif
*/



/** System API Functions (implemented in system.c)
  * ========================================================================<BR>
  * @note   Session management is typically done via the System module.
  *
  * Request building process notes:
  * 1. Determin session parameters and store into a session_tmpl struct
  * 2. Call otapi_new_session() with the session_tmpl as a parameter
  * 3. Call otapi_open_request()
  * 4. Call transport protocol constructors (i.e. M2QP otapi functions)
  * 5. Call otapi_close_request()
  * 6. Call otapi_start_dialog() or otapi_start_flood() to send the request
  *
  * This process allows total configurability of the request.  If your 
  * application can be simplified, and it does not need total configurability,
  * you can pretty easily write subroutines that wrap this process into a single
  * function that is simple to use.
  */

#define OTAPI_SYSTEM_FUNCTIONS  6

/** @brief Refreshes system settings, wipes sessions, and brings to idle.
  * @param  none
  * @retval ot_u16      0/1 on failure/success
  * @ingroup OTAPI_c
  * @sa sys_refresh()
  *
  * Despite the name, otapi_sysinit() is a wrapper for sys_refresh() and not
  * sys_init().  sys_refresh(), and hence otapi_sysinit(), will restart OpenTag 
  * without clobbering the user application callbacks & data objects.
  */
ot_u16 otapi_sysinit();



/** @brief Creates new ad-hoc session and prepares communication task
  * @param  s_tmpl      (session_tmpl*) Session parameters
  * @param  applet      (void*) Converted to a Session Applet Pointer
  * @retval ot_u16      The session number (see otapi_session_number)
  * @ingroup OTAPI_c
  * @sa otapi_new_advdialog()
  *
  * Call this when some event occurs that makes you want to send a DASH7 packet.
  * A session needs to exist for the DASH7 engine to do its job.  All sessions
  * that you would generate by this function are ad-hoc, meaning they are not
  * scheduled for some time in the future (they happen right away).  Scheduled
  * sessions are reserved for internal DASH7 usage.
  */
ot_u16 otapi_new_dialog(session_tmpl* s_tmpl, void* applet);


/** @brief Creates new ad-hoc session and prepares communication task, with advertising
  * @param  adv_tmpl    (advert_tmpl*) Advertising parameters
  * @param  s_tmpl      (session_tmpl*) Session parameters
  * @param  applet      (void*) Converted to a Session Applet Pointer
  * @retval ot_u16      The session number (see otapi_session_number)
  * @ingroup OTAPI_c
  * @sa otapi_new_dialog()
  *
  */
ot_u16 otapi_new_advdialog(advert_tmpl* adv_tmpl, session_tmpl* s_tmpl, void* applet);



/** @brief  Manually creates (and opens) a request frame in the top session
  * @param  addr        (addr_type) enumerated addressing method (unicast, broadcast, etc)
  * @param  routing     (routing_tmpl*) A routing_tmpl
  * @retval ot_u16      The post-op length of the TX queue in bytes
  * @ingroup OTAPI_c
  * @sa otapi_close_request()
  * @sa otapi_new_session()
  *
  * Most often you will call this immediately after otapi_new_session().  Any
  * kind of event-generated frame/packet in DASH7 is always a request.  A DASH7
  * response is defined as something that follows a request, so if you are 
  * starting a new ad-hoc session, the outgoing packet will always be a request.
  *
  * The second input parameter, routing, is a void pointer and must be used
  * according to the value of addr.  
  * - If addr is BROADCAST or MULTICAST, routing should = NULL.  
  * - If addr is UNICAST, routing should be of type (id_tmpl*).
  * - If addr is ANYCAST, routing should be of type (routing_tmpl*)
  *
  * As you can see, Anycast addressing is the only form of request addressing  
  * that supports multi-hop routing.
  */
ot_u16 otapi_open_request(addr_type addr, routing_tmpl* routing);



/** @brief Manually finishes a request frame in the top session
  * @retval ot_u16      The post-op length of the TX queue in bytes
  * @ingroup OTAPI_c
  * @sa otapi_open_request()
  * @sa otapi_start_flood()
  * @sa otapi_start_dialog()
  *
  * In certain cases, footer data needs to be appended to the frame (mostly,
  * this is when you are doing some kind of encryption).  In these cases, 
  * otapi_close_request() will make sure to put it in the TX queue after the
  * frame data.
  *
  * Best practice is to always call otapi_close_request() when you are done
  * building the request.  When there is no encryption, you technically do not
  * need to call it, but this can lead to bugs in your application if you 
  * decide to add encryption later.  (If you are not encrypting, it adds maybe
  * only 8 instruction cycles to your runtime)
  */
ot_u16 otapi_close_request();



/** @brief  Begins a DASH7 M2AdvP flood onto the top session.
  * @param  flood_duration  (ot_u16) Number of ticks for the flood duration
  * @retval ot_u16          0/1 on failure/success of the flood initialization
  * @ingroup OTAPI_c
  * @sa otapi_new_session()
  * @sa otapi_new_request()
  * @sa otapi_start_dialog()
  *
  * This function is implemented in a way that matches a portion of the DASH7
  * Mode 2 spec. To understand the data I/O completely, refer to the spec.
  *
  * An M2AdvP flood is a way to take control of a channel so that a session can 
  * be guaranteed to occur at some point in the near future (precisely, a few 
  * ticks after the time of flood initialization + flood_duration).  
  *
  * To initialize a flood, a session has to be started with otapi_new_session().
  * Then call otapi_open_request(), whatever M2QP functions you need to build 
  * that request, and otapi_close_request().  Then call otapi_start_flood() to
  * begin the flood.  When the flood is over, OpenTag will automatically begin
  * the request dialog (no need to call otapi_start_dialog()).
  */
ot_u16 otapi_start_flood(ot_u16 flood_duration);



/** @brief  Begins a DASH7 dialog from the immediate session
  * @param  timeout         (ot_u16) Number of ticks to attempt TX or RX
  * @retval ot_u16          0/1 on failure/success of the dialog initialization
  * @ingroup OTAPI_c
  * @sa otapi_new_session()
  * @sa otapi_new_request()
  * @sa otapi_start_flood()
  *
  * Call this when you want to kick-off a dialog from an external application.
  */
ot_u16 otapi_start_dialog(ot_u16 timeout);









/** Session API Functions (implemented in session.c)
  * ========================================================================<BR>
  * The session API is special-purpose and does not follow the standard form.
  *                                                                           
  * @note   Session management is generally automatic.  These functions just  
  *         allow future exploration at the API/App level                     
  */

#define OTAPI_SESSION_FUNCTIONS     3

/** @brief Returns a 16 bit value uniquely corresponding to the top session
  * @param  none
  * @retval ot_u16      The 16 bit session value
  * @ingroup OTAPI_c
  *
  * The session value is actually just a concatenation of the 8 bit "channel"
  * element (channel ID) and the 8 bit "dialog_id" element (random dialog num)
  * that are part of a session structure.  The session stack may contain only
  * one scheduled channel for each of the supported channels at any given time,
  * so the session value will always be unique.
  *
  * The return value is always 0 for the case when the stack is empty.
  */
ot_u16 otapi_session_number();



/** @brief Deletes all expired sessions in the session stack
  * @param  none
  * @retval ot_u16      The size of the session stack after flushing
  * @ingroup OTAPI_c
  */
ot_u16 otapi_flush_sessions();



/** @brief Indicates if a given channel is already in the session stack.
  * @param  chan_id     (ot_u8) channel ID to check for blocking
  * @retval ot_u16      0/1 if supplied channel ID is unblocked/blocked
  * @ingroup OTAPI_c
  *
  * The session stack can only contain one session of a given channel at any
  * given time.  Sessions are not typically very long, so this feature should
  * not be a hinderance to DASH7 operation.  On the other hand, it does allow
  * simplicity by removing needs for priorities and such in session mgmt.
  */
ot_u16 otapi_is_session_blocked(ot_u8 chan_id);











/**
  * M2QP API Functions (implemented in m2_transport.c)
  * ========================================================================<BR>
  * The M2QP API follows the standard form.  Future API extensions will follow
  * the form: ot_u16 otapi_function(ot_u8* status, void* data_type)
  */

#define OTAPI_M2QP_FUNCTIONS    11

/** @brief Writes M2QP command parameters onto the request TX queue
  * @param status       (ot_u8*) returns a status code (0 = error)
  * @param command      (command_tmpl*) command input parameter template
  * @retval ot_u16      post-op length of the TX queue
  * @ingroup OTAPI_c
  * 
  * This function is implemented in a way that matches a portion of the DASH7
  * Mode 2 spec. To understand the data I/O completely, refer to the spec.
  *
  */
ot_u16 otapi_put_command_tmpl(ot_u8* status, command_tmpl* command);



/** @brief Write A2P/NA2P dialog parameters onto the request TX queue
  * @param status       (ot_u8*) returns a status code (0 = error)
  * @param dialog       (dialog_tmpl*) dialog input parameter template
  * @retval ot_u16      post-op length of the TX queue
  * @ingroup OTAPI_c
  *
  * API dir_cmd code == 0x02
  * 
  * This function is implemented in a way that matches a portion of the DASH7
  * Mode 2 spec. To understand the data I/O completely, refer to the spec.
  *
  * One spec-driven usage note is that, by setting rx_channels = 0, the chanlist
  * argument will be ignored and the pre-existing settings (from the request TX)
  * will be used automatically.
  */
ot_u16 otapi_put_dialog_tmpl(ot_u8* status, dialog_tmpl* dialog);



/** @brief Write M2QP Query parameters onto the end of the TX queue
  * @param  status      (ot_u8*) returns a status code (0 = error)
  * @param  query       (query_tmpl*) query input parameter template
  * @retval ot_u16       post-op length of the TX queue
  * @ingroup OTAPI_c
  *
  * API dir_cmd == 0x03
  *
  * This function is implemented in a way that matches a portion of the DASH7
  * Mode 2 spec. To understand the data I/O completely, refer to the spec.
  *
  * query->length param: the query token and mask are the same length, which is
  * the number of bytes set in this parameter.
  *
  * query->code param: b7 is 0/1 if the mask is disabled/enabled.
  *                    (see Mode 2 spec for further definition)
  */
ot_u16 otapi_put_query_tmpl(ot_u8* status, query_tmpl* query);



/** @brief  Write a device ack list onto the end of the TX queue
  * @param  status      (ot_u8*) returns a status code (0 = error)
  * @param  ack         (ack_tmpl*) ack input parameter template
  * @retval ot_u16      post-op length of the TX queue
  * @ingroup OTAPI_c
  *
  * API dir_cmd == 0x04
  */
ot_u16 otapi_put_ack_tmpl(ot_u8* status, ack_tmpl* ack);



/** @brief  Writes an error tmpl to a response.  Not currently implemented.
  * @param  status      (ot_u8*) returns a status code (0 = error)
  * @param  error       (error_tmpl*) error input parameter template
  * @retval ot_u16      post-op length of the TX queue
  * @ingroup OTAPI_c
  *
  * API dir_cmd == 0x05
  *
  * This function is implemented in a way that matches a portion of the DASH7
  * Mode 2 spec. To understand the data I/O completely, refer to the spec.
  */
ot_u16 otapi_put_error_tmpl(ot_u8* status, error_tmpl* error);



/** @brief  Writes ISF comparison data to the request queue.
  * @param  status      (ot_u8*) returns a status code (0 = error)
  * @param  isfcomp     (comp_tmpl*) isf comparison data for request
  * @retval ot_u16      post-op length of the TX queue
  * @ingroup OTAPI_c
  *
  * API dir_cmd == 0x06
  *
  * This function is implemented in a way that matches a portion of the DASH7
  * Mode 2 spec. To understand the data I/O completely, refer to the spec.
  */
ot_u16 otapi_put_isf_comp(ot_u8* status, isfcomp_tmpl* isfcomp);



/** @brief  Writes ISF call data to the request queue.
  * @param  status      (ot_u8*) returns a status code (0 = error)
  * @param  isfcomp     (comp_tmpl*) isf call data for request
  * @retval ot_u16      post-op length of the TX queue
  * @ingroup OTAPI_c
  *
  * API dir_cmd == 0x07
  *
  * This function is implemented in a way that matches a portion of the DASH7
  * Mode 2 spec. To understand the data I/O completely, refer to the spec.
  */
ot_u16 otapi_put_isf_call(ot_u8* status, isfcall_tmpl* isfcall);



/** @brief "Calls" ISF or ISF series and writes the return data to the TX Queue 
  * @param  status      (ot_u8*) returns a status code (0 = error)
  * @param  isfcall     (isfcall_tmpl*) ISF call template (matches typedef)
  * @retval ot_u16      post-op length of the TX queue
  * @ingroup OTAPI_c
  *
  * API dir_cmd == 0x08
  *
  * This function is implemented in a way that matches a portion of the DASH7
  * Mode 2 spec. To understand the data I/O completely, refer to the spec.
  */
ot_u16 otapi_put_isf_return(ot_u8* status, isfcall_tmpl* isfcall);




/** @brief  Writes the request datastream command, including read directives
  * @param  status      (ot_u8*) returns a status code (0 = error)
  * @param  dsq         (ot_queue*) Queue where the request datastream is stored
  * @retval ot_u16      post-op length of the TX queue
  * @ingroup OTAPI_c
  *
  * API dir_cmd == 0x09
  *
  * This function is implemented in a way that matches a portion of the DASH7
  * Mode 2 spec. To understand the data I/O completely, refer to the spec.
  *
  * The request datastream must be non-null, and point to a datastream that
  * carries one or more application subprotocol read directives. The request 
  * datastream (read directives) must be able to fit inside the request frame 
  * of this initial request.
  *
  * The remainder of the datastream handshaking and transfer process is managed
  * automatically.  The complete, received datastream will be loaded into the
  * internal datastream queue, and it can be logged if desired.
  */
ot_u16 otapi_put_reqds(ot_u8* status, ot_queue* dsq);



/** @brief  Writes the propose datastream command, including optional write directives
  * @param  status      (ot_u8*) returns a status code (0 = error)
  * @param  dsq         (ot_queue*) Queue where the propose [write] datastream is stored
  * @retval ot_u16      post-op length of the TX queue
  * @ingroup OTAPI_c
  *
  * API dir_cmd == 0x0A
  *
  * This function is implemented in a way that matches a portion of the DASH7
  * Mode 2 spec. To understand the data I/O completely, refer to the spec.
  *
  * If the proposed datastream is too long to fit inside the initial request
  * frame, it will be placed inside a subsequent datastream transfer.
  *
  * The remainder of the datastream handshaking and transfer process is managed
  * automatically.  The complete, transmitted datastream will be loaded into an
  * internal datastream queue prior to transmission.  The recipient of the 
  * datastream typically writes it to veelite.
  */
ot_u16 otapi_put_propds(ot_u8* status, ot_queue* dsq);



/** @brief  Writes udp data to the request queue.
  * @param  status      (ot_u8*) returns a status code (0 = error)
  * @param  shell       (shell_tmpl*) Shell template
  * @retval ot_u16      post-op length of the TX queue
  * @ingroup OTAPI_c
  *
  * API dir_cmd == 0x0B
  *
  * This function is implemented in a way that matches a portion of the DASH7
  * Mode 2 spec. To understand the data I/O completely, refer to the spec.
  */
ot_u16 otapi_put_udp_tmpl(ot_u8* status, udp_tmpl* udp);

#endif










#endif


