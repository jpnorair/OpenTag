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
  * @file       /platform/stm32f0xx/gptim_driver.c
  * @author     JP Norair
  * @version    R101
  * @date       30 Aug 2013
  * @brief      System timer drivers for STM32F0
  * @ingroup    Platform
  *
  * STM32F0 does not have TIM9 or TIM10, therefore there isn't a reliable way 
  * to run ticklessly through STOP mode.  Instead, general purpose TIM3 is used
  * as the system timer.  The implementation is rather straightforward.
  *
  * STM32F1 devices operate in a similar manner.
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>
#include <otsys/syskern.h>

gptim_struct gptim;





#define _KTIM_WATCHDOG_EXTRATICKS   256


void platform_isr_rtcwakeup(void) {
/// This ISR is normally used as a watchdog for the kernel scheduler.  itimer,
/// however, can be used for other purposes as well (generally on startup 
/// before the kernel gets fully started).  So the watchdog code (restarting
/// the kernel) will only get called if the kernel is actually running in Idle.

    if (gptim.flags & GPTIM_FLAG_SLEEP) {
        ///@todo log a glitch
        platform_init_OT();
    }
}






#ifndef EXTF_platform_init_itimer
void platform_init_itimer(ot_uint period) {
/// OpenTag does NOT use the ARM Cortex M SysTick.  Instead, it uses the RTC
/// Wakeup Timer feature to produce a 1 ti (1/1024sec) interval, which actually
/// is implemented as GPTIM for STM32L.  So, the setup for establishing this
/// interval is done in the gptim initialization (above).
    ot_u32 rtc_cr;

    // Ensure Wakeup Timer is off
    rtc_cr  = RTC->CR;
    RTC->CR = rtc_cr & ~RTC_CR_WUTE;

    // Ticks interval to run wakeup.
    RTC->WUTR = period;

    // Enable Wakeup with interrupt
    // This will do nothing more than wake-up the chip from STOP at the set
    // interval.  The default ISR in platform_isr_STM32L.c is sufficient.
    RTC->CR = rtc_cr | (RTC_CR_WUTIE | RTC_CR_WUTE);
}
#endif

#ifndef EXTF_platform_stop_itimer
void platform_stop_itimer() {
/// STOP RTC Interval Timer
    RTC->CR &= ~RTC_CR_WUTE;
}
#endif








void platform_isr_tim3() {
// TIM9 CCR1 is Kernel Timer interrupt
// TIM9 CCR2 is the GPTIM2 interrupt, typically the MAC timer
    ot_u16 test;
    test        = TIM9->SR;
    TIM9->SR    = 0;

#   if (OT_FEATURE(M2))
    if (test & TIM_SR_CC2IF) {
        radio_mac_isr();
    }
#   endif
    if (test & TIM_SR_CC1IF) {
        gptim.flags = 0;
    }
}


#ifndef EXTF_platform_init_gptim
void platform_init_gptim(ot_uint prescaler) {

/*
    /// 1. Configure the RTC.
    // Unlock RTC as a whole, put into INIT mode 
    RTC->WPR    = 0xCA;
    RTC->WPR    = 0x53;
    RTC->CR     = 0;
    RTC->ISR    = RTC_ISR_INIT;
    
    // Wait for init to be ready, then set to 1-ti (1s/1024) update period.
    while((RTC->ISR & RTC_ISR_INITF) == 0);
    RTC->TR     = 0;

    // If 32768 Hz LSE is used for RTC clock, prescaler is 32
    // If ~37kHz LSI is used, we need to calibrate it and use the best prescale value
#   if (BOARD_FEATURE(LFXTAL))    
        RTC->PRER   = (31 << 16) | 0;
        //RTC->CALIBR = ...;    // Calibration could be here
#   else
#       error "DEBUG mode requires an LF-XTAL of 32768Hz"
#   endif

    RTC->WUTR   = 0;

    // Re-enable RTC.  Wakeup timer is set to 1/1024 sec (1 tick).
    // ALARMB and wakeup interrupts are always on.  ALARMA is controlled by the
    // platform_...ktim() functions
    RTC->CR     = RTC_CR_WUTIE | b100;
    RTC->ISR    = 0;
*/

    /// Configure TIM3
    gptim.flags     = 0;
    gptim.stamp1    = 0;
    gptim.stamp2    = 0;
    
    NVIC->IP[(uint32_t)(TIM3_IRQn)]         = ((b0001) << 4);
    NVIC->ISER[((uint32_t)(TIM3_IRQn)>>5)]  = (1 << ((uint32_t)(TIM3_IRQn) & 0x1F));

    OT_GPTIM->CR1   = 0;
    OT_GPTIM->CR2   = 0;
    OT_GPTIM->SMCR  = 0;
    OT_GPTIM->ARR   = 65535;
    OT_GPTIM->PSC   = (OT_GPTIM_CLOCK / OT_GPTIM_RES);  // Should be always 1024 Hz
    OT_GPTIM->EGR   = TIM_PSCReloadMode_Immediate;      // generate update to lock-in prescaler
    OT_GPTIM->SR    = 0;                                // clear update flag
    OT_GPTIM->DIER  = TIM_DIER_UIE;                     // Update Interrupt
    OT_GPTIM->CR1  |= TIM_CR1_CEN;                      // enable
}
#endif



