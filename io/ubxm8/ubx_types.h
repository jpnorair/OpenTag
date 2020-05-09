/****************************************************************************
 *
 *   Copyright (c) 2012-2016 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file ubx_types.h
 *
 * U-Blox protocol types definition. Following u-blox 6/7/8 Receiver 
 * Description including Prototol Specification.
 *
 * @author Thomas Gubler <thomasgubler@student.ethz.ch>
 * @author Julian Oes <julian@oes.ch>
 * @author Anton Babushkin <anton.babushkin@me.com>
 *
 * @author Hannes Delago
 *   (rework, add ubx7+ compatibility)
 *
 */

#ifndef UBX_TYPES_H_
#define UBX_TYPES_H_

#include <stdint.h>

// Put this stuff into protocol header

/* Message Classes */
#define UBX_CLASS_NAV       0x01
#define UBX_CLASS_INF       0x04
#define UBX_CLASS_ACK       0x05
#define UBX_CLASS_CFG       0x06
#define UBX_CLASS_MGA       0x13
#define UBX_CLASS_MON       0x0A
#define UBX_CLASS_RTCM3     0xF5 /**< This is undocumented (?) */
#define UBX_CLASS_RXM       0x02

/* Message IDs */
#define UBX_ID_NAV_POSLLH   0x02
#define UBX_ID_NAV_DOP      0x04
#define UBX_ID_NAV_SOL      0x06
#define UBX_ID_NAV_PVT      0x07
#define UBX_ID_NAV_VELNED   0x12
#define UBX_ID_NAV_TIMEUTC  0x21
#define UBX_ID_NAV_SVINFO   0x30
#define UBX_ID_NAV_SAT      0x35
#define UBX_ID_NAV_SVIN     0x3B
#define UBX_ID_NAV_RELPOSNED 0x3C
#define UBX_ID_NAV_AOPSTATUS 0x60
#define UBX_ID_INF_DEBUG    0x04
#define UBX_ID_INF_ERROR    0x00
#define UBX_ID_INF_NOTICE   0x02
#define UBX_ID_INF_WARNING  0x01
#define UBX_ID_ACK_NAK      0x00
#define UBX_ID_ACK_ACK      0x01
#define UBX_ID_CFG_CFG      0x09
#define UBX_ID_CFG_MSG      0x01
#define UBX_ID_CFG_NAV5     0x24
#define UBX_ID_CFG_NAVX5	0x23
#define UBX_ID_CFG_PM2		0x3B
#define UBX_ID_CFG_PMS      0x86
#define UBX_ID_CFG_PRT      0x00
#define UBX_ID_CFG_RATE     0x08
#define UBX_ID_CFG_RXM      0x11
#define UBX_ID_CFG_SBAS     0x16
#define UBX_ID_CFG_TMODE3   0x71
#define UBX_ID_MGA_ACK      0x60
#define UBX_ID_MGA_ANO      0x20
#define UBX_ID_MON_VER      0x04
#define UBX_ID_MON_HW       0x09
#define UBX_ID_CFG_RST      0x04
#define UBX_ID_RTCM3_1005   0x05
#define UBX_ID_RTCM3_1077   0x4D
#define UBX_ID_RTCM3_1087   0x57
#define UBX_ID_RXM_PMREQ    0x41

