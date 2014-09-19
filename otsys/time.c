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
  * @file       /otsys/time.c
  * @author     JP Norair
  * @version    R100
  * @date       31 Aug 2014
  * @brief      System Time Implementation
  * @ingroup    Time
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otsys/time.h>
#include <platform/timers.h>




#if OT_FEATURE(TIME)

static ot_time  systime;
static ot_time  starttime;


void sub_load_now(ot_time* now) {
    ot_u32 ticks;
    ot_u32 scratch;
    *now        = systime;
    ticks       = systim_get();
    scratch     = ticks + now->ticks;
    now->upper  = (scratch < ticks);
    now->ticks  = scratch;
}



void time_init_utc(ot_u32 utc) {
    time_set_utc(utc);
    starttime = systime;
}


void time_set_utc(ot_u32 utc) {
    systime.upper   = (utc >> 22);
    systime.ticks   = (utc << 10);
}


void time_add_ti(ot_u32 ticks) {
    ot_u32 scratch;
    scratch         = ticks + systime.ticks;
    systime.upper  += (scratch < ticks);
    systime.ticks   = scratch;
}


ot_u32 time_get_utc(void) {
    ot_time now;
    sub_load_now(&now);
    now->upper  <<= 22;
    now->ticks  >>= 10;
    return (now->upper | now->ticks);
}


ot_u32 time_uptime(void) {
    ot_time now;
    sub_load_now(&now);
    
    now->upper<<= 22;
    now->upper += (now->ticks >> 10);
    now->ticks  = (starttime.upper << 22);
    now->ticks += (starttime.ticks >> 10);

    return (now->upper - now->ticks);
}


ot_u32 time_uptime_ti(void) {
    ot_time now;
    sub_load_now(&now);
    return (now->ticks - starttime.ticks);
}




/* For Arduino only
ot_u32 time_uptime_sti(void)
    ot_u32 up_sti;
    up_sti  = (systime.ticks - starttime.ticks) << 5;
    up_sti += systim_get_clocker();
    return up_sti;
}
*/

#else
void time_init_utc(ot_u32 utc)          { }
void time_set_utc(ot_u32 utc)           { }
void time_add_ti(ot_u32 ticks)          { }
ot_u32 time_get_utc(void)               { return 0; }
ot_u32 time_uptime(void)                { return 0; }
ot_u32 time_uptime_ti(void)             { return 0; }

#endif





/* Old stuff

void platform_enable_rtc() {
/// OpenTag's RTC feature on STM32L is entirely managed in software by a
/// kernel task of specific design.  It is always running, so killing it
/// actually just resets the task.
#if 0 //(OT_FEATURE(TIME) == ENABLED)
    sys_kill(TASK_RTC);
#endif
}


void platform_disable_rtc() {
/// OpenTag's RTC feature on STM32L is entirely managed in software by a
/// kernel task of specific design.  The task is always running, so it is
/// "disabled" by setting the update interval to a slow amount (256 sec).
/// This is done by killing the task!
#if 0 //(OT_FEATURE(TIME) == ENABLED)
    TASK_RTC->cursor = 255;
    sys_kill(TASK_RTC);
#endif
}



void platform_set_time(ot_u32 utc_time) {
/// OpenTag's RTC feature on STM32L is entirely managed in software.
#if OT_FEATURE(TIME)
    // set to backup register
#endif
}



ot_u32 platform_get_time() {
/// UTC time is always maintained
#if OT_FEATURE(TIME)
    // pull from backup register
#endif
}



void platform_set_rtc_alarm(ot_u8 alarm_id, ot_u8 task_id, ot_u16 offset) {
///@todo STM32L OpenTag alarming must be done as a task.
#if 0 //(OT_FEATURE(TIME) == ENABLED)
#   if defined(__DEBUG__) || defined(__PROTO__)
    if (alarm_id < RTC_ALARMS)
#   endif
    {
        ot_u16 mask_lsb;
        ot_u16 best_limit;
        vlFILE* fp;

        fp                          = ISF_open_su( ISF_ID(real_time_scheduler) );
        rtc.alarm[alarm_id].disabled= 0;
        rtc.alarm[alarm_id].taskid  = task_id;
        rtc.alarm[alarm_id].mask    = PLATFORM_ENDIAN16(ISF_read(fp, offset));
        rtc.alarm[alarm_id].value   = PLATFORM_ENDIAN16(ISF_read(fp, offset+2));
        vl_close(fp);

        // Determine largest allowable wakeup interval based on alarm criteria,
        // and set it.  (Do checks so not to exceed any other alarm's criteria)
        mask_lsb    = 1;
        best_limit  = + 1;  // RTC->WUTR + 1;
        while ( (mask_lsb != 0) \
            && ((mask_lsb & rtc.alarm[alarm_id].mask) == 0) \
            && ((mask_lsb & best_limit) == 0) ) {
            mask_lsb <<= 1;
        }
    }
#endif
}
*/


