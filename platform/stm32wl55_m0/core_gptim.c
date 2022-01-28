/* Copyright 2014-2019 JP Norair
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
  * @file       /platform/stm32wl55_m0/core_gptim.c
  * @author     JP Norair
  * @version    R101
  * @date       16 Dec 2021
  * @brief      GPTIM driver for STM32L0
  * @ingroup    Platform
  *
  * STM32WL can use the RTC in binary mode OR LPTIM1 as its kernel timer (GPTIM).
  * 
  * STM32L4 uses its LPTIM timer as the kernel timer.  GPTIM2 is implemented
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

#if (OT_FEATURE(M2))
#	include <m2/radio.h>
#endif

systim_struct systim;



/// Local Options Flags (systim.opt)
#define SYSTIM_INSERTION_ON     1

// Debugging
#ifndef OT_FEATURE_TIME
#   define OT_FEATURE_TIME  DISABLED
#endif
#define _USE_HW_TIME    (DISABLED && (OT_FEATURE_TIME))






/** @brief LPTIM is not using the APB clock, so there can be clock sync errors
  *        when reading changing registers like the counter register.  The
  *        STM32L4 reference manual recommends reading the CNT in a loop until
  *        two successive reads match.  That is the purpose of this function.
  */
ot_u16 __read_lptim_cnt(uint32_t* lptim) {
    ot_u16 a, b;
    b = lptim->CNT;
    do {
        a = b;
        b = lptim->CNT;
    } while (b != a);

    return b;
}


void __write_lptim_cmp(uint32_t* lptim, ot_u16 new_cmp) {
// Clear the CMPOK bit ahead of setting CMP
// Set the CMP to the new value
// Assure that CMP was successfully written 
    lptim->ICR = LPTIM_ICR_CMPOKCF;
    lptim->CMP = new_cmp;
    while ((LPTIM1->ISR & LPTIM_ISR_CMPOK) == 0) {
        //test_cmp++;
    }
}

void __write_lptim_arr(uint32_t* lptim, ot_u16 new_arr) {
    lptim->ICR = LPTIM_ICR_ARROKCF;
    lptim->ARR = new_arr;
    while ((lptim->ISR & LPTIM_ISR_ARROK) == 0);
}



void __read_lptim1_cnt(ot_u16 new_cmp) {
    __read_lptim_cmp(LPTIM1, ot_u16 new_arr)
}
void __write_lptim1_cmp(ot_u16 new_cmp) {
    __write_lptim_arr(LPTIM1, ot_u16 new_arr)
}
void __write_lptim1_arr(ot_u16 new_arr) {
    __write_lptim_arr(LPTIM1, ot_u16 new_arr)
}
void __read_lptim2_cnt(ot_u16 new_cmp) {
    __read_lptim_cmp(LPTIM2, ot_u16 new_arr)
}
void __write_lptim2_cmp(ot_u16 new_cmp) {
    __write_lptim_arr(LPTIM2, ot_u16 new_arr)
}
void __write_lptim2_arr(ot_u16 new_arr) {
    __write_lptim_arr(LPTIM2, ot_u16 new_arr)
}



/** ISRs used by System Time Module 
  * ========================================================================<BR>
  * LPTIM1, RTC-Wakeup, TIM6
  */

/// Used for SYSTIM-main
void platform_isr_lptim1() {
    ot_u8 lptim_flags;
    
    //volatile ot_u16 test;
    //test = __read_lptim1_cnt();
    
    // Clear interrupt flags
    lptim_flags = LPTIM1->ISR;
    LPTIM1->ICR = lptim_flags;

    // Kernel Timer Compare Interrupt: clear the sleep flag that prevents the
    // scheduler from running
    if (lptim_flags & LPTIM_ISR_CMPM) {
        systim.flags = 0;
    }
    
    //test = 0;
}


/// Used for Insertor & Ticker:
/// - The ticker doesn't do a damn thing except wakeup from sleep 
/// - The Insertor manually vectors to radio_mac_isr(), although this should be
///   changed in the future to a dynamic callback.
void platform_isr_rtcwakeup() { 
#if (OT_FEATURE(M2))
#if (RF_FEATURE(CSMATIMER) != ENABLED)
    if (systim.opt & SYSTIM_INSERTION_ON) {
        systim.opt ^= SYSTIM_INSERTION_ON;
        RTC->SCR    = RTC_SCR_CWUTF;
        RTC->CR    &= ~RTC_CR_WUTE;
        radio_mac_isr();
    }
#endif
#endif
}


/// Would be used for Clocker, if clocker actually cared interrupts
// void platform_isr_tim16() { }




