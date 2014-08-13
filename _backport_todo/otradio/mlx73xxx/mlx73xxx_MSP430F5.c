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
  * @file       /Platforms/MLX73290/mlx73_STM32.h
  * @author     JP Norair
  * @version    V1.0
  * @date       26 November 2011
  * @brief      MLX73xxx transceiver interface using STM32 MCU
  * @ingroup    MLX73xxx
  *
  ******************************************************************************
  */

#include "OT_config.h"
#include "OT_platform.h"
#include "OT_utils.h"
#include "OT_types.h"

#include "mlx73xxx_interface.h"

//For test
#include "OTAPI.h"






// If you want to reach high SPI speeds or do non-blocking SPI access to the MLX
// you need to use the DMA, and it needs to be double buffered
#ifdef MLX73_DMA_BUFFER
    ot_u8 __dma_buffer[MLX73_DMA_BUFFER];
#endif





#if (ISR_EMBED(RADIO) == ENABLED)

#if (((RADIO_IRQ0_SRCLINE >= 5) && (RADIO_IRQ0_SRCLINE <= 9)) || \
     ((RADIO_IRQ1_SRCLINE >= 5) && (RADIO_IRQ1_SRCLINE <= 9)) || \
     ((RADIO_IRQ2_SRCLINE >= 5) && (RADIO_IRQ2_SRCLINE <= 9)) || \
     ((RADIO_IRQ3_SRCLINE >= 5) && (RADIO_IRQ3_SRCLINE <= 9)))
void EXTI9_5_IRQHandler(void) { mlx73_globalirq_isr(); }
#endif

#if (((RADIO_IRQ0_SRCLINE >= 10) && (RADIO_IRQ0_SRCLINE <= 15)) || \
     ((RADIO_IRQ1_SRCLINE >= 10) && (RADIO_IRQ1_SRCLINE <= 15)) || \
     ((RADIO_IRQ2_SRCLINE >= 10) && (RADIO_IRQ2_SRCLINE <= 15)) || \
     ((RADIO_IRQ3_SRCLINE >= 10) && (RADIO_IRQ3_SRCLINE <= 15)))
void EXTI15_10_IRQHandler(void) { mlx73_globalirq_isr(); }
#endif

#if (RADIO_IRQ0_SRCLINE == 0)
void EXTI0_IRQHandler(void) { mlx73_irq0_isr(); }

#elif (RADIO_IRQ0_SRCLINE == 1)
void EXTI1_IRQHandler(void)  { mlx73_irq0_isr(); }

#elif (RADIO_IRQ0_SRCLINE == 2)
void EXTI2_IRQHandler(void)  { mlx73_irq0_isr(); }

#elif (RADIO_IRQ0_SRCLINE == 3)
void EXTI3_IRQHandler(void)  { mlx73_irq0_isr(); }

#elif (RADIO_IRQ0_SRCLINE == 4)
void EXTI4_IRQHandler(void)  { mlx73_irq0_isr(); }

#endif

#if (RADIO_IRQ1_SRCLINE == 0)
void EXTI0_IRQHandler(void)  { mlx73_irq1_isr(); }

#elif (RADIO_IRQ1_SRCLINE == 1)
void EXTI1_IRQHandler(void)  { mlx73_irq1_isr(); }

#elif (RADIO_IRQ1_SRCLINE == 2)
void EXTI2_IRQHandler(void)  { mlx73_irq1_isr(); }

#elif (RADIO_IRQ1_SRCLINE == 3)
void EXTI3_IRQHandler(void)  { mlx73_irq1_isr(); }

#elif (RADIO_IRQ1_SRCLINE == 4)
void EXTI4_IRQHandler(void)  { mlx73_irq1_isr(); }

#endif

#if (RADIO_IRQ2_SRCLINE == 0)
void EXTI0_IRQHandler(void)  { mlx73_irq2_isr(); }

#elif (RADIO_IRQ2_SRCLINE == 1)
void EXTI1_IRQHandler(void)  { mlx73_irq2_isr(); }

#elif (RADIO_IRQ2_SRCLINE == 2)
void EXTI2_IRQHandler(void)  { mlx73_irq2_isr(); }

#elif (RADIO_IRQ2_SRCLINE == 3)
void EXTI3_IRQHandler(void)  { mlx73_irq2_isr(); }

#elif (RADIO_IRQ2_SRCLINE == 4)
void EXTI4_IRQHandler(void)  { mlx73_irq2_isr(); }

#endif



#if (RADIO_IRQ3_SRCLINE == 0)
void EXTI0_IRQHandler(void)  { mlx73_irq3_isr(); }

#elif (RADIO_IRQ3_SRCLINE == 1)
void EXTI1_IRQHandler(void)  { mlx73_irq3_isr(); }

#elif (RADIO_IRQ3_SRCLINE == 2)
void EXTI2_IRQHandler(void)  { mlx73_irq3_isr(); }

#elif (RADIO_IRQ3_SRCLINE == 3)
void EXTI3_IRQHandler(void)  { mlx73_irq3_isr(); }

#elif (RADIO_IRQ3_SRCLINE == 4)
void EXTI4_IRQHandler(void)  { mlx73_irq3_isr(); }

#endif


//Legacy defines
void IRQ0_ISR() { mlx73_irq0_isr(); }
void IRQ1_ISR() { mlx73_irq1_isr(); }
void IRQ2_ISR() { mlx73_irq2_isr(); }
void IRQ3_ISR() { mlx73_irq3_isr(); }


#endif // end of ISR_EMBED(RADIO) part




mlx73_struct mlx73;




/** Basic Control <BR>
  * ============================================================================
  */

