/* Copyright 2010-2013 JP Norair
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
  * @file       /otradio/spirit1/SPIRIT1_defaults.h
  * @author     JP Norair
  * @version    R100
  * @date       4 Jan 2013
  * @brief      SPIRIT1 Radio register default definitions & macros.
  * @ingroup    SPIRIT1
  *
  ******************************************************************************
  */

#ifndef __SPIRIT1_defaults_H
#define __SPIRIT1_defaults_H

#include "OT_platform.h"
#include "OT_types.h"
#include "SPIRIT1_registers.h"


// Some radio register settings constants
// These are basically for the CC430, and the POSIX impl just ignores them
#ifndef RF_PARAM_BAND
#   define RF_PARAM_BAND    433
#endif

#if (RF_PARAM_BAND != 433)
#   error "RF_PARAM_BAND is not set to an implemented value"
#endif


#if (BOARD_FEATURE_RFXTAL != ENABLED)
#   undef BOARD_PARAM_RFHz
#   define BOARD_PARAM_RFHz  BOARD_PARAM_HFHz
#   define _XTAL_SRC    _EXT_REF
#else
#   define _XTAL_SRC    0
#endif


#if (BOARD_PARAM_RFHz == 24000000)
#   define _24MHz
#elif (BOARD_PARAM_RFHz == 25000000)
#   define _25MHz
#elif (BOARD_PARAM_RFHz == 26000000)
#   define _26MHz
#elif (BOARD_PARAM_RFHz == 48000000)
#   define _48MHz
#elif (BOARD_PARAM_RFHz == 50000000)
#   define _50MHz
#elif (BOARD_PARAM_RFHz == 52000000)
#   define _52MHz
#else
#   error "BOARD_PARAM_RFHz must be 24, 25, 26, 48, 50, or 52 MHz"
#endif



//R00 (Power-up default)
#define DRF_ANA_FUNC_CONF1      (_GM_CONF_25m6s | _SET_BLD_LVL_2V7)

//R01
#if (BOARD_FEATURE_RFXTAL != ENABLED)
#   define _XTAL_BYPASS     _EXT_REF
#else
#   define _XTAL_BYPASS     0
#endif
#if (defined(_24MHz) || defined(_48MHz))
#   define DRF_ANA_FUNC_CONF0   (0x80 | _XTAL_BYPASS)
#else
#   define DRF_ANA_FUNC_CONF0   (0x80 | _24_26MHz_SELECT | _XTAL_BYPASS)
#endif


//R02 (power-up default)
#define DRF_GPIO3_CONF          (_GPIO_SELECT(RFGPO_GND) | _GPIO_MODE_OUTPUT)

//R03 (power-up default)
#define DRF_GPIO2_CONF          (_GPIO_SELECT(RFGPO_GND) | _GPIO_MODE_OUTPUT)

//R04 (power-up default)
#define DRF_GPIO1_CONF          (_GPIO_SELECT(RFGPO_GND) | _GPIO_MODE_OUTPUT)

//R05 (power-up default)
#define DRF_GPIO0_CONF          (_GPIO_SELECT(RFGPO_nPOR) | _GPIO_MODE_OUTPUT)

//R06 (power-up default)
#define DRF_MCU_CK_CONF         (0)

//RB4 
#if (defined(_24MHz) || defined(_25MHz) || defined(_26MHz))
#   define DRF_XO_RCO_TEST      (_PD_CLKDIV)
#else
#   define DRF_XO_RCO_TEST      (0)
#endif

//R9E
#if (RF_PARAM_BAND == 433)
#   define DRF_SYNTH_CONFIG1    _VCO_433MHz_SEL
#elif (RF_PARAM_BAND == 866)
#   define DRF_SYNTH_CONFIG1    _VCO_866MHz_SEL
#endif

//R9F
#define DRF_SYNTH_CONFIG0       (_SEL_TSPLIT_3n47s)

//R07
#if defined(_24MHz)
#   define DRF_IF_OFFSET_ANA    _IF_OFFSET_ANA_24MHz
#elif defined(_25MHz)
#   define DRF_IF_OFFSET_ANA    _IF_OFFSET_ANA_25MHz
#elif defined(_26MHz)
#   define DRF_IF_OFFSET_ANA    _IF_OFFSET_ANA_26MHz
#elif defined(_48MHz)
#   define DRF_IF_OFFSET_ANA    _IF_OFFSET_ANA_48MHz
#elif defined(_50MHz)
#   define DRF_IF_OFFSET_ANA    _IF_OFFSET_ANA_50MHz
#elif defined(_52MHz)
#   define DRF_IF_OFFSET_ANA    _IF_OFFSET_ANA_52MHz
#endif

