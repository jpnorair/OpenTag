/* Copyright 2013 JP Norair
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
  * @file       /otlib/m2_network.h
  * @author     JP Norair
  * @version    R101
  * @date       11 Sep 2013
  * @brief      Network Layer interface for DASH7 Mode 2
  * @ingroup    Network
  *
  * Includes implementation for the following protocols:
  * 1. M2AdvP
  * 2. M2ResP (not implemented in this version)
  * 3. M2NP
  * 4. M2DP
  ******************************************************************************
  */


#include "OT_config.h"
#if !defined(__M2_NETWORK_H) /* && OT_FEATURE(M2) */
#define __M2_NETWORK_H

#include "OT_types.h"
#include "session.h"
#include "alp.h"

/** M2NP Routing Parameters
  * ============================================================================
  * @todo adjust to lastest spec
  */

#define M2LC_FRCONT             (1<<7)
#define M2LC_RSCODE             (1<<6)
#define M2LC_DMGMARK            (1<<5)
#define M2LC_CRC5               (31<<0)

#define M2FI_LISTEN             (1<<7)
#define M2FI_DLLS               (1<<6)
#define M2FI_NLS                (1<<5)
#define M2FI_VID                (1<<4)
#define M2FI_EXT                (1<<3)
#define M2FI_STREAM             (1<<2)
#define M2FI_ADDRMASK           (3<<0)
#define M2FI_UNICAST            (0<<0)
#define M2FI_BROADCAST          (1<<0) 
#define M2FI_ANYCAST            (2<<0)
#define M2FI_MULTICAST          (3<<0)

// Routing Aliases
#define M2FI_FRDIALOG           0
#define M2RT_UNICAST            M2FI_UNICAST
#define M2RT_BROADCAST          M2FI_BROADCAST
#define M2RT_ANYCAST            M2FI_ANYCAST
#define M2RT_MULTICAST          M2FI_MULTICAST

// Query Aliases
#define M2QUERY_GLOBAL          M2FI_ANYCAST
#define M2QUERY_LOCAL           M2FI_BROADCAST


// EXT stuff, rarely used
#define M2EXT_APPFLAGS          (15<<0)
#define M2EXT_RFU               (15<<4)

// M2NP Command Code Options
#define M2CC_SLEEP              (0x01 << 4) 
#define M2CC_EXT                (0x01 << 5) 

// Mode 2 Hop Control Options
#define M2HC_HOPMASK            (0x0F)
#define M2HC_VID                (0x01 << 4)
#define M2HC_DEST               (0x01 << 5)
#define M2HC_ORIG               (0x01 << 6) 
#define M2HC_EXT                (0x01 << 7) 

// Application Device Flags
#define M2DF_EALARM             (0x01 << 3)
#define M2DF_SALARM             (0x01 << 4)
#define M2DF_LOWBATT            (0x01 << 5)
#define M2DF_SYSFAULT           (0x01 << 6)
#define M2DF_NACK               (0x01 << 7)

// Datastream Flags
#define M2DS_DISABLE_ACKREQ      (0x01 << 7)
#define M2DS_DISABLE_ACKRESP     (0x01 << 1)




typedef struct {
    ot_u8   fr_info;
    ot_u8   ext_info;
} header_struct;


typedef struct {
    ot_sigv2 route;
} m2npsig_struct;


typedef struct {
    routing_tmpl    rt;
    header_struct   header;
#   if (OT_FEATURE(M2NP_CALLBACKS) == ENABLED) 
        m2npsig_struct  signal;    
#   endif
} m2np_struct;


typedef struct {
    ot_u8   ctl;
    //ot_u8   fr_total;
    //ot_u8   fr_per_pkt;
    ot_u8   dmg_count;
    //ot_u16  data_total;
} dscfg_struct;


typedef struct {
    dscfg_struct    dscfg;
} m2dp_struct;


#if (OT_FEATURE(M2DP) == ENABLED)
    extern m2dp_struct m2dp;
#endif
extern m2np_struct m2np;







/** Low-Level Network Functions
  * ============================================================================
  * - In some OSI models, these might be in the "LLC" layer of the MAC.  They
  *   fit more nicely and cleanly in this module, though.
  */

/** @brief  Initializes all persistent network data (objects)
  * @param  None
  * @retval None
  * @ingroup Network
  * @sa sys_init()
  *
  * Call when initializing your runtime application (it gets called within 
  * sys_init() in system.c)
  */