void mlx73_coredump(ot_int bank) {
///debugging function to dump-out register values of RF core
    ot_int i;
    
    i = (bank >= 0) ? 0 : 4;

    for (; i<255; i+=2) {
        ot_u8 output[6];
        ot_u8 regaddr;
        ot_u8 regval; 
        regaddr     = (ot_u8)i;
    
        if (bank >= 0)  regval = mlx73_read_bank(BANK_0, regaddr);
        else            regval = mlx73_read(regaddr);
        
        regaddr   >>= 1;
        output[0]   = regaddr >> 4;
        output[0]  += (output[0] < 10) ? '0' : ('A'-10);
        output[1]   = regaddr & 0xF;
        output[1]  += (output[1] < 10) ? '0' : ('A'-10);
        output[2]   = ':';
        output[3]   = regval >> 4;
        output[3]  += (output[3] < 10) ? '0' : ('A'-10);
        output[4]   = regval & 0xF;
        output[4]  += (output[4] < 10) ? '0' : ('A'-10);
    
        otapi_log_msg(4, 5, "DUMP", output);
        platform_swdelay_ms(5);
    }
}



#ifdef MLX_PROTOTYPE
void sub_prototype_defaults() {
    static const ot_u8 unbanked1[] = { 0x02<<1, \
/*        0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F  */
/*00*/              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
/*10*/  0xc8, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
/*20*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    static const ot_u8 unbanked2[] = { 0x34<<1, \
/*        0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F  */
/*30*/                          0x00, 0xad, 0x00, 0x00, 0x12, 0x13, 0x34, 0x14, 0x00, 0x00, 0x00, 0x00, \
/*40*/  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
/*50*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00 }; 

    static const ot_u8 unbanked3[] = { 0x60<<1, \
/*        0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F  */
/*60*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
/*70*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11 };
    
    static const ot_u8 bank0_all[] = { 0x00, 0x00, \
/*        0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F  */
/*00*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x12, 0x01, \
/*10*/  0x00, 0x00, 0x00, 0x07, 0x02, 0x30, 0x00, 0x00, 0x00, 0x00, 0x77, 0x33, 0x33, 0x33, 0x33, 0x33, \
/*20*/  0xaa, 0xea, 0xea, 0x77, 0xf7, 0x54, 0x54, 0x00, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, \
/*30*/  0x00, 0x33, 0x3e, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x23, 0x20, \
/*40*/  0x08, 0x10, 0x10, 0x28, 0x46, 0x64, 0x06, 0x64, 0x24, 0x55, 0x55, 0x0B, 0x00, 0x00, 0x00, 0x00, \
/*50*/  0x06, 0x19, 0x12, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
/*60*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
/*70*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    
    mlx73_spibus_io(0, sizeof(unbanked1), 0, (ot_u8*)unbanked1, NULL);
    mlx73_spibus_io(0, sizeof(unbanked2), 0, (ot_u8*)unbanked2, NULL);
    mlx73_spibus_io(0, sizeof(unbanked3), 0, (ot_u8*)unbanked3, NULL);
    mlx73_spibus_io(1, sizeof(bank0_all), 0, (ot_u8*)bank0_all, NULL);
}
#endif


void mlx73_load_defaults() {
/// Register defaults are stored in static arrays: { ADDR, VAL } 
/// <LI> Default Bank0 registers are for recommended operation of DASH7 @ 433 MHz </LI>
/// <LI> Note: some settings in Bank 0 will be improved after further characterization </LI>
/// <LI> Default Unbanked registers are for recommended operation of DASH7 @ 433 MHz,
///      and also for the radio module setup.  The radio module also sets some of the
///      unbanked registers dynamically during runtime.  These ones are noted in the
///      comments inline with the unbanked default array. </LI>
///
///@note Remember to clear the final comma in the array notation, if changing array values.

    static const ot_u8 bank0_defaults[][2] = {
      //{0x07<<1, 0x1E},   /* CRC FFFF, TI PN9 */
      //{0x0D<<1, 0x40},   /* Disable Write Protection */
      //{0x11<<1, 0x60},   /* Force On TX Power Amp and Preamp */
      //{0x12<<1, 0x10},   /* Force On the SCL Quadrature divider */
      //{0x13<<1, 0x07},   /* Enable MASH Dithering and change MASH order */
      //{0x31<<1, 0x03},   /* Use XTAL start-up current type 0 and bias current type 3 */
      //{0x32<<1, 0x3E},   /* FPGA VCO current selection */
      //{0x3C<<1, 0x63},   /* OOK mod word to max (don't care) */
      //{0x3D<<1, 0x20},   /* Enable OOK_en direct access (don't care) */
      //{0x3F<<1, 0x00},   /* Fine Frequency recovery time constant = 255 (max) */
      //{0x40<<1, 0x00},   /* Rough Frequency recovery time constant = 255 (max) */
      //{0x41<<1, 0x80},   /* Symbol Clock Recovery time constant = 32 */
      //{0x42<<1, 0x80},   /* Data rate recovery time constant = 255 (max) */
        {0x43<<1, 0x4B},   /* Fine carrier recovery amplitude setting 0 */
        {0x44<<1, 0x4B},   /* Fine carrier recovery amplitude setting 1 */ 
        {0x45<<1, 0x4B},   /* Fine carrier recovery amplitude setting 2 */
        {0x46<<1, 0x4B},   /* ISI coefficient */
      //{0x48<<1, 0xFF},   /* Using slowest settings for PLL locking, VCO calibration, and VCO settling */
        {0x49<<1, 0xAA},   /* Preamble = AA */
        {0x4A<<1, 0x1C},   /* AGC dead zone margin = +/- 6dB, AGC bias = -4.5dB*/
        {0x4B<<1, 0x03}    /* Set AGC Mode to ON */
    };

    static const ot_u8 unbanked_defaults[][2] = {
        //{ RFREG(AFC_L),         RFDEF(AFC_L) }, 
        //{ RFREG(AFC_H),         RFDEF(AFC_H) }, 
        //{ RFREG(IQCORR_GAIN),   RFDEF(IQCORR_GAIN) }, 
        //{ RFREG(IQCORR_PHASE),  RFDEF(IQCORR_PHASE) }, 
        //{ RFREG(DROFFSET),      RFDEF(DROFFSET) }, 
        { RFREG(LNACFG),        RFDEF(LNACFG) },            // Will prob need to go into chan selection
        { RFREG(FILTER_GAIN),   RFDEF(FILTER_GAIN) },       // Will need to go into chan selection
        { RFREG(RXDSPCFG),      RFDEF(RXDSPCFG) }, 
        //{ RFREG(RXPWR1),        RFDEF(RXPWR1) },
        //{ RFREG(RXPWR2),        RFDEF(RXPWR2) },
        //{ RFREG(TXPWR1),        RFDEF(TXPWR1) },          // Set in channel selection
        //{ RFREG(TXPWR2),        RFDEF(TXPWR2) },          // Set in channel selection
        //{ RFREG(PATTERN0),      RFDEF(PATTERN0) },        // Set in RX Init
        //{ RFREG(PATTERN1),      RFDEF(PATTERN1) },        // Set in RX Init
        //{ RFREG(PATTERN2),      RFDEF(PATTERN2) },        // Set in RX Init
        //{ RFREG(PATTERN3),      RFDEF(PATTERN3) },        // Set in RX Init
#   if ((M2_FEATURE(FEC) != ENABLED) && (M2_FEATURE(MULTIFRAME) != ENABLED))
        { RFREG(PKTCFG0),       RFDEF(PKTCFG0) }, 
#   endif
        //{ RFREG(PKTCFG1),       RFDEF(PKTCFG1) },         // set in buffer config
        //{ RFREG(FREQ0),         RFDEF(FREQ0) },           // Set in channel selection
        //{ RFREG(FREQ1),         RFDEF(FREQ1) },           // Set in channel selection
        //{ RFREG(FREQ2),         RFDEF(FREQ2) },           // Set in channel selection
        //{ RFREG(FREQ3),         RFDEF(FREQ3) },           // Set in channel selection
        { RFREG(MDMCFG0),       RFDEF(MDMCFG0) }, 
        //{ RFREG(MDMCFG1),       RFDEF(MDMCFG1) },         // Set in channel selection
        //{ RFREG(MDMCFG2),       RFDEF(MDMCFG2) },         // Set in channel selection
        { RFREG(MULTCFG),       RFDEF(MULTCFG) }, 
        { RFREG(TXRAMP),        RFDEF(TXRAMP) }, 
        //{ RFREG(PREAMBLE),      RFDEF(PREAMBLE_NORMAL) },        // Set in channel selection (length)
        //{ RFREG(DEMODCFG),      RFDEF(DEMODCFG_NORMAL) },  // Set in channel selection
#   if (M2_FEATURE(FEC) != ENABLED)
        { RFREG(DATACFG),       RFDEF(DATACFG_NONFEC) },
#   endif
        { RFREG(MODCFG),        RFDEF(MODCFG) }, 
        //{ RFREG(CSCFG),         RFDEF(CSCFG) },           // Set in channel selection
        //{ RFREG(RXTIMEOUT),     RFDEF(RXTIMEOUT) },       // Set during RX init
        { RFREG(MCSM0),         RFDEF(MCSM0) }, 
        { RFREG(PWRUPCFG),      RFDEF(PWRUPCFG) },
        { RFREG(MCSM1),         RFDEF(MCSM1) },           // Uses chip reset defaults
        //{ RFREG(GPIODRV),       RFDEF(GPIODRV) },         // Uses chip reset defaults
        { RFREG(GPIO0CFG),      RFDEF(GPIO0CFG) }        // RF_XTALRDY
        //{ RFREG(GPIO1CFG),      RFDEF(GPIO1CFG) },        // Uses chip reset defaults
        //{ RFREG(GPIO2CFG),      RFDEF(GPIO2CFG) },        // Uses chip reset defaults
        //{ RFREG(GPIO3CFG),      RFDEF(GPIO3CFG) },        // Uses chip reset defaults
        //{ RFREG(ADCCFG0),       RFDEF(ADCCFG0) },         // ADC not currently used
        //{ RFREG(ADCCFG1),       RFDEF(ADCCFG1) },         // ADC not currently used
        //{ RFREG(TMRCFG0),       RFDEF(TMRCFG0) },         // Uses chip reset defaults  
        //{ RFREG(SYSTIMECFG3),   RFDEF(SYSTIMECFG3) },     // System time not currently used
        //{ RFREG(POLLOFFSET0),   RFDEF(POLLOFFSET0) },     // HW Poll not currently used
        //{ RFREG(POLLOFFSET1),   RFDEF(POLLOFFSET1) },     // HW Poll not currently used
        //{ RFREG(POLLPERIOD0),   RFDEF(POLLPERIOD0) },     // HW Poll not currently used
        //{ RFREG(POLLPERIOD1),   RFDEF(POLLPERIOD1) },     // HW Poll not currently used
        //{ RFREG(POLLCFG),       RFDEF(POLLCFG) },         // HW Poll not currently used
        //{ RFREG(RCOSCCFG0),     RFDEF(RCOSCCFG0) },       // Uses chip reset defaults
        //{ RFREG(RCOSCCFG1),     RFDEF(RCOSCCFG1) },       // Uses chip reset defaults 
        //{ LFREG(TUNING1),       LFDEF(TUNING1) },         // LF not implemented yet
        //{ LFREG(TUNING2),       LFDEF(TUNING2) },         // LF not implemented yet
        //{ LFREG(TUNING3),       LFDEF(TUNING3) },         // LF not implemented yet
        //{ LFREG(MANUALCFG),     LFDEF(MANUALCFG) },       // LF not implemented yet
        //{ LFREG(HEADER0),       LFDEF(HEADER0) },         // LF not implemented yet
        //{ LFREG(HEADER1),       LFDEF(HEADER1) },         // LF not implemented yet
        //{ LFREG(HEADER2),       LFDEF(HEADER2) },         // LF not implemented yet
        //{ LFREG(HEADER3),       LFDEF(HEADER3) },         // LF not implemented yet
        //{ LFREG(MDMCFG0),       LFDEF(MDMCFG0) },         // LF not implemented yet
        //{ LFREG(MDMCFG1),       LFDEF(MDMCFG1) }          // LF not implemented yet
    };

    ot_int i;
    
#   ifdef MLX_PROTOTYPE
        /// Write Chip Defaults to the prototype board
        sub_prototype_defaults();
#   endif

    /// 3. Load Bank0 defaults for DASH7
    for (i=0; i<(sizeof(bank0_defaults)/2); i++) {
        mlx73_write_bank(BANK_0, bank0_defaults[i][0], bank0_defaults[i][1]);
    }
    
    /// 4. Load unbanked defaults for DASH7
    for (i=0; i<(sizeof(unbanked_defaults)/2); i++) {
        mlx73_write(unbanked_defaults[i][0], unbanked_defaults[i][1]);
    }

}



