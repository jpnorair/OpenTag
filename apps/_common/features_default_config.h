/* Copyright 2010-2014 JP Norair
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
  * @file       /apps/_common/features_default_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R101
  * @date       31 Aug 2014
  * @brief      Feature configuration for a typical app & platform
  *
  * #include <../_common/feathers_default_config.h>
  *
  ******************************************************************************
  */

#ifndef __FEATURES_DEFAULT_CONFIG_H
#define __FEATURES_DEFAULT_CONFIG_H


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

#define ALP(NAME)                       ALP_##NAME
#ifndef ALP_FILE_MGR
#   define ALP_FILE_MGR                 ENABLED
#endif
#ifndef ALP_LLAPI
#   define ALP_LLAPI                    DISABLED
#endif
#if ALP_SECURE_MGR
#   define ALP_SECURE_MGR               DISABLED
#endif
#ifndef ALP_LOGGER
#   define ALP_LOGGER                   ENABLED
#endif
#ifndef ALP_DASHFORTH
#   define ALP_DASHFORTH                DISABLED
#endif



#define OT_PARAM(VAL)                   OT_PARAM_##VAL
#define OT_FEATURE(VAL)                 OT_FEATURE_##VAL


// User/App task lists: comment-out if none.  
// Use comma separated with TASK_xxx nomenclature (e.g. TASK_0, TASK_1, TASK_2)
#ifndef OT_PARAM_EXOTASKS
#   define OT_PARAM_EXOTASKS            0
#   define OT_PARAM_EXOTASK_IDS         
#   define OT_PARAM_EXOTASK_HANDLES     
#endif
#ifndef OT_PARAM_KERNELTASKS
#   define OT_PARAM_KERNELTASKS         0
#   define OT_PARAM_KERNELTASK_IDS      
#   define OT_PARAM_KERNELTASK_HANDLES 
#endif
#ifndef OT_PARAM_USER_KERNELTASKS
#   define OT_PARAM_USER_KERNELTASKS    TASK_external
#endif
#ifndef OT_PARAM_VLFPS
#   define OT_PARAM_VLFPS               3                                   // Number of files that can be open simultaneously
#endif
#ifndef OT_PARAM_SESSION_DEPTH
#   define OT_PARAM_SESSION_DEPTH       4                                   // Max simultaneous sessions (i.e. tasks)
#endif
#ifndef OT_PARAM_BUFFER_SIZE
// 2x frame buffers plus one large ALP stream
#   define OT_PARAM_BUFFER_SIZE         (256+256+1024)                      // Typically, must be at least 512 bytes    
#endif
#ifndef OT_PARAM_KERNEL_LIMIT
#   define OT_PARAM_KERNEL_LIMIT        -1                                  // Maximum ticks between kernel calls (if<=0, no limit)
#endif



