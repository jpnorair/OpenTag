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
/** @file       /Platforms/CC430/cc430_lib/cc430_flash.c
  * @author     JP Norair
  * @version    V1.0
  * @date       15 Nov 2011
  * @brief      Library resources for Flash peripheral
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */



#include "cc430_lib.h"


/** @note
  * For writing and erasing Flash, the watchdog timer needs to be halted.  Each
  * function in this module that writes or erases flash will temporarily halt
  * the watchdog timer.  The implementation directly accesses the WDT registers,
  * so referencing the watchdog module is unnecessary
  */

#define STOP_WDT()      ( WDTA->CTL = ( 0x5A80 | (WDTA->CTL & 0x007F) ) )
#define START_WDT()     ( WDTA->CTL = ( 0x5A00 | (WDTA->CTL & 0x007F) ) )
#define RESUME_WDT(VAL) ( WDTA->CTL = ( 0x5A00 | VAL | (WDTA->CTL & 0x007F) ) )

#define FLASH_READY()   ( FLASH_GetStatus() & 0x08 )
#define FLASH_FLAGS()   








u8 FLASH_Erase( u16 flags, u16* DummyAddress );
u8 FLASH_PrepWrite(u16 flags);
u8 FLASH_CheckWrite();




#define SAVE_GIE(VAL)       do { \
                                VAL = (__get_SR_register() & GIE); \
                                __bic_SR_register(GIE); \
                            } while(0)

#define REVERT_GIE(VAL)     __bis_SR_register(VAL)



/*
 it is recommended to set or clear this bit using BIS.B or BIC.B instructions, 
 rather than MOV.B or CLR.B instructions. See the System Resets, Interrupts, 
 and Operating Modes, System Control Module (SYS) chapter for more details.
 */
void FLASH_ITConfig( FunctionalState NewState ) {

    if (NewState != DISABLE) {
        SFRIE1 |= FLASH_SystemIT_ACCVIE;
    }
    else {
        SFRIE1 &= ~FLASH_SystemIT_ACCVIE;
    }
}


/** @brief Check Flash controller's single interrupt (Access Violation)
  * @param none
  * @retval bool : TRUE if set
  * @ingroup CC430_lib
  */
bool FLASH_GetITStatus( ) {
    
    if ( (FLASH->CTL3 & FLASH_CTL_ACCVIFG) != 0 ) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}



void FLASH_ClearITPendingBit( ) {
    FLASH->CTL3 = 0xA500 | (FLASH->CTL3 & 0x00FB);
}



u8 FLASH_GetStatus( ) {
    u8 status;
    
    /// First, flip the WAIT / Ready Status bit
    status = (FLASHb->CTL3_L & FLASH_CTL_WAIT) ^ FLASH_CTL_WAIT;
    
    /// Second, load the rest of the bits
    status |= FLASHb->CTL3_L & ( FLASH_CTL_KEYV | FLASH_CTL_BUSY );
    
    /// Third, add the Volt Error bit
    if ( (FLASH->CTL4 & FLASH_CTL_VPE) != 0 ) {
        status |= (u8)FLASH_VoltError;
    }
    
    return status;
}



u8 FLASH_GetLockStatus( ) {
    return FLASHb->CTL3_L & ( FLASH_CTL_LOCKA | FLASH_CTL_LOCK );
}




void FLASH_ClearStatus( ) {
    FLASH->CTL3 = 0xA500 | (FLASH->CTL3 & 0x00FD);
    FLASH->CTL4 = 0xA500 | (FLASH->CTL4 & 0x00FE);
}




u8 FLASH_EraseSegment( u16* SegAddress ) {
    if ( ((u16)SegAddress >= 0x1980) && ((u16)SegAddress < (0x1A00)) ) {
        if (FLASH->CTL3 & 0x0040) {
            FLASH->CTL3 |= 0x0040;
        }
    }
    return FLASH_Erase( (FLASH_CTL_ERASE), SegAddress );
}


u8 FLASH_EraseBank( u16* BankAddress ) {
    if ( ((u16)BankAddress >= 0x1800) && ((u16)BankAddress < (0x1A00)) ) {
        if (FLASH->CTL3 & 0x0040) {
            //FLASH->CTL3 |= 0x0040;
        }
    }
    return FLASH_Erase( (FLASH_CTL_MERAS), BankAddress );
}


u8 FLASH_EraseAll( ) {
    if (FLASH->CTL3 & 0x0040) {
        FLASH->CTL3 |= 0x0040;
    }
    return FLASH_Erase( (FLASH_CTL_MERAS | FLASH_CTL_ERASE), (u16*)FLASH_BankBase_Main0 );
}



u8 FLASH_WriteByte( u8* Address, u8 Data ) {
    u8 wdtstate;
    u16 saved_gie;
    
    SAVE_GIE(saved_gie);
    wdtstate = FLASH_PrepWrite(WRT);
    *Address = Data;
    RESUME_WDT(wdtstate);
    REVERT_GIE(saved_gie);
    
    return FLASH_CheckWrite();
}

u8 FLASH_WriteShort( u16* Address, u16 Data ) {
    u8 wdtstate;
    u16 saved_gie;
    
    SAVE_GIE(saved_gie);
    wdtstate = FLASH_PrepWrite(WRT);
    *Address = Data;
    RESUME_WDT(wdtstate);
    REVERT_GIE(saved_gie);
    
    return FLASH_CheckWrite();
}

