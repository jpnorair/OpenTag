/* Copyright 2014 JP Norair
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
  * @file       /otradio/spirit1/SPIRIT1_override.h
  * @author     JP Norair
  * @version    R100
  * @date       4 Feb 2014
  * @brief      Register Overrides for SPIRIT1
  * @ingroup    SPIRIT1
  *
  ******************************************************************************
  */

#ifndef __SPIRIT1_override_H
#define __SPIRIT1_override_H



///Select (uncomment) one of these basic settings

// Well tested Ultra-low-rate modulations
//#define _10kHz_MSK
//#define _11k1Hz_MSK
#define _13k24Hz_MSK        // (13560 / 1024) kHz

// Out-of-favor low/mid modulations
//#define _20kHz_MSK
//#define _26k48Hz_MSK        // (13560 / 512) kHz

// Mid rate modulations
//#define _52k97Hz_MSK        // (13560 / 256) kHz
#define _52k97Hz_FSK10        // (13560 / 256) kHz ***
//#define _52k97Hz_FSK18
//#define _52k97Hz_FSK20
//#define _52k97Hz_FSK24
//#define _52k97Hz_FSK30

//#define _70k63Hz_MSK        // (13560 / 192) KHz

// Upper rate Modulations
#define _105k94Hz_MSK       // (13560 / 128) kHz
//#define _211k88Hz_MSK       // (13560 / 64) kHz


///Select one of these DSSS settings, if you want DSSS
//#define _DSSS_5





// Change TX FIFO Threshold when doing DSSS (normally = 5).  Depending on your
// DSSS length, it will need to be larger
#ifdef _DSSS_5
#   undef DRF_FIFO_CONFIG0
#   define DRF_FIFO_CONFIG0        10
#endif






// Special Data Rate Settings

#ifdef _105k94Hz_MSK
#   define _DR_M_HS_24MHz       33          // 105834.96
#   define _DR_E_HS_24MHz       12
#   define _MODULATION_HS       _MOD_TYPE_MSK
#   define _CHFLT_M_HS_24MHz    (7<<4)      //270 kHz
#   define _CHFLT_E_HS_24MHz    (1<<0)
#endif

#ifdef _70k63Hz_MSK
#   define _DR_M_LS_24MHz       130         // 70680.96
#   define _DR_E_LS_24MHz       11
#   define _MODULATION_LS       _MOD_TYPE_MSK
#   define _CHFLT_M_LS_24MHz    (2<<4)      // 200 kHz  
#   define _CHFLT_E_LS_24MHz    (2<<0)
#endif

#ifdef _52k97Hz_MSK
#   define _DR_M_LS_24MHz       33          // 52917.48
#   define _DR_E_LS_24MHz       11
#   define _MODULATION_LS       _MOD_TYPE_MSK
#   define _CHFLT_M_LS_24MHz    (4<<4)      //166 kHz
#   define _CHFLT_E_LS_24MHz    (2<<0)

#   define _FDEV_E_LS_48MHz     (4<<4)      //20 kHz (token)
#   define _FDEV_M_LS_48MHz     6
#endif

#ifdef _52k97Hz_FSK10
#   define _DR_M_LS_24MHz       33          // 52917.48
#   define _DR_E_LS_24MHz       11
#   define _MODULATION_LS       _MOD_TYPE_2FSK  //(_BT_SEL_1  | _MOD_TYPE_GFSK)
#   define _CHFLT_M_LS_24MHz    (7<<4)      //7=136, 6=150, 5=157, 4=166
#   define _CHFLT_E_LS_24MHz    (2<<0)
#   define _FDEV_E_LS_48MHz     (5<<4)      // 26.367 kHz (roughly 26.484)
#   define _FDEV_M_LS_48MHz     1           // (can also try 2)
#   define _FDEV_E_LS_50MHz     (5<<4)
#   define _FDEV_M_LS_50MHz     1
#endif

#ifdef _52k97Hz_FSK18
#   define _DR_M_LS_24MHz       33          // 52917.48
#   define _DR_E_LS_24MHz       11
#   define _MODULATION          _MOD_TYPE_2FSK
#   define _CHFLT_M_LS_24MHz    (0<<4)      // 4=166, 3=175, 2=185, 1=196, 0=207
#   define _CHFLT_E_LS_24MHz    (2<<0)
#   define _FDEV_E_LS_48MHz     (6<<4)      // 46.875 kHz (roughly 47.672)
#   define _FDEV_M_LS_48MHz     0 
#   define _FDEV_E_LS_50MHz     (6<<4)
#   define _FDEV_M_LS_50MHz     0
#endif

