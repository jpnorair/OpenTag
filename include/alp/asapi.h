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
  * @file       /hb_asapi/hb_asapi.h
  * @author     JP Norair
  * @version    R102
  * @date       20 Mar 2014
  * @brief      Abridged Session API for H-Builder
  * @defgroup   hb_asapi (H-Builder Abridged Session API)
  * @ingroup    hb_asapi
  *
  * ASAPI is an ALP-based library for creating and managing DASH7 sessions on
  * an OpenTag server, via a client.  It is geared towards M2QP query
  * operations, including UDP support.
  *
  * The ASAPI protocol is an ALP with ID=0x07.
  *
  * ASAPI Server
  * This ASAPI Server implementation is a task-less, non-atomic ALP. Therefore,
  * the OpenTag server that runs the ASAPI server does not require a task in
  * addition to the built-in Mode 2 task.  An ASAPI Applet is bound to a Mode 2
  * session that the ASAPI server creates when it needs to invoke Mode 2
  * communications.  The Mode 2 Task takes care of the rest, in combination
  * with callbacks to this ASAPI Applet, which manages its own state.
  *
  * ASAPI Client
  * The client functions are simple library functions for generate ASAPI ALP
  * protocol messages for sending to the server, and manage state based on the
  * ACK responses to these messages.  These functions must be integrated into
  * the communication model of the client.
  *
  *
  * +-------+-------------------------+
  * | B0    | ALP Flags               |
  * | B1    | ALP record length       |
  * | B2    | ALP ID (0x07)           |
  * | B3    | ASAPI ALP-CMD (bitmask) |
  * +-------+-------------------------+
  *
  * ASAPI ALP-CMD Structure
  * +------+------+-----+--------+------+------+--------+
  * | RESP | HOLD | EXT | STATUS | INIT | DLLS |  ADDR  |
  * |  b7  |  b6  | b5  |   b4   |  b3  |  b2  |  b1:0  |
  * +------+------+-----+--------+------+------+--------+
  *
  * Template Ordering in ASAPI
  * +-----------+-------+-------+-----------------------------------+
  * | Trans ID  | ---   | 2     | Always Present                    |
  * | EXT       | Std   | 1     | CMD.EXT=1                         |
  * | Status    | Std   | 4     | CMD.STATUS=1                      |
  * | Init      | Std   | 8     | CMD.INIT=1                        |
  * | Dialog    | Std   | 6     | CMD.INIT=0                        |
  * | DLLS      | Std   | 2     | CMD.INIT=0 && CMD.DLLS=1          |
  * | Routing   | Std   | 10    | CMD.INIT=0 && Unicast             |
  * | Filecomp  | Std   | 4     | CMD.INIT=0 && (M.cast or A.cast)  |
  * | Filecall  | Ext   | 6     | CMD.INIT=0 && EXT.FILECALL=1      |
  * | G. Query  | Std   | 2-34  | CMD.INIT=0 && Multicast           |
  * | L. Query  | Std   | 2-34  | CMD.INIT=0 && (M.cast or A.cast)  |
  * | UDP       | Ext   | 4+    | CMD.INIT=0 && EXT.UDP=1           |
  * +-----------+-------+-------+-----------------------------------+
  *
  * Special commands
  * +-------+------------------------+------------------+---------------+
  * | NAME  | DESCRIPTION            | DIRECTION        | BITS          |
  * | Kill  | Ends & Scrubs Server   | Client->Server   | STATUS+INIT   |
  * +-------+------------------------+------------------+---------------+
  *
  ******************************************************************************
  */


#ifndef _HB_ASAPI_H_
#define _HB_ASAPI_H_

#include <otstd.h>
#include <otlib.h>
//#include <m2/tmpl.h>

#if (OT_FEATURE(SERVER))
#   include <m2api.h>
#endif


/// ID value
#define ASAPI_ID                0x07
#define ASAPI_ALP_ID            ASAPI_ID

