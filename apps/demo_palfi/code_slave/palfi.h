/* Copyright 2012 JP Norair
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
  */
/**
  * @file       /apps/demo_palfi/code_slave/palfi.c
  * @author     JP Norair
  * @version    R100
  * @date       10 October 2012
  * @brief      PaLFi Demo Slave Features
  *
  * This application for OpenTag would qualify as an intermediate example (OK, 
  * maybe an advanced example) of what can be done with an asynchronous-pre-
  * emptive kernel (APE).  The PaLFi app requires a lot of ordered wait-slots, 
  * and the APE kernel is great for this.  
  *
  * The TI code that was used as a reference is entirely blocking, and this 
  * code is almost entirely non-blocking.  Making it non-blocking allows the 
  * MCU to sleep, saving power.  Make sure to check the eventno in the sleeping
  * loop, because this is like a Mutex for the PaLFi application, and PaLFi 
  * cannot go into LPM3 while it is engaged (it needs port IO).  In the main.c
  * associated with this demo, this Mutex feature is already implemented.
  *
  * The PaLFi application has multiple features, depending on what buttons are
  * pressed on the board or what type of PaLFi LF signal is sent from the base
  * station.  The features and their processes are managed as an internal state
  * machine.  The event handle (sys.evt.EXT.eventno) is used to identify the
  * active feature.  When eventno == 0, the PaLFi App is not underway, which
  * means also that there is no PaLFi signal.  When eventno != 0, it means that
  * something is happening with PaLFi.  The precise value of eventno depends on
  * what state of operation the PaLFi app is undergoing.  In addition, for some
  * complex features (like trimming), there is an internal state-tracking 
  * mechanism that uses a function pointer to select different routines, with
  * wait slots in between.
  *
  ******************************************************************************
  */

#include "OTAPI.h"



typedef enum {
    CH1 = 1,
    CH2 = 2,
    CH3 = 3
} 
palfi_CHAN;

typedef ot_int (*action_fn)(void);

typedef struct {
	float   slope_dC;
	float   offset_dC;
}
tempmodel_struct;

typedef struct {
    ot_uint count;
    ot_uint startcount;
    ot_uint endcount;
    ot_uint startval;
    ot_uint endval;
    
    float   tlow[3];            ///@todo develop fixed-point solution
    float   thigh[3];
} 
trim_struct;

typedef struct {
    action_fn   action;         // Action function (internal usage mostly)
    trim_struct trim;           // LF Trimming process data (internal usage mostly)
    
    ot_s8       trimval[3];     // Normalized Trim values for three channels
    
    ot_u8       wake_event;     // Event ID
    ot_u8       channel;        // Channel ID (internal usage mostly)
    
    ot_u8       prestatus;
    ot_u8       status[4];
    
    ot_u8       rxdata[8];
    
    ot_u8       rssi_info;
    ot_u8       rssi1;
    ot_u8       rssi2;
    ot_u8       rssi3;
    ot_u8       read_addr;
    ot_u8       read_ext;
} 
palfi_struct;


typedef struct {
    ot_u8       crcresult[2];
    ot_u8       crcdata[12];
}
palfiext_struct;


extern palfi_struct palfi;
extern palfiext_struct palfiext;





/** Top Level PaFLi Functions & subroutines
  * ========================================================================<BR>
  *
  */
  
/** @brief  Initialize the MCU resources needed for PaLFi
  * @param  None
  * @retval None
  * @ingroup PaLFi
  */
void palfi_init();



/** @brief  Power-down PaLFi systems, but no de-initializing
  * @param  None
  * @retval None
  * @ingroup PaLFi
  */
void palfi_powerdown();



/** @brief  Startup PaLFi SPI-link and read status to palfi.status buffer
  * @param  None
  * @retval None
  * @ingroup PaLFi
  */
void palfi_spi_startup();



/** @brief  Send a stream of data to the PaLFi core, via SPI-link
  * @param  src     (ot_u8*) byte buffer of data to send via SPI
  * @retval None
  * @ingroup PaLFi
  *
  * The stream pointed-to by "src" must have the non-inclusive length supplied
  * as the first byte.  For example, a typical 4 byte command might be supplied
  * as: src[] = { 03, XX, XX, XX }
  */
void palfi_writeout(ot_u8* src);



/** @brief  Receive a stream of data from the PaLFi core, via SPI-Link
  * @param  dst     (ot_u8*) byte buffer allocation of received data
  * @param  size    (ot_int) number of bytes to read
  * @retval None
  * @ingroup PaLFi
  */
void palfi_readback(ot_u8* dst, ot_int size);



/** @brief  Send a RAID control command over SPI-Link
  * @param  cmd     (ot_u8) RAID control command byte
  * @retval None
  * @ingroup PaLFi
  * @sa palfi_writeout()
  *
  * RAID Control Commands are 4 bytes:      <BR>
  * B0: Length                      <BR>
  * B1: Write Address               <BR>
  * B2: Write Address Extension     <BR>
  * B3: Command Byte                <BR>
  *
  * Possible commands   <BR>
  * 0D  =  ‘00001100’:  Power-Down (delayed after MSP-Access-Uplink)    <BR>
  * 0E  =  ‘00001110’:  Power-Down Command                              <BR>
  * 0F  =  ‘00001111’:  Power-Down with write of Button-Stuck-Mask      <BR>
  *
  * 1?  =  ‘0001??00’:  Switch 500us clock to CLK_OUT pin (default).    <BR>
  * 1?  =  ‘0001??01’:  Switch sampling clock switched to CLK_OUT pin   <BR>
  * 1?  =  ‘0001??10’:  Switch immobilizer clock to CLK_OUT pin         <BR>
  * 1?  =  ‘0001??11’:  Switch off CLK_OUT pin                          <BR>
  * 1?  =  ‘000100??’:  Switch EOBA to EOB pin (default)                <BR>
  * 1?  =  ‘000101??’:  Switch EOB from Transponder to EOB pin          <BR>
  * 1?  =  ‘000110??’:  Switch Buffer-Read-Request to EOB pin (default) <BR>
  * 1?  =  ‘000111??’:  Switch off EOB pin                              <BR>
  *
  * 2?  = ‘001011?0’:  Immobilizer disable              <BR>
  * 2?  = ‘001011?1’:  Immobilizer enable (default)     <BR>
  * 2?  = ‘0010110?’:  WDE disable                      <BR>
  * 2?  = ‘0010111?’:  WDE enable (default)             <BR>
  * 
  * FF  = ‘11111111’:   SPI-timeout Reset 
  */
void palfi_raidctrl(ot_u8 cmd);



/** @brief  Request and receive RSSI values from the PaLFi core
  * @param  None
  * @retval None
  * @ingroup PaLFi
  *
  * The results get read-into palfi.rssi section
  */
void palfi_cmdrssi();



/** @brief  Request and receive status values from the PaLFi core
  * @param  None
  * @retval None
  * @ingroup PaLFi
  *
  * The results get read-into palfi.status section
  */
void palfi_cmdstatus();



