/* Copyright 2009 JP Norair
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
/** @file       /Platforms/CC430/cc430_lib/cc430_rf.h
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Dec 2009
  * @brief      Library resources for RF peripheral
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#ifndef __CC430_LIB_RF_H
#define __CC430_LIB_RF_H



#include "cc430_map.h"



// CC430 RSSI OFFSET (Always 74)
#define RF_RSSIOffset               74


// CC430 MARCStates as returned by bits 4:0 in the MARCSTATE register
#define RF_MARCState_SLEEP          0x00
#define RF_MARCState_IDLE           0x01
#define RF_MARCState_XOFF           0x02
#define RF_MARCState_VCOON_MC       0x03
#define RF_MARCState_REGON_MC       0x04
#define RF_MARCState_MANCAL         0x05
#define RF_MARCState_VCOON          0x06
#define RF_MARCState_REGON          0x07
#define RF_MARCState_STARTCAL       0x08
#define RF_MARCState_BWBOOST        0x09
#define RF_MARCState_FS_LOCK        0x0A
#define RF_MARCState_IFADCON        0x0B
#define RF_MARCState_ENDCAL         0x0C
#define RF_MARCState_RX             0x0D
#define RF_MARCState_RX_END         0x0E
#define RF_MARCState_RX_RST         0x0F
#define RF_MARCState_TXRX_SWITCH    0x10
#define RF_MARCState_RXFIFO_OVF     0x11
#define RF_MARCState_FSTXON         0x12
#define RF_MARCState_TX             0x13
#define RF_MARCState_TX_END         0x14
#define RF_MARCState_RXTX_SWITCH    0x15
#define RF_MARCState_TXFIFO_UNF     0x16




//RFWord->IFCTL0      bit 1       0: convert little endian to big endian      1: don't convert
#define RF_CTL_ConvertEndian        0x0002


//RFWord->IFCTL1      bit 15-12, 10-8: interrupt enabled
//                bit 7-4, 2-0: interrupt pending flags
#define RF_IFIT_DOUT                0x8000
#define RF_IFIT_STAT                0x4000
#define RF_IFIT_DIN                 0x2000
#define RF_IFIT_INSTR               0x1000
#define RF_IFIT_ERR                 0x0400
#define RF_IFIT_TX                  0x0200
#define RF_IFIT_RX                  0x0100
#define RF_IFIT_ALL                 (RF_IFIT_DOUT | RF_IFIT_STAT | RF_IFIT_DIN | RF_IFIT_INSTR | RF_IFIT_ERR | RF_IFIT_TX | RF_IFIT_RX)



//RFWord->IFCTL2      reserved

//RFWord->IFERR       bit 3-0: error flags
#define RF_Error_OPOVERR            0x0008
#define RF_Error_OUTERR             0x0004
#define RF_Error_OPERR              0x0002
#define RF_Error_LVERR              0x0001


//RFWord->IFERRV      bits 5-0: error vector, 0,2,4,6,8
//RFWord->IFIV        bits 5-0: interface interrupt vector, 0,2,4,...,E
#define RF_VECT_NOERR               0x0000
#define RF_VECT_LVERR               0x0002
#define RF_VECT_OPERR               0x0004
#define RF_VECT_OUTERR              0x0006
#define RF_VECT_OPOVERR             0x0008

#define RF_VECT_IFError             0x0002
#define RF_VECT_IFDataOut           0x0004
#define RF_VECT_IFStatusOut         0x0006
#define RF_VECT_IFDataIn            0x0008
#define RF_VECT_IFInstrIn           0x000A
#define RF_VECT_FIFORX              0x000C
#define RF_VECT_FIFOTX              0x000E




// radio core instruction word registers
// Control Radio by writing command strobes & data into these registers
// The return to the command strobe is placed in the STATW register

#define RF_CoreStrobe_RES               b00110000
#define RF_CoreStrobe_FSTXON            b00110001
#define RF_CoreStrobe_XOFF              b00110010
#define RF_CoreStrobe_CAL               b00110011
#define RF_CoreStrobe_RX                b00110100
#define RF_CoreStrobe_TX                b00110101
#define RF_CoreStrobe_IDLE              b00110110
#define RF_CoreStrobe_WOR               b00111000
#define RF_CoreStrobe_PWD               b00111001
#define RF_CoreStrobe_FRX               b00111010
#define RF_CoreStrobe_FTX               b00111011
#define RF_CoreStrobe_WORRST            b00111100
#define RF_CoreStrobe_NOP               b00111101

#define RF_CoreInstr_SNGLREGRD          b10000000
#define RF_CoreInstr_SNGLREGWR          b00000000
#define RF_CoreInstr_REGRD              b11000000
#define RF_CoreInstr_REGWR              b01000000
#define RF_CoreInstr_STATREGRD          b11000000
#define RF_CoreInstr_SNGLPATABRD        b10111110
#define RF_CoreInstr_SNGLPATABWR        b00111110
#define RF_CoreInstr_PATABRD            b11111110
#define RF_CoreInstr_PATABWR            b01111110

#define RF_CoreInstr_SNGLRXRD           b10111111
#define RF_CoreInstr_SNGLTXWR           b00111111
#define RF_CoreInstr_RXFIFORD           b11111111
#define RF_CoreInstr_TXFIFOWR           b01111111




// radio core status registers

#define RF_CoreStatus_RDYn              b10000000
#define RF_CoreStatus_STATEx            b01110000
#define RF_CoreStatus_STATE_IDLE        b00000000
#define RF_CoreStatus_STATE_RX          b00010000
#define RF_CoreStatus_STATE_TX          b00100000
#define RF_CoreStatus_STATE_FSTXON      b00110000
#define RF_CoreStatus_STATE_CALIBRATE   b01000000
#define RF_CoreStatus_STATE_SETTLING    b01010000
#define RF_CoreStatus_STATE_RXFIFO_OV   b01100000
#define RF_CoreStatus_STATE_TXFIFO_OV   b01110000
#define RF_CoreStatus_FIFO_BYTESx       b00001111




// Radio Core Interrupt IDs
// Page 490 of users guide

#define RF_CoreIT_ALL               (u32)0xFFFF0000
#define RF_CoreIT_WORevent1         (u32)0x80000000
#define RF_CoreIT_OscStable         (u32)0x80008000
#define RF_CoreIT_WORevent0         (u32)0x40000000
#define RF_CoreIT_WORevent0ACLK     (u32)0x40004000
#define RF_CoreIT_CarrierSense2     (u32)0x20000000         /* same thing as RF_CoreIT_CarrierSense (u32)0x10001000 */
#define RF_CoreIT_ClearChannel2     (u32)0x20002000
#define RF_CoreIT_ClearChannel      (u32)0x10000000
#define RF_CoreIT_CarrierSense      (u32)0x10001000
#define RF_CoreIT_PQTReached        (u32)0x08000000
#define RF_CoreIT_LPW               (u32)0x08000800
#define RF_CoreIT_CRCValid          (u32)0x04000000
#define RF_CoreIT_RXFirstByte       (u32)0x04000400
#define RF_CoreIT_SyncWord          (u32)0x02000000
#define RF_CoreIT_EndState          (u32)0x02000200
#define RF_CoreIT_TXUnderflow       (u32)0x01000000
#define RF_CoreIT_TXFlushed         (u32)0x01000100
#define RF_CoreIT_RXOverflow        (u32)0x00800000
#define RF_CoreIT_RXFlushed         (u32)0x00800080
#define RF_CoreIT_TXFull            (u32)0x00400000
#define RF_CoreIT_TXNotFull         (u32)0x00400040             
#define RF_CoreIT_TXAboveThresh     (u32)0x00200000
#define RF_CoreIT_TXBelowThresh     (u32)0x00200020
#define RF_CoreIT_RXFullOrDone      (u32)0x00100000
#define RF_CoreIT_RXEmpty           (u32)0x00100010
#define RF_CoreIT_RXFull            (u32)0x00080000
#define RF_CoreIT_RXNotFull         (u32)0x00080008
#define RF_CoreIT_IOCFG2            (u32)0x00040000
#define RF_CoreIT_IOCFG1            (u32)0x00020000
#define RF_CoreIT_IOCFG0            (u32)0x00010000



