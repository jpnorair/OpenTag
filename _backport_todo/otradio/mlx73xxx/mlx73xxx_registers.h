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
  * @file       /Platforms/MLX73290/mlx73xxx_registers.h
  * @author     JP Norair
  * @version    V1.0
  * @date       26 November 2011
  * @brief      Register Addressing Constants for MLX73xxx family
  * @ingroup    MLX73xxx
  *
  ******************************************************************************
  */






/** Register call macros that automatically shift the address 1 bit, as needed
  * by the MLX73xxx.  The lowest bit is the R/W control bit: 1/0 = R/W.  You
  * must set (or clear) it in your own SPI interfacing code.
  */
#define RFREG(VAL)      (RF_##VAL << 1)
#define LFREG(VAL)      (LF_##VAL << 1)
#define B0REG(VAL)      (B0_##VAL << 1)
#define B1REG(VAL)      (B1_##VAL << 1)
#define B2REG(VAL)      (B2_##VAL << 1)
#define B3REG(VAL)      (B3_##VAL << 1)


#ifndef RF_CSTHR_OFFSET
#   define RF_CSTHR_OFFSET 0
#endif

#ifndef RADIO_PKT_OVERHEAD
#   define RADIO_PKT_OVERHEAD 2
#endif




/** Unbanked Registers for RF Configuration
  */
#define RF_TMRHDR_L         0x02
#define RF_TMRHDR_H         0x03
#define RF_RSSI             0x04
#define RF_RSSIHDR          0x05
#define RF_AFC_L            0x06
#define RF_AFC_H            0x07
#define RF_IQCORR_GAIN      0x08
#define RF_IQCORR_PHASE     0x09
#define RF_DROFFSET         0x0A

#define RF_LNACFG           0x0B
#   define DIS_S1_12DB          (1 << 7)
#   define S2_ATTEN             (3 << 5)
#   define S2_18DB              (0 << 5)
#   define S2_12DB              (1 << 5)
#   define S2_6DB               (2 << 5)
#   define S2_0DB               (3 << 5)
#   define PGA_GAIN             (31 << 0)
#   define SET_PGA_GAIN(dB)     ((dB/2) & 31)

#define RF_FILTER_GAIN      0x0C
#   define FILTER_GAIN_E    (7 << 4)
#   define FILTER_GAIN_M    (7 << 0)

#define RF_RXDSPCFG         0x0D
#   define EN_CORRECT_ISI       (1 << 7) 
#   define EN_FINE_RECOV        (1 << 6) 
#   define EN_ROUGH_RECOV       (1 << 5) 
#   define EN_FAST_PRE_CR       (1 << 4) 
#   define EN_DR_OFFS_ADJUST    (1 << 3) 
#   define AFC_MODE             (3 << 1)
#   define AFC_MODE_NONE        (0 << 1)
#   define AFC_MODE_FEIOK       (1 << 1)
#   define AFC_MODE_HDREND      (2 << 1)
#   define AFC_MODE_PKTEND      (3 << 1)
#   define EN_CORRECT_CFREQ     (1 << 0)

#define RF_RXPWR1           0x10
#define RF_RXPWR2           0x11
#define RF_TXPWR1           0x12
#define RF_TXPWR2           0x13
#define RF_PATTERN0         0x14
#define RF_PATTERN1         0x15
#define RF_PATTERN2         0x16
#define RF_PATTERN3         0x17

#define RF_PKTCFG0          0x18
#   define EN_D7M1              (1 << 7)
#   define D7M1_TAG_NINT        (1 << 6)
#   define EN_MULTI_FRAME       (1 << 5)
#   define EN_DESERIAL          (1 << 4)
#   define PATTERN_WORD_LEN     (3 << 2)
#   define PATTERN_WORD_LEN_16  (0 << 2)
#   define PATTERN_WORD_LEN_24  (1 << 2)
#   define PATTERN_WORD_LEN_32  (2 << 2)
#   define PATTERN_WORD_LEN_NA  (3 << 2)
#   define PATTERN_MAX_ERR      (3 << 0)
#   define PATTERN_MAX_ERR_0    (0 << 0)
#   define PATTERN_MAX_ERR_1    (1 << 0)
#   define PATTERN_MAX_ERR_2    (2 << 0)
#   define PATTERN_MAX_ERR_3    (3 << 0)

#define RF_PKTCFG1          0x19
#   define FIFO_SIZE            (1 << 7)
#   define FIFO_SIZE_1x256      (0 << 7)
#   define FIFO_SIZE_2x128      (1 << 7)
#   define PACKET_LEN_FIXED     (1 << 6)
#   define PACKET_LEN_VAR       (0 << 6)
#   define PACKET_LEN_POS       (3 << 4)
#   define PACKET_LEN_POS_0     (0 << 4)
#   define PACKET_LEN_POS_1     (1 << 4) 
#   define PACKET_LEN_POS_2     (2 << 4)
#   define PACKET_LEN_POS_3     (3 << 4)
#   define PACKET_LEN_CORR      (15 << 0)
#   define SET_PACKET_LEN_CORR(VAL) (VAL & 0xF)         // range: -8 <= VAL <= 7

#define RF_PKTLEN           0x1A
#define RF_ADDRESS          0x1B

#define RF_FREQ0            0x1C
#   define CENTER_FREQ_0        (7 << 5)
#   define SET_CENTER_FREQ_0(VAL) ((VAL & 7) << 5)   

#define RF_FREQ1            0x1D
#   define CENTER_FREQ_1        (255 << 0)
#   define SET_CENTER_FREQ_1(VAL) (VAL) 

