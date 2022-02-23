/* Copyright 2016 JP Norair
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
  */
/**
  * @file       /io/stm32wl_lora/stm32wl_lora_interface.c
  * @author     JP Norair
  * @version    R101
  * @date       31 Jan 2022
  * @brief      Generic, High-level interface routines for STM32WL transceiver
  * @ingroup    STM32WL_LoRa
  * 
  * @todo this file is a very incomplete port from SX127x to STM32WL LoRa.
  *       It is only used right now for compiling platform tests, and it
  *       will be overhauled shortly. 
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
#if defined(__STM32WL_LORA__)


#include <io/stm32wl_lora/interface.h>
#include <io/stm32wl_lora/config.h>

//#include <platform/config.h>


#include <otlib/memcpy.h>
#include <otlib/delay.h>


///@todo this is just for the shim, it's a faux SX127x register bank.
///      Driver will need to be completely changed for WL.
static ot_u8 faux_regs[256];

///@todo right now these are used with m2DLL.... move those functions elsewhere
#include <m2/dll.h>


// These only for driver testing purposes (wllora_coredump())
#include <otlib/utils.h>
#include <otlib/logger.h>



#if defined(_STM32WL_PROFILE)
    ot_s16 set_line;
    ot_s16 line_hits[256];
    ot_u16 count_hits = 0;
#endif



/** Module Data for radio driver interface <BR>
  * ========================================================================
  */
wllora_struct wllora;





/** Generic Command Functions from SX126x & STM32WL manuals
  * For specific networking stacks, it is often better to use custom-prepared
  * versions of these command calls (with fewer arguments) rather than the
  * generic implementations below.
  */

void sub_set0x1_cmd(ot_u8 opcode) {
    wllora.cmd.raw[0]   = opcode;
    wllora_spibus_io(1, 0, wllora.cmd.raw);
}

void sub_set1x1_cmd(ot_u8 opcode, ot_u8 val) {
    wllora.cmd.raw[0]   = opcode;
    wllora.cmd.raw[1]   = val;
    wllora_spibus_io(2, 0, wllora.cmd.raw);
}

void sub_set2x1_cmd(ot_u8 opcode, ot_u8 val1, ot_u8 val2) {
    wllora.cmd.raw[0]   = opcode;
    wllora.cmd.raw[1]   = va11;
    wllora.cmd.raw[2]   = val2;
    wllora_spibus_io(3, 0, wllora.cmd.raw);
}

void sub_set4x1_cmd(ot_u8 opcode, ot_u8 val1, ot_u8 val2, ot_u8 val3, ot_u8 val4) {
    wllora.cmd.raw[0]   = opcode;
    wllora.cmd.raw[1]   = val1;
    wllora.cmd.raw[2]   = val2;
    wllora.cmd.raw[3]   = val3;
    wllora.cmd.raw[4]   = val4;
    wllora_spibus_io(5, 0, wllora.cmd.raw);
}

void sub_set1x3_cmd(ot_u8 opcode, ot_u32 val) {
    wllora.cmd.raw[0]   = opcode;
    wllora.cmd.raw[1]   = ((ot_u8*)&val)[B2];
    wllora.cmd.raw[2]   = ((ot_u8*)&val)[B1];
    wllora.cmd.raw[3]   = ((ot_u8*)&val)[B0];
    wllora_spibus_io(4, 0, wllora.cmd.raw);
}

ot_u8 sub_get_1x1(ot_u8 opcode) {
    wllora.cmd.raw[0] = opcode;
    wllora_spibus_io(1, 2, wllora.cmd.raw);
    return wllora.cmd.raw[2];
}

ot_u16 sub_get1x2_cmd(ot_u8 opcode) {
    ot_u32 temp;
    wllora.cmd.raw[0] = opcode;
    wllora_spibus_io(1, 3, wllora.cmd.raw);

    temp = *(ot_u16*)&wllora.cmd.raw[2];
    return (ot_u16)__REV16(temp);
}

void wllora_wrreg_cmd(lr_addr_u addr, ot_u8 value) {
    wllora.cmd.wrreg.opcode     = 0x0D;
    wllora.cmd.wrreg.addr[0]    = addr.u8[0];
    wllora.cmd.wrreg.addr[1]    = addr.u8[1];
    wllora.cmd.wrreg.data[0]    = value;
    wllora_spibus_io(4, 0, wllora.cmd.raw);
}

