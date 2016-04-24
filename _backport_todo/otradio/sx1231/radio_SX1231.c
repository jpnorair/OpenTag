
/* Copyright 2010-2011
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
  * @file       /Platforms/SX1231/radio_SX1231.c
  * @author
  * @version    V1.0
  * @date       8 Feb 2012
  * @brief      Radio Driver (RF transceiver) for SX1231
  * @defgroup   Radio (Radio Module)
  * @ingroup    Radio
  *
  * The header file for this implementation is /OTlib/radio.h.  It is universal
  * for all platforms, even though the implementation (this file) can differ.
  *
  * For DASH7 Silicon certification, there are four basic tiers of HW features:
  * 1. PHY      The HW has a buffered I/O and the basic features necessary
  * 2. PHY+     The HW can do encoding, CRC, and some packet handling
  * 3. MAC      The HW can automate some inner loops, like Adv Flood and CSMA
  * 4. MAC+     The HW has most features of the MAC integrated
  *
  ******************************************************************************
  */

#include <otsys/types.h>
#include <otsys/config.h>
#include <otlib/utils.h>
#include <otplatform.h>

#include <m2/radio.h>
#include <m2/encode.h>
#include <otlib/crc16.h>
#include <otlib/buffers.h>
#include <otlib/queue.h>
#include <otsys/veelite.h>
#include <m2/session.h>

#ifdef RADIO_DEBUG
#   include "debug_uart.h"
#endif
#include "sx1231_registers.h"
#include "stm32_sx1231_private.h"

#define RXBW_25KHZ      (RF_RXBW_MANT_20 | RF_RXBW_EXP_4)
#define RXBW_31KHZ      (RF_RXBW_MANT_16 | RF_RXBW_EXP_4)
#define RXBW_42KHZ      (RF_RXBW_MANT_24 | RF_RXBW_EXP_3)
#define RXBW_50KHZ      (RF_RXBW_MANT_20 | RF_RXBW_EXP_3)
#define RXBW_62KHZ      (RF_RXBW_MANT_16 | RF_RXBW_EXP_3)
#define RXBW_83KHZ      (RF_RXBW_MANT_24 | RF_RXBW_EXP_2)
#define RXBW_100KHZ     (RF_RXBW_MANT_20 | RF_RXBW_EXP_2)
#define RXBW_125KHZ     (RF_RXBW_MANT_16 | RF_RXBW_EXP_2)
#define RXBW_167KHZ     (RF_RXBW_MANT_24 | RF_RXBW_EXP_1)
#define RXBW_200KHZ     (RF_RXBW_MANT_20 | RF_RXBW_EXP_1)

#define REG_BITRATEMSB_55555BPS     0x02
#define REG_BITRATELSB_55555BPS     0x40

#define REG_BITRATEMSB_200KBPS      0x00
#define REG_BITRATELSB_200KBPS      0xa0

#define FRFMSB_43392    0x6c
#define FRFMID_43392    0x7a
#define FRFLSB_43392    0xe1

#define RXTIM_PRESCALE_SHIFT    5   // 2^this = prescale, i.e 2^5 = 32 for 1024Hz from 32768Hz


#define NUM_FRF        15
static const ot_u16 frf[NUM_FRF] = {    // FRFMSB always 0x6c
    0x4a7f, //    0     433.164MHz    7096959
    0x5168, //    1     433.272MHz    7098728
    0x5852, //    2     433.380MHz    7100498
    0x5f3b, //    3     433.488MHz    7102267
    0x6625, //    4     433.596MHz    7104037
    0x6f5c, //    5     433.704MHz    7106396
    0x73f8, //    6     433.812MHz    7107576
    0x7ae1, //    7     433.920MHz    7109345
    0x81cb, //    8     434.028MHz    7111115
    0x88b4, //    9     434.136MHz    7112884
    0x8f9e, //    a     434.244MHz    7114654
    0x9687, //    b     434.352MHz    7116423
    0x9d71, //    c     434.460MHz    7118193
    0xa45a, //    d     434.568MHz    7119962
    0xab44  //    e     434.676MHz    7121732
};

radio_struct radio;

#define SIZEOF_RF_DATA_BUF        632
ot_u8 rf_data_buf[SIZEOF_RF_DATA_BUF];
int num_bytes_sent;
int num_bytes_to_send;

phymac_struct   phymac[M2_PARAM_MI_CHANNELS];

RegFifoThresh_t RegFifoThresh;
RegOpMode_t RegOpMode;
RegRxBw_t RegRxBw;
RegPaLevel_t RegPaLevel;
RegDataModul_t RegDataModul;
RegLna_t RegLna;

EXTI_InitTypeDef exti9_5_init;
EXTI_InitTypeDef exti4_init;

volatile ot_bool hold_tx_power = False;

static void
sub_set_txpower( ot_u8 eirp_code )
{
    /// Sets the tx output power.
    /// "eirp_code" is a value, 0-127, that is: eirp_code/2 - 40 = TX dBm
    /// i.e. eirp_code=0 => -40 dBm, eirp_code=80 => 0 dBm, etc

    /* SX1231 transmit can do -18dBm to +13dBm */
    eirp_code >>= 1;    // SX1231 tx-power capable of 1dB steps

    if (eirp_code < 22)
        eirp_code = 0;
    else
        eirp_code -= 22;

    if (eirp_code > 31)
        eirp_code = 31;

    RegPaLevel.bits.OutputPower = eirp_code;

    WriteReg_Sx1231(REG_PALEVEL, RegPaLevel.octet);
}

static void
sub_chan_config(ot_u8 old_chan, ot_u8 old_eirp)
{
    ot_u8 fc_i;
    ot_uni16 cur_frf;

    /// Center Frequency index = lower four bits channel ID
    fc_i = (phymac[0].channel & 0x0F);

#ifdef RADIO_DEBUG
    debug_printf("sub_chan_config(0x%x, %d) [34mfc_i=%d[0m\r\n", old_chan, old_eirp, fc_i);
#endif /* RADIO_DEBUG */
    /// Reprogram the PA Table if eirp of new channel isn't the same as before
    if (!hold_tx_power && (old_eirp != phymac[0].tx_eirp) ) {
        sub_set_txpower( phymac[0].tx_eirp );
    }

    if ( (old_chan ^ phymac[0].channel) & 0x70 ) {
#ifdef RADIO_DEBUG
        debug_printf("new bitrate %x\r\n", (phymac[0].channel >> 4) );
#endif /* RADIO_DEBUG */
        switch ((phymac[0].channel >> 4) & 0x03) {
            case 0: fc_i = 7;
            case 1:     /// 55 kS/s method
                WriteReg_Sx1231(REG_BITRATEMSB, REG_BITRATEMSB_55555BPS);
                WriteReg_Sx1231(REG_BITRATELSB, REG_BITRATELSB_55555BPS);
                // slower bitrate, slightly less occupied bandwidth
                RegRxBw.bits.RxBw = RXBW_83KHZ;
                break;
            case 2:
            case 3:     /// 200 kS/s method
                WriteReg_Sx1231(REG_BITRATEMSB, REG_BITRATEMSB_200KBPS);
                WriteReg_Sx1231(REG_BITRATELSB, REG_BITRATELSB_200KBPS);
                // faster bitrate, slightly more occupied bandwidth
                RegRxBw.bits.RxBw = RXBW_100KHZ;
                break;
        } // ...switch ((phymac[0].channel >> 4) & 0x03)
        WriteReg_Sx1231(REG_RXBW, RegRxBw.octet);
    }

    if ( fc_i != (old_chan & 0x0F) ) {
        fc_i &= 0xf;
        cur_frf.ushort = frf[fc_i];
#ifdef RADIO_DEBUG
        debug_printf("new freq 0x%x: %04x %02x %02x\r\n", fc_i, cur_frf.ushort, cur_frf.ubyte[UPPER], cur_frf.ubyte[LOWER]);
#endif /* RADIO_DEBUG */
        WriteReg_Sx1231(REG_FRFMID, cur_frf.ubyte[UPPER]);
        WriteReg_Sx1231(REG_FRFLSB, cur_frf.ubyte[LOWER]);
    }

}

