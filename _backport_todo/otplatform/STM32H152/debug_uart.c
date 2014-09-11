#ifdef RADIO_DEBUG
#include <otsys/types.h>
#include "debug_uart.h"
#include "stm32l1xx.h"
#include <app/build_config.h>
#include "platform_STM32L1xx.h"

#define UART_RX_BUF_SIZE        16
char uart_rx_buf[UART_RX_BUF_SIZE+1];    // +1 for null termination
ot_u8 uart_rx_buf_idx;
ot_bool new_uart_rx;

#ifndef BLOCKING_UART_TX

#define TX_DMA_FIFO_SIZE    7000
ot_u8 usart_tx_dmafifo[TX_DMA_FIFO_SIZE];
int usart_tx_dmafifo_in_idx;
int usart_tx_dmafifo_out_idx;
volatile char dma_running;

void
kick_dma_usart_tx(char context)
{
    dma_running = context;

    DMA_ITConfig(USART3_TX_DMA_CHANNEL, DMA_IT_TC, DISABLE);
    DMA_DeInit(USART3_TX_DMA_CHANNEL);
    if (usart_tx_dmafifo_in_idx < usart_tx_dmafifo_out_idx) {
        // wrap-around condition, will need to be sent in two parts, first do unsent up to end of buffer
        UTX_DMA_Init.DMA_BufferSize = (uint16_t)(TX_DMA_FIFO_SIZE - usart_tx_dmafifo_out_idx);
    } else {
        UTX_DMA_Init.DMA_BufferSize = (uint16_t)(usart_tx_dmafifo_in_idx - usart_tx_dmafifo_out_idx);
    }
    UTX_DMA_Init.DMA_MemoryBaseAddr = (uint32_t)&usart_tx_dmafifo[usart_tx_dmafifo_out_idx];
    if (UTX_DMA_Init.DMA_BufferSize > 1) {
        DMA_Init(USART3_TX_DMA_CHANNEL, &UTX_DMA_Init);

        DMA_ClearFlag(DMA1_FLAG_TC2);
        DMA_Cmd(USART3_TX_DMA_CHANNEL, ENABLE);
        DMA_ITConfig(USART3_TX_DMA_CHANNEL, DMA_IT_TC, ENABLE);
    } else {
        USART_SendData(USART3, usart_tx_dmafifo[usart_tx_dmafifo_out_idx]);
        USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
    }

}

