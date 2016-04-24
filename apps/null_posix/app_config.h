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
  * @file       /apps/null_posix/app_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       31 July 2012
  * @brief      Application Configuration File for NULL POSIX
  *
  * Don't actually include this.  Include OTAPI.h or (OT_config.h + OT_types.h)
  * instead.
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
#define OT_PARAM_VLFPS                  3                                   // Number of files that can be open simultaneously
#define OT_PARAM_SESSION_DEPTH          4                                   // Max simultaneous sessions (i.e. tasks)
#define OT_PARAM_BUFFER_SIZE            4608                                // Typically, must be at least 512 bytes    
#define OT_PARAM_WATCHDOG_PERIOD        16                                  // Number of ticks before exception, following expected event return time
#define OT_PARAM_KERNEL_LIMIT           -1                                  // Maximum ticks between kernel calls (if<=0, no limit)

#define OT_FEATURE(VAL)                 OT_FEATURE_##VAL
#define OT_FEATURE_SERVER               DISABLED                            // "Server" is a build containing the OpenTag stack
#define OT_FEATURE_CLIENT               DISABLED                            // "Client" is a command console (typ. PC)
#define OT_FEATURE_CAPI                 DISABLED                            // "otapi" C function usage in server-side apps
#define OT_FEATURE_C_SERVER             OT_FEATURE_CAPI                     // "otapi" C function usage in server-side apps
#define OT_FEATURE_DASHFORTH            NOT_AVAILABLE                       // DASHFORTH Applet VM (server-side), or JIT (client-side)
#define OT_FEATURE_LOGGER               DISABLED                            // Mpipe-based data logging & printing
#define OT_FEATURE_ALP                  (DISABLED || OT_FEATURE_CLIENT)      // Application Layer Protocol (ALP) Support
#define OT_FEATURE_ALPAPI               (ENABLED && OT_FEATURE_ALP)         // Built-in ALP callable API's
#define OT_FEATURE_ALPEXT               (ENABLED && OT_FEATURE_ALP)         // Unknown ALP IDs get forwarded to User App
#define OT_FEATURE_MPIPE                DISABLED
#define OT_FEATURE_NDEF                 OT_FEATURE_MPIPE                    // NDEF wrapper for Messaging API
#define OT_FEATURE_VEELITE              DISABLED                            // Veelite DASH7 File System
#define OT_FEATURE_VLFPS                OT_PARAM_VLFPS
#define OT_FEATURE_VLNVWRITE            DISABLED                            // File writes in Veelite
#define OT_FEATURE_VLNEW                DISABLED                            // File create/delete in Veelite
#define OT_FEATURE_VLRESTORE            DISABLED                            // File restore in Veelite
#define OT_FEATURE_VL_SECURITY          NOT_AVAILABLE                       // AES128 on pre-shared key, for stored files
#define OT_FEATURE_DLL_SECURITY         DISABLED                            // AES128 on pre-shared key, for data-link
#define OT_FEATURE_NL_SECURITY          NOT_AVAILABLE                       // Network Layer Security & key exchange
#define OT_FEATURE_SENSORS              NOT_AVAILABLE                       // (formal, spec-based sensor config)
#define OT_FEATURE_LF                   NOT_AVAILABLE                       // Optional LF interface for event generation
#define OT_FEATURE_HF                   NOT_AVAILABLE                       // Optional HF interface for event generation
#define OT_FEATURE_AUTOCOPY             NOT_AVAILABLE                       // A DMA method for moving batch data (experimental)
#define OT_FEATURE_CRC_TXSTREAM         ENABLED                             // Streams CRC computation inline with encoding
#define OT_FEATURE_CRC_RXSTREAM         ENABLED                             // Streams CRC computation inline with encoding
#define OT_FEATURE_TIME                  DISABLED                            // Do you have a precise 32768 Hz clock?
#define OT_FEATURE_M1                   NOT_AVAILABLE                       // Mode 1 Featureset: Generally not implemented
#define OT_FEATURE_M2                   ENABLED                             // Mode 2 Featureset: Implemented
#define OT_FEATURE_EXTERNAL_EVENT       DISABLED
#define OT_FEATURE_SESSION_DEPTH        OT_PARAM_SESSION_DEPTH
#define OT_FEATURE_BUFFER_SIZE          OT_PARAM_BUFFER_SIZE    
#define OT_FEATURE_SYSKERN_CALLBACKS    DISABLED                             // Kernel callbacks from system layer
#define OT_FEATURE_SYSRF_CALLBACKS      DISABLED                             // RF Process callbacks from system layer
#define OT_FEATURE_SYSIDLE_CALLBACKS    DISABLED                            // Idle Process callbacks from system layer
#define OT_FEATURE_M2NP_CALLBACKS       DISABLED                             // Dynamic callbacks from Network (M2NP) layer
#define OT_FEATURE_M2QP_CALLBACKS       DISABLED                             // Dynamic callbacks from Transport (M2QP) layer
#define OT_FEATURE_MPIPE_CALLBACKS      DISABLED                            // Dynamic callbacks from MPIPE
#define OT_FEATURE_SW_WATCHDOG          DISABLED
#define OT_FEATURE_HW_WATCHDOG          DISABLED
#define OT_FEATURE_WATCHDOG_PERIOD      OT_PARAM_WATCHDOG_PERIOD                             