// Radio Core Registers

#define RF_CoreReg_IOCFG2           0x00
#define RF_CoreReg_IOCFG1           0x01
#define RF_CoreReg_IOCFG0           0x02
#define RF_CoreReg_FIFOTHR          0x03    
#define RF_CoreReg_SYNC1            0x04
#define RF_CoreReg_SYNC0            0x05
#define RF_CoreReg_PKTLEN           0x06
#define RF_CoreReg_PKTCTRL1         0x07
#define RF_CoreReg_PKTCTRL0         0x08
#define RF_CoreReg_ADDR             0x09
#define RF_CoreReg_CHANNR           0x0A
#define RF_CoreReg_FSCTRL1          0x0B
#define RF_CoreReg_FSCTRL0          0x0C
#define RF_CoreReg_FREQ2            0x0D
#define RF_CoreReg_FREQ1            0x0E
#define RF_CoreReg_FREQ0            0x0F
#define RF_CoreReg_MDMCFG4          0x10
#define RF_CoreReg_MDMCFG3          0x11
#define RF_CoreReg_MDMCFG2          0x12
#define RF_CoreReg_MDMCFG1          0x13
#define RF_CoreReg_MDMCFG0          0x14
#define RF_CoreReg_DEVIATN          0x15
#define RF_CoreReg_MCSM2            0x16
#define RF_CoreReg_MCSM1            0x17
#define RF_CoreReg_MCSM0            0x18
#define RF_CoreReg_FOCCFG           0x19
#define RF_CoreReg_BSCFG            0x1A
#define RF_CoreReg_AGCCTRL2         0x1B
#define RF_CoreReg_AGCCTRL1         0x1C
#define RF_CoreReg_AGCCTRL0         0x1D
#define RF_CoreReg_WOREVT1          0x1E
#define RF_CoreReg_WOREVT0          0x1F
#define RF_CoreReg_WORCTRL          0x20
#define RF_CoreReg_FREND1           0x21
#define RF_CoreReg_FREND0           0x22
#define RF_CoreReg_FSCAL3           0x23
#define RF_CoreReg_FSCAL2           0x24
#define RF_CoreReg_FSCAL1           0x25
#define RF_CoreReg_FSCAL0           0x26
#define RF_CoreReg_FSTEST           0x29
#define RF_CoreReg_PTEST            0x2A
#define RF_CoreReg_AGCTEST          0x2B
#define RF_CoreReg_TEST2            0x2C
#define RF_CoreReg_TEST1            0x2D
#define RF_CoreReg_TEST0            0x2E
#define RF_CoreReg_PARTNUM          0x30
#define RF_CoreReg_VERSION          0x31
#define RF_CoreReg_FREQEST          0x32
#define RF_CoreReg_LQI              0x33
#define RF_CoreReg_RSSI             0x34
#define RF_CoreReg_MARCSTATE        0x35
#define RF_CoreReg_WORTIME1         0x36
#define RF_CoreReg_WORTIME0         0x37
#define RF_CoreReg_PKTSTATUS        0x38
#define RF_CoreReg_VCO_VC_DAC       0x39
#define RF_CoreReg_TXBYTES          0x3A
#define RF_CoreReg_RXBYTES          0x3B