/// CMD byte, a bitfield describing templates that are present within the data
#define ASAPI_CMD_RESPONSE      (1<<7)
#define ASAPI_CMD_HOLDOPEN      (1<<6)
#define ASAPI_CMD_EXT           (1<<5)
#define ASAPI_CMD_STATUS        (1<<4)
#define ASAPI_CMD_INIT          (1<<3)
#define ASAPI_CMD_DLLS          (1<<2)
#define ASAPI_CMD_ADDRMASK      (3<<0)
#define ASAPI_CMD_UNICAST       (0<<0)      //(0<<0)
#define ASAPI_CMD_BROADCAST     (1<<0)
#define ASAPI_CMD_ANYCAST       (2<<0)
#define ASAPI_CMD_MULTICAST     (3<<0)
#define ASAPI_CMD_FILECOMP      (ASAPI_CMD_ANYCAST)
#define ASAPI_CMD_GQUERY        (ASAPI_CMD_INIT+ASAPI_CMD_MULTICAST)
#define ASAPI_CMD_LQUERY        (ASAPI_CMD_ANYCAST)

/// EXT template, 1 byte
#define ASAPI_EXT_EXT           (1<<7)
#define ASAPI_EXT_FILECALL      (1<<6)
#define ASAPI_EXT_UDP           (1<<5)     // ignored if FILECALL = 1

/// ASAPI Error Codes used by server (all odd, positive)
/// In known OpenTag implementations, "ERR_STACKFULL" can never happen, because
/// the session module is designed to pop old sessions in favor of new sessions
/// if the stack is full.
#define ASAPI_ERR_SERVERBUSY    1                       // Server invoked on other ALP
#define ASAPI_ERR_PRIVILEGES    3                       // ALP user is not root
#define ASAPI_ERR_BADDATA       6                       // (just centerpoint for below)
#define ASAPI_ERR_TOOFEWBYTES   (ASAPI_ERR_BADDATA-1)   // missing data/parameters
#define ASAPI_ERR_TOOMANYBYTES  (ASAPI_ERR_BADDATA+1)   // more data than expected
#define ASAPI_ERR_STACKFULL     9                       // session stack is overloaded
#define ASAPI_ERR_NOSESSION     11                      // session msg expected, but missing
#define ASAPI_ERR_FLUSH         255                     // Not really an error

/// ASAPI Error Codes used by Client (all even, negative)
#define ASAPI_ERR_TRANSIDRESP   -3                      // (just centerpoint for below)
#define ASAPI_ERR_TRANSIDLOW    (ASAPI_ERR_TRANSIDRESP-1) //-4
#define ASAPI_ERR_TRANSIDHIGH   (ASAPI_ERR_TRANSIDRESP+1) //-2
#define ASAPI_ERR_NOTOPEN       -6                      // Client not open, can't add msg
#define ASAPI_ERR_MISSINGTMPLS  -8                      // Dialog or Init Tmpl is minimum
#define ASAPI_ERR_MALFORMEDDLLS -10
#define ASAPI_ERR_STREAMFULL    -12                     // Client output stream is full
#define ASAPI_ERR_MISSINGCOMP   -14
#define ASAPI_ERR_MISSINGGQUERY -16
#define ASAPI_ERR_MISSINGLQUERY -18
#define ASAPI_ERR_CALLANDUDP    -20
#define ASAPI_ERR_NOTASAPI      -128
#define ASAPI_ERR_BADRESPONSE   -130

/// Sizes of Templates
#define ASAPI_TRANSID_SIZE      2
#define ASAPI_EXT_SIZE          1
#define ASAPI_STATUS_SIZE       4

/// Delay template: 2 bytes describing time offset before starting new session
#define ASAPI_WAIT_MASK         (0xFFFF)
#define ASAPI_WAIT_SIZE         2


/** ASAPI Client-centric Data Types  <BR>
  * ========================================================================<BR>
  */
typedef struct {
    ot_u8   errcode;
    ot_u8   offset;
    ot_u16  freespace;
} asapi_status_tmpl;


typedef struct {
    isfcomp_tmpl    comp;
    query_tmpl      query;
    ot_u8           qvalue[16];
    ot_u8           qmask[16];
} asapi_search_tmpl;



/** ASAPI Client & Server Data Types  <BR>
  * ========================================================================<BR>
  */