#ifndef OT_FEATURE_SERVER
#   define OT_FEATURE_SERVER            ENABLED                             // "Server" is a build containing the OpenTag stack
#endif
#ifndef OT_FEATURE_CLIENT
#   define OT_FEATURE_CLIENT            (OT_FEATURE_SERVER != ENABLED)      // "Client" is a command console (typ. PC)
#endif
#ifndef OT_FEATURE_M1
#   define OT_FEATURE_M1                NOT_AVAILABLE                       // Mode 1 Featureset: Generally not implemented
#endif
#ifndef OT_FEATURE_M2
#   define OT_FEATURE_M2                ENABLED                             // Mode 2 Featureset: Implemented
#endif
#ifndef OT_FEATURE_CAPI
#   define OT_FEATURE_CAPI              ENABLED                             // "otapi" C function usage in server-side apps
#endif
#ifndef OT_FEATURE_C_SERVER
#   define OT_FEATURE_C_SERVER          (OT_FEATURE_CAPI)                   // "otapi" C function usage in server-side apps
#endif
#ifndef OT_FEATURE_DASHFORTH
#   define OT_FEATURE_DASHFORTH         NOT_AVAILABLE                       // DASHFORTH Applet VM (server-side), or JIT (client-side)
#endif
#ifndef OT_FEATURE_LOGGER
#   define OT_FEATURE_LOGGER            ENABLED                             // Mpipe-based data logging & printing
#endif
#ifndef OT_FEATURE_ALP
#   define OT_FEATURE_ALP               (ENABLED || (OT_FEATURE_CLIENT))    // Application Layer Protocol Support
#endif
#ifndef OT_FEATURE_ALPAPI
#   define OT_FEATURE_ALPAPI            (ENABLED && (OT_FEATURE_ALP))       // Application Layer Protocol callable API's
#endif
#ifndef OT_FEATURE_ALPEXT
#   define OT_FEATURE_ALPEXT            ENABLED                             
#endif
#ifndef OT_FEATURE_MPIPE
#   define OT_FEATURE_MPIPE             ENABLED
#endif
#ifndef OT_FEATURE_NDEF
#   define OT_FEATURE_NDEF              DISABLED                            // NDEF wrapper for Messaging API
#endif
#ifndef OT_FEATURE_VEELITE
#   define OT_FEATURE_VEELITE           ENABLED                             // Veelite DASH7 File System
#endif
#ifndef OT_FEATURE_VLNVWRITE
#   define OT_FEATURE_VLNVWRITE         ENABLED                             // File writes in Veelite
#endif
#ifndef OT_FEATURE_VLNEW
#   define OT_FEATURE_VLNEW             ENABLED                             // File create/delete in Veelite
#endif
#ifndef OT_FEATURE_VLRESTORE
#   define OT_FEATURE_VLRESTORE         DISABLED                            // File restore in Veelite
#endif
#ifndef OT_FEATURE_VL_SECURITY
#   define OT_FEATURE_VL_SECURITY       NOT_AVAILABLE                       // AES128 on pre-shared key, for stored files
#endif
#ifndef OT_FEATURE_DLL_SECURITY
#   define OT_FEATURE_DLL_SECURITY      DISABLED                            // AES128 on pre-shared key, for data-link
#endif
#ifndef OT_FEATURE_NL_SECURITY
#   define OT_FEATURE_NL_SECURITY       NOT_AVAILABLE                       // Network Layer Security & key exchange
#endif
#ifndef OT_FEATURE_SENSORS
#   define OT_FEATURE_SENSORS           NOT_AVAILABLE                       // (formal, spec-based sensor config)
#endif
#ifndef OT_FEATURE_LF
#   define OT_FEATURE_LF                NOT_AVAILABLE                       // Optional LF interface for event generation
#endif
#ifndef OT_FEATURE_HF
#   define OT_FEATURE_HF                NOT_AVAILABLE                       // Optional HF interface for event generation
#endif
#ifndef OT_FEATURE_CRC_TXSTREAM
#   define OT_FEATURE_CRC_TXSTREAM      ENABLED                             // Streams CRC computation inline with encoding
#endif
#ifndef OT_FEATURE_CRC_RXSTREAM
#   define OT_FEATURE_CRC_RXSTREAM      ENABLED                             // Streams CRC computation inline with encoding
#endif
#ifndef OT_FEATURE_TIME
#   define OT_FEATURE_TIME              ENABLED                             // Do you have a precise 32768 Hz clock?
#endif
#ifndef OT_FEATURE_RF_LINKINFO
#   define OT_FEATURE_RF_LINKINFO       ENABLED
#endif
#ifndef OT_FEATURE_EXT_TASK
#   define OT_FEATURE_EXT_TASK          ENABLED
#endif
#ifndef OT_FEATURE_SYSKERN_CALLBACKS
#   define OT_FEATURE_SYSKERN_CALLBACKS DISABLED                            // Dynamic Kernel Callbacks (panic, sleep, etc)
#endif
#ifndef OT_FEATURE_SYSTASK_CALLBACKS
#   define OT_FEATURE_SYSTASK_CALLBACKS DISABLED                            // Dynamic Task callbacks
#endif
#ifndef OT_FEATURE_DLLRF_CALLBACKS
#   define OT_FEATURE_DLLRF_CALLBACKS   DISABLED                            // Dynamic RF Init, Terminate Callbacks
#endif
#ifndef OT_FEATURE_MPIPE_CALLBACKS
#   define OT_FEATURE_MPIPE_CALLBACKS   DISABLED                            // Signal callbacks from MPIPE
#endif
#ifndef OT_FEATURE_M2NP_CALLBACKS
#   define OT_FEATURE_M2NP_CALLBACKS    DISABLED                            // Signal callbacks from Network (M2NP) layer
#endif
#ifndef OT_FEATURE_M2QP_CALLBACKS
#   define OT_FEATURE_M2QP_CALLBACKS    DISABLED                            // Signal callbacks from Transport (M2QP) layer
#endif


// Legacy definitions for Top Level Featureset (Deprecated)
#define M1_FEATURESET                   OT_FEATURE_M1
#define M2_FEATURESET                   OT_FEATURE_M2
#define LF_FEATURESET                   OT_FEATURE_LF