// Legacy definitions for Top Level Featureset (Deprecated)
#define M1_FEATURESET                   OT_FEATURE_M1
#define M2_FEATURESET                   OT_FEATURE_M2
#define LF_FEATURESET                   OT_FEATURE_LF


/// Logging Features (only available if C Server is enabled)
/// These control the things that are logged.  The way things are logged depends
/// on the implementation of the logging driver.
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
#define M2_FEATURE_GATEWAY              ENABLED                             // Gateway device mode
#define M2_FEATURE_SUBCONTROLLER        DISABLED                            // Subcontroller device mode
#define M2_FEATURE_ENDPOINT             DISABLED                            // Endpoint device mode
#define M2_FEATURE_BLINKER              DISABLED                            // Blinker device mode
#define M2_FEATURE_M2DP                 DISABLED                            // Datastreams & associated commands
#define M2_FEATURE_DATASTREAM           M2_FEATURE_M2DP
#define M2_FEATURE_FECTX                DISABLED  /* test */                          // FEC support for transmissions
#define M2_FEATURE_FECRX                DISABLED  /* test */                          // FEC support for receptions
#define M2_FEATURE_BASE                 ENABLED                             // Base channels (ch 00, 80)
#define M2_FEATURE_LEGACY               NOT_AVAILABLE                       // Legacy (Mode 1) channel (ch 01)
#define M2_FEATURE_NORMAL               ENABLED                             // Low-speed channels (ch 1x, 9x)
#define M2_FEATURE_TURBO                ENABLED                             // High-speed channels (ch 2x, Ax)
#define M2_FEATURE_BLINK                DISABLED                            // Blink channels (ch 3x, Bx)
#define M2_FEATURE_AUTOSCALE            DISABLED                            // Adaptive TX power fall-off algorithm
#define M2_FEATURE_BEACONS              ENABLED                             // Automated Beacon transmissions
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




/** Veelite Addressing constants
  * For each of the three types of virtual memory, plus mirroring, which is
  * supported by ISFB files.  Mirroring stores a copy of the IFSB data in
  * RAM (see veelite.h, veelite.c, veelite_core.h, veelite_core.c)
  */

#define VL_WORD             2
#define _ALLOC_OFFSET       (VL_WORD-1)
#define _ALLOC_SHIFT        1
#define _MIRALLOC_OFFSET    _ALLOC_OFFSET
#define _MIRALLOC_SHIFT     _ALLOC_SHIFT




/** Filesystem Overhead Data   <BR>
  * ========================================================================<BR>
  * The front of the filesystem stores file headers.  The amount below must
  * be coordinated with your linker file.
  */
#define OVERHEAD_START_VADDR                0x0000
#define OVERHEAD_TOTAL_BYTES                0x0360





/** ISFSB Files (Indexed Short File Series Block)   <BR>
  * ========================================================================<BR>
  * ISFSB Files are strings of ISF IDs that bundle/batch related ISF's.  ISFs
  * are not all the same length (max length = 16).  Also, make sure that the 
  * TOTAL_BYTES you allocate to the ISFSB bank corresponds to the amount set in
  * the linker file.
  */
