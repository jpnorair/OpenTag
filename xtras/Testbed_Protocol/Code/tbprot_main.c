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
  * @file       /Platform/POSIX_Client/main_console.c
  * @author     JP Norair
  * @version    V1.0
  * @date       13 August 2011
  * @brief      Terminal-based console app for Mpipe & ALP control
  * @defgroup   Client_Console
  * @ingroup    Client_Console
  *
  * This is a program for managing the mpipe between an OpenTag server and a
  * POSIX client.  It runs inside a terminal/shell.  When a GUI is eventually
  * made, it may reference this system.
  ******************************************************************************
  */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "OTAPI.h"
#include "OT_platform.h"
#include "session.h"
#include "system.h"
#include "veelite.h"

#include "input_parser.h"



// Device ID's for the testbed (somewhat arbitrary)
#define __UID    0x1D, 0xAA, 0xAA, 0x1D, 0xBB, 0xBB, 0xBB, 0xB0
#define __VID    0x1D, 0xB0



/** Stuff that replaces parts of the OTlib that we are not testing
  * ============================================================================
  */
sys_struct sys;

void radio_init() { }
void sys_init() { }





/** Local Function Prototypes
  * ============================================================================
  */
int     tstate = 0;
FILE*   fp_req;
FILE*   fp_resp;

  
void program_defaults();


/** @brief Processes a received frame and runs it through upper layers
  * @param pcode    (ot_int) The number of frames remaining in the packet or,
  *                             when negative, a radio reception error
  * @param fcode    (ot_int) Set to negative if the received frame is known to
  *                             be damaged.
  * @retval none
  * @sa rcevt_frx()
  *
  * @note This function is identical to rcevt_frx() from system.c, but it is
  * transplanted here so that the whole system.c does not have to be compiled 
  * (system.c contains some things that are difficult to simulate).
  */
void sub_rcevt_frx(ot_int pcode, ot_int fcode);

ot_bool sub_mac_filter();

int test_against_reference(FILE* fp_ref, Queue* q_ref, Queue* q_test);







/** Local Functions
  * ============================================================================
  */
void bail_out(const char* kill_msg, int err_code) {
    if (err_code != 0) {
        fprintf(stderr, "\nFAIL, %s: code = %d\n", kill_msg, err_code);
    }
    else {
        fprintf(stderr, "\n%s reached EOF: Test complete\n", kill_msg);
    }
    fclose(fp_req);
    fclose(fp_resp);
    
    exit(0);
}
  
  
  
void sub_rcevt_frx(ot_int pcode, ot_int fcode) {
    ot_int frx_code = 0;
    
    if (pcode < 0) {
        frx_code = -5;
    }
    else {
        m2session*  session = session_top();
        ot_bool     dstream = (ot_bool)(session->netstate & M2_NETSTATE_DSDIALOG);
        
        /// Handle damaged frames (CRC)
        /// - Multiframe datastreams: mark the packet as bad, but proceed
        /// - Normal data packets (single frame): ignore the packet
        if (fcode < 0) {
            frx_code = -1;
            if (dstream)    m2dp_mark_dsframe(session);
            else            goto rcevt_frx_end;
        }
        
        /// Test link budget and subnet on all good frames         
        else if (sub_mac_filter() == False) {
            frx_code = -2;
            goto rcevt_frx_end;
        }
        
        // If the packet is done, process it
        if (pcode == 0) {
            ot_int test_qual;
            //radio_sleep();
            //sys.mutex     = SYS_MUTEX_PROCESSING;
            session->subnet = sys.netconf.subnet;
            
            test_qual       = network_route_ff(session);
            frx_code        = (test_qual < 0) ? -3 : 1;
            
            if (test_qual > 0) {
                //sub_fceval(test_qual);
            }
            
            //sys.idle_state = (session->flags & M2FI_LISTEN) ? \
                                        M2_MACIDLE_LISTEN : M2_MACIDLE_HOLD;
        }
    }
    
    rcevt_frx_end: 

    /// Finish up when parsing has been completed (frx_code != 0)
    if (frx_code != 0) {
        if (frx_code < 0) {
            //sys_idle( 0 );  // RX error
            fprintf(stdout, "--> Frame received incorrectly (frx_code=%d)\n", frx_code);
        }
        //sys.mutex = 0;
        //platform_ot_preempt();
    }
}