//R08
#if defined(_24MHz)
#   define DRF_SYNT3_SYNT_HI    _SYNT_HI_D7_24MHz
#elif defined(_25MHz)
#   define DRF_SYNT3_SYNT_HI    _SYNT_HI_D7_25MHz
#elif defined(_26MHz)
#   define DRF_SYNT3_SYNT_HI    _SYNT_HI_D7_26MHz
#elif defined(_48MHz)
#   define DRF_SYNT3_SYNT_HI    _SYNT_HI_D7_48MHz
#elif defined(_50MHz)
#   define DRF_SYNT3_SYNT_HI    _SYNT_HI_D7_50MHz
#elif defined(_52MHz)
#   define DRF_SYNT3_SYNT_HI    _SYNT_HI_D7_52MHz
#endif
#if (RF_PARAM_BAND == 169)
#   define DRF_SYNT3_WCP        _WCP_169MHz
#elif (RF_PARAM_BAND == 315)
#   define DRF_SYNT3_WCP        _WCP_315MHz
#elif (RF_PARAM_BAND == 433)
#   define DRF_SYNT3_WCP        _WCP_433MHz
#elif (RF_PARAM_BAND == 866)
#   define DRF_SYNT3_WCP        _WCP_866MHz
#endif
#define DRF_SYNT3               (DRF_SYNT3_WCP | DRF_SYNT3_SYNT_HI)

//R09
#if defined(_24MHz)
#   define DRF_SYNT2            _SYNT2_D7_24MHz
#elif defined(_25MHz)
#   define DRF_SYNT2            _SYNT2_D7_25MHz
#elif defined(_26MHz)
#   define DRF_SYNT2            _SYNT2_D7_26MHz
#elif defined(_48MHz)
#   define DRF_SYNT2            _SYNT2_D7_48MHz
#elif defined(_50MHz)
#   define DRF_SYNT2            _SYNT2_D7_50MHz
#elif defined(_52MHz)
#   define DRF_SYNT2            _SYNT2_D7_52MHz
#endif

//R0A
#if defined(_24MHz)
#   define DRF_SYNT1            _SYNT1_D7_24MHz
#elif defined(_25MHz)
#   define DRF_SYNT1            _SYNT1_D7_25MHz
#elif defined(_26MHz)
#   define DRF_SYNT1            _SYNT1_D7_26MHz
#elif defined(_48MHz)
#   define DRF_SYNT1            _SYNT1_D7_48MHz
#elif defined(_50MHz)
#   define DRF_SYNT1            _SYNT1_D7_50MHz
#elif defined(_52MHz)
#   define DRF_SYNT1            _SYNT1_D7_52MHz
#endif

//R0B
#if defined(_24MHz)
#   define DRF_SYNT0_SYNT_LO    _SYNT_LO_D7_24MHz
#elif defined(_25MHz)
#   define DRF_SYNT0_SYNT_LO    _SYNT_LO_D7_25MHz
#elif defined(_26MHz)
#   define DRF_SYNT0_SYNT_LO    _SYNT_LO_D7_26MHz
#elif defined(_48MHz)
#   define DRF_SYNT0_SYNT_LO    _SYNT_LO_D7_48MHz
#elif defined(_50MHz)
#   define DRF_SYNT0_SYNT_LO    _SYNT_LO_D7_50MHz
#elif defined(_52MHz)
#   define DRF_SYNT0_SYNT_LO    _SYNT_LO_D7_52MHz
#endif
#if (RF_PARAM_BAND == 169)
#   define DRF_SYNT0_BS         _BS_169MHz
#elif (RF_PARAM_BAND == 315)
#   define DRF_SYNT0_BS         _BS_315MHz
#elif (RF_PARAM_BAND == 433)
#   define DRF_SYNT0_BS         _BS_433MHz
#elif (RF_PARAM_BAND == 866)
#   define DRF_SYNT0_BS         _BS_866MHz
#endif
#define DRF_SYNT0               (DRF_SYNT0_SYNT_LO | DRF_SYNT0_BS)

