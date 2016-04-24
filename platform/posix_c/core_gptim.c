/* Copyright 2016 JP Norair
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
  * @file       /otplatform/posix_c/core_gptim.c
  * @author     JP Norair
  * @version    R101
  * @date       30 Jan 2016
  * @brief      Special GPTIM driver for Posix simulator
  * @ingroup    Platform
  *
  * The POSIX C implementation uses itimer to generate the system timer.
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>
#include <otsys/syskern.h>

systim_struct systim;


void sub_create_resource(sem_t* sem_ptr, const char* sem_name, signal_t IRQHandler) {
    sem_ptr = sem_open(sem_name, O_CREAT, 0x644, 0);
    if (sem_ptr == SEM_FAILED) {
        perror( sprintf("%s semaphore initilization", sem_name) );
        exit(1);
    }
    
    //looks like each resource needs its own thread...
	pthread_create(&m_thread, (pthread_attr_t*)0, &timerthread, (void*)0);
	signal(SIGALRM, IRQHandler);
}


///@todo need to do resource allocation in core_main.c
void blah(void) {
    sem_unlink("TIM0_sem");
    sem_unlink("TIM1_sem");
    
    
}

void blah_return(void) {
    sem_close("TIM0_sem");
    sem_close("TIM1_sem");
}




#define _KTIM_WATCHDOG_EXTRATICKS   256


void platform_isr_rtcwakeup(void) {
/// This ISR is normally used as a watchdog for the kernel scheduler.  itimer,
/// however, can be used for other purposes as well (generally on startup
/// before the kernel gets fully started).  So the watchdog code (restarting
/// the kernel) will only get called if the kernel is actually running in Idle.

    if (systim.flags & GPTIM_FLAG_SLEEP) {
        ///@todo log a glitch
        platform_init_OT();
    }
}






#ifndef EXTF_systim_set_ticker
void systim_set_ticker(ot_uint period) {
/// OpenTag does NOT use the ARM Cortex M SysTick.  Instead, it uses the RTC
/// Wakeup Timer feature to produce a 1 ti (1/1024sec) interval, which actually
/// is implemented as GPTIM for STM32L.  So, the setup for establishing this
/// interval is done in the systim initialization (above).
    ot_u32 rtc_cr;

    // Ensure Wakeup Timer is off and writeable
    rtc_cr  = RTC->CR;
    RTC->CR = rtc_cr & ~(RTC_CR_WUTIE | RTC_CR_WUTE);
    while ((RTC->ISR & RTC_ISR_WUTWF) == 0);

    // Ticks interval to run wakeup.
    RTC->WUTR = period;

    // Enable Wakeup with interrupt
    // This will do nothing more than wake-up the chip from STOP at the set
    // interval.  The default ISR in platform_isr_STM32L.c is sufficient.
    RTC->CR = rtc_cr | (RTC_CR_WUTIE | RTC_CR_WUTE);
}
#endif

#ifndef EXTF_systim_stop_ticker
void systim_stop_ticker() {
/// STOP RTC Interval Timer
    RTC->CR &= ~RTC_CR_WUTE;
}
#endif





void platform_isr_tim0() {
// tim0 is the kernel timer interrupt
    systim.flags = 0;
}

void platform_isr_tim1() {
// tim1 is the GPTIM2 interrupt, typically the MAC timer
    radio_mac_isr();
}




#ifndef EXTF_systim_init
void systim_init(void* tim_init) {
///@todo change the statements below to use sub_create_resource()
    
    TIM0_sem = sem_open("TIM0_sem", O_CREAT, 0x644, 0);
    if (TIM0_sem == SEM_FAILED) {
        perror("tim0_sem semaphore initilization");
        exit(1);    ///@todo call syskill
    }
    
    TIM1_sem = sem_open("TIM1_sem", O_CREAT, 0x644, 0);
    if (TIM1_sem == SEM_FAILED) {
        perror("tim1_sem semaphore initilization");
        exit(1);    ///@todo call syskill
    }
    
	///@todo Cortex-M has ostensibly 2 threads: M and P.
	///      M is interrupt (main) context, so tim0 and tim1 enable this context
	///      P is program context, this gets enabled by the kernel
	
	signal(SIGALRM, TIM0_IRQHandler);
	signal(SIGALRM, TIM1_IRQHandler);
}
#endif



ot_u32 systim_chronstamp(ot_u32* timestamp) {
    ot_u32 timer_cnt;
    timer_cnt = systim_get();

    if (timestamp == NULL) {
        return timer_cnt;
    }
    timer_cnt  -= *timestamp;
    return timer_cnt;
}


ot_u32 systim_get() {
    ot_u32 timer_cnt;
    
    ///@todo get itimer present value and convert units into ticks
    timer_cnt   = ...;

    return timer_cnt;
}

ot_u16 systim_next() {
    return systim.stamp1 - systim_get();
}

void systim_enable() {
    ///@todo figure out how to couple onto the signal
    TIM9->DIER |= TIM_DIER_CC1IE;
}

void systim_disable() {
    ///@todo figure out how to decouple from the signal
    TIM9->DIER &= ~TIM_DIER_CC1IE;
}

void systim_pend() {
    ///@todo figure out how to scrub the timer and manually fire the signal
    TIM9->EGR |= TIM_EGR_CC1G;
}

void systim_flush() {
    systim_disable();
    systim.stamp1 = systim_get();
}

void sub_set_tim0(ot_u32 ticks) {
    long ticks_us, ticks_s;
    
    ticks       = (long)((double)ticks * 1000.0 * (1000.0/1024.0));
    ticks_us    = ticks % 1000000;
    ticks_s     = ticks / 1000000; 
    
    {   struct itimerval tval = { 
            /* subsequent firings */ 
            .it_interval = { 
                .tv_sec = 64, 
                .tv_usec = 0
            }, 
            /* first firing */
            .it_value = { 
                .tv_sec = ticks_s, 
                .tv_usec = ticks_us 
            }
        };
    
        setitimer(ITIMER_REAL, &tval, (struct itimerval*)0);
    }
}