ot_bool sub_mac_filter() {
/// Link Budget Filtering (LBF) is a normalized RSSI Qualifier.
/// Subnet Filtering is an numerical qualifier
    ot_bool qualifier;
    {
        // TX EIRP encoded value    = (dBm + 40) * 2
        // TX EIRP dBm              = ((encoded value) / 2) - 40
        // Link Loss                = TX EIRP dBm - Detected RX dBm
        // Link Quality Filter      = (Link Loss <= Link Loss Limit)
        ot_int linkloss;
        linkloss    = ((ot_int)((rxq.front[1] >> 1) & 0x3F) - 40) - (-75);  //radio_rssi(); 
        qualifier   = (ot_bool)(linkloss <= (ot_int)90 );  //phymac[0].link_qual);
    }
    {
        ot_u8 fr_subnet, dsm, specifier, mask;
        
        fr_subnet   = rxq.front[2];
        dsm         = sys.netconf.subnet & 0x0F;
        mask        = fr_subnet & dsm;
        specifier   = (fr_subnet ^ sys.netconf.subnet) & 0xF0;
        fr_subnet  &= 0xF0;
        qualifier  &= (ot_bool)(((fr_subnet == 0xF0) || (specifier == 0)) && (mask == dsm));
    }
    
    return qualifier;
}









int test_against_reference(FILE* fp_ref, Queue* q_ref, Queue* q_test) {
    int test;
    
    while (1) {
        switch (tstate) {
            // Go past any sort of comments, and deal with syntax errors
            case 0: test    = parse_sequence(fp_ref, q_ref);
                    tstate += (test > 0);
                    if (test < 0) {
                        bail_out("Response Reference", test+1);
                    }
                    break;
            
            // Read all the data until ";" and process it
            case 1: test = parse_sequence(fp_ref, q_ref);
                    if (test == -3) {
                        tstate++;
                    }
                    else if (test < 0) {
                        bail_out("Response Reference", test+1);
                    }
                    break;
        
            case 2: {
                int i;
                int testlen = q_length(q_test);
                int testx   = 255;
                
                if (q_length(q_test) != q_length(q_ref)) {
                    fprintf(stdout, "... Warning, response is not same length as reference.\n");
                    testx   = 0;
                    testlen = (q_length(q_test) >= q_length(q_ref)) ? \
                                q_length(q_test) : q_length(q_ref);
                }
                
                for (i=0; i<testlen; i++) {
                    if (q_test->front[i] != q_ref->front[i]) {
                        testx = 0;
                        fprintf(stdout, "... Inequality at position %d: {test, ref} = {0x%02X, 0x%02X}\n", \
                                i, q_test->front[i], q_ref->front[i]);
                    }
                }
            
                if (testx != 0) {
                    fprintf(stdout, "!!! Test & Reference Response are the same !!!\n\n");
                }
                else {
                    fprintf(stdout, "??? Test Response has errors (printing response) ???\n");
                    for (i=0; i<q_length(q_ref); ) {
                        if ((i & 7) == 0) {
                            fprintf(stdout, "%03d: ", i);
                        }
                        fprintf(stdout, "%02X ", q_test->front[i]);
                        i++;
                        if ((i & 7) == 0) {
                            fputc('\n', stdout);
                        }
                    }
                    if ((i & 7) != 0) {
                        fputc('\n', stdout);
                    }
                }
                
                tstate = 0;
                return 0;
            }
                    
            default: bail_out("Response State Machine", 1);
        }
    }
    
    return -1;
}