#ifndef EXTF_systim_init
void systim_init(void* tim_init) {
    /// 1. The iTimer uses the RTC Wakeup Timer, as does the Mode-2
    ///    MAC Timer.  We must:
    ///    <LI> Unlock RTC as a whole </LI>
    ///    <LI> put into INIT mode </LI>
    ///    <LI> Use a prescaler to match the clock used.  For LSE, this is 1.
    ///         (set to 0).  For LSI, this is...
    RCC->BDCR  |= RCC_BDCR_RTCEN;
    
    RTC->WPR    = 0xCA;
    RTC->WPR    = 0x53;
    RTC->CR     = 0;    //(1<<5);
    RTC->ICSR  |= RTC_ICSR_INIT;
    while ((RTC->ICSR & RTC_ICSR_INITF) == 0);
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

    // Clear any flags.
    RTC->SCR        = 0xFFFFFFFF;

    /// 2. Set LPTIM1 to use LSE/32 (or LSI/32)
    
    // IER and CFGR must be set with LPTIM disabled
    LPTIM1->CR      = 0;            // disable timer
    LPTIM1->IER     = LPTIM_IER_CMPMIE;
    LPTIM1->CFGR    = ((5-MCU_PARAM(LPTIM_PS_SHIFT)) << 9);    // generally div32, 16, 8
    
    // ARR and CMP must be set with LPTIM enabled
    LPTIM1->CR      = LPTIM_CR_ENABLE;
    __write_lptim1_arr(65535);
    __write_lptim1_cmp(65535);
    LPTIM1->ICR     = 0x7F;         // clear interrupt flags
    //LPTIM1->ICR     = 0x1FF;
    
    // Start Timer
    LPTIM1->CR      = LPTIM_CR_CNTSTRT | LPTIM_CR_ENABLE;
    

    /// 3. Clear flags and stamps
    systim.flags    = 0;
    systim.stamp1   = 0;
    
    
    /// 4. Clocker initialization.  OpenTag doesn't really care about Clocker
    ///    so we also need a way to configure if it should be enabled or not.
    RCC->APB1ENR1  |= (RCC_APB1ENR1_TIM16EN);
    
    LPTIM2->CR      = 0;            // disable timer
    LPTIM2->IER     = 0;            // no interrupt for clocker
    LPTIM2->CFGR    = (0 << 9);     // no prescaler (32768 Hz)
    
    // ARR and CMP must be set with LPTIM enabled
    LPTIM2->CR      = LPTIM_CR_ENABLE;
    __write_lptim2_arr(65535);
    __write_lptim2_cmp(65535);
    LPTIM2->ICR     = 0x7F;         // clear interrupt flags
    //LPTIM2->ICR     = 0x1FF;
    LPTIM2->CR      = 0
}
#endif





ot_u32 systim_get() {
    ot_u16 timer_cnt;
    timer_cnt   = __read_lptim1_cnt();
    timer_cnt  -= (ot_u16)systim.stamp1;
    return (ot_u32)(timer_cnt >> OT_GPTIM_OVERSAMPLE);
}

ot_u16 systim_next() {
    ot_u16 clocks;
    clocks = (systim.stamp1 - __read_lptim1_cnt());
    return (clocks >> OT_GPTIM_OVERSAMPLE);
}


void systim_enable() {
/// systim_enable and systim_disable are empty on STM32L0
    //LPTIM1->IER = LPTIM_IER_CMPMIE;
}

void systim_disable() {
/// systim_enable and systim_disable are empty on STM32L0
    //LPTIM1->IER = 0;
}

///@todo the SWIER line guarantees the pend occurs
void systim_pend() {
    systim.stamp1 = __read_lptim1_cnt();
    __write_lptim1_cmp(systim.stamp1);
    EXTI->SWIER2 = (1<<(32-32));
}

void systim_flush() {
    //systim_disable();
    systim.stamp1 = __read_lptim1_cnt();
}

