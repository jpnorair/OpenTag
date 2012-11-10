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
/**
  * @file       /Platforms/CC430/cc430_lib/cc430_rf.c
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Dec 2009
  * @brief      Library resources for RF peripheral
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */

#include "cc430_lib.h"

#ifdef __CC430__

// Workaround for the delay slot: assumes 19.97 MHz (20 MHz)
#define CLOCKS_PER_1us  20


/* --------------- RF Private Function Prototypes ----------------- */






/* --------------- RF Interface Functions ----------------- */




void RF_BigEndian( FunctionalState NewState ) {
    if (NewState != DISABLE) {
        RFWord->IFCTL0 |= RF_CTL_ConvertEndian;
    }
    else {
        RFWord->IFCTL0 &= ~RF_CTL_ConvertEndian;
    }
}




void RF_IFITConfig(u16 RF_IFITMask, FunctionalState NewState) {
    if (NewState != DISABLE) {
        RFWord->IFCTL1 |= RF_IFITMask;
    }
    else {
        RFWord->IFCTL1 &= ~RF_IFITMask;
    }
}




bool RF_GetIFITStatus(u16 RF_IFITMask) {
    RF_IFITMask >>= 8;
    return (bool)( (RFWord->IFCTL1 & RF_IFITMask) != 0 );
}




void RF_ClearIFITPendingBit(u16 RF_IFITMask) {
    RF_IFITMask >>= 8;
    RFWord->IFCTL1 &= ~RF_IFITMask;
}




bool RF_GetErrorFlagStatus(u16 RF_ErrorMask) {
    return (bool)( (RFWord->IFERR & RF_ErrorMask) != 0 );
}




void RF_ClearErrorFlag(u16 RF_ErrorMask) {
    RFWord->IFERR &= ~RF_ErrorMask;
}







/* --------------- RF Core Functions ----------------- */


void RF_CoreITConfig(u32 RF_CoreITMask, FunctionalState NewState) {
    u16 EdgeMask        = (u16)(RF_CoreITMask);
    u16 InterruptMask   = (u16)(RF_CoreITMask >> 16);
    
    // Clear the Edge Select
    RFWord->IES &= ~InterruptMask;

    // Set or Clear the Interrupt Select
    if ( NewState != DISABLE ) {
        RFWord->IE  |= InterruptMask;
        RFWord->IES |= EdgeMask;
    }
    else {
        RFWord->IE  &= ~InterruptMask;
    }
}




bool RF_GetCoreITStatus(u32 RF_CoreITMask) {
    return (bool)( (RFWord->IFG & (u16)(RF_CoreITMask >> 16)) );
}




bool RF_GetCoreITLevel(u32 RF_CoreITMask) {
    return (bool)( (RFWord->IN & (u16)(RF_CoreITMask >> 16)) );
}




void RF_ClearCoreITPendingBit(u32 RF_CoreITMask) {
    RFWord->IFG &= ~(u16)(RF_CoreITMask >> 16);
}






/** @brief Initializes the Radio Core Registers
  * @param RF_CoreInitStruct : (RF_CoreInitType*) Initialization Structure
  * @retval none 
  * @ingroup CC430_lib
  */
