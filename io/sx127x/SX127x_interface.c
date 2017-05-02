/* Copyright 2016 JP Norair
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
  * @file       /io/sx127x/SX127x_interface.c
  * @author     JP Norair
  * @version    R101
  * @date       28 Oct 2016
  * @brief      Generic, High-level interface routines for SX127x transceiver
  * @ingroup    SX127x
  * 
  * The functions and data implemented in this file provide a mostly-generic
  * interface to SX127x control.  It is designed with Mode 2 wireless spec in
  * mind, but at this level Mode 2 is not appreciably different than most other
  * specs that could run on the SX127x.
  *
  * You will need to implement a low-level driver module that implements a
  * handful of functions which require knowledge of the platform itself.  See
  * SX127x_interface.h for more information on which functions these are.
  *
  ******************************************************************************
  */


#include <otstd.h>
#include <board.h>
#if defined(__SX127x__)


#include <io/sx127x/interface.h>
#include <io/sx127x/config.h>

//#include <platform/config.h>


#include <otlib/memcpy.h>
#include <otlib/delay.h>



///@todo right now these are used with m2DLL.... move those functions elsewhere
#include <m2/dll.h>


// These only for driver testing purposes (sx127x_coredump())
#include <otlib/utils.h>
#include <otlib/logger.h>



#if defined(_SX127X_PROFILE)
    ot_s16 set_line;
    ot_s16 line_hits[256];
    ot_u16 count_hits = 0;
#endif



/** Module Data for radio driver interface <BR>
  * ========================================================================
  */
sx127x_struct sx127x;




/** High-Level Pin Interrupt Management Functions <BR>
  * ========================================================================<BR>
  * These functions manage the usage of the SPIRIT1 GPIO bus, which is used to
  * signal status interrupts to the platform.  The Low-Level, platform-specific
  * driver must implement the following low-level functions which are called by
  * these high-level functions:
  * <LI> sx127x_int_config() </LI>
  * <LI> sx127x_int_txdone() </LI>
  * <LI> sx127x_int_clearall() </LI>
  * <LI> sx127x_int_force() </LI>
  * <LI> sx127x_int_turnon() </LI>
  * <LI> sx127x_int_turnoff() </LI>
  */
OT_WEAK void sx127x_int_off() {
    sx127x_int_config(0);
}

OT_WEAK void sx127x_int_on() {
    ot_u32 ie_sel;
    switch (sx127x.imode) {
        case MODE_Listen:   ie_sel = RFI_LISTEN;
        case MODE_RXData:   ie_sel = RFI_RXDONE;
        case MODE_CSMA:     ie_sel = RFI_CSMA;
        case MODE_TXData:   ie_sel = RFI_TXDONE;
        default:            ie_sel = 0;
    }
    sx127x_int_config(ie_sel);
}


inline void sx127x_iocfg_cad()  {
    sx127x_int_clearall();
    sx127x_write(RFREG_LR_DIOMAPPING1, _DIOMAPPING1_CAD);
}

inline void sx127x_iocfg_rx()  {
    sx127x_int_clearall();
    sx127x_write(RFREG_LR_DIOMAPPING1, _DIOMAPPING1_RX);
}

inline void sx127x_iocfg_tx()  {
    sx127x_int_clearall();
    sx127x_write(RFREG_LR_DIOMAPPING1, _DIOMAPPING1_TX);
}

inline void sx127x_int_listen() {
    sx127x.imode = MODE_Listen;
    sx127x_int_config(RFI_LISTEN);
}

inline void sx127x_int_rxdata() {
    sx127x.imode = MODE_RXData;
    sx127x_int_config(RFI_RXDATA);
}

inline void sx127x_int_csma() {
    sx127x.imode = MODE_CSMA;
    sx127x_int_config(RFI_CSMA);
}

inline void sx127x_int_txdata() {
    sx127x.imode = MODE_TXData;
    sx127x_int_config(RFI_TXDONE);
}