// The data for this init structure is taken directly from SmartRF Studio
// You may desire to dump the default settings into a contiguous block of Flash 
// (such as the lookup space) and cast the block as an RF_InitType pointer

// For memory optimization, some registers could potentially be hard-coded
typedef struct {
    u8 CC_IOCFG2;
    u8 CC_IOCFG1;
    u8 CC_IOCFG0;
    u8 CC_FIFOTHR;      
    u8 CC_SYNC1;        // optional ... may be useful for some implementations
    u8 CC_SYNC0;        // optional ... may be useful for some implementations
    u8 CC_PKTLEN;       // N/A ... CC Packet HW not used
    u8 CC_PKTCTRL1;     // N/A ... CC Packet HW not used
    u8 CC_PKTCTRL0;
    u8 CC_ADDR;         // N/A ... CC Packet HW not used
    u8 CC_CHANNR;       // optional ... may be useful for some implementations
    u8 CC_FSCTRL1;
    u8 CC_FSCTRL0;
    u8 CC_FREQ2;
    u8 CC_FREQ1;
    u8 CC_FREQ0;
    u8 CC_MDMCFG4;
    u8 CC_MDMCFG3;
    u8 CC_MDMCFG2;      // optional ... may be useful for some implementations
    u8 CC_MDMCFG1;      // optional ... may be useful for some implementations
    u8 CC_MDMCFG0;      // optional ... may be useful for some implementations
    u8 CC_DEVIATN;
    u8 CC_MCSM2;        // optional ... may be useful for some implementations
    u8 CC_MCSM1;        
    u8 CC_MCSM0;
    u8 CC_FOCCFG;
    u8 CC_BSCFG;
    u8 CC_AGCCTRL2;
    u8 CC_AGCCTRL1;
    u8 CC_AGCCTRL0;
    u8 CC_WOREVT1;      // use WOREVT1:0 = 2447 and WOR_RES = 1 to achieve 2.39s WOR
    u8 CC_WOREVT0;
    u8 CC_WORCTRL;      // Assure bits 1:0 (WOR_RES) are 01 for proper WOR timing
    u8 CC_FREND1;
    u8 CC_FREND0;       // PATABLE (2:0) can always be 000 due to FSK
    u8 CC_FSCAL3;
    u8 CC_FSCAL2;
    u8 CC_FSCAL1;
    u8 CC_FSCAL0;
    u8 CC_TEST2;
    u8 CC_TEST1;
    u8 CC_TEST0;
} RF_CoreInitType;



