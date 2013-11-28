/* Copyright 2010-2012 JP Norair
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
  * @file       /otlib/radio.h
  * @author     JP Norair
  * @version    R100
  * @date       20 Oct 2012
  * @brief      Generic Radio (RF transceiver) interface
  * @defgroup   Radio (Radio Module)
  * @ingroup    Radio
  *
  * The radio module links to one or more driver implementations.  Right now,
  * it can probably only link to one, which has to be the Mode 2 version.  In
  * the future I might have to change this slightly, but possibly not.
  *
  * There needs to be an implementation file, otradio/xxx/radio_xxx.c or 
  * something like this. It acts as the driver and encapsulates all of the HW
  * management functions.
  *
  * There also needs to be an implementation of the data link layer you want to
  * use with the radio driver.  As mentioned, the radio driver is designed for
  * DASH7 Mode 2, but it also needs a link-layer taskset.  In OpenTag, this is
  * already part of otlib: otlib/m2_dll.c
  *
  ******************************************************************************
  */


#ifndef __RADIO_H
#define __RADIO_H

#include "OT_types.h"
#include "OT_config.h"



typedef enum {
    RADIO_Idle      = 0,
    RADIO_Listening = 1,
    RADIO_Csma      = 2,
    RADIO_DataRX    = 5,
    RADIO_DataTX    = 6
} radio_state;



/** Additional link information
  *
  */
#define RADIO_LINK_CORRECTIONS  (1<<6)
#define RADIO_LINK_PQI          (1<<3)
#define RADIO_LINK_SQI          (1<<2)
#define RADIO_LINK_LQI          (1<<1)
#define RADIO_LINK_AGC          (1<<0)

typedef struct {
    ot_u8 flags;
    ot_u8 corrections;  
    ot_u8 reserved_2;
    ot_u8 reserved_3;
    ot_u8 pqi;          // Preamble Quality Index
    ot_u8 sqi;          // Synchronization Quality Index
    ot_u8 lqi;          // Link Quality Index
    ot_u8 agc;          // AGC Parameters
} radio_link_struct;



/** Universal Radio Driver Control
  * state       A high-level description of what is happening on the radio
  * last_rssi   Used to buffer the last-read RSSI value -- not always implemented
  * evtdone     A callback that is used when RX or TX is completed (i.e. done)
  */
typedef struct {
    radio_state state;
    ot_int      last_rssi;
    ot_int      last_linkloss;
    ot_sig2     evtdone;
#if (OT_FEATURE(RF_LINKINFO))
    radio_link_struct link;
#endif
} radio_struct;

extern radio_struct radio;











/** Mode 2 PHY-MAC variables
  * These variables come from the Channel Configuration ISFS File, and are 
  * stored here (and possibly altered slightly) before the channel is used by
  * the PHY.  Some of these parameters measure RSSI or EIRP.  These assume a
  * perfectly matched, 100% efficient antenna.  You will have to offset them
  * to account for the losses of your platform's antenna and matching circuit.
  * You will probably have to figure those out by testing, but if you are just 
  * hacking around, 6 dB attenuation usually suits a compact 433 MHz antenna
  * system, 3 dB for a monopole whip, 1-2 dB for dipole.
  *
  * phymac_struct description
  * 
  * tg              (ot_int) channel guard time (in ti units) as determined by
  *                 the spec.  It is 2, 3, 5, or 10 ti depending on the channel.
  * 
  * channel         (ot_u8) channel id to use
  *
  * autoscale       (ot_u8) A code that specifies an algorithm used for adaptive
  *                 scaling of tx eirp and/or link budget filtering.
  *
  * tx_eirp         (ot_u8) A value for tx eirp (radiated power) that is applied
  *                 to transmitted packets. eirp dBm = (0x7F & tx_eirp)/2 + 40
  *
  * link_qual       (ot_u8) Used for link quality filtering of received packets.
  *                 Mode 2 packets report TX EIRP in the header.  Received EIRP
  *                 minus received RSSI yields the packet link quality (reverse
  *                 of link budget).  Lower number is better quality.  This
  *                 derived value must be less than the programmed value for the
  *                 packet to pass the link quality filter.
  *
  * cs_rssi         (ot_u8) the min RSSI value to validate carrier sense, for 
  *                 receiving packets.  It should be large enough so that the
  *                 data can be reliably decoded at an acceptable BER.
  *
  * cca_rssi        (ot_u8) the max RSSI value to validate clear chan assesment 
  *                 for CSMA-CA.  It is usually lower than the decodable min
  *                 RSSI, in order to prevent "hidden node problem" in CSMA.
  */