void wllora_wrburst_cmd(lr_addr_u addr, ot_u8 len, ot_u8* data) {
    wllora.cmd.wrreg.opcode     = 0x0D;
    wllora.cmd.wrreg.addr[0]    = addr.u8[0];
    wllora.cmd.wrreg.addr[1]    = addr.u8[1];

    if (len <= WLLORA_WRMAX) {
        memcpy(wllora.cmd.wrreg.data, data, len);
        wllora_spibus_io(3+len, 0, wllora.cmd.raw);
    }
}

ot_u8 wllora_rdreg_cmd(lr_addr_u addr) {
    wllora.cmd.rdreg.opcode     = 0x1D;
    wllora.cmd.rdreg.addr[0]    = addr.u8[0];
    wllora.cmd.rdreg.addr[1]    = addr.u8[1];
    wllora_spibus_io(3, 2, wllora.cmd.raw);

    return wllora.cmd.rdreg.data[0];
}

void wllora_rdburst_cmd(lr_addr_u addr, ot_u8 len, ot_u8* data) {
    wllora.cmd.rdreg.opcode     = 0x1D;
    wllora.cmd.rdreg.addr[0]    = addr.u8[0];
    wllora.cmd.rdreg.addr[1]    = addr.u8[1];

    if (len <= WLLORA_RDMAX) {
        wllora_spibus_io(3, 1+len, wllora.cmd.raw);
        memcpy(data, wllora.cmd.rdreg.data, len);
    }
}

void wllora_wrbuf_cmd(ot_u8 offset, ot_u8 len, ot_u8* data) {
    wllora.cmd.wrbuf.opcode     = 0x0E;
    wllora.cmd.wrbuf.offset     = offset;

    if (len <= WLLORA_WRBUFMAX) {
        memcpy(wllora.cmd.wrbuf.data, data, len);
        wllora_spibus_io(2+len, 0, wllora.cmd.raw);
    }
}

void wllora_rdbuf_cmd(ot_u8 offset, ot_u8 len, ot_u8* data) {
    wllora.cmd.rdbuf.opcode     = 0x1E;
    wllora.cmd.rdbuf.offset     = offset;

    if (len <= WLLORA_WRBUFMAX) {
        wllora_spibus_io(3+len, 0, wllora.cmd.raw);
        memcpy(data, wllora.cmd.rdbuf.data, len);
    }
}

void wllora_sleep_cmd(ot_u8 sleep_cfg) {
    sub_set1x1_cmd(0x84, sleep_cfg);
//    wllora.cmd.sleep.opcode     = 0x84;
//    wllora.cmd.sleep.sleep_cfg  = sleep_cfg;
//    wllora_spibus_io(2, 0, wllora.cmd.raw);
}

void wllora_standby_cmd(ot_u8 standby_cfg) {
    sub_set1x1_cmd(0x80, standby_cfg);
//    wllora.cmd.standby.opcode       = 0x80;
//    wllora.cmd.standby.standby_cfg  = standby_cfg;
//    wllora_spibus_io(2, 0, wllora.cmd.raw);
}

void wllora_fs_cmd(void) {
    sub_set0x1_cmd(0xC1);
//    wllora.cmd.fs.opcode = 0xC1;
//    wllora_spibus_io(1, 0, wllora.cmd.raw);
}

void wllora_tx_cmd(ot_u32 timeout) {
    sub_set1x3_cmd(0x83, timeout);
//    wllora.cmd.tx.opcode        = 0x83;
//    wllora.cmd.tx.timeout[0]    = ((ot_u8*)&timeout)[B2];
//    wllora.cmd.tx.timeout[1]    = ((ot_u8*)&timeout)[B1];
//    wllora.cmd.tx.timeout[2]    = ((ot_u8*)&timeout)[B0];
//    wllora_spibus_io(4, 0, wllora.cmd.raw);
}

void wllora_rx_cmd(ot_u32 timeout) {
    sub_set1x3_cmd(0x82, timeout);
//    wllora.cmd.rx.opcode        = 0x82;
//    wllora.cmd.rx.timeout[0]    = ((ot_u8*)&timeout)[B2];
//    wllora.cmd.rx.timeout[1]    = ((ot_u8*)&timeout)[B1];
//    wllora.cmd.rx.timeout[2]    = ((ot_u8*)&timeout)[B0];
//    wllora_spibus_io(4, 0, wllora.cmd.raw);
}