int main(int argc, char** argv) {
    Queue   reftxq;
    ot_u8   reftxq_buf[512];
    ot_bool usage_error = True;
    int     state       = 0;
    int     test;
    
    
    if (argc > 2) {
        fp_req = fopen(argv[1], "r");
        if (fp_req != NULL) {
            fp_resp = fopen(argv[2], "r");
            if (fp_req != NULL) {
                usage_error = False;
            }
            else {
                fprintf(stderr, "Error, could not open file: %s\n", argv[2]);
            }
        }
        else {
            fprintf(stderr, "Error, could not open file: %s\n", argv[1]);
        }
    }
    
    if (usage_error) {
        fprintf(stderr, "Usage: tbprotocol [request references] [response references]\n");
        exit(0);
    }
    
    // Simulation device startup routine
    // - Program initial data to Flash analogue (custom routine for posix sims)
    // - Run normal platform power-on init
    program_defaults();
    platform_poweron();
    
    // Ripped from sys_init()
    session_init();
    {
        Twobytes scratch;
        vlFILE* fp;
        
        fp = ISF_open_su( 0x00 );
        scratch.ushort          = vl_read(fp, 2);
        sys.netconf.subnet      = scratch.ubyte[0];
        sys.netconf.b_subnet    = scratch.ubyte[1];
        scratch.ushort          = vl_read(fp, 6);
        sys.netconf.dd_flags    = scratch.ubyte[0];
        sys.netconf.b_attempts  = scratch.ubyte[1];
        sys.netconf.active      = vl_read(fp, 4);
        sys.netconf.hold_limit  = vl_read(fp, 8);
        vl_close(fp);
    }
    
    // initialize reference queue
    q_init(&reftxq, reftxq_buf, 512);
    q_empty(&rxq);
    q_empty(&txq);
    
    // Load the test data to the RXQ, process it, and then compare the results
    // (stored in the TXQ) against the reference.
    while (1) {
        switch (state) {
            // Go past any sort of comments, and deal with syntax errors
            case 0: test    = parse_sequence(fp_req, &rxq);
                    state  += (test > 0);
                    if (test < 0) {
                        bail_out("Request Reference", test+1);
                    }
                    break;
            
            // Read all the data until ";" and process it
            case 1: test = parse_sequence(fp_req, &rxq);
                    if (test == -3) {
                        // Create a new session for the dialog
                        m2session* session;
                        session = session_new(0, (M2_NETSTATE_REQRX | M2_NETSTATE_INIT), 0x10);
                        session->subnet = sys.netconf.subnet;
                        
                        // Run RX process
                        sub_rcevt_frx(0, 0);
                        
                        // These operations (on the response) are done at the 
                        // MAC layer (below the protocol layer).  So in this
                        // testbed we have to run them here.
                        txq.front[0] = (ot_u8)q_length(&txq);
                        txq.front[1] = 80;
                        
                        // Test
                        test_against_reference(fp_resp, &reftxq, &txq);
                        
                        // Flush everything for next test
                        session_flush();
                        q_empty(&rxq);
                        q_empty(&txq);
                        q_empty(&reftxq);
                        state = 0;
                    }
                    else if (test < 0) {
                        bail_out("Request Reference", test+1);
                    }
                    break;
                    
            default: bail_out("Request State Machine", 1);
                     break;
        }
    }
    
    return 0;
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
static const ot_u8 overhead_files[] = {     
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
static const ot_u8 isfs_stock_codes[] = {   
    0x10, 0x11, 0xFF, 0xFF, 
    0x12, 0x13, 0x14, 0xFF, 
    0x15, 0xFF, 
    0x16, 0xFF, 
    0x00, 0x01, 
    0x01, 0x06, 0x07, 0x17, 
    0x02, 0x03, 0x04, 0x05, 
    0x0A, 0xFF, 
};




static const ot_u8 gfb_stock_files[] = {0xFF, 0xFF};





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
static const ot_u8 isf_stock_files[] = {    
    /* network settings: id=0x00, len=8, alloc=8 */
    __VID,                                              /* VID */
    0x11,                                               /* Device Subnet */
    0x00,                                               /* Beacon Subnet */
    b00000100, b11000110,                               /* Active Setting */
    0x00,                                               /* Default Device Flags */
    3,                                                  /* Beacon Attempts */
    SPLIT_SHORT(256),                                   /* Hold Scan Sequence Cycles */
    
    /* device features: id=0x01, len=48, alloc=48 */
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




void sub_load_overhead_data() {
    ot_long     i;
    ot_long     data_amount;
    ot_u16*     cursor;
                                         
    // Write overhead data
    cursor      = (ot_u16*)overhead_files;
    data_amount = (GFB_NUM_FILES + ISFS_NUM_LISTS + ISF_NUM_STOCK_FILES) * sizeof(vl_header);
    for (i=0; i<data_amount; i+=2) {
        vworm_mark(i, *cursor); 
        cursor++;
    }
}

void sub_load_isfs_data() {
    ot_long     i;
    ot_long     data_amount;
    ot_u16*     cursor;
    
    // Write ISFS data
    cursor      = (ot_u16*)isfs_stock_codes;
    data_amount = (ISFS_STOCK_HEAP_BYTES);
    data_amount += ISFS_START_VADDR;
    for (i=ISFS_START_VADDR; i<data_amount; i+=2) {
        vworm_mark(i, *cursor);
        cursor++;
    } 
}

void sub_load_isf_data() {
    ot_long     i;
    ot_long     data_amount;
    ot_u16*     cursor;                                        

    // Write ISF File data
    cursor      = (ot_u16*)isf_stock_files;
    data_amount = (ISF_HEAP_BYTES);
    data_amount += ISF_START_VADDR;
    for (i=ISF_START_VADDR; i<data_amount; i+=2) {
        vworm_mark(i, *cursor);
        cursor++;
    }
}

void program_defaults() {
    vworm_format();
    sub_load_overhead_data();
    sub_load_isfs_data();
    sub_load_isf_data();
}

