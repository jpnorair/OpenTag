/* Copyright 2009-2011 JP Norair
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
  * @file       /otradio/cc1101/CC1101_STM32F10x.h
  * @author     JP Norair
  * @version    V1.0
  * @date       2 April 2012
  * @brief      CC1101 transceiver interface implementation for STM32F10x
  * @ingroup    CC1101
  *
  * Untested until I have a STM32+CC1101 board, but it is very similar to other
  * implementations that I have confirmed.
  ******************************************************************************
  */

#include "OT_platform.h"
#ifdef PLATFORM_STM32F10x


#include "OT_utils.h"
#include "OT_types.h"
#include "OT_config.h"


#include "cc1101_interface.h"

//For test
#include "OTAPI.h"


#ifdef RADIO_DB_ATTENUATION
#   define _ATTEN_DB    RADIO_DB_ATTENUATION
#else
#   define _ATTEN_DB    0
#endif





#define __SPI_CS_HIGH() RADIO_SPICS_PORT->BSRR = (ot_u32)RADIO_SPICS_PIN
#define __SPI_CS_LOW()  RADIO_SPICS_PORT->BRR = (ot_u32)RADIO_SPICS_PIN
#define __SPI_CS_ON()   __SPI_CS_LOW()
#define __SPI_CS_OFF()  __SPI_CS_HIGH()
#define __SPI_ENABLE()  RADIO_SPI->CR1 |= SPI_CR1_SPE
#define __SPI_DISABLE() RADIO_SPI->CR1 &= ~SPI_CR1_SPE
#define __SPI_GET(VAL)  VAL = RADIO_SPI->DR
#define __SPI_PUT(VAL)  RADIO_SPI->DR = VAL





#if (ISR_EMBED(RADIO) == ENABLED)

#if (((RADIO_IRQ0_SRCLINE >= 5) && (RADIO_IRQ0_SRCLINE <= 9)) || \
     ((RADIO_IRQ1_SRCLINE >= 5) && (RADIO_IRQ1_SRCLINE <= 9)) || \
     ((RADIO_IRQ2_SRCLINE >= 5) && (RADIO_IRQ2_SRCLINE <= 9)) || \
     ((RADIO_IRQ3_SRCLINE >= 5) && (RADIO_IRQ3_SRCLINE <= 9)))
void EXTI9_5_IRQHandler(void) { cc1101_globalirq_isr(); }
#endif

#if (((RADIO_IRQ0_SRCLINE >= 10) && (RADIO_IRQ0_SRCLINE <= 15)) || \
     ((RADIO_IRQ1_SRCLINE >= 10) && (RADIO_IRQ1_SRCLINE <= 15)) || \
     ((RADIO_IRQ2_SRCLINE >= 10) && (RADIO_IRQ2_SRCLINE <= 15)) || \
     ((RADIO_IRQ3_SRCLINE >= 10) && (RADIO_IRQ3_SRCLINE <= 15)))
void EXTI15_10_IRQHandler(void) { cc1101_globalirq_isr(); }
#endif

#if (RADIO_IRQ0_SRCLINE == 0)
void EXTI0_IRQHandler(void) { cc1101_irq0_isr(); }

#elif (RADIO_IRQ0_SRCLINE == 1)
void EXTI1_IRQHandler(void)  { cc1101_irq0_isr(); }

#elif (RADIO_IRQ0_SRCLINE == 2)
void EXTI2_IRQHandler(void)  { cc1101_irq0_isr(); }

#elif (RADIO_IRQ0_SRCLINE == 3)
void EXTI3_IRQHandler(void)  { cc1101_irq0_isr(); }

#elif (RADIO_IRQ0_SRCLINE == 4)
void EXTI4_IRQHandler(void)  { cc1101_irq0_isr(); }

#endif

#if (RADIO_IRQ2_SRCLINE == 0)
void EXTI0_IRQHandler(void)  { cc1101_irq2_isr(); }