/** ISFSB Files (Indexed Short File Series Block)   <BR>
  * ========================================================================<BR>
  * ISFSB Files are strings of ISF IDs that bundle/batch related ISF's.  ISFs
  * are not all the same length (max length = 16).  Also, make sure that the 
  * TOTAL_BYTES you allocate to the ISFSB bank corresponds to the amount set in
  * the linker file.
  */
#define ISFS_TOTAL_BYTES        			 0xA0
#define ISFS_NUM_M1_LISTS                    4
#define ISFS_NUM_M2_LISTS                    4
#define ISFS_NUM_EXT_LISTS                   0

#define ISFS_START_VADDR                     (OVERHEAD_START_VADDR + OVERHEAD_TOTAL_BYTES)
#define ISFS_NUM_USER_LISTS                  ISFS_NUM_EXT_LISTS
#define ISFS_NUM_STOCK_LISTS                 (ISFS_NUM_M1_LISTS + ISFS_NUM_M2_LISTS)
#define ISFS_NUM_LISTS                       (ISFS_NUM_STOCK_LISTS + ISFS_NUM_USER_LISTS)

#define ISFS_ID(VAL)                         ISFS_ID_##VAL
#define ISFS_ID_transit_data                 0x00
#define ISFS_ID_capability_data              0x01
#define ISFS_ID_query_results                0x02
#define ISFS_ID_hardware_fault               0x03
#define ISFS_ID_device_discovery             0x10
#define ISFS_ID_device_capability            0x11
#define ISFS_ID_device_channel_utilization   0x12
#define ISFS_ID_location_data                0x18

#define ISFS_MOD(VAL)                        b00100100

#define ISFS_LEN(VAL)                        ISFS_LEN_##VAL
#define ISFS_LEN_transit_data                3
#define ISFS_LEN_capability_data             4
#define ISFS_LEN_query_results               2
#define ISFS_LEN_hardware_fault              2
#define ISFS_LEN_device_discovery            2
#define ISFS_LEN_device_capability           3
#define ISFS_LEN_device_channel_utilization  4
#define ISFS_LEN_location_data               2

#define ISFS_MAX(VAL)                        ISFS_MAX_##VAL
#define ISFS_MAX_default                     16
#define ISFS_MAX_transit_data                4
#define ISFS_MAX_capability_data             4
#define ISFS_MAX_query_results               2
#define ISFS_MAX_hardware_fault              2
#define ISFS_MAX_device_discovery            2
#define ISFS_MAX_device_capability           3
#define ISFS_MAX_device_channel_utilization  4
#define ISFS_MAX_location_data               2

// The +1 and bit shifting assures that 
// the ALLOC value will be half-word (16 bit) aligned
#define ISFS_ALLOC(VAL)                      (((ISFS_MAX_##VAL + _ALLOC_OFFSET) >> _ALLOC_SHIFT) << _ALLOC_SHIFT)

#define ISFS_BASE(VAL)                       ISFS_BASE_##VAL
#define ISFS_BASE_transit_data               (ISFS_START_VADDR)
#define ISFS_BASE_capability_data            (ISFS_BASE_transit_data+ISFS_ALLOC(transit_data))
#define ISFS_BASE_query_results              (ISFS_BASE_capability_data+ISFS_ALLOC(capability_data))
#define ISFS_BASE_hardware_fault             (ISFS_BASE_query_results+ISFS_ALLOC(query_results))
#define ISFS_BASE_device_discovery           (ISFS_BASE_hardware_fault+ISFS_ALLOC(hardware_fault))
#define ISFS_BASE_device_capability          (ISFS_BASE_device_discovery+ISFS_ALLOC(device_discovery))
#define ISFS_BASE_device_channel_utilization (ISFS_BASE_device_capability+ISFS_ALLOC(device_capability))
#define ISFS_BASE_location_data              (ISFS_BASE_device_channel_utilization+ISFS_ALLOC(device_channel_utilization))
#define ISFS_BASE_NEXT                       (ISFS_BASE_location_data+ISFS_ALLOC(location_data))


