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
  * @file       /otplatform/stm32l1xx/platform_gptim_STM32L.c
  * @author     JP Norair
  * @version    R101
  * @date       30 Aug 2013
  * @brief      Special GPTIM implementation for STM32L
  * @ingroup    Platform
  *
  * The STM32L is difficult to use with OpenTag, but with some tricks it is
  * possible to implement the GPTIM and Kernel timing features via the RTC HW.
  * However, there are read & write latency limitations with the RTC module
  * that cause performance and low-power bottlenecks.  So, GPTIM is implemented
  * as a system of multiple timers and variables that run in different modes.
  *
  ******************************************************************************
  */


#include "OT_platform.h"
#include "system.h"
gptim_struct gptim;






#if defined(__DEBUG__)
/** Debug Implementation <BR>
  * ========================================================================<BR>
  * When __DEBUG__ is defined, we use TIM9 and we don't send the system to 
  * STOP mode.  Using TIM9 for the GPTIM disallows STOP mode, but it allows
  * the breakpoints to stop the timer.  That is very important for debugging.
  */

void platform_isr_tim9() {
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
/// This is the DEBUG implementation that uses TIM9 at 1024 Hz rather than
/// the RTC hybrid method.

    /// 2. Configure the RTC.
    ///    <LI> WKUP is used as Advertising flood counter. </LI>
    ///    <LI> RTC clock output (idealy 1024 Hz) can be used as input to TIM10 </LI>
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
#   if (BOARD_FEATURE_LFXTAL == ENABLED)    
        RTC->PRER   = (31 << 16) | 0;
        //RTC->CALIBR = ...;    // Calibration could be here
#   else
    {   ot_u16 lsi_hz;
        ot_u16 remainder;
        lsi_hz      = platform_ext_lsihz();
        remainder   = (lsi_hz & 512) >> 9;
        lsi_hz    >>= 10;
        lsi_hz     += remainder;
        RTC->PRER   = (lsi_hz << 16) | 0;
    }
#   endif

    RTC->WUTR   = 0;

    // Re-enable RTC.  Wakeup timer is set to 1/1024 sec (1 tick).
    // ALARMB and wakeup interrupts are always on.  ALARMA is controlled by the
    // platform_...ktim() functions
    RTC->CR     = RTC_CR_WUTIE | b100;
    RTC->ISR    = 0;


    /// 1. In debug mode, we must configure TIM9 to act as GPTIM.
    gptim.flags     = 0;
    gptim.stamp1    = 0;
    gptim.stamp2    = 0;

    // Timer 9 is our timer
    NVIC->IP[(uint32_t)(TIM9_IRQn)]         = ((b0001) << 4);
    NVIC->ISER[((uint32_t)(TIM9_IRQn)>>5)]  = (1 << ((uint32_t)(TIM9_IRQn) & 0x1F));

    TIM9->DIER  = 0;                    // TIM interrupts unused
    TIM9->SR    = 0;                    // clear update flags
    TIM9->CCMR1 = 0;
    TIM9->PSC   = 15;
    TIM9->OR    = 0;

    // Timer Mode 2 with (1/8) prescale yields 4096 Hz
    TIM9->SMCR  = TIM_SMCR_ECE | TIM_SMCR_ETPS_DIV8;   

    // Set TIM9 OC interrupts
#   if (RF_FEATURE(MAC_TIMER) != ENABLED)
    TIM9->DIER  = TIM_DIER_CC1IE;
    TIM9->CCER  = TIM_CCER_CC1E | TIM_CCER_CC2E;    // Enable Kernel & RF MAC timers
#   else
    TIM9->DIER  = TIM_DIER_CC1IE;
    TIM9->CCER  = TIM_CCER_CC1E;                    // Only Enable Kernel Timer
#   endif    

    // Re-enable Timer with continuous mode and update disabled
    TIM9->ARR   = 65535;
    TIM9->CCR1  = 65535;
    TIM9->CR1   = (TIM_CR1_UDIS | TIM_CR1_CEN);
    TIM9->EGR   = TIM_EGR_UG;
}
#endif



ot_u32 platform_get_interval(ot_u32* timestamp) {
    ot_u16 timer_cnt;
    timer_cnt = TIM9->CNT >> 2;

    if (timestamp == NULL) {
        return (ot_u32)timer_cnt;
    }
    timer_cnt  -= (ot_u16)*timestamp;
    timer_cnt  &= 0x3FFF;
    return timer_cnt;
}


