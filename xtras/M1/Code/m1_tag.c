/*********************************************************************************** 
 * Validates ISO18000-7 Mode1 PHY functionality, when used as tag                  *
 ***********************************************************************************/
#include "OT_platform.h"
#include "radio.h"
#include "debug_uart.h"
#include "m1_commands.h"

static ot_bool tag_sleep;
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

    if (tag_sleep)
        return 2253;  // 2.2 second sleep
    else
        return 16;  // the number of ticks between wakeups
}
/**************** ...not using OTlib for PHY test ********************/


ot_u8 bar;



const ot_u32 tag_sn = 0x12345678;

tag_status_t tag_status;
ot_u16 session_id;
ot_u16 tag_mfg_id;

extern volatile ot_bool tag_test_tx;

static void
delay(int d)
{
    while (d-- > 0) {
        asm("nop");
        asm("nop");
    }
}

void
tag_to_interrogator(ot_u8 command_code, const ot_u8 *response_data, int response_data_length)
{
    q_empty(&txq);

    q_writebyte(&txq, PROTOCOL_ID);                 // 0
    q_writebyte(&txq, tag_status.word >> 8);        // 1   mode1 is big-endian over the air
    q_writebyte(&txq, tag_status.word & 0xff);      // 2
    q_writebyte(&txq, response_data_length + 15);   // 3   packet length
    q_writebyte(&txq, session_id >> 8);             // 4    mode1 is big-endian over the air
    q_writebyte(&txq, session_id & 0xff);           // 5
    q_writebyte(&txq, tag_mfg_id >> 8);             // 6    mode1 is big-endian over the air
    q_writebyte(&txq, tag_mfg_id & 0xff);           // 7

    txq.putcursor[0] = tag_sn >> 24;            // 8
    txq.putcursor[1] = (tag_sn >> 16) & 0xff;   // 9
    txq.putcursor[2] = (tag_sn >> 8) & 0xff;    // 10
    txq.putcursor[3] = tag_sn & 0xff;           // 11
    txq.putcursor += 4;
    txq.length += 4;

    q_writebyte(&txq, command_code);            // 12

    debug_printf("rd %d: ", response_data_length);
    while (response_data_length > 0) {
        debug_printf("%02x ", *response_data);
        q_writebyte(&txq, *response_data);            // 13+
        response_data++;
        response_data_length--;
    }

    // 14,15 crc appended by radio phy
    debug_printf("txq.length=%d\r\n", txq.length);

    from_tag = True;
    rm2_txinit_ff(0, NULL);
}

const ot_u8* fw_ver = (ot_u8 *)"0123";

static void
reply_firmware_version()
{
    tag_to_interrogator(CMD_R_FIRMWARE_VERSION, fw_ver, 4);
}

static void
reply_error(iso18000_cmd_e cmd, iso8000_error_e err)
{
    ot_u8 err_data[4];

    debug_printf("reply_error %x %x\r\n", cmd, err);
    // Table 8
    err_data[0] = err;  // error code
    // Sub-code             optional
    // Error Parameter Data optional
    // Mfg Data             optional

    tag_status.bits.nack = 1;

    tag_to_interrogator(cmd, err_data, 1);
}

ot_u16 window_size, current_window, tx_window;
ot_u8 max_packet_length;
ot_u8 udb_typecode;

static void
collection_response(void)
{
    ot_u8 data[8];

    data[0] = udb_typecode; // udb type code
    data[1] = 0; // total udb data length MSbyte
    data[2] = 3; // total udb data length LSbyte
    data[3] = 0; // request offset
    data[4] = 0; // request offset
    // (one or more) udb data follows (TLD)
    data[5] = UDB_ELEMENT_TYPE__ROUTING_CODE; // T: element type ID (table 30)
    data[6] = 1; // L: length of following data
    data[7] = 1; // D: content
    tag_to_interrogator(CMD_R_COLLECTION_UDB, data, data[2]);
}


static void
set_tx_window(void)
{
    tx_window = platform_prand_u8();

    if (window_size > 0xff)
        tx_window += platform_prand_u8();

    tx_window %= window_size;
    tx_window++;    // 0 would mean no transmit
    debug_printf("txw%d\r\n", tx_window);
}