typedef struct {
    ot_int  tg;
    ot_u8   channel;
    ot_u8   autoscale;
    ot_u8   tx_eirp;
    ot_u8   link_qual;
    ot_u8   cs_thr;
    ot_u8   cca_thr;
} 
phymac_struct;


/** PHY-MAC Channel Data
  * The MAC/System level rx chanlist maps to this array.  Mode 2 only supports
  * single channel transmit, so transmit channel data is always phymac[0].  In
  * most systems, this array will only be length 1, because most systems are 
  * SISO hosts.
  */
#ifndef M2_PARAM_MI_CHANNELS
#   define M2_PARAM_MI_CHANNELS  1
#endif
  
extern phymac_struct   phymac[M2_PARAM_MI_CHANNELS];



#define RADIO_ERROR_NDATA   
#define RADIO_ERROR_TDATA   


#if (PLATFORM_TYPE == SIMULATOR)
    /** Air buffer (simulator only)
      * For simulation, the TX puts information to the air buffer, and RX 
      * gets information from the air buffer.
      */
    extern ot_int air_i;
    extern ot_u8 air[520];
#endif




/** Basic Mode 2 Channel IDs
  * These may or may not be useful
  */

// Base Transport Channel
#define RM2_CHAN_BASE       0x00

// Legacy Transport Channel (Mode 1)
#define RM2_CHAN_LEGACY     0x01

// Normal Channels
#define RM2_CHAN_NORMAL_0   0x10
#define RM2_CHAN_NORMAL_2   0x12
#define RM2_CHAN_NORMAL_4   0x14
#define RM2_CHAN_NORMAL_6   0x16
#define RM2_CHAN_NORMAL_8   0x18
#define RM2_CHAN_NORMAL_A   0x1A
#define RM2_CHAN_NORMAL_C   0x1C
#define RM2_CHAN_NORMAL_E   0x1E

// Turbo Channels
#define RM2_CHAN_TURBO_1    0x21
#define RM2_CHAN_TURBO_3    0x23
#define RM2_CHAN_TURBO_5    0x25
#define RM2_CHAN_TURBO_7    0x27
#define RM2_CHAN_TURBO_9    0x29
#define RM2_CHAN_TURBO_B    0x2B
#define RM2_CHAN_TURBO_D    0x2D

// Blink Channels
#define RM2_CHAN_BLINK_2    0x32
#define RM2_CHAN_BLINK_C    0x3C

// Wildcard Channel
#define RM2_CHAN_WILDCARD   0x7F

// MAC configuration of PHY parameters via synchronizer packet
#define RM2_ENCODING_FEC    0x80

#define RM2_ERR_KILL        -1
#define RM2_ERR_CCAFAIL     -2
#define RM2_ERR_BADCHANNEL  -3
#define RM2_ERR_TIMEOUT     -4
#define RM2_ERR_LINK        -5
#define RM2_ERR_GENERIC     -6





/** Radio Module control    <BR>
  * ========================================================================<BR>
  * Used for module configuration and utility
  */