void sx127x_irq0_isr() {   sx127x_virtual_isr(sx127x.imode);     }
void sx127x_irq1_isr() {   sx127x_virtual_isr(sx127x.imode + 1); }
void sx127x_irq2_isr() {   sx127x_virtual_isr(sx127x.imode + 2); }
void sx127x_irq3_isr() {   sx127x_virtual_isr(sx127x.imode + 3); }
void sx127x_irq4_isr() {   sx127x_virtual_isr(sx127x.imode + 4); }
void sx127x_irq5_isr() {   sx127x_virtual_isr(sx127x.imode + 5); }











/** Basic Control <BR>
  * ============================================================================
  */
  
void sx127x_load_defaults() {
/// The data ordering is: WRITE LENGTH, WRITE HEADER (0), START ADDR, VALUES
/// Ignore registers that are set later, are unused, or use the hardware default values.
#   define __REGSET(NAME)  RFREG_##NAME, DRF_##NAME
    static const ot_u8 defaults[] = {
        __REGSET(LR_OPMODE),
        __REGSET(LR_PACONFIG),
        __REGSET(LR_PARAMP),
        __REGSET(LR_OCP),
        __REGSET(LR_LNA),
        __REGSET(LR_FIFOADDRPTR),
        __REGSET(LR_FIFOTXBASEADDR),
        __REGSET(LR_FIFORXBASEADDR),
        __REGSET(LR_MODEMCONFIG1),
        __REGSET(LR_MODEMCONFIG2),
        __REGSET(LR_SYMBTIMEOUTLSB),
        __REGSET(LR_PREAMBLEMSB),
        __REGSET(LR_PREAMBLELSB),
        __REGSET(LR_PAYLOADLENGTH),
        __REGSET(LR_PAYLOADMAXLENGTH),
        __REGSET(LR_HOPPERIOD),
#       if defined(__SX1276__) || defined(__SX1277__) || defined(__SX1278__) || defined(__SX1279__)
        __REGSET(LR_MODEMCONFIG3),
#       endif
        __REGSET(LR_DETECTOPTIMIZE),
        __REGSET(LR_DETECTOPTIMIZE),
        __REGSET(LR_INVERTIQ),
        __REGSET(LR_DETECTIONTHRESHOLD),
        __REGSET(LR_SYNCWORD),
        __REGSET(LR_INVERTIQ2),
        __REGSET(LR_DIOMAPPING1),
        __REGSET(LR_DIOMAPPING2),
        0   //Terminating 0
    };

    ot_u8* cursor;
    
    // SX127x should be asleep when running this function
    //sx127x_strobe(_OPMODE_SLEEP);
    
    ///@todo do a burst write
    cursor = (ot_u8*)defaults;
    while (*cursor != 0) {
        sx127x_write(cursor[0], cursor[1]);
        cursor += 2;
    }
    
#   undef __REGSET
}

void sx127x_coredump() {
///debugging function to dump-out register values of RF core (not all are used)
    ot_u8 i = 0x00;
    ot_u8 regval;
    ot_u8 label[]   = { 'R', 'E', 'G', '_', 0, 0 };

    do {
        regval = sx127x_read(i);
        otutils_bin2hex(&label[4], &i, 1);
        logger_msg(MSG_raw, 6, 1, label, &regval);
        //mpipedrv_wait();
        delay_ti(5);
    }
    while (++i < 0x71);
}



/** Control & Status Functions <BR>
  * ========================================================================<BR>
  * These functions utilize the pin-wrapper driver functions, which
  * must be implemented in the platform-specific driver. 
  */

ot_bool sx127x_isready() {
///@todo The only way to monitor state transition signaling is via the ModeReady
///      signal on DIO5, but many schematics don't route DIO5!  So for now just
///      assume true.
    return True;
}

void sx127x_waitfor_ready() {
/// SLEEP->STANDBY should take about 75us (500 watchdogs).
/// @todo There is no way to check ready without DIO5, so this function isn't 
///       used inside the normal driver.
/// @todo Write failure code in OT that logs hardware fault and resets OT
    ot_uint watchdog = 500;
    while ((sx127x_readypin_ishigh() == 0) && (--watchdog));
    if (watchdog == 0){
        //ready_fails++;
        sx127x_reset();
        delay_us(300);
        dll_init();
    }   
}

