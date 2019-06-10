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
  * @file       /otlib/m2_transport.h
  * @author     JP Norair
  * @version    R101
  * @date       20 Mar 2014
  * @brief      DASH7 Mode 2 Query Protocol (Transport Layer)
  * @ingroup    M2QP
  *
  ******************************************************************************
  */

#if !defined(__M2QP_H) /* && OT_FEATURE(M2) */
#define __M2QP_H

#include <otsys/types.h>
#include <m2/tmpl.h>
#include <m2/session.h>
#include <otsys/syskern.h>
#include <otlib/queue.h>



// Mode 2 Application Subprotocol IDs
#define M2SPID_NULL             (0x00)
#define M2SPID_FILEDATAv1       (0x01)
#define M2SPID_SENSORv1         (0x02)
#define M2SPID_SEC_INITv1       (0x11)
#define M2SPID_SEC_FINISHv1     (0x12)
#define M2SPID_SEC_BCASTKEYv1   (0x13)
#define M2SPID_SEC_PUBKEYv1     (0x14)



// M2QP Transport Types
// NA2P = "Non Arbitrated 2-Party [Transport]"
// A2P  = "Arbitrated 2-Party [Transport]"
//      -> Formal behavior is for sequential query casting (or retries).
//      -> Can also be hacked to do many very cool things, without violating spec
#define M2TT_MASK               (7<<4)      //0x70
#define M2TT_RESPONSE           (0<<4)      //0x00
#define M2TT_ERROR              (1<<4)      //0x10
#define M2TT_REQNA2P            (2<<4)      //0x20        //NA2P Request (Normal)
#define M2TT_REQA2P             (4<<4)      //0x40
#define M2TT_REQA2P_I           (4<<4)      //0x40       //Initial A2P Request
#define M2TT_REQA2P_X           (5<<4)      //0x50        //Intermediate A2P Request
#define M2TT_REQA2P_T           (7<<4)      //0x70        //Terminal (Final) A2P Request

/// Future updates: mostly nomenclature changes
#define M2TT_REQ_QUERY          (1<<4)
#define M2TT_REQ_UB             (2<<4)      // Unicast/Broadcast Request (non arbitrated, no queries)
#define M2TT_REQ_A              (3<<4)      // Anycast Request (non arbitrated, global query)
#define M2TT_REQ_M              (4<<4)      // Mask for Multicast requests
#define M2TT_REQ_M_INIT         (4<<4)      // Initial Multicast Request (global + local queries)
#define M2TT_REQ_M_MID          (5<<4)      // Intermediate Multicast Request
#define M2TT_REQ_M_FIN          (7<<4)      // Final Multicast Request




// M2QP Opcodes
#define M2OP_MASK               0x0F
#define M2OP_ANN_F              0x00        //Announcement of File
#define M2OP_ANN_S              0x01        //Announcement of File Series
#define M2OP_INV_F              0x02        //Inventory from File
#define M2OP_INV_S              0x03        //Inventory from File Series
#define M2OP_UDP_F              0x04
#define M2OP_UDP_S              0x05
#define M2OP_COL_FF             0x06        //Collection: File/File
#define M2OP_COL_SF             0x07        //Collection: Series/File
#define M2OP_COL_FS             0x08        //Collection: File/Series
#define M2OP_COL_SS             0x09        //Collection: Series/Series
// reserved: 0x0A through 0x0E
#define M2OP_RFU                0x0F        //Reserved



// M2QP Command Extension Options
#define M2CE_NORESP             (1 << 1)
#define M2CE_NOCSMA             (1 << 2)
#define M2CE_CA_MASK            (7 << 3)
#define M2CE_CA_RIGD            (0 << 3)
#define M2CE_CA_RAIND           (1 << 3)
#define M2CE_CA_AIND            (2 << 3)
#define M2CE_SCRAP              (1 << 6)
#define M2CE_NOACK              (1 << 7)

// Mode 2 Query Comparisons
#define M2QC_MASKED             (0x80)
#define M2QC_VAL_EXISTS         (0x00)
#define M2QC_ALU                (0x20)
#define M2QC_ALU_NE             (0x20)
#define M2QC_ALU_EQ             (0x21)
#define M2QC_ALU_LT             (0x22)
#define M2QC_ALU_LTE            (0x23)
#define M2QC_ALU_GT             (0x24)
#define M2QC_ALU_GTE            (0x25)
#define M2QC_COR_SEARCH         (0x40)
#define M2QC_COR_THRMASK        (0x1F)
#define M2QC_ERROR              (0xFF)