#define RF_FREQ2            0x1E
#   define CENTER_FREQ_2        (255 << 0)
#   define SET_CENTER_FREQ_2(VAL) (VAL)

#define RF_FREQ3            0x1F
#   define CENTER_FREQ_3        (63 << 0)
#   define SET_CENTER_FREQ_3(VAL) (VAL & 63) 

#define RF_MDMCFG0          0x20
#   define EN_PWRDET2       (1 << 5)    // Not all devices have power-detector 2
#   define EN_PWRDET1       (1 << 4)
#   define EN_FREND2        (1 << 3)    // Not all devices have Front-end 2, but on EVK it is 433
#   define EN_FREND1        (1 << 2)
#   define BAND_SEL         (3 << 0)
#   define BAND_315         (0 << 0)
#   define BAND_433         (1 << 0)
#   define BAND_610         (2 << 0)
#   define BAND_900         (3 << 0)

#define RF_MDMCFG1          0x21
#   define DR_LIMIT             (3 << 6)
#   define DR_LIMIT_0           (0 << 6)
#   define DR_LIMIT_3           (1 << 6)
#   define DR_LIMIT_6           (2 << 6)
#   define DR_LIMIT_12          (3 << 6)
#   define DR_E                 (7 << 0)
#   define SET_DR_E(VAL)        (VAL & 7)

#define RF_MDMCFG2          0x22
#   define DR_M                 (63 << 0)
#   define SET_DR_M(VAL)        (VAL & 63)

#define RF_MULTCFG          0x23
#   define MULT_EXP             (15 << 4)
#   define SET_MULT_EXP(VAL)    ((VAL & 15) << 4)
#   define MULT_MANTISSA        (15 << 0)
#   define SET_MULT_MANTISSA    (VAL & 15)

// Probably use 16 or 32us
#define RF_TXRAMP           0x24
#   define TX_RAMP              (7 << 0)
#   define TX_RAMP_0us          (0 << 0)
#   define TX_RAMP_32us         (1 << 0)
#   define TX_RAMP_16us         (2 << 0)
#   define TX_RAMP_8us          (3 << 0)
#   define TX_RAMP_192us        (4 << 0)
#   define TX_RAMP_96us         (5 << 0)
#   define TX_RAMP_48us         (6 << 0)
#   define TX_RAMP_24us         (7 << 0)
#   define SET_TX_RAMP(VAL)     (VAL & 7)

#define RF_PREAMBLE         0x25
#   define PREAMBLE_BYTES       (255 << 0)

#define RF_DEMODCFG         0x26
#   define EN_IQ_CORR           (1 << 7)
#   define CAL_IQ_CORR          (1 << 6)
#   define RSSI_DEC             (3 << 4)
#   define RSSI_DEC_1x          (0 << 4)
#   define RSSI_DEC_2x          (1 << 4)
#   define RSSI_DEC_4x          (2 << 4)
#   define RSSI_DEC_8x          (3 << 4)
#   define CHANBW               (15 << 0)
#   define CHANBW_600kHz        (0)
#   define CHANBW_300kHz        (1)
#   define CHANBW_200kHz        (2)
#   define CHANBW_150kHz        (3)
#   define CHANBW_120kHz        (4)
#   define CHANBW_100kHz        (5)
#   define CHANBW_75kHz         (6)
#   define CHANBW_60kHz         (7)
#   define CHANBW_50kHz         (8)
#   define CHANBW_38kHz         (9)
#   define CHANBW_30kHz         (10)
#   define CHANBW_25kHz         (11)
#   define CHANBW_19kHz         (12)
#   define CHANBW_15kHz         (13)
#   define CHANBW_13kHz         (14)
#   define CHANBW_9kHz          (15)

#define RF_DATACFG          0x27
#   define EN_LSB_FIRST         (1 << 7)
#   define EN_BIT_INVERT        (1 << 6)
#   define EN_MANCHESTER        (1 << 5)
#   define EN_DATAWHITE         (1 << 4)
#   define EN_PREAMBLE          (1 << 3)
#   define EN_PACKETRX          (1 << 2)
#   define EN_PACKET            EN_PACKETRX
#   define EN_CRC               (1 << 1)
#   define EN_ADDRESS           (1 << 0)

#define RF_MODCFG           0x28
#   define MOD_FSK              (1 << 7)
#   define MOD_OOK              (0 << 7)
#   define EN_GAUSSIAN          (1 << 6)
#   define EN_INTERP            (1 << 5)
#   define POSNEG_MIX           (1 << 4)
#   define POSNEG_IF            (1 << 3)    /** @note is there a benefit to NEG vs POS? */
#   define DIRECT_MOD           (7 << 0)
#   define DIRECT_MOD_NONE      (0)
#   define DIRECT_MOD_FIXED0    (2)
#   define DIRECT_MOD_FIXED1    (3)
#   define DIRECT_MOD_GPIO0     (4)
#   define DIRECT_MOD_GPIO1     (5)
#   define DIRECT_MOD_GPIO2     (6)
#   define DIRECT_MOD_GPIO3     (7)

#define RF_CSCFG            0x29
#   define RX_RSSI_TH           (15 << 0)
#   define SET_RX_RSSI_TH(VAL)  (VAL & 15)

#define RF_RXTIMEOUT        0x2A
#   define RXTERM_EXP           (15 << 4)
#   define SET_RXTERM_EXP(VAL)  ((VAL & 15) << 4)
#   define RXTERM_MANT          (15 << 0)
#   define SET_RXTERM_MANT(VAL) (VAL & 15)

