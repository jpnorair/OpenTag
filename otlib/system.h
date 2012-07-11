/* Copyright 2010-2011 JP Norair
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
  * @file       /OTlib/system.h
  * @author     JP Norair
  * @version    V1.0
  * @date       2 November 2011
  * @brief      ISO 18000-7.4 Mode 2 System Framework
  * @defgroup   System (System Module)
  * @ingroup    System
  *
  * The System Module is the centerpiece of OpenTag, and it acts as an event
  * manager.  In basic implementations of OpenTag, the only function that really
  * needs to be called is ot_run(), which maps 1:1 with sys_run().
  *
  * The System Module does some of the work of the Mode 2 MAC, and some of the
  * work of the Session Layer.  Both of these layers need to handle events and
  * states, so they are bundled together in the System Module.
  ******************************************************************************
  */


#ifndef __SYSTEM_H
#define __SYSTEM_H

//#include "OTAPI.h"
#include "OT_types.h"
#include "OT_config.h"
//#include "OT_utils.h"
//#include "buffers.h"
//#include "queue.h"



#define SYS_RECEIVE         ( (M2_FEATURE(GATEWAY) == ENABLED) \
                            ||  (M2_FEATURE(SUBCONTROLLER) == ENABLED) \
                            ||  (M2_FEATURE(ENDPOINT) == ENABLED) )
                            
#define SYS_FLOOD           ((M2_FEATURE(GATEWAY) == ENABLED) \
                            ||  (M2_FEATURE(SUBCONTROLLER) == ENABLED) )

#define SYS_SESSION         (OT_FEATURE(CAPI) == ENABLED)

#define SYS_RFA_RECEIVE     ((RF_FEATURE(RXTIMER) == DISABLED) && SYS_RECEIVE)
#define SYS_RFA_TRANSMIT    (RF_FEATURE(TXTIMER) == DISABLED)
#define SYS_RFA_FLOOD       ((RF_FEATURE(TXTIMER) == DISABLED) && SYS_FLOOD)




/** SYS Configuration
  * This may at some point find its way into OT_config.h
  * 
  * SYS_EVENT_MAX       max duration in ticks that an event may be queued
  * SYS_RUN_MAX         max duration in ticks that the sys_run can go between calls
  * SYS_BEACON_THRESH   block beacons that are this many ticks away from a priority comm event
  */
#define SYS_EVENT_MAX               (ot_long)2147483647
#define SYS_RUN_MAX                 65535
#define SYS_BEACON_THRESH           (M2_FEATURE(BEACON_THRESH) * PLATFORM_GPTIM_TICKS_PER_TI)



