/* Copyright 2013-2014 JP Norair
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
  * @file       /include/io/sx127x/sx1272_registers.h
  * @author     JP Norair
  * @version    R101
  * @date       4 Nov 2016
  * @brief      Register Addressing Constants for SX1272
  * @ingroup    SX127x
  *
  ******************************************************************************
  */

#ifndef __SX1272_registers_H
#define __SX1272_registers_H


/** Register call macros: mainly for nomenclature consistency with other TRX
  * drivers that have been implemented.
  */
#define RFREG(VAL)      (RFREG_##VAL)
#define STROBE(VAL)     (RFSTROBE_##VAL)
#define RFGPO(VAL)      ((RFGPO_##VAL << 3) | 2)


/** SX1272 Internal registers Address
  * ============================================================================
  */
#define RFREG_LR_FIFO                               0x00 

#define RFREG_LR_OPMODE                             0x01 
#   define _LORAMODE                                (1<<7)
#   define _FSKMODE                                 (0<<7)
#   define _ACCESS_SHARED_REG                       (1<<6)
#   define _OPMODE                                  (7<<0)
#   define __OPMODE(VAL)                            ((7&(VAL))<<0)
#   define _OPMODE_SLEEP                            (0<<0)
#   define _OPMODE_STANDBY                          (1<<0)
#   define _OPMODE_FSTX                             (2<<0)
#   define _OPMODE_TX                               (3<<0)
#   define _OPMODE_FSRX                             (4<<0)
#   define _OPMODE_RXCONT                           (5<<0) 
#   define _OPMODE_RXSINGLE                         (6<<0)
#   define _OPMODE_CAD                              (7<<0)             


// Frequency Select bytes
// Some preconfigured frequency selection is done on 125kHz spacing in ERC 860 
// band, and 500 kHz spacing in US 915 MHz band.  Also, due to 32 MHz crystal, 
// it is inadvisable to use frequencies in the following ranges:
// - 863-865 MHz (ERC SRD)
// - 927-928 (FCC ISM)
#define RFREG_LR_FRFMSB                             0x06 
#define RFREG_LR_FRFMIB                             0x07
#define RFREG_LR_FRFLSB                             0x08
#define _FRFMSB_915_MHZ                             0xE4  // Default
#define _FRFMID_915_MHZ                             0xC0  // Default
#define _FRFLSB_915_MHZ                             0x00  // Default
#define _FRF_865MHz         0xD84000
#define _FRF_865M125Hz      0xD84800
#define _FRF_865M250Hz      0xD85000
#define _FRF_865M375Hz      0xD85800
#define _FRF_865M500Hz      0xD86000
#define _FRF_865M625Hz      0xD86800
#define _FRF_865M750Hz      0xD87000
#define _FRF_865M875Hz      0xD87800
#define _FRF_866MHz         0xD88000
#define _FRF_866M125Hz      0xD88800
#define _FRF_866M250Hz      0xD89000
#define _FRF_866M375Hz      0xD89800
#define _FRF_866M500Hz      0xD8A000
#define _FRF_866M625Hz      0xD8A800
#define _FRF_866M750Hz      0xD8B000
#define _FRF_866M875Hz      0xD8B800
#define _FRF_867MHz         0xD8C000
#define _FRF_867M125Hz      0xD8C800
#define _FRF_867M250Hz      0xD8D000
#define _FRF_867M375Hz      0xD8D800
#define _FRF_867M500Hz      0xD8E000
#define _FRF_867M625Hz      0xD8E800
#define _FRF_867M750Hz      0xD8F000
#define _FRF_867M875Hz      0xD8F800
#define _FRF_868MHz         0xD90000


// Tx settings
#define RFREG_LR_PACONFIG                           0x09
#   define _PABOOST                                 (1<<7)
#   define _PANORMAL                                (0<<7)
#   define _MAX_PWR_10dBm8                          (0<<4)
#   define _MAX_PWR_11dBm4                          (1<<4)
#   define _MAX_PWR_12dBm0                          (2<<4)
#   define _MAX_PWR_12dBm6                          (3<<4)
#   define _MAX_PWR_13dBm2                          (4<<4)
#   define _MAX_PWR_13dBm8                          (5<<4)
#   define _MAX_PWR_14dBm4                          (6<<4)
#   define _MAX_PWR_15dBm0                          (7<<4)
#   define _MAX_PWR(VAL)                            ((7&(VAL))<<4)
#   define _OUTPUT_PWR                              (15<<0)
#   define __OUTPUT_PWR(VAL)                        ((15&(VAL))<<0)