#define RF_MCSM0            0x2B
#   define RXTERM_COND          (3 << 6)
#   define RXTERM_COND_OFF      (0 << 6)
#   define RXTERM_COND_ON       (1 << 6)
#   define RXTERM_COND_NOCS     (2 << 6)
#   define RXTERM_COND_NORX     (3 << 6)
#   define CALIB_MODE           (7 << 0)
#   define CALIB_MODE_1in1      (0)
#   define CALIB_MODE_1in4      (1)
#   define CALIB_MODE_1in8      (2)
#   define CALIB_MODE_1in16     (3)
#   define CALIB_MODE_1in32     (4)
#   define CALIB_MODE_1in64     (5)
#   define CALIB_MODE_1in128    (6)
#   define CALIB_MODE_OFF       (7)

#define RF_PWRUPCFG         0x2C
#   define PWRUP_MODE           (1 << 4)
#   define PWRUP_MODE_VAR       (0 << 4)
#   define PWRUP_MODE_FIXED     (1 << 4)
#   define PWRUP_TIME           (15 << 0)
#   define PWRUP_TIME_200us     (0)
#   define PWRUP_TIME_250us     (1)
#   define PWRUP_TIME_400us     (2)
#   define PWRUP_TIME_500us     (3)
#   define PWRUP_TIME_750us     (4)
#   define PWRUP_TIME_1000us    (5)
#   define PWRUP_TIME_1500us    (6)
#   define PWRUP_TIME_2000us    (7)
#   define PWRUP_TIME_3000us    (8)
#   define PWRUP_TIME_4000us    (9)
#   define PWRUP_TIME_6000us    (10)
#   define PWRUP_TIME_8000us    (11)
#   define PWRUP_TIME_12000us   (12)
#   define PWRUP_TIME_16000us   (13)
#   define PWRUP_TIME_24000us   (14)
#   define PWRUP_TIME_32000us   (15)

#define RF_MCSM1            0x2D
#   define RXTERM_ACT           (1 << 7)
#   define RXTERM_ACT_STOP      (0 << 7)
#   define RXTERM_ACT_CALRX     (1 << 7)
#   define RXUNLOCK_ACT         (1 << 5)
#   define RXUNLOCK_ACT_ERR     (0 << 5)
#   define RXUNLOCK_ACT_RERX    (1 << 5)
#   define RXERR_ACT            (1 << 4)
#   define RXERR_ACT_ERR        (0 << 4)
#   define RXERR_ACT_RERX       (1 << 4)
#   define RXOK_ACT             (1 << 3)
#   define RXOK_ACT_STOP        (0 << 3)
#   define RXOK_ACT_RERX        (1 << 3)
#   define TXOK_ACT             (3 << 0)
#   define TXOK_ACT_STOP        (0)
#   define TXOK_ACT_RETX        (1)
#   define TXOK_ACT_FASTRX      (2)     // no calibration
#   define TXOK_ACT_RX          (3)

#define RF_STATUS0          0x2F    //2E / 2F
#   define RFMODE               (7 << 5)
#   define RFMODE_STOPPEDTX     (0 << 5)
#   define RFMODE_STOPPEDRX     (1 << 5)
#   define RFMODE_TX            (2 << 5)
#   define RFMODE_RX            (3 << 5)
#   define RFMODE_MANCALTX      (4 << 5)
#   define RFMODE_MANCALRX      (5 << 5)
#   define RFMODE_AUTOCALTX     (6 << 5)
#   define RFMODE_AUTOCALRX     (7 << 5)
#   define INFO                 (7 << 2)
#   define INFO_OK              (0 << 2)    // during modes 0-1
#   define INFO_PWRUP           (0 << 2)    // during modes 2-7
#   define INFO_PWRUPTIMEOUT    (1 << 2)    // during modes 0-1
#   define INFO_CALVCO          (1 << 2)    // during modes 2-7
#   define INFO_PLLUNLOCK       (2 << 2)    // during modes 0-1
#   define INFO_CALPLL          (2 << 2)    // during modes 2-7
#   define INFO_FIFOERR         (3 << 2)    // during modes 0-1
#   define INFO_RES3            (3 << 2)    // during modes 2-7
#   define INFO_RXTIMEOUT       (4 << 2)    // during modes 0-1
#   define INFO_RXWAITING       (4 << 2)    // during modes 2-7
#   define INFO_BADPKTLEN       (5 << 2)    // during modes 0-1
#   define INFO_RXPAYLOAD       (5 << 2)    // during modes 2-7
#   define INFO_BADADDRESS      (6 << 2)    // during modes 0-1
#   define INFO_RES6            (6 << 2)    // during modes 2-7
#   define INFO_BADCRC          (7 << 2)    // during modes 0-1
#   define INFO_TXPAYLOAD       (7 << 2)    // during modes 2-7

#define RF_STATUS1          0x2E    //2F / 2E
#   define EN_XTAL              (1 << 4)
#   define EN_VDIG              (1 << 3)
#   define KEEP_PLL_ON          (1 << 2)
#   define TXFIFO_STATUS        (1 << 1)
#   define TXFIFO_STATUS_EMPTY  (0 << 1)
#   define TXFIFO_STATUS_INUSE  (1 << 1)
#   define RXFIFO_STATUS        (1 << 0)
#   define RXFIFO_STATUS_EMPTY  (0 << 0)
#   define RXFIFO_STATUS_INUSE  (1 << 0)