#ifdef _52k97Hz_FSK20
#   define _DR_M_LS_24MHz       33          // 52917.48
#   define _DR_E_LS_24MHz       11
#   define _MODULATION_LS       _MOD_TYPE_2FSK
#   define _CHFLT_M_LS_24MHz    (0<<4)      // 4=166, 3=175, 2=185, 1=196, 0=207
#   define _CHFLT_E_LS_24MHz    (2<<0)
#   define _FDEV_E_LS_48MHz     (6<<4)      // 52.734 kHz (roughly 52.97)
#   define _FDEV_M_LS_48MHz     3
#   define _FDEV_E_LS_50MHz     (6<<4)
#   define _FDEV_M_LS_50MHz     2
#endif

#ifdef _52k97Hz_FSK24
#   define _DR_M_LS_24MHz       33          // 52917.48
#   define _DR_E_LS_24MHz       11
#   define _MODULATION_LS       _MOD_TYPE_2FSK
#   define _CHFLT_M_LS_24MHz    (0<<4)      // 4=166, 3=175, 2=185, 1=196, 0=207
#   define _CHFLT_E_LS_24MHz    (2<<0)
#   define _FDEV_E_LS_48MHz     (6<<4)      // 64.453 kHz (roughly 63.563 kHz)
#   define _FDEV_M_LS_48MHz     1
#   define _FDEV_E_LS_50MHz     (6<<4)
#   define _FDEV_M_LS_50MHz     1
#endif

#ifdef _52k97Hz_FSK30
#   define _DR_M_LS_24MHz       33          // 52917.48
#   define _DR_E_LS_24MHz       11
#   define _MODULATION_LS       _MOD_TYPE_2FSK
#   define _CHFLT_M_LS_24MHz    (8<<4)      // 8=250, 7=272, 6=300, 5=315
#   define _CHFLT_E_LS_24MHz    (1<<0)
#   define _FDEV_E_LS_48MHz     (6<<4)      // 76.172 kHz (roughly 79.45)
#   define _FDEV_M_LS_48MHz     5
#   define _FDEV_E_LS_50MHz     (6<<4)
#   define _FDEV_M_LS_50MHz     5
#endif




#ifdef _26k48Hz_MSK
#   define _DR_M_SS_24MHz       33          // 26458.74
#   define _DR_E_SS_24MHz       10
#   define _MODULATION_SS       _MOD_TYPE_MSK
#   define _CHFLT_M_SS_24MHz    (5<<4)      // 79 kHz
#   define _CHFLT_E_SS_24MHz    (3<<0)   
#endif

// 25kHz
//#define _DR_M_SS_24MHz      17      // 24993.9
//#define _DR_E_SS_24MHz      10

#ifdef _20kHz_MSK
#   define _DR_M_SS_24MHz       181         
#   define _DR_E_SS_24MHz       9
#   define _MODULATION_SS       _MOD_TYPE_MSK
#   define _CHFLT_M_SS_24MHz    (8<<4)      // 62 kHz
#   define _CHFLT_E_SS_24MHz    (3<<0)
#   define _CHFLT_M_SS_25MHz    (8<<4)
#   define _CHFLT_E_SS_25MHz    (3<<0)
#endif  

// (13.56 / 1024)
#ifdef _13k24Hz_MSK
#   define _DR_M_SS_24MHz       32  //33          // 13229.37 (13183.6 works more closely for MSK)
#   define _DR_E_SS_24MHz       9
#   define _MODULATION_SS       _MOD_TYPE_MSK
#   define _CHFLT_M_SS_24MHz    (1<<4)      // 49 kHz
#   define _CHFLT_E_SS_24MHz    (4<<0)
#   define _FDEV_E_SS_48MHz     (4<<4)      //20kHz (token)
#   define _FDEV_M_SS_48MHz     6
#endif

// 11.11 kbps
#ifdef _11k1Hz_MSK
#   define _DR_M_SS_24MHz       230         // 11123.66
#   define _DR_E_SS_24MHz       8
#   define _MODULATION_SS       _MOD_TYPE_MSK
#   define _CHFLT_M_SS_24MHz    (2<<4)      // 
#   define _CHFLT_E_SS_24MHz    (4<<0)
#   define _FDEV_E_SS_48MHz     (4<<4)      //20kHz (token)
#   define _FDEV_M_SS_48MHz     6
#endif

