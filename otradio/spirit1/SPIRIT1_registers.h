/* Copyright 2009-2013 JP Norair
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
  * @file       /otradio/spirit1/SPIRIT1_registers.h
  * @author     JP Norair
  * @version    R100
  * @date       4 Jan 2013
  * @brief      Register Addressing Constants for SPIRIT1 family
  * @ingroup    SPIRIT1
  *
  ******************************************************************************
  */

#ifndef __SPIRIT1_registers_H
#define __SPIRIT1_registers_H


/** Register call macros: mainly for nomenclature consistency with other TRX
  * drivers that have been implemented.
  */
#define RFREG(VAL)      (RF_##VAL)
#define STROBE(VAL)     (RFSTROBE_##VAL)
#define RFGPO(VAL)      ((RFGPO_##VAL << 3) | 2)



// Read/Write Registers

#define RF_ANA_FUNC_CONF1       0x00
#   define _GM_CONF             (7<<2)
#   define __GM_CONF(VAL)       ((VAL&7)<<2)
#   define _GM_CONF_13m2s       (0<<2)
#   define _GM_CONF_18m2s       (1<<2)
#   define _GM_CONF_21m5s       (2<<2)
#   define _GM_CONF_25m6s       (3<<2)
#   define _GM_CONF_28m8s       (4<<2)
#   define _GM_CONF_33m9s       (5<<2)
#   define _GM_CONF_38m5s       (6<<2)
#   define _GM_CONF_43m0s       (7<<2)
#   define _SET_BLD_LVL         0x3
#   define _SET_BLD_LVL_2V7     0
#   define _SET_BLD_LVL_2V5     1
#   define _SET_BLD_LVL_2V3     2
#   define _SET_BLD_LVL_2V1     3
  
#define RF_ANA_FUNC_CONF0       0x01
#   define _24_26MHz_SELECT     (1<<6)
#   define _AES_ON              (1<<5)
#   define _EXT_REF             (1<<4)
#   define _BROWN_OUT           (1<<2)
#   define _BATTERY_LEVEL       (1<<1)
#   define _TS                  (1<<0)

#define RF_GPIO3_CONF           0x02
#define RF_GPIO2_CONF           0x03
#define RF_GPIO1_CONF           0x04
#define RF_GPIO0_CONF           0x05
#   define _GPIO_SELECT(VAL)    ((0x1F&VAL)<<3)
#   define _GPIO_MODE(VAL)      (0x03&VAL)
#   define _GPIO_MODE_ANALOG    0                   //GPIO0 only
#   define _GPIO_MODE_INPUT     1
#   define _GPIO_MODE_OUTPUT    2
#   define _GPIO_MODE_HIDRIVE   3
  
#define RF_MCU_CK_CONF          0x06
#   define _EN_MCU_CLK          (1<<7)
#   define _EN_CLOCK_TAIL       (3<<5)
#   define _EN_CLOCK_TAIL_0     (0<<5)
#   define _EN_CLOCK_TAIL_64    (1<<5)
#   define _EN_CLOCK_TAIL_256   (2<<5)
#   define _EN_CLOCK_TAIL_512   (0<<5)
#   define _XO_RATIO(VAL)       ((0xf&VAL)<<1)
#   define _RCO_RATIO           (1<<0)
#   define _RCO_RATIO_1         (0<<0)
#   define _RCO_RATIO_1in128    (1<<0)

#define RF_XO_RCO_TEST          0xB4
#   define _PD_CLKDIV           (1<<3)      //set (disable) on 24-26 MHz systems
#   define _XO_RCO_TEST_RESERVED 0x21

#define RF_SYNTH_CONFIG0        0x9F
#   define _SEL_TSPLIT          (1<<7)
#   define _SEL_TSPLIT_1n75s    (0x40)
#   define _SEL_TSPLIT_3n47s    (0xA0)      //default

#define RF_SYNTH_CONFIG1        0x9E
#   define _REFDIV              (1<<7)
#   define _REFDIV_XO           (0<<7)
#   define _REFDIV_XO1in2       (1<<7)
#   define _VCO_L_SEL           (1<<2)
#   define _VCO_H_SEL           (1<<1)
#   define _VCO_169MHz_SEL      (0x5B)
#   define _VCO_315MHz_SEL      (0x5D)
#   define _VCO_433MHz_SEL      (0x5B)
#   define _VCO_866MHz_SEL      (0x5B)

#define RF_IF_OFFSET_ANA        0x07
#   define _IF_OFFSET_ANA(IF, XO)   (((IF*3*4096)/XO) - 64)
#   define _IF_OFFSET_ANA_24MHz 0xB6
#   define _IF_OFFSET_ANA_25MHz 0xAC
#   define _IF_OFFSET_ANA_26MHz 0xA3
#   define _IF_OFFSET_ANA_48MHz 0x3B
#   define _IF_OFFSET_ANA_50MHz 0x36
#   define _IF_OFFSET_ANA_52MHz 0x31 




