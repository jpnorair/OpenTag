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
  * @file       /Platforms/MLX73290/mlx73_interface.h
  * @author     JP Norair
  * @version    V1.0
  * @date       26 November 2011
  * @brief      Functions for the MLX73xxx transceiver interface
  * @defgroup   MLX73xxx (MLX73xxx family support)
  *
  * These functions are typically implemented in Platforms/radio_MLX73xxx.c,
  * since they depend on the MCU library
  ******************************************************************************
  */


#include <otsys/types.h>
#include "mlx73xxx_registers.h"
#include "mlx73xxx_defaults.h"



///Comment this if not using the MLX Prototype
#define MLX_PROTOTYPE

///Comment this if not using the SPI DMA (experimental)
///You can give it a value to correspond the amount of bytes the buffer will have
//#define MLX73_DMA_BUFFER	16





typedef enum {
    BANK_0  = 0,
    BANK_1  = 1,
    BANK_2  = 2,
    BANK_3  = 3
} MLX73_Bank;


typedef enum {
    MODE_Listen = 1,
    MODE_CSMA   = 4,
    MODE_RXData = 7,
    MODE_TXData = 10,
	MODE_Test   = 13
} MLX73_IMode;


typedef struct {
//    ot_u8 rfstatus[2];
//    ot_u8 lfstatus;         //lf not currently implemented
    MLX73_IMode imode;
    //ot_u8 burstbuf[6];    //only needed if you use burstwrite functions (they aren't used)
} mlx73_struct;

extern mlx73_struct mlx73;





/** Basic Control <BR>
  * ============================================================================
  */

/** @brief  Loads default register values into the MLX73xxx
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_load_defaults();


/** @brief  Performs a "soft reset" on the MLX73xxx core
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_reset();


/** @brief  Gets the values of the two RF STATUS registers
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  * The result of mlx73_get_rfstatus() is that the 16 bit status data is stored
  * into the global variable mlx73.rfstatus
  */
void mlx73_get_rfstatus();


/** @brief  Gets the values of the LF STATUS register
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  * The result of mlx73_get_lfstatus() is that the 8 bit status data is stored
  * into the global variable mlx73.lfstatus
  */
void mlx73_get_lfstatus();


/** @brief  Returns the IO status register (0x35), which contains the 4x GPIO
  *         levels as well as some other information
  * @param  None
  * @retval ot_u8       IOSTATUS register value
  * @ingroup MLX73xxx
  */
ot_u8 mlx73_iostatus();


/** @brief  Returns the power status register (0/0x03), which contains
  *         diagnostic information about the power supply
  * @param  None
  * @retval ot_u8       PWRSTATUS register value
  * @ingroup MLX73xxx
  */
ot_u8 mlx73_pwrstatus();



/** @brief  Returns an 8bit random number from the onboard RNG
  * @param  None
  * @retval ot_u8       8 bit random number
  * @ingroup MLX73xxx
  */
ot_u8 mlx73_random();



/** @brief  Performs manual RX calibration of MLX73xxx core
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_calibrate_rx();


/** @brief  Performs manual TX calibration of MLX73xxx core
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_calibrate_tx();


/** @brief  Performs IQ Correction calibration
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_iqcorrection();







/** Bus interface (SPI + 4x GPIO) <BR>
  * ============================================================================
  */

/** @brief  Initialize the bus interface of the MLX73xxx
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  * @sa radio_init()
  *
  * This function does not set the default registers.  That is done in the
  * generic radio module function, radio_init().  This function needs to be run
  * before radio_init(), since radio_init() requires the bus.  Best practice is
  * to actually call this function inside radio_init(), at the beginning.
  */
void mlx73_init_bus();



/** @brief  Causes the System to wait until the SPI bus is fully de-asserted
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_spibus_wait();



/** @brief  Master function for SPI bus I/O
  * @param  bank            (ot_u8) 0 if accessing unbanked, 1 if banked
  * @param  cmd_len         (ot_u8) length in bytes of the command (write data)
  * @param  resp_len        (ot_u8) length in bytes of the response (read data)
  * @param  data            (ot_u8*) the data buffer to TX and RX
  * @retval (ot_s8)			0 on success, negative on error (0x80 - 0xFF)
  * @ingroup MLX73xxx
  * @sa radio_init()
  *
  * mlx73_spibus_io() can be used for any sort of SPI-based IO to the MLX73xxx.
  * It is important to make sure that the data array includes the bank number
  * (if using banked access) the register address, and then any write data.
  *
  * The user is responsible for doing any double buffering that might be
  * necessary.  In other words, mlx73_spibus_io() MAY clobber your input data
  * (the TX part).  The higher level read & write functions (below) will do
  * the double buferring when necessary.
  */
