/* Copyright 2012-2013 JP Norair
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
  * @file       /apps/demo_ponglt/code/default_data.c
  * @author     JP Norair
  * @version    R101
  * @date       14 Feb 2013
  * @brief      PongLT Demo Default Data
  *
  ******************************************************************************
  */

#ifndef _DEFAULT_DATA_C
#define _DEFAULT_DATA_C

#include <otsys/support.h>
#include <otsys/version.h>
#include <otsys/types.h>


#if defined(__DEBUG__) || defined(__PROTO__)
/// The ID's are written as FFF... so that the OpenTag startup routine can write
/// a derived ID to them without any kind of flash wear-leveling
#   define __VID    0xFF, 0xFF
#   define __UID    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
#endif




/** Default File data allocations
  * ============================================================================
  * - Veelite also uses an additional 1536 bytes for wear leveling
  * - Wear leveling overhead is configurable, but fixed for all FS sizes
  * - Veelite virtual addressing allocations of key sectors below:
  *     Overhead:   0000 to 03FF        (1024 bytes alloc)
  *     ISFSB:      0400 to 049F        (160 bytes alloc)
  *     GFB:        04A0 to 089F        (1024 bytes)
  *     ISFB:       08A0 to 0FFF        (1888 bytes)
  */
#define SPLIT_SHORT(VAL)    (ot_u8)((ot_u16)(VAL) >> 8), (ot_u8)((ot_u16)(VAL) & 0x00FF)
#define SPLIT_LONG(VAL)     (ot_u8)((ot_u32)(VAL) >> 24), (ot_u8)(((ot_u32)(VAL) >> 16) & 0xFF), \
                            (ot_u8)(((ot_u32)(VAL) >> 8) & 0xFF), (ot_u8)((ot_u32)(VAL) & 0xFF)

#define SPLIT_SHORT_LE(VAL) (ot_u8)((ot_u16)(VAL) & 0x00FF), (ot_u8)((ot_u16)(VAL) >> 8)
#define SPLIT_LONG_LE(VAL)  (ot_u8)((ot_u32)(VAL) & 0xFF), (ot_u8)(((ot_u32)(VAL) >> 8) & 0xFF), \
                            (ot_u8)(((ot_u32)(VAL) >> 16) & 0xFF), (ot_u8)((ot_u32)(VAL) >> 24)


/// These overhead are the Veelite vl_header files. They are hard coded,
/// and they must be in the endian of the platform. (Little endian here)

