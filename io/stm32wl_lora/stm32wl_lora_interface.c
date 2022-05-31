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
  * This file is the generic interface to the stm32wl radio inter
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
wllora_io_t wllora;
wllora_ext_t wllora_ext;




/** Generic Command Functions from SX126x & STM32WL manuals
  * For specific networking stacks, it is often better to use custom-prepared
  * versions of these command calls (with fewer arguments) rather than the
  * generic implementations below.
  */

static void sub_set0x1_cmd(const ot_u8 opcode) {
    wllora.cmd.buf.tx[0]   = opcode;
    wllora_spibus_io(1, 0, wllora.cmd.buf.tx);
}

static void sub_set1x1_cmd(const ot_u8 opcode, const ot_u8 val) {
    wllora.cmd.buf.tx[0]   = opcode;
    wllora.cmd.buf.tx[1]   = val;
    wllora_spibus_io(2, 0, wllora.cmd.buf.tx);
}

static void sub_set2x1_cmd(const ot_u8 opcode, const ot_u8 val1, const ot_u8 val2) {
    wllora.cmd.buf.tx[0]   = opcode;
    wllora.cmd.buf.tx[1]   = val1;
    wllora.cmd.buf.tx[2]   = val2;
    wllora_spibus_io(3, 0, wllora.cmd.buf.tx);
}

static void sub_set4x1_cmd(const ot_u8 opcode, const ot_u8 val1, const ot_u8 val2, const ot_u8 val3, const ot_u8 val4) {
    wllora.cmd.buf.tx[0]   = opcode;
    wllora.cmd.buf.tx[1]   = val1;
    wllora.cmd.buf.tx[2]   = val2;
    wllora.cmd.buf.tx[3]   = val3;
    wllora.cmd.buf.tx[4]   = val4;
    wllora_spibus_io(5, 0, wllora.cmd.buf.tx);
}

static void sub_set1x3_cmd(const ot_u8 opcode, const ot_u32 val) {
    wllora.cmd.buf.tx[0]   = opcode;
    wllora.cmd.buf.tx[1]   = ((ot_u8*)&val)[B2];
    wllora.cmd.buf.tx[2]   = ((ot_u8*)&val)[B1];
    wllora.cmd.buf.tx[3]   = ((ot_u8*)&val)[B0];
    wllora_spibus_io(4, 0, wllora.cmd.buf.tx);
}

static ot_u8 sub_get_1x1(const ot_u8 opcode) {
    wllora.cmd.buf.tx[0] = opcode;
    wllora_spibus_io(1, 2, wllora.cmd.buf.tx);
    return wllora.cmd.buf.rx[2];
}

static ot_u16 sub_get1x2_cmd(const ot_u8 opcode) {
    ot_u32 temp;
    wllora.cmd.buf.tx[0] = opcode;
    wllora_spibus_io(1, 3, wllora.cmd.buf.tx);

    temp = *(ot_u16*)&wllora.cmd.buf.rx[2];
    return (ot_u16)__REV16(temp);
}




void wllora_wrreg(lr_regs_e reg, const ot_u8 value) {
    lr_addr_u tmp = { .name = reg };
    //tmp.name = reg;
    wllora_wrreg_cmd(tmp, value);
}

void wllora_wrburst(lr_regs_e reg, const ot_u8 len, const ot_u8* data) {
    lr_addr_u tmp = { .name = reg };
    //tmp.name = reg;
    wllora_wrburst_cmd(tmp, len, data);
}

ot_u8 wllora_rdreg(lr_regs_e reg) {
    lr_addr_u tmp = { .name = reg };
    //tmp.name = reg;
    return wllora_rdreg_cmd(tmp);
}

void wllora_rdburst(lr_regs_e reg, const ot_u8 len, ot_u8* data) {
    lr_addr_u tmp = { .name = reg };
    //tmp.name = reg;
    wllora_rdburst_cmd(tmp, len, data);
}



void wllora_wrreg_cmd(lr_addr_u addr, const ot_u8 value) {
    wllora.cmd.wrreg.opcode     = 0x0D;
    wllora.cmd.wrreg.addr[0]    = addr.u8[0];
    wllora.cmd.wrreg.addr[1]    = addr.u8[1];
    wllora.cmd.wrreg.data[0]    = value;
    wllora_spibus_io(4, 0, wllora.cmd.buf.tx);
}