#define RFREG_LR_PARAMP                             0x0A
#   define _LOW_PN_TX_PLLOFF                        (1<<4)
#   define _LOW_PN_TX_PLLON                         (0<<4)
#   define _PA_RAMP                                 (15<<0)
#   define __PA_RAMP(VAL)                           ((15&(VAL))<<0)
#   define _PA_RAMP_3m4s                            (0)
#   define _PA_RAMP_2ms                             (1)
#   define _PA_RAMP_1ms                             (2)
#   define _PA_RAMP_500us                           (3)
#   define _PA_RAMP_250us                           (4)
#   define _PA_RAMP_125us                           (5)
#   define _PA_RAMP_100us                           (6)
#   define _PA_RAMP_62us                            (7)
#   define _PA_RAMP_50us                            (8)
#   define _PA_RAMP_40us                            (9)
#   define _PA_RAMP_31us                            (10)
#   define _PA_RAMP_25us                            (11)
#   define _PA_RAMP_20us                            (12)
#   define _PA_RAMP_15us                            (13)
#   define _PA_RAMP_12us                            (14)
#   define _PA_RAMP_10us                            (15)

#define RFREG_LR_OCP                                0x0B 
#   define _OCP_ON                                  (1<<5)
#   define _OCP_OFF                                 (0<<5)
#   define _OCP_TRIM                                (31<<0)
#   define __OCP_TRIM(VAL)                          ((31&(VAL))<<0)
#   define _OCP_TRIM_45mA                           0x00
#   define _OCP_TRIM_50mA                           0x01   
#   define _OCP_TRIM_55mA                           0x02 
#   define _OCP_TRIM_60mA                           0x03 
#   define _OCP_TRIM_65mA                           0x04 
#   define _OCP_TRIM_70mA                           0x05 
#   define _OCP_TRIM_75mA                           0x06 
#   define _OCP_TRIM_80mA                           0x07  
#   define _OCP_TRIM_85mA                           0x08
#   define _OCP_TRIM_90mA                           0x09 
#   define _OCP_TRIM_95mA                           0x0A 
#   define _OCP_TRIM_100mA                          0x0B  // Default
#   define _OCP_TRIM_105mA                          0x0C 
#   define _OCP_TRIM_110mA                          0x0D 
#   define _OCP_TRIM_115mA                          0x0E 
#   define _OCP_TRIM_120mA                          0x0F 
#   define _OCP_TRIM_130mA                          0x10
#   define _OCP_TRIM_140mA                          0x11   
#   define _OCP_TRIM_150mA                          0x12 
#   define _OCP_TRIM_160mA                          0x13 
#   define _OCP_TRIM_170mA                          0x14 
#   define _OCP_TRIM_180mA                          0x15 
#   define _OCP_TRIM_190mA                          0x16 
#   define _OCP_TRIM_200mA                          0x17  
#   define _OCP_TRIM_210mA                          0x18
#   define _OCP_TRIM_220mA                          0x19 
#   define _OCP_TRIM_230mA                          0x1A 
#   define _OCP_TRIM_240mA                          0x1B


// Rx settings
#define RFREG_LR_LNA                                0x0C 
#   define _LNA_GAIN                                (7<<5)
#   define __LNA_GAIN(VAL)                          ((7&(VAL))<<5)
#   define _LNA_GAIN_G1                             (1<<5)
#   define _LNA_GAIN_G2                             (2<<5)
#   define _LNA_GAIN_G3                             (3<<5)
#   define _LNA_GAIN_G4                             (4<<5)
#   define _LNA_GAIN_G5                             (5<<5)
#   define _LNA_GAIN_G6                             (6<<5)
#   define _LNA_BOOST_ON                            (3<<0)
#   define _LNA_BOOST_OFF                           (0<<0)


// LoRa FIFO Registers
#define RFREG_LR_FIFOADDRPTR                        0x0D 
#define RFREG_LR_FIFOTXBASEADDR                     0x0E 
#define RFREG_LR_FIFORXBASEADDR                     0x0F 
#define RFREG_LR_FIFORXCURRENTADDR                  0x10 


// IRQ Flags & Flag Mask
#define RFREG_LR_IRQFLAGSMASK                       0x11 
#   define _RXTIMEOUT_MASK                          0x80 
#   define _RXDONE_MASK                             0x40 
#   define _PAYLOADCRCERROR_MASK                    0x20 
#   define _VALIDHEADER_MASK                        0x10 
#   define _TXDONE_MASK                             0x08 
#   define _CADDONE_MASK                            0x04 
#   define _FHSSCHANGEDCHANNEL_MASK                 0x02 
#   define _CADDETECTED_MASK                        0x01 

#define RFREG_LR_IRQFLAGS                           0x12 
#   define _RXTIMEOUT                               0x80 
#   define _RXDONE                                  0x40 
#   define _PAYLOADCRCERROR                         0x20 
#   define _VALIDHEADER                             0x10 
#   define _TXDONE                                  0x08 
#   define _CADDONE                                 0x04 
#   define _FHSSCHANGEDCHANNEL                      0x02 
#   define _CADDETECTED                             0x01 



