
#include "OTAPI.h"
#include "OT_platform.h"
#include "system.h"
#include "session.h"
#include "radio.h"
#include "debug_uart.h"


/** Compile-Time Device ID configuration <BR>
  * ===========================================================================
  * If you are just using two devices for testing, they can probably use the
  * same ID's without conflict (conflict is relative, it only matters because
  * it is hard to tell which device is actually sending the response).  You can
  * also select a ID from below at compile-time (or add more) .
  */
#define __USE_ID0
//#define __USE_ID1

#if defined(__USE_ID0)
#   define __UID    0x1D, 0xAA, 0xA0, 0x1D, 0xB0, 0xB0, 0xB0, 0xB0
#   define __VID    0x1D, 0xC0
#elif defined(__USE_ID1)
#   define __UID    0x1D, 0xAA, 0xA1, 0x1D, 0xB1, 0xB1, 0xB1, 0xB1
#   define __VID    0x1D, 0xC1
#else
#   error "Device ID index is not selected"
#endif

#if (SYS_RECEIVE != ENABLED)
#   error SYS_RECEIVE   // want to test receiver?
#endif

static void test_radio_fgrx(void);

char tim11_compared;
static ot_int rx_fcode;

void sub_print_queue(Queue* q) {
    int i, j;
    
    if (q == &rxq) {
        if (rx_fcode == 0)
            debug_printf("[32m"); // for rxq printing: green foreground
        else
            debug_printf("[31m"); // for rxq printing: red foreground on rx failure
    }

    debug_printf("Printing Queue: length=%d\r\n", q_length(q));
    
    for (i=0; i<q_length(q); ) {
        debug_printf("%04X: ", i); 
        for (j=0; (j<16) && (i<q_length(q)); j++, i++) {
            debug_printf("%02X ", q->front[i]);
        }
        debug_printf("\r\n");
    }
    debug_printf("[0m\r\n");
}

ot_bool repeat_fgtx = False;
ot_bool repeat_fgrx = False;
volatile int cnt;

volatile ot_bool got_rx_callback;
ot_u8 test_channel; // spectrum ID
ot_u8 test_rx_timeout;
ot_u8 test_length = 0;

static void
init_sys_comm(ot_u8 channel)
{
    /// Setup System Comm Variables, which are used by the MAC, Network, and
    /// Transport Layers in the complete build
    dll.comm.tca            = 2048;
    dll.comm.rx_timeout     = test_rx_timeout << 8;
    dll.comm.csmaca_params  = (M2_CSMACA_NA2P | M2_CSMACA_MACCA);
    dll.comm.redundants     = 1;
    dll.comm.tx_channels    = 1;
    dll.comm.rx_channels    = 1;
    dll.comm.tx_chanlist    = &dll.comm.scratch[0];
    dll.comm.tx_chanlist    = &dll.comm.scratch[1];
    dll.comm.scratch[0]     = channel;
    dll.comm.scratch[1]     = channel;
}

static int
create_session()
{
    m2session* session;
    int data_length, i;

    /// Basic initialization
    q_start(&txq, 0, 0);
    q_start(&rxq, 0, 0);
    
    /// Create a new ad-hoc session on channel 0x10.  Fill up the remaining
    /// session parameters with [effectively] dummy values.
    session = session_new(0, M2_NETSTATE_INIT, 0x10);
    if (session == NULL) {
        debug_printf("-> Session could not be created (Fatal error)\r\n");
        return -1;
    }
    session->dialog_id  = platform_prand_u8();
    session->subnet     = 0xF0;
    session->extra      = 0;
    session->flags      = 0;
    session->channel    = test_channel;
    
    init_sys_comm(session->channel);
        
    /// Load up the first few bytes of the frame, which are:
    /// Length, TX EIRP, Subnet, Frame Info
    /// - TX EIRP is filled automatically by PHY
    /// - Except length, these fields don't really matter for chain test
    
    if (test_length == 0) {
        data_length     = platform_prand_u8();  // 4 to 255 bytes
        if (data_length < 4)
            data_length  = 4;
    } else
        data_length = test_length;

    txq.front[0]    = data_length+2;
    txq.front[1]    = 0;
    txq.front[2]    = session->subnet;
    txq.front[3]    = (session->flags & 0xC0) | (0x20) | (0x02);
    txq.putcursor   = &txq.front[4];
 //#txq.length      = 4;
    
    /// write the rest of the data to the queue
    for (i=0; i<(data_length-4); i++) {
        ot_u8 input;
        input = platform_prand_u8();
        q_writebyte(&txq, input);
    }
    sub_print_queue(&txq);
    
    return 0;
}


