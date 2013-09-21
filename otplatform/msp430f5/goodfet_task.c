/* Copyright 2010-2012 JP Norair
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
  * @file       /omg/base/goodfet_msp430x2.c
  * @author     JP Norair
  * @version    R104
  * @date       17 Nov 2012
  * @brief      GoodFET task for implementing JTAG for MSP430X2 cores
  * @defgroup   GoodFET_MSP430X2
  * @ingroup    GoodFET_MSP430X2
  *
  *
  ******************************************************************************
  */

#include "OTAPI.h"

#if (OMG_FEATURE(GOODFET))





// jtag.h

#define JTAG            0x10
#define JTAG430X2       0x11
#define JTAG430         0x16

#define MSP430MODE      0
#define MSP430XMODE     1
#define MSP430X2MODE    2



//! All states in the JTAG TAP
enum eTAPState {
    UNKNOWN             = 0x0000,
    TEST_LOGIC_RESET    = 0x0001,
    RUN_TEST_IDLE       = 0x0002,
    SELECT_DR_SCAN      = 0x0004,
    CAPTURE_DR          = 0x0008,
    SHIFT_DR            = 0x0010,
    EXIT1_DR            = 0x0020,
    PAUSE_DR            = 0x0040,
    EXIT2_DR            = 0x0080,
    UPDATE_DR           = 0x0100,
    SELECT_IR_SCAN      = 0x0200,
    CAPTURE_IR          = 0x0400,
    SHIFT_IR            = 0x0800,
    EXIT1_IR            = 0x1000,
    PAUSE_IR            = 0x2000,
    EXIT2_IR            = 0x4000,
    UPDATE_IR           = 0x8000
};

//! jtag_trans_n flags
enum eTransFlags {
    MSB                 = 0x0,
    LSB                 = 0x1,
    NOEND               = 0x2,
    NORETIDLE           = 0x4
};


extern int savedtclk;

extern unsigned char jtagid;

//! the global state of the JTAG TAP
extern enum eTAPState jtag_state;



//jtag430.h
extern unsigned int drwidth;
extern unsigned int jtag430mode;



//Pins.  Both SPI and JTAG names are acceptable.
//#define SS            BIT0
#define MOSI            BIT1
#define MISO            BIT2
#define SCK             BIT3

#define TMS             BIT0
#define TDI             BIT1
#define TDO             BIT2
#define TCK             BIT3

#define TCLK            TDI

//These are not on P5
#define RST             BIT6
#define TST             BIT0

//This could be more accurate.
//Does it ever need to be?
#define JTAGSPEED       20
#define JTAGDELAY(x)    delay(x)

#define SETMOSI         SPIOUT|=MOSI
#define CLRMOSI         SPIOUT&=~MOSI
#define SETCLK          SPIOUT|=SCK
#define CLRCLK          SPIOUT&=~SCK
#define READMISO        (SPIIN&MISO?1:0)
#define SETTMS          SPIOUT|=TMS
#define CLRTMS          SPIOUT&=~TMS
#define SETTCK          SPIOUT|=TCK
#define CLRTCK          SPIOUT&=~TCK
#define SETTDI          SPIOUT|=TDI
#define CLRTDI          SPIOUT&=~TDI

#define SETTST          P4OUT|=TST
#define CLRTST          P4OUT&=~TST
#define SETRST          P2OUT|=RST
#define CLRRST          P2OUT&=~RST

#define SETTCLK         SETTDI
#define CLRTCLK         CLRTDI

#define SAVETCLK        savedtclk = SPIOUT&TCLK;
#define RESTORETCLK     if(savedtclk) SPIOUT|=TCLK; else SPIOUT&=~TCLK

//JTAG commands
#define JTAG_IR_SHIFT               0x80
#define JTAG_DR_SHIFT               0x81
#define JTAG_RESET_TAP              0x82
#define JTAG_RESET_TARGET           0x83
#define JTAG_DETECT_IR_WIDTH        0x84
#define JTAG_DETECT_CHAIN_LENGTH    0x85
#define JTAG_GET_DEVICE_ID          0x86
//#define JTAG_DR_SHIFT20           0x91






//16-bit MSP430 JTAG commands, bit-swapped
#define IR_CNTRL_SIG_16BIT          0xC8   // 0x13
#define IR_CNTRL_SIG_CAPTURE        0x28   // 0x14
#define IR_CNTRL_SIG_RELEASE        0xA8   // 0x15

// Instructions for the JTAG Fuse
#define IR_PREPARE_BLOW             0x44   // 0x22
#define IR_EX_BLOW                  0x24   // 0x24

// Instructions for the JTAG data register
#define IR_DATA_16BIT               0x82   // 0x41
#define IR_DATA_QUICK               0xC2   // 0x43

