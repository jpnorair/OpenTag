/* Copyright 2010-2011 JP Norair
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
  * @file       /OTradio/CC430/CC430_defaults.h
  * @author     JP Norair
  * @version    V1.0
  * @date       29 Aug 2011
  * @brief      CC430 Radio register definitions & macros.
  * @defgroup   Radio (Radio Module)
  * @ingroup    Radio
  *
  ******************************************************************************
  */

#ifndef __CC430_defaults_H
#define __CC430_defaults_H

#include "OT_types.h"
#include "OT_config.h"


// Some radio register settings constants
// These are basically for the CC430, and the POSIX impl just ignores them
#ifndef RF_PARAM_BAND
#   define RF_PARAM_BAND    433
#endif

#if ( (RF_PARAM_BAND != 433) && \
	  (RF_PARAM_BAND != 866) )
#   error "RF_PARAM_BAND is not set to an implemented value"
#endif




///@note Achtung!  Do not change these GDO values, ever!
#define RADIO_GDO2_VALUE        0x29        // GDO2.GDO2_CFG (RF_Ready)
#define RADIO_GDO1_VALUE        0x1E        // GDO1.GDO1_CFG (RSSI_Valid)
#define RADIO_GDO0_VALUE        0x1F        // GDO0.GDO0_CFG (RX Timeout)

///@note for testing purposes, consider using RADIO_ATTENUATE at MAX
#define RADIO_ATTENUATE_OFF     (0 << 4)    // FIFOTHR.CLOSE_IN_RX (default)
#define RADIO_ATTENUATE_MAX     (3 << 4)    // FIFOTHR.CLOSE_IN_RX
#define RADIO_FIFO_THR          7           // FIFOTHR.FIFO_THR

#define RADIO_SYNC_0A           0xE6D0      // SYNC1/SYNC0
#define RADIO_SYNC_0B           0x0B67      // SYNC1/SYNC0
#define RADIO_SYNC_1A           0xF498      // SYNC1/SYNC0
#define RADIO_SYNC_1B           0x192F      // SYNC1/SYNC0

#define RADIO_PKTLEN_BF         7           // PKTLEN
#define RADIO_PKTLEN_FF         255         // PKTLEN (default)

#define RADIO_PKT_PQT           (1 << 5)    // PKTCTRL1.PQT must be at least 4 good bits
#define RADIO_PKT_STATUS        0           // PKTCTRL1.APPEND_STATUS

#define RADIO_PKT_PN9           (1 << 6)    // PKTCTRL0.WHITE_DATA is ON!
#define RADIO_PKT_CRC           (0)         // PKTCTRL0.CRC_EN is OFF!
#define RADIO_PKT_FIXED         (0)         // PKTCTRL0.LENGTH_CONFIG is fixed mode
#define RADIO_PKT_AUTO          (1)         // PKTCTRL0.LENGTH_CONFIG is variable (default)
#define RADIO_PKT_BUFFER        (2)         // PKTCTRL0.LENGTH_CONFIG is infinite

#define RADIO_CHAN              7           // CHANNR (default)

#define RADIO_FREQ_IF_NORMAL    0x06        // FSCTRL1.FREQ_IF
#define RADIO_FREQ_IF_TURBO     0x06        // FSCTRL1.FREQ_IF *

#define RADIO_FREQOFF           0           // FSCTRL0 (may be supplied after unit testing)

#define RADIO_FREQ_433          ((ot_u32)0x0010A8FF)   // pchan 1 fc = 433163949 Hz
#define RADIO_FREQ_866          ((ot_u32)0x00216061)   // pchan 1 fc = 866163483 Hz
#if (RF_PARAM_BAND == 433)
#   define     RADIO_FREQ2         (ot_u8)(RADIO_FREQ_433>>16 & 0xFF)
#   define     RADIO_FREQ1         (ot_u8)(RADIO_FREQ_433>>8 & 0xFF)
#   define     RADIO_FREQ0         (ot_u8)(RADIO_FREQ_433 & 0xFF)
#elif (RF_PARAM_BAND == 866)
#   define     RADIO_FREQ2         (ot_u8)(RADIO_FREQ_866>>16 & 0xFF)
#   define     RADIO_FREQ1         (ot_u8)(RADIO_FREQ_866>>8 & 0xFF)
#   define     RADIO_FREQ0         (ot_u8)(RADIO_FREQ_866 & 0xFF)
#endif