void
test_rcevt_frx(ot_int pcode, ot_int fcode) 
{
    debug_printf("rcevt_frx(");
    switch (pcode) {
        case RM2_ERR_KILL: debug_printf("RM2_ERR_KILL"); break;
        case RM2_ERR_CCAFAIL: debug_printf("RM2_ERR_CCAFAIL"); break;
        case RM2_ERR_TIMEOUT: debug_printf("RM2_ERR_TIMEOUT"); break;
        case RM2_ERR_BADCHANNEL: debug_printf("RM2_ERR_BADCHANNEL"); break;
        case RM2_ERR_GENERIC: debug_printf("RM2_ERR_GENERIC"); break;
        default: debug_printf("%d", pcode); break;
    } // ..switch (pcode)
    debug_printf(", %d)\r\n", fcode);

    rx_fcode = fcode;

    // pcode: the number of frames remaining, or a radio error when negative
    if (pcode < 0) {
    } else {
    }

    got_rx_callback = True;
}

void
test_rcevt_ftx(ot_int pcode, ot_int scratch)
{
    radio_sleep();                      

    debug_printf("rcevt_ftx(%d, %d)\r\n", pcode, scratch);

    if (pcode == 0) {
        // packet done
    } else if (pcode == 1) {
        // Non-final frame TX'ed in multiframe packet
    } else {
        // Error of some sort
    }
}

static void
test_radio_fgtx()
{
    ot_int csma_code;
    ot_u16 wait_until = 0;

    //rm2_txinit_ff(ot_int est_frames, ot_sig2 callback);
    rm2_txinit_ff(1, &test_rcevt_ftx);

    do {
        csma_code = rm2_txcsma();
        if (csma_code >= 0)
            wait_until = OT_GPTIM->CNT + csma_code; // todo: handle rollover

        switch (csma_code) {
            /// While TX is in CSMA, subtract radio process time from Tca
            case RM2_ERR_BADCHANNEL:
                    debug_printf("RM2_ERR_BADCHANNEL = rm2_txcsma()\r\n");
                    break;
            case RM2_ERR_CCAFAIL:
                    debug_printf("RM2_ERR_CCAFAIL = rm2_txcsma()\r\n");
                    break;
        }

        if (csma_code >= 0) {
            while (OT_GPTIM->CNT < wait_until)  // todo: handle rollover
                asm("nop");
        }

    } while (csma_code != -1);
}

static ot_bool do_rx;

void
console_service()
{
    if (new_uart_rx == 0)
        return;

    do_rx = False;

    spi_save_restore(True);

    radio_console_service();

    if (uart_rx_buf[0] == 't' && uart_rx_buf[1] == 'x') {
        create_session();
        test_radio_fgtx();
    } else if (uart_rx_buf[0] == 'T' && uart_rx_buf[1] == 'X') {
        repeat_fgtx ^= True;
        cnt = 0;
    } else if (uart_rx_buf[0] == 'r' && uart_rx_buf[1] == 'x') {
        do_rx = True;
    } else if (uart_rx_buf[0] == 'R' && uart_rx_buf[1] == 'X') {
        repeat_fgrx ^= True;
        do_rx = repeat_fgrx;
    } else if (uart_rx_buf[0] == 't' && uart_rx_buf[1] == ' ') {
        from_hex(uart_rx_buf+2, &test_rx_timeout);
    } else if (uart_rx_buf[0] == 's' && uart_rx_buf[1] == 0) {
        debug_printf("\r\nsid: %02x", test_channel);
    } else if (uart_rx_buf[0] == 's' && uart_rx_buf[1] == ' ') {
        from_hex(uart_rx_buf+2, &test_channel);
    } else if (uart_rx_buf[0] == 'l' && uart_rx_buf[1] == ' ') {
        from_hex(uart_rx_buf+2, &test_length);
    }

    debug_printf("\r\n> ");

    new_uart_rx = 0;
    uart_rx_buf_idx = 0;

    spi_save_restore(False);

    if (do_rx)
        test_radio_fgrx();
}