void wllora_wrburst_cmd(lr_addr_u addr, const ot_u8 len, const ot_u8* data) {
    wllora.cmd.wrreg.opcode     = 0x0D;
    wllora.cmd.wrreg.addr[0]    = addr.u8[0];
    wllora.cmd.wrreg.addr[1]    = addr.u8[1];

    if (len <= WLLORA_WRMAX) {
        memcpy(wllora.cmd.wrreg.data, data, len);
        wllora_spibus_io(3+len, 0, wllora.cmd.buf.tx);
    }
}

ot_u8 wllora_rdreg_cmd(lr_addr_u addr) {
    wllora.cmd.rdreg.opcode     = 0x1D;
    wllora.cmd.rdreg.addr[0]    = addr.u8[0];
    wllora.cmd.rdreg.addr[1]    = addr.u8[1];
    wllora_spibus_io(3, 2, wllora.cmd.buf.tx);

    return wllora.cmd.rdreg.data[0];
}

void wllora_rdburst_cmd(lr_addr_u addr, const ot_u8 len, ot_u8* data) {
    wllora.cmd.rdreg.opcode     = 0x1D;
    wllora.cmd.rdreg.addr[0]    = addr.u8[0];
    wllora.cmd.rdreg.addr[1]    = addr.u8[1];

    if (len <= WLLORA_RDMAX) {
        wllora_spibus_io(3, 1+len, wllora.cmd.buf.tx);
        memcpy(data, wllora.cmd.rdreg.data, len);
    }
}

void wllora_wrbuf_cmd(const ot_u8 offset, const ot_u8 len, const ot_u8* data) {
    wllora.cmd.wrbuf.opcode     = 0x0E;
    wllora.cmd.wrbuf.offset     = offset;

    if (len <= WLLORA_WRBUFMAX) {
        memcpy(wllora.cmd.wrbuf.data, data, len);
        wllora_spibus_io(2+len, 0, wllora.cmd.buf.tx);
    }
}

void wllora_rdbuf_cmd(const ot_u8 offset, const ot_u8 len, ot_u8* data) {
    wllora.cmd.rdbuf.opcode     = 0x1E;
    wllora.cmd.rdbuf.offset     = offset;

    if (len <= WLLORA_WRBUFMAX) {
        wllora_spibus_io(3+len, 0, wllora.cmd.buf.tx);
        memcpy(data, wllora.cmd.rdbuf.data, len);
    }
}

void wllora_sleep_cmd(const ot_u8 sleep_cfg) {
    sub_set1x1_cmd(0x84, sleep_cfg);
}

void wllora_standby_cmd(const ot_u8 standby_cfg) {
    sub_set1x1_cmd(0x80, standby_cfg);
}

void wllora_fs_cmd(void) {
    sub_set0x1_cmd(0xC1);
}

void wllora_tx_cmd(const ot_u32 timeout) {
    sub_set1x3_cmd(0x83, timeout);
}

void wllora_rx_cmd(const ot_u32 timeout) {
    sub_set1x3_cmd(0x82, timeout);
}

void wllora_stoprxtim_cmd(const ot_u8 rx_timeout_stop) {
    sub_set1x1_cmd(0x9F, rx_timeout_stop);
}

//2x3
void wllora_rxduty_cmd(const ot_u32 rx_period, const ot_u32 sleep_period) {
    wllora.cmd.rxduty.opcode            = 0x94;
    wllora.cmd.rxduty.rx_period[0]      = ((ot_u8*)&rx_period)[B2];
    wllora.cmd.rxduty.rx_period[1]      = ((ot_u8*)&rx_period)[B1];
    wllora.cmd.rxduty.rx_period[2]      = ((ot_u8*)&rx_period)[B0];
    wllora.cmd.rxduty.sleep_period[0]   = ((ot_u8*)&sleep_period)[B2];
    wllora.cmd.rxduty.sleep_period[1]   = ((ot_u8*)&sleep_period)[B1];
    wllora.cmd.rxduty.sleep_period[2]   = ((ot_u8*)&sleep_period)[B0];
    wllora_spibus_io(7, 0, wllora.cmd.buf.tx);
}

void wllora_cad_cmd(void) {
    sub_set0x1_cmd(0xC5);
}

void wllora_txcontwave_cmd(void) {
    sub_set0x1_cmd(0xD1);
}

