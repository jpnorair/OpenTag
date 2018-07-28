/* Copyright 2012-2014 JP Norair
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
  * @file       /otlib/m2_dll.h
  * @author     JP Norair
  * @version    R101
  * @date       7 May 2014
  * @brief      Data Link Layer for DASH7
  * @defgroup   DLL (Data Link Layer Module)
  * @ingroup    DLL
  *
  ******************************************************************************
  */



#if !defined(__M2_DLL_H) /* && OT_FEATURE(M2) */
#define __M2_DLL_H

#include <otsys/types.h>
#include <otsys/config.h>
#include <m2/session.h>
#include <otsys/syskern.h>



#define SYS_RECEIVE         ( (M2_FEATURE(GATEWAY) == ENABLED) \
                            ||  (M2_FEATURE(SUBCONTROLLER) == ENABLED) \
                            ||  (M2_FEATURE(ENDPOINT) == ENABLED) )
                            
#define SYS_FLOOD           ((M2_FEATURE(GATEWAY) == ENABLED) \
                            ||  (M2_FEATURE(SUBCONTROLLER) == ENABLED) )

#define SYS_SESSION         (OT_FEATURE(CAPI) == ENABLED)

#define SYS_RFA_RECEIVE     ((RF_FEATURE(RXTIMER) == DISABLED) && SYS_RECEIVE)
#define SYS_RFA_TRANSMIT    (RF_FEATURE(TXTIMER) == DISABLED)
#define SYS_RFA_FLOOD       ((RF_FEATURE(TXTIMER) == DISABLED) && SYS_FLOOD)




/** SYS Mutexes (DEPRECATED)
  * The SYS module is the centerpiece of OpenTag.  It manages access of platform
  * resources within other OpenTag modules.  You can at any time check the 
  * mutexes.  This can gate the runtime of your app.  There is plenty of room
  * left for more mutexes.
  */
#define SYS_MUTEX_RADIO_LISTEN      0x01                                // bit 0
#define SYS_MUTEX_RADIO_DATA        0x02                                // bit 1
#define SYS_MUTEX_PROCESSING        0x04                                // bit 2





/** Mode 2 Default Channel Guard Periods (Tgd)
  * Default Channel Guard Period is derived by the Channel ID, measured in ti.
  * (ti = 2^-10 sec).
  */
#define M2_TGD_55FULL       5
#define M2_TGD_55HALF       10
#define M2_TGD_200FULL      2
#define M2_TGD_200HALF      3




/** Mode 2 Advertising Slop
  * The Advertising Protocol is supposed to be accurate to within 2 ti.  Thus,
  * we need to consider that the event will occur possibly 2 ti before or after
  * the event is scheduled to occur, plus some margin for timing slop on this
  * device, which is clocking out the event.
  *
  * @todo Connect this to the configuration interface, not just the hardcoded
  * values below.
  */
#define M2_ADV_OFFSET       (5)
#define M2_ADV_LISTEN       (10)                 //must be <= 15
#define M2_ADV_MAXSLOP      (8)
//#define M2_ADV_ERRDIV       16384



/** Beacon CSMA-CA waiting
  * Right now this is just a constant.  It could be expanded to more than that.
  */
#define M2_BEACON_TCA       M2_FEATURE(BEACON_TCA)




/** SYS Mode 2 Network Configuration
  * This is a buffer for the data that, in its default states, is in the Network
  * Configuration Settings File.  Since the data is stored as big endian, it
  * needs to be mangled in order to work with little-endian systems.
  */
#ifdef __BIG_ENDIAN__
#   define M2_SET_SCHEDMASK            0xE000
#   define M2_SET_HOLDSCHED            0x8000
#   define M2_SET_SLEEPSCHED           0x4000
#   define M2_SET_BEACONSCHED          0x2000
#   define M2_SET_CLASSMASK            0x0700
#   define M2_SET_GATEWAY              0x0400
#   define M2_SET_SUBCONTROLLER        0x0200
#   define M2_SET_ENDPOINT             0x0100
#   define M2_SET_DSTREAMMASK          0x00C0
#   define M2_SET_345WAY               0x0080
#   define M2_SET_2WAY                 0x0040
#   define M2_SET_FECMASK              0x0030
#   define M2_SET_FECTX                0x0020
#   define M2_SET_FECRX                0x0010
#   define M2_SET_CHANMASK             0x000E
#   define M2_SET_BLINK                0x0008
#   define M2_SET_TURBO                0x0004
#   define M2_SET_STANDARD             0x0002
#else
#   define M2_SET_SCHEDMASK            0x00E0
#   define M2_SET_HOLDSCHED            0x0080
#   define M2_SET_SLEEPSCHED           0x0040
#   define M2_SET_BEACONSCHED          0x0020
#   define M2_SET_CLASSMASK            0x0007
#   define M2_SET_GATEWAY              0x0004
#   define M2_SET_SUBCONTROLLER        0x0002
#   define M2_SET_ENDPOINT             0x0001
#   define M2_SET_DSTREAMMASK          0xC000
#   define M2_SET_345WAY               0x8000
#   define M2_SET_2WAY                 0x4000
#   define M2_SET_FECMASK              0x3000
#   define M2_SET_FECTX                0x2000
#   define M2_SET_FECRX                0x1000
#   define M2_SET_CHANMASK             0x0E00
#   define M2_SET_BLINK                0x0800
#   define M2_SET_TURBO                0x0400
#   define M2_SET_STANDARD             0x0200
#endif