#elif (RADIO_IRQ2_SRCLINE == 1)
void EXTI1_IRQHandler(void)  { cc1101_irq2_isr(); }

#elif (RADIO_IRQ2_SRCLINE == 2)
void EXTI2_IRQHandler(void)  { cc1101_irq2_isr(); }

#elif (RADIO_IRQ2_SRCLINE == 3)
void EXTI3_IRQHandler(void)  { cc1101_irq2_isr(); }

#elif (RADIO_IRQ2_SRCLINE == 4)
void EXTI4_IRQHandler(void)  { cc1101_irq2_isr(); }

#endif


//Legacy defines
void IRQ0_ISR() { cc1101_irq0_isr(); }
void IRQ2_ISR() { cc1101_irq2_isr(); }


#endif // end of ISR_EMBED(RADIO) part




/// Persistent Data for radio driver
cc1101_struct cc1101;







/** Basic Control <BR>
  * ============================================================================
  */

void cc1101_coredump() {
///debugging function to dump-out register values of RF core
    ot_u8 i;

    for (i=0; i<0x2E; i++) {
        ot_u8 label[]   = { 'R', 'E', 'G', '_', 0, 0 };
        ot_u8 regval    = cc1101_read(i);

        otutils_bin2hex(&i, &label[4], 1);
        otapi_log_msg(MSG_raw, 6, 1, label, &regval);
        //platform_swdelay_ms(5);
    }
}



void cc1101_load_defaults() {
/// Load OpenTag/DASH7 startup defaults. This static array gets written by SPI.
    static const ot_u8 defaults[] = {
        0x40,
        DRF_IOCFG2,
        DRF_IOCFG1,
        DRF_IOCFG0,
        DRF_FIFOTHR,
        DRF_SYNC1,
        DRF_SYNC0,
        DRF_PKTLEN,
        DRF_PKTCTRL1,
        DRF_PKTCTRL0,
        DRF_ADDR,
        DRF_CHANNR,
        DRF_FSCTRL1,
        DRF_FSCTRL0,
        DRF_FREQ2,
        DRF_FREQ1,
        DRF_FREQ0,
        DRF_MDMCFG4,
        DRF_MDMCFG3,
        DRF_MDMCFG2,
        DRF_MDMCFG1,
        DRF_MDMCFG0,
        DRF_DEVIATN,
        DRF_MCSM2,
        DRF_MCSM1,
        DRF_MCSM0,
        DRF_FOCCFG,
        DRF_BSCFG,
        DRF_AGCCTRL2,
        DRF_AGCCTRL1,
        DRF_AGCCTRL0,
        DRF_WOREVT1,
        DRF_WOREVT0,
        DRF_WORCTRL,
        DRF_FREND1,
        DRF_FREND0,
        DRF_FSCAL3,
        DRF_FSCAL2,
        DRF_FSCAL1,
        DRF_FSCAL0,
        DRF_RCCTRL1,
        DRF_RCCTRL0,
        DRF_FSTEST,
        DRF_PTEST,
        DRF_AGCTEST,
        DRF_TEST2,
        DRF_TEST1,
        DRF_TEST0
    };

    cc1101_spibus_io(sizeof(defaults), 0, (ot_u8*)defaults, NULL);

    {
        volatile ot_u8 test;
        ot_u8 i;

        for (i=0; i<=0x2F; ++i) {
            test = cc1101_read(i);
        }
    }
}


void cc1101_reset()     {
/// Send Reset strobe, then wait for MISO to go low.  We don't need to set
/// MISO back to SPI (setting SEL), because this gets done in each SPI usage.
    cc1101_int_turnoff(RFI_ALL);
    cc1101_strobe(STROBE(SRES));
    while(RADIO_SPIMISO_PORT->IDR & RADIO_SPIMISO_PIN);
}


void cc1101_calibrate() { cc1101_strobe(STROBE(SCAL)); }

