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
  * @file       /otplatform/stdc/platform_stdc.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2012
  * @brief      ISRs and hardware services abstracted by the platform module
  * @ingroup    Platform
  *
  * ISRs implemented in this file shall use only the Platform module macro
  * abstractions, not functions or register nomenclature specific to any given
  * hardware platform.
  *
  ******************************************************************************
  */

#include "OTAPI.h"
#include "OT_platform.h"

// OT low-level modules that need initialization
#include "veelite_core.h"
#include "mpipe.h"
#include "radio.h"

//#include "auth.h"         //should be initialized via system (sys_init())
//#include "session.h"      //should be initialized via system (sys_init())


flash_heap  platform_flash;



// Platform-specific crc table (same as normal CRC table)
#include "crc16_table.h"
ot_u16 p_crcval;
static const ot_u16 p_crctable[256] = { 
    CRCx00, CRCx01, CRCx02, CRCx03, CRCx04, CRCx05, CRCx06, CRCx07, 
    CRCx08, CRCx09, CRCx0A, CRCx0B, CRCx0C, CRCx0D, CRCx0E, CRCx0F, 
    CRCx10, CRCx11, CRCx12, CRCx13, CRCx14, CRCx15, CRCx16, CRCx17, 
    CRCx18, CRCx19, CRCx1A, CRCx1B, CRCx1C, CRCx1D, CRCx1E, CRCx1F, 
    CRCx20, CRCx21, CRCx22, CRCx23, CRCx24, CRCx25, CRCx26, CRCx27, 
    CRCx28, CRCx29, CRCx2A, CRCx2B, CRCx2C, CRCx2D, CRCx2E, CRCx2F, 
    CRCx30, CRCx31, CRCx32, CRCx33, CRCx34, CRCx35, CRCx36, CRCx37, 
    CRCx38, CRCx39, CRCx3A, CRCx3B, CRCx3C, CRCx3D, CRCx3E, CRCx3F, 
    CRCx40, CRCx41, CRCx42, CRCx43, CRCx44, CRCx45, CRCx46, CRCx47, 
    CRCx48, CRCx49, CRCx4A, CRCx4B, CRCx4C, CRCx4D, CRCx4E, CRCx4F, 
    CRCx50, CRCx51, CRCx52, CRCx53, CRCx54, CRCx55, CRCx56, CRCx57, 
    CRCx58, CRCx59, CRCx5A, CRCx5B, CRCx5C, CRCx5D, CRCx5E, CRCx5F, 
    CRCx60, CRCx61, CRCx62, CRCx63, CRCx64, CRCx65, CRCx66, CRCx67, 
    CRCx68, CRCx69, CRCx6A, CRCx6B, CRCx6C, CRCx6D, CRCx6E, CRCx6F, 
    CRCx70, CRCx71, CRCx72, CRCx73, CRCx74, CRCx75, CRCx76, CRCx77, 
    CRCx78, CRCx79, CRCx7A, CRCx7B, CRCx7C, CRCx7D, CRCx7E, CRCx7F, 
    CRCx80, CRCx81, CRCx82, CRCx83, CRCx84, CRCx85, CRCx86, CRCx87, 
    CRCx88, CRCx89, CRCx8A, CRCx8B, CRCx8C, CRCx8D, CRCx8E, CRCx8F, 
    CRCx90, CRCx91, CRCx92, CRCx93, CRCx94, CRCx95, CRCx96, CRCx97, 
    CRCx98, CRCx99, CRCx9A, CRCx9B, CRCx9C, CRCx9D, CRCx9E, CRCx9F, 
    CRCxA0, CRCxA1, CRCxA2, CRCxA3, CRCxA4, CRCxA5, CRCxA6, CRCxA7, 
    CRCxA8, CRCxA9, CRCxAA, CRCxAB, CRCxAC, CRCxAD, CRCxAE, CRCxAF, 
    CRCxB0, CRCxB1, CRCxB2, CRCxB3, CRCxB4, CRCxB5, CRCxB6, CRCxB7, 
    CRCxB8, CRCxB9, CRCxBA, CRCxBB, CRCxBC, CRCxBD, CRCxBE, CRCxBF, 
    CRCxC0, CRCxC1, CRCxC2, CRCxC3, CRCxC4, CRCxC5, CRCxC6, CRCxC7, 
    CRCxC8, CRCxC9, CRCxCA, CRCxCB, CRCxCC, CRCxCD, CRCxCE, CRCxCF, 
    CRCxD0, CRCxD1, CRCxD2, CRCxD3, CRCxD4, CRCxD5, CRCxD6, CRCxD7, 
    CRCxD8, CRCxD9, CRCxDA, CRCxDB, CRCxDC, CRCxDD, CRCxDE, CRCxDF, 
    CRCxE0, CRCxE1, CRCxE2, CRCxE3, CRCxE4, CRCxE5, CRCxE6, CRCxE7, 
    CRCxE8, CRCxE9, CRCxEA, CRCxEB, CRCxEC, CRCxED, CRCxEE, CRCxEF, 
    CRCxF0, CRCxF1, CRCxF2, CRCxF3, CRCxF4, CRCxF5, CRCxF6, CRCxF7, 
    CRCxF8, CRCxF9, CRCxFA, CRCxFB, CRCxFC, CRCxFD, CRCxFE, CRCxFF
}; 







