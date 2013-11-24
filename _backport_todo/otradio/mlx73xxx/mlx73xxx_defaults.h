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
  * @file       /Platforms/MLX73290/mlx73xxx_defaults.h
  * @author     JP Norair
  * @version    V1.0
  * @date       26 November 2011
  * @brief      Default Values for MLX73xxx Registers
  * @ingroup    MLX73xxx
  * 
  ******************************************************************************
  */

#define RFDEF(VAL)      DRF_##VAL
#define RFNRM(VAL)      NRF_##VAL
#define RFTRB(VAL)      TRF_##VAL

#define LFDEF(VAL)      DLF_##VAL
#define B0DEF(VAL)      DB0_##VAL
#define B1DEF(VAL)      DB1_##VAL
#define B2DEF(VAL)      DB2_##VAL
#define B3DEF(VAL)      DB3_##VAL


/** Default MLX73xxx RF registers for DASH7 Operation
  */

/** Unbanked Registers for RF Configuration
  */
//Unbanked 0x02-03
#define DRF_TMRHDR_L 	0
#define DRF_TMRHDR_H 	0

//Unbanked 0x04-05
#define DRF_RSSI 		0
#define DRF_RSSIHDR		0

//Unbanked 0x06-07
#define DRF_AFC_L 		        0
#define DRF_AFC_H 		        0

//Unbanked 0x08-09
#define DRF_IQCORR_GAIN	        0
#define DRF_IQCORR_PHASE        0

//Unbanked 0x0A
#define DRF_DROFFSET 	        0

//Unbanked 0x0B
#define DRF_LNACFG              0xEF    // Nominal LNA Config, from Prototype
   
//Unbanked 0x0C
#define DRF_FILTER_GAIN_TURBO   0x04
#define DRF_FILTER_GAIN_NORMAL  0x66    // Gaussian = 0x65, non-gaussian = 0x66
#define DRF_FILTER_GAIN         0x66 

//Unbanked 0x0D
#define DRF_RXDSPCFG    (EN_CORRECT_ISI | EN_FAST_PRE_CR | AFC_MODE_HDREND | EN_CORRECT_CFREQ)

//Unbanked 0x10-13
#define DRF_RXPWR1      0x2B
#define DRF_RXPWR2      0x2B
#define DRF_TXPWR1      0x3F
#define DRF_TXPWR2      0x3F

//Unbanked 0x14-17
#define DRF_PATTERN0    0xD0
#define DRF_PATTERN1    0xE6
#define DRF_PATTERN2    0xAA
#define DRF_PATTERN3    0xAA

//Unbanked 0x18
///@note PATTERN_WORD_LEN_24 is 16 bits in prototype
#define DRF_PKTCFG0     (EN_DESERIAL | PATTERN_WORD_LEN_24 | PATTERN_MAX_ERR_0)

//Unbanked 0x19 
//- Do not set PACKET_LEN_FIXED/PACKET_LEN_VAR, this is done at runtime)
//- Currently, offset=-3 works when using integrated CRC
#define DRF_PKTCFG1     (FIFO_SIZE_2x128 | PACKET_LEN_POS_0 | SET_PACKET_LEN_CORR(-3) )

//Unbanked 0x1A
#define DRF_PKTLEN      255

//Unbanked 0x1B
#define DRF_ADDRESS     0

//Unbanked 0x1C - 1F
// Supplied default is 433.920 MHz
#define DRF_FREQ0       0x30
#define DRF_FREQ1       0xE7
#define DRF_FREQ2       0x1E
#define DRF_FREQ3       0x1B

//Unbanked 0x20
///@todo MDMCFG0 needs to be updated after the prototype
///@note: may need to twiddle PWRDET 2 & 1... not necessarily aligned with FREND
#define DRF_MDMCFG0     (/*EN_PWRDET2 | EN_PWRDET1 |*/ EN_FREND2 | /*EN_FREND1 | */ BAND_433)

//Unbanked 0x21
#define DR_E_TURBO      0x00
#define DR_E_NORMAL     0x00
#define DRF_MDMCFG1     (/*DR_LIMIT_3 |*/ DR_E_NORMAL)

//Unbanked 0x22
#define DR_M_TURBO      0x09
#define DR_M_NORMAL     0x23
#define DRF_MDMCFG2     (DR_M_NORMAL)

//Unbanked 0x23
#define DRF_MULTCFG_NORMAL      0x3A //0x67
#define DRF_MULTCFG_TURBO       0x84 /* @todo needs updating */
#define DRF_MULTCFG             DRF_MULTCFG_NORMAL

//Unbanked 0x24
#define DRF_TXRAMP              (TX_RAMP_192us)