/* Message Classes & IDs */
#define UBX_MSG_NAV_POSLLH  ((UBX_CLASS_NAV) | UBX_ID_NAV_POSLLH << 8)
#define UBX_MSG_NAV_SOL     ((UBX_CLASS_NAV) | UBX_ID_NAV_SOL << 8)
#define UBX_MSG_NAV_DOP     ((UBX_CLASS_NAV) | UBX_ID_NAV_DOP << 8)
#define UBX_MSG_NAV_PVT     ((UBX_CLASS_NAV) | UBX_ID_NAV_PVT << 8)
#define UBX_MSG_NAV_VELNED  ((UBX_CLASS_NAV) | UBX_ID_NAV_VELNED << 8)
#define UBX_MSG_NAV_TIMEUTC ((UBX_CLASS_NAV) | UBX_ID_NAV_TIMEUTC << 8)
#define UBX_MSG_NAV_SVINFO  ((UBX_CLASS_NAV) | UBX_ID_NAV_SVINFO << 8)
#define UBX_MSG_NAV_SAT     ((UBX_CLASS_NAV) | UBX_ID_NAV_SAT << 8)
#define UBX_MSG_NAV_SVIN    ((UBX_CLASS_NAV) | UBX_ID_NAV_SVIN << 8)
#define UBX_MSG_NAV_RELPOSNED ((UBX_CLASS_NAV) | UBX_ID_NAV_RELPOSNED << 8)
#define UBX_MSG_NAV_AOPSTATUS ((UBX_CLASS_NAV) | UBX_ID_NAV_AOPSTATUS << 8)
#define UBX_MSG_INF_DEBUG   ((UBX_CLASS_INF) | UBX_ID_INF_DEBUG << 8)
#define UBX_MSG_INF_ERROR   ((UBX_CLASS_INF) | UBX_ID_INF_ERROR << 8)
#define UBX_MSG_INF_NOTICE  ((UBX_CLASS_INF) | UBX_ID_INF_NOTICE << 8)
#define UBX_MSG_INF_WARNING ((UBX_CLASS_INF) | UBX_ID_INF_WARNING << 8)
#define UBX_MSG_ACK_NAK     ((UBX_CLASS_ACK) | UBX_ID_ACK_NAK << 8)
#define UBX_MSG_ACK_ACK     ((UBX_CLASS_ACK) | UBX_ID_ACK_ACK << 8)
#define UBX_MSG_CFG_CFG     ((UBX_CLASS_CFG) | UBX_ID_CFG_CFG << 8)
#define UBX_MSG_CFG_PM2     ((UBX_CLASS_CFG) | UBX_ID_CFG_PM2 << 8)
#define UBX_MSG_CFG_PMS     ((UBX_CLASS_CFG) | UBX_ID_CFG_PMS << 8)
#define UBX_MSG_CFG_PRT     ((UBX_CLASS_CFG) | UBX_ID_CFG_PRT << 8)
#define UBX_MSG_CFG_MSG     ((UBX_CLASS_CFG) | UBX_ID_CFG_MSG << 8)
#define UBX_MSG_CFG_RATE    ((UBX_CLASS_CFG) | UBX_ID_CFG_RATE << 8)
#define UBX_MSG_CFG_RXM     ((UBX_CLASS_CFG) | UBX_ID_CFG_RXM << 8)
#define UBX_MSG_CFG_NAV5    ((UBX_CLASS_CFG) | UBX_ID_CFG_NAV5 << 8)
#define UBX_MSG_CFG_NAVX5   ((UBX_CLASS_CFG) | UBX_ID_CFG_NAVX5 << 8)
#define UBX_MSG_CFG_RST     ((UBX_CLASS_CFG) | UBX_ID_CFG_RST << 8)
#define UBX_MSG_CFG_SBAS    ((UBX_CLASS_CFG) | UBX_ID_CFG_SBAS << 8)
#define UBX_MSG_CFG_TMODE3  ((UBX_CLASS_CFG) | UBX_ID_CFG_TMODE3 << 8)
#define UBX_MSG_MGA_ACK     ((UBX_CLASS_MGA) | UBX_ID_MGA_ACK << 8)
#define UBX_MSG_MGA_ANO     ((UBX_CLASS_MGA) | UBX_ID_MGA_ANO << 8)
#define UBX_MSG_MON_HW      ((UBX_CLASS_MON) | UBX_ID_MON_HW << 8)
#define UBX_MSG_MON_VER     ((UBX_CLASS_MON) | UBX_ID_MON_VER << 8)
#define UBX_MSG_RTCM3_1005  ((UBX_CLASS_RTCM3) | UBX_ID_RTCM3_1005 << 8)
#define UBX_MSG_RTCM3_1077  ((UBX_CLASS_RTCM3) | UBX_ID_RTCM3_1077 << 8)
#define UBX_MSG_RTCM3_1087  ((UBX_CLASS_RTCM3) | UBX_ID_RTCM3_1087 << 8)
#define UBX_MSG_RXM_PMREQ   ((UBX_CLASS_RXM) | UBX_ID_RXM_PMREQ << 8)

/* RX NAV-PVT message content details */
/*   Bitfield "valid" masks */
#define UBX_RX_NAV_PVT_VALID_VALIDDATE      0x01    /**< validDate (Valid UTC Date) */
#define UBX_RX_NAV_PVT_VALID_VALIDTIME      0x02    /**< validTime (Valid UTC Time) */
#define UBX_RX_NAV_PVT_VALID_FULLYRESOLVED  0x04    /**< fullyResolved (1 = UTC Time of Day has been fully resolved (no seconds uncertainty)) */

/*   Bitfield "flags" masks */
#define UBX_RX_NAV_PVT_FLAGS_GNSSFIXOK      0x01    /**< gnssFixOK (A valid fix (i.e within DOP & accuracy masks)) */
#define UBX_RX_NAV_PVT_FLAGS_DIFFSOLN       0x02    /**< diffSoln (1 if differential corrections were applied) */
#define UBX_RX_NAV_PVT_FLAGS_PSMSTATE       0x1C    /**< psmState (Power Save Mode state (see Power Management)) */
#define UBX_RX_NAV_PVT_FLAGS_HEADVEHVALID   0x20    /**< headVehValid (Heading of vehicle is valid) */
#define UBX_RX_NAV_PVT_FLAGS_CARRSOLN       0xC0    /**< Carrier phase range solution (RTK mode) */



