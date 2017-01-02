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
  * @file       /io/sx127x/radio_SPIRIT1.h
  * @author     JP Norair
  * @version    R100
  * @date       27 Oct 2016
  * @brief      Mode 2 interface
  * @ingroup    Radio
  *
  * Mode 2 interface for SX127x
  *
  ******************************************************************************
  */

#ifndef __RADIO_RM2_H
#define __RADIO_RM2_H

#include <otstd.h>
#include <io/sx127x/config.h>
#include <io/sx127x/interface.h>

#include <otlib/buffers.h>
#include <otlib/crc16.h>
#include <otlib/utils.h>



/** Patch functions implemented in the driver     <BR>
  * ========================================================================<BR>
  */
#if (   !defined(EXTF_em2_encode_data)       \
    ||  !defined(EXTF_em2_encode_newframe)   \
    ||  !defined(EXTF_em2_encode_newpacket)  \
    ||  !defined(EXTF_em2_decode_data)       \
    ||  !defined(EXTF_em2_decode_newframe)   \
    ||  !defined(EXTF_em2_decode_newpacket)  )
#   error "For SX127x, some encoder functions must be patched in the driver.  See include/io/sx127x/config.h."
#endif


// Custom functions not declared in m2/encode.h
void em2_encode_data(void);
void em2_decode_data(void);




#include <m2/dll.h>
#include <m2/encode.h>
#include <m2/radio.h>
#include <m2/session.h>
#include <m2/bgcrc8.h>  // Unique for LoRa



/** Special things implemented in radio module  <BR>
  * ========================================================================<BR>
  */

/** SX127x RF Module local Data
  * Data that is useful for the internal use of this module.  The list below is
  * comprehensive, and it may not be needed in entirety for all implementations.
  * For implementations that don't use the values, comment out.
  *
  * state       Radio State, partially implementation-dependent
  * flags       A local store for usage flags
  */
typedef struct {
    ot_u8   state;
    ot_u8   flags;
    ot_u16  tries;  //used mainly for BG Scan
} rfctl_struct;

extern rfctl_struct rfctl;


typedef enum {
    MODE_bg = 0,
    MODE_fg = 3
} MODE_enum;


void sx127xdrv_force_ready();
void sx127xdrv_smart_ready(void);
void sx127xdrv_smart_sleep(void);
void sx127xdrv_unsync_isr();

void sx127xdrv_buffer_config(MODE_enum mode, ot_u16 param);
void sx127xdrv_save_linkinfo();




/** Some local constants, variables, macros
  */
#define _MAXPKTLEN (M2_PARAM(MAXFRAME) * M2_PARAM(MFPP))
#if (_MAXPKTLEN > 256)
#   warning "This implementation of LoRa must use 256 bytes packet length"
#   undef _MAXPKTLEN
#   define _MAXPKTLEN   256
#elif (_MAXPKTLEN == 0)
#   warning "Max packet length could not be derived from app/config.h, using 256 bytes"
#   undef _MAXPKTLEN
#   define _MAXPKTLEN   256
#endif

#if (RF_FEATURE(AUTOCAL) != ENABLED)
#   undef RF_FEATURE_AUTOCAL
#   define RF_FEATURE_AUTOCAL   ENABLED
#endif

#if (RF_FEATURE(MFPP) == ENABLED)
#   undef RF_FEATURE_MFPP
#   define RF_FEATURE_MFPP      DISABLED
#endif



/** Internal Radio States
  *
  * b5:3        b2:0
  * TX States   RX States
  */
#define RADIO_STATE_RXSHIFT     0
#define RADIO_STATE_RXMASK      (3 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXAUTO      (0 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXINIT      (1 << RADIO_STATE_RXSHIFT)
#define RADIO_STATE_RXDONE      (2 << RADIO_STATE_RXSHIFT)


#define RADIO_STATE_TXSHIFT     3
#define RADIO_STATE_TXMASK      (7 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXDONE		(0 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXINIT      (1 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXCAD1      (2 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXCCA1      (3 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXCAD2      (4 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXCCA2      (5 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXSTART     (6 << RADIO_STATE_TXSHIFT)
#define RADIO_STATE_TXDATA      (7 << RADIO_STATE_TXSHIFT)

/** Internal Radio Flags
*/
#define RADIO_FLAG_BG           (1 << 0)
#define RADIO_FLAG_CONT         (1 << 1)
#define RADIO_FLAG_BGFLOOD      (3 << 0)

#define RADIO_FLAG_SETPWR		(1 << 5)
#define RADIO_FLAG_PWRMASK      (3 << 6)
#define RADIO_FLAG_XOOFF        (0 << 6)
#define RADIO_FLAG_XOON         (1 << 6)

//#define RADIO_FLAG_CRC5         (1 << 2)  // not used with SX127x
//#define RADIO_FLAG_RESIZE       (1 << 3)  // MFP not available with SX127x
//#define RADIO_FLAG_AUTOCAL      (1 << 4)  // Autocal is forced



/** Internal Radio Interrupt Flags
  * For performance reasons, sometimes interrupt flags will be stored locally
  * and used in later conditionals.  The usage is implementation dependent.
  */
#define RADIO_INT_CCA           (1 << 3)
#define RADIO_INT_CS            (1 << 4)




#endif