/** SYS Mutexes
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
#define M2_ADV_OFFSET       (-3)
#define M2_ADV_LISTEN       6
#define M2_ADV_SLOP         (M2_ADV_OFFSET * -2)
#define M2_ADV_ERRDIV       16384



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
#   define M2_SET_SLEEPSCHED           0x8000
#   define M2_SET_HOLDSCHED            0x4000
#   define M2_SET_BEACONSCHED          0x2000
#   define M2_SET_CLASSMASK            0x0F00
#   define M2_SET_GATEWAY              0x0800
#   define M2_SET_SUBCONTROLLER        0x0400
#   define M2_SET_ENDPOINT             0x0200
#   define M2_SET_BLINKER              0x0100
#   define M2_SET_DSTREAMMASK          0x00C0
#   define M2_SET_345WAY               0x0080
#   define M2_SET_2WAY                 0x0040
#   define M2_SET_FECMASK              0x0030
#   define M2_SET_FECTX                0x0020
#   define M2_SET_FECRX                0x0010
#   define M2_SET_CHANMASK             0x000F
#   define M2_SET_BLINK                0x0008
#   define M2_SET_TURBO                0x0004
#   define M2_SET_STANDARD             0x0002
#   define M2_SET_LEGACY               0x0001
#else
#   define M2_SET_SCHEDMASK            0x00E0
#   define M2_SET_SLEEPSCHED           0x0080
#   define M2_SET_HOLDSCHED            0x0040
#   define M2_SET_BEACONSCHED          0x0020
#   define M2_SET_CLASSMASK            0x000F
#   define M2_SET_GATEWAY              0x0008
#   define M2_SET_SUBCONTROLLER        0x0004
#   define M2_SET_ENDPOINT             0x0002
#   define M2_SET_BLINKER              0x0001
#   define M2_SET_DSTREAMMASK          0xC000
#   define M2_SET_345WAY               0x8000
#   define M2_SET_2WAY                 0x4000
#   define M2_SET_FECMASK              0x3000
#   define M2_SET_FECTX                0x2000
#   define M2_SET_FECRX                0x1000
#   define M2_SET_CHANMASK             0x0F00
#   define M2_SET_BLINK                0x0800
#   define M2_SET_TURBO                0x0400
#   define M2_SET_STANDARD             0x0200
#   define M2_SET_LEGACY               0x0100
#endif

typedef struct {
    ot_u8   subnet;             // Device Subnet from UDB 0
    ot_u8   b_subnet;
    ot_u8   dd_flags;           // Default Device flags (see M2DF's in protocol_M2.h)
    ot_u8   b_attempts;         // Beacon Tries from UDB 0
    ot_u16  active;             // Active settings from UDB 0
    ot_u16  hold_limit;         // Hold limit from UDB 0
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
  
#define M2_MACSTATE_NULL    0x00
#define M2_MACSTATE_IDLE    0x01
#define M2_MACSTATE_RX      0x02
#define M2_MACSTATE_TX      0x03

#define M2_MACATTR_HSC      0x04
#define M2_MACATTR_ON       0x08

#define M2_MACIDLE_OFF      0x00
#define M2_MACIDLE_SLEEP    0x01
#define M2_MACIDLE_HOLD     0x02
#define M2_MACIDLE_LISTEN   0x03




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
    ot_uint rx_timeout;
    ot_u8   csmaca_params;      // (A2P | NA2P) + (RIGD | RAIND | AIND) + CSMA on/off
    ot_u8   redundants;         // number of attempts
    ot_u8   tx_channels;        // num channels on which the tx may be issued
    ot_u8   rx_channels;        // num channels on which the rx may come from (usually 1)
    ot_u8*  tx_chanlist;
    ot_u8*  rx_chanlist;
    ot_u8   scratch[2];         // intended for chanlist storage during ad-hoc single channel dialogs
} m2comm_struct;


typedef struct {
    netconf_struct  netconf;
    m2comm_struct   comm;
    ot_u8           idle_state;
} m2dll_struct;

extern m2dll_struct dll;




/** @brief Null callback routine for the kernel app-loading feature
  * @param none
  * @retval ot_bool     sys_loadapp_null() always returns False
  * @ingroup System
  * 
  * The System Kernel can load a user "applet" during idle time.  The
  * user applet must return True if creating a new session.  The user
  * can manage the app that gets loaded however he (or she) wants.
  */
ot_bool sys_loadapp_null(void);



/** @brief Initializes data objects for System Module and all connected Modules
  * @param none
  * @retval none
  * @ingroup System
  * @sa platform_OT_init(), sys_refresh()
  *
  * Call this once at startup, or whenever you want to do something akin to a
  * soft restart.  It puts all non-platform modules into their default states,
  * therefore it doesn't do anything to the radio driver, veelite, MPipe, or 
  * other forms of platform I/O.  For a full restart of OpenTag, instead call 
  * platform_OT_init().  If you just want to bring OpenTag back to its default 
  * idle state, without clobbering objects & callbacks, use sys_refresh().
  * 
  * Non-Platform modules that may contain data objects are:
  * <LI> Authentication module (auth) </LI>
  * <LI> Buffers module (buffers) </LI>
  * <LI> External module (ext) </LI>
  * <LI> M2QP Transport Layer module (m2qp) </LI>
  * <LI> Network Layer module (network) </LI>
  *
  */
void sys_init();


/** @brief Refresh system settings, wipe sessions, and put system into idle
  * @param none
  * @retval none  
  * @ingroup System
  * @sa otapi_sysinit(), sys_init()
  *
  * sys_refresh() does two things.  First, it grabs the registry data from the
  * Network Settings ISF (ISF 0) and applies it to the system object.  Then it
  * puts OpenTag in a default idle state with no pending or ongoing sessions.
  */
void sys_refresh();


/** @brief Changes the device settings (see Network Settings ISF)
  * @param new_mask         (ot_u16) Bitmask for applying new settings
  * @param new_settings     (ot_u16) Compared with supported settings, and set.
  * @retval (none)  
  * @ingroup System
  *
  * This will alter the active device settings.  If settings cannot be 
  * supported (check supported settings), the value you input might be modified
  * in order to meet supported settings.
  */
