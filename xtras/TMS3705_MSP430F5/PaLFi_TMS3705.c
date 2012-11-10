/*  Copyright (c) 2012, JP Norair
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions are met:
  *    * Redistributions of source code must retain the above copyright
  *      notice, this list of conditions and the following disclaimer.
  *    * Redistributions in binary form must reproduce the above copyright
  *      notice, this list of conditions and the following disclaimer in the
  *      documentation and/or other materials provided with the distribution.
  *    * Neither "OpenTag" nor the names of its contributors may be used to 
  *      endorse or promote products derived from this software without
  *      specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  * ARE DISCLAIMED. IN NO EVENT SHALL JP NORAIR BE LIABLE FOR ANY DIRECT,
  * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  */
/**
  * @file       /Apps/Demo_PaLFi/Code_Master/PaLFi_TMS3705.c
  * @author     JP Norair
  * @version    V1.0
  * @date       5 March 2012
  * @brief      Driver for TMS3705 PaLFi Master chip
  * @ingroup    Demo_PaLFi
  *
  ******************************************************************************
  */




#include "palfi.h"
#include "OT_platform.h"



/** Board configuration settings    <BR>
  * ========================================================================<BR>
  * These might go into the board configuration, maybe not.
  */
#define _SMCLK_HZ       (PLATFORM_HSCLOCK_HZ/PLATFORM_SMCLK_DIV)
#define _BRW            (ot_u16)(_SMCLK_HZ/15625)   

#define _TXCT_PORT      GPIO1       //change
#define _TXCT_PIN       GPIO_Pin_2  //change
#define _SCIO_PORT      GPIO1       //change
#define _SCIO_PIN       GPIO_Pin_2  //change





/** Palfi command bytes    <BR>
  * ========================================================================<BR>
  * Command bytes are part of the API messages that come from the client
  * console.  The 2nd command byte is not actually used.
  */
// command byte 1 bitfields
#define PALFICMD_NORMAL         (1<<0)  // 0: Normal Mode       1: Setup Mode       //UNUSED
#define PALFICMD_PPM            (1<<1)  // 0: PWM               1: PPM/BLC
#define PALFICMD_LFRESP         (1<<2)  // 0: UHF Response      1: LF Response      //UNUSED
#define PALFICMD_X26X22         (1<<3)  // 0: No WP on 37122    1: No WP on 37126   //UNUSED
#define PALFICMD_SELECT0        (1<<4)  // 0/1: Select Mode low bit
#define PALFICMD_SELECT1        (1<<5)  // 0/1: Select Mode high bit
#define PALFICMD_2BPB           (1<<6)  // 0: 1-byte powerburst 1: 2-byte powerburst
#define PALFICMD_EXT            (1<<7)  // 0: No CMD 2          1: CMD 2 follows

// command byte 2 bitfields
#define PALFICMD_FDX            (1<<0)  // 0: HDX-FSK           1: FDX-ASK          //UNUSED
#define PALFICMD_BIPHASE        (1<<1)  // 0: Manchester ASK    1: Biphase ASK      //UNUSED
#define PALFICMD_ASK4K          (1<<2)  // 0: 2kbps ASK         1: 4kbps ASK        //UNUSED
#define PALFICMD_ANTICOLLISION  (1<<3)  // 0: Off               1: On               //UNUSED
#define PALFICMD_KEEPTXON       (1<<4)  // 0: TX Off after PB2  1: TX On after PB2  //UNUSED
#define PALFICMD_BIT5           (1<<5)  // RFU 
#define PALFICMD_BIT6           (1<<6)  // RFU
//#define PALFICMD_EXT                  // Same as in command byte 1






/** PalFi States     <BR>
  * ========================================================================<BR>
  * The PaLFi driver uses a software-implemented PHY for TX, so there are many
  * TX driver states.  The TX driver is interrupt-based and it uses the Timer
  * Output Capture hardware to generate the TXCT waveform.
  *
  * For RX, the TMS3705 does the PHY decoding, and it has a one-byte FIFO.  The
  * UART is used to do most of this work in hardware.
  */
