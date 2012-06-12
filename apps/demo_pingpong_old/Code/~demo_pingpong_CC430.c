/* Copyright 2009-2011 JP Norair
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  * http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */
/**
  * @file       /Platforms/CC430/~demo_pingpong_CC430.c
  * @author     JP Norair
  * @version    V1.0
  * @date       11 August 2011
  * @brief      Self configuring ping-pong test
  * 
  * Most ping-pong tests are between master and slave.  This one is slightly
  * different, because it is between two masters.  Each device picks a random
  * amount of time, and it will send a request at that time.  At other times, 
  * it will respond to the request from the other device.
  *
  * @note This test can work if the two devices have the same device ID or not.
  *       In normal circumstances, no two devices should have the same ID.
  */


#include "OTAPI.h"
#include "OT_platform.h"
#include "system.h"




// You can configure this at compile time if you want to have two devices using
// different ID's
#define __USE_ID0
//#define __USE_ID1

#ifdef __USE_ID0
#   define __UID    0x1D, 0xAA, 0xAA, 0x1D, 0xBB, 0xBB, 0xBB, 0xB0
#   define __VID    0x1D, 0xB0
#else
#   define __UID    0x1D, 0xAA, 0xAA, 0x1D, 0xBB, 0xBB, 0xBB, 0xB1
#   define __VID    0x1D, 0xB1
#endif






/** Global Variables
  * ============================================================================
  * - t1a3_wait: used in sleep loop to assure proper timing while allowing 
  *              other interrupts to operate.
  *
  * - red_vector: "red" = "Radio Event Debug."  This is a bitvector used by the 
  *               callback routines associated with the RF event "terminate" 
  *               and "init" signals.  During crashes, you can see which bit is
  *               set to determine which event went wrong.
  */
ot_bool app_wait;      
ot_u16  red_vector;








/** Timer Interrupts
  * ============================================================================
  * - TIM0A5 is the OpenTag GPTIM.  Don't use it!
  * - TIM1A3 is free to use by the application.  OT doesn't touch it.
  * - You could change these around if you wanted.  You would do this via the
  *   "#define OT_GPTIM" setting in /Platforms/CC430/platform_config_CC430.h.
  *   The featureset of TIM1A3 is enough for OT.
  */
#define APPTIM  TIM1A3


#pragma vector=TIMER0_A1_VECTOR     //0xFFEC
__interrupt void TIM0A5_X_ISR(void) {
    /// OpenTag only uses the overflow interrupt.  Different interrupt = FAIL
    if (TA0IV != 0x0E) {
        ///@todo log system fault
        ///@todo platform_poweroff();
        ///@todo reset
        __no_operation();   // here for debug
    }
    
    // You MAY want to add code here to disable your application interrupts.
    // (You will re-enable them afterwards).  This is usually not necessary, 
    // but if you have an application that generates and services a lot of 
    // interrupts, it is not a bad idea and can save power.
    
    // Turn off LED2: just here for the demo to show non-blocking of kernel
    platform_trig2_low();   
    
    // This function invokes the OpenTag kernel
    platform_ot_run();
    
    // If you are re-enabling your local interrupts here, put this code ahead
    // of what you are re-enabling.  GIE is automatically set when ISR exits.
    //__bic_SR_register(GIE);
    //__no_operation();
    
    // Clear All Sleep Bits: very important for MSP430
    LPM4_EXIT;
}


#pragma vector=TIMER1_A1_VECTOR     //0xFFEC
__interrupt void TIM1A3_X_ISR(void) {
    ot_u16 rand_time;

    //bounds: 128 to 624 ticks (125 to 609ms)
    app_wait        = False;
    rand_time       = (ot_u16)(platform_prand_u8() & 0x1F) << 4;
    rand_time      += 128;
    
    // Clear Timer, Set new interval, restart
    APPTIM->CTL    &= ~0x0033;
    APPTIM->CTL    |= 0x0004;
    APPTIM->CCR0    = rand_time;
    APPTIM->CTL    |= 0x0012;
    
    LPM4_EXIT;
}