// Instructions for the JTAG PSA mode
#define IR_DATA_PSA                 0x22   // 0x44
#define IR_SHIFT_OUT_PSA            0x62   // 0x46

// Instructions for the JTAG address register
#define IR_ADDR_16BIT               0xC1   // 0x83
#define IR_ADDR_CAPTURE             0x21   // 0x84
#define IR_DATA_TO_ADDR             0xA1   // 0x85

// Bypass instruction
#define IR_BYPASS                   0xFF   // 0xFF

//MSP430X2 unique
#define IR_COREIP_ID                0xE8   // 0x17 
#define IR_DEVICE_ID                0xE1   // 0x87

//MSP430 or MSP430X
#define MSP430JTAGID                0x89

//MSP430X2 only
#define MSP430X2JTAGID              0x91


// Global Commands
#define READ    0x00
#define WRITE   0x01
#define PEEK    0x02
#define POKE    0x03
#define SETUP   0x10
#define START   0x20
#define STOP    0x21
#define CALL    0x30
#define EXEC    0x31
#define LIMIT   0x7B    // limit reached 
#define EXIST   0x7C    // already or doesnt exist 
#define NMEM    0x7D    // OOM 
#define NOK     0x7E
#define OK      0x7F

//JTAG430 commands
#define JTAG430_HALTCPU             0xA0
#define JTAG430_RELEASECPU          0xA1
#define JTAG430_SETINSTRFETCH       0xC1
#define JTAG430_SETPC               0xC2
#define JTAG430_SETREG              0xD2
#define JTAG430_GETREG              0xD3

#define JTAG430_WRITEMEM            0xE0
#define JTAG430_WRITEFLASH          0xE1
#define JTAG430_READMEM             0xE2
#define JTAG430_ERASEFLASH          0xE3
#define JTAG430_ERASECHECK          0xE4
#define JTAG430_VERIFYMEM           0xE5
#define JTAG430_BLOWFUSE            0xE6
#define JTAG430_ISFUSEBLOWN         0xE7
#define JTAG430_ERASEINFO           0xE8
#define JTAG430_COREIP_ID           0xF0
#define JTAG430_DEVICE_ID           0xF1







//! Returns true if we're in any of the data register states
int in_dr();

//! Returns true if we're in any of the instruction register states
int in_ir();

//! Returns true if we're in run-test-idle state
int in_run_test_idle();

//! Check the state
int in_state(enum eTAPState state);










// generic jtag

//! Returns true if we're in any of the data register states
ot_int in_dr() {
    return (ot_int)(jtag_state & (SELECT_DR_SCAN | CAPTURE_DR |
                               SHIFT_DR | EXIT1_DR | PAUSE_DR |
                               EXIT2_DR | UPDATE_DR));
}


//! Returns true if we're in any of the instruction register states
ot_int in_ir() {
    return (ot_int)(jtag_state & (SELECT_IR_SCAN | CAPTURE_IR |
                               SHIFT_IR | EXIT1_IR | PAUSE_IR |
                               EXIT2_IR | UPDATE_IR));
}


//! Returns true if we're in run-test-idle state
ot_int in_run_test_idle() {
    return (ot_int)(jtag_state & RUN_TEST_IDLE);
}


//! Check the state
ot_int in_state(enum eTAPState state) {
    return (ot_int)(jtag_state & state);
}


//! Reset the target device
void jtag_reset_target() {
    SETRST;
    led_toggle();
    CLRRST;
    led_toggle();
}


//! Clock the JTAG clock line
void jtag_tcktock() {
    CLRTCK; 
    led_toggle();
    SETTCK; 
    led_toggle();
}



//! Goes through test-logic-reset and ends in run-test-idle
void jtag_reset_tap() {
    CLRMOSI;
    SETTMS;
    jtag_tcktock();
    jtag_tcktock();
    jtag_tcktock();
    jtag_tcktock();
    jtag_tcktock();  // now in Reset state
    CLRTMS;
    jtag_tcktock();  // now in Run-Test/Idle state
    jtag_state = RUN_TEST_IDLE;
}



//! Set up the pins for JTAG mode.
void jtag_setup() {
    P5DIR|=MOSI+SCK+TMS;
    P5DIR&=~MISO;
    P4DIR|=TST;
    P2DIR|=RST;
    msdelay(100);
    jtag_state = UNKNOWN;
}



//! Stop JTAG, release pins
void jtag_stop() {
    P5OUT=0;
    P4OUT=0;
}