/*
void sub_waitfor_opmode(ot_u8 target, ot_u8 mode, ot_uint fromsleep, ot_uint fromactive) {
/// @todo Write failure code in OT that logs hardware fault and resets OT
    ot_uint wdog;
    wdog = (mode == 0) ? fromsleep : fromactive;
    mode = sx127x_mode();
     
    while (mode != target) {
        if (--wdog == 0) {
            sx127x_reset();
            delay_us(300);
            dll_init();
            return;
        }
        mode = sx127x_mode();
    }
}

void sx127x_waitfor_fsrx()    { sub_waitfor_opmode(_OPMODE_FSRX, 250, 125); }
void sx127x_waitfor_fstx()    { sub_waitfor_opmode(_OPMODE_FSTX, 250, 125); }
void sx127x_waitfor_cad()     { sub_waitfor_opmode(_OPMODE_CAD, 250, 125); }

void sx127x_waitfor_standby() {
/// Assume 500us (125 watchdogs) worst case sleep->standby
/// Assume 25us (7 watchdogs) worst case nonsleep->standby
    sub_waitfor_opmode(_OPMODE_STANDBY, 125, 7);
    
//    ot_u8   mode;
//    ot_uint wdog;
//    mode = sx127x_mode();
//    wdog = (mode == 0) ? 125 : 7;
//    
//    while (mode != 1) {
//        if (--wdog == 0) {
//            sx127x_reset();
//            delay_us(300);
//            dll_init();
//            return;
//        }
//        mode = sx127x_mode();
//    }
}

void sx127x_waitfor_sleep() {
/// Assume 25us (7 watchdogs) worst case nonsleep->sleep
    sub_waitfor_opmode(_OPMODE_SLEEP, 0, 7);
    
//    ot_uint wdog;
//    wdog = 7;
//    while (sx127x_mode() != 0) {
//        if (--wdog == 0) {
//            sx127x_reset();
//            delay_us(300);
//            dll_init();
//            return;
//        }
//    }
}
*/
ot_bool sx127x_check_cadpin() {
    return (ot_bool)(sx127x_cadpin_ishigh() != 0);
}

ot_u8 sx127x_getstatus() {
/// Status is register IRQFLAGS
    sx127x.status = sx127x_read(RFREG_LR_IRQFLAGS);
    return sx127x.status;
}

ot_u8 sx127x_mode() {
    return sx127x_read(RFREG_LR_OPMODE) & _OPMODE;
}

ot_u8 sx127x_rxbytes()    { return sx127x_read(RFREG_LR_RXNBBYTES); }
ot_u8 sx127x_rssi()       { return sx127x_read(RFREG_LR_RSSIVALUE); }
ot_u8 sx127x_pktrssi()    { return sx127x_read(RFREG_LR_PKTRSSIVALUE) >> 2; }
ot_s8 sx127x_pktsnr()     { return sx127x_read(RFREG_LR_PKTSNRVALUE); }





/** High-Level Read, Write, and Load-Defaults Functions <BR>
  * ========================================================================<BR>
  * These utilize the driver function: sx127x_spibus_io()
  * This function must be implemented specific to the platform.
  */

void sx127x_strobe(ot_u8 new_mode, ot_bool blocking) {
/// "strobe" must be one of the _OPMODE values from 0-7
/// Assume 500us (125 watchdogs) worst case sleep->standby
/// Assume 600us (150 watchdogs) worst case sleep->FS-ON
/// Assume 640us (160 watchdogs) worst case sleep->RX/TX/CAD
/// Assume 100us (25 watchdogs)  worst case standby->FS-ON
/// Assume 140us (35 watchdogs)  worst case standby->RX/TX/CAD
/// Assume 40us  (10 watchdogs)  worst case active->sleep
/// 000  SLEEP
/// 001  STDBY
/// 010  Frequency synthesis TX (FSTX) 
/// 011  Transmit (TX)
/// 100  Frequency synthesis RX (FSRX) 
/// 101  Receive continuous (RXCONTINUOUS) 
/// 110  receive single (RXSINGLE) 
/// 111  Channel activity detection (CAD)
    static const ot_u8 wdog_amount[16] = {  
         1,10,  125,1,  150,25,  160,35,  150,25,  160,35,  160,35,  160,35
    };
    
    if (!blocking) {
        sx127x_write(RFREG_LR_OPMODE, _LORAMODE|new_mode);
    }
    else {
        ot_u8 old_mode = sx127x_mode();
        
        if (old_mode != new_mode) {
            ot_uint wdog;
            
            sx127x_write(RFREG_LR_OPMODE, _LORAMODE|new_mode);
            wdog = wdog_amount[(new_mode<<1) + (old_mode!=0)];
            
            do {
                if (--wdog == 0) {
                    sx127x_reset();
                    delay_us(300);
                    dll_init();
                    return;
                }
                old_mode = sx127x_mode();
                
            } while (old_mode != new_mode);
        }
    }
}

