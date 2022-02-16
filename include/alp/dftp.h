/*  Copyright 2008-2022, JP Norair
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted provided that the following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright 
  *    notice, this list of conditions and the following disclaimer in the 
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
  * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
  * POSSIBILITY OF SUCH DAMAGE.
  */
/**
  * @file       /hb_dftp/hb_dftp.h
  * @author     JP Norair
  * @version    R102
  * @date       16 Jan 2014
  * @brief      DASH7 FTP (DFTP) Applet
  * @defgroup   hb_dftp
  * @ingroup    hb_dftp
  *
  * DFTP is for "DASH7 File Transfer Protocol."  It is actually a revised
  * version of TFTP (Trivial File Transfer Protocol) designed for the DASH7
  * filesystem, and also with some additional low-power features.  DFTP uses
  * UDP on port 69, just like TFTP.
  *
  * DFTP was designed originally to do over-the-air firmware updates, but it is
  * useful for any sort of long file transfer that requires multiple packets.
  * It does not use its own ALP ID, it uses the normal File-Transfer ALP (1)
  * encapsulated inside UDP port 69.  A subset of File-Transfer ALP commands
  * are available for DFTP.
  *
  * <PRE>
  * Request Datagram Structure
  * +-----------+---------+------------+
  * |  CONTROL  |   TID   |  FILE ALP  |
  * |  1 byte   |  1 byte |  6 bytes   |
  * | 0lbrxxxx  |  0-255  | (see note) |
  * +-----------+---------+------------+
  *
  * Error Datagram Structure
  * +-----------+---------+------------+
  * |  CONTROL  |   TID   |  Err Code  |
  * |  1 byte   |  1 byte |  2 bytes   |
  * | 000r0101  |  0-255  |            |
  * +-----------+---------+------------+
  *
  * Data Datagram Structure
  * +-----------+---------+------------+--------------+---------+
  * |  CONTROL  |   TID   |  Block #   |  Data Block  |  CRC32  |
  * |  1 byte   |  1 byte |  2 bytes   |  128 bytes   | 4 bytes |
  * | clbr0011  |  0-255  |            |              | (opt.)  |
  * +-----------+---------+------------+--------------+---------+
  *
  * Ack Datagram Structure
  * +-----------+---------+------------+
  * |  CONTROL  |   TID   |  Block #   |
  * |  1 byte   |  1 byte |  2 bytes   |
  * | 000r0100  |  0-255  |            |
  * +-----------+---------+------------+
  *
  * GPAck Datagram Structure
  * +-----------+---------+------------+
  * |  CONTROL  |   TID   |  Timeout   |
  * |  1 byte   |  1 byte |  2 bytes   |
  * | 000r0100  |  0-255  |   (ti)     |
  * +-----------+---------+------------+
  *
  * Control Field
  * +-------+-------+------+-------+--------+
  * | CRC32 |  LZO  | B128 |  RESP | Opcode |
  * |  b7   |  b6   |  b5  |  b4   |  b3:0  |
  * +-------+-------+------+-------+--------+
  * RFU:    set to 0
  * Opcode:  1  Read Request (RRQ)      [same as TFTP]
  *          2  Write Request (WRQ)     [same as TFTP]
  *          3  Data (DATA)             [same as TFTP]
  *          4  Acknowledgement (ACK)   [same as TFTP]
  *          5  Error (ERROR)           [same as TFTP]
  *          9  Get Request (GRQ)       [new in DFTP]
  *         10  Put Request (PRQ)       [new in DFTP]
  *</PRE>
  *
  * Usage of CRC32:
  * The CRC32 field covers the data block in DATA frames, if bit 7 of the
  * control byte is set.  It is intended to provide guaranteed error protection
  * if DFTP might be used across MACs that do not have sufficient error
  * protection (for example, 802.15.4 or Bluetooth-LE).  CRC32 is typcially
  * superfluous when using DFTP on DASH7.
  *
  * Usage of LZO:
  * The LZO bit may be set as bit 6 of the control byte of a Request frame, and
  * it should also be set in each control byte of DATA frame, until a new
  * request is issued (on a new TID).  When set, it indicates that the file
  * being sent is compressed with LZO compression, CBLOCK = 512 bytes.  DFTP
  * should decompress the data before writing it to file.
  *
  * Usage of B128:
  * The B128 bit may be set as bit 5 of the control byte of a Request Frame,
  * and it should also be set in each control byte of DATA frames, until a new
  * request is issued (on a new TID).  When set, it indicates that the block
  * size of the DATA frame is 128.  When clear, block size is normal 512 bytes.
  *
  * Usage of Response:
  * The RESP (Response) bit may be set when request-response oriented lower
  * layer protocols are used with DFTP.  Note that Request-response protocols
  * are required for GET/PUT.
  *
  * Usage of Opcodes:
  * All opcodes used with TFTP can be read-about in RFC1350 documentation. New
  * opcode "Get Request" is a handshaking stage that informs the server to
  * begin a "Write Request" of the specified file.  That secondary WRQ is
  * handled just like in TFTP.  New opcode "Put request" is the same thing,
  * just using "Read Request" as the secondary operation.  The dataflows of RRQ
  * and GRQ are identical, just as are dataflows of WRQ and PRQ.
  *
  * Usage of TID:
  * In normal TFTP, the 16 bit source port and 16 bit target port are selected
  * randomly, and together they form the TFTP-TID (transport ID) of the session.
  * In DASH7-UDP, the source and destination ports are only 8 bits each.  In
  * order to reduce the likelihood of two subsequent DFTP sessions having the
  * same DTFP-TID, there is an additional byte in the DFTP application header.
  * It is chosen by the initiator of the request, and thus the 4 byte TFTP-TID
  * can be synthesized as follows: [Src Port] [DFTP-TID] [Dst Port] [DFTP-TID]
  *
  * Usage of File ALP:
  * The Read File Data Command (rxxx0100) must be used with Read/Get, and the
  * Write File Data Command (rxxx0111) must be used with Write/Put.  Only a
  * single File ID is allowed per DFTP session, unlike the formal File-Transfer
  * ALP which can batch reads and writes to multiple files.  Check the OpenTag
  * wiki for more information on the File-Transfer ALP.
  * http://wiki.indigresso.com.
  *
  * Translation with TFTP:
  * Stateless translation between DFTP and TFTP is possible, simply by
  * exchanging the binary ALP template with a text-based representation of it,
  * as TFTP identifies files by a text-string path.  Additionally, TFTP does
  * not offer the CRC32 or LZO features of DFTP, nor does offer Get and Put, so
  * these unique DFTP features must not be used with TFTP.
  *
  *
  * Read Session Transfer Pattern:
  * The "Read Session" is driven by Client Request and Server Response.  The
  * first Client request is RRQ, and subsequent requests are ACKs.  This model
  * is good for low-power clients, because the client doesn't need to wait for
  * request (the server does).
  * <PRE>
  * Client                 Server
  *    |------->[RRQ]------->|
  *    |<-------[DAT]<-------|
  *    |------->[ACK]------->|
  *  ...<-------[DAT]<-------|
  * </PRE>
  *
  * Write Session Transfer Pattern:
  * The "Write Session" is driven by Client Request and Server Response.  The
  * first Client request is WRQ, and subsequent requests are DATA packets.
  * Like Read Session, this model is best suited for low-power clients.
  * <PRE>
  * Client                 Server
  *    |------->[WRQ]------->|
  *    |<-------[ACK]<-------|
  *    |------->[DAT]------->|
  *  ...<-------[ACK]<-------|
  * </PRE>
  *
  * Get Session Transfer Pattern:
  * The "Get Session" is initiated by the Client, and then it becomes driven by
  * the Server.  The Get Session is similar to the Read Session, but it is
  * intended for low power Servers.
  * <PRE>
  * Client                 Server
  *    |------->[GRQ]------->|
  *    |<------[GPACK]<------|
  *     .... Max X ticks ....
  *    |<-------[DAT]<-------|
  *    |------->[ACK]------->...
  * </PRE>
  *
  * Put Session Transfer Pattern:
  * The "Put Session" is initiated by the Client, and then it becomes driven by
  * the Server.  The Put Session is similar to the Wrie Session, but it is
  * intended for low power Servers.
  * <PRE>
  * Client                 Server
  *    |------->[PRQ]------->|
  *    |<------[GPACK]<------|
  *     .... Max X ticks ....
  *    |<-------[ACK]<-------|
  *    |------->[DAT]------->...
  * </PRE>
  ******************************************************************************
  */