void mlx73_reset() {
/// Write 0x56 to this reg to soft reset (per chip spec)
/// Default GPIO0 is chip ready (RDY).  Make sure chip is ready, else
/// flag a hardware error.  (chip should be ready at startup)
/// @todo Put in a watchdog here
    mlx73_write( RFREG(CHIP_ID), 0x56 );
    while ((RADIO_IRQ0_PORT->IDR & RADIO_IRQ0_PIN) == 0);
}


void mlx73_get_rfstatus() {
//    ot_u8 addr = RFREG(STATUS0);
//    mlx73_spibus_io(0, 1, 2, &addr, mlx73.rfstatus);
}

void mlx73_get_lfstatus() {
//    mlx73_burstread(LFREG(STATUS), 1, &mlx73.lfstatus);
}

ot_u8 mlx73_iostatus() {
    return (ot_u8)mlx73_read(RFREG(IOSTATUS));
}

ot_u8 mlx73_pwrstatus() {
    return (ot_u8)mlx73_read_bank(BANK_0, B0REG(PWRSTATUS));
}

ot_u8 mlx73_check_crc() {
/// Return 0 when crc is good (similar to checking CRC value == 0)
    //return ((ot_u8)mlx73_read(RFREG(STATUS1)) & 1) - 1;

    ot_u8 status2E;
    status2E    = mlx73_read(RFREG(STATUS1));
    status2E   &= ~(7<<2);
    return (status2E == (7<<2));
}