// Test Registers for CC430.  Generally Read only.
typedef struct {
    u8 CC_FSTEST;
    u8 CC_PTEST;
    u8 CC_AGCTEST;
    u8 CC_TEST2;
    u8 CC_TEST1;
    u8 CC_TEST0;
    u8 CC_VCO_VC_DAC;
} RF_TestInfoType;



// System Info.  Generally Read Only.
// Contains Partnumber and version of the radio
typedef struct {    
    u8 CC_PARTNUM;
    u8 CC_VERSION;
} RF_SystemInfoType;



// Link Info.  Generally Read Only
// Contains Estimated Frequency offset from expected carrier, Link Quality Index, and RSSI
typedef struct {  
    u8 CC_FREQEST;  
    u8 CC_LQI;
    u8 CC_RSSI;
} RF_LinkInfoType;



/** @typedef RF_CoreState
  * An enumerated type that mimics the "MARCState" value of the CC430 RF core.
  */  
typedef enum { 
    RF_SLEEP        = RF_MARCState_SLEEP,
    RF_IDLE         = RF_MARCState_IDLE,
    RF_XOFF         = RF_MARCState_XOFF,
    RF_VCOON_MC     = RF_MARCState_VCOON_MC,
    RF_REGON_MC     = RF_MARCState_REGON_MC,
    RF_MANCAL       = RF_MARCState_MANCAL,
    RF_VCOON        = RF_MARCState_VCOON,
    RF_REGON        = RF_MARCState_REGON,
    RF_STARTCAL     = RF_MARCState_STARTCAL,
    RF_BWBOOST      = RF_MARCState_BWBOOST,
    RF_FS_LOCK      = RF_MARCState_FS_LOCK,
    RF_IFADCON      = RF_MARCState_IFADCON,
    RF_ENDCAL       = RF_MARCState_ENDCAL,
    RF_RX           = RF_MARCState_RX,
    RF_RX_END       = RF_MARCState_RX_END,
    RF_RX_RST       = RF_MARCState_RX_RST,
    RF_TXRX_SWITCH  = RF_MARCState_TXRX_SWITCH,
    RF_RXFIFO_OVF   = RF_MARCState_RXFIFO_OVF,
    RF_FSTXON       = RF_MARCState_FSTXON,
    RF_TX           = RF_MARCState_TX,
    RF_TX_END       = RF_MARCState_TX_END,
    RF_RXTX_SWITCH  = RF_MARCState_RXTX_SWITCH,
    RF_TXFIFO_UNF   = RF_MARCState_TXFIFO_UNF 
} RF_CoreState;