/** RED Vector Callbacks
  * ============================================================================
  * - Radio Event init signal is type ot_sig: "void signal(ot_int)"
  * - Radio Event terminate signal is type ot_sig2: "void signal(ot_int, ot_int)"
  * - Init is called when state opens, terminate when state closes.
  * - Argument 1 is a state ID, 1-5
  * - Argument 2 is a state-dependent value
  * - See system.h (System Module) for more information
  */
void red_init(ot_int state_id) {
    red_vector |= (1 << (state_id-1));
    
    // LED-ON shows that radio I/O is underway.  
    // The radio itself may be off, but there is a good chance it is on.
    platform_trig1_high();              
}

void red_terminate(ot_int state_id, ot_int input) {
    red_vector &= ~(1 << (state_id-1));
    
    // LED-OFF shows that there is no radio I/O.
    // The radio itself is always off here.
    platform_trig1_low();
}





/** Pingpong application request (a system callback)
  * ============================================================================
  * - Invoked whenever the System Event Manager passes through an idle state.
  * - returns True if an API sequence has been loaded
  */
  
ot_bool app_pingpong(void) {
/// Build a Dialog Session.  This could be encapsulated into builder functions, 
/// to make things simple, but the API is lower level, to allow the most 
/// functionality as "lowest common denomenator."

    return False;
}

/*
    /// Flip app_wait to false to enable this app load
    /// This technique would resemble a mutex for a serial command input (i.e.
    /// stop waiting once the command is fully loaded).  This example waits for
    /// an independent timer to complete, which is not really practical, but it 
    /// makes a good example.
    if (app_wait) {
        return False;
    }
    
    app_wait = True;
    
    /// 1. Create an ad-hoc (pre-emptive) session from our app layer
    {
        session_tmpl app_session;
        app_session.channel     = 0x10;
        app_session.subnet      = 0xFF;     //universal subnet (hits everything)
        app_session.subnetmask  = 0xFF;
        app_session.flags       = 0x00;     //don't care about the built-in flags
        app_session.flagmask    = 0x00;
        app_session.timeout     = 64;       //Contention period = 64 ticks
                                            //(can be 0-65535 ticks, 1 ti = 0.977 ms)
        otapi_new_session(&app_session);
    }
    
    /// 2. Open a Broadcast Addressed request (Simplest)
    otapi_open_request(BROADCAST, NULL);
    
    /// 3. Write some command to the Request. I chose Discovery Collection. 
           
    {   // The command tmpl dictates what protocol templates come next
        command_tmpl command;
        command.type        = 0x40;
        command.opcode      = 0x04;
        command.extension   = 0;
        otapi_put_command_tmpl(&status, &command);
    }
    {   // The dialog tmpl instructs the receivers how to respond
        dialog_tmpl dialog;
        dialog.timeout      = 60;           // should be same or less than app_session.timeout
        dialog.channels     = 0;            // use the active channel only
        otapi_put_dialog_tmpl(&status, &dialog);
    }
    {   // The ISFcomp tmpl makes receivers run a comparison check on the 
        // specified file (or batch files).  If you preceed with a query,
        // the comparision can be elaborate.  For broadcast, it just makes
        // sure only receivers that contain the specified file respond.
        isfcomp_tmpl isfcomp;
        isfcomp.is_series   = 0;
        isfcomp.isf_id      = ISF_ID(device_features);
        isfcomp.offset      = 0;
        otapi_put_isf_comp(&status, &isfcomp);
    }
    {   // The ISFcall tmpl tells receivers which file to respond, and what
        // data in the file.  (doesn't have to be the same as comp file)
        isfcall_tmpl isfcall;
        isfcall.is_series   = 0;
        isfcall.isf_id      = ISF_ID(device_features);
        isfcall.max_return  = 80;       // cap file response at 80 bytes
        isfcall.offset      = 0;        // start file response at first byte
        otapi_put_isf_call(&status, &isfcall);
    }
    
    /// 4. Close the request (package for output).  This allows footer data
    ///     to be written to the frame/packet.
    otapi_close_request();
    
    otapi_log_msg(4, 0, (ot_u8*)"PING", NULL);
    
    return True;
}
*/