#if (CC_SUPPORT == GCC)
const ot_u8 overhead_files[] __attribute__ ((section(".vl_ov"))) = {
#elif (CC_SUPPORT == CL430)
#pragma DATA_SECTION(overhead_files, ".vl_ov")
const ot_u8 overhead_files[] = {
#endif
    //0x00, 0x00, 0x00, 0x01,                 /* GFB ELements 0 - 3 */
    //0x00, GFB_MOD_standard,
    //0x00, 0x14, 0xFF, 0xFF,
    //0x00, 0x00, 0x00, 0x01,
    //0x01, GFB_MOD_standard,
    //0x00, 0x15, 0xFF, 0xFF,
    //0x00, 0x00, 0x00, 0x01,
    //0x02, GFB_MOD_standard,
    //0x00, 0x16, 0xFF, 0xFF,
    //0x00, 0x00, 0x00, 0x01,
    //0x03, GFB_MOD_standard,
    //0x00, 0x17, 0xFF, 0xFF,

    ISFS_LEN(transit_data), 0x00,
    ISFS_ALLOC(transit_data), 0x00,
    ISFS_ID(transit_data),
    ISFS_MOD(transit_data),
    SPLIT_SHORT_LE(ISFS_BASE(transit_data)),
    0xFF, 0xFF,

    ISFS_LEN(capability_data), 0x00,
    ISFS_ALLOC(capability_data), 0x00,
    ISFS_ID(capability_data),
    ISFS_MOD(capability_data),
    SPLIT_SHORT_LE(ISFS_BASE(capability_data)),
    0xFF, 0xFF,

    ISFS_LEN(query_results), 0x00,
    ISFS_ALLOC(query_results), 0x00,
    ISFS_ID(query_results),
    ISFS_MOD(query_results),
    SPLIT_SHORT_LE(ISFS_BASE(query_results)),
    0xFF, 0xFF,

    ISFS_LEN(hardware_fault), 0x00,
    ISFS_ALLOC(hardware_fault), 0x00,
    ISFS_ID(hardware_fault),
    ISFS_MOD(hardware_fault),
    SPLIT_SHORT_LE(ISFS_BASE(hardware_fault)),
    0xFF, 0xFF,

    ISFS_LEN(device_discovery), 0x00,
    ISFS_ALLOC(device_discovery), 0x00,
    ISFS_ID(device_discovery),
    ISFS_MOD(device_discovery),
    SPLIT_SHORT_LE(ISFS_BASE(device_discovery)),
    0xFF, 0xFF,

    ISFS_LEN(device_capability), 0x00,
    ISFS_ALLOC(device_capability), 0x00,
    ISFS_ID(device_capability),
    ISFS_MOD(device_capability),
    SPLIT_SHORT_LE(ISFS_BASE(device_capability)),
    0xFF, 0xFF,

    ISFS_LEN(device_channel_utilization), 0x00,
    ISFS_ALLOC(device_channel_utilization), 0x00,
    ISFS_ID(device_channel_utilization),
    ISFS_MOD(device_channel_utilization),
    SPLIT_SHORT_LE(ISFS_BASE(device_channel_utilization)),
    0xFF, 0xFF,

    ISFS_LEN(location_data), 0x00,
    ISFS_ALLOC(location_data), 0x00,
    ISFS_ID(location_data),
    ISFS_MOD(location_data),
    SPLIT_SHORT_LE(ISFS_BASE(location_data)),
    0xFF, 0xFF,

    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* Mode 2 ISFs, written as little endian */
    ISF_LEN(network_settings), 0x00,                /* Length, little endian */
    SPLIT_SHORT_LE(ISF_ALLOC(network_settings)),    /* Alloc, little endian */
    ISF_ID(network_settings),                       /* ID */
    ISF_MOD(network_settings),                      /* Perms */
    SPLIT_SHORT_LE(ISF_BASE(network_settings)),
    SPLIT_SHORT_LE(ISF_MIRROR(network_settings)),

    ISF_LEN(device_features), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(device_features)),
    ISF_ID(device_features),
    ISF_MOD(device_features),
    SPLIT_SHORT_LE(ISF_BASE(device_features)),
    SPLIT_SHORT_LE(ISF_MIRROR(device_features)),

    ISF_LEN(channel_configuration), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(channel_configuration)),
    ISF_ID(channel_configuration),
    ISF_MOD(channel_configuration),
    SPLIT_SHORT_LE(ISF_BASE(channel_configuration)),
    SPLIT_SHORT_LE(ISF_MIRROR(channel_configuration)),

    ISF_LEN(real_time_scheduler), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(real_time_scheduler)),
    ISF_ID(real_time_scheduler),
    ISF_MOD(real_time_scheduler),
    SPLIT_SHORT_LE(ISF_BASE(real_time_scheduler)),
    SPLIT_SHORT_LE(ISF_MIRROR(real_time_scheduler)),

    ISF_LEN(hold_scan_sequence), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(hold_scan_sequence)),
    ISF_ID(hold_scan_sequence),
    ISF_MOD(hold_scan_sequence),
    SPLIT_SHORT_LE(ISF_BASE(hold_scan_sequence)),
    SPLIT_SHORT_LE(ISF_MIRROR(hold_scan_sequence)),

    ISF_LEN(sleep_scan_sequence), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(sleep_scan_sequence)),
    ISF_ID(sleep_scan_sequence),
    ISF_MOD(sleep_scan_sequence),
    SPLIT_SHORT_LE(ISF_BASE(sleep_scan_sequence)),
    SPLIT_SHORT_LE(ISF_MIRROR(sleep_scan_sequence)),

    ISF_LEN(beacon_transmit_sequence), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(beacon_transmit_sequence)),
    ISF_ID(beacon_transmit_sequence),
    ISF_MOD(beacon_transmit_sequence),
    SPLIT_SHORT_LE(ISF_BASE(beacon_transmit_sequence)),
    SPLIT_SHORT_LE(ISF_MIRROR(beacon_transmit_sequence)),

    ISF_LEN(protocol_list), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(protocol_list)),
    ISF_ID(protocol_list),
    ISF_MOD(protocol_list),
    SPLIT_SHORT_LE(ISF_BASE(protocol_list)),
    SPLIT_SHORT_LE(ISF_MIRROR(protocol_list)),

    ISF_LEN(isfs_list), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(isfs_list)),
    ISF_ID(isfs_list),
    ISF_MOD(isfs_list),
    SPLIT_SHORT_LE(ISF_BASE(isfs_list)),
    SPLIT_SHORT_LE(ISF_MIRROR(isfs_list)),

    ISF_LEN(gfb_file_list), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(gfb_file_list)),
    ISF_ID(gfb_file_list),
    ISF_MOD(gfb_file_list),
    SPLIT_SHORT_LE(ISF_BASE(gfb_file_list)),
    SPLIT_SHORT_LE(ISF_MIRROR(gfb_file_list)),

    ISF_LEN(location_data_list), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(location_data_list)),
    ISF_ID(location_data_list),
    ISF_MOD(location_data_list),
    SPLIT_SHORT_LE(ISF_BASE(location_data_list)),
    SPLIT_SHORT_LE(ISF_MIRROR(location_data_list)),

    ISF_LEN(ipv6_addresses), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(ipv6_addresses)),
    ISF_ID(ipv6_addresses),
    ISF_MOD(ipv6_addresses),
    SPLIT_SHORT_LE(ISF_BASE(ipv6_addresses)),
    SPLIT_SHORT_LE(ISF_MIRROR(ipv6_addresses)),

    ISF_LEN(sensor_list), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(sensor_list)),
    ISF_ID(sensor_list),
    ISF_MOD(sensor_list),
    SPLIT_SHORT_LE(ISF_BASE(sensor_list)),
    SPLIT_SHORT_LE(ISF_MIRROR(sensor_list)),

    ISF_LEN(sensor_alarms), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(sensor_alarms)),
    ISF_ID(sensor_alarms),
    ISF_MOD(sensor_alarms),
    SPLIT_SHORT_LE(ISF_BASE(sensor_alarms)),
    SPLIT_SHORT_LE(ISF_MIRROR(sensor_alarms)),

    ISF_LEN(root_authentication_key), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(root_authentication_key)),
    ISF_ID(root_authentication_key),
    ISF_MOD(root_authentication_key),
    SPLIT_SHORT_LE(ISF_BASE(root_authentication_key)),
    SPLIT_SHORT_LE(ISF_MIRROR(root_authentication_key)),

    ISF_LEN(user_authentication_key), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(user_authentication_key)),
    ISF_ID(user_authentication_key),
    ISF_MOD(user_authentication_key),
    SPLIT_SHORT_LE(ISF_BASE(user_authentication_key)),
    SPLIT_SHORT_LE(ISF_MIRROR(user_authentication_key)),

    ISF_LEN(routing_code), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(routing_code)),
    ISF_ID(routing_code),
    ISF_MOD(routing_code),
    SPLIT_SHORT_LE(ISF_BASE(routing_code)),
    SPLIT_SHORT_LE(ISF_MIRROR(routing_code)),

    ISF_LEN(user_id), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(user_id)),
    ISF_ID(user_id),
    ISF_MOD(user_id),
    SPLIT_SHORT_LE(ISF_BASE(user_id)),
    SPLIT_SHORT_LE(ISF_MIRROR(user_id)),

    ISF_LEN(optional_command_list), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(optional_command_list)),
    ISF_ID(optional_command_list),
    ISF_MOD(optional_command_list),
    SPLIT_SHORT_LE(ISF_BASE(optional_command_list)),
    SPLIT_SHORT_LE(ISF_MIRROR(optional_command_list)),

    ISF_LEN(memory_size), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(memory_size)),
    ISF_ID(memory_size),
    ISF_MOD(memory_size),
    SPLIT_SHORT_LE(ISF_BASE(memory_size)),
    SPLIT_SHORT_LE(ISF_MIRROR(memory_size)),

    ISF_LEN(table_query_size), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(table_query_size)),
    ISF_ID(table_query_size),
    ISF_MOD(table_query_size),
    SPLIT_SHORT_LE(ISF_BASE(table_query_size)),
    SPLIT_SHORT_LE(ISF_MIRROR(table_query_size)),

    ISF_LEN(table_query_results), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(table_query_results)),
    ISF_ID(table_query_results),
    ISF_MOD(table_query_results),
    SPLIT_SHORT_LE(ISF_BASE(table_query_results)),
    SPLIT_SHORT_LE(ISF_MIRROR(table_query_results)),

    ISF_LEN(hardware_fault_status), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(hardware_fault_status)),
    ISF_ID(hardware_fault_status),
    ISF_MOD(hardware_fault_status),
    SPLIT_SHORT_LE(ISF_BASE(hardware_fault_status)),
    SPLIT_SHORT_LE(ISF_MIRROR(hardware_fault_status)),

    ISF_LEN(application_extension), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(application_extension)),
    ISF_ID(application_extension),
    ISF_MOD(application_extension),
    SPLIT_SHORT_LE(ISF_BASE(application_extension)),
    SPLIT_SHORT_LE(ISF_MIRROR(application_extension)),
};