#define PALFISTATE_RX           0x80

#define PALFISTATE_TX           0
#define PALFISTATE_HEADER       0
#define PALFISTATE_POSTHEADER   2
#define PALFISTATE_PPMPB0       4
#define PALFISTATE_EXTRA        6
#define PALFISTATE_NODE1        8
#define PALFISTATE_NODE2        10
#define PALFISTATE_DATA         12
#define PALFISTATE_PB1          14
#define PALFISTATE_WAIT         16
#define PALFISTATE_END          18
#define PALFISTATE_NODE0        20







/** Pulse Definitions (TX PHY)      <BR>
  * ========================================================================<BR>
  * The TX PHY is driven bit-by-bit.  Each bit is period, with a rising and 
  * falling edge somewhere in the pulse pulse period.  So, each bit must 
  * contain two numbers
  */
#define PULSE_US            2
#define PULSE_UNITS(VAL)    (VAL/PULSE_US)
  
static const ot_u16 symbol_table[] = {
    PULSE_UNITS(170),       //PWMtimes.usToffL
    PULSE_UNITS(330),       //PWMtimes.usTonL
    PULSE_UNITS(480),       //PWMtimes.usToffH
    PULSE_UNITS(520),       //PWMtimes.usTonH
    PULSE_UNITS(170),       //BLCtimes.usToff
    PULSE_UNITS(230),       //BLCtimes.usTonL
    PULSE_UNITS(170),       //BLCtimes.usToff
    PULSE_UNITS(350)        //BLCtimes.usTonH
    //PULSE_UNITS(600)      //BLCtimes.usTonSC
}

const ot_u16 seq_charge[] = {
    PULSE_UNITS(2000)  //HIGH
    PULSE_UNITS(50),
    PULSE_UNITS(110000),
    PULSE_UNITS(50),
    PULSE_UNITS(3000),
    PULSE_UNITS(0)
};

const ot_u16 seq_wake[] = {
    PULSE_UNITS(50),    //Low
    PULSE_UNITS(3000)
};

static const ot_u16 seq_initmc[] = { \
    PULSE_UNITS(100),      //high
    PULSE_UNITS(350)        //low
};





/** PaLFi driver data elements      <BR>
  * ========================================================================
  */
typedef enum timpol_enum {
    POL_hilo    = (OUTMOD_7 + OUT),
    POL_lohi    = (OUTMOD_3)
}
  
typedef struct {
    ot_u16  powerburst[4];
    ot_u8*  extra;
    ot_u8*  tx;
    ot_u8   cmd1;
    ot_u8   cmd2;
    ot_u8   extra_bytes;
    ot_u8   tx_bytes;
    ot_u8   rx_bytes;
} pktformat_struct;

typedef struct {
    pktformat_struct    pkt;
    ot_u8               periods;
    ot_u8               state;
    ot_u16*             symbol;
    ot_u8*              data;
    Queue*              in_q;
    Queue*              out_q;
} palfi_struct;

palfi_struct palfi;








/** Platform-dependent subroutines      <BR>
  * ========================================================================
  */
void sub_timoc_lohi();
void sub_timoc_lohi();
void sub_timoc_start();
void sub_turnon_amp();
void sub_turnoff_amp();
void sub_turnon_txctint();
void sub_turnoff_txctint();
void sub_turnon_rxuart();
void sub_turnoff_rxuart();
ot_u8 sub_get_rxbyte();
void sub_put_txbyte(ot_u8 byte);
void sub_turnoff_rxtimeout();
void sub_turnon_rxtimeout();
void sub_uart_rxmode();
void sub_uart_txmode();
void sub_uart_txload(ot_u8 byte);
void sub_uart_wait();
void sub_map_ports(ot_u16 txct_map);






/** PaLFi Interrupts        <BR>
  * ========================================================================<BR>
  * These interrupts are platform-dependent, and the ones here are defined for
  * MSP430F5 chips.  They may also work for some other MSP430s.
  */

