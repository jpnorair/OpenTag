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
/**
  * @file       /apps/mpipe_bridge/app/app_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R100
  * @date       27 Sept 2014
  * @brief      Application Configuration File for MPipe Bridge Demo
  *
  * In your application code, you could include this file as <app/app_config.h>
  * but this is not recommended.  Instead, use:
  *
  * #include <otstd.h>
  * or
  * #include <otsys/config.h>
  *
  * As these are tested to properly bundle everything properly through the 
  * compiler.
  *
  ******************************************************************************
  */

#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#include <app/build_config.h>



/// Macro settings: ENABLED, DISABLED, NOT_AVAILABLE
#ifdef ENABLED
#   undef ENABLED
#endif
#define ENABLED  1

#ifdef DISABLED
#   undef DISABLED
#endif
#define DISABLED  0

#ifdef NOT_AVAILABLE
#   undef NOT_AVAILABLE
#endif
#define NOT_AVAILABLE   DISABLED



/** Top Level Device Featureset <BR>
  * ========================================================================<BR>
  * For more information on feature configuration, check the wiki:
  * http://www.indigresso.com/wiki/doku.php?id=opentag:configuration
  *
  * The "Device Featureset" documents compiled-in features.  By changing the
  * setting to ENABLED/DISABLED, you are changing the way OpenTag compiles.
  * Disabling features you don't need will make the build smaller -- sometimes
  * a lot smaller.  Total build sizes tend to range between 10 - 40 KB.
  * 
  * Main device features are ultimately summarized in the DEV_FEATURES_BITMAP
  * constant, defined at the bottom of the section.  This 32 bit bitmap is 
  * converted into BASE64 along with the firmware type (OpenTag) and the version
  * and stored in the "Firmware Version" element of ISF 1 (Device Features).
  * By reading some ISF's (especially Device Features and Protocol List), a 
  * DASH7 gateway can figure out exactly what capabilities this device has.
  */
#define OT_PARAM(VAL)                   OT_PARAM_##VAL

// User/App task lists: comment-out if none.  
// Use comma separated with TASK_xxx nomenclature (e.g. TASK_0, TASK_1, TASK_2)
//#define OT_PARAM_USER_EXOTASKS
#define OT_PARAM_USER_KERNELTASKS       TASK_external

#define OT_PARAM_VLFPS                  3                                   // Number of files that can be open simultaneously
#define OT_PARAM_SESSION_DEPTH          4                                   // Max simultaneous sessions (i.e. tasks)
#define OT_PARAM_BUFFER_SIZE            2560                                // Typically, must be at least 512 bytes    
#define OT_PARAM_KERNEL_LIMIT           -1                                  // Maximum ticks between kernel calls (if<=0, no limit)

#define OT_FEATURE(VAL)                 OT_FEATURE_##VAL
#define OT_FEATURE_SERVER               ENABLED                             // "Server" is a build containing the OpenTag stack
#define OT_FEATURE_CLIENT               (OT_FEATURE_SERVER != ENABLED)      // "Client" is a command console (typ. PC)
#define OT_FEATURE_CAPI                 ENABLED                             // "otapi" C function usage in server-side apps
#define OT_FEATURE_C_SERVER             (OT_FEATURE_CAPI)                   // "otapi" C function usage in server-side apps
#define OT_FEATURE_DASHFORTH            NOT_AVAILABLE                       // DASHFORTH Applet VM (server-side), or JIT (client-side)
#define OT_FEATURE_LOGGER               ENABLED                             // Mpipe-based data logging & printing
#define OT_FEATURE_ALP                  (ENABLED || (OT_FEATURE_CLIENT))    // Application Layer Protocol Support
#define OT_FEATURE_ALPAPI               (ENABLED && (OT_FEATURE_ALP))       // Application Layer Protocol callable API's
#define OT_FEATURE_MPIPE                ENABLED
#define OT_FEATURE_NDEF                 (OT_FEATURE_MPIPE)                  // NDEF wrapper for Messaging API
#define OT_FEATURE_VEELITE              ENABLED                             // Veelite DASH7 File System
#define OT_FEATURE_VLFPS                OT_PARAM_VLFPS
#define OT_FEATURE_VLNVWRITE            ENABLED                             // File writes in Veelite
#define OT_FEATURE_VLNEW                ENABLED                             // File create/delete in Veelite
#define OT_FEATURE_VLRESTORE            DISABLED                            // File restore in Veelite
#define OT_FEATURE_VL_SECURITY          NOT_AVAILABLE                       // AES128 on pre-shared key, for stored files
#define OT_FEATURE_DLL_SECURITY         DISABLED                            // AES128 on pre-shared key, for data-link
#define OT_FEATURE_NL_SECURITY          NOT_AVAILABLE                       // Network Layer Security & key exchange
#define OT_FEATURE_SENSORS              NOT_AVAILABLE                       // (formal, spec-based sensor config)
#define OT_FEATURE_LF                   NOT_AVAILABLE                       // Optional LF interface for event generation
#define OT_FEATURE_HF                   NOT_AVAILABLE                       // Optional HF interface for event generation
#define OT_FEATURE_CRC_TXSTREAM         ENABLED                             // Streams CRC computation inline with encoding
#define OT_FEATURE_CRC_RXSTREAM         ENABLED                             // Streams CRC computation inline with encoding
#define OT_FEATURE_TIME                 DISABLED                             // Do you have a precise 32768 Hz clock?
#define OT_FEATURE_M1                   NOT_AVAILABLE                       // Mode 1 Featureset: Generally not implemented
#define OT_FEATURE_M2                   DISABLED                            // Mode 2 Featureset: Implemented
#define OT_FEATURE_SESSION_DEPTH        OT_PARAM_SESSION_DEPTH
#define OT_FEATURE_BUFFER_SIZE          OT_PARAM_BUFFER_SIZE
#define OT_FEATURE_EXT_TASK             ENABLED
#define OT_FEATURE_SYSKERN_CALLBACKS    DISABLED                            // Dynamic Kernel Callbacks (panic, sleep, etc)
#define OT_FEATURE_SYSTASK_CALLBACKS    DISABLED                            // Dynamic Task callbacks
#define OT_FEATURE_DLLRF_CALLBACKS      DISABLED                            // Dynamic RF Init, Terminate Callbacks
#define OT_FEATURE_MPIPE_CALLBACKS      DISABLED                            // Signal callbacks from MPIPE
#define OT_FEATURE_M2NP_CALLBACKS       DISABLED                            // Signal callbacks from Network (M2NP) layer
#define OT_FEATURE_M2QP_CALLBACKS       DISABLED                            // Signal callbacks from Transport (M2QP) layer