//Unbanked 0x25
#define DRF_PREAMBLE_NORMAL     4
#define DRF_PREAMBLE_TURBO      6
#define DRF_PREAMBLE            DRF_PREAMBLE_NORMAL

//Unbanked 0x26
#define DRF_DEMODCFG_TURBO      (/*EN_IQ_CORR |*/ RSSI_DEC_1x | CHANBW_300kHz)
#define DRF_DEMODCFG_NORMAL     (/*EN_IQ_CORR |*/ RSSI_DEC_1x | CHANBW_150kHz)
#define DRF_DEMODCFG            DRF_DEMODCFG_NORMAL

//Unbanked 0x27
#define DRF_DATACFG_NONFEC      (EN_DATAWHITE | EN_CRC | EN_PREAMBLE | EN_PACKETRX)
#define DRF_DATACFG_FEC         (EN_PREAMBLE)
#define DRF_DATACFG             DRF_DATACFG_NONFEC

//Unbanked 0x28
#define DRF_MODCFG              (MOD_FSK /*| EN_GAUSSIAN | EN_INTERP | POSNEG_MIX*/)

//Unbanked 0x29
#define DRF_CSCFG       (0 /** @todo need to set this when RSSI gets qualified */)

//Unbanked 0x2A
#define DRF_RXTIMEOUT   0

//Unbanked 0x2B
#define DRF_MCSM0       (RXTERM_COND_OFF | CALIB_MODE_OFF)

//Unbanked 0x2C
#define DRF_PWRUPCFG    (PWRUP_MODE_VAR | PWRUP_TIME_6000us)

//Unbanked 0x2D
#define DRF_MCSM1       (RXTERM_ACT_STOP | RXUNLOCK_ACT_ERR | RXERR_ACT_ERR | \
                         RXOK_ACT_STOP | TXOK_ACT_STOP) //chip default

//Unbanked 0x2E (2F)
#define DRF_STATUS0     (0) //read only

//Unbanked 0x2F (2E)
#define DRF_STATUS1     (0) 

//Unbanked 0x30
#define DRF_TXFIFOCNT   (0) //read only

//Unbanked 0x31
#define DRF_RXFIFOCNT   (0) //read only

//Unbanked 0x32
#define DRF_RXFIFORD    (0) //read only
#define DRF_TXFIFOWR    (0) //write only


#define DRF_RNG         (0) // read only
#define DRF_CHIPSTATUS  (0) // read only

//using chip defaults (0)
#define DRF_GPIODRV     (GPIO3_DRV_3mA | GPIO2_DRV_3mA | GPIO1_DRV_3mA | GPIO0_DRV_3mA) 

//using chip defaults at power-up
#define DRF_GPIO0CFG        SIG(XTAL_RDY)
#define DRF_GPIO1CFG        SIG(RDY)       //default
#define DRF_GPIO2CFG        SIG(TMR_FLAG)  //default
#define DRF_GPIO3CFG        SIG(BATTOK)    //default

#define DRF_ADCCFG0         (0) //chip default
#define DRF_ADCCFG1         (0) //chip default
#define DRF_ADCCAL_L        (0) //read only
#define DRF_ADCCAL_H        (0) //read only

//Termination Timer is setup for CSMA & background sniff purposes.  It can also
//be used to do listening timing, as long as the listen period is < 64 ticks
#define DRF_TMRCFG0         (0) //default
#define DRF_TMRCFG1_OFF     (TMR_SOURCE_OFF | TMR_MODE_TIMER | SET_TMR_EXP(13))
#define DRF_TMRCFG1_ON      (TMR_SOURCE_XTAL | TMR_MODE_TIMER | SET_TMR_EXP(13))
#define DRF_TMRSTATUS       0x42

//System Timer is not used in OpenTag implementation
#define DRF_SYSTIME0        (0) //read only
#define DRF_SYSTIME1        (0) //read only
#define DRF_SYSTIME2        (0) //read only
#define DRF_SYSTIMECFG      (SYS_TIME_OFF) //chip default

//Polling Timer can be used to terminate listening windows or response windows.
//It is not sophisticated enough to be used for channel scanning in DASH7.  In
//OpenTag, it is easier to just use the system timer to do window timing.  The
//setup here makes approx 1 tick resolution (15.6 kHz / 16).
#define DRF_POLLOFFSET0     (0) //chip default
#define DRF_POLLOFFSET1     (0) //chip default
#define DRF_POLLPERIOD0     (0) //chip default
#define DRF_POLLPERIOD1     (0) //chip default
#define DRF_POLLCFG         (POLL_OFF | POLL_RFRX_OFF | (4))

//RC Oscillator calibration & trimming.  (needed if you use poll timer)
#define DRF_RCOSCCFG0       0xE8    //chip default
#define DRF_RCOSCCFG1       0x03    //chip default