//API wrappers
void otapi_poweron()    { platform_poweron(); }
void otapi_poweroff()   { platform_poweroff(); }
void otapi_init()       { platform_init_OT(); }
void otapi_exec()       { platform_ot_run(); }
void otapi_preempt()    { platform_ot_preempt(); }
void otapi_pause()      { platform_ot_pause(); }

#ifndef EXTF_otapi_led1_on
void otapi_led1_on() {
#if (BOARD_FEATURE(INVERT_TRIG1) == ENABLED)
    platform_trig1_low();
#else
    platform_trig1_high();
#endif
}
#endif

#ifndef EXTF_otapi_led2_on
void otapi_led2_on() {
#if (BOARD_FEATURE(INVERT_TRIG2) == ENABLED)
    platform_trig2_low();
#else
    platform_trig2_high();
#endif
}
#endif

#ifndef EXTF_otapi_led1_off
void otapi_led1_off() {
#if (BOARD_FEATURE(INVERT_TRIG1) == ENABLED)
    platform_trig1_high();
#else
    platform_trig1_low();
#endif
}
#endif

#ifndef EXTF_otapi_led2_off
void otapi_led2_off() {
#if (BOARD_FEATURE(INVERT_TRIG2) == ENABLED)
    platform_trig2_high();
#else
    platform_trig2_low();
#endif
}
#endif






/** Feature Configuration Macros <BR>
  * ========================================================================<BR>
  * These should be defined in apps/.../app_config.h.  If one or more are
  * missing, use the defaults.
  */
#ifndef OT_FEATURE_RTC
#   define OT_FEATURE_RTC       DISABLED
#endif
#ifndef OT_FEATURE_MPIPE
#   define OT_FEATURE_MPIPE     DISABLED
#endif
//#ifndef OT_FEATURE_STDGPIO
//#   define OT_FEATURE_STDGPIO   DISABLED
//#endif





/** Platform Data <BR>
  * ============================================================================
  */
platform_struct platform;

#if (OT_FEATURE(RTC) == ENABLED)
#   define RTC_ALARMS       0 //(ALARM_beacon + __todo_IS_STM32L__)
#   define RTC_OVERSAMPLE   0
    // RTC_OVERSAMPLE: min 0, max 16

    typedef struct {
        ot_bool active;
        ot_u16  mask;
        ot_u16  value;
    } rtcalarm;

    typedef struct {
#       if (RTC_OVERSAMPLE != 0)
            ot_u32      utc;
#       endif
#       if (RTC_ALARMS > 0)
            rtcalarm    alarm[RTC_ALARMS]
#       endif
    } otrtc_struct;