typedef struct {
    ot_u8   btemp[8];
    ot_u8   uid[8];
    ot_u8   vid[2];
    ot_u8   subnet;             // Device Subnet from ISF 0
    ot_u8   b_subnet;
    ot_u16  active;             // Active settings from ISF 0
    ot_u8   dd_flags;           // Default Device flags
    ot_u8   b_attempts;         // Beacon Tries from ISF 0
    ot_u16  hold_limit;         // Hold limit from ISF 0
} netconf_struct;



/** Mode 2 MAC state management
  * There are three Data Link Layer States, described by the Mode 2 spec: 
  * Idle:   The idle time processes are running, and no rx/tx is occuring
  * RX:     RX is actively occuring, and other events are blocked-out
  * TX:     TX is actively occuring, and other events are blocked-out
  *
  * In addition, there are some state attributes that modify the way the Idle
  * state runs.
  * HSC:    Hold Scan Cycling -- used only on Endpoints, allowing an alternate
  *         idle-time process to be used during dialog interactions.
  *
  * ON:     If not on, the idle-time processes won't run.  The MAC will only
  *         create and manage wake-on events if ON.
  */
  
#define M2_DLLSTATE_NULL    0x00
#define M2_DLLSTATE_IDLE    0x01
#define M2_DLLSTATE_RX      0x02
#define M2_DLLSTATE_TX      0x03

#define M2_DLLATTR_HSC      0x04
#define M2_DLLATTR_ON       0x08

#define M2_DLLIDLE_OFF      0x00
#define M2_DLLIDLE_SLEEP    0x01
#define M2_DLLIDLE_HOLD     0x02
#define M2_DLLIDLE_LISTEN   0x03




/** Mode 2 Comm variables
  * This is a data structure for controlling the way the radio module works. 
  * The values are usually provided by other layers, especially the transport
  * layer (M2QP).  While multiple sessions may overlap, only one comm dialog
  * can happen at any given time.  The comm dialog may use multiple channels,
  * although this is impossible in implementations where only one RF transceiver
  * exists (a SISO host).  Implementations with multiple receivers (MISO) can
  * use one channel for TX (at any given time) and multiple channels for RX
  * (at any given time).
  *
  * m2comm_struct description
  * 
  * timeout         (ot_uint) MAC is half duplex, so only one timeout for TX or
  *                 RX.  It is measured in units of Ti (2^-10 sec).  For TX, it
  *                 is used to timeout the CA process.  For RX, it is used to 
  *                 timeout the response window.
  *
  * tca             (ot_long) This is a parameter used in the feedback process 
  *                 for CA.  The transport layer flow and congestion control
  *                 functions manipulate its value.  The MAC layer uses the 
  *                 value to plan events.
  * 
  * csmaca_params   (ot_u8) CSMA-CA is defined in the MAC layer but configured
  *                 in the Transport layer.  Instead of using a bunch of memory 
  *                 for function pointer callbacks, flags are used, so the MAC
  *                 layer can make the right decision on how to call Transport
  *                 layer functions and/or memory elements.
  *
  * tx_redundants   (ot_u8) Number of redundant times a transmission should be
  *                 issues.  If a positive ACK is received before going through
  *                 all the redundants, comm will stop at that point.
  *
  * tx_channels     (ot_u8) Number of channels that may be used to transmit a
  *                 packet.  This is used with tx_chanlist (see that param if
  *                 you are confused how multiple channels are used).
  *
  * rx_channels     (ot_u8) Number of channel that may be used to receive a
  *                 packet.  This is used with rx_chanlist (see that param if
  *                 you are confused how multiple channels are used).
  *
  * tx_chanlist     (ot_u8) The device will do CSMA on these channels until it
  *                 finds one that passes the testing.  Transmission will take
  *                 place only on one channel from the list (the first one that
  *                 passes CSMA).
  *
  * rx_chanlist     (ot_u8) A MISO device can receive packets on multiple
  *                 channels.  For these, rx_chanlist is the list of channels 
  *                 that packets could be coming-in on.  For SISO devices, this
  *                 is never going to be more than one channel in length.
  *
  * scratch[2]      (ot_u8) Implicit single channel transport protocols can 
  *                 store the single channel RX & TX chanlists in this dump and
  *                 point tx_chanlist & rx_chanlist to it, accordingly.  By
  *                 convention, [1] is used for rx_chanlist and [0] for tx.
  */