// Legacy definitions for Top Level Featureset (Deprecated)
#define M1_FEATURESET                   OT_FEATURE_M1
#define M2_FEATURESET                   OT_FEATURE_M2
#define LF_FEATURESET                   OT_FEATURE_LF


/// Logging Features (only available if C Server is enabled)
/// Somewhat defunct, legacy features
#define LOG_FEATURE(VAL)                ((LOG_FEATURE_##VAL) && (OT_FEATURE_LOGGER))
#define LOG_FEATURE_FAULTS              ENABLED                             // Logs System Faults (errors that cause reset)
#define LOG_FEATURE_FAILS               ENABLED                             // Logs System Failures (detected glitches)                
#define LOG_FEATURE_RESPONSES           ENABLED
#define LOG_FEATURE_ANY                 OT_FEATURE_LOGGER
#define LOG_METHOD_DEFAULT              0                                   // Logging over NDEF+MPIPE, using OTAPI_logger.c
#define LOG_METHOD_SOMETHINGELSE        1                                   // Just an example
#define LOG_METHOD                      LOG_METHOD_DEFAULT


/// Mode 2 Features:    
/// These are generally handled by the ISF settings files, but these defines 
/// can limit scope of the compilation if you are trying to optimize the build.
#define M2_FEATURE(VAL)                 ((M2_FEATURE_##VAL) && (M2_FEATURESET))
#define M2_PARAM(VAL)                   (M2_PARAM_##VAL)
#define M2_FEATURE_RTCSLEEP             DISABLED
#define M2_FEATURE_RTCHOLD              DISABLED
#define M2_FEATURE_RTCBEACON            DISABLED
#define M2_FEATURE_GATEWAY              DISABLED                             // Gateway device mode
#define M2_FEATURE_SUBCONTROLLER        DISABLED                             // Subcontroller device mode
#define M2_FEATURE_ENDPOINT             DISABLED                             // Endpoint device mode
#define M2_FEATURE_M2DP                 DISABLED                            // Datastreams & associated commands
#define M2_FEATURE_DATASTREAM           M2_FEATURE_M2DP
#define M2_FEATURE_FECTX                DISABLED  /* test */                          // FEC support for transmissions
#define M2_FEATURE_FECRX                DISABLED  /* test */                          // FEC support for receptions
#define M2_FEATURE_LEGACY               NOT_AVAILABLE                       // Legacy (Mode 1) channel (ch 01)
#define M2_FEATURE_NORMAL               DISABLED                             // Low-speed channels (ch 1x, 9x)
#define M2_FEATURE_TURBO                DISABLED                             // High-speed channels (ch 2x, Ax)
#define M2_FEATURE_AUTOSCALE            DISABLED                            // Adaptive TX power fall-off algorithm
#define M2_FEATURE_BEACONS              DISABLED                             // Automated Beacon transmissions
#define M2_PARAM_BEACON_TCA             12                                  // Ticks to do CSMA for Beacons
#define M2_PARAM_MI_CHANNELS            1                                   // Multi-input channel support, e.g. "MIMO" (1-8)
#define M2_PARAM_MAXFRAME               255                                 // Max supported frame length in bytes (127-255)
#if (M2_FEATURE_M2DP == ENABLED)
#    define M2_PARAM_MFPP             1                                     // Max Frames Per Packet (1-255, partly device-dependent)
#else
#    define M2_PARAM_MFPP             1                                     // MFPP always 1 when M2DP is DISABLED (don't change)
#endif