// 10 kbps
#ifdef _10kHz_MSK
#   define _DR_M_SS_24MHz       181         
#   define _DR_E_SS_24MHz       8
#   define _DR_M_SS_25MHz       163         
#   define _DR_E_SS_25MHz       8
#   define _MODULATION_SS       _MOD_TYPE_MSK
#   define _CHFLT_M_SS_24MHz    (8<<4)      // 37 kHz
#   define _CHFLT_E_SS_24MHz    (4<<0)
#   define _CHFLT_M_SS_25MHz    (8<<4)      // 38 kHz
#   define _CHFLT_E_SS_25MHz    (4<<0)
#endif





// Custom Modulation & Data Rate Type Settings


#ifdef _DR_M_HS_24MHz
#   undef   DRF_CHFLT_HS
#   undef   DRF_MOD1_HS
#   undef   DRF_MOD0_HS
#   undef   DRF_FDEV0
#   define  DRF_MOD1_HS     _DR_M_HS_24MHz
#   define  DRF_MOD0_HS     (_MODULATION_HS | _DR_E_HS_24MHz)
#   define  DRF_FDEV0       (_FDEV_E_HS_48MHz | 0 | _FDEV_M_HS_48MHz)   // _CLOCK_REC_ALGO_SEL     //PLL
#   define  DRF_CHFLT_HS    (_CHFLT_M_HS_24MHz | _CHFLT_E_HS_24MHz)
#endif

#ifdef _DR_M_LS_24MHz
#   undef   DRF_CHFLT_LS
#   undef   DRF_MOD1_LS
#   undef   DRF_MOD0_LS
#   undef   DRF_FDEV0
#   define  DRF_MOD1_LS     _DR_M_LS_24MHz
#   define  DRF_MOD0_LS     (_MODULATION_LS | _DR_E_LS_24MHz)
#   define  DRF_FDEV0       (_FDEV_E_LS_48MHz | 0 | _FDEV_M_LS_48MHz)   // _CLOCK_REC_ALGO_SEL     //PLL
#   define  DRF_CHFLT_LS    (_CHFLT_M_LS_24MHz | _CHFLT_E_LS_24MHz)
#endif

#ifdef _DR_M_SS_24MHz
#   undef   DRF_CHFLT_SS
#   undef   DRF_MOD1_SS
#   undef   DRF_MOD0_SS
#   undef   DRF_FDEV0
#   define  DRF_MOD1_SS     _DR_M_SS_24MHz
#   define  DRF_MOD0_SS     (_MODULATION_SS | _DR_E_SS_24MHz)
#   define  DRF_FDEV0       (_FDEV_E_SS_48MHz | 0 | _FDEV_M_SS_48MHz)   // _CLOCK_REC_ALGO_SEL     //PLL
#   define  DRF_CHFLT_SS    (_CHFLT_M_SS_24MHz | _CHFLT_E_SS_24MHz)
#endif



#undef DRF_MOD1
#define DRF_MOD1        DRF_MOD1_LS

#undef DRF_MOD0
#define DRF_MOD0        DRF_MOD0_LS



// Special Frequency Deviation Settings, & Clock Recovery

// 50 kHz: normal setting

// 29 kHz
//#   define _FDEV_E_SS_48MHz     (5<<4)
//#   define _FDEV_M_SS_48MHz     2

// 20 kHz
//#   define _FDEV_E_SS_48MHz     (4<<4)
//#   define _FDEV_M_SS_48MHz     6
//#   define _FDEV_E_SS_50MHz     (4<<4)
//#   define _FDEV_M_SS_50MHz     6

// 5 kHz
//#   define _FDEV_E_SS_48MHz     (6<<4)
//#   define _FDEV_M_SS_48MHz     2
//#   define _FDEV_E_SS_50MHz     (6<<4)
//#   define _FDEV_M_SS_50MHz     2

// 2.4 kHz 
//#   define _FDEV_E_SS_48MHz     (1<<4)
//#   define _FDEV_M_SS_48MHz     5           


//#define _CLKREC_METHOD      0                       //DLL (default)
//#define _CLKREC_METHOD      _CLOCK_REC_ALGO_SEL     //PLL