#define RF_TXFIFOCNT        0x30
#   define TXFIFO_EMPTY         0       // while RFSTATUS1.TXFIFO_STATUS = 0
#   define TXFIFO_UNDERRUN      1       // while RFSTATUS1.TXFIFO_STATUS = 0
#   define TXFIFO_OVERRUN       2       // while RFSTATUS1.TXFIFO_STATUS = 0

#define RF_RXFIFOCNT        0x31
#   define RXFIFO_EMPTY         0       // while RFSTATUS1.RXFIFO_STATUS = 0
#   define RXFIFO_UNDERRUN      1       // while RFSTATUS1.RXFIFO_STATUS = 0
#   define RXFIFO_OVERRUN       2       // while RFSTATUS1.RXFIFO_STATUS = 0

#define RF_RXFIFORD         0x32
#define RF_TXFIFOWR         0x32        // same as RXFIFORD

#define RF_RNG              0x33        // random number generator

#define RF_IOSTATUS         0x35
#   define BATTOK               (1 << 7)
#   define BATTOK_BACKUP        (0 << 7)
#   define BATTOK_NORMAL        (1 << 7)
#   define PLL_LOCKED           (1 << 6)
#   define CYCLE_SLIP           (1 << 5)
#   define RSSI_GOOD            (1 << 4)    // RSSI above CS THR
#   define GPIO3_LV             (1 << 3)
#   define GPIO2_LV             (1 << 2)
#   define GPIO1_LV             (1 << 1)
#   define GPIO0_LV             (1 << 0)

#define RF_GPIODRV          0x37
#   define GPIO3_DRV            (3 << 6)
#   define GPIO3_DRV_3mA        (0 << 6)
#   define GPIO3_DRV_6mA        (1 << 6)
#   define GPIO3_DRV_9mA        (2 << 6)
#   define GPIO3_DRV_12mA       (3 << 6)
#   define GPIO2_DRV            (3 << 4)
#   define GPIO2_DRV_3mA        (0 << 4)
#   define GPIO2_DRV_6mA        (1 << 4)
#   define GPIO2_DRV_9mA        (2 << 4)
#   define GPIO2_DRV_12mA       (3 << 4)
#   define GPIO1_DRV            (3 << 2)
#   define GPIO1_DRV_3mA        (0 << 2)
#   define GPIO1_DRV_6mA        (1 << 2)
#   define GPIO1_DRV_9mA        (2 << 2)
#   define GPIO1_DRV_12mA       (3 << 2)
#   define GPIO0_DRV            (3 << 0)
#   define GPIO0_DRV_3mA        (0 << 0)
#   define GPIO0_DRV_6mA        (1 << 0)
#   define GPIO0_DRV_9mA        (2 << 0)
#   define GPIO0_DRV_12mA       (3 << 0)

// Put IOSEL into these regs to configure how the GPIOs work
#define RF_GPIO0CFG         0x38
#define RF_GPIO1CFG         0x39
#define RF_GPIO2CFG         0x3A
#define RF_GPIO3CFG         0x3B


#define RF_ADCCFG0          0x3C
#   define ADC_REF_SEL          (7 << 5)
#   define ADC_REF_SEL_VREF     (0 << 5)
#   define ADC_REF_SEL_VDDA3    (1 << 5)
#   define ADC_REF_SEL_VDDA3D2  (2 << 5)
#   define ADC_REF_SEL_VANA     (3 << 5)
#   define ADC_REF_SEL_AIO0     (4 << 5)
#   define ADC_REF_SEL_AIO1     (5 << 5)
#   define ADC_REF_SEL_AIO2     (6 << 5)
#   define ADC_REF_SEL_AIO3     (7 << 5)
#   define ADC_CH_SEL           (15 << 0)
#   define ADC_CH_SEL_03VDDA3   (0)
#   define ADC_CH_SEL_03VMAIN   (1)
#   define ADC_CH_SEL_06VDIG    (2)
#   define ADC_CH_SEL_06VANA    (3)
#   define ADC_CH_SEL_06VVCO    (4)
#   define ADC_CH_SEL_06VPA     (5)
#   define ADC_CH_SEL_PD1       (6)
#   define ADC_CH_SEL_PD2       (7)
#   define ADC_CH_SEL_AIO0      (8)
#   define ADC_CH_SEL_AIO1      (9)
#   define ADC_CH_SEL_AIO2      (10)
#   define ADC_CH_SEL_AIO3      (11)
#   define ADC_CH_SEL_TEMPSENS  (12)
#   define ADC_CH_SEL_3DLFRSSI  (13)
#   define ADC_CH_SEL_RES14     (14)
#   define ADC_CH_SEL_RES15     (15)

#define RF_ADCCFG1          0x3D
#   define ADC_NEWDATA          (1 << 7)
#   define ADC_CONTINU          (1 << 3)
#   define ADC_CLK_SEL          (3 << 1)
#   define ADC_CLK_SEL_RCOSC    (0 << 1)
#   define ADC_CLK_SEL_XTALD64  (1 << 1)
#   define ADC_CLK_SEL_XTALD32  (2 << 1)
#   define ADC_CLK_SEL_XTALD16  (3 << 1)
#   define ADC_START_EOC        (1 << 0)

#define RF_ADCCAL_L         0x3E
#define RF_ADCCAL_H         0x3F

#define RF_TMRCFG0          0x40
#   define TMR_MANT             (255)