// Settings for DASH7
// Official Fbase = 433.16064 
//
// | XO | Fbase            | SYNT (dec)    | SYNT (hex)
// +----+------------------+---------------+----------------
// | 24 | 433161210.9375   | 28387653.12   | 0x01B12945
// | 25 | 433161638.1836   | 27252173.88   | 0x019FD5CE
// | 26 | 433159074.7070   | 26203858.26   | 0x018FD6D2
// | 48 | 433161210.937    | 28387653.12   | 0x00D894A2
// | 50 | 433161638.1836   | 13626087.97   | 0x00CFEAE7
// | 52 | 433164628.90625  | 26204194.26   | 0x00C7EC11
#define _SYNT_D7_24MHz          0x01B12945
#define _SYNT_D7_25MHz          0x019FD5CE
#define _SYNT_D7_26MHz          0x018FD6D2
#define _SYNT_D7_48MHz          0x00D894A2 
#define _SYNT_D7_50MHz          0x00CFEAE7 
#define _SYNT_D7_52MHz          0x00C7EC11  
#define _DR_M_D7LS_24MHz        47          // 47.407
#define _DR_E_D7LS_24MHz        11
//#define _DR_M_D7LS_25MHz        35          // 35.27
//#define _DR_E_D7LS_25MHz        11
#define _DR_M_D7LS_25MHz        163         // 163.4
#define _DR_E_D7LS_25MHz        9
#define _DR_M_D7LS_26MHz        24          // 24.068
#define _DR_E_D7LS_26MHz        11
#define _DR_M_D7HS_24MHz        17          // 17.067
#define _DR_E_D7HS_24MHz        13
#define _DR_M_D7HS_25MHz        6           // 6.144
#define _DR_E_D7HS_25MHz        13
#define _DR_M_D7HS_26MHz        248         // 248.123
#define _DR_E_D7HS_26MHz        12


#define RF_SYNT3                0x08
#   define _WCP                 (7<<5)
#   define _WCP_169MHz          (3<<5)
#   define _WCP_315MHz          (6<<5)
#   define _WCP_433MHz          (0<<5)
#   define _WCP_866MHz          (0<<5)
#   define _SYNT_HI             (31<<0)
#   define _SYNT_HI_D7_24MHz    ((ot_u8)((_SYNT_D7_24MHz>>21)&_SYNT_HI))
#   define _SYNT_HI_D7_25MHz    ((ot_u8)((_SYNT_D7_25MHz>>21)&_SYNT_HI))
#   define _SYNT_HI_D7_26MHz    ((ot_u8)((_SYNT_D7_26MHz>>21)&_SYNT_HI))
#   define _SYNT_HI_D7_48MHz    ((ot_u8)((_SYNT_D7_48MHz>>21)&_SYNT_HI))
#   define _SYNT_HI_D7_50MHz    ((ot_u8)((_SYNT_D7_50MHz>>21)&_SYNT_HI))
#   define _SYNT_HI_D7_52MHz    ((ot_u8)((_SYNT_D7_52MHz>>21)&_SYNT_HI))

#define RF_SYNT2                0x09
#   define _SYNT2_D7_24MHz      ((ot_u8)((_SYNT_D7_24MHz>>13)&0xff))
#   define _SYNT2_D7_25MHz      ((ot_u8)((_SYNT_D7_25MHz>>13)&0xff))
#   define _SYNT2_D7_26MHz      ((ot_u8)((_SYNT_D7_26MHz>>13)&0xff))
#   define _SYNT2_D7_48MHz      ((ot_u8)((_SYNT_D7_48MHz>>13)&0xff))
#   define _SYNT2_D7_50MHz      ((ot_u8)((_SYNT_D7_50MHz>>13)&0xff))
#   define _SYNT2_D7_52MHz      ((ot_u8)((_SYNT_D7_52MHz>>13)&0xff))

#define RF_SYNT1                0x0A
#   define _SYNT1_D7_24MHz      ((ot_u8)((_SYNT_D7_24MHz>>5)&0xff))
#   define _SYNT1_D7_25MHz      ((ot_u8)((_SYNT_D7_25MHz>>5)&0xff))
#   define _SYNT1_D7_26MHz      ((ot_u8)((_SYNT_D7_26MHz>>5)&0xff))
#   define _SYNT1_D7_48MHz      ((ot_u8)((_SYNT_D7_48MHz>>5)&0xff))
#   define _SYNT1_D7_50MHz      ((ot_u8)((_SYNT_D7_50MHz>>5)&0xff))
#   define _SYNT1_D7_52MHz      ((ot_u8)((_SYNT_D7_52MHz>>5)&0xff))

#define RF_SYNT0                0x0B
#   define _SYNT_LO             (31<<3)
#   define _SYNT_LO_D7_24MHz    ((ot_u8)((_SYNT_D7_24MHz<<3)&_SYNT_LO))
#   define _SYNT_LO_D7_25MHz    ((ot_u8)((_SYNT_D7_25MHz<<3)&_SYNT_LO))
#   define _SYNT_LO_D7_26MHz    ((ot_u8)((_SYNT_D7_26MHz<<3)&_SYNT_LO))
#   define _SYNT_LO_D7_48MHz    ((ot_u8)((_SYNT_D7_48MHz<<3)&_SYNT_LO))
#   define _SYNT_LO_D7_50MHz    ((ot_u8)((_SYNT_D7_50MHz<<3)&_SYNT_LO))
#   define _SYNT_LO_D7_52MHz    ((ot_u8)((_SYNT_D7_52MHz<<3)&_SYNT_LO))
#   define _BS                  (7<<0)
#   define _BS_868MHz           (1<<0)
#   define _BS_660MHz           (2<<0)
#   define _BS_433MHz           (3<<0)
#   define _BS_315MHz           (4<<0)
#   define _BS_169MHz           (5<<0)