#define RADIO_FILTER_NORMAL     ((2<<6)|(1<<4)) // MDMCFG4.CHANBW_E & M (default)
#define RADIO_FILTER_TURBO      ((1<<6)|(1<<4)) // MDMCFG4.CHANBW_E & M
#define RADIO_DRATE_NORMAL_E    11          // MDMCFG4.DRATE_E (default)
#define RADIO_DRATE_TURBO_E     12          // MDMCFG4.DRATE_E

#define RADIO_DRATE_NORMAL_M    26          // MDMCFG3.DRATE_M (default)
#define RADIO_DRATE_TURBO_M     248         // MDMCFG3.DRATE_M

#define RADIO_DCFILT_NORMAL     (0 << 7)    // MDMCFG2.DEM_DCFILT_OFF (default)
#define RADIO_DCFILT_TURBO      (0 << 7)    // MDMCFG2.DEM_DCFILT_OFF
#define RADIO_MOD_NORMAL        (1 << 4)    // MDMCFG2.MOD_FORMAT (default)
#define RADIO_MOD_TURBO         (1 << 4)    // MDMCFG2.MOD_FORMAT
#define RADIO_SYNCMODE_PN9      (6)         // MDMCFG2.SYNC_MODE (default)
#define RADIO_SYNCMODE_FEC      (5)         // MDMCFG2.SYNC_MODE

#define RADIO_PREAMBLE_NORMAL   (2 << 4)    // MDMCFG1.NUM_PREAMBLE (default)
#define RADIO_PREAMBLE_TURBO    (3 << 4)    // MDMCFG1.NUM_PREAMBLE
#define RADIO_CHANNEL_SPC_E     (2)         // MDMCFG1.CHANSPC_E (107.94 kHz)

#define RADIO_CHANNEL_SPC_M     (16)        // MDMCFG0.CHANSPC_M (107.94 kHz)

#define RADIO_DEV               ((5<<4)|0)  // DEVIATN (default, FSK 50 kHz)

#define RADIO_RXCS_ENABLE       (1 << 4)    // MCSM2.RX_TIME_RSSI (sed for bf)
#define RADIO_RXCS_DISABLE      (0 << 4)    // MCSM2.RX_TIME_RSSI (used for ff)
#define RADIO_RXTO_ENABLE       (0)         // MCSM2.RX_TIME (the EVENT0 timeout needs to be adjusted to suit)
#define RADIO_RXTO_DISABLE      (7)         // MCSM2.RX_TIME

#define RADIO_CCA_UNGUARDED     (0 << 4)    // MCSM1.CCA_MODE 
#define RADIO_CCA_GUARDED       (1 << 4)    // MCSM1.CCA_MODE (default)
#define RADIO_RXOFF_IDLE        (0 << 2)    // MCSM1.CCA_MODE (default)
#define RADIO_RXOFF_FSTXON      (1 << 2)    // MCSM1.CCA_MODE
#define RADIO_RXOFF_TX          (2 << 2)    // MCSM1.CCA_MODE
#define RADIO_RXOFF_RX          (3 << 2)    // MCSM1.CCA_MODE
#define RADIO_TXOFF_IDLE        (0 << 2)    // MCSM1.CCA_MODE (default)
#define RADIO_TXOFF_FSTXON      (1 << 2)    // MCSM1.CCA_MODE
#define RADIO_TXOFF_TX          (2 << 2)    // MCSM1.CCA_MODE
#define RADIO_TXOFF_RX          (3 << 2)    // MCSM1.CCA_MODE

#define RADIO_AUTOCAL_OFF       (0 << 4)    // MCSM0.AUTOCAL
#define RADIO_AUTOCAL_STARTUP   (1 << 4)    // MCSM0.AUTOCAL
#define RADIO_AUTOCAL_SHUTDOWN  (2 << 4)    // MCSM0.AUTOCAL
#define RADIO_AUTOCAL_SHUTDOWN4 (3 << 4)    // MCSM0.AUTOCAL (default)