void RF_CoreInit( RF_CoreInitType* RF_CoreInitStruct ) {
    
    /*
    RF_WriteSingleReg( RF_CoreReg_IOCFG2, RF_CoreInitStruct->CC_IOCFG2 );
    RF_WriteSingleReg( RF_CoreReg_IOCFG1, RF_CoreInitStruct->CC_IOCFG1 );
    RF_WriteSingleReg( RF_CoreReg_IOCFG0, RF_CoreInitStruct->CC_IOCFG0 );
    RF_WriteSingleReg( RF_CoreReg_FIFOTHR, RF_CoreInitStruct->CC_FIFOTHR );
    RF_WriteSingleReg( RF_CoreReg_SYNC1, RF_CoreInitStruct->CC_SYNC1 );
    RF_WriteSingleReg( RF_CoreReg_SYNC0, RF_CoreInitStruct->CC_SYNC0 );
    RF_WriteSingleReg( RF_CoreReg_PKTLEN, RF_CoreInitStruct->CC_PKTLEN );
    RF_WriteSingleReg( RF_CoreReg_PKTCTRL1, RF_CoreInitStruct->CC_PKTCTRL1 );
    RF_WriteSingleReg( RF_CoreReg_PKTCTRL0, RF_CoreInitStruct->CC_PKTCTRL0 );
    RF_WriteSingleReg( RF_CoreReg_ADDR, RF_CoreInitStruct->CC_ADDR );
    RF_WriteSingleReg( RF_CoreReg_CHANNR, RF_CoreInitStruct->CC_CHANNR );
    RF_WriteSingleReg( RF_CoreReg_FSCTRL1, RF_CoreInitStruct->CC_FSCTRL1 );
    RF_WriteSingleReg( RF_CoreReg_FSCTRL0, RF_CoreInitStruct->CC_FSCTRL0 );
    RF_WriteSingleReg( RF_CoreReg_FREQ2, RF_CoreInitStruct->CC_FREQ2 );
    RF_WriteSingleReg( RF_CoreReg_FREQ1, RF_CoreInitStruct->CC_FREQ1 );
    RF_WriteSingleReg( RF_CoreReg_FREQ0, RF_CoreInitStruct->CC_FREQ0 );
    RF_WriteSingleReg( RF_CoreReg_MDMCFG4, RF_CoreInitStruct->CC_MDMCFG4 );
    RF_WriteSingleReg( RF_CoreReg_MDMCFG3, RF_CoreInitStruct->CC_MDMCFG3 );
    RF_WriteSingleReg( RF_CoreReg_MDMCFG2, RF_CoreInitStruct->CC_MDMCFG2 );
    RF_WriteSingleReg( RF_CoreReg_MDMCFG1, RF_CoreInitStruct->CC_MDMCFG1 );
    RF_WriteSingleReg( RF_CoreReg_MDMCFG0, RF_CoreInitStruct->CC_MDMCFG0 );
    RF_WriteSingleReg( RF_CoreReg_DEVIATN, RF_CoreInitStruct->CC_DEVIATN );
    RF_WriteSingleReg( RF_CoreReg_MCSM2, RF_CoreInitStruct->CC_MCSM2 );
    RF_WriteSingleReg( RF_CoreReg_MCSM1, RF_CoreInitStruct->CC_MCSM1 );
    RF_WriteSingleReg( RF_CoreReg_MCSM0, RF_CoreInitStruct->CC_MCSM0 );
    RF_WriteSingleReg( RF_CoreReg_FOCCFG, RF_CoreInitStruct->CC_FOCCFG );
    RF_WriteSingleReg( RF_CoreReg_BSCFG, RF_CoreInitStruct->CC_BSCFG );
    RF_WriteSingleReg( RF_CoreReg_AGCCTRL2, RF_CoreInitStruct->CC_AGCCTRL2 );
    RF_WriteSingleReg( RF_CoreReg_AGCCTRL1, RF_CoreInitStruct->CC_AGCCTRL1 );
    RF_WriteSingleReg( RF_CoreReg_AGCCTRL0, RF_CoreInitStruct->CC_AGCCTRL0 );
    RF_WriteSingleReg( RF_CoreReg_WOREVT1, RF_CoreInitStruct->CC_WOREVT1 );
    RF_WriteSingleReg( RF_CoreReg_WOREVT0, RF_CoreInitStruct->CC_WOREVT0 );
    RF_WriteSingleReg( RF_CoreReg_WORCTRL, RF_CoreInitStruct->CC_WORCTRL );
    RF_WriteSingleReg( RF_CoreReg_FREND1, RF_CoreInitStruct->CC_FREND1 );
    RF_WriteSingleReg( RF_CoreReg_FREND0, RF_CoreInitStruct->CC_FREND0 );
    RF_WriteSingleReg( RF_CoreReg_FSCAL3, RF_CoreInitStruct->CC_FSCAL3 );
    RF_WriteSingleReg( RF_CoreReg_FSCAL2, RF_CoreInitStruct->CC_FSCAL2 );
    RF_WriteSingleReg( RF_CoreReg_FSCAL1, RF_CoreInitStruct->CC_FSCAL1 );
    RF_WriteSingleReg( RF_CoreReg_FSCAL0, RF_CoreInitStruct->CC_FSCAL0 );
    RF_WriteSingleReg( RF_CoreReg_TEST2, RF_CoreInitStruct->CC_TEST2 );         // Then write the test regs
    RF_WriteSingleReg( RF_CoreReg_TEST1, RF_CoreInitStruct->CC_TEST1 );
    RF_WriteSingleReg( RF_CoreReg_TEST0, RF_CoreInitStruct->CC_TEST0 );
    */
    
    RF_WriteBurstReg(RF_CoreReg_IOCFG2, &(RF_CoreInitStruct->CC_IOCFG2), 39); // Write first 39 (0x00 - 0x26) registers
    RF_WriteBurstReg(RF_CoreReg_TEST2, &(RF_CoreInitStruct->CC_TEST2), 3);      // Then write the test regs
}