/* RX NAV-TIMEUTC message content details */
/*   Bitfield "valid" masks */
#define UBX_RX_NAV_TIMEUTC_VALID_VALIDTOW   0x01    /**< validTOW (1 = Valid Time of Week) */
#define UBX_RX_NAV_TIMEUTC_VALID_VALIDKWN   0x02    /**< validWKN (1 = Valid Week Number) */
#define UBX_RX_NAV_TIMEUTC_VALID_VALIDUTC   0x04    /**< validUTC (1 = Valid UTC Time) */
#define UBX_RX_NAV_TIMEUTC_VALID_UTCSTANDARD 0xF0    /**< utcStandard (0..15 = UTC standard identifier) */

/* TX CFG-CFG message contents */
#define UBX_TX_CFG_CFG_CLEARMASK        0
#define UBX_TX_CFG_CFG_SAVEMASK         ((1<<0)|(1<<3)) // Specifies ioPort and navConf only, here
#define UBX_TX_CFG_CFG_LOADMASK         0
#define UBX_TX_CFG_CFG_DEVICEMASK       (1<<1)          // Save to Internal Flash

/* TX CFG-PRT message contents */
#define UBX_TX_CFG_PRT_PORTID           0x01        /**< UART1 */
#define UBX_TX_CFG_PRT_PORTID_USB       0x03        /**< USB */
#define UBX_TX_CFG_PRT_PORTID_SPI       0x04        /**< SPI */
#define UBX_TX_CFG_PRT_MODE             0x000008D0  /**< 0b0000100011010000: 8N1 */
#define UBX_TX_CFG_PRT_MODE_SPI         0x00000100
#define UBX_TX_CFG_PRT_BAUDRATE         115200       /**< choose 115200 as GPS baudrate */
#define UBX_TX_CFG_PRT_INPROTOMASK_GPS  ((1<<5) | 0x01) /**< RTCM3 in and UBX in */
#define UBX_TX_CFG_PRT_INPROTOMASK_RTCM (0x01)  /**< UBX in */
#define UBX_TX_CFG_PRT_OUTPROTOMASK_GPS (0x01)          /**< UBX out */
#define UBX_TX_CFG_PRT_OUTPROTOMASK_RTCM ((1<<5) | 0x01)     /**< RTCM3 out and UBX out */

/* TX CFG-RXM message contents */
#define UBX_TX_CFG_RXM_CONTINUOUS       0x00
#define UBX_TX_CFG_RXM_POWERSAVE        0x01

/* TX CFG-PM2 message contents */
#define UBX_TX_CFG_PM2_VERSION			0x02


/* TX CFG-RATE message contents */
#define UBX_TX_CFG_RATE_MEASINTERVAL    1000    /// milliseconds per sample ("measurement")
#define UBX_TX_CFG_RATE_NAVRATE         1       /// ratio between the number of measurements and the number of navigation solutions
#define UBX_TX_CFG_RATE_TIMEREF         0       /**< 0: UTC, 1: GPS time */

/* TX CFG-NAV5 message contents */
#define UBX_TX_CFG_NAV5_MASK            0x0005      /**< Only update dynamic model and fix mode */
#define UBX_TX_CFG_NAV5_DYNMODEL        3       /**< 0 Portable, 2 Stationary, 3 Pedestrian, 4 Automotive, 5 Sea, 6 Airborne <1g, 7 Airborne <2g, 8 Airborne <4g */
#define UBX_TX_CFG_NAV5_DYNMODEL_RTCM   2
#define UBX_TX_CFG_NAV5_FIXMODE         2       /// 1 2D only, 2 3D only, 3 Auto 2D/3D.  (originally was auto, but 3D fix is better)

/* TX CFG-SBAS message contents */
#define UBX_TX_CFG_SBAS_MODE_ENABLED    1               /**< SBAS enabled */
#define UBX_TX_CFG_SBAS_MODE_DISABLED   0               /**< SBAS disabled */
#define UBX_TX_CFG_SBAS_MODE            UBX_TX_CFG_SBAS_MODE_DISABLED   /**< SBAS enabled or disabled */

/* TX CFG-TMODE3 message contents */
#define UBX_TX_CFG_TMODE3_FLAGS         1           /**< start survey-in */
#define UBX_TX_CFG_TMODE3_SVINMINDUR    (3*60)      /**< survey-in: minimum duration [s] (higher=higher precision) */
#define UBX_TX_CFG_TMODE3_SVINACCLIMIT  (10000) /**< survey-in: position accuracy limit 0.1[mm] */

