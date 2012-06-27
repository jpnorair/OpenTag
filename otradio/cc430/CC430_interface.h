/* Copyright 2009-2012 JP Norair
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
  * @file       /otradio/cc430/CC430_interface.h
  * @author     JP Norair
  * @version    V1.0
  * @date       5 April 2012
  * @brief      Functions for the CC430 transceiver interface
  * @defgroup   CC430 (CC430 family support)
  *
  ******************************************************************************
  */

#ifndef __CC430_interface_H
#define __CC430_interface_H

#include "OT_types.h"
#include "CC430_registers.h"
#include "CC430_defaults.h"






/** Basic Control <BR>
  * ============================================================================
  */

//void cc430_load_defaults();



//void cc430_reset();



#define cc430_calibrate()   RF_CmdStrobe(RF_CoreStrobe_CAL)


//ot_u16 cc430_rfstatus();


//ot_u16 cc430_wortime();


//ot_u16 cc430_rcctrl();


//ot_u8 cc430_estfreqoffset();



//ot_u8 cc430_lqi();


/** @brief  Returns RSSI value from CC430 core
  * @param  None
  * @retval ot_u8       RSSI register value
  * @ingroup CC430
  */
//ot_u8 cc430_rssi();


/** @brief  Returns VCO VC DAC value from CC430 core
  * @param  None
  * @retval ot_u8       VCO_VC_DAC register value
  * @ingroup CC430
  */
//ot_u8 cc430_vcovcdac();


#define cc430_rxbytes()     RF_GetRXBYTES()
#define cc430_txbytes()     RF_GetTXBYTES()




  

/** @brief  Configures GDOs for listen: RX Sync + RX Idle
  * @param  None
  * @retval None
  * @ingroup CC430
  */ 
OT_INLINE_H void cc430_iocfg_listen() {
    RFWord->IFG = 0;
#   if (RF_FEATURE(RXTIMER) == ENABLED)
    RFWord->IE  = (ot_u16)((RF_CoreIT_IOCFG0 | RF_CoreIT_SyncWord) >> 16);
    RFWord->IES = (ot_u16)(RF_CoreIT_IOCFG0 | RF_CoreIT_SyncWord);
#   else
    RFWord->IE  = (ot_u16)(RF_CoreIT_SyncWord >> 16);
    RFWord->IES = (ot_u16)(RF_CoreIT_SyncWord);
#   endif
}


/** @brief  Configures GDOs for RX'ing: RX-end (invert Sync) + RX FIFO thresh
  * @param  None
  * @retval None
  * @ingroup CC430
  */ 
OT_INLINE_H void cc430_iocfg_rxdata() {
    RFWord->IE  = (ot_u16)((RF_CoreIT_RXFull | RF_CoreIT_EndState) >> 16);
    RFWord->IES = (ot_u16)(RF_CoreIT_RXFull | RF_CoreIT_EndState);
}


/** @brief  Configures GDOs for TX CSMA: Carrier Sense + Clear Channel Assement
  * @param  None
  * @retval None
  * @ingroup CC430
  */ 
OT_INLINE_H void cc430_iocfg_txcsma() {
    RFWord->IFG = 0;
    RFWord->IE  = (ot_u16)(RF_CoreIT_IOCFG1 >> 16); 
    RFWord->IES = (ot_u16)RF_CoreIT_IOCFG1;
}


/** @brief  Configures GDOs for TX'ing: TX FIFO underflow + TX FIFO threshold
  * @param  None
  * @retval None
  * @ingroup CC430
  */ 
OT_INLINE_H void cc430_iocfg_txdata() {
    RFWord->IFG = 0;
    RFWord->IE  = (ot_u16)((/*RF_CoreIT_TXUnderflow*/ 0 | \
                            RF_CoreIT_TXBelowThresh | \
                            RF_CoreIT_EndState) >> 16);

    RFWord->IES = (ot_u16)( /*RF_CoreIT_TXUnderflow*/ 0 | \
                            RF_CoreIT_TXBelowThresh | \
                            RF_CoreIT_EndState      );
}


OT_INLINE_H void cc430_iocfg_rxend() {
    RFWord->IE  = (ot_u16)(RF_CoreIT_EndState >> 16);
    RFWord->IES = (ot_u16)(RF_CoreIT_EndState);
}


OT_INLINE_H void cc430_iocfg_txend() {

}









/** Stuff below this line is deprecated, and it may be removed later
  * ========================================================================<BR>
  */

/// Packet overhead & slop, 
/// These are parts of the packet that do not carry preamble, sync word, or 
/// frame data.  The units are in bytes.
#define RADIO_RAMP_UP       1
#define RADIO_RAMP_DOWN     1
#define RADIO_PKT_PADDING   1
#define RADIO_PKT_OVERHEAD  (RADIO_RAMP_UP + RADIO_RAMP_DOWN + RADIO_PKT_PADDING)


/// CS/CCA offsets
/// Used for setup of the CS RSSI Thresholding Mechanism.  The CCA can be handled
/// by the same mechanism, (the code for this is commented-out), but testing has
/// shown that a direct RSSI method is faster & more reliable.
#define RF_CSTHR_OFFSET     37
#define RF_CCATHR_OFFSET    37




/// Radio Control Macros
/// These are great for test and debug, but when you're going to release, you
/// will get better results by integrating the macros into your driver file.

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


/// Some stuff that is basically just used for the simulator
#define RFCONFIG_TXINIT();
#define RFPUT_TXDATA(); 
#define RFGET_RXDATA(); 


/// tell chip to go to idle after TX
#define RFCONFIG_TXTOIDLE() \
    RF_WriteSingleReg(RF_CoreReg_MCSM1, b00000000)


/// tell chip to begin transmitting new packet after old one is done
#define RFCONFIG_TXTOTX() \
    RF_WriteSingleReg(RF_CoreReg_MCSM1, b00000010)


/// Flush commands
#define RFCONFIG_FLUSHTX()  RF_CmdStrobe( RF_CoreStrobe_FTX )
#define RFCONFIG_FLUSHRX()  RF_CmdStrobe( RF_CoreStrobe_RTX )


/// Set RX buffer threshold
#define RFCONFIG_RXBUFFER(FIFOTHRESH) \
    RF_WriteSingleReg(RF_CoreReg_FIFOTHR, (ot_u8)((FIFOTHRESH >> 2) - 1));

#define RFCONFIG_TXBUFFER(FIFOTHRESH) \
    RF_WriteSingleReg(RF_CoreReg_FIFOTHR, (ot_u8)(15 - (FIFOTHRESH >> 2)));


/// Enable rx fifo filled interrupt and packet done interrupt (if available)
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