static void
test_radio_fgrx()
{
    got_rx_callback = False;

    do {
        init_sys_comm(test_channel);

        // (channel, netstate, frames, callback)
        rm2_rxinit_ff(test_channel, M2_NETSTATE_CONNECTED, 1, &test_rcevt_frx);

        if (!got_rx_callback)
            debug_printf("waiting for rx callback...\r\n");
        while (!got_rx_callback && do_rx) {
            asm("nop");
            if (new_uart_rx)
                console_service();
        } // ..while (!got_rx_callback)
        got_rx_callback = False;

        sub_print_queue(&rxq);
    } while (repeat_fgrx);
}



int
main(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    /// Platform power-up initialization:
    platform_poweron();     // 1. Clocks, Timers, IRQ's, etc
    platform_init_OT();     // 2. OpenTag module inits, radio_init, buffer_init

    /// Set up the user timer (you can change this if you want).
    TIM_TimeBaseStructure.TIM_Period = 0xffff;
    TIM_TimeBaseStructure.TIM_Prescaler = 4;    // 4=122us per
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM11, &TIM_TimeBaseStructure);

    TIM_ETRClockMode2Config(TIM11, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);

    // put gptim into the so-called "free running", counting from 0->0xffff without ISR firing
    platform_flush_gptim();

    test_rx_timeout = 0x4;  // 0x400 = 1024 = 1 second
    test_channel = 0x12;

    for (;;) {
        console_service();
        if (repeat_fgtx) {
            if (++cnt > 0x70000) {
                cnt = 0;
                create_session();
                test_radio_fgtx();
            }
        }
        asm("nop");
        asm("nop");
    } // ...for (;;)

/*
    while (1) {
        SLEEP_MCU();
        if (tim11_compared) {
            tim11_compared = 0;
        }
    } // ...while (1)
*/
}


/*************************************************************************************/


#define SPLIT_SHORT(VAL)    (ot_u8)((ot_u16)(VAL) >> 8), (ot_u8)((ot_u16)(VAL) & 0x00FF)
#define SPLIT_LONG(VAL)     (ot_u8)((ot_u32)(VAL) >> 24), (ot_u8)(((ot_u32)(VAL) >> 16) & 0xFF), \
                            (ot_u8)(((ot_u32)(VAL) >> 8) & 0xFF), (ot_u8)((ot_u32)(VAL) & 0xFF)

#define SPLIT_SHORT_LE(VAL) (ot_u8)((ot_u16)(VAL) & 0x00FF), (ot_u8)((ot_u16)(VAL) >> 8)
#define SPLIT_LONG_LE(VAL)  (ot_u8)((ot_u32)(VAL) & 0xFF), (ot_u8)(((ot_u32)(VAL) >> 8) & 0xFF), \
                            (ot_u8)(((ot_u32)(VAL) >> 16) & 0xFF), (ot_u8)((ot_u32)(VAL) >> 24)


/// These overhead are the Veelite vl_header files. They are hard coded,
/// and they must be in the endian of the platform. (Little endian here)

__attribute__((section(".vl_ov")))
const ot_u8 overhead_files[] = {
    //0x00, 0x00, 0x00, 0x01,                 /* GFB ELements 0 - 3 */
    //0x00, GFB_MOD_standard,
    //0x00, 0x14, 0xFF, 0xFF,
    //0x00, 0x00, 0x00, 0x01,
    //0x01, GFB_MOD_standard,
    //0x00, 0x15, 0xFF, 0xFF,
    //0x00, 0x00, 0x00, 0x01,
    //0x02, GFB_MOD_standard,
    //0x00, 0x16, 0xFF, 0xFF,
    //0x00, 0x00, 0x00, 0x01,
    //0x03, GFB_MOD_standard,
    //0x00, 0x17, 0xFF, 0xFF,

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
    0xFF, 0xFF, /*SPLIT_SHORT_LE(ISF_MIRROR(channel_configuration)), */

    ISF_LEN(real_time_scheduler), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(real_time_scheduler)),
    ISF_ID(real_time_scheduler),
    ISF_MOD(real_time_scheduler),
    SPLIT_SHORT_LE(ISF_BASE(real_time_scheduler)),
    0xFF, 0xFF,

    ISF_LEN(sleep_scan_sequence), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(sleep_scan_sequence)),
    ISF_ID(sleep_scan_sequence),
    ISF_MOD(sleep_scan_sequence),
    SPLIT_SHORT_LE(ISF_BASE(sleep_scan_sequence)),
    0xFF, 0xFF,

    ISF_LEN(hold_scan_sequence), 0x00,
    SPLIT_SHORT_LE(ISF_ALLOC(hold_scan_sequence)),
    ISF_ID(hold_scan_sequence),
    ISF_MOD(hold_scan_sequence),
    SPLIT_SHORT_LE(ISF_BASE(hold_scan_sequence)),
    0xFF, 0xFF,

    ISF_LEN(beacon_transmit_sequence), 0x00,
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
__attribute__((section(".vl_isfs")))
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