#define RADIO_FOCCFG            (22)        // FOCCFG (default)

#define RADIO_BSCFG             (108)       // BSCFG (default)

#define RADIO_DVGA_GAIN         (0 << 6)    // AGCCTRL2.MAX_DVGA_GAIN
#define RADIO_LNA_GAIN          (0 << 3)    // AGCCTRL2.MAX_LNA_GAIN
#define RADIO_MAGN_TARGET       (3)         // AGCCTRL2.MAGN_TARGET (check this)

#define RADIO_LNA_PRIORITY      (1 << 6)    // AGCCTRL1.MAX_DVGA_GAIN 
#define RADIO_CSRT              (0 << 4)    // AGCCTRL1.CARRIER_SENSE_REL_THR 
#define RADIO_CSAT              (0)         // AGCCTRL1.CARRIER_SENSE_ABS_THR 

#define RADIO_HYST_LEVEL        (2 << 6)    // AGCCTRL0.HYST_LEVEL (SRFS == 2)
#define RADIO_WAIT_TIME         (1 << 4)    // AGCCTRL0.WAIT_TIME (SRFS == 1)
#define RADIO_AGC_FREEZE        (0 << 2)    // AGCCTRL0.AGC_FREEZE (SRFS == 0)
#define RADIO_FILTER_LEN        (1)         // AGCCTRL0.FILTER_LENGTH (SRFS == 1)

#define RADIO_WOREVT1           (128)       // WOREVT1 (dynamic)
#define RADIO_WOREVT0           (0)         // WOREVT2 (dynamic)

#define RADIO_WOR_ENABLE        (0 << 7)    // WORCTRL.ACLK_PD (default, WOR is never used, but RX timeout is)
#define RADIO_WOR_DISABLE       (1 << 7)    // WORCTRL.ACLK_PD 
#define RADIO_WOREV1_DELAY      (0 << 4)    // WORCTRL.EVENT1 (default, minimum delay, EVT1 never used)
#define RADIO_WOR_RES           (1)         // WORCTRL.WOR_RES (default, 1 ti)

#define RADIO_FREND1            (0x56)      // FREND1 (default)

#define RADIO_FREND0            (0x10)      // FREND0 (default)

#define RADIO_FSCAL3_CTRL       (3 << 6)    // FSCAL3.FSCAL3 (default)
#define RADIO_CAL_PUMP_ENABLE   (2 << 4)    // FSCAL3.CHP_CURR_CAL_EN (default)
#define RADIO_CAL_PUMP_DISABLE  (0 << 4)    // FSCAL3.CHP_CURR_CAL_EN
#define RADIO_FSCAL3_RESULT     (0x0F)      // FSCAL3 Result Mask

#define RADIO_HIGHVCO_ENABLE    (1 << 5)    // FSCAL2.VCO_CORE_H_EN (default)
#define RADIO_HIGHVCO_DISABLE   (0 << 5)    // FSCAL2.VCO_CORE_H_EN 
#define RADIO_FSCAL2_RESULT     (0x1F)      // FSCAL2 Result Mask

#define RADIO_FSCAL1_RESULT     (0x3F)      // FSCAL1 Result Mask

#define RADIO_FSCAL0            (0x1F)      // FSCAL0 (default)

#define RADIO_TEST0_DEF         (2 << 2)    // TEST0
#define RADIO_CAL_VCO_ENABLE    (1 << 1)    // TEST0
#define RADIO_CAL_VCO_DISABLE   (0 << 1)    // TEST0 (default!)