static ot_bool
sub_channel_lookup(ot_u8 chan_id, vlFILE* fp)
{
/// Called during channel scans.
/// Duty: (a) See if the supplied channel is supported on this device & config.
///       If yes, return true.  (b) Determine if recalibration is required
///       before changing to the new channel, and recalibrate if so.

    ot_u8       fec_id;
    ot_u8       spectrum_id;
    ot_int      i;
    ot_uni16    scratch;

    //debug_printf("sub_channel_lookup(%x)", chan_id);
    /// Only do the channel lookup if the new channel is different than before
    if (chan_id == phymac[0].channel) {
        //debug_printf("sub_channel_lookup(%x) same\r\n", chan_id);
        return True;
    }
#ifdef RADIO_DEBUG
    debug_printf("sub_channel_lookup(%x) ", chan_id);
#endif /* RADIO_DEBUG */

    /// pull spectrum id and encoding type out of chan_id
    fec_id      = chan_id & 0x80;
    spectrum_id = chan_id & ~0x80;


#if (0)
    /// Upper layer debugging, to make sure spectrum id is in scope
    if ((spectrum_id != RM2_CHAN_BASE) && \
        (spectrum_id != RM2_CHAN_LEGACY) && \
        (spectrum_id != RM2_CHAN_NORMAL_0) && \
        (spectrum_id != RM2_CHAN_NORMAL_2) && \
        (spectrum_id != RM2_CHAN_NORMAL_4) && \
        (spectrum_id != RM2_CHAN_NORMAL_6) && \
        (spectrum_id != RM2_CHAN_NORMAL_8) && \
        (spectrum_id != RM2_CHAN_NORMAL_A) && \
        (spectrum_id != RM2_CHAN_NORMAL_C) && \
        (spectrum_id != RM2_CHAN_NORMAL_E) && \
        (spectrum_id != RM2_CHAN_TURBO_1) && \
        (spectrum_id != RM2_CHAN_TURBO_3) && \
        (spectrum_id != RM2_CHAN_TURBO_5) && \
        (spectrum_id != RM2_CHAN_TURBO_7) && \
        (spectrum_id != RM2_CHAN_TURBO_9) && \
        (spectrum_id != RM2_CHAN_TURBO_B) && \
        (spectrum_id != RM2_CHAN_TURBO_D) && \
        (spectrum_id != RM2_CHAN_BLINK_2) && \
        (spectrum_id != RM2_CHAN_BLINK_C) && \
        (spectrum_id != RM2_CHAN_WILDCARD)) {

        /// @note good to put a break here, or a trap
        return False;
    }
#endif

    /// If FEC is requested by the new channel, but this device does not support
    /// FEC, then make sure to return False.
    if (fec_id) {
#ifdef RADIO_DEBUG
        debug_printf("fec_id ");
#endif /* RADIO_DEBUG */
#       if (M2_FEATURE(FEC) == ENABLED)
            i = 0;

#           if (M2_FEATURE(FECRX) == ENABLED)
                i   = (radio.state & RADIO_STATE_RXMASK);
#           endif
#           if (M2_FEATURE(FECTX) == ENABLED)
                i  |= (radio.state & RADIO_STATE_TXMASK);
#           endif

            if (i == 0) {
                return False;
            }
#       else
#ifdef RADIO_DEBUG
            debug_printf(" no-fec ");
#endif /* RADIO_DEBUG */
            return False;
#       endif
    }

    /// 0x7F is the wildcard spectrum id.  It means use same spectrum as before.
    /// In this case, of course no recalibration is necessary.
    if (spectrum_id == 0x7F) {
#ifdef RADIO_DEBUG
        debug_printf("wildcard spectrum ID\r\n");
#endif /* RADIO_DEBUG */
        return True;
    }
#ifdef RADIO_DEBUG
    debug_printf("\r\n");
#endif /* RADIO_DEBUG */

    /// Look through the channel list to find the one with matching spectrum id.
    /// The channel list is not necessarily sorted.
#   if (M2_FEATURE(AUTOSCALE) != ENABLED)
#       define AUTOSCALE_MASK(VAL)      ((VAL) & 0x7F)
#   else
#       define AUTOSCALE_MASK(VAL)      (VAL)
#   endif

    for (i=0; i<fp->length; i+=8) {
        scratch.ushort = vl_read(fp, i);

        if (spectrum_id == scratch.ubyte[0]) {
            ot_u8 old_chan_id   = phymac[0].channel;
            ot_u8 old_tx_eirp   = phymac[0].tx_eirp;

            phymac[0].tg        = rm2_default_tgd(chan_id);
            phymac[0].channel   = chan_id;
            phymac[0].autoscale = scratch.ubyte[1];

            scratch.ushort      = vl_read(fp, i+2);
            phymac[0].tx_eirp   = AUTOSCALE_MASK(scratch.ubyte[0]);
            phymac[0].link_qual = AUTOSCALE_MASK(scratch.ubyte[1]);

            scratch.ushort      = vl_read(fp, i+4);
            phymac[0].cs_thr    = AUTOSCALE_MASK(scratch.ubyte[0]);
            phymac[0].cca_thr   = AUTOSCALE_MASK(scratch.ubyte[1]);
#ifdef RADIO_DEBUG
            debug_printf("sid %2x: cca=%d cs=%d\r\n", spectrum_id, phymac[0].cca_thr, phymac[0].cs_thr);
#endif /* RADIO_DEBUG */
            /* 0 = -140dBm, 140=0dBm, 60=80dBm */

            /// value 0-127 that is: input - 140 = threshold in dBm
            // todo: phymac[0].cs_thr
            /* CS RSSI threshold:   for Esm. scan minimum energy is minimum signal strength required to receive something.
             * CCA RSSI threshold:  for Ecca, clear channel assessment. weaker than this means channel vacant
             */

#ifdef RADIO_DEBUG
            debug_printf("phymac[0].channel=0x%x\r\n", phymac[0].channel);
#endif /* RADIO_DEBUG */
            sub_chan_config(old_chan_id, old_tx_eirp);
            return True;
        }
    }

#ifdef RADIO_DEBUG
    debug_printf("spectrum_id %x not found ", spectrum_id);
#endif /* RADIO_DEBUG */
    return False;
}

ot_uni16 sync_value; // global for access from transmitter

