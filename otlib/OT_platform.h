/* Copyright 2010-2011 JP Norair
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
  * @file       /OTlib/OT_platform.h
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Jan 2011
  * @brief      Platform Library functions
  * @defgroup   Platform (Platform Module)
  * @ingroup    Platform
  *
  * OpenTag requires the implementation of certain platform functions.  Others
  * are optional (and they are marked as such).  When porting to a new platform,
  * these functions need to be implemented, as they are typically not hardware
  * agnostic.
  ******************************************************************************
  */
  
  
#ifndef __OT_PLATFORM_H
#define __OT_PLATFORM_H

#include "OT_types.h"
#include "OT_config.h"
#include "platform_config.h"




typedef struct {
    ot_int error_code;
} platform_struct;

extern platform_struct platform;





void platform_rtc_isr();




/** @brief Routine for putting maskable interrupts on hold
  * @param None
  * @retval None
  * @ingroup Platform
  */
void platform_disable_interrupts();

/** @brief Routine for taking maskable interrupts off of hold
  * @param None
  * @retval None
  * @ingroup Platform
  */
void platform_enable_interrupts();


/** @brief The function that pauses OpenTag
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * Pauses OpenTag until you call platform_ot_run() again.  Use this to prevent
  * OpenTag from interrupting some code you have that might be important.  Be
  * careful when you use it, though!
  *
  * @note if you do not resume OpenTag within 65535 ticks of calling pause, you
  * may run into problems. 
  */
void platform_ot_pause();



/** @brief The function that invokes OpenTag -- typically 100% automatic.
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * This function will get buried into some ISR loop, usually of a timer that it
  * controls.  You can also call this function anytime you want to pre-empt the
  * internal task/event manager.  For example, if you create an ad-hoc dialog,
  * you will want to call platform_ot_run() immediately after creating it.
  */
void platform_ot_run();



/** @brief Pre-emption function for OpenTag
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * This function can be called by device drivers that are linked into the 
  * system module.  Basically, this means the only thing that should ever use
  * this function are the "rcevt" Radio I/O events implemented in system.c.
  * Calling it outside those confines shouldn't do anything bad, but it is not
  * necessary ... and I want to emphasize "shouldn't" ...
  */
void platform_ot_preempt();




/** Initialization functions 
  */

/** @brief Initializes HW at Power On
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * This function should be called when the chip has a cold or warm start.
  */
void platform_poweron();



/** @brief Safely shuts-down: call when resetting or shutting down.
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * Not all implementations will need this, but usually ones that write to 
  * flash will require calling it any time prior to turning-off the SRAM.
  */
void platform_poweroff();



/** @brief Initializes the platform for the OpenTag runtime
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * Call this prior to entering the OpenTag runtime in order to configure the
  * system for OpenTag.  If your application does not have functionality beyond
  * OpenTag (or if the functionality does not change the OpenTag settings), then
  * this function only needs to be run whenever the registers need to be 
  * refreshed.
  */
void platform_init_OT();




/** @brief An optimized method for initializing the platform for the OpenTag runtime
  * @param None
  * @retval None
  * @ingroup Platform
  * 
  * fast_initOT() is basically like init_OT(), except it is optimized for the 
  * platform.  It is especially useful for platforms that retain register 
  * settings (SRAM) through low power modes, since it can be minimized to meet
  * very specific needs.
  *
  * fast_initOT() is intended to be modified by the user to best fit the 
  * application.
  */
void platform_fastinit_OT();




/** @brief Initializes the bus clocks that OpenTag uses.
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * @note this is particularly relevant to ARM implementations, as it configures
  * the AHB bus(es).  On the MSP430, it configures all of the main clocks (MCLK,
  * SMCLK, ACLK).
  */
void platform_init_busclk();




/** @brief Initializes the peripheral clocks that OpenTag uses.  If there are
  *        peripherals already enabled which OpentTag does not use, it will not
  *        disable them.
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * Not every platform has a clock system that differentiates between bus clocks
  * and peripheral clocks.  In this case, init_OTPeriphClocks is unimplemented.
  *
  * @note this is particularly relevant to ARM implementations, as they have to
  * enable clocking of each peripheral from APB1, APB2, etc.
  */
void platform_init_periphclk();




/** @brief Initializes Global Interrupt Functionality for OpenTag resources.
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * The purpose is to prepare the system for OpenTag interrupts, and to shut
  * down interrupts that could affect the OpenTag runtime in unpredictable ways.
  */
void platform_init_interruptor();




/** @brief Initializes GPIO used by OpenTag, not including the radio module.
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * @note OpenTag doesn't usually need any GPIOs.  This function is here for
  * forward compatibility reasons, but is also helpful for debugging.
  */
void platform_init_gpio();




