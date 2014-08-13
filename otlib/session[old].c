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
  * @file       /otlib/session.c
  * @author     JP Norair
  * @version    R101
  * @date       7 Oct 2013
  * @brief      DASH7 M2 (ISO 18000-7.4) Session Framework
  * @ingroup    Session
  *
  * The session stack is not exposed, because it may be implemented in a lot of
  * different ways.  The way it is implemented here is not very efficient for
  * stacks any larger than a small few elements (maybe 4).  Many devices only
  * need one session, and even more can do fine with just two, so it is not a
  * big deal, yet.  For a complex device with large functionality, you would
  * probably want to do change the data-copying method used here to some sort 
  * of pointer shifting method.
  *
  ******************************************************************************
  */


#include "session.h"
#include "OT_platform.h"

#define _1ST    0
#define _2ND    1
#define _LAST   (OT_PARAM(SESSION_DEPTH)-1)
#define _END    (OT_PARAM(SESSION_DEPTH))


session_struct session;





#ifndef EXTF_session_init
void session_init() {
    session.top = &session.heap[_END];
}
#endif



#ifndef EXTF_session_getnext
ot_uint session_getnext() {
/// Not idiot proof.  Do not call this unless you have already checked 
/// session_notempty().
    ot_uint wait;
    wait                    = session.top->counter;
    session.top->counter    = 0;
    return wait;
}
#endif




m2session* sub_store_session(m2session* store, ot_app applet, ot_u16 wait, ot_u8 netstate, ot_u8 channel) {
/// Attach session to location specified
    store->applet       = applet;
    store->counter      = wait;
    store->channel      = channel;
    store->netstate     = netstate;
    store->extra        = 0;
    store->dialog_id    = platform_prand_u8();
    return store;
}


#ifndef EXTF_session_extend
m2session* session_extend(ot_app applet, ot_u16 wait, ot_u8 channel, ot_u8 netstate) {
    m2session* extend;
    
    // If not one free session, there's no room!
    if (session.top <= &session.heap[_1ST]) {
        return NULL;
    }
    
    // We're adding a new session...
    session.top--;
    extend = session.top;
    
    // If the new session is on the last spot, it means the heap is actually
    // empty, so no movement is necessary.  If heap is not empty and not full
    // (we know this by condition above and condition below), go through the
    // session heap to see where to put this extended session, and move all
    // higher data one spot up.
    if (extend < &session.heap[_LAST]) {
        do {
            // Increment session, noting that first increment simply counters
            // "session.top--" above.
            extend++;
        
            // Inspect following session. If it is INIT, then we extend the 
            // session sequence below this point.  Else, following session is 
            // still part of the current session sequence, so keep looking.
            if (extend[1].netstate & M2_NETSTATE_INIT) {
                break;
            }
            
            // The above "if (session.top <= &session.heap[_1ST])" make it OK
            // if we are at the 1st place in the session heap
        } while (extend < &session.heap[_LAST]);
        
        // Shift the stack down one notch, from top to extension point.
        memcpy( (ot_u8*)session.top, 
                (ot_u8*)&session.top[1], 
                (ot_uint)((ot_u8*)extend - (ot_u8*)session.top) );
    }

    return sub_store_session(extend, applet, wait, channel, netstate);
}
#endif


#ifndef EXTF_session_new
m2session* session_new(ot_app applet, ot_u16 wait, ot_u8 channel, ot_u8 netstate) {
    ot_u8* src;
    ot_uint length;
    
    // Always reserve an extra session for extension.
    // i.e. There must be two or more free sessions to do session_new()
    if (session.top <= &session.heap[_2ND] ) {
        return NULL;
    }
    
    // We're adding a new session to the bottom of the heap/stack/queue...
    src         = (ot_u8*)session.top;
    session.top--;
    length      = (ot_uint)((ot_u8*)&session.heap[_END] - src);
    
    // Only do the copy if the size is not zero
    if (length != 0) {
        memcpy( (ot_u8*)session.top, src, length);
    }
    
    return sub_store_session(&session.heap[_LAST], applet, wait, netstate, channel);
}

#endif


