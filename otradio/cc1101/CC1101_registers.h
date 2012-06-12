/* Copyright 2009-2011 JP Norair
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
  * @file       /OTradio/CC1101/CC1101_registers.h
  * @author     JP Norair
  * @version    V1.0
  * @date       1 April 2012
  * @brief      Register Addressing Constants for CC1101 family
  * @ingroup    CC1101
  *
  ******************************************************************************
  */


/** Register call macros: mainly for nomenclature consistency with other TRX
  * drivers that have been implemented.
  */
#define RFREG(VAL)      (RF_##VAL)
#define STROBE(VAL)     (RFSTROBE_##VAL)


/** Register addresses (00 - 3F) and bitfield components
  */
#define RF_IOCFG2               0x00
#define RF_IOCFG1               0x01
#define RF_IOCFG0               0x02

#define RF_FIFOTHR              0x03
#   define _ADC_RETENTION       0x40
#   define _CLOSE_IN_RX         0x30
#   define _CLOSE_IN_RX_0dB     0x00
#   define _CLOSE_IN_RX_6dB     0x10
#   define _CLOSE_IN_RX_12dB    0x20
#   define _CLOSE_IN_RX_18db    0x30
#   define _FIFO_TXTHR(VAL)     ((((VAL&0x0F)-1)>>2) ^ 0x0F)
#   define _FIFO_RXTHR(VAL)     (((VAL&0x0F)-1)>>2)

#define RF_SYNC1                0x04
#define RF_SYNC0                0x05

#define RF_PKTLEN               0x06

#define RF_PKTCTRL1             0x07
#   define _PQT(VAL)            ((VAL&7)<<5)
#   define _CRC_AUTOFLUSH       0x08
#   define _APPEND_STATUS       0x04
#   define _ADR_CHK_NONE        0x00
#   define _ADR_CHK_ON          0x01
#   define _ADR_CHK_ON_00       0x02
#   define _ADR_CHK_ON_FF       0x03

#define RF_PKTCTRL0             0x08
#   define _WHITE_DATA          0x40
#   define _PKT_FORMAT_NORMAL   0x00
#   define _PKT_FORMAT_SSERIAL  0x10
#   define _PKT_FORMAT_RANDOM   0x20
#   define _PKT_FORMAT_ASERIAL  0x30
#   define _PKT_CRC_EN          0x04
#   define _PKT_LENCFG_FIXED    0x00
#   define _PKT_LENCFG_VARIABLE 0x01
#   define _PKT_LENCFG_INFINITE 0x02

#define RF_ADDR                 0x09

#define RF_CHANNR               0x0A

#define RF_FSCTRL1              0x0B
#   define _FREQ_IF(VAL)        (VAL)

#define RF_FSCTRL0              0x0C
#   define _FREQOFF(VAL)        (VAL)

#define RF_FREQ2                0x0D
#   define _FREQHI(VAL)         (VAL)

#define RF_FREQ1                0x0E
#   define _FREQMID(VAL)        (VAL)

#define RF_FREQ0                0x0F
#   define _FREQLO(VAL)         (VAL)

#define RF_MDMCFG4              0x10
#   define _CHANBW_E(VAL)       ((VAL&3)<<6)
#   define _CHANBW_M(VAL)       ((VAL&3)<<4)
#   define _DRATE_E(VAL)        (VAL&15)

#define RF_MDMCFG3              0x11
#   define _DRATE_M(VAL)        (VAL)

#define RF_MDMCFG2              0x12
#   define _DEM_DCFILT_OFF      0x80
#   define _DEM_DCFILT_ON       0x00
#   define _MOD_FORMAT_2FSK     (0<<4)
#   define _MOD_FORMAT_GFSK     (1<<4)
#   define _MOD_FORMAT_ASK      (3<<4)
#   define _MOD_FORMAT_MSK      (7<<4)
#   define _MANCHESTER_EN       0x08
#   define _SYNC_MODE_NONE      0x00
#   define _SYNC_MODE_15in16    0x01
#   define _SYNC_MODE_16in16    0x02
#   define _SYNC_MODE_30in32    0x03
#   define _SYNC_MODE_CSONLY    0x04
#   define _SYNC_MODE_15in16CS  0x05
#   define _SYNC_MODE_16in16CS  0x06
#   define _SYNC_MODE_30in32CS  0x07

