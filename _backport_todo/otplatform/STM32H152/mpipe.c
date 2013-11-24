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
  * @file       /Platforms/STM32H152/mpipe.c
  * @author     
  * @version    V1.0
  * @date       1 Dec 2011
  * @brief      Message Pipe (MPIPE) UART implementation for STM32F10x
  * @defgroup   MPipe (Message Pipe)
  * @ingroup    MPipe
  *
  * Baudrates supported:    9600, 115200                                        <BR>
  * Byte structure:         8N1                                                 <BR>
  * Duplex:                 Half                                                <BR>
  * Flow control:           Custom, ACK-based                                   <BR>
  * Connection:             RS-232, DTE-DTE (use a null-modem connector)        <BR><BR>
  * 
  * Implemented Mpipe Protocol:                                                 <BR>
  * The Mpipe protocol is a simple wrapper to NDEF.                             <BR>
  * Legend: [ NDEF Header ] [ NDEF Payload ] [ Seq. Number ] [ CRC16 ]          <BR>
  * Bytes:        6             <= 255             2             2              <BR><BR>
  *
  * The protocol includes an ACK/NACK feature.  After receiving a message, the  <BR>
  * Mpipe send an ACK/NACK.  The "YY" byte is 0 for ACK and non-zero for ACK.   <BR>
  * Presently, 0x7F is used as the YY NACK value.                               <BR>
  * [ Seq ID ] 0xDD 0x00 0x00 0x02 0x00 0xYY  [ CRC16 ]
  ******************************************************************************
  */

#define __wbr__    1
#include "OT_config.h"
#include "mpipe.h"
#include "OT_platform.h"

/*#if (OT_FEATURE(MPIPE) == ENABLED)
#error mpipe_enabled
#else
#error mpipe_disabled
#endif
#if (MCU_FEATURE(MPIPEVCOM) == ENABLED)
#error mpipe_vcom_enabled
#else
#error mpipe_vcom_disabled
#endif*/

// Compile only when MPipe is enabled, but USB is disabled
#if ((OT_FEATURE(MPIPE) == ENABLED) && (MCU_FEATURE(MPIPEVCOM) != ENABLED))

//#include "OT_utils.h"
#include "crc16.h"

/** Mpipe Module Data
  * At present this consumes 24 bytes of SRAM.  6 bytes could be freed by
  * removing the callbacks, which might not be used.
  */
typedef struct {
    mpipe_state     state;
    mpipe_priority  priority;
    Twobytes        sequence;
    ot_u8*          pktbuf;
    ot_int          pktlen;
    ot_u8           ackbuf[10];
    
#   if (OT_FEATURE(MPIPE_CALLBACKS) == ENABLED)
        void (*sig_rxdone)(ot_int);
        void (*sig_txdone)(ot_int);
        void (*sig_rxdetect)(ot_int);
#   endif
} mpipe_struct;

mpipe_struct mpipe;

volatile ot_bool dma_running = False;

static ot_u8 mpipe_tx_buf[256];

static void
mpipe_txdone()
{
    mpipe.sequence.ushort++;    //increment sequence on TX Done
    mpipe.priority  = MPIPE_Low;
    mpipe.state     = MPIPE_Idle;
#   if (OT_FEATURE(MPIPE_CALLBACKS) == ENABLED)
        mpipe.sig_txdone(0);
#   endif
}

#ifndef RADIO_DEBUG
/* using dma, void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_TXE) != RESET) {   
        USART_ClearFlag(USART3, USART_FLAG_TXE);
        dma_running = False;
        mpipe_txdone();
    }
}*/

void
DMA1_Channel2_IRQHandler()
{
    if (DMA_GetITStatus(DMA1_IT_TC2) == SET) {

        /*if (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == SET) {
        }*/

        DMA_ClearITPendingBit(DMA1_IT_GL2);
        DMA_ClearFlag(USART3_TX_DMA_FLAG_GL);

        dma_running = False;
        mpipe_txdone();

    } // ...if (DMA_GetITStatus(DMA1_IT_TC2) == SET)
}
#endif /* RADIO_DEBUG */

volatile ot_u8 data_two;

/* returns the count of bytes still to be transmitted: blocking returns zero */
ot_int mpipe_txndef(ot_u8* data, ot_bool blocking, mpipe_priority data_priority) {
#ifndef RADIO_DEBUG
    Twobytes crcval;
    ot_int data_length = data[2];
    int i;

    data_length += 6;
    data_two = data[2];

    if (data_priority != MPIPE_Ack) {
        if (mpipe.state != MPIPE_Idle) {
            return -1;
        }
        mpipe.priority  = data_priority;
        mpipe.pktbuf    = data;
        mpipe.pktlen    = data_length;
    }

    data[data_length++] = mpipe.sequence.ubyte[UPPER];
    data[data_length++] = mpipe.sequence.ubyte[LOWER];
    // no hardware crc16 on this cpu
    crcval.ushort       = crc_calc_block(data_length, data);
    data[data_length++] = crcval.ubyte[UPPER];
    data[data_length++] = crcval.ubyte[LOWER];

    
    mpipe.state = MPIPE_Tx_Wait;

    if (dma_running) {
        for (;;)
            asm("nop"); // starting to send when was already sending
    }

    DMA_DeInit(USART3_TX_DMA_CHANNEL);
    UTX_DMA_Init.DMA_BufferSize = (uint16_t)data_length;
    //UTX_DMA_Init.DMA_MemoryBaseAddr = (uint32_t)&data[0];
    // ? is data on the stack or does it persist after return ? //
    if (data_length >= sizeof(mpipe_tx_buf)) {
        for (;;)
            asm("nop");
    }
    for (i = 0; i < data_length; i++)
        mpipe_tx_buf[i] = data[i];
    UTX_DMA_Init.DMA_MemoryBaseAddr = (uint32_t)&mpipe_tx_buf[0];
    /*UTX_DMA_Init.DMA_BufferSize = (uint16_t)(sizeof(const_data)-1);
    UTX_DMA_Init.DMA_MemoryBaseAddr = (uint32_t)&const_data[0];*/
    DMA_Init(USART3_TX_DMA_CHANNEL, &UTX_DMA_Init);

    DMA_ClearFlag(DMA1_FLAG_TC2);
    DMA_Cmd(USART3_TX_DMA_CHANNEL, ENABLE);

    if (blocking == True) {
        while (DMA_GetITStatus(DMA1_IT_TC2) == RESET)
            asm("nop");
        mpipe_txdone();
        // always return 0 on success
        return 0;
    } else {
        DMA_ITConfig(USART3_TX_DMA_CHANNEL, DMA_IT_TC, ENABLE);
        // return number of bytes being transmitted
        dma_running = 1;
        return data_length;
    }
#endif /* !RADIO_DEBUG */
}

ot_int mpipe_init(void* port_id) {
    // usart already initialized
/// 0. "port_id" is unused in this impl, and it may be NULL
/// 1. Set all signal callbacks to NULL, and initialize other variables.

#   if (OT_FEATURE(MPIPE_CALLBACKS) == ENABLED)
        mpipe.sig_rxdone    = &sub_signull;
        mpipe.sig_txdone    = &sub_signull;
        mpipe.sig_rxdetect  = &sub_signull;
#   endif

    //mpipe.sequence.ushort   = 0;          //not actually necessary
    mpipe.state             = MPIPE_Idle;
    mpipe.priority          = MPIPE_Low;

    return 0;
}

#endif /* ((OT_FEATURE(MPIPE) == ENABLED) && (MCU_FEATURE(MPIPEVCOM) != ENABLED))  */
