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
  * @file       /Apps/Testbed_Protocol/app_config_TestbedProtocol.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       22 August 2011
  * @brief      Application Configuration File for Protocol Testbed
  *
  * Don't actually include this.  Include OT_config.h instead, which will 
  * include the appropriate application configuration file based on its setup.
  *
  ******************************************************************************
  */

#ifndef __APP_CONFIG_TESTBEDPROTOCOL_H
#define __APP_CONFIG_TESTBEDPROTOCOL_H

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

#define OT_PARAM(VAL)                   OT_PARAM_##VAL
#define OT_PARAM_SESSION_DEPTH          4                                   // Max simultaneous sessions (i.e. tasks)
#define OT_PARAM_BUFFER_SIZE            1024                                // Typically, must be at least 512 bytes    


/** Top Level Featureset
  */
#define OT_FEATURE(VAL)                 OT_FEATURE_##VAL
#define OT_FEATURE_SERVER               ENABLED                             // "Server" is a build containing the OpenTag stack
#define OT_FEATURE_CLIENT               (OT_FEATURE_SERVER != ENABLED)      // "Client" is a command console (typ. PC)
#define OT_FEATURE_CAPI                 ENABLED                             // "otapi" C function usage in server-side apps
#define OT_FEATURE_C_SERVER             (OT_FEATURE_CAPI)                   // "otapi" C function usage in server-side apps
#define OT_FEATURE_DASHFORTH            NOT_AVAILABLE                       // DASHFORTH Applet VM (server-side), or JIT (client-side)
#define OT_FEATURE_LOGGER               DISABLED
#define OT_FEATURE_ALP                  (DISABLED || OT_FEATURE_CLIENT)      // Application Layer Protocol Support
#define OT_FEATURE_ALPAPI               (DISABLED && OT_FEATURE_ALP)        // Application Layer Protocol callable API's
#define OT_FEATURE_MPIPE                DISABLED
#define OT_FEATURE_NDEF                 (OT_FEATURE_MPIPE)                  // NDEF wrapper for Messaging API
#define OT_FEATURE_VEELITE              ENABLED                             // Veelite DASH7 File System
#define OT_FEATURE_VLFPS                2                                   // Number of files that can be open simultaneously
#define OT_FEATURE_VLNEW                ENABLED                             // File create/delete in Veelite
#define OT_FEATURE_VLRESTORE            DISABLED                            // File restore in Veelite
#define OT_FEATURE_SENSORS              NOT_AVAILABLE                       // (formal, spec-based sensor config)
#define OT_FEATURE_SECURITY             NOT_AVAILABLE                       // (formal, spec-based security config)
#define OT_FEATURE_LF                   NOT_AVAILABLE                       // Optional LF interface for event generation
#define OT_FEATURE_HF                   NOT_AVAILABLE                       // Optional HF interface for event generation
#define OT_FEATURE_AUTOCOPY             NOT_AVAILABLE                       // A DMA method for moving batch data (experimental)
#define OT_FEATURE_CRC_TXSTREAM         ENABLED                             // Streams CRC computation inline with encoding
#define OT_FEATURE_CRC_RXSTREAM         ENABLED                             // Streams CRC computation inline with encoding
#define OT_FEATURE_RTC                  ENABLED
#define OT_FEATURE_M1                   NOT_AVAILABLE                       // Mode 1 Featureset: Generally not implemented
#define OT_FEATURE_M2                   ENABLED                             // Mode 2 Featureset: Implemented
#define OT_FEATURE_SYSTEM_CALLBACKS     ENABLED                             // Signal callbacks from system layer
#define OT_FEATURE_M2NP_CALLBACKS       ENABLED                             // Signal callbacks from Network (M2NP) layer
#define OT_FEATURE_M2QP_CALLBACKS       ENABLED                             // Signal callbacks from Transport (M2QP) layer
#define OT_FEATURE_MPIPE_CALLBACKS      ENABLED                             // Signal callbacks from MPIPE
#define OT_FEATURE_SW_WATCHDOG          DISABLED
#define OT_FEATURE_HW_WATCHDOG          DISABLED                                  



// Legacy definitions for Top Level Featureset (Deprecated)
#define M1_FEATURESET                   OT_FEATURE_M1
#define M2_FEATURESET                   OT_FEATURE_M2
#define LF_FEATURESET                   OT_FEATURE_LF


