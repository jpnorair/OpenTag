#include "OT_types.h"

int open_pipe_for_rx(int timeout, ot_u8 sid);

int open_pipe_for_tx(unsigned char *txdata, int data_len, ot_u8 sid);

#define RX_BUF_SIZE 64
extern unsigned char rx_buf[RX_BUF_SIZE];
extern int rx_buf_in_idx;
extern int rx_buf_out_idx;

void radio_pipe_read(void);

void rx_done_isr(ot_int pcode); // from radio_SIM.c

void radio_pipe_close(void);