static void
sub_syncword_config(ot_u8 sync_class)
{

#   if (M2_FEATURE(FEC) == ENABLED)
    if (phymac[0].channel & 0x80) {   //Actual= 0xF498 : 0x192F;
        sync_value.ushort = (sync_class == 0) ? 0xF498 : 0x192F;
    }
    else
#   endif
    {                                 //Actual= 0xE6D0 : 0x0B67;
        //sync_value.ushort = (sync_class == 0) ? 0xE6D0 : 0x0B67;
        if (sync_class == 0) {
            if (sync_value.ushort == 0xe6d0)
                return;
            sync_value.ushort = 0xe6d0;
        } else {
            if (sync_value.ushort == 0x0b67)
                return;
            sync_value.ushort = 0x0b67;
        }
    }

    /* sync_value is transmitted in start_tx() */

    /* sync byte order is for CC430 compatability:
     * for example, where 0x0b67 is used, 0x67 is transmitted first in time, then 0x0b.
     * And MSbit (of each byte) is sent first.
     * @todo: validate this against Dash7 specification */
#ifdef RADIO_DEBUG
    debug_printf("sync %02x %02x\r\n", sync_value.ubyte[LOWER], sync_value.ubyte[UPPER]);
#endif /* RADIO_DEBUG */
    // SyncValue1 is last byte of preamble
    WriteReg_Sx1231(REG_SYNCVALUE2, sync_value.ubyte[LOWER]);
    WriteReg_Sx1231(REG_SYNCVALUE3, sync_value.ubyte[UPPER]);
}

void
set_chip_mode(ot_u8 chip_mode, char nonblocking)
{
    RegOpMode.bits.Mode = chip_mode;

#ifdef RADIO_DEBUG
/*    if (!nonblocking) {
        debug_printf("chip_mode -> ");
        switch (RegOpMode.bits.Mode) {
            case CHIP_MODE_SLEEP:       debug_printf("sleep"); break;
            case CHIP_MODE_STANDBY:     debug_printf("stby"); break;
            case CHIP_MODE_SYNTHESIZER: debug_printf("fs"); break;
            case CHIP_MODE_RECEIVER:    debug_printf("receive"); break;
            case CHIP_MODE_TRANSMITTER: debug_printf("Tx"); break;
        } // ...switch (mcparam1.bits.chip_mode)
        debug_printf("\r\n");
    }*/
#endif /* RADIO_DEBUG */

    if (nonblocking)
        WriteReg_Sx1231__nonblocking(REG_OPMODE, RegOpMode.octet);
    else
        WriteReg_Sx1231(REG_OPMODE, RegOpMode.octet);


    if (RegOpMode.bits.Mode == CHIP_MODE_TRANSMITTER) {
        RED_LED_ON;
        GREEN_LED_OFF;
    } else if (RegOpMode.bits.Mode == CHIP_MODE_RECEIVER) {
        RED_LED_OFF;
        GREEN_LED_ON;
    } else {
        RED_LED_OFF;
        GREEN_LED_OFF;
    }
}

void
update_shadow_regs(ot_u8 addr, ot_u8 val)
{
    switch (addr) {
        case REG_OPMODE:
            RegOpMode.octet = val;
            break;
        case REG_RXBW:
            RegRxBw.octet = val;
            break;
        case REG_DATAMODUL:
            RegDataModul.octet = val;
            break;
        case REG_PALEVEL:
            RegPaLevel.octet = val;
            break;
        case REG_LNA:
            RegLna.octet = val;
            break;
        case REG_FIFOTHRESH:
            RegFifoThresh.octet = val;
            break;
    } // ...switch (addr)
}

void
radio_init(void)
{
    static const ot_u8 defaults[][2] = {
        { REG_OPMODE, RF_OPMODE_SEQUENCER_OFF | RF_OPMODE_STANDBY },
        { REG_DATAMODUL, RF_DATAMODUL_DATAMODE_PACKET| RF_DATAMODUL_MODULATIONTYPE_FSK | RF_DATAMODUL_MODULATIONSHAPING_01 },   // BT = 1.0
        { REG_BITRATEMSB, REG_BITRATEMSB_55555BPS },
        { REG_BITRATELSB, REG_BITRATELSB_55555BPS },
        { REG_FDEVMSB, RF_FDEVMSB_50000 },
        { REG_FDEVLSB, RF_FDEVLSB_50000 },
        { REG_FRFMSB, FRFMSB_43392 },
        { REG_FRFMID, FRFMID_43392 },
        { REG_FRFLSB, FRFLSB_43392 },
        { REG_RXBW, RF_RXBW_DCCFREQ_110 | RXBW_83KHZ }, //  110b=DCC 0.25%
        { REG_SYNCVALUE1, PREAMBLE_BYTE },
        { REG_PREAMBLEMSB, 0 }, // length of preamble msb
        { REG_PREAMBLELSB, 3 }, // premble length: first byte of sync is last byte of preamble
        { REG_SYNCCONFIG, RF_SYNC_ON | RF_SYNC_SIZE_3 }, // one byte preamble + two bytes sync = 24bits total
        { REG_PACKETCONFIG1, RF_PACKET1_FORMAT_FIXED }, // fixed length for unlimited packet length format
        { REG_PAYLOADLENGTH, 0 }, // zero length for unlimited packet length format
        { REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | SX1231_FIFO_SIZE_HALF },
        //{ REG_LNA, RF_LNA_GAINSELECT_MAX }, // auto lna gain setting only runs at rx start anyways
        //{ REG_DAGCTEST, RF_DAGC_MANUAL | 2 },   // receiver baseband gain fixed at 2
        { REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_OUTPUTPOWER_00000 },  //  00000=minimum power
        //{ REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_OUTPUTPOWER_11111 },  //  11111=max power
        { REG_TESTDAGC, RF_DAGC_CONTINUOUS },   // run dagc always in rx mode
        // dio4:RxReady(10) dio3:PllLock(11) dio1:FifoLevel(00) dio0:PacketSent(00)
        { REG_DIOMAPPING1, RF_DIOMAPPING1_DIO3_10 | RF_DIOMAPPING1_DIO1_00 | RF_DIOMAPPING1_DIO0_00 },  // dio0 dio1 dio2 dio3=SyncAddress(debugging)
        { REG_DIOMAPPING2, RF_DIOMAPPING2_DIO4_10 | RF_DIOMAPPING2_DIO5_01 }, // dio4=RxReady, dio5=data
        {255, 0}
    };
    int i;
    TIM_TimeBaseInitTypeDef tbase_in;
    NVIC_InitTypeDef NVIC_InitStructure;


    /* in case we are starting too soon, before radio, test register writing */
    do {
        WriteReg_Sx1231(REG_AESKEY1, 0x55);
    } while (ReadReg_Sx1231(REG_AESKEY1) != 0x55);

    do {
        WriteReg_Sx1231(REG_AESKEY1, 0xaa);
    } while (ReadReg_Sx1231(REG_AESKEY1) != 0xaa);

    for (i = 0; defaults[i][0] != 255; i++) {
        WriteReg_Sx1231(defaults[i][0], defaults[i][1]);
        update_shadow_regs(defaults[i][0], defaults[i][1]);
    }

#ifdef RADIO_DEBUG
    i = ReadReg_Sx1231(REG_VERSION);
    debug_printf("sx1231 init %02x\r\n", i);
#endif /* RADIO_DEBUG */

    /* radio receiver timer */
    tbase_in.TIM_Prescaler      = 1 << RXTIM_PRESCALE_SHIFT;
    tbase_in.TIM_Period         = 0xffff;    // ARR
    TIM_TimeBaseInit(RXTIM, &tbase_in);
    TIM_ETRClockMode2Config(RXTIM, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);

    radio.evtdone       = &otutils_sig2_null;   // in case enabling RXTIM interrupt causes evtdone()
    TIM_ITConfig(RXTIM, TIM_IT_CC1, ENABLE);

    {
        vlFILE* fp          = ISF_open_su( ISF_ID(channel_configuration) );
        debug_printf("fp=%p\r\n", fp);
        phymac[0].channel   = 0x55;         // 55=invalid, forces calibration.
        phymac[0].tx_eirp   = 0x00;
        radio.state         = 0;            // (idle)

        sub_channel_lookup(0x00, fp);
        vl_close(fp);
    }

    /* SPI2 RX/TX DMA CH5 Configuration:  -----------------------------------------*/
    SPI2_DMA_Init.DMA_PeripheralBaseAddr = SPI2_DR_ADDRESS;
    SPI2_DMA_Init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    SPI2_DMA_Init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    SPI2_DMA_Init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    SPI2_DMA_Init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    SPI2_DMA_Init.DMA_Mode = DMA_Mode_Normal;
    SPI2_DMA_Init.DMA_M2M = DMA_M2M_Disable;

    SPI2_DMA_Init.DMA_DIR = DMA_DIR_PeripheralDST;  // for tx
    SPI2_DMA_Init.DMA_Priority = DMA_Priority_High;

    /* Enable the SPI2 TX DMA1_CH5 global Interrupt */
    //NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    //NVIC_Init(&NVIC_InitStructure);

    /* Enable the SPI2 RX DMA1_CH5 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    exti9_5_init.EXTI_Line = EXTI_Line5;   // PA5    SX1231-DIO1 sensitive to either edge
    exti9_5_init.EXTI_Mode = EXTI_Mode_Interrupt;

    exti4_init.EXTI_Line = EXTI_Line4;   // PA4    SX1231-DIO0
    exti4_init.EXTI_Trigger = EXTI_Trigger_Rising;  // always rising edge sensitive
    exti4_init.EXTI_Mode = EXTI_Mode_Interrupt;
}

void
sub_kill(ot_int main_err, ot_int frame_err)
{
    radio.state = 0;
    sys_quit_rf();
    radio_gag();

    radio_sleep();

    radio.evtdone(main_err, frame_err);
    radio.evtdone = &otutils_sig2_null;
}


#if (SYS_RECEIVE == ENABLED)
void
rx_done_isr(ot_int pcode)
{
    ot_int c;
#ifdef RADIO_DEBUG
    ot_int dBm;
#endif /* RADIO_DEBUG */

    //debug_printf("ft:%d ", RegFifoThresh.bits.FifoThreshold);
    // rssi samples were taken during packet reception
    radio.rssi_sum >>= RSSI_SUM_SHIFT;

    // argument 2 is negative on bad Frame CRC
    c = (ot_int)crc_check();
//    radio.evtdone(0, (ot_int)crc_check() - 1);
#ifdef RADIO_DEBUG
    dBm = radio.rssi_sum >> 1;
    dBm = -dBm;
    if (c)
        debug_printf("[42mcrc ok");   // green
    else
        debug_printf("[41mcrc fail"); // red
    debug_printf("[0m rssi=%ddBm (%d)\r\n", dBm, radio.rssi_count);
#endif /* RADIO_DEBUG */
    //debug_printf("crc_check(): %x\r\n", c);
    radio.evtdone(pcode, c - 1);
    radio.evtdone = &otutils_sig2_null;

    radio.state = RADIO_STATE_RXDONE;
}
#endif /* SYS_RECEIVE == ENABLED */