/** Your Local Services
  * ============================================================================
  * - Things you want to run in parallel to OpenTag
  *     - Sensor monitoring
  *     - Non-DASH7 data I/O
  *     - User interface handling
  *     - Your implementation of space invaders
  *     - etc.
  *
  * - They can interface with OT via:
  *     - Normal use: re-mapping sys.loadapi to another app function
  *     - Veelite file access, especially for data logging or reconfiguration
  *     - Global variables (experts only!)
  *     - Manually calling OT routines and then pre-empting (experts only!)
  *     - Writing ALP-API information to dir_in buffer (not recommended)
  *     - Writing DASHForth applet onto dir_in (not yet available)
  *
  * @note
  * Services run during idle time where OpenTag is not doing anything.  If you
  * want to be an "expert" please check sys.mutex before pre-empting.  You may
  * also want to inspect the difference between OT_GPTIM's present value and
  * OT_GPTIM's reset value, which is the time until the next OpenTag event.
  */
void my_local_services(void) {
/// All I do now is turn-on an LED.
    platform_trig2_high();
}








/** Main Function
  * ============================================================================
  * - If you don't know what a main function is... I can't help you :)
  */

void main( void ) {
    
    /// Platform power-up initialization:
    platform_poweron();     // 1. Clocks, Timers, IRQ's, etc
    platform_init_gpio();   // 2. Not built into platform_poweron()
    platform_init_OT();     // 3. OpenTag module inits

    /// Set up the user timer (you can change this if you want).  This code is
    /// lifted from platform_CC430.c (platform_init_gptim)
    APPTIM->CTL    |= 0x0004;
    APPTIM->CTL     = (0x01C3 & 0x01E0);
    APPTIM->EX0     = (0x01C3 & 0x0007);
    APPTIM->CCR0    = 0;
    APPTIM->CTL    |= 0x0013;


    /// Bind the System RFA callbacks to use the red_ functions
    red_vector              = 0;
    sys.evt.RFA.init        = &red_init;
    sys.evt.RFA.terminate   = &red_terminate;
    
    /// Bind the api callback to our api-based app
    sys.loadapp             = &app_pingpong;
    
    
    /// Start the OpenTag Daemon:
    /// Run this once.  Afterward, it will automatically run whenever there is
    /// a GPTIM interrupt (usually TIM0A5).  OT manages this timer internally.
    platform_ot_run();
    
    
    /// Wait around until something interesting happens!
    /// - If you have code that runs in parallel, put it in here
    while (1) {
        my_local_services();
        SLEEP_MCU();
    }

}










/** Default File data allocations
  * ============================================================================
  * - Veelite also uses an additional 1536 bytes for wear leveling
  * - Wear leveling overhead is configurable, but fixed for all FS sizes
  * - Veelite virtual addressing allocations of key sectors below:
  *     Overhead:   0000 to 03FF        (1024 bytes alloc)
  *     ISFSB:      0400 to 049F        (160 bytes alloc)
  *     GFB:        04A0 to 089F        (1024 bytes)
  *     ISFB:       08A0 to 0FFF        (1888 bytes)
  */

#define SPLIT_SHORT(VAL)    (ot_u8)((ot_u16)(VAL) >> 8), (ot_u8)((ot_u16)(VAL) & 0x00FF)
#define SPLIT_LONG(VAL)     (ot_u8)((ot_u32)(VAL) >> 24), (ot_u8)(((ot_u32)(VAL) >> 16) & 0xFF), \
                            (ot_u8)(((ot_u32)(VAL) >> 8) & 0xFF), (ot_u8)((ot_u32)(VAL) & 0xFF)

#define SPLIT_SHORT_LE(VAL) (ot_u8)((ot_u16)(VAL) & 0x00FF), (ot_u8)((ot_u16)(VAL) >> 8)
#define SPLIT_LONG_LE(VAL)  (ot_u8)((ot_u32)(VAL) & 0xFF), (ot_u8)(((ot_u32)(VAL) >> 8) & 0xFF), \
                            (ot_u8)(((ot_u32)(VAL) >> 16) & 0xFF), (ot_u8)((ot_u32)(VAL) >> 24)

    