//R0C
#if defined(_24MHz)
#   define DRF_CHSPACE          _CHSPACE_D7_24MHz
#elif defined(_25MHz)
#   define DRF_CHSPACE          _CHSPACE_D7_25MHz
#elif defined(_26MHz)
#   define DRF_CHSPACE          _CHSPACE_D7_26MHz
#elif defined(_48MHz)
#   define DRF_CHSPACE          _CHSPACE_D7_48MHz
#elif defined(_50MHz)
#   define DRF_CHSPACE          _CHSPACE_D7_50MHz
#elif defined(_52MHz)
#   define DRF_CHSPACE          _CHSPACE_D7_52MHz
#endif

//R0D
#if defined(_24MHz)
#   define DRF_IF_OFFSET_DIG    _IF_OFFSET_DIG_24MHz
#elif defined(_25MHz)
#   define DRF_IF_OFFSET_DIG    _IF_OFFSET_DIG_25MHz
#elif defined(_26MHz)
#   define DRF_IF_OFFSET_DIG    _IF_OFFSET_DIG_26MHz
#elif defined(_48MHz)
#   define DRF_IF_OFFSET_DIG    _IF_OFFSET_DIG_48MHz
#elif defined(_50MHz)
#   define DRF_IF_OFFSET_DIG    _IF_OFFSET_DIG_50MHz
#elif defined(_52MHz)
#   define DRF_IF_OFFSET_DIG    _IF_OFFSET_DIG_52MHz
#endif

//R0E (power-up default)
#define DRF_FC_OFFSET1          (0)

//R0F (power-up default)
#define DRF_FC_OFFSET0          (0)

//R10-17: Power ramp.  It is set automatically in the driver
///@todo create half-dB LUT from hermite-cubic interpolated model
#define DRF_PAPOWER8            3       //12dBm     -4
#define DRF_PAPOWER7            7       //6dBm      -6
#define DRF_PAPOWER6            13      //0dBm      -24
#define DRF_PAPOWER5            37      //-6dBm     -16
#define DRF_PAPOWER4            53      //-12dBm    -11
#define DRF_PAPOWER3            64      //-18dBm    -14
#define DRF_PAPOWER2            78      //-24dBm    -50
#define DRF_PAPOWER1            0       //-30dBm

//R18
#if (RF_PARAM_BAND == 169)
#   define DRF_PAPOWER0         (_CWC_3p6F | _PA_RAMP_ENABLE | __PA_RAMP_STEP_WIDTH(3) | __PA_LEVEL_MAX_INDEX(7))
#elif (RF_PARAM_BAND == 315)
#   define DRF_PAPOWER0         (_CWC_2p4F | _PA_RAMP_ENABLE | __PA_RAMP_STEP_WIDTH(3) | __PA_LEVEL_MAX_INDEX(7))
#elif (RF_PARAM_BAND == 433)
#   define DRF_PAPOWER0         (_CWC_1p2F | _PA_RAMP_ENABLE | __PA_RAMP_STEP_WIDTH(3) | __PA_LEVEL_MAX_INDEX(7))
#elif (RF_PARAM_BAND == 866)
#   define DRF_PAPOWER0         (_CWC_0pF  | _PA_RAMP_ENABLE | __PA_RAMP_STEP_WIDTH(3) | __PA_LEVEL_MAX_INDEX(7))
#endif

//R19 (N/A)


//R1A: It is set in the driver when picking a channel
#if defined(_24MHz)
#   define DRF_MOD1_LS          _DR_M_D7LS_24MHz
#   define DRF_MOD1_HS          _DR_M_D7HS_24MHz
#elif defined(_25MHz)
#   define DRF_MOD1_LS          _DR_M_D7LS_25MHz
#   define DRF_MOD1_HS          _DR_M_D7HS_25MHz
#elif defined(_26MHz)
#   define DRF_MOD1_LS          _DR_M_D7LS_26MHz
#   define DRF_MOD1_HS          _DR_M_D7HS_26MHz
#elif defined(_48MHz)
#   define DRF_MOD1_LS          _DR_M_D7LS_24MHz
#   define DRF_MOD1_HS          _DR_M_D7HS_24MHz
#elif defined(_50MHz)
#   define DRF_MOD1_LS          _DR_M_D7LS_25MHz
#   define DRF_MOD1_HS          _DR_M_D7HS_25MHz
#elif defined(_52MHz)
#   define DRF_MOD1_LS          _DR_M_D7LS_26MHz
#   define DRF_MOD1_HS          _DR_M_D7HS_26MHz
#endif
#define DRF_MOD1                DRF_MOD1_LS