// Parameters for csmaca_params
#define M2_CSMACA_NA2P      0x00
#define M2_CSMACA_A2P       0x40
#define M2_CSMACA_ARBMASK   0x40
#define M2_CSMACA_NOCSMA    0x04
#define M2_CSMACA_CAMASK    0x38
#define M2_CSMACA_RIGD      0x00
#define M2_CSMACA_RAIND     0x08
#define M2_CSMACA_AIND      0x10
#define M2_CSMACA_MACCA     0x38

typedef struct {
    ot_long tc;                 // Contention Period (Tc, sometimes also called Tcp)
    ot_long tca;                // Collision avoidance period (Tca)
    ot_long rx_timeout;
    ot_u8   scratch[2];         // intended for chanlist storage during ad-hoc single channel dialogs
    ot_u8   csmaca_params;      // (A2P | NA2P) + (RIGD | RAIND | AIND) + CSMA on/off
    ot_u8   redundants;         // number of attempts
    ot_u8   tx_channels;        // num channels on which the tx may be issued
    ot_u8   rx_channels;        // num channels on which the rx may come from (usually 1)
    ot_u8*  tx_chanlist;
    ot_u8*  rx_chanlist;
} m2comm_struct;

typedef struct {
    ot_sig  rfinit;
    ot_sig2 rfterminate;
} dll_sig_struct;

typedef struct {
    ot_u8           idle_state;
    ot_u8           reserved;
    ot_uint         counter;
    ot_uint         adv_time;
    netconf_struct  netconf;
    m2comm_struct   comm;
#   if (OT_FEATURE(DLLRF_CALLBACKS))
    dll_sig_struct  sig;
#   endif
} m2dll_struct;

extern m2dll_struct dll;





/** Macros <BR>
  * ========================================================================<BR>
  */
#if defined(EXTF_dll_sig_rfinit)
#   define DLL_SIG_RFINIT(CODE)                 dll_sig_rfinit(CODE)
#elif (OT_FEATURE(DLLRF_CALLBACKS) == ENABLED)
#   define DLL_SIG_RFINIT(CODE)                 dll.sig.rfinit(CODE)
#else
#   define DLL_SIG_RFINIT(CODE)                 while(0)
#endif

#if defined(EXTF_dll_sig_rfterminate)
#   define DLL_SIG_RFTERMINATE(CODE1, CODE2)    dll_sig_rfterminate(CODE1, CODE2)
#elif (OT_FEATURE(DLLRF_CALLBACKS) == ENABLED)
#   define DLL_SIG_RFTERMINATE(CODE1, CODE2)    dll.sig.rfterminate(CODE1, CODE2)
#else
#   define DLL_SIG_RFTERMINATE(CODE1, CODE2)    while(0)
#endif


//For synchronization only
//extern volatile ot_u32 sample_t0;

//extern volatile ot_u32 sample_t1;




/** DLL IO Tasks <BR>
  * ========================================================================<BR>
  */
void dll_block(void);
void dll_unblock(void);
void dll_clock(ot_uint ticks);
void dll_next(void);
void dll_systask_init(void);
void dll_systask_rf(ot_task task_dll);



/** DLL Idle-Time Tasks <BR>
  * ========================================================================<BR>
  */
void dll_systask_holdscan(ot_task task);
void dll_systask_sleepscan(ot_task task);
void dll_systask_beacon(ot_task task);

void dll_block_idletasks(void);

void dll_default_applet(m2session* active);

void dll_response_applet(m2session* s_active);
void dll_scan_applet(m2session* s_active);
void dll_beacon_applet(m2session* s_active);



/** DLL Default Signal Callers <BR>
  * ========================================================================<BR>
  * These are implemented in m2_dll_task.c as empty functions, and they are 
  * linked with "weak" attribute so you can replace them with your own versions.
  */

void dll_sig_rfinit(ot_int pcode);
void dll_sig_rfterminate(ot_int pcode, ot_int scode);