#if (SYS_RECEIVE == ENABLED)
void
rm2_rxtimeout_isr()
{
    TIM_Cmd(RXTIM, DISABLE);

#ifdef DISABLE_TRANSMIT
    if (radio.state == RADIO_STATE_DUMMY_TX) {
        /* receiver timer is used to fake the Tx_done */
        radio.evtdone(0, 0);
        radio.state = RADIO_STATE_TXDONE;
    } else {
#endif
        // disable interrupt PA5 SX1231-DIO1 FifoLevel
        exti9_5_init.EXTI_LineCmd = DISABLE;
        EXTI_Init(&exti9_5_init);

        // were running from ISR.. debug_printf("rx timeout\r\n");
        radio.evtdone(RM2_ERR_TIMEOUT, 0);    // main_err, frame_err
#ifdef DISABLE_TRANSMIT
    }
#endif

    radio.state = RADIO_STATE_RXDONE;
    radio.evtdone = &otutils_sig2_null;

}
#endif /* SYS_RECEIVE == ENABLED */

#if (SYS_RECEIVE == ENABLED)
static void
sx1231_start_rx()
{
    //debug_printf("sys.comm.rx_timeout=%d\r\n", sys.comm.rx_timeout);

    // ok to already be in receive mode upon entry
    if (RegOpMode.bits.Mode == CHIP_MODE_RECEIVER) {
        // if already in receive and fifo not empty...
        set_chip_mode(CHIP_MODE_STANDBY, 0);
        // fifo will be cleared when going from standby to rx
    }

    set_chip_mode(CHIP_MODE_RECEIVER, 0);

    RegFifoThresh.bits.FifoThreshold = em2_remaining_bytes();
/*    if (RegFifoThresh.bits.FifoThreshold > SX1231_FIFO_SIZE) {
        for (;;)
            asm("nop");
    }*/
    WriteReg_Sx1231(REG_FIFOTHRESH, RegFifoThresh.octet);
    //debug_printf("remaining: %d\r\n", em2_remaining_bytes());

#if (SYS_FLOOD == ENABLED)
    if (radio.flags & RADIO_FLAG_FLOOD) {
        RegIrqFlags1_t RegIrqFlags1;
        int rssi;

        TIM_SetCounter(RXTIM, 0);
        TIM_SetCompare1(RXTIM, RXTIM->ARR);
        TIM_Cmd(RXTIM, ENABLE);
        while (GPIO_ReadInputDataBit(GPIO_Port_RxReady, GPIO_Pin_RxReady) == Bit_RESET) {
            if (RXTIM->CNT > 1) {
                TIM_Cmd(RXTIM, DISABLE);
#ifdef RADIO_DEBUG
                debug_printf("flood no rxready\r\n");
#endif
                radio.evtdone(RM2_ERR_CCAFAIL, 0);    // main_err, frame_err
                return;
            }
        }
        TIM_Cmd(RXTIM, DISABLE);
        //debug_printf("rxready at %d\r\n", TIM2->CNT);
        RegIrqFlags1.octet = ReadReg_Sx1231(REG_IRQFLAGS1);
        //debug_printf("flags: 0x%02x\r\n", RegIrqFlags1.octet);

        // sufficient received signal to start SX1231 receiver
        rssi = 0;
        for (radio.rssi_count = 0; radio.rssi_count < 4; radio.rssi_count++) {
            rssi = ReadReg_Sx1231(REG_RSSIVALUE);
#ifdef RADIO_DEBUG
            debug_printf("rssi:%d ", rssi);
#endif /* RADIO_DEBUG */
            rssi >>= 1;
            rssi = 0 - rssi;
            rssi += 140;
            if (rssi > phymac[0].cs_thr)    // Esm
                break;
        }

        if (rssi < phymac[0].cs_thr) {  // Esm
#ifdef RADIO_DEBUG
            debug_printf(" %d < %d\r\n", rssi, phymac[0].cs_thr);
#endif /* RADIO_DEBUG */
            radio.evtdone(RM2_ERR_CCAFAIL, 0);    // main_err, frame_err
            return;
        }

        // how long to receive this?
        TIM_SetCompare1(RXTIM, 300);
    } else {
#endif /* ...SYS_FLOOD == ENABLED */
        /* RXTIM runs at 1024Hz, same as sys.comm.rx_timeout */
        TIM_SetCompare1(RXTIM, dll.comm.rx_timeout);
#if (SYS_FLOOD == ENABLED)
    }
#endif /* ...SYS_FLOOD == ENABLED */

    // prepare for rssi sampling during packet reception
    radio.rssi_sum = 0;
    radio.rssi_count = 0;

    TIM_SetCounter(RXTIM, 0);
    TIM_Cmd(RXTIM, ENABLE);
    GPIO_WriteBit(GPIO_Port_CON2_40, GPIO_Pin_CON2_40, Bit_SET);    // tmp debug


    radio.state = RADIO_STATE_RX;

    // PA5 SX1231-DIO1 FifoLevel
    exti9_5_init.EXTI_Trigger = EXTI_Trigger_Rising;
    exti9_5_init.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti9_5_init);
}
#endif /* SYS_RECEIVE == ENABLED */