//R1B: It is set in the driver when picking a channel
#if defined(_24MHz)
#   define DRF_MOD0_LS          (_BT_SEL_1  | _MOD_TYPE_GFSK | _DR_E_D7LS_24MHz)
#   define DRF_MOD0_HS          (_BT_SEL_05 | _MOD_TYPE_GFSK | _DR_E_D7HS_24MHz)
#elif defined(_25MHz)
#   define DRF_MOD0_LS          (_BT_SEL_1  | _MOD_TYPE_GFSK | _DR_E_D7LS_25MHz)
#   define DRF_MOD0_HS          (_BT_SEL_05 | _MOD_TYPE_GFSK | _DR_E_D7HS_25MHz)
#elif defined(_26MHz)
#   define DRF_MOD0_LS          (_BT_SEL_1  | _MOD_TYPE_GFSK | _DR_E_D7LS_26MHz)
#   define DRF_MOD0_HS          (_BT_SEL_05 | _MOD_TYPE_GFSK | _DR_M_D7HS_26MHz)
#elif defined(_48MHz)
#   define DRF_MOD0_LS          (_BT_SEL_1  | _MOD_TYPE_GFSK | _DR_E_D7LS_24MHz)
#   define DRF_MOD0_HS          (_BT_SEL_05 | _MOD_TYPE_GFSK | _DR_E_D7HS_24MHz)
#elif defined(_50MHz)
#   define DRF_MOD0_LS          (_BT_SEL_1  | _MOD_TYPE_GFSK | _DR_E_D7LS_25MHz)
#   define DRF_MOD0_HS          (_BT_SEL_05 | _MOD_TYPE_GFSK | _DR_E_D7HS_25MHz)
#elif defined(_52MHz)
#   define DRF_MOD0_LS          (_BT_SEL_1  | _MOD_TYPE_GFSK | _DR_E_D7LS_26MHz)
#   define DRF_MOD0_HS          (_BT_SEL_05 | _MOD_TYPE_GFSK | _DR_E_D7HS_26MHz)
#endif
#define DRF_MOD0                DRF_MOD0_LS

//R1C
#if defined(_24MHz)
#   define DRF_FDEV0            (_FDEV_E_D7_24MHz | _FDEV_M_D7_24MHz)
#elif defined(_25MHz)
#   define DRF_FDEV0            (_FDEV_E_D7_25MHz | _FDEV_M_D7_25MHz)
#elif defined(_26MHz)
#   define DRF_FDEV0            (_FDEV_E_D7_26MHz | _FDEV_M_D7_26MHz)
#elif defined(_48MHz)
#   define DRF_FDEV0            (_FDEV_E_D7_48MHz | _FDEV_M_D7_48MHz)
#elif defined(_50MHz)
#   define DRF_FDEV0            (_FDEV_E_D7_50MHz | _FDEV_M_D7_50MHz)
#elif defined(_52MHz)
#   define DRF_FDEV0            (_FDEV_E_D7_52MHz | _FDEV_M_D7_52MHz)
#endif

//R1D: It is set in the driver when picking a channel
#if defined(_24MHz)
#   define DRF_CHFLT_LS         (_CHFLT_M_D7LS_24MHz | _CHFLT_E_D7LS_24MHz)
#   define DRF_CHFLT_HS         (_CHFLT_M_D7HS_24MHz | _CHFLT_E_D7HS_24MHz)
#elif defined(_25MHz)
#   define DRF_CHFLT_LS         (_CHFLT_M_D7LS_25MHz | _CHFLT_E_D7LS_25MHz)
#   define DRF_CHFLT_HS         (_CHFLT_M_D7HS_25MHz | _CHFLT_E_D7HS_25MHz)
#elif defined(_26MHz)
#   define DRF_CHFLT_LS         (_CHFLT_M_D7LS_26MHz | _CHFLT_E_D7LS_26MHz)
#   define DRF_CHFLT_HS         (_CHFLT_M_D7HS_26MHz | _CHFLT_E_D7HS_26MHz)
#elif defined(_48MHz)
#   define DRF_CHFLT_LS         (_CHFLT_M_D7LS_24MHz | _CHFLT_E_D7LS_24MHz)
#   define DRF_CHFLT_HS         (_CHFLT_M_D7HS_24MHz | _CHFLT_E_D7HS_24MHz)
#elif defined(_50MHz)
#   define DRF_CHFLT_LS         (_CHFLT_M_D7LS_25MHz | _CHFLT_E_D7LS_25MHz)
#   define DRF_CHFLT_HS         (_CHFLT_M_D7HS_25MHz | _CHFLT_E_D7HS_25MHz)
#elif defined(_52MHz)
#   define DRF_CHFLT_LS         (_CHFLT_M_D7LS_26MHz | _CHFLT_E_D7LS_26MHz)
#   define DRF_CHFLT_HS         (_CHFLT_M_D7HS_26MHz | _CHFLT_E_D7HS_26MHz)
#endif

