/* Copyright 2009-2016 JP Norair
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
  * @file       /include/io/sx127x/interface.h
  * @author     JP Norair
  * @version    R102
  * @date       27 Aug 2016
  * @brief      Functions for the SX127x transceiver interface
  * @defgroup   SX127x (SX127x family support)
  *
  ******************************************************************************
  */

#ifndef __IO_SX127X_INTERFACE_H
#define __IO_SX127X_INTERFACE_H

#include <otstd.h>
#include <io/sx127x/config.h>

///@todo if SX1272 ...
#include <io/sx127x/SX1272_registers.h>
#include <io/sx127x/SX1272_defaults.h>


#ifndef BOARD_FEATURE_RFXTALOUT
#   define BOARD_FEATURE_RFXTALOUT 0
#endif
  
#ifdef RADIO_DB_ATTENUATION
#   define _ATTEN_DB    RADIO_DB_ATTENUATION
#else
#   define _ATTEN_DB    6
#endif




/** @typedef sx127x_link
  * A data element for returning link quality parameters other than RSSI.  RSSI
  * can be retrieved by upper layers, universally for all different types of
  * RF transceivers, via radio.last_rssi. 
  */

/** @typedef sx127x_struct
  * <LI>imode   (SX127x_IMode) Enumeration used for IRQ state management </LI>
  * <LI>status  (ot_u8) Stores the 2-byte chip-status obtained on each SPI access </LI>
  * <LI>busrx   (ot_u8) scratch space for RX'ed SPI bus data </LI>
  *
  * @note The maximum SPI transfer depends on the allocation of busrx.  A DMA
  *       is used with the SPI, so it needs to dump the RX data here.  24 bytes
  *       is quite safe with the current implementation, and it is unlikely to
  *       be too small for any implementation based on the current one.
  *
  * @note The results of any SPI read will get stored in sx127x.busrx.  If you
  *       use a function in this interface library that returns a value from
  *       read data, it is returning data copied from sx127x.busrx.  Use this
  *       knowledge to optimize your code, or do hacks & tricks.
  */
  
// Enumeration used for IRQ state management.  You can ignore it.
typedef enum {
    MODE_Listen = 0,
    MODE_RXData = 1,
    MODE_CSMA   = 5,
    MODE_TXData = 6
} SX127x_IMode;

typedef struct {
#   if (BOARD_FEATURE_RFXTALOUT)
    ot_bool         clkreq;
#   endif
    SX127x_IMode    imode;
    ot_u8           status;
    ot_u8           spi_addr;
    ot_u8           busrx[24];
} sx127x_struct;

extern sx127x_struct sx127x;



/** SET_LINE and CLR_LINE macros for some profiling tasks.
  */
//#define _SX127X_PROFILE
  
#if defined(_SX127X_PROFILE)
    extern ot_s16 set_line;
    extern ot_s16 line_hits[256];
    extern ot_u16 count_hits;
    
#   define __SET_LINE(NUMBER)   (set_line = NUMBER)
#   define __CLR_LINE(NUMBER)   (set_line = -1)
#else
#   define __SET_LINE(NUMBER);
#   define __CLR_LINE(NUMBER);
#endif




/** Functions typically implemented in the radio layer module <BR>
  * ==========================================================================
  * i.e. io/sx127x/radio_rm2.c
  */
void sx127x_virtual_isr(ot_u8 code);





/** Functions implemented typically in the platform driver <BR>
  * ==========================================================================
  * i.e. /platform/stm32l1xx/io_sx127x.c
  */
void    sx127x_int_clearall(void);
ot_uint sx127x_resetpin_ishigh(void);
ot_uint sx127x_resetpin_setlow(void);
ot_uint sx127x_resetpin_sethigh(void);
ot_uint sx127x_readypin_ishigh(void);
ot_uint sx127x_cadpin_ishigh(void);


void sx127x_int_off();
void sx127x_int_on();

/** @brief  Generic Interrupt Config Function
  * @param  ie_sel     (ot_u32) interrupt enable select
  * @retval None
  * @ingroup SX127x
  */
void sx127x_int_config(ot_u32 ie_sel);


/** @brief  Forces an interrupt on masked, selected sources (sets flag high)
  * @param  ifg_sel     (ot_u16) interrupt flag select
  * @retval None
  * @ingroup SX127x
  */
void sx127x_int_force(ot_u16 ifg_sel);