ot_u16 systim_schedule(ot_u32 nextevent, ot_u32 overhead) {
/// This should only be called from the scheduler.

    /// If the task to be scheduled is already due (considering the runtime of
    /// the scheduler itself) return 0.  This will cause the sleep process to
    /// be ignored and the task to start immediately.
    if ( (ot_long)(nextevent-overhead) <= 0 ) {
        systim.flags = 0;
        return 0;
    }
    
    /// If the task to be scheduled is too far away for the hardware, crop it.
    if (nextevent > 65535) {
        nextevent = 65535;
    }

    /// Start the interval timer, using some additional ticks more than the
    /// scheduled arrival time of the kernel timer.  In debug mode, this can
    /// be a problem because this timer doesn't stop with the debugger.
    //systim_set_ticker(nextevent + _KTIM_WATCHDOG_EXTRATICKS);

    /// Program the scheduled time into the timer, in ticks.
    systim.flags    = GPTIM_FLAG_SLEEP;
    systim.stamp1   = 0;
    systim_disable();
    sub_set_tim0(nextevent);
    systim_enable();

    return (ot_u16)nextevent;
}



void systim_set_insertion(ot_u16 value) {
/// systim2 is often used for RF MAC timing.  It includes "value" = 0 protection
/// because often a time-slot is started at position 0.
    TIM9->DIER     &= ~TIM_DIER_CC2IE;
    systim.stamp2    = TIM9->CNT;

    if (value == 0) TIM9->EGR   = TIM_EGR_CC2G;
    else            TIM9->CCR2  = systim.stamp2 + (ot_u16)(value << 2);

    TIM9->DIER     |= TIM_DIER_CC2IE;
}


void systim_enable_insertion() {
    TIM9->DIER |= TIM_DIER_CC2IE;
}


void systim_disable_insertion() {
    TIM9->DIER &= ~TIM_DIER_CC2IE;
    TIM9->SR   &= ~TIM_SR_CC2IF;
}



void systim_start_clocker() {
}

void systim_restart_clocker() {
}

ot_u16 systim_get_clocker() {
    return systim_get() << 5;
}

void systim_stop_clocker() {
}












/** Future Stuff
  */


#if 0 //(OT_FEATURE(TIME) == ENABLED)
void sub_juggle_rtc_alarm() {
    ot_u8  i;
    ot_u32 comp;
    ot_u32 next     = ~0;
    ot_u32 rtcval   = RTC_GetCounter();

    for (i=0; i<RTC_ALARMS; i++) {
        if (otrtc.alarm[i].active) {
            comp    = rtcval & ((ot_u32)otrtc.alarm[i].mask << RTC_OVERSAMPLE);
            comp   += (ot_u32)otrtc.alarm[i].value << RTC_OVERSAMPLE;
            next    = (comp < next) ? comp : next;
        }
    }

    RTC_SetAlarm(rtcval+next);
}
#endif // End of RTC stuff