/// TX RXM-PMREQ
#define UBX_TX_RXM_PMREQ_DURATION       0
#define UBX_TX_RXM_FLAGS                (1<<1)      // go into backup

/// TX CFG-RXM
#define UBX_TX_CFG_RXM_RESERVED         0
#define UBX_TX_CFG_RXM_LPMODE           1           // Power Save Mode


/* RTCM3 */
#define RTCM3_PREAMBLE                  0xD3
#define RTCM_INITIAL_BUFFER_LENGTH      300     /**< initial maximum message length of an RTCM message */

typedef struct {
    uint8_t         *buffer;
    uint16_t        buffer_len;
    uint16_t        pos;                        ///< next position in buffer
    uint16_t        message_length;                 ///< message length without header & CRC (both 3 bytes)
} rtcm_message_t;


/*** u-blox protocol binary message and payload definitions ***/
#pragma pack(push, 1)

typedef struct {
    ot_u8   syncB5;
    ot_u8   sync62;
} ubx_sync_t;

/* General: Header */
typedef struct {
    uint16_t    msg;
    uint16_t    length;
} ubx_header_t;

/* Rx NAV-AOPSTATUS */
/// AssistNow Autonomous Status
typedef struct {
    uint32_t    iTOW;           /**< GPS Time of Week [ms] */
    uint8_t     aopCfg;         /**< 0/1 if AOP is enabled */
    uint8_t     status;         /**< 0 if AOP is idle, which means done downloading. */
    uint8_t     reserved[10];
} ubx_payload_rx_nav_aopstatus_t;

/* Rx NAV-POSLLH */
/// Geodetic Position Solution!
typedef struct {
    uint32_t    iTOW;       /**< GPS Time of Week [ms] */
    int32_t     lon;        /**< Longitude [1e-7 deg] */
    int32_t     lat;        /**< Latitude [1e-7 deg] */
    int32_t     height;     /**< Height above ellipsoid [mm] */
    int32_t     hMSL;       /**< Height above mean sea level [mm] */
    uint32_t    hAcc;       /**< Horizontal accuracy estimate [mm] */
    uint32_t    vAcc;       /**< Vertical accuracy estimate [mm] */
} ubx_payload_rx_nav_posllh_t;

/* Rx NAV-DOP */
/// Dilution of Precision
typedef struct {
    uint32_t    iTOW;       /**< GPS Time of Week [ms] */
    uint16_t    gDOP;       /**< Geometric DOP [0.01] */
    uint16_t    pDOP;       /**< Position DOP [0.01] */
    uint16_t    tDOP;       /**< Time DOP [0.01] */
    uint16_t    vDOP;       /**< Vertical DOP [0.01] */
    uint16_t    hDOP;       /**< Horizontal DOP [0.01] */
    uint16_t    nDOP;       /**< Northing DOP [0.01] */
    uint16_t    eDOP;       /**< Easting DOP [0.01] */
} ubx_payload_rx_nav_dop_t;

/* Rx NAV-SOL */
/// Navigation Solution information
typedef struct {
    uint32_t    iTOW;       /**< GPS Time of Week [ms] */
    int32_t     fTOW;       /**< Fractional part of iTOW (range: +/-500000) [ns] */
    int16_t     week;       /**< GPS week */
    uint8_t     gpsFix;     /**< GPSfix type: 0 = No fix, 1 = Dead Reckoning only, 2 = 2D fix, 3 = 3d-fix, 4 = GPS + dead reckoning, 5 = time only fix */
    uint8_t     flags;
    int32_t     ecefX;
    int32_t     ecefY;
    int32_t     ecefZ;
    uint32_t    pAcc;
    int32_t     ecefVX;
    int32_t     ecefVY;
    int32_t     ecefVZ;
    uint32_t    sAcc;
    uint16_t    pDOP;       /**< Position DOP [0.01] */
    uint8_t     reserved1;
    uint8_t     numSV;      /**< Number of SVs used in Nav Solution */
    uint32_t    reserved2;
} ubx_payload_rx_nav_sol_t;