//! Get into Shift-IR or Shift-DR state
void jtag_shift_register() {
    // assumes we're in any state that can transition to Shift-IR or Shift-DR
    if (!in_state(CAPTURE_DR | CAPTURE_IR | SHIFT_DR | SHIFT_IR | EXIT2_DR | EXIT2_IR )) {
        debugstr("Invalid JTAG state");
        return;
    }

    CLRMOSI;
    CLRTMS;
    jtag_tcktock();

    if (in_dr())    jtag_state = SHIFT_DR;
    else            jtag_state = SHIFT_IR;
}




//! Get into Capture-IR state
void jtag_capture_ir() {
    // assumes you're in Run-Test/Idle, Update-DR or Update-IR
    if (!in_state(RUN_TEST_IDLE | UPDATE_DR | UPDATE_IR)) {
        debugstr("Invalid JTAG state");
        return;
    }

    CLRMOSI;
    SETTMS;
    jtag_tcktock(); // Select-DR-Scan
    jtag_tcktock(); // Select-IR-Scan
    CLRTMS;
    jtag_tcktock(); // Capture-IR

    jtag_state = CAPTURE_IR;
}




//! Get into Capture-DR state
void jtag_capture_dr() {
    // assumes you're in Run-Test/Idle, Update-DR or Update-IR
    if (!in_state(RUN_TEST_IDLE | UPDATE_DR | UPDATE_IR)) {
        debugstr("Invalid JTAG state");
        return;
    }

    CLRMOSI;
    SETTMS;
    jtag_tcktock(); // Select-DR-Scan
    CLRTMS;
    jtag_tcktock(); // Capture-IR

    jtag_state = CAPTURE_DR;
}

//! Gets back to run-test-idle without going through the test-logic-reset
void jtag_run_test_idle() {
    CLRMOSI;

    if (in_state(SELECT_DR_SCAN | SELECT_IR_SCAN)) {
        CLRTMS;
        jtag_tcktock();
        jtag_state <<= 1; //CAPTURE_DR or CAPTURE_IR
    }

    if (in_state(CAPTURE_DR | CAPTURE_IR)) {
        SETTMS;
        jtag_tcktock();
        jtag_state <<= 2; //EXIT1_DR or EXIT1_IR
    }

    if (in_state(SHIFT_DR | SHIFT_IR)) {
        SETTMS;
        jtag_tcktock();
        jtag_state <<= 1; //EXIT1_DR or EXIT1_IR
    }

    if (in_state(EXIT1_DR | EXIT1_IR)) {
        SETTMS;
        jtag_tcktock();
        jtag_state <<=3; //UPDATE_DR or UPDATE_IR
    }

    if (in_state(PAUSE_DR | PAUSE_IR)) {
        SETTMS;
        jtag_tcktock();
        jtag_state <<= 1; // EXIT2_DR or EXIT2_IR
    }

    if (in_state(EXIT2_DR | EXIT2_IR)) {
        SETTMS;
        jtag_tcktock();
        jtag_state <<= 1; // UPDATE_DR or UPDATE_IR
    }

    if (in_state(UPDATE_DR | UPDATE_IR | TEST_LOGIC_RESET)) {
        CLRTMS;
        jtag_tcktock();
        jtag_state = RUN_TEST_IDLE;
    }
}



int savedtclk;
//  NOTE: important: THIS MODULE REVOLVES AROUND RETURNING TO RUNTEST/IDLE, OR 
//  THE FUNCTIONAL EQUIVALENT
//! Shift N bits over TDI/TDO.  May choose LSB or MSB, and select whether to 
//  terminate (TMS-high on last bit) and whether to return to RUNTEST/IDLE
//      flags should be 0 for most uses.  
//      for the extreme case, flags should be  (NOEND|NORETDLE|LSB)
//      other edge cases can involve a combination of those three flags
//
//      the max bit-size that can be be shifted is 32-bits.  
//      for longer shifts, use the NOEND flag (which infers NORETIDLE so the 
//      additional flag is unnecessary)
//
//      NORETIDLE is used for special cases where (as with arm) the debug 
//      subsystem does not want to return to the RUN-TEST/IDLE state between 
//      setting IR and DR
ot_ulong jtag_trans_n(ot_ulong word, ot_u8 bitcount, enum eTransFlags flags) {
    ot_u8      bit;
    ot_ulong   high    = (1L << (bitcount - 1));
    ot_ulong   mask    = (high - 1);

    if (!in_state(SHIFT_IR | SHIFT_DR)) {
        debugstr("jtag_trans_n from invalid TAP state");
        return 0;
    }

    SAVETCLK;

    if (flags & LSB) {
        for (bit = bitcount; bit > 0; bit--) {
            /* write MOSI on trailing edge of previous clock */
            if (word & 1)    SETMOSI;
            else             CLRMOSI;

            word >>= 1;

            if ((bit == 1) && !(flags & NOEND))
                SETTMS; //TMS high on last bit to exit.

            jtag_tcktock();

            if ((bit == 1) && !(flags & NOEND))
                jtag_state <<= 1; // Exit1-DR or Exit1-IR

            // read MISO on trailing edge 
            if (READMISO) {
                word += (high);
            }
        }
    } 
    else {
        for (bit = bitcount; bit > 0; bit--) {
            /* write MOSI on trailing edge of previous clock */
            if (word & high) SETMOSI;
            else             CLRMOSI;
            
            word = (word & mask) << 1;

            if ((bit==1) && !(flags & NOEND))
                SETTMS; //TMS high on last bit to exit.

            jtag_tcktock();

            if ((bit == 1) && !(flags & NOEND))
                jtag_state <<= 1; // Exit1-DR or Exit1-IR

            // read MISO on trailing edge 
            word |= (READMISO);
        }
    }
    
    //This is needed for 20-bit MSP430 chips.
    //Might break another 20-bit chip, if one exists.
    if (bitcount==20) {
       word = ((word << 16) | (word >> 4)) & 0x000FFFFF;
    }
    
    RESTORETCLK;

    if (!(flags & NOEND)) {
        // exit state
        jtag_tcktock();

        jtag_state <<= 3; // Update-DR or Update-IR

        // update state
        if (!(flags & NORETIDLE)) {
            CLRTMS;
            jtag_tcktock();

            jtag_state = RUN_TEST_IDLE;
        }
    }

    return word;
}