#   if ((RTC_OVERSAMPLE != 0) || (RTC_ALARMS > 0))
        otrtc_struct otrtc;
#   endif
#endif









/** Platform Interrupt Service Routines (ISR's) <BR>
  * ========================================================================<BR>
  */

// 1. User NMI Interrupt (segmentation faults, i.e. firmware faults)
OT_INTERRUPT void platform_usernmi_isr(void) {
}


// 2. System NMI Interrupt (bus errors and other more serious faults)
OT_INTERRUPT void platform_sysnmi_isr(void) {
}




// 4. Watchdog Interrupt
//#pragma vector=WDT_VECTOR
//__interrupt void Watchdog_ISR(void) {
//}




OT_INTERRUPT void platform_gptim_isr() {
    platform_ot_run();
}



void sub_juggle_rtc_alarm() {
}


OT_INTERRUPT void platform_rtc_isr() {
}









/** Platform Interrupt & Event Management Routines <BR>
  * ========================================================================<BR>
  */
void platform_disable_interrupts() {
}

void platform_enable_interrupts() {
}

void platform_ot_preempt() {
/// Manually kick the GPTIM interrupt flag in order to pre-empt the kernel.
/// Also, save the current value of the timer so that the kernel can subtract
/// whatever time passed since the last event.
    ot_u16 scratch;
    scratch = platform_get_gptim();
    platform_set_gptim(scratch);
    ///@todo do something here
}

void platform_ot_pause() {
    platform_ot_preempt();
    platform_flush_gptim();
}

void platform_ot_run() {
/// 1. Save the amount of time that just passed
/// 2. Put the timer into free-running upcounter to time kernel process
/// 3. Run System Kernel, which returns its next scheduled call
/// 4. Put the next scheduled call into the timer, and turn it back on
    ot_u16 next_event;
    ot_u16 elapsed_time;
    elapsed_time    = platform_get_gptim();
    next_event      = sys_event_manager( elapsed_time );

#   if (OT_PARAM(KERNEL_LIMIT) > 0)
        if (next_event > OT_PARAM(KERNEL_LIMIT))
            next_event = OT_PARAM(KERNEL_LIMIT);
#   endif

    platform_set_gptim( next_event );
}







/** Platform Startup and Shutdown Routines <BR>
  * ========================================================================<BR>
  */

void platform_poweron() {
    /// 2. Initialize OpenTag platform peripherals
    platform_init_busclk();
    platform_init_interruptor();

    platform_init_gptim(0);
    platform_init_gpio();
    platform_init_memcpy();
    platform_init_prand(0);

    /// 3. Initialize Low-Level Drivers (worm, mpipe)
    // Restore vworm (following save on shutdown)
    vworm_init();

    // Mpipe (message pipe) typically used for serial-line comm.
#   if (OT_FEATURE(MPIPE) == ENABLED)
        mpipe_init(NULL);
#   endif
}


void platform_poweroff() {
/// 2. Put any mirror data into the flash <BR>
/// 3. Save the vworm mapping table
    ISF_syncmirror();
    vworm_save();
}


void platform_init_OT() {
	buffers_init(); //buffers init must be first in order to do core dumps
	vl_init();      //Veelite init must be second
	radio_init();   //radio init third
	sys_init();     //system init last
	
#   if (defined(__DEBUG__) || defined(__PROTO__))
    /// If debugging, find the Chip ID and use 6 out of 8 bytes of it to yield
    /// the UID.  This ID might not be entirely unique -- technically, there is
    /// 1/65536 chance of non-uniqueness, but practically the chance is much
    /// lower, given the way chips are distributed.  For test/debug, this is 
    /// adequately unique.
    ///
    /// @note the ID is inserted via Veelite, so it is abstracted from the 
    /// file memory configuration of your board and/or app. 
    {
		vlFILE*   fpid;
		ot_u16*   hwid;
		uint64_t  number; 

        fpid    = ISF_open_su(1);
        number  = rand();
        hwid    = (ot_u16*)&number;

        for (i=2; i<8; i+=2) {
            vl_write(fpid, i, *hwid++);
        }
        vl_close(fpid);
    }
#   endif
}


