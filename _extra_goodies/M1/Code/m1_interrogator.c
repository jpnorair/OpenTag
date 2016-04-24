/***********************************************************************************
 * Validates ISO18000-7 Mode1 PHY functionality, when used as interrogator          *
 ***********************************************************************************/
#include <otplatform.h>
#include <m2/radio.h>
#include "debug_uart.h"
#include "m1_commands.h"

#include <otlib/rand.h>

static ot_bool sem = False;

/**************** not using OTlib for PHY test... ********************/
void sys_init(void) { }
void session_init(void) { }
void auth_init(void) { }
void vl_init(void) { }
ot_int mpipe_init(void* port_id) { return 0; }
ot_u8 vworm_init(void) { return 1; /* 1 for not implemented */ }
ot_uint sys_event_manager(ot_uint elapsed)
{
    sem = True;
    return 16;  // the number of ticks between wakeups
}
/*********************** ...not using OTlib for PHY test ********************/

ot_bool poll_version = False;

typedef struct {
    ot_bool broadcast;
    ot_u8   command_code;
    int     cmd_arg_length;
    ot_u8*   cmd_args;
    ot_u16  tag_mfg_id;
    ot_u32  tag_sn;
} to_tag_args_t;


ot_u16 session_id;

void
interrogator_to_tag(to_tag_args_t *args)
{
    ot_u8* ptr;

    q_empty(&txq);

    q_writebyte(&txq, PROTOCOL_ID);                     // 0
    if (args->broadcast) {
        // broadcast (tag S/N and mfg ID not present)
        q_writebyte(&txq, 0x04);                        // 1: packet options    (only bit1 is used)
        q_writebyte(&txq, args->cmd_arg_length + 8);    // 2: packet length (from protocol ID field up to and included CRC field)
    } else {
        // point-to-point (tag S/N and mfg ID are present)
        q_writebyte(&txq, 0x06);    // 1: packet options    (only bit1 is used)
        q_writebyte(&txq, args->cmd_arg_length + 14);    // 2: packet length (from protocol ID field up to and included CRC field)
        q_writebyte(&txq, args->tag_mfg_id >> 8);       // 3        mode1 is big-endian over the air
        q_writebyte(&txq, args->tag_mfg_id & 0xff);     // 4
        // tag s/n:
        txq.putcursor[0] = args->tag_sn >> 24;          // 5
        txq.putcursor[1] = (args->tag_sn >> 16) & 0xff; // 6
        txq.putcursor[2] = (args->tag_sn >> 8) & 0xff;  // 7
        txq.putcursor[3] = args->tag_sn & 0xff;         // 8
        txq.putcursor += 4;
     //#txq.length += 4;
    }

    q_writebyte(&txq, session_id >> 8);     // 3        mode1 is big-endian over the air
    q_writebyte(&txq, session_id & 0xff);   // 4
    q_writebyte(&txq, args->command_code);    // 5: command code    // bcast:5
    ptr = args->cmd_args;
    while (args->cmd_arg_length > 0) {
        q_writebyte(&txq, *ptr++);    // n: command arguments
        args->cmd_arg_length--;
    }

    //debug_printf("txq.length:%d\r\n", txq.length);
    from_tag = False;
    rm2_txinit_ff(0, NULL);
}



void
console_service()
{
    to_tag_args_t args;
    ot_u8 cmd_args[8];

    if (new_uart_rx == 0)
        return;

    radio_console_service();

    args.broadcast = False;
    args.cmd_arg_length = 0;
    args.cmd_args = NULL;
    args.tag_mfg_id = 0xbeef;
    args.tag_sn = 0x12345678;

    if (uart_rx_buf[0] == '?') {
        debug_printf("\r\nc     collection");
        debug_printf("\r\nv     request version");
        debug_printf("\r\nm     request modelnum");
        debug_printf("\r\ns     sleep");
        debug_printf("\r\nS     sleep all but");
        debug_printf("\r\nW     send wake signal");
        debug_printf("\r\nt     unmodulated transmit");
    } else if (uart_rx_buf[0] == 'c') {
        args.command_code = CMD_R_COLLECTION_UDB;
        args.broadcast = True;
        cmd_args[0] = 0; // window size MSbyte, count of 57.3ms intervals
        cmd_args[1] = 5; // window size LSbyte, count of 57.3ms intervals
        cmd_args[2] = 64; // max packet length
        cmd_args[3] = UDB_TYPECODE_TRANSIT_DATA; // udb type code (table 40)
        args.cmd_arg_length = 4;
        args.cmd_args = cmd_args;
        interrogator_to_tag(&args);
        rm2_rxinit_ff(0, 0, 0, NULL);
    } else if (uart_rx_buf[0] == 'V') {
        poll_version ^= 1;
    } else if (uart_rx_buf[0] == 'v') {
        args.command_code = CMD_R_FIRMWARE_VERSION;
        interrogator_to_tag(&args);
        rm2_rxinit_ff(0, 0, 0, NULL);
    } else if (uart_rx_buf[0] == 'm') {
        args.command_code = CMD_R_MODEL_NUM;
        interrogator_to_tag(&args);
        rm2_rxinit_ff(0, 0, 0, NULL);
    } else if (uart_rx_buf[0] == 's') { // sleep
        args.command_code = CMD_W_SLEEP;
        interrogator_to_tag(&args);
    } else if (uart_rx_buf[0] == 'S') { // sleep all but
        args.broadcast = True;
        args.command_code = CMD_W_SLEEP_ALL_BUT;
        cmd_args[0] = 0xbe; // mfg id hi    MSbyte first
        cmd_args[1] = 0xef; // mfg id lo
        cmd_args[2] = 0x01; // tagsn    MSbyte first
        cmd_args[3] = 0x02; // tagsn
        cmd_args[4] = 0x03; // tagsn
        cmd_args[5] = 0x04; // tagsn
        args.cmd_arg_length = 6;
        args.cmd_args = cmd_args;
        interrogator_to_tag(&args);
    } else if (uart_rx_buf[0] == 'W') {
        // send wake signal
        rm2_txinit_bf(NULL);
    }

    debug_printf("\r\n> ");

    new_uart_rx = 0;
    uart_rx_buf_idx = 0;
}


