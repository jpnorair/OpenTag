/* Copyright 2010-2014 JP Norair
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
  * @file       /io/spirit1/radio_SPIRIT1.h
  * @author     JP Norair
  * @version    R100
  * @date       27 Aug 2014
  * @brief      Declarations of some stuff used by radio_SPIRIT1.c
  * @ingroup    Radio
  *
  * By putting these declarations here instead of in radio_SPIRIT1.c, it allows
  * us to patch radio_SPIRIT1.c in special ways.
  * 
  ******************************************************************************
  */

#ifndef __RADIO_RM2_H
#define __RADIO_RM2_H

#include <otstd.h>
#include <io/spirit1/config.h>
#include <io/spirit1/interface.h>

#include <otlib/buffers.h>
#include <otlib/crc16.h>
#include <otlib/utils.h>



/** Patch functions implemented in the driver     <BR>
  * ========================================================================<BR>
  */
#if (   defined(EXTF_em2_encode_data)       \
    ||  defined(EXTF_em2_encode_newframe)   \
    ||  defined(EXTF_em2_encode_newpacket)  \
    ||  defined(EXTF_em2_decode_data)       \
    ||  defined(EXTF_em2_decode_newframe)   \
    ||  defined(EXTF_em2_decode_newpacket)  )
#   error "For SPIRIT1, some encoder functions should be patched in the driver."
#endif

#define EXTF_em2_encode_data
#define EXTF_em2_encode_newframe
#define EXTF_em2_encode_newpacket
#define EXTF_em2_decode_data
#define EXTF_em2_decode_newframe
#define EXTF_em2_decode_newpacket


#include <m2/dll.h>
#include <m2/encode.h>
#include <m2/radio.h>
#include <m2/session.h>




/** Special things implemented in radio module (radio_SPIRIT1.c)  <BR>
  * ========================================================================<BR>
  * MODE_fg = 2 or 4, depending on length of syncword.  
  * As you might guess, this is an alignment hack.
  */

/** SPIRIT1 RF Module local Data
  * Data that is useful for the internal use of this module.  The list below is
  * comprehensive, and it may not be needed in entirety for all implementations.
  * For implementations that don't use the values, comment out.
  *
  * state       Radio State, partially implementation-dependent
  * flags       A local store for usage flags
  * txlimit     An interrupt/event comes when tx buffer gets below this number of bytes
  * rxlimit     An interrupt/event comes when rx buffer gets above this number of bytes
  */
typedef struct {
    ot_u8   state;
    ot_u8   flags;
    ot_int  nextcal;
    ot_int  txlimit;
    ot_int  rxlimit;
} rfctl_struct;

extern rfctl_struct rfctl;


typedef enum {
    MODE_bg = 0,
    MODE_fg = (2 << (DRF_SYNC_BYTES > 2))
} MODE_enum;


void spirit1drv_force_ready();
void spirit1drv_smart_ready(void);
void spirit1drv_smart_standby(void);
void spirit1drv_unsync_isr();
void spirit1drv_ccafail_isr();
void spirit1drv_ccapass_isr();
void spirit1drv_txend_isr();
void spirit1drv_buffer_config(MODE_enum mode, ot_u16 param);
void spirit1drv_save_linkinfo();




/** Some local constants, variables, macros
  */
#define _MAXPKTLEN (M2_PARAM(MAXFRAME) * M2_PARAM(MFPP))
#if (_MAXPKTLEN == 0)
#   warning "Max packet length could not be derived from app/config.h, using 256 bytes"
#   undef _MAXPKTLEN
#   define _MAXPKTLEN   256
#endif

#if (0)
#   define _DSSS
#   define _SPREAD      5
#   define _RXMAXTHR    80
#   define _RXMINTHR    (_SPREAD * 2)
#else
#   define _SPREAD      1
#   define _RXMAXTHR    64
#   define _RXMINTHR    8
#endif


#if (RF_FEATURE(AUTOCAL) != ENABLED)
#   define __CALIBRATE()    spirit1drv_offline_calibration()
#else
#   define __CALIBRATE();
#endif




/** Internal Radio States
  *
  * b5:3        b2:0
  * TX States   RX States
  */
#define RADIO_STATE_RXSHIFT     0
#define RADIO_STATE_RXMASK      (3 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXINIT      (4 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXDONE      (5 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXAUTO      (0 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXPAGE      (1 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXSLOT      (2 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXMFP       (3 << RADIO_STATE_RXSHIFT)

#define RADIO_STATE_TXSHIFT     3
#define RADIO_STATE_TXMASK      (7 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXINIT      (1 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXCCA1      (2 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXCCA2      (3 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXSTART     (4 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXDATA      (5 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXDONE      (6 << RADIO_STATE_TXSHIFT)

/** Internal Radio Flags
*/
#define RADIO_FLAG_BG           (1 << 0)
#define RADIO_FLAG_CONT         (1 << 1)
#define RADIO_FLAG_BGFLOOD      (3 << 0)
#define RADIO_FLAG_CRC5         (1 << 2)
#define RADIO_FLAG_RESIZE       (1 << 3)
#define RADIO_FLAG_AUTOCAL		(1 << 4)
#define RADIO_FLAG_SETPWR		(1 << 5)
#define RADIO_FLAG_PWRMASK      (3 << 6)
#define RADIO_FLAG_XOOFF        (0 << 6)
#define RADIO_FLAG_XOON         (1 << 6)



/** Internal Radio Interrupt Flags
  * For performance reasons, sometimes interrupt flags will be stored locally
  * and used in later conditionals.  The usage is implementation dependent.
  */
#define RADIO_INT_CCA           (1 << 3)
#define RADIO_INT_CS            (1 << 4)




#endif