/// Logging Features (only available if C Server is enabled)
/// These control the things that are logged.  The way things are logged depends
/// on the implementation of the logging driver.
#define LOG_FEATURE(VAL)                (LOG_FEATURE_##VAL && OT_FEATURE_LOGGER)
#define LOG_FEATURE_FAULTS              ENABLED                             // Logs System Faults (errors that cause reset)
#define LOG_FEATURE_FAILS               ENABLED                             // Logs System Failures (detected glitches)                
#define LOG_FEATURE_ANY                 OT_FEATURE_LOGGER

#define LOG_METHOD_DEFAULT              0                                   // Logging over NDEF+MPIPE, using OTAPI_logger.c
#define LOG_METHOD_SOMETHINGELSE        1                                   // Just an example
#define LOG_METHOD                      LOG_METHOD_DEFAULT


/// Mode 2 Features:    
/// These are generally handled by the ISF settings files, but these defines 
/// can limit scope of the compilation if you are trying to optimize the build.
#define M2_FEATURE(VAL)                 (M2_FEATURESET && M2_FEATURE_##VAL)
#define M2_PARAM(VAL)                   (M2_PARAM_##VAL)
#define M2_FEATURE_M2DP                 ENABLED                             // Datastreams & associated commands
#define M2_FEATURE_DATASTREAM           M2_FEATURE_M2DP
#define M2_FEATURE_GATEWAY              DISABLED                             // Gateway device mode
#define M2_FEATURE_SUBCONTROLLER        ENABLED                             // Subcontroller device mode
#define M2_FEATURE_ENDPOINT             DISABLED                             // Endpoint device mode
#define M2_FEATURE_BLINKER              DISABLED                            // Blinker device mode
#define M2_FEATURE_BASE                 ENABLED                             // Base channels (ch 00, 80)
#define M2_FEATURE_LEGACY               NOT_AVAILABLE                       // Legacy (Mode 1) channel (ch 01)
#define M2_FEATURE_NORMAL               ENABLED                             // Low-speed channels (ch 1x, 9x)
#define M2_FEATURE_TURBO                ENABLED                             // High-speed channels (ch 2x, Ax)
#define M2_FEATURE_BLINK                DISABLED                            // Blink channels (ch 3x, Bx)
#define M2_FEATURE_FECTX                ENABLED                             // FEC support for transmissions
#define M2_FEATURE_FECRX                ENABLED                             // FEC support for receptions
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
                                ((ot_u32)OT_FEATURE_VLNEW << 24) | \
                                ((ot_u32)OT_FEATURE_VLRESTORE << 23) | \
                                ((ot_u32)OT_FEATURE_SENSORS << 22) | \
                                ((ot_u32)OT_FEATURE_SECURITY << 21) | \
                                ((ot_u32)OT_FEATURE_M2 << 15) | \
                                ((ot_u32)OT_FEATURE_M1 << 14) | \
                                ((ot_u32)OT_FEATURE_LF << 13) | \
                                ((ot_u32)OT_FEATURE_HF << 11) | \
                                ((ot_u32)OT_FEATURE_RTC << 7)       )



/** Flash Memory Setup: 
  * "OTF" means "Open Tag Flash," but if flash is not used, it just means 
  * storage memory.  Unfortunately this does not begin with F.
  */
#define OTF_VWORM_PAGES         12
#define OTF_VWORM_FALLOW_PAGES  3
#define OTF_VWORM_PAGESIZE      FLASH_PAGE_SIZE
#define OTF_VWORM_WORD_BYTES    FLASH_WORD_BYTES
#define OTF_VWORM_WORD_BITS     FLASH_WORD_BITS
#define OTF_VWORM_SIZE          (OTF_VWORM_PAGES * OTF_VWORM_PAGESIZE)
#define OTF_VWORM_START_PAGE    (0)
#define OTF_VWORM_START_ADDR    (FLASH_START_ADDR)

#define OTF_CRC_TABLE           DISABLED //toggle
#define OTF_UHF_TABLE           DISABLED
#define OTF_UHF_TABLESIZE       0
#define OTF_M1_ENCODE_TABLE     DISABLED
#define OTF_M2_ENCODE_TABLE     ENABLED

// Total number of pages taken from program memory
#define OTF_TOTAL_PAGES         (OTF_VWORM_PAGES)