void platform_fastinit_OT() {
    platform_init_OT();
}


void platform_init_busclk() {
}


void platform_init_periphclk() {
}


void platform_init_interruptor() {
}


void platform_init_gpio() {
}


struct itimerval tconfig;

void platform_init_gptim(ot_uint prescaler) {
    struct sigaction timer_action;
    timer_action.sa_handler = timer_handler;
    sigemptyset(&timer_action.sa_mask);
    timer_action.sa_flags = 0;
    sigaction( SIGALRM, &timer_action, NULL );
}


void platform_init_watchdog() {
    platform_reset_watchdog(64);
}


void platform_init_resetswitch() {
}



void platform_init_systick(ot_uint period) {
}



void platform_init_rtc(ot_u32 value) {
}


void platform_init_memcpy() { }








/** Platform Peripheral Access Routines <BR>
  * ========================================================================<BR>
  */

ot_u16 platform_get_gptim() {
/// Have to do a lot of hackery, because unix itimer is a downcounter with a
/// highly annoying setup

    struct itimerval gettime;
    int time1, time0;
    
    if (getitimer(ITIMER_REAL, &gettime) != 0) {
        fprintf(stderr, "ERROR: itimer cannot be get\n");
    }
    
    time0   = tconfig.it_value.tv_sec * 1024;
    time0  += tconfig.it_value.tv_usec / 976;
    time1   = gettime.it_value.tv_sec * 1024;
    time1  += gettime.it_value.tv_usec / 976;
    
    time1   = time0 - time1;
    time1  *= (time1 >= 0);
    
    return time1;
}

void platform_set_gptim(ot_u16 value) {
/// Have to do a lot of hackery, because unix itimer is a downcounter with a
/// highly annoying setup

    tconfig.it_interval.tv_sec  = 0;
    tconfig.it_interval.tv_usec = 0;
    tconfig.it_value.tv_sec     = (value/1024);
    tconfig.it_value.tv_usec    = (value & 1023) * 976;
    
    if (setitimer(ITIMER_REAL, &tconfig, NULL) != 0) {
        fprintf(stderr, "ERROR: itimer cannot be set\n");
    }
}

void platform_flush_gptim() {
    platform_set_gptim(65535);
}

void platform_run_watchdog() {

}

void platform_reset_watchdog(ot_u16 reset) {

}

void platform_enable_rtc() {

}

void platform_disable_rtc() {

}

ot_u32 platform_get_time() {
#if (OT_FEATURE(RTC) == ENABLED)
#   if (RTC_OVERSAMPLE == 0)
        return ///@todo find time register, if it exists
#   else
        return otrtc.utc;
#   endif
#else
    return 0;
#endif
}

void platform_set_time(ot_u32 utc_time) {
#if (RTC_OVERSAMPLE != 0)
    otrtc.utc   = utc_time;
#else
    ///@todo find time register, if it exists
#endif
}

void platform_set_rtc_alarm(ot_u8 alarm_i, ot_u16 mask, ot_u16 value) {
#if (OT_FEATURE(RTC) == ENABLED)
    otrtc.alarm[alarm_i].mask     = mask;
    otrtc.alarm[alarm_i].value    = value;
#endif
}

void platform_enable_rtc_alarm(ot_u8 alarm_id, ot_bool enable) {
#if (OT_FEATURE(RTC) == ENABLED)
    otrtc.alarm[alarm_i].active   = enable;
#endif
}