//! Detects the width of the IR register
ot_u16 jtag_detect_ir_width() {
    ot_int i;
    ot_u16 width = 0;

    if (!in_run_test_idle()) {
        debugstr("Not in run-test-idle state");
        return 0;
    }

    // get to shift-ir state
    jtag_capture_ir();
    jtag_shift_register();

    // first we shift in 1024 zeros
    CLRMOSI;
    for (i = 0; i < 1024; i++) {
        jtag_tcktock();
    }

    // now we'll clock in ones until we see one
    SETMOSI;
    for (i = 0; i < 1024; i++) {
        jtag_tcktock();
        if (READMISO)
            break;
            
        width++;
    }

    // now get back to run-test-idle
    jtag_run_test_idle();

    return width;
}




//! Detects how many TAPs are in the JTAG chain
ot_u16 jtag_detect_chain_length() {
    ot_int i;
    ot_int bit;
    ot_u16 length = 0;

    if (!in_run_test_idle()) {
        debugstr("detect chain length must start from run-test-idle");
        return 0;
    }

    // The standard JTAG instruction for "bypass" mode is to set all 1's in the
    // instruction register.  When in "bypass" mode, the DR acts like a 1-bit
    // shift regiser.  So we can use that to detect how many TAPs are in the 
    // chain.
    
    // first get into shift IR mode
    jtag_capture_ir();
    jtag_shift_register();

    // then we flood the IR chain with all 1's to put all device's DRs
    // into bypass mode
    CLRTMS;
    SETMOSI;
    for (i = 0; i < 1024; i++) {
        if (i == 1023)
            SETTMS; // exit on last bit
        jtag_tcktock();
    }
    jtag_state = EXIT1_IR;

    // go to Update-IR
    CLRMOSI;
    jtag_tcktock();
    jtag_state = UPDATE_IR;

    // go to Shift-DR state
    jtag_capture_dr();
    jtag_shift_register();

    // flush the DR's with zeros
    CLRTMS;
    CLRMOSI;
    for (i = 0; i < 1024; i++) {
        jtag_tcktock();
    }

    // send 1's into the DRs until we see one come out the other side
    SETMOSI;
    for (i = 0; i < 1024; i++) {
        jtag_tcktock();
        bit = READMISO;
        if (bit)
            break;
        length++;
    }

    // now get back to run-test-idle
    jtag_run_test_idle();

    return length;
}





//! Gets device ID for specified chip in the chain
ot_u32 jtag_get_device_id(ot_int chip) {
    ot_int i;
    ot_u16 chain_length;
    ot_u32 id = 0;

    // reset everything
    jtag_reset_tap();

    // figure out how many devices are in the chain
    chain_length = jtag_detect_chain_length();

    if (chip >= chain_length) {
        debugstr("invalid part index");
        return 0;
    }

    // reset everything again because going through test-logic-reset forces
    // all IR's to have their manufacturer specific instruction for IDCODE
    // and loads the DR's with the chip's ID code.
    jtag_reset_tap();

    // get into shift DR state
    jtag_capture_dr();
    jtag_shift_register();

    // read out the 32-bit ID codes for each device
    CLRTMS;
    CLRMOSI;
    for (i = 0; i < (chip+1); i++) {
        id = jtag_trans_n(0xFFFFFFFF, 32, LSB | NOEND | NORETIDLE);
    }

    jtag_run_test_idle();

    return id;
}





