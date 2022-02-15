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
  * STM32WL can use the RTC in binary mode as its kernel & general purpose SW
  * timer (GPTIM).  This is the only supported setup, at present.
  * - GPTIM1 via Alarm A
  * - GPTIM2 via Alarm B
  * 
  * The OpenTag System Time module is driven also by GPTIM in this impl.
  * It can be performed in hardware via the Wakeup Timer or RTC-SSRU interrupt,
  * or done in software via the kernel scheduler (default).
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>
#include <otsys/syskern.h>

#if (OT_GPTIM_ID != 'R')
#   error "STM32WL OpenTag implementation only supports RTC as GPTIM, at present."
#endif

#if (BOARD_FEATURE(LFXTAL) != ENABLED)
#   error "To use RTC as kernel timer, LSE must be enabled."
#endif

#if (OT_GPTIM_OVERSAMPLE > 5)
#   warning "OT_GPTIM_OVERSAMPLE is set to a higher value than advisable"
#endif

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
#define _USE_HW_TIME    (DISABLED && OT_FEATURE(TIME))

// Used for...
#define _KTIM_WATCHDOG_EXTRATICKS   256



void BOARD_SWEXTI1_ISR(void) {
/// This is the SW EXTI interrupt used with the insertion timer
    BOARD_SWEXTI1_CLR();
#   if (RF_FEATURE_CSMATIMER != ENABLED)
        radio_mac_isr();
#   endif
}




void platform_isr_rtcwakeup(void) {
/// This ISR is normally used as a watchdog for the kernel scheduler.
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
///@todo implement ticker via rtc wakeup timer
}
#endif

#ifndef EXTF_systim_stop_ticker
void systim_stop_ticker() {
///@todo implement ticker via rtc wakeup timer
}
#endif



/** GPTIM Subroutine Prototypes <BR>
  * ========================================================================<BR>
  */

ot_u32 sub_rtc2ticks();
ot_u32 sub_ticks2alarm(ot_u32 ticks);
ot_u32 sub_get_nextalarm(ot_u16 next) __attribute__((flatten));


/** Platform ISRs implemented here <BR>
  * ========================================================================<BR>
  * <LI> platform_isr_rtcalarm() </LI>
  * <LI> platform_isr_tim10() </LI>
  * @todo use rtc wakeup timer instead of tim10
  */


void platform_isr_rtcalarm() {
/// The kernel timer uses ALARM-A and the radio MAC-timer is usually mapped to
/// ALARM-B.
    ot_u32 irqs;

    // Clocker gets started on wakeup from stop, that's enough
    //systim_start_clocker();

    /// Get the RTC IRQs and clear them in HW
    irqs        = RTC->MISR;
    RTC->SCR    = irqs;

    /// ALARM B event: Radio MAC alarm, only if not in radio HW
    if (irqs & RTC_MISR_ALRBMF) {
        RTC->CR &= ~RTC_CR_ALRBE;

#       if (RF_FEATURE_CSMATIMER != ENABLED)
            radio_mac_isr();
#       endif
    }

    /// ALARM A event: Task is pending
    /// Setting systim.flags to 0 will cause the tasking loop to exit sleep on
    /// exit from this interrupt.
    if (irqs & RTC_MISR_ALRAMF) {
        //RTC->CR &= ~RTC_CR_ALRAE;
        systim.flags = 0;
    }
}






/** Standard Platform functions implemented here <BR>
  * ========================================================================<BR>
  * <LI> systim_init()                      </LI>
  * <LI> ot_u32 systim_get(ot_u16* marker)   </LI>
  * <LI> void systim_flush()                 </LI>
  * <LI> ot_u16 systim_next()                </LI>
  * <LI> void systim_enable()                </LI>
  * <LI> void systim_disable()               </LI>
  * <LI> void systim_pend()                  </LI>
  * <LI> void platform_set_ktim(ot_u16 value)       </LI>
  * <LI> void systim_set_insertion(ot_u16 value)     </LI>
  * <LI> void systim_enable_insertion()              </LI>
  * <LI> void systim_disable_insertion()             </LI>
  * <LI> ot_u16 platform_get_systim()                </LI>
  */