ot_u8 mlx73_random() {
    return (ot_u8)mlx73_read(RFREG(RNG));
}


void mlx73_rfcalibrate_rx() {
    mlx73_write(RFREG(STATUS1), (RFDEF(STATUS1) & ~RFMODE) | RFMODE_MANCALRX);
}


void mlx73_calibrate_tx() {
    mlx73_write(RFREG(STATUS1), (RFDEF(STATUS1) & ~RFMODE) | RFMODE_MANCALTX);
}


void mlx73_iqcorrection() {
    mlx73_write(RFREG(GPIO0CFG), SIG(XTAL_RDY) );
    mlx73_write(RFREG(STATUS1), 0x18);
    while((RADIO_IRQ0_PORT->IDR & RADIO_IRQ0_PIN) == 0);

    mlx73_write(RFREG(GPIO0CFG), SIG(IQ_CORR_CAL) );
    mlx73_write(RFREG(DEMODCFG), (RFDEF(DEMODCFG) | CAL_IQ_CORR));
    while((RADIO_IRQ0_PORT->IDR & RADIO_IRQ0_PIN) == 0);
    
    mlx73_write(RFREG(STATUS1), 0);
}







/** Bus interface (SPI + 4x GPIO) <BR>
  * ============================================================================
  * @note MAJOR IMPORTANT NOTE: 
  * The MLX73xxx chip specifies that SPI Chip Select (SS, CS, etc) needs to be 
  * ACTIVE HIGH.  However, the Xilinx prototype platform uses ACTIVE LOW.  So, 
  * when migrating to the release chip, switch the defines below.
  */

//Swap these two (below) when going to release silicon
#define __SPI_CS_ON()   (RADIO_SPICS_PORT->ODR &= ~RADIO_SPICS_PIN)
#define __SPI_CS_OFF()  (RADIO_SPICS_PORT->ODR |= RADIO_SPICS_PIN)

//Leave these the same when going to release silicon
#define __SPI_ENABLE()  (RADIO_SPI->CR1 |= SPI_CR1_SPE)
#define __SPI_DISABLE() (RADIO_SPI->CR1 &= ~SPI_CR1_SPE)
#define __SPI_GET(VAL)  (VAL = RADIO_SPI->DR)
#define __SPI_PUT(VAL)  (RADIO_SPI->DR = VAL)

  
#if (RADIO_SPI_INDEX == 1)
#elif (RADIO_SPI_INDEX == 2)
#elif (RADIO_SPI_INDEX == 3)
#else
#   error "RADIO_SPI & RADIO_SPI_INDEX not defined as SPI1, 2, or 3."
#endif  



void sub_clear_irqs() {
/// Clear the External Interrupt lines completely.  They will be
/// configured dynamically during runtime
    EXTI->PR    =  (RFI_SOURCE0 | RFI_SOURCE1 | RFI_SOURCE2 | RFI_SOURCE3);
    EXTI->RTSR &= ~(RFI_SOURCE0 | RFI_SOURCE1 | RFI_SOURCE2 | RFI_SOURCE3);
    EXTI->FTSR &= ~(RFI_SOURCE0 | RFI_SOURCE1 | RFI_SOURCE2 | RFI_SOURCE3);
    EXTI->IMR  &= ~(RFI_SOURCE0 | RFI_SOURCE1 | RFI_SOURCE2 | RFI_SOURCE3);
    EXTI->EMR  &= ~(RFI_SOURCE0 | RFI_SOURCE1 | RFI_SOURCE2 | RFI_SOURCE3);
}