/// These overhead are the Veelite vl_header files. They are hard coded,
/// and they must be in the endian of the platform. (Little endian here)
#pragma DATA_SECTION(overhead_files, ".vl_ov")
const ot_u8 overhead_files[] = {     
    0x00, 0x00, 0x00, 0x01,                 /* GFB ELements 0 - 3 */
    0x00, GFB_MOD_standard, 
    0x00, 0x14, 0xFF, 0xFF, 
    0x00, 0x00, 0x00, 0x01, 
    0x01, GFB_MOD_standard, 
    0x00, 0x15, 0xFF, 0xFF, 
    0x00, 0x00, 0x00, 0x01, 
    0x02, GFB_MOD_standard, 
    0x00, 0x16, 0xFF, 0xFF, 
    0x00, 0x00, 0x00, 0x01, 
    0x03, GFB_MOD_standard, 
    0x00, 0x17, 0xFF, 0xFF,
    
    ISFS_LEN(transit_data), 0x00, 
    ISFS_ALLOC(transit_data), 0x00, 
    ISFS_ID(transit_data), 
    ISFS_MOD(transit_data),
    SPLIT_SHORT_LE(ISFS_BASE(transit_data)), 
    0xFF, 0xFF, 
    
    ISFS_LEN(capability_data), 0x00, 
    ISFS_ALLOC(capability_data), 0x00, 
    ISFS_ID(capability_data), 
    ISFS_MOD(capability_data),
    SPLIT_SHORT_LE(ISFS_BASE(capability_data)), 
    0xFF, 0xFF, 
    
    ISFS_LEN(query_results), 0x00, 
    ISFS_ALLOC(query_results), 0x00, 
    ISFS_ID(query_results), 
    ISFS_MOD(query_results),
    SPLIT_SHORT_LE(ISFS_BASE(query_results)),  
    0xFF, 0xFF, 
    
    ISFS_LEN(hardware_fault), 0x00, 
    ISFS_ALLOC(hardware_fault), 0x00, 
    ISFS_ID(hardware_fault), 
    ISFS_MOD(hardware_fault),
    SPLIT_SHORT_LE(ISFS_BASE(hardware_fault)),  
    0xFF, 0xFF, 
    
    ISFS_LEN(device_discovery), 0x00, 
    ISFS_ALLOC(device_discovery), 0x00, 
    ISFS_ID(device_discovery), 
    ISFS_MOD(device_discovery),
    SPLIT_SHORT_LE(ISFS_BASE(device_discovery)),  
    0xFF, 0xFF, 
    
    ISFS_LEN(device_capability), 0x00, 
    ISFS_ALLOC(device_capability), 0x00, 
    ISFS_ID(device_capability), 
    ISFS_MOD(device_capability),
    SPLIT_SHORT_LE(ISFS_BASE(device_capability)), 
    0xFF, 0xFF, 
    
    ISFS_LEN(device_channel_utilization), 0x00, 
    ISFS_ALLOC(device_channel_utilization), 0x00, 
    ISFS_ID(device_channel_utilization), 
    ISFS_MOD(device_channel_utilization),
    SPLIT_SHORT_LE(ISFS_BASE(device_channel_utilization)),  
    0xFF, 0xFF, 
    
    ISFS_LEN(location_data), 0x00, 
    ISFS_ALLOC(location_data), 0x00, 
    ISFS_ID(location_data), 
    ISFS_MOD(location_data),
    SPLIT_SHORT_LE(ISFS_BASE(location_data)),  
    0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    
    /* Mode 2 ISFs, written as little endian */
    ISF_LEN(network_settings), 0x00,                /* Length, little endian */
    SPLIT_SHORT_LE(ISF_ALLOC(network_settings)),    /* Alloc, little endian */
    ISF_ID(network_settings),                       /* ID */
    ISF_MOD(network_settings),                      /* Perms */
    SPLIT_SHORT_LE(ISF_BASE(network_settings)),
    SPLIT_SHORT_LE(ISF_MIRROR(network_settings)),
    
    ISF_LEN(device_features), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(device_features)),
    ISF_ID(device_features), 
    ISF_MOD(device_features),
    SPLIT_SHORT_LE(ISF_BASE(device_features)), 
    0xFF, 0xFF, 
    
    ISF_LEN(channel_configuration), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(channel_configuration)),
    ISF_ID(channel_configuration), 
    ISF_MOD(channel_configuration),
    SPLIT_SHORT_LE(ISF_BASE(channel_configuration)), 
    SPLIT_SHORT_LE(ISF_MIRROR(channel_configuration)), 
    
    ISF_LEN(real_time_scheduler), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(real_time_scheduler)),
    ISF_ID(real_time_scheduler), 
    ISF_MOD(real_time_scheduler),
    SPLIT_SHORT_LE(ISF_BASE(real_time_scheduler)), 
    0xFF, 0xFF,  
    
    24 /* Length of SSS file */, 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(sleep_scan_sequence)), 
    ISF_ID(sleep_scan_sequence), 
    ISF_MOD(sleep_scan_sequence),
    SPLIT_SHORT_LE(ISF_BASE(sleep_scan_sequence)), 
    0xFF, 0xFF, 
    
    24 /* Length of HSS file */, 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(hold_scan_sequence)),
    ISF_ID(hold_scan_sequence), 
    ISF_MOD(hold_scan_sequence),
    SPLIT_SHORT_LE(ISF_BASE(hold_scan_sequence)), 
    0xFF, 0xFF, 
    
    16 /* Length of BTS file */, 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(beacon_transmit_sequence)),
    ISF_ID(beacon_transmit_sequence), 
    ISF_MOD(beacon_transmit_sequence),
    SPLIT_SHORT_LE(ISF_BASE(beacon_transmit_sequence)), 
    0xFF, 0xFF, 
    
    ISF_LEN(protocol_list), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(protocol_list)),
    ISF_ID(protocol_list), 
    ISF_MOD(protocol_list),
    SPLIT_SHORT_LE(ISF_BASE(protocol_list)), 
    0xFF, 0xFF, 
    
    ISF_LEN(isfs_list), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(isfs_list)),
    ISF_ID(isfs_list), 
    ISF_MOD(isfs_list),
    SPLIT_SHORT_LE(ISF_BASE(isfs_list)), 
    0xFF, 0xFF, 
    
    ISF_LEN(gfb_file_list), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(gfb_file_list)), 
    ISF_ID(gfb_file_list), 
    ISF_MOD(gfb_file_list),
    SPLIT_SHORT_LE(ISF_BASE(gfb_file_list)), 
    0xFF, 0xFF, 
    
    ISF_LEN(location_data_list), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(location_data_list)),
    ISF_ID(location_data_list), 
    ISF_MOD(location_data_list),
    SPLIT_SHORT_LE(ISF_BASE(location_data_list)), 
    0xFF, 0xFF, 
    
    ISF_LEN(ipv6_addresses), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(ipv6_addresses)),
    ISF_ID(ipv6_addresses), 
    ISF_MOD(ipv6_addresses),
    SPLIT_SHORT_LE(ISF_BASE(ipv6_addresses)), 
    0xFF, 0xFF, 
    
    ISF_LEN(sensor_list), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(sensor_list)),
    ISF_ID(sensor_list), 
    ISF_MOD(sensor_list),
    SPLIT_SHORT_LE(ISF_BASE(sensor_list)), 
    0xFF, 0xFF, 
    
    ISF_LEN(sensor_alarms), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(sensor_alarms)),
    ISF_ID(sensor_alarms), 
    ISF_MOD(sensor_alarms),
    SPLIT_SHORT_LE(ISF_BASE(sensor_alarms)), 
    0xFF, 0xFF, 
    
    ISF_LEN(root_authentication_key), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(root_authentication_key)), 
    ISF_ID(root_authentication_key), 
    ISF_MOD(root_authentication_key),
    SPLIT_SHORT_LE(ISF_BASE(root_authentication_key)),  
    0xFF, 0xFF,
    
    ISF_LEN(user_authentication_key), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(user_authentication_key)),
    ISF_ID(user_authentication_key), 
    ISF_MOD(user_authentication_key),
    SPLIT_SHORT_LE(ISF_BASE(user_authentication_key)),  
    0xFF, 0xFF, 
    
    ISF_LEN(routing_code), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(routing_code)),
    ISF_ID(routing_code), 
    ISF_MOD(routing_code),
    SPLIT_SHORT_LE(ISF_BASE(routing_code)),  
    0xFF, 0xFF, 
    
    ISF_LEN(user_id), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(user_id)), 
    ISF_ID(user_id), 
    ISF_MOD(user_id),
    SPLIT_SHORT_LE(ISF_BASE(user_id)),  
    0xFF, 0xFF, 
    
    ISF_LEN(optional_command_list), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(optional_command_list)),
    ISF_ID(optional_command_list), 
    ISF_MOD(optional_command_list),
    SPLIT_SHORT_LE(ISF_BASE(optional_command_list)),  
    0xFF, 0xFF, 
    
    ISF_LEN(memory_size), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(memory_size)),
    ISF_ID(memory_size), 
    ISF_MOD(memory_size),
    SPLIT_SHORT_LE(ISF_BASE(memory_size)), 
    0xFF, 0xFF, 
    
    ISF_LEN(table_query_size), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(table_query_size)),
    ISF_ID(table_query_size), 
    ISF_MOD(table_query_size),
    SPLIT_SHORT_LE(ISF_BASE(table_query_size)), 
    0xFF, 0xFF, 
    
    ISF_LEN(table_query_results), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(table_query_results)), 
    ISF_ID(table_query_results), 
    ISF_MOD(table_query_results),
    SPLIT_SHORT_LE(ISF_BASE(table_query_results)), 
    0xFF, 0xFF, 
    
    ISF_LEN(hardware_fault_status), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(hardware_fault_status)), 
    ISF_ID(hardware_fault_status), 
    ISF_MOD(hardware_fault_status),
    SPLIT_SHORT_LE(ISF_BASE(hardware_fault_status)), 
    0xFF, 0xFF, 
    
    ISF_LEN(external_events_list), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(external_events_list)),
    ISF_ID(external_events_list), 
    ISF_MOD(external_events_list),
    SPLIT_SHORT_LE(ISF_BASE(external_events_list)),  
    0xFF, 0xFF, 
    
    ISF_LEN(external_events_alarm_list), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(external_events_alarm_list)),
    ISF_ID(external_events_alarm_list), 
    ISF_MOD(external_events_alarm_list),
    SPLIT_SHORT_LE(ISF_BASE(external_events_alarm_list)),  
    0xFF, 0xFF, 
    
    ISF_LEN(application_extension), 0x00, 
    SPLIT_SHORT_LE(ISF_ALLOC(application_extension)),
    ISF_ID(application_extension), 
    ISF_MOD(application_extension),
    SPLIT_SHORT_LE(ISF_BASE(application_extension)), 
    0xFF, 0xFF, 
};