typedef enum {
    MSG_Execute = 0,
    MSG_Hold    = ASAPI_CMD_HOLDOPEN
} MSG_Ctl;

typedef enum {
    ADDR_Unicast    = 0,
    ADDR_Broadcast  = 1,
    ADDR_Anycast    = 2,
    ADDR_Multicast  = 3
} ADDR_Code;


/// DLLS template: 2 bytes including DLLS
#define ASAPI_DLLS_SIZE     2
typedef struct {
    ot_u8   encrypt_index;
    ot_u8   user_index;
    ot_u8*  key;
} asapi_dlls_tmpl;


/// Init & Adv template: 8 bytes
/// @note region code determines the method of initialization.  For
#define ASAPI_INIT_SIZE     8
typedef struct {
    ot_u8       type_code;
    ot_u8       lc_flags;
    advert_tmpl adv;
} asapi_init_tmpl;


/// Dialog template: 4 bytes
#define ASAPI_DIALOG_SIZE   6
typedef struct {
    ot_u8   req_subnet;
    ot_u8   req_channel;
    ot_u8   resp_channel;
    ot_u8   resp_window_code;
    ot_u16  delay_to_next;
} asapi_dialog_tmpl;


/// Routing, no multihop
#define ASAPI_ROUTING_SIZE  10
typedef struct {
    ot_u8   hop_code;
    id_tmpl target;
} asapi_routing_tmpl;


/// Query: use query_tmpl from standard templates


/// isfcomp: use isfcomp_tmpl from standard templates


/// isfcall: uwse isfcall_tmpl from standard templates


/// udp: use udp_tmpl from standard templates
#define ASAPI_UDP_SIZE      4


///Derived ASAPI template sizes
#define ASAPI_UNICAST_SIZE      (ASAPI_ROUTING_SIZE+ASAPI_DIALOG_SIZE)
#define ASAPI_BROADCAST_SIZE    (ASAPI_DIALOG_SIZE)
#define ASAPI_ANYCAST_SIZE      (ASAPI_DIALOG_SIZE+__SIZEOF_isfcomp_tmpl)
#define ASAPI_MULTICAST_SIZE    (ASAPI_DIALOG_SIZE+__SIZEOF_isfcomp_tmpl)
#define ASAPI_FILECALL_SIZE     (__SIZEOF_isfcall_tmpl)












/** ASAPI Client functions  <BR>
  * ========================================================================<BR>
  * These are the primitive functions that run on the client, for the purpose
  * of building an output stream containing ASAPI message data.  There are two
  * variants: one variant that uses OpenTag ot_queues (ot_queue) and another
  * variant that simply dumps to a byte pointer, which the client must manage.
  */

#if (OT_FEATURE(CLIENT))

/** @brief ASAPI Server library initialization
  * @param None
  * @retval None
  * @ingroup asapi
  *
  * Initializes static memory elements of ASAPI with startup values.  Call this
  * only once during OpenTag startup.
  */
void asapi_init_cli();


/** @brief Opens an ASAPI Raw Client Output Stream on a given byte stream
  * @param out_stream   (ot_u8*) message output buffer pointer
  * @param alloc        (ot_int) maximum extent of output buffer
  * @retval None
  * @ingroup asapi
  * @sa asapi_addmsg_cli
  * @sa asapi_closeraw_cli
  *
  * Call this to open a raw output stream on the client.  A "raw" stream is
  * simply a stream that uses an unstructured byte-buffer as the interface.
  * asapi_addmsg() may be used to add messages to this stream, after open.
  */
void asapi_openraw_cli(ot_u8* out_stream, ot_int alloc);



/** @brief Closes the ASAPI Raw Client Output Stream
  * @param None
  * @retval None
  * @ingroup asapi
  * @sa asapi_openraw_cli
  *
  * Once a raw output stream is open, messages can be added to it.  This
  * function simply closes the raw stream so that asapi_addmsg_cli() cannot add
  * more data to it.
  *
  * The user is responsible for actually sending the data over the communication
  * interface of choosing.  This function only does the data protection and
  * buffer management aspect.
  */