#pragma vector=PALFI_TIMER_VECTOR
OT_INTERRUPT void palfi_tim_isr(void) {
/// This interrupt is the normal CCR1-7 & Update IRQ, not the CCR0 IRQ.
/// The TXCT process uses a symbol-driving interrupt based on Output Compare.
/// The RX process uses a timeout based on normal upcounting

    // RX: implicit update interrupt
    if (palfi.state & PALFISTATE_RX) {
        PALFI_TIM->CTL &= ~0x03;    // Kill timeout & clear flag
        palfi.rx_callback(-5);
    }
    
    // TX: implicit CCR1 interrupt
    else if (palfi.periods == 0)    palfi_tx_isr();     //palfi.data++;
    else if (palfi.data == NULL)    sub_load_txirreg();
    else                            sub_load_txbit();

    LPM4_EXIT;
}



#pragma vector=PALFI_UART_VECTOR
OT_INTERRUPT void palfi_uart_isr(void) {
/// TX: Not currently used, could be used for multibyte mode control
/// RX: used for read-out of data coming back from the TMS3705

    switch (PALFI_UART->IV) {
        //RX interrupt
        case 2: palfi_rx_isr();
                break;
        
        //TX interrupt: Not currently used
        //case 4: 
        //        break;
    }
    
    LPM4_EXIT;
}






/** Platform Dependent Subroutines        <BR>
  * ========================================================================<BR>
  * These subroutines are platform-dependent, and the ones here are defined for
  * MSP430F5 chips.  They may also work for some other MSP430s.  To port this
  * driver to another MCU, you will need to change these subroutines.  If the
  * target MCU has a similar timer functionality as the MSP430F5, then the port
  * will be quite transparent (many popular MCUs qualify).
  */

void sub_load_txbit() {
    ot_u16* symbol  = palfi.symbol;                     // Get pulse sequence
    (ot_u8*)symbol += (*palfi.data & 1) << 1;           // Advance to Logic 1 pulse if needed
    *palfi.data   >>= 1;
    PALFI_TIM->CCR1 = symbol[0];                        // load first half of pulse
    PALFI_TIM->CCR0 = symbol[0] + symbol[1];            // load second half of pulse
    palfi.periods--;
}

void sub_load_txirreg() {
    PALFI_TIM->CCR1 = palfi.symbol[0];                      // load first half of pulse
    PALFI_TIM->CCR0 = palfi.symbol[0] + palfi.symbol[1];    // load second half of pulse
    palfi.symbol   += 2;                                    // increment irregular symbol sequence
    palfi.periods--;
}

void sub_timoc_hilo() {
    PALFI_TIM->CCTL1   |= OUTMOD_7; 
}

void sub_timoc_lohi() {
    PALFI_TIM->CCTL1   &= ~OUTMOD_4;
}

void sub_timoc_stop() {
    PALFI_TIM->CTL     &= ~(TBCLMC_3);
}

void sub_timoc_start(timpol_enum polarity) {
    PALFI_TIM->CTL     &= ~MC_3;                        //clear timer
    PALFI_TIM->CCTL0    = 0;                            //latch CCR0 immediately
    PALFI_TIM->CCTL1    = (ot_u16)polarity | CCIE;      //latch CCR1 immediately
    
    if (palfi.data == NULL) sub_load_txirreg();
    else                    sub_load_txbit();
    
    PALFI_TIM->CTL     |= MC_1;                         //start timer
    PALFI_TIM->CCTL0   |= CLLD_2;                       //change update mode
    PALFI_TIM->CCTL1   |= CCLD_2;                       //change update mode
}

void sub_turnon_amp() {
/// This feature is not available on the stock Dev Kit, which has the amplifier
/// always on and no simple interface to turn on or off.
//    PALFI_PORT->DOUT       &= ~PALFI_TXCT_PIN;          // leave on RF Field
//    PALFI_AMP_PORT->DOUT   |= PALFI_AMP_PIN;            // enable UCC Amplifier
}

void sub_turnoff_amp() {
/// This feature is not available on the stock Dev Kit, which has the amplifier
/// always on and no simple interface to turn on or off.
//    PALFI_PORT->DOUT       |= PALFI_TXCT_PIN;           // turn off RF Field
//    PALFI_AMP_PORT->DOUT   &= ~PALFI_AMP_PIN;           // disable UCC Amplifier
}

