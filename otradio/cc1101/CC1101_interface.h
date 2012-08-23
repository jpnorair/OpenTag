/* Copyright 2009-2012 JP Norair
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
  * @file       /otradio/cc1101/CC1101_interface.h
  * @author     JP Norair
  * @version    V1.0
  * @date       5 April 2012
  * @brief      Functions for the CC1101 transceiver interface
  * @defgroup   CC1101 (CC1101 family support)
  *
  ******************************************************************************
  */


#include "OT_types.h"
#include "CC1101_registers.h"
#include "CC1101_defaults.h"


// Enumeration used for IRQ state management.  You can ignore it.
#ifdef RADIO_IRQ2_PIN
typedef enum {
    MODE_Listen = 0,
    MODE_RXData = 2,
    MODE_TXData = 4,
    MODE_CSMA   = 6
} CC1101_IMode;
#else
typedef enum {
    MODE_Listen = 0,
    MODE_RXData = 1,
    MODE_TXData = 2,
    MODE_ERR   = 3
} CC1101_IMode;
#endif

/** @typedef cc1101_struct
  * <LI>chipstatus  (ot_u8) Stores the chip-status byte returned during SPI access </LI>
  * <LI>imode       (CC1101_IMode) Enumeration used for IRQ state management </LI>
  */
typedef struct {
    ot_u8 chipstatus;
    CC1101_IMode imode;
} cc1101_struct;

extern cc1101_struct cc1101;





/** Basic Control <BR>
  * ============================================================================
  */

/** @brief  Loads default register values into the CC1101
  * @param  None
  * @retval None
  * @ingroup CC1101
  */
void cc1101_load_defaults();


/** @brief  Performs a "soft reset" on the CC1101 core
  * @param  None
  * @retval None
  * @ingroup CC1101
  */
void cc1101_reset();


/** @brief  Performs manual calibration of CC1101 core
  * @param  None
  * @retval None
  * @ingroup CC1101
  */
void cc1101_calibrate();


/** @brief  Sends chip to idle, and waits for it to get to idle
  * @param  None
  * @retval None
  * @ingroup CC1101
  */
void cc1101_waitforidle();


/** @brief  Returns the values of MARCSTATE and PKTSTATUS registers
  * @param  None
  * @retval ot_u16      b12:8 = MARCSTATE, b7:0 = PKTSTATUS
  * @ingroup CC1101
  */
ot_u16 cc1101_rfstatus();


/** @brief  Returns the value of 16bit WOR timer
  * @param  None
  * @retval ot_u16      WORTIME1-WORTIME0 registers
  * @ingroup CC1101
  */
ot_u16 cc1101_wortime();


/** @brief  Returns the Radio Core Control Status value
  * @param  None
  * @retval ot_u16      RCCTRL1_STATUS-RCCTRL0_STATUS registers
  * @ingroup CC1101
  */
ot_u16 cc1101_rcctrl();


/** @brief  Returns Estimated Frequency Offset value from CC1101 core
  * @param  None
  * @retval ot_u8       FEQOFF_EST register value
  * @ingroup CC1101
  */
ot_u8 cc1101_estfreqoffset();


/** @brief  Returns LQI (link quality index) value
  * @param  None
  * @retval ot_u8       LQI register value
  * @ingroup CC1101
  */
ot_u8 cc1101_lqi();


/** @brief  Returns RSSI value from CC1101 core
  * @param  None
  * @retval ot_u8       RSSI register value
  * @ingroup CC1101
  */
ot_u8 cc1101_rssi();


/** @brief  Returns VCO VC DAC value from CC1101 core
  * @param  None
  * @retval ot_u8       VCO_VC_DAC register value
  * @ingroup CC1101
  */
ot_u8 cc1101_vcovcdac();


/** @brief  Returns RX BYTES value from CC1101 core
  * @param  None
  * @retval ot_u8       RXBYTES register value
  * @ingroup CC1101
  */
ot_u8 cc1101_rxbytes();


/** @brief  Returns TX BYTES value from CC1101 core
  * @param  None
  * @retval ot_u8       TXBYTES register value
  * @ingroup CC1101
  */
ot_u8 cc1101_txbytes();






/** Bus interface (SPI + 2x GDO) <BR>
  * ============================================================================
  */

/** @brief  Initialize the bus interface of the CC1101
  * @param  None
  * @retval None
  * @ingroup CC1101
  * @sa radio_init()
  *
  * This function does not set the default registers.  That is done in the 
  * generic radio module function, radio_init().  This function needs to be run
  * before radio_init(), since radio_init() requires the bus.  Best practice is
  * to actually call this function inside radio_init(), at the beginning.
  */
void cc1101_init_bus();



/** @brief  Causes the System to wait until the SPI bus is fully de-asserted
  * @param  None
  * @retval None
  * @ingroup CC1101
  */
void cc1101_spibus_wait();