#define RF_MDMCFG1              0x13
#   define _FEC_EN              0x80
#   define _NUM_PREAMBLE_2B     (0<<4)
#   define _NUM_PREAMBLE_3B     (1<<4)
#   define _NUM_PREAMBLE_4B     (2<<4)
#   define _NUM_PREAMBLE_6B     (3<<4)
#   define _NUM_PREAMBLE_8B     (4<<4)
#   define _NUM_PREAMBLE_12B    (5<<4)
#   define _NUM_PREAMBLE_16B    (6<<4)
#   define _NUM_PREAMBLE_24B    (7<<4)
#   define _CHANSPC_E(VAL)      (VAL&3)

#define RF_MDMCFG0              0x14
#   define _CHANSPC_M(VAL)      (VAL)

#define RF_DEVIATN              0x15
#   define _DEVIATION_E(VAL)    ((VAL&7)<<4)
#   define _DEVIATION_M(VAL)    (VAL&7)

#define RF_MCSM2                0x16
#   define _RX_TIME_RSSI        0x10
#   define _RX_TIME_QUAL        0x08
#   define _RX_TIME(VAL)        (VAL)

#define RF_MCSM1                0x17
#   define _CCA_MODE_ALWAYS     (0<<4)
#   define _CCA_MODE_RSSILOW    (1<<4)
#   define _CCA_MODE_UNLESSRX   (2<<4)
#   define _CCA_MODE_RSSILOWRX  (3<<4)
#   define _RXOFF_MODE_IDLE     (0<<2)
#   define _RXOFF_MODE_FSTXON   (1<<2)
#   define _RXOFF_MODE_TX       (2<<2)
#   define _RXOFF_MODE_RX       (3<<2)
#   define _TXOFF_MODE_IDLE     (0)
#   define _TXOFF_MODE_FSTXON   (1)
#   define _TXOFF_MODE_TX       (2)
#   define _TXOFF_MODE_RX       (3)

#define RF_MCSM0                0x18
#   define _FS_AUTOCAL_NEVER    (0<<4)
#   define _FS_AUTOCAL_FROMIDLE (1<<4)
#   define _FS_AUTOCAL_TOIDLE   (2<<4)
#   define _FS_AUTOCAL_4THIDLE  (3<<4)
#   define _PO_TIMEOUT_3us      (0<<2)
#   define _PO_TIMEOUT_39us     (1<<2)
#   define _PO_TIMEOUT_155us    (2<<2)
#   define _PO_TIMEOUT_620us    (3<<2)
#   define _PIN_CTRL_EN         0x02
#   define _XOSC_FORCE_ON       0x01

#define RF_FOCCFG               0x19
#   define _FOC_BS_CS_GATE      (1<<5)
#   define _FOC_PRE_K_1K        (0<<3)
#   define _FOC_PRE_K_2K        (1<<3)
#   define _FOC_PRE_K_3K        (2<<3)
#   define _FOC_PRE_K_4K        (3<<3)
#   define _FOC_POST_K_EQ       (0<<2)
#   define _FOC_POST_K_HALFK    (1<<2)
#   define _FOC_LIMIT_0         (0)
#   define _FOC_LIMIT_8THBW     (1)
#   define _FOC_LIMIT_4THBW     (2)
#   define _FOC_LIMIT_HALFBW    (3)