ot_s8 mlx73_spibus_io(ot_u8 bank, ot_u8 cmd_len, ot_u8 resp_len, ot_u8* cmd, ot_u8* resp);




/** @brief  Reads one byte of data from an unbanked, addressed register
  * @param  addr        (ot_u8) Register address (must be shifted)
  * @retval ot_int      read data (non-negative) or Error code (negative)
  * @ingroup MLX73xxx
  *
  * The return value is 0-255 when everything goes as normal, or some TBD
  * negative value when something goes wrong.
  *
  * @note The MLX73xxx has 7 bit register addresses, from 0-127, and the lowest
  * bit of the 8 bit addressing is used for R/W control.  The user must shift
  * the 7 bit reg address up by one when calling this function.  However, the
  * register call macros in mlx73_registers.h will do this for you
  * automatically.
  */
ot_u8 mlx73_read(ot_u8 addr);



/** @brief  Burst read (multiple bytes) from an unbanked, addressed register
  * @param  start_addr  (ot_u8) Start Register address (must be shifted)
  * @param  length      (ot_u8) Burst data length in bytes
  * @param  data        (ot_u8*) Data buffer to read data into
  * @retval ot_int      read data (non-negative) or Error code (negative)
  * @ingroup MLX73xxx
  *
  * This function is nearly identical to mlx73_read().  The differences
  * should be self-explanatory.
  */
ot_u8 mlx73_burstread(ot_u8 start_addr, ot_u8 length, ot_u8* data);



/** @brief  Writes one byte of data to an unbanked, addressed register
  * @param  addr        (ot_u8) Register address (must be shifted)
  * @param  data        (ot_u8) Data to write to register
  * @retval ot_int      Error code
  * @ingroup MLX73xxx
  *
  * The return value is 0 when everything goes as normal, or some TBD negative
  * value when something goes wrong.
  *
  * @note The MLX73xxx has 7 bit register addresses, from 0-127, and the lowest
  * bit of the 8 bit addressing is used for R/W control.  The user must shift
  * the 7 bit reg address up by one when calling this function.  However, the
  * register call macros in mlx73_registers.h will do this for you
  * automatically.
  */
ot_u8 mlx73_write(ot_u8 addr, ot_u8 data);



/** @brief  Burst write (multiple bytes) to unbanked registers
  * @param  start_addr  (ot_u8) Start Register address (must be shifted)
  * @param  length      (ot_u8) Burst data length in bytes
  * @param  data        (ot_u8*) Data array to write to bank
  * @retval ot_int      Error code
  * @ingroup MLX73xxx
  * @sa mlx73_write_u()
  *
  * This function is nearly identical to mlx73_write().  The differences
  * should be self-explanatory.  The "data" parameter may or may not be double
  * buffered, depending on the implementation.  Double buffering is not required
  * by the spec, but it is usually necessary if you are doing in non-blocking.
  */
ot_u8 mlx73_burstwrite(ot_u8 start_addr, ot_u8 length, ot_u8* data);



/** @brief  Reads one byte of data from a banked, addressed register
  * @param  bank        (MLX73xxx_Bank) register bank (0-3)
  * @param  addr        (ot_u8) Register address (must be shifted)
  * @retval ot_int      Error Code
  * @ingroup MLX73xxx
  *
  * The return value is 0-255 when everything goes as normal, or some TBD
  * negative value when something goes wrong.
  *
  * @note The MLX73xxx has 7 bit register addresses, from 0-127, and the lowest
  * bit of the 8 bit addressing is used for R/W control.  The user must shift
  * the 7 bit reg address up by one when calling this function.  However, the
  * register call macros in mlx73_registers.h will do this for you
  * automatically.
  */
ot_u8 mlx73_read_bank(MLX73_Bank bank, ot_u8 addr);



/** @brief  Burst read (multiple bytes) from a banked, addressed register
  * @param  bank        (MLX73_Bank) register bank (0-3)
  * @param  start_addr  (ot_u8) Start Register address (must be shifted)
  * @param  length      (ot_u8) Burst data length in bytes
  * @param  data        (ot_u8*) Data buffer to read data into
  * @retval ot_int      Error Code
  * @ingroup MLX73xxx
  *
  * This function is nearly identical to mlx73_read_bank().  The differences
  * should be self-explanatory.
  */
ot_u8 mlx73_burstread_bank(MLX73_Bank bank, ot_u8 start_addr, ot_u8 length, ot_u8* data);




/** @brief  Writes one byte of data to a banked, addressed register
  * @param  bank        (MLX73_Bank) register bank (0-3)
  * @param  addr        (ot_u8) Register address (must be shifted)
  * @param  data        (ot_u8) Data to write to register
  * @retval ot_int      Error code
  * @ingroup MLX73xxx
  *
  * The return value is 0 when everything goes as normal, or some TBD negative
  * value when something goes wrong.
  *
  * @note The MLX73xxx has 7 bit register addresses, from 0-127, and the lowest
  * bit of the 8 bit addressing is used for R/W control.  The user must shift
  * the 7 bit reg address up by one when calling this function.  However, the
  * register call macros in mlx73_registers.h will do this for you
  * automatically.
  */
