/* Copyright 2014 JP Norair
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
  * @file       /confit/confit.c
  * @author     JP Norair
  * @version    R100
  * @date       31 Mar 2014
  * @brief      ConfIt implementation for H-Builder and OpenTag/ALP
  * @defgroup   ConfIt
  * @ingroup    ConfIt
  * 

  ******************************************************************************
  */


#include <otstd.h>

#if (OT_FEATURE(SERVER) && OT_FEATURE(ALP))
#include <alp/confit.h>


// ConfIt not enabled in app_config.h
#if 1 || (ALP(CONFIT) != ENABLED)
ot_bool confit_proc(alp_tmpl* alp, id_tmpl* user_id) {
    return True;
}


// ConfIt is enabled in app_config.h
#else

ot_bool sub_ISF0_isprotected(id_tmpl* user_id) {
    vlFILE* fp;
    ot_bool test;
    
    fp      = ISF_open(0, VL_ACCESS_W, user_id);
    test    = (fp == NULL);
    vl_close(fp);
    
    return test;
}



void sub_set_addressing(ot_u8* status, ot_queue* inq, id_tmpl* user_id) {
    *(ot_u16*)dll.netconf.vid   = q_readshort(inq);
    dll.netconf.subnet          = q_readbyte(inq);
    dll.netconf.b_subnet        = q_readbyte(inq);
    dll.netconf.b_attempts      = q_readbyte(inq);
}


void sub_set_beacon(ot_u8* status, ot_queue* inq, id_tmpl* user_id) {
    vlFILE* fp;
    
    fp = ISF_open(6, VL_ACCESS_W, user_id);
    if (fp == NULL) {
        *status |= 1;
    }
    else {
        dll.netconf.dd_flags            = 1;                        // makes DLL use btemp only
        dll.netconf.btemp[0]            = q_readbyte(inq);          // chan
        dll.netconf.btemp[1]            = 0x80 | q_readbyte(inq);   // flags
        *(ot_u16*)&dll.netconf.btemp[2] = 0;                        // call tmpl (unused)
        *(ot_u16*)&dll.netconf.btemp[4] = 0;                        // call tmpl (unused)
        dll.netconf.btemp[6]            = q_readbyte(inq);          // interval hi
        dll.netconf.btemp[7]            = q_readbyte(inq);          // interval lo
        vl_close(fp);
    }
}



void sig_revert(void* a) {
/// revert affected files: yes, it is this easy
    dll_refresh();
}


ot_bool confit_proc(alp_tmpl* alp, id_tmpl* user_id) {
/// All ConfIt message management is atomic, so the data is pulled from the 
/// interface's alp queue without an additional app queue.
#   define inq  (alp->inq)
#   define outq (alp->outq)

    static const ot_u8 reflen[6]    = { 0, 5, 10, 15, 9, 201 };
    static const ot_u8 breaktmpl[6] = { 0, 1, 2, 3, 2, 201 };

    ot_u32  expiry = 0;
    ot_u8   status = 0;
    ot_u8   msglen;
    ot_u8   type;
    
    // Grab payload length, then Go past the header
    msglen          = inq->getcursor[1];
    inq->getcursor += 4;
    
    // Process the payload starting with the type field
    // Supported types are: 0-3, 200, 201
    type    = q_readbyte(inq);
    type    -= (type > 3) ? (200-4) : 0;
    
    // Validate that type is supported and that length meets reference length
    if (type > 5)               return True;
    if (msglen != reflen[type]) return True;
    
    // At the moment, all types require write access to ISF0 (network config),
    // so it is validated globally here.
    if (sub_ISF0_isprotected(user_id)) {
        return True;
    }
    
    // Do custom template parts for the types
    switch (type) {
    
        // Type 0: Revert temporary network configurations
        case 0: dll_refresh();
                return True;
        
        // Type 1: Discovery Beacon Setup
        confit_proc_DISCBEACON:
        case 1: dll.netconf.b_attempts  = q_readbyte(inq);
                break;
                
        // Type 2: Network Join
        // Type 3: Network Join & Alert
        case 2: 
        case 3: expiry                  = q_readlong(inq);
                dll.netconf.hold_limit  = q_readshort(inq);
                if (type & 1) {
                    goto confit_proc_DISCBEACON;
                }
                break;
        
        // Type 200 (4): HayTag Join Network & Alert
        case 4: expiry = ((ot_u32)q_readbyte(inq)) << 10;
                break;
        
        // Type 201 (5): HayTag Unjoin Network (todo)
        case 5: break;
        
        // Type 202 (6): HayTag Unjoin Network without key-update
        // Not sure this is needed, type0 might do
        //case 6: break;
    }
    
    // deal with common templates
    if (breaktmpl[type] & 2)    sub_set_addressing(&status, inq, user_id);
    if (breaktmpl[type] & 1)    sub_set_beacon(&status, inq, user_id);
    
    // If status is good and expiry is set, add a cron process to revert the
    // network settings (effectively to unjoin)
    if ((status == 0) && (expiry != 0)) {
#   if OT_FEATURE(OTCRON)
        otcron_add(&dll_systask_rf, CRONTIME_relative, expiry, CRONMASK_none);
#   else

        //quick-dirty method for time being, before cron works properly
        sys.task[TASK_external].event       = 8;
        sys.task[TASK_external].nextevent   = (expiry << 10);
        
#   endif
    }
    
    // All Confit management is atomic
    return True;
}



#endif

#endif