void wllora_txcontpreamble_cmd(void) {
    sub_set0x1_cmd(0xD2);
}

void wllora_setpkttype_cmd(const ot_u8 pkt_type) {
    sub_set1x1_cmd(0x8A, pkt_type);
}

ot_u8 wllora_getpkttype_cmd(void) {
    return sub_get_1x1(0x11);
}

void wllora_rffreq_cmd(const ot_u32 freq) {
    wllora.cmd.rffreq.opcode    = 0x86;
    wllora.cmd.rffreq.freq[0]   = ((ot_u8*)&freq)[B3];
    wllora.cmd.rffreq.freq[1]   = ((ot_u8*)&freq)[B2];
    wllora.cmd.rffreq.freq[2]   = ((ot_u8*)&freq)[B1];
    wllora.cmd.rffreq.freq[3]   = ((ot_u8*)&freq)[B0];
    wllora_spibus_io(5, 0, wllora.cmd.buf.tx);
}

void wllora_txparams_cmd(const ot_u8 power, const ot_u8 ramp_time) {
    sub_set2x1_cmd(0x8E, power, ramp_time);
}

void wllora_paconfig_cmd(const ot_u8 pa_duty, const ot_u8 hp_max, const ot_u8 pa_sel) {
    sub_set4x1_cmd(0x95, pa_duty, hp_max, pa_sel, 1);
}

void wllora_txrxfallback_cmd(const ot_u8 fallback_mode) {
    sub_set1x1_cmd(0x93, fallback_mode);
}


void wllora_cadparams_cmd(const ot_u8 num_symbol, const ot_u8 det_peak,
            const ot_u8 det_min, const ot_u8 exit_mode, const ot_u32 timeout) {
    wllora.cmd.cadparams.opcode     = 0x88;
    wllora.cmd.cadparams.num_symbol = num_symbol;
    wllora.cmd.cadparams.det_peak   = det_peak;
    wllora.cmd.cadparams.det_min    = det_min;
    wllora.cmd.cadparams.exit_mode  = exit_mode;
    wllora.cmd.cadparams.timeout[0] = ((ot_u8*)&timeout)[B2];
    wllora.cmd.cadparams.timeout[1] = ((ot_u8*)&timeout)[B1];
    wllora.cmd.cadparams.timeout[2] = ((ot_u8*)&timeout)[B0];
    wllora_spibus_io(8, 0, wllora.cmd.buf.tx);
}

void wllora_bufbase_cmd(const ot_u8 tx_base_addr, const ot_u8 rx_base_addr) {
    sub_set2x1_cmd(0x8F, tx_base_addr, rx_base_addr);
}

void wllora_modparams_cmd(const ot_u8 sf, const ot_u8 bw, const ot_u8 cr, const ot_u8 ldro) {
    sub_set4x1_cmd(0x8B, sf, bw, cr, ldro);
}


void wllora_pktparams_cmd(const ot_u16 preamble_len, const ot_u8 hdr_type,
        const ot_u8 payload_len, const ot_u8 crc_type, const ot_u8 invert_iq) {
    wllora.cmd.pktparams.opcode         = 0x8C;
    wllora.cmd.pktparams.preamble_len[0]= ((ot_u8*)&preamble_len)[UPPER];
    wllora.cmd.pktparams.preamble_len[1]= ((ot_u8*)&preamble_len)[LOWER];
    wllora.cmd.pktparams.hdr_type       = hdr_type;
    wllora.cmd.pktparams.payload_len    = payload_len;
    wllora.cmd.pktparams.crc_type       = crc_type;
    wllora.cmd.pktparams.invert_iq      = invert_iq;
    wllora_spibus_io(7, 0, wllora.cmd.buf.tx);
}

void wllora_symtimeout_cmd(const ot_u8 sym_num) {
    sub_set1x1_cmd(0xA0, sym_num);
}


ot_u8 wllora_status_cmd(void) {
    wllora.cmd.status.opcode = 0xC0;
    wllora_spibus_io(1, 1, wllora.cmd.buf.tx);
    return wllora.cmd.status.status;
}

lr_rxbufstatus_t wllora_rxbufstatus_cmd(void) {
///@todo make sure this return typecast doesn't crash on M0 core
    wllora.cmd.rxbufstatus.opcode = 0x13;
    wllora_spibus_io(1, 3, wllora.cmd.buf.tx);
    return *(lr_rxbufstatus_t*)&wllora.cmd.rxbufstatus.rx_payload_len;
}