#define ISFS_STOCK_HEAP_BYTES   (ISFS_ALLOC(transit_data) + \
                                    ISFS_ALLOC(capability_data) + \
                                    ISFS_ALLOC(query_results) + \
                                    ISFS_ALLOC(hardware_fault) + \
                                    ISFS_ALLOC(device_discovery) + \
                                    ISFS_ALLOC(device_capability) + \
                                    ISFS_ALLOC(device_channel_utilization) + \
                                    ISFS_ALLOC(location_data) )

#define ISFS_HEAP_BYTES         (ISFS_STOCK_HEAP_BYTES)






/** GFB (Generic File Block)
  * ========================================================================<BR>
  * GFB is a mostly unstructured data space.  You can change the definitions 
  * below to match your application & platform.  As always, make sure that the
  * TOTAL_BYTES setting matches that from your linker file.
  */
#define GFB_TOTAL_BYTES         0x0000
#define GFB_FILE_BYTES          0   //256
#define GFB_NUM_STOCK_FILES     0   //1
#define GFB_NUM_USER_FILES      0   //3

#define GFB_START_VADDR         (ISFS_START_VADDR + ISFS_TOTAL_BYTES)
#define GFB_NUM_FILES           (GFB_NUM_STOCK_FILES + GFB_NUM_USER_FILES)
#define GFB_HEAP_BYTES          (GFB_FILE_BYTES*GFB_NUM_STOCK_FILES)
#define GFB_MOD_standard        b00110100









/** ISFB (Indexed Short File Block)  <BR>
  * ========================================================================<BR>
  * The ISFB contains up to 256 files (IDs 0x00 to 0xFF), length <= 255 bytes.
  * As always, make sure that the TOTAL_BYTES allocated to the ISFB matches the 
  * value from your linker file.  
  *
  * If just using the base registry, the amount of bytes the ISFB requires is
  * typically between 512-1024, depending on how many features you are using.
  * 1.5KB is not a lot of space, but it is enough for the complete registry
  * plus at least two additional user ISFs.
  */
#define ISF_NUM_M1_FILES                        7
#define ISF_NUM_M2_FILES                        16
#define ISF_NUM_EXT_FILES                       2   // Usually at least 1
#define ISF_NUM_USER_FILES                      0  //max allowed user files

///@todo define this after mirror is alloc'ed
#define ISF_MIRROR_VADDR                        0x0000
#define ISF_TOTAL_BYTES         				0x600
#define ISF_START_VADDR                         (GFB_START_VADDR + GFB_TOTAL_BYTES)
#define ISF_NUM_STOCK_FILES                     (ISF_NUM_M1_FILES + ISF_NUM_M2_FILES + ISF_NUM_EXT_FILES)
#define ISF_NUM_FILES                           (ISF_NUM_STOCK_FILES + ISF_NUM_USER_FILES)





/** ISFB Structure    <BR>
  * ========================================================================<BR>
  * Here is the breakdown:
  * <LI> 0x00 to 0x0F: Mode 2 Configuration and Application Data Elements </LI>
  * <LI> 0x10 to 0x1F: Mode 1 & 2 Application Data </LI>
  * <LI> 0x20 to 0x7F: Reserved for future use </LI>
  * <LI> 0x80 to 0x9F: Mode 1 & 2 extended services data (not really used) </LI>
  * <LI> 0xA0 to 0xFE: Proprietary </LI>
  * <LI> 0xFF: Proprietary Data Extension </LI>
  *
  * Some files have allocations less than 255 bytes.  Many of the files from IDs 
  * 0x00 to 0x1F have limited allocations because they are config registers.
  *
  * There are several types of MACROS for handling ISFB constants.  To use, put
  * the name of the ISF into the argument, such as:
  * @c ISF_ID(network_settings) @c
  *
  * The macros are:
  * <LI> @c ISF_ID(file_name) @c :     File ID (0-255) </LI>
  * <LI> @c ISF_MOD(file_name) @c :    File Privilege bitmask (1 byte) </LI>
  * <LI> @c ISF_LEN(file_name) @c :    File Length (0-255) </LI>
  * <LI> @c ISF_MAX(file_name) @c :    Maximum Length of the file Data (0-255) </LI>
  * <LI> @c ISF_ALLOC(file_name) @c :  Allocated Bytes for file (0-256) </LI>
*/

