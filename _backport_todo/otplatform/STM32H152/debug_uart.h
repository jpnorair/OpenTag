#ifdef RADIO_DEBUG

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
      set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE;

extern ot_bool new_uart_rx;
extern char uart_rx_buf[];
extern ot_u8 uart_rx_buf_idx;

void from_hex(char *str, unsigned char *out);
void debug_uart_init(void);
void console_service(void);

#ifndef BLOCKING_UART_TX
void kick_dma_usart_tx(char context);
#endif


#endif /* RADIO_DEBUG */