#ifndef EXTF_systim_init
void systim_init(void* tim_init) {
/// For STM32WL, the RTC is the best choice for OpenTag Kernel Timer because it
/// can bring the system out of STOP2 and STANDBY modes.  STM32WL RTC also has
/// a binary counter mode, which makes it reasonably straightforward to use for
/// kernel timing.
/// <LI> The RTC is configured as a free-running binary counter, and the kernel
///        timer is utilizing ALARM A.</LI>
/// <LI> Radio MAC timer (sometimes called GPTIM2) is connected to ALARM B.</LI>
/// <LI> Wakeup Timer is used as the kernel watchdog and/or chronograph.

    // These are here in case we need to option the settings in the future
#   define _IE_WAKEUP   0   //RTC_CR_WUTIE
#   define _A_PRESCALER (32 - (1 << OT_GPTIM_OVERSAMPLE))

    /// Activate the clocks and system features to turn-on RTC
    PWR->CR1           |= PWR_CR1_DBP;
    RCC->C2APB1ENR1    |= RCC_C2APB1ENR1_RTCAPBEN;

    /// Initialize the RTC itself
    /// - unlock it
    /// - disable all control features
    /// - Set ICSR to use binary counter feature
    /// - Enter Initialization Mode, and wait for it to come.
    RTC->WPR    = 0xCA;
    RTC->WPR    = 0x53;
    RTC->CR     = 0;
    RTC->ICSR   = (b01 << 8)
                | RTC_ICSR_INIT;
    while ((RTC->ICSR & RTC_ICSR_INITF) == 0);

    /// RTC is being used as a binary counter.
    /// Set Alarm A and Alarm B to use all 32 bits
    RTC->ALRMASSR   = (32 << 24);
    RTC->ALRMBSSR   = (32 << 24);

    /// Prescaler is dependent on the oversample power
    /// For oversample = 5 (typical), prescaler is 0
    RTC->PRER       = (_A_PRESCALER << 16) | 0;

    ///@todo Calibration could be here
    RTC->CALR       = RTC_CALR_LPCAL;

    /// Re-enable RTC:
    /// - Wakeup timer is set to maximum length by default.
    /// - Direct Register Access enabled (Bypass shadow regs)
    /// - Enable Wakeup interrupt by default (but Wakeup timer off)
    /// - Enable Alarm-A/B interrupts by default
    /// - Clear all the ISR flags in SCR
    /// - Set RTC to use Binary Counter mode in ICSR
    RTC->WUTR   = 0x0000FFFF;
    RTC->CR     = /* 0 */ RTC_CR_BYPSHAD
                | _IE_WAKEUP
                | RTC_CR_ALRAIE
                | RTC_CR_ALRBIE
                | b100;
    RTC->SCR    = 0x7F;
    RTC->ICSR   = (b01 << 8);

    ///@todo re-lock RTC?
    //RTC->WPR    = 0;

    /// 3. Clear local static variables
    systim.clk_stamp    = 0;
    systim.k_stamp      = 0;
    systim.flags        = 0;

}
#endif



/** STM32L GPTIM Clocker/Chronometer functions <BR>
  * ========================================================================<BR>
  * Clocker / Chronometer isn't really necessary for STM32WL implementation.
  * It is needed in certain implementations, to fill gaps in access to the main
  * timer caused by clock domain synchronization.
  */

inline void systim_start_clocker() {
/// This function should be called in the ISR Wakeup hook of any ISR that is
/// able to bring the system out of STOP mode.
    systim_restart_clocker();
}

inline void systim_restart_clocker() {
/// This is a special version of systim_start_clocker(), which for some
/// platforms must have slight differences to systim_start_clocker().
    systim.clk_stamp = RTC->SSR;
}


inline ot_u16 systim_get_clocker() {
    return (ot_u16)(systim.clk_stamp - RTC->SSR);
}

inline void systim_stop_clocker() {
/// Clocker always runs in this implementation
}




ot_u32 systim_get() {
    ot_u32 clocks;
    //RTC->CR        &= ~RTC_CR_ALRAE;
    clocks      = systim.k_stamp - RTC->SSR;
    clocks    >>= OT_GPTIM_OVERSAMPLE;
    return clocks;
}

ot_u16 systim_next() {
    ot_u32 clocks;
    clocks      = RTC->SSR - systim.k_stamp;
    clocks    >>= OT_GPTIM_OVERSAMPLE;
    return (ot_u16)clocks;
}

void systim_flush() {
    uint32_t rtc_ssr = RTC->SSR;
    systim.clk_stamp = rtc_ssr;
    systim.k_stamp   = rtc_ssr;
}

inline void systim_enable() {
    RTC->CR |= RTC_CR_ALRAIE;
}

inline void systim_disable() {
    RTC->CR &= ~RTC_CR_ALRAIE;
}

//void systim_pend() {
// Not supported on STM32WL ... not used in the kernel anyway
//}

ot_u16 systim_schedule(ot_u32 nextevent, ot_u32 overhead) {
/// This should only be called from the scheduler.

    /// Subtracting the overhead of the scheduler runtime yields the true time
    /// to schedule the nextevent.
    ///@todo could this just be done against clk_stamp?
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
    systim.k_stamp  = RTC->SSR;
    RTC->CR        &= ~RTC_CR_ALRAE;
    RTC->SCR        = RTC_SCR_CALRAF;
    RTC->ALRABINR   = systim.k_stamp - (ot_u32)(nextevent << OT_GPTIM_OVERSAMPLE);
    RTC->CR        |= RTC_CR_ALRAE;

    return (ot_u16)nextevent;
}



// void platform_set_ktim(ot_u16 value) {
//}



void systim_set_insertion(ot_u16 value) {
///@todo really should change value to ot_u32 or ot_timint
///
/// systim2 is often used for RF MAC timing.  RTC ALARMB is the implementation.
/// One caveat of the STM32WL is that it CANNOT send a SWIER to the RTC ISR(s)
/// the way other STM32s can.
///@note An EXTI line is commandeered to handle the value==0 case.

    if (value != 0) {
        ot_u32 rtc_ssr;
        rtc_ssr         = RTC->SSR;
        RTC->CR        &= ~RTC_CR_ALRBE;
        RTC->SCR        = RTC_SCR_CALRBF;
        RTC->ALRABINR   = rtc_ssr - (ot_u32)value;
        RTC->CR        |= RTC_CR_ALRBE;
    }
    else {
        BOARD_SWEXTI1_SET();
    }

}

void systim_enable_insertion() {
    RTC->CR |= RTC_CR_ALRBE;
}

void systim_disable_insertion() {
    RTC->CR &= ~RTC_CR_ALRBE;
}




ot_u32 systim_chronstamp(ot_u32* timestamp) {
    ot_u32 timer_cnt;

    timer_cnt = RTC->SSR;
    if (timestamp != NULL) {
        timer_cnt = *timestamp - timer_cnt;
    }

    return timer_cnt;
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