#define RF_TMRCFG1          0x41
#   define TMR_SOURCE           (3 << 6)
#   define TMR_SOURCE_OFF       (0 << 6)
#   define TMR_SOURCE_XTAL      (1 << 6)
#   define TMR_SOURCE_RCOSC32   (2 << 6)
#   define TMR_SOURCE_RCOSC15   (3 << 6)
#   define TMR_MODE             (1 << 5)
#   define TMR_MODE_CLKGEN      (0 << 5)
#   define TMR_MODE_TIMER       (1 << 5)
#   define TMR_EXP              (31 << 0)
#   define SET_TMR_EXP(VAL)     (VAL & 0x1F)

#define RF_TMRSTATUS        0x42
#   define TMR_FLAG             (1 << 7)

#define RF_SYSTIME0         0x44
#   define SYS_TIME_L           (255)

#define RF_SYSTIME1         0x45
#   define SYS_TIME_M           (255)

#define RF_SYSTIME2         0x46
#   define SYS_TIME_OVF         (1 << 7)
#   define SYS_TIME_H           (127)

#define RF_SYSTIMECFG       0x47
#   define SYS_TIME_ON          (1 << 7)
#   define SYS_TIME_OFF         (0 << 7)

#define RF_POLLOFFSET0      0x48
#   define POLL_OFFSET_L        (255)

#define RF_POLLOFFSET1      0x49
#   define POLL_OFFSET_H        (255)

#define RF_POLLPERIOD0      0x4A
#   define POLL_PERIOD_L        (255)

#define RF_POLLPERIOD1      0x4B
#   define POLL_PERIOD_H        (255)

#define RF_POLLCFG          0x4C
#   define POLL_TMR_ON          (1 << 7)
#   define POLL_TMR_OFF         (0 << 7)
#   define POLL_FLAG            (1 << 6)
#   define POLL_RFRX_ON         (1 << 5)
#   define POLL_RFRX_OFF        (0 << 5)
#   define POLL_EXP             (15 << 0)

#define RF_RCOSCCFG0        0x4E
#   define RC_CAL_L             (255)

#define RF_RCOSCCFG1        0x4F
#   define RC_CAL_H             (7)


#define LF_TUNING1          0x50
#   define COIL1_TRIM           (63)

#define LF_TUNING2          0x51
#   define COIL2_TRIM           (63)

#define LF_TUNING3          0x52
#   define COIL3_TRIM           (63)

#define LF_MANUALCFG        0x53
#   define FORCE_MOD_3DLF       (1 << 2)
#   define FORCE_COIL_HEADER    (1 << 1)
#   define FORCE_COIL_LISTEN    (1 << 0)

#define LF_HEADER0          0x54
#   define LFRX_HDR_B0          (255)

#define LF_HEADER1          0x55
#   define LFRX_HDR_B1          (255)

#define LF_HEADER2          0x56
#   define LFRX_HDR_B2          (255)

#define LF_HEADER3          0x57
#   define LFRX_HDR_B3          (255)

#define LF_MDMCFG0          0x59
#   define LFRX_POL             (1 << 7)
#   define LFRX_POL_NORMAL      (0 << 7)
#   define LFRX_POL_INVERT      (1 << 7)
#   define LFRX_HDRLEN          (7 << 4)
#   define LFRX_HDRLEN_NONE     (0 << 4)
#   define LFRX_HDRLEN_8bit     (4 << 4)
#   define LFRX_HDRLEN_16bit    (5 << 4)
#   define LFRX_HDRLEN_24bit    (6 << 4)
#   define LFRX_HDRLEN_32bit    (7 << 4)
#   define LFTX_POL             (1 << 3)
#   define LFTX_POL_NORMAL      (0 << 3)
#   define LFTX_POL_INVERT      (1 << 3)
#   define LFTX_MODE            (7)
#   define LFTX_MODE_NRZLSB     (0)
#   define LFTX_MODE_NRZMSB     (1)
#   define LFTX_MODE_MANCLSB    (2)
#   define LFTX_MODE_MANCMSB    (3)
#   define LFTX_MODE_DIRECT0    (4)
#   define LFTX_MODE_DIRECT1    (5)
#   define LFTX_MODE_DIRECT2    (6)
#   define LFTX_MODE_DIRECT3    (7)

#define LF_MDMCFG1          0x5A
#   define LFRX_MASK            (7 << 5)
#   define LFRX_MASK_COIL1      (1 << 5)
#   define LFRX_MASK_COIL2      (2 << 5)
#   define LFRX_MASK_COIL3      (4 << 5)
#   define LFRX_ORDER           (1 << 4)
#   define LFRX_ORDER_LSB       (0 << 4)
#   define LFRX_ORDER_MSB       (1 << 4)
#   define LFRX_STBY            (15 << 0)
#   define LFRX_STBY_190us      (0)
#   define LFRX_STBY_250us      (1)
#   define LFRX_STBY_220us      (2)
#   define LFRX_STBY_340us      (3)
#   define LFRX_STBY_280us      (4)
#   define LFRX_STBY_530us      (5)
#   define LFRX_STBY_410us      (6)
#   define LFRX_STBY_910us      (7)
#   define LFRX_STBY_660us      (8)
#   define LFRX_STBY_1700us     (9)
#   define LFRX_STBY_1000us     (10)
#   define LFRX_STBY_3000us     (11)
#   define LFRX_STBY_2000us     (12)
#   define LFRX_STBY_6000us     (13)
#   define LFRX_STBY_4000us     (14)
#   define LFRX_STBY_OFF        (15)