/* Rx NAV-PVT (ubx8) */
/// Position Velocity Time Solution : we can get this big data, but it is
/// probably surplus to requirements
typedef struct {
    uint32_t    iTOW;       /**< GPS Time of Week [ms] */
    uint16_t    year;       /**< Year (UTC)*/
    uint8_t     month;      /**< Month, range 1..12 (UTC) */
    uint8_t     day;        /**< Day of month, range 1..31 (UTC) */
    uint8_t     hour;       /**< Hour of day, range 0..23 (UTC) */
    uint8_t     min;        /**< Minute of hour, range 0..59 (UTC) */
    uint8_t     sec;        /**< Seconds of minute, range 0..60 (UTC) */
    uint8_t     valid;      /**< Validity flags (see UBX_RX_NAV_PVT_VALID_...) */
    uint32_t    tAcc;       /**< Time accuracy estimate (UTC) [ns] */
    int32_t     nano;       /**< Fraction of second (UTC) [-1e9...1e9 ns] */
    uint8_t     fixType;    /**< GNSSfix type: 0 = No fix, 1 = Dead Reckoning only, 2 = 2D fix, 3 = 3d-fix, 4 = GNSS + dead reckoning, 5 = time only fix */
    uint8_t     flags;      /**< Fix Status Flags (see UBX_RX_NAV_PVT_FLAGS_...) */
    uint8_t     flags2;
    uint8_t     numSV;      /**< Number of SVs used in Nav Solution */
    int32_t     lon;        /**< Longitude [1e-7 deg] */
    int32_t     lat;        /**< Latitude [1e-7 deg] */
    int32_t     height;     /**< Height above ellipsoid [mm] */
    int32_t     hMSL;       /**< Height above mean sea level [mm] */
    uint32_t    hAcc;       /**< Horizontal accuracy estimate [mm] */
    uint32_t    vAcc;       /**< Vertical accuracy estimate [mm] */
    int32_t     velN;       /**< NED north velocity [mm/s]*/
    int32_t     velE;       /**< NED east velocity [mm/s]*/
    int32_t     velD;       /**< NED down velocity [mm/s]*/
    int32_t     gSpeed;     /**< Ground Speed (2-D) [mm/s] */
    int32_t     headMot;    /**< Heading of motion (2-D) [1e-5 deg] */
    uint32_t    sAcc;       /**< Speed accuracy estimate [mm/s] */
    uint32_t    headAcc;    /**< Heading accuracy estimate (motion and vehicle) [1e-5 deg] */
    uint16_t    pDOP;       /**< Position DOP [0.01] */
    uint16_t    reserved2;
    uint32_t    reserved3;
    int32_t     headVeh;    /**< (ubx8+ only) Heading of vehicle (2-D) [1e-5 deg] */
    uint32_t    reserved4;  /**< (ubx8+ only) */
} ubx_payload_rx_nav_pvt_t;
#define UBX_PAYLOAD_RX_NAV_PVT_SIZE_UBX7    (sizeof(ubx_payload_rx_nav_pvt_t) - 8)
#define UBX_PAYLOAD_RX_NAV_PVT_SIZE_UBX8    (sizeof(ubx_payload_rx_nav_pvt_t))


/* Rx NAV-TIMEUTC */
/// UTC Time return -- not the UTC 32 bit time I know.
typedef struct {
    uint32_t    iTOW;       /**< GPS Time of Week [ms] */
    uint32_t    tAcc;       /**< Time accuracy estimate (UTC) [ns] */
    int32_t     nano;       /**< Fraction of second, range -1e9 .. 1e9 (UTC) [ns] */
    uint16_t    year;       /**< Year, range 1999..2099 (UTC) */
    uint8_t     month;      /**< Month, range 1..12 (UTC) */
    uint8_t     day;        /**< Day of month, range 1..31 (UTC) */
    uint8_t     hour;       /**< Hour of day, range 0..23 (UTC) */
    uint8_t     min;        /**< Minute of hour, range 0..59 (UTC) */
    uint8_t     sec;        /**< Seconds of minute, range 0..60 (UTC) */
    uint8_t     valid;      /**< Validity Flags (see UBX_RX_NAV_TIMEUTC_VALID_...) */
} ubx_payload_rx_nav_timeutc_t;

/* Rx NAV-SVINFO Part 1 */
typedef struct {
    uint32_t    iTOW;       /**< GPS Time of Week [ms] */
    uint8_t     numCh;      /**< Number of channels */
    uint8_t     globalFlags;
    uint16_t    reserved2;
} ubx_payload_rx_nav_svinfo_part1_t;

/* Rx NAV-SVINFO Part 2 (repeated) */
typedef struct {
    uint8_t     chn;        /**< Channel number, 255 for SVs not assigned to a channel */
    uint8_t     svid;       /**< Satellite ID */
    uint8_t     flags;
    uint8_t     quality;
    uint8_t     cno;        /**< Carrier to Noise Ratio (Signal Strength) [dbHz] */
    int8_t      elev;       /**< Elevation [deg] */
    int16_t     azim;       /**< Azimuth [deg] */
    int32_t     prRes;      /**< Pseudo range residual [cm] */
} ubx_payload_rx_nav_svinfo_part2_t;