//R1E (using power-up defaults)
#define DRF_AFC2                (_AFC_ENABLED | _AFC_MODE_CLOSE_ON_SLICER | __AFC_PD_LEAKAGE(8))

//R1F (using power-up defaults)
#define DRF_AFC1                0x18

//R20 (using power-up defaults)
#define DRF_AFC0                (__AFC_FAST_GAIN(2) | __AFC_SLOW_GAIN(5))

//R21 (using power-up defaults)
#define DRF_RSSI_FLT            (__RSSI_FLT(14) | _CS_MODE_STATIC)

//R22: It is set in the driver when picking a channel
#define DRF_RSSI_TH             __RSSI_TH(-100)

//R23 (using power-up defaults)
#define DRF_CLOCKREC            (__CLK_REC_P_GAIN(2) | _PSTFLT_LEN | __CLK_REC_I_GAIN(8))

//R24 (using power-up defaults)
#define DRF_AGCCTRL2             ((2<<4) | __MEAS_TIME(2))

//R25 (using power-up defaults)
#define DRF_AGCCTRL1            (__THRESHOLD_HI(6) | __THRESHOLD_LO(5))

//R26 (using power-up defaults)
#define DRF_AGCCTRL0            (_AGC_ENABLE | 0x0A)

//R27: Target is (100us < RSSI measuring time < 200us)
#if defined(_24MHz) || defined(_25MHz) || defined(_26MHz)
#   define DRF_ANT_SELECT_CONF  (__AS_MEAS_TIME(5))
#else
#   define DRF_ANT_SELECT_CONF  (__AS_MEAS_TIME(4))
#endif

//R28-R2F (N/A)

//R30 (using power-up defaults)
///@note depending on observation of SPIRIT1 FEC capabilities, this may change
#define DRF_PCKTCTRL4           (_ADDRESS_LEN_BASIC | __CONTROL_LEN(0))

//R31 (using power-up defaults)
///@note depending on observation of SPIRIT1 FEC capabilities, this may change
#define DRF_PCKTCTRL3           (_PCKT_FRMT_BASIC | _RX_MODE_NORMAL | __LEN_WID(8))

//R32: The preamble length & fixed/var is set when picking a channel
#define DRF_PCKTCTRL2_LSFG      (__PREAMBLE_LENGTH(4) | _SYNC_LENGTH_3 | _FIX_VAR_LEN)
#define DRF_PCKTCTRL2_LSBG      (__PREAMBLE_LENGTH(4) | _SYNC_LENGTH_3)
#define DRF_PCKTCTRL2_HSFG      (__PREAMBLE_LENGTH(6) | _SYNC_LENGTH_3 | _FIX_VAR_LEN)
#define DRF_PCKTCTRL2_HSBG      (__PREAMBLE_LENGTH(6) | _SYNC_LENGTH_3)

//R33: It is changed when picking a channel
///@todo observe CRC operation and potentially change this
#define DRF_PCKTCTRL1           (_CRC_MODE_NONE | _WHIT_EN | _TXSOURCE_NORMAL)

//R34 (using power-up default)
#define DRF_PCKTLEN1            0

//R35: set when TX'ing data
#define DRF_PCKTLEN0            0x14

//R36: unused
#define DRF_SYNC4               0x88

//R37: unused
#define DRF_SYNC3               0x88

//R38: set when selecting FG or BG data, and FEC mode
#define DRF_SYNC2               0xE6

//R39: set when selecting FG or BG data, and FEC mode
#define DRF_SYNC1               0xD0

//R3A:                
#define DRF_QI                  (__SQI_TH(0) | __PQI_TH(4) | _SQI_EN | _PQI_EN)

//R3B-3D: unused
#define DRF_MBUS_PRMBL          0x20
#define DRF_MBUS_PSTMBL         0x20
#define DRF_MBUS_CTRL           0