ot_u32 platform_get_ktim() {
    ot_u16 timer_cnt;
    timer_cnt   = (TIM9->CNT - (ot_u16)gptim.stamp1);
    timer_cnt >>= 2;
    return ((ot_u32)timer_cnt & 0x3FFF);
}

ot_u16 platform_next_ktim() {
    return ((gptim.stamp1 - TIM9->CNT) >> 2);
}

void platform_enable_ktim() {
    TIM9->DIER |= TIM_DIER_CC1IE;
}

void platform_disable_ktim() {
    TIM9->DIER &= ~TIM_DIER_CC1IE;
}

void platform_pend_ktim() {
    TIM9->EGR |= TIM_EGR_CC1G;
}

void platform_flush_ktim() {
    TIM9->DIER     &= ~TIM_DIER_CC1IE;
    gptim.stamp1    = TIM9->CNT;
}

ot_u16 platform_schedule_ktim(ot_u32 nextevent, ot_u32 overhead) {
/// This should only be called from the scheduler.  Note how this function 
/// implements tail-chaining
    if ( (ot_long)(nextevent-overhead) <= 0 ) {
        gptim.flags = 0;
        return 0;
    }
    gptim.flags     = GPTIM_FLAG_SLEEP;
    TIM9->DIER     &= ~TIM_DIER_CC1IE;
    gptim.stamp1    = TIM9->CNT;
    TIM9->CCR1      = gptim.stamp1 + (ot_u16)(nextevent << 2);
    TIM9->DIER     |= TIM_DIER_CC1IE;
    
    return (ot_u16)nextevent;
}



void platform_set_gptim2(ot_u16 value) {
/// gptim2 is often used for RF MAC timing.  It includes "value" = 0 protection 
/// because often a time-slot is started at position 0.
    TIM9->DIER     &= ~TIM_DIER_CC2IE;
    gptim.stamp2    = TIM9->CNT;
    
    if (value == 0) TIM9->EGR   = TIM_EGR_CC2G;
    else            TIM9->CCR2  = gptim.stamp2 + (ot_u16)(value << 2);
    
    TIM9->DIER     |= TIM_DIER_CC2IE;
}


void platform_enable_gptim2() {
    TIM9->DIER |= TIM_DIER_CC2IE;
}


void platform_disable_gptim2() {
    TIM9->DIER &= ~TIM_DIER_CC2IE;
    TIM9->SR   &= ~TIM_SR_CC2IF;
}








#else
/** Proto and Release Implementation <BR>
  * ========================================================================<BR>
  * Use the RTC-based implementation for __PROTO__ or __RELEASE__ builds
  */


/** GPTIM Subroutine Prototypes <BR>
  * ========================================================================<BR>
  */

ot_u32 sub_rtc2ticks();

ot_u32 sub_ticks2alarm(ot_u32 ticks);

#if (CC_SUPPORT == GCC)
ot_u32 sub_get_nextalarm(ot_u16 next) __attribute__((flatten));
#else
ot_u32 sub_get_nextalarm(ot_u16 next);
#endif








/** Platform ISRs implemented here <BR>
  * ========================================================================<BR>
  * <LI> platform_isr_rtcalarm() </LI>
  * <LI> platform_isr_tim9() </LI>
  */

void platform_isr_rtcalarm() {
/// The kernel timer uses ALARM-A, and on boards with radios that cannot do CCA
/// internally the MAC-Timer is mapped to ALARM-B.
    ot_u32 flags;
    
    flags       = RTC->ISR;
    RTC->ISR    = flags & ~(RTC_ISR_ALRBF | RTC_ISR_ALRAF); 
    flags      |= (EXTI->SWIER & (1<<17)) >> 8;    //shift to bit 9, i.e. ALRBF
    EXTI->PR    = (1<<17);
    
    /// ALARM B event: Radio MAC alarm
    if (flags & RTC_ISR_ALRBF) {
        RTC->CR &= ~RTC_CR_ALRBE;
        gptim_start_chrono();
        radio_mac_isr();
    }
    
    /// ALARM A event: Task is pending
    /// <LI> To update ALARM A later, we must disable it now.  The RTC TR and 
    ///        ALRM registers are blocked for at most 61 us coming-out of STOP
    ///        mode.  The task and scheduler runtimes will eat-up this delay. </LI>
    /// <LI> ALARM A is set and re-enabled via platform_set_ktim(), which is 
    ///        called by sys_event_manager() after a task is scheduled. </LI>
    if (flags & RTC_ISR_ALRAF) {
        RTC->CR     &= ~RTC_CR_ALRAE;
        gptim.flags  = GPTIM_FLAG_RTCBYPASS;    // a GPTIM flag
        gptim_restart_chrono();                         // flush the chrono

#       if (OT_PARAM_SYSTHREADS != 0)
        ///@todo I'll need to implement a check to see if this is a task-end or
        ///      task-start event
        sys_task_manager();
#       endif
    }

}