lr_pktlink_t wllora_pktlink_cmd(void) {
///@todo make sure this return typecast doesn't crash on M0 core
    wllora.cmd.pktlink.opcode = 0x14;
    wllora_spibus_io(1, 4, wllora.cmd.buf.tx);
    return *(lr_pktlink_t*)&wllora.cmd.pktlink.rssi_pkt;
}

ot_u8 wllora_rssi_cmd(void) {
    return sub_get_1x1(0x15);
}

lr_pktstats_t wllora_pktstats_cmd(void) {
///@todo make sure this return typecast doesn't crash on M0 core
    wllora.cmd.pktstats.opcode = 0x10;
    wllora_spibus_io(1, 7, wllora.cmd.buf.tx);
    return *(lr_pktstats_t*)&wllora.cmd.pktstats.num_pkt_rxed;
}

void wllora_resetstats_cmd(void) {
    memset(wllora.cmd.buf.tx, 0, 7);
    wllora_spibus_io(7, 0, wllora.cmd.buf.tx);
}

void wllora_dioirq_cmd(const ot_u16 irq_mask, const ot_u16 irq1_mask,
                        const ot_u16 irq2_mask, const ot_u16 irq3_mask) {
    wllora.cmd.dioirq.opcode        = 0x08;
    wllora.cmd.dioirq.irq_mask[0]   = ((ot_u8*)&irq_mask)[UPPER];
    wllora.cmd.dioirq.irq_mask[1]   = ((ot_u8*)&irq_mask)[LOWER];
    wllora.cmd.dioirq.irq1_mask[0]  = ((ot_u8*)&irq1_mask)[UPPER];
    wllora.cmd.dioirq.irq1_mask[1]  = ((ot_u8*)&irq1_mask)[LOWER];
    wllora.cmd.dioirq.irq2_mask[0]  = ((ot_u8*)&irq2_mask)[UPPER];
    wllora.cmd.dioirq.irq2_mask[1]  = ((ot_u8*)&irq2_mask)[LOWER];
    wllora.cmd.dioirq.irq3_mask[0]  = ((ot_u8*)&irq3_mask)[UPPER];
    wllora.cmd.dioirq.irq3_mask[1]  = ((ot_u8*)&irq3_mask)[LOWER];
    wllora_spibus_io(9, 0, wllora.cmd.buf.tx);
}

ot_u16 wllora_getirq_cmd(void) {
    return sub_get1x2_cmd(0x12);
}

void wllora_clrirq_cmd(void) {
    sub_set2x1_cmd(0x02, 0xff, 0xff);
}

void wllora_calibrate_cmd(const ot_u8 calib_cfg) {
    sub_set1x1_cmd(0x89, calib_cfg);
}

void wllora_calimage_cmd(const ot_u8 calfreq1, const ot_u8 calfreq2) {
    sub_set2x1_cmd(0x98, calfreq1, calfreq2);
}

void wllora_regmode_cmd(const ot_u8 reg_mode) {
    sub_set1x1_cmd(0x96, reg_mode);
}

ot_u16 wllora_geterr_cmd(void) {
    return sub_get1x2_cmd(0x17);
}

void wllora_clrerr_cmd(void) {
    sub_set1x1_cmd(0x07, 0);
}