/** Platform Debug Triggers <BR>
  * ========================================================================<BR>
  * Triggers are optional pins mostly used for debugging.  Sometimes they are
  * hooked up to LEDs, logic probes, etc.  The platform module requires that at
  * two triggers exist (if triggers are defined at all).  More triggers can be
  * defined in the application code.
  */

#ifdef __DEBUG__
void platform_trig1_high() { }
void platform_trig1_low() { }
void platform_trig1_toggle() { }
void platform_trig2_high() { }
void platform_trig2_low() { }
void platform_trig2_toggle() { }

#else
void platform_trig1_high() { }
void platform_trig1_low() { }
void platform_trig1_toggle() { }
void platform_trig2_high() { }
void platform_trig2_low() { }
void platform_trig2_toggle() { }

#endif






/** Platform Default CRC Routine <BR>
  * ========================================================================<BR>
  * CC430 has a compliant, internal CRC16 engine.  This setup allows the ASCII
  * string "123456789" produce 0x29B1.
  */

ot_u16 platform_crc_init() { 
    p_crcval = 0xFFFF;
    return 0xFFFF;
}

ot_u16 platform_crc_block(ot_u8* block_addr, ot_int block_size) { 
    ot_u8* data     = block_addr;
    ot_u16 crc16    = 0xFFFF;

    while (block_size > 0) {
        crc16 = (crc16 << 8) ^ p_crctable[((crc16 >> 8) & 0xff) ^ *data++];
        block_size--;
    }
    return crc16;
}

void platform_crc_byte(ot_u8 databyte) { 
    p_crcval = (p_crcval << 8) ^ p_crctable[((p_crcval >> 8) & 0xff) ^ databyte];
}


ot_u16 platform_crc_result() { 
    return p_crcval;
}





/** Platform Random Number Generation Routines <BR>
  * ========================================================================<BR>
  * The platform must be able to compute a strong random number (via function
  * platform_rand()) and a "pseudo" random number (via platform_prand_u8()).
  */

void platform_rand(ot_u8* rand_out, ot_int bytes_out) {
    if (bytes_out > 0) {
        unsigned int    rand_data;
        int             loops;
        ot_u8*          rand_ptr;
        
        rand_data   = rand();
        rand_ptr    = (ot_u8*)&rand_data;
        loops       = (bytes_out + 3) >> 2;
        
        switch (bytes_out & 3) {
            case 0: do {    *rand_out++ = *rand_ptr++;
            case 3:         *rand_out++ = *rand_ptr++;
            case 2:         *rand_out++ = *rand_ptr++;
            case 1:         *rand_out++ = *rand_ptr++;
                            rand_data   = rand();
                            rand_ptr    = (ot_u8*)&rand_data;
                        } 
                        while (--loops > 0);
        }
    }
}



void platform_init_prand(ot_u16 seed) {
    srand(time(NULL));
}

ot_u8 platform_prand_u8() {
    return (ot_u8)(rand() & 0xFF);
}

ot_u16 platform_prand_u16() {
    return (ot_u8)(rand() & 0xFFFF);
}





/** Platform memcpy, memset routines <BR>
  * ========================================================================<BR>
  * Similar to standard implementation of "memcpy" and "memset"
  * Behavior is always blocking
  */

void platform_memcpy(ot_u8* dest, ot_u8* src, ot_int length) {
    memcpy(dest, src, length);
}



void platform_memset(ot_u8* dest, ot_u8 value, ot_int length) {
    memset(dest, value, length);
}







/** Platform Utility Functions <BR>
  * ========================================================================<BR>
  * Random crap
  */

void platform_delay(ot_uint n) { }


void platform_swdelay_ms(ot_uint n) {
    useconds_t n_us;
    n_us = n * 1000;
    usleep(n_us);
}


void platform_swdelay_us(ot_uint n) {
    usleep( (useconds_t)n );
}