void asapi_closeraw_cli();



/** @brief Process ASAPI protocol response data from a Raw Client Input Stream
  * @param status       (asapi_status_tmpl*) Server Status Response
  * @param stream       (ot_u8*)
  * @retval ot_int
  * @ingroup asapi
  *
  * Responses to ASAPI client messages are simple acknowledgements containing a
  * 32 bit status code.  This function will breakdown the ASAPI ALP protocol
  * data and return that status code.
  *
  * A list of status codes is documented in hb_asapi.h.  The status response
  * structure is:
  * <PRE>
  * +------------+--------------+---------------------------------+
  * | Error Code | Error Offset | Bytes remaining in Server ot_queue |
  * +------------+--------------+---------------------------------+
  * | 31      24 | 23        16 | 15                            0 |
  * +------------+--------------+---------------------------------+
  * </PRE>
  * "Error Offset" is used to describe the position in the message payload
  * where an error was detected, but not every error uses it.
  *
  * The lower 16 bits of the status value is an unsigned integer describing the
  * number of bytes remaining in the Server's ASAPI execution queue.
  */
ot_int asapi_procraw_cli( asapi_status_tmpl* status, ot_u8* stream);




/** @brief Build an ASAPI Client message from templates, and load it to output stream
  * @param hold_or_exec (MSG_Ctl) Control varible to delay or commence session execution
  * @param addressing   (ADDR_Code) Specifies Unicast, Broadcast, Anycast, or Multicast modes
  * @param init         (asapi_init_tmpl*) ASAPI initialization (optional)
  * @param dialog       (asapi_dialog_tmpl*) Dialog (mandatory)
  * @param dlls         (asapi_dlls_tmpl*) ASAPI DLLS (optional)
  * @param routing      (asapi_routing_tmpl*) Routing (conditional on unicast)
  * @param comp         (isfcomp_tmpl*) ISF file comparison (conditional on anycast/multicast)
  * @param call         (isfcall_tmpl*) ISF file call (optional)
  * @param g_query      (query_tmpl*) Global Query (conditional on anycast/multicast)
  * @param l_query      (query_tmpl*) Local Queury (conditional on multicast)
  * @param udp          (udp_tmpl*) UDP data (conditional on call==NULL)
  *
  * @retval ot_int      An input error code (if negative) or the number of
  *                     bytes remaining in the output stream (if non-negative)
  * @ingroup asapi
  *
  * asapi_addmsg_cli() is a big function for building ASAPI protocol messages.
  * The input templates may be set to NULL if they are optional or conditional,
  * and not used.
  *
  * Direct usage of asapi_addmsg_cli() requires some knowledge of DASH7 M2QP
  * and sessioning.  Here is a very brief explanation of the structure:
  * <LI> The initialization template creates an advertising flood.  If the init
  *         template is included, no other templates should be included.  They
  *         will not be written to the message, in any case.  </LI>
  * <LI> The non-init templates form a request-response dialog.  In DASH7, a
  *         session is often just one req-resp dialog.  A session can also span
  *         multiple req-resp dialogs.  This is the method for streaming and
  *         doing arbitrated Queries.  To enable chained dialogs, set "wait"
  *         parameter to 0. </LI>
  * <LI> If you use an advertising flood (init message), 99.9% of the time you
  *         will want to add a second message that includes a dialog.  It will
  *         be sent after the advertising is done. </LI>
  *
  * Indirect usage of asapi_addmsg_cli() is possible through H-Builder Client
  * Packs.  These include idiot-proof functions for generating ASAPI sessions
  * along a narrower scope of features.  But, there is no limit to the number
  * of Client Packs a client contains (apart from Flash to store the code).
  */
ot_int asapi_addmsg_cli(    MSG_Ctl             hold_or_exec,
                            ADDR_Code           addressing,
                            asapi_init_tmpl*    init,
                            asapi_dialog_tmpl*  dialog,
                            asapi_dlls_tmpl*    dlls,
                            asapi_routing_tmpl* routing,
                            isfcomp_tmpl*       comp,
                            isfcall_tmpl*       call,
                            query_tmpl*         g_query,
                            query_tmpl*         l_query,
                            udp_tmpl*           udp     );