/** Initializers state management functions <BR>
  * ========================================================================<BR>
  */

/** @brief  Initializes the DLL module, should be run at startup
  * @param  None
  * @retval None
  * @ingroup DLL
  */
void dll_init(void);


/** @brief Refresh system settings, wipe sessions, and put system into idle
  * @param none
  * @retval none  
  * @ingroup DLL
  * @sa otapi_sysinit(), sys_init()
  *
  * dll_refresh() does two things.  First, it grabs the registry data from the
  * Network Settings ISF (ISF 0) and applies it to the system object.  Then it
  * puts OpenTag in a default idle state with no pending or ongoing sessions.
  */
void dll_refresh(void);


/** @brief Refreshes the DLL real-time scheduler features (requires OTcron)
  * @param  None
  * @retval None
  * @ingroup DLL
  * @sa dll_refresh
  * 
  * If your build includes OTcron, this function will pull data from the Real
  * Time Scheduler file (ISF 04) and align Sleep, Hold, and Beacon RTS to what
  * is in the file.  If your build does not include OTcron, this function does
  * nothing.
  *
  * The function dll_refresh() calls this dll_refresh_rts() internally.
  */
void dll_refresh_rts(void);



/** @brief Changes the device settings (see Network Settings ISF)
  * @param new_mask         (ot_u16) Bitmask for applying new settings
  * @param new_settings     (ot_u16) Compared with supported settings, and set.
  * @retval (none)  
  * @ingroup DLL
  *
  * This will alter the active device settings.  If settings cannot be 
  * supported (check supported settings), the value you input might be modified
  * in order to meet supported settings.
  */
void dll_change_settings(ot_u16 new_mask, ot_u16 new_settings);


/** @brief Disables/enables all idle radio operations (i.e. sleep/hold/beacon)
  * @param onoff        (ot_bool) true silences, false unsilences.
  * @retval (none)  
  * @ingroup DLL
  */
void dll_silence(ot_bool onoff);


/** @brief An idle-state transformation to Off
  * @param none
  * @retval (none)  
  * @ingroup DLL
  *
  * Generally, this doesn't need to be called from the outside.
  * 
  * Deprecated: use dll_silence() instead
  */
void dll_goto_off(void);



/** @brief Puts device into idle state
  * @param none
  * @retval (none)  
  * @ingroup DLL
  *
  * Puts device into idle, immediately.  Generally, this is only used by the 
  * automated internal code.  You can use it to kill system tasks, though.  One
  * example is to use sys_idle() in a USB suspend callback, to make sure that
  * radio operation is shut off.
  */
void dll_idle(void);



/** @brief Sets DLL comm parameters (dll.comm) to default values
  * @param session  (m2session*) Communication session pointer
  * @retval None
  * @ingroup DLL
  * 
  * The input session may have some impact on the defaults, depending on its
  * active netstate at the time of call.
  */
void dll_set_defaults(m2session* session);






/** Internal Task Calls, exposed to enable patching <BR>
  * ========================================================================<BR>
  */
void dll_scan_timeout(void);
void dll_processing(void);
void dll_activate(void);
void dll_init_rx(m2session* active);
void dll_init_tx(ot_u8 is_btx);
void dll_txcsma(void);



/** Radio Driver DLL Callbacks, exposed to enable patching <BR>
  * ========================================================================<BR>
  */
void dll_rfevt_brx(ot_int scode, ot_int fcode);
void dll_rfevt_frx(ot_int pcode, ot_int fcode);
void dll_rfevt_txcsma(ot_int pcode, ot_int tcode);
void dll_rfevt_btx(ot_int flcode, ot_int scratch);
void dll_rfevt_ftx(ot_int pcode, ot_int scratch);



/** @brief Refresh the M2 Advertising payload between transmissions.
  * @retval None
  * @ingroup DLL
  * 
  * This is a subroutine with side-effects.  The only reason it is exposed 
  * to the API is for the purpose of having application patches.
  */
void dll_m2advp_refresh(void);



/** IO Management routines (deprecated) <BR>
  * ========================================================================<BR>
  */

/** @brief Feed it a channel ID, it will tell you if CSMA is required
  * @param none
  * @retval (ot_u8)     This is either 0 or M2_CSMACA_NOCSMA  
  * @ingroup DLL
  */
ot_u8 dll_default_csma(ot_u8 chan_id);


/** @brief Gracefully terminates the RF task managed by the kernel.
  * @param none
  * @retval none
  * @ingroup DLL
  */
void dll_quit_rf(void);




#endif