/* Rx NAV-SVIN (survey-in info) */
typedef struct {
    uint8_t     version;
    uint8_t     reserved1[3];
    uint32_t    iTOW;
    uint32_t    dur;
    int32_t     meanX;
    int32_t     meanY;
    int32_t     meanZ;
    int8_t      meanXHP;
    int8_t      meanYHP;
    int8_t      meanZHP;
    int8_t      reserved2;
    uint32_t    meanAcc;
    uint32_t    obs;
    uint8_t     valid;
    uint8_t     active;
    uint8_t     reserved3[2];
} ubx_payload_rx_nav_svin_t;

/* Rx NAV-VELNED */
typedef struct {
    uint32_t    iTOW;       /**< GPS Time of Week [ms] */
    int32_t     velN;       /**< North velocity component [cm/s]*/
    int32_t     velE;       /**< East velocity component [cm/s]*/
    int32_t     velD;       /**< Down velocity component [cm/s]*/
    uint32_t    speed;      /**< Speed (3-D) [cm/s] */
    uint32_t    gSpeed;     /**< Ground speed (2-D) [cm/s] */
    int32_t     heading;    /**< Heading of motion 2-D [1e-5 deg] */
    uint32_t    sAcc;       /**< Speed accuracy estimate [cm/s] */
    uint32_t    cAcc;       /**< Course / Heading accuracy estimate [1e-5 deg] */
} ubx_payload_rx_nav_velned_t;

/* Rx MON-HW (ubx6) */
typedef struct {
    uint32_t    pinSel;
    uint32_t    pinBank;
    uint32_t    pinDir;
    uint32_t    pinVal;
    uint16_t    noisePerMS;
    uint16_t    agcCnt;
    uint8_t     aStatus;
    uint8_t     aPower;
    uint8_t     flags;
    uint8_t     reserved1;
    uint32_t    usedMask;
    uint8_t     VP[25];
    uint8_t     jamInd;
    uint16_t    reserved3;
    uint32_t    pinIrq;
    uint32_t    pullH;
    uint32_t    pullL;
} ubx_payload_rx_mon_hw_ubx6_t;

/* Rx MON-HW (ubx7+) */
typedef struct {
    uint32_t    pinSel;
    uint32_t    pinBank;
    uint32_t    pinDir;
    uint32_t    pinVal;
    uint16_t    noisePerMS;
    uint16_t    agcCnt;
    uint8_t     aStatus;
    uint8_t     aPower;
    uint8_t     flags;
    uint8_t     reserved1;
    uint32_t    usedMask;
    uint8_t     VP[17];
    uint8_t     jamInd;
    uint16_t    reserved3;
    uint32_t    pinIrq;
    uint32_t    pullH;
    uint32_t    pullL;
} ubx_payload_rx_mon_hw_ubx7_t;

/* Rx MON-VER Part 1 */
typedef struct {
    uint8_t     swVersion[30];
    uint8_t     hwVersion[10];
} ubx_payload_rx_mon_ver_part1_t;

/* Rx MON-VER Part 2 (repeated) */
typedef struct {
    uint8_t     extension[30];
} ubx_payload_rx_mon_ver_part2_t;

/* Rx ACK-ACK */
typedef union {
    uint16_t    msg;
    struct {
        uint8_t clsID;
        uint8_t msgID;
    };
} ubx_payload_rx_ack_ack_t;

/* Rx ACK-NAK */
typedef union {
    uint16_t    msg;
    struct {
        uint8_t clsID;
        uint8_t msgID;
    };
} ubx_payload_rx_ack_nak_t;


typedef struct {
    uint8_t     type;
    uint8_t     version;
    uint8_t     infoCode;
    uint8_t     msgId;
    uint8_t     msgPayloadStart[4];
} ubx_payload_rx_mga_ack_t;


/* Tx CFG-CFG */
typedef struct {
    uint32_t    clearMask;
    uint32_t    saveMask;
    uint32_t    loadMask;
    uint8_t     deviceMask;
} ubx_payload_tx_cfg_cfg_t;


/* Tx CFG-PRT */
typedef struct {
    uint8_t     portID;
    uint8_t     reserved0;
    uint16_t    txReady;
    uint32_t    mode;
    uint32_t    baudRate;
    uint16_t    inProtoMask;
    uint16_t    outProtoMask;
    uint16_t    flags;
    uint16_t    reserved5;
} ubx_payload_tx_cfg_prt_t;

/* Tx CFG-RXM */
typedef struct {
    uint8_t     reserved1;
    uint8_t     lpmode;
} ubx_payload_tx_cfg_rxm_t;