void wllora_tcxomode_cmd(const ot_u8 reg_txco_trim, const ot_u32 timeout) {
    wllora.cmd.tcxomode.opcode          = 0x97;
    wllora.cmd.tcxomode.reg_txco_trim   = reg_txco_trim;
    wllora.cmd.tcxomode.timeout[0]      = ((ot_u8*)&timeout)[B2];
    wllora.cmd.tcxomode.timeout[1]      = ((ot_u8*)&timeout)[B1];
    wllora.cmd.tcxomode.timeout[2]      = ((ot_u8*)&timeout)[B0];
    wllora_spibus_io(5, 0, wllora.cmd.buf.tx);
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
  *
  * The wllora_rfio...() functions modify the Global IRQ Mask or potentially
  * the DIO configuration.  Right now the impl uses Global IRQ.
  */

void wllora_rfio_cad()  {
//    wllora_clrirq_cmd();
//    wllora_dioirq_cmd(
//            LR_IRQ_CADDONE | LR_IRQ_CADDET,
//            0,0,0);
}

void wllora_rfio_rx()  {
//    wllora_clrirq_cmd();
//    wllora_dioirq_cmd(
//            LR_IRQ_HDRVALID | LR_IRQ_RXDONE | LR_IRQ_TIMEOUT,
//            0,0,0);
}

void wllora_rfio_tx()  {
//    wllora_clrirq_cmd();
//    wllora_dioirq_cmd(
//            LR_IRQ_TXDONE | LR_IRQ_TIMEOUT,
//            0,0,0);
}


static void sub_rfirq(WLLora_IMode mode, ot_u16 irqmask) {
    wllora.imode = mode;
    wllora_clrirq_cmd();
    wllora_dioirq_cmd(irqmask, irqmask, 0, 0);
    wllora_mcuirq_on();
}

inline void wllora_rfirq_listen() {
    sub_rfirq(MODE_Listen, LR_IRQ_CADDONE | LR_IRQ_CADDET);
}

inline void wllora_rfirq_rxdata() {
    sub_rfirq(MODE_RXData, LR_IRQ_HDRVALID);
}

inline void wllora_rfirq_rxend() {
    sub_rfirq(MODE_RXData, LR_IRQ_RXDONE);
}

inline void wllora_rfirq_cad() {
    sub_rfirq(MODE_CSMA, LR_IRQ_CADDONE | LR_IRQ_CADDET);
}

inline void wllora_rfirq_txdata() {
    sub_rfirq(MODE_TXData, LR_IRQ_TXDONE);
}










/** Basic Control <BR>
  * ============================================================================
  */
  
void wllora_load_defaults() {
/// Load startup/reset defaults into the radio core.
/// radio core must be in standby for this to work.

    /// 1. If SMPS is enabled on this board, set it to the default drive.
#   if BOARD_FEATURE(RFSMPS)
    wllora_ext.smps_setting = WLLORA_SMPS_DEFAULT;
    wllora_wrreg(LR_SMPSC2R, WLLORA_SMPS_DEFAULT);
    wllora_regmode_cmd(1);
#   else
    wllora_regmode_cmd(0);
#   endif

#   if BOARD_FEATURE(TCXO)
    wllora_tcxomode_cmd((LR_REG_TCXO_TRIM & 7), LR_REG_TCXO_TIMEOUT);
    wllora_wrreg(LR_HSEINTRIMR, 0);

#   else
    wllora_wrreg(LR_HSEINTRIMR, BOARD_PARAM(HSETRIM));
    wllora_wrreg(LR_HSEOUTTRIMR, BOARD_PARAM(HSETRIM));

#   endif

    /// 2. PA OCR setting cache
#   if defined(__STM32WL_22dBm__) && defined(__STM32WL_15dBm__)
    wllora_ext.ocr_setting = 0x18;
    wllora_wrreg(LR_PAOCPR, 0x18);
#   elif defined(__STM32WL_22dBm__)
    wllora_wrreg(LR_PAOCPR, 0x38);
#   else
    wllora_wrreg(LR_PAOCPR, 0x18);
#   endif

    /// 3. First command *must* be to set LoRa mode.
    wllora_setpkttype_cmd(0x01);

    /// 4. Set the image calibration settings
    ///@todo choose the right settings based on region.  Now 902-928 MHz.
    wllora_calimage_cmd(0xE1, 0xE9);

    wllora_coredump_uart1(0x000, 0xA00);

    /// 5. Perform a calibration
    wllora_calibrate_cmd(0x7F);
    delay_us(1);
    while(wllora_isbusy());

    ///@todo rest of implementation

}

void wllora_corelog() {
///debugging function to dump-out register values of RF core (not all are used)
    ot_u16 i =0;
    lr_addr_u addr;
    ot_u8 regval;
    ot_u8 label[]   = { 'R', 'E', 'G', '_', 0, 0 };

    do {
        addr.u8[0]  = ((ot_u8*)&i)[1];
        addr.u8[1]  = ((ot_u8*)&i)[0];
        regval      = wllora_rdreg_cmd(addr);

        otutils_bin2hex(&label[4], addr.u8, 2);
        logger_msg(MSG_raw, 6, 1, label, &regval);
        //mpipedrv_wait();
        delay_ti(5);
    }
    while (++i < 0x1000);
}

void wllora_coredump(ot_u8* dst, ot_uint limit) {
///@todo this function must be ported, it is a relic of the past right now.
///debugging function to dump-out register values of RF core (not all are used)
    ot_u8 i;

    if (limit > 0x70) {
        limit = 0x70;
    }

    *dst++ = 0; //FIFO register: it gets corrupted on reads

    for (i=0x01; i<limit; i++) {
        *dst++ = 0; ///@todo need to put this here
    }
}

#include <stdio.h>

void wllora_coredump_uart1(ot_u16 lreg_addr, ot_u16 hreg_addr) {
/// UART1 on PA9 (ARD:D9 / CN5:2 on the nucleo)
/// Brings up the interface and shuts it down afterward
/// On CPU1, would be easier to use SWO here

    ot_u8 val;
    lr_addr_u addr;
    char buf[16];
    char* cursor;

    // Clock and port enable
    GPIOA->BSRR     = (1<<9);
    GPIOA->MODER   &= ~(3 << (9*2));
    GPIOA->MODER   |= ~(GPIO_MODER_ALT << (9*2));
    GPIOA->AFR[1]  &= ~(15 << (1*4));
    GPIOA->AFR[1]  |= (7 << (1*4));
    RCC->C2APB2ENR |= RCC_C2APB2ENR_USART1EN;

    // UART Setup
    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;
    USART1->BRR = platform_get_clockhz(2) / 250000;
    USART1->CR1 = USART_CR1_TE | USART_CR1_UE;

    while (lreg_addr < hreg_addr) {
        addr.u8[0] = ((ot_u8*)&lreg_addr)[1];
        addr.u8[1] = ((ot_u8*)&lreg_addr)[0];

        val = wllora_rdreg_cmd(addr);
        sprintf(buf, "%04X, %02X;\n", lreg_addr, val);

        cursor = buf;
        while (*cursor != 0) {
            USART1->TDR = *cursor++;
            while ((USART1->ISR & USART_ISR_TXE_TXFNF) == 0);
            //USART1->ICR |= USART_ICR_TXFECF;
        }

        lreg_addr++;
    }

    while ((USART1->ISR & USART_ISR_TC) == 0);

    // Take down UART, clock, port
    MPIPE_UART->CR1 = 0;
    RCC->C2APB2ENR &= ~RCC_C2APB2ENR_USART1EN;
    GPIOA->MODER   |= (3 << (9*2));
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


ot_u8 wllora_calc_rssithr(ot_u8 input) {
/// Convert DASH7 formatted RSSI to WL LoRa format.  There isn't a built-in
/// RSSI Threshold format in WL LoRa.
    ot_int rssi;
    ot_u8 wl_rssi;

    rssi    = (ot_int)input - 140;
    wl_rssi = (ot_u8)(-rssi * 2);

    return wl_rssi;
}


ot_u8 wllora_clip_txeirp(ot_u8 input_eirp) {
///@todo This considers High Power Mode (22 dBm top end).  The board support
///      header should have constants/macros to specify what the board can do.

#   define _MAX_DBM_EIRP (((22*2) - RF_HDB_ATTEN) + 80)
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
///@todo This considers High Power Mode (22 dBm top end).  The board support
///      header should have constants/macros to specify what the board can do.

/// PA optimal settings given in the table below must be used to maximize the
/// PA efficiency for the maximum targeted output power. Matching network
/// determination must be done using these settings (see the application note
/// AN5457 for more details).
    static const ot_u8 pa_params[2][4] = {
        { 0x06, 0x00, 0x3, 0 },  // +15  (LP)
        { 0x04, 0x07, 0x2, 0 },  // +22  (HP)
    };
    ot_s8 dBm;
    const ot_u8* params;

/// Sets the tx output power
/// "pwr_code" is a value, 0-127, that is: eirp_code/2 - 40 = TX dBm
/// i.e. eirp_code=0 => -40 dBm, eirp_code=80 => 0 dBm, etc
    
    dBm                     = ((pwr_code + RF_HDB_ATTEN) / 2) - 40;
    wllora_ext.use_boost    = (dBm > 15);
    params                  = pa_params[(dBm > 15)];

    wllora_paconfig_cmd(params[0], params[1], params[2] & 1);

    ///@todo This uses hard-coded ramp time.
    ///      It will need to correspond to the channel type and regulations.
    wllora_txparams_cmd((ot_s8)(dBm + params[3]), 2);
}







#endif