/// Register defaults for CC430
#define RFREG_IOCFG2       RADIO_GDO2_VALUE
#define RFREG_IOCFG1       RADIO_GDO1_VALUE
#define RFREG_IOCFG0       RADIO_GDO0_VALUE
#define RFREG_FIFOTHR      (RADIO_ATTENUATE_OFF | RADIO_FIFO_THR)
#define RFREG_SYNC1        0xF4        
#define RFREG_SYNC0        0x98
#define RFREG_PKTLEN       RADIO_PKTLEN_FF 
#define RFREG_PKTCTRL1     (RADIO_PKT_PQT | RADIO_PKT_STATUS)       
#define RFREG_PKTCTRL0     (RADIO_PKT_PN9 | RADIO_PKT_CRC | RADIO_PKT_AUTO)        
#define RFREG_ADDR         0x00 
#define RFREG_CHANNR       RADIO_CHAN  
#define RFREG_FSCTRL1      RADIO_FREQ_IF_NORMAL  
#define RFREG_FSCTRL0      RADIO_FREQOFF 
#define RFREG_FREQ2        RADIO_FREQ2
#define RFREG_FREQ1        RADIO_FREQ1  
#define RFREG_FREQ0        RADIO_FREQ0    
#define RFREG_MDMCFG4      (RADIO_FILTER_NORMAL | RADIO_DRATE_NORMAL_E)
#define RFREG_MDMCFG3      RADIO_DRATE_NORMAL_M     
#define RFREG_MDMCFG2      (RADIO_DCFILT_NORMAL | RADIO_MOD_NORMAL | RADIO_SYNCMODE_PN9)
#define RFREG_MDMCFG1      (RADIO_PREAMBLE_NORMAL | RADIO_CHANNEL_SPC_E)
#define RFREG_MDMCFG0      RADIO_CHANNEL_SPC_M    
#define RFREG_DEVIATN      RADIO_DEV    
#define RFREG_MCSM2        (RADIO_RXCS_DISABLE | RADIO_RXTO_ENABLE)    
#define RFREG_MCSM1        (RADIO_CCA_GUARDED | RADIO_RXOFF_IDLE | RADIO_TXOFF_IDLE)   
#define RFREG_MCSM0        (RADIO_AUTOCAL_SHUTDOWN4)        
#define RFREG_FOCCFG       RADIO_FOCCFG        
#define RFREG_BSCFG        RADIO_BSCFG        
#define RFREG_AGCCTRL2     (RADIO_DVGA_GAIN | RADIO_LNA_GAIN | RADIO_MAGN_TARGET)
#define RFREG_AGCCTRL1     (RADIO_LNA_PRIORITY | RADIO_CSRT | RADIO_CSAT) 
#define RFREG_AGCCTRL0     (RADIO_HYST_LEVEL | RADIO_WAIT_TIME | RADIO_AGC_FREEZE | RADIO_FILTER_LEN)        
#define RFREG_WOREVT1      RADIO_WOREVT1        
#define RFREG_WOREVT0      RADIO_WOREVT0
#if (RF_FEATURE(RXTIMER) == ENABLED)
#define RFREG_WORCTL       (RADIO_WOR_ENABLE | RADIO_WOREV1_DELAY | RADIO_WOR_RES)
#else
#define RFREG_WORCTL       (RADIO_WOR_DISABLE | RADIO_WOREV1_DELAY | RADIO_WOR_RES)
#endif
#define RFREG_FREND1       RADIO_FREND1        
#define RFREG_FREND0       RADIO_FREND0        
#define RFREG_FSCAL3       (RADIO_FSCAL3_CTRL | RADIO_CAL_PUMP_ENABLE)
#define RFREG_FSCAL2       (RADIO_HIGHVCO_ENABLE)        
#define RFREG_FSCAL1       0x00        
#define RFREG_FSCAL0       RADIO_FSCAL0        

//#define RFREG_FSTEST       0x59        
//#define RFREG_PTEST        0x7F        
//#define RFREG_AGCTEST      0x3F        
//#define RFREG_TEST2        0x81        
//#define RFREG_TEST1        0x35       
#define RFREG_TEST0        (RADIO_TEST0_DEF | RADIO_CAL_VCO_DISABLE)   







/** Packet overhead & slop, 
  * These are parts of the packet that do not carry preamble, sync word, or 
  * frame data.  The units are in bytes.
  */
#define RADIO_RAMP_UP       1
#define RADIO_RAMP_DOWN     1
#define RADIO_PKT_PADDING   1
#define RADIO_PKT_OVERHEAD  (RADIO_RAMP_UP + RADIO_RAMP_DOWN + RADIO_PKT_PADDING)