//! Shift 8 bits in/out of selected register
ot_u8 jtag_trans_8(ot_u8 in) {
    ot_u32 out = jtag_trans_n((ot_u32)in, 8, MSB);
    
    return (ot_u8)(0xFF & out);
}



//! Shift 16 bits in/out of selected register
ot_u16 jtag_trans_16(ot_u16 in)] {
    ot_u32 out = jtag_trans_n((ot_u32)in, 16, MSB);
    
    return (ot_u16)(0xFFFF & out);
}




//! Shift 8 bits of the IR.
uint8_t jtag_ir_shift_8(uint8_t in) {
  /* Huseby's code, which breaks MSP430 support.
     The code is broken because either the invalid jtag state error
     causes the client to give up, or because it adds an extra clock edge.
     
    if (!in_run_test_idle()) {
        debugstr("Not in run-test-idle state");
        return 0;
    }

    // get intot the right state
    jtag_capture_ir();
    jtag_shift_register();
  */
  
    // idle
    SETTMS;
    jtag_tcktock();
    
    // select DR
    jtag_tcktock();
    
    // select IR
    CLRTMS;
    jtag_tcktock();
    
    // capture IR
    jtag_tcktock();
    
    //jtag_state = CAPTURE_IR;
    jtag_state = SHIFT_IR;
    
    // shift IR bits
    return jtag_trans_8(in);
}




//! Shift 16 bits of the DR.
ot_u16 jtag_dr_shift_16(ot_u16 in) {

    if (!in_run_test_idle()) {
        debugstr("Not in run-test-idle state");
        return 0;
    }

    // get intot the right state
    jtag_capture_dr();
    jtag_shift_register();

    // shift DR, then idle
    return jtag_trans_16(in);
}











// jtag430x2

#define JTAG430X2 0x11

//! Shift 20 bits of the DR.
uint32_t jtag430_dr_shift_20(uint32_t in);





//! Shift 20 bits of the DR.
ot_ulong jtag430_dr_shift_20(ot_ulong in) {
    if (!in_run_test_idle()) {
        debugstr("Not in run-test-idle state");
        return 0;
    }

    // get intot the right state
    jtag_capture_dr();
    jtag_shift_register();

    // shift DR, then idle
    return jtag_trans_n(in, 20, MSB);
}



//! Grab the core ID.
ot_ulong jtag430_coreid () {
    jtag_ir_shift_8(IR_COREIP_ID);
    return jtag_dr_shift_16(0);
}



//! Grab the device ID.
ot_ulong jtag430_deviceid () {
    jtag_ir_shift_8(IR_DEVICE_ID);
    return jtag430_dr_shift_20(0);
}



//! Write data to address
void jtag430x2_writemem(ot_ulong adr, ot_uint data) {
    jtag_ir_shift_8(IR_CNTRL_SIG_CAPTURE);
    if (jtag_dr_shift_16(0) & 0x0301) {
        CLRTCLK;
        jtag_ir_shift_8(IR_CNTRL_SIG_16BIT);
        
        if (adr>=0x100) jtag_dr_shift_16(0x0500);//word mode
        else            jtag_dr_shift_16(0x0510);//byte mode
        
        jtag_ir_shift_8(IR_ADDR_16BIT);
        jtag430_dr_shift_20(adr);
    
        SETTCLK;
    
        jtag_ir_shift_8(IR_DATA_TO_ADDR);
        jtag_dr_shift_16(data);//16 word

        CLRTCLK;
        jtag_ir_shift_8(IR_CNTRL_SIG_16BIT);
        jtag_dr_shift_16(0x0501);
        SETTCLK;

        CLRTCLK;
        SETTCLK;
        //init state
    }
    else {
        while(1) led_toggle(); //loop if locked up
    }
}




//! Read data from address
ot_uint jtag430x2_readmem(ot_ulong adr){
    ot_int toret = 0;
    //unsigned int tries=5;
  
    while (1) {
        do {
            jtag_ir_shift_8(IR_CNTRL_SIG_CAPTURE);
        } 
        while (!(jtag_dr_shift_16(0) & 0x0301));
    
        if (jtag_dr_shift_16(0) & 0x0301) {
            // Read Memory
            CLRTCLK;
            jtag_ir_shift_8(IR_CNTRL_SIG_16BIT);
      
            jtag_dr_shift_16(0x0501);//word read
      
            jtag_ir_shift_8(IR_ADDR_16BIT);
            jtag430_dr_shift_20(adr); //20
      
            jtag_ir_shift_8(IR_DATA_TO_ADDR);
            SETTCLK;
            CLRTCLK;
            toret = jtag_dr_shift_16(0x0000);
      
            SETTCLK;
      
            //Cycle a bit.
            CLRTCLK;
            SETTCLK;
            return toret;
        }
    
        return 0xdead;
    }
    //return toret;
}