/// Stock Mode 2 ISF File IDs               <BR>
/// ID's 0x00 to 0x0F:  Mode 2 only         <BR>
/// ID's 0x10 to 0xFF:  Mode 1 and Mode 2
#define ISF_ID(VAL)                             ISF_ID_##VAL
#define ISF_ID_network_settings                 0x00
#define ISF_ID_device_features                  0x01
#define ISF_ID_channel_configuration            0x02
#define ISF_ID_real_time_scheduler              0x03
#define ISF_ID_sleep_scan_sequence              0x04
#define ISF_ID_hold_scan_sequence               0x05
#define ISF_ID_beacon_transmit_sequence         0x06
#define ISF_ID_protocol_list                    0x07
#define ISF_ID_isfs_list                        0x08
#define ISF_ID_gfb_file_list                    0x09
#define ISF_ID_location_data_list               0x0A
#define ISF_ID_ipv6_addresses                   0x0B
#define ISF_ID_sensor_list                      0x0C
#define ISF_ID_sensor_alarms                    0x0D
#define ISF_ID_root_authentication_key          0x0E
#define ISF_ID_user_authentication_key          0x0F
#define ISF_ID_routing_code                     0x10
#define ISF_ID_user_id                          0x11
#define ISF_ID_optional_command_list            0x12
#define ISF_ID_memory_size                      0x13
#define ISF_ID_table_query_size                 0x14
#define ISF_ID_table_query_results              0x15
#define ISF_ID_hardware_fault_status            0x16
#define ISF_ID_port_254                         0xFE
#define ISF_ID_application_extension            0xFF

/// ISF Mirror Enabling: <BR>
/// ISFB files can be mirrored in RAM.  Set to 0/1 to Disable/Enable each file 
/// mirror.  Mirroring speeds-up file access, but it can consume a lot of RAM.
#define ISF_ENMIRROR(VAL)                       0


/// ISF file default privileges                                     <BR>
/// Mod Byte: EXrwxrwx                                              <BR>
/// root can always read & write, and he can execute when X is 1    <BR>
/// E:          data is encrypted in storage (not supported atm)    <BR>
/// X:          data is executable (a program)                      <BR>
/// 1st rwx:    read/write/exec for user                            <BR>
/// 2nd rwx:    read/write/exec for guest
#define ISF_MOD(VAL)                            ISF_MOD_##VAL
#define ISF_MOD_file_standard                   b00110100
#define ISF_MOD_network_settings                ISF_MOD_file_standard
#define ISF_MOD_device_features                 b00100100
#define ISF_MOD_channel_configuration           ISF_MOD_file_standard
#define ISF_MOD_real_time_scheduler             ISF_MOD_file_standard
#define ISF_MOD_sleep_scan_sequence             ISF_MOD_file_standard
#define ISF_MOD_hold_scan_sequence              ISF_MOD_file_standard
#define ISF_MOD_beacon_transmit_sequence        ISF_MOD_file_standard
#define ISF_MOD_protocol_list                   b00100100
#define ISF_MOD_isfs_list                       b00100100
#define ISF_MOD_gfb_file_list                   ISF_MOD_file_standard
#define ISF_MOD_location_data_list              b00100100
#define ISF_MOD_ipv6_addresses                  ISF_MOD_file_standard
#define ISF_MOD_sensor_list                     b00100100
#define ISF_MOD_sensor_alarms                   b00100100
#define ISF_MOD_root_authentication_key         b00000000
#define ISF_MOD_user_authentication_key         b00100000
#define ISF_MOD_routing_code                    ISF_MOD_file_standard
#define ISF_MOD_user_id                         ISF_MOD_file_standard
#define ISF_MOD_optional_command_list           b00100100
#define ISF_MOD_memory_size                     b00100100
#define ISF_MOD_table_query_size                b00100100
#define ISF_MOD_table_query_results             b00100100
#define ISF_MOD_hardware_fault_status           b00100100
#define ISF_MOD_port_254                        b00100100
#define ISF_MOD_application_extension           b00100100