/** CS/CCA offsets
  * Used for setup of the CS RSSI Thresholding Mechanism.  The CCA can be handled
  * by the same mechanism, (the code for this is commented-out), but testing has
  * shown that a direct RSSI method is faster & more reliable.
  */
#define RF_CSTHR_OFFSET     37
#define RF_CCATHR_OFFSET    37






/** Radio Control Macros
  * These are great for test and debug, but when you're going to release, you
  * will get better results by integrating the macros into your driver file.
  */


/// Turn off radio (absolute lowest power state)
#define RFCTRL_OFF()    RFCTRL_SLEEP()


/// Put radio into sleep state (lowest power with registers alive)
#define RFCTRL_SLEEP()  do { \
                            RF_CmdStrobe(RF_CoreStrobe_IDLE); \
                            RF_CmdStrobe(RF_CoreStrobe_PWD); \
                        } while(0)


/// Put radio into idle (PA off, FS on)
#define RFCTRL_IDLE()   RF_CmdStrobe( RF_CoreStrobe_IDLE )


/// Turn-on radio RX
#define RFCTRL_RX()     RF_CmdStrobe( RF_CoreStrobe_RX )


/// Turn-on radio TX
#define RFCTRL_TX()     RF_CmdStrobe( RF_CoreStrobe_TX )


/// Manual calibration
#define RFCTRL_MANCAL() RF_CmdStrobe( RF_CoreStrobe_CAL )




/// Check the CCA Flag (via the interrupt
#define RFCHECK_CCA()




/** Some stuff that is basically just used for the simulator
  */
#define RFCONFIG_TXINIT();
#define RFPUT_TXDATA(); 
#define RFGET_RXDATA(); 





// tell chip to go to idle after TX
#define RFCONFIG_TXTOIDLE() \
    RF_WriteSingleReg(RF_CoreReg_MCSM1, b00000000)


// tell chip to begin transmitting new packet after old one is done
#define RFCONFIG_TXTOTX() \
    RF_WriteSingleReg(RF_CoreReg_MCSM1, b00000010)



// Flush commands
#define RFCONFIG_FLUSHTX()  RF_CmdStrobe( RF_CoreStrobe_FTX )
#define RFCONFIG_FLUSHRX()  RF_CmdStrobe( RF_CoreStrobe_RTX )




//Set RX buffer threshold
#define RFCONFIG_RXBUFFER(FIFOTHRESH) \
    RF_WriteSingleReg(RF_CoreReg_FIFOTHR, (ot_u8)((FIFOTHRESH >> 2) - 1));




#define RFCONFIG_TXBUFFER(FIFOTHRESH) \
    RF_WriteSingleReg(RF_CoreReg_FIFOTHR, (ot_u8)(15 - (FIFOTHRESH >> 2)));




///Enable rx fifo filled interrupt and packet done interrupt (if available)
#define RFCONFIG_RXFIFO_INTON()     RF_CoreITConfig(RF_CoreIT_RXFullOrDone, ENABLE)




#define RFCONFIG_TXFIFO_INTON() do { \
    RFWord->IE = (ot_u16)((RF_CoreIT_TXBelowThresh | RF_CoreIT_TXUnderflow | RF_CoreIT_EndState) >> 16); \
    RFWord->IES = (ot_u16)(RF_CoreIT_TXBelowThresh | RF_CoreIT_TXUnderflow | RF_CoreIT_EndState); \
} while(0)

//disable TX buffer empty/done interrupt
#define RFCONFIG_TXFIFODONE_INTON() RF_CoreITConfig(RF_CoreIT_TXUnderflow, ENABLE)

//disable TX buffer too low interrupt
#define RFCONFIG_TXFIFOLOW_INTOFF() RF_CoreITConfig(RF_CoreIT_TXBelowThresh, DISABLE)

//disable TX buffer empty/done interrupt
#define RFCONFIG_TXFIFODONE_INTOFF() RF_CoreITConfig(RF_CoreIT_TXUnderflow, DISABLE)