void network_init();


/** @brief  parses a background frame, namely one using M2AdvP
  * @param  session     (m2session*) Pointer to active session
  * @retval ot_int      -1 on ignore, non-negative on session modified/created
  * @ingroup Network
  */
m2session* network_parse_bf();




/** @brief  Marks a foreground frame as damaged (CRC is bad).
  * @param  none
  * @retval none
  * @ingroup Network
  *
  * This function will set the frametype to 3 (damaged frame).  With OpenTag,
  * this is important only for multi-frame packets.  single-frame packets with
  * CRC violation are treated as a bad packet directly by the MAC layer.
  */
void network_mark_ff();



/** @brief  parses and routes a foreground frame
  * @param  active     (m2session*) Pointer to active session
  * @retval ot_int      -1 on ignore, non-negative is Routing Index of the
  *                     device/host that should be forwarded the message.
  * @ingroup Network
  *
  * The routing index return value will always be 0 if this device does not 
  * support multihop routing.  0 is the Routing Index of the localhost.  Any 
  * other positive return value is the index of the routing table which 
  * corresponds to the host that shall be forwarded the frame.
  */
ot_int network_route_ff(m2session* active);



/** @brief  Continues a dialog/session by cloning the existing session
  * @param  applet      (ot_app) Applet pointer for new session
  * @param  wait        (ot_uint) Number of ticks to wait following dll-idle
  * @retval m2session*  Pointer to newly cloned session
  * @ingroup Network
  *
  * You must pass a valid applet into the "applet" argument, or NULL.  NULL
  * will follow the default session behavior, which is simply to respond
  * appropriately to requests.  Passing-in session_top()->applet will use 
  * the applet from the current session.
  *
  * The "wait" argument is a tail-chained number of ticks that starts 
  * counting after the DLL goes to idle.  For example, the DLL goes to
  * idle after the response window expires.
  */
m2session* network_cont_dialog(ot_app applet, ot_uint wait);



/** @brief  Optional static callback function for network router
  * @param  route       (void*) Pointer to routing status information (ot_int*)
  * @param  active      (void*) Pointer to active session
  * @retval None
  * @ingroup Network
  * @sa network_route_ff()
  *
  * The user application code can implement this function when the constant 
  * EXTF_network_sig_route is defined in extf_config.h.  network_route_ff() 
  * will call it (or m2np.signal.route(), which is the dynamic version of this
  * function) after it does the routing but immediately before it returns.
  *
  * In applications that do not need dynamic callbacks, some program code and
  * global memory can be saved by using static callbacks.
  */
void network_sig_route(void* route, void* active);







/** M2NP Network Functions
  * ============================================================================
  * - M2NP = Mode 2 Network Protocol.
  * - Routable, primary data-networking protocol for DASH7 Mode 2.
  */

/** @brief  Loads Mode 2 DLL and Network layer frame headers into the TX queue
  * @param  active      (m2session*) Pointer to active session
  * @param  addressing  (ot_u8) forced addressing flags per Mode 2 DLL spec
  * @param  nack        (ot_u8) set to 1 to declare frame as NACK, else set to 0
  * @retval None
  * @ingroup Network
  *
  * Call this function when you have the session allocated, and you are 
  * beginning the process of generating an M2NP-type transmission.
  */
void m2np_header(m2session* active, ot_u8 addressing, ot_u8 nack);



/** @brief  Loads Mode 2 DLL and Network layer frame footers into the TX queue
  * @retval None
  * @ingroup Network
  *
  * Call this function when you have constructed an M2NP-type frame, and after 
  * you have appended the payload.  It will attach the Mode 2 DLL + M2NP footer
  * which at least contains the CRC.  It will also calculate the frame length
  * and set that field of the frame accordingly.
  */
void m2np_footer();



/** @brief  Appends to the TX Queue the Device ID of this Device
  * @param  use_vid     (ot_bool) Set to True to use 2 byte Virtual ID
  * @retval None
  * @ingroup Network
  *
  * This function is typically used by upper layer protocols that need to refer
  * to this device's ID in their transmission.
  */
void m2np_put_deviceid(ot_bool use_vid);