/** @brief  Enables interrupt on masked, selected sources (set enable bit high)
  * @param  ie_sel     (ot_u16) interrupt enable select
  * @retval None
  * @ingroup SX127x
  */
void sx127x_int_turnon(ot_u16 ie_sel);


/** @brief  Disables interrupt on masked, selected sources (set enable bit low)
  * @param  ie_sel     (ot_u16) interrupt enable select
  * @retval None
  * @ingroup SX127x
  */
void sx127x_int_turnoff(ot_u16 ie_sel);



/** @brief  Initialize the bus interface of the SX127x
  * @param  None
  * @retval None
  * @ingroup SX127x
  * @sa radio_init()
  *
  * This function does not set the default registers.  That is done in the 
  * generic radio module function, radio_init().  This function needs to be run
  * before radio_init(), since radio_init() requires the bus.  Best practice is
  * to actually call this function inside radio_init(), at the beginning.
  */
void sx127x_init_bus();



/** @brief  Causes the System to wait until the SPI bus is fully de-asserted
  * @param  None
  * @retval None
  * @ingroup SX127x
  */
void sx127x_spibus_wait();



/** @brief  Master function for SPI bus I/O
  * @param  cmd_len         (ot_u8) length in bytes of the command (write data)
  * @param  resp_len        (ot_u8) length in bytes of the response (read data)
  * @param  cmd             (ot_u8*) the data buffer to TX
  * @retval none
  * @ingroup SX127x
  * @sa radio_init()
  *
  * sx127x_spibus_io() can be used for any sort of SPI-based IO to the SX127x.
  * The return data from the SX127x is stored in sx127x.busrx.  The allocation
  * of sx127x.busrx (typically 24 bytes) stipulates the maximum amount of data
  * that can be transfered (+1) in a single call to this function.  Therefore,
  * with 24 bytes allocated to busrx, (cmd_len + resp_len) must be less than or 
  * equal to 25 bytes.  In practice, this is more than enough for OpenTag in 
  * its current implementation.
  */
void sx127x_spibus_io(ot_u8 cmd_len, ot_u8 resp_len, ot_u8* cmd);




ot_u8 sx127x_getbasepwr();





/** Common GPIO setup & interrupt functions  <BR>
  * ========================================================================<BR>
  * Your radio ISR function should be of the type void radio_isr(ot_u8), as it
  * will be a soft ISR.  The input parameter is an interrupt vector.  The vector
  * values are shown below:
  *
  * -------------- RX MODES (set sx127x_iocfg_rx()) --------------
  * IMode = 0       CAD Done:                   0
  * (Listen)        CAD Detected:               -
  *                 Hop (Unused)                -
  *                 Valid Header:               -
  *
  * IMode = 1       RX Done:                    1
  * (RX Data)       RX Timeout:                 2
  *                 Hop (Unused)                -
  *                 Valid Header:               4
  *
  * -------------- TX MODES (set sx127x_iocfg_tx()) --------------
  * IMode = 5       CAD Done:                   5   (CCA done)
  * (CSMA)          CAD Detected:               -   (0/1 = pass/fail)
  *                 Hop (Unused)                -
  *                 Valid Header                -
  *
  * IMode = 6       TX Done:                    6
  * (TX)            
  */

#define RFINT(VAL)      RFI_##VAL

#define RFI_SOURCE0     RADIO_IRQ0_PIN
#define RFI_SOURCE1     RADIO_IRQ1_PIN
#define RFI_SOURCE2     0   // Not used with this driver
#define RFI_SOURCE3     RADIO_IRQ3_PIN   // Not used with this driver
#define RFI_SOURCE4     0   // Not used with this driver
#define RFI_SOURCE5     0   // Not used with this driver

#define RFI_ALL         (RFI_SOURCE0 | RFI_SOURCE1 | RFI_SOURCE2 | RFI_SOURCE3 | RFI_SOURCE4 | RFI_SOURCE5)

#define RFI_CADDONE     RFI_SOURCE0
#define RFI_RXDONE      RFI_SOURCE0
#define RFI_RXTIMEOUT   RFI_SOURCE1
#define RFI_RXHEADER    RFI_SOURCE3
#define RFI_TXDONE      RFI_SOURCE0