/// Disable all RF Core interrupts
#define RFCONFIG_INTS_OFF()         do { \
                                        RF_CoreITConfig(RF_CoreIT_ALL, DISABLE); \
                                        RF_ClearCoreITPendingBit(RF_CoreIT_ALL); \
                                    } while(0)


//fail if rx timeout flag is on
//RX Timeout uses the GDO interrupt, number 0x1F, on IOCFG0
#define RFGET_RXTIMEOUT_FLAG()      RF_GetCoreITLevel(RF_CoreIT_IOCFG0)
#define RFCLEAR_RXTIMEOUT_FLAG()    RF_ClearCoreITPendingBit(RF_CoreIT_IOCFG0)


/// RX init for Background Frame detection
/// - rx fifo threshold to amount from FIFO_THRESH
/// - set carrier sense limiter to phymac[0].cs_rssi (-140 + dBm)
/// - enable any feature that cancels RX immediately on low carrier sense
/// - enable the following interrupts, disable all others:
///     1. RX Timeout interrupt
///     2. Sync Detect
///     3. CS too low (cca)
///     4. Packet done interrupt (success)
/// - turn receiver on
///
#define RFCONFIG_RXINIT_BF(TIMEOUT, FIFOTHRESH) do { \
    RF_WriteSingleReg(RF_CoreReg_FIFOTHR, (ot_u8)((FIFOTHRESH >> 2) - 1)); \
    RF_WriteSingleReg(RF_CoreReg_AGCCTRL2, phymac[0].cs_thr); /* CS */ \
    /* RF_WriteSingleReg(RF_CoreReg_AGCCTRL1, b01000111);         CS */ \
    RF_WriteSingleReg(RF_CoreReg_WOREVT1, (ot_u8)(TIMEOUT>>8)); \
    RF_WriteSingleReg(RF_CoreReg_WOREVT0, (ot_u8)(TIMEOUT)); \
    RF_WriteSingleReg(RF_CoreReg_MCSM2, b00010000); \
    /* RF_WriteSingleReg(RF_CoreReg_MCSM1, b00000000); */ \
    /* RF_WriteSingleReg(RF_CoreReg_WORCTRL, b00000001); */ \
    RF_ClearCoreITPendingBit(RF_CoreIT_ALL); \
    RFWord->IE = (ot_u16)((RF_CoreIT_IOCFG0 | RF_CoreIT_SyncWord | \
                RF_CoreIT_CarrierSense | RF_CoreIT_RXFullOrDone) >> 16); \
    RFWord->IES = (ot_u16)(RF_CoreIT_IOCFG0 | RF_CoreIT_SyncWord | \
                RF_CoreIT_CarrierSense | RF_CoreIT_RXFullOrDone); \
    RFCTRL_RX(); \
} while(0)
//sim init


/// Disable interrupts that are not the RX Done interrupt
#define RFCONFIG_RXCONT_BF()    do { \
    RF_ClearCoreITPendingBit(RF_CoreIT_ALL); \
    RFWord->IE = (ot_u16)(RF_CoreIT_RXFullOrDone >> 16); \
    RFWord->IES = (ot_u16)(RF_CoreIT_RXFullOrDone); \
} while(0)



/// - Set rx fifo threshold to amount from FIFO_THRESH
/// - Enable the following interrupts, disable all others:
//      1. rx timeout interrupt
//      2. sync word detect interrupt
/// - turn receiver on
#define RFCONFIG_RXINIT_FF(TIMEOUT, FIFO_THRESH) do { \
    RF_WriteSingleReg(RF_CoreReg_FIFOTHR, (ot_u8)((FIFOTHRESH >> 2) - 1)); \
    RF_WriteSingleReg(RF_CoreReg_AGCCTRL2, phymac[0].cs_thr); /* CS */ \
    /* RF_WriteSingleReg(RF_CoreReg_AGCCTRL1, b01000111);         CS */ \
    RF_WriteSingleReg(RF_CoreReg_WOREVT1, (ot_u8)(TIMEOUT>>8)); \
    RF_WriteSingleReg(RF_CoreReg_WOREVT0, (ot_u8)(TIMEOUT)); \
    RF_WriteSingleReg(RF_CoreReg_MCSM2, b00000000); \
    /* RF_WriteSingleReg(RF_CoreReg_MCSM1, b00000000); */ \
    /* RF_WriteSingleReg(RF_CoreReg_WORCTRL, b00000001); */ \
    RF_ClearCoreITPendingBit(RF_CoreIT_ALL); \
    RFWord->IE = (ot_u16)((RF_CoreIT_IOCFG0 | RF_CoreIT_SyncWord ) >> 16); \
    RFWord->IES = (ot_u16)(RF_CoreIT_IOCFG0 | RF_CoreIT_SyncWord ); \
    RFCTRL_RX(); \
} while(0)
//sub_sim_init(); 




