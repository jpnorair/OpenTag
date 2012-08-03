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
  */
/**
  * @file       /otlib/system_native.h
  * @author     JP Norair
  * @version    V1.0
  * @date       31 January 2012
  * @brief      System Framework additions for Native System Environment
  * @ingroup    System
  *
  * Sys
  ******************************************************************************
  */


#ifndef __SYSTEM_NATIVE_H
#define __SYSTEM_NATIVE_H

#include "system.h"


/** Event processor Functions
  * This is the function type required of event processors.  I haven't yet
  * implemented external events, but it is basically one line of code away (it
  * is commented-out).
  *
  */
typedef ot_int (*ot_sysevt)();
  
void sys_set_extevent(ot_u8 event_no, ot_long nextevent);
void sys_set_extprocess(ot_sigv process);



/** Event Mechanism
  * DO NOT WRITE TO sysevent_struct UNLESS YOU KNOW WHAT YOU ARE DOING.  The one
  * possible exception is the sys.evt.EXT component, but even with this you 
  * probably still need to know what you are doing.
  *
  * sysevent_struct holds all the data for the event table.  It could be remade 
  * into a queue, but for now at least it is a static list.
  */

typedef struct {
    ot_u8   event_no;
    ot_int  nextevent;  // radio events never will be too long
#   if (OT_FEATURE(SYSRF_CALLBACKS) == ENABLED)
#   ifndef EXTF_sys_sig_rfainit
        ot_sig  init;
#   endif
#   ifndef EXTF_sys_sig_rfaterminate
        ot_sig2 terminate;
#   endif
#   endif
} radio_event;


//typedef struct {
//    ot_u8   event_no;
//    ot_long nextevent;
//} wakeon_event;


typedef struct {
    ot_u8   event_no;
    ot_u8   sched_id;
    ot_int  cursor;
    ot_long nextevent;
#   if (OT_FEATURE(SYSIDLE_CALLBACKS) == ENABLED)
        ot_sigv prestart;
#   endif
} idletime_event;


#ifndef OT_FEATURE_EXTERNAL_EVENT
#define OT_FEATURE_EXTERNAL_EVENT	ENABLED
#endif

#define HSS_INDEX       0
#define SSS_INDEX       (HSS_INDEX+(M2_FEATURE(ENDPOINT) == ENABLED))
#define BTS_INDEX       (SSS_INDEX+(M2_FEATURE(BEACONS) == ENABLED))
#define EXT_INDEX       (BTS_INDEX+(OT_FEATURE(EXTERNAL_EVENT) == ENABLED))
#define IDLE_EVENTS     (EXT_INDEX+1)

#define HSS     idle[HSS_INDEX]
#define SSS     idle[SSS_INDEX]
#define BTS     idle[BTS_INDEX]
#define EXT     idle[EXT_INDEX]




/// Organized in order of priority (highest priority first).
typedef struct {
    ot_sysevt       process;            // event processing function
    ot_uint         adv_time;           // Time for advertising
    ot_uint         hold_cycle;         // current hold cycle
    radio_event     RFA;                // RF Active event
    idletime_event  idle[IDLE_EVENTS];
} 
event_struct;







/** System main structure
  * The sys_struct is a grouping of the other data structures available. 
  * OpenTag's sys will always use the "object" sys.  Other modules may use sys
  * directly, or they may use function parameters that request a pointer to one
  * of the component data structures above.
  */
typedef struct {
    event_struct    evt;
    ot_u8           block;
    ot_u8           mutex;
    
#   if (OT_FEATURE(SW_WATCHDOG) == ENABLED)
        ot_u16      watchdog;
#   endif
#   if (OT_FEATURE(SYSKERN_CALLBACKS) == ENABLED)
        ot_bool (*loadapp)(void);
#   ifndef EXTF_sys_sig_panic
        ot_sig  panic;
#   endif
#   endif
	
} sys_struct;

extern sys_struct sys;





#endif