#define RF_CHSPACE              0x0C
#   define _CHSPACE(BW, XOHZ)   (ot_u8)( ((float)BW) / (((float)XOHZ)/32768))
#   define _CHSPACE_D7_24MHz    148
#   define _CHSPACE_D7_25MHz    142
#   define _CHSPACE_D7_26MHz    137
#   define _CHSPACE_D7_48MHz    74
#   define _CHSPACE_D7_50MHz    71
#   define _CHSPACE_D7_52MHz    68   

#define RF_IF_OFFSET_DIG        0x0D
#   define _IF_OFFSET_DIG(IF, CLK)  (((IF*3*4096)/CLK) - 64)
#   define _IF_OFFSET_DIG_24MHz 0xB6
#   define _IF_OFFSET_DIG_25MHz 0xAC
#   define _IF_OFFSET_DIG_26MHz 0xA3
#   define _IF_OFFSET_DIG_48MHz 0xB6
#   define _IF_OFFSET_DIG_50MHz 0xAC
#   define _IF_OFFSET_DIG_52MHz 0xA3

#define RF_FC_OFFSET1           0x0E
#define RF_FC_OFFSET0           0x0F

#define RF_PAPOWER8             0x10
#define RF_PAPOWER7             0x11
#define RF_PAPOWER6             0x12
#define RF_PAPOWER5             0x13
#define RF_PAPOWER4             0x14
#define RF_PAPOWER3             0x15
#define RF_PAPOWER2             0x16
#define RF_PAPOWER1             0x17
#define RF_PAPOWER0             0x18
#define RF_PAPOWER_CTL          RF_FC_PAPOWER0
#   define _CWC                 (3<<6)
#   define _CWC_0pF             (0<<6)
#   define _CWC_1p2F            (1<<6)
#   define _CWC_2p4F            (2<<6)
#   define _CWC_3p6F            (3<<6)
#   define _PA_RAMP_ENABLE      (1<<5)
#   define __PA_RAMP_STEP_WIDTH(VAL) ((3&VAL)<<3)
#   define __PA_LEVEL_MAX_INDEX(VAL) ((7&VAL)<<0)

#define RF_MOD1                 0x1A
#   define _DATARATE_M          (255)

#define RF_MOD0                 0x1B
#   define _CW                  (1<<7)
#   define _BT_SEL              (1<<6)
#   define _BT_SEL_1            (0<<6)
#   define _BT_SEL_05           (1<<6)
#   define _MOD_TYPE            (3<<4)
#   define _MOD_TYPE_2FSK       (0<<4)
#   define _MOD_TYPE_GFSK       (1<<4)
#   define _MOD_TYPE_ASK        (2<<4)
#   define _MOD_TYPE_MSK        (3<<4)
#   define _DATARATE_E          (15<<0)

#define RF_FDEV0                0x1C
#   define _FDEV_E              (15<<4)
#   define _FDEV_E_D7_24MHz     (7<<4)      // 46.875 kHz
#   define _FDEV_E_D7_25MHz     (7<<4)      // 48.828 kHz
#   define _FDEV_E_D7_26MHz     (7<<4)      // 50.781 kHz
#   define _FDEV_E_D7_48MHz     (6<<4)      // 46.875 kHz
#   define _FDEV_E_D7_50MHz     (6<<4)      // 48.828 kHz
#   define _FDEV_E_D7_52MHz     (6<<4)      // 50.781 kHz
#   define _CLOCK_REC_ALGO_SEL  (1<<3)
#   define _FDEV_M              (7<<0)
#   define _FDEV_M_D7_24MHz     0           // 46.875 kHz
#   define _FDEV_M_D7_25MHz     0           // 48.828 kHz
#   define _FDEV_M_D7_26MHz     0           // 50.781 kHz
#   define _FDEV_M_D7_48MHz     0           // 46.875 kHz 
#   define _FDEV_M_D7_50MHz     0           // 48.828 kHz 
#   define _FDEV_M_D7_52MHz     0           // 50.781 kHz

