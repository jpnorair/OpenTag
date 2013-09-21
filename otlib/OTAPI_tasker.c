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
  *
  */
/**
  * @file       /otlib/OTAPI_tasker.c
  * @author     JP Norair
  * @version    R100
  * @date       31 Sep 2012
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
/// Make sure the radio is stopped, flush any interfering sessions,
/// and create the new session to occur immediately (offset = 0).
    if (radio.state != RADIO_Idle) {
        rm2_kill();
    }
    session_flush();
    return sub_newtask(s_tmpl, applet, 0);
}


m2session* otapi_task_schedule(session_tmpl* s_tmpl, ot_app applet, ot_u16 offset) {
    return sub_newtask(s_tmpl, applet, offset);
}



m2session* otapi_task_advertise(advert_tmpl* adv_tmpl, session_tmpl* s_tmpl, ot_app applet) {
/// This is a more complicated process than the others, because it actually 
/// creates two sessions: one for the flood and one for the request.
#   define _FLOOD_NETSTATE  (M2_NETFLAG_FLOOD | M2_NETSTATE_INIT | M2_NETSTATE_REQTX)
    m2session* com_session;
    m2session* adv_session;
    
    /// 1.  Clear any sessions between now and the request, and push the 
    ///     request session onto the stack.  If the push failed, exit.  If the
    ///     advertising is 0, also exit and do this session without flood.
    session_crop(adv_tmpl->duration);
    com_session = sub_newtask(s_tmpl, applet, adv_tmpl->duration);
    if ((com_session == NULL) || (adv_tmpl->duration == 0))
        return NULL;
    
    /// 2.  Flood duty cycling is not supported at this time.  All floods 
    ///     are implemented as 100% duty cycle.
    
    /// 3.  Push the Advertising session onto the stack, for immediate running.
    ///     <LI> For basic flooding, the applet can be empty </LI>
    adv_session = session_new(&otutils_applet_null, 0, _FLOOD_NETSTATE, adv_tmpl->channel);
    if (adv_session == NULL) {
        session_pop();  //pop the com session from above
        return NULL;
    }
    adv_session->subnet = adv_tmpl->subnet;
    
    return com_session;
#   undef _FLOOD_NETSTATE
}



#endif