void
start_tx(ot_bool initial)
{
    num_bytes_to_send = 0;    // initialize rf buffer
    num_bytes_sent = 0;    // initialize

    // SX1231 sends by itself preamble and sync word in unlimited packet length format

    if (initial) {
#       if (SYS_FLOOD == ENABLED)
        /// Packet flooding.  Only needed on devices that can send M2AdvP
        if (radio.flags & RADIO_FLAG_FLOOD) {
            sub_syncword_config(0);     // use syncword class 0
        }
        else
#       endif
        {
            sub_syncword_config(1);     // use syncword class 1
        }
    }

    set_chip_mode(CHIP_MODE_TRANSMITTER, 0);

    RegFifoThresh.bits.FifoThreshold = SX1231_FIFO_SIZE_HALF;
    WriteReg_Sx1231(REG_FIFOTHRESH, RegFifoThresh.octet);

    num_bytes_sent = 0;

    /* em2_encode_data() will encode bytes until txopen() returns false.
     * putbyte() called for each */
    em2_encode_data();
#ifdef RADIO_DEBUG
    debug_printf("start_tx 0x%02x %d\r\n", phymac[0].channel, num_bytes_to_send);
#endif /* RADIO_DEBUG */

    if (num_bytes_to_send > RF_FEATURE_TXFIFO_BYTES)
        SPI2_DMA_Init.DMA_BufferSize = RF_FEATURE_TXFIFO_BYTES;
    else
        SPI2_DMA_Init.DMA_BufferSize = num_bytes_to_send;

    /* SPI2_TX DMA will be started in SPI2 ISR (burst transfer to radio) */
    SPI_DataSizeConfig(SPI2, SPI_DataSize_8b);
    ASSERT_NSS_CONFIG();
    SPI_I2S_SendData(SPI2, 0x80);   // bit 7 to address 0: write to fifo
    spi2_state = SPI2_STATE__START_TX_DMA;
    start_tx_from = 1;

}

ot_int
rm2_default_tgd(ot_u8 chan_id)
{
#if ((M2_FEATURE(FEC) == DISABLED) && (M2_FEATURE(TURBO) == DISABLED))
    return M2_TGD_55FULL;

#elif ((M2_FEATURE(FEC) == DISABLED) && (M2_FEATURE(TURBO) == ENABLED))
    return (chan_id & 0x60) ? M2_TGD_200FULL : M2_TGD_55FULL;

#elif ((M2_FEATURE(FEC) == ENABLED) && (M2_FEATURE(TURBO) == DISABLED))
    return (chan_id & 0x80) ? M2_TGD_55FULL : M2_TGD_55HALF;

#elif ((M2_FEATURE(FEC) == ENABLED) && (M2_FEATURE(TURBO) == ENABLED))
    /// @note This is an incredible hack, but it is fast and compact.
    /// To understand it, you need to know the way the channel ID works.
    static const ot_int tgd[4] = {
        M2_TGD_55FULL,
        M2_TGD_200FULL,
        M2_TGD_55HALF,
        M2_TGD_200HALF
    };

    chan_id    += 0x20;
    chan_id   >>= 6;

    return tgd[chan_id];

#else
#   error "Missing definitions of M2_FEATURE(FEC) and/or M2_FEATURE(TURBO)"
    return 0;
#endif
}

/** Packet overhead & slop,
  * These are parts of the packet that do not carry preamble, sync word, or
  * frame data.  The units are in bytes.
  */
#define RADIO_RAMP_UP       1
#define RADIO_RAMP_DOWN     1
#define RADIO_PKT_PADDING   1
#define RADIO_PKT_OVERHEAD  (RADIO_RAMP_UP + RADIO_RAMP_DOWN + RADIO_PKT_PADDING)

ot_int
rm2_pkt_duration(ot_int pkt_bytes)
{
/// Wrapper function for rm2_scale_codec that adds some slop overhead
/// Slop = preamble bytes + sync bytes + ramp-up + ramp-down + padding

    pkt_bytes  += RADIO_PKT_OVERHEAD;
    pkt_bytes  += ((phymac[0].channel & 0x60) != 0) << 1;

    return rm2_scale_codec(pkt_bytes + 8);
}

static ot_bool
sub_chan_scan( void ) {
    vlFILE* fp;
    ot_int  i;

    fp = ISF_open_su( ISF_ID(channel_configuration) );
    ///@todo assert fp

    /// Go through the list of tx channels
    /// - Make sure the channel ID is valid
    /// - Make sure the transmission can fit within the contention period.
    /// - Scan it, to make sure it can be used
    for (i=0; i<dll.comm.tx_channels; i++) {
        if (sub_channel_lookup(dll.comm.tx_chanlist[i], fp) != False) {
            // rm2_txcsma() is measuring rssi for energy detection
            break;
        }
    }

    vl_close(fp);
    return (ot_bool)(i < dll.comm.tx_channels);
}

static ot_bool
sub_nocsma_init()
{
/// Called directly by TX radio function(s) when CSMA is disabled via System
/// Duty: bypass CSMA scan
    return sub_chan_scan( );
}

static ot_bool
sub_csma_init() {
/// Called directly by TX radio function(s)
/// Duty: Initialize csma process, and run scan.
    ot_bool cca1_status;

    /// Setup channel, scan it, and power down RF on scan fail
    cca1_status = sub_chan_scan( );
    if (cca1_status == False) {         //Optimizers may remove this if() for
        radio_sleep();                  //certain implementations
    }

    if (RegOpMode.bits.Mode != CHIP_MODE_RECEIVER)
        set_chip_mode(CHIP_MODE_RECEIVER, 0);

    return cca1_status;
}