#define RF_CHFLT                0x1D
#   define _CHFLT_M             (15<<4)
#   define _CHFLT_M_D7LS_24MHz  (6<<4)      //150 kHz
#   define _CHFLT_M_D7LS_25MHz  (6<<4)      //155 kHz
#   define _CHFLT_M_D7LS_26MHz  (7<<4)      //147 kHz
#   define _CHFLT_M_D7LS_48MHz  (6<<4)      //150 kHz
#   define _CHFLT_M_D7LS_50MHz  (6<<4)      //155 kHz
#   define _CHFLT_M_D7LS_52MHz  (7<<4)      //147 kHz
#   define _CHFLT_M_D7HS_24MHz  (6<<4)      //300 kHz
#   define _CHFLT_M_D7HS_25MHz  (6<<4)      //310 kHz
#   define _CHFLT_M_D7HS_26MHz  (7<<4)      //294 kHz
#   define _CHFLT_M_D7HS_48MHz  (6<<4)      //300 kHz
#   define _CHFLT_M_D7HS_50MHz  (6<<4)      //310 kHz
#   define _CHFLT_M_D7HS_52MHz  (7<<4)      //294 kHz
#   define _CHFLT_E             (15<<0)
#   define _CHFLT_E_D7LS_24MHz  (2<<0)      //150 kHz
#   define _CHFLT_E_D7LS_25MHz  (2<<0)      //155 kHz
#   define _CHFLT_E_D7LS_26MHz  (2<<0)      //147 kHz
#   define _CHFLT_E_D7LS_48MHz  (2<<0)      //150 kHz
#   define _CHFLT_E_D7LS_50MHz  (2<<0)      //155 kHz
#   define _CHFLT_E_D7LS_52MHz  (2<<0)      //147 kHz
#   define _CHFLT_E_D7HS_24MHz  (1<<0)      //300 kHz
#   define _CHFLT_E_D7HS_25MHz  (1<<0)      //310 kHz
#   define _CHFLT_E_D7HS_26MHz  (1<<0)      //294 kHz
#   define _CHFLT_E_D7HS_48MHz  (1<<0)      //300 kHz
#   define _CHFLT_E_D7HS_50MHz  (1<<0)      //310 kHz
#   define _CHFLT_E_D7HS_52MHz  (1<<0)      //294 kHz

#define RF_AFC2                 0x1E
#   define _AFC_FREEZE_ON_SYNC  (1<<7)
#   define _AFC_ENABLED         (1<<6)
#   define _AFC_MODE            (1<<5)
#   define _AFC_MODE_CLOSE_ON_SLICER (0<<5)     //default
#   define _AFC_MODE_CLOSE_ON_STAGE2 (1<<5)
#   define _AFC_PD_LEAKAGE      (31<<0)
#   define __AFC_PD_LEAKAGE(VAL) (VAL&31)

#define RF_AFC1                 0x1F
#   define _AFC_FAST_PERIOD     (255)
#   define __AFC_FAST_PERIOD(VAL) (VAL)

#define RF_AFC0                 0x20
#   define _AFC_FAST_GAIN       (15<<4)
#   define __AFC_FAST_GAIN(VAL) ((VAL&15)<<4)
#   define _AFC_SLOW_GAIN       (15<<0)
#   define __AFC_SLOW_GAIN(VAL) (VAL&15)

#define RF_RSSI_FLT             0x21
#   define _RSSI_FLT            (15<<4)
#   define __RSSI_FLT(VAL)      ((VAL&15)<<4)
#   define _CS_MODE             (3<<2)
#   define _CS_MODE_STATIC      (0<<2)      // Good for DASH7 bscan & general purpose
#   define _CS_MODE_DYN6dB      (1<<2)      // Possible for DASH7 fscan with FEC
#   define _CS_MODE_DYN12dB     (2<<2)      // Possible for DASH7 fscan without FEC
#   define _CS_MODE_DYN18dB     (3<<2)
#   define _OOK_PEAK_DECAY      (3<<0)

#define RF_RSSI_TH              0x22
#   define _RSSI_TH             255
#   define __RSSI_TH(dB)        ((dB+130)*2)    //0.5 dB increments: 0=-130, 20=-120, etc.

#define RF_CLOCKREC             0x23
#   define _CLK_REC_P_GAIN      (7<<5)
#   define __CLK_REC_P_GAIN(VAL) ((VAL&7)<<5)
#   define _PSTFLT_LEN          (1<<4)
#   define _PSTFLT_LEN_8        (0<<4)
#   define _PSTFLT_LEN_16       (1<<4)
#   define _CLK_REC_I_GAIN      (15<<0)
#   define __CLK_REC_I_GAIN(VAL) (VAL&15)

#define RF_AGCCTRL2             0x24
#   define _MEAS_TIME           (15<<0)
#   define __MEAS_TIME(VAL)     (VAL&15)

#define RF_AGCCTRL1             0x25
#   define _THRESHOLD_HI        (15<<4)
#   define __THRESHOLD_HI(VAL)  ((VAL&15)<<4)
#   define _THRESHOLD_LO        (15<<0)
#   define __THRESHOLD_LO(VAL)  (VAL&15)

#define RF_AGCCTRL0             0x26
#   define _AGC_ENABLE          (1<<7)

#define RF_ANT_SELECT_CONF      0x27
#   define _CS_BLANKING         (1<<4)
#   define _AS_ENABLE           (1<<3)
#   define _AS_MEAS_TIME        (7<<0)
#   define __AS_MEAS_TIME(VAL)  (7&VAL)

#define RF_PCKTCTRL4            0x30
#   define _ADDRESS_LEN         (3<<3)
#   define _ADDRESS_LEN_NONE    (0<<3)
#   define _ADDRESS_LEN_BASIC   (1<<3)
#   define _ADDRESS_LEN_STACK   (2<<3)
#   define _CONTROL_LEN         (7<<0)
#   define __CONTROL_LEN(VAL)   (7&VAL)