/// This array contains stock codes for isfs.  They are ordered strings.
#pragma DATA_SECTION(isfs_stock_codes, ".vl_isfs")
const ot_u8 isfs_stock_codes[] = {   
    0x10, 0x11, 0x18, 0xFF, 
    0x12, 0x13, 0x14, 0x17, 0xFF, 0xFF, 
    0x15, 0xFF, 
    0x16, 0xFF, 
    0x00, 0x01, 
    0x01, 0x06, 0x07, 0x17, 
    0x02, 0x03, 0x04, 0x05, 
    0x11, 0xFF, 
};




#pragma DATA_SECTION(gfb_stock_files, ".vl_gfb")
const ot_u8 gfb_stock_files[] = {0xFF, 0xFF};





/// Firmware & Version information for ISF1 (Device Features)
/// This will look something like "OTv1  xyyyyyyy" where x is a letter and 
/// yyyyyyy is a Base64 string containing a 16 bit build-id and a 32 bit mask
/// indicating the features compiled-into the build.
#include "OT_version.h"

#define BV0     (ot_u8)(OT_VERSION_MAJOR + 48)
#define BT0     (ot_u8)(OT_BUILDTYPE)
#define BC0     OT_BUILDCODE0
#define BC1     OT_BUILDCODE1
#define BC2     OT_BUILDCODE2
#define BC3     OT_BUILDCODE3
#define BC4     OT_BUILDCODE4
#define BC5     OT_BUILDCODE5
#define BC6     OT_BUILDCODE6
#define BC7     OT_BUILDCODE7