#define RF_BSCFG                0x1A
#   define _BS_PRE_KI_1KI       (0<<6)
#   define _BS_PRE_KI_2KI       (1<<6)
#   define _BS_PRE_KI_3KI       (2<<6)
#   define _BS_PRE_KI_4KI       (3<<6)
#   define _BS_PRE_KP_1KP       (0<<4)
#   define _BS_PRE_KP_2KP       (1<<4)
#   define _BS_PRE_KP_3KP       (2<<4)
#   define _BS_PRE_KP_4KP       (3<<4)
#   define _BS_POST_KI_EQ       (0<<3)
#   define _BS_POST_KI_1KP      (1<<3)
#   define _BS_POST_KP_EQ       (0<<2)
#   define _BS_POST_KP_1KP      (1<<2)
#   define _BS_LIMIT_0          (0)
#   define _BS_LIMIT_32NDOFFSET (1)
#   define _BS_LIMIT_16THOFFSET (2)
#   define _BS_LIMIT_8THOFFSET  (3)

#define RF_AGCCTRL2             0x1B
#   define _MAX_DVGA_GAIN_ALL   (0<<6)
#   define _MAX_DVGA_GAIN_SUB1  (1<<6)
#   define _MAX_DVGA_GAIN_SUB2  (2<<6)
#   define _MAX_DVGA_GAIN_SUB3  (3<<6)
#   define _MAX_LNA_GAIN_SUB0   (0<<3)
#   define _MAX_LNA_GAIN_SUB26  (1<<3)
#   define _MAX_LNA_GAIN_SUB61  (2<<3)
#   define _MAX_LNA_GAIN_SUB74  (3<<3)
#   define _MAX_LNA_GAIN_SUB92  (4<<3)
#   define _MAX_LNA_GAIN_SUB115 (5<<3)
#   define _MAX_LNA_GAIN_SUB146 (6<<3)
#   define _MAX_LNA_GAIN_SUB171 (7<<3)
#   define _MAX_MAGN_TARGET_24  (0)
#   define _MAX_MAGN_TARGET_27  (1)
#   define _MAX_MAGN_TARGET_30  (2)
#   define _MAX_MAGN_TARGET_33  (3)
#   define _MAX_MAGN_TARGET_36  (4)
#   define _MAX_MAGN_TARGET_38  (5)
#   define _MAX_MAGN_TARGET_40  (6)
#   define _MAX_MAGN_TARGET_42  (7)

#define RF_AGCCTRL1             0x1C
#   define _AGC_LNA_PRIORITY    (1<<6)
#   define _CS_REL_THR_DISABLED (0<<4)
#   define _CS_REL_THR_PLUS6    (1<<4)
#   define _CS_REL_THR_PLUS10   (2<<4)
#   define _CS_REL_THR_PLUS14   (3<<4)
#   define _CS_ABS_THR_FLAT     (0)
#   define _CS_ABS_THR_PLUS1    (1)
#   define _CS_ABS_THR_PLUS2    (2)
#   define _CS_ABS_THR_PLUS3    (3)
#   define _CS_ABS_THR_PLUS4    (4)
#   define _CS_ABS_THR_PLUS5    (5)
#   define _CS_ABS_THR_PLUS6    (6)
#   define _CS_ABS_THR_PLUS7    (7)
#   define _CS_ABS_THR_SUB8     (8)
#   define _CS_ABS_THR_SUB7     (9)
#   define _CS_ABS_THR_SUB6     (10)
#   define _CS_ABS_THR_SUB5     (11)
#   define _CS_ABS_THR_SUB4     (12)
#   define _CS_ABS_THR_SUB3     (13)
#   define _CS_ABS_THR_SUB2     (14)
#   define _CS_ABS_THR_SUB1     (15)