#define RF_PCKTCTRL3            0x31
#   define _PCKT_FRMT           (3<<6)
#   define _PCKT_FRMT_BASIC     (0<<6)
#   define _PCKT_FRMT_WMBUS     (2<<6)
#   define _PCKT_FRMT_STACK     (3<<6)
#   define _RX_MODE             (3<<4)
#   define _RX_MODE_NORMAL      (0<<4)
#   define _RX_MODE_FIFO        (1<<4)
#   define _RX_MODE_GPIO        (2<<4)
#   define _LEN_WID             (15<<0)
#   define __LEN_WID(VAL)       (15&(VAL-1))

#define RF_PCKTCTRL2            0x32
#   define _PREAMBLE_LENGTH     (31<<3)
#   define __PREAMBLE_LENGTH(VAL) ((31&(VAL-1))<<3)
#   define _SYNC_LENGTH         (3<<1)
#   define _SYNC_LENGTH_1       (0<<1)
#   define _SYNC_LENGTH_2       (1<<1)
#   define _SYNC_LENGTH_3       (2<<1)
#   define _SYNC_LENGTH_4       (3<<1)
#   define _FIX_VAR_LEN         (1<<0)

#define RF_PCKTCTRL1            0x33
#   define _CRC_MODE            (7<<5)
#   define _CRC_MODE_NONE       (0<<5)
#   define _CRC_MODE_07         (1<<5)
#   define _CRC_MODE_8005       (2<<5)
#   define _CRC_MODE_1021       (3<<5)      //DASH7 variant, although not proven to work on this chip
#   define _CRC_MODE_864CBF     (4<<5)
#   define _WHIT_EN             (1<<4)
#   define _TXSOURCE            (3<<2)
#   define _TXSOURCE_NORMAL     (0<<2)
#   define _TXSOURCE_FIFO       (1<<2)
#   define _TXSOURCE_GPIO       (2<<2)
#   define _TXSOURCE_PN9        (3<<2)
#   define _FEC_EN              (1<<0)

#define RF_PCKTLEN1             0x34
#define RF_PCKTLEN0             0x35

#define RF_SYNC4                0x36
#define RF_SYNC3                0x37
#define RF_SYNC2                0x38
#define RF_SYNC1                0x39

#define RF_QI                   0x3A
#   define _SQI_TH              (3<<6)
#   define __SQI_TH(VAL)        ((VAL&3)<<6)
#   define _PQI_TH              (15<<2)
#   define __PQI_TH(VAL)        ((VAL&15)<<2)
#   define _SQI_EN              (1<<1)
#   define _PQI_EN              (1<<0)

#define RF_MBUS_PRMBL           0x3B
#define RF_MBUS_PSTMBL          0x3C
#define RF_MBUS_CTRL            0x3D
#   define _MBUS_SUBMODE        (7<<1)

#define RF_FIFO_CONFIG3         0x3E
#   define _RXAFTHR             (127<<1)

#define RF_FIFO_CONFIG2         0x3F
#   define _RXAETHR             (127<<1)

#define RF_FIFO_CONFIG1         0x40
#   define _TXAFTHR             (127<<1)

#define RF_FIFO_CONFIG0         0x41
#   define _TXAETHR             (127<<1)

#define RF_PCKT_FLT_GOALS12     0x42
#   define _CONTROL0_MASK       (255)

#define RF_PCKT_FLT_GOALS11     0x43
#   define _CONTROL1_MASK       (255)

#define RF_PCKT_FLT_GOALS10     0x44
#   define _CONTROL2_MASK       (255)

#define RF_PCKT_FLT_GOALS9      0x45
#   define _CONTROL3_MASK       (255)

#define RF_PCKT_FLT_GOALS8      0x46
#   define _CONTROL0_FIELD      (255)

#define RF_PCKT_FLT_GOALS7      0x47
#   define _CONTROL1_FIELD      (255)

#define RF_PCKT_FLT_GOALS6      0x48
#   define _CONTROL2_FIELD      (255)

#define RF_PCKT_FLT_GOALS5      0x49
#   define _CONTROL3_FIELD      (255)

#define RF_PCKT_FLT_GOALS4      0x4A
#   define _RX_SOURCE_MASK      (255)

#define RF_PCKT_FLT_GOALS3      0x4B
#   define _RX_SOURCE_ADDR      (255)

#define RF_PCKT_FLT_GOALS2      0x4C
#   define _BROADCAST           (255)

#define RF_PCKT_FLT_GOALS1      0x4D
#   define _MULTICAST           (255)

#define RF_PCKT_FLT_GOALS0      0x4E
#   define _TX_SOURCE_ADDR      (255)

#define RF_PCKT_FLT_OPTIONS     0x4F
#   define _RX_TIMEOUT_AND_OR_SELECT (1<<6)
#   define _CONTROL_FILTERING   (1<<5)
#   define _SOURCE_FILTERING    (1<<4)
#   define _DEST_VS_SOURCE_ADDR (1<<3)
#   define _DEST_VS_MULTICAST_ADDR (1<<2)
#   define _DEST_VS_BROADCAST_ADDR (1<<1)
#   define _CRC_CHECK           (1<<0)