void
DMA1_Channel2_IRQHandler()
{
    if (DMA_GetITStatus(DMA1_IT_TC2) == SET) {

        if (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == SET) {
            if (usart_tx_dmafifo_out_idx == usart_tx_dmafifo_in_idx) {
                dma_running = 0;    // no more to send, shut down
                DMA_ITConfig(USART3_TX_DMA_CHANNEL, DMA_IT_TC, DISABLE);
                DMA_Cmd(USART3_TX_DMA_CHANNEL, DISABLE);
            } else {
                if (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {
                    debug_printf("DMACH2\r\n");
                    for (;;)
                        asm("nop"); // why am i here?
                }
                kick_dma_usart_tx(4);
            }
        } else {
            USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
            dma_running = 3;    // indicate DMA TC occurred (but not yet USART TC)
        }

        DMA_ClearITPendingBit(DMA1_IT_GL2);
        //redundant DMA_ClearFlag(DMA1_FLAG_TC2);
        DMA_ClearFlag(USART3_TX_DMA_FLAG_GL);   // in example


    } // ...if (DMA_GetITStatus(DMA1_IT_TC2) == SET)
    else {
        asm("nop"); // why am i here?
    }
}

#endif /* !BLOCKING_UART_TX */


void USART3_IRQHandler(void)
{
    ot_u8 rxchar;

    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
        /* Read one byte from the receive data register */
        rxchar = USART_ReceiveData(USART3);
        if (rxchar == '\r') {
            uart_rx_buf[uart_rx_buf_idx++] = 0;    // null terminate
            new_uart_rx = 1;
        } else if (rxchar == 8) {
            if (uart_rx_buf_idx > 0) {
                __io_putchar(8);
                __io_putchar(' ');
                __io_putchar(8);
                uart_rx_buf_idx--;
#ifndef BLOCKING_UART_TX
                if (dma_running == 0)
                    kick_dma_usart_tx(2);
#endif
            }
        } else if (uart_rx_buf_idx < UART_RX_BUF_SIZE) {
                uart_rx_buf[uart_rx_buf_idx++] = rxchar;
                __io_putchar(rxchar);
#ifndef BLOCKING_UART_TX
                if ( (dma_running == 0) && (rxchar != '\n') )
                    kick_dma_usart_tx(2);
#endif
        }
    }

    if (USART_GetITStatus(USART3, USART_IT_TXE) != RESET) {
        USART_ClearFlag(USART3, USART_FLAG_TXE);
        USART_ITConfig(USART3, USART_IT_TXE, DISABLE);

#ifndef BLOCKING_UART_TX
        usart_tx_dmafifo_out_idx += UTX_DMA_Init.DMA_BufferSize;
        if (usart_tx_dmafifo_out_idx == TX_DMA_FIFO_SIZE)
            usart_tx_dmafifo_out_idx = 0;   // this was a "wrap"

        if (usart_tx_dmafifo_out_idx == usart_tx_dmafifo_in_idx) {
            // no more to send, shut down
            dma_running = 0;
            DMA_ITConfig(USART3_TX_DMA_CHANNEL, DMA_IT_TC, DISABLE);
            DMA_Cmd(USART3_TX_DMA_CHANNEL, DISABLE);
        } else {
            kick_dma_usart_tx(2);
        }
#endif
    } // ...if (USART_GetITStatus(USART3, USART_IT_TC) != RESET)

    //lr = LR_U3;
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
/*    if (usart3_initialized == 0)
        return ch;*/

#ifdef BLOCKING_UART_TX
    USART_SendData(USART3, (uint8_t) ch);

    /* Loop until transmit data register is empty */
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
    {}
#else

    usart_tx_dmafifo[usart_tx_dmafifo_in_idx] = ch;
    //total_bytes_tx++;
    if (++usart_tx_dmafifo_in_idx == TX_DMA_FIFO_SIZE)
        usart_tx_dmafifo_in_idx = 0;

/* will be dropping chars:   if (usart_tx_dmafifo_in_idx == usart_tx_dmafifo_out_idx ) { */
    if (usart_tx_dmafifo_in_idx == usart_tx_dmafifo_out_idx ) {
        for (;;)
            asm("nop"); // full
    }

    /* using newline as delimiter, start DMA transmitting if its not running already */
    if ( (dma_running == 0) && (ch == '\n') ) {
        kick_dma_usart_tx(1);

/*        while (dma_running != 0)    // to make each line a blocking call
            asm("nop");*/

    } // ...dma not running and newline just "sent"

#endif
    return ch;
}

/* from ascii hex */
void
from_hex(char *str, unsigned char *out)
{
    unsigned char nhi, nlo;

    nhi = str[0];
    nlo = str[1];

    if (nhi > 'Z')
        nhi -= 0x20;    // to upper case
    if (nlo > 'Z')
        nlo -= 0x20;    // to upper case

    if (nhi > '9')
        nhi -= 55;    // 'A' to 10
    else
        nhi -= '0';    // '9' to 9

    if (nlo > '9')
        nlo -= 55;    // 'A' to 10
    else
        nlo -= '0';    // '9' to 9

    *out = (nhi << 4) + nlo;
}

#ifdef PLATFORM_CONSOLE
void    /* generic minimal default */
console_service()
{
    asm("nop");

    if (new_uart_rx == 0)
        return;

    spi_save_restore(True);

    radio_console_service();

    debug_printf("\r\n> ");

    new_uart_rx = 0;
    uart_rx_buf_idx = 0;

    spi_save_restore(False);
}
#else
    // defined in app
#endif

void
debug_uart_init()
{
#ifndef BLOCKING_UART_TX
    usart_tx_dmafifo_in_idx = 0;
    usart_tx_dmafifo_out_idx = 0;
    dma_running = 0;
#endif

    uart_rx_buf_idx = 0;
    new_uart_rx = 0;
}

#endif /* RADIO_DEBUG */