//! Syncs a POR.
ot_uint jtag430x2_syncpor() {
    jtag_ir_shift_8(IR_CNTRL_SIG_16BIT);
    
    //JTAG mode
    jtag_dr_shift_16(0x1501); 
    
    while(!(jtag_dr_shift_16(0) & 0x200));  //0x100 or 0x200?
    
    return jtag430x2_por();
}



//! Executes an MSP430X2 POR
ot_uint jtag430x2_por(){
    ot_int i = 0;
  
    // tick
    CLRTCLK;
    SETTCLK;

    jtag_ir_shift_8(IR_CNTRL_SIG_16BIT);
    jtag_dr_shift_16(0x0C01);
    jtag_dr_shift_16(0x0401);
  
    //cycle
    for (i=10; i>0; i--) {
        CLRTCLK;
        SETTCLK;
    }
  
    jtag_dr_shift_16(0x0501);
  
    // tick
    CLRTCLK;
    SETTCLK;
  
    // Disable WDT
    jtag430x2_writemem(0x015C, 0x5A80);
  
    // check state
    jtag_ir_shift_8(IR_CNTRL_SIG_CAPTURE);
    if(jtag_dr_shift_16(0) & 0x0301)
        return(1);//ok
  
    return 0;//error
}



ot_uint jtag430x2_fusecheck() {
    ot_int i;
    for (i=3; i>0; i--) {
        jtag_ir_shift_8(IR_CNTRL_SIG_CAPTURE);
        
        // Check Blown Fuse, return 1 if blown
        if (jtag_dr_shift_16(0xAAAA) == 0x5555)
            return 1;
  }
  
  // Fuse is not blown
  return 0;
}











#define GF_GETLONG_I(OFFSET)    ((OFFSET+4)>>2)
#define GF_GETSHORT_I(OFFSET)   ((OFFSET+4)>>1)
#define GF_GETLONG(I)           gf.buf.ulong[GF_GETLONG_I(I)]
#define GF_GETSHORT(I)          gf.buf.ushort[GF_GETSHORT_I(I)]

typedef struct {
    ot_u8 id;
    ot_u8 index;
} kvp;

static const kvp verb_search[22] = {
    0x02,  2,   //PEEK                  --> gf_jtag430_readmem
    0x03,  3,   //POKE                  --> gf_jtag430_writemem
    0x10,  4,   //SETUP                 --> gf_setup
    0x20,  5,   //START                 --> gf_start
    0x21,  6,   //STOP                  --> gf_stop
    0x80,  7,   //IR_SHIFT              --> gf_jtag_ir_shift
    0x81,  8,   //DR_SHIFT              --> gf_jtag_dr_shift
    0x82,  9,   //RESET_TAP             --> gf_jtag_reset_tap
    0x83, 10,   //RESET_TARGET          --> gf_jtag_reset_target
    0x84, 11,   //DETECT_IR_WIDTH       --> gf_jtag_detect_ir_width
    0x85, 12,   //DETECT_CHAIN_LENGTH   --> gf_jtag_detect_chain_length
    0x86, 13,   //GET_DEVICE_ID         --> gf_jtag_get_device_id
    0xA0,  1,   //JTAG430_HALTCPU       --> gf_nimpl   (not implemented yet)
    0xA1,  1,   //JTAG430_RELEASECPU    --> gf_nimpl   (not implemented yet)
    0xC1,  1,   //JTAG430_SETINSTRFETCH --> gf_nimpl   (not implemented yet)
    0xC2,  1,   //JTAG430_SETPC         --> gf_nimpl   (not implemented yet)
    0xE0,  3,   //JTAG430_WRITEMEM      --> gf_jtag430_writemem
    0xE1,  3,   //JTAG430_WRITEFLASH    --> gf_jtag430_writemem
    0xE2,  2,   //JTAG430_READMEM       --> gf_jtag430_readmem
    0xE3,  1,   //JTAG430_ERASEFLASH    --> gf_nimpl   (not implemented yet)
    0xF0, 14,   //JTAG430_COREIP_ID     --> gf_jtag430_coreip_id
    0xF1, 15    //JTAG430_DEVICE_ID     --> gf_jtag430_device_id
};


