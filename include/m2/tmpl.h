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
  * @file       /include/m2/tmpl.h
  * @author     JP Norair
  * @version    R101
  * @date       24 Mar 2014
  * @brief      Templates for M2, usually used for organizing ALP-API data 
  *             into queued form that is assimilable by API functions.  Some 
  *             tmpls are also used within the OTlib itself.
  *
  * Note:   All templates must be even-byte-aligned (i.e. halfword aligned) and
  *         in doing so, developers should use OT's explicit types instead of 
  *         implicit types.  For example, use ot_u16 instead of ot_uint.
  *
  *         Templates that are not even-byte-aligned or are not using explicit
  *         types are not currently being used -- they are just placeholders.
  *         Do make sure to fix them (if any exist) if you are going to build
  *         some new functionality that uses them.  To make even-byte-aligned,
  *         pad the struct with an ot_u8 (call it "reserved").
  *
  * Note:   There is no official requirement for the endianess of tmpl I/O via
  *         the ALP-API queue, although big endian is generally used because 
  *         that is what the ot_queue module supports.  You could potentially make 
  *         some optimizations by matching the endianess of the client & server.
  *
  ******************************************************************************
  */


#ifndef __M2_TMPL_H
#define __M2_TMPL_H

#include <otsys/types.h>


#define __SIZEOF(TMPL)      (__SIZEOF_##TMPL)
#define __ALLOCOF(TMPL)     ((__SIZEOF_##TMPL + (PLATFORM_WORD_SIZE-1))/PLATFORM_WORD_SIZE)


/// Veelite Templates
#define __SIZEOF_vladdr_tmpl (1+1)
typedef struct {
    ot_u8   block;
    ot_u8   id;
} vladdr_tmpl;

#define __SIZEOF_vlheader_tmpl (1+1+1+1+2+2)
typedef struct {
    ot_u8   block;
    ot_u8   id;
    ot_u8   permissions;
    ot_u8   is_mirrored;
    ot_u16  length;
    ot_u16  alloc;
} vlheader_tmpl;

#define __SIZEOF_vldata_tmpl (2+2+(1*PLATFORM_POINTER_SIZE))
typedef struct {
    ot_u16  offset;
    ot_u16  bytes;
    ot_u8*  data;
} vldata_tmpl;




/// Session Template

#define __SIZEOF_session_tmpl (1+1+1+1+1+1)
typedef struct {
    ot_u8   reserved;
    ot_u8   channel;
    ot_u8   subnet;
    ot_u8   subnetmask;
    ot_u8   flags;
    ot_u8   flagmask;
} session_tmpl;





/// Mode 2 MAC/Protocol Templates
//typedef enum {
//    ADDR_unicast    = 0,    //2
//    ADDR_broadcast  = 1,    //1
//    ADDR_anycast    = 2,    //removed
//    ADDR_multicast  = 3     //removed
//} addr_type;
typedef enum {
    ADDR_stream     = 0,
    ADDR_broadcast  = 1,
    ADDR_unicast    = 2,
    ADDR_unicastvid = 3
} addr_type;

#define ADDR_Type   addr_type


#define __SIZEOF_adv_tmpl (1+1+1+1+2)
typedef struct {
    ot_u8  duty_off;
    ot_u8  duty_on;
    ot_u8  subnet;
    ot_u8  channel;
    ot_u16 duration;
} advert_tmpl;


#define __SIZEOF_chanlist_tmpl (2+(1*PLATFORM_POINTER_SIZE))
typedef struct {
    ot_u16 length;
    ot_u8* list;
} chanlist_tmpl;


#define __SIZEOF_csma_tmpl (1+2+2+2)
typedef struct {
    ot_u8  csma_type;
    ot_u16 csma_guard_us;
    ot_u16 csma_guess_us;       // usually determined by algorithm
    ot_u16 csma_timeout;        // usually determined by algorithm
} csma_tmpl;


#define __SIZEOF_id_tmpl (1+(1*PLATFORM_POINTER_SIZE))
typedef struct {
    ot_u8 length;
    ot_u8* value;
} id_tmpl;


#define __SIZEOF_idlist_tmpl (1+1+(1*PLATFORM_POINTER_SIZE))
typedef struct {
    ot_u8 idcount;
    ot_u8 listlen;
    ot_u8* idlist;
} idlist_tmpl;


#define __SIZEOF_routing_tmpl (1+1+(3*__ALLOCOF(id_tmpl))
typedef struct {
    ot_u8   hop_code;  
    ot_u8   hop_ext;
    id_tmpl dlog;
    id_tmpl orig;
    id_tmpl dest;
} routing_tmpl;







/// Mode 2 Protocol Templates

// Legacy
//#define __SIZEOF_header_tmpl (1+1+1+1)
//typedef struct {
//    ot_u8   df_mask;
//    ot_u8   df_values;
//    ot_u8   cmd_code;
//    ot_u8   cmd_ext;
//} header_tmpl;


#define __SIZEOF_ack_tmpl (1+1+(1*PLATFORM_POINTER_SIZE))
typedef struct {
    ot_u8   count;
    ot_u8   length;
    ot_u8*  list;
} ack_tmpl;