void wllora_stoprxtim_cmd(ot_u8 rx_timeout_stop) {
    sub_set1x1_cmd(0x9F, rx_timeout_stop);
//    wllora.cmd.stoprxtim.opcode             = 0x9F;
//    wllora.cmd.stoprxtim.rx_timeout_stop    = rx_timeout_stop;
//    wllora_spibus_io(2, 0, wllora.cmd.raw);
}

//2x3
void wllora_rxduty_cmd(ot_u32 rx_period, ot_u32 sleep_period) {
    wllora.cmd.rxduty.opcode            = 0x94;
    wllora.cmd.rxduty.rx_period[0]      = ((ot_u8*)&rx_period)[B2];
    wllora.cmd.rxduty.rx_period[1]      = ((ot_u8*)&rx_period)[B1];
    wllora.cmd.rxduty.rx_period[2]      = ((ot_u8*)&rx_period)[B0];
    wllora.cmd.rxduty.sleep_period[0]   = ((ot_u8*)&sleep_period)[B2];
    wllora.cmd.rxduty.sleep_period[1]   = ((ot_u8*)&sleep_period)[B1];
    wllora.cmd.rxduty.sleep_period[2]   = ((ot_u8*)&sleep_period)[B0];
    wllora_spibus_io(7, 0, wllora.cmd.raw);
}

void wllora_cad_cmd(void) {
    sub_set0x1_cmd(0xC5);
//    wllora.cmd.fs.opcode = 0xC5;
//    wllora_spibus_io(1, 0, wllora.cmd.raw);
}

void wllora_txcontwave_cmd(void) {
    sub_set0x1_cmd(0xD1);
//    wllora.cmd.txcontwave.opcode = 0xD1;
//    wllora_spibus_io(1, 0, wllora.cmd.raw);
}

void wllora_txcontpreamble_cmd(void) {
    sub_set0x1_cmd(0xD2);
//    wllora.cmd.txcontpreamble.opcode = 0xD2;
//    wllora_spibus_io(1, 0, wllora.cmd.raw);
}

void wllora_setpkttype_cmd(ot_u8 pkt_type) {
    sub_set1x1_cmd(0x8A, pkt_type);
//    wllora.cmd.setpkttype.opcode    = 0x8A;
//    wllora.cmd.setpkttype.pkt_type  = pkt_type;
//    wllora_spibus_io(2, 0, wllora.cmd.raw);
}

ot_u8 wllora_getpkttype_cmd(void) {
    return sub_get_1x1(0x11);
//    wllora.cmd.getpkttype.opcode = 0x11;
//    wllora_spibus_io(1, 2, wllora.cmd.raw);
//    return wllora.cmd.getpkttype.pkt_type;
}

void wllora_rffreq_cmd(ot_u32 freq) {
    wllora.cmd.rffreq.opcode    = 0x86;
    wllora.cmd.rffreq.freq[0]   = ((ot_u8*)&freq)[B3];
    wllora.cmd.rffreq.freq[1]   = ((ot_u8*)&freq)[B2];
    wllora.cmd.rffreq.freq[2]   = ((ot_u8*)&freq)[B1];
    wllora.cmd.rffreq.freq[3]   = ((ot_u8*)&freq)[B0];
    wllora_spibus_io(5, 0, wllora.cmd.raw);
}

void wllora_txparams_cmd(ot_u8 power, ot_u8 ramp_time) {
    sub_set2x1_cmd(0x8E, power, ramp_time);
//    wllora.cmd.txparams.opcode      = 0x8E;
//    wllora.cmd.txparams.power       = power;
//    wllora.cmd.txparams.ramp_time   = ramp_time;
//    wllora_spibus_io(3, 0, wllora.cmd.raw);
}

void wllora_paconfig_cmd(ot_u8 pa_duty, ot_u8 hp_max, ot_u8 pa_sel) {
    sub_set4x1_cmd(0x95, pa_duty, hp_max, pa_sel, 1);
//    wllora.cmd.paconfig.opcode      = 0x95;
//    wllora.cmd.paconfig.pa_duty     = pa_duty;
//    wllora.cmd.paconfig.hp_max      = hp_max;
//    wllora.cmd.paconfig.pa_sel      = pa_sel;
//    wllora.cmd.paconfig.fixed_01    = 1;
//    wllora_spibus_io(5, 0, wllora.cmd.raw);
}

void wllora_txrxfallback_cmd(ot_u8 fallback_mode) {
    sub_set1x1_cmd(0x93, fallback_mode);
//    wllora.cmd.txrxfallback.opcode          = 0x93;
//    wllora.cmd.txrxfallback.fallback_mode   = fallback_mode;
//    wllora_spibus_io(2, 0, wllora.cmd.raw);
}