/** @brief  Compares void input string to this Device's ID
  * @param  length      (ot_int) use 2 or 8 to select VID or UID comparison
  * @param  id          (*void) pointer to device ID for comparison
  * @retval None
  * @ingroup Network
  *
  * In typical usage, the "id" parameter comes from the RX Queue, applied as
  * in the following example:
  * <PRE> if (m2np_idcmp(length, q_markbyte(&rxq, length)) { ... } </PRE>
  *
  * The "length" parameter should be 2 or 8 in order for this usage to work
  * properly with Mode 2 Device ID types (2byte VID and 8byte UID).
  */
ot_bool m2np_idcmp(ot_int length, void* id);








/** M2AdvP Network Functions
  * ============================================================================
  * - M2AdvP = Advertising Protocol, i.e. flooding for group synchronization.
  */

/** @brief  Prepares an M2AdvP flood from a session container
  * @param  follower    (m2session*) pointer to session after advertising
  * @retval none
  * @ingroup Network
  * @sa m2advp_close()
  * @sa m2advp_init_flood()
  *
  * This function is typically called by the Data Link Layer / System module
  * in order to begin an M2AdvP flood process.  It reconfigures the TX Queue
  * to transmit background frames.
  */
void m2advp_open(m2session* follower);



/** @brief  Reverts the data framing from flood to normal mode
  * @param  none
  * @retval none
  * @ingroup Network
  * @sa m2advp_open()
  *
  * This function is typically called by the Data Link Layer / System module
  * in order to end an M2AdvP flood process.  It reconfigures the TX Queue
  * to transmit foreground frames (the normal configuration).
  */
void m2advp_close();



/** @brief  Updates an open flood process
  * @param  countdown
  * @retval none
  * @ingroup Network
  * @sa m2advp_open()
  *
  * The system / data link layer / kernel should supply the flood countdown to
  * this function when the next flood packet is being loaded.
  */
void m2advp_update(ot_u16 countdown);



/* @brief  Initializes the M2AdvP Flood
  * @param  session     (m2session*) Pointer to active session
  * @param  schedule    (ot_u16) number of ticks to run flood
  * @retval ot_int      -1 if there's an error preventing flood initialization
  * @ingroup Network
  * @sa m2advp_open()
  * @sa m2advp_close()
  *
  * This function is typically called by the Data Link Layer / System module
  * after using m2advp_open() to initialize an M2AdvP Flood.  In the current
  * implementation, the maximum value for the parameter "schedule" may be
  * safely assumed to be 32750.  Additionally, the return value is only 
  * relevant to Debug builds and builds in which flooding is unavailable.
  */
//ot_int m2advp_init_flood(m2session* session, ot_u16 schedule);






/** M2DP Network Functions
  * ============================================================================
  * <LI> M2DP = Mode 2 Datastream Protocol  </LI>
  * <LI> Very little overhead, good for arbitrary data encapsulation  </LI>
  * <LI> For OpenTag usage, M2DP frames may only occur in a multiframe packet,
  *      as part of datastream transport.  In this case, the first frame in
  *      the packet is always an M2NP frame.  </LI>
  * <PRE>
  * M2DP Frame Format
  * 
  * 0     1        4     4+a         4+a+n    
  * +-----+--------+------+------------+---------------+-----+
  * | Len | FF Hdr | DLLS | UL Payload | DLLS Padding  | CRC |
  * +-----+--------+------+------------+---------------+-----+
  *                 [Opt]       n
  * 
  * The Upper Layer Payload and Padding may be encrypted.  Padding bytes take
  * random values, and the amount of padding is specified in DLLS field.
  *
  * - FF Hdr Info Field must have En_Addr = 0 and Frame_Type = 2
  * - a: length of DLLS header.  Typcially 1 byte in M2DP.
  * - n: length of Upper Layer Payload
  * </PRE>
  */

/** @brief  Appends an M2DP frame to the packet, and adjusts previous frame
  * @param  none
  * @retval none
  * @ingroup Network
  *
  * OpenTag only supports usage of M2DP frames as subsequent frames in 
  * multiframe packets, where the first frame is always an M2NP frame.  This
  * function can be used to append such an M2DP frame to the packet currently
  * in the transmit queue.
  *
  * m2dp_append() does three things:
  * <LI> only writes the header(s) of the M2DP frame, and it a
  */
void m2dp_append();



/** @brief  Writes the footer of the M2DP frame, thus finishing the frame
  * @param  none
  * @retval none
  * @ingroup Network
  */
void m2dp_footer();





#endif