void mlx73_init_bus() {
/// @note platform_init_periphclk() should have alread enabled RADIO_SPI clock
/// and GPIO clocks

    ///1. Set-up SPI peripheral for Master Mode, Full Duplex
    RADIO_SPI->CR1      =   SPI_Direction_2Lines_FullDuplex |
                            SPI_Mode_Master | \
                            SPI_DataSize_8b | \
                            SPI_CPOL_Low | \
                            SPI_CPHA_1Edge | \
                            SPI_NSS_Soft | \
                            SPI_BaudRatePrescaler_16 | \
                            SPI_FirstBit_MSB;
    RADIO_SPI->CR2      = 0;
    RADIO_SPI->I2SCFGR  = 0;


    ///2. Set-up GPIO to match SPI & IRQ requirements.  The GPIO init routine
    ///   could be optimized quite a bit at the cost of port/pin assignment 
    ///   flexibility.  Current implementation is flexible.
    {
        GPIO_InitTypeDef GPIOinit;
#       if (RADIO_SPI_REMAP == ENABLED)
            ///@todo put remap code in here.  There is a function to call somewhere
#       endif

        GPIOinit.GPIO_Speed = GPIO_Speed_50MHz;
        GPIOinit.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIOinit.GPIO_Pin   = (RADIO_SPISCK_PIN | RADIO_SPIMOSI_PIN);
        GPIO_Init(RADIO_SPISCK_PORT, &GPIOinit);
        
        //MISO on MLX chip is tri-state when not used, so it is good to pull-up or 
        //pull-down on STM32.  Pull-up allows you to see the MISO usage on a logic
        //analyzer, when the MLX activates it and pulls the signal to ground.
        GPIOinit.GPIO_Mode  = GPIO_Mode_IPU;
        GPIOinit.GPIO_Pin   = (RADIO_SPIMISO_PIN);
        GPIO_Init(RADIO_SPISCK_PORT, &GPIOinit);
        
        GPIOinit.GPIO_Mode  = GPIO_Mode_Out_PP;
        GPIOinit.GPIO_Pin   = RADIO_SPICS_PIN;
        GPIO_Init(RADIO_SPICS_PORT, &GPIOinit); 
        __SPI_CS_OFF();
    
        GPIOinit.GPIO_Mode  = GPIO_Mode_IPD;
        GPIOinit.GPIO_Pin   = RADIO_IRQ0_PIN;
        GPIO_Init(RADIO_IRQ0_PORT, &GPIOinit);
        
        GPIOinit.GPIO_Pin   = RADIO_IRQ1_PIN;
        GPIO_Init(RADIO_IRQ1_PORT, &GPIOinit);
        
        GPIOinit.GPIO_Pin   = RADIO_IRQ2_PIN;
        GPIO_Init(RADIO_IRQ2_PORT, &GPIOinit);
        
        GPIOinit.GPIO_Pin   = RADIO_IRQ3_PIN;
        GPIO_Init(RADIO_IRQ3_PORT, &GPIOinit);
        
        //GPIO_EXTILineConfig(RADIO_IRQ0_SRCPORT, RADIO_IRQ0_SRCPIN);
        GPIO_EXTILineConfig(RADIO_IRQ1_SRCPORT, RADIO_IRQ1_SRCPIN);
        GPIO_EXTILineConfig(RADIO_IRQ2_SRCPORT, RADIO_IRQ2_SRCPIN);
        GPIO_EXTILineConfig(RADIO_IRQ3_SRCPORT, RADIO_IRQ3_SRCPIN);
    }
    
    ///3. Set-up external source interrupts on RADIO_IRQ pins.  Note that the
    ///   NVIC configuration takes place in platform_init_interruptor() in the
    ///   platform_MLX73290.c file, as that setup has ramification to other
    ///   OpenTag components (namely, which interrupts have priority).  We 
    ///   cannot set that up here because there is no knowledge of the 
    ///   requirements of the other interrupts used by OpenTag
    sub_clear_irqs();
    
    // Using Interrupt Priority 0,1 per OpenTag requirements
    NVIC->IP[(uint32_t)(RADIO_IRQ0_IRQn)]       = b0001 << (8 - __NVIC_PRIO_BITS);
    NVIC->IP[(uint32_t)(RADIO_IRQ1_IRQn)]       = b0001 << (8 - __NVIC_PRIO_BITS);
    NVIC->IP[(uint32_t)(RADIO_IRQ2_IRQn)]       = b0001 << (8 - __NVIC_PRIO_BITS);
    NVIC->IP[(uint32_t)(RADIO_IRQ3_IRQn)]       = b0001 << (8 - __NVIC_PRIO_BITS);
    NVIC->ISER[((uint32_t)(RADIO_IRQ0_IRQn)>>5)]= (1 << ((uint32_t)(RADIO_IRQ0_IRQn) & 0x1F));
    NVIC->ISER[((uint32_t)(RADIO_IRQ1_IRQn)>>5)]= (1 << ((uint32_t)(RADIO_IRQ1_IRQn) & 0x1F));
    NVIC->ISER[((uint32_t)(RADIO_IRQ2_IRQn)>>5)]= (1 << ((uint32_t)(RADIO_IRQ2_IRQn) & 0x1F));
    NVIC->ISER[((uint32_t)(RADIO_IRQ3_IRQn)>>5)]= (1 << ((uint32_t)(RADIO_IRQ3_IRQn) & 0x1F));
    
 
    ///4. Set-up Radio DMA's.  The MLX implementation requires DMAs to manage
    ///   the SPI transfer.  Suitable channels for the DMA are shown below:
    ///   
    ///   | RADIO_SPI | RADIO_DMA | RX Chan | TX Chan |
    ///   +-----------+-----------+---------+---------+
    ///   |   SPI1    |   DMA1    | Chan 2  | Chan 3  |
    ///   |   SPI2    |   DMA1    | Chan 4  | Chan 5  |
    ///   |   SPI3    |   DMA2    | Chan 1  | Chan 2  |
#   ifdef MLX73_DMA_BUFFER
    ///@note Duplex DMA SPI is a clever hack.  In order to understand what is going on,
    /// you need to understand how the STM32 SPI works and also how the MLX73 SPI works.
    /// The result is that TX puts idle-data (00/FF for MISO pull-down/pull-up) onto the
    /// line after the TX command is done, and only a single DMA buffer is needed for
    /// both RX and TX.

    RADIO_DMA_RXCHAN->CCR   =   DMA_DIR_PeripheralSRC       | \
                                DMA_Mode_Normal             | \
                                DMA_PeripheralInc_Disable   | \
                                DMA_MemoryInc_Enable        | \
                                DMA_PeripheralDataSize_Byte | \
                                DMA_MemoryDataSize_Byte     | \
                                DMA_Priority_VeryHigh       | \
                                DMA_M2M_Disable;
    
    RADIO_DMA_TXCHAN->CCR   =   DMA_DIR_PeripheralDST       | \
                                DMA_Mode_Circular           | \
                                DMA_PeripheralInc_Disable   | \
                                DMA_MemoryInc_Enable        | \
                                DMA_PeripheralDataSize_Byte | \
                                DMA_MemoryDataSize_Byte     | \
                                DMA_Priority_VeryHigh       | \
                                DMA_M2M_Disable;
    
    RADIO_DMA_RXCHAN->CPAR  = (ot_u32)&RADIO_SPI->DR;
    RADIO_DMA_TXCHAN->CPAR  = (ot_u32)&RADIO_SPI->DR;
    
    // If Interrupts are used with DMA, Priority should be 0,0
    //NVIC->IP[(uint32_t)(RADIO_DMA_RXIRQn)]          = b0000 << (8 - __NVIC_PRIO_BITS);
    //NVIC->IP[(uint32_t)(RADIO_DMA_TXIRQn)]          = b0000 << (8 - __NVIC_PRIO_BITS);
    //NVIC->ISER[((uint32_t)(RADIO_DMA_RXIRQn)>>5)]   = (1 << ((uint32_t)(RADIO_DMA_RXIRQn) & 0x1F));
    //NVIC->ISER[((uint32_t)(RADIO_DMA_TXIRQn)>>5)]   = (1 << ((uint32_t)(RADIO_DMA_TXIRQn) & 0x1F));
#   endif
    
    
}