/* --------------- RF Interface Functions ----------------- */




/** @brief Don't use endianness converter between MSP430 and Radio Core
  * @param NewState         (FunctionalState) "Enable" or "Disable"
  * @retval none 
  * @ingroup CC430_lib
  *
  * The Radio Core is Big Endian, the MSP430 is little endian (Why oh why?).
  * This function changes the default setting (Disabled), in which there is a 
  * built-in converter between the little endian MSP430 and big endian radio 
  * core.  TI states that this may be useful for certain types of pass-through 
  * applications, but for normal SoC usage, leave it disabled (default).
  */
void RF_BigEndian( FunctionalState NewState );





/** @brief Configure Radio Interface Interrupts
  * @param RF_IFITMask      (u32) Interface Interrupt Enable/Disable Mask
  * @param NewState         (FunctionalState) "Enable" or "Disable"
  * @retval none 
  * @ingroup CC430_lib
  * @sa RF_CoreITConfig
  *
  * There are seven interface interrupts between the Radio Core and MSP430.
  * There are additional interrupts that occur exclusively on the Radio Core,
  * and those are not handled by RF_CoreITConfig, not this function.
  *
  * The interrupt mask argument (RF_IFITMask) can be a logical OR'ed combination
  * of RF_IFIT... constants, as defined in cc430_rf.h.
  */
void RF_IFITConfig(u16 RF_IFITMask, FunctionalState NewState);





/** @brief Get Status of Radio Interface Interrupt Pending Bits
  * @param RF_IFITMask      (u16) Interface Interrupt Bit Mask
  * @retval bool            True if ANY of the supplied interrupt pending bits are set 
  * @ingroup CC430_lib
  *
  * The interrupt mask argument (RF_IFITMask) can be a logical OR'ed combination
  * of RF_IFIT... constants, as defined in cc430_rf.h.
  */
bool RF_GetIFITStatus(u16 RF_IFITMask);





/** @brief Clear Radio Interface Interrupt Pending Bits
  * @param RF_IFITMask      (u16) Interface Interrupt Bit Mask
  * @retval none
  * @ingroup CC430_lib
  *
  * The interrupt mask argument (RF_IFITMask) can be a logical OR'ed combination
  * of RF_IFIT... constants, as defined in cc430_rf.h.
  *
  * @note
  * If the RFERRIFG bit is cleared, this will result in the clearing of all
  * error flags.  Conversely, clearing all error flags will automatically clear
  * RFERRIFG.
  */
void RF_ClearIFITPendingBit(u16 RF_IFITMask);





/** @brief Get Status of Radio Interface Error Bits
  * @param RF_IFITMask      (u16) Error Bit Mask
  * @retval bool            True if ANY of the supplied error bits are set 
  * @ingroup CC430_lib
  *
  * The error mask argument (RF_ErrorMask) can be a logical OR'ed combination
  * of RF_Error... constants, as defined in cc430_rf.h.
  */
bool RF_GetErrorFlagStatus(u16 RF_ErrorMask);





/** @brief Clear Radio Interface Error Bits
  * @param RF_IFITMask      (u16) Error Bit Mask
  * @retval none
  * @ingroup CC430_lib
  *
  * The error mask argument (RF_ErrorMask) can be a logical OR'ed combination
  * of RF_Error... constants, as defined in cc430_rf.h.
  *
  * @note
  * Clearing all error flags will automatically clear the RF_IFIT Pending Bit, 
  * RFERRIFG.
  */
void RF_ClearErrorFlag(u16 RF_ErrorMask);







/* --------------- RF Core Functions ----------------- */





/** @brief Configure Radio Core Interrupts
  * @param RF_CoreITMask    (u32) Radio Core Interrupt Enable/Disable Mask
  * @param NewState         (FunctionalState) "Enable" or "Disable"
  * @retval none 
  * @ingroup CC430_lib
  * @sa RF_CoreITConfig
  *
  * The Radio Core contains sixteen interrupts and many more interrupt sources.
  * Each of the sixteen interrupts available to the MSP430 may be programmed by
  * instructions as described by the CC430 Users' Guide.
  *
  */