void wllora_cadparams_cmd(ot_u8 num_symbol, ot_u8 det_peak, ot_u8 exit_mode, ot_u32 timeout) {
    wllora.cmd.cadparams.opcode     = 0x88;
    wllora.cmd.cadparams.num_symbol = num_symbol;
    wllora.cmd.cadparams.det_peak   = det_peak;
    wllora.cmd.cadparams.det_min    = det_min;
    wllora.cmd.cadparams.exit_mode  = exit_mode;
    wllora.cmd.cadparams.timeout[0] = ((ot_u8*)&timeout)[B2];
    wllora.cmd.cadparams.timeout[1] = ((ot_u8*)&timeout)[B1];
    wllora.cmd.cadparams.timeout[2] = ((ot_u8*)&timeout)[B0];
    wllora_spibus_io(8, 0, wllora.cmd.raw);
}

void wllora_bufbase_cmd(ot_u8 tx_base_addr, ot_u8 rx_base_addr) {
    sub_set2x1_cmd(0x8F, tx_base_addr, rx_base_addr);
//    wllora.cmd.bufbase.opcode       = 0x8F;
//    wllora.cmd.bufbase.tx_base_addr = tx_base_addr;
//    wllora.cmd.bufbase.rx_base_addr = rx_base_addr;
//    wllora_spibus_io(3, 0, wllora.cmd.raw);
}

void wllora_modparams_cmd(ot_u8 sf, ot_u8 bw, ot_u8 cr, ot_u8 ldro) {
    sub_set4x1_cmd(0x8B, sf, bw, cr, ldro);
//    wllora.cmd.modparams.opcode = 0x8B;
//    wllora.cmd.modparams.sf     = sf;
//    wllora.cmd.modparams.bw     = bw;
//    wllora.cmd.modparams.cr     = cr;
//    wllora.cmd.modparams.ldro   = ldro;
//    wllora_spibus_io(5, 0, wllora.cmd.raw);
}


void wllora_pktparams_cmd(ot_u16 preamble_len, ot_u8 hdr_type, ot_u8 payload_len, ot_u8 crc_type, ot_u8 invert_iq) {
    wllora.cmd.pktparams.opcode         = 0x8C;
    wllora.cmd.pktparams.preamble_len[0]= ((ot_u8*)&preamble_len)[UPPER];
    wllora.cmd.pktparams.preamble_len[1]= ((ot_u8*)&preamble_len)[LOWER];
    wllora.cmd.pktparams.hdr_type       = hdr_type;
    wllora.cmd.pktparams.payload_len    = payload_len;
    wllora.cmd.pktparams.crc_type       = crc_type;
    wllora.cmd.pktparams.invert_iq      = invert_iq;
    wllora_spibus_io(7, 0, wllora.cmd.raw);
}

void wllora_symtimeout_cmd(ot_u8 sym_num) {
    sub_set1x1_cmd(0xA0, sym_num);
//    wllora.cmd.symtimeout.opcode    = 0xA0;
//    wllora.cmd.symtimeout.sym_num   = pkt_type;
//    wllora_spibus_io(2, 0, wllora.cmd.raw);
}


ot_u8 wllora_status_cmd(void) {
    wllora.cmd.status.opcode = 0xC0;
    wllora_spibus_io(1, 1, wllora.cmd.raw);
    return wllora.cmd.status.status;
}

lr_rxbufstatus_t wllora_rxbufstatus_cmd(void) {
///@todo make sure this return typecast doesn't crash on M0 core
    wllora.cmd.rxbufstatus.opcode = 0x13;
    wllora_spibus_io(1, 3, wllora.cmd.raw);
    return *(lr_rxbufstatus_t*)&wllora.cmd.rxbufstatus.rx_payload_len;
}

lr_pktlink_t wllora_pktlink_cmd(void) {
///@todo make sure this return typecast doesn't crash on M0 core
    wllora.cmd.pktlink.opcode = 0x14;
    wllora_spibus_io(1, 4, wllora.cmd.raw);
    return *(lr_pktlink_t*)&wllora.cmd.pktlink.rssi_pkt;
}

ot_u8 wllora_rssi_cmd(void) {
    return sub_get_1x1(0x15);
//    wllora.cmd.rssi.opcode = 0x15;
//    wllora_spibus_io(1, 2, wllora.cmd.raw);
//    return wllora.cmd.rssi.rssi_inst;
}

