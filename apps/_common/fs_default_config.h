/* Copyright 2017 JP Norair
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
  * @file       /apps/_common/fs_default_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R101
  * @date       30 Apr 2017
  * @brief      Configuration for typical demo filesystem
  *
  * To use this typical FS configuration, include into your app/config.h header
  * following the FEATURE configuration section.
  *
  * #include <../_common/fs_default_config.h>
  * 
  ******************************************************************************
  */

#ifndef __DEFAULT_FS_CONFIG_H
#define __DEFAULT_FS_CONFIG_H

#include <app/build_config.h>


#if defined(__VLSRAM__)
#   define _NOMIRROR    1
#else
#   define _NOMIRROR    0
#endif



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
#define ISFS_TOTAL_BYTES                     0x00A0
#define ISFS_NUM_M1_LISTS                    4
#define ISFS_NUM_M2_LISTS                    4
#define ISFS_NUM_EXT_LISTS                   16

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
#define ISF_TOTAL_BYTES                         1536
#define ISF_NUM_M1_FILES                        7
#define ISF_NUM_M2_FILES                        18
#define ISF_NUM_EXT_FILES                       1   // Usually at least 1 (app ext)
#define ISF_NUM_USER_FILES                      0  //max allowed user files

///@todo define this after mirror is alloc'ed
#define ISF_MIRROR_VADDR                        0xC000

#define ISF_START_VADDR                         (GFB_START_VADDR + GFB_TOTAL_BYTES)
#define ISF_NUM_STOCK_FILES                     (ISF_NUM_M1_FILES + ISF_NUM_M2_FILES)
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
#define ISF_ID_hold_scan_sequence               0x04
#define ISF_ID_sleep_scan_sequence              0x05
#define ISF_ID_beacon_transmit_sequence         0x06
#define ISF_ID_isf_list                         0x07
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
#define ISF_ID_gnss_output                      0x17
#define ISF_ID_agps_input                       0x18
#define ISF_ID_application_extension            0xFF

/// ISF Mirror Enabling: <BR>
/// ISFB files can be mirrored in RAM.  Set to 0/1 to Disable/Enable each file 
/// mirror.  Mirroring speeds-up file access, but it can consume a lot of RAM.
#define __SET_MIRROR(VAL)                       (VAL && !_NOMIRROR)
#define ISF_ENMIRROR(VAL)                       ISF_ENMIRROR_##VAL
#define ISF_ENMIRROR_network_settings           __SET_MIRROR(1)
#define ISF_ENMIRROR_device_features            __SET_MIRROR(0)
#define ISF_ENMIRROR_channel_configuration      __SET_MIRROR(0)
#define ISF_ENMIRROR_real_time_scheduler        __SET_MIRROR(0)
#define ISF_ENMIRROR_hold_scan_sequence         __SET_MIRROR(0)
#define ISF_ENMIRROR_sleep_scan_sequence        __SET_MIRROR(0)
#define ISF_ENMIRROR_beacon_transmit_sequence   __SET_MIRROR(0)
#define ISF_ENMIRROR_isf_list                   __SET_MIRROR(0)
#define ISF_ENMIRROR_isfs_list                  __SET_MIRROR(0)
#define ISF_ENMIRROR_gfb_file_list              __SET_MIRROR(0)
#define ISF_ENMIRROR_location_data_list         __SET_MIRROR(1)
#define ISF_ENMIRROR_ipv6_addresses             __SET_MIRROR(0)
#define ISF_ENMIRROR_sensor_list                __SET_MIRROR(0)
#define ISF_ENMIRROR_sensor_alarms              __SET_MIRROR(0)
#define ISF_ENMIRROR_root_authentication_key    __SET_MIRROR(0)
#define ISF_ENMIRROR_user_authentication_key    __SET_MIRROR(0)
#define ISF_ENMIRROR_routing_code               __SET_MIRROR(0)
#define ISF_ENMIRROR_user_id                    __SET_MIRROR(0)
#define ISF_ENMIRROR_optional_command_list      __SET_MIRROR(0)
#define ISF_ENMIRROR_memory_size                __SET_MIRROR(0)
#define ISF_ENMIRROR_table_query_size           __SET_MIRROR(0)
#define ISF_ENMIRROR_table_query_results        __SET_MIRROR(0)
#define ISF_ENMIRROR_hardware_fault_status      __SET_MIRROR(0)
#define ISF_ENMIRROR_gnss_output                __SET_MIRROR(1)
#define ISF_ENMIRROR_agps_input                 __SET_MIRROR(1)
#define ISF_ENMIRROR_application_extension      __SET_MIRROR(1)


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
#define ISF_MOD_hold_scan_sequence              ISF_MOD_file_standard
#define ISF_MOD_sleep_scan_sequence             ISF_MOD_file_standard
#define ISF_MOD_beacon_transmit_sequence        ISF_MOD_file_standard
#define ISF_MOD_isf_list                        b00100100
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
#define ISF_MOD_gnss_output                     b00100000
#define ISF_MOD_agps_input                      b00110000
#define ISF_MOD_application_extension           b00100100