void RF_CoreITConfig(u32 RF_CoreITMask, FunctionalState NewState);





/** @brief Get Status of Radio Core Interrupt Pending Bits
  * @param RF_CoreITMask    (u32) Radio Core Interrupt Bit Mask
  * @retval bool            True if ANY of the supplied interrupt pending bits are set 
  * @ingroup CC430_lib
  *
  * The interrupt mask argument (RF_CoreITMask) can be a logical OR'ed
  * combination of RF_CoreIT... constants, as defined in cc430_rf.h.  Typically,
  * though, just one RF_CoreIT bit should be checked at a time.
  */
bool RF_GetCoreITStatus(u32 RF_CoreITMask);




/** @brief Get Signal Level of Radio Core Interrupt Signals
  * @param RF_CoreITMask    (u32) Radio Core Interrupt Bit Mask
  * @retval bool            True if ANY of the supplied bits are at the HIGH level
  * @ingroup CC430_lib
  *
  * The interrupt mask argument (RF_CoreITMask) can be a logical OR'ed
  * combination of RF_CoreIT... constants, as defined in cc430_rf.h.  Typically,
  * though, just one RF_CoreIT bit should be checked at a time.
  */
bool RF_GetCoreITLevel(u32 RF_CoreITMask);




/** @brief Clear Radio Core Interrupt Pending Bits
  * @param RF_CoreITMask    (u32) Radio Core Interrupt Bit Mask
  * @retval none
  * @ingroup CC430_lib
  *
  * The interrupt mask argument (RF_CoreITMask) can be a logical OR'ed
  * combination of RF_CoreIT... constants, as defined in cc430_rf.h.
  */
void RF_ClearCoreITPendingBit(u32 RF_CoreITMask);






/** @brief Initializes the Radio Core Registers
  * @param RF_CoreInitStruct    (RF_CoreInitType*) Initialization Structure
  * @retval none 
  * @ingroup CC430_lib
  */
void RF_CoreInit( RF_CoreInitType* RF_CoreInitStruct );






/** @brief Issues a command strobe to the Radio Core
  * @param Strobe       (u8) Chipcon command strobe byte
  * @retval None
  * @ingroup CC430_lib
  *
  * RF_Strobe() is meant to work with command strobes only, not other 
  * instructions.  A command strobe has no input data and returns only the
  * status, which can also be found in the register RFWord->STATB.
  */
u8 RF_CmdStrobe( u8 Strobe );




/** @brief Issues a command strobe to the Radio Core
  * @param buffer       (u8*) pointer to array of 64 bytes
  * @retval u8          The Status Byte returned by the Radio Core 
  * @ingroup CC430_lib
  *
  * Dumps the RF Core registers to a local array.
  * A wrapper to RF_ReadBurstReg() that is helpful for debugging.
  */
void RF_CoreDump( u8 *buffer );





u8 RF_ReadSingleReg (u8 addr);
void RF_WriteSingleReg(u8 addr, u8 value);
void RF_ReadBurstReg(u8 addr, u8 *buffer, u8 count);
void RF_WriteBurstReg(u8 addr, u8 *buffer, u8 count);
void RF_Reset( );
void RF_WritePATable( u8 power_value );
void RF_WriteBurstPATable(u8 *buffer, u8 count);
void RF_FIFOTransmit(u8 *buffer, u8 length);
//void RF_FIFOReceive(u8 *buffer, u8 length);
void RF_ReceiveOn( );
void RF_ReceiveOff( );



/** @brief Returns the MARC State
  * @param none
  * @retval u8          The value of the MARCSTATE Register
  * @ingroup CC430_lib
  *
  * The CC430 Radio Core has 23 operational states.  The value returned by this
  * function matches one of the RF_MARCState... constants defined in cc430_rf.h.
  */
u8 RF_GetMARCState();