//#undef DRF_FDEV0
//#if defined(_48MHz)
//#   define DRF_FDEV0     (_FDEV_E_SS_48MHz | _CLKREC_METHOD | _FDEV_M_SS_48MHz)
//#elif defined(_50MHz)
//#   define DRF_FDEV0     (_FDEV_E_SS_50MHz | _CLKREC_METHOD | _FDEV_M_SS_50MHz)
//#endif 


// Clock Recovery Customizations

//#define RF_CLOCKREC             0x23
//#   define _CLK_REC_P_GAIN      (7<<5)
//#   define __CLK_REC_P_GAIN(VAL) ((VAL&7)<<5)
//#   define _PSTFLT_LEN          (1<<4)
//#   define _PSTFLT_LEN_8        (0<<4)
//#   define _PSTFLT_LEN_16       (1<<4)
//#   define _CLK_REC_I_GAIN      (15<<0)
//#   define __CLK_REC_I_GAIN(VAL) (VAL&15)

//R23
// Using DLL clock recovery, higher P-Gain values allow better noise filtering,
// but they also require longer preambles.

// I've used __CLK_REC_P_GAIN(5) in the past... might have small improvement
// over __CLK_REC_P_GAIN(4), but merits further testing
#undef DRF_CLOCKREC
#define DRF_CLOCKREC            (__CLK_REC_P_GAIN(5) | _PSTFLT_LEN | __CLK_REC_I_GAIN(8))











// Channel Filtering Customizations
//R1D: It is set in the driver when picking a channel











// AFC Customizations
//R1E 
#undef DRF_AFC2
#define DRF_AFC2                ( _AFC_FREEZE_ON_SYNC | _AFC_ENABLED | \
                                    _AFC_MODE_CLOSE_ON_STAGE2 /*_AFC_MODE_CLOSE_ON_SLICER*/ | __AFC_PD_LEAKAGE(8))
//R1F 
//#define DRF_AFC1                __AFC_FAST_PERIOD(24)

//R20 (using power-up defaults)
//#define DRF_AFC0                (__AFC_FAST_GAIN(2) | __AFC_SLOW_GAIN(5))



///@note: - with 16 bit sync, PQI_TH must be 3 in order to get decent results
///       - with 24 bit sync, PQI_TH can be 2
///       - with 32 bit sync, PQI_TH can be 1 or 2
#undef DRF_QI
#define DRF_QI                  (__SQI_TH(3) | __PQI_TH(2) | _SQI_EN | _PQI_EN)


#undef DRF_SYNC_BYTES
#undef _SYNC_BYTES
#define DRF_SYNC_BYTES          4
#define _SYNC_BYTES             ((DRF_SYNC_BYTES-1) << 1)

#undef DRF_PREAMBLE_BYTES
#define DRF_PREAMBLE_BYTES      10

#undef DRF_PCKTCTRL2_LSFG
#undef DRF_PCKTCTRL2_LSBG
#define DRF_PCKTCTRL2_LSFG      (__PREAMBLE_LENGTH((DRF_PREAMBLE_BYTES-1)) | _SYNC_BYTES)
#define DRF_PCKTCTRL2_LSBG      (__PREAMBLE_LENGTH((DRF_PREAMBLE_BYTES-1)) | _SYNC_BYTES)




// RSSI Filtering customizations

//R21 (using power-up defaults)
//#define DRF_RSSI_FLT            (__RSSI_FLT(14) | _CS_MODE_STATIC | 0)










// AGC Customizations

//#define RF_AGCCTRL0             0x26
//#   define _AGC_ENABLE          (1<<7)

//#define RF_ANT_SELECT_CONF      0x27
//#   define _CS_BLANKING         (1<<4)      // make sure not enabled
//#   define _AS_ENABLE           (1<<3)
//#   define _AS_MEAS_TIME        (7<<0)
//#   define __AS_MEAS_TIME(VAL)  (7&VAL)



//R24 (using power-up defaults)
///@todo __MEAS_TIME(2) = 2us AGC clock.  Might be worth trying __MEAS_TIME(3) = 4us
//#define DRF_AGCCTRL2             (0x20 | __MEAS_TIME(2))

//R25
//#define DRF_AGCCTRL1            (__THRESHOLD_HI(6) | __THRESHOLD_LO(2))

//R26 (using power-up defaults)
//#define DRF_AGCCTRL0            (_AGC_ENABLE | 0x0A)







// VCO Calibration Customizations