/// ISF file default length: 
/// (that is, the initial length of the ISF)
#define ISF_LEN(VAL)                            ISF_LEN_##VAL
#define ISF_LEN_network_settings                10
#define ISF_LEN_device_features                 48
#define ISF_LEN_channel_configuration           24
#define ISF_LEN_real_time_scheduler             12
#define ISF_LEN_hold_scan_sequence              4
#define ISF_LEN_sleep_scan_sequence             4
#define ISF_LEN_beacon_transmit_sequence        16
#define ISF_LEN_isf_list                        1
#define ISF_LEN_isfs_list                       12
#define ISF_LEN_gfb_file_list                   GFB_NUM_FILES
#define ISF_LEN_location_data_list              0
#define ISF_LEN_ipv6_addresses                  0
#define ISF_LEN_sensor_list                     16
#define ISF_LEN_sensor_alarms                   2
#define ISF_LEN_root_authentication_key         0
#define ISF_LEN_user_authentication_key         0
#define ISF_LEN_routing_code                    0
#define ISF_LEN_user_id                         11
#define ISF_LEN_optional_command_list           7
#define ISF_LEN_memory_size                     12
#define ISF_LEN_table_query_size                1
#define ISF_LEN_table_query_results             7
#define ISF_LEN_hardware_fault_status           3
#define ISF_LEN_gnss_output                     0
#define ISF_LEN_agps_input                      0
#define ISF_LEN_application_extension           0

/// Stock ISF file max data lengths (not aligned, just max)
#define ISF_MAX(VAL)                            ISF_MAX_##VAL
#define ISF_MAX_USER_FILE                       255
#define ISF_MAX_network_settings                10
#define ISF_MAX_device_features                 48
#define ISF_MAX_channel_configuration           48
#define ISF_MAX_real_time_scheduler             12
#define ISF_MAX_hold_scan_sequence              32  //8 scans
#define ISF_MAX_sleep_scan_sequence             32  //8 scans
#define ISF_MAX_beacon_transmit_sequence        24  //3 beacons
#define ISF_MAX_isf_list                        24  //24 isf
#define ISF_MAX_isfs_list                       16  //16 isfs indices
#define ISF_MAX_gfb_file_list                   8   //8 gfb files
#define ISF_MAX_location_data_list              64  //8 location vertices (or 16 if using VIDs)
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
#define ISF_MAX_gnss_output                     96  // Designed to support UBX-8 NAV-PVT
#define ISF_MAX_agps_input                      200 // Designed for GPS MGA messages on UBX-8
#define ISF_MAX_application_extension           64


/// BEGINNING OF AUTOMATIC ISF STUFF (You can probably leave it alone)

/// Stock ISF file memory & mirror allocations (aligned, typically 16bit)
#define ISF_ALLOC(VAL)          (((ISF_MAX_##VAL + _ALLOC_OFFSET) >> _ALLOC_SHIFT) << _ALLOC_SHIFT)
#define ISF_MIRALLOC(VAL)       (ISF_ENMIRROR(VAL) * (((ISF_MAX_##VAL + 2 + _MIRALLOC_OFFSET) >> _MIRALLOC_SHIFT) << _MIRALLOC_SHIFT))