void cc1101_waitforidle() {
/// Uses CC1101 pin-control to go from sleep to idle, and waits for XTAL ready.
/// (2) Set CS low,
/// (3) Wait for MISO to go low,
/// (4) Re-attach SPI pins to SPI control
    __SPI_CS_LOW();
    __no_operation();
    while(RADIO_SPIMISO_PORT->IDR & RADIO_SPIMISO_PIN);
}

ot_u16 cc1101_rfstatus() {
    Twobytes readout;
    readout.ubyte[0] = cc1101_read(RF_MARCSTATE);
    readout.ubyte[1] = cc1101_read(RF_PKTSTATUS);
    return readout.ushort;
}

ot_u16 sub_get2regs(ot_u8 start) {
    Twobytes readout;
    cc1101_spibus_io(1, 2, &start, &readout.ubyte[0]);
    return readout.ushort;
}

ot_u16  cc1101_wortime()        { return sub_get2regs( RFREG(WORTIME1) ); }
ot_u16  cc1101_rcctrl()         { return sub_get2regs( RFREG(RCCTRL1_STATUS) ); }
ot_u8   cc1101_estfreqoffset()  { return cc1101_read( RFREG(FREQEST) ); }
ot_u8   cc1101_lqi()            { return cc1101_read( RFREG(LQI) ); }
ot_u8   cc1101_rssi()           { return cc1101_read( RFREG(RSSI) ); }
ot_u8   cc1101_vcovcdac()       { return cc1101_read( RFREG(VCO_VC_DAC) ); }
ot_u8   cc1101_rxbytes()        { return cc1101_read( RFREG(RXBYTES) ); }
ot_u8   cc1101_txbytes()        { return cc1101_read( RFREG(TXBYTES) ); }






/** Bus interface (SPI + 2x GPIO) <BR>
  * ============================================================================
  */

void cc1101_init_bus() {
/// @note platform_init_periphclk() should have alread enabled RADIO_SPI clock
/// and GPIO clocks

    ///1. Set-up SPI peripheral for Master Mode, Full Duplex
    RADIO_SPI->CR1      =   SPI_Direction_2Lines_FullDuplex |
                            SPI_Mode_Master | \
                            SPI_DataSize_8b | \
                            SPI_CPOL_High | \
                            SPI_CPHA_1Edge | \
                            SPI_NSS_Soft | \
                            RADIO_SPI_PRESCALER | \
                            SPI_FirstBit_MSB;
    RADIO_SPI->CR2      = 0;
    RADIO_SPI->I2SCFGR  = 0;


    ///2. Set-up SPI port: this could be moved into a BOARD inline function
    BOARD_RADIO_SPI_PORTCONF();
    

    // EXTI binding
    GPIO_EXTILineConfig(RADIO_IRQ0_SRCPORT, RADIO_IRQ0_SRCPIN);
    GPIO_EXTILineConfig(RADIO_IRQ2_SRCPORT, RADIO_IRQ2_SRCPIN);
    
    // Clear any interrupt flags on the lines
    EXTI->PR    =  RFI_ALL;
    EXTI->RTSR &= ~RFI_ALL;
    EXTI->FTSR &= ~RFI_ALL;
    EXTI->IMR  &= ~RFI_ALL;
    EXTI->EMR  &= ~RFI_ALL;

    // Using Interrupt Priority 0,1 per OpenTag requirements
#   if (RADIO_IRQ0_IRQn != RADIO_IRQ2_IRQn)
    NVIC->IP[(uint32_t)(RADIO_IRQ0_IRQn)]       = b0001 << (8 - __NVIC_PRIO_BITS);
    NVIC->IP[(uint32_t)(RADIO_IRQ2_IRQn)]       = b0001 << (8 - __NVIC_PRIO_BITS);
    NVIC->ISER[((uint32_t)(RADIO_IRQ0_IRQn)>>5)]= (1 << ((uint32_t)(RADIO_IRQ0_IRQn) & 0x1F));
    NVIC->ISER[((uint32_t)(RADIO_IRQ2_IRQn)>>5)]= (1 << ((uint32_t)(RADIO_IRQ2_IRQn) & 0x1F));
#   else
    NVIC->IP[(uint32_t)(RADIO_IRQ0_IRQn)]       = b0001 << (8 - __NVIC_PRIO_BITS);
    NVIC->ISER[((uint32_t)(RADIO_IRQ0_IRQn)>>5)]= (1 << ((uint32_t)(RADIO_IRQ0_IRQn) & 0x1F));
#   endif
    
    
    ///4. Power-up reset via CS pin & MISO pin
    __SPI_CS_HIGH();
    platform_swdelay_us(30);
    __SPI_CS_LOW();
    platform_swdelay_us(30);
    __SPI_CS_HIGH();
    platform_swdelay_us(45);
    __SPI_CS_LOW();
    while (RADIO_SPIMISO_PORT->IDR & RADIO_SPIMISO_PIN);
    __SPI_CS_HIGH();
    
    
    ///5. Perform reset of the digital core.
    cc1101_reset();
}