#define RF_PROTOCOL2            0x50
#   define _CS_TIMEOUT_MASK     (1<<7)
#   define _SQI_TIMEOUT_MASK    (1<<6)
#   define _PQI_TIMEOUT_MASK    (1<<5)
#   define _TX_SEQ_NUM_RELOAD   (3<<3)
#   define __TX_SEQ_NUM_RELOAD(VAL) ((VAL&3)<<3)
#   define _RCO_CALIBRATION     (1<<2)
#   define _VCO_CALIBRATION     (1<<1)
#   define _LDC_MODE            (1<<0)

#define RF_PROTOCOL1            0x51
#   define _LDC_RELOAD_ON_SYNC  (1<<7)
#   define _PIGGYBACKING        (1<<6)
#   define _SEED_RELOAD         (1<<3)
#   define _CSMA_ON             (1<<2)
#   define _CSMA_PERS_ON        (1<<1)
#   define _AUTO_PCKT_FLT       (1<<0)

#define RF_PROTOCOL0            0x52
#   define _NMAX_RETX           (15<<4)
#   define __NMAX_RETX(VAL)     ((VAL&15)<<4)
#   define _NACK_TX             (1<<3)
#   define _AUTO_ACK            (1<<2)
#   define _PERS_RX             (1<<1)
#   define _PERS_TX             (1<<0)

#define RF_TIMERS5              0x53
#   define _RX_TIMEOUT_PRESCALER (255)

#define RF_TIMERS4              0x54
#   define _RX_TIMEOUT_COUNTER  (255)

#define RF_TIMERS3              0x55
#   define _LDC_PRESCALER       (255)

#define RF_TIMERS2              0x56
#   define _LDC_COUNTER         (255)

#define RF_TIMERS1              0x57
#   define _LDC_RELOAD_PRESCALER (255)

#define RF_TIMERS0              0x58
#   define _LCD_RELOAD_COUNTER  (255)

// CSMA engine does not appear to be sophisticated enough to do DASH7 CSMA, but
// the documentation available is limited as of 4 Jan 2013

#define RF_CSMA_CONFIG3         0x64
#   define _BU_COUNTER_SEED_HI  (255)

#define RF_CSMA_CONFIG2         0x65
#   define _BU_COUNTER_SEED_LO  (255)

#define RF_CSMA_CONFIG1         0x66
#   define _BU_PRESCALER        (63<<2)
#   define __BU_PRESCALER(VAL)  ((63&VAL)<<2)
#   define _CCA_PERIOD          (3<<0)
#   define _CCA_PERIOD_64bit    (0)
#   define _CCA_PERIOD_128bit   (1)
#   define _CCA_PERIOD_256bit   (2)
#   define _CCA_PERIOD_512bit   (3)

#define RF_CSMA_CONFIG0         0x67
#   define _CCA_LENGTH          (15<<4)
#   define __CCA_LENGTH(VAL)    ((15&VAL)<<4)
#   define _NBACKOFF_MAX        (7<<0)
#   define __NBACKOFF_MAX(VAL)  (VAL&7)

#define RF_TX_CTRL_FIELD3       0x68
#define RF_TX_CTRL_FIELD2       0x69
#define RF_TX_CTRL_FIELD1       0x6A
#define RF_TX_CTRL_FIELD0       0x6B

#define RF_CHNUM                0x6C

#define RF_VCO_CONFIG           0xA1
#   define _VCO_GEN_CURR        (63<<0)
#   define __VCO_GEN_CURR(VAL)  (63&VAL)

#define RF_RCO_VCO_CALIBR_IN2   0x6D
#   define _RWT_IN              (15<<4)
#   define __RWT_IN(VAL)        ((15&VAL)<<4)
#   define _RFB_IN_HI           (15<<0)
#   define __RFB_IN(VAL)        (15&VAL)

#define RF_RCO_VCO_CALIBR_IN1   0x6E
#   define _RFB_IN_LO           (1<<7)
#   define _VCO_CALIBR_TX       (127<<0)
#   define __VCO_CALIBR_TX(VAL) (127&VAL)

#define RF_RCO_VCO_CALIBR_IN0   0x6F
#   define _VCO_CALIBR_RX       (127<<0)
#   define __VCO_CALIBR_RX(VAL) (127&VAL)

#define RF_AES_KEY_IN15         0x70
#define RF_AES_KEY_IN14         0x71
#define RF_AES_KEY_IN13         0x72
#define RF_AES_KEY_IN12         0x73
#define RF_AES_KEY_IN11         0x74
#define RF_AES_KEY_IN10         0x75
#define RF_AES_KEY_IN9          0x76
#define RF_AES_KEY_IN8          0x77
#define RF_AES_KEY_IN7          0x78
#define RF_AES_KEY_IN6          0x79
#define RF_AES_KEY_IN5          0x7A
#define RF_AES_KEY_IN4          0x7B
#define RF_AES_KEY_IN3          0x7C
#define RF_AES_KEY_IN2          0x7D
#define RF_AES_KEY_IN1          0x7E
#define RF_AES_KEY_IN0          0x7F