#if (GFB_TOTAL_BYTES > 0)
__attribute__((section(".vl_gfb")))
const ot_u8 gfb_stock_files[] = {0xFF, 0xFF};
#endif




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
__attribute__((section(".vl_isf")))
const ot_u8 isf_stock_files[] = {
    /* network settings: id=0x00, len=8, alloc=8 */
    __VID,                                              /* VID */
    0x11,                                               /* Device Subnet */
    0x11,                                               /* Beacon Subnet */
    SPLIT_SHORT(OT_ACTIVE_SETTINGS),                    /* Active Setting */
    0x00,                                               /* Default Device Flags */
    3,                                                  /* Beacon Attempts */
    SPLIT_SHORT(2),                                     /* Hold Scan Sequence Cycles */

    /* device features: id=0x01, len=46, alloc=46 */
    __UID,                                              /* UID: 8 bytes*/
    SPLIT_SHORT(OT_SUPPORTED_SETTINGS),                 /* Supported Setting */
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
    OT_PARAM(SESSION_DEPTH),                          /* Session Stack Depth */
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
    0x10, 0x41, 0x0C, 0x00,                             /* Channel X scan, Scan Code, Next Scan ms */
    0xFF, 0xFF, 0xFF, 0xFF,                             /* NOTE: Scan Code should be less than     */
    0xFF, 0xFF, 0xFF, 0xFF,                             /*       Next Scan, or else you will be    */
    0xFF, 0xFF, 0xFF, 0xFF,                             /*       doing nothing except scanning!    */
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,

    /* hold scan periods: id=0x05, len=12, alloc=32 */
    /* Period data format in Section X.9.4.5 of Mode 2 spec */
    0x10, 0x43, 0x00, 0x01,                             /* Channel X scan, Scan Code, Next Scan ms */
    0x10, 0x3F, 0x00, 0xA0,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,

    /* beacon transmit periods: id=0x06, len=12, alloc=24 */
    /* Period data format in Section X.9.4.7 of Mode 2 spec */ //0x0240
    0x10, 0x06, 0x20, 0x00, 0x00, 0x08, 0x00, 0x20,     /* Channel X beacon, Beacon ISF File, Next Beacon ms */
    0x10, 0x06, 0x20, 0x00, 0x00, 0x08, 0x00, 0x20,
    0x10, 0x06, 0x20, 0x00, 0x00, 0x08, 0x0B, 0x00,

    /* App Protocol List: id=0x07, len=4, alloc=16 */
    0x00, 0x01, 0x02, 0x04, 0xFF, 0xFF, 0xFF, 0xFF,     /* List of Protocols supported (Tentative)*/
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* ISFS list: id=0x08, len=12, alloc=24 */
    0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x18,
    0x80, 0x81, 0x82, 0x83, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* GFB File List: id=0x09, len=4, alloc=8 */
    0x00, 0x01, 0x02, 0x03, 0xFF, 0xFF, 0xFF, 0xFF,

    /* Location Data List: id=0x0A, len=0, alloc=96 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* IPv6 Addresses: id=0x0B, len=0, alloc=48 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* Sensor List: id=0x0C, len=16, alloc=16 (just dummy values right now) */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x00,

    /* Sensor Alarms: id=0x0D, len=2, alloc=2 (just dummy values right now) */
    0x00, 0x00,

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