u8 RF_CmdStrobe( u8 Strobe ) {
    u8 statusByte;
    //u16 gdo_state;    ///@note, always permanently set GDO2 to 0x29
  
    // Check for valid strobe command 
    if ((Strobe == 0xBD) || ((Strobe >= RF_SRES) && (Strobe <= RF_SNOP))) {
     
        RFWord->IFCTL1 &= ~(RFSTATIFG);                                             // Clear the Status read flag
        while( !(RFWord->IFCTL1 & RFINSTRIFG));                                     // Wait for radio to be ready for next instruction

        // Write the strobe instruction
        if ((Strobe > RF_SRES) && (Strobe < RF_SNOP)) {                         // If strobe is anything except Reset or NOP ...
            //gdo_state = RF_ReadSingleReg(IOCFG2);                                      // buffer IOCFG2 state
            //RF_WriteSingleReg(IOCFG2, 0x29);                                           // chip-ready to GDO2
            RFByte->INSTRB = Strobe;
            
            // If chip is in sleep mode, wait for RF_RDY (idle state) before
            // processing the strobe.  The exceptions are sXOFF, SPWD, SWOR, SFRX, SFTX, 
            // which can be run from sleep.
            //@note: This code is basically a debugging trap, because execution *WILL*
            //       get locked into the while loop if a strobe is sent without idle.
            if ((RFWord->IN&0x04) == 0x04) {
                if ((Strobe != RF_SXOFF) && ((Strobe & 0x0C) != 0x08)) {
                    while ((RFWord->IN&0x04) == 0x04);
                    if ((UCS->CTL6 & XT2OFF) != 0)
                    	__delay_cycles(810*CLOCKS_PER_1us);
                }
            }
            //RF_WriteSingleReg(IOCFG2, gdo_state);                                      // restore IOCFG2 setting

            while( !(RFWord->IFCTL1 & RFSTATIFG) );
        }
        else {                                                                  // Else (strobe is Reset or NOP)	
            RFByte->INSTRB = Strobe;
        }
        
    statusByte = RFByte->STATB;
    }
  
    return statusByte;
}





void RF_CoreDump( u8 *buffer ) {
    RF_ReadBurstReg(RF_CoreReg_IOCFG2, buffer, 64);
}





u8 RF_ReadSingleReg (u8 addr) {
    u8 data_out;
  
  // Check for valid configuration register address, 0x3E refers to PATABLE 
  if ((addr <= 0x2E) || (addr == 0x3E))
    // Send address + Instruction + 1 dummy byte (auto-read)
    RFByte->INSTR1B = (addr | RF_SNGLREGRD);
  else
    // Send address + Instruction + 1 dummy byte (auto-read)
    RFByte->INSTR1B = (addr | RF_STATREGRD);
  
  while (!(RFWord->IFCTL1 & RFDOUTIFG) );       ///@todo there is a bug somewhere in here
  data_out = RFByte->DOUTB;                    // Read data and clears the RFDOUTIFG

  return data_out;
}




void RF_WriteSingleReg(u8 addr, u8 value) {
  while (!(RFWord->IFCTL1 & RFINSTRIFG));       // Wait for the Radio to be ready for next instruction
  RFByte->INSTRB = (addr | RF_SNGLREGWR);	    // Send address + Instruction

  RFByte->DINB = value; 			    // Write data in

  __no_operation(); 
}




void RF_ReadBurstReg(u8 addr, u8 *buffer, u8 count) {
    u8 i;
  
    if(count > 0) {
        while (!(RFWord->IFCTL1 & RFINSTRIFG));       // Wait for INSTRIFG
    
        RFByte->INSTR1B = (addr | RF_REGRD);          // Send addr of first conf. reg. to be read
                                              // ... and the burst-register read instruction
        for (i = 0; i < (count-1); i++) {
            while (!(RFDOUTIFG & RFWord->IFCTL1));        // Wait for the Radio Core to update the RF1ADOUTB reg
            buffer[i] = RFByte->DOUT1B;                 // Read DOUT from Radio Core + clears RFDOUTIFG
                                              // Also initiates auo-read for next DOUT byte
        }
        
        buffer[count-1] = RFByte->DOUTB;             // Store the last DOUT from Radio Core
    }
}  




