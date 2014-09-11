#include <otsys/syskern.h> // get SYS_* definitions
#include "stm32l1xx.h"
#include "stm32l1xx_it.h"
#include "sx1231_registers.h"
#include <app/board_config.h>
#include "stm32_sx1231_private.h"
#include <m2/encode.h>
#include <otlib/crc16.h>
#include <m2/radio.h>

spi2_state_e spi2_state = SPI2_STATE__NONE;
static volatile ot_u16 spi_rx_word;
static volatile ot_bool spi_busy = False;
static volatile ot_bool spi_16b_rx = False;

DMA_InitTypeDef SPI2_DMA_Init;


void
WriteReg_Sx1231__nonblocking(ot_u8 addr, ot_u8 val)
{
    ot_uni16 output;

    if (addr == REG_FIFOTHRESH && ((val & 0x7f) > RF_FEATURE_TXFIFO_BYTES) ) {
        for (;;)
            asm("nop");
    }

    // Copy address and register value to output variable
    output.ubyte[UPPER] = addr | 0x80;    // bit 7 is high for write register to radio
    output.ubyte[LOWER] = val;

    ASSERT_NSS_CONFIG();

    spi_busy = 1;
    SPI_I2S_SendData(SPI2, output.ushort);

    //lr = LR_SPI_WRN;
}

void
WriteReg_Sx1231(ot_u8 addr, ot_u8 val)
{
    ot_uni16 output;

    while (spi_busy == 1)
        ;

    if (addr == REG_FIFOTHRESH && ((val & 0x7f) > RF_FEATURE_TXFIFO_BYTES) ) {
        for (;;)
            asm("nop");
    }

    // Copy address and register value to output variable
    output.ubyte[UPPER] = addr | 0x80;    // bit 7 is high for write register to radio
    output.ubyte[LOWER] = val;

    ASSERT_NSS_CONFIG();

    /* Send it through the SPI2 peripheral */
    spi_busy = 1;
    spi_16b_rx = False;
    SPI_I2S_SendData(SPI2, output.ushort);

    /* Wait for receive to complete */
    while (spi_16b_rx == False)
        ;
    //lr = LR_SPI_WR;
}

ot_u8
ReadReg_Sx1231(ot_u8 addr)
{
    ot_uni16 output;

    while (spi_busy == 1)
        ;

    // Copy address and register value to output variable
    output.ubyte[UPPER] = addr; // bit 7 is low for read register from radio

    ASSERT_NSS_CONFIG();

    /* Send it through SPI2 peripheral */
    spi_busy = 1;
    spi_16b_rx = False;
    SPI_I2S_SendData(SPI2, output.ushort);

    /* Wait for receive to complete */
    while (spi_16b_rx == False)
        ;

    /* Return the byte read from the SPI bus */
    //lr = LR_SPI_RD;
    return spi_rx_word & 0xff;
}

void
ReadReg_Sx1231__nonblocking(ot_u8 addr, spi2_state_e s)
{
    ot_uni16 output;

/*    ignoring, called from SPI2_ISR: while (spi_busy == 1)
        ;*/

    // Copy address and register value to output variable
    output.ubyte[UPPER] = addr; // bit 7 is low for read register from radio

    ASSERT_NSS_CONFIG();

    spi2_state = s;    // handler for received data

    /* Send byte through the SPI2 peripheral */
    spi_busy = 1;
    SPI_I2S_SendData(SPI2, output.ushort);

    //lr = LR_SPI_RDN;
}

/***********************************************************************/

void EXTI4_IRQHandler(void)
{

    if (EXTI_GetITStatus(EXTI_Line4) != RESET) {
        /* SX1231-DIO0 PacketSent (end of transmission) */
        EXTI_ClearITPendingBit(EXTI_Line4);

#if (SYS_FLOOD == ENABLED)
        if (radio.flags & RADIO_FLAG_FLOOD) {
            debug_printf("exti4@flood\r\n");
            for (;;)
                asm("nop");
        }
#endif /* SYS_FLOOD == ENABLED */

        exti4_init.EXTI_LineCmd = DISABLE;
        EXTI_Init(&exti4_init);

        radio.evtdone(0, 0);  // will take radio out of transmit
        radio.state = RADIO_STATE_TXDONE;
    }
    //lr = LR_EXTI4;
}