/** @brief Initializes Radio
  * @param None
  * @retval None
  * @ingroup Radio
  *
  * radio_init() turns-on, initializes, and enables the radio module by setting 
  * device-specific default values for the radio part and MCU facilities.
  *
  * A simple practice is to call init_radio() before looking for the wakeup
  * tone. By default, the Protocol module handles this automatically. Otherwise,
  * usage of init_radio() is somewhat device-dependent (see the device-specific
  * initialization function for more information).  Typically, it needs to be
  * done whenever:
  *  A. The radio is being turned-on
  *  B. the registers on the radio device need resetting to defaults
  *  C. the peripherals on the MCU need to be reconfigured for UHF usage
  */
void radio_init();



/** @brief Invokes ISR subroutines that use the MAC timer
  * @param None
  * @retval None
  * @ingroup Radio
  *
  * MAC timer functions include: CSMA control on radios without HW for it, 
  * TX flooding counter management.
  *
  * In typical usage, the MAC timer is multiplexed with the GPTIM (kernel timer)
  * and the HW ISR for GPTIM is in otplatform/xxx/platform_xxx.c.  The HW ISR
  * should call radio_mactimer() if the mactimer channel is interrupting.
  */
void radio_mac_isr();


///@todo comments for these functions
void radio_mac_configure();
ot_bool radio_mac_filter();




/** @brief  Returns the RSSI value of the most recent reception.
  * @param None
  * @retval ot_int      RSSI of last reception in units of 0.5 dBm
  * @ingroup Radio
  *
  * This function is callable outside the process of a dialog, because it 
  * returns the RSSI detected during the most recent receive operation, not the
  * instantaneous RSSI.  RSSI detection happens early in the course of each RX,
  * so in the case that this function is called during process of a dialog, it 
  * will return the RSSI of the current reception.
  *
  * On most hardware, the RSSI range of -100 to -40 dBm [-200 to -80 returned] 
  * is the area of interest.
  */
ot_int radio_rssi();



/** @brief  Calculates and saves the link information
  * @param None
  * @retval None
  * @ingroup Radio
  *
  * This function calculates RSSI and Link-Loss, and it saves them into:
  * radio.last_linkloss
  * radio.last_rssi
  *
  * It should be called within the radio driver when a packet is finished being 
  * received.
  */
void radio_calc_link();



/** @brief Debug function for checking radio buffer
  * @param index        (ot_int) buffer index
  * @retval ot_u8       buffered value at index
  * @ingroup Radio
  *
  * This function is primarily for testing & hacking purposes.  On certain
  * platforms there may not be a radio buffer localized on the MCU, and on these
  * builds the functionality of radio_buffer() is not guaranteed.
  */
ot_u8 radio_buffer(ot_int index);








/** Radio Core control    <BR>
  * ========================================================================<BR>
  * - Basic radio control functions         <BR>
  * - called mostly by Encode module        <BR>
  */

/** @brief  Turns off the radio transceiver
  * @param None
  * @retval None
  * @ingroup Radio
  *
  * The transceiver is "off" when it is not retaining its registers.  This mode
  * usually is extremely low in power, because only leakage current is used.
  * 
  * The radio_init() function must be called to bring radio from the off state
  * into the sleep state.
  */
void radio_off();



/** @brief Disables all Events/Interrupts used by the radio module
  * @param None
  * @retval None
  * @ingroup Radio
  *
  * In certain cases, latent interrupt bits don't get cleared when they should
  * be (like if you manually reset a HW RF Core MAC).  This function can be 
  * called to make sure the radio doesn't keep sending interrupts when you want
  * it to just go idle and shut-up.
  */
void radio_gag();

void radio_ungag();



/** @brief  Puts the transceiver core in the sleep mode.
  * @param  None
  * @retval None
  * @ingroup Radio
  *
  * Different radio HW may have different names for sleep mode.  In any case,
  * radio_sleep() will put the radio into the mode that is lowest power, where
  * register contents are retained.  On HW that I've studied, going from sleep 
  * to active RX or TX takes anywhere from 400 µs to 2000 µs.  Here are specs 
  * from the chips I like most:
  *
  * CC430:      ~400 µs      (CC430 calibrates after every fourth TX/RX usage)
  * ADµCRF101:  ~600 µs
  * SX1212:     ~1500 µs     (Integer PLL is slow but allows 2.5mA RX)
  */