void RF_WriteBurstReg(u8 addr, u8 *buffer, u8 count) {  
    u8 i;

    if(count > 0) {
        while (!(RFWord->IFCTL1 & RFINSTRIFG));       // Wait for the Radio to be ready for next instruction
        
        RFWord->INSTRW = ((addr | RF_REGWR)<<8 ) + buffer[0]; // Send address + Instruction
  
        for (i = 1; i < count; i++) {
            RFByte->DINB = buffer[i];                   // Send data
            while (!(RFDINIFG & RFWord->IFCTL1));       // Wait for TX to finish
        } 
        i = RFByte->DOUTB;                            // Reset RFDOUTIFG flag which contains status byte
    }
}




void RF_Reset( ) {
    RF_CmdStrobe(RF_SRES);                          // Reset the Radio Core
    RF_CmdStrobe(RF_SNOP);                          // Reset Radio Pointer
}



void RF_WritePATable( u8 power_value ) {
    while( !(RFWord->IFCTL1 & RFINSTRIFG));
    RFWord->INSTRW = 0x3E00 + power_value;              // PA Table single write

    while( !(RFWord->IFCTL1 & RFINSTRIFG));
    RFByte->INSTRB = RF_SNOP;                     // reset PA_Table pointer
}



void RF_WriteBurstPATable(u8 *buffer, u8 count) {
    volatile u8 i = 0; 
    
    while( !(RFWord->IFCTL1 & RFINSTRIFG));
    RFWord->INSTRW = 0x7E00 + buffer[i];          // PA Table burst write
    
    for (i = 1; i < count; i++) {
        RFByte->DINB = buffer[i];                   // Send data
        while (!(RFDINIFG & RFWord->IFCTL1));       // Wait for TX to finish
    } 
    i = RFByte->DOUTB;                            // Reset RFDOUTIFG flag which contains status byte
    
    while( !(RFWord->IFCTL1 & RFINSTRIFG));
    RFByte->INSTRB = RF_SNOP;                     // reset PA Table pointer
}



void RF_FIFOTransmit(u8 *buffer, u8 length) {
    RFWord->IES |= BIT9;
    RFWord->IFG &= ~BIT9;                         // Clear pending interrupts
    RFWord->IE |= BIT9;                           // Enable RFIFG9 TX end-of-packet interrupts
  
    // RFWord->IN_9 => Rising edge indicates SYNC sent/received and
    //             Falling edge indicates end of packet.
    //             Configure it to interrupt on falling edge. 
    RF_WriteBurstReg(RF_TXFIFOWR, buffer, length);     
  
    RF_CmdStrobe( RF_STX );                         // Strobe STX   
}



void RF_ReceiveOn( ) {  
    RFWord->IFG &= ~BIT4;                         // Clear a pending interrupt
    RFWord->IE  |= BIT4;                          // Enable the interrupt
    RF_CmdStrobe( RF_SIDLE );
    RF_CmdStrobe( RF_SRX );                      
}



void RF_ReceiveOff( ) {
    RFWord->IE &= ~BIT4;                          // Disable RX interrupts
    RFWord->IFG &= ~BIT4;                         // Clear pending IFG
    RF_CmdStrobe( RF_SIDLE );
    RF_CmdStrobe( RF_SFRX);      // Flush the receive FIFO of any residual data 
}





u8 RF_GetMARCState() {
    return RF_ReadSingleReg( RF_CoreReg_MARCSTATE );
}


u8 RF_GetFREQEST() {
    return RF_ReadSingleReg( RF_CoreReg_FREQEST );
}


u8 RF_GetRSSI() {
    return RF_ReadSingleReg( RF_CoreReg_RSSI );
}


u8 RF_GetLQI() {
    return RF_ReadSingleReg( RF_CoreReg_LQI );
}