ot_u8 sx127x_read(ot_u8 addr) {
    sx127x_spibus_io(1, 1, &addr);
    return sx127x.busrx[0];
}

void sx127x_burstread(ot_u8 start_addr, ot_u8 length, ot_u8* data) {
    sx127x_spibus_io(1, length, &start_addr);
    memcpy(data, sx127x.busrx, length);
}

void sx127x_write(ot_u8 addr, ot_u8 data) {
    ot_u8 cmd[2];
    cmd[0]  = 0x80 | addr;
    cmd[1]  = data;
    sx127x_spibus_io(2, 0, cmd);
}

void sx127x_burstwrite(ot_u8 start_addr, ot_u8 length, ot_u8* cmd_data) {
    ot_u8 save;
    cmd_data--;
    save        = *cmd_data;
    *cmd_data   = 0x80 | start_addr;
    sx127x_spibus_io(1+length, 0, cmd_data);
    *cmd_data   = save;
}





/** Counter Management Functions <BR>
  * ========================================================================<BR>
  * Certain MAC processes require a running timer.  Instead of using any
  * internal timers of the SX127x, we instead use the more reliable interval
  * timer feature of OpenTag.
  */
static ot_u32 macstamp;

void sx127x_start_counter() {
    macstamp = systim_chronstamp(NULL);
}

void sx127x_stop_counter() {
}

ot_u16 sx127x_get_counter() {
    ot_u16 value;
    value = dll.counter - (ot_u16)systim_chronstamp(&macstamp);
    return value;
}






/** Advanced Configuration <BR>
  * ========================================================================<BR>
  */

ot_int sx127x_calc_rssi(ot_u8 encoded_value, ot_s8 packet_snr) {
#   if defined(__SX1272__) || defined(__SX1273__)
    ot_int rssi;
    rssi = -139 + (ot_int)encoded_value;
    if (packet_snr < 0) {
        rssi += packet_snr/4;
    }
    return rssi;
    
#   elif defined(__SX1276__) || defined(__SX1277__) || defined(__SX1278__) || defined(__SX1279__)
#   if (RF_PARAM_BAND < 750)
    return -164 + (ot_int)encoded_value;
#   else
    return -157 + (ot_int)encoded_value;
#   endif
#   endif
}


ot_u8 sx127x_calc_rssithr(ot_u8 input) {
/// SX127x treats RSSI thresholding through the normal RSSI engine.  The specs
/// are the same as those used in sx127x_calc_rssi() above, but the process is
/// using different input and output.
///
/// Input is a whole-dBm value encoded linearly as: {0=-140dBm, 127=-13dBm}.
/// Output is the value that should go into RSSI_TH field.
#if defined(__SX1272__) || defined(__SX1273__)
    // SX1272/3 uses -139 as baseline, DASH7 -140
    // Clip baseline at 0
    ot_int rssi_thr;
    rssi_thr = (ot_int)input - 1;
    if (rssi_thr < 0)
        rssi_thr = 0;

    // Multiply by 2 to yield half-dBm.
    rssi_thr  <<= 1;
    return rssi_thr;

#elif defined(__SX1276__) || defined(__SX1277__) || defined(__SX1278__) || defined(__SX1279__)
#   if (RF_PARAM_BAND < 750)
    return (164 - 140) + input;
#   else
    return (157 - 140) + input;
#   endif

#endif
}