void sub_turnon_txctint() {
    PALFI_TIM->CCTL1 |= CCIE;
}

void sub_turnoff_txctint() {
    PALFI_TIM->CCTL1 &= ~CCIE;
}

void sub_turnon_rxuart() {
    PALFI_UART->IFG = 0;
    PALFI_UART->IE |= UCRXIE;
}

void sub_turnoff_rxuart() {
    PALFI_UART->IE &= ~UCRXIE;
}

ot_u8 sub_get_rxbyte() {
    return PALFI_UART->RXBUF;
}

void sub_put_txbyte(ot_u8 byte) {
    PALFI_UART->TXBUF = byte;
}

void sub_turnoff_rxtimeout() {
    PALFI_TIM->IE &= ~0x02;     //update interrupt
}

void sub_turnon_rxtimeout() {
    PALFI_TIM->IE |= 0x02;      //update interrupt
}
 
void sub_uart_rxmode() {
/// For RX, use 15625 bps and 8 bits
    PALFI_UART->CTL0    = 0;                //Basic config, starts disabled
    PALFI_UART->BR0     = (ot_u8)_BRW;
    PALFI_UART->BR1     = (ot_u8)(_BRW>>8);
}

void sub_uart_txmode() {
/// For TX mode control writes, change to half-rate (7812.5 bps) and 7 bits
    PALFI_UART->CTL0    = UC7BIT;           //7Bit Byte config, starts disabled
    PALFI_UART->BR0     = (ot_u8)(_BRW/2);
    PALFI_UART->BR1     = (ot_u8)((_BRW/2)>>8);
}

void sub_uart_txload(ot_u8 byte) {
/// load a byte into the TX hardware buffer
    PALFI_UART->TXBUF = byte;
}

void sub_uart_wait() {
/// Block the CPU until the UART is completely paged-out (or paged-in)
    while (PALFI_UART->STAT & UCBUSY); 
}

void sub_map_ports(ot_u16 txct_map) {
/// Usage:                              <BR>
/// sub_map_ports(PALFI_TIMOC_MAP);     <BR>
/// sub_map_ports(PALFI_TXUART_MAP);    <BR>
    PM->KEYID           = 0x2D52;
    PALFI_Px->SCIO_MAPy = PALFI_UARTRX_MAP;     //PM_UCA1RXD
    PALFI_Px->TXCT_MAPy = txct_map;             //PM_TB0CCR1A
    PM->KEYID           = 0;
}

void sub_init_ports() {
    ///1. Map SCIO to UART RX.  Map TXCT to a Timer OC channel.
    sub_map_ports(PALFI_TIMOC_MAP);

    ///2. Setup SCIO for input, TXCT for output
    PALFI_PORT->SEL    |= PALFI_PINS;       //Set pins to alternate function
    PALFI_PORT->IE     &= ~PALFI_PINS;      //Disable pin interrupts (if any)
    PALFI_PORT->IFG    &= ~PALFI_PINS;      //Clear any interrupt flags
    PALFI_PORT->DIR    &= ~PALFI_PINS;      //Clear direction (set input)
    PALFI_PORT->DIR    |= PALFI_TCXT_PIN;   //Set TXCT to output
    PALFI_PORT->DS     |= PALFI_TCXT_PIN;   //Set drive strength to high on TXCT
    
    ///3. Configure UART
    PALFI_UART->CTL1    = UCSSEL_2;         //Always use SMCLK
    
    ///4. Write Default Mode Control Byte (auto, asynchronous-read).
    ///   The RX UART mode and TXCT output mode are always entered after write.
    palfi_write_mctrl(b00011110);
    
    
    ///4. The maximum pulse time in the implementation in 110ms.  The 16 bit 
    ///   Timer will be configured to use 2us units, allowing pulses between
    ///   2us and 131.072ms.  Therefore, SMCLK must be a multiple of 500kHz and
    ///   be prescale-able by the MSP430 timer.  Acceptable values are: 0.5,
    ///   1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 5.0, 6.0, 7.0, 8.0, 10.0, 12.0, 
    ///   14.0, 16.0, 20.0, 24.0 MHz
#   define _SMCLK_INT   (_SMCLK_HZ/500000)
#   if ((_SMCLK_HZ % 500000) != 0)
#       warn "SMCLK is not a multiple of 500kHz.  PaLFi timing might not be accurate"
#   endif
#   if ((_SMCLK_INT!=1) || (_SMCLK_INT!=2) || (_SMCLK_INT!=3) || (_SMCLK_INT!=4) || \
        (_SMCLK_INT!=5) || (_SMCLK_INT!=6) || (_SMCLK_INT!=7) || (_SMCLK_INT!=8) || \
        (_SMCLK_INT!=10) || (_SMCLK_INT!=12) || (_SMCLK_INT!=14) || (_SMCLK_INT!=16) || \
        (_SMCLK_INT!=20) || (_SMCLK_INT!=24) || (_SMCLK_INT!=28) || (_SMCLK_INT!=32) || \
        (_SMCLK_INT!=40) || (_SMCLK_INT!=48))
