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
  * @file       /io/spirit1/generic_interface.c
  * @author     JP Norair
  * @version    R101
  * @date       28 Aug 2014
  * @brief      Generic, High-level interface routines for SPIRIT1 transceiver
  * @ingroup    SPIRIT1
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


#include <io/spirit1/interface.h>
#include <io/spirit1/config.h>

//#include <platform/config.h>


#include <otlib/memcpy.h>

#include <otlib/delay.h>



///@todo right now these are used with m2DLL.... move those functions elsewhere

#include <m2/dll.h>


// These only for driver testing purposes (spirit1_coredump())
#include <otlib/utils.h>
#include <otlib/logger.h>



#if defined(_SPIRIT1_PROFILE)
    ot_s16 set_line;
    ot_s16 line_hits[256];
    ot_u16 count_hits = 0;
#endif



/** Module Data for radio driver interface <BR>
  * ========================================================================
  */
spirit1_struct spirit1;




/** High-Level Pin Interrupt Management Functions <BR>
  * ========================================================================<BR>
  * These functions manage the usage of the SPIRIT1 GPIO bus, which is used to
  * signal status interrupts to the platform.  The Low-Level, platform-specific
  * driver must implement the following low-level functions which are called by
  * these high-level functions:
  * <LI> spirit1_int_config() </LI>
  * <LI> spirit1_int_txdone() </LI>
  * <LI> spirit1_int_clearall() </LI>
  * <LI> spirit1_int_force() </LI>
  * <LI> spirit1_int_turnon() </LI>
  * <LI> spirit1_int_turnoff() </LI>
  */
OT_WEAK void spirit1_int_off() {
    spirit1_int_config(0);
}

OT_WEAK void spirit1_int_on() {
    ot_u32 ie_sel;
    switch (spirit1.imode) {
        case MODE_Listen:   ie_sel = RFI_LISTEN;
        case MODE_RXData:   ie_sel = RFI_RXDATA;
        case MODE_CSMA:     ie_sel = RFI_CSMA;
        case MODE_TXData:   ie_sel = RFI_TXFIFO;
        default:            ie_sel = 0;
    }
    spirit1_int_config(ie_sel);
}

static const ot_u8 gpio_rx[5] = {
    0, RFREG(GPIO2_CONF),
    RFGPO(RX_FIFO_ALMOST_FULL),  //indicate buffer threshold condition (kept for RX)
    RFGPO(SYNC_WORD),            //indicate when sync word is qualified
    RFGPO(TRX_INDICATOR)             //indicate when RX is active (falling edge)
};

static const ot_u8 gpio_tx[5] = {
    0, RFREG(GPIO2_CONF),
    RFGPO(TX_FIFO_ALMOST_EMPTY), //indicate buffer threshold condition
    RFGPO(RSSI_ABOVE_THR),       //indicate if RSSI goes above/below CCA threshold
    RFGPO(TRX_INDICATOR)         //indicate when TX or RX is active
};

inline void spirit1_iocfg_rx()  {
    spirit1_int_clearall();
    spirit1_spibus_io(5, 0, (ot_u8*)gpio_rx);
}

inline void spirit1_iocfg_tx()  {
    spirit1_int_clearall();
    spirit1_spibus_io(5, 0, (ot_u8*)gpio_tx);
}

inline void spirit1_int_listen() {
    spirit1.imode = MODE_Listen;
    spirit1_int_config(RFI_LISTEN);
}

inline void spirit1_int_rxdata() {
    spirit1.imode = MODE_RXData;
    spirit1_int_config(RFI_RXDATA);
}

inline void spirit1_int_csma() {
    spirit1.imode = MODE_CSMA;
    spirit1_int_config(RFI_CSMA);
}

inline void spirit1_int_txdata() {
    spirit1.imode = MODE_TXData;
    spirit1_int_config(RFI_TXFIFO);
}


void spirit1_irq0_isr() {   spirit1_virtual_isr(spirit1.imode);     }
void spirit1_irq1_isr() {   spirit1_virtual_isr(spirit1.imode + 1); }
void spirit1_irq2_isr() {   spirit1_virtual_isr(spirit1.imode + 2); }
void spirit1_irq3_isr() {   spirit1_virtual_isr(spirit1.imode + 3); }







/** High-Level Read, Write, and Load-Defaults Functions <BR>
  * ========================================================================<BR>
  * These utilize the driver function: spirit1_spibus_io()
  * This function must be implemented specific to the platform.
  */

void spirit1_strobe(ot_u8 strobe) {
    ot_u8 cmd[2];
    cmd[0]  = 0x80;
    cmd[1]  = strobe;
    spirit1_spibus_io(2, 0, cmd);
}