#define LF_STATUS           0x5B
#   define LFMODE               (7 << 4)
#   define LFMODE_OFF           (0 << 4)
#   define LFMODE_RSSI          (1 << 4)
#   define LFMODE_RX            (3 << 4)
#   define LFMODE_TXOFF         (4 << 4)
#   define LFMODE_TXRSSI        (5 << 4)
#   define LFMODE_TXRX          (7 << 4)
#   define LFRX_STATE           (3 << 2)
#   define LFRX_STATE_IDLE      (0 << 2)
#   define LFRX_STATE_LISTEN    (1 << 2)
#   define LFRX_STATE_HEADER    (2 << 2)
#   define LFRX_STATE_RXDATA    (3 << 2)
#   define LFRX_COIL            (3 << 0)
#   define LFRX_COIL_1          (1)
#   define LFRX_COIL_2          (2)
#   define LFRX_COIL_3          (3)

#define LF_TXFIFOCNT        0x5D
#define LF_RXFIFOCNT        0x5E
#define LF_TXFIFOWR         0x5F
#define LF_RXFIFORD         0x5F

#define RF_CHIP_ID          0x7F




/** Bank 0 Registers
  */
#define B0_TEST0            0x00
#   define B0_DTB               (15 << 0)

#define B0_SPAREINPUT       0x01
#   define SPARE_IN_3V          (15 << 4)
#   define SPARE_IN_1V7         (15 << 0)

#define B0_LVINPUT          0x02
#   define LV_VPA_DB            (1 << 7)
#   define LV_VVCO_DB           (1 << 6)
#   define LV_VANA_DB           (1 << 5)
#   define LV_VDIG_DB           (1 << 4)
#   define LV_VPA               (1 << 3)
#   define LV_VVCO              (1 << 2)
#   define LV_VANA              (1 << 1)
#   define LV_VDIG              (1 << 0)

#define B0_PWRSTATUS        0x03
#   define XTAL_RUN_OK          (1 << 7)
#   define LOWBAT               (1 << 6)
#   define POW_SWX_STAT         (3 << 4)
#   define POW_SWX_STAT_ALLOFF  (0 << 4)
#   define POW_SWX_STAT_BATON   (1 << 4)
#   define POW_SWX_STAT_LFON    (2 << 4)
#   define POW_SWX_STAT_ALLON   (3 << 4)
#   define VDIG_RDY             (1 << 3)
#   define VANA_RDY             (1 << 2)
#   define VVCO_RDY             (1 << 1)
#   define VPA_RDY              (1 << 0)

#define B0_TEST1            0x08
#   define EN_TESTDIV           (1 << 7)
#   define PLL_DIV_TEST_REG     (63 << 0)

#define B0_SIGROUTE         0x09
#   define ATB_SEL              (15 << 4)
#   define DTB_SEL              (15)

#define B0_SPAREOUTPUT0     0x0A
#   define SPARE_1V7            (255)

#define B0_SPAREOUTPUT1     0x0B
#   define SPARE_3V             (255)

#define B0_PWRCFG0           0x0C
#   define EN_BROWNOUT          (1 << 7)
#   define E_RCO                (1 << 6)
#   define EN_BG                (1 << 5)
#   define E_LBD                (1 << 4)
#   define LBD_XORSEL           (1 << 3)
#   define POW_SWX_CTRL         (7 << 0)
#   define POW_SWX_CTRL_AUTO    (0)
#   define POW_SWX_CTRL_CONT    (4)
#   define POW_SWX_CTRL_BATON   (5)
#   define POW_SWX_CTRL_LFON    (6)
#   define POW_SWX_CTRL_OFF     (7)

#define B0_PWRCFG1          0x0D
#   define EN_WRPROT            (1 << 7)
#   define EN_RCAL              (1 << 6)
#   define E_LVD                (15 << 0)

#define B0_PWRCFG2          0x0E
#   define PAVREG_ON            (1 << 7)
#   define PAVREG_SHORT         (1 << 6)
#   define PAVREG_BYPASS        (1 << 5)
#   define PAVREG_TRIM          (3 << 3)
#   define EN_PTAT              (1 << 2)
#   define PTAT_SEL             (3 << 0)

#define B0_PWRCFG3          0x0F
#   define EN_VCOREREG          (1 << 7)
#   define EN_DVREG             (1 << 6)
#   define DVREG_TRIM           (7 << 3)
#   define EN_AVREG             (1 << 2)
#   define AVREG_TRIM           (3 << 0)

#define B0_CLKCFG           0x10
#   define XTAL_DEBUG           (1 << 7)
#   define EN_TESTCLK           (1 << 6)
#   define EN_DIGCLK            (1 << 5)
#   define EN_MODCLK            (1 << 4)
#   define EN_XTAL_LOAD         (1 << 3)
#   define PLL_CLK_POL          (1 << 2)
#   define SDM_CLK_POL          (1 << 1)
#   define OOK_CLK_POL          (1 << 0)

#define B0_BLOCKCFG         0x11
#   define EN_PA                (1 << 6)
#   define EN_PREAMP            (1 << 5)
#   define EN_DAC               (1 << 4)
#   define EN_ADC               (1 << 3)
#   define EN_PGA               (1 << 2)
#   define EN_MIX               (1 << 1)
#   define EN_LNA               (1 << 0)

#define B0_AFECFG           0x12
#   define CSLIP_DIS            (1 << 6)
#   define PLL_START_VBG        (1 << 5)
#   define EN_QUADDIV           (1 << 4)
#   define EN_DYNDIV            (1 << 3)
#   define EN_CHP_PFD           (1 << 2)
#   define EN_PLLDIV            (1 << 1)
#   define EN_VCO               (1 << 0)

#define B0_MASHCFG          0x13
#   define EN_MASH              (3 << 3)
#   define MASH_DITHER          (1 << 2)
#   define MASH_ORDER           (3 << 0)