/** External Facing Datatypes   <BR>
  * ========================================================================<BR>
  * These data types are sometimes referred by external functions.
  */
typedef struct {
    ot_u8   code;
    ot_u8   ext;
} cmd_data;

typedef struct {
    ot_u16  sequence;   // datastream sequence counter
    ot_u8   fpp;        // frames per packet field
    ot_u8   srcport;    // source port field
} ds_data;



/** PM2 data types  <BR>
  * ========================================================================<BR>
  * These are not really supposed to be used externally, but leaving them
  * exposed does not hurt anything, and it makes module testing much easier.
  */

typedef union {
    ot_s32  slong[4];
    ot_u32  ulong[4];
    ot_s16  sshort[8];
    ot_u16  ushort[8];
    ot_u8   ubyte[16];
} dstr_16;

typedef union {
    ot_s32  slong[2];
    ot_u32  ulong[2];
    ot_s16  sshort[4];
    ot_u16  ushort[4];
    ot_u8   ubyte[8];
} dstr_8;

typedef struct {
    ot_u8   comp_id;
    ot_int  comp_offset;
} query_data;





/** ot_m2qpsig function pointer type
  * param1  (ot_u8) Transport data (callback-dependent)
  * param2  (ot_u8) Transport data (callback-dependent)
  * param3  (id_tmpl*)  Device ID pointer of responding device
  *
  * Do not alter the Device ID data in your callback!!!
  * It points to m2np.rt.dlog from m2_network.h
  */
typedef ot_bool (*ot_m2qpsig)(ot_u8, ot_u8, id_tmpl*);


#define M2QP_HANDLES_ERROR  (M2_FEATURE(GATEWAY) || M2_FEATURE(SUBCONTROLLER))
#define M2QP_HANDLES_A2P    (M2_FEATURE(GATEWAY) || M2_FEATURE(SUBCONTROLLER))

typedef struct {
#if !defined(EXTF_m2qp_sig_isf)
    ot_m2qpsig isf;
#endif



// Old UDP callback now goes through ALP
//#if !defined(EXTF_m2qp_sig_udp)
//    ot_m2qpsig udp;
//#endif



// Old Datastream stuff is no longer available
//#if !defined(EXTF_m2qp_sig_dspkt) && M2_FEATURE(M2DP)
//    ot_m2qpsig dspkt;
//#endif
//#if !defined(EXTF_m2qp_sig_dsack) && M2_FEATURE(M2DP)
//    ot_m2qpsig dsack;
//#endif


#if !defined(EXTF_m2qp_sig_ctl) && M2QP_HANDLES_ERROR
    ot_m2qpsig ctl;
#endif
#if !defined(EXTF_m2qp_sig_a2p) && M2QP_HANDLES_A2P
    ot_m2qpsig a2p;
#endif
}
m2qp_sigs;



// potentially used in datastream frame fragmentation, but this feature is not
// implemented yet, and this structure may change markedly

typedef struct {
    cmd_data        cmd;        // internal usage
    query_data      qdata;      // internal usage
    query_tmpl      qtmpl;


#   if (OT_FEATURE(M2QP_CALLBACKS) == ENABLED)
        m2qp_sigs   sig;
#   endif


// Datastream features now removed, getting integrated into SCTP adaptor

//#   if (M2_FEATURE(M2DP) == ENABLED)
//        alp_tmpl    ds;
//#   endif


} m2qp_struct;


extern m2qp_struct m2qp;







/** Protocol Automated Interface Functions "pm2_put"
  */

/** @brief A shortcut function to prepare beacons.
  * @param  cmd_code    (ot_u8) beacon command (must be announcement)
  * @param  cmd_ext     (ot_u8) beacon command extension (optional)
  * @param  input       (ot_queue*) queue pointer to File/File Series Call Template
  * @retval none
  * @ingroup M2QP
  *
  * The cmd_ext param should be 0 if you are not using it.
  */
ot_int m2qp_put_beacon(ot_u8 cmd_code, ot_u8 cmd_ext, ot_queue* input);