/// ISF file default length: 
/// (that is, the initial length of the ISF)
#define ISF_LEN(VAL)                            ISF_LEN_##VAL
#define ISF_LEN_network_settings                10
#define ISF_LEN_device_features                 48
#define ISF_LEN_channel_configuration           18
#define ISF_LEN_real_time_scheduler             12
#define ISF_LEN_sleep_scan_sequence             0
#define ISF_LEN_hold_scan_sequence              8
#define ISF_LEN_beacon_transmit_sequence        16
#define ISF_LEN_protocol_list                   7
#define ISF_LEN_isfs_list                       12
#define ISF_LEN_gfb_file_list                   GFB_NUM_FILES
#define ISF_LEN_location_data_list              0
#define ISF_LEN_ipv6_addresses                  0
#define ISF_LEN_sensor_list                     16
#define ISF_LEN_sensor_alarms                   2
#define ISF_LEN_root_authentication_key         0
#define ISF_LEN_user_authentication_key         0
#define ISF_LEN_routing_code                    0
#define ISF_LEN_user_id                         0
#define ISF_LEN_optional_command_list           7
#define ISF_LEN_memory_size                     12
#define ISF_LEN_table_query_size                1
#define ISF_LEN_table_query_results             7
#define ISF_LEN_hardware_fault_status           3
#define ISF_LEN_port_254                        0
#define ISF_LEN_application_extension           0

/// Stock ISF file max data lengths (not aligned, just max)
#define ISF_MAX(VAL)                            ISF_MAX_##VAL
#define ISF_MAX_USER_FILE                       255
#define ISF_MAX_network_settings                10
#define ISF_MAX_device_features                 48
#define ISF_MAX_channel_configuration           48
#define ISF_MAX_real_time_scheduler             12
#define ISF_MAX_sleep_scan_sequence             0   //Not used by gateway
#define ISF_MAX_hold_scan_sequence              32  //8 scans
#define ISF_MAX_beacon_transmit_sequence        24  //3 beacons
#define ISF_MAX_protocol_list                   16  //16 protocols
#define ISF_MAX_isfs_list                       24  //24 isfs indices
#define ISF_MAX_gfb_file_list                   0   //8 gfb files
#define ISF_MAX_location_data_list              96  //8 location vertices (or 16 if using VIDs)
#define ISF_MAX_ipv6_addresses                  48
#define ISF_MAX_sensor_list                     16  //1 sensor
#define ISF_MAX_sensor_alarms                   2   //1 sensor
#define ISF_MAX_root_authentication_key         0
#define ISF_MAX_user_authentication_key         0
#define ISF_MAX_routing_code                    50
#define ISF_MAX_user_id                         60
#define ISF_MAX_optional_command_list           8
#define ISF_MAX_memory_size                     12
#define ISF_MAX_table_query_size                1
#define ISF_MAX_table_query_results             7
#define ISF_MAX_hardware_fault_status           3
#define ISF_MAX_port_254                        256
#define ISF_MAX_application_extension           256


/// BEGINNING OF AUTOMATIC ISF STUFF (You can probably leave it alone)

/// Stock ISF file memory & mirror allocations (aligned, typically 16bit)
#define ISF_ALLOC(VAL)          (((ISF_MAX_##VAL + _ALLOC_OFFSET) >> _ALLOC_SHIFT) << _ALLOC_SHIFT)
#define ISF_MIRALLOC(VAL)       (ISF_ENMIRROR(VAL) * (((ISF_MAX_##VAL + 2 + _MIRALLOC_OFFSET) >> _MIRALLOC_SHIFT) << _MIRALLOC_SHIFT))