/// ISF file base address computation
#define ISF_BASE(VAL)                           ISF_BASE_##VAL
#if _NOMIRROR
#   undef ISF_MIRALLOC
#   define ISF_MIRALLOC(VAL)                    0
#   define ISF_BASE_network_settings            (ISF_START_VADDR)
#   define ISF_BASE_device_features             (ISF_BASE_network_settings+ISF_ALLOC(network_settings))
#   define ISF_BASE_channel_configuration       (ISF_BASE_device_features+ISF_ALLOC(device_features))
#   define ISF_BASE_real_time_scheduler         (ISF_BASE_channel_configuration+ISF_ALLOC(channel_configuration))
#   define ISF_BASE_hold_scan_sequence          (ISF_BASE_real_time_scheduler+ISF_ALLOC(real_time_scheduler))
#   define ISF_BASE_sleep_scan_sequence         (ISF_BASE_hold_scan_sequence+ISF_ALLOC(hold_scan_sequence))
#   define ISF_BASE_beacon_transmit_sequence    (ISF_BASE_sleep_scan_sequence+ISF_ALLOC(sleep_scan_sequence))
#   define ISF_BASE_isf_list                    (ISF_BASE_beacon_transmit_sequence+ISF_ALLOC(beacon_transmit_sequence))
#   define ISF_BASE_isfs_list                   (ISF_BASE_isf_list+ISF_ALLOC(isf_list))
#   define ISF_BASE_gfb_file_list               (ISF_BASE_isfs_list+ISF_ALLOC(isfs_list))
#   define ISF_BASE_location_data_list          (ISF_BASE_gfb_file_list+ISF_ALLOC(gfb_file_list))
#   define ISF_BASE_ipv6_addresses              (ISF_BASE_location_data_list+ISF_ALLOC(location_data_list))
#   define ISF_BASE_sensor_list                 (ISF_BASE_ipv6_addresses+ISF_ALLOC(ipv6_addresses))
#   define ISF_BASE_sensor_alarms               (ISF_BASE_sensor_list+ISF_ALLOC(sensor_list))
#   define ISF_BASE_root_authentication_key     (ISF_BASE_sensor_alarms+ISF_ALLOC(sensor_alarms))
#   define ISF_BASE_user_authentication_key     (ISF_BASE_root_authentication_key+ISF_ALLOC(root_authentication_key))
#   define ISF_BASE_routing_code                (ISF_BASE_user_authentication_key+ISF_ALLOC(user_authentication_key))
#   define ISF_BASE_user_id                     (ISF_BASE_routing_code+ISF_ALLOC(routing_code))
#   define ISF_BASE_optional_command_list       (ISF_BASE_user_id+ISF_ALLOC(user_id))
#   define ISF_BASE_memory_size                 (ISF_BASE_optional_command_list+ISF_ALLOC(optional_command_list))
#   define ISF_BASE_table_query_size            (ISF_BASE_memory_size+ISF_ALLOC(memory_size))
#   define ISF_BASE_table_query_results         (ISF_BASE_table_query_size+ISF_ALLOC(table_query_size))
#   define ISF_BASE_hardware_fault_status       (ISF_BASE_table_query_results+ISF_ALLOC(table_query_results))
#   define ISF_BASE_gnss_output                 (ISF_BASE_hardware_fault_status+ISF_ALLOC(hardware_fault_status))
#   define ISF_BASE_agps_input                  (ISF_BASE_gnss_output+ISF_ALLOC(gnss_output))
#   define ISF_BASE_application_extension       (ISF_BASE_agps_input+ISF_ALLOC(agps_input))
#   define ISF_BASE_NEXT                        (ISF_BASE_application_extension+ISF_ALLOC(application_extension ))

#else
#   define ISF_BASE_network_settings            (ISF_START_VADDR)
#   define ISF_BASE_device_features             (ISF_BASE_network_settings+ISF_ALLOC(network_settings))
#   define ISF_BASE_channel_configuration       (ISF_BASE_device_features+ISF_ALLOC(device_features))
#   define ISF_BASE_real_time_scheduler         (ISF_BASE_channel_configuration+ISF_ALLOC(channel_configuration))
#   define ISF_BASE_hold_scan_sequence          (ISF_BASE_real_time_scheduler+ISF_ALLOC(real_time_scheduler))
#   define ISF_BASE_sleep_scan_sequence         (ISF_BASE_hold_scan_sequence+ISF_ALLOC(hold_scan_sequence))
#   define ISF_BASE_beacon_transmit_sequence    (ISF_BASE_sleep_scan_sequence+ISF_ALLOC(sleep_scan_sequence))
#   define ISF_BASE_isf_list                    (ISF_BASE_beacon_transmit_sequence+ISF_ALLOC(beacon_transmit_sequence))
#   define ISF_BASE_isfs_list                   (ISF_BASE_isf_list+ISF_ALLOC(isf_list))
#   define ISF_BASE_gfb_file_list               (ISF_BASE_isfs_list+ISF_ALLOC(isfs_list))
#   define ISF_BASE_location_data_list          (0xFFFF)
#   define ISF_BASE_ipv6_addresses              (ISF_BASE_gfb_file_list+ISF_ALLOC(gfb_file_list))
#   define ISF_BASE_sensor_list                 (ISF_BASE_ipv6_addresses+ISF_ALLOC(ipv6_addresses))
#   define ISF_BASE_sensor_alarms               (ISF_BASE_sensor_list+ISF_ALLOC(sensor_list))
#   define ISF_BASE_root_authentication_key     (ISF_BASE_sensor_alarms+ISF_ALLOC(sensor_alarms))
#   define ISF_BASE_user_authentication_key     (ISF_BASE_root_authentication_key+ISF_ALLOC(root_authentication_key))
#   define ISF_BASE_routing_code                (ISF_BASE_user_authentication_key+ISF_ALLOC(user_authentication_key))
#   define ISF_BASE_user_id                     (ISF_BASE_routing_code+ISF_ALLOC(routing_code))
#   define ISF_BASE_optional_command_list       (ISF_BASE_user_id+ISF_ALLOC(user_id))
#   define ISF_BASE_memory_size                 (ISF_BASE_optional_command_list+ISF_ALLOC(optional_command_list))
#   define ISF_BASE_table_query_size            (ISF_BASE_memory_size+ISF_ALLOC(memory_size))
#   define ISF_BASE_table_query_results         (ISF_BASE_table_query_size+ISF_ALLOC(table_query_size))
#   define ISF_BASE_hardware_fault_status       (ISF_BASE_table_query_results+ISF_ALLOC(table_query_results))
#   define ISF_BASE_gnss_output                 (0xFFFF)
#   define ISF_BASE_agps_input                  (0xFFFF)
#   define ISF_BASE_application_extension       (0xFFFF)
#   define ISF_BASE_NEXT                        (ISF_BASE_hardware_fault_status+ISF_ALLOC(hardware_fault_status))
#endif