volatile ot_u8 start_tx_from = 0;

static void
initiate_tx_burst(char context)
{
    int remaining;

    ASSERT_NSS_CONFIG();
    SPI_I2S_SendData(SPI2, 0x80);   // bit 7 to address 0x00: write to fifo
    spi2_state = SPI2_STATE__START_TX_DMA;
    start_tx_from = 10 + context;   // debug

    remaining = num_bytes_to_send - num_bytes_sent;
    /* FifoLevel threshold is set to half of fifo size */
    if (remaining > SX1231_FIFO_SIZE_HALF)
        SPI2_DMA_Init.DMA_BufferSize = SX1231_FIFO_SIZE_HALF;
    else
        SPI2_DMA_Init.DMA_BufferSize = remaining;
}


void EXTI9_5_IRQHandler(void)
{

    if (EXTI_GetITStatus(EXTI_Line5) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line5);
        /* PA5 SX1231-DIO1 FifoLevel */

#if (SYS_RECEIVE == ENABLED)
        if (radio.state == RADIO_STATE_RX) {
            /*if (GPIO_ReadInputDataBit(GPIO_Port_FifoLevel, GPIO_Pin_FifoLevel) == Bit_RESET) {
                for (;;)
                    asm("nop"); // bad sample
            }*/
            /* a received packet is now starting to come in */
            TIM_SetCounter(RXTIM, 0);   // keep it from tripping
            ASSERT_NSS_CONFIG();
            SPI_DataSizeConfig(SPI2, SPI_DataSize_8b); 
            SPI_I2S_SendData(SPI2, 0x00);   // bit 0 to address 0x00: read from fifo
            spi2_state = SPI2_STATE__START_RX_DMA;
        } else {
#endif /* SYS_RECEIVE == ENABLED */
            /*if (GPIO_ReadInputDataBit(GPIO_Port_FifoLevel, GPIO_Pin_FifoLevel) == Bit_SET) {
                for (;;)
                    asm("nop"); // bad sample
            }*/
            /* transmit: DIO1 FifoLevel deasserted, initiate another SPI2 TX burst */
            if (spi2_state == SPI2_STATE__START_TX_DMA) {
                initiate_tx_burst(2);
            }
#if (SYS_RECEIVE == ENABLED)
        }
#endif /* SYS_RECEIVE == ENABLED */

        exti9_5_init.EXTI_LineCmd = DISABLE;
        EXTI_Init(&exti9_5_init);
    } // ...if (EXTI_GetITStatus(EXTI_Line5) != RESET)
#ifdef RADIO_DEBUG
    else {
        debug_printf("EXTI9_5\r\n");
        for (;;)
            asm("nop");
    }
#endif /* RADIO_DEBUG */
    //lr = LR_EXTI9_5;
}