/* Tx CFG-PM2 */
typedef struct {
    uint8_t     version;
    uint8_t		reserved1;
    uint8_t		maxStartupStateDur;
    uint8_t     reserved2;
    uint32_t	flags;
    uint32_t	updatePeriod;
    uint32_t	searchPeriod;
    uint32_t	gridOffset;
    uint16_t	onTime;
	uint16_t	minAcqTime;
	uint8_t		reserved3[20];
	uint32_t	extintInactivityMs;
} ubx_payload_tx_cfg_pm2_t;

/* Tx CFG-PMS */
typedef struct {
    uint8_t     version;
    uint8_t     powerSetupValue;
    uint16_t    period;
    uint16_t    onTime;
    uint8_t     reserved1[2];
} ubx_payload_tx_cfg_pms_t;

/* Tx CFG-RATE */
typedef struct {
    uint16_t    measRate;   /**< Measurement Rate, GPS measurements are taken every measRate milliseconds */
    uint16_t    navRate;    /**< Navigation Rate, in number of measurement cycles. This parameter cannot be changed, and must be set to 1 */
    uint16_t    timeRef;    /**< Alignment to reference time: 0 = UTC time, 1 = GPS time */
} ubx_payload_tx_cfg_rate_t;

/* Tx CFG-NAV5 */
typedef struct {
    uint16_t    mask;
    uint8_t     dynModel;   /**< Dynamic Platform model: 0 Portable, 2 Stationary, 3 Pedestrian, 4 Automotive, 5 Sea, 6 Airborne <1g, 7 Airborne <2g, 8 Airborne <4g */
    uint8_t     fixMode;    /**< Position Fixing Mode: 1 2D only, 2 3D only, 3 Auto 2D/3D */
    int32_t     fixedAlt;
    uint32_t    fixedAltVar;
    int8_t      minElev;
    uint8_t     drLimit;
    uint16_t    pDop;
    uint16_t    tDop;
    uint16_t    pAcc;
    uint16_t    tAcc;
    uint8_t     staticHoldThresh;
    uint8_t     dgpsTimeOut;
    uint8_t     cnoThreshNumSVs;    /**< (ubx7+ only, else 0) */
    uint8_t     cnoThresh;          /**< (ubx7+ only, else 0) */
    uint16_t    reserved;
    uint16_t    staticHoldMaxDist;  /**< (ubx8+ only, else 0) */
    uint8_t     utcStandard;        /**< (ubx8+ only, else 0) */
    uint8_t     reserved3;
    uint32_t    reserved4;
} ubx_payload_tx_cfg_nav5_t;

/* Tx CFG-NAVX5 (ubx8) */
/// Position Velocity Time Solution : we can get this big data, but it is
/// probably surplus to requirements
typedef struct {
	uint16_t	version;
	uint16_t	mask1;
	uint32_t	mask2;
	uint8_t		reserved1[2];
	uint8_t		minSVs;
	uint8_t		maxSVs;
	uint8_t		minCNO;
	uint8_t		reserved2;
	uint8_t		iniFix3D;
	uint8_t		reserved3[2];
	uint8_t		ackAiding;
	uint16_t	wknrollover;
	uint8_t		sigAttnCompMode;
	uint8_t		reserved4;
	uint8_t		reserved5[2];
	uint8_t		reserved6[2];
	uint8_t		usePPP;
	uint8_t		aopCfg;
	uint8_t		reserved7[2];
	uint16_t	aopOrbMaxErr;
	uint8_t		reserved8[4];
	uint8_t		reserved9[3];
	uint8_t		useAdr;
} ubx_payload_tx_cfg_navx5_t;

/* Tx NAV-AOPSTATUS */
/// AssistNow Autonomous Status
typedef ubx_payload_rx_nav_aopstatus_t ubx_payload_tx_nav_aopstatus_t;

/* tx cfg-sbas */
typedef struct {
    uint8_t     mode;
    uint8_t     usage;
    uint8_t     maxSBAS;
    uint8_t     scanmode2;
    uint32_t    scanmode1;
} ubx_payload_tx_cfg_sbas_t;

/* Tx CFG-MSG */
typedef struct {
    union {
        uint16_t    msg;
        struct {
            uint8_t msgClass;
            uint8_t msgID;
        };
    };
    uint8_t rate;
} ubx_payload_tx_cfg_msg_t;

/* Tx CFG-RST*/
typedef struct {
    uint16_t    navBbrMask;
    uint8_t     resetMode;
    uint8_t     reserved1;
} ubx_payload_tx_cfg_rst_t;

/* CFG-TMODE3 ublox 8 (protocol version >= 20) */
typedef struct {
    uint8_t     version;
    uint8_t     reserved1;
    uint16_t    flags;
    int32_t     ecefXOrLat;
    int32_t     ecefYOrLon;
    int32_t     ecefZOrAlt;
    int8_t      ecefXOrLatHP;
    int8_t      ecefYOrLonHP;
    int8_t      ecefZOrAltHP;
    uint8_t     reserved2;
    uint32_t    fixedPosAcc;
    uint32_t    svinMinDur;
    uint32_t    svinAccLimit;
    uint8_t     reserved3[8];
} ubx_payload_tx_cfg_tmode3_t;