void mlx73_spibus_wait() {
    ///@todo have a windowed-watchdog here
    while (RADIO_SPI->SR & SPI_SR_BSY);
}




ot_s8 mlx73_spibus_io(ot_u8 bank, ot_u8 cmd_len, ot_u8 resp_len, ot_u8* cmd, ot_u8* resp) {   
#ifdef MLX73_DMA_BUFFER
    ///Still experimental, use with care
    ///DMA is recommended if: Desired SPI clock < (System Clock / 16)

    /// Check to see if read/write is too big.  This is commented-out because my
    /// driver never sends too much data at one time
    //if ((cmd_len + resp_len) > MLX73_DMA_BUFFER) {
    //    return -1;
    //}

    /// If you modify this function to be non-blocking, make sure to wait
    //mlx73_spibus_wait();
    
    /// The Radio DMA's should be mostly configured via mlx73_init_bus().
    /// What remains is to make the lengths correspond.
    data[bank]             |= (resp_len != 0);  //Add read bit if required
    RADIO_DMA->IFCR         = (RADIO_DMA_TXINT | RADIO_DMA_RXINT);
    RADIO_DMA_TXCHAN->CNDTR = (ot_u16)cmd_len;
    RADIO_DMA_RXCHAN->CNDTR = (ot_u16)(cmd_len + resp_len);
    RADIO_DMA_TXCHAN->CMAR  = (ot_u32)data;
    RADIO_DMA_RXCHAN->CMAR  = (ot_u32)data;
    RADIO_DMA_TXCHAN->CCR  |= DMA_CCR1_EN;
    RADIO_DMA_RXCHAN->CCR  |= DMA_CCR1_EN;
    RADIO_SPI->CR2          = (SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx);
    __SPI_CS_ON();
    __SPI_ENABLE();
    
    //blocking: wait for RX to be done
    while ((RADIO_DMA->ISR & RADIO_DMA_RXINT) == 0);
    __SPI_CS_OFF();
    __SPI_DISABLE();
    RADIO_DMA_TXCHAN->CCR  &= ~DMA_CCR1_EN;
    RADIO_DMA_RXCHAN->CCR  &= ~DMA_CCR1_EN;
    
    return 0;

#else
    ot_u8 dummy;
    
    /// Put read bit high if required
    /// This needs to be in an "if" so that static memory can be used to write.
    if (resp_len != 0) {
        cmd[bank] |= 1;
    }
    
    /// Enable SPI and write first byte
    __SPI_CS_ON();
    __SPI_ENABLE();
    
    ///Wait for TX to be done.  RX needs to happen too (or else SPI will
    ///die), so we save it to a dummy variable.
    while (cmd_len != 0) {
        cmd_len--;
        while ((RADIO_SPI->SR & SPI_I2S_FLAG_TXE) == 0);
        __SPI_PUT(*cmd); cmd++;
        while ((RADIO_SPI->SR & SPI_I2S_FLAG_RXNE) == 0);
        __SPI_GET(dummy);
    }

    ///Wait for RX to be done.
    while (resp_len != 0) {
        resp_len--;
        while ((RADIO_SPI->SR & SPI_I2S_FLAG_TXE) == 0);
        __SPI_PUT(0);
        while ((RADIO_SPI->SR & SPI_I2S_FLAG_RXNE) == 0);
        __SPI_GET(*resp); resp++;
    }

    __SPI_CS_OFF();
    __SPI_DISABLE();
    
    return 0;
#endif
}