ot_u32 platform_get_interval(ot_u32* timestamp) {
    ot_u16 timer_cnt;
    timer_cnt = OT_GPTIM->CNT;

    if (timestamp != NULL) {
        timer_cnt -= (ot_u16)*timestamp;
    }
    return timer_cnt;
}


ot_u32 platform_get_ktim() {
    return (OT_GPTIM->CNT - (ot_u16)gptim.stamp1);
}

ot_u16 platform_next_ktim() {
    return (gptim.stamp1 - OT_GPTIM->CNT);
}

void platform_enable_ktim() {
    OT_GPTIM->DIER |= TIM_DIER_CC1IE;
}

void platform_disable_ktim() {
    OT_GPTIM->DIER &= ~TIM_DIER_CC1IE;
}

void platform_pend_ktim() {
    OT_GPTIM->EGR |= TIM_EGR_CC1G;
}

void platform_flush_ktim() {
    OT_GPTIM->DIER &= ~TIM_DIER_CC1IE;
    gptim.stamp1    = OT_GPTIM->CNT;
}

ot_u16 platform_schedule_ktim(ot_u32 nextevent, ot_u32 overhead) {
/// This should only be called from the scheduler.  

    /// If the task to be scheduled is already due (considering the runtime of
    /// the scheduler itself) return 0.  This will cause the sleep process to 
    /// be ignored and the task to start immediately.
    if ( (ot_long)(nextevent-overhead) <= 0 ) {
        gptim.flags = 0;
        return 0;
    }
    
    /// Start the interval timer, using some additional ticks more than the 
    /// scheduled arrival time of the kernel timer.
    //platform_init_itimer(nextevent + _KTIM_WATCHDOG_EXTRATICKS);
    
    /// Program the scheduled time into the timer, in ticks.
    gptim.flags     = GPTIM_FLAG_SLEEP;
    OT_GPTIM->DIER &= ~TIM_DIER_CC1IE;
    gptim.stamp1    = OT_GPTIM->CNT;
    OT_GPTIM->CCR1  = gptim.stamp1 + (ot_u16)nextevent;
    OT_GPTIM->DIER |= TIM_DIER_CC1IE;
    
    return (ot_u16)nextevent;
}



void platform_set_gptim2(ot_u16 value) {
/// gptim2 is often used for RF MAC timing.  It includes "value" = 0 protection 
/// because often a time-slot is started at position 0.
    OT_GPTIM->DIER &= ~TIM_DIER_CC2IE;
    gptim.stamp2    = OT_GPTIM->CNT;
    
    if (value == 0) OT_GPTIM->EGR   = TIM_EGR_CC2G;
    else            OT_GPTIM->CCR2  = gptim.stamp2 + (ot_u16)value;
    
    OT_GPTIM->DIER |= TIM_DIER_CC2IE;
}


void platform_enable_gptim2() {
    OT_GPTIM->DIER |= TIM_DIER_CC2IE;
}


void platform_disable_gptim2() {
    OT_GPTIM->DIER &= ~TIM_DIER_CC2IE;
    OT_GPTIM->SR   &= ~TIM_SR_CC2IF;
}



void gptim_start_chrono() {
}

void gptim_restart_chrono() {
}

ot_u16 gptim_get_chrono() {
    return (OT_GPTIM->CNT - (ot_u16)gptim.stamp1) << 5;
}

void gptim_stop_chrono() {
}



