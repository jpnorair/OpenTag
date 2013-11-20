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
  * @file       /otlib/OTAPI_tasker.c
  * @author     JP Norair
  * @version    R101
  * @date       27 Sep 2013
  * @brief      Default implementation of OTAPI communication tasker functions
  * @ingroup    OTAPI
  *
  ******************************************************************************
  */

#include "OTAPI.h"

#if (OT_FEATURE(M2) && OT_FEATURE(SERVER))



m2session* sub_newtask(session_tmpl* s_tmpl, ot_app applet, ot_u16 offset) {
    m2session* session;
    
    /// Create new session and verfy that it was successfully added to the 
    /// stack.  (session always begins with req tx)
    session = session_new(applet, offset, (M2_NETSTATE_INIT | M2_NETSTATE_REQTX), s_tmpl->channel);
    
    if (session != NULL) {
        // Apply custom-masked subnet and flags to session (typically unused)
        session->subnet = (dll.netconf.subnet & ~s_tmpl->subnetmask) | \
                                        (s_tmpl->subnet & s_tmpl->subnetmask);
        session->flags  = (dll.netconf.dd_flags & ~s_tmpl->flagmask) | \
                                        (s_tmpl->flags & s_tmpl->flagmask);
    }
    
    return session;
}



m2session* otapi_task_immediate(session_tmpl* s_tmpl, ot_app applet) {
/// This call doesn't actually cause the session to occur immediately,
/// but it will happen immediately following any sessions happenning 
/// at this very moment.
    return sub_newtask(s_tmpl, applet, 0);
}


m2session* otapi_task_schedule(session_tmpl* s_tmpl, ot_app applet, ot_u16 offset) {
    return sub_newtask(s_tmpl, applet, offset);
}





m2session* otapi_task_advertise(advert_tmpl* adv_tmpl, session_tmpl* s_tmpl, ot_app applet) {
/// This is a more complicated process than the others, because it actually 
/// creates two sessions: one for the flood and one for the request.
#   define _FLOOD_NETSTATE  (M2_NETFLAG_FLOOD | M2_NETSTATE_INIT | M2_NETSTATE_REQTX)

    /// Make sure there are at least two free sessions
    if (session_numfree() < 2) {
        return NULL;
    }

    /// Only add the flood if the user isn't an idiot (or an algorithm of some sort)
    if (adv_tmpl->duration != 0) {
        m2session* adv_session;
        adv_session         = session_new(&dll_default_applet, 0, _FLOOD_NETSTATE, adv_tmpl->channel);
        adv_session->subnet = adv_tmpl->subnet;
    }
    
    return session_new(applet, adv_tmpl->duration, M2_NETSTATE_REQTX, s_tmpl->channel);

#   undef _FLOOD_NETSTATE
}



#endif