/** Abstracted Flash Organization: 
  * OpenTag uses Flash to store 2 kinds of data.  The default setup puts 
  * Filesystem memory in front.
  * 1. Program code (obviously)
  * 2. Filesystem Memory
  *
  * FLASH_xxx constants are defined in the platform_config_xxx.h file.  
  */
#define OTF_TOTAL_SIZE          (FLASH_PAGE_SIZE * OTF_TOTAL_PAGES)
#define OTF_START_PAGE          (FLASH_NUM_PAGES - OTF_TOTAL_PAGES)
#define OTF_START_ADDR          (FLASH_START_ADDR + (OTF_START_PAGE * FLASH_PAGE_SIZE) )

#define OTF_LOOKUP_LAST_PAGE    ( OTF_LOOKUP_START_PAGE + OTF_LOOKUP_PAGES - 1 )
#define OTF_VWORM_LAST_PAGE     ( OTF_VWORM_START_PAGE + OTF_VWORM_PAGES - 1 )
#define OTF_VWORM_END_ADDR      ( OTF_VWORM_START_ADDR + (OTF_VWORM_PAGES * OTF_VWORM_PAGESIZE) - 1 )

//Legacy stuff
#define OTF_CRCLOOKUP_START_ADDR -1
#define OTF_UHFLOOKUP_START_ADDR -1
#define OTF_DECLOOKUP_START_ADDR -1



/** Internal Allocation Shortcuts:
  */
#define _ALLOC_OFFSET               (OTF_VWORM_WORD_BYTES-1)

#if (OTF_VWORM_WORD_BYTES == 1)
#   define _ALLOC_SHIFT             0
#elif (OTF_VWORM_WORD_BYTES == 2)
#   define _ALLOC_SHIFT             1
#elif (OTF_VWORM_WORD_BYTES == 4)
#   define _ALLOC_SHIFT             2
#elif (OTF_VWORM_WORD_BYTES == 8)
#   define _ALLOC_SHIFT             3
#else
#   error "You have a funky-sized VWORM/FLASH word, which is not going to work out of the box"
#endif







/** Veelite Addressing constants
  * For each of the three types of virtual memory, plus mirroring, which is
  * supported by ISFB files.  Mirroring stores a copy of the IFSB data in
  * RAM (see veelite.h, veelite.c, veelite_core.h, veelite_core.c)
  */
#define IN_VWORM    0x01
#define IN_VEEPROM  0x02        // VEEPROM doesn't actually exist anymore!
#define IN_VSRAM    0x04
#define IN_MIRROR   0x80




/** Mode 2 Cryptographic Key Data
  * Crypto is not implemented yet.

#define KEY_NOMINAL_LEN         16
#define KEY_NUM_ELEMENTS        10
#define KEY_SPACE               IN_MIRROR
#define KEY_DATA_BYTES          (KEY_NOMINAL_LEN * KEY_NUM_ELEMENTS)

*/


/** Mode 1 Tag Data Design (VWORM)
  * This is not used implemented in OpenTag
  
#define M1TAG_SPACE             IN_VWORM

#define M1TAG_LEN_manuf_id      2
#define M1TAG_LEN_serial        4
#define M1TAG_LEN_model_no      4
#define M1TAG_LEN_fw_version    3
#define M1TAG_LEN_max_response  1
#define M1TAG_MAX(VAL)          M1TAG_LEN_##VAL
#define M1TAG_ALLOC(VAL)        (((ot_u8)(M1TAG_LEN_##VAL) + 1) & 0xFE)

#define M1TAG_HEAP_BYTES        (M1TAG_ALLOC(manuf_id) + \
                                 M1TAG_ALLOC(serial) + \
                                 M1TAG_ALLOC(model_no) + \
                                 M1TAG_ALLOC(fw_version) + \
                                 M1TAG_ALLOC(max_response))
*/



/** Mode 1 Table Data Design (VWORM)
  * Tables as defined in ISO 18000-7 Mode 1 are not currently implemented in OpenTag.

#define TABLEDATA_SPACE         IN_VWORM
#define TABLE_HEAP_SIZE         0
*/




/** GFB (Generic File Block)
  * GFB is a mostly unstructured data space.  You can change the definitions 
  * below to match your application & platform
  */
/// legacy defs
#define RDB GFB
#define rdb gfb
  