/** @brief A shortcut function to prepare beacons.
  * @param  cmd_code    (ot_u8) beacon command (must be announcement)
  * @param  cmd_ext     (ot_u8) beacon command extension (optional)
  * @param  input       (ot_queue*) queue pointer to File/File Series Call Template
  * @retval none
  * @ingroup M2QP
  *
  * The cmd_ext param should be 0 if you are not using it.
  */
void m2qp_put_na2ptmpl(ot_u16 rx_timeout, ot_u8 rx_channels, ot_u8* rx_chanlist);

void m2qp_put_a2ptmpl(ot_u16 rx_timeout, ot_u8 csma_guard, ot_u8 rx_channels, ot_u8* rx_chanlist);

void m2qp_set_suppliedid(ot_bool vid, ot_u8* supplied_addr);

///@todo I might be able to remove these
ot_int m2qp_put_iss( ot_u8* isf_template );
ot_int m2qp_put_isf( ot_u8 isf_id, ot_u8 offset, ot_u16 max_length );










/** M2QP Parsing Functions  <BR>
  * =======================================================================
  */

/** @brief A Null Callback for ot_sigresp types (response callbacks)
  * @param  responder_id    (id_tmpl*) pointer to m2np.rt.txer
  * @param  payload_length  (ot_int) length of the response payload, in bytes
  * @param  payload         (ot_u8*) pointer to payload data
  * @retval ot_bool         Returning False will abort remaining response processing
  * @ingroup M2QP
  *
  * This function does nothing.  m2qp_init() will set all callbacks to this
  * function.  It is exposed in order to illustrate the arguments for ot_sigresp
  * callbacks, so the user can write M2QP callbacks correctly.
  */
ot_bool m2qp_sigresp_null(id_tmpl* responder_id, ot_int payload_length, ot_u8* payload);


/** @brief  Initializes the M2QP Module
  * @param  none
  * @retval none
  * @ingroup M2QP
  * @sa sys_init()
  *
  * Run this function after a reboot, or any time that the protocol module
  * memory needs to be re-initialized.  sys_init() calls it, so as long as
  * you call sys_init() on startup, you're covered.
  */
void m2qp_init();


/** @brief  Parses an incoming Protocol Frame (Request, Response, Data)
  * @param  active      (m2session*) Active communication session
  * @retval ot_int      Zero on success, Non-zero on Failure.
  * @ingroup M2QP
  *
  * The incoming Protocol Frame must be present, in full, in the RX queue at the
  * time of calling.  pm2_parse_frame() does not check CRC, but it does perform
  * decryption in cases when the frame is encrypted.
  */
ot_int m2qp_parse_frame(m2session* active);


/** @brief  Parses a datastream packet, processes the data in the case where the
  *         packet is error-free, and prepare an ACK response.
  * @param  active      (m2session*) Active communication session
  * @retval none
  * @ingroup M2QP
  *
  * Run this in the system module when a datastream frame is received with errors.
  */
void m2qp_parse_dspkt(m2session* active);


/** @brief  Marks a datastream frame as a broken frame, and handles stream processing
  * @param  none
  * @retval none
  * @ingroup M2QP
  *
  * Run this in the system module when a datastream frame is received with errors.
  */
void m2qp_mark_dsframe();





/** Static Callbacks    <BR>
  * ========================================================================<BR>
  * The user can assume that the payload data is stored in the RXQ and the
  * queue pointers are set properly.
  */
ot_bool m2qp_sig_isf(   ot_u8 type,     ot_u8 opcode,   id_tmpl* user_id    );
ot_bool m2qp_sig_ctl(   ot_u8 code,     ot_u8 subcode,  id_tmpl* user_id    );
ot_bool m2qp_sig_a2p(   ot_u8 code,     ot_u8 subcode,  id_tmpl* user_id    );



// Old UDP callback is now going through ALP instead

//ot_bool m2qp_sig_udp(   ot_u8 srcport,  ot_u8 dstport,  id_tmpl* user_id    );

// Old Datastream feature no longer available
//ot_bool m2qp_sig_dspkt( ot_u8 code,     ot_u8 subcode,  id_tmpl* user_id    );
//ot_bool m2qp_sig_dsack( ot_u8 code,     ot_u8 subcode,  id_tmpl* user_id    );