/** @brief Returns the Carrier Frequency Offset estimation
  * @param none
  * @retval u8          The value of the FREQEST Register
  * @ingroup CC430_lib
  * @sa RF_GetLinkInfo
  *
  * The Frequency Offset estmation of the received signal.  The function 
  * RF_GetLinkInfo also provides this information.
  */
u8 RF_GetFREQEST();





/** @brief Returns the value of the RSSI register
  * @param none
  * @retval u8          The value of the RSSI Register
  * @ingroup CC430_lib
  * @sa RF_GetLinkInfo
  *
  * The RSSI is also returned by RF_GetLinkInfo.
  */
u8 RF_GetRSSI();





/** @brief Returns the LQI, which is a proprietary value.
  * @param none
  * @retval u8          The value of the FREQEST Register
  * @ingroup CC430_lib
  * @sa RF_GetLinkInfo
  *
  * The LQI is also returned by RF_GetLinkInfo.
  */
u8 RF_GetLQI();





/** @brief Returns the combined value of the WORTIME Registers
  * @param none
  * @retval u16         The combined value of the WORTIME Registers
  * @ingroup CC430_lib
  *
  * The WORTIME Registers measure the number of "ticks" until the next wakeup
  * event, as long as the wake-on-timer is enabled.
  */
u16 RF_GetWORTime();





/** @brief Returns the value of the PKTSTATUS register
  * @param none
  * @retval u8          The value of the PKTSTATUS register
  * @ingroup CC430_lib
  *
  * PKTSTATUS is a register of flags.  The RF_CorePKTSTATUS... constants are 
  * defined in cc430_rf.h.
  */
u8 RF_GetPKTSTATUS();





/** @brief Returns the number of bytes in the TX buffer
  * @param none
  * @retval u8          The number of TX bytes in the buffer.
  * @ingroup CC430_lib
  */
u8 RF_GetTXBYTES();





/** @brief Returns the number of bytes in the RX buffer
  * @param none
  * @retval u8          The number of bytes in the RX buffer
  * @ingroup CC430_lib
  */
u8 RF_GetRXBYTES();





/** @brief Returns a group of link parameters: RSSI, LQI, and FREQEST
  * @param RF_LinkInfo  (RF_LinkInfoType*)
  * @retval none
  * @ingroup CC430_lib
  */
void RF_GetLinkInfo(RF_LinkInfoType* RF_LinkInfo);





/** @brief Returns a group of Test data from the CC430 Radio Core
  * @param RF_TestInfo  (RF_TestInfoType*)
  * @retval none
  * @ingroup CC430_lib
  *
  * Test Data includes the following Radio Core registers:
  * FSTEST, PTEST, AGCTEST, TEST2, TEST1, TEST0, VCO_VC_DAC
  */
void RF_GetTestInfo(RF_TestInfoType* RF_TestInfo);





/** @brief Returns a group of System data
  * @param RF_SystemInfo    (RF_SystemInfoType*)
  * @retval none
  * @ingroup CC430_lib
  *
  * System data includes the following Radio Core registers:
  * PARTNUM, VERSION;
  */
void RF_GetSystemInfo(RF_SystemInfoType* RF_SystemInfo);






/* @brief Issues an instruction to the Radio Core
  * @param Instr        (u8) Chipcon instruction byte
  * @param ReadBytes    (u8) Number of bytes to AutoRead (0 to 2)
  * @param InputData    (u8*) pointer to string where Write byte is stored
  * @param OutputData   (u8*) pointer to string where Read bytes are stored
  * @retval u8 :        The Status Byte returned by the Radio Core 
  * @ingroup CC430_lib
  *
  * RF_Instruction() works with radio core instructions, which are much like 
  * command strobes except that they may have input and output data beyond the
  * Status byte alone.  There may be as many as two bytes returned via the
  * OutputData String.
  *
  * If the AutoRead feature is not used, set AutoRead to 0.  If no Data is 
  * expected at the output, set OutputData to NULL.
  *
  * If there is no input data, set InputByte to NULL
 
u8 RF_Instruction( u8 Instr, u8 ReadBytes, u8* InputByte, u8* OutputData );
 */

#endif