/// ISF file base address computation
#define ISF_BASE(VAL)                           ISF_BASE_##VAL
#define ISF_BASE_network_settings               (ISF_START_VADDR)
#define ISF_BASE_device_features                (ISF_BASE_network_settings+ISF_ALLOC(network_settings))
#define ISF_BASE_channel_configuration          (ISF_BASE_device_features+ISF_ALLOC(device_features))
#define ISF_BASE_real_time_scheduler            (ISF_BASE_channel_configuration+ISF_ALLOC(channel_configuration))
#define ISF_BASE_sleep_scan_sequence            (ISF_BASE_real_time_scheduler+ISF_ALLOC(real_time_scheduler))
#define ISF_BASE_hold_scan_sequence             (ISF_BASE_sleep_scan_sequence+ISF_ALLOC(sleep_scan_sequence))
#define ISF_BASE_beacon_transmit_sequence       (ISF_BASE_hold_scan_sequence+ISF_ALLOC(hold_scan_sequence))
#define ISF_BASE_protocol_list                  (ISF_BASE_beacon_transmit_sequence+ISF_ALLOC(beacon_transmit_sequence))
#define ISF_BASE_isfs_list                      (ISF_BASE_protocol_list+ISF_ALLOC(protocol_list))
#define ISF_BASE_gfb_file_list                  (ISF_BASE_isfs_list+ISF_ALLOC(isfs_list))
#define ISF_BASE_location_data_list             (ISF_BASE_gfb_file_list+ISF_ALLOC(gfb_file_list))
#define ISF_BASE_ipv6_addresses                 (ISF_BASE_location_data_list+ISF_ALLOC(location_data_list))
#define ISF_BASE_sensor_list                    (ISF_BASE_ipv6_addresses+ISF_ALLOC(ipv6_addresses))
#define ISF_BASE_sensor_alarms                  (ISF_BASE_sensor_list+ISF_ALLOC(sensor_list))
#define ISF_BASE_root_authentication_key        (ISF_BASE_sensor_alarms+ISF_ALLOC(sensor_alarms))
#define ISF_BASE_user_authentication_key        (ISF_BASE_root_authentication_key+ISF_ALLOC(root_authentication_key))
#define ISF_BASE_routing_code                   (ISF_BASE_user_authentication_key+ISF_ALLOC(user_authentication_key))
#define ISF_BASE_user_id                        (ISF_BASE_routing_code+ISF_ALLOC(routing_code))
#define ISF_BASE_optional_command_list          (ISF_BASE_user_id+ISF_ALLOC(user_id))
#define ISF_BASE_memory_size                    (ISF_BASE_optional_command_list+ISF_ALLOC(optional_command_list))
#define ISF_BASE_table_query_size               (ISF_BASE_memory_size+ISF_ALLOC(memory_size))
#define ISF_BASE_table_query_results            (ISF_BASE_table_query_size+ISF_ALLOC(table_query_size))
#define ISF_BASE_hardware_fault_status          (ISF_BASE_table_query_results+ISF_ALLOC(table_query_results))
#define ISF_BASE_port_254                       (ISF_BASE_hardware_fault_status+ISF_ALLOC(hardware_fault_status))
#define ISF_BASE_application_extension          (ISF_BASE_port_254+ISF_ALLOC(port_254))
#define ISF_BASE_NEXT                           (ISF_BASE_application_extension+ISF_ALLOC(application_extension))