ot_u16 systim_schedule(ot_u32 nextevent, ot_u32 overhead) {
/// This should only be called from the scheduler.

    /// Subtracting the overhead of the scheduler runtime yields the true time
    /// to schedule the nextevent.
    nextevent = (ot_long)nextevent - (ot_long)overhead;
    
    /// If the task to be scheduled is already due return 0.
    /// This will ignore the sleep process and go straight to the task.
    if ((ot_long)nextevent <= 0) {
        systim.flags = 0;
        return 0;
    }
    
    /// Program the scheduled time into the timer, in ticks.
    /// Oversampling is done at the driver level, versus shifting which is done
    /// at the kernel level.  Oversampling can help timers without synchronized 
    /// updates whereas shifting can improve scheduling precision.
    systim.flags    = GPTIM_FLAG_SLEEP;
    LPTIM1->ICR     = 0x7f;     //LPTIM_ICR_CMPMCF;                 // Clear compare match
    systim.stamp1   = __read_lptim1_cnt();
    
    LPTIM1->CMP     = systim.stamp1 + (ot_u16)(nextevent << OT_GPTIM_OVERSAMPLE);

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
//    scratch = RCC->BDCR;
//    if !(scratch & RCC_BDCR_RTCEN) {
//        RCC->BDCR = scratch | RCC_BDCR_RTCEN;
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
//    scratch = RCC->BDCR;
//    if (scratch & RCC_BDCR_RTCEN) {
//        RCC->BDCR = scratch ^ RCC_BDCR_RTCEN;
//    }
//#   endif
}

void sub_set_wkuptim(ot_uint period) {
    ot_u32 rtc_cr;
    ot_uint wdog;

    // Ensure Wakeup Timer is off
    rtc_cr  = RTC->CR;
    RTC->CR = rtc_cr & ~RTC_CR_WUTE;

    // Poll WUTWF until it is set, afterwhich WUTR can be set
    // SW watchdog assumes 10 clocks per loop, to yield ~80us at 32MHz
    wdog = 256; // ~70 us at 32 MHz 
    while ((RTC->ICSR & RTC_ICSR_WUTWF) == 0) {
        if (--wdog == 0) {
            ///@todo error, do reset 
            break;
        }
    }
    
    // Ticks interval to run wakeup.
    RTC->WUTR = (period << _TICKER_SHIFT);

    // Enable Wakeup with interrupt
    // This will do nothing more than wake-up the chip from STOP at the set
    // interval.  The default ISR in platform_isr_STM32L.c is sufficient.
    RTC->CR = rtc_cr | RTC_CR_WUTIE | RTC_CR_WUTE | b000;
}



#ifndef EXTF_systim_set_ticker
void systim_set_ticker(ot_uint period) {
/// Ticker is not used on STM32L4 implementation
    //sub_set_wkuptim(period);
}
#endif

#ifndef EXTF_systim_stop_ticker
void systim_stop_ticker() {
/// Ticker is not used on STM32L4 implementation
    //sub_disable_wkuptim();
}
#endif





/** Insertion feature
  * ========================================================================<BR>
  * The insertion feature (or, "insertor"), is a one-shot counter that will 
  * cause a software interrupt when it expires.  The time base is 1-tick for
  * all standard implementations.
  *
  * @note In OpenTag, the Mode 2 MAC timer needs the insertor, so if Mode 2 is 
  * being used then you can't really use it except for special cases (e.g. 
  * during bootup).
  */
void systim_set_insertion(ot_u16 value) {
    if (systim.opt & SYSTIM_INSERTION_ON) {
        if (value == 0) {
            EXTI->SWIER1 = (1<<20);
        }
        else {
            sub_set_wkuptim(value);
        }
    }
}

OT_INLINE void systim_enable_insertion() {
    systim.opt |= SYSTIM_INSERTION_ON;
    //sub_enable_wkuptim();
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
  * about 4 microseconds (~4us).  32768Hz is a popular frequency, but it's not
  * so important.
  *
  * @note Generally speaking, don't use the clocker directly.  The Scheduler
  * usually needs it, and it will manage it in a specific way.  Clocker uses
  * LPTIM2 in the STM32WL implementation.
  */

void systim_start_clocker() {
    LPTIM2->CR = LPTIM_CR_CNTSTRT | LPTIM_CR_ENABLE;
}

void systim_restart_clocker() {
    systim_start_clocker();
}

ot_u16 systim_get_clocker() {
    return __read_lptim2_cmp();
}

void systim_stop_clocker() {
    LPTIM2->CR = 0;
}




/** Chronometer Stamp feature
  * ========================================================================<BR>
  * The interval counter needs a free-running counter to use as a time source,
  * and it needs the timer to run through STOP mode.  So, LPTIM is the perfect
  * choice.  Since LPTIM is already set-up as a free-running counter as the
  * kernel timer, all we need to do in this implementation is to read it.
  */

ot_u32 systim_chronstamp(ot_u32* timestamp) {
    ot_u16 timer_cnt = __read_lptim1_cnt();
    timer_cnt >>= OT_GPTIM_OVERSAMPLE;
    
    if (timestamp != NULL) {
        timer_cnt -= (ot_u16)*timestamp;
    }
    return (ot_u32)timer_cnt;
}






/** Future Usage (May also be removed or relocated)
  * ========================================================================<BR>
  */

#if 0 //_USE_HW_TIME
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