#define B0_PLLCFG           0x14
#   define EN_VCOCLK            (1 << 4)
#   define EN_CHP_OFFS          (1 << 3)
#   define PLL_OPENLP           (7 << 0)
#   define PLL_OPENLP_NEVER     (0)
#   define PLL_OPENLP_CALMIN    (1)
#   define PLL_OPENLP_CALMED    (2)
#   define PLL_OPENLP_CALMAX    (3)
#   define PLL_OPENLP_MIN       (5)
#   define PLL_OPENLP_MED       (6)
#   define PLL_OPENLP_MAX       (7)

#define B0_DACCFG           0x15
#   define SDM_FC               (15 << 4)
#   define DAC_FC               (15 << 0)

#define B0_RXMIXERBIAS      0x18
#   define RF_RX_MIX_2          (15 << 4)
#   define RF_RX_MIX_1          (15 << 0)

#define B0_RXLNABIAS        0x19
#   define RF_RX_LNA_2          (15 << 4)
#   define RF_RX_LNA_1          (15 << 0)

#define B0_TXPABIAS         0x1A
#   define RF_TX_PA_2           (15 << 4)
#   define RF_TX_PA_1           (15 << 0)

#define B0_TXOOKBIAS        0x1B
#   define RF_TX_OOK_2          (15 << 4)
#   define RF_TX_OOK_1          (15 << 0)

#define B0_TXPREAMPBIAS     0x1C
#   define RF_TX_PREAMP_2       (15 << 4)
#   define RF_TX_PREAMP_1       (15 << 0)

#define B0_PLLBIAS0         0x1D
#   define RF_TX_REG_PA_BIAS    (15 << 4)
#   define IQCLK_SCL_LOOP       (15 << 0)

#define B0_TRXIQBIAS        0x1E
#   define IQCLK_SCL_RX         (15 << 4)
#   define IQCLK_SCL_TX         (15 << 0)

#define B0_PLLBIAS1         0x20
#   define PLL_VCO_HI           (15 << 4)
#   define PLL_VCO_LO           (15 << 0)

#define B0_PLLBIAS2         0x21
#   define PLL_CHP              (15 << 4)
#   define PLL_VCO_REG          (15 << 0)

#define B0_ADCBIAS          0x22
#   define ADC_Q_BIAS           (15 << 4)
#   define ADC_I_BIAS           (15 << 0)

#define B0_PGABIAS          0x23
#   define PGA_Q_BIAS           (15 << 4)
#   define PGA_I_BIAS           (15 << 0)

#define B0_PGAACBIAS        0x24
#   define PGA_AC_Q_BIAS        (15 << 4)
#   define PGA_AC_I_BIAS        (15 << 0)

#define B0_PWDETBIAS        0x25
#   define RF_PWDET_CORE        (15 << 4)
#   define RF_PWDET_OPA         (15 << 0)

#define B0_IBG0             0x26
#   define IBG1                 (15 << 4)
#   define IBG0                 (15 << 0)

#define B0_IBG1             0x27
#   define IBG3                 (15 << 4)
#   define IBG2                 (15 << 0)

#define B0_IBG2             0x28
#   define IBG5                 (15 << 4)
#   define IBG4                 (15 << 0)

#define B0_IBG3             0x29
#   define IBG7                 (15 << 4)
#   define IBG6                 (15 << 0)

#define B0_IBGCFG           0x2A
#   define EN_IBG7              (1 << 7)
#   define EN_IBG6              (1 << 6)
#   define EN_IBG5              (1 << 5)
#   define EN_IBG4              (1 << 4)
#   define EN_IBG3              (1 << 3)
#   define EN_IBG2              (1 << 2)
#   define EN_IBG1              (1 << 1)
#   define EN_IBG0              (1 << 0)

#define B0_XTALBIASCFG      0x2B
#   define XTAL_STEADY          (3 << 6)
#   define XTAL_STARTUP         (3 << 4)
#   define IPTAT_TEST           (15 << 0)

#define B0_VCOBIASCFG       0x2C
#   define VCOREG_TRIM          (3 << 6)
#   define VCO_CURR             (15 << 1)
#   define EN_IPTAT_TEST        (1 << 0)

#define B0_SCLAMP           0x2D
#   define SCL_LOOP_AMP         (3 << 4)
#   define SCL_RX_AMP           (3 << 2)
#   define SCL_TX_AMP           (3 << 0)

#define B0_VCORXCFG         0x30
#   define VCO_CAL_RX           (63 << 1)
#   define VCO_LSB2_RX          (1 << 0)

#define B0_VCOTXCFG         0x31
#   define VCO_CAL_TX           (63 << 1)
#   define VCO_LSB2_TX          (1 << 0)

#define B0_IQCORRCFG        0x33
#   define IQ_CORR_ACC          (3 << 5)
#   define IQ_CORR_CONV         (3 << 3)
#   define IQ_CORR_TAU          (7 << 0)

#define B0_OOKCFG0          0x34
#   define OOK_MOD              (255)

#define B0_OOKCFG1          0x35
#   define EN_OOK               (1 << 5)
#   define OOK_LEVEL            (15 << 0)

#define B0_OOKCFG2          0x36
#   define T_OOK                (7 << 4)
#   define T_OOK_RATIO          (7 << 0)

#define B0_TAUCFG0          0x37
#   define TAU_PHASE_RECOV      (255)

#define B0_TAUCFG1          0x38
#   define TAU_CARRIER_RECOV    (255)

#define B0_TAUCFG2          0x39
#   define TAU_CLK_RECOV        (255)