void sys_change_settings(ot_u16 new_mask, ot_u16 new_settings);


/** @brief Puts device into idle state
  * @param none
  * @retval (none)  
  * @ingroup System
  *
  * Puts device into idle, immediately.  Generally, this is only used by the 
  * automated internal code.  You can use it to kill system tasks, though.  One
  * example is to use sys_idle() in a USB suspend callback, to make sure that
  * radio operation is shut off.
  */
void sys_idle();


/** @brief An idle-state transformation to Off
  * @param none
  * @retval (none)  
  * @ingroup System
  *
  * Generally, this doesn't need to be called from the outside.
  */
void sys_goto_off();


/** @brief An idle-state transformation to Sleep
  * @param none
  * @retval (none)  
  * @ingroup System
  *
  * Generally, this doesn't need to be called from the outside.
  */
void sys_goto_sleep();


/** @brief An idle-state transformation to Hold
  * @param none
  * @retval (none)  
  * @ingroup System
  *
  * Generally, this doesn't need to be called from the outside.
  */
void sys_goto_hold();


/** @brief An idle-state transformation to Off
  * @param none
  * @retval (none)  
  * @ingroup System
  *
  * Generally, this doesn't need to be called from the outside.
  */
void sys_goto_listen();


/** @brief System Panic
  * @param err_code     (ot_u8) error code, similar to POSIX death signals
  * @retval None
  * @ingroup System
  *
  * When called, sys_panic() will shut-down the OpenTag kernel and then invoke
  * the sys.panic system callback (if system callbacks are enabled), so that
  * the user can do whatever he wants to do in response to the panic.  Usually
  * the best policy is to shut things down, but for less serious errors (such as
  * Bus Error and Seg fault), the user could initiate some form of emergency
  * beacon that runs from static memory.
  *
  * Known (Official) Panic error codes <BR>
  * 1   Power failure (compare SIGHUP)                  <BR>
  * 4   Physical Memory Violation (compare to SIGKILL)  <BR>
  * 7   Virtual addressing Bus Error                    <BR>
  * 11  Virtual addressing Segmentation Fault           <BR>
  */
void sys_panic(ot_u8 err_code);



/** @brief Feed it a channel ID, it will tell you if CSMA is required
  * @param none
  * @retval (ot_u8)     This is either 0 or M2_CSMACA_NOCSMA  
  * @ingroup System
  */
ot_u8 sys_default_csma(ot_u8 chan_id);



/** @brief Initializes sys features from settings stored in UDB elements
  * @param none
  * @retval (none)  
  * @ingroup System
  *
  * Run this when the sys module parameter RAM needs to be refreshed.
  * For example: on power up, on task activation (if running an OS), etc.
  */
void sys_init();


/** @brief Gracefully terminates the RF task managed by the kernel.
  * @param none
  * @retval none
  * @ingroup System
  */
void sys_quit_rf();


/** @brief Sets mutexes based on mask input
  * @param set_mask         (ot_uint) The mutex value to set
  * @retval none
  * @ingroup System
  */
void sys_set_mutex(ot_uint set_mask);


/** @brief Clears mutexes based on mask input
  * @param clear_mask       (ot_uint) a bitmask -- 0 for each bit to clear
  * @retval none
  * @ingroup System
  */
void sys_clear_mutex(ot_uint clear_mask);


/** @brief Returns the mutex value
  * @param none
  * @retval ot_int          The mutex value
  * @ingroup System
  */
ot_int sys_get_mutex();



/** @brief Event Management and Processing
  * @param elapsed_ms   (ot_uint) Supply number of ticks since last call.
  * @retval (ot_uint)   Number of ticks until you need to call it next
  * @ingroup System
  * 
  * This is one of the functions that has to be in your client program.  It is
  * wrapped inside otapi_run(), which can contain other logic, if desired.
  */  
ot_uint sys_event_manager(ot_uint elapsed);






/** System Static Callbacks (optional) <BR>
  * ========================================================================<BR>
  * Static callbacks are optional and implemented in the user application code.
  * To enable a static callback, define the appropriate EXTF_sys_sig... value
  * in the extf_config.h file (which should be in the /App/[App-Name]/Code 
  * folder), and implement the function somewhere in your application code.
  */  