/// Logging Features (only available if C Server is enabled)
/// Somewhat defunct, legacy features
#define LOG_FEATURE(VAL)                ((LOG_FEATURE_##VAL) && (OT_FEATURE_LOGGER))
#ifndef LOG_FEATURE_FAULTS
#   define LOG_FEATURE_FAULTS           ENABLED                             // Logs System Faults (errors that cause reset)
#endif
#ifndef LOG_FEATURE_FAILS
#   define LOG_FEATURE_FAILS            ENABLED                             // Logs System Failures (detected glitches)                
#endif
#ifndef LOG_FEATURE_RESPONSES
#   define LOG_FEATURE_RESPONSES        ENABLED
#endif

/// These are RFU
#define LOG_METHOD_DEFAULT              0                                   // Logging over NDEF+MPIPE, using OTAPI_logger.c
#define LOG_METHOD_SOMETHINGELSE        1                                   // Just an example
#define LOG_METHOD                      LOG_METHOD_DEFAULT


/// Mode 2 Features:    
/// These are generally handled by the ISF settings files, but these defines 
/// can limit scope of the compilation if you are trying to optimize the build.
#define M2_FEATURE(VAL)                 ((M2_FEATURE_##VAL) && (M2_FEATURESET))
#define M2_PARAM(VAL)                   (M2_PARAM_##VAL)

#ifndef M2_FEATURE_RTCSLEEP
#   define M2_FEATURE_RTCSLEEP          DISABLED
#endif
#ifndef M2_FEATURE_RTCHOLD
#   define M2_FEATURE_RTCHOLD           DISABLED
#endif
#ifndef M2_FEATURE_RTCBEACON
#   define M2_FEATURE_RTCBEACON         DISABLED
#endif
#ifndef M2_FEATURE_GATEWAY
#   define M2_FEATURE_GATEWAY           ENABLED                             // Gateway device mode
#endif
#ifndef M2_FEATURE_SUBCONTROLLER
#   define M2_FEATURE_SUBCONTROLLER     ENABLED                             // Subcontroller device mode
#endif
#ifndef M2_FEATURE_ENDPOINT
#   define M2_FEATURE_ENDPOINT          ENABLED                             // Endpoint device mode
#endif
#ifndef M2_FEATURE_M2DP
#   define M2_FEATURE_M2DP              DISABLED                            // Datastreams & associated commands
#endif
#ifndef M2_FEATURE_FECTX
#   define M2_FEATURE_FECTX             ENABLED                             // FEC support for transmissions
#endif
#ifndef M2_FEATURE_FECRX
#   define M2_FEATURE_FECRX             ENABLED                             // FEC support for receptions
#endif
#ifndef M2_FEATURE_AUTOSCALE
#   define M2_FEATURE_AUTOSCALE         DISABLED                            // Adaptive TX power fall-off algorithm
#endif
#ifndef M2_FEATURE_BEACONS
#   define M2_FEATURE_BEACONS           ENABLED                             // Automated Beacon transmissions
#endif
#ifndef M2_PARAM_BEACON_TCA
#   define M2_PARAM_BEACON_TCA          12                                  // Ticks to do CSMA for Beacons
#endif
#ifndef M2_PARAM_MI_CHANNELS
#   define M2_PARAM_MI_CHANNELS         1                                   // Multi-input channel support, e.g. "MIMO" (1-8)
#endif
#ifndef M2_PARAM_MAXFRAME
#   define M2_PARAM_MAXFRAME            255                                 // Max supported frame length in bytes (127-255)
#endif


///@todo these are subject to change
#define M2_FEATURE_NORMAL               ENABLED                             // Low-speed channels (ch 1x, 9x)
#define M2_FEATURE_TURBO                ENABLED                             // High-speed channels (ch 2x, Ax)



/// General derived constants
#if (!defined(M2_PARAM_MFPP) && M2_FEATURE_M2DP)
#   define M2_PARAM_MFPP                4                                   // Max Frames Per Packet (1-255, partly device-dependent)
#else
#   undef M2_PARAM_MFPP
#   define M2_PARAM_MFPP                1                                   // MFPP always 1 when M2DP is DISABLED (don't change)
#endif
#define M2_FEATURE_MULTIFRAME           (M2_PARAM_MFPP > 1)
#define M2_FEATURE_RTC_SCHEDULER        (M2_FEATURE_RTCSLEEP || M2_FEATURE_RTCHOLD || M2_FEATURE_RTCSBEACON)
#define M2_FEATURE_FEC                  (M2_FEATURE_FECTX || M2_FEATURE_FECTX)






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



#endif 
