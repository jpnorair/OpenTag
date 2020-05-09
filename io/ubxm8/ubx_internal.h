/* Copyright (c) 2017 JP Norair
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
 * @file ubx_internal.h
 *
 * U-Blox Task, Driver, and external data management.
 *
 * @author JP Norair
 */

#ifndef UBX_INTERNAL_H_
#define UBX_INTERNAL_H_

#include "ubx_types.h"
#include <otsys.h>
#include <otlib/queue.h>


#define UBX_RETRY_MAX 4

// You can select these manually here, to do different driver builds.
//#define UBX_CONFIGURE_SBAS
//#define UBX_CONFIGURE_NAV_DOP
//#define UBX_CONFIGURE_NAV_SVINFO
#define UBX_CONFIGURE_NAVX5
//#define UBX_CONFIGURE_MON



typedef enum {
    UBX_cold_start      = 0,
    UBX_warm_start,
    UBX_hot_start
} ubx_startmode_t;


typedef enum {
    UBX_9600bps   = 0,
    //UBX_28800bps,
    //UBX_57600bps,
    UBX_115200bps,
    UBX_250000bps,
    UBX_500000bps,
    UBX_BAUDRATE_MAX
} ubx_speed_t;


/* Rx Task states */
typedef enum {
        UBX_TASK_INIT = 0,
        UBX_TURNON,
        UBX_WAKEUP,
        UBX_CFG_UART,
        UBX_CFG_UART_ACK,
        UBX_CFG_UART2,
        UBX_CFG_UART_ACK2,
        UBX_CFG_RATE,
        UBX_CFG_RATE_ACK,
#       ifdef UBX_CONFIGURE_SBAS
        UBX_CFG_SBAS,
        UBX_CFG_SBAS_ACK,
#       endif
#		ifdef UBX_CONFIGURE_NAVX5
		UBX_CFG_NAVX5,
		UBX_CFG_NAVX5_ACK,
#		endif
        UBX_CFG_NAV_PVT,
        UBX_CFG_NAV_PVT_ACK,
#       ifdef UBX_CONFIGURE_NAV_DOP
        UBX_CFG_NAV_DOP,
        UBX_CFG_NAV_DOP_ACK,
#		endif
#		ifdef UBX_CONFIGURE_NAV_SVINFO
        UBX_CFG_NAV_SVINFO,
        UBX_CFG_NAV_SVINFO_ACK,
#		endif
#		ifdef UBX_CONFIGURE_MON
        UBX_CFG_MON_HW,
        UBX_CFG_MON_HW_ACK,
        UBX_CFG_MON_VER,      // We don't care about version info
        UBX_CFG_MON_VER_ACK,
#       endif
        //UBX_CFG_CFG,          // Presently, device isn't known to have flashing capability
        //UBX_CFG_CFG_ACK,
        UBX_MAIN_PROC,
        UBX_MGA_ANO,
        UBX_MGA_ANO_ACK,
        UBX_MODE_ENTRY,
        UBX_MODE_ENTRY_ACK,
        UBX_RXDATA,
        UBX_TIMEOUT,
        UBX_SHUTDOWN,
		UBX_SHUTDOWN2,
        UBX_STATE_TERMINUS
} ubx_state_t;



typedef enum {
    UBX_Null        = -1,
    UBX_Idle        = 0,
    UBX_RxHeader    = 1,
    UBX_RxPayload   = 2,
    UBX_Tx_Wait     = 3,
    UBX_Tx_Done     = 4,
    UBX_Tx_Sync     = 5
} ubx_drvstate_t;


typedef struct __attribute__((packed)) {
    ubx_sync_t      sync;
    ubx_header_t    header;
    ubx_buf_t       payload;
    ot_u8           footer[2];
} ubx_buffer_t;


typedef struct {
    // Buffer
    ubx_buffer_t    buf;
    
    // Driver stuff
    ubx_drvstate_t  state;
    ot_u16          frame_len;
    ot_u8*          cursor;
    
    // Shared memory
    ot_u32          baudrate;
    
    // Task stuff
    ot_task         task;
    ot_sig2         dataready;
    ot_int          onoff_interval;
    ot_u16          msg_stamp;
    ot_u32          ephemeris_stamp; 
    ubx_state_t     start_state;
    ubx_startmode_t start_mode;
    
#if (MCU_CONFIG(MULTISPEED))
    ot_int          clkhandle;
#endif   
} ubx_module_t;





/** Callbacks from driver
  * ========================================================================
  */
void ubxevt_rxdone(ot_int code);




/** Driver functions from Task
  * ========================================================================
  */


void ubxdrv_kill(void);
ot_int ubxdrv_init(void* port_id);
void ubxdrv_reset(void);
ot_u16 ubxdrv_txsync(void);
ot_int ubxdrv_tx(void);
void ubxdrv_setbaudrate(ot_u32 baudrate);
void ubxdrv_rx(void);
ot_long ubxdrv_turnon(void);
void ubxdrv_turnoff(void);
ot_u8 ubxdrv_getpwrcode(void);

ot_long ubxdrv_extwakeup(void);
void ubxdrv_extsleep(void);









#endif
