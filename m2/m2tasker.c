/* Copyright 2013 JP Norair
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
  * @file       /otlib/m2_tasker.c
  * @author     JP Norair
  * @version    R101
  * @date       27 Sep 2013
  * @brief      Default implementation of OTAPI communication tasker functions
  * @ingroup    OTAPI
  *
  ******************************************************************************
  */

#include <otstd.h>
#if (OT_FEATURE(M2) && OT_FEATURE(SERVER))

#include <m2/dll.h>
#include <m2/m2task.h>
#include <m2/session.h>


void sub_apply_subnet_flags(session_tmpl* s_tmpl) {
    // Apply custom-masked subnet and flags to session (typically unused)
    s_tmpl->subnet  = (dll.netconf.subnet & ~s_tmpl->subnetmask) | \
                                        (s_tmpl->subnet & s_tmpl->subnetmask);
    s_tmpl->flags   = (dll.netconf.dd_flags & ~s_tmpl->flagmask) | \
                                        (s_tmpl->flags & s_tmpl->flagmask);
}


m2session* sub_newtask(session_tmpl* s_tmpl, ot_app applet, ot_u16 offset) {
    m2session* next;
    
    /// Make sure there is a free session... it is easiest this way
    if (session_numfree() < 1) {
        return NULL;
    }
    
    /// Create new session (session always begins with req tx)
    next = session_new(applet, offset, s_tmpl->channel, (M2_NETSTATE_INIT | M2_NETSTATE_REQTX));
    sub_apply_subnet_flags(s_tmpl);
    next->subnet    = s_tmpl->subnet;
    next->flags     = s_tmpl->flags;

    return next;
}



m2session* m2task_immediate(session_tmpl* s_tmpl, ot_app applet) {
/// This call doesn't actually cause the session to occur immediately,
/// but it will happen immediately following any sessions happenning 
/// at this very moment.
    return sub_newtask(s_tmpl, applet, 0);
}


m2session* m2task_schedule(session_tmpl* s_tmpl, ot_app applet, ot_u16 offset) {
    return sub_newtask(s_tmpl, applet, offset);
}





m2session* m2task_advertise(advert_tmpl* adv_tmpl, session_tmpl* s_tmpl, ot_app applet) {
/// This is a more complicated process than the others, because it actually 
/// creates two sessions: one for the flood and one for the request.
#   define _FLOOD_NETSTATE  (M2_NETFLAG_BG | M2_NETFLAG_STREAM | M2_NETSTATE_INIT | M2_NETSTATE_REQTX)
    m2session* next;
    
    /// Make sure there are at least two free sessions
    if (session_numfree() < 2) {
        return NULL;
    }

    /// Apply session flags
    sub_apply_subnet_flags(s_tmpl);
    
    /// Only add the flood if the user isn't an idiot (or an algorithm of some sort)
    if (adv_tmpl->duration != 0) {
        next        = session_new(&dll_default_applet, 0, adv_tmpl->channel, _FLOOD_NETSTATE);
        next->subnet= s_tmpl->subnet;
        next->flags = s_tmpl->flags;
    }
    
    next        = session_new(applet, adv_tmpl->duration, s_tmpl->channel, M2_NETSTATE_REQTX);
    next->subnet= s_tmpl->subnet;
    next->flags = s_tmpl->flags;
    
    return next;
    
#   undef _FLOOD_NETSTATE
}




m2session* m2task_page(session_tmpl* s_tmpl, ot_app applet) {
/// This is an experimental function
    m2session* next;
    next 			= sub_newtask(s_tmpl, applet, 0);
    next->netstate |= M2_NETFLAG_BG;
    next->flags     = s_tmpl->flags;

    return next;
}



#endif