void cc1101_spibus_wait() {
    ///@todo have a windowed-watchdog here
    while (RADIO_SPI->STAT & UCBUSY);
}




void cc1101_spibus_io(ot_u8 cmd_len, ot_u8 resp_len, ot_u8* cmd, ot_u8* resp) {
    cc1101_waitforidle();
    __SPI_ENABLE();
    
    ///Wait for TX to be done.  Meanwhile, save the status byte
    while (cmd_len != 0) {
        cmd_len--;
        while ((RADIO_SPI->IFG & UCTXIFG) == 0);
        __SPI_PUT(*cmd++);
        while ((RADIO_SPI->IFG & UCRXIFG) == 0);
        __SPI_GET(cc1101.chipstatus);
    }

    ///Wait for RX to be done.
    while (resp_len != 0) {
        resp_len--;
        while ((RADIO_SPI->IFG & UCTXIFG) == 0);
        __SPI_PUT(0);
        while ((RADIO_SPI->IFG & UCRXIFG) == 0);
        __SPI_GET(*resp++);
    }

    ///Wait for SPI to be completely done, then kill
    //cc1101_spibus_wait();
    __SPI_DISABLE();
    __SPI_CS_OFF();
}


void cc1101_strobe(ot_u8 strobe) {
    cc1101_spibus_io(1, 0, &strobe, NULL);
}

ot_u8 cc1101_read(ot_u8 addr) {
    ot_u8 read_data;
    addr |= 0x80;
    cc1101_spibus_io(1, 1, &addr, &read_data);
    return read_data;
}

void cc1101_burstread(ot_u8 start_addr, ot_u8 length, ot_u8* data) {
    start_addr |= 0xC0;
    cc1101_spibus_io(1, length, &start_addr, data);
}

void cc1101_write(ot_u8 addr, ot_u8 data) {
    ot_u8 cmd[2];
    cmd[0]  = addr;
    cmd[1]  = data;
    cc1101_spibus_io(2, 0, cmd, NULL);
}

void cc1101_burstwrite(ot_u8 length, ot_u8* cmd_data) {
    cmd_data[0] |= 0x40;
    cc1101_spibus_io(length, 0, cmd_data, NULL);
}












/** Advanced Configuration <BR>
  * ========================================================================<BR>
  */