static void
ver_req_test()
{
    to_tag_args_t args;

    args.broadcast = False;
    args.cmd_arg_length = 0;
    args.cmd_args = NULL;
    args.tag_mfg_id = 0xbeef;
    args.tag_sn = 0x12345678;
    args.command_code = CMD_R_FIRMWARE_VERSION;
    interrogator_to_tag(&args);
    rm2_rxinit_ff(0, 0, 0, NULL);
}

static void
parse_rx()
{
    /* from tag parsing */
    ot_u32 rx_tag_sn;
    ot_u16 mfg_id, tag_session_id;
    tag_status_t tag_status;
    ot_u8 packet_length, command_code, data_length;
    ot_u8 prot_id = q_readbyte(&rxq);   // 0

    tag_status.word = q_readbyte(&rxq);   // 1
    tag_status.word <<= 8;  // mode1 is big-endian over the air
    tag_status.word += q_readbyte(&rxq);   // 2

    packet_length = q_readbyte(&rxq); // 3
    data_length = packet_length - 15;

    tag_session_id = q_readbyte(&rxq);  // 4
    tag_session_id <<= 8;   // mode1 is big-endian over the air
    tag_session_id += q_readbyte(&rxq);  // 5

    mfg_id = q_readbyte(&rxq);  // 6
    mfg_id <<= 8;   // mode1 is big-endian over the air
    mfg_id += q_readbyte(&rxq); // 7

    if (prot_id != PROTOCOL_ID) {
        debug_printf("prot_id=%x ", prot_id);
        // should i drop it?
    }
    if (tag_session_id != session_id)
        debug_printf("sid=%x vs %x ", tag_session_id, session_id);    // what to do with mismatched session_id?

    rx_tag_sn = rxq.getcursor[0] << 24; // 8
    rx_tag_sn += rxq.getcursor[1] << 16; // 9
    rx_tag_sn += rxq.getcursor[2] << 8; // 10
    rx_tag_sn += rxq.getcursor[3]; // 11
    rxq.getcursor += 4;

    command_code = q_readbyte(&rxq); // 12

    debug_printf("len=%d(%d) mfg=%x sn=%x cmd=%x ", packet_length, q_length(&rxq), mfg_id, (unsigned int)rx_tag_sn, command_code);
    while (data_length > 0) {
        debug_printf("%02x ", q_readbyte(&rxq)); // 13+
        data_length--;
    }

    debug_printf("\r\n");
}

/*static void
send_wake_test()
{
    // send wake signal
    debug_printf("send_wake_test()\r\n");
    rm2_txinit_bf(NULL);
}*/

volatile ot_u16 foobar;

int
main()
{
    platform_poweron();     // Clocks, Timers, IRQ's, etc
    platform_init_OT();     // OpenTag module inits

    rx_filter = RX_FILTER_TAG_ONLY;

    do {
        session_id = rand_prn8();
        session_id <<= 8;
        session_id += rand_prn8();
    } while (session_id == 0);
    //session_id = 0xdead;

    platform_ot_run();

    // set_chip_ mode(CHIP_MODE_TRANSMITTER, 0);

    while (1) {
        console_service();
        SLEEP_MCU();
        if (tim4dma_int != TIM4DMA_NONE)
            radio_dma_service();
        if (rxq_ready) {
            rxq_ready = False;
            parse_rx();
            //if you want to receive more... rm2_rxinit_ff(0, 0, 0, NULL);    // start receiver
        }
        if (sem) {
            sem = False;
            //if (++foobar > 512) {   // 512 = 8s
            //send_wake_test();
            if (poll_version) {
                if (++foobar > 32) {    // 32 = 500ms
                    foobar = 0;
                    ver_req_test();
                }
            }
        } // ...if (sem)
    }

}