/** @brief  Master function for SPI bus I/O
  * @param  cmd_len         (ot_u8) length in bytes of the command (write data)
  * @param  resp_len        (ot_u8) length in bytes of the response (read data)
  * @param  data            (ot_u8*) the data buffer to TX and RX
  * @retval none
  * @ingroup CC1101
  * @sa radio_init()
  *
  * cc1101_spibus_io() can be used for any sort of SPI-based IO to the CC1101.
  *
  * The user is responsible for doing any double buffering that might be
  * necessary.  In other words, cc1101_spibus_io() MAY clobber your input data
  * (the TX part).  The higher level read & write functions (below) will do
  * the double buferring when necessary.
  */
void cc1101_spibus_io(ot_u8 cmd_len, ot_u8 resp_len, ot_u8* cmd, ot_u8* resp);



/** @brief  Sends a one-byte command strobe to the CC1101 via SPI
  * @param  strobe      (ot_u8) Strobe Address
  * @retval none
  * @ingroup CC1101
  */
void cc1101_strobe(ot_u8 strobe);



/** @brief  Reads one byte of data from an unbanked, addressed register
  * @param  addr        (ot_u8) Register address
  * @retval ot_u8       read data
  * @ingroup CC1101
  */
ot_u8 cc1101_read(ot_u8 addr);



/** @brief  Burst read (multiple bytes) from addressed register
  * @param  start_addr  (ot_u8) Start Register address (must be shifted)
  * @param  length      (ot_u8) Burst data length in bytes
  * @param  data        (ot_u8*) Data buffer to read data into
  * @retval none
  * @ingroup CC1101
  *
  * This function is nearly identical to cc1101_read().  The differences
  * should be self-explanatory.
  */
void cc1101_burstread(ot_u8 start_addr, ot_u8 length, ot_u8* data);



/** @brief  Writes one byte of data to an addressed register
  * @param  addr        (ot_u8) Register address
  * @param  data        (ot_u8) Data to write to register
  * @retval none
  * @ingroup CC1101
  */
void cc1101_write(ot_u8 addr, ot_u8 data);



/** @brief  Burst write (multiple bytes) to registers
  * @param  length      (ot_u8) Burst data length in bytes
  * @param  cmd_data    (ot_u8*) address + write data
  * @retval none
  * @ingroup CC1101
  *
  * The cmd_data argument must contain the register address at position 0.
  * Therefore, the length parameter will be write-data + 1.
  */
void cc1101_burstwrite(ot_u8 length, ot_u8* cmd_data);







/** Advanced Configuration <BR>
  * ========================================================================<BR>
  */

/** @brief  Converts a DASH7-spec RSSI threshold into CC1101 HW CS value
  * @param  input       (ot_u8) RSSI Threshold from DASH7
  * @param  offset      (ot_u8) an offset from the threshold
  * @retval ot_u8       Value that can be written to CC1101 AGCCTRL b7:3 for CS
  * @ingroup CC1101
  */
ot_u8 cc1101_calc_rssithr(ot_u8 input, ot_u8 offset);



/** @brief  Computes a signed-integer RSSI value from CC1101 encoded value
  * @param  encoded_value (ot_u8) CC1101 encoded RSSI
  * @retval ot_int         RSSI as signed integer
  * @ingroup CC1101
  */
ot_int cc1101_calc_rssi(ot_u8 encoded_value);



/** @brief Sets the TX output power based on input DASH7 Power Code
  * @param pwr_code     (ot_u8) pwr_code = 2*((dBm EIRP) + 40dBm)
  * @retval none
  * @ingroup CC1101
  *
  * This function will set the target power in PA TABLE 0, and it will fill the
  * PA TABLE slots 1-7 with ramped-down powers so that the TX ramp-up/ramp-down
  * is nice and smooth.
  */
void cc1101_set_txpwr(ot_u8 pwr_code);






/** Common GDO setup & interrupt functions <BR>
  * ========================================================================<BR>
  * Your radio ISR function should be of the type void radio_isr(ot_u8), as it 
  * will be a soft ISR.  The input parameter is an interrupt vector.  The vector
  * values are shown below:
  *
  * IMode = 0   RF SYNC:            0
  * (Listen)    RX GOTO IDLE:       1       (GDO_LNA_PD)
  *
  * IMode = 2   RF RX ERR:          2
  * (RX Data)   RF RXFIFOTHR/END:   3
  *
  * IMode = 4   RF TXERR:           4
  * (TX Data)   RF TXFIFOTHR:       5
  *
  * IMode = 6   RF CS:              6
  * (CCA)       RF CCA:             7
  */

#define RFINT(VAL)          RFI_##VAL

#define RFI_SOURCE0         (1 << RADIO_IRQ0_SRCLINE)
#define RFI_SOURCE1         0 //(1 << RADIO_IRQ1_SRCLINE)