#define ISF_MIRROR(VAL)                         (unsigned short)(((ISF_ENMIRROR_##VAL != 0) - 1) | (ISF_MIRROR_##VAL) )
#define ISF_MIRROR_network_settings             (ISF_MIRROR_VADDR)
#define ISF_MIRROR_device_features              (ISF_MIRROR_network_settings+ISF_MIRALLOC(network_settings))
#define ISF_MIRROR_channel_configuration        (ISF_MIRROR_device_features+ISF_MIRALLOC(device_features))
#define ISF_MIRROR_real_time_scheduler          (ISF_MIRROR_channel_configuration+ISF_MIRALLOC(channel_configuration))
#define ISF_MIRROR_hold_scan_sequence           (ISF_MIRROR_real_time_scheduler+ISF_MIRALLOC(real_time_scheduler))
#define ISF_MIRROR_sleep_scan_sequence          (ISF_MIRROR_hold_scan_sequence+ISF_MIRALLOC(hold_scan_sequence))
#define ISF_MIRROR_beacon_transmit_sequence     (ISF_MIRROR_sleep_scan_sequence+ISF_MIRALLOC(sleep_scan_sequence))
#define ISF_MIRROR_isf_list                     (ISF_MIRROR_beacon_transmit_sequence+ISF_MIRALLOC(beacon_transmit_sequence))
#define ISF_MIRROR_isfs_list                    (ISF_MIRROR_isf_list+ISF_MIRALLOC(isf_list))
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
#define ISF_MIRROR_gnss_output                  (ISF_MIRROR_hardware_fault_status+ISF_MIRALLOC(hardware_fault_status))
#define ISF_MIRROR_agps_input                   (ISF_MIRROR_gnss_output+ISF_MIRALLOC(gnss_output))
#define ISF_MIRROR_application_extension        (ISF_MIRROR_agps_input+ISF_MIRALLOC(agps_input))
#define ISF_MIRROR_NEXT                         (ISF_MIRROR_application_extension+ISF_MIRALLOC(application_extension))


/// Total amount of stock ISF data stored in ROM
#define ISF_VWORM_STOCK_BYTES   (ISF_ALLOC(network_settings) + \
                                ISF_ALLOC(device_features) + \
                                ISF_ALLOC(channel_configuration) + \
                                ISF_ALLOC(real_time_scheduler) + \
                                ISF_ALLOC(hold_scan_sequence) + \
                                ISF_ALLOC(sleep_scan_sequence) + \
                                ISF_ALLOC(beacon_transmit_sequence) + \
                                ISF_ALLOC(isf_list) + \
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
                                ISF_ALLOC(gnss_output) + \
                                ISF_ALLOC(agps_input) + \
                                ISF_ALLOC(application_extension))

#define ISF_VWORM_HEAP_BYTES    ISF_VWORM_STOCK_BYTES
#define ISF_HEAP_BYTES          ISF_VWORM_HEAP_BYTES
//#define ISF_VWORM_USER_BYTES   (ISF_ALLOC(USER_FILE) * ISF_NUM_USER_FILES)


/// Total amount of allocation to the Mirror
#define ISF_MIRROR_HEAP_BYTES                ((ISF_MIRROR_NEXT) - (ISF_MIRROR_VADDR))

/// END OF AUTOMATIC ISF STUFF 

#endif 