#define RFI_LISTEN      (RFI_CADDONE)
#define RFI_RXDATA      (RFI_RXDONE | RFI_RXTIMEOUT | RFI_RXHEADER)
#define RFI_CSMA        (RFI_CADDONE)
#define RFI_TXDATA      (RFI_TXDONE)

#define RFIV_LISTEN     0
#define RFIV_RXDONE     1
#define RFIV_RXTIMEOUT  2
#define RFIV_RXHEADER   4
#define RFIV_CCA        5
#define RFIV_TXDONE     6








/** @brief  Configures SX127x outputs for RX Data Modes
  * @param  None
  * @retval None
  * @ingroup SX127x
  */ 
void sx127x_iocfg_rx();

/** @brief  Configures SX127x outputs for TX Data Modes
  * @param  None
  * @retval None
  * @ingroup SX127x
  */ 
void sx127x_iocfg_tx();

/** @brief  Configures SX127x outputs for Listen or CSMA Modes
  * @param  None
  * @retval None
  * @ingroup SX127x
  */ 
void sx127x_iocfg_cad();


/** @brief  Configures GPIO Interrupts for Listen Mode
  * @param  None
  * @retval None
  * @ingroup SX127x
  */ 
void sx127x_int_listen();

/** @brief  Configures GPIO Interrupts for RX Data Mode
  * @param  None
  * @retval None
  * @ingroup SX127x
  */ 
void sx127x_int_rxdata();

/** @brief  Configures GPIO Interrupts for CSMA Mode
  * @param  None
  * @retval None
  * @ingroup SX127x
  */ 
void sx127x_int_csma();

/** @brief  Configures GPIO Interrupts for TX Data Mode
  * @param  None
  * @retval None
  * @ingroup SX127x
  */ 
void sx127x_int_txdata();



/** @brief  Individual ISR function for SX127x GPIO0 (similar for 1,2,3,4,5)
  * @param  None
  * @retval None
  * @ingroup SX127x
  */
void sx127x_irq0_isr();
void sx127x_irq1_isr();
void sx127x_irq2_isr();
void sx127x_irq3_isr();
void sx127x_irq4_isr();
void sx127x_irq5_isr();


/** @brief  Wait for event.  i.e. do a blocking wait for something to finish.
  * @param  None
  * @retval None
  * @ingroup SX127x
  */
void sx127x_wfe(ot_u16 ifg_sel);





/** Basic Control <BR>
  * ============================================================================
  */

/** @brief  Loads default register values into the SX127x
  * @param  None
  * @retval None
  * @ingroup SX127x
  */
void sx127x_load_defaults();


/** @brief  Performs a "soft reset" on the SX127x core
  * @param  None
  * @retval None
  * @ingroup SX127x
  */
void sx127x_reset();


/** @brief  Indicates if READY signal is high
  * @param  None
  * @retval None
  * @ingroup SX127x
  */
ot_bool sx127x_isready();


/** @brief  Holds MCU in blocking wait until SX127x is ready
  * @param  None
  * @retval None
  * @ingroup SX127x
  */
void sx127x_waitfor_ready();


/** @brief  Holds MCU in blocking wait until SX127x enters FSRX
  * @param  None
  * @retval None
  * @ingroup SX127x
  */
void sx127x_waitfor_fsrx();


/** @brief  Holds MCU in blocking wait until SX127x enters FSTX
  * @param  None
  * @retval None
  * @ingroup SX127x
  */
void sx127x_waitfor_fstx();


/** @brief  Holds MCU in blocking wait until SX127x enters CAD
  * @param  None
  * @retval None
  * @ingroup SX127x
  */
void sx127x_waitfor_cad();


/** @brief  Holds MCU in blocking wait until SX127x enters standby
  * @param  None
  * @retval None
  * @ingroup SX127x
  */
void sx127x_waitfor_standby();


/** @brief  Holds MCU in blocking wait until SX127x enters sleep
  * @param  None
  * @retval None
  * @ingroup SX127x
  */
void sx127x_waitfor_sleep();


/** @brief Returns the value from the CAD-Detect Pin
  * @param None
  * @retval (ot_bool)   True/False on High/Low of CS pin
  * @ingroup SX127x
  *
  * The implementation uses a pin for CAD.  It is nominally DIO1.
  */
ot_bool sx127x_check_cadpin();


/** @brief  Manually refreshes chip status bits (8 bits)
  * @param  None
  * @retval (ot_u8)    8 bit status field (register IRQFLAGS)
  * @ingroup SX127x
  */