ot_u8 spirit1_read(ot_u8 addr) {
    ot_u8 cmd[2];
    cmd[0]  = 1;
    cmd[1]  = addr;
    spirit1_spibus_io(2, 1, cmd);
    return spirit1.busrx[0];
}

void spirit1_burstread(ot_u8 start_addr, ot_u8 length, ot_u8* data) {
    ot_u8 cmd[2];
    cmd[0]  = 1;
    cmd[1]  = start_addr;
    spirit1_spibus_io(2, length, (ot_u8*)cmd);
    memcpy(data, spirit1.busrx, length);
}

void spirit1_write(ot_u8 addr, ot_u8 data) {
    ot_u8 cmd[3];
    cmd[0]  = 0;
    cmd[1]  = addr;
    cmd[2]  = data;
    spirit1_spibus_io(3, 0, cmd);
}

void spirit1_burstwrite(ot_u8 start_addr, ot_u8 length, ot_u8* cmd_data) {
    cmd_data[0] = 0;
    cmd_data[1] = start_addr;
    spirit1_spibus_io((2+length), 0, cmd_data);
}

void spirit1_load_defaults() {
/// The data ordering is: WRITE LENGTH, WRITE HEADER (0), START ADDR, VALUES
/// Ignore registers that are set later, are unused, or use the hardware default values.
    static const ot_u8 spirit1_defaults[] = {
        15, 0,  0x01,   DRF_ANA_FUNC_CONF0,
                        RFGPO(READY), RFGPO(GND), RFGPO(GND), RFGPO(GND),
                        DRF_MCU_CK_CONF,
                        DRF_IF_OFFSET_ANA, DRF_SYNT3, DRF_SYNT2, DRF_SYNT1,
                        DRF_SYNT0, DRF_CHSPACE, DRF_IF_OFFSET_DIG,
        3,  0,  0xB4,   DRF_XO_RCO_TEST,
        4,  0,  0x9E,   DRF_SYNTH_CONFIG1, DRF_SYNTH_CONFIG0,
        3,  0,  0x18,   DRF_PAPOWER0,
        6,  0,  0x1C,   DRF_FDEV0, DRF_CHFLT_LS, DRF_AFC2, DRF_AFC1,
        7,  0,  0x23,   DRF_CLOCKREC, DRF_AGCCTRL2, DRF_AGCCTRL1, DRF_AGCCTRL0, DRF_ANT_SELECT_CONF,
        3,  0,  0x3A,   DRF_QI,
        3,  0,  0x41,   DRF_FIFO_CONFIG0,
        4,  0,  0x4F,   DRF_PCKT_FLT_OPTIONS, DRF_PROTOCOL2,
      //3,  0,  0x93,   RFINT_TX_FIFO_ERROR,
        6,  0,  0xA3,   DRF_DEM_ORDER, DRF_PM_CONFIG2, DRF_PM_CONFIG1, DRF_PM_CONFIG0,
        0   //Terminating 0
    };

    ot_u8* cursor;
    cursor = (ot_u8*)spirit1_defaults;

    while (*cursor != 0) {
        ot_u8 cmd_len   = *cursor++;
        ot_u8* cmd      = cursor;
        cursor         += cmd_len;
        spirit1_spibus_io(cmd_len, 0, cmd);
    }

    // Early debugging test to make sure data was written (look at first write block)
    //{
    //    volatile ot_u8 test;
    //    ot_u8 i;
    //    for (i=0x01; i<=0x0D; ++i) {
    //        test = spirit1_read(i);
    //    }
    //}
}