ot_u8 mlx73_write_bank(MLX73_Bank bank, ot_u8 addr, ot_u8 data);




/** @brief  Burst write (multiple bytes) to a bank
  * @param  bank        (MLX73_Bank) register bank (0-3)
  * @param  start_addr  (ot_u8) Start Register address (must be shifted)
  * @param  length      (ot_u8) Burst data length in bytes
  * @param  data        (ot_u8*) Data array to write to bank
  * @retval ot_int      Error code
  * @ingroup MLX73xxx
  * @sa mlx73_banked_w()
  *
  * This function is nearly identical to mlx73_write_bank().  The differences
  * should be self-explanatory.  The "data" parameter may or may not be double
  * buffered, depending on the implementation.  Double buffering is not required
  * by the spec, but it is usually necessary if you are doing in non-blocking.
  */
ot_u8 mlx73_burstwrite_bank(MLX73_Bank bank, ot_u8 start_addr, ot_u8 length, ot_u8* data);






/** RSSI detection <BR>
  * ========================================================================<BR>
  * The MLX73xxx has a unique RSSI mechanism.  You can get the "fresh" RSSI
  * (most recent output of the RSSI detector) or the RSSI detected when the
  * header (sync word & preamble) was found.
  */

#define mlx73_GET_FRESHRSSI()  \
    mlx73_offsetrssi( (ot_u8)mlx73_read(RFREG(RSSI)) )

#define mlx73_GET_SAVEDRSSI()  \
    mlx73_offsetrssi( (ot_u8)mlx73_read(RFREG(RSSIHDR)) )

#define mlx73_GET_RSSI       mlx73_GET_SAVEDRSSI


/** @brief  Converts the encoded RSSI value reported by MLX73xxx into signed dBm
  * @param  encoded_value   (ot_u8) RSSI as encoded by MLX73xxx
  * @retval ot_int          signed dBm of RSSI
  * @ingroup MLX73xxx
  */
ot_int mlx73_offsetrssi(ot_u8 encoded_value);


/// Value for unbanked register 0x29
ot_u8 mlx73_calc_csthr(ot_u8 cs_code);


/// Sets the RX Timer, approximately
ot_u8 mlx73_calc_rxtimeout(ot_u16 ticks);


/// Sets the Center Frequency based on DASH7 Channel Index
void mlx73_set_fc(ot_u8 ch_index);


/// Sets the TX output power
///pwr_code = 2*((dBm EIRP) + 40dBm)
void mlx73_set_txpwr(ot_u8 pwr_code);






/** Common GPIO register setup & interrupt functions <BR>
  * ========================================================================<BR>
  * Your radio ISR function should be of the type void radio_isr(ot_u8), as it
  * will be a soft ISR.  The input parameter is an interrupt vector.  The vector
  * values are shown below:
  *
  * OFFSET=0    RF XTALRDY:     0
  * (Listen)    TMR FLAG:       1
  *             RF RX PAYLOAD:  2
  *             RF RSSICS:      3
  *
  * OFFSET=3    RF RXPKTDONE:   4
  * (RX Data)   RF RXFIFOTHR:   5
  *             RF RXFIFOERR:   6
  *
  * OFFSET=3    RF RXPKTDONE:   4
  * (RX Data)   RF RXFIFOTHR:   5
  *             RF RXFIFOERR:   6

  *
  * OFFSET=6    RF PKTDONE:     7
  * (TX Data)   RF TXFIFOTHR:   8
  *             RF TXFIFOERR:   9
  */

#define RFINT(VAL)          RFI_##VAL

#define RFI_SOURCE0         (1 << RADIO_IRQ0_SRCLINE)
#define RFI_SOURCE1         (1 << RADIO_IRQ1_SRCLINE)
#define RFI_SOURCE2         (1 << RADIO_IRQ2_SRCLINE)
#define RFI_SOURCE3         (1 << RADIO_IRQ3_SRCLINE)
#define RFI_ALL             (RFI_SOURCE0 | RFI_SOURCE1 | RFI_SOURCE2 | RFI_SOURCE3)

#define RFI_XTALRDY         RFI_SOURCE0
#define RFI_TMRFLAG         RFI_SOURCE1
#define RFI_PAYLOADRX       RFI_SOURCE2
#define RFI_RSSICS          RFI_SOURCE3
#define RFI_RXPKTERR        RFI_SOURCE1
#define RFI_PKTDONE         RFI_SOURCE2
#define RFI_RXFIFOTHR       RFI_SOURCE3

