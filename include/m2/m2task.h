/* Copyright 2010-2014 JP Norair
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
  * @file       /include/m2/m2task.h
  * @author     JP Norair
  * @version    R101
  * @date       26 Aug 2014
  * @brief      For initiating Mode2 communication tasks via a C API
  * @defgroup   M2-Tasker
  *
  * For initiating Mode2 communication tasks via a C API
  *
  ******************************************************************************
  */


#ifndef __M2_TASK_H
#define __M2_TASK_H

#include <otstd.h>
#include <m2/tmpl.h>            // Template data types used a lot in OT/DASH7
#include <m2/session.h>


/** Server/Client
  * Just in case you don't know client/server nomenclature, the device that is
  * remote is the server.  The local device is the client.  So, the client is
  * the one that probably doesn't have DASH7 on it, and the server is the one
  * that is running DASH7 and most of the OpenTag code.
  */




/** Comm-Tasker function definitions        <BR>
  * ========================================================================<BR>
  * These are implemented in m2task.c.  They are API functions used to create
  * OpenTag communication (session) tasks of various formats.
  */


/** @brief  Creates a session task that is entered immediately
  * @param  s_tmpl      (session_tmpl*) Session Template provided by caller
  * @param  applet      (ot_app) Applet that gets called when Session activates
  * @retval m2session*  Pointer to the session that was created
  * @ingroup OTAPI
  * @sa m2task_schedule(), m2task_flood()
  *
  * If you call this function somewhere in your application, it will create a
  * new, ad hoc communication session and put it in the session stack.  The 
  * kernel will process this session as soon as it is free.
  *
  * Like all session tasks, there is an attached applet.  The applet is a 
  * callback function that the kernel calls when it activates the session.
  * The applet needs to do whatever is required to build the request frame.
  * See the applets that come with OpenTag (in /otlibext) for ideas.
  *
  * This is a good task function to use, because it is predictable.  Generally,
  * if you application is able to call this function at all, it means the
  * kernel is not blocking session creation in favor of some higher-priority
  * task (e.g. I/O drivers).  So, when you call it, the chance that for some
  * reason your session is blocked (and flushed) is effectively 0.
  */
m2session* m2task_immediate(session_tmpl* s_tmpl, ot_app applet);



/** @brief  Creates a session task that is entered at a scheduled, later time
  * @param  s_tmpl      (session_tmpl*) Session Template provided by caller
  * @param  applet      (ot_app) Applet that gets called when Session activates
  * @param  offset      (ot_u16) Time offset, in ticks, when session gets activated
  * @retval m2session*  Pointer to the session that was created
  * @ingroup OTAPI
  * @sa m2task_immediate(), m2task_flood()
  *
  * This function is identical to m2task_immediate() but it also includes
  * the "offset" parameter.  The value supplied into "offset" is the number of
  * ticks (1/1024s) between the time of calling and when the kernel should 
  * activate the session.
  *
  * The caveat of using this function is that the kernel might be engaged in a
  * higher-priority I/O task at the time the scheduling occurs.  In this case,
  * sometimes the kernel will try to run the session late and sometimes it will
  * just flush the session.  The decision of late-run/flush is dependent on the
  * kernel implementation.  Typically, tasks that run really fast (<=1 tick) 
  * will not cause the sessions to be flushed, but longer tasks will.
  */
m2session* m2task_schedule(session_tmpl* s_tmpl, ot_app applet, ot_u16 offset);



/** @brief  Creates a session task that includes advertising flooding
  * @param  adv_tmpl    (advert_tmpl*) Advertising Template provided by caller
  * @param  s_tmpl      (session_tmpl*) Session Template provided by caller
  * @param  applet      (ot_app) Applet that gets called when Session activates
  * @retval m2session*  Pointer to the session that was created
  * @ingroup OTAPI
  * @sa m2task_immediate(), m2task_schedule()
  *
  * This function is identical to m2task_immediate() but it also includes
  * the "adv_tmpl" parameter for specifying the advertising method.  DASH7 M2
  * can do "background advertising," a method of flooding tiny packets onto a
  * channel for the purpose of synchronizing a group of unsynchronized devices. 
  * A session (starting with request) always follows the advertising flood.
  *
  * Note on advertising: Advertising is a unique feature of DASH7.  Some other
  * systems so similar things, but there is no system that has as powerful a
  * method as DASH7 has.  Basic usage is to have duty = 100%.  In some regions
  * (i.e. USA) you can set duty_on/duty_off to reduce average TX power, which
  * makes long advertising durations practical even with oppressive FCC rules.
  *
  * <LI> advert_tmpl.duty_off: ticks to take a break from advertising, before
  *      resuming.  Set to 0 to enable 100% duty. </LI>
  * <LI> advert_tmpl.duty_on: ticks to advertise before taking a break.  It is
  *      ignored if duty_off == 0. </LI>
  * <LI> advert_tmpl.channel: Channel ID to do the advertising on.  The session
  *      request channel may be the same channel or a different one. </LI>
  * <LI> advert_tmpl.duration: ticks to run the entire flooding process.  After
  *      this number of ticks the advertising will end, and the session request
  *      will begin. </LI>
  */
m2session* m2task_advertise(advert_tmpl* adv_tmpl, session_tmpl* s_tmpl, ot_app applet);



///@note experimental
m2session* m2task_telegram(session_tmpl* s_tmpl, ot_app applet);


#endif