/** Protocol ISF Functions  <BR>
  * =======================================================================
  */

/** @brief Breaks down ISF Comparison Template, and runs the comparison
  * @param  is_series   (ot_u8) 0 is for ISF Comp, non-zero for ISS Comp
  * @param  user_id      (id_tmpl*) user identifier of this comp
  * @retval ot_int:     Score value for the comparison (see notes below)
  * @ingroup Protocol_Special
  *
  * This is used to run the comparisions typical of anycast and multicast routed
  * requests.  It pulls the template from the RX ot_queue, and the ot_queue cursor
  * should be set up on the first byte of the template.
  *
  *
  * The score value (output) is structured as follows:
  * - negative values:  An error has occurred.
  *                     The value returned is -1 * [Mode 2 Error Code]
  *
  * - zero (0):         The comparison did not match (Score is 0)
  *
  * - positive:         The comparison did match.  The specific value indicates
  *                     the score, the higher the better.
  *
  * @note Comparison scoring
  * In some comparisons, the score is just binary (pass/fail).  In others, there
  * is a real score.  So far, the only type of scoring is correlation, where the
  * maximum score is equal to the length in bytes of the compare token.  For
  * these types of comparisons, a threshold value is specified in the comparison
  * input data.  If the score is below threshold, it will be returned as 0.  If
  * it is equal or higher, the actual score will be returned.
  */
ot_int m2qp_isf_comp(ot_u8 is_series, id_tmpl* user_id);




/** @brief Breaks down ISF Call Template, and queues the ISF Return Template
  * @param is_series    (ot_u8) 0 is for ISF Call, non-zero for ISS Call
  * @param input_q      (ot_queue*) data queue containing isf call template
  * @param user_id      (id_tmpl*) user identifier of this call
  * @retval ot_int      Number of total data bytes in the called dataset
  * @ingroup Protocol_Special
  *
  * Parses and manages a ISF File or ISF List Call Template, writing to the
  * TX queue the ISF File or ISF List Return Template.
  *
  * The parameter "input_q" is typcially the Mode2 RX queue.  The Mode2 DLL
  * also uses this function for beacon generation, though, via a temporary
  * command queue.
  *
  * The score value (output) is structured as follows:
  * - negative values:  An error has occurred.
  *                     The value returned is -1 * [Mode 2 Error Code]
  *
  * - non-negative:     The number of ISF data bytes in the called dataset.
  *                     This does not include ISF header bytes, which are
  *                     2 bytes for each ISF file in the called dataset.
  */
ot_int m2qp_isf_call(ot_u8 is_series, ot_queue* input_q, id_tmpl* user_id);




/** @brief Toolkit function for accessing ISF data, processing it, and returning a value
  * @param  is_series     (ot_u8)   0 is for ISF File, non-zero for ISF List
  * @param  isf_id        (ot_int)  ID for the ISF File or List
  * @param  offset        (ot_int)  Byte offset into the ISF dataset
  * @param  window_bytes  (ot_int)  Number of bytes, following offset, to process
  * @param  load_function (ot_int (*)(ot_int*, ot_u8) ) Processing function
  * @retval ot_int        A running sum of returns from the processing function
  * @ingroup Protocol_Special
  * @sa pm2_isf_comp()
  * @sa pm2_isf_call()
  * @sa sub_load_charcorrelation()
  * @sa sub_load_comparison()
  * @sa sub_load_return()
  *
  * In a nutshell, this function can do basically anything to any kind of ISF
  * dataset.  It is typically only used by pm2_isf_comp() or pm2_isf_call().
  * It is one of the cooler and more useful functions in OpenTag.
  *
  * The processing function is a subroutine that takes in an index pointer (this
  * is issued by pm2_load_isf() ) and a byte of data.  It can do whatever it
  * wants to the data, and return whatever it wants.  There are currently three
  * subroutines used as processing functions for different tasks:
  * sub_load_charcorrelation(), sub_load_comparison(), sub_load_return()
  */
ot_int m2qp_load_isf(   ot_u8       is_series,
                        ot_u8       isf_id,
                        ot_int      offset,
                        ot_int      window_bytes,
                        ot_int      (*load_function)(ot_int*, ot_u8),
                        id_tmpl*    user_id );


#endif