#define RFI_TXPAYLOAD       RFI_SOURCE1
#define RFI_TXFIFOTHR       RFI_SOURCE2
#define RFI_TXFIFOERR       RFI_SOURCE3

#define RFIV_XTALRDY        0
#define RFIV_TMRFLAG        1
#define RFIV_PAYLOADRX      2
#define RFIV_RSSICS         3

#define RFIV_CCAEXP         4
#define RFIV_CCAERR         5
#define RFIV_CCAFAIL        6

#define RFIV_RXPKTERR       7
#define RFIV_RXPKTDONE      8
#define RFIV_RXFIFOTHR      9

#define RFIV_TXPKTDONE      10
#define RFIV_TXFIFOTHR      11
#define RFIV_TXFIFOERR      12


/** @brief  Configures 4x GPIO signals for signal listening
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  *
  * Listening includes a listen timeout (needs to be put in separately),
  * notification that a decodable signal has been detected, and notification
  * that any signal has been detected (decodable or not).
  *
  * Typical Sync detection GPIO setup:
  * XTAL_RDY (goes high when XTAL is ready) +
  * TMR_FLAG (timer expired) +
  * RFRX_PAYLOAD (sync word detected successfully) +
  * RFRX_RSSICS (carrier sensed, used on background rx) +
  */
void mlx73_iocfg_listen();



/** @brief  Configures 4x GPIO signals for Data RX
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  *
  * Data RX includes a notification when the RX FIFO could use downloading, a
  * notification when the packet is done (for modes where MLX73xxx uses its
  * packet control), and a notification when the FIFO is underflowed (for modes
  * where the HW packet control is not used)
  *
  * Typical Data RX GPIO setup:
  * XTAL_RDY (goes high when XTAL is ready) +
  * RF_PKTDONE (packet fully received) +
  * RFRX_FIFO64BYTES (rising edge means it's time to do some decoding) +
  * RFRX_FIFOERR (goes high when underflow/overflow)
  */
void mlx73_iocfg_rxdata();



/** @brief  Configures 4x GPIO signals for TX CSMA
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  *
  * TX CSMA is the same as listening.  It is "listen-before-talk."
  *
  * Typical Data TX-CSMA GPIO setup: (same as listen mode)
  */
void mlx73_iocfg_txcsma();



/** @brief  Configures 4x GPIO signals for Data TX
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  *
  * Data TX includes a notification when the TX FIFO could use uploading, a
  * notification when the packet is done (for modes where MLX73xxx uses its
  * packet control), and a notification when the FIFO is underflowed (for modes
  * where the HW packet control is not used)
  *
  * Typical Data TX GPIO setup: (similar to RX mode)
  * XTAL_RDY (goes high when XTAL is ready) +
  * RF_PKTDONE (packet fully transmitted) +
  * RFTX_FIFO64BYTES (falling edge means it's time to do some encoding) +
  * RFTX_FIFOERR (goes high when underflow/overflow)
  */
void mlx73_iocfg_txdata();



/** @brief  Configures interrupts for listen mode
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_intcfg_listen();

/** @brief  Configures interrupts for Data RX mode
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_intcfg_rxdata();

/** @brief  Configures interrupts for TX CSMA mode
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_intcfg_txcsma();

/** @brief  Configures interrupts for Data TX mode
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_intcfg_txdata();



/** @brief  Forces an interrupt on masked, selected sources (sets flag high)
  * @param  ifg_sel     (ot_u16) interrupt flag select
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_int_force(ot_u16 ifg_sel);

/** @brief  Enables interrupt on masked, selected sources (set enable bit high)
  * @param  ie_sel     (ot_u16) interrupt enable select
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_int_turnon(ot_u16 ie_sel);

/** @brief  Disables interrupt on masked, selected sources (set enable bit low)
  * @param  ie_sel     (ot_u16) interrupt enable select
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_int_turnoff(ot_u16 ie_sel);




/** @brief  Global ISR function for MLX73xxx outputs
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  *
  * This ISR may be used if you have a system with a single IRQ for the bank of
  * MLX73xxx IRQ outputs.  If you have a system with individual IRQs for each of
  * the MLX73xxx outputs, then you should use the individual ISRs instead.
  */
void mlx73_globalirq_isr();

/** @brief  Individual ISR function for MLX73xxx output 0
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_irq0_isr();

/** @brief  Individual ISR function for MLX73xxx output 1
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_irq1_isr();

/** @brief  Individual ISR function for MLX73xxx output 2
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_irq2_isr();

/** @brief  Individual ISR function for MLX73xxx output 3
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_irq3_isr();

/** @brief  ISR for databus (SPI) on the MLX73xxx
  * @param  None
  * @retval None
  * @ingroup MLX73xxx
  */
void mlx73_databus_isr();