//RA1 (using power-up defaults)
// Value 17 (0x11) is sufficient to drive TX or RX once calibrated, but
// it is not sufficient to do so for calibration.  Use 25 (0x19).
// REF: DocID023165 Rev 5 (Errata Sheet DM00053990.pdf, March 2013)
//#define DRF_VCO_CONFIG          __VCO_GEN_CURR(17)
//__VCO_GEN_CURR(25)

//R6D (using power-up defaults)
//#define DRF_RCO_VCO_CALIBR_IN2  (__RWT_IN(7) | __RFB_IN(0))

//R6E ***
//#define DRF_RCO_VCO_CALIBR_IN1  (__VCO_CALIBR_TX(b1000001))

//R6F ***
//#define DRF_RCO_VCO_CALIBR_IN0  (__VCO_CALIBR_RX(b1000010))






// SMPS Customizations
#undef DRF_PM_CONFIG0
#undef DRF_PM_CONFIG1
#undef DRF_PM_CONFIG2

// 2.4 MHz SMPS     (Crashed)
//#define DRF_PM_CONFIG2      (0)
//#define DRF_PM_CONFIG1      (_EN_RM | __KRM_HI(6))
//#define DRF_PM_CONFIG0      (__KRM_LO(102))

// 3.0 MHz SMPS     (Crashed)
//#define DRF_PM_CONFIG2      (0)
//#define DRF_PM_CONFIG1      (_EN_RM | __KRM_HI(8))
//#define DRF_PM_CONFIG0      (__KRM_LO(0))

// 3.2 MHz SMPS (3.33) : 2184
#define DRF_PM_CONFIG2      (0)
#define DRF_PM_CONFIG1      (_EN_RM | __KRM_HI(0x8))
#define DRF_PM_CONFIG0      (__KRM_LO(0x88))

// 3.3 MHz SMPS : 2258
//#define DRF_PM_CONFIG2      (0)
//#define DRF_PM_CONFIG1      (_EN_RM | __KRM_HI(0x8))
//#define DRF_PM_CONFIG0      (__KRM_LO(0xD2))

// 3.4 MHz SMPS : 2321
//#define DRF_PM_CONFIG2      (0)
//#define DRF_PM_CONFIG1      (_EN_RM | __KRM_HI(0x9))
//#define DRF_PM_CONFIG0      (__KRM_LO(0x11))

// 3.6 MHz SMPS : 2457
//#define DRF_PM_CONFIG2      (_SET_SMPS_VTUNE)
//#define DRF_PM_CONFIG1      (_EN_RM | __KRM_HI(0x9))
//#define DRF_PM_CONFIG0      (__KRM_LO(0x99))

// 3.8 MHz SMPS : 2594
//#define DRF_PM_CONFIG2      (_SET_SMPS_VTUNE)
//#define DRF_PM_CONFIG1      (_EN_RM | __KRM_HI(0xA))
//#define DRF_PM_CONFIG0      (__KRM_LO(0x22))

// 4.0 MHz SMPS     (Best result: set to 0)
//#define DRF_PM_CONFIG2      0  //(_SET_SMPS_VTUNE)           //can also be (0)
//#define DRF_PM_CONFIG1      (_EN_RM | __KRM_HI(0xA))
//#define DRF_PM_CONFIG0      (__KRM_LO(0xAA))         

// 5.0 MHz SMPS
//#define DRF_PM_CONFIG2      (_SET_SMPS_PLLBW | _SET_SMPS_VTUNE)           //can also be (_SET_SMPS_PLLBW) or (_SET_SMPS_PLLBW | _SET_SMPS_VTUNE)
//#define DRF_PM_CONFIG1      (_EN_RM | __KRM_HI(13))
//#define DRF_PM_CONFIG0      (__KRM_LO(85)) 

// 6.0 MHz SMPS (default)
//#define DRF_PM_CONFIG2      (_SET_SMPS_VTUNE)           //can also be (_SET_SMPS_PLLBW) or (_SET_SMPS_PLLBW | _SET_SMPS_VTUNE)
//#define DRF_PM_CONFIG1      (_EN_RM | __KRM_HI(16))
//#define DRF_PM_CONFIG0      (__KRM_LO(0)) 


// This mysterious thing!

//#define RF_DEM_CONFIG           0xA3
//#   define _DEM_RESERVED        0x35
//#   define _DEM_ORDER           (1<<1)
//

//RA3
//#undef DRF_DEM_ORDER
//#define DRF_DEM_ORDER           (_DEM_RESERVED)
//





#endif