// General derived constants
#define M2_FEATURE_MULTIFRAME           (M2_FEATURE_MFPP > 1)
#if ((M2_FEATURE_FECTX == ENABLED) || (M2_FEATURE_FECTX == ENABLED))
#    define M2_FEATURE_FEC              ENABLED
#else
#    define M2_FEATURE_FEC              DISABLED
#endif
#if ((M2_FEATURE_RTCSLEEP == ENABLED) || \
     (M2_FEATURE_RTCHOLD == ENABLED) || \
     (M2_FEATURE_RTCSBEACON == ENABLED) )
#    define M2_FEATURE_RTC_SCHEDULER    ENABLED
#else
#    define M2_FEATURE_RTC_SCHEDULER    DISABLED
#endif

/// Mode 1 Features: 
/// Just here for show.  Mode 1 is the legacy version of DASH7, and it is 
/// generally obsolete circa 2010.  I have no plans to implement Mode 1, but
/// someone else may want to do so.  Mode 1 is old, and it uses a PHY that is
/// not well suited to digital radios (and is naive in general, but I digress).
/// Most of these config settings are for PHY implementation in software.
#define M1_FEATURE(VAL)                 (OT_FEATURE_M1 && M1_FEATURE_##VAL)
#define M1_FEATURE_PERIOD_S             2.350                               // sec for wakeup tone interval
#define M1_FEATURE_PERIOD_MS            2350                                // ms for wakeup tone interval
#define M1_FEATURE_AUTOSYNC             DISABLED                            // Sync-word detection in HW
#define M1_FEATURE_INTEGRATED_PHY       DISABLED                            // PHY features in Radio HW
#define M1_FEATURE_INTEGRATED_MAC       DISABLED                            // MAC features in Radio HW (pipe dream)
#define M1_FEATURE_INTERFACE_SPI        DISABLED                            // MCU<-->Radio is via SPI 
#define M1_FEATURE_INTERFACE_TXSYNC     DISABLED                            // Synchronous RX bit generation
#define M1_FEATURE_INTERFACE_RXSYNC     DISABLED                            // Synchronous RX bit detection
#define M1_FEATURE_TUNE                 -1                                  // microseconds to offset input async RX bit



/// For the Device Features
#define DEV_FEATURES_BITMAP (   ((ot_u32)OT_FEATURE_SERVER << 31) | \
                                ((ot_u32)OT_FEATURE_CAPI << 30) | \
                                ((ot_u32)OT_FEATURE_DASHFORTH << 29) | \
                                ((ot_u32)OT_FEATURE_LOGGER << 28) | \
                                ((ot_u32)OT_FEATURE_ALP << 27) | \
                                ((ot_u32)OT_FEATURE_NDEF << 26) | \
                                ((ot_u32)OT_FEATURE_VEELITE << 25) | \
                                ((ot_u32)OT_FEATURE_VLNVWRITE << 24) | \
                                ((ot_u32)OT_FEATURE_VLNEW << 23) | \
                                ((ot_u32)OT_FEATURE_VLRESTORE << 22) | \
                                ((ot_u32)OT_FEATURE_VL_SECURITY << 21) | \
                                ((ot_u32)OT_FEATURE_DLL_SECURITY << 20) | \
                                ((ot_u32)OT_FEATURE_NL_SECURITY << 19) | \
                                ((ot_u32)OT_FEATURE_SENSORS << 18) | \
                                ((ot_u32)OT_FEATURE_M2 << 15) | \
                                ((ot_u32)OT_FEATURE_M1 << 14) | \
                                ((ot_u32)OT_FEATURE_LF << 13) | \
                                ((ot_u32)OT_FEATURE_HF << 11) | \
                                ((ot_u32)OT_FEATURE_TIME << 7)       )




/** Filesystem constants, setup, and boundaries <BR>
  * ========================================================================<BR>
  * For this app, we are just using the default FS setup, which is located in
  * /apps/_common/
  */
#include <../_common/fs_default_config.h>

#endif 