#       error "SMCLK is not set to a value that can be prescaled.  Refer to PaLIi_TMS3705.c"
#   endif
#   if (_SMCLK_HZ < 4000000)
#       define _CLKDIV  1
#       define _ID      0
#   elif (_SMCLK_HZ < 8000000)
#       define _CLKDIV  2
#       define _ID      1
#   elif (_SMCLK_HZ < 16000000)
#       define _CLKDIV  4
#       define _ID      2
#   else 
#       define _CLKDIV  8
#       define _ID      3
#   endif 
    PALFI_TIM->CTL      = TASSEL_2 | (_ID<<6) | TAIE;
    //PALFI_TIM->CCTL0    = 0;                        //CCR0 is unused
    //PALFI_TIM->CCTL1    = OUTMOD_4 | CCIE | OUT;    //Toggle compare, default high
    PALFI_TIM->EX0      = ((_SMCLK_HZ/_CLKDIV)/500000) & 0x07;
}








/** Platform Independent Subroutines        <BR>
  * ========================================================================<BR>
  * These subroutines do not call platform features directly, so they do not
  * need to be ported.
  */

void sub_set_dataseq() {
/// This routine runs during the TX ISR, which gets called on the second-half
/// of a pulse period.  Thus, the output modulation can be changed safely before
/// end of the pulse falls (or rises).
    palfi.symbol = &symbol_table[(palfi.pkt.cmd1 & PALFICMD_PPM) << 1];
    sub_timoc_hilo();
}


//ot_u16* sub_get_dataseq(ot_u8 bit) {
//    ot_u16* bitseq;
//    palfi.symbol.length = 2;
//    bitseq              = palfi.symbol.seq;
//    (ot_u8*)bitseq     += (bit) ? 4 : 0;    // 1 (+4) or 0 bit
//    
//    return bitseq;
//}


void sub_powerburst(ot_u8 number) {
/// This routine runs during the TX ISR, which gets called on the second-half
/// of a pulse period.  Thus, the output modulation can be changed safely before
/// end of the pulse falls (or rises).

    palfi.pburst    = &palfi.powerburst[number];
    palfi.data      = NULL;
    
    if ((palfi.pkt.cmd1 & PALFICMD_PPM) && (number == 0)) {
        palfi.state     = PALFISTATE_TX | PALFISTATE_PPMPB0;
        palfi.symbol    = &symbol_table[4];
        sub_timoc_hilo();
    }
    else {
        palfi.symbol    = palfi.pburst;
        sub_timoc_lohi();
    }
}


//Universal
void sub_get_powerburst(ot_u8 number) {
    palfi.powerburst[number] = q_readbyte(palfi.in_q);
    
    if (palfi.pkt.cmd1 & PALFICMD_2BPB) {
        palfi.powerburst[number]  <<= 4;
        palfi.powerburst[number]   += q_readbyte(palfi.in_q);
    }
}