void SPI2_IRQHandler(void)
{

    if (SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_RXNE) == SET) {

        spi_rx_word = SPI_I2S_ReceiveData(SPI2);

        switch (spi2_state) {
#if (SYS_RECEIVE == ENABLED)
            case SPI2_STATE__START_RX_DMA:
                /* burst transfer to sx1231: address was just sent, now data from radio fifo */
                SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
                // tx dma only needed to initiate transfers
                SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
                SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);

                SPI2_DMA_Init.DMA_BufferSize = RegFifoThresh.bits.FifoThreshold;
                //debug_printf("{%d} ", SPI2_DMA_Init.DMA_BufferSize);
                SPI2_DMA_Init.DMA_MemoryBaseAddr = (uint32_t)&rf_data_buf[0];

                SPI2_DMA_Init.DMA_DIR = DMA_DIR_PeripheralDST;  // for tx
                DMA_Init(SPI2TX_DMA_CHANNEL, &SPI2_DMA_Init);
                SPI2_DMA_Init.DMA_DIR = DMA_DIR_PeripheralSRC;  // for rx
                DMA_Init(SPI2RX_DMA_CHANNEL, &SPI2_DMA_Init);

                DMA_ITConfig(SPI2RX_DMA_CHANNEL, DMA_IT_TC, ENABLE);
                DMA_Cmd(SPI2TX_DMA_CHANNEL, ENABLE);
                DMA_Cmd(SPI2RX_DMA_CHANNEL, ENABLE);
                break;
            case SPI2_STATE__RX_DMA_NEXT:
                DEASSERT_NSS_CONFIG();
                TIM_SetCounter(RXTIM, 0);   // keep it from tripping
                // is fifo threshold already exceeded?
                if (GPIO_ReadInputDataBit(GPIO_Port_FifoLevel, GPIO_Pin_FifoLevel) == Bit_SET) {
                    // yes, start next rx dma now
                    ASSERT_NSS_CONFIG();
                    SPI_DataSizeConfig(SPI2, SPI_DataSize_8b); 
                    SPI_I2S_SendData(SPI2, 0x00);   // bit 0 to address 0x00: read from fifo
                    spi2_state = SPI2_STATE__START_RX_DMA;
                } else {
                    // fifo not full enough, do another (or first) rssi if needed
                    if (radio.rssi_count < RSSI_SUM_COUNT) {
                        // sample rssi during packet reception, then poll FifoLevel
                        ReadReg_Sx1231__nonblocking(REG_RSSIVALUE, SPI2_STATE__RX_RSSI);
                    } else {
                        // rssi done: enable FifoLevel interrupt now. EXTI9_5
                        // @todo: sub_killonlowrssi()
                        exti9_5_init.EXTI_Trigger = EXTI_Trigger_Rising;
                        exti9_5_init.EXTI_LineCmd = ENABLE;
                        EXTI_Init(&exti9_5_init);
                        if (GPIO_ReadInputDataBit(GPIO_Port_FifoLevel, GPIO_Pin_FifoLevel) == Bit_SET) {
                            /* bad sample */
                            /*for (;;)
                                asm("nop"); // bad sample */
                            exti9_5_init.EXTI_LineCmd = DISABLE;
                            EXTI_Init(&exti9_5_init);
                            EXTI_ClearITPendingBit(EXTI_Line5);

                            ASSERT_NSS_CONFIG();
                            SPI_DataSizeConfig(SPI2, SPI_DataSize_8b); 
                            SPI_I2S_SendData(SPI2, 0x00);   // bit 0 to address 0x00: read from fifo
                            spi2_state = SPI2_STATE__START_RX_DMA;
                            if (GPIO_ReadInputDataBit(GPIO_Port_FifoLevel, GPIO_Pin_FifoLevel) == Bit_RESET) {
                                // this would be called "having a bad day"
                                debug_printf("FifoLevel\r\n");
                                for (;;)
                                    asm("nop");
                            }
                        } // ..if FifoLevel asserted
                    } // ..if rssi sampling done
                } // ..if FifoLevel de-asserted
                break;
            case SPI2_STATE__RX_RSSI:
                DEASSERT_NSS_CONFIG();
                radio.rssi_sum += spi_rx_word & 0xff;
                radio.rssi_count++;

                if (GPIO_ReadInputDataBit(GPIO_Port_FifoLevel, GPIO_Pin_FifoLevel) == Bit_SET) {
                    ASSERT_NSS_CONFIG();
                    SPI_DataSizeConfig(SPI2, SPI_DataSize_8b); 
                    SPI_I2S_SendData(SPI2, 0x00);   // bit 0 to address 0x00: read from fifo
                    spi2_state = SPI2_STATE__START_RX_DMA;
                } else {
                    // fifo not full enough, do another rssi
                    if (radio.rssi_count < RSSI_SUM_COUNT) {
                        // sample rssi during packet reception, then poll FifoLevel
                        ReadReg_Sx1231__nonblocking(REG_RSSIVALUE, SPI2_STATE__RX_RSSI);
                    } else {
                        // rssi done: enable FifoLevel interrupt now. EXTI9_5
                        // @todo: sub_killonlowrssi()
                        exti9_5_init.EXTI_Trigger = EXTI_Trigger_Rising;
                        exti9_5_init.EXTI_LineCmd = ENABLE;
                        EXTI_Init(&exti9_5_init);
                    }
                }
                break;
            case SPI2_STATE__RX_RSSI_TAIL:
                DEASSERT_NSS_CONFIG();
                radio.rssi_sum += spi_rx_word & 0xff;

                if (++radio.rssi_count < RSSI_SUM_COUNT)
                    ReadReg_Sx1231__nonblocking(REG_RSSIVALUE, SPI2_STATE__RX_RSSI_TAIL);
                else {
                    spi_busy = 0;
                    spi2_state = SPI2_STATE__NONE;
                    rx_done_isr(0);
                }
                break;
#endif /* SYS_RECEIVE == ENABLED */
            case SPI2_STATE__START_TX_DMA:
                /* burst transfer to sx1231: address was just sent, now data to radio fifo */
                SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
                // both tx & rx dma enable, tx occurs before rx
                SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);
                SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

                SPI2_DMA_Init.DMA_MemoryBaseAddr = (uint32_t)&rf_data_buf[num_bytes_sent];
                SPI2_DMA_Init.DMA_DIR = DMA_DIR_PeripheralDST;  // for tx
                DMA_Init(SPI2TX_DMA_CHANNEL, &SPI2_DMA_Init);
                SPI2_DMA_Init.DMA_DIR = DMA_DIR_PeripheralSRC;  // for rx
                DMA_Init(SPI2RX_DMA_CHANNEL, &SPI2_DMA_Init);

                //not using tx int DMA_ITConfig(SPI2TX_DMA_CHANNEL, DMA_IT_TC, ENABLE);
                DMA_ITConfig(SPI2RX_DMA_CHANNEL, DMA_IT_TC, ENABLE);
                DMA_Cmd(SPI2TX_DMA_CHANNEL, ENABLE);
                DMA_Cmd(SPI2RX_DMA_CHANNEL, ENABLE);
                //GPIO_WriteBit(GPIO_Port_CON2_40, GPIO_Pin_CON2_40, Bit_SET);    // tmp debug
                break;
            default:
                DEASSERT_NSS_CONFIG();
                if (SPI2->CR1 & SPI_DataSize_16b) {
                    // 16bit size: NSS_CONFIG
                    spi_16b_rx = True;
                }