//  static const ot_u8 dftp_nominal_length[] = { 255,
//                                                __, __, 130, 2, 2,
//                                                255, 255, 255,



#ifndef _DFTP_H_
#define _DFTP_H_

#include <otstd.h>
#if (OT_FEATURE(SERVER) != ENABLED)
#   error "At this time, DFTP requires an OpenTag Server"
#endif

#include <m2api.h>
#include <otlib.h>

#define DFTP_CTL_CRC32      (1<<7)
#define DFTP_CTL_LZO        (1<<6)
#define DFTP_CTL_GETPUT     (1<<3)
#define DFTP_CTL_CMDMASK    (15<<0)
#define DFTP_CTL_TFTPMASK   (7<<0)

#define DFTP_ALP_OPMASK     (15<<0)
#define DFTP_ALP_READ       (b0100)
#define DFTP_ALP_WRITE      (b0111)

#define DFTP_OPC_RRQ        1
#define DFTP_OPC_WRQ        2
#define DFTP_OPC_DAT        3
#define DFTP_OPC_ACK        4
#define DFTP_OPC_ERR        5
#define DFTP_OPC_GRQ        9
#define DFTP_OPC_PRQ        10
#define DFTP_OPC_GPACK      12








// client functions

// start get/put, start read/write.  read/write must include file information
// in the arguments.


void dftp_read(ot_sig sigdone, ot_u8 block, ot_u8 file_id, id_tmpl* target);
void dftp_write(ot_sig sigdone, ot_u8 block, ot_u8 file_id, id_tmpl* target);
void dftp_get(ot_sig sigdone, ot_u8 block, ot_u8 file_id, id_tmpl* target);
void dftp_put(ot_sig sigdone, ot_u8 block, ot_u8 file_id, id_tmpl* target);

void dftp_applet(m2session* active);



/** DFTP Agent  <BR>
  * ========================================================================<BR>
  * This is the function that gets called by the transport manager whenever a
  * packet comes in on UDP port 69.
  */

/** @brief Runs the DFTP Processor/Agent, which manages DFTP messages and session
  * @param alp      (alp_tmpl*) ALP stream for data I/O
  * @param user_id  (id_tmpl*) User ID for Authentication
  * @retval ot_bool True if a response is loaded
  * @ingroup hb_dftp
  *
  * The input "alp" can be set to NULL, in which case dftp_proc() will directly
  * use txq and rxq.
  */
ot_bool dftp_proc(alp_tmpl* alp, id_tmpl* user_id);



typedef struct {
    ot_u8   block;
    ot_u8   id;
    ot_u16  offset;
    ot_u16  xsize;
} file_info;


typedef struct {
    ot_u8       cmd;
    ot_u8       tid;
    ot_u8       state;
    ot_u8       flags;
    file_info   file;
} dftp_struct;




#endif

