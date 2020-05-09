/* Copyright (c) 2017 JP Norair
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
 * @file ubx_gnss.h
 *
 * External Header for User API
 *
 * @author JP Norair
 */

#ifndef UBX_GNSS_H_
#define UBX_GNSS_H_

#include <board.h>
#include <otstd.h>

#if ( OT_FEATURE(GNSS) && BOARD_FEATURE(UBX_GNSS) )

///@todo For some reason, including syskern.h isn't doing the job.  
/// I can't figure out why, but it's possibly due to some naming collisions in the preprocessor.
#include <otsys/syskern.h>
#include <otsys/kernels/system_hicculp.h>


/// This is the data from NAV-PVT, which is stored to file
typedef struct {
    uint32_t    iTOW;       ///< GPS Time of Week [ms] 
    uint16_t    year;       ///< Year (UTC)
    uint8_t     month;      ///< Month, range 1..12 (UTC) 
    uint8_t     day;        ///< Day of month, range 1..31 (UTC) 
    uint8_t     hour;       ///< Hour of day, range 0..23 (UTC) 
    uint8_t     min;        ///< Minute of hour, range 0..59 (UTC) 
    uint8_t     sec;        ///< Seconds of minute, range 0..60 (UTC) 
    uint8_t     valid;      ///< Validity flags (see UBX_RX_NAV_PVT_VALID_...) 
    uint32_t    tAcc;       ///< Time accuracy estimate (UTC) [ns] 
    int32_t     nano;       ///< Fraction of second (UTC) [-1e9...1e9 ns] 
    uint8_t     fixType;    ///< GNSSfix type: 0 = No fix, 1 = Dead Reckoning only, 2 = 2D fix, 3 = 3d-fix, 4 = GNSS + dead reckoning, 5 = time only fix 
    uint8_t     flags;      ///< Fix Status Flags (see UBX_RX_NAV_PVT_FLAGS_...) 
    uint8_t     flags2;
    uint8_t     numSV;      ///< Number of SVs used in Nav Solution 
    int32_t     lon;        ///< Longitude [1e-7 deg] 
    int32_t     lat;        ///< Latitude [1e-7 deg] 
    int32_t     height;     ///< Height above ellipsoid [mm] 
    int32_t     hMSL;       ///< Height above mean sea level [mm] 
    uint32_t    hAcc;       ///< Horizontal accuracy estimate [mm] 
    uint32_t    vAcc;       ///< Vertical accuracy estimate [mm] 
    int32_t     velN;       ///< NED north velocity [mm/s]
    int32_t     velE;       ///< NED east velocity [mm/s]
    int32_t     velD;       ///< NED down velocity [mm/s]
    int32_t     gSpeed;     ///< Ground Speed (2-D) [mm/s] 
    int32_t     headMot;    ///< Heading of motion (2-D) [1e-5 deg] 
    uint32_t    sAcc;       ///< Speed accuracy estimate [mm/s] 
    uint32_t    headAcc;    ///< Heading accuracy estimate (motion and vehicle) [1e-5 deg] 
    uint16_t    pDOP;       ///< Position DOP [0.01] 
    uint16_t    reserved2;
    uint32_t    reserved3;
    int32_t     headVeh;    ///< (ubx8+ only) Heading of vehicle (2-D) [1e-5 deg] 
    uint32_t    reserved4;  ///< (ubx8+ only) 
} ubx_navpvt_t;



/** User hooks/messages
  * =======================================================================
  */

/** @brief Activates the UBX Task and UBX Hardware Receiver
  * @param dataready        (ot_sig2) callback function with code parameters.  See below.
  * @param onoff_interval   (ot_int) number of seconds for ON/OFF interval
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
void ubx_config(ot_sig2 dataready, ot_int onoff_interval);
void ubx_connect(void);

/** @brief Graceful (if required) disconnect of the UBX hardware and Task.
  * @param None  
  * @retval None
  *
  * The UBX task will keep running, updating the data memory.  The app should
  * call disconnect to make it stop.  When UBX is running, it is consuming lots
  * of power.
  */ 
void ubx_disconnect(void);


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
void ubx_agps_stamp(void);


ot_int ubx_latlon_sprint(ot_u8* dst, ot_s32 latlon);

ot_int ubx_distance_sprint(ot_u8* dst, ot_s32 distance);



/** Kernel Task Function
  * =======================================================================
  */
#define gnss_systask ubx_systask
void ubx_systask(ot_task task);



/** Platform Driver Hooks
  * =======================================================================
  */
void ubxdrv_isr(void);

void ubxdrv_rxsync_isr(void);



#endif

#endif