lr_pktstats_t wllora_pktstats_cmd(void) {
///@todo make sure this return typecast doesn't crash on M0 core
    wllora.cmd.pktstats.opcode = 0x10;
    wllora_spibus_io(1, 7, wllora.cmd.raw);
    return *(lr_pktstats_t*)&wllora.cmd.pktstats.num_pkt_rxed;
}

void wllora_resetstats_cmd(void) {
    memset(wllora.cmd.raw, 0, 7);
    wllora_spibus_io(7, 0, wllora.cmd.raw);
    return wllora.cmd.status.status;
}

void wllora_dioirq_cmd(ot_u16 irq_mask, ot_u16 irq1_mask, ot_u16 irq2_mask, ot_u16 irq3_mask) {
    wllora.cmd.dioirq.opcode        = 0x08;
    wllora.cmd.dioirq.irq_mask[0]   = ((ot_u8*)&irq_mask)[UPPER];
    wllora.cmd.dioirq.irq_mask[1]   = ((ot_u8*)&irq_mask)[LOWER];
    wllora.cmd.dioirq.irq1_mask[0]  = ((ot_u8*)&irq1_mask)[UPPER];
    wllora.cmd.dioirq.irq1_mask[1]  = ((ot_u8*)&irq1_mask)[LOWER];
    wllora.cmd.dioirq.irq2_mask[0]  = ((ot_u8*)&irq2_mask)[UPPER];
    wllora.cmd.dioirq.irq2_mask[1]  = ((ot_u8*)&irq2_mask)[LOWER];
    wllora.cmd.dioirq.irq3_mask[0]  = ((ot_u8*)&irq3_mask)[UPPER];
    wllora.cmd.dioirq.irq3_mask[1]  = ((ot_u8*)&irq3_mask)[LOWER];
    wllora_spibus_io(9, 0, wllora.cmd.raw);
}

ot_u16 wllora_getirq_cmd(void) {
    return sub_get1x2_cmd(0x12);
//    ot_u32 temp;
//    wllora.cmd.getirq.opcode = 0x12;
//    wllora_spibus_io(1, 3, wllora.cmd.raw);
//
//    temp = *(ot_u16*)wllora.cmd.getirq.irq_status;
//    return (ot_u16)__REV16(temp);
}

void wllora_calibrate_cmd(ot_u8 calib_cfg) {
    sub_set1x1_cmd(0x89, calib_cfg);
//    wllora.cmd.calibrate.opcode     = 0x89;
//    wllora.cmd.calibrate.calib_cfg  = calib_cfg;
//    wllora_spibus_io(2, 0, wllora.cmd.raw);
}

void wllora_calimage_cmd(ot_u8 calfreq1, ot_u8 calfreq2) {
    sub_set2x1_cmd(0x98, calfreq1, calfreq2);
//    wllora.cmd.calimage.opcode      = 0x98;
//    wllora.cmd.calimage.calfreq1    = calfreq1;
//    wllora.cmd.calimage.calfreq2    = calfreq2;
//    wllora_spibus_io(3, 0, wllora.cmd.raw);
}

void wllora_regmode_cmd(ot_u8 reg_mode) {
    sub_set1x1_cmd(0x96, reg_mode);
//    wllora.cmd.regmode.opcode   = 0x96;
//    wllora.cmd.regmode.reg_mode = calib_cfg;
//    wllora_spibus_io(2, 0, wllora.cmd.raw);
}

ot_u16 wllora_geterr_cmd(void) {
    return sub_get1x2_cmd(0x17);
//    ot_u32 temp;
//    wllora.cmd.geterr.opcode = 0x17;
//    wllora_spibus_io(1, 3, wllora.cmd.raw);
//
//    temp = *(ot_u16*)wllora.cmd.geterr.op_error;
//    return (ot_u16)__REV16(temp);
}

void wllora_clrerr_cmd(void) {
    sub_set1x1_cmd(0x07, 0);
//    wllora.cmd.clrerr.opcode    = 0x07;
//    wllora.cmd.clrerr.zero      = 0;
//    wllora_spibus_io(2, 0, wllora.cmd.raw);
}