#define RF_AGCCTRL0             0x1D
#   define _HYST_LEVEL_NONE     (0<<6)
#   define _HYST_LEVEL_LOW      (1<<6)
#   define _HYST_LEVEL_MED      (2<<6)
#   define _HYST_LEVEL_HIGH     (3<<6)
#   define _WAIT_ITME_8         (0<<4)
#   define _WAIT_ITME_16        (1<<4)
#   define _WAIT_ITME_24        (2<<4)
#   define _WAIT_ITME_32        (3<<4)
#   define _AGC_FREEZE_NORMAL   (0<<2)
#   define _AGC_FREEZE_ONSYNC   (1<<2)
#   define _AGC_FREEZE_AN       (2<<2)
#   define _AGC_FREEZE_ANDIG    (3<<2)
#   define _FILTER_LENGTH_8     (0)
#   define _FILTER_LENGTH_16    (1)
#   define _FILTER_LENGTH_32    (2)
#   define _FILTER_LENGTH_64    (3)

#define RF_WOREVT1              0x1E
#   define _EVENT0_HI(VAL)      (VAL)

#define RF_WOREVT0              0x1F
#   define _EVENT0_LO(VAL)      (VAL)

#define RF_WORCTRL              0x20
#   define _RC_PD               (1<<7)
#   define _EVENT1_TIMEOUT4     (0<<4)
#   define _EVENT1_TIMEOUT6     (1<<4)
#   define _EVENT1_TIMEOUT8     (2<<4)
#   define _EVENT1_TIMEOUT12    (3<<4)
#   define _EVENT1_TIMEOUT16    (4<<4)
#   define _EVENT1_TIMEOUT24    (5<<4)
#   define _EVENT1_TIMEOUT32    (6<<4)
#   define _EVENT1_TIMEOUT48    (7<<4)
#   define _WOR_RES_29us        (0)
#   define _WOR_RES_920us       (1)
#   define _WOR_RES_30ms        (2)
#   define _WOR_RES_940ms       (3)

#define RF_FREND1               0x21
#   define _LNA_CURRENT(VAL)            ((VAL&3)<<6)
#   define _LNA2MIX_CURRENT(VAL)        ((VAL&3)<<4)
#   define _LODIV_BUF_CURRENT_RX(VAL)   ((VAL&3)<<2)
#   define _MIX_CURRENT(VAL)            (VAL&3)

#define RF_FREND0               0x22
#   define _LODIV_BUF_CURRENT_TX(VAL)   ((VAL&3)<<4)
#   define _PA_POWER(VAL)               (VAL&7)

#define RF_FSCAL3               0x23
#   define _FSCAL3_HI(VAL)          ((VAL&3)<<6)
#   define _CHP_CURR_CAL_EN(VAL)    ((VAL&3)<<4)
#   define _FSCAL3_LO(VAL)          (VAL&15)

#define RF_FSCAL2               0x24
#   define _VCO_CORE_H_EN       (1<<5)
#   define _FSCAL2(VAL)         (VAL&31)

#define RF_FSCAL1               0x25
#   define _FSCAL1(VAL)         (VAL&31)

#define RF_FSCAL0               0x26
#   define _FSCAL0(VAL)         (VAL&127)

#define RF_RCCTRL1              0x27
#   define _RCCTRL1(VAL)        (VAL&127)

#define RF_RCCTRL0              0x28
#   define _RCCTRL0(VAL)        (VAL&127)

#define RF_FSTEST               0x29
#   define _FSTEST(VAL)         (VAL)

#define RF_PTEST                0x2A
#   define _PTEST(VAL)          (VAL)

#define RF_AGCTEST              0x2B
#   define _AGCTEST(VAL)        (VAL)

#define RF_TEST2                0x2C
#   define _TEST2(VAL)          (VAL)

#define RF_TEST1                0x2D
#   define _TEST1(VAL)          (VAL)

#define RF_TEST0                0x2E
#   define _TEST0_HI(VAL)       ((VAL&63)<<2)
#   define _VCO_SEL_CAL_EN      (1<<1)
#   define _TEST0_LO_EN         (1)


