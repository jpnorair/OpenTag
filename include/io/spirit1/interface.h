/* Copyright 2009-2014 JP Norair
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
  * @file       /include/io/spirit1/interface.h
  * @author     JP Norair
  * @version    R102
  * @date       27 Aug 2014
  * @brief      Functions for the SPIRIT1 transceiver interface
  * @defgroup   SPIRIT1 (SPIRIT1 family support)
  *
  ******************************************************************************
  */

#ifndef __IO_SPIRIT1_INTERFACE_H
#define __IO_SPIRIT1_INTERFACE_H

#include <otstd.h>
#include <io/spirit1/config.h>
#include <io/spirit1/SPIRIT1_registers.h>
#include <io/spirit1/SPIRIT1_defaults.h>


#ifndef BOARD_FEATURE_RFXTALOUT
#   define BOARD_FEATURE_RFXTALOUT 0
#endif
  
#ifdef RADIO_DB_ATTENUATION
#   define _ATTEN_DB    RADIO_DB_ATTENUATION
#else
#   define _ATTEN_DB    6
#endif

#ifndef DRF_SYNC_BYTES
#   define DRF_SYNC_BYTES   4
#endif



/** @typedef spirit1_link
  * A data element for returning link quality parameters other than RSSI.  RSSI
  * can be retrieved by upper layers, universally for all different types of
  * RF transceivers, via radio.last_rssi. 
  */

/** @typedef spirit1_struct
  * <LI>imode   (SPIRIT1_IMode) Enumeration used for IRQ state management </LI>
  * <LI>status  (ot_u8) Stores the 2-byte chip-status obtained on each SPI access </LI>
  * <LI>busrx   (ot_u8) scratch space for RX'ed SPI bus data </LI>
  *
  * @note The maximum SPI transfer depends on the allocation of busrx.  A DMA
  *       is used with the SPI, so it needs to dump the RX data here.  24 bytes
  *       is quite safe with the current implementation, and it is unlikely to
  *       be too small for any implementation based on the current one.
  *
  * @note The results of any SPI read will get stored in spirit1.busrx.  If you
  *       use a function in this interface library that returns a value from
  *       read data, it is returning data copied from spirit1.busrx.  Use this
  *       knowledge to optimize your code, or do hacks & tricks.
  */
  
// Enumeration used for IRQ state management.  You can ignore it.
typedef enum {
    MODE_Listen = 0,
    MODE_RXData = 2,
    MODE_CSMA   = 5,
    MODE_TXData = 7
} SPIRIT1_IMode;

typedef struct {
#   if (BOARD_FEATURE_RFXTALOUT)
    ot_bool         clkreq;
#   endif
    SPIRIT1_IMode   imode;
    ot_u16          status;
    ot_u8           busrx[24];
} spirit1_struct;

extern spirit1_struct spirit1;



/** SET_LINE and CLR_LINE macros for some profiling tasks.  They have been used
  * so far to save the line number of a call to spirit1_smart_standby().
  */
//#define _SPIRIT1_PROFILE
  
#if defined(_SPIRIT1_PROFILE)
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
  * i.e. io/spirit1/radio_rm2.c
  */
void spirit1_virtual_isr(ot_u8 code);





/** Functions implemented typically in the platform driver <BR>
  * ==========================================================================
  * i.e. platform/stm32l1xx/io_spirit1.c
  */
void spirit1_int_clearall(void);
ot_uint spirit1_resetpin_ishigh(void);
ot_uint spirit1_abortpin_ishigh(void);
ot_uint spirit1_readypin_ishigh(void);
ot_uint spirit1_cspin_ishigh(void);
ot_uint spirit1_sdnpin_setlow(void);
ot_uint spirit1_sdnpin_sethigh(void);


void spirit1_int_config(ot_u32 ie_sel);

/** @brief  Configures GPIO Interrupts for TX Done Mode
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */ 
void spirit1_int_txdone();


/** @brief  Forces an interrupt on masked, selected sources (sets flag high)
  * @param  ifg_sel     (ot_u16) interrupt flag select
  * @retval None
  * @ingroup SPIRIT1
  */
void spirit1_int_force(ot_u16 ifg_sel);


/** @brief  Enables interrupt on masked, selected sources (set enable bit high)
  * @param  ie_sel     (ot_u16) interrupt enable select
  * @retval None
  * @ingroup SPIRIT1
  */