void wllora_tcxomode_cmd(ot_u8 reg_txco_trim, ot_u32 timeout) {
    wllora.cmd.tcxomode.opcode          = 0x97;
    wllora.cmd.tcxomode.reg_txco_trim   = reg_txco_trim;
    wllora.cmd.tcxomode.timeout[0]      = ((ot_u8*)&timeout)[B2];
    wllora.cmd.tcxomode.timeout[1]      = ((ot_u8*)&timeout)[B1];
    wllora.cmd.tcxomode.timeout[2]      = ((ot_u8*)&timeout)[B0];
    wllora_spibus_io(5, 0, wllora.cmd.raw);
}




/** High-Level Pin Interrupt Management Functions <BR>
  * ========================================================================<BR>
  * These functions manage the interrupt control and states.
  *
  * For STM32WL LoRa, it is possible to configure PB3, PB5, and PB8 as outputs
  * representing Radio DIO1,2,3 (sometimes also written as DIO0,1,2).  In this
  * configuration, it's possible to get direct interrupts by setting the EXTI
  * FTSR or RTSR on each pin, even though they are outputs.  But that method is
  * not implemented in the present driver.
  */

void platform_isr_rfirq() {
///@todo read the IRQ bytes and vector accordingly

    // This is placeholder code only
    wllora_virtual_isr(wllora.imode);
}

OT_WEAK void wllora_int_off() {
    PWR->C2CR3 &= ~PWR_C2CR3_EWRFIRQ;
}

OT_WEAK void wllora_int_on() {
    PWR->C2CR3 |= PWR_C2CR3_EWRFIRQ;
}


inline void wllora_iocfg_cad()  {
    wllora_int_clearall();
    wllora_write(RFREG_LR_DIOMAPPING1, _DIOMAPPING1_CAD);
}

inline void wllora_iocfg_rx()  {
    wllora_int_clearall();
    wllora_write(RFREG_LR_DIOMAPPING1, _DIOMAPPING1_RX);
}

inline void wllora_iocfg_tx()  {
    wllora_int_clearall();
    wllora_write(RFREG_LR_DIOMAPPING1, _DIOMAPPING1_TX);
}

inline void wllora_int_listen() {
    wllora.imode = MODE_Listen;
    wllora_int_config(RFI_LISTEN);
}

inline void wllora_int_rxdata() {
    wllora.imode = MODE_RXData;
    wllora_int_config(RFI_RXDATA);
}

inline void wllora_int_rxend() {
    wllora.imode = MODE_RXData;
    wllora_int_config(RFI_RXEND);
}

inline void wllora_int_csma() {
    wllora.imode = MODE_CSMA;
    wllora_int_config(RFI_CSMA);
}

inline void wllora_int_txdata() {
    wllora.imode = MODE_TXData;
    wllora_int_config(RFI_TXDONE);
}

// Not used with WL
//void wllora_irq0_isr() {   wllora_virtual_isr(wllora.imode);     }
//void wllora_irq1_isr() {   wllora_virtual_isr(wllora.imode + 1); }
//void wllora_irq2_isr() {   wllora_virtual_isr(wllora.imode + 2); }
//void wllora_irq3_isr() {   wllora_virtual_isr(wllora.imode + 3); }
//void wllora_irq4_isr() {   wllora_virtual_isr(wllora.imode + 4); }
//void wllora_irq5_isr() {   wllora_virtual_isr(wllora.imode + 5); }










/** Basic Control <BR>
  * ============================================================================
  */
  
void wllora_load_defaults() {
/// The data ordering is: WRITE LENGTH, WRITE HEADER (0), START ADDR, VALUES
/// Ignore registers that are set later, are unused, or use the hardware default values.
#   define __REGSET(NAME)  RFREG_##NAME, DRF_##NAME
    static const ot_u8 defaults[] = {
        // Regs 0x02-0x05 are used in FSK mode and may cause trouble with LoRa.  Set to 0x00.
        0x02, 0x00,
        0x03, 0x00,
        0x04, 0x00,
        0x05, 0x00,
        
        // LoRa Registers
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
//        __REGSET(LR_MODEMCONFIG3),
        __REGSET(LR_DETECTOPTIMIZE),
        __REGSET(LR_INVERTIQ),
        __REGSET(LR_DETECTIONTHRESHOLD),
        __REGSET(LR_SYNCWORD),
        __REGSET(LR_INVERTIQ2),
//        __REGSET(LR_AGCREF),
        __REGSET(LR_DIOMAPPING1),
        __REGSET(LR_DIOMAPPING2),
        0   //Terminating 0
    };

    ot_u8* cursor;
    
    // SX127x should be asleep when running this function
    //wllora_strobe(_OPMODE_SLEEP);
    
    ///@todo do a burst write
    cursor = (ot_u8*)defaults;
    while (*cursor != 0) {
        wllora_write(cursor[0], cursor[1]);
        cursor += 2;
    }
    
#   undef __REGSET
}

