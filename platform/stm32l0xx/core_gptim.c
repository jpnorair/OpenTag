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
/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  * @file       /platform/stm32l0xx/core_gptim.c
  * @author     JP Norair
  * @version    R101
  * @date       14 Sep 2014
  * @brief      GPTIM driver for STM32L0
  * @ingroup    Platform
  *
  * STM32L0 uses its LPTIM timer as the kernel timer.  GPTIM2 is implemented
  * as a one-shot via the RTC Wakeup timer (RTC->WUTR).
  *
  * The OpenTag System Time module is driven also by LPTIM.  Each time the
  * LPTIM loops, an update interrupt is invoked that carries the bit into the
  * UTC time register.  The lower 16 bits of LPTIM are used directly by System
  * Time to describe 6 bits between 1-64 seconds and then the 10 bits between
  * 1 tick (1/1024 sec) and 1 second.
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>
#include <otsys/syskern.h>

systim_struct systim;


/// Local Options Flags (systim.opt)
#define SYSTIM_INSERTION_ON     1




/** ISRs used by System Time Module 
  * ========================================================================<BR>
  * LPTIM1, RTC-Wakeup, TIM6
  */

/// Used for SYSTIM-main
void platform_isr_lptim1() {
    ot_u8 lptim_flags = LPTIM1->ISR;

    // Clear all the interrupt flags in the register
    // Also make sure only the CMP interrupt is in usage
    LPTIM1->ICR = 0x7F;
#if OT_FEATURE(TIME)
    LPTIM1->IER = LPTIM_IER_CMPMIE | LPTIM_IER_ARRMIE;
#else
    LPTIM1->IER = LPTIM_IER_CMPMIE;
#endif
#if OT_FEATURE(TIME)
    if (lptim_flags & LPTIM_ISR_ARRM) {
        // add to time
    }
    if (lptim_flags & LPTIM_ISR_CMPM)
#endif
    // Kernel Timer Compare Interrupt: clear the sleep flag that prevents the
    // scheduler from running
    {   systim.flags = 0;
    }
}


/// Used for Insertor & Ticker:
/// - The ticker doesn't do a damn thing except wakeup from sleep 
/// - The Insertor manually vectors to radio_mac_isr(), although this should be
///   changed in the future to a dynamic callback.
void platform_isr_rtcwakeup() { 
#   if (OT_FEATURE(M2))
#       if (RF_FEATURE(CSMATIMER) != ENABLED))
        if (systim.opt & SYSTIM_INSERTION_ON) {
            systim.opt ^= SYSTIM_INSERTION_ON;
            RTC->CR &= ~RTC_CR_WUTE;
            radio_mac_isr();
        }
#       endif
#   endif
}


/// Would be used for Clocker, if clocker actually cared interrupts
// void platform_isr_tim6() { }







#ifndef EXTF_systim_init
void systim_init(void* tim_init) {
/// This is the DEBUG implementation that uses TIM9 at 1024 Hz rather than
/// the RTC hybrid method.

    /// 1. The iTimer uses the RTC Wakeup Timer, as does the Mode-2
    ///    MAC Timer.  We must:
    ///    <LI> Unlock RTC as a whole </LI>
    ///    <LI> put into INIT mode </LI>
    ///    <LI> Use a prescaler to match the clock used.  For LSE, this is 1.
    ///         (set to 0).  For LSI, this is...
    RTC->WPR    = 0xCA;
    RTC->WPR    = 0x53;
    RTC->CR     =  0;   //(1<<5);
    RTC->ISR    = 0xFFFFFFFF;
    while((RTC->ISR & RTC_ISR_INITF) == 0);
    RTC->TR     = 0;

#   if BOARD_FEATURE(LFXTAL)
        //RTC->PRER               = (127 << 16) | 255;      // this is reset default value
#   else
    {   ot_u32 lsi_hz           = platform_ext_lsihz();
        platform_ext.lsi_remhz  = (lsi_hz & 1023);        // = remainder for clocks/tick
        platform_ext.lsi_khz    = (lsi_hz >> 10);
        RTC->PRER               = (127 << 16) | lsi_hz >> 7;
    }
#   endif

    // Re-enable RTC.  Wakeup timer is set to LSClock/16.
    // Wakeup interrupt is always on, but wakeup timer is not always on.
    RTC->WUTR   = 1023;                 // token initialization amount
    RTC->CR     = RTC_CR_WUTIE | b000;  // LSClock/16
    RTC->ISR    = 0;

    /// 2. Set LPTIM1 to use LSE/32 (or LSI/32)
    LPTIM1->CR      = 0;            // disable timer
    LPTIM1->ICR     = 0x3F;         // clear all interrupt flags
    LPTIM1->CFGR    = (b101<<9);    // div32
    LPTIM1->CMP     = 65535;
    LPTIM1->ARR     = 65535;
#   if OT_FEATURE(TIME)
    LPTIM->IER      = LPTIM_IER_ARRMIE | LPTIM_IER_CMPMIE;
#   else
    LPTIM1->IER     = LPTIM_IER_CMPMIE;
#   endif
    LPTIM1->CR      = LPTIM_CR_ENABLE;
    LPTIM1->CR      = LPTIM_CR_CNTSTRT | LPTIM_CR_ENABLE;


    /// 3. Clear flags and stamps
    systim.flags    = 0;
    systim.stamp1   = 0;
    
    
    /// 4. Clocker initialization.  OpenTag doesn't really care about Clocker
    ///    so we also need a way to configure if it should be enabled or not.
    TIM6->CR1   = 0;
    TIM6->CR2   = (b001 << 4);
    TIM6->DIER  = 0;
    TIM6->ARR   = 65535;
    TIM6->PSC   = ((PLATFORM_HSCLOCK_HZ / BOARD_PARAM_APB1CLKDIV) / 4);
    //TIM6->EGR   = TIM_EGR_UG;
}
#endif