static const ot_sub verb_parser[16] = {
    &gf_nok,                // 0
    &gf_nimpl,              // 1
    &gf_jtag430_readmem,    // 2
    &gf_jtag430_writemem,   // 3
    &gf_jtag_setup,         // 4
    &gf_jtag_start,         // 5
    &gf_jtag_stop,          // 6
    &gf_jtag_ir_shift,      // 7
    &gf_jtag_dr_shift,      // 8
    &gf_jtag_reset_tap,     // 9
    &gf_jtag_reset_target,  // 10
    &gf_jtag_detect_ir_width,       // 11
    &gf_jtag_detect_chain_length,   // 12
    &gf_jtag_get_device_id,         // 13
    &gf_jtag430_coreip_id,          // 14    
    &gf_jtag430_device_id           // 15
}


ot_sub gf_verb_bsearch(ot_u8 verb) {
/// Binary search for the verb: this might not actually be much (or any) faster 
/// than a linear search, because there are only 23 implemented commands.    
    ot_int mid  = 11;
    ot_int left = 0;
    ot_int rite = 22;
    
    while (left != rite) {
        ot_u8 id = verb_search[mid].id
    
        if (verb == id) {
            return verb_parser[verb_search[mid].index];
        }
        else if (verb < id) {
            rite    = mid;
            mid     = left + (mid << 1);
        }
        else {
            left    = mid+1;
            mid     = rite - (mid<<1);
        }
    }
    
    return &gf_nok;
}







/// Right now this is the only "app" of GoodFET that is implemented.  If we 
/// implement more, we can switch the task handle as needed.
void goodfet(ot_task* task) {

}


void gf_setlength(ot_u16 length) {
    gf.buf.ushort[1] = length;
}


void goodfet_proc() {
    ot_sub verb_fn;
  
    /// 1.  Select "app" from the first byte of the message
    ///     The only GoodFET "app" implemented at present is the JTAG430X2 app.
    if ((gf.buf.ubyte[0] != GFAPP_JTAG430X2) && (jtag430mode != MSP430X2MODE)) {
        // could send a message here, or just return something and have the 
        // task do it.
        return;
    }
    
    // Set response length to zero (default)
    gf_setlength(0);
    
    verb_fn = gf_verb_bsearch(gf.buf.ubyte[1]);
    verb_fn();
    
    //shit-out the response loaded into gf.buf by verb_fn()
    gfdrv_tx();
    
    goodfet_proc_EXIT:
    jtag430_resettap();
}





void gf_nok(void) {
    gf.buf.ubyte[1] = NOK;  //change "verb" to NOK
}

void gf_nimpl(void) {
    gf.buf.ubyte[1] = NOK;  //change "verb" to NOK
    debugstr("This function is not yet implemented for MSP430X2.");
    debughex(verb);
}

void gf_setup(void) {           //10
    jtag_setup();
}

void gf_start(void) {
    gf_setlength(1);
    //do 
    gf.buf.ubyte[4] = jtag430x2_start();
    //while(GF_GETBYTE(0)==00 || GF_GETBYTE(0)==0xFF);
    
    if (jtagid != MSP430X2JTAGID) {
        gf.buf.ubyte[1] = NOK;
        debugstr("JTAG version unknown.");
    }
    else {
        jtag430mode = MSP430X2MODE;
        drwidth     = 20;
    
        jtag430x2_fusecheck();
        jtag430x2_syncpor();
        jtag430_resettap();
    }
}
        
void gf_stop(void) {
    jtag_stop();
}
        
void gf_jtag_ir_shift(void) {
    gf_setlength(1);
    GF_GETBYTE(0) = jtag_ir_shift_8(GF_GETBYTE(0));
}

void gf_jtag_dr_shift(void) {
    gf_setlength(2);
    GF_GETSHORT(0) = htons(jtag_dr_shift_16(ntohs(GF_GETSHORT(0))));
}

void gf_jtag_reset_tap(void) {
    jtag_reset_tap();
}

void gf_jtag_reset_target(void) {
    jtag_reset_tap();
    jtag_reset_target();
}

void gf_jtag_detect_ir_width(void) {
    gf_setlength(2);
    jtag_reset_tap();
    GF_GETSHORT(0) = htons(jtag_detect_ir_width());
}

void gf_jtag_detect_chain_length(void) {
    gf_setlength(2);
    jtag_reset_tap();
    GF_GETSHORT(0) = htons(jtag_detect_chain_length());
}

void gf_jtag_get_device_id(void) {
    gf_setlength(4);
    jtag_reset_tap();
    GF_GETLONG(0) = htonl(jtag_get_device_id(ntohs(GF_GETSHORT(0))));
}
                
void gf_jtag430x2_readmem(void) {
    ot_ulong    at;
    ot_int      resp_len;
    ot_int      i;
    
    at          = GF_GETLONG(0);
    resp_len    = 2;                    //basic assumption: small peek, 2 bytes

    //Fetch large blocks for bulk fetches, (always even bytes)
    // correct potentially malformed length values (always even)
    if (gf.buf.sshort[1] > 5) {
        resp_len    = GF_GETSHORT(4);
        resp_len    = (resp_len + 1) & ~1;
    }
    gf_setlength(resp_len);
    
    resp_len += 6;
    for(i=6; i<resp_len; i+=2, at+=2) {
        //jtag430_resettap();
        //delay(10);
        GF_GETSHORT(i) = jtag430x2_readmem(at);
    }
}