#define RFREG_LR_RXNBBYTES                          0x13 
#define RFREG_LR_RXHEADERCNTVALUEMSB                0x14 
#define RFREG_LR_RXHEADERCNTVALUELSB                0x15 
#define RFREG_LR_RXPACKETCNTVALUEMSB                0x16 
#define RFREG_LR_RXPACKETCNTVALUELSB                0x17 


// Modem Status
#define RFREG_LR_MODEMSTAT                          0x18 
#   define _RX_CR_MASK                              0x1F 
#   define _MODEM_STATUS_MASK                       0xE0 


#define RFREG_LR_PKTSNRVALUE                        0x19 
#define RFREG_LR_PKTRSSIVALUE                       0x1A 
#define RFREG_LR_RSSIVALUE                          0x1B 


// Channel Hopping Configuration
#define RFREG_LR_HOPCHANNEL                         0x1C 
#   define _PLL_LOCK_TIMEOUT_MASK                   0x7F 
#   define _PLL_LOCK_FAIL                           0x80 
#   define _PLL_LOCK_SUCCEED                        0x00 // Default                                                
#   define _CRCONPAYLOAD_MASK                       0xBF
#   define _CRCONPAYLOAD_ON                         0x40
#   define _CRCONPAYLOAD_OFF                        0x00 // Default
#   define _CHANNEL_MASK                            0x3F 


// Modem Config 1 & 2
#define RFREG_LR_MODEMCONFIG1                       0x1D
#   define _BW_MASK                                 0x3F 
#   define _BW_125_KHZ                              0x00 // Default
#   define _BW_250_KHZ                              0x40 
#   define _BW_500_KHZ                              0x80 
#   define _CODINGRATE_MASK                         0xC7 
#   define _CODINGRATE_4_5                          0x08
#   define _CODINGRATE_4_6                          0x10 // Default
#   define _CODINGRATE_4_7                          0x18 
#   define _CODINGRATE_4_8                          0x20 
#   define _IMPLICITHEADER_MASK                     0xFB 
#   define _IMPLICITHEADER_ON                       0x04 
#   define _IMPLICITHEADER_OFF                      0x00 // Default
#   define _RXPAYLOADCRC_MASK                       0xFD 
#   define _RXPAYLOADCRC_ON                         0x02 
#   define _RXPAYLOADCRC_OFF                        0x00 // Default
#   define _LOWDATARATEOPTIMIZE_MASK                0xFE 
#   define _LOWDATARATEOPTIMIZE_ON                  0x01 
#   define _LOWDATARATEOPTIMIZE_OFF                 0x00 // Default

#define RFREG_LR_MODEMCONFIG2                       0x1E 
#   define _SF_MASK                                 0x0F 
#   define _SF_6                                    0x60 
#   define _SF_7                                    0x70 // Default
#   define _SF_8                                    0x80 
#   define _SF_9                                    0x90 
#   define _SF_10                                   0xA0 
#   define _SF_11                                   0xB0 
#   define _SF_12                                   0xC0 
#   define _TX_CONT_ON                              (1<<3)
#   define _TX_CONT_OFF                             (0<<3)
#   define _AGC_AUTO_ON                             (1<<2)
#   define _AGC_AUTO_OFF                            (0<<2)
#   define _SYMBTIMEOUT_MSB                         (3<<0)
#   define __SYMBTIMEOUT_MSB(VAL)                   ((3&(VAL))<<0)


#define RFREG_LR_SYMBTIMEOUTLSB                     0x1F 
#define RFREG_LR_PREAMBLEMSB                        0x20 
#define RFREG_LR_PREAMBLELSB                        0x21 
#define RFREG_LR_PAYLOADLENGTH                      0x22 
#define RFREG_LR_PAYLOADMAXLENGTH                   0x23 
#define RFREG_LR_HOPPERIOD                          0x24 
#define RFREG_LR_FIFORXBYTEADDR                     0x25
#define RFREG_LR_FEIMSB                             0x28
#define RFREG_LR_FEIMID                             0x29
#define RFREG_LR_FEILSB                             0x2A
#define RFREG_LR_RSSIWIDEBAND                       0x2C



#define RFREG_LR_DETECTOPTIMIZE                     0x31
#   define _DETECT_MASK                             0xF8
#   define _DETECT_SF7_TO_SF12                      (0xC0 | 0x03) // Default
#   define _DETECT_SF6                              (0xC0 | 0x05)



#define RFREG_LR_INVERTIQ                           0x33
#   define _INVERTIQ_RX_MASK                        0xBF
#   define _INVERTIQ_RX_OFF                         0x00
#   define _INVERTIQ_RX_ON                          0x40
#   define _INVERTIQ_TX_MASK                        0xFE
#   define _INVERTIQ_TX_OFF                         0x01
#   define _INVERTIQ_TX_ON                          0x00