/* return True: receiver (re)enabled after return */
static ot_bool
parse_rx()
{
    /* from interrogator parsing */
    ot_u8 prot_id = q_readbyte(&rxq);
    ot_u8 packet_opt = q_readbyte(&rxq);
    ot_u8 packet_length = q_readbyte(&rxq);
    ot_u32 rx_tag_sn = 0;
    iso18000_cmd_e cmd;
    ot_bool en_rx = True;

    if (prot_id != PROTOCOL_ID) {
        debug_printf("prot=%02x ", prot_id);
        // ignore this message?
    }

    debug_printf("opt=%02x len=%d(%d) ", packet_opt, packet_length, rxq.length);

    if (packet_opt & 0x02) {
        // point-to-point (s/n and mfg-id present)
        tag_mfg_id = q_readbyte(&rxq);
        tag_mfg_id <<= 8;   // mode 1 is big endian over the air
        tag_mfg_id += q_readbyte(&rxq);
        debug_printf("mfg=%04x ", tag_mfg_id);
        rx_tag_sn = rxq.getcursor[0] << 24;
        rx_tag_sn += rxq.getcursor[1] << 16;
        rx_tag_sn += rxq.getcursor[2] << 8;
        rx_tag_sn += rxq.getcursor[3];
        rxq.getcursor += 4;
        debug_printf("sn=0x%x ", (unsigned int)rx_tag_sn);
    } else {
        // broadcast (s/n and mfg-id not present)
        debug_printf("bcast ");
    }

    session_id = q_readbyte(&rxq);
    session_id <<= 8;
    session_id += q_readbyte(&rxq);
    cmd = q_readbyte(&rxq);
    debug_printf("sid=%04x ", session_id);

    /* received message considered "well formed": reset sleep timeout */
    TIM_SetCounter(APPTIM, 0);


    if (packet_opt & 0x02) {
        if (rx_tag_sn != tag_sn) {
            // not for me
            debug_printf("not me\r\n");
            return en_rx;
        }

        switch (cmd) {
            case CMD_W_SLEEP:
                debug_printf("CMD_W_SLEEP ");
                tag_sleep = True;
                TIM_Cmd(APPTIM, DISABLE);
                radio_sleep();
                break;
            case CMD_R_FIRMWARE_VERSION:
                delay(5000);
                reply_firmware_version();
                break;
            default:
                debug_printf("cmd=%02x", cmd);
                reply_error(cmd, ERROR_INVALID_COMMAND_CODE);
                break;
        } // ...switch (cmd)
    } else {
        // received broadcast

        switch (cmd) {
            case CMD_R_COLLECTION_UDB:
                window_size = q_readbyte(&rxq);
                window_size <<= 8;   // mode 1 is big-endian over the air
                window_size += q_readbyte(&rxq);
                debug_printf("window_size=%d\r\n", window_size);
                max_packet_length = q_readbyte(&rxq);
                udb_typecode = q_readbyte(&rxq);
                current_window = 0;
                set_tx_window();
                TIM_SetCounter(TIM2, 0);
                TIM_ClearFlag(TIM2, TIM_FLAG_CC1);
                TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
                TIM_Cmd(TIM2, ENABLE);
                en_rx = False;
                // @todo: collection with UDB
                break;
            // @todo: table query
            case CMD_W_SLEEP_ALL_BUT:
                // tag_sn/mfg is in the command arguments
                tag_mfg_id = q_readbyte(&rxq);
                tag_mfg_id <<= 8;   // mode 1 is big-endian over the air
                tag_mfg_id += q_readbyte(&rxq);
                debug_printf("SAB mfg=%04x ", tag_mfg_id);
                rx_tag_sn = rxq.getcursor[0] << 24;
                rx_tag_sn += rxq.getcursor[1] << 16;
                rx_tag_sn += rxq.getcursor[2] << 8;
                rx_tag_sn += rxq.getcursor[3];
                rxq.getcursor += 4;
                debug_printf("sn=0x%x ", (unsigned int)rx_tag_sn);
                if (rx_tag_sn != tag_sn) {
                    // i'm not the one who should stay awake
                    debug_printf("sleep\r\n");
                    tag_sleep = True;
                    TIM_Cmd(APPTIM, DISABLE);
                    radio_sleep();
                }
                break;
            default:
                // unknown command: no response to broadcast
                break;
        } // ...switch (cmd)

        /* received message considered "well formed": reset sleep timeout */
        TIM_SetCounter(APPTIM, 0);
    }

    debug_printf("\r\n");
    return en_rx;
}