ot_u8 mlx73_read(ot_u8 addr) {
    ot_u8 read_data;
    mlx73_spibus_io(0, 1, 1, &addr, &read_data);
    return read_data;
}

ot_u8 mlx73_burstread(ot_u8 start_addr, ot_u8 length, ot_u8* data) {
/// Unused and deprecated for the moment
//    data--;
//    *data = start_addr;
//    return mlx73_spibus_io(0, 1, length+1, data);
}

ot_u8 mlx73_write(ot_u8 addr, ot_u8 data) {
    ot_u8 cmd[2];
    cmd[0]  = addr;
    cmd[1]  = data;
    return mlx73_spibus_io(0, 2, 0, cmd, NULL);
}

ot_u8 mlx73_burstwrite(ot_u8 length, ot_u8 start_addr, ot_u8* data) {
/// Unused and deprecated for the moment
//    data--;
//    *data = start_addr;
//    length++;
//    return mlx73_spibus_io(0, length, 0, data);
}

ot_u8 mlx73_read_bank(MLX73_Bank bank, ot_u8 addr) {
    ot_u8 cmd[2];
    ot_u8 read_data;
    cmd[0]  = (ot_u8)bank;
    cmd[1]  = addr;
    mlx73_spibus_io(1, 2, 1, cmd, &read_data);
    return read_data;
}

ot_u8 mlx73_burstread_bank(MLX73_Bank bank, ot_u8 start_addr, ot_u8 length, ot_u8* data) {
/// Unused and deprecated for the moment
//    data--; *data = start_addr;
//    data--; *data = (ot_u8)bank;
//    return mlx73_spibus_io(1, 1, length+2, data);
}

ot_u8 mlx73_write_bank(MLX73_Bank bank, ot_u8 addr, ot_u8 data) {
    ot_u8 cmd[3];
    cmd[0] = (ot_u8)bank;
    cmd[1] = addr;
    cmd[2] = data;
    return mlx73_spibus_io(1, 3, 0, cmd, NULL);
}

ot_u8 mlx73_burstwrite_bank(MLX73_Bank bank, ot_u8 start_addr, ot_u8 length, ot_u8* data) {
/// Unused and deprecated for the moment
//    data--; *data = start_addr;
//    data--; *data = (ot_u8)bank;
//    return mlx73_spibus_io(1, length+2, 0, data);
}








/** MLX73xxx Utility Functions <BR>
  * ============================================================================
  */

ot_int mlx73_offsetrssi(ot_u8 encoded_value) {
/// RSSI encoding method not yet determined
/// @todo get info from MLX and implement
    ot_int rssi_dbm;
    return rssi_dbm;
}

ot_u8 mlx73_calc_csthr(ot_u8 cs_code) {
///@todo CS RSSI THR is not characterized yet
    return 0;
}


ot_u8 mlx73_calc_rxtimeout(ot_u16 ticks) {
/// This implementation uses a fixed lookup table to reduce the amount of 
/// required division/multiplication.  The LUT is the same size as 20 CPU
/// instructions on 16 bit ISA (e.g. MSP430, ARM Thumb2)
/// Timeout = C + (C<<EXP)*MANT + (C<<EXP)*16, C = 2048/Fxtal (64us @ 32MHz)
/// Therefore, RADIO_TICKBASE = 15.259 (1 tick / 64us) @ 32 MHz
#ifndef RADIO_TICKBASE
#   define RADIO_TICKBASE 15.259
#endif
    static const ot_u16 calc_lut[20] = {   \
        1/RADIO_TICKBASE, 2/RADIO_TICKBASE, 4/RADIO_TICKBASE, 8/RADIO_TICKBASE, \
        16/RADIO_TICKBASE, 32/RADIO_TICKBASE, 64/RADIO_TICKBASE, 128/RADIO_TICKBASE, \
        256/RADIO_TICKBASE, 512/RADIO_TICKBASE, 1024/RADIO_TICKBASE, 2048/RADIO_TICKBASE, \
        4096/RADIO_TICKBASE, 81932/RADIO_TICKBASE, 16384/RADIO_TICKBASE, 32768/RADIO_TICKBASE, \
        65536/RADIO_TICKBASE, 131072/RADIO_TICKBASE, 262144/RADIO_TICKBASE, 524288/RADIO_TICKBASE \
    };
    
    ot_int exp, mant;
    ot_u8 output;

    for (exp=5; (ticks < calc_lut[exp]) && (exp < 20); exp++);
    
    exp    -= 1;
    ticks  -= calc_lut[exp];
    exp    -= 4;
    mant    = ticks / calc_lut[exp];
    output  = (exp << 4) | (mant & 0x0F);
    output += (ticks != 0);                 //round-up
    
    return output;
}



void mlx73_set_fc(ot_u8 ch_index) {
///Fc settings for DASH7 channel centers
///<PRE>
///CHAN    INT     FRAC    FRAC DELTA
///0       27      77033   0
///1       27      84111   7078
///2       27      91189   7078
///3       27      98268   7079
///4       27      105346  7078
///5       27      112424  7078
///6       27      119502  7078
///7       27      126580  7078
///8       27      133658  7078
///9       27      140736  7078
///10      27      147814  7078
///11      27      154892  7078
///12      27      161970  7078
///13      27      169048  7078
///14      27      176126  7078
///</PRE>

#   define int_pll  27
    ot_u32 frac_pll;
    ot_u8 fc_data[5];
    
    frac_pll    = 77033 + ((ot_u32)ch_index * 7078);
    frac_pll   += (ch_index > 2);       //account for rounding
    fc_data[0]  = RFREG(FREQ0);
    fc_data[1]  = (ot_u8)(frac_pll << 4);   //note: MLX73290 spec needs div/2
    fc_data[2]  = (ot_u8)(frac_pll >> 4);
    fc_data[3]  = (ot_u8)(frac_pll >> 12);
    fc_data[4]  = (ot_u8)(int_pll);

    mlx73_spibus_io(0, 5, 0, fc_data, NULL);
}