ot_u8 cc1101_calc_rssithr(ot_u8 input, ot_u8 offset) {
/// This function must prepare any hardware registers needed for automated
/// CS/CCA threshold value.
/// - "input" is a value 0-127 that is: input - 140 = threshold in dBm
/// - "offset" is a value subtracted from "input" that depends on chip impl
/// - return value is chip-specific threshold value

// cs_lut table specs:
// Base dBm threshold of CCA/CS is ~-92 dBm at 200 kbps or ~-96 dBm at 55 kbps.
// Each incremented byte in the array is roughly an additional 2 dB above the base dBm threshold.
/// @todo !!!elements 8 and 11 in lut are the same, this can't be right!!!
    static const ot_u8 cs_lut[18] = {
        b00000011, b00001011, b00001011, b01000011, b01001011, b00100011, b10000011, b10001011,
        b10100011, b11000011, b11001011, b10100011, b11010011, b11011011, b11100011, b11101011,
        b11110011, b11111011
    };

    ot_int thr;
    thr     = (ot_int)input - _ATTEN_DB;    // subtract dBm encoding offset
    thr   >>= 1;                            // divide by two (array is 2dB increments)
    thr    -= offset;               // (1) account for turbo vs normal rate
    
    if (thr > 17)       thr = 17;   // (2) make max if dBm threshold is above range
    else if (thr < 0)   thr = 0;    // (3) make 0 if dBm threshold is lower than range

    return cs_lut[thr];
}



ot_int cc1101_calc_rssi(ot_u8 encoded_value) {
/// CC1101 stores the RSSI in a special register, as a 2's complement number, of
/// offset 0.5 dBm units.  This function translates it into normal dBm units.
    ot_int rssi_val;
    rssi_val    = (ot_int)encoded_value;    // Convert to signed 16 bit (1 instr on MSP)
    rssi_val   += 128;                      // Make it positive...
    rssi_val  >>= 1;                        // ...So division to 1 dBm units can be a shift...
    rssi_val   -= (64 + RF_RSSI_OFFSET);    // ...and then rescale it, including offset

    return rssi_val;
}



void cc1101_set_txpwr(ot_u8 pwr_code) {
/// Sets the tx output power.
/// "pwr_code" is a value, 0-127, that is: eirp_code/2 - 40 = TX dBm
/// i.e. eirp_code=0 => -40 dBm, eirp_code=80 => 0 dBm, etc
///
/// CC1101 has an 8-value amplifier table.  TX power gets ramped-up, step by
/// step of the table.  Each value in the amp table is a power code specific
/// to the CC (from the lut).
///
/// CC1101-specific power selector table
/// The index value is pwr_code.  As you may notice, pwr_code does not map
/// linearly to TX dBm on CC, which is why there has to be a table.
    static const ot_u8 pa_lut[100] = {
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,     //-40 to -35.5 dBm
        0x01, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x04,     //-35 to -30.5
        0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x07,     //-30 to -25.5
        0x07, 0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0C, 0x0D,     //-25 to -20.5
        0x0E, 0x0E, 0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1C,     //-20 to -15.5
        0x1D, 0x1E, 0x1E, 0x1F, 0x24, 0x24, 0x33, 0x33, 0x25, 0x25,     //-15 to -10.5
        0x26, 0x26, 0x27, 0x27, 0x35, 0x28, 0x29, 0x36, 0x2A, 0x37,     //-10 to -5.5
        0x8F, 0x8F, 0x57, 0x56, 0x56, 0x55, 0x54, 0x53, 0x52, 0x51,     //-5 to -0.5
        0x60, 0x50, 0x50, 0x8D, 0x8C, 0x8B, 0x8A, 0x88, 0x87, 0x86,     //0 to 5
        0x85, 0x83, 0x81, 0xCA, 0xC9, 0xC7, 0xC6, 0xC4, 0xC3, 0xC1      //5 to 9.5
    };

    ot_u8   pa_table[9];
    ot_int  i;
    ot_int  eirp_val;

// Should be defined in board config file 
// Depends on matching circuit, antenna, and board physics
// Described in 0.5 dB units (12 = 6dB system loss)
#   ifndef RF_HDB_ATTEN
#       define RF_HDB_ATTEN 12
#   endif

    eirp_val    = pwr_code + RF_HDB_ATTEN;
    eirp_val    = (eirp_val < 99) ? eirp_val : 99;

    for(i=8; (i>=1) && (eirp_val>=0); i--, eirp_val-=6) {
        pa_table[i] = pa_lut[eirp_val];
    }

    pa_table[i] = RFREG(PATABLE);
    cc1101_burstwrite( (ot_u8)(9-i), &pa_table[i] );
    cc1101_write( RFREG(FREND0), (DRF_FREND0 | (ot_u8)(8-i)) );
}