ot_u8 sx127x_getstatus();


/** @brief  Returns the 3 bit mode value
  * @param  None
  * @retval ot_u16      OPMODE(2:0) (reg 0x01)
  * @ingroup SX127x
  */
ot_u8 sx127x_mode();


/** @brief  Returns RX BYTES in FIFO value from SX127x core
  * @param  None
  * @retval ot_u8      
  * @ingroup SX127x
  */
ot_u8 sx127x_rxbytes();


/** @brief  Returns RSSI value from SX127x core
  * @param  None
  * @retval ot_u8       RSSI register value
  * @ingroup SX127x
  *
  */
ot_u8 sx127x_rssi();
ot_u8 sx127x_pktrssi();
ot_s8 sx127x_pktsnr();








/** High-Level Read, Write, and Load-Defaults Functions <BR>
  * ============================================================================
  */

/** @brief  Sends a one-byte (really 3 bit) command strobe to the SX127x via SPI
  * @param  new_mode    (ot_u8) Strobe Opmode (0-7)
  * @param  blocking    (ot_bool) True: function will not return until mode change is verified
  * @retval none
  * @ingroup SX127x
  *
  * The only commands you get with SX127x are mode-change operations.
  */
void sx127x_strobe(ot_u8 new_mode, ot_bool blocking);



/** @brief  Reads one byte of data from an unbanked, addressed register
  * @param  addr        (ot_u8) Register address
  * @retval ot_u8       read data
  * @ingroup SX127x
  */
ot_u8 sx127x_read(ot_u8 addr);



/** @brief  Burst read (multiple bytes) from addressed register
  * @param  start_addr  (ot_u8) Start Register address (must be shifted)
  * @param  length      (ot_u8) Burst data length in bytes
  * @param  data        (ot_u8*) Data buffer to read data into
  * @retval none
  * @ingroup SX127x
  *
  * This function is nearly identical to sx127x_read().  The differences
  * should be self-explanatory.
  */
void sx127x_burstread(ot_u8 start_addr, ot_u8 length, ot_u8* data);



/** @brief  Writes one byte of data to an addressed register
  * @param  addr        (ot_u8) Register address
  * @param  data        (ot_u8) Data to write to register
  * @retval none
  * @ingroup SX127x
  */
void sx127x_write(ot_u8 addr, ot_u8 data);



/** @brief  Burst write (multiple bytes) to registers
  * @param  start_addr  (ot_u8) Start address for write
  * @param  length      (ot_u8) Burst data length in bytes
  * @param  cmd_data    (ot_u8*) address + write data
  * @retval none
  * @ingroup SX127x
  *
  * The cmd_data argument must have a 2 byte offset.  In other words, the data
  * should begin at position cmd_data[2].
  */
void sx127x_burstwrite(ot_u8 start_addr, ot_u8 length, ot_u8* cmd_data);




/** Counter Management Functions <BR>
  * ========================================================================<BR>
  * Certain MAC processes require a running timer.  Instead of using any
  * internal timers of the SX127x, we instead use the more reliable interval
  * timer feature of OpenTag.
  */
  
void sx127x_start_counter();

void sx127x_stop_counter();

ot_u16 sx127x_get_counter();





/** Advanced Configuration <BR>
  * ========================================================================<BR>
  */
/** @brief  Computes a signed-integer RSSI value from SX127x encoded value
  * @param  encoded_value 	(ot_u8) SX127x encoded RSSI
  * @param	packet_snr		(ot_s8) SX127x packet SNR figure
  * @retval ot_int         	RSSI as signed integer
  * @ingroup SX127x
  */
ot_int sx127x_calc_rssi(ot_u8 encoded_value, ot_s8 packet_snr);

ot_u8 sx127x_calc_rssithr(ot_u8 input);

ot_u8 sx127x_clip_txeirp(ot_u8 input_eirp);


/** @brief Sets the TX output power based on input DASH7 Power Code
  * @param pwr_code     (ot_u8) tx_eirp value (typically from PHYMAC struct)
  * @retval none
  * @ingroup SX127x
  *
  * This function will set the target power in PA TABLE 0, and it will fill the
  * PA TABLE slots 8-1 with ramped-down powers so that the TX ramp-up/ramp-down
  * is nice and smooth.
  */
void sx127x_set_txpwr(ot_u8 pwr_code);



#endif