void radio_sleep();



/** @brief  Puts the transceiver core in idle mode.
  * @param  None
  * @retval None
  * @ingroup Radio
  *
  * Different radio HW may have different names for idle mode.  On some chips it
  * may be called "FS on" (Frequency Synthesizer On).  Going from idle to active
  * RX or TX is usually quite fast.  Power used in idle mode varies, but it is
  * usually between 0.5mW and 5mW.
  */
void radio_idle();



void radio_set_mactimer(ot_u16 clocks);


ot_u16 radio_get_countdown();


/** @brief Flushes the TX buffer
  * @param None
  * @retval None
  * @ingroup Radio
  *
  * The radio buffer is either implemented in the radio core (HW) or via SW in
  * the radio driver module.  In both cases, this flushes the TX buffer, 
  * however, it may also flush the RX buffer if they are co-located.
  */
void radio_flush_tx();

/** @brief Flushes the RX buffer
  * @param None
  * @retval None
  * @ingroup Radio
  *
  * The radio buffer is either implemented in the radio core (HW) or via SW in
  * the radio driver module.  In both cases, this flushes the RX buffer, 
  * however, it may also flush the TX buffer if they are co-located.
  */
void radio_flush_rx();



/** @brief Puts a byte to the TX radio buffer
  * @param databyte     (ot_u8) Byte to put on TX
  * @retval None
  * @ingroup Radio
  */
void radio_putbyte(ot_u8 databyte);

/** @brief Puts four bytes to the TX radio buffer
  * @param databyte     (ot_u8*) byte array to the four bytes
  * @retval None
  * @ingroup Radio
  * 
  * This function exists primarily as a way to dump a 32 bit block that has
  * come from the FEC interleaver.  You can supply an ot_ulong (or ot_long) as
  * the argument by this method: (ot_u8*)&(ot_ulong).
  */
void radio_putfourbytes(ot_u8* data);



/** @brief Gets a byte from the RX radio buffer
  * @param none
  * @retval ot_u8       byte from radio RX
  * @ingroup Radio
  */
ot_u8 radio_getbyte();

/** @brief Gets 4 bytes from the RX radio buffer
  * @param data         (ot_u8*) pointer to an array to load into
  * @retval none
  * @ingroup Radio
  *
  * This function exists primarily as a way to grab a 32 bit block to put into
  * FEC deinterleaver.  You can supply an ot_ulong (or ot_long) as the argument
  * by this method: (ot_u8*)&(ot_ulong).
  */
void radio_getfourbytes(ot_u8* data);



/** @brief Checks the RX buffer to see if there is at least 1 more byte in it
  * @param none
  * @retval ot_bool     True if at least 1 more byte in buffer
  * @ingroup Radio
  */
ot_bool radio_rxopen();

/** @brief Checks the RX buffer to see if there are at least 4 more bytes in it
  * @param none
  * @retval ot_bool     True if at least 4 more bytes in buffer
  * @ingroup Radio
  */
ot_bool radio_rxopen_4();



/** @brief Checks the TX buffer to see if at least 1 more byte can fit in it
  * @param none
  * @retval ot_bool     True if at least 1 more byte can fit
  * @ingroup Radio
  */
ot_bool radio_txopen();

/** @brief Checks the TX buffer to see if at least 4 more bytes can fit in it
  * @param none
  * @retval ot_bool     True if at least 4 more bytes can fit
  * @ingroup Radio
  */
ot_bool radio_txopen_4();









/****************************************************************
  * Mode 2 Radio Library Functions                              *
  * - radio I/O specific to the way DASH7 Mode 2 works          *
  * - Implemented in radio_task.c                               *
  ***************************************************************/