//LF Registers (not currently implemented)
#define DLF_TUNING1         0x50
#define DLF_TUNING2         0x51
#define DLF_TUNING3         0x52
#define DLF_MANUALCFG       0x53
#define DLF_HEADER0         0x54
#define DLF_HEADER1         0x55
#define DLF_HEADER2         0x56
#define DLF_HEADER3         0x57
#define DLF_MDMCFG0         0x59
#define DLF_MDMCFG1         0x5A
#define DLF_STATUS          0x5B
#define DLF_TXFIFOCNT       0x5D
#define DLF_RXFIFOCNT       0x5E
#define DLF_TXFIFOWR        0x5F
#define DLF_RXFIFORD        0x5F

// Chip ID (read), Soft Reset (write)
#define DRF_CHIP_ID     0x11 //read only
#define DRF_SOFT_RESET  0x56


/*
Reg 0x0C:  65

Set the channel filter bandwitdh to:  600 kHz
Reg 0x26 mask:  00

Reg 0x0D:  f9
Reg 0x21 mask:  c0

Configuring for Packet
Reg 0x14 E6D0
Reg 0x18 mask:  18
Reg 0x19:  80
Reg 0x1A:  ff

Reg 0x25:  06
Reg 0x27 mask:  0c

VCO calibration: Reg 0x2f = da


//Reg B0 0x3c: ff (PKT)    -- Reg B0 0x3C:  63 (RX)
//Reg B0 0x3d: 20


Reg B0 0x07: 02
Reg B0 0x11: 60
Reg B0 0x12: 10

Reg B0 0x43: 26
Reg B0 0x44: 42
Reg B0 0x45: 5f
Reg B0 0x46: 06
Reg B0 0x47: 26
Reg B0 0x48: ff
Reg B0 0x49: aa
Reg B0 0x4A: 3f
Reg B0 0x4B: 03

*/


/** Bank 0 Registers (currently ignored)
  */
#define DB0_TEST0            0x00

#define DB0_SPAREINPUT       0x01

#define DB0_LVINPUT          0x02

#define DB0_CHIPSTATUS       0x03

#define DB0_TEST1            0x08

#define DB0_SIGROUTE         0x09

#define DB0_SPAREOUTPUT0     0x0A

#define DB0_SPAREOUTPUT1     0x0B

#define DB0_PWRCFG0           0x0C

#define DB0_PWRCFG1          0x0D

#define DB0_PWRCFG2          0x0E

#define DB0_PWRCFG3          0x0F

#define DB0_CLKCFG           0x10

#define DB0_BLOCKCFG         0x11

#define DB0_AFECFG           0x12

#define DB0_MASHCFG          0x13

#define DB0_PLLCFG           0x14

#define DB0_DACCFG           0x15

#define DB0_RXMIXERBIAS      0x18

#define DB0_RXLNABIAS        0x19

#define DB0_TXPABIAS         0x1A

#define DB0_TXOOKBIAS        0x1B

#define DB0_TXPREAMPBIAS     0x1C

#define DB0_PLLBIAS0         0x1D

#define DB0_TRXIQBIAS        0x1E

#define DB0_PLLBIAS1         0x20

#define DB0_PLLBIAS2         0x21

#define DB0_ADCBIAS          0x22

#define DB0_PGABIAS          0x23

#define DB0_PGAACBIAS        0x24

#define DB0_PWDETBIAS        0x25

#define DB0_IBG0             0x26

#define DB0_IBG1             0x27

#define DB0_IBG2             0x28

#define DB0_IBG3             0x29

#define DB0_IBGCFG           0x2A

#define DB0_XTALBIASCFG      0x2B

#define DB0_VCOBIASCFG       0x2C

#define DB0_SCLAMP           0x2D

#define DB0_VCORXCFG         0x30

#define DB0_VCOTXCFG         0x31

#define DB0_IQCORRCFG        0x33

#define DB0_OOKCFG0          0x34

#define DB0_OOKCFG1          0x35

#define DB0_OOKCFG2          0x36

#define DB0_TAUCFG0          0x37

#define DB0_TAUCFG1          0x38

#define DB0_TAUCFG2          0x39

#define DB0_TAUCFG3          0x3A

#define DB0_FSKFCRAMP1       0x3B
#define DB0_FSKFCRAMP2       0x3C
#define DB0_FSKFCRAMP3       0x3D

#define DB0_ISICFG0          0x3E

#define DB0_ISICFG1          0x3F

#define DB0_DELAYCFG         0x40


#define DB0_PREAMBLE         0x41


#define DB0_LFCFG0           0x44

#define DB0_LFCFG1           0x45

#define DB0_LFCFG2           0x46

#define DB0_LFCFG3           0x47