/** @brief Loadapp callback function
  * @param none
  * @retval ot_bool     True if your app created a session for the kernel
  * @ingroup System
  * 
  * Return True if your implementation of this function creates a session that
  * you want the kernel to manage.  Otherwise, return False.
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.loadapp().  Often, for this 
  * callback, the implementation is nicer looking when using the dynamic 
  * approach, and there is minimal difference (if any) in resource usage.
  */  
ot_bool sys_sig_loadapp(void);



/** @brief System Panic callback function
  * @param code         (ot_int) a kernel error code (platform dependent)
  * @retval None
  * @ingroup System
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.panic().
  */
void sys_sig_panic(ot_int code);



/** @brief RF-Active initialization callback function
  * @param pcode        (ot_int) a radio process code
  * @retval None
  * @ingroup System
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.evt.RFA.init().  It is called by
  * the system module / kernel when an active RF process (RX or TX) is being
  * initialized.
  * 
  * The radio process code is dependent on the kernel, but the OpenTag native 
  * kernel uses the settings below:
  * <LI>1: background scan </LI>
  * <LI>2: foreground scan </LI>
  * <LI>3: CSMA (unused) </LI>
  * <LI>4: foreground tx </LI>
  * <LI>5: background tx </LI>
  */
void sys_sig_rfainit(ot_int pcode);


/** @brief RF-Active initialization callback function
  * @param pcode        (ot_int) a radio process code
  * @param scode        (ot_int) a radio termination status code
  * @retval None
  * @ingroup System
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.evt.RFA.init().  It is called by
  * the system module / kernel when an active RF process (RX or TX) is being
  * initialized.
  * 
  * The radio process code is dependent on the kernel, but the OpenTag native 
  * kernel uses the settings below:
  * <LI>1: background scan </LI>
  * <LI>2: foreground scan </LI>
  * <LI>3: CSMA </LI>
  * <LI>4: foreground tx </LI>
  * <LI>5: background tx </LI>
  *
  * The status code is >= 0 when the radio process terminated on account of
  * a successful transfer of a packet.
  */
void sys_sig_rfaterminate(ot_int pcode, ot_int scode);



/** @brief Beacon process prestart callback function
  * @param event_data   (void*) pointer to kernel-dependent event datatype
  * @retval None
  * @ingroup System
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.evt.BTS.prestart().  It is
  * called by the system module / kernel when a beacon data process begins.
  */
void sys_sig_btsprestart(void* event_data);


/** @brief Hold-scan process prestart callback function
  * @param event_data   (void*) pointer to kernel-dependent event datatype
  * @retval None
  * @ingroup System
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.evt.HSS.prestart().  It is
  * called by the system module / kernel when a hold-scan data process begins.
  */
void sys_sig_hssprestart(void* event_data);


/** @brief Sleep-scan process prestart callback function
  * @param event_data   (void*) pointer to kernel-dependent event datatype
  * @retval None
  * @ingroup System
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.evt.SSS.prestart().  It is
  * called by the system module / kernel when a sleep-scan data process begins.
  */
void sys_sig_sssprestart(void* event_data);


/** @brief External process callback function
  * @param event_data   (void*) pointer to kernel-dependent event datatype
  * @retval None
  * @ingroup System
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.evt.EXT.process().  It is
  * called by the system module / kernel when an external data process begins.
  */
void sys_sig_extprocess(void* event_data);







/* Scheduler function, not yet!

   @brief Checks the scheduler against the current RTC value
  * @param sync_data :   (ot_int) byte offset into Scheduler UDB Element
  * @retval (ot_ulong) : Time remaining until next scheduled event.
  * @ingroup System
  *
  * This is optional, mostly for testing purposes.  The scheduler is not advised
  * for use unless there's an RTC that can generate its own interrupt.  The 
  * returned value can be converted to approximate milliseconds by shifting
  * right 6 bits, or actual milliseconds by dividing by 65.536 (or thereabouts)
  *
  * The output format (Time remaining) is in the units of 1/65536 seconds.
  * The input format can be shortcutted by macros that should be declared nearby
  * this function prototype.

ot_ulong sys_check_scheduler(ot_int sync_data);
#define SSS_SYNC_DATA     6
#define HSS_SYNC_DATA     14
#define BTS_SYNC_DATA     22
  */  






#endif