/** Interface Functions        <BR>
  * ========================================================================<BR>
  * The interface functions are callable externally, and they are platform
  * independent.  Most likely, the only one that should actually be called is
  * palfi_proc(), which will break-down an input command message, run the 
  * command, and dump-out the response.  palfi_new() is similar, except that it
  * does not use the OpenTag ALP data wrapper (it uses the bare TI command).
  */
void palfi_tx_isr() {
    switch (palfi.state & PALFISTATE_TXMASK) {
        case PALFISTATE_HEADER:
            palfi.state++;
            //palfi_write_mctrl(ucMCW1);    ///@todo why would this be here?
            
        case PALFISTATE_POSTHEADER:
            palfi.state = PALFISTATE_TX + PALFISTATE_EXTRA;
            sub_turnon_amp();
            sub_powerburst(0); // Entering Charge phase : deliver Power Burst 0
            break;
        
        case PALFISTATE_PPMPB0:
            palfi.state++;
            palfi.symbol    = palfi.pburst;
            sub_lohi();
            break;
    
        case PALFISTATE_EXTRA: 
            if (palfi.pkt.cmd1 & PALFICMD_SELECT0) { //BBUP or PE_WP
                if (palfi.extra_bytes == 0) {
                    goto palfi_tx_isr_NODE0;
                }
                if (palfi.data == NULL) {
                    palfi.data = palfi.pkt.extra;
                    sub_set_dataseq();
                }
                palfi.periods = 8;
                palfi.pkt.extra_bytes--;
                break;
            }
        
        case PALFISTATE_NODE1:
            if ((palfi.pkt.cmd1 & b00111000) == b00101000) { // PE_noWP26 mode
                palfi.data      = NULL;
                palfi.periods   = 1;
                palfi.symbol    = seq_initmc;
                palfi.state     = PALFISTATE_TX+PALFISTATE_NODE2;
                break;
            }
        
        case PALFISTATE_NODE2:
            if ((palfi.pkt.cmd1 & b00110000) & (palfi.pkt.cmd1 & b00011000)) { //BBUP, PE_WP, or PE_noWP_26 modes
                palfi.state = PALFISTATE_TX+PALFISTATE_TXDATA;
                sub_powerburst(2);
                break;
            }
    
        case PALFISTATE_DATA:
            if (palfi.tx_bytes == 0) {
                goto palfi_tx_isr_PB1;
            }
            if (palfi.data == NULL) {
                palfi.data = palfi.pkt.tx;
                sub_set_dataseq();
            }
            palfi.periods = 8;
            palfi.pkt.tx_bytes--;
            break;
    
        case PALFISTATE_PB1:
        palfi_tx_isr_PB1:
            palfi.state = PALFISTATE_TX+PALFISTATE_WAIT;
            sub_powerburst(1);
            break;
            
        case PALFISTATE_WAIT:
            // wait for the final pulse to complete, and startup RX UART in the
            // meantime to be ready for the HDX response.
            palfi.state = PALFISTATE_TX+PALFISTATE_END;
            sub_timoc_wait();
            if (palfi.pkt.rx_bytes != 0) {
                palfi_read();
            }
            break;
    
        case PALFISTATE_END:
        palfi_tx_isr_END;
            sub_timoc_stop();   //disable interrupts for TXCT (tim CCR)
#           if (PALFI_AMP_PORT != NULL)
                if (palfi.pkt.cmd2 & PALFICMD_KEEPTXON)
                    sub_turnon_amp();
                else
                    sub_turnoff_amp();
#           endif
            break;
        
        case PALFISTATE_NODE0:
        palfi_tx_isr_NODE0:
            palfi.state = PALFISTATE_TX+PALFISTATE_NODE1;
            palfi.data  = NULL;
            
            if (palfi.pkt.cmd1 & PALFICMD_SELECT1) { //BBUP
                sub_powerburst(3);
            }
            else { //PE_WP
                palfi.periods   = 1;
                palfi.symbol    = &symbol_table[0];
                sub_hilo();
            }
            break;
            
       default: goto palfi_tx_isr_END;
                //error
    }
}