/** @brief Initializes the OpenTag general purpose timer.
  * @param prescaler    (16 bit) prescaler value.
  * @retval None
  * @ingroup Platform
  *
  * The formatting of the prescaler value can differ between platforms.  Please
  * refer to the implementation comments / documentation for more specific 
  * usage information for the timer_prescaler input parameter.
  */
#ifdef PLATFORM_POSIX
    void platform_init_gptim(ot_u16 prescaler, void (*timer_handler)(int));
#else 
    void platform_init_gptim(ot_u16 prescaler);
#endif



void platform_init_watchdog();



/** @brief Initializes a reset switch that may be used outside of OpenTag.
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * Not required for all platforms
  */
void platform_init_resetswitch();




/**
  * @brief Initializes a "system tick" peripheral, but doesn't begin running it.
  * @param period : system tick period
  * @retval None
  * @ingroup Platform
  *
  * Some platforms have a dedicated SysTick peripheral (the STM32).  Others
  * may not, but almost all have some form of watchdog functionality that may
  * be used for the same purpose.
  *
  * The input parameter "period" is often the milliseconds between SysTick
  * interrupts (or flagging), but in some platforms it has a different value.
  * Please check with the implementation comments / documentation to determine
  * the usage with your platform,
  */
void platform_init_systick(ot_uint period);




/** @brief Initializes a real time clock.
  * @param value: the default RTC value, in seconds
  * @retval None
  * @ingroup Platform
  * 
  * The RTC value is the number of seconds from 1/1/1990, 00:00:00
  */
void platform_init_rtc(ot_u32 value);


void platform_init_memcpy();



void platform_init_prand(ot_u16 seed);





/** @brief Gets the current GPTIM value, does not alter GPTIM behavior
  * @param None
  * @retval ot_u16      current timer value
  * @ingroup Platform
  */
ot_u16 platform_get_ktim();


/** @brief Returns the amount of timer clocks until the next interrupt
  * @param None
  * @retval ot_u16      timer clocks until next interrupt
  * @ingroup Platform
  */
ot_u16 platform_next_ktim();



void platform_pend_ktim();


/** @brief Enables GPTIM interrupt
  * @param None
  * @retval None
  * @ingroup Platform
  */
void platform_enable_ktim();



void platform_disable_ktim();



/** @brief Zeros GPTIM and sets an upper limit
  * @param value        (ot_u16)Number of ticks before next kernel event
  * @retval None
  * @ingroup Platform
  */
void platform_set_ktim(ot_u16 value);

void platform_set_gptim2(ot_u16 value);
void platform_set_gptim3(ot_u16 value);
void platform_set_gptim4(ot_u16 value);
void platform_set_gptim5(ot_u16 value);


/** @brief Zeros GPTIM, turns off interrupt, and puts into free-running
  * @param None
  * @retval None
  * @ingroup Platform
  */
void platform_flush_ktim();



void platform_run_watchdog();



void platform_reset_watchdog(ot_u16 reset);



void platform_enable_rtc();
void platform_disable_rtc();
ot_u32 platform_get_time();
void platform_set_time();


#if (OT_FEATURE(RTC) == ENABLED)
#define ALARM_sleep     (OT_FEATURE(ENDPOINT) == ENABLED)
#define ALARM_hold      (ALARM_sleep + 1)
#define ALARM_beacon    (ALARM_hold + (M2_FEATURE(BEACONS) == ENABLED))
#define ALARM_event     (ALARM_beacon + 1)
#endif

/** @brief Sets an indexed RTC alarm.
  * @param alarm_id     (ot_u8) Alarm index
  * @param task_id      (ot_u8) kernel task handle bound to alarm
  * @param offset       (ot_u16) offset into RTC Scheduler ISF where parameters exist
  * @retval None
  * @ingroup Platform
  *
  * This function is typically called from sys_refresh_scheduler().  You 
  * probably shouldn't call it from anywhere else.  In your apps, it is better
  * to use use sys_refresh_scheduler() instead.
  */
void platform_set_rtc_alarm(ot_u8 alarm_id, ot_u8 task_id, ot_u16 offset);


/** @brief Clears all RTC alarms
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * This function is typically called from sys_refresh_scheduler().  You 
  * probably shouldn't call it from anywhere else.  In your apps, it is better
  * to use use sys_refresh_scheduler() instead.
  */
void platform_clear_rtc_alarms();





/** @brief Trigger Functions for debug. (Pin control)
  * @param : None
  * @retval : None
  * @ingroup Platform
  */
void platform_trig1_high();
void platform_trig1_low();
void platform_trig1_toggle();
void platform_trig2_high();
void platform_trig2_low();
void platform_trig2_toggle();