#define RF_AES_DATA_IN15        0x80
#define RF_AES_DATA_IN14        0x81
#define RF_AES_DATA_IN13        0x82
#define RF_AES_DATA_IN12        0x83
#define RF_AES_DATA_IN11        0x84
#define RF_AES_DATA_IN10        0x85
#define RF_AES_DATA_IN9         0x86
#define RF_AES_DATA_IN8         0x87
#define RF_AES_DATA_IN7         0x88
#define RF_AES_DATA_IN6         0x89
#define RF_AES_DATA_IN5         0x8A
#define RF_AES_DATA_IN4         0x8B
#define RF_AES_DATA_IN3         0x8C
#define RF_AES_DATA_IN2         0x8D
#define RF_AES_DATA_IN1         0x8E
#define RF_AES_DATA_IN0         0x8F

#define RF_IRQ_MASK3            0x90
#define RF_IRQ_MASK2            0x91
#define RF_IRQ_MASK1            0x92
#define RF_IRQ_MASK0            0x93


#define RF_DEM_CONFIG           0xA3
#   define _DEM_RESERVED        0x35
#   define _DEM_ORDER           (1<<1)

#define RF_PM_CONFIG2           0xA4
#   define _EN_TS_BUFFER        (1<<6)

#define RF_PM_CONFIG1           0xA5
#   define _EN_RM               (1<<7)
#   define _KRM_HI              (127)
#   define __KRM_HI(VAL)        (VAL&127)

#define RF_PM_CONFIG0           0xA6
#   define _KRM_LO              (255)
#   define __KRM_LO(VAL)        (VAL&255)

#define RF_XO_RCO_CONFIG        0xA7
#   define _XO_RCO_RESERVED     (0xE1)
#   define _EXT_RCOSC           (1<<3)

#define RF_TEST_SELECT          0xA8

#define RF_PM_TEST              0xB2




/// Read only Registers

#define RF_MC_STATE1            0xC0
#   define _ANT_SELECT          (1<<3)
#   define _TX_FIFO_FULL        (1<<2)
#   define _RX_FIFO_EMPTY       (1<<1)
#   define _ERROR_LOCK          (1<<0)

#define RF_MC_STATE0            0xC1
#   define _STATE               (127<<1)
#   define _XO_ON               (1<<0)

#define RF_TX_PCKT_INFO         0xC2
#   define _TX_SEQ_NUM          (3<<4)
#   define _N_RETX              (0x0F)

#define RF_RX_PCKT_INFO         0xC3
#   define _NACK_RX             (1<<2)
#   define _RX_SEQ_NUM          (3<<0)

#define RF_AFC_CORR             0xC4

#define RF_LINK_QUALIF2         0xC5
#   define _PQI                 (0xFF)

#define RF_LINK_QUALIF1         0xC6
#   define _CS                  (1<<7)
#   define _SQI                 (0x7F)

#define RF_LINK_QUALIF0         0xC7
#   define _LQI                 (0xF0)
#   define _AGC_WORD            (0x0F)

#define RF_RSSI_LEVEL           0xC8

#define RF_RX_PCKT_LEN1         0xC9
#define RF_RX_PCKT_LEN0         0xCA

#define RF_CRC_FIELD2           0xCB
#define RF_CRC_FIELD1           0xCC
#define RF_CRC_FIELD0           0xCD

#define RF_RX_CTRL_FIELD3       0xCE
#define RF_RX_CTRL_FIELD2       0xCF
#define RF_RX_CTRL_FIELD1       0xD0
#define RF_RX_CTRL_FIELD0       0xD1

#define RF_RX_ADDR_FIELD1       0xD2
#define RF_RX_ADDR_FIELD0       0xD3

#define RF_AES_DATA_OUT15       0xD4
#define RF_AES_DATA_OUT14       0xD5
#define RF_AES_DATA_OUT13       0xD6
#define RF_AES_DATA_OUT12       0xD7
#define RF_AES_DATA_OUT11       0xD8
#define RF_AES_DATA_OUT10       0xD9
#define RF_AES_DATA_OUT9        0xDA
#define RF_AES_DATA_OUT8        0xDB
#define RF_AES_DATA_OUT7        0xDC
#define RF_AES_DATA_OUT6        0xDD
#define RF_AES_DATA_OUT5        0xDE
#define RF_AES_DATA_OUT4        0xDF
#define RF_AES_DATA_OUT3        0xE0
#define RF_AES_DATA_OUT2        0xE1
#define RF_AES_DATA_OUT1        0xE2
#define RF_AES_DATA_OUT0        0xE3

#define RF_RCO_VCO_CALIBR_OUT1  0xE4
#   define _RWT_OUT             (0xF0)
#   define _RFB_OUT_HI          (0x0F)

#define RF_RCO_VCO_CALIBR_OUT0  0xE5
#   define _RFB_OUT_LO
#   define _VCO_CALIBR_DATA     (0x7F)

#define RF_LINEAR_FIFO_STATUS1  0xE6
#   define _ELEM_TXFIFO         (0x7F)

#define RF_LINEAR_FIFO_STATUS0  0xE7
#   define _ELEM_RXFIFO         (0x7F)

#define RF_DEVICE_INFO1         0xF0
#define RF_DEVICE_INFO0         0xF1


// Read and Reset Registers
#define RF_IRQ_STATUS3          0xFA
#define RF_IRQ_STATUS2          0xFB
#define RF_IRQ_STATUS1          0xFC
#define RF_IRQ_STATUS0          0xFD


//FIFO
#define RF_FIFO                 0xFF