u8 FLASH_WriteLong( u32* Address, u32 Data ) {
    u8 wdtstate;
    u16 saved_gie;
    
    SAVE_GIE(saved_gie);
    wdtstate = FLASH_PrepWrite(WRT);
    *Address = Data;
    RESUME_WDT(wdtstate);
    REVERT_GIE(saved_gie);
    
    return FLASH_CheckWrite();
}



u8 FLASH_WriteByteBlock( u8* Address, u8* Data, u16 Length ) {
    u8 wdtstate;
    u16 saved_gie;
    
    SAVE_GIE(saved_gie);
    wdtstate = FLASH_PrepWrite(WRT);
    while (Length > 0) {
        while (FLASH->CTL3 & BUSY);
        *Address = *Data;
        Length--;
        Data++;
        Address++;
    }
    RESUME_WDT(wdtstate);
    REVERT_GIE(saved_gie);
    
    return FLASH_CheckWrite();
}

u8 FLASH_WriteShortBlock( u16* Address, u16* Data, u16 Length ) {
    u8 wdtstate;
    u16 saved_gie;
    
    SAVE_GIE(saved_gie);
    wdtstate = FLASH_PrepWrite(WRT);
    while (Length > 0) {
        while (FLASH->CTL3 & BUSY);
        *Address = *Data;
        Length--;
        Data++;
        Address++;
    }
    RESUME_WDT(wdtstate);
    REVERT_GIE(saved_gie);
    
    return FLASH_CheckWrite();
}

u8 FLASH_WriteLongBlock( u32* Address, u32* Data, u16 Length ) {
    u8 wdtstate;
    u16 saved_gie;
    
    SAVE_GIE(saved_gie);
    wdtstate = FLASH_PrepWrite(BLKWRT);
    while (Length > 0) {
        while (FLASH->CTL3 & BUSY);
        *Address = *Data;
        Length--;
        Data++;
        Address++;
    }
    RESUME_WDT(wdtstate);
    REVERT_GIE(saved_gie);
    
    return FLASH_CheckWrite();
}




u8 FLASH_ProgramBlock( u32* Address, u32* Data, u16 Length) {
    u8 status;
    u8 wdtstate;
    
    /// 1. assure Flash Status is good before doing operations
    status = FLASH_GetStatus();
    if ( status == 0 ) {
        u16 saved_gie;
        
        /// 2. Temporarily Disable Watchdog & general interrupts
        SAVE_GIE(saved_gie);
        wdtstate = WDTAb->CTL_L & 0x80;
        STOP_WDT();
    
        /// 3. Set Flash Controller for Long Block Access
        FLASH->CTL1 = FLASH_FWKEY | FLASH_CTL_BLKWRT | FLASH_CTL_WRT;
    
        /// 4. Write Long Block
        while ( Length > 0 ) {
            while (FLASH->CTL3 & BUSY);
            *Address = *Data;
            Length--;
            Data++;
            Address++;
        }
    
        status = FLASH_CheckWrite();
        RESUME_WDT(wdtstate);
        REVERT_GIE(saved_gie);
    }
    
    return status;
}




/** @brief General purpose Flash Erase function
  * @param flags : (u16) Flash erase method flags
  * @param DummyAddress : (u16) Address of Segment, Bank, or Mass to erase
  * @retval u8 : Logical OR'ed group of status flags as described in FlashStatus enum datatype
  * @ingroup CC430_lib
  *
  * Appropriate Flash controller Segments must be unlocked before use!
  */
u8 FLASH_Erase( u16 flags, u16* DummyAddress ) {
    u8 status;
    u8 wdtstate;
    
    /// 1. assure Flash Status is good before doing operations
    status = FLASH_GetStatus();
    if ( status == 0 ) {
        u16 saved_gie;
        
        /// 2. Temporarily Disable Watchdog & general interrupts
        SAVE_GIE(saved_gie);
        wdtstate = WDTAb->CTL_L & 0x80;
        STOP_WDT();
    
        /// 3. Setup Flash Controller and Erase Mode, note clearing of lock bits
        FLASH->CTL3 = FWKEY;
        FLASH->CTL1 = FLASH_FWKEY | flags;
    
        /// 4. Dummy Write to base address
        *DummyAddress = 0xFFFF;
        
        status = FLASH_CheckWrite();
        RESUME_WDT(wdtstate);
        REVERT_GIE(saved_gie);
    }
    
    return status;
}





/** @brief Prepares Regs for FLASH Write
  * @param flags : (u16) Flash erase method flags
  * @retval u8 : state of watchdog timer
  * @ingroup CC430_lib
  */
u8 FLASH_PrepWrite(u16 flags) {
    u8 wdtstate;

    /// 1. Temporarily Disable Watchdog
    wdtstate = WDTAb->CTL_L & 0x80;
    STOP_WDT();
    
    /// 2. Set Flash Controller for access
    ///    Note that Lock bits are cleared
    FLASH->CTL3 = FWKEY;
    FLASH->CTL1 = FWKEY | flags;
    
    return wdtstate;
}




/** @brief Checks and completes the write process
  * @param None
  * @retval u8 : Logical OR'ed group of status flags as described in FlashStatus enum datatype
  * @ingroup CC430_lib
  */
u8 FLASH_CheckWrite() {
    
    /// 1. Terminate Write
    FLASH->CTL1 = FWKEY;                            // Clear Erase bit
    FLASH->CTL3 = FWKEY+LOCK+LOCKA;
    
    /// 
    
    return FLASH_GetStatus();
}