#ifdef RADIO_DEBUG
                else {
                    // 8bit size: NSS_DATA
                    debug_printf("8bit\r\n");
                    for (;;)
                        asm("nop");
                }
#endif /* RADIO_DEBUG */
                spi_busy = 0;
                break;
        } // ...switch (spi2_state)

    } // ...if (SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_RXNE) == SET)
    //lr = LR_SPI2;
}

#if (SYS_RECEIVE == ENABLED)
static void
update_fifo_threshold(ot_u8 thr)
{
/*    if (thr > SX1231_FIFO_SIZE_HALF)
        thr = SX1231_FIFO_SIZE_HALF;*/
    if (thr > SX1231_FIFO_SIZE_HALF)
        RegFifoThresh.bits.FifoThreshold = SX1231_FIFO_SIZE_HALF;
    else
        RegFifoThresh.bits.FifoThreshold = thr;
/*    if (thr > 33) {
        for (;;)
            asm("nop");
    }*/
    if (RegFifoThresh.bits.FifoThreshold > 33) {
        debug_printf("ft>33\r\n");
        for (;;)
            asm("nop");
    }
    WriteReg_Sx1231__nonblocking(REG_FIFOTHRESH, RegFifoThresh.octet);
}
#endif /* SYS_RECEIVE == ENABLED */