void spirit1_int_turnon(ot_u16 ie_sel);


/** @brief  Disables interrupt on masked, selected sources (set enable bit low)
  * @param  ie_sel     (ot_u16) interrupt enable select
  * @retval None
  * @ingroup SPIRIT1
  */
void spirit1_int_turnoff(ot_u16 ie_sel);



/** @brief  Initialize the bus interface of the SPIRIT1
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  * @sa radio_init()
  *
  * This function does not set the default registers.  That is done in the 
  * generic radio module function, radio_init().  This function needs to be run
  * before radio_init(), since radio_init() requires the bus.  Best practice is
  * to actually call this function inside radio_init(), at the beginning.
  */
void spirit1_init_bus();



/** @brief  Causes the System to wait until the SPI bus is fully de-asserted
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */
void spirit1_spibus_wait();



/** @brief  Master function for SPI bus I/O
  * @param  cmd_len         (ot_u8) length in bytes of the command (write data)
  * @param  resp_len        (ot_u8) length in bytes of the response (read data)
  * @param  cmd             (ot_u8*) the data buffer to TX
  * @retval none
  * @ingroup SPIRIT1
  * @sa radio_init()
  *
  * spirit1_spibus_io() can be used for any sort of SPI-based IO to the SPIRIT1.
  * The return data from the SPIRIT1 is stored in spirit1.busrx.  The allocation
  * of spirit1.busrx (typically 24 bytes) stipulates the maximum amount of data
  * that can be transfered (+2) in a single call to this function.  Therefore,
  * with 24 bytes allocated to busrx, (cmd_len + resp_len) must be less than or 
  * equal to 26 bytes.  In practice, this is more than enough for OpenTag in 
  * its current implementation.
  */
void spirit1_spibus_io(ot_u8 cmd_len, ot_u8 resp_len, ot_u8* cmd);



ot_u8 sprit1_getbasepwr();







/** Basic Control <BR>
  * ============================================================================
  */

/** @brief  Loads default register values into the SPIRIT1
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */
void spirit1_load_defaults();


/** @brief  Performs a "soft reset" on the SPIRIT1 core
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */
void spirit1_reset();


/** @brief  Holds processing until POR signal goes low, indicating safe RESET
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */
void spirit1_waitforreset();



/** @brief  Indicates if READY signal is high
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */
ot_u16 spirit1_isready();



/** @brief  Holds processing until READY signal goes high
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */
void spirit1_waitforready();
void spirit1_waitforstandby();
void spirit1_waitforabort();



/** @brief  Manually refreshes chip status bits (16 bits)
  * @param  None
  * @retval (ot_u16)    16 bit status field (copied from spirit1.status)
  * @ingroup SPIRIT1
  *
  * The SPIRIT1 has a 16bit status field that is returned on every SPI bus 
  * access.  If you do any access whatsoever, you can just refer to the status
  * data in spirit1.status after the access is done.  If you have not done an
  * SPI access recently, you can use this function to manually update the 
  * status field.  It will also return the status field, for convenience.
  */
ot_u16 spirit1_getstatus();


/** @brief  Returns the values of MC_STATE[1:0] registers
  * @param  None
  * @retval ot_u16      MC_STATE[1:0] (regs 0xC0, 0xC1)
  * @ingroup SPIRIT1
  */
ot_u16 spirit1_mcstate();


/** @brief  Returns the value of 8bit LDC timer
  * @param  None
  * @retval ot_u8       LDC timer value (8bits)
  * @ingroup SPIRIT1
  */
ot_u8 spirit1_ldctime();


/** @brief  Returns the value of 8bit LDC-Reload timer
  * @param  None
  * @retval ot_u8       LDCR timer value (8bits)
  * @ingroup SPIRIT1
  */
ot_u8 spirit1_ldcrtime();


/** @brief  Returns the value of 8bit RX timeout timer
  * @param  None
  * @retval ot_u8       RX timeout timer value (8bits)
  * @ingroup SPIRIT1
  */
ot_u8 spirit1_rxtime();



/** @brief  Returns RSSI value from SPIRIT1 core
  * @param  None
  * @retval ot_u8       RSSI_LEVEL register value
  * @ingroup SPIRIT1
  *
  * @note The SPIRIT1 datasheet mentions that RSSI_LEVEL is only valid following
  *       the reception of a packet.  There is some ambiguity regarding what
  *       stipulates the end of a received packet, but nonetheless it is not
  *       advised to use this function until after a packet RX is finished.
  */
