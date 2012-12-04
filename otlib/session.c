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
  * @file       /otlib/session.c
  * @author     JP Norair
  * @version    R100
  * @date       8 Aug 2012
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


#include "OT_config.h"
#include "session.h"

//#include "OTAPI_c.h"
#include "OT_platform.h"


#define SESSION_STACK_DEPTH     OT_FEATURE(SESSION_DEPTH)
//#define SESSION_STACK_DEPTH     4


#define Session0    session.heap[session.top]

session_struct session;



#ifndef EXTF_session_init
void session_init() {
    session.top = -1;
    //platform_memset(session.heap, 0, sizeof(session.heap)); //is this needed?
}
#endif



#ifndef EXTF_session_refresh
ot_bool session_refresh(ot_uint elapsed_ti) {
    ot_bool test = False;
    ot_s8   i;

    for (i=session.top; (i>=0) /* && (session.heap[i].netstate != 0) */; i--) {
        ot_long scratch;
        scratch                 = (ot_long)session.heap[i].counter - elapsed_ti;
        session.heap[i].counter = (scratch < 0) ? 0 : (ot_uint)scratch;
        test                   |= (session.heap[i].counter == 0);
    }

    return test;
}
#endif





#ifndef EXTF_session_new
m2session* session_new(ot_app applet, ot_uint new_counter, ot_u8 new_netstate, ot_u8 new_channel) {
    ot_s8 pos = session.top+1;
    
    /// If this is a scheduled session (not ad-hoc) follow the session rules
    /// when adding the session.
    if (new_counter != 0) {
        ot_s8  i;
        for (i=session.top; i>=0; i--) {
            // Set the position for the new session, by ascending order
            pos -= (new_counter > session.heap[i].counter);
            
            // Discontinued feature: fail if there is already a session on this channel
            //if (new_channel == session.heap[i].channel) {
            //    return NULL;
            //}
        }
    }

    /// If the session stack is not full, shift prioritized sessions up
    if (session.top < (OT_FEATURE(SESSION_DEPTH)-1)) {
        ot_s8 i;
        session.top++;

        for (i=session.top; i>pos; ) {
            ///@note The order of the new = old seems to matter.
            //ot_u32* new = (ot_u32*)&session.heap[i];
            //ot_u32* old = (ot_u32*)&session.heap[--i];
            //new[1]      = old[1];
            //new[0]      = old[0];
            ot_u8* new = (ot_u8*)&session.heap[i];
            ot_u8* old = (ot_u8*)&session.heap[--i];
            platform_memcpy(old, new, sizeof(session));
        }
    }
    
    /// If the stack is full, shift de-prioritized sessions down (deleting furthest)
    else {
        ot_int i;
        pos--;
        
        for (i=0; i<pos; ) {
            ///@note The order of the new = old seems to matter.
            //ot_u32* new = (ot_u32*)&session.heap[i];
            //ot_u32* old = (ot_u32*)&session.heap[++i];
            //new[0]      = old[0];
            //new[1]      = old[1];
            ot_u8* new = (ot_u8*)&session.heap[i];
            ot_u8* old = (ot_u8*)&session.heap[++i];
            platform_memcpy(new, old, sizeof(session));
        }
    }
    
    /// Write-out the session
    session.heap[pos].applet    = applet;
    session.heap[pos].counter   = new_counter;
    session.heap[pos].channel   = new_channel;
    session.heap[pos].netstate  = new_netstate;     ///@note, may need to OR with M2_NETSTATE_INIT
  //session.heap[pos].protocol  = 0;                ///session.protocol is deprecated  
    session.heap[pos].dialog_id = ++session.seq_number;
    
    return &session.heap[pos];
}
#endif




#ifndef EXTF_session_occupied
ot_bool session_occupied(ot_u8 chan_id) {
    ot_s8 i;
    
    for (   i=session.top; 
            (i>=0) && (chan_id != session.heap[i].channel); 
            i-- );
            
    return (ot_bool)(i>=0);
}
#endif



#ifndef EXTF_session_pop
void session_pop() {
    //Session0.netstate = 0;
    session.top--;
}
#endif



#ifndef EXTF_session_flush
void session_flush() {
    while ((Session0.counter == 0) && (session.top >= 0)) {
        session_pop();
    }
}
#endif


#ifndef EXTF_session_crop
void session_crop(ot_u16 threshold) {
    while ((Session0.counter <= threshold) && (session.top >= 0)) {
        session_pop();
    }
}
#endif


#ifndef EXTF_session_drop
void session_drop() {
    ot_u16 temp = Session0.counter;
    
    Session0.counter = 1;
    session_flush();
    Session0.counter = temp;
}
#endif



#ifndef EXTF_session_count
ot_int session_count() {
    return session.top;
}
#endif


#ifndef EXTF_session_top
m2session* session_top() {
    return &Session0;
}
#endif


#ifndef EXTF_session_netstate
ot_u8 session_netstate() {
    return session.heap[session.top].netstate;
}
#endif




#if (OT_FEATURE(CAPI) == ENABLED)

/** OTAPI Server Functions
  * Only compiled with builds that leverage OTAPI Server, either in C, LLDP, or
  * DASHForthVM form
  */

#ifndef EXTF_otapi_session_number
ot_u16 otapi_session_number() {

    if (session.top >= 0) {
        return *((ot_u16*)&Session0.channel);
        //return ((ot_u16)Session0.channel << 8) | (ot_u16)Session0.dialog_id;
    }
    return 0;
}
#endif


#ifndef EXTF_otapi_flush_sessions
ot_u16 otapi_flush_sessions() {
    session_flush();
    return session.top+1;
}
#endif


#ifndef EXTF_otapi_is_session_blocked
ot_u16 otapi_is_session_blocked(ot_u8 chan_id) {
    return (ot_u16)session_occupied(chan_id);
}
#endif


#endif






/* For your amusement, if you want to test this module on its own.
   (make sure to change the STACK_SIZE constant up top to a fixed value, and
   comment out OT_config.h)

#include <stdio.h>


void print_stack() {
    ot_int i;
    
    printf("Number of Sessions: %d\n", session.top+1);
    
    if (session.top >= 0)
        printf("=======================================\n");
    
    for (i=session.top; i>=0; i--) {
        printf("%d: 0x%04X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n", i,
            session.heap[i].counter, 
            session.heap[i].channel, 
            session.heap[i].dialog_id, 
            session.heap[i].netstate,
            session.heap[i].subnet, 
            session.heap[i].protocol, 
            session.heap[i].flags);
    }
    
    printf("\n");
}



int main(void) {
    m2session* session;
    
    session_init();
    
    session = session_new(0x1234, 0x21, 0x12);
    session = session_new(0x0000, 0x32, 0x10);
    session = session_new(0x5678, 0x24, 0x14);
    session = session_new(0x0000, 0x04, 0x12);
    print_stack();
    
    session = session_new(0x0700, 0x34, 0x2D);
    print_stack();
    
    session_refresh(0x0100);
    print_stack();
    
    session_flush();
    print_stack();
    
    session = session_new(0x0100, 0x04, 0x16);
    print_stack();
    
    printf("session channel 0x%02X occupied?  (%d)\n", 0x13, session_occupied(0x13) );
    printf("session channel 0x%02X occupied?  (%d)\n", 0x12, session_occupied(0x12) );
    
    session = session_new(0x0000, 0x04, 0x21);
    session->dialog_id = 0x59;
    
    printf("top session number: 0x%04X\n", otapi_session_number() );
    
    return 0;
}

*/