ot_int
rm2_txcsma()
{
    ot_u8 rssi;

    // retval -1: channel is clear
    // retval non negative: approx callback time guard time
    // retval -2: rm2 cca failed

#ifdef RADIO_DEBUG
    //debug_printf("csma%02x ", radio.state);
    if (spi2_state != SPI2_STATE__NONE) {
        debug_printf("csma spi2_state:%d\r\n", spi2_state);
        for (;;)
            asm("nop");    // currently radio message in progress
    }
#endif /* RADIO_DEBUG */

    if (radio.state == RADIO_STATE_TXCCA_INIT) {
        if (dll.comm.csmaca_params & M2_CSMACA_NOCSMA) {
            if (sub_nocsma_init()) {
                radio.state = RADIO_STATE_TXSTART;
                return 0;
            }
            return RM2_ERR_BADCHANNEL;
        }

        if (sub_csma_init() == False) {
            return RM2_ERR_CCAFAIL;
        }


        radio.rssi_sum = 0;
        radio.rssi_count = 0;
#ifdef RADIO_DEBUG
        radio.unlock_count = 0;
#endif /* RADIO_DEBUG */
        radio.state = RADIO_STATE_TXCCA_;
        return 0;
    } else if (radio.state == RADIO_STATE_TXCCA_) {
        if (dll.comm.csmaca_params & M2_CSMACA_NOCSMA) {
            radio.state = RADIO_STATE_TXSTART;
            return 0;
        }
        if (GPIO_ReadInputDataBit(GPIO_Port_RxReady, GPIO_Pin_RxReady) == Bit_RESET) {
            RegIrqFlags1_t RegIrqFlags1;
            // swiss engineering: if ModeReady and PllLock, but not RxReady, this indicates no signal present
            RegIrqFlags1.octet = ReadReg_Sx1231(REG_IRQFLAGS1);
            if (RegIrqFlags1.bits.ModeReady && RegIrqFlags1.bits.PllLock) {
                //debug_printf("ModeReady && PllLock && !RxReady\r\n");
                radio.state = RADIO_STATE_TXSTART;
                return 0;
            }
#ifdef RADIO_DEBUG
            if (!RegIrqFlags1.bits.PllLock) {
                if (++radio.unlock_count > 5) {
                    debug_printf("!PllLock\r\n");
                    if (!RegIrqFlags1.bits.ModeReady)
                        debug_printf("!ModeReady\r\n");
                }
            }
#endif /* RADIO_DEBUG */
            return 5;
        }

        /*while (spi_busy != 0)
            asm("nop"); // if we are called faster than spi speed */

        rssi = ReadReg_Sx1231(REG_RSSIVALUE);
        //debug_printf("%d: %d\r\n", rssi, radio.rssi_count);
        radio.rssi_sum += rssi;
        if (++radio.rssi_count == RSSI_SUM_COUNT) {
            int rssi;
            radio.rssi_sum >>= RSSI_SUM_SHIFT;
            // convert sx1231 rssi to dash7 rssi:
#ifdef RADIO_DEBUG
            debug_printf("cca_thr:%d sum:%d ", phymac[0].cca_thr, radio.rssi_sum);
#endif /* RADIO_DEBUG */
            rssi = radio.rssi_sum >> 1; // >>1: half dB steps to 1 dB steps
            rssi = 0 - rssi;
            rssi += 140;
            //debug_printf("rssi=%d ", rssi);
            if (rssi > phymac[0].cca_thr) {
#ifdef RADIO_DEBUG
                debug_printf("[41m%d[0m\r\n", rssi);
#endif /* RADIO_DEBUG */
                radio.rssi_sum = 0;
                radio.rssi_count = 0;
                return RM2_ERR_CCAFAIL;
            }

#ifdef DISABLE_TRANSMIT
            radio.state = RADIO_STATE_DUMMY_TX;
#else
            radio.state = RADIO_STATE_TXSTART;
#endif
        } // ...if (++radio.rssi_count > RSSI_SUM_COUNT)

        return 1;
    } else if (radio.state == RADIO_STATE_TXSTART) {
#ifdef RADIO_DEBUG
        debug_printf("TX ");
#endif /* RADIO_DEBUG */

        OT_LOG_TX();

        em2_encode_newpacket();    // move to txinit_ff
        em2_encode_newframe();    // move to txinit_ff
        if (em2_encode_data == NULL) {
            /* fail in a predictable way */
            radio.evtdone(RM2_ERR_GENERIC, 0);
            return -2;
        }

        radio.state = RADIO_STATE_TXDATA;

        start_tx(True);

#ifdef DISABLE_TRANSMIT
    } else if (radio.state == RADIO_STATE_DUMMY_TX) {
        TIM_SetCompare1(RXTIM, DUMMY_TX_TIME);
        TIM_SetCounter(RXTIM, 0);
        TIM_Cmd(RXTIM, ENABLE);

        set_chip_mode(CHIP_MODE_SYNTHESIZER, 0);
#endif /* DISABLE_TRANSMIT */
    }


    return -1;
}

void
radio_sleep()
{
    /* might go to sleep for lower current, with awareness of longer wakeup time */
    if (RegOpMode.bits.Mode != CHIP_MODE_STANDBY) {
        //called from isr.. debug_printf("radio_sleep    %x    (chip_mode=%x)\r\n", radio.state, mcparam1.bits.chip_mode);
        /* nonblocking: might be called from ISR context */
        set_chip_mode(CHIP_MODE_STANDBY, 1);
    }
}

ot_int rm2_scale_codec(ot_int buf_bytes) {
/// Turns a number of bytes (buf_bytes) into a number of ti units.
/// To refresh your memory: 1 ti = ((1sec/32768) * 2^5) = 2^-10 sec = ~0.977 ms

    /// Pursuant to DASH7 Mode 2 spec, b6:4 of channel ID corresponds to kS/s.
    /// 55.555 kS/s = 144us per buffer byte
    /// 200.00 kS/s = 40us per buffer bytes
    buf_bytes *= (phymac[0].channel & 0x60) ? 40 : 144;

    /// Divide us into Ticks
    /// (shift right 10 = divide by 1024)
#   if ((M2_FEATURE(FEC) == ENABLED) && (RF_FEATURE(FEC) == ENABLED))
        buf_bytes >>= (10 - ((phymac[0].channel & 0x80) != 0) );
#   else
        buf_bytes >>= 10;
#   endif

    return buf_bytes;
}

// radio_txopen_4(): Checks the TX buffer to see if at least 4 more bytes can fit in it
ot_bool
radio_txopen_4()
{
    if (num_bytes_to_send < (SIZEOF_RF_DATA_BUF-4))
        return True;    // has space
    else
        return False;    // full
}

// radio_txopen(): Checks the TX buffer to see if at least 1 more byte can fit in it
ot_bool
radio_txopen()
{
    if (num_bytes_to_send < SIZEOF_RF_DATA_BUF)
        return True;    // has space
    else
        return False;    // full
}

// radio_putbyte(): Puts a byte to the TX radio buffer
void radio_putbyte(ot_u8 databyte)
{
    //debug_printf(" %02x", databyte);
    rf_data_buf[num_bytes_to_send++] = databyte;
}

static void
sub_prep_q(ot_queue* q)
{
/// Put some special data in the queue options field.
/// Lower byte is encoding options (i.e. FEC)
/// Upper byte is processing options (i.e. CRC)
    q->options.ubyte[LOWER]    = (phymac[0].channel & 0x80);
    q->options.ubyte[UPPER]    = 1;
    /*q->options.ubyte[UPPER]    = (RF_FEATURE(PACKET) != ENABLED) || \
                                  (RF_FEATURE(CRC) != ENABLED) || \
                                  ( (RF_FEATURE(PN9) != ENABLED) && \
                                    (q->options.ubyte[LOWER] == 0)) || \
                                  ( (RF_FEATURE(FEC) != ENABLED) && \
                                    (q->options.ubyte[LOWER] == 1));
    */
}