void gf_jtag430x2_writemem(void) {
    ot_u32  addr;
    ot_u16  data;
    gf_setlength(2);
    
    // Write Data to address
    addr    = GF_GETLONG(0);
    data    = GF_GETSHORT(4);
    jtag430x2_writemem(addr, data);
    
    //Verify Data Written at Address
    GF_GETSHORT(0) = jtag430x2_readmem(addr);
}

void gf_jtag430x2_coreip_id(void) {
    gf_setlength(2);
    GF_GETSHORT(0) = jtag430_coreid();
}
    
void gf_jtag430x2_device_id(void) {
    gf_setlength(4);
    GF_GETSHORT(0) = jtag430_deviceid();
}













//Junk

void serputc(char c, fifo_t *fp);
void serputs(char *cpt, fifo_t *fp);
void serputb(char c, fifo_t *fp);
void serputw(int w, fifo_t *fp);
int sergetc(fifo_t *fp);
int seravailable(fifo_t *fp);
void serflush(fifo_t *fp);
void ser0_init(int baud, fifo_t *rd, fifo_t *wr);
void ser1_init(int baud, fifo_t *rd, fifo_t *wr);
void serclear(fifo_t *fp);

//int seravailable(fifo_t * fp) {
//  return fp->count;
//}

ot_int gf_available() {
    return q_length(&gfet.inq);
}



void serflush(fifo_t* fp) {
    while (seravailable(fp) > 0) {
        delay_ms(1);
    }
}

//int sergetc(fifo_t* fp) {
//  int c;
//  if (fp == NULL)
//      return -1;
//  if (fp->count) {
//      c = fifo_rd(fp);
//  } else {
//      fp->empty = TRUE;
//      c = -1;
//  }
//  return c;
//}

ot_int gf_getc() {
    if (gfet.inq.getcursor == gfet.inq.back) {
        return -1
    }
    return q_readbyte(&gfet.inq);
}

//void serputc(char c, fifo_t * fp) {
//  if (fp == NULL)
//      return;
//  while (seravailable(fp) == FIFO_SZ) {
//  }
//  fifo_wr(fp, c);     // magic is in count-- indivisible, do not optimize
//  if (fp->empty && fp->count) {   // buffer had been empty
//      fp->empty = FALSE;
//      c = fifo_rd(fp);
//      if (fp == txfp0) {
//          TXBUF0 = c;
//      } else {
//          TXBUF1 = c;
//      }
//  }
//}



void gf_putc(ot_u8 c) {
    if (gfet.outq.putcursor != gfet.outq.back) {
        q_writebyte(&gfet.outq, c);
    }
}



//void serputs(char *cpt, fifo_t * fp) {
//  while (*cpt) {
//      serputc(*cpt++, fp);
//  }
//}

void gf_puts(ot_u8* cpt) {
    while (*cpt) {
        gf_putc(*cpt++);
    }
}



void gf_puthex(ot_u8* dst, ot_u8* src, ot_int len) {
    if (gfet.outq.putcursor < (gfet.outq.back-len)) {
        len = otutils_bin2hex(dst, src, len);
        q_writestring(&gfet.outq, dst, len);
    }
}



//void serputb(char c, fifo_t * fp) {
//  serputc(hex2c(c>>4), fp);
//  serputc(hex2c(c), fp);
//}
void gf_putb(ot_u8 c) {
    ot_u8 hexc[2];
    gf_puthex(hexc, &c, 1);
}



//void serputw(int w, fifo_t * fp) {
//  serputb(w >> 8, fp);
//  serputb(w & 0xff, fp);
//}
void gf_putw(ot_u16 w) {
    ot_u8 hexc[4];
    gf_puthex(hexc, (ot_u8*)&w, 2);
}



//void serclear(fifo_t *fp) {
//  while (seravailable(fp) > 0) {
//      sergetc(fp);
//  }
//}
void gf_clear(ot_queue* q) {
    q_empty(q);
}



//
ot_u8 serial0_rx() {
    ot_u8 c;
    while (gf_avail(rxfp0) == 0) {  // wait for data to be available
        // FIXME we should sleep
    }
    c = sergetc(rxfp0);
    dddputs(dddlog_input(c));
    return c;
}


void gfevt_rxdone() {
/// USB has loaded-in 64 bytes.
}


void serial0_tx(uint8_t x)
{
    serputc(x, txfp0);
}



#endif