u16 RF_GetWORTime() {
    
    union {
        u16 Short;
        u8  Byte[2];   
    } val_WORTIME;

    val_WORTIME.Byte[1] = RF_ReadSingleReg( RF_CoreReg_WORTIME1 );
    val_WORTIME.Byte[0] = RF_ReadSingleReg( RF_CoreReg_WORTIME0 );
    
    return val_WORTIME.Short;
}


u8 RF_GetPKTSTATUS() {
    return RF_ReadSingleReg( RF_CoreReg_PKTSTATUS );
}


u8 RF_GetTXBYTES() {
    return RF_ReadSingleReg( RF_CoreReg_TXBYTES );
}


u8 RF_GetRXBYTES() {
    return RF_ReadSingleReg( RF_CoreReg_RXBYTES );
}



void RF_GetLinkInfo(RF_LinkInfoType* RF_LinkInfo) {
    RF_LinkInfo->CC_FREQEST = RF_GetFREQEST();
    RF_LinkInfo->CC_LQI     = RF_GetLQI();
    RF_LinkInfo->CC_RSSI    = RF_GetRSSI();
}



void RF_GetTestInfo(RF_TestInfoType* RF_TestInfo) {
    RF_TestInfo->CC_FSTEST      = RF_ReadSingleReg( RF_CoreReg_FSTEST );
    RF_TestInfo->CC_PTEST       = RF_ReadSingleReg( RF_CoreReg_PTEST );
    RF_TestInfo->CC_AGCTEST     = RF_ReadSingleReg( RF_CoreReg_AGCTEST );
    RF_TestInfo->CC_TEST2       = RF_ReadSingleReg( RF_CoreReg_TEST2 );
    RF_TestInfo->CC_TEST1       = RF_ReadSingleReg( RF_CoreReg_TEST1 );
    RF_TestInfo->CC_TEST0       = RF_ReadSingleReg( RF_CoreReg_TEST0 );
    RF_TestInfo->CC_VCO_VC_DAC  = RF_ReadSingleReg( RF_CoreReg_VCO_VC_DAC );
}



void RF_GetSystemInfo(RF_SystemInfoType* RF_SystemInfo) {
    RF_SystemInfo->CC_PARTNUM = RF_ReadSingleReg( RF_CoreReg_PARTNUM );
    RF_SystemInfo->CC_VERSION = RF_ReadSingleReg( RF_CoreReg_VERSION );
}














/*
// Called if an interface error has occured. No interface errors should 
// exist in application code, so this is intended to be used for debugging
// or to catch errant operating conditions. 
static void RF_interface_error_handler(void) {

    switch(__even_in_range(RFWord->IFERRV,8)) {
        case 0: break;                          // No error
        
        case 2: P1OUT &= ~BIT0;						// 00 = on LED's [D2,D1]
                P3OUT &= ~BIT6; 
                __no_operation();
                break;
                
        case 4: P1OUT |= BIT0;						// 01 = on LED's [D2,D1]
                P3OUT &= ~BIT6; 
                __no_operation();
                break;  
    
        case 6: P1OUT &= ~BIT0;						// 10 = on LED's [D2,D1]
                P3OUT |= BIT6; 
                __no_operation();
                break;
        
        case 8: P1OUT |= BIT0;						// 11 = on LED's [D2,D1]
                P3OUT |= BIT6; 
                __no_operation();
                break; 
    }
}
*/






/*
u8 RF_Instruction( u8 Instr, u8 ReadBytes, u8* InputByte, u8* OutputData ) {

    u8 status;

    // Read Instructions (the basic default)
    // Uses 1 or 2 byte autoread functionality
    if ( ReadBytes != 0 ) {
        
        if (InputByte != NULL) {
            RFByte->DINB    = *InputByte;
        }
    
        if ( ReadBytes == 1 ) {
            RFByte->INSTR1B         = Instr;
            *OutputData         = RFByte->DOUTB;
            status              = RFByte->STAT1B;
        }
        else {
            RFByte->INSTR2B         = Instr;
            (u16*)(*OutputData) = RFWord->DOUTW;
            status              = RFByte->STAT2B;
        }
    }
    
    // Write Instructions
    // Access to CC430 RF Core allows only 1 byte writes
    else if (InputByte != NULL ) {
        RFByte->DINB    = *InputByte;
        RFByte->INSTRB  = Instr;
        status      = RFByte->STATB;
    }
    
    // Strobe Instructions
    else {
        status = RF_CmdStrobe( Instr );
    }
    
    return status;
}
*/

#endif