void spirit1_coredump() {
///debugging function to dump-out register values of RF core (not all are used)
    ot_u8 i = 0;
    ot_u8 regval;
    ot_u8 label[]   = { 'R', 'E', 'G', '_', 0, 0 };

    do {
        regval = spirit1_read(i);
        otutils_bin2hex(&label[4], &i, 1);
        logger_msg(MSG_raw, 6, 1, label, &regval);
        //mpipedrv_wait();
        delay_ms(5);
    }
    while (++i != 0);
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

void spirit1_shutdown(ot_uint us) {
/// Raise the Shutdown Line
    spirit1_sdnpin_sethigh();
    delay_us(us);
}

void spirit1_reset() {
/// Turn-off interrupts, send Reset strobe, and wait for reset to finish.
    spirit1_int_turnoff(RFI_ALL);
    spirit1_strobe(STROBE(SRES));
    spirit1_waitforreset();
}

void spirit1_waitforreset() {
///@todo Save non-blocking implementation for a rainy day.
/// Blocking implementation: Wait for POR signal to rise using a busy loop.
/// There is a watchdog variable that should count at least 1ms.  The loop
/// itself should take 8 cycles to execute (by inspection), and we assume
/// a clock speed of 16MHz.
    ot_uint watchdog = 2000;

    while ((spirit1_resetpin_ishigh() == 0) && (--watchdog));
    if (watchdog == 0) {
        ///@todo failure code that logs hardware fault and resets OT
    }
}

ot_u16 spirit1_isready() {
    return spirit1_readypin_ishigh();
}

ot_bool spirit1_check_cspin(void) {
    return (ot_bool)spirit1_cspin_ishigh();
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

void spirit1_waitforabort() {
/// Wait for the RX/TX indicator pin to go off.
/// @todo this may need to be state-based.  In other words, if ABORT is
///       called during the CSMA mode, the pin may be different or it may
///       not be included at all.

    ///@todo implement this using WFE instead of while loop
    ot_uint watchdog = 100;
    while (spirit1_abortpin_ishigh() && (--watchdog));

    if (watchdog == 0) {
        //abort_fails++;
        ///@todo failure code that logs hardware fault and resets OT
        spirit1_shutdown(300);
        dll_init();
    }
}

void spirit1_waitforready() {
/// Wait for the Ready Pin to go high (reset pin is remapped in init).
/// STANDBY->READY should take about 75us, although the absolute worst
/// case is: 220us, 230us, 240us, 440us, 460us, 480us with respective
/// 52, 50, 48, 26, 25, 24 MHz crystals.  By inspection, the loop takes
/// 8 cycles to complete one iteration, and we assume a clock speed of
/// 16 MHz.

    ///@todo implement this using WFE instead of while loop
    ot_uint watchdog = 500;
    while ((spirit1_readypin_ishigh() == 0) && (--watchdog));

    if (watchdog == 0){
        //ready_fails++;
        ///@todo failure code that logs hardware fault and resets OT
        spirit1_shutdown(300);
        dll_init();
    }
}

void spirit1_waitforstandby() {
/// Wait for the Ready Pin to go low (reset pin is remapped in init).
///@todo implement this using WFE instead of while loop
    ot_uint watchdog = 10;

    spirit1_waitforstandby_TOP:
    while (spirit1_readypin_ishigh() && (--watchdog));

    // Critical Failure
    if (watchdog == 0) {
        ///@todo failure code that logs hardware fault and resets OT
        spirit1_shutdown(300);
        dll_init();
    }
}

ot_u16 spirit1_getstatus() {
/// Status is sent during every SPI access, so refresh it manually by doing a
/// dummy read, and then returning the global status data that is obtained
/// during the read process.
    spirit1_read(0);
    return spirit1.status;
}

ot_u16 spirit1_mcstate() {
    static const ot_u8 cmd[2] = { 1, RFREG(MC_STATE1) };
    spirit1_spibus_io(2, 2, (ot_u8*)cmd);
    return (ot_u16)*((ot_u16*)spirit1.busrx);
}

ot_u8   spirit1_ldctime()       { return spirit1_read( RFREG(TIMERS2) ); }
ot_u8   spirit1_ldcrtime()      { return spirit1_read( RFREG(TIMERS0) ); }
ot_u8   spirit1_rxtime()        { return spirit1_read( RFREG(TIMERS4) ); }
ot_u8   spirit1_rxbytes()       { return spirit1_read( RFREG(LINEAR_FIFO_STATUS0) ); }
ot_u8   spirit1_txbytes()       { return spirit1_read( RFREG(LINEAR_FIFO_STATUS1) ); }
ot_u8   spirit1_rssi()          { return spirit1_read( RFREG(RSSI_LEVEL) ); }






/** Counter Management Functions <BR>
  * ========================================================================<BR>
  * Certain MAC processes require a running timer.  Instead of using any
  * internal timers of the SPIRIT1, we instead use the more reliable interval
  * timer feature of OpenTag.
  */
ot_u32 macstamp;

void spirit1_start_counter() {
    macstamp = systim_chronstamp(NULL);
}

void spirit1_stop_counter() {
}

ot_u16 spirit1_get_counter() {
    ot_u16 value;
    value = dll.counter - (ot_u16)systim_chronstamp(&macstamp);
    return value;
}






/** Data Functions <BR>
  * ========================================================================<BR>
  */

ot_int spirit1_calc_rssi(ot_u8 encoded_value) {
/// From SPIRIT1 datasheet: "The measured power is reported in steps of half-dB
/// from 0 to 255 and is offset in such a way that -120 dBm corresponds to
/// about 20."  In other words, it is linear: { 0 = -130dBm, 255 = -2.5dBm }.
/// This function turns the coded value into a normal, signed int.
    ot_int rssi_val;
    rssi_val    = (ot_int)encoded_value;    // Convert to signed int
    rssi_val   -= 260;                      // Apply 130 dBm offset (260 half-dBm)
    return rssi_val;
}


ot_u8 spirit1_calc_rssithr(ot_u8 input) {
/// SPIRIT1 treats RSSI thresholding through the normal RSSI engine.  The specs
/// are the same as those used in spirit1_calc_rssi() above, but the process is
/// using different input and output.
///
/// Input is a whole-dBm value encoded linearly as: {0=-140dBm, 127=-13dBm}.
/// Output is the value that should go into SPIRIT1 RSSI_TH field.
    ot_int rssi_thr;

    // SPIRIT1 uses -130 as baseline, DASH7 -140
    // Clip baseline at 0
    rssi_thr = (ot_int)input - 10;
    if (rssi_thr < 0)
        rssi_thr = 0;

    // Multiply by 2 to yield half-dBm.
    rssi_thr  <<= 1;
    return rssi_thr;
}


ot_u8 spirit1_clip_txeirp(ot_u8 input_eirp) {
    if ( input_eirp > ((22 - RF_HDB_ATTEN) + 80) ) {
        input_eirp = (22 - RF_HDB_ATTEN) + 80;
    }
    return input_eirp;
}


void spirit1_set_txpwr(ot_u8* pwr_code) {
/// Sets the tx output power.
/// "pwr_code" is a value, 0-127, that is: eirp_code/2 - 40 = TX dBm
/// i.e. eirp_code=0 => -40 dBm, eirp_code=80 => 0 dBm, etc
    static const ot_u8 pa_lut[84] = {
          87, 0x57, 0x56, 0x55, 0x54, 0x53, 0x53, 0x52, 0x52, 0x50,     //-30 to -25.5
        0x4F, 0x4E, 0x4D, 0x4C, 0x4B, 0x4B, 0x4A, 0x49, 0x48, 0x47,     //-25 to -20.5
        0x46, 0x45, 0x44, 0x43, 0x42, 0x41, 0x40, 0x3F, 0x3E, 0x3C,     //-20 to -15.5
        0x3B, 0x3A, 0x39, 0x38, 0x37, 0x36, 0x34, 0x33, 0x32, 0x31,     //-15 to -10.5
        0x30, 0x2F, 0x2D, 0x2C, 0x2B, 0x2A, 0x29, 0x27,   42, 0x25,     //-10 to -5.5
        0x24, 0x23, 0x22, 0x20, 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x19,     //-5 to -0.5
          30, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10, 0x0F,     // 0 to 4.5
        0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05,     // 5 to 9.5
        0x04, 0x03,    1, 0x01                                          // 10 to 11.5
    };

    ot_u8   pa_table[10];
    ot_u8*  cursor;
    ot_int  step;
    ot_int  eirp_val;

    ///@todo autoscaling algorithm, and refresh value in *pwr_code
    // Autoscaling: Try to make RSSI at receiver be -90 < RX_RSSI < -80
    // The simple algorithm uses the last_rssi and last_linkloss values
    //if (*pwr_code & 0x80) {
    //}
    //else {
    //}

    // Not autoscaling: extract TX power directly from pwr_code
    eirp_val = *pwr_code;

    // Offset SPIRIT1 PA CFG to match DASH7 PA CFG, plus antenna losses
    // SPIRIT1: 0 --> -30 dBm, 83 --> 11.5 dBm, half-dBm steps
    // DASH7: 0 --> -40 dBm, 127 --> 23.5 dBm, half-dBm steps
    eirp_val += (-10*2) + (RF_HDB_ATTEN);

    // Adjust base power code in case it is out-of-range:
    // SPIRIT1 PA starts at -30, DASH7 pwr_code starts at -40.
    if (eirp_val < 0)       eirp_val = 0;
    else if (eirp_val > 83) eirp_val = 83;

    // Build PA RAMP using 8 steps of variable size.
    pa_table[0] = 0;
    pa_table[1] = RFREG(PAPOWER8);
    cursor      = &pa_table[2];
    step        = eirp_val >> 3;
    do {
        *cursor++   = pa_lut[eirp_val];
        eirp_val   -= step;
    } while (cursor != &pa_table[9]);


    // Write new PA Table to device
    spirit1_spibus_io(10, 0, pa_table);
}


#endif