#ifdef RADIO_IRQ2_PIN
#   define RFI_SOURCE2         (1 << RADIO_IRQ2_SRCLINE)
#	define RFI_ALL             (RFI_SOURCE0 | RFI_SOURCE1 | RFI_SOURCE2)
#	define RFI_SYNC            RFI_SOURCE0
#	define RFI_RXIDLE          RFI_SOURCE2
#	define RFI_RXEND           RFI_SOURCE0
#   define RFI_RXFIFOTHR	   RFI_SOURCE2
#	define RFI_TXIDLE          RFI_SOURCE0
#	define RFI_TXFIFOTHR       RFI_SOURCE2
#	define RFI_CS              RFI_SOURCE0
#	define RFI_CCA             RFI_SOURCE2
#	define RFIV_SYNC           0
#	define RFIV_RXIDLE         1
#	define RFIV_RXEND          2
#	define RFIV_RXFIFOTHR      3
#	define RFIV_TXIDLE         4
#	define RFIV_TXFIFOTHR      5
#	define RFIV_CS             6
#	define RFIV_CCA            7

#else
#   define RFI_SOURCE2         0
#	define RFI_RXIDLE		   0
#	define RFI_TXFIFOTHR	   0

#	define RFI_ALL             (RFI_SOURCE0)
#	define RFI_SYNC            RFI_SOURCE0
#	define RFI_RX              RFI_SOURCE0
#	define RFI_RXFIFOTHR       RFI_RX
#	define RFI_TX              RFI_SOURCE0
#	define RFI_ERR             RFI_SOURCE0
#	define RFIV_SYNC           0
#	define RFIV_RX             1
#	define RFIV_TX             2
#	define RFIV_ERR            3



#endif


  
/** @brief  Configures GDOs for startup: Chip Ready + HiZ
  * @param  None
  * @retval None
  * @ingroup CC1101
  */ 
void cc1101_iocfg_startup();

/** @brief  Configures GDOs for listen: RX Sync + RX Idle
  * @param  None
  * @retval None
  * @ingroup CC1101
  */ 
void cc1101_iocfg_listen();

/** @brief  Configures GDOs for RX'ing: RX-end (invert Sync) + RX FIFO thresh
  * @param  None
  * @retval None
  * @ingroup CC1101
  */ 
void cc1101_iocfg_rxdata();

/** @brief  Configures GDOs for TX CSMA: Carrier Sense + Clear Channel Assement
  * @param  None
  * @retval None
  * @ingroup CC1101
  */ 
void cc1101_iocfg_txcsma();

/** @brief  Configures GDOs for TX'ing: TX FIFO underflow + TX FIFO threshold
  * @param  None
  * @retval None
  * @ingroup CC1101
  */ 
void cc1101_iocfg_txdata();


#ifndef RADIO_IRQ2_PIN
void cc1101_iocfg_rxend();
void cc1101_iocfg_txend();
#endif





/** @brief  Forces an interrupt on masked, selected sources (sets flag high)
  * @param  ifg_sel     (ot_u16) interrupt flag select
  * @retval None
  * @ingroup CC1101
  */
void cc1101_int_force(ot_u16 ifg_sel);

/** @brief  Enables interrupt on masked, selected sources (set enable bit high)
  * @param  ie_sel     (ot_u16) interrupt enable select
  * @retval None
  * @ingroup CC1101
  */
void cc1101_int_turnon(ot_u16 ie_sel);

/** @brief  Disables interrupt on masked, selected sources (set enable bit low)
  * @param  ie_sel     (ot_u16) interrupt enable select
  * @retval None
  * @ingroup CC1101
  */
void cc1101_int_turnoff(ot_u16 ie_sel);




/** @brief  Platform independent CC1101 ISR function
  * @param  irq			(ot_u8) An IRQ code, 0-7
  * @retval None
  * @ingroup CC1101
  *
  * The CC1101 has two independent interrupt sources, and multiple states.
  * The IRQ0 and IRQ2 ISRs call this function, and supply it a value 0-7
  * depending on the active radio state.  This function is implemented in
  * radio_CC1101.c
  */
void cc1101_virtual_isr(ot_u8 irq);

/** @brief  Global ISR function for CC1101 outputs
  * @param  None
  * @retval None
  * @ingroup CC1101
  *
  * This ISR may be used if you have a system with a single IRQ for the bank of
  * CC1101 IRQ outputs.  If you have a system with individual IRQs for each of
  * the CC1101 outputs, then you should use the individual ISRs instead.
  */
void cc1101_globalirq_isr();

/** @brief  Individual ISR function for CC1101 GDO0
  * @param  None
  * @retval None
  * @ingroup CC1101
  */
void cc1101_irq0_isr();

/** @brief  Individual ISR function for CC1101 GDO1
  * @param  None
  * @retval None
  * @ingroup CC1101
  */
void cc1101_irq1_isr();

/** @brief  Individual ISR function for CC1101 GDO2
  * @param  None
  * @retval None
  * @ingroup CC1101
  */
void cc1101_irq2_isr();

/** @brief  ISR for databus (SPI) on the CC1101 (optional)
  * @param  None
  * @retval None
  * @ingroup CC1101
  */
void cc1101_databus_isr();