#define RF_PARTNUM              0xF0    //Burst read only
#define RF_VERSION              0xF1    //Burst read only
#define RF_FREQEST              0xF2    //Burst read only
#define RF_LQI                  0xF3    //Burst read only
#define RF_RSSI                 0xF4    //Burst read only
#define RF_MARCSTATE            0xF5    //Burst read only
#define RF_WORTIME1             0xF6    //Burst read only
#define RF_WORTIME0             0xF7    //Burst read only
#define RF_PKTSTATUS            0xF8    //Burst read only
#define RF_VCO_VC_DAC           0xF9    //Burst read only
#define RF_TXBYTES              0xFA    //Burst read only
#define RF_RXBYTES              0xFB    //Burst read only
#define RF_RCCTRL1_STATUS       0xFC    //Burst read only
#define RF_RCCTRL0_STATUS       0xFD    //Burst read only
#define RF_PATABLE              0x3E
#define RF_RXFIFO               0x3F    //read only
#define RF_TXFIFO               0x3F    //write only

#define RFSTROBE_SRES           0x30    //single r/w only
#define RFSTROBE_SFSTXON        0x31    //single r/w only
#define RFSTROBE_SXOFF          0x32    //single r/w only
#define RFSTROBE_SCAL           0x33    //single r/w only
#define RFSTROBE_SRX            0x34    //single r/w only
#define RFSTROBE_STX            0x35    //single r/w only
#define RFSTROBE_SIDLE          0x36    //single r/w only
#define RFSTROBE_SWOR           0x38    //single r/w only
#define RFSTROBE_SPWD           0x39    //single r/w only
#define RFSTROBE_SFRX           0x3A    //single r/w only
#define RFSTROBE_SFTX           0x3B    //single r/w only
#define RFSTROBE_SWORRST        0x3C    //single r/w only
#define RFSTROBE_SNOP           0x3D    //single r/w only




#define GDO_RXFHIGH             0x00
#define GDO_RXFHIGH_PKTDONE     0x01
#define GDO_TXFHIGH             0x02
#define GDO_TXFFULL             0x03
#define GDO_RXFOVER             0x04
#define GDO_TXFUNDER            0x05
#define GDO_SYNCDETECT          0x06
#define GDO_PKT_CRCOK           0x07
#define GDO_PQT_REACHED         0x08
#define GDO_CCAOK               0x09
#define GDO_PLL_LOCK            0x0A
#define GDO_SCLK                0x0B
#define GDO_SSDO                0x0C
#define GDO_ASDO                0x0D
#define GDO_CSOK                0x0E
#define GDO_CRCOK               0x0F
#define GDO_RX_HARD_DATA1       0x16
#define GDO_RX_HARD_DATA0       0x17
#define GDO_PA_PD               0x1B
#define GDO_LNA_PD              0x1C
#define GDO_RX_SYMBOL_TICK      0x1D
#define GDO_WOR_EVNT0           0x24
#define GDO_WOR_EVNT1           0x25
#define GDO_CLK_32k             0x27
#define GDO_CHIP_RDYn           0x29
#define GDO_XOSC_STABLE         0x2B
#define GDO_GDO0_Z_EN_N         0x2D
#define GDO_HIZ                 0x2E
#define GDO_HW0                 0x2F
#define GDO_CLK_XOSC_D_1        0x30
#define GDO_CLK_XOSC_D_1_5      0x31
#define GDO_CLK_XOSC_D_2        0x32
#define GDO_CLK_XOSC_D_3        0x33
#define GDO_CLK_XOSC_D_4        0x34
#define GDO_CLK_XOSC_D_6        0x35
#define GDO_CLK_XOSC_D_8        0x36
#define GDO_CLK_XOSC_D_12       0x37
#define GDO_CLK_XOSC_D_16       0x38
#define GDO_CLK_XOSC_D_24       0x39
#define GDO_CLK_XOSC_D_32       0x3A
#define GDO_CLK_XOSC_D_48       0x3B
#define GDO_CLK_XOSC_D_64       0x3C
#define GDO_CLK_XOSC_D_96       0x3D
#define GDO_CLK_XOSC_D_128      0x3E
#define GDO_CLK_XOSC_D_192      0x3F