#define RFREG_LR_DETECTIONTHRESHOLD                 0x37
#   define _THRESH_SF7_TO_SF12                      0x0A // Default
#   define _THRESH_SF6                              0x0C



#define RFREG_LR_SYNCWORD                           0x39



#define RFREG_LR_INVERTIQ2                          0x3B
#   define _INVERTIQ2_ON                            0x19
#   define _INVERTIQ2_OFF                           0x1D


// end of documented register in datasheet

// I/O settings
#define RFREG_LR_DIOMAPPING1                        0x40
#   define _DIO0_MASK                               0x3F
#   define _DIO0_00                                 0x00  // Default
#   define _DIO0_01                                 0x40
#   define _DIO0_10                                 0x80
#   define _DIO0_11                                 0xC0
#   define _DIO1_MASK                               0xCF
#   define _DIO1_00                                 0x00  // Default
#   define _DIO1_01                                 0x10
#   define _DIO1_10                                 0x20
#   define _DIO1_11                                 0x30
#   define _DIO2_MASK                               0xF3
#   define _DIO2_00                                 0x00  // Default
#   define _DIO2_01                                 0x04
#   define _DIO2_10                                 0x08
#   define _DIO2_11                                 0x0C
#   define _DIO3_MASK                               0xFC
#   define _DIO3_00                                 0x00  // Default
#   define _DIO3_01                                 0x01
#   define _DIO3_10                                 0x02
#   define _DIO3_11                                 0x03
#   define _DIOMAPPING1_CAD                         (_DIO0_10 | _DIO1_10 | _DIO3_01)    //Cad-Done + Cad-Detect + Valid Header
#   define _DIOMAPPING1_BRX                         (_DIO0_00 | _DIO1_00 | _DIO3_10)
#   define _DIOMAPPING1_FRX                         (_DIO0_00 | _DIO1_00)
#   define _DIOMAPPING1_BTX                         (_DIO0_01)
#   define _DIOMAPPING1_FTX                         (_DIO0_01)
#   define _DIOMAPPING1_RX                          (_DIO0_00 | _DIO1_00 | _DIO3_01)    //RX-Done + RX-Timeout + Valid Header
#   define _DIOMAPPING1_TX                          (_DIO0_01)

#define RFREG_LR_DIOMAPPING2                        0x41
#   define _DIO4_MASK                               0x3F
#   define _DIO4_00                                 0x00  // Default
#   define _DIO4_01                                 0x40
#   define _DIO4_10                                 0x80
#   define _DIO4_11                                 0xC0
#   define _DIO5_MASK                               0xCF
#   define _DIO5_00                                 0x00  // Default
#   define _DIO5_01                                 0x10
#   define _DIO5_10                                 0x20
#   define _DIO5_11                                 0x30
#   define _MAP_MASK                                0xFE
#   define _MAP_PREAMBLEDETECT                      0x01
#   define _MAP_RSSI                                0x00  // Default



// Version
#define RFREG_LR_VERSION                            0x42


// Additional settings
#define RFREG_LR_AGCREF                             0x43
#define RFREG_LR_AGCTHRESH1                         0x44
#define RFREG_LR_AGCTHRESH2                         0x45
#define RFREG_LR_AGCTHRESH3                         0x46


#define RFREG_LR_PLLHOP                             0x4B
#   define _FASTHOP_MASK                            0x7F
#   define _FASTHOP_ON                              0x80
#   define _FASTHOP_OFF                             0x00 // Default


#define RFREG_LR_TCXO                               0x58
#   define _TCXOINPUT_MASK                          0xEF
#   define _TCXOINPUT_ON                            0x10
#   define _TCXOINPUT_OFF                           0x00  // Default


#define RFREG_LR_PADAC                              0x5A
#   define _PADAC_20DBM_ON                          0x17
#   define _PADAC_20DBM_OFF                         0x14  // Default


#define RFREG_LR_PLL                                0x5C
#   define _PLL_BW_MASK                             0x3F
#   define _PLL_BW_75                               0x00
#   define _PLL_BW_150                              0x40
#   define _PLL_BW_225                              0x80
#   define _PLL_BW_300                              0xC0  // Default


#define RFREG_LR_PLLLOWPN                           0x5E
#   define _PLLLOWPN_BW_MASK                        0x3F
#   define _PLLLOWPN_BW_75                          0x00
#   define _PLLLOWPN_BW_150                         0x40
#   define _PLLLOWPN_BW_225                         0x80
#   define _PLLLOWPN_BW_300                         0xC0  // Default


#define RFREG_LR_FORMERTEMP                         0x6C




#endif