#define GFB_TOTAL_BYTES         1024
#define GFB_START_VADDR         0x0400
  
#define GFB_FILE_BYTES          256
#define GFB_NUM_STOCK_FILES     1
#define GFB_NUM_USER_FILES      3
#define GFB_NUM_FILES           (GFB_NUM_STOCK_FILES + GFB_NUM_USER_FILES)
#define GFB_MOD_standard        b00110100
#define GFB_HEAP_BYTES          (GFB_FILE_BYTES*GFB_NUM_STOCK_FILES)







/** ISFSB Files (Indexed Short File Series Block)
  * ISFSB Files are strings of ISFB File IDs that are used
  * to bundle related ISFB Files.
  */

/// legacy defs
#define UDBLIST     ISFS
#define udblist     isfs
#define udb_list    isfs

#define ISFS_TOTAL_BYTES                     160
#define ISFS_START_VADDR                     0x0360
  
#define ISFS_NUM_M1_LISTS                    4
#define ISFS_NUM_M2_LISTS                    4
#define ISFS_NUM_EXT_LISTS                   16
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
#define ISFS_ID_extended_service             0x80

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
#define ISFS_MAX_device_capability           4
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







/** ISFB (Indexed Short File Block)
  * The ISFB contains up to 256 files (IDs 0x00 to 0xFF), length <= 255 bytes.
  *
  * Here is the breakdown:
  * 0x00 to 0x0F: Mode 2 Configuration and Application Data Elements
  * 0x10 to 0x1F: Mode 1 & 2 Application Data
  * 0x20 to 0x7F: Reserved for future use
  * 0x80 to 0x9F: Mode 1 & 2 extended services data (not really used)
  * 0xA0 to 0xFE: Proprietary
  *         0xFF: Proprietary Data Extension
  *
  * Some files have allocations less than 255 bytes.  Many of the files from IDs 
  * 0x00 to 0x1F have limited allocations because they are config registers.
  *
  * There are several types of MACROS for handling ISFB constants.  To use, put
  * the name of the ISF into the argument, such as:
  * @c ISF_ID(network_settings) @c
  *
  * The macros are:
  * @c ISF_ID(file_name) @c :       File ID (0-255)
  * @c ISF_MOD(file_name) @c :      File Privilege bitmask (1 byte)
  * @c ISF_LEN(file_name) @c :      File Length (0-255)
  * @c ISF_MAX(file_name) @c :      Maximum Length of the file Data (0-255)
  * @c ISF_ALLOC(file_name) @c :    Allocated Bytes for file (0-256)
*/

///Legacy def
#define UDB     ISF
#define udb     isf

#define ISF_TOTAL_BYTES                         2048
#define ISF_START_VADDR                         0x0800
#define ISF_MIRROR_VADDR                        0xC000

/// Known Number of ISF files
#define ISF_NUM_M1_FILES                        10
#define ISF_NUM_M2_FILES                        16
#define ISF_NUM_STOCK_FILES                     (ISF_NUM_M1_FILES + ISF_NUM_M2_FILES)
#define ISF_NUM_USER_FILES                      16
#define ISF_NUM_FILES                           (ISF_NUM_STOCK_FILES + ISF_NUM_USER_FILES)

/// Stock Mode 2 ISF File IDs
/// ID's 0x00 to 0x0F:  Mode 2 only
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
#define ISF_ID_external_events_list             0x17
#define ISF_ID_external_events_alarm_list       0x18
#define ISF_ID_application_extension            0xFF


/// ISF file default privileges
/// Mod Byte: EXrwxrwx
/// root can always read & write, and he can execute when X is 1
///
/// E:          data is encrypted in storage (not supported atm)
/// X:          data is executable (a program)
/// 1st rwx:    read/write/exec for user
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
#define ISF_MOD_external_events_list            b00100100
#define ISF_MOD_external_events_alarm_list      b00100100
#define ISF_MOD_application_extension           b00100100