#define __SIZEOF_command_tmpl (1+1+1)
typedef struct {
    ot_u8  type;
    ot_u8  opcode;
    ot_u8  extension;
} command_tmpl; 


typedef enum {
    CMDEXT_none             = 0,
    CMDEXT_no_response      = 0x02,
    CMDEXT_no_csma          = 0x04,
    CMDEXT_ca_raind         = (1<<3),
    CMDEXT_ca_aind          = (2<<3),
    CMDEXT_ca_mac           = (7<<3),
    CMDEXT_null_file        = (1<<6)
} command_extensions;


typedef enum {
    CMD_announce_file            = 0,
    CMD_announce_series          = 1,
    CMD_inventory_on_file        = 2,
    CMD_inventory_on_series      = 3,
    CMD_udp_on_file              = 4,
    CMD_udp_on_series            = 5,
    CMD_collect_file_on_file     = 6,
    CMD_collect_series_on_file   = 7,
    CMD_collect_file_on_series   = 8,
    CMD_collect_series_on_series = 9,
    CMD_request_datastream       = 12,
    CMD_propose_datastream       = 13,
    CMD_ack_datastream           = 14,
    CMD_reserved                 = 15
} command_opcodes;


//typedef enum {
//    CMDTYPE_response            = 0,
//    CMDTYPE_error               = (1 << 4),
//    CMDTYPE_na2p_request        = (2 << 4),
//    CMDTYPE_a2p_init_request    = (4 << 4),
//    CMDTYPE_a2p_inter_request   = (5 << 4),
//    CMDTYPE_a2p_final_request   = (7 << 4)
//} command_types;

typedef enum {
    CMDTYPE_response            = 0,
    CMDTYPE_control             = (1 << 4),
    CMDTYPE_bcast_request       = (2 << 4),
    CMDTYPE_acast_request       = (3 << 4),
    CMDTYPE_mcast_request       = (4 << 4),
    CMDTYPE_mcast_continue      = (5 << 4),
    CMDTYPE_mcast_finish        = (7 << 4),
} command_types;



#define __SIZEOF_error_tmpl (1+1+(1*PLATFORM_POINTER_SIZE))
typedef struct {
    ot_u8   code;
    ot_u8   subcode;
    ot_u8*  data;
} error_tmpl;


#define __SIZEOF_dialog_tmpl (1+1+(1*PLATFORM_POINTER_SIZE))
typedef struct {
    ot_u8  timeout;
    ot_u8  channels;
    ot_u8* chanlist;
} dialog_tmpl; 


#define __SIZEOF_dp_tmpl (2+1+1+(1*PLATFORM_POINTER_SIZE))
typedef struct {
    ot_u16  data_condition;
    ot_u8   length;
    ot_u8   pid;
    ot_u8*  data;
} dp_tmpl;


#define __SIZEOF_isfcall_tmpl (1+1+2+2)
typedef struct {
    ot_u8   is_series;
    ot_u8   isf_id;
    ot_s16  offset;
    ot_s16  max_return;
} isfcall_tmpl;


#define __SIZEOF_isfcomp_tmpl (1+1+2)
typedef struct {
    ot_u8   is_series;
    ot_u8   isf_id;
    ot_s16  offset;
} isfcomp_tmpl;


#define __SIZEOF_isfreturn_tmpl (1+1+1+(1*PLATFORM_POINTER_SIZE))
typedef struct {
    ot_u8   isf_id;
    ot_u8   offset;
    ot_u8   length;
    ot_u8*  data;
} isfreturn_tmpl;


#define __SIZEOF_isseriesreturn_tmpl (1+1+2+2+(2*PLATFORM_POINTER_SIZE))
typedef struct {
    ot_u8   iss_id;
    ot_u8   series_length;
    ot_s16  contents_offset;
    ot_s16  content_length;
    ot_u8*  series_data;
    ot_u8*  contents_data;
} isseriesreturn_tmpl;


typedef enum {
    QCODE_ismasked          = 0x80,
    QCODE_null              = 0x00,
    QCODE_nonnull           = 0x01,
    QCODE_notequal          = 0x20,
    QCODE_equal             = 0x21,
    QCODE_lessthan          = 0x22,
    QCODE_lessthan_equal    = 0x23,
    QCODE_greaterthan       = 0x24,
    QCODE_greaterthan_equal = 0x25,
    QCODE_search            = 0x40
} query_codes;


#define __SIZEOF_query_tmpl (1+1+(2*PLATFORM_POINTER_SIZE))
typedef struct {
    ot_u8   code;
    ot_u8   length;
    ot_u8*  mask;
    ot_u8*  value;
} query_tmpl;


#define __SIZEOF_shell_tmpl (1+1+2+(1*PLATFORM_POINTER_SIZE))   //deprecated
#define __SIZEOF_udp_tmpl (1+1+2+(1*PLATFORM_POINTER_SIZE))
typedef struct {
    ot_u16  data_length;
    ot_u8   dst_port;
    ot_u8   src_port;
    ot_u8*  data;
} udp_tmpl;




#endif





