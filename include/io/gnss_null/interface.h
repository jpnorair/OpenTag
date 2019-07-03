/* Copyright 2009-2016 JP Norair
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
  * @file       /include/io/gnss_null/interface.h
  * @author     JP Norair
  * @version    R102
  * @date       27 Jan 2019
  * @brief      Stub Interface for GNSS IO Module
  * @defgroup   GNSS-Null (GNSS Stub functions)
  *
  ******************************************************************************
  */

#ifndef __IO_GNSSNULL_INTERFACE_H
#define __IO_GNSSNULL_INTERFACE_H

#include <board.h>
#include <otstd.h>

#if ( OT_FEATURE(GNSS) && BOARD_FEATURE(GNSSNULL) )

///@todo For some reason, including syskern.h isn't doing the job.  
/// I can't figure out why, but it's possibly due to some naming collisions in the preprocessor.
#include <otsys/syskern.h>
#include <otsys/kernels/system_hicculp.h>


/** User hooks/messages
  * =======================================================================
  */

/** @brief Activates the UBX Task and UBX Hardware Receiver
  * @param dataready        (ot_sig2) callback function with code parameters.  See below.
  * @param onoff_interval   (ot_u16) number of seconds for ON/OFF interval
  * @retval None
  *
  * The dataready callback will be sent to the user with two parameters.  The
  * first is an identifier of the source of the data.  It is always 0.  0 means
  * UBX GNSS Receiver.  The second parameter identifies the type of data that
  * was returned.  Here's the legend for that:
  * 
  * 0 - an error occured, timeout, no data.
  * 1 - NAV PVT
  * 2+ RFU
  */ 
void gnssnull_config(ot_sig2 dataready, ot_u16 onoff_interval);
void gnssnull_connect(void);

/** @brief Graceful (if required) disconnect of the UBX hardware and Task.
  * @param None  
  * @retval None
  *
  * The UBX task will keep running, updating the data memory.  The app should
  * call disconnect to make it stop.  When UBX is running, it is consuming lots
  * of power.
  */ 
void gnssnull_disconnect(void);


/** @brief Call this function when you update the AGPS file, from online source.
  * @param None
  * @retval None
  *
  * AGPS is very time dependent.  The UBX subsystem will automatically use the 
  * best startup model (hot, warm, cold) depending on the last time it had a
  * successful download of time and ephemeris data.  This data may be derived
  * from the satellites or supplied by the user.  Hot starts are much faster 
  * and use less power that cold or warm starts.
  *
  * All you need to do is call this function.  UBX task will map the call to 
  * system time.
  */ 
void gnssnull_agps_stamp(void);


ot_int gnssnull_latlon_sprint(ot_u8* dst, ot_s32 latlon);

ot_int gnssnull_distance_sprint(ot_u8* dst, ot_s32 distance);



/** Kernel Task Function
  * =======================================================================
  */
#define gnss_systask gnssnull_systask
void gnssnull_systask(ot_task task);



#endif

#endif