#if (MCU_FEATURE(CRC) == ENABLED)
    ot_u16 platform_crc_init();
    ot_u16 platform_crc_block(ot_u8* block_addr, ot_int block_size);
    void platform_crc_byte(ot_u8 databyte);
    ot_u16 platform_crc_result();
#endif




/** @brief A random number generator.  Used within OpenTag.
  * @param rand_out     (ot_u8*) Pointer to the output random data
  * @param bytes_out    (ot_int) Number of random bytes to output
  * @retval none
  * @ingroup Platform
  *
  * @note The quality and performance of the random number generation are
  * implementation dependent.  It is a design goal for the random data output
  * to be FIPS 140-2 compliant and be computable in under 100us/byte.  Ideally,
  * it should be much faster, and one way to make it faster is to apply a zener 
  * diode, which guarantees more bits of noise per acquisition.  A good zener
  * circuit and a 200ksps 12 bit ADC can therefore provide 5us/byte.
  */
void platform_rand(ot_u8* rand_out, ot_int bytes_out);



/** @brief Quick and dirty 8 bit pseudo-random value
  * @param None
  * @retval ot_u8       8 bit pseudo random number
  * @ingroup Platform
  *
  * A quickly generated 8 bit random number, not recommended for crypto.
  */
ot_u8 platform_prand_u8();



/** @brief Quick and dirty 16 bit pseudo-random value
  * @param None
  * @retval ot_u16       16 bit pseudo random number
  * @ingroup Platform
  *
  * A quickly generated 16 bit random number, not recommended for crypto.
  */
ot_u16 platform_prand_u16();



/** @brief platform-specific memcpy, in some cases wraps to OS-level memcpy
  * @param  dest        (ot_u8*) destination memory address
  * @param  src         (ot_u8*) source memory address
  * @param  length      (ot_int) number of bytes to transfer/copy
  * @retval None
  * @ingroup Platform
  * @sa platform_memcpy_2()
  * @sa platform_memset()
  */
void platform_memcpy(ot_u8* dest, ot_u8* src, ot_int length);



/** @brief platform-specific half-word memcpy, in some cases wraps to OS-level memcpy
  * @param  dest        (ot_u8*) destination memory address
  * @param  src         (ot_u8*) source memory address
  * @param  length      (ot_int) number of half-words to transfer/copy
  * @retval None
  * @ingroup Platform
  * @sa platform_memcpy()
  * @sa platform_memset()
  */
void platform_memcpy_2(ot_u16* dest, ot_u16* src, ot_int length);



/** @brief platform-specific memset, in some cases wraps to OS-level memset
  * @param  dest        (ot_u8*) destination memory address
  * @param  value       (ot_u8) byte to put into memory (repeatedly)
  * @param  length      (ot_int) number of bytes to set
  * @retval None
  * @ingroup Platform
  * @sa platform_memcpy()
  * @sa platform_memcpy_2()
  */
void platform_memset(ot_u8* dest, ot_u8 value, ot_int length);




/** @brief Low Power Optimized Busywait function with 1 sti resolution
  * @param  sti         (ot_u16) Number of sti to block
  * @retval None
  * @ingroup Platform
  * @platform_swdelay_ms()
  * @platform_swdelay_us()
  * @platform_delay()
  *
  * platform_block() is a low-power optimizing blocking loop that does
  * not engage a watchdog timer.  In platform implementations that include
  * a programmable NMI timer, platform_block() will probably link to the
  * same code as platform_delay().  In other implementations, platform_delay()
  * might use a watchdog timer.  So, if you want to be certain that your
  * delay process is not using a watchdog, use platform_block().
  *
  * The timing resolution is approximately 1 sti = 1/32768s.  In many
  * implementations there is a minimum runtime of this function, due to
  * some setup overhead.  For example, on an MSP430F5 core, if sti<=10
  * the block loop will take about the same time (~300us).  Check the
  * implementation for more information.
  */
void platform_block(ot_u16 sti);




/** @brief Inserts a delay time via a SysTick mechanism
  * @param n        (ot_uint) Number of ticks before resuming.
  * @retval None
  * @ingroup Platform
  *
  * The "systick" is platform dependent and not always implemented
  *
  * @note the behavior of the system during SysTick wait state is implementation
  * dependent.  Typically, it uses the lowest-power sleep mode that allows the
  * SRAM to stay alive.
  */
void platform_delay(ot_uint n);




/** @brief Uses busywait loop to delay the processor for number of milliseconds
  * @param n : Number of milliseconds to delay, up to 65535 
  * @retval None
  * @ingroup Platform
  */
void platform_swdelay_ms(ot_uint n);




/** @brief Uses busywait loop to delay the processor for number of microseconds
  * @param n : Number of microseconds to delay, up to 65535
  * @retval None
  * @ingroup Platform
  */
void platform_swdelay_us(ot_uint n);




#endif