/// This array contains the stock ISF data.  ISF data must be big endian!
#pragma DATA_SECTION(isf_stock_files, ".vl_isf")
const ot_u8 isf_stock_files[] = {    
    /* network settings: id=0x00, len=8, alloc=8 */
    __VID,                                              /* VID */
    0x11,                                               /* Device Subnet */
    0x00,                                               /* Beacon Subnet */
    b00000100, b11000110,                               /* Active Setting */
    0x00,                                               /* Default Device Flags */
    3,                                                  /* Beacon Attempts */
    SPLIT_SHORT(256),                                   /* Hold Scan Sequence Cycles */
    
    /* device features: id=0x01, len=46, alloc=46 */
    __UID,                                              /* UID: 8 bytes*/
    b00000110, b11000110,                               /* Supported Setting */
    M2_PARAM(MAXFRAME),                                 /* Max Frame Length */
    1,                                                  /* Max Frames per Packet */
    SPLIT_SHORT(0),                                     /* DLLS Methods */
    SPLIT_SHORT(0),                                     /* NLS Methods */
    SPLIT_SHORT(ISF_TOTAL_BYTES),                       /* ISFB Total Memory */
    SPLIT_SHORT(ISF_TOTAL_BYTES-ISF_HEAP_BYTES),        /* ISFB Available Memory */
    SPLIT_SHORT(ISFS_TOTAL_BYTES),                      /* ISFSB Total Memory */
    SPLIT_SHORT(ISFS_TOTAL_BYTES-ISFS_HEAP_BYTES),      /* ISFSB Available Memory */
    SPLIT_SHORT(GFB_TOTAL_BYTES),                       /* GFB Total Memory */
    SPLIT_SHORT(GFB_TOTAL_BYTES-GFB_HEAP_BYTES),        /* GFB Available Memory */
    SPLIT_SHORT(GFB_FILE_BYTES),                        /* GFB File Size */
    0,                                                  /* RFU */
    OT_FEATURE(SESSION_DEPTH),                          /* Session Stack Depth */
    'O','T','v',BV0,' ',' ',
    BT0,BC0,BC1,BC2,BC3,BC4,BC5,BC6,BC7, 0,             /* Firmware & Version as C-string */
    
    /* channel configuration: id=0x02, len=32, alloc=64 */
    0x00,                                               /* Channel Spectrum ID */                     
    0x00,                                               /* Channel Parameters */
    (ot_u8)(( (-15) + 40 )*2),                          /* Channel TX Power Limit */
    (ot_u8)( 100 ),                                     /* Channel Link Quality Filter Level */
    (ot_u8)( (-85) + 140 ),                             /* CS RSSI Threshold */
    (ot_u8)( (-92) + 140 ),                             /* CCA RSSI Threshold*/
    0x00,                                               /* Regulatory Code */
    0x01,                                               /* Duty Cycle (100%) */
    
    0x10,                                               /* Channel Spectrum ID */                     
    0x00,                                               /* Channel Parameters */
    (ot_u8)(( (-15) + 40 )*2),                          /* Channel TX Power Limit */
    (ot_u8)( 100 ),                                     /* Channel Link Quality Filter Level */
    (ot_u8)( (-85) + 140 ),                             /* CS RSSI Threshold */
    (ot_u8)( (-92) + 140 ),                             /* CCA RSSI Threshold*/
    0x00,                                               /* Regulatory Code */
    0x01,                                               /* Duty Cycle (100%) */
    
    0x12,                                               /* Channel Spectrum ID */                     
    0x00,                                               /* Channel Parameters */
    (ot_u8)(( (-15) + 40 )*2),                          /* Channel TX Power Limit */
    (ot_u8)( 100 ),                                     /* Channel Link Quality Filter Level */
    (ot_u8)( (-85) + 140 ),                             /* CS RSSI Threshold */
    (ot_u8)( (-92) + 140 ),                             /* CCA RSSI Threshold*/
    0x00,                                               /* Regulatory Code */
    0x01,                                               /* Duty Cycle (100%) */
    
    0x2D,                                               /* Channel Spectrum ID */                     
    0x00,                                               /* Channel Parameters */
    (ot_u8)(( (-15) + 40 )*2),                          /* Channel TX Power Limit */
    (ot_u8)( 100 ),                                     /* Channel Link Quality Filter Level */
    (ot_u8)( (-80) + 140 ),                             /* CS RSSI Threshold */
    (ot_u8)( (-90) + 140 ),                             /* CCA RSSI Threshold*/
    0x00,                                               /* Regulatory Code */
    0x01,                                               /* Duty Cycle (100%) */
    
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    
    
    /* real time scheduler: id=0x03, len=12, alloc=12 */
    0x00, 0x0F,                                         /* SSS Sync Mask */
    0x00, 0x08,                                         /* SSS Sync Value */
    0x00, 0x03,                                         /* HSS Sync Mask */
    0x00, 0x02,                                         /* HSS Sync Value */
    0x00, 0x03,                                         /* BTS Sync Mask */
    0x00, 0x02,                                         /* BTS Sync Value */
    
    /* sleep scan periods: id=0x04, len=12, alloc=32 */
    /* Period data format in Section X.9.4.5 of Mode 2 spec */
    0x10, 0x3F, 0x00, 0x80,                             /* Channel X scan, Scan Code, Next Scan ms */
    0x2D, 0x1F, 0x00, 0x80,                             /* NOTE: Scan Code should be less than     */
    0x10, 0x2F, 0x00, 0x80,                             /*       Next Scan, or else you will be    */
    0x2D, 0x2F, 0x00, 0x80,                             /*       doing nothing except scanning!    */
    0x10, 0x1F, 0x00, 0x80,
    0x2D, 0x3F, 0x00, 0x80,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,

    /* hold scan periods: id=0x05, len=12, alloc=32 */
    /* Period data format in Section X.9.4.5 of Mode 2 spec */
    0x10, 0x3F, 0x00, 0x80,                             /* Channel X scan, Scan Code, Next Scan ms */
    0x2D, 0x1F, 0x00, 0x80,
    0x10, 0x2F, 0x00, 0x80,
    0x2D, 0x2F, 0x00, 0x80,
    0x10, 0x1F, 0x00, 0x80,
    0x2D, 0x3F, 0x00, 0x80,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    
    /* beacon transmit periods: id=0x06, len=12, alloc=24 */
    /* Period data format in Section X.9.4.7 of Mode 2 spec */
    0x10, 0x00, 0x20, 0x00, 0x00, 0x08, 0x10, 0x00,     /* Channel X beacon, Beacon ISF File, Next Beacon ms */
    0x2D, 0x00, 0x20, 0x00, 0x00, 0x08, 0x10, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    
    /* Protocol List: id=0x07, len=4, alloc=16 */
    0x00, 0x50, 0x60, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF,     /* List of Protocols supported (Tentative)*/
    0x60, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    
    /* ISFS list: id=0x08, len=12, alloc=24 */
    0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x18,     
    0x80, 0x81, 0x82, 0x83, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    
    /* GFB File List: id=0x09, len=4, alloc=8 */
    0x00, 0x01, 0x02, 0x03, 0xFF, 0xFF, 0xFF, 0xFF,
                                                        
    /* Location Data List: id=0x0A, not used in this build */
    
    /* IPv6 Addresses: id=0x0B, len=0, alloc=48 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    
    /* Sensor List:         id=0x0C, not used in this build */
    /* Sensor Alarms:       id=0x0D, not used in this build */
    /* root auth key:       id=0x0E, not used in this build */
    /* Admin auth key:      id=0x0F, not used in this build */
    
    /* Routing Code: id=0x10, len=0, alloc=50 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,     
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF,
    
    /* User ID: id=0x11, len=0, alloc=60 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,     
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 
    
    /* Mode 1 Optional Command list: id=0x12, len=7, alloc=8 */
    0x13, 0x93, 0x0C, 0x0E, 0x60, 0xE0, 0x8E, 0xFF,     
    
    /* Mode 1 Memory Size: id=0x13, len=12, alloc=12 */
    0x00, 0x00, 0x01, 0x00,                             
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00,
    
    /* Mode 1 Table Query Size: id=0x14, len=1, alloc=2 */
    0x00, 0xFF,                                         
    
    /* Mode 1 Table Query Results: id=0x15, len=7, alloc=8 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,     
    
    /* HW Fault Status: id=0x16, len=3, alloc=4 */
    0x00, 0x00, 0x00, 0xFF,                             
    
    /* Ext Services List:   id=0x17, not used in this build */
    /* Ext Services Alarms: id=0x18, not used in this build */
    
    /* Application Extension: id=0xFF, len=0, alloc=16 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,     
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