ot_u8 sx127x_clip_txeirp(ot_u8 input_eirp) {
/// This considers Normal-Mode.  In TX Boost mode, 13dBm --> 20dBm
#if defined(__SX1272__) || defined(__SX1273__)
#   define _MAX_DBM_EIRP (((13*2) - RF_HDB_ATTEN) + 80)
#elif defined(__SX1276__) || defined(__SX1277__) || defined(__SX1278__) || defined(__SX1279__)
#   define _MAX_DBM_EIRP (((14*2) - RF_HDB_ATTEN) + 80)
#endif
    if (input_eirp > _MAX_DBM_EIRP) {
        input_eirp = _MAX_DBM_EIRP;
    }
    return input_eirp;
    
#   undef _MAX_DBM_EIRP
}


void sx127x_set_txpwr(ot_u8 pwr_code) {
/// Sets the tx output power (non boost)
/// "pwr_code" is a value, 0-127, that is: eirp_code/2 - 40 = TX dBm
/// i.e. eirp_code=0 => -40 dBm, eirp_code=80 => 0 dBm, etc
    ot_int dBm;
    
    // get dBm
    dBm = (-80 + (ot_int)pwr_code) / 2;

#if defined(__SX1272__) || defined(__SX1273__)
    // Convert to SX1272/3 units (PA Boost off)
    dBm = dBm + 1;
    pwr_code = (ot_u8)dBm & 0x0F;

#elif defined(__SX1276__) || defined(__SX1277__) || defined(__SX1278__) || defined(__SX1279__)
    // Convert to SX1276/7/8/9 units (PA Boost off)
    dBm = dBm + 1;
    pwr_code = (5<<4) | ((ot_u8)dBm & 0x0F);

#endif

    // Write new PA Table to device
    sx127x_write(RFREG_LR_PACONFIG, pwr_code);
}


// LoRaWAN Shit
#if (0)

/** Set power for LoRaWAN */
#ifndef _PABOOST_OFFSET
#   define _PABOOST_OFFSET  0
#else
#   undef _PABOOST_OFFSET
#   define _PABOOST_OFFSET  3
#endif

#define __SX127x_DBM_TO_TXPWR(DBM)  ((DBM+1-_PABOOST_OFFSET) & 0xF)

const ot_u8 lorawan_915_powercodes[11] = {
    __SX127x_DBM_TO_TXPWR(30),
    __SX127x_DBM_TO_TXPWR(28),
    __SX127x_DBM_TO_TXPWR(26),
    __SX127x_DBM_TO_TXPWR(24),
    __SX127x_DBM_TO_TXPWR(22),
    __SX127x_DBM_TO_TXPWR(20),
    __SX127x_DBM_TO_TXPWR(18),
    __SX127x_DBM_TO_TXPWR(16),
    __SX127x_DBM_TO_TXPWR(14),
    __SX127x_DBM_TO_TXPWR(12),
    __SX127x_DBM_TO_TXPWR(10)
};

const ot_u8 lorawan_868_powercodes[6] = {
    __SX127x_DBM_TO_TXPWR(20),
    __SX127x_DBM_TO_TXPWR(14),
    __SX127x_DBM_TO_TXPWR(11),
    __SX127x_DBM_TO_TXPWR(8),
    __SX127x_DBM_TO_TXPWR(5),
    __SX127x_DBM_TO_TXPWR(2)
};

const ot_u8 lorawan_433_powercodes[6] = {
    __SX127x_DBM_TO_TXPWR(10),
    __SX127x_DBM_TO_TXPWR(7),
    __SX127x_DBM_TO_TXPWR(4),
    __SX127x_DBM_TO_TXPWR(1),
    __SX127x_DBM_TO_TXPWR(-2),
    __SX127x_DBM_TO_TXPWR(-5)
};

#define lorawan_780_powercodes  lorwan_433_powercodes


/** Set Data Rate for LoRaWAN */