ot_u8 spirit1_rssi();


/** @brief  Returns RX BYTES in FIFO value from SPIRIT1 core
  * @param  None
  * @retval ot_u8       LINEAR_FIFO_STATUS0 register value
  * @ingroup SPIRIT1
  */
ot_u8 spirit1_rxbytes();


/** @brief  Returns TX BYTES in FIFO value from SPIRIT1 core
  * @param  None
  * @retval ot_u8       LINEAR_FIFO_STATUS1 register value
  * @ingroup SPIRIT1
  */
ot_u8 spirit1_txbytes();






/** Bus interface (SPI + 4x GPIO) <BR>
  * ============================================================================
  */

/** @brief  Sends a one-byte command strobe to the SPIRIT1 via SPI
  * @param  strobe      (ot_u8) Strobe Address
  * @retval none
  * @ingroup SPIRIT1
  */
void spirit1_strobe(ot_u8 strobe);



/** @brief  Reads one byte of data from an unbanked, addressed register
  * @param  addr        (ot_u8) Register address
  * @retval ot_u8       read data
  * @ingroup SPIRIT1
  */
ot_u8 spirit1_read(ot_u8 addr);



/** @brief  Burst read (multiple bytes) from addressed register
  * @param  start_addr  (ot_u8) Start Register address (must be shifted)
  * @param  length      (ot_u8) Burst data length in bytes
  * @param  data        (ot_u8*) Data buffer to read data into
  * @retval none
  * @ingroup SPIRIT1
  *
  * This function is nearly identical to spirit1_read().  The differences
  * should be self-explanatory.
  */
void spirit1_burstread(ot_u8 start_addr, ot_u8 length, ot_u8* data);



/** @brief  Writes one byte of data to an addressed register
  * @param  addr        (ot_u8) Register address
  * @param  data        (ot_u8) Data to write to register
  * @retval none
  * @ingroup SPIRIT1
  */
void spirit1_write(ot_u8 addr, ot_u8 data);



/** @brief  Burst write (multiple bytes) to registers
  * @param  start_addr  (ot_u8) Start address for write
  * @param  length      (ot_u8) Burst data length in bytes
  * @param  cmd_data    (ot_u8*) address + write data
  * @retval none
  * @ingroup SPIRIT1
  *
  * The cmd_data argument must have a 2 byte offset.  In other words, the data
  * should begin at position cmd_data[2].
  */
void spirit1_burstwrite(ot_u8 start_addr, ot_u8 length, ot_u8* cmd_data);







/** Advanced Configuration <BR>
  * ========================================================================<BR>
  */
/** @brief  Computes a signed-integer RSSI value from SPIRIT1 encoded value
  * @param  encoded_value (ot_u8) SPIRIT1 encoded RSSI
  * @retval ot_int         RSSI as signed integer
  * @ingroup SPIRIT1
  */
ot_int spirit1_calc_rssi(ot_u8 encoded_value);


/** @brief  Converts a DASH7-spec CS or CCA threshold into SPIRIT1 RSSI Threshold
  * @param  input       (ot_u8) CS/CCA Threshold from DASH7 encoding
  * @retval ot_u8       Value that can be written to SPIRIT1 RSSI_THR field
  * @ingroup SPIRIT1
  */
ot_u8 spirit1_calc_rssithr(ot_u8 input);



ot_u8 spirit1_clip_txeirp(ot_u8 input_eirp);


/** @brief Sets the TX output power based on input DASH7 Power Code
  * @param pwr_code     (ot_u8*) pointer to PHYMAC struct tx_eirp value
  * @retval none
  * @ingroup SPIRIT1
  *
  * This function will set the target power in PA TABLE 0, and it will fill the
  * PA TABLE slots 8-1 with ramped-down powers so that the TX ramp-up/ramp-down
  * is nice and smooth.
  */
void spirit1_set_txpwr(ot_u8* pwr_code);



/** @brief Returns the value from the CS pin
  * @param None
  * @retval (ot_bool)   True/False on High/Low of CS pin
  * @ingroup SPIRIT1
  *
  * The implementation uses a pin for CS.  It is nominally GPIO2.
  */
ot_bool spirit1_check_cspin(void);