void platform_isr_tim9() {
/// TIM9 is used as the chronometer.  It actually doesn't use this interrupt.
}











/** Standard Platform functions implemented here <BR>
  * ========================================================================<BR>
  * <LI> platform_init_gptim()                      </LI>
  * <LI> ot_u32 platform_get_ktim(ot_u16* marker)   </LI>
  * <LI> void platform_flush_ktim()                 </LI>
  * <LI> ot_u16 platform_next_ktim()                </LI>
  * <LI> void platform_enable_ktim()                </LI>
  * <LI> void platform_disable_ktim()               </LI>
  * <LI> void platform_pend_ktim()                  </LI>
  * <LI> void platform_set_ktim(ot_u16 value)       </LI>
  * <LI> void platform_set_gptim2(ot_u16 value)     </LI>
  * <LI> void platform_enable_gptim2()              </LI>
  * <LI> void platform_disable_gptim2()             </LI>
  * <LI> ot_u16 platform_get_gptim()                </LI>
  */

#ifndef EXTF_platform_init_gptim
void platform_init_gptim(ot_uint prescaler) {
/// For STM32L, the RTC must be used as the Kernel Timer because it is the only
/// timer of any kind that can bring the system out of STOP and/or STANDBY
/// modes.  To make matters worse, the STM32L RTC has an inflexible and annoying
/// design.  But, at least STM32L has a fast hardware ALU needed to do the
/// time-base conversions.
/// <LI> The RTC is configured as a free-running counter (as you would expect),  
///        and the kernel timer is utilizing ALARM A.</LI>
/// <LI> Some radios have internal resources sufficient for performing common
///        requirements of GPTIM2 as MAC-Timer (e.g. SPIRIT1).  For these, the
///        MAC-Timer/GPTIM2 can be implemented simply as the RTC WAKEUP, even
///        without need for an interrupt. </LI>
/// <LI> Otherwise GPTIM2 is normally connected to ALARM B of the RTC.  </LI>
/// <LI> Yet in other setups, GPTIM2 can be connected to TIM9, 10, or 11, using
///        the LSI/LSE divided by 8 to yield 4096Hz timebase. </LI> 

#   if (RF_FEATURE(TXTIMER) != ENABLED)
#       define _IE_ALARMB   RTC_CR_ALRBIE
#       define _IE_WAKEUP   RTC_CR_WUTIE
#   else
#       define _IE_ALARMB   0
#       define _IE_WAKEUP   0
#   endif
    
    /// 2. Configure RTC to use as main Kernel & MAC timer
    
    // Unlock RTC as a whole, put into INIT mode 
    RTC->WPR    = 0xCA;
    RTC->WPR    = 0x53;
    RTC->CR     = 0;
    RTC->ISR    = 0xFFFFFFFF;
    
    // Wait for init to be ready, the set to 1-ti (1s/1024) update period.
    while((RTC->ISR & RTC_ISR_INITF) == 0);
    RTC->TR     = 0;
#   if (BOARD_FEATURE_LFXTAL == ENABLED)    
        RTC->PRER   = (31 << 16) | 0;
#   else
    {   ot_u16 lsi_hz;
        ot_u16 remainder;
        lsi_hz      = platform_ext_lsihz();
        remainder   = (lsi_hz & 512) >> 9;
        lsi_hz    >>= 10;
        lsi_hz     += remainder;
        RTC->PRER   = (lsi_hz << 16) | 0;
    }

#   endif
    RTC->WUTR   = 0;

    // Calibration could be here
    //RTC->CALIBR = ...
    
    // Re-enable RTC.  Wakeup timer is set to 1/1024 sec (1 tick).
    // ALARMB and wakeup interrupts are always on.  ALARMA is controlled by the
    // platform_...ktim() functions
    RTC->CR     = _IE_WAKEUP | _IE_ALARMB | b100;
    RTC->ISR    = 0;

    
    /// 3. Configure TIM9 or 10 to use as a 32768 Hz chronometer.
    ///    It will get enabled on-demand by platform_flush_gptim().
        TIM9->CCR1 = 2;
      //TIM9->DIER = 0;
        TIM9->SR   = 0;
        TIM9->SMCR = TIM_SMCR_ECE;
        TIM9->PSC  = 7;
        TIM9->ARR  = 65535;
    
    
    /// 3. Clear local static variables
    gptim.evt_span  = 0;
    gptim.evt_stamp = 0;
    gptim.flags     = 0;
}
#endif

 