/** @brief LPTIM is not using the APB clock, so there can be clock sync errors
  *        when reading changing registers like the counter register.  The
  *        STM320 reference manual recommends reading the CNT in a loop until
  *        two successive reads match.  That is the purpose of this function.
  */
ot_u16 __read_lptim_cnt() {
    ot_u16 a, b;
    b = LPTIM1->CNT;
    do {
        a = b;
        b = LPTIM1->CNT;
    } while (b != a);

    return b;
}




ot_u32 systim_get() {
    ot_u16 timer_cnt;
    timer_cnt   = __read_lptim_cnt();
    timer_cnt  -= (ot_u16)systim.stamp1;
    return (ot_u32)timer_cnt;
}

ot_u16 systim_next() {
    return systim.stamp1 - __read_lptim_cnt();
}


void systim_enable() {
#   if OT_FEATURE(TIME)
    LPTIM1->IER = LPTIM_IER_ARRMIE | LPTIM_IER_CMPMIE;
#   else
    LPTIM1->IER = LPTIM_IER_CMPMIE;
#   endif
}

void systim_disable() {
#   if OT_FEATURE(TIME)
    LPTIM1->IER = LPTIM_IER_ARRMIE;
#   else
    LPTIM1->IER = 0;
#   endif
}

void systim_pend() {
    LPTIM1->IER    |= LPTIM_IER_CMPOKIE;
    systim.stamp1   = __read_lptim_cnt();
    LPTIM1->CMP     = systim.stamp1;
}

void systim_flush() {
    systim_disable();
    systim.stamp1 = __read_lptim_cnt();
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

    /// Program the scheduled time into the timer, in ticks.
    systim_disable();
    systim.flags    = GPTIM_FLAG_SLEEP;
    LPTIM1->ICR     = LPTIM_ICR_CMPMCF;                 // Clear compare match
    systim.stamp1   = __read_lptim_cnt();
    LPTIM1->CMP     = systim.stamp1 + (ot_u16)nextevent;
    systim_enable();

    return (ot_u16)nextevent;
}









/** Ticker feature
  * ========================================================================<BR>
  * Ticker is a ... 
  */
  
#define _TICKER_Hz      (PLATFORM_LSCLOCK_HZ/16)

#if (_TICKER_Hz < 1024)
#   warning "LSCLOCK is too slow to guarantee proper functioning, ideally is 32768Hz"
#   define _TICKER_SHIFT    0
#   define _LSCLOCK_NON1024
#elif (_TICKER_Hz == 1024)
#   define _TICKER_SHIFT    0
#elif (_TICKER_Hz < 1536)
#   define _TICKER_SHIFT    0
#   define _LSCLOCK_NON1024
#elif (_TICKER_Hz < 2048)
#   define _TICKER_SHIFT    1
#   define _LSCLOCK_NON1024
#elif (_TICKER_Hz == 2048)
#   define _TICKER_SHIFT    1
#elif (_TICKER_Hz < 3072)
#   define _TICKER_SHIFT    1
#   define _LSCLOCK_NON1024
#else
#   error "LSCLOCK is too fast to guarantee proper functioning, ideally is 32768Hz"
#endif
#ifdef _LSCLOCK_NON1024
#   warning "LSCLOCK Hz is not a multiple of 1024.  Things should work, but it is not a reliable source of real-time."
#endif



void sub_enable_wkuptim() {
/// <LI> Make sure RTC is on (if required, sometimes it is always on) </LI>
/// <LI> Make sure Wakeup Timer is on </LI>
/// <LI> Wakeup Timer interrupt is always enabled (see systim_init()) </LI>
    ot_u32 scratch;

//#   if (BOARD_FEATURE(HWRTC) != ENABLED)
//    scratch = RCC->CSR;
//    if !(scratch & RCC_CSR_RTCEN) {
//        RCC->CSR = scratch | RCC_CSR_RTCEN;
//    }
//#   endif
    scratch = RTC->CR;
    if ((scratch & RTC_CR_WUTE) == 0) {
        RTC->CR = scratch | RTC_CR_WUTE;
    }
}