void wllora_corelog() {
///debugging function to dump-out register values of RF core (not all are used)
    ot_u8 i = 0x00;
    ot_u8 regval;
    ot_u8 label[]   = { 'R', 'E', 'G', '_', 0, 0 };

    do {
        regval = wllora_read(i);
        otutils_bin2hex(&label[4], &i, 1);
        logger_msg(MSG_raw, 6, 1, label, &regval);
        //mpipedrv_wait();
        delay_ti(5);
    }
    while (++i < 0x71);
}

void wllora_coredump(ot_u8* dst, ot_uint limit) {
///debugging function to dump-out register values of RF core (not all are used)
    ot_u8 i;

    if (limit > 0x70) {
        limit = 0x70;
    }

    *dst++ = 0; //FIFO register: it gets corrupted on reads

    for (i=0x01; i<limit; i++) {
        *dst++ = wllora_read(i);
    }
}



/** Control & Status Functions <BR>
  * ========================================================================<BR>
  * These functions utilize the pin-wrapper driver functions, which
  * must be implemented in the platform-specific driver. 
  */

ot_bool wllora_isready() {
	return True;
}

void wllora_waitfor_ready() {
}

ot_bool wllora_check_cadpin() {
    return True;
}

ot_u8 wllora_getstatus() {
/// Status is register IRQFLAGS
    wllora.status = wllora_read(RFREG_LR_IRQFLAGS);
    return wllora.status;
}

ot_u8 wllora_mode() {
    return wllora_read(RFREG_LR_OPMODE) & _OPMODE;
}

ot_u8 wllora_rxbytes()    { return wllora_read(RFREG_LR_RXNBBYTES); }
ot_u8 wllora_rssi()       { return wllora_read(RFREG_LR_RSSIVALUE); }
ot_u8 wllora_pktrssi()    { return wllora_read(RFREG_LR_PKTRSSIVALUE); }
ot_s8 wllora_pktsnr()     { return wllora_read(RFREG_LR_PKTSNRVALUE); }





/** High-Level Read, Write, and Load-Defaults Functions <BR>
  * ========================================================================<BR>
  * These utilize the driver function: wllora_spibus_io()
  * This function must be implemented specific to the platform.
  */

ot_u8 wllora_rdreg(ot_uni16 addr) {
    ot_u8 cmd[3] = { 0x1D, addr.ubyte[UPPER], addr.ubyte[LOWER] };
    wllora_spibus_io(3, 2, (const ot_u8*)cmd);

    return wllora.buf.rd.data[0];
}

void wllora_wrreg(ot_u16 addr, ot_u8 val) {
    ot_u8 cmd[4] = { 0x0D, addr.ubyte[UPPER], addr.ubyte[LOWER], val };
    wllora_spibus_io(4, 0, (const ot_u8*)cmd);
}

void wllora_mode_sleep(ot_bool blocking) {

}

void wllora_mode_standby(ot_bool blocking) {
}

void wllora_mode_fs(ot_bool blocking) {

}

void wllora_mode_tx(ot_bool blocking) {

}

void wllora_mode_rx(ot_bool blocking) {

}



void wllora_strobe(ot_u8 new_mode, ot_bool blocking) {
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
        wllora_write(RFREG_LR_OPMODE, _LORAMODE|new_mode);
    }
    else {
        ot_u8 old_mode = wllora_mode();
        
        if (old_mode != new_mode) {
            ot_uint wdog;
            
            wllora_write(RFREG_LR_OPMODE, _LORAMODE|new_mode);
            wdog = wdog_amount[(new_mode<<1) + (old_mode!=0)];
            
            do {
                if (--wdog == 0) {
                    wllora_reset();
                    delay_us(400);
                    dll_init();
                    return;
                }
                old_mode = wllora_mode();
                
            } while (old_mode != new_mode);
        }
    }
}

ot_u8 wllora_read(ot_u8 addr) {
    return faux_regs[addr];
}

void wllora_burstread(ot_u8 start_addr, ot_u8 length, ot_u8* data) {
    if (start_addr == RFREG_LR_FIFO) {
        memset(data, 0, length);
    }
    else {
        memcpy(data, &faux_regs[start_addr], length);
    }
}