// Initializes TX engine for "foreground" packet transmission
// est_frames  (ot_int) Number of frames in packet to transmit
// callback    (ot_sig2) callback for when TX is done, on error or complete
void rm2_txinit_ff(ot_int est_frames, ot_sig2 callback)
{
#ifdef RADIO_DEBUG
    debug_printf("rm2_txinit_ff\r\n");
    if (est_frames != 1)
        debug_printf("est_frames=%d\r\n", est_frames);
    if (spi2_state != SPI2_STATE__NONE) {
        debug_printf("txinit_ff spi2_state:%d\r\n", spi2_state);
        for (;;)
            asm("nop");    // currently radio message in progress
    }
#endif /* RADIO_DEBUG */

    radio.state     = RADIO_STATE_TXCCA_INIT;
    radio.flags     = (est_frames > 1);
    radio.evtdone   = callback;

    /// Prepare the foreground frame packet
    txq.getcursor   = txq.front;
    txq.front[1]    = phymac[0].tx_eirp;

    sub_prep_q(&txq);

}

static ot_bool
sub_test_channel(ot_u8 channel, ot_u8 netstate) {
#if (SYS_RECEIVE == ENABLED)
    ot_bool test = True;

    if ((channel != phymac[0].channel) || (netstate == M2_NETSTATE_UNASSOC)) {
        vlFILE* fp;

        /// Open the Mode 2 FS Config register that contains the channel list
        /// for this host, and make sure the channel we want to use is available
        fp = ISF_open_su( ISF_ID(channel_configuration) );
        ///@todo assert fp

        //debug_printf("sub_test_channel(0x%02x)\r\n", channel);
        test = sub_channel_lookup(channel, fp);
        vl_close(fp);
    }

    return test;
#else
    return True;
#endif
}

/* same as rxinit_ff, but used to to detect background floods
 * terminates once a packet is received */
void
rm2_rxinit_bf(ot_u8 channel, ot_sig2 callback)
{
#if (SYS_RECEIVE == ENABLED)

#ifdef RADIO_DEBUG
    debug_printf("rm2_rxinit_bf(%d)\r\n", channel);
    if (spi2_state != SPI2_STATE__NONE) {
        debug_printf("rxinit_bf spi2_state:%d\r\n", spi2_state);
        for (;;)
            asm("nop");    // currently radio message in progress
    }
#endif /* RADIO_DEBUG */

    if (sub_test_channel(channel, M2_NETSTATE_UNASSOC) == False) {
#ifdef RADIO_DEBUG
        debug_printf("bf [41mFalse = sub_test_channel(%d)[0m\r\n", channel);
#endif /* RADIO_DEBUG */
        radio.evtdone(RM2_ERR_BADCHANNEL, 0);
        return;
    }

    radio.state     = RADIO_STATE_RXINIT;
    radio.flags     = RADIO_FLAG_FLOOD;
    radio.evtdone   = callback;

    q_empty(&rxq);

    sub_prep_q(&rxq);
    em2_decode_newpacket();
    em2_decode_newframe();

    sub_syncword_config(0);
    sx1231_start_rx();

    /* background scan requires minimum rssi (Esm) prior to sync word */
    /* terminate scan immediately if rssi below Esm */
    /* otherwise wait Tbsd for sync word (background scan detection timeout) */

#else /* SYS_RECEIVE == ENABLED */
    // BLINKER only (no RX)
    callback(RM2_ERR_GENERIC, 0);
#endif
    //lr = LR_RM2_RXINIT_BF;
}

// Checks the RX buffer to see if there are at least 1 more bytes in it
ot_bool
radio_rxopen()
{
    if (num_bytes_sent < SPI2_DMA_Init.DMA_BufferSize)
        return True;
    else
        return False;
}

// Gets a byte from the RX radio buffer
ot_u8
radio_getbyte()
{
    //debug_printf("%02x ", rf_data_buf[num_bytes_sent]);
    return rf_data_buf[num_bytes_sent++];
}

ot_int
radio_rssi()
{
    ot_int ret;
    /* called from recvt_frx() via sub_mac_filter() */

    // returning RSSI measured during mostly recently received packet
    ret = -radio.rssi_sum;
    return ret + 140;
}

// Initializes TX engine for "background" packet flooding
void
rm2_txinit_bf(ot_sig2 callback)
{
#if (SYS_FLOOD == ENABLED)

#ifdef RADIO_DEBUG
    if (spi2_state != SPI2_STATE__NONE) {
        debug_printf("txinit_bf spi2_state:%d\r\n", spi2_state);
        for (;;)
            asm("nop");    // currently radio message in progress
    }
    debug_printf("txinit_bf %x\r\n", radio.state);
#endif /* RADIO_DEBUG */

    radio.state     = RADIO_STATE_TXCCA_INIT;
    /* background transmissions are associated with advertizing and reservations,
     * which means we are flooding */
    radio.flags     = RADIO_FLAG_FLOOD;
    radio.evtdone   = callback;

    /// Prepare the foreground frame packet
    txq.getcursor   = txq.front;
    txq.front[1]    = phymac[0].tx_eirp;

    sub_prep_q(&txq);
#endif /* ...SYS_FLOOD == ENABLED */
}

/** Initializes RX engine for "foreground" packet reception
  */
void rm2_rxinit_ff(ot_u8 channel, ot_u8 netstate, ot_int est_frames, ot_sig2 callback)
{
#if (SYS_RECEIVE == ENABLED)

#ifdef RADIO_DEBUG
    debug_printf("rm2_rxinit_ff(0x%x, 0x%x, %d) %d\r\n", channel, netstate, est_frames, dll.comm.rx_timeout);
    if (spi2_state != SPI2_STATE__NONE) {
        debug_printf("rxinit_ff spi2_state:%d %d\r\n", spi2_state, start_tx_from);
        for (;;)
            asm("nop");    // state machine got hung up?
    }
#endif /* RADIO_DEBUG */

    if (sub_test_channel(channel, M2_NETSTATE_UNASSOC) == False) {
#ifdef RADIO_DEBUG
        debug_printf("ff [41mFalse = sub_test_channel(%d)[0m\r\n", channel);
#endif /* RADIO_DEBUG */
        radio.evtdone(RM2_ERR_BADCHANNEL, 0);
        return;
    }

    radio.state     = RADIO_STATE_RXINIT;
#   if (M2_FEATURE(MULTIFRAME) == ENABLED)
        radio.flags = (est_frames > 1); //sets RADIO_FLAG_FRCONT
#   else
        radio.flags = 0;
#   endif
    //radio.rxlimit   = 8;
    radio.evtdone   = callback;

    q_empty(&rxq);

    sub_prep_q(&rxq);
    em2_decode_newpacket();
    em2_decode_newframe();

    sub_syncword_config(1);
    sx1231_start_rx();

    // turns on when reception done
    //GPIO_WriteBit(GPIO_Port_LED_YELLOW, GPIO_Pin_LED_YELLOW, Bit_RESET);

#else /* (SYS_RECEIVE == ENABLED) */
    // BLINKER only (no RX)
    callback(RM2_ERR_GENERIC, 0);
#endif /* (SYS_RECEIVE == ENABLED) */
    //lr = LR_RM2_RXINIT_FF;
}