/** Common GPIO setup & interrupt functions <BR>
  * ========================================================================<BR>
  * Your radio ISR function should be of the type void radio_isr(ot_u8), as it 
  * will be a soft ISR.  The input parameter is an interrupt vector.  The vector
  * values are shown below:
  *
  * -------------- RX MODES (set spirit1_iocfg_rx()) --------------
  * IMode = 0       RX Timeout (Finished):      0 
  * (Listen)        Sync word RX'ed:            1 
  *                 RX FIFO thr [IRQ off]:      -
  *                 
  * IMode = 2       RX Finished:                2  
  * (RX Data)       Sync word RX'ed [IRQ off]:  -
  *                 RX FIFO threshold:          4
  *
  * -------------- TX MODES (set spirit1_iocfg_tx()) --------------
  * IMode = 5       CCA Sense Timeout:          5   (pass)
  * (CSMA)          CS Indicator:               6   (fail)
  *                 TX FIFO thr [IRQ off]:      - 
  *
  * IMode = 7       TX finished:                7
  * (TX)            CS Indicator [IRQ off]:     -
  *                 TX FIFO threshold:          9 
  */

#define RFINT(VAL)      RFI_##VAL

#define RFI_SOURCE0     (1 << RADIO_IRQ0_SRCLINE)
#define RFI_SOURCE1     (1 << RADIO_IRQ1_SRCLINE)
#define RFI_SOURCE2     (1 << RADIO_IRQ2_SRCLINE)
#define RFI_SOURCE3     0 //(1 << RADIO_IRQ3_SRCLINE)   //used for READY pin, no interrupt

#define RFI_ALL         (RFI_SOURCE0 | RFI_SOURCE1 | RFI_SOURCE2 /*| RFI_SOURCE3*/)

#define RFI_RXTIMEOUT   RFI_SOURCE0
#define RFI_RXSYNC      RFI_SOURCE1
#define RFI_LISTEN      (RFI_RXTIMEOUT | RFI_RXSYNC)

#define RFI_RXEND       RFI_SOURCE0
#define RFI_RXFIFO      RFI_SOURCE2
#define RFI_RXDATA      (RFI_RXEND | /*RFI_RXSYNC |*/ RFI_RXFIFO)

#define RFI_CCATIMEOUT  RFI_SOURCE0
#define RFI_CCAFAIL     RFI_SOURCE1
#define RFI_CSMA        (RFI_CCATIMEOUT | RFI_CCAFAIL)

#define RFI_TXEND       RFI_SOURCE0
#define RFI_TXFIFO      RFI_SOURCE2
#define RFI_TXDATA      (RFI_TXEND | RFI_TXFIFO)

#define RFIV_RXTERM     0
#define RFIV_RXSYNC     1
#define RFIV_RXEND      2
#define RFIV_RXFIFO     4
#define RFIV_CCAPASS    5
#define RFIV_CCAFAIL    6
#define RFIV_TXEND      7
#define RFIV_TXFIFO     9



void spirit1_start_counter();

void spirit1_stop_counter();

ot_u16 spirit1_get_counter();




/** @brief  Configures SPIRIT1 outputs for Listen or RX Data Modes
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */ 
void spirit1_iocfg_rx();

/** @brief  Configures SPIRIT1 outputs for CSMA or TX Data Modes
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */ 
void spirit1_iocfg_tx();


/** @brief  Configures GPIO Interrupts for Listen Mode
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */ 
void spirit1_int_listen();


/** @brief  Configures GPIO Interrupts for RX Data Mode
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */ 
void spirit1_int_rxdata();


/** @brief  Configures GPIO Interrupts for CSMA Mode
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */ 
void spirit1_int_csma();


/** @brief  Configures GPIO Interrupts for TX Data Mode
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */ 
void spirit1_int_txdata();



/** @brief  Individual ISR function for SPIRIT1 GPIO0 (similar for 1,2,3)
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */
void spirit1_irq0_isr();
void spirit1_irq1_isr();
void spirit1_irq2_isr();
void spirit1_irq3_isr();


/** @brief  Wait for event.  i.e. do a blocking wait for something to finish.
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */
void spirit1_wfe();


/** @brief  Prepare WFE (wait for event) for AES process completion
  * @param  None
  * @retval None
  * @ingroup SPIRIT1
  */
void spirit1_wfe_aes();


#endif