//R3E-41: set during RX & TX, except TX-almost-empty which is always 5
#define DRF_FIFO_CONFIG3        96
#define DRF_FIFO_CONFIG2        96
#define DRF_FIFO_CONFIG1        96
#define DRF_FIFO_CONFIG0        5

//R42-4E: unused for time being.  Control Field might be used in later impls.
#define DRF_PCKT_FLT_GOALS12     0x42
#define DRF_PCKT_FLT_GOALS11     0x43
#define DRF_PCKT_FLT_GOALS10     0x44
#define DRF_PCKT_FLT_GOALS9      0x45
#define DRF_PCKT_FLT_GOALS8      0x46
#define DRF_PCKT_FLT_GOALS7      0x47
#define DRF_PCKT_FLT_GOALS6      0x48
#define DRF_PCKT_FLT_GOALS5      0x49
#define DRF_PCKT_FLT_GOALS4      0x4A
#define DRF_PCKT_FLT_GOALS3      0x4B
#define DRF_PCKT_FLT_GOALS2      0x4C
#define DRF_PCKT_FLT_GOALS1      0x4D
#define DRF_PCKT_FLT_GOALS0      0x4E

//R4F
//more investigation on CRC & control fields needed
#define DRF_PCKT_FLT_OPTIONS    (_RX_TIMEOUT_AND_OR_SELECT)
//#define DRF_PCKT_FLT_OPTIONS    (_RX_TIMEOUT_AND_OR_SELECT | _CRC_CHECK)

//R50 (using power-up defaults)
#define DRF_PROTOCOL2           (_VCO_CALIBRATION)
//#   define _TX_SEQ_NUM_RELOAD   (3<<3)
//#   define _RCO_CALIBRATION     (1<<2)

//R51 (using power-up defaults)
///@note more investigation needed for CSMA engine
#define DRF_PROTOCOL1           0

//R52 (using power-up defaults)
///@note more investigation needed on RETX, PERS_RX, PERS_TX
#define DRF_PROTOCOL0           (__NMAX_RETX(0) | _NACK_TX)

//R53-54: Set during RX or CSMA configuration
//RX_TIMEOUT 16bit timer.  Needs more investigation
#if (defined(_24MHz) || defined(_48MHz))
#   define DRF_TIMERS5          (1)
#else
#   define DRF_TIMERS5          (1)
#endif
#define DRF_TIMERS4             (0)

//R55-58: Used for OpenTag driver CSMA
#if defined(_34700Hz)
#   define DRF_TIMERS3          33
#   define DRF_TIMERS1          33
#elif defined(_32768Hz)
#   define DRF_TIMERS3          32
#   define DRF_TIMERS1          32
#endif
#define DRF_TIMERS2             0
#define DRF_TIMERS0             0

// CSMA engine does not appear to be sophisticated enough to do DASH7 CSMA, but
// the documentation available is limited as of 4 Jan 2013

//R64-67: unused
#define DRF_CSMA_CONFIG3        255
#define DRF_CSMA_CONFIG2        0
#define DRF_CSMA_CONFIG1        (__BU_PRESCALER(1) | _CCA_PERIOD_64bit)
#define DRF_CSMA_CONFIG0        (__CCA_LENGTH(0) | __NBACKOFF_MAX(0))

//R64-67: unused
#define DRF_TX_CTRL_FIELD3      0
#define DRF_TX_CTRL_FIELD2      0
#define DRF_TX_CTRL_FIELD1      0
#define DRF_TX_CTRL_FIELD0      0

//R6C: It is set by driver when selecting a channel
#define DRF_CHNUM               7

//RA1 (using power-up defaults)
#define DRF_VCO_CONFIG          __VCO_GEN_CURR(17)

//R6D (using power-up defaults)
#define DRF_RCO_VCO_CALIBR_IN2  (__RWT_IN(7) | __RFB_IN(0))

//R6E (using power-up defaults)
#define DRF_RCO_VCO_CALIBR_IN1  (__VCO_CALIBR_TX(72))

//R6F (using power-up defaults)
#define DRF_RCO_VCO_CALIBR_IN0  (__VCO_CALIBR_RX(72))

//R90-93 (using power-up defaults)
#define DRF_IRQ_MASK3           (0)
#define DRF_IRQ_MASK2           (0)
#define DRF_IRQ_MASK1           (0)
#define DRF_IRQ_MASK0           (0)

//RA4 (using power-up defaults)
#define DRF_PM_CONFIG           (0x0C)






#endif