/// ISF file mirror address computation
#define ISF_MIRROR(VAL)                         (unsigned short)(((ISF_ENMIRROR_##VAL != 0) - 1) | (ISF_MIRROR_##VAL) )
#define ISF_MIRROR_network_settings             (ISF_MIRROR_VADDR)
#define ISF_MIRROR_device_features              (ISF_MIRROR_network_settings+ISF_MIRALLOC(network_settings))
#define ISF_MIRROR_channel_configuration        (ISF_MIRROR_device_features+ISF_MIRALLOC(device_features))
#define ISF_MIRROR_real_time_scheduler          (ISF_MIRROR_channel_configuration+ISF_MIRALLOC(channel_configuration))
#define ISF_MIRROR_sleep_scan_sequence          (ISF_MIRROR_real_time_scheduler+ISF_MIRALLOC(real_time_scheduler))
#define ISF_MIRROR_hold_scan_sequence           (ISF_MIRROR_sleep_scan_sequence+ISF_MIRALLOC(sleep_scan_sequence))
#define ISF_MIRROR_beacon_transmit_sequence     (ISF_MIRROR_hold_scan_sequence+ISF_MIRALLOC(hold_scan_sequence))
#define ISF_MIRROR_protocol_list                (ISF_MIRROR_beacon_transmit_sequence+ISF_MIRALLOC(beacon_transmit_sequence))
#define ISF_MIRROR_isfs_list                    (ISF_MIRROR_protocol_list+ISF_MIRALLOC(protocol_list))
#define ISF_MIRROR_gfb_file_list                (ISF_MIRROR_isfs_list+ISF_MIRALLOC(isfs_list))
#define ISF_MIRROR_location_data_list           (ISF_MIRROR_gfb_file_list+ISF_MIRALLOC(gfb_file_list))
#define ISF_MIRROR_ipv6_addresses               (ISF_MIRROR_location_data_list+ISF_MIRALLOC(location_data_list))
#define ISF_MIRROR_sensor_list                  (ISF_MIRROR_ipv6_addresses+ISF_MIRALLOC(ipv6_addresses))
#define ISF_MIRROR_sensor_alarms                (ISF_MIRROR_sensor_list+ISF_MIRALLOC(sensor_list))
#define ISF_MIRROR_root_authentication_key      (ISF_MIRROR_sensor_alarms+ISF_MIRALLOC(sensor_alarms))
#define ISF_MIRROR_user_authentication_key      (ISF_MIRROR_root_authentication_key+ISF_MIRALLOC(root_authentication_key))
#define ISF_MIRROR_routing_code                 (ISF_MIRROR_user_authentication_key+ISF_MIRALLOC(user_authentication_key))
#define ISF_MIRROR_user_id                      (ISF_MIRROR_routing_code+ISF_MIRALLOC(routing_code))
#define ISF_MIRROR_optional_command_list        (ISF_MIRROR_user_id+ISF_MIRALLOC(user_id))
#define ISF_MIRROR_memory_size                  (ISF_MIRROR_optional_command_list+ISF_MIRALLOC(optional_command_list))
#define ISF_MIRROR_table_query_size             (ISF_MIRROR_memory_size+ISF_MIRALLOC(memory_size))
#define ISF_MIRROR_table_query_results          (ISF_MIRROR_table_query_size+ISF_MIRALLOC(table_query_size))
#define ISF_MIRROR_hardware_fault_status        (ISF_MIRROR_table_query_results+ISF_MIRALLOC(table_query_results))
#define ISF_MIRROR_port_254                     (ISF_MIRROR_hardware_fault_status+ISF_MIRALLOC(hardware_fault_status))
#define ISF_MIRROR_application_extension        (ISF_MIRROR_port_254+ISF_MIRALLOC(port_254))
#define ISF_MIRROR_NEXT                         (ISF_MIRROR_application_extension+ISF_MIRALLOC(application_extension))

/// Total amount of stock ISF data stored in ROM
#define ISF_VWORM_STOCK_BYTES   (ISF_ALLOC(network_settings) + \
                                ISF_ALLOC(device_features) + \
                                ISF_ALLOC(channel_configuration) + \
                                ISF_ALLOC(real_time_scheduler) + \
                                ISF_ALLOC(sleep_scan_sequence) + \
                                ISF_ALLOC(hold_scan_sequence) + \
                                ISF_ALLOC(beacon_transmit_sequence) + \
                                ISF_ALLOC(protocol_list) + \
                                ISF_ALLOC(isfs_list) + \
                                ISF_ALLOC(gfb_file_list) + \
                                ISF_ALLOC(location_data_list) + \
                                ISF_ALLOC(ipv6_addresses) + \
                                ISF_ALLOC(sensor_list) + \
                                ISF_ALLOC(sensor_alarms) + \
                                ISF_ALLOC(root_authentication_key) + \
                                ISF_ALLOC(user_authentication_key) + \
                                ISF_ALLOC(routing_code) + \
                                ISF_ALLOC(user_id) + \
                                ISF_ALLOC(optional_command_list) + \
                                ISF_ALLOC(memory_size) + \
                                ISF_ALLOC(table_query_size) + \
                                ISF_ALLOC(table_query_results) + \
                                ISF_ALLOC(hardware_fault_status) + \
                                ISF_ALLOC(port_254) + \
                                ISF_ALLOC(application_extension))


#define ISF_VWORM_HEAP_BYTES    ISF_VWORM_STOCK_BYTES
#define ISF_HEAP_BYTES          ISF_VWORM_HEAP_BYTES
//#define ISF_VWORM_USER_BYTES   (ISF_ALLOC(USER_FILE) * ISF_NUM_USER_FILES)


/// Total amount of allocation to the Mirror
#define ISF_MIRROR_HEAP_BYTES   ((ISF_MIRROR_NEXT) - (ISF_MIRROR_VADDR))

/// END OF AUTOMATIC ISF STUFF 

#endif 