/// Disable interrupts that are not the RX Done interrupt
#define RFCONFIG_RXCONT_FF()    do { \
  /*  RF_ClearCoreITPendingBit(RF_CoreIT_ALL); */ \
    RFWord->IE = (ot_u16)(RF_CoreIT_RXFullOrDone >> 16); \
    RFWord->IES = (ot_u16)(RF_CoreIT_RXFullOrDone); \
} while(0)




/// - Enable interrupt on cs / rssi above threshold (fail)
/// - Enable interrupt on cca / rssi below threshold (pass)
//#define RFCONFIG_CSMA_INTON()   do { \
//    RF_ClearCoreITPendingBit(RF_CoreIT_ALL); \
//    RFWord->IE = (ot_u16)((RF_CoreIT_ClearChannel2 | RF_CoreIT_CarrierSense) >> 16); \
//    RFWord->IES = (ot_u16)(RF_CoreIT_ClearChannel2 | RF_CoreIT_CarrierSense); \
//} while(0)

#define RFCONFIG_CSMA_INTON()   do { \
    RFWord->IFG = 0; \
    RFWord->IE  = (ot_u16)(RF_CoreIT_IOCFG1 >> 16); \
    RFWord->IES = (ot_u16)RF_CoreIT_IOCFG1; \
} while(0)




/// Disable cca and cs interrupts
//#define RFCONFIG_CSMA_INTOFF()  \
//    RF_CoreITConfig((RF_CoreIT_ClearChannel2 | RF_CoreIT_CarrierSense), DISABLE)

#define RFCONFIG_CSMA_INTOFF()  RF_CoreITConfig(RF_CoreIT_IOCFG1, DISABLE)




/// Set cca rssi threshold
//#define RFCONFIG_CCA()  do { \
//    RFWord->IFG = 0; \
//    RF_WriteSingleReg(RF_CoreReg_AGCCTRL2, phymac[0].cca_thr); /* CCA */ \
//    RF_WriteSingleReg(RF_CoreReg_AGCCTRL1, b01001001);         /* CCA */ \
//} while(0)

#define RFCONFIG_CCA() (RFWord->IFG = 0)





/// Write the 16 bit sync_value to the RF sync regs.
#define RFCONFIG_SYNCWORD(SYNCARRAY)  \
    RF_WriteBurstReg(RF_CoreReg_SYNC1, SYNCARRAY, 2)



/// Set packet mode on, variable length packets
/// Set max length
#define RFCONFIG_VARPKT(MAXLENGTH) do { \
    ot_u8 regval; \
    regval = (phymac[0].channel & 0x80) ? b00000001 : b01000001; \
    RF_WriteSingleReg(RF_CoreReg_PKTLEN, (ot_u8)MAXLENGTH); \
    RF_WriteSingleReg(RF_CoreReg_PKTCTRL0, regval); \
} while(0)



///Set buffered mode, infinite length packets
#define RFCONFIG_INFBUFFER() do { \
    ot_u8 regval; \
    regval = (phymac[0].channel & 0x80) ? b00000010 : b01000010; \
    RF_WriteSingleReg(RF_CoreReg_PKTCTRL0, regval); \
} while(0)



/// Set packet mode on, fixed length packets
/// Set length
#define RFCONFIG_FIXEDPKT(LENGTH) do { \
    ot_u8 regval; \
    regval = (phymac[0].channel & 0x80) ? b00000000 : b01000000; \
    RF_WriteSingleReg(RF_CoreReg_PKTLEN, (ot_u8)LENGTH); \
    RF_WriteSingleReg(RF_CoreReg_PKTCTRL0, regval); \
} while(0)