typedef struct {
    uint8_t     type;
    uint8_t     version;
    uint8_t     svId;
    uint8_t     gnssId;
    uint8_t     year;
    uint8_t     month;
    uint8_t     day;
    uint8_t     reserved1;
    uint8_t     data[64];
    uint32_t    reserved2;
} ubx_payload_tx_mga_ano_t;


typedef struct {
    uint32_t    duration;
    uint32_t    flags;
} ubx_payload_tx_rxm_pmreq_t;




/* General message and payload buffer union */
typedef union {
    ubx_payload_rx_ack_ack_t            rx_ack_ack;
    ubx_payload_rx_ack_nak_t            rx_ack_nak;
    ubx_payload_rx_mga_ack_t            rx_mga_ack;
    ubx_payload_rx_mon_hw_ubx7_t        rx_mon_hw;
    ubx_payload_rx_mon_ver_part1_t      rx_mon_ver_part1;
    ubx_payload_rx_mon_ver_part2_t      rx_mon_ver_part2;
    ubx_payload_rx_nav_pvt_t            rx_nav_pvt;
    ubx_payload_rx_nav_aopstatus_t      rx_nav_aopstatus;
    ubx_payload_rx_nav_posllh_t         rx_nav_posllh;
    ubx_payload_rx_nav_sol_t            rx_nav_sol;
    ubx_payload_rx_nav_dop_t            rx_nav_dop;
    ubx_payload_rx_nav_timeutc_t        rx_nav_timeutc;
    ubx_payload_rx_nav_svinfo_part1_t   rx_nav_svinfo_part1;
    ubx_payload_rx_nav_svinfo_part2_t   rx_nav_svinfo_part2;
    ubx_payload_rx_nav_svin_t           rx_nav_svin;
    ubx_payload_rx_nav_velned_t         rx_nav_velned;
    ubx_payload_tx_cfg_cfg_t            tx_cfg_cfg;
    ubx_payload_tx_cfg_pm2_t			tx_cfg_pm2;
    ubx_payload_tx_cfg_pms_t            tx_cfg_pms;
    ubx_payload_tx_cfg_prt_t            tx_cfg_prt;
    ubx_payload_tx_cfg_rst_t            tx_cfg_rst;
    ubx_payload_tx_cfg_rate_t           tx_cfg_rate;
    ubx_payload_tx_cfg_rxm_t			tx_cfg_rxm;
    ubx_payload_tx_cfg_nav5_t           tx_cfg_nav5;
    ubx_payload_tx_cfg_navx5_t			tx_cfg_navx5;
    ubx_payload_tx_cfg_sbas_t           tx_cfg_sbas;
    ubx_payload_tx_cfg_msg_t            tx_cfg_msg;
    ubx_payload_tx_cfg_tmode3_t         tx_cfg_tmode3;
    ubx_payload_tx_mga_ano_t            tx_mga_ano;
    ubx_payload_tx_rxm_pmreq_t          tx_rxm_pmreq;
    ubx_payload_tx_nav_aopstatus_t      tx_nav_aopstatus;
} ubx_buf_t;

#pragma pack(pop)
/*** END OF u-blox protocol binary message and payload definitions ***/









/// Calculate FNV1 hash
/*
uint32_t fnv1_32_str(uint8_t *str, uint32_t hval);

struct vehicle_gps_position_s *_gps_position {nullptr};
struct satellite_info_s *_satellite_info {nullptr};
uint64_t        _last_timestamp_time{0};
bool            _configured{false};
ubx_ack_state_t     _ack_state{UBX_ACK_IDLE};
bool            _got_posllh{false};
bool            _got_velned{false};
ubx_decode_state_t  _decode_state{};
uint16_t        _rx_msg{};
ubx_rxmsg_state_t   _rx_state{UBX_RXMSG_IGNORE};
uint16_t        _rx_payload_length{};
uint16_t        _rx_payload_index{};
uint8_t         _rx_ck_a{};
uint8_t         _rx_ck_b{};
gps_abstime     _disable_cmd_last{0};
uint16_t        _ack_waiting_msg{0};
ubx_buf_t       _buf{};
uint32_t        _ubx_version{0};
bool            _use_nav_pvt{false};
OutputMode      _output_mode{OutputMode::GPS};

rtcm_message_t  *_rtcm_message{nullptr};

const Interface     _interface;
*/

#endif /* UBX_H_ */