/// This array contains stock codes for isfs.  They are ordered strings.
#if (CC_SUPPORT == GCC)
const ot_u8 isfs_stock_codes[] __attribute__((section(".vl_isfs"))) = {
#elif (CC_SUPPORT == CL430)
#pragma DATA_SECTION(isfs_stock_codes, ".vl_isfs")
const ot_u8 isfs_stock_codes[] = {
#endif
    0x10, 0x11, 0x18, 0xFF,
    0x12, 0x13, 0x14, 0xFF,
    0x15, 0xFF,
    0x16, 0xFF,
    0x00, 0x01,
    0x01, 0x06, 0x07, 0xFF,
    0x02, 0x03, 0x04, 0x05,
    0x11, 0xFF,
};


#if (GFB_TOTAL_BYTES > 0)
#if (CC_SUPPORT == GCC)
__attribute__((section(".vl_gfb")))
#elif (CC_SUPPORT == CL430)
#pragma DATA_SECTION(gfb_stock_files, ".vl_gfb")
#endif
const ot_u8 gfb_stock_files[] = {0xFF, 0xFF};
#endif




/// Firmware & Version information for ISF1 (Device Features)
/// This will look something like "OTv1  xyyyyyyy" where x is a letter and
/// yyyyyyy is a Base64 string containing a 16 bit build-id and a 32 bit mask
/// indicating the features compiled-into the build.
#include <otsys/version.h>

#define BV0     (ot_u8)(OT_VERSION_MAJOR + 48)
#define BT0     (ot_u8)(OT_BUILDTYPE)
#define BC0     OT_BUILDCODE0
#define BC1     OT_BUILDCODE1
#define BC2     OT_BUILDCODE2
#define BC3     OT_BUILDCODE3
#define BC4     OT_BUILDCODE4
#define BC5     OT_BUILDCODE5
#define BC6     OT_BUILDCODE6
#define BC7     OT_BUILDCODE7

/// This array contains the stock ISF data.  ISF data must be big endian!
#if (CC_SUPPORT == GCC)
const ot_u8 isf_stock_files[] __attribute__((section(".vl_isf"))) = {
#elif (CC_SUPPORT == CL430)
#pragma DATA_SECTION(isf_stock_files, ".vl_isf")
const ot_u8 isf_stock_files[] = {
#endif
    /* network settings: id=0x00, len=8, alloc=8 */
    __VID,                                              /* VID */
    0x11,                                               /* Device Subnet */
    0x11,                                               /* Beacon Subnet */
    SPLIT_SHORT(OT_ACTIVE_SETTINGS),                    /* Active Setting */
    0x00,                                               /* Default Device Flags */
    1,                                                  /* Beacon Attempts */
    SPLIT_SHORT(2),                                     /* Hold Scan Sequence Cycles */

    /* device features: id=0x01, len=48, alloc=48 */
    __UID,                                              /* UID: 8 bytes*/
    SPLIT_SHORT(OT_SUPPORTED_SETTINGS),                 /* Supported Setting */
    M2_PARAM(MAXFRAME),                                 /* Max Frame Length */
    1,                                                  /* Max Frames per Packet */
    SPLIT_SHORT(0),                                     /* DLLS Methods */
    SPLIT_SHORT(0),                                     /* NLS Methods */
    SPLIT_SHORT(ISF_TOTAL_BYTES),                       /* ISFB Total Memory */
    SPLIT_SHORT(ISF_TOTAL_BYTES-ISF_HEAP_BYTES),        /* ISFB Available Memory */
    SPLIT_SHORT(ISFS_TOTAL_BYTES),                      /* ISFSB Total Memory */
    SPLIT_SHORT(ISFS_TOTAL_BYTES-ISFS_HEAP_BYTES),      /* ISFSB Available Memory */
    SPLIT_SHORT(GFB_TOTAL_BYTES),                       /* GFB Total Memory */
    SPLIT_SHORT(GFB_TOTAL_BYTES-GFB_HEAP_BYTES),        /* GFB Available Memory */
    SPLIT_SHORT(GFB_FILE_BYTES),                        /* GFB File Size */
    0,                                                  /* RFU */
    OT_PARAM(SESSION_DEPTH),                          /* Session Stack Depth */
    'O','T','v',BV0,' ',' ',
    BT0,BC0,BC1,BC2,BC3,BC4,BC5,BC6,BC7, 0,             /* Firmware & Version as C-string */

    /* channel configuration: id=0x02, len=32, alloc=48 */
    0x00, 0x00,                                         /* Header: 2 bytes RFU */
    0x00,                                               /* Header: Regulatory Code */
    0x00,                                               /* Header: TX Duty Cycle */
    0x00,                                               /* Header: TX Power Autoscaling Control */
    0x00,                                               /* Header: RFU */

    (0x1F), 0x00,                                       /* Channel Spectrum ID & RFU */
    (ot_u8)(( (-2) + 40 )*2),                           /* Channel TX Power Limit */
    (ot_u8)( 140 ),                                     /* Channel Link Quality Filter Level */
    (ot_u8)( (-88) + 140 ),                             /* CS RSSI Threshold */
    (ot_u8)( (-80) + 140 ),                             /* CCA RSSI Threshold*/

    (0x2F), 0x00,                                       /* Channel Spectrum ID & RFU */
    (ot_u8)(( (-2) + 40 )*2),                           /* Channel TX Power Limit */
    (ot_u8)( 140 ),                                     /* Channel Link Quality Filter Level */
    (ot_u8)( (-88) + 140 ),                             /* CS RSSI Threshold */
    (ot_u8)( (-80) + 140 ),                             /* CCA RSSI Threshold*/

    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,


    /* real time scheduler: id=0x03, len=12, alloc=12 */
    0x00, 0x0F,                                         /* HSS Sync Mask */
    0x00, 0x08,                                         /* HSS Sync Value */
    0x00, 0x03,                                         /* SSS Sync Mask */
    0x00, 0x02,                                         /* SSS Sync Value */
    0x00, 0x03,                                         /* BTS Sync Mask */
    0x00, 0x02,                                         /* BTS Sync Value */

    /* hold scan periods: id=0x04, len=8, alloc=32 */
    /* Period data format in Section X.9.4.5 of Mode 2 spec */
    0x07, 0x40, 0x04, 0x00,                             /* Channel X scan, Scan Code, Next Scan ms */ //FIX
    0x17, 0x40, 0x08, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,

    /* sleep scan periods: id=0x05, len=4, alloc=32 */
    /* Period data format in Section X.9.4.5 of Mode 2 spec */
    0x07, 0x50, 0x0C, 0x00,                             /* Channel X scan, Scan Code, Next Scan ms */
    0xFF, 0xFF, 0xFF, 0xFF,                             /* NOTE: Scan Code should be less than     */
    0xFF, 0xFF, 0xFF, 0xFF,                             /*       Next Scan, or else you will be    */
    0xFF, 0xFF, 0xFF, 0xFF,                             /*       doing nothing except scanning!    */
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,

    /* beacon transmit periods: id=0x06, len=16, alloc=24 */
    /* Period data format in Section X.9.4.7 of Mode 2 spec */ //0x0240
    0x07, 0x06, 0x20, 0x00, 0x00, 0x08, 0x01, 0x00,     /* Channel X beacon, Beacon ISF File, Next Beacon ms */
    0x2A, 0x06, 0x20, 0x00, 0x00, 0x08, 0x0B, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* App Protocol List: id=0x07, len=4, alloc=16 */
    0x00, 0x01, 0x02, 0x04, 0xFF, 0xFF, 0xFF, 0xFF,     /* List of Protocols supported (Tentative)*/
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* ISFS list: id=0x08, len=12, alloc=16 */
    0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x18,
    0x80, 0x81, 0x82, 0x83, 0xFF, 0xFF, 0xFF, 0xFF,

    /* GFB File List: id=0x09, len=0, alloc=8 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* Location Data List: id=0x0A, len=0, alloc=64 */
    /* Stored exclusively in Mirror */

    /* IPv6 Addresses: id=0x0B, len=0, alloc=48 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* Sensor List: id=0x0C, len=16, alloc=16 (just dummy values right now) */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x00,

    /* Sensor Alarms: id=0x0D, len=2, alloc=2 (just dummy values right now) */
    0x00, 0x00,

    /* root auth key:       id=0x0E, not used in this build */
    /* Admin auth key:      id=0x0F, not used in this build */

    /* Routing Code: id=0x10, len=0, alloc=50 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF,

    /* User ID: id=0x11, len=0, alloc=60 */
    'A','P','P','=','P','o','n','g','L','T',' ',
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF,

    /* Mode 1 Optional Command list: id=0x12, len=7, alloc=8 */
    0x13, 0x93, 0x0C, 0x0E, 0x60, 0xE0, 0x8E, 0xFF,

    /* Mode 1 Memory Size: id=0x13, len=12, alloc=12 */
    0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    /* Mode 1 Table Query Size: id=0x14, len=1, alloc=2 */
    0x00, 0xFF,

    /* Mode 1 Table Query Results: id=0x15, len=7, alloc=8 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,

    /* HW Fault Status: id=0x16, len=3, alloc=4 */
    0x00, 0x00, 0x00, 0xFF,

    /* Application Extension: id=0xFF, len=0, alloc=64 */
    /* Stored Exclusively in mirror */
};



//__attribute__((section(".vl_fallow")))
//const ot_u8 vl_fallow_space[ (FLASH_PAGE_SIZE*OTF_VWORM_FALLOW_PAGES) ];
#endif