/// ISF file default length
/// that is, the initial length of the ISF
#define ISF_LEN(VAL)                            ISF_LEN_##VAL
#define ISF_LEN_network_settings                10
#define ISF_LEN_device_features                 48
#define ISF_LEN_channel_configuration           32
#define ISF_LEN_real_time_scheduler             12
#define ISF_LEN_sleep_scan_sequence             12
#define ISF_LEN_hold_scan_sequence              12
#define ISF_LEN_beacon_transmit_sequence        24
#define ISF_LEN_protocol_list                   4
#define ISF_LEN_isfs_list                       12
#define ISF_LEN_gfb_file_list                   GFB_NUM_FILES
#define ISF_LEN_location_data_list              0
#define ISF_LEN_ipv6_addresses                  0
#define ISF_LEN_sensor_list                     0
#define ISF_LEN_sensor_alarms                   0
#define ISF_LEN_root_authentication_key         0
#define ISF_LEN_user_authentication_key         0
#define ISF_LEN_routing_code                    0
#define ISF_LEN_user_id                         0
#define ISF_LEN_optional_command_list           7
#define ISF_LEN_memory_size                     12
#define ISF_LEN_table_query_size                1
#define ISF_LEN_table_query_results             7
#define ISF_LEN_hardware_fault_status           3
#define ISF_LEN_external_events_list            0
#define ISF_LEN_external_events_alarm_list      0
#define ISF_LEN_application_extension           0

/// Stock ISF file max data lengths
#define ISF_MAX(VAL)                            ISF_MAX_##VAL
#define ISF_MAX_USER_FILE                       255
#define ISF_MAX_network_settings                10
#define ISF_MAX_device_features                 48
#define ISF_MAX_channel_configuration           64
#define ISF_MAX_real_time_scheduler             12
#define ISF_MAX_sleep_scan_sequence             32
#define ISF_MAX_hold_scan_sequence              32
#define ISF_MAX_beacon_transmit_sequence        24
#define ISF_MAX_protocol_list                   16
#define ISF_MAX_isfs_list                       24
#define ISF_MAX_gfb_file_list                   8
#define ISF_MAX_location_data_list              0
#define ISF_MAX_ipv6_addresses                  48
#define ISF_MAX_sensor_list                     0
#define ISF_MAX_sensor_alarms                   0
#define ISF_MAX_root_authentication_key         0
#define ISF_MAX_user_authentication_key         0
#define ISF_MAX_routing_code                    50
#define ISF_MAX_user_id                         60
#define ISF_MAX_optional_command_list           24
#define ISF_MAX_memory_size                     12
#define ISF_MAX_table_query_size                1
#define ISF_MAX_table_query_results             7
#define ISF_MAX_hardware_fault_status           3
#define ISF_MAX_external_events_list            0
#define ISF_MAX_external_events_alarm_list      0
#define ISF_MAX_application_extension           16

/// Stock ISF file memory allocations 
/// - must be half-word aligned
#define ISF_ALLOC(VAL)          (((ISF_MAX_##VAL + _ALLOC_OFFSET) >> _ALLOC_SHIFT) << _ALLOC_SHIFT)

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
#define ISF_BASE_external_events_list           (ISF_BASE_hardware_fault_status+ISF_ALLOC(hardware_fault_status))
#define ISF_BASE_external_events_alarm_list     (ISF_BASE_external_events_list+ISF_ALLOC(external_events_list))
#define ISF_BASE_application_extension          (ISF_BASE_external_events_alarm_list+ISF_ALLOC(external_events_alarm_list))
#define ISF_BASE_NEXT                           (ISF_BASE_application_extension+ISF_ALLOC(application_extension))

#define ISF_MIRROR(VAL)                         ISF_MIRROR_##VAL
#define ISF_MIRROR_network_settings             (ISF_MIRROR_VADDR)
#define ISF_MIRROR_channel_configuration        (ISF_MIRROR_network_settings+ISF_ALLOC(network_settings))
#define ISF_MIRROR_NEXT                         (ISF_MIRROR_channel_configuration+ISF_ALLOC(channel_configuration))



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
                                ISF_ALLOC(external_events_list) + \
                                ISF_ALLOC(external_events_alarm_list) + \
                                ISF_ALLOC(application_extension))
                               
//#define ISF_VWORM_USER_BYTES   (ISF_ALLOC(USER_FILE) * ISF_NUM_USER_FILES)

#define ISF_VWORM_HEAP_BYTES    ISF_VWORM_STOCK_BYTES

#define ISF_MIRROR_HEAP_BYTES  (ISF_ALLOC(network_settings) + \
                                ISF_ALLOC(channel_configuration) )

#define ISF_HEAP_BYTES          ISF_VWORM_HEAP_BYTES




#endif 
