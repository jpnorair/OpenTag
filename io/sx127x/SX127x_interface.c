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
  * @file       /io/sx127x/generic_interface.c
  * @author     JP Norair
  * @version    R101
  * @date       28 Oct 2016
  * @brief      Generic, High-level interface routines for SX127x transceiver
  * @ingroup    SX127x
  *
  * The functions and data implemented in this file provide a mostly-generic
  * interface to SPIRIT1 control.  It is designed with Mode 2 wireless spec in
  * mind, but at this level Mode 2 is not appreciably different than most other
  * specs that could run on the SPIRIT1.
  *
  * You will need to implement a low-level driver module that implements a
  * handful of functions which require knowledge of the platform itself.  See
  * SPIRIT1_interface.h for more information on which functions these are.
  *
  ******************************************************************************
  */


#include <otstd.h>
#include <board.h>
#if defined(__SPIRIT1__)


#include <io/sx127x/interface.h>
#include <io/sx127x/config.h>

//#include <platform/config.h>


#include <otlib/memcpy.h>
#include <otlib/delay.h>



///@todo right now these are used with m2DLL.... move those functions elsewhere
#include <m2/dll.h>


// These only for driver testing purposes (spirit1_coredump())
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

OT_WEAK void spirit1_int_on() {
    ot_u32 ie_sel;
    switch (sx127x.imode) {
        case MODE_Listen:   ie_sel = RFI_CAD;
        case MODE_RXData:   ie_sel = RFI_RXDATA;
        case MODE_CSMA:     ie_sel = RFI_CAD;
        case MODE_TXData:   ie_sel = RFI_TXDATA;
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
    sx127x_int_config(RFI_TXFIFO);
}


void sx127x_irq0_isr() {   sx127x_virtual_isr(spirit1.imode);     }
void sx127x_irq1_isr() {   sx127x_virtual_isr(spirit1.imode + 1); }
void sx127x_irq2_isr() {   sx127x_virtual_isr(spirit1.imode + 2); }
void sx127x_irq3_isr() {   sx127x_virtual_isr(spirit1.imode + 3); }
void sx127x_irq4_isr() {   sx127x_virtual_isr(spirit1.imode + 4); }
void sx127x_irq5_isr() {   sx127x_virtual_isr(spirit1.imode + 5); }






/** High-Level Read, Write, and Load-Defaults Functions <BR>
  * ========================================================================<BR>
  * These utilize the driver function: spirit1_spibus_io()
  * This function must be implemented specific to the platform.
  */

void sx127x_strobe(ot_u8 strobe) {
    ot_u8 cmd[2];
    cmd[0]  = 0x80;
    cmd[1]  = strobe;
    sx127x_spibus_io(2, 0, cmd);
}

ot_u8 sx127x_read(ot_u8 addr) {
    ot_u8 cmd[2];
    cmd[0]  = addr;
    sx127x_spibus_io(2, 0, cmd);
    return sx127x.busrx[0];
}

void sx127x_burstread(ot_u8 start_addr, ot_u8 length, ot_u8* data) {
    ot_u8 cmd[2];
    cmd[0]  = start_addr;
    spirit1_spibus_io(1, length, (ot_u8*)cmd);
    memcpy(data, sx127x.busrx, length);
}

void sx127x_write(ot_u8 addr, ot_u8 data) {
    ot_u8 cmd[2];
    cmd[0]  = 0x80 | addr;
    cmd[1]  = data;
    spirit1_spibus_io(2, 0, cmd);
}

void sx127x_burstwrite(ot_u8 start_addr, ot_u8 length, ot_u8* cmd_data) {
    cmd_data[0] = 0x80 | start_addr;
    spirit1_spibus_io(1+length, 0, cmd_data);
}



void sx127x_load_defaults() {
/// The data ordering is: WRITE LENGTH, WRITE HEADER (0), START ADDR, VALUES
/// Ignore registers that are set later, are unused, or use the hardware default values.
#   define __REGSET(NAME)  RFREG_##NAME, DRF_##NAME
    static const ot_u8 defaults[] = {
        __REGSET(LR_OPMODE),
        __REGSET(LR_FRMSB), __REGSET(LR_FRMIB), __REGSET(LR_FRLSB),
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
    
    /// Put SX127x into sleep in order to change registers via LoRa mode 
    ///(required by documentation)
    sx127x_sleep();
    
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
        delay_ms(5);
    }
    while (++i < 0x71);
}






/** Control & Status Functions <BR>
  * ========================================================================<BR>
  * These functions utilize the pin-wrapper driver functions (below), which
  * must be implemented in the platform-specific driver.  Typically, they are
  * just implemented as simple macros.
  * <LI> spirit1_sdnpin_sethigh() </LI>
  * <LI> spirit1_sdnpin_setlow() </LI>
  * <LI> spirit1_resetpin_ishigh() </LI>
  * <LI> spirit1_readypin_ishigh() </LI>
  * <LI> spirit1_abortpin_ishigh() </LI>
  * <LI> spirit1_cspin_ishigh() </LI>
  */

void sx127x_shutdown(ot_uint us) {
/// Raise the Shutdown Line
    sx127x_rstpin_set();
    delay_us(us);
}

void sx127x_reset() {
/// Turn-off interrupts, send Reset strobe, and wait for reset to finish.
    sx127x_int_turnoff(RFI_ALL);
    sx127x_shutdown(120);
    sx127x_waitforreset();
}

void sx127x_waitforreset() {
///@todo Save non-blocking implementation for a rainy day.
/// Blocking implementation: Wait for POR signal to rise using a busy loop.
/// There is a watchdog variable that should count at least 1ms.  The loop
/// itself should take 8 cycles to execute (by inspection), and we assume
/// a clock speed of 16MHz.
    ot_uint watchdog = 2000;
    delay_ms(5);
    while ((sx127x_resetpin_ishigh() == 0) && (--watchdog));
    if (watchdog == 0) {
        ///@todo failure code that logs hardware fault and resets OT
    }
}

ot_u16 sx127x_isready() {
    return sx127x_readypin_ishigh();
}

ot_bool sx127x_check_cspin(void) {
    return (ot_bool)sx127x_cspin_ishigh();
}

/* ot_int sub_failsafe_standby(void) {
/// This code is a failsafe way to bring SPIRIT1 into shutdown, mainly for
/// testing purposes.  It has been observed to have the same result as
/// the normal standby routine.
    ot_u8   nextstrobe;
    ot_u8   mcstate;
    ot_u8   teststate;
    ot_uint watchdog;

    mcstate = spirit1_read(RFREG(MC_STATE0));

    sub_failsafe_standby_TOP:
    switch (mcstate>>1) {
        // Already in STANDBY, which is an error
        case 0x40:  return 1;

        // SLEEP
        case 0x36:  nextstrobe  = RFSTROBE_READY;
                    teststate   = 0x07;
                    break;

        // RX or TX or LOCK
        case 0x0F:
        case 0x33:
        case 0x5f:  nextstrobe  = RFSTROBE_SABORT;
                    teststate   = 0x07;
                    break;

        // READY
        case 0x03:  nextstrobe  = RFSTROBE_STANDBY;
                    teststate   = (0x40<<1);
                    break;

        //Unknown State (error)
        default:    return 3;
    }

    spirit1_strobe(nextstrobe);
    watchdog = 100;

    do {
        mcstate = spirit1_read(RFREG(MC_STATE0));
        watchdog--;
    } while ((watchdog != 0) && (mcstate != teststate));


    if (watchdog == 0)  return 2;
    else                goto sub_failsafe_standby_TOP;
} */

void sx127x_waitforabort() {
/// Wait for the RX/TX indicator pin to go off.
/// @todo this may need to be state-based.  In other words, if ABORT is
///       called during the CSMA mode, the pin may be different or it may
///       not be included at all.

    ///@todo implement this using WFE instead of while loop
    ot_uint watchdog = 100;
    while (sx127x_abortpin_ishigh() && (--watchdog));

    if (watchdog == 0) {
        //abort_fails++;
        ///@todo failure code that logs hardware fault and resets OT
        sx127x_shutdown(300);
        dll_init();
    }
}

void sx127x_waitforready() {
/// Wait for the Ready Pin to go high (reset pin is remapped in init).
/// STANDBY->READY should take about 75us, although the absolute worst
/// case is: 220us, 230us, 240us, 440us, 460us, 480us with respective
/// 52, 50, 48, 26, 25, 24 MHz crystals.  By inspection, the loop takes
/// 8 cycles to complete one iteration, and we assume a clock speed of
/// 16 MHz.

    ///@todo implement this using WFE instead of while loop
    ot_uint watchdog = 500;
    while ((sx127x_readypin_ishigh() == 0) && (--watchdog));

    if (watchdog == 0){
        //ready_fails++;
        ///@todo failure code that logs hardware fault and resets OT
        sx127x_shutdown(300);
        dll_init();
    }
}

void sx127x_waitforsleep() {
/// Use Mode-ready setting on DIO5
    sx127x_waitforready();
}

ot_u16 sx127x_getstatus() {
/// Status is register IRQFLAGS
    return sx127x_read(RFREG_LR_IRQFLAGS);
}

ot_u8   sx127x_rxbytes()    { return sx127x_read(RFREG_LR_FIFORXBYTEADDR); }
ot_u8   sx127x_rssi()       { return sx127x_read(RFREG_LR_RSSIVALUE); }
ot_u8   sx127x_pktrssi()    { return sx127x_read(RFREG_LR_PKTRSSIVALUE) >> 2; }
ot_s8   sx127x_pktsnr()     { return sx127x_read(RFREG_LR_PKTSNRVALUE); }




/** Counter Management Functions <BR>
  * ========================================================================<BR>
  * Certain MAC processes require a running timer.  Instead of using any
  * internal timers of the SX127x, we instead use the more reliable interval
  * timer feature of OpenTag.
  */
ot_u32 macstamp;

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






/** Data Functions <BR>
  * ========================================================================<BR>
  */

ot_int sx127x_calc_rssi(ot_u8 encoded_value, ot_s8 packet_snr) {
    ot_int rssi;
    ot_int snr;
    rssi    = -139 + (ot_int)encoded_value;
    snr     = (ot_int)packet_snr;
    if (packet_snr < 0) {
        rssi += packet_snr/4;
    }
    return rssi;
}


ot_u8 sx127x_clip_txeirp(ot_u8 input_eirp) {
/// This considers Normal-Mode.  In TX Boost mode, 13dBm --> 20dBm

#   define _MAX_DBM_EIRP (((13*2) - RF_HDB_ATTEN) + 80)
    
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
    
    // Convert to SX127x units (PA Boost off)
    dBm = dBm + 1;
    pwr_code = (ot_u8)dBm & 0x0F;

    // Write new PA Table to device
    sx127x_write(RFREG_LR_PACONFIG, pwr_code);
}


#endif