ot_u32 platform_get_ktim() {
/// platform_get_ktim() should be called only from the scheduler or preemptor:
/// sys_event_manager() and sys_preempt() respectively.  When it is called, the
/// Scheduler MUST be invoked before the system goes back to sleep.  This is 
/// way things will always work unless you are hacking the kernel and having
/// problems.  :)

    // RTC is not needed for this get.  Return the elapsed time read from the 
    // chronometer, plus whatever is on the marker.
    if (gptim.flags & GPTIM_FLAG_RTCBYPASS) {
        ot_u16 elapsed_chrono;
        elapsed_chrono = gptim_get_chrono() - gptim.chron_stamp;
        return gptim.evt_span + (elapsed_chrono >> (5-OT_GPTIM_SHIFT));
    }
    
    // RTC is needed for this get.  We need to wait for it to be ready to read.
    // It might be ready now, depending on how long the process has been 
    // between STOP mode and now (it must be >= 2 RTC Clocks, i.e. 61us).
    RTC->CR &= ~RTC_CR_ALRAE;
    //while ((RTC->ISR & RTC_ISR_RSF) == 0);
    
    while ((TIM9->SR & TIM_SR_CC1IF) == 0) {
        TIM9->DIER = TIM_DIER_CC1IE;
        ///@todo get WFE working... now just looping
        //__WFE();
    }
    TIM9->DIER = 0;

    // Compute the elapsed time from the RTC, but also synchronize the chrono
    // so that it can be used from now-on
    {   ot_long elapsed_time;
        elapsed_time = sub_rtc2ticks() - gptim.evt_stamp;
        if (elapsed_time < 0)
            elapsed_time += 86400;
        
        gptim.evt_span      = elapsed_time;
        gptim.chron_stamp   = gptim_get_chrono();
        gptim.flags        |= GPTIM_FLAG_RTCBYPASS;
        
        return (ot_u32)elapsed_time;
    }
}



void platform_flush_ktim() {
/// Only called in scheduler after platform_get_ktim, so BYPASS always ON
    gptim.chron_stamp   = gptim_get_chrono();
    gptim.evt_span      = 0;
    gptim.evt_stamp     = sub_rtc2ticks();
}


ot_u16 platform_next_ktim() {
/// This should only be used in the sys_powerdown() function.  This version is
/// indeed a bit kludgey, but it works for this purpose.
    return gptim.evt_span;
}

void platform_enable_ktim() {
    RTC->CR |= RTC_CR_ALRAIE; 
}

void platform_disable_ktim() {
    RTC->CR &= ~RTC_CR_ALRAIE; 
}


//void platform_pend_ktim() {
// Not supported on STM32L ... not used in the kernel anyway
//}



ot_u16 platform_schedule_ktim(ot_u32 nextevent, ot_u32 overhead) {
/// This should only be called from the scheduler.  Note how this function 
/// implements tail-chaining
    
    if ( (ot_long)(nextevent-overhead) <= 0 ) {
        gptim.flags &= ~GPTIM_FLAG_SLEEP;
        return 0;
    }
    
    // Wait for the Alarm flag to go high.  There is no easy way to do this 
    // with an interrupt or event.  This loop will only get used if you are 
    // running the system & scheduler very fast (above 16 MHz)
    while ((RTC->ISR & RTC_ISR_ALRAWF) == 0);
    
    // Set-up the Alarm.  gptim.evt_stamp is refreshed straight from the RTC.
    gptim.flags     = GPTIM_FLAG_SLEEP;
    gptim.evt_span  = nextevent;
    RTC->ALRMAR     = sub_get_nextalarm(nextevent);
    RTC->CR        |= RTC_CR_ALRAE;
    
    return (ot_u16)nextevent;
}


// void platform_set_ktim(ot_u16 value) {
//}



