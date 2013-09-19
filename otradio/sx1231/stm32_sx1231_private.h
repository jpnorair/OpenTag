
#define PREAMBLE_BYTE   0x55

#define SPI2_DR_ADDRESS 0x4000380c
#define SPI2TX_DMA_CHANNEL            DMA1_Channel5
#define SPI2TX_DMA_FLAG_TC            DMA1_FLAG_TC5
#define SPI2TX_DMA_FLAG_GL            DMA1_FLAG_GL5
#define SPI2TX_DMA_IT_TC              DMA1_IT_TC5

#define SPI2RX_DMA_CHANNEL            DMA1_Channel4
#define SPI2RX_DMA_FLAG_TC            DMA1_FLAG_TC4
#define SPI2RX_DMA_FLAG_GL            DMA1_FLAG_GL4
#define SPI2RX_DMA_IT_TC              DMA1_IT_TC4


typedef enum {
    SPI2_STATE__NONE = 0,
    SPI2_STATE__START_TX_DMA,   // 1
#if (SYS_RECEIVE == ENABLED)
    SPI2_STATE__START_RX_DMA,   // 2
    SPI2_STATE__RX_DMA_NEXT,    // 3
    SPI2_STATE__RX_RSSI,        // 4
    SPI2_STATE__RX_RSSI_TAIL,   // 5
#endif /* SYS_RECEIVE == ENABLED */
} spi2_state_e;

extern spi2_state_e spi2_state;

typedef struct {
    ot_u8   state;
    ot_u8   flags;
#ifdef RADIO_DEBUG
    ot_u8    unlock_count; 
#endif /* RADIO_DEBUG */
    ot_u8    rssi_count;
    ot_u16    rssi_sum;
    ot_sig2 evtdone;
} radio_struct;

extern radio_struct radio;

extern ot_u8 rf_data_buf[]; // from radio_SX1231.c
extern int num_bytes_sent; // from radio_SX1231.c
extern int num_bytes_to_send; // from radio_SX1231.c

extern DMA_InitTypeDef SPI2_DMA_Init;   // from spi_sx1231.c

extern EXTI_InitTypeDef exti4_init;
extern EXTI_InitTypeDef exti9_5_init;

void WriteReg_Sx1231(ot_u8 addr, ot_u8 val);
ot_u8 ReadReg_Sx1231(ot_u8 addr);
void WriteReg_Sx1231__nonblocking(ot_u8 addr, ot_u8 val);


extern volatile ot_u8 start_tx_from;    // diag

extern RegFifoThresh_t RegFifoThresh;

#if (SYS_RECEIVE == ENABLED)
void rx_done_isr(ot_int pcode);
void rm2_rxtimeout_isr(void);
#endif /* SYS_RECEIVE == ENABLED */

extern Twobytes sync_value; // global for access from transmitter
