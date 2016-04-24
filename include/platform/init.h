/* Copyright 2013-14 JP Norair
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
  * @file       /include/platform/init.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Mar 2014
  * @brief      Platform Initialization functions
  * @ingroup    Platform
  *
  * OpenTag requires the implementation of certain platform functions.  Others
  * are optional (and they are marked as such).  When porting to a new platform,
  * these functions need to be implemented, as they are typically not hardware
  * agnostic.
  *
  * @todo Break-up OT_platform into its own directory of includes, perhaps like
  *       inc/otplatform/...
  ******************************************************************************
  */


#ifndef __PLATFORM_INIT_H
#define __PLATFORM_INIT_H

#include <otsys/types.h>
#include <otsys/config.h>
#include <app/build_config.h>



/** Platform Enablers <BR>
  * ========================================================================<BR>
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






/** Platform Low-Level Resource Initializers <BR>
  * ========================================================================<BR>
  * User code can usually avoid making any calls to these, because they are all
  * called internally by platform_init_OT().  Use that function instead.
  */

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


/** @brief Initializes a reset switch that may be used outside of OpenTag.
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * Not required for all platforms
  */
void platform_init_resetswitch();


void platform_init_memcpy();




#endif