void sub_disable_wkuptim() {
/// <LI> Wakeup Timer interrupt is always enabled (see systim_init()) </LI>
/// <LI> Make sure Wakeup Timer is off </LI>
/// <LI> Make sure RTC is off (if required, sometimes it is always on) </LI>
    ot_u32 scratch;
    
    scratch = RTC->CR;
    if (scratch & RTC_CR_WUTE) {
        RTC->CR = scratch ^ RTC_CR_WUTE;
    }
//#   if (PLATFORM_FEATURE(HWRTC) != ENABLED)
//    scratch = RCC->CSR;
//    if (scratch & RCC_CSR_RTCEN) {
//        RCC->CSR = scratch ^ RCC_CSR_RTCEN;
//    }
//#   endif
}

void sub_set_wkuptim(ot_uint period) {
    ot_u32 rtc_cr;

    // Ensure Wakeup Timer is off
    rtc_cr  = RTC->CR;
    RTC->CR = rtc_cr & ~RTC_CR_WUTE;

    // Ticks interval to run wakeup.
    RTC->WUTR = (period << _TICKER_SHIFT);

    // Enable Wakeup with interrupt
    // This will do nothing more than wake-up the chip from STOP at the set
    // interval.  The default ISR in platform_isr_STM32L.c is sufficient.
    RTC->CR = rtc_cr | (RTC_CR_WUTIE | RTC_CR_WUTE);
}



#ifndef EXTF_systim_set_ticker
void systim_set_ticker(ot_uint period) {
    sub_enable_wkuptim();
    sub_set_wkuptim(period);
}
#endif

#ifndef EXTF_systim_stop_ticker
void systim_stop_ticker() {
    sub_disable_wkuptim();
}
#endif





/** Insertion feature
  * ========================================================================<BR>
  * The insertion feature (or, "insertor"), is a one-shot counter that will 
  * cause a software interrupt when it expires.  The time base is 1-tick for
  * all standard implementations.
  *
  * @note On STM32L0, the insertor utilizes the ticker, therefore you can only
  * use one or the other safely.  Also, in OpenTag, the Mode 2 MAC timer needs
  * the insertor, so if Mode 2 is being used then you can't really use either
  * except for special cases (e.g. during bootup).
  */
void systim_set_insertion(ot_u16 value) {
    if (systim.opt & SYSTIM_INSERTION_ON) {
        sub_set_wkuptim(value);
    }
}

OT_INLINE void systim_enable_insertion() {
    systim.opt |= SYSTIM_INSERTION_ON;
    sub_enable_wkuptim();
}

OT_INLINE void systim_disable_insertion() {
    systim.opt &= ~SYSTIM_INSERTION_ON;
    sub_disable_wkuptim();
}




/** Clocker feature
  * ========================================================================<BR>
  * In virtually all OpenTag implementations the "clocker" is used as a high
  * resolution task timer.  It is set/reset whenever the kernel does a context
  * switch (i.e. loads a task).  OpenTag enforces a 256-tick (250ms) maximum
  * contiguous runtime for a task, so the loop-period of the clocker timer MUST
  * be at least this long.  For 16 bit timers, the minimum clock-period is thus
  * about 4 microseconds (~4us).
  *
  * @note Generally speaking, don't use the clocker directly.  The Scheduler
  * usually needs it, and it will manage it in a specific way.  Clocker uses
  * TIM6 in the STM32L0 implementation.
  *
  * @note Using it with MULTISPEED features may be unreliable.  Some platforms
  * like STM32L1 have timers that are independent from the core or bus clocks,
  * and on these platforms it is easier to implement clocker.  If clocker is
  * run from a bus-clock, you will need to change the bus-clock divider,
  * correspondingly, whenever changing the core speed.
  */

///@todo clocker implementation.  OpenTag doesn't really care about clocker, so
/// we also need a way to configure if it should be enabled or not.

void systim_start_clocker() {
    TIM6->EGR = TIM_EGR_UG;
    TIM6->CR1 = TIM_CR1_CEN;
}

void systim_restart_clocker() {
    systim_start_clocker();
}

ot_u16 systim_get_clocker() {
    return TIM6->CNT;
}

void systim_stop_clocker() {
    TIM6->CR1 = 0;
}






/** Chronometer Stamp feature
  * ========================================================================<BR>
  * The interval counter needs a free-running counter to use as a time source,
  * and it needs the timer to run through STOP mode.  So, LPTIM is the perfect
  * choice.  Since LPTIM is already set-up as a free-running counter as the
  * kernel timer, all we need to do in this implementation is to read it.
  */

ot_u32 systim_chronstamp(ot_u32* timestamp) {
    ot_u16 timer_cnt = __read_lptim_cnt();

    if (timestamp != NULL) {
        timer_cnt -= (ot_u16)*timestamp;
    }
    return (ot_u32)timer_cnt;
}






/** Future Usage (May also be removed or relocated)
  * ========================================================================<BR>
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