void
console_service()
{
    if (new_uart_rx == 0)
        return;

    radio_console_service();

    debug_printf("\r\n> ");

    new_uart_rx = 0;
    uart_rx_buf_idx = 0;
}

void
rcevt_bscan(ot_int scode, ot_int fcode)
{
    if (scode == 0 && fcode == 0) {
        tag_sleep = False;
        debug_printf("rcevt_bscan() wake\r\n");

        //TIM_SetCompare1(APPTIM, 3072);  // 3072 = 3sec
        TIM_SetCompare1(APPTIM, 30720);  // 30720 = 30sec
        TIM_SetCounter(APPTIM, 0);
        TIM_ClearFlag(APPTIM, TIM_FLAG_CC1);
        TIM_Cmd(APPTIM, ENABLE);

        rm2_rxinit_ff(0, 0, 0, NULL);    // start receiver
    }
}

//volatile ot_u16 foobar;

extern volatile ot_bool tim2_cc1;   // from tim2 ISR

int
main()
{
    TIM_TimeBaseInitTypeDef tbase_in;
    NVIC_InitTypeDef NVIC_InitStructure;

    platform_poweron();     // Clocks, Timers, IRQ's, etc
    platform_init_OT();     // OpenTag module inits

    /* APPTIM: puts me to sleep after 30 seconds. */
    tbase_in.TIM_Prescaler      = 32;
    tbase_in.TIM_Period         = 0xffff;   // ARR
    tbase_in.TIM_CounterMode    = TIM_CounterMode_Up;
    tbase_in.TIM_ClockDivision  = TIM_CKD_DIV1;
    TIM_TimeBaseInit(APPTIM, &tbase_in);
    TIM_ETRClockMode2Config(APPTIM, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);

    /*********************** TIM2: window timer... ***********************************/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseStructInit(&tbase_in);
    tbase_in.TIM_Period = 99; // ARR in 57.3ms
    tbase_in.TIM_Prescaler = 18335;    // 31.25ns prescaled to 573us
    tbase_in.TIM_ClockDivision = 0;
    tbase_in.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tbase_in);
    TIM_SetCompare1(TIM2, 99); // compare in 57.3ms

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);
    /*********************** ...TIM2: window timer ***********************************/

    platform_ot_run();

    tag_sleep = True; 
    rx_filter = RX_FILTER_INTEROGATOR_ONLY;

    rm2_rxinit_ff(0, 0, 0, NULL);    // start receiver

    window_size = 5;

    while (1) {
        console_service();

        sem = False;
        SLEEP_MCU();

        if (tim4dma_int != TIM4DMA_NONE)
            radio_dma_service();

        if (rxq_ready) {
            rxq_ready = False;
            //if (tag_sleep == False)
            if (parse_rx() == True)
                rm2_rxinit_ff(0, 0, 0, NULL);    // start receiver
        }

        if (tag_sleep && sem) {
            // periodic check for wake signal
            rm2_rxinit_bf(0, &rcevt_bscan);
        }


        if (!tag_sleep) {
            if (TIM_GetFlagStatus(APPTIM, TIM_FLAG_CC1) == SET) {
                debug_printf("apptim expired\r\n");
                tag_sleep = True;
                TIM_Cmd(APPTIM, DISABLE);
                radio_sleep();
            }
        } // ...if (!tag_sleep)

        // window timer
        if (tim2_cc1) {
            tim2_cc1 = False;
            ++current_window;
            GPIO_WriteBit(GPIO_Port_IO1,
                GPIO_Pin_IO1,
                (BitAction)(1 - GPIO_ReadOutputDataBit(GPIO_Port_IO1, GPIO_Pin_IO1))
            );
            if (current_window == tx_window) {
                collection_response();  // transmit
            }
            if (current_window == window_size) {
                TIM_Cmd(TIM2, DISABLE);
                rm2_rxinit_ff(0, 0, 0, NULL);    // start receiver
            }
            TIM_SetCounter(APPTIM, 0);
        }

    } // ...while (1)

}