void palfi_rx_isr() {
    sub_turnoff_rxtimeout();
    q_writebyte(palfi.rxq, sub_get_rxbyte());
    palfi.pkt.rx_bytes--;
    
    if ((palfi.pkt.rx_bytes == 0) || (palfi.rxq->putcursor >= palfi.rxq->back)) {
        sub_turnoff_rxuart();
        palfi.rx_callback(0);
    }
    else {
        sub_turnon_rxtimeout();
    }
}






void palfi_write_mctrl(ot_u8 mode_byte) {
/// Write the mode byte.  This is a blocking routine.
    sub_map_ports(PALFI_UARTTX_MAP);
    sub_uartmode_tx();
    sub_uart_loadtx(mode_byte);
    sub_uart_wait();
    sub_map_ports(PALFI_TIMOC_MAP)
    sub_uartmode_rx();
}














//Universal
void palfi_new(Queue* in_q, Queue* out_q) {
/// Parses the somewhat-unusual TI PaLFi Message-API frame and loads control
/// data into the local structure.
    palfi.in_q      = in_q;
    palfi.out_q     = out_q;
    palfi.state     = 0;
    palfi.pkt.cmd2  = 0;
    
    /// Basic initial fields for all packets
    palfi.pkt.cmd1  = q_readbyte(in_q);
    if (palfi.pkt.cmd1 & PALFICMD_EXT) {
        palfi.pkt.cmd2  = q_readbyte(in_q);
    }
    sub_get_powerburst(0);
    
    /// Read extra data in Battery Backup (BBUP) or Wake Pattern (PE_WP) packets
    if (palfi.pkt.cmd1 & PALFICMD_SELECT0) {
        palfi.pkt.extra_bytes   = q_readbyte(palfi.in_q) >> 3;  //convert bits to bytes
        palfi.pkt.extra         = q_markbyte(palfi.in_q, palfi.pkt.extra_bytes);
        if (palfi.pkt.cmd1 & PALFICMD_SELECT1) {
            sub_get_powerburst(3);
        }
    }
    
    /// Packets that contain a Power Burst 2
    if ((palfi.pkt.cmd1 & b00110000) & (palfi.pkt.cmd1 & b00011000)) {
        sub_get_powerburst(2);
    }
    
    /// Basic remaining fields for all packets
    palfi.pkt.tx_bytes  = q_readbyte(palfi.in_q) >> 3;  //convert bits to bytes
    palfi.pkt.tx        = q_markbyte(palfi.in_q, palfi.pkt.tx_bytes);
    sub_get_powerburst(1);
    palfi.pkt.rx_bytes  = q_readbyte(palfi.in_q);
}




void palfi_write() {
/// Call to initialize a Data TX from the TMS3705, using a packet that has been
/// already loaded.  Note that According to TMS3705 datasheet, when TXCT pin is
/// low, the TMS3705 is not asleep and does not need to be awaken.  When TXCT 
/// pin is high, the chip is asleep and DOES need to get the wake-up sequence 
/// on the TXCT pin.
    timpol_enum polarity;
    palfi.state = PALFISTATE_TX+PALFISTATE_HEADER;
    palfi.data  = NULL;

    // Wakeup needed
    if (PALFI_PORT->DOUT & PALFI_TCXT_PIN) {
        palfi.periods   = 1;
        palfi.symbol    = seq_wake;
        polarity        = POL_lohi;
	}
	// Wakeup not needed
	else {	
        palfi.periods   = 3;
        palfi.symbol    = seq_charge;
        polarity        = POL_hilo;
    }
    
    sub_timoc_start(polarity);
}




ot_bool palfi_read() {
/// Call to initialize a readback from the one-byte buffer in the TMS3705.  In
/// this mode, the asynchronous UART is used to load data off the SCIO pin.
    ot_bool output = False;
    
    if ((palfi.rx_bytes != 0) && (palfi.rxq != NULL)) {
        sub_turnon_rxtimeout();
        sub_turnon_rxuart();
        output = True;
    }
    return output;
}