const ot_u8 lorawan_915_drcodes[16][2] = {
    {_BW_125_KHZ, _SF10},
    {_BW_125_KHZ, _SF9},
    {_BW_125_KHZ, _SF8},
    {_BW_125_KHZ, _SF7},
    {_BW_500_KHZ, _SF8},
    {_BW_500_KHZ, _SF7},    // RFU in spec: defaulted 
    {_BW_500_KHZ, _SF7},    // RFU in spec: defaulted
    {_BW_500_KHZ, _SF7},    // RFU in spec: defaulted
    {_BW_500_KHZ, _SF12},
    {_BW_500_KHZ, _SF11}, 
    {_BW_500_KHZ, _SF10}, 
    {_BW_500_KHZ, _SF9}, 
    {_BW_500_KHZ, _SF8}, 
    {_BW_500_KHZ, _SF7},
    {_BW_500_KHZ, _SF7},    // RFU in spec: defaulted
    {_BW_500_KHZ, _SF7}     // RFU in spec: defaulted
};

const ot_u8 lorawan_868_drcodes[8][2] = {
    {_BW_125_KHZ, _SF12},
    {_BW_125_KHZ, _SF11},
    {_BW_125_KHZ, _SF10},
    {_BW_125_KHZ, _SF9},
    {_BW_125_KHZ, _SF8},
    {_BW_125_KHZ, _SF7},  
    {_BW_250_KHZ, _SF7},    
    {0, 0}                  // FSK placeholder
};

#define lorawan_780_drcodes lorawan_868_drcodes
#define lorawan_433_drcodes lorawan_868_drcodes


/** LoRaWAN RX Window channel */
// Channel Hertz: (replace with reg vals)
// DR code: index in lorawan_xxx_drcodes

const ot_u32 lorawan_915_rxwnd2[2] = { 923300000, 8};
const ot_u32 lorawan_868_rxwnd2[2] = { 869525000, 0};
const ot_u32 lorawan_780_rxwnd2[2] = { 786000000, 0};
const ot_u32 lorawan_433_rxwnd2[2] = { 434665000, 0};


/** LoRaWAN Bands */

const ot_uint lorawan_915_bandspec[1][4] = {
    { 1, 5, 0, 0 }  // 5=20dBm, 1=100% duty
};  

const ot_uint lorawan_868_bandspec[5][4] = { 
    { 100,  1, 0, 0 },  // 1=14dBm, 100=1% duty
    { 100,  1, 0, 0 },  // 1=14dBm, 100=1% duty
    { 1000, 1, 0, 0 },  // 1=14dBm, 1000=0.1% duty
    { 10,   1, 0, 0 },  // 1=14dBm, 10=10% duty
    { 100,  1, 0, 0 }   // 1=14dBm, 100=1% duty
};

const ot_uint lorawan_780_bandspec[1][4] = {
    { 100, 0, 0, 0 }  // 0=10dBm, 100=1% duty
}; 

#define lorawan_433_bandspec lorawan_780_bandspec 


/** LoRaWAN Default Channels */

//Todo: rubric for 915 channels
// LORA_MAX_NB_CHANNELS must be 8 or more
// 125 kHz channels go from 0 to (LORA_MAX_NB_CHANNELS-8)
// 500 kHz channels go from (LORA_MAX_NB_CHANNELS-8) to LORA_MAX_NB_CHANNELS
// Just set a pointer to some line in the drcodes matrix

const ot_u8 lorawan_868_defchan[3][3] = {
    { 868100000, (DR_5<<4) | DR_0, 0 },
    { 868300000, (DR_5<<4) | DR_0, 0 },
    { 868500000, (DR_5<<4) | DR_0, 0 }
};

const ot_u8 lorawan_780_defchan[3][3] = {
    { 779500000, (DR_5<<4) | DR_0, 0 },
    { 779700000, (DR_5<<4) | DR_0, 0 },
    { 779900000, (DR_5<<4) | DR_0, 0 }
};

const ot_u8 lorawan_433_defchan[3][3] = {
    { 433175000, (DR_5<<4) | DR_0, 0 },
    { 433375000, (DR_5<<4) | DR_0, 0 },
    { 433575000, (DR_5<<4) | DR_0, 0 }
};




/** LoRaWAN Duty Cycle Back-off Procedure */




/** LoRaWAN mega value table */

#endif // End of LoRaWAN shit




#endif