void rm2_txstop_flood() {
#if (SYS_FLOOD == ENABLED)
    //radio.flags &= ~RADIO_FLAG_FLOOD;
    if (radio.flags & RADIO_FLAG_FLOOD)
        radio.flags |= RADIO_FLAG_FLOOD_STOP;
#endif
}

void
radio_gag()
{
    // Disables all Events/Interrupts used by the radio module

    /* radio_sleep() is sufficient. SX1231 doesnt do anything in sleep or standby modes. */
}


void rm2_kill() {
    sub_kill(RM2_ERR_KILL, 0);
}

void
spi_save_restore(ot_bool saving)
{
}

#ifdef RADIO_DEBUG

static void
frf_adj(ot_bool up)
{
    ot_u16 f;

    f = ReadReg_Sx1231(REG_FRFMID);
    f <<= 8;
    f += ReadReg_Sx1231(REG_FRFLSB);

    if (up)
        f += 16;    // up nearly 1KHz
    else
        f -= 16;    // down nearly 1KHz

    WriteReg_Sx1231(REG_FRFMID, f >> 8);
    WriteReg_Sx1231(REG_FRFLSB, f & 0xff);
}

void
radio_console_service()
{
    unsigned char addr, val, nlo;

    if (uart_rx_buf[0] == '?') {
        debug_printf("\r\nr %%02x           read register");
        debug_printf("\r\nw %%02x %%02x     write register");
        debug_printf("\r\nR               read all registers");
        debug_printf("\r\nsw[12]          RF switch position");
    } else if (uart_rx_buf[0] == 'r' && uart_rx_buf[1] == ' ') {
        /* read register */
        from_hex(uart_rx_buf+2, &addr);
        val = ReadReg_Sx1231(addr);
        debug_printf("\r\nat 0x%02x: 0x%02x", addr, val);
        update_shadow_regs(addr, val);
    } else if (uart_rx_buf[0] == 'w' && uart_rx_buf[1] == ' ') {
        /* write register */
        from_hex(uart_rx_buf+2, &addr);
        from_hex(uart_rx_buf+5, &val);

        WriteReg_Sx1231(addr, val);

        val = ReadReg_Sx1231(addr);
        debug_printf("\r\nat 0x%02x: 0x%02x", addr, val);
        update_shadow_regs(addr, val);
    } else if (uart_rx_buf[0] == 'R' && uart_rx_buf[1] == ' ') {
        /* read all registers */
        for (nlo = 0; nlo < 0x40; nlo++) {
            val = ReadReg_Sx1231(nlo);
            debug_printf("\r\nat 0x%02x: 0x%02x", nlo, val);
            update_shadow_regs(nlo, val);
        }
#ifndef _STM32L152VBT6_
    } else if (uart_rx_buf[0] == 's' && uart_rx_buf[1] == 'w') {
        if (uart_rx_buf[2] == '1') {
            // on-board antenna
            GPIO_WriteBit(GPIO_Port_RFSW_V1, GPIO_Pin_RFSW_V1, Bit_RESET);
            GPIO_WriteBit(GPIO_Port_RFSW_V2, GPIO_Pin_RFSW_V2, Bit_SET);
        } else if (uart_rx_buf[2] == '2') {
            // RF connector
            GPIO_WriteBit(GPIO_Port_RFSW_V1, GPIO_Pin_RFSW_V1, Bit_SET);
            GPIO_WriteBit(GPIO_Port_RFSW_V2, GPIO_Pin_RFSW_V2, Bit_RESET);
        }
#endif
    } else if (uart_rx_buf[0] == '+') {
        if (RegRxBw.bits.DccFreq == 7)
            RegRxBw.bits.DccFreq = 0;
        else
            RegRxBw.bits.DccFreq++;

        WriteReg_Sx1231(REG_RXBW, RegRxBw.octet);
        debug_printf("\r\ndcc=%d", RegRxBw.bits.DccFreq);
    } else if (uart_rx_buf[0] == '-') {
        if (RegRxBw.bits.DccFreq == 0)
            RegRxBw.bits.DccFreq = 7;
        else
            RegRxBw.bits.DccFreq--;

        WriteReg_Sx1231(REG_RXBW, RegRxBw.octet);
        debug_printf("\r\ndcc=%d", RegRxBw.bits.DccFreq);
    } else if (uart_rx_buf[0] == '1') { // rxbw
        RegRxBw.bits.RxBw = RXBW_62KHZ;
        WriteReg_Sx1231(REG_RXBW, RegRxBw.octet);
        debug_printf("\r\nrxbw 62khz");
    } else if (uart_rx_buf[0] == '2') { // rxbw
        RegRxBw.bits.RxBw = RXBW_83KHZ;
        WriteReg_Sx1231(REG_RXBW, RegRxBw.octet);
        debug_printf("\r\nrxbw 83khz");
    } else if (uart_rx_buf[0] == '3') { // rxbw
        RegRxBw.bits.RxBw = RXBW_100KHZ;
        WriteReg_Sx1231(REG_RXBW, RegRxBw.octet);
        debug_printf("\r\nrxbw 100khz");
    } else if (uart_rx_buf[0] == '4') { // rxbw
        RegRxBw.bits.RxBw = RXBW_125KHZ;
        WriteReg_Sx1231(REG_RXBW, RegRxBw.octet);
        debug_printf("\r\nrxbw 125khz");
    } else if (uart_rx_buf[0] == '5') { // rxbw
        RegRxBw.bits.RxBw = RXBW_167KHZ;
        WriteReg_Sx1231(REG_RXBW, RegRxBw.octet);
        debug_printf("\r\nrxbw 167khz");
    } else if (uart_rx_buf[0] == 'g') { // trigger rssi
        // trigger not necessary if DAGC running continuously
        WriteReg_Sx1231(REG_RSSICONFIG, RF_RSSI_START);
    } else if (uart_rx_buf[0] == 'i') { // read rssi
        val = ReadReg_Sx1231(REG_RSSIVALUE);
        debug_printf("\r\nrssi: %d", val);
    } else if (uart_rx_buf[0] == '.') {
        debug_printf("\r\nDIO0: ");
        if (GPIO_ReadInputDataBit(GPIO_Port_PacketSent, GPIO_Pin_PacketSent) == Bit_RESET)
            __io_putchar('0');
        else
            __io_putchar('1');
#ifndef BLOCKING_UART_TX
        kick_dma_usart_tx(6);
#endif
    } else if (uart_rx_buf[0] == ')' && uart_rx_buf[1] == 0) { // freq up 1KHz
        frf_adj(True);
    } else if (uart_rx_buf[0] == '(' && uart_rx_buf[1] == 0) { // freq down 1KHz
        frf_adj(False);
    } else if (uart_rx_buf[0] == 'F' && uart_rx_buf[1] == 0) { // read freq
        unsigned int frf;
        float f;
        frf = ReadReg_Sx1231(REG_FRFMSB);
        frf <<= 8;
        frf += ReadReg_Sx1231(REG_FRFMID);
        frf <<= 8;
        frf += ReadReg_Sx1231(REG_FRFLSB);
        f = frf * 61.03515625;
        debug_printf("\r\nfrf: %d, %dHz", frf, (int)f);
    /*} else if (uart_rx_buf[0] == 'I') { // poll rssi
        print_rssi = !print_rssi;*/
    }
}
#endif /* RADIO_DEBUG */