#define _N(VAL)     (VAL | 0x40)

//GDO2, 1, 0
#ifdef RADIO_IRQ2_PIN
static const ot_u8 gdo_listen[4]    = { 0x40,   GDO_LNA_PD,         GDO_HIZ,    GDO_SYNCDETECT };
static const ot_u8 gdo_rxdata[4]    = { 0x40,   GDO_RXFHIGH,        GDO_HIZ,    _N(GDO_SYNCDETECT) };
static const ot_u8 gdo_txdata[4]    = { 0x40,   _N(GDO_TXFHIGH),    GDO_HIZ,    GDO_PA_PD };
static const ot_u8 gdo_txcsma[4]    = { 0x40,   GDO_CCAOK,          GDO_HIZ,    GDO_CSOK };

void sub_iocfg(CC1101_IMode imode, ot_u8* gdo_cfg) {
    EXTI->PR        = RFI_ALL;
    cc1101.imode    = imode;
    cc1101_spibus_io(4, 0, gdo_cfg, NULL);
}

void cc1101_iocfg_listen()  { sub_iocfg(MODE_Listen, (ot_u8*)gdo_listen); }
void cc1101_iocfg_rxdata()  { sub_iocfg(MODE_RXData, (ot_u8*)gdo_rxdata); }
void cc1101_iocfg_txdata()  { sub_iocfg(MODE_TXData, (ot_u8*)gdo_txdata); }
void cc1101_iocfg_txcsma()  { sub_iocfg(MODE_CSMA,   (ot_u8*)gdo_txcsma); }

#else

void sub_iocfg(CC1101_IMode imode, ot_u8 gdo_cfg) {
    EXTI->PR        = RFI_ALL;
    cc1101.imode    = imode;
    cc1101_write(RF_IOCFG0, gdo_cfg);
}

void cc1101_iocfg_listen()  {
    sub_iocfg(MODE_Listen, GDO_SYNCDETECT);
}

void cc1101_iocfg_rxdata()  {
    sub_iocfg(MODE_RXData, GDO_RXFHIGH);
}

void cc1101_iocfg_rxend()   {
    sub_iocfg(MODE_RXData, _N(GDO_SYNCDETECT));
}

void cc1101_iocfg_txdata()  {
    sub_iocfg(MODE_TXData, _N(GDO_TXFHIGH));
}

void cc1101_iocfg_txend()   {
    sub_iocfg(MODE_TXData, GDO_PA_PD);
}

void cc1101_iocfg_txcsma()  {  }

#endif




void cc1101_int_force(ot_u16 ifg_sel)   { EXTI->SWIER |= ifg_sel; }
void cc1101_int_turnon(ot_u16 ie_sel)   { EXTI->IMR   |= ie_sel;  }
void cc1101_int_turnoff(ot_u16 ie_sel)  { EXTI->IMR   &= ~ie_sel; }


void cc1101_irq0_isr() {
    EXTI->PR = RFI_SOURCE0;
    cc1101_virtual_isr(cc1101.imode);
}

void cc1101_irq1_isr() { cc1101_virtual_isr(cc1101.imode); }

void cc1101_irq2_isr() {
    EXTI->PR = RFI_SOURCE2;
    cc1101_virtual_isr(cc1101.imode+1);
}


void cc1101_globalirq_isr() { 
    ot_int irq_sel;
    
    irq_sel     = ((EXTI->PR & RFI_SOURCE0) == 0);
    EXTI->PR    = (irq_sel) ? RFI_SOURCE2 : RFI_SOURCE0;

    cc1101_virtual_isr(cc1101.imode + irq_sel);
}

void cc1101_databus_isr() { }

#endif