void platform_set_gptim2(ot_u16 value) {
/// gptim2 is often used for RF MAC timing.  It includes "value" = 0 protection 
/// because often a time-slot is started at position 0.  In that case, fuck the
/// annoying RTC, just force it with a software interrupt on the EXTI.

    // RTC alarm event is tied to EXTI line 17
    if (value == 0) {
        EXTI->SWIER |= (1<<17); 
    }
    
    // This should be familiar from platform_set_ktim()
    else {
        ot_u32 scratch;
        while ((RTC->ISR & RTC_ISR_ALRBWF) == 0);
        RTC->ALRMBR = sub_get_nextalarm(value);
        RTC->CR    |= RTC_CR_ALRBE;
    }    
}


void platform_enable_gptim2() {
    //OT_GPTIM->CCER |= TIM_CCER_CC2E;
    //EXTI->IMR      |= BOARD_GPTIM2_PIN;
}


void platform_disable_gptim2() {
    //OT_GPTIM->CCER &= ~TIM_CCER_CC2E;
    //EXTI->IMR      &= ~BOARD_GPTIM2_PIN;
}


// Not supported on STM32L
//ot_u16 platform_get_gptim() {
//    return TIM11->CNT;
//}














/** RTC Banging <BR>
  * ========================================================================<BR>
  * Unfortunately the STM32L does not have a suitable low-power timer.  Also
  * unfortunately, it has an inflexible RTC.  Fortunately, however, it has an 
  * ALU that can annihilate anything short of a PowerPC 6xx, so this seemingly

  * elaborate conversion actually happens in under 3us at 16 MHz.
  */

#define _BCD(T,O)   ((T<<4) | O)
#define _INT(I) ((I&15) + ((I>>4)*10))

const ot_u8 int2bcd_lut[60] = {
    _BCD(0,0),  _BCD(0,1),  _BCD(0,2),  _BCD(0,3),  _BCD(0,4),  
    _BCD(0,5),  _BCD(0,6),  _BCD(0,7),  _BCD(0,8),  _BCD(0,9), 
    _BCD(1,0),  _BCD(1,1),  _BCD(1,2),  _BCD(1,3),  _BCD(1,4),  
    _BCD(1,5),  _BCD(1,6),  _BCD(1,7),  _BCD(1,8),  _BCD(1,9), 
    _BCD(2,0),  _BCD(2,1),  _BCD(2,2),  _BCD(2,3),  _BCD(2,4),  
    _BCD(2,5),  _BCD(2,6),  _BCD(2,7),  _BCD(2,8),  _BCD(2,9), 
    _BCD(3,0),  _BCD(3,1),  _BCD(3,2),  _BCD(3,3),  _BCD(3,4),  
    _BCD(3,5),  _BCD(3,6),  _BCD(3,7),  _BCD(3,8),  _BCD(3,9), 
    _BCD(4,0),  _BCD(4,1),  _BCD(4,2),  _BCD(4,3),  _BCD(4,4),  
    _BCD(4,5),  _BCD(4,6),  _BCD(4,7),  _BCD(4,8),  _BCD(4,9), 
    _BCD(5,0),  _BCD(5,1),  _BCD(5,2),  _BCD(5,3),  _BCD(5,4),  
    _BCD(5,5),  _BCD(5,6),  _BCD(5,7),  _BCD(5,8),  _BCD(5,9)
};