/** @brief  Returns default Tgd (guard period) for the specified channel
  * @param  chan_id     (ot_u8) Mode 2 channel ID
  * @retval ot_int      Tgd in ticks (units: 1/1024 seconds)
  * @ingroup Radio
  */
ot_int rm2_default_tgd(ot_u8 chan_id);



/** @brief  Returns minimum rxtimeout value on pending/active channel
  * @param  chan_id     (ot_u8) Mode 2 channel ID
  * @retval ot_u16      minimum timeout in ticks (units: 1/1024 seconds)
  * @ingroup Radio
  */
ot_u16 rm2_rxtimeout_floor(ot_u8 chan_id);



/** @brief  Returns approximate duration of pending packet, in ticks.
  * @param  pkt_bytes   (ot_int) total bytes of all frames in the packet
  * @retval ot_int      Duration of packet in ticks (units: 1/1024 seconds)
  * @ingroup Radio
  * @sa rm2_scale_codec()
  * @sa rm2_bgpkt_duration()
  *
  * Basically identical to rm2_scale_codec(), except that some padding is added
  * to the output in order to account for FIFO lag of the radio.
  */
ot_int rm2_pkt_duration(ot_int pkt_bytes);



/** @brief  Returns approximate duration of BG packet, in ticks
  * @param  None
  * @retval ot_int      Duration of packet in ticks (units: 1/1024 seconds)
  * @ingroup Radio
  * @sa rm2_pkt_duration()
  *
  * This is an optimized version of rm2_pkt_duration(), for background
  * packets.
  */
ot_int rm2_bgpkt_duration();



/** @brief  Returns transmission duration of X bytes on the active channel
  * @param  pkt_bytes   (ot_u8) number of bytes (or bytes left) in the packet
  * @retval ot_int      Duration of packet in ticks (units: 1/1024 seconds)
  * @ingroup Radio
  * @sa rm2_pkt_duration()
  *
  * An implicit (internal) parameter to this function is phymac[N].channel
  */
ot_int rm2_scale_codec(ot_int buf_bytes);










/****************************************************************
  * Mode 2 Radio Driver Functions                               *
  * - radio I/O specific to the way DASH7 Mode 2 works          *
  * - Implemented in radio_task.c                               *
  ***************************************************************/


/** @brief  Instructs radio to re-enter RX mode immediately
  * @param  callback 	(ot_bool) re-supplied RX-done callback
  * @retval None
  * @ingroup Radio
  *
  * This function is used in certain places in the Data Link Layer, and also
  * [typically] internally, to restart RX.  The RX core is not altered, so 
  * whatever settings are in the core get used for this re-entered RX.  Mode 2
  * relies on this function to re-enter RX in the response window or after an
  * erroneous request is detected.
  *
  * The function may be used internally, for code-reuse, with callback set to
  * NULL.  Many implementations use this technique.
  */
void rm2_reenter_rx(ot_sig2 callback);



/** @brief  Prepares the radio chain to resend a packet
  * @param  callback	(ot_sig2) re-supplied callback for TX-Done handler
  * @retval None
  * @ingroup Radio
  *
  * This works with the redundant-TX feature.  It can be implemented in a
  * variety of ways, depending on how the radio core is designed.  For radio
  * cores that have full low-level MAC support of DASH7, it does nothing.
  */
void rm2_resend(ot_sig2 callback);