void mlx73_set_txpwr(ot_u8 pwr_code) {
///pwr_code = 2*((dBm EIRP) + 40dBm)

    /// Power characterization isn't really done yet
    ot_u8 pwr_cmd[3];
    pwr_cmd[0] = RFREG(TXPWR1);
    pwr_cmd[1] = 0x3F;  //dummy power setting for Front End 1
    pwr_cmd[2] = 0x3F;  //dummy power setting for Front End 2

    mlx73_spibus_io(0, 3, 0, pwr_cmd, NULL);
}






static const ot_u8 iocfg_listen[] = \
    { RFREG(GPIO1CFG), SIG(TMR_FLAG), SIG(RFRX_PAYLOAD), SIG(RFRX_RSSICS) };

static const ot_u8 iocfg_rxdata[] = \
    { RFREG(GPIO1CFG), SIG(RF_ERRSTOP), SIG(RF_PKTDONE), SIG(RFRX_FIFO64BYTES)  };
    
static const ot_u8 iocfg_txdata[] = \
    { RFREG(GPIO1CFG), SIG_N(RFTX_PAYLOAD), SIG_N(RFTX_FIFO64BYTES), SIG(RFTX_FIFOERR) };

static const ot_u8 iocfg_utils[] = \
    { RFREG(GPIO1CFG), SIG(PLL_LOCK), SIG_N(IQ_CORR_CAL), SIG(SYS_TIMEOVF) };

void sub_iocfg(ot_u8* iocfg_data) {
    mlx73_int_turnoff(RFI_ALL);
    mlx73_spibus_io(0, 4, 0, iocfg_data, NULL);
}

void mlx73_iocfg_listen() { sub_iocfg((ot_u8*)iocfg_listen); }
void mlx73_iocfg_rxdata() { sub_iocfg((ot_u8*)iocfg_rxdata); }
void mlx73_iocfg_txcsma() { sub_iocfg((ot_u8*)iocfg_listen); }
void mlx73_iocfg_txdata() { sub_iocfg((ot_u8*)iocfg_txdata); }



void mlx73_intcfg_listen() {
    mlx73.imode = MODE_Listen;
    sub_clear_irqs();
    EXTI->IMR  |= /*RFINT(XTALRDY) | RFINT(TMRFLAG) |*/ RFINT(PAYLOADRX) /*| RFINT(RSSICS)*/;
    EXTI->RTSR |= /*RFINT(XTALRDY) | RFINT(TMRFLAG) |*/ RFINT(PAYLOADRX) /*| RFINT(RSSICS)*/;
}

void mlx73_intcfg_txcsma() { 
    mlx73.imode = MODE_CSMA;
    sub_clear_irqs();
    EXTI->IMR  |= /*RFINT(XTALRDY) |*/ RFINT(TMRFLAG) | RFINT(PAYLOADRX) | RFINT(RSSICS);
    EXTI->RTSR |= /*RFINT(XTALRDY) |*/ RFINT(TMRFLAG) | RFINT(PAYLOADRX) | RFINT(RSSICS);
}

void mlx73_intcfg_rxdata() {
    mlx73.imode = MODE_RXData;
    sub_clear_irqs();
    EXTI->IMR  |= /*RFINT(XTALRDY) |*/ RFINT(RXPKTERR) | RFINT(PKTDONE) | RFINT(RXFIFOTHR); 
    EXTI->RTSR |= /*RFINT(XTALRDY) |*/ RFINT(RXPKTERR) | RFINT(PKTDONE) | RFINT(RXFIFOTHR); 
}

void mlx73_intcfg_txdata() { 
    mlx73.imode = MODE_TXData;
    sub_clear_irqs();
    EXTI->IMR  |= /*RFINT(XTALRDY) |*/ RFINT(TXPAYLOAD) | RFINT(TXFIFOTHR) /*| RFINT(TXFIFOERR)*/; 
    EXTI->RTSR |= /*RFINT(XTALRDY) |*/ RFINT(TXPAYLOAD) | RFINT(TXFIFOTHR) /*| RFINT(TXFIFOERR)*/;
}


void mlx73_int_force(ot_u16 ifg_sel) {  EXTI->SWIER |= ifg_sel; }
void mlx73_int_turnon(ot_u16 ie_sel) {  EXTI->IMR   |= ie_sel;  }
void mlx73_int_turnoff(ot_u16 ie_sel) { EXTI->IMR   &= ~ie_sel; }



void mlx73_irq0_isr() {
///Permanent RF Stopped Interrupt
    EXTI->PR = RFI_SOURCE0;                 // Clear pending interrupt bit
    radio_isr(0);                           // call vector 0 (rf stopped)
}

void mlx73_irq1_isr() {
///TMRFLAG (1), TMRFLAG (4), RXPKTERR (7), PKTDONE (10)
    EXTI->PR = RFI_SOURCE1;                 // Clear pending interrupt bit
    radio_isr( (ot_u8)mlx73.imode );        // call vector 1/4/7/10
}

void mlx73_irq2_isr() {
///PAYLOADRX (2), PAYLOADRX (5), RXPKTDONE (8), TXFIFOTHR (11)
    EXTI->PR = RFI_SOURCE2;                 // Clear pending interrupt bit
    radio_isr( (ot_u8)mlx73.imode + 1 );    // call vector 2/5/8/11
}

void mlx73_irq3_isr() {
///RSSICS (3), RSSICS (6), RXFIFOTHR (9), TXFIFOERR (12)
    EXTI->PR = RFI_SOURCE3;                 // Clear pending interrupt bit
    radio_isr( (ot_u8)mlx73.imode + 2 );    // call vector 3/6/9/12
}