/** @brief Scrub/Kill the session running on the server, and clear queue
  * @param None
  * @retval ot_int      An input error code (if negative) or the number of
  *                     bytes remaining in the output stream (if non-negative)
  * @ingroup asapi
  */
ot_int asapi_scrub_cli();


#endif









/** ASAPI Server functions  <BR>
  * ========================================================================<BR>
  * Server functions must read the incoming ASAPI messages that get sent by
  * the client, do some work, and optionally provide a response.
  *
  * The ASAPI server works together with the ALP processing layer.  Function
  * asapi_proc() must be called wherever ALP processes inputs with ID = 0x07.
  * Generally, this is in alp_ext_proc(), but it could also be integrated
  * directly into alp_proc() as is done with this ASAPI distribution.
  */
#if (OT_FEATURE(SERVER))



/** @brief ASAPI Server library initialization
  * @param None
  * @retval None
  * @ingroup asapi
  *
  * Initializes static memory elements of ASAPI with startup values.  Call this
  * only once during OpenTag startup.
  */
void asapi_init_srv();




/** @brief Opens the ASAPI server on a given ALP stream
  * @param alp      (alp_tmpl*) pointer to ALP stream to use with Server
  * @retval None
  * @ingroup asapi
  * @sa asapi_close_srv
  *
  * Call this only when starting-up the ASAPI server on the supplied ALP.
  * The server may only operate on one ALP stream at a given time.  Calling
  * asapi_open_srv() when the server is already open will have no effect.
  * To change the server to another ALP, first call asapi_close_srv().
  */
void asapi_open_srv(alp_tmpl* alp);



/** @brief Closes the ASAPI server
  * @param None
  * @retval ot_bool     True if server closed successfully
  * @ingroup asapi
  * @sa asapi_open_srv
  *
  * Call this to close an ASAPI server that is open.  Calling asapi_close_srv()
  * when the server is not open will return False, but will have no effect.
  * Calling asapi_close_srv() when the server is presently active (processing
  * sessions) will return False, and the server will not close.
  */
ot_bool asapi_close_srv();



/** @brief ASAPI ALP processing routine
  * @param alp          (alp_tmpl*) alp stream
  * @param user_id      (id_tmpl*) user_id for protocol, conveyed by comm bus
  * @retval ot_bool     True if atomic (on error), False if non atomic
  * @ingroup asapi
  * @sa alp_proc
  *
  * asapi_proc() manages the non-atomic ASAPI ALP protocol.  Non-atomic ALPs
  * are processed as the message data arrives into the ALP stream, but actual
  * execution of the ALP protocol data may occur at a later time.
  *
  * The ASAPI ALP protocol data will be executed once the "HOLDOPEN" flag in
  * the ASAPI Command byte is set to 0.  Messages with HOLDOPEN set to 1 will
  * be buffered and executed later, as soon as a message arrives with HOLDOPEN
  * set to 0.
  *
  * Immediately when ASAPI protocol data arrives, alp_proc() will call
  * asapi_proc(), and this function must check the message for framing and
  * access errors.  If the message is error free, it will be added to the ASAPI
  * execution queue.
  *
  * asapi_proc() checks the ALP input record for violations and for flags that
  * cause the ASAPI process to actually begin.  It will respond immediately,
  * even though ASAPI doesn't begin until the entire message is buffered.
  */
ot_bool asapi_proc(alp_tmpl* alp, const id_tmpl* user_id);





/** @brief Session Applet function for ASAPI
  * @param active       (m2session*) active communication session
  * @retval None
  * @ingroup asapi
  * @sa asapi_proc
  *
  * asapi_applet() is an OpenTag Session Applet, which is attached to a session
  * at the time the session is created [by alp_proc()].  When the session is
  * being invoked by the OpenTag Mode2 Data Link Layer, it calls this function.
  *
  * Therefore, delayed execution of the ASAPI protocol occurs in this function
  * at the time of session invocation.
  */
void asapi_applet(m2session* active);


#endif





#endif