void em2_decode_data_PN9();
#if (M2_FEATURE(FEC) == ENABLED)
void em2_decode_data_FEC();
#endif

void    /* SPI2 RX */
DMA1_Channel4_IRQHandler(void)
{

    if (DMA_GetITStatus(SPI2RX_DMA_IT_TC)) {
        DMA_ClearITPendingBit(SPI2RX_DMA_IT_TC);

        /* this interrupt occurs after spi activity finished */
        DEASSERT_NSS_CONFIG();

        SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, DISABLE);
        SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, DISABLE);

        DMA_Cmd(SPI2TX_DMA_CHANNEL, DISABLE);
        DMA_Cmd(SPI2RX_DMA_CHANNEL, DISABLE);

        SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
        if (spi2_state == SPI2_STATE__START_TX_DMA) {
            //GPIO_WriteBit(GPIO_Port_CON2_40, GPIO_Pin_CON2_40, Bit_RESET);    // tmp debug
            num_bytes_sent += SPI2_DMA_Init.DMA_BufferSize;
#if (SYS_FLOOD == ENABLED)
            /*if ( (radio.flags & RADIO_FLAG_FLOOD) && (num_bytes_sent > num_bytes_to_send) ) {
                for (;;)
                    asm("nop");
            }*/
            if ( (radio.flags & RADIO_FLAG_FLOOD) && (num_bytes_sent == num_bytes_to_send) ) {
                radio.evtdone(2, 0);    // might be taking us out of flood
                if (radio.flags & RADIO_FLAG_FLOOD_STOP) {
                    //debug_printf("FloodStop ");
                    radio.flags &= ~(RADIO_FLAG_FLOOD | RADIO_FLAG_FLOOD_STOP);
                    // PacketSent EXTI4 will terminate this
                } else {
                    num_bytes_sent = 0;
                    num_bytes_to_send = 0;
                    txq.getcursor = txq.front;  // to start
                    em2_encode_newframe();
                    radio_putbyte(PREAMBLE_BYTE);    
                    radio_putbyte(PREAMBLE_BYTE);    
                    radio_putbyte(PREAMBLE_BYTE);    
                    radio_putbyte(PREAMBLE_BYTE);    
                    // byte order for CC430 compatability
                    radio_putbyte(sync_value.ubyte[LOWER]);
                    radio_putbyte(sync_value.ubyte[UPPER]);
                    em2_encode_data();
                }
            }
#endif /* SYS_FLOOD == ENABLED */
            if (num_bytes_sent < num_bytes_to_send) {
                // more to send, always wait for FifoLevel to de-assert
                if (GPIO_ReadInputDataBit(GPIO_Port_FifoLevel, GPIO_Pin_FifoLevel) == Bit_RESET) {
                    initiate_tx_burst(1);
                } else {
                    // PA5   EXTI9_5 SX1231-DIO1
                    exti9_5_init.EXTI_Trigger = EXTI_Trigger_Falling;
                    exti9_5_init.EXTI_LineCmd = ENABLE;
                    EXTI_Init(&exti9_5_init);
                    start_tx_from = 4;
                }
            } else {
#if (SYS_FLOOD == ENABLED)
                if (radio.flags & RADIO_FLAG_FLOOD) {
                    debug_printf("flood\r\n");
                    for (;;)
                        asm("nop"); // looking for packet-end while flooding
                }
#endif /* SYS_FLOOD == ENABLED */
                // done filling fifo, wait for PacketSent: PA4 EXTI4 SX1231-DIO0
                exti4_init.EXTI_LineCmd = ENABLE;
                EXTI_Init(&exti4_init);

                SPI_DataSizeConfig(SPI2, SPI_DataSize_16b); 
                spi2_state = SPI2_STATE__NONE;
                start_tx_from = 0;
            }
        } else {    // ********** reception...
#if (SYS_RECEIVE == ENABLED)
            int i;
            /*if (GPIO_ReadInputDataBit(GPIO_Port_FifoLevel, GPIO_Pin_FifoLevel) == Bit_SET) {
                for (;;)    // FifoLevel asserted, fifo should be damn near empty
                    asm("nop");
            }*/
            num_bytes_sent = 0; // num bytes
            //debug_printf("bufsize:%d ", SPI2_DMA_Init.DMA_BufferSize);
            /* decoding here might be too time-consuming to call from ISR */
            // @todo: see if i can call function pointer directly from ISR
            if (em2_decode_data == &em2_decode_data_PN9)
                em2_decode_data_PN9();
#if (M2_FEATURE(FEC) == ENABLED)
            else if (em2_decode_data == &em2_decode_data_FEC)
                em2_decode_data_FEC();
#endif
            i = em2_remaining_bytes();
            //debug_printf("(%d)\r\n", i);
            //debug_printf("num_bytes_sent:%d,%d ", num_bytes_sent, i);
            SPI_DataSizeConfig(SPI2, SPI_DataSize_16b); 
            if (i < 0) {
                // damaged header, abort reception
                spi2_state = SPI2_STATE__NONE;
                radio.evtdone(em2_remaining_frames(), -1);   // report it as bad crc
            } else if (i == 0) {
                ot_int frames_left;
                // packet reception complete
                TIM_Cmd(RXTIM, DISABLE);
                spi_busy = 0;
                spi2_state = SPI2_STATE__NONE;
                frames_left = em2_remaining_frames();
                if (frames_left  > 0) {
                    radio.evtdone(frames_left, (ot_int)crc_check() - 1);
                    // Prepare the next frame by moving the "front" pointer and 
                    q_rebase(&rxq, rxq.putcursor);
                    // re-initializing the decoder engine
                    em2_decode_newframe();
                } else {
                    if (radio.rssi_count < RSSI_SUM_COUNT) {
                        /* small packet, hopefully this is flood reception
                         * or last samples could read transmitter unkeying */
                        ReadReg_Sx1231__nonblocking(REG_RSSIVALUE, SPI2_STATE__RX_RSSI_TAIL);
                    } else
                        rx_done_isr(i);
                }
            } else {
                // more to come..
                update_fifo_threshold(i);
                spi2_state = SPI2_STATE__RX_DMA_NEXT;
                TIM_SetCounter(RXTIM, 0);   // keep it from tripping
            }
#endif /* SYS_RECEIVE == ENABLED */
        } /// ..if (spi2_state != SPI2_STATE__START_TX_DMA)

    } // ...if (DMA_GetITStatus(SPI2RX_DMA_IT_TC))
    //lr = LR_DMA_4;
}


/* TIM10: RXTIM */
void
TIM10_IRQHandler(void)
{
    if (TIM_GetITStatus(RXTIM, TIM_IT_CC1) != RESET) {
        TIM_ClearITPendingBit(RXTIM, TIM_IT_CC1);

#if (SYS_RECEIVE == ENABLED)
        GPIO_WriteBit(GPIO_Port_CON2_40, GPIO_Pin_CON2_40, Bit_RESET);    // tmp debug
        if (spi2_state != SPI2_STATE__NONE) {
            debug_printf("TIM10\r\n");
            for (;;)
                asm("nop"); // state machine lockup
        }
        rm2_rxtimeout_isr();
#endif /* SYS_RECEIVE == ENABLED */
    }

    if (TIM_GetITStatus(RXTIM, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(RXTIM, TIM_IT_Update);
        // @todo: find out who enabled the update interrupt
        asm("nop");
    } else if (TIM_GetFlagStatus(RXTIM, TIM_FLAG_Update) != RESET) {
        TIM_ClearITPendingBit(RXTIM, TIM_IT_Update);
        //TIM_ClearFlag(RXTIM, TIM_FLAG_Update);
        asm("nop");
    }
    //lr = LR_TIM10;
}