/** @brief  Initializes RX engine for "foreground" packet reception
  * @param  channel     (ot_u8) Mode 2 channel ID to look for packet
  * @param  psettings   (ot_u8) Packet settings bits
  * @param  callback    (ot_sig2) callback for when RX is done, on error or complete
  * @retval None
  * @ingroup Radio
  *
  * Call this to listen for a packet.  Using "psettings = 0" will look for a
  * 0-frame packet, that is, a "background packet."  If psettings is non-zero,
  * The radio will assume a packet of at least this many "foreground frames"
  * is on the way.
  *
  * A foreground packet contains one or more foreground frames (usually 1)
  * which have a certain format and are subject to a timeout -- if Sync Word
  * is not found by the timeout, the RX is killed.
  *
  * Background flood packets follow very specific rules in the Mode 2 spec
  * (they are used mostly for group synchronization).  Background RX terminates
  * once a packet is received, it is killed, or if the channel energy detection
  * (carrier sense) fails.  Background packets contain only one frame each.
  *
  * This is a non-blocking RX.  It starts an interrupt-driven process that runs
  * in the background until the packet is complete, the RX times-out, or the
  * RX is manually killed with rm2_kill().  These are the termination events.
  *
  * A callback function is used when any of the RX termination events occur or
  * any time a frame is successfully received.  Mode 2 supports multiframe 
  * packets, so the callback is used following the concept below:
  * 
  * void callback(ot_int arg1, ot_int arg2)
  * arg1: negative on RX error, 0 on RX complete, positive on frame complete
  * arg2: 0 on frame received successfully, negative on error.
  */
void rm2_rxinit(ot_u8 channel, ot_u8 psettings, ot_sig2 callback);




/** @brief  Initializes TX engine for "foreground" packet transmission
  * @param  psettings   (ot_u8) Packet settings bits
  * @param  callback    (ot_sig2) callback for when TX is done, on error or complete
  * @retval None
  * @ingroup Radio
  * @sa rm2_txinit_bf, rm2_txcsma
  *
  * Call to prepare the TX system and the TX callback for foreground or background
  * packet.  This function does not usually start TX -- on most hardware,
  * rm2_txcsma() must be called to run the csma-ca routine and begin actual TX.
  *
  * the callback: void callback(ot_int arg1, ot_int arg2)
  * - called on TX termination and whenever a frame is completed
  * - arg 1 is negative on error, 0 on complete, positive if more frames to TX
  * - arg 2 is ignored
  */
void rm2_txinit(ot_u8 psettings, ot_sig2 callback);




/** @brief  Stops the flood harmoniously (finishes the current packet and stops)
  * @param  None
  * @retval None
  * @ingroup Radio
  * @sa rm2_txinit_bf
  * 
  * This will cause the TX callback to occur.
  */
void rm2_txstop_flood();




/** @brief  Terminates RX or TX process (typically for internal use)
  * @param  None
  * @retval None
  * @ingroup Radio
  */
void rm2_kill();




/** @brief  ISR that is called internally when a sync word is detected in RX
  * @param  None
  * @retval None
  * @ingroup Radio
  * 
  * __Best to ignore__.  If you are an expert you can call this to force radio 
  * actions, which may force callbacks.
  */
void rm2_rxsync_isr();



/** @brief  ISR that is called internally when FF RX times out
  * @param  None
  * @retval None
  * @ingroup Radio
  * 
  * __Best to ignore__.  If you are an expert you can call this to force radio 
  * actions, which may force callbacks.
  */
void rm2_rxtimeout_isr();



/** @brief  ISR that is called internally whenever the RX FIFO fills up
  * @param  None
  * @retval None
  * @ingroup Radio
  * 
  * __Best to ignore__.  If you are an expert you can call this to force radio 
  * actions, which may force callbacks.
  */
void rm2_rxdata_isr();



/** @brief  ISR/general-routine that is called when the RX process ends.
  * @param  None
  * @retval None
  * @ingroup Radio
  * 
  * __Best to ignore__.  If you are an expert you can call this to force RX 
  * termination.
  */
void rm2_rxend_isr();



/** @brief  ISR that is called internally when CSMA process is going
  * @param  none
  * @retval none
  * @ingroup Radio
  * @sa rm2_txinit_ff, rm2_txinit_bf
  *
  */
void rm2_txcsma_isr();



/** @brief  ISR that is called internally whenever TX FIFO needs filling
  * @param  None
  * @retval None
  * @ingroup Radio
  * 
  * __Best to ignore__.  If you are an expert you can call this to force radio 
  * actions, which may force callbacks.
  */
void rm2_txdata_isr();






#endif