// IRQs
#define RFINT_RX_DATA_READY         ((ot_u32)1<<0)
#define RFINT_RX_DATA_DISCARDED     ((ot_u32)1<<1)
#define RFINT_TX_DATA_SENT          ((ot_u32)1<<2)
#define RFINT_TX_MAX_RETX           ((ot_u32)1<<3)
#define RFINT_CRC_ERROR             ((ot_u32)1<<4)
#define RFINT_TX_FIFO_ERROR         ((ot_u32)1<<5)
#define RFINT_RX_FIFO_ERROR         ((ot_u32)1<<6)
#define RFINT_TX_FIFO_ALMOST_FULL   ((ot_u32)1<<7)
#define RFINT_TX_FIFO_ALMOST_EMPTY  ((ot_u32)1<<8)
#define RFINT_RX_FIFO_ALMOST_FULL   ((ot_u32)1<<9)
#define RFINT_RX_FIFO_ALMOST_EMPTY  ((ot_u32)1<<10)
#define RFINT_CSMA_EXPIRED          ((ot_u32)1<<11)
#define RFINT_VALID_PREAMBLE        ((ot_u32)1<<12)
#define RFINT_SYNC_WORD             ((ot_u32)1<<13)
#define RFINT_RSSI_ABOVE_THR        ((ot_u32)1<<14)
#define RFINT_LDCR_WAKEUP_TIMEOUT   ((ot_u32)1<<15)
#define RFINT_READY                 ((ot_u32)1<<16)
#define RFINT_ENTERING_STANDBY      ((ot_u32)1<<17)
#define RFINT_LOW_BATTERY           ((ot_u32)1<<18)
#define RFINT_POWER_ON_RESET        ((ot_u32)1<<19)
#define RFINT_BROWNOUT_EVENT        ((ot_u32)1<<20)
#define RFINT_LOCK                  ((ot_u32)1<<21)
//22-28 RFU
#define RFINT_RX_OPERATION_TIMEOUT  ((ot_u32)1<<29)
#define RFINT_AES_COMPLETE          ((ot_u32)1<<30)

// GPOs
#define RFGPO_nIRQ                  0
#define RFGPO_nPOR                  1
#define RFGPO_WAKEUP_TIMER_EXPIRED  2
#define RFGPO_LOW_BATTERY           3
#define RFGPO_TX_DATA_CLK           4
#define RFGPO_TX_INDICATOR          5
#define RFGPO_TX_FIFO_ALMOST_EMPTY  6
#define RFGPO_TX_FIFO_ALMOST_FULL   7
#define RFGPO_RX_DATA_OUT           8
#define RFGPO_RX_DATA_CLK           9
#define RFGPO_RX_STATE_INDICATOR    10
#define RFGPO_RX_FIFO_ALMOST_FULL   11
#define RFGPO_RX_FIFO_ALMOST_EMPTY  12
#define RFGPO_ANTENNA_SWITCH        13
#define RFGPO_VALID_PREAMBLE        14
#define RFGPO_SYNC_WORD             15
#define RFGPO_RSSI_ABOVE_THR        16
#define RFGPO_MCU_CLK               17
#define RFGPO_TRX_INDICATOR         18
#define RFGPO_VDD                   19
#define RFGPO_GND                   20
#define RFGPO_EXTERNAL_SMPS_ON      21
#define RFGPO_SLEEP_OR_STANDBY      22
#define RFGPO_READY                 23
#define RFGPO_LOCK                  24
#define RFGPO_WAITING_FOR_LOCK      25
#define RFGPO_WAITING_FOR_STABLE    26
#define RFGPO_WAITING_FOR_READY     27
#define RFGPO_WAITING_FOR_PM        28
#define RFGPO_WAITING_FOR_VCO       29
#define RFGPO_SYNTH_INDICATOR       30
#define RFGPO_WAITING_FOR_RCO       31

// GPIs
#define RFGPI_TX_COMMAND            0
#define RFGPI_RX_COMMAND            1
#define RFGPI_TX_DATA               2
#define RFGPI_WAKEUP                3
#define RFGPI_EXTCLOCK              4





/** SPI Command Values (Strobes) <BR>
  * ========================================================================<BR>
  * The term "strobe" is used, in order to share a common vocabulary with the
  * comparable TI-Chipcon units.
  */
#define RFSTROBE_TX                 0x60
#define RFSTROBE_RX                 0x61
#define RFSTROBE_READY              0x62
#define RFSTROBE_STANDBY            0x63
#define RFSTROBE_SLEEP              0x64
#define RFSTROBE_LOCKRX             0x65
#define RFSTROBE_LOCKTX             0x66
#define RFSTROBE_SABORT             0x67
#define RFSTROBE_LDC_RELOAD         0x68
#define RFSTROBE_SEQUENCE_UPDATE    0x69
#define RFSTROBE_AES_ENC            0x6A
#define RFSTROBE_AES_KEY            0x6B
#define RFSTROBE_AES_DEC            0x6C
#define RFSTROBE_AES_KEYDEC         0x6D
#define RFSTROBE_SRES               0x70
#define RFSTROBE_FLUSHRXFIFO        0x71
#define RFSTROBE_FLUSHTXFIFO        0x72



#endif