#define B0_TAUCFG3          0x3A
#   define TAU_DR_RECOV         (255)

#define B0_FSKFCRAMP1       0x3B
#define B0_FSKFCRAMP2       0x3C
#define B0_FSKFCRAMP3       0x3D

#define B0_ISICFG0          0x3E
#   define COEF_CORRECT_ISI     (31 << 0)

#define B0_ISICFG1          0x3F
#   define ISI_FREQ_CORRECT     (255)

#define B0_DELAYCFG         0x40
#   define WAIT_LOCK            (3 << 6)
#   define CAL_MEAS             (7 << 3)
#   define CAL_SETTLE           (7 << 0)


#define B0_PREAMBLE         0x41


#define B0_LFCFG0           0x44
#   define LFRX_MINCNT          (31 << 0)

#define B0_LFCFG1           0x45
#   define LFRX_MAXCNT          (31 << 0)

#define B0_LFCFG2           0x46
#   define LFRX_RATE            (127 << 0)

#define B0_LFCFG3           0x47
#   define LFTX_RATE            (255)






/** GPIO Selection table
  */
#   define SIG(VAL)             (IOSEL_##VAL)
#   define SIG_N(VAL)           (IOSEL_##VAL | 1)

#   define IOSEL_FIXED0         (0x08 << 1)
#   define IOSEL_RDY            (0x09 << 1)
#   define IOSEL_BATTOK         (0x0A << 1)
#   define IOSEL_RCOSC          (0x0B << 1)
#   define IOSEL_DTB0           (0x0C << 1)
#   define IOSEL_DTB1           (0x0D << 1)
#   define IOSEL_DTB2           (0x0E << 1)
#   define IOSEL_DTB3           (0x0F << 1)
#   define IOSEL_LV_VDIG        (0x10 << 1)
#   define IOSEL_LV_VANA        (0x11 << 1)
#   define IOSEL_LV_VVCO        (0x12 << 1)
#   define IOSEL_LV_VPA         (0x13 << 1)
#   define IOSEL_LOWBAT         (0x14 << 1)
#   define IOSEL_XTAL_RDY       (0x15 << 1)
#   define IOSEL_ADC_NEWDATA    (0x18 << 1)
#   define IOSEL_SYS_TIMEOVF    (0x19 << 1)
#   define IOSEL_TMR_FLAG       (0x1A << 1)
#   define IOSEL_POLL_FLAG      (0x1B << 1)
#   define IOSEL_PLL_CSLIP      (0x1C << 1)
#   define IOSEL_PLL_LOCK       (0x1D << 1)
#   define IOSEL_RFRX_CLK       (0x1E << 1)
#   define IOSEL_RFRX_DATA      (0x1F << 1)
#   define IOSEL_RFRX_WAITHDR   (0x20 << 1)
#   define IOSEL_RFRX_PAYLOAD   (0x21 << 1)
#   define IOSEL_RFRX_ACTIVE    (0x22 << 1)
#   define IOSEL_RFTX_PAYLOAD   (0x23 << 1)
#   define IOSEL_RF_STOPPED     (0x24 << 1)
#   define IOSEL_RF_ERRSTOP     (0x25 << 1)
#   define IOSEL_RFTX_FIFOERR   (0x26 << 1)
#   define IOSEL_RFRX_FIFOERR   (0x27 << 1)
#   define IOSEL_RFRX_FIFONOTEMPTY (0x28 << 1)
#   define IOSEL_RFRX_FIFO64BYTES (0x29 << 1)
#   define IOSEL_RFTX_FIFONOTEMPTY (0x2A << 1)
#   define IOSEL_RFTX_FIFO64BYTES (0x2B << 1)
#   define IOSEL_RFTX_SYMBOLCLK (0x2C << 1)
#   define IOSEL_RFRX_RSSICS    (0x2D << 1)
#   define IOSEL_RF_PKTDONE     (0x2E << 1)
#   define IOSEL_RF_PKTDONE_LFRX_PAYLOAD (0x2F << 1)
#   define IOSEL_IQ_CORR_CAL    (0x30 << 1)
#   define IOSEL_LFRX_ACTIVE    (0x31 << 1)
#   define IOSEL_LFRX_PAYLOAD   (0x32 << 1)
#   define IOSEL_LFRX_HDRPAYLOAD (0x33 << 1)
#   define IOSEL_RF_PKTDONE_LFRX_PAYLOAD_SYS_TIMEOVF (0x34 << 1)
#   define IOSEL_LFRX_FIFONOTEMPTY (0x35 << 1)
#   define IOSEL_LFRX_FIFO5BYTES (0x36 << 1)
#   define IOSEL_LFRX_FIFO8BYTES (0x37 << 1)
#   define IOSEL_RF_PKTDONE_LFRX_PAYLOAD_SYS_TIMEOVF_TMR_FLAG (0x38 << 1)
#   define IOSEL_LFTX_FIFONOTEMPTY (0x39 << 1)
#   define IOSEL_LFTX_FIFO5BYTES (0x3A << 1)
#   define IOSEL_LFTX_FIFO8BYTES (0x3B << 1)
#   define IOSEL_LFRX_FCCLK     (0x3C << 1)
#   define IOSEL_LFRX_AMSIG     (0x3D << 1)
#   define IOSEL_LFRX_MANCCLK   (0x3E << 1)
#   define IOSEL_LFRX_MANCDATA  (0x3F << 1)












/** Default MLX73xxx RF registers for DASH7 Operation
  */












/** Default MLX73xxx LF registers for DASH7/TPMS
  */







/** Common GPIO register setup Macros for OpenTag
  */