void wllora_write(ot_u8 addr, ot_u8 data) {
    faux_regs[addr] = data; 
}

void wllora_burstwrite(ot_u8 start_addr, ot_u8 length, ot_u8* cmd_data) {
    if (start_addr == RFREG_LR_FIFO) {
        // do nothing, no fifo
    }
    else {
        memcpy(&faux_regs[start_addr], cmd_data, length);
    }
}





/** Counter Management Functions <BR>
  * ========================================================================<BR>
  * Certain MAC processes require a running timer.  Instead of using any
  * internal timers of WL SUBGHZ, we instead use the more reliable chronstamp
  * feature of OpenTag.
  */
static ot_u32 macstamp;

void wllora_start_counter() {
    macstamp = systim_chronstamp(NULL);
}

void wllora_stop_counter() {
}

ot_u16 wllora_get_counter() {
    ot_u16 value;
    value = dll.counter - (ot_u16)systim_chronstamp(&macstamp);
    return value;
}






/** Advanced Configuration <BR>
  * ========================================================================<BR>
  */
ot_int wllora_calc_rssi(ot_u8 encoded_value, ot_s8 packet_snr) {
    ot_int rssi;
    
    /// SX1276 RSSI calculation process
    rssi = (ot_int)encoded_value;
    if (packet_snr < 0) {
        rssi += ((ot_int)packet_snr - 2) / 4;
    }
    else {
        rssi = (rssi * 16) / 15;
    }

    rssi += -157;

    return rssi;
}


ot_u8 wllora_calc_rssithr(ot_u8 input) {
    ot_int rssi_thr;
    
/// SX127x treats RSSI thresholding through the normal RSSI engine.  The specs
/// are the same as those used in wllora_calc_rssi() above, but the process is
/// using different input and output.
///
/// Input is a whole-dBm value encoded linearly as: {0=-140dBm, 127=-13dBm}.
/// Output is the value that should go into RSSI_TH field.
#   if defined(__SX1272__) || defined(__SX1273__)
    // SX1272/3 uses -125 as baseline, DASH7 -140
    // Clip baseline at 0
    ///@todo SATURATION INSTRUCTION: 0 minimum
    rssi_thr = (ot_int)input - (140-125);
    if (rssi_thr < 0)
        rssi_thr = 0;

#   elif defined(__SX1276__) || defined(__SX1277__) || defined(__SX1278__) || defined(__SX1279__)
#   if (RF_PARAM_BAND < 750)
        rssi_thr = (164 - 140) + input;
#   else
        rssi_thr = (157 - 140) + input;
#   endif
#   endif
        
    return (ot_u8)rssi_thr;
}


ot_u8 wllora_clip_txeirp(ot_u8 input_eirp) {
/// This considers Normal-Mode.  In TX Boost mode, 13dBm --> 20dBm
#   define _MAX_DBM_EIRP (((20*2) - RF_HDB_ATTEN) + 80)
#   define _MIN_DBM_EIRP (((5*2) - RF_HDB_ATTEN) + 80)

    if (input_eirp > _MAX_DBM_EIRP) {
        input_eirp = _MAX_DBM_EIRP;
    }
    else if (input_eirp < _MIN_DBM_EIRP) {
        input_eirp = _MIN_DBM_EIRP;
    }
    
    return input_eirp;
    
#   undef _MAX_DBM_EIRP
#   undef _MIN_DBM_EIRP
}


void wllora_set_txpwr(ot_u8 pwr_code) {
/// Sets the tx output power (non boost)
/// "pwr_code" is a value, 0-127, that is: eirp_code/2 - 40 = TX dBm
/// i.e. eirp_code=0 => -40 dBm, eirp_code=80 => 0 dBm, etc
    ot_int dBm;
    ot_u8 padac;
    
    // get dBm
    dBm = (((ot_int)pwr_code + RF_HDB_ATTEN) >> 1) - 40;

    // Convert to SX1276/8/9 units (PA Boost on, max 20dBm)
    if (dBm > 17) {
        padac   = _PADAC_20DBM_ON;
        dBm    -= 5;
    }
    else {
        padac   = _PADAC_20DBM_OFF;
        dBm    -= 2;
    }
    wllora_write(RFREG_LR_PADAC, padac);
    pwr_code = (1<<7) | (7<<4) | ((ot_u8)dBm & 0x0F);

    // Write new PA Table to device
    wllora_write(RFREG_LR_PACONFIG, pwr_code);
}







#endif


