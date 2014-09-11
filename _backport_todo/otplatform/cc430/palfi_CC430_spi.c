/* Copyright 2010-2012 JP Norair
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
  * @file       /otplatform/cc430/palfi_CC430.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 Oct 2012
  * @brief      PaLFI interface driver, using SPI, GPIO, and trimming timer
  * @defgroup   PaLFI
  * @ingroup    PaLFI
  *
  * @note This driver should be virtually identical to the one for MSP430F5
  * 
  ******************************************************************************
  */


#include <otplatform.h>

#if (defined(__CC430__))
#if (OT_FEATURE(PALFI))

#include <otlib/buffers.h>
#include "palfi.h"




/** Mpipe Module Data (used by all Mpipe implementations)   <BR>
  * ========================================================================<BR>
  */
  
typedef struct {
    ot_uni16        seq;
#if (MPIPE_USE_ACKS)
    mpipe_priority  priority;
#endif
} tty_struct;


tty_struct tty;





/** Mpipe Subs (Board & Platform dependent)  <BR>
  * ========================================================================
  */





/** Mpipe Low-Level ISRs (Platform-dependent)  <BR>
  * ========================================================================
  */

#if (ISR_EMBED(PALFI) == ENABLED)
#   undef ISR_EMBED_PALFI_GPIO
#   undef ISR_EMBED_PALFI_TIM
#   define ISR_EMBED_PALFI_GPIO ENABLED
#   define ISR_EMBED_PALFI_TIM  ENABLED
#endif




#if (ISR_EMBED(PALFI_GPIO) == ENABLED)

#if (CC_SUPPORT == CL430)
#   pragma vector=PALFI_BUSY_VECTOR
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(PALFI_BUSY_VECTOR)
#elif (CC_SUPPORT == IAR_V5)
#else
#   error "A known compiler has not been defined"
#endif
    OT_INTERRUPT void palfi_exti_isr(void) {
        palfi_signal_isr(signal_pin);
    }
#endif




#if (ISR_EMBED(PALFI_TIM) == ENABLED)

#if (CC_SUPPORT == CL430)
#   pragma vector=PALFI_TIM_VECTOR
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(PALFI_TIM_VECTOR)
#elif (CC_SUPPORT == IAR_V5)
#else
#   error "A known compiler has not been defined"
#endif
    OT_INTERRUPT void palfi_tim_isr(void) {
        palfi_trimming_isr();
    }
#endif




/** Mpipe Main Public Functions  <BR>
  * ========================================================================
  */
#ifndef EXTF_mpipedrv_footerbytes
ot_u8 mpipedrv_footerbytes() {
    return MPIPE_FOOTERBYTES;
}
#endif


#ifndef EXTF_mpipedrv_init
ot_int mpipedrv_init(void* port_id) {

}
#endif



#ifndef EXTF_mpipedrv_block
void mpipedrv_block() {

}
#endif


#ifndef EXTF_mpipedrv_unblock
void mpipedrv_unblock() {

}
#endif



#ifndef EXTF_mpipedrv_kill
void mpipedrv_kill() {
    MPIPE_DMAEN(OFF);
}
#endif



#ifndef EXTF_mpipedrv_wait
void mpipedrv_wait() {

}
#endif



#ifndef EXTF_mpipedrv_setspeed
void mpipedrv_setspeed(mpipe_speed speed) {

}
#endif




#ifndef EXTF_mpipedrv_txndef
void mpipedrv_txndef(ot_bool blocking, mpipe_priority data_priority) {

}
#endif




#ifndef EXTF_palfidrv_spitx
void palfidrv_spitx(ot_int tx_length, ot_u8* tx_data) {
    palfidrv_spitrx(tx_length, tx_data, NULL);
}
#endif

#ifndef EXTF_palfidrv_spirx
void palfidrv_spirx(ot_int rx_length, ot_u8* rx_data) {
    palfidrv_spitrx(rx_length, NULL, rx_data);
}
#endif

#ifndef EXTF_palfidrv_spitrx
void palfidrv_spitrx(ot_int trx_length, ot_u8* tx_data, ot_u8* rx_data) {
    palfidrv.trxlen = trx_length;
    palfidrv.txbuf  = tx_data;
    palfidrv.rxbuf  = rx_data;
    
    //kickoff SPI via interrupt
}
#endif



#ifndef EXTF_palfidrv_signal_isr
void palfidrv_signal_isr(ot_u16 signal_index) {
    switch (__even_in_range(signal_index, 32) {
        case (PALFI_WAKE_PINNUM<<1):
        case (PALFI_BUSY_PINNUM<<1):    
        case ...
        case ...
        case ...
        case ...
    }
}
#endif


#ifndef EXTF_palfidrv_data_isr
void palfidrv_data_isr() {

}
#endif



#endif
#endif