// 60 + ((16-10)*(60/10 - 1)) = 90 
const ot_u8 bcd2int_lut[90] = {
    _INT(0),    _INT(1),    _INT(2),    _INT(3),    _INT(4),
    _INT(5),    _INT(6),    _INT(7),    _INT(8),    _INT(9),
    _INT(10),   _INT(11),   _INT(12),   _INT(13),   _INT(14),
    _INT(15),   _INT(16),   _INT(17),   _INT(18),   _INT(19),
    _INT(20),   _INT(21),   _INT(22),   _INT(23),   _INT(24),
    _INT(25),   _INT(26),   _INT(27),   _INT(28),   _INT(29),
    _INT(30),   _INT(31),   _INT(32),   _INT(33),   _INT(34),
    _INT(35),   _INT(36),   _INT(37),   _INT(38),   _INT(39),
    _INT(40),   _INT(41),   _INT(42),   _INT(43),   _INT(44),
    _INT(45),   _INT(46),   _INT(47),   _INT(48),   _INT(49),
    _INT(50),   _INT(51),   _INT(52),   _INT(53),   _INT(54),
    _INT(55),   _INT(56),   _INT(57),   _INT(58),   _INT(59),
    _INT(60),   _INT(61),   _INT(62),   _INT(63),   _INT(64),
    _INT(65),   _INT(66),   _INT(67),   _INT(68),   _INT(69),
    _INT(70),   _INT(71),   _INT(72),   _INT(73),   _INT(74),
    _INT(75),   _INT(76),   _INT(77),   _INT(78),   _INT(79),
    _INT(80),   _INT(81),   _INT(82),   _INT(83),   _INT(84),
    _INT(85),   _INT(86),   _INT(87),   _INT(88),   _INT(89)
    //_INT(90),   _INT(91),   _INT(92),   _INT(93),   _INT(94),
    //_INT(95),   _INT(96),   _INT(97),   _INT(98),   _INT(99),
    //_INT(100),  _INT(101),  _INT(102),  _INT(103),  _INT(104),
    //_INT(105),  _INT(106),  _INT(107),  _INT(108),  _INT(109),
    //_INT(110),  _INT(111),  _INT(112),  _INT(113),  _INT(114),
    //_INT(115),  _INT(116),  _INT(117),  _INT(118),  _INT(119),
    //_INT(120),  _INT(121),  _INT(122),  _INT(123),  _INT(124),
    //_INT(125),  _INT(126),  _INT(127)
};

ot_u32 sub_rtc2ticks() {
    ot_u32 time;
    ot_u32 ticks;
    time    = RTC->TR;                          // get present time counter
    ticks   = bcd2int_lut[(ot_u8)time];         // extract "seconds," convert to int
    time  >>= 8;
    ticks  += 60 * bcd2int_lut[(ot_u8)time];    // extract "minutes," add to seconds
    time  >>= 8;
    ticks  += 3600 * bcd2int_lut[time];         // extract "hours," add to seconds
    
    return ticks;
}

ot_u32 sub_ticks2alarm(ot_u32 ticks) {
    ot_u32 time;
    ot_u32 tmins;
    time    = (ticks/3600);             // get hours
    ticks   = ticks - (time*3600);      // get remaining seconds
    tmins   = (ticks/60);               // get minutes
    ticks   = ticks - (tmins*60);       // get remaining seconds
    time    = int2bcd_lut[time];        // put "hours" into [23:16]
    time  <<= 8;
    time   |= int2bcd_lut[tmins];       // put "minutes" into [15:8]
    time  <<= 8;
    time   |= int2bcd_lut[ticks];       // put "seconds" into [7:0]
    
    return (RTC_ALRMAR_MSK4 | time);
}


ot_u32 sub_get_nextalarm(ot_u16 next) {
    ot_long alarm_ticks;
    alarm_ticks = gptim.evt_stamp + next;
    if (alarm_ticks >= 86400)
        alarm_ticks -= 86400; 
        
    return sub_ticks2alarm(alarm_ticks);
}



ot_u32 platform_get_interval(ot_u32* timestamp) {
    ot_long timer_cnt;
    timer_cnt = sub_rtc2ticks();

    if (timestamp == NULL) {
        return ot_u32 timer_cnt;
    }
    
    timer_cnt -= *timestamp;
    if (timer_cnt < 0) {
        timer_cnt = 0-timer_cnt;
    }
    
    return timer_cnt;
}






/** STM32L GPTIM Chronometer functions <BR>
  * ========================================================================<BR>
  * Chronometer is required to fill-in the gaps of the RTC, while the device
  * is not in powerdown.
  */

  
void gptim_start_chrono() {
/// This function should be called in the ISR Wakeup hook of any ISR that is
/// able to bring the system out of STOP mode... except for the RTC ALARM ISR
/// which handles it in a special way.
    if (TIM9->CR1 == 0) {
        gptim_restart_chrono();
    }
}

void gptim_restart_chrono() {
/// This function should be called in the ALARM-A section of the RTC ALARM ISR.
    gptim.chron_stamp   = 0;
    TIM9->CR1           = (TIM_CR1_UDIS | TIM_CR1_CEN);
    TIM9->EGR           = TIM_EGR_UG;
}


inline ot_u16 gptim_get_chrono() {
    return TIM9->CNT;
}

inline void gptim_stop_chrono() {
    TIM9->CR1 = 0;
}


#endif
// END of ifdef __DEBUG__







/** Future Stuff
  */


#if 0 //(OT_FEATURE(RTC) == ENABLED)
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