#ifndef EXTF_session_occupied
//DEPRECATED
ot_bool session_occupied(ot_u8 chan_id) {
    m2session* next;
    next = session.top;
    
    while (next < &session.heap[_END]) {
        next++;
    }
    return (ot_bool)(next < &session.heap[_END]);
}
#endif



#ifndef EXTF_session_pop
void session_pop() {
    session.top++;
}
#endif



#ifndef EXTF_session_flush
void session_flush() {
    while (session_notempty()) {
        if (session.top->netstate & M2_NETSTATE_INIT) {
            break;
        }
        session_pop();
    }
}
#endif


#ifndef EXTF_session_top
m2session* session_top() {
    return session.top;
}
#endif


#ifndef EXTF_session_numfree
ot_int session_numfree() {
    ot_uint block;
    block   = (ot_uint)((ot_u8*)session.top - (ot_u8*)&session.heap[_1ST]);
    block  /= sizeof(m2session);
    block  -= 1;            //because we always keep one free for extensions
    
    return (ot_int)block;
}
#endif


#ifndef EXTF_session_notempty
ot_bool session_notempty() {
    return (session.top < &session.heap[_END]);
}
#endif



#ifndef EXTF_session_follower
m2session* session_follower() {
    if (session.top < &session.heap[_LAST]) {
        return &session.top[1];
    }
    return NULL;
}
#endif


#ifndef EXTF_session_follower_wait
ot_u16 session_follower_wait() {
    if (session.top < &session.heap[_LAST]) {
        return session.top[1].counter;
    }
    return 65535;
}
#endif


#ifndef EXTF_session_invite_follower
void session_invite_follower() {
    if (session.top < &session.heap[_LAST]) {
        session.top[1].counter      = 0;
        session.top[1].netstate    &= ~M2_NETSTATE_INIT;
    }
}
#endif



#ifndef EXTF_session_postpone_inactives
void session_postpone_inactives(ot_u16 postponement) {
    m2session* next;
    next = session.top;
    
    while (next < &session.heap[_END]) {
        if (next->netstate & M2_NETSTATE_INIT) {
            ot_long scratch;
            scratch         = next->counter + postponement;
            next->counter   = (scratch < 65535) ? (ot_u16)scratch : 65535;
            break;
        }
        next++;
    }
}
#endif



#ifndef EXTF_session_netstate
ot_u8 session_netstate() {
    return session.top->netstate;
}
#endif




#if (defined(__STDC__) || defined (__POSIX__))
#include <stdio.h>

void session_print() {
    ot_int i;
    m2session* test;
    
    i = (ot_int)((ot_u8*)&session.heap[_END] - (ot_u8*)session.top);
    printf("Number of Sessions: %d\n", i);
    
    if (i > 0) {
        printf("===  SCHED CHAN N.ST D.ID SNET EXTR FLAG\n");
        test = session.top;
        do {
            printf("%02d: 0x%04X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n", i++,
                test->counter, 
                test->channel, 
                test->netstate,
                test->dialog_id, 
                test->subnet, 
                test->extra, 
                test->flags);
                
            test++;
        } while (test < &session.heap[_END]);
    }
    
    printf("\n");
}



/* For your amusement, if you want to test this module on its own.
   (make sure to change the STACK_SIZE constant up top to a fixed value, and
   comment out OT_config.h)

int main(void) {
    m2session* session;
    
    session_init();
    
    session = session_new(0x1234, 0x21, 0x12);
    session = session_new(0x0000, 0x32, 0x10);
    session = session_new(0x5678, 0x24, 0x14);
    session = session_new(0x0000, 0x04, 0x12);
    session_print();
    
    session = session_new(0x0700, 0x34, 0x2D);
    session_print();
    
    session_refresh(0x0100);
    session_print();
    
    session_flush();
    session_print();
    
    session = session_new(0x0100, 0x04, 0x16);
    session_print();
    
    printf("session channel 0x%02X occupied?  (%d)\n", 0x13, session_occupied(0x13) );
    printf("session channel 0x%02X occupied?  (%d)\n", 0x12, session_occupied(0x12) );
    
    session = session_new(0x0000, 0x04, 0x21);
    session->dialog_id = 0x59;
    
    printf("top session number: 0x%04X\n", otapi_session_number() );
    
    return 0;
}

*/

#endif




