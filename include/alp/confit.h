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
  * @file       /confit/confit.h
  * @author     JP Norair
  * @version    R100
  * @date       30 Mar 2014
  * @brief      OpenTag Configure-It app (ConfIt)
  * @defgroup   hb_confit
  * @ingroup    hb_confit
  *
  * "ConfIt" is short for "Configure It."  It is a small application providing
  * some features for configuring lower layer attributes of the DASH7/OpenTag
  * stack, during runtime.  Everything ConfIt does could be accomplished
  * alternatively by updating the DASH7 special files through DFTP or the
  * DASH7 File ALP, but ConfIt is sometimes preferable because it is very
  * efficient at performing common configuration tasks.  ConfIt abides by the
  * filesystem privileges set in the device.
  *
  * ConfIt uses M2DEF (ALP framework), and thus it can be used with MPipe,
  * DASH7-UDP, or any other lower layers that can support M2DEF.  The ConfIT
  * protocol is described below.
  *
  * ConfIt M2DEF record/message (standard)
  * The ID for ConfIt application messages must be 0xXX.  The length value
  * depends on the amount of payload data.  <PRE>
  * +-------------------------------------------+-----------+
  * |              M2DEF/ALP Header             |  Payload  |
  * +----------+----------+----------+----------+-----------+
  * |   Flags  |  Length  |    ID    |   CMD    |  Payload  |
  * |  1 byte  |  1 byte  |  1 byte  |  1 byte  |  N bytes  |
  * | xxxxxxxx |    N     |   0x88   |  0-255   |           |
  * +----------+----------+----------+----------+-----------+ </PRE>
  *
  * Response:
  * ConfIt provides no response.  However, lower layers may still send a low
  * level response, but it will contain no application data from ConfIt, just
  * an acknowledgement.  Responses are sent back to the source port.
  *
  * Command & Payload:
  * At present, the Command is ignored, thereby making usage with UDP basically
  * transparent.
  * ----------------+-------------------------------+
  *  ...M2DEF hdr   |     M2DEF/ConfIt Payload      |
  * ---+------------+---------------+---------------+
  *    |    CMD     |  Custom Type  |  Custom Data  |
  *    |   1 byte   |    1 byte     |   N-1 bytes   |
  *    |  0 or 128  |    0-255      |               |
  * ---+------------+---------------+---------------+ </PRE>
  *
  * Custom Types & Data:
  * <PRE>
  * +------+------------+-----------------------------------------------------+
  * | Type | Data Bytes | Description                                         |
  * +------+------------+-----------------------------------------------------+
  * |   0  |      0     | Flush/refresh DASH7 lower layers
  * |   1  |      5     | Discovery Beacon Setup
  * |   2  |     10     | Join Network (VID, Subnet, Hold Time)
  * |   3  |     15     | Join Network & Alert
  * | 200  |      9     | HayTag Join Network & Alert
  * +------+------------+-----------------------------------------------------+
  *
  * +-------------------------------------------------------------------+
  * |                          Join Network (2)                         |
  * +----------+-----------+-----------+----------+----------+----------+
  * |   Type   |  Expiry   | Hold Time |    VID   |  Subnet  | B.Subnet |
  * |  1 byte  |  4 bytes  |  2 bytes  |  2 bytes |  1 byte  |  1 byte  |
  * |    1     |  (ticks)  |   XXXX    |   XXXX   |    XX    |    XX    |
  * +----------+-----------+-----------+----------+----------+----------+
  *
  * +--------------------------------------------------------+
  * |               Discovery Beacon Setup (1)               |
  * +----------+----------+----------+----------+------------+
  * |   Type   |  B.Tries |  B.Chan  |  B.Flags | B.Interval |
  * |  1 byte  |  1 byte  |  1 byte  |  1 byte  |  2 bytes   |
  * |    2     |    XX    |    XX    |    XX    |  (ticks)   |
  * +----------+----------+----------+----------+------------+
  *
  * +--------------------------------------------------------------------...
  * |                      Join Network & Alert (3)
  * +----------+-----------+-----------+----------+----------+----------+...
  * |   Type   |  Expiry   | Hold Time |    VID   |  Subnet  | B.Subnet |
  * |  1 byte  |  4 bytes  |  2 bytes  |  2 bytes |  1 byte  |  1 byte  |
  * |    3     |  (ticks)  |   XXXX    |   XXXX   |    XX    |    XX    |
  * +----------+-----------+-----------+----------+----------+----------+...
  *     ...---------------------------------------------+
  *     ...continued from above                         |
  *     ...----------+----------+----------+------------+
  *          B.Tries |  B.Chan  |  B.Flags | B.Interval |
  *          1 byte  |  1 byte  |  1 byte  |  2 bytes   |
  *            XX    |    XX    |    XX    |  (ticks)   |
  *     ...----------+----------+----------+------------+
  *
  * +-------------------------------------------------------------------...
  * |                  HayTag Join Network & Alert (200)
  * +----------+-----------+----------+----------+----------+----------+...
  * |   Type   |  Expiry   |    VID   |  Subnet  | B.Subnet |  B.Chan  |
  * |  1 byte  |  1 byte   |  2 bytes |  1 byte  |  1 byte  |  1 byte  |
  * |   200    | (1 sec)   |   XXXX   |    XX    |    XX    |    XX    |
  * +----------+-----------+----------+----------+----------+----------+...
  *     ...-----------------------+
  *     ...continued              |
  *     ...----------+------------+
  *          B.Flags | B.Interval |
  *          1 byte  |  2 bytes   |
  *            XX    |  (ticks)   |
  *     ...----------+------------+
  *
  * </PRE>
  *
  * Usage Note:
  * The byte-fields in the message formats above are defined in section 10.4 of
  * the DASH7 Mode 2 spec.
  *
  ******************************************************************************
  */

#ifndef _CONFIT_H_
#define _CONFIT_H_

#include <otstd.h>
#if (OT_FEATURE(SERVER) != ENABLED)
#   error "At this time, ConfIt requires an OpenTag Server"
#endif


#include <otlib/alp.h>    //this should do the job
//#include <m2api.h>
//#include <otlib.h>


ot_bool confit_proc(alp_tmpl* alp, id_tmpl* user_id);






#endif