/// Set up transceiver for FEC
/// Set Preamble qualifier on the low side (eg PQT=4 on CC430)
/// Set Sync Word qualifier to allow up to two bit errors
#define RFCONFIG_FECQUAL() do { \
    RF_WriteSingleReg(RF_CoreReg_PKTCTRL1, b00100000); \
    RF_WriteSingleReg(RF_CoreReg_MDMCFG2, b10010101); \
} while(0)



/// Set HW FEC to ON and PN9 to ON
#define RFCONFIG_PN9FECHW();

/// Set HW FEC to ON
#define RFCONFIG_FECHW(); 

/// Set FEC to OFF and PN9 to ON
/// @note, done in buffer call
#define RFCONFIG_PN9HW();


/// Set Preamble qualifier on the high side (eg PQT=8 on CC430)
/// Set Sync qualifier to allow no bit errors (16/16)
#define RFCONFIG_PN9QUAL() do { \
    RF_WriteSingleReg(RF_CoreReg_PKTCTRL1, b01000000); \
    RF_WriteSingleReg(RF_CoreReg_MDMCFG2, b10010110); \
} while(0)



/// Set tx power to value from phymac[0].tx_eirp
#define RFCONFIG_POWER(TX_EIRP) do { \
    /* @todo get these values */ \
} while(0)


/// Set symbol rate to 55k
/// Set RX filter bandwidth for somewhere in the 160 kHz range
/// Set AFE config for 55k PHY
/// Set IF config for 55k PHY
/// Set preamble to between 32 and 64 bits
#define RFCONFIG_FSK18() do { \
    RF_WriteSingleReg(RF_CoreReg_MDMCFG4, RADIO_FILTER_NORMAL | RADIO_DRATE_NORMAL_E); \
    RF_WriteSingleReg(RF_CoreReg_MDMCFG3, RADIO_DRATE_NORMAL_M); \
    RF_WriteSingleReg(RF_CoreReg_MDMCFG1, RADIO_PREAMBLE_NORMAL | RADIO_CHANNEL_SPC_E); \
} while(0)





/// Set symbol rate to 200k
/// Set RX filter bandwidth for somewhere in the 300 kHz range
/// Set AFE config for 200k PHY
/// Set IF config for 200k PHY
/// Set preamble to between 48 and 128 bits
#define RFCONFIG_FSK05() do { \
    RF_WriteSingleReg(RF_CoreReg_MDMCFG4, RADIO_FILTER_TURBO | RADIO_DRATE_TURBO_E); \
    RF_WriteSingleReg(RF_CoreReg_MDMCFG3, RADIO_DRATE_TURBO_M); \
    RF_WriteSingleReg(RF_CoreReg_MDMCFG1, RADIO_PREAMBLE_TURBO | RADIO_CHANNEL_SPC_E); \
} while(0)




/// Change channel center frequency (433.164MHz + (fc_i * 108kHz) )
#define RFCONFIG_FC(FC_INDEX) \
    RF_WriteSingleReg(RF_CoreReg_CHANNR, (ot_u8)FC_INDEX)



/// Tell radio to auto-cal on next idle->active transition ...
#define RFCONFIG_STAGECAL() RFCTRL_MANCAL()
//#define RFCONFIG_STAGECAL() \
//    RF_WriteSingleReg(RF_CoreReg_MCSM0, b00010000)



/// Optionally recalibrate, using the 1/4 cal option
//#define RFCONFIG_SMARTCAL() \
//    RF_WriteSingleReg(RF_CoreReg_MCSM0, b00110000)
#define RFCONFIG_SMARTCAL();


/// Number of bytes until next buffer interrupt
#define RFGET_RXFIFO_NEXTINT()  (radio.rxlimit - (ot_int)RF_GetRXBYTES())
#define RFGET_TXFIFO_NEXTINT()  ((ot_int)RF_GetTXBYTES() - radio.txlimit)




#endif
