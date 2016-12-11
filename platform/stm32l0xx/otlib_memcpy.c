/* Copyright 2014 JP Norair
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
  * @file       /otplatform/stm32l1xx/memcpy_driver.c
  * @author     JP Norair
  * @version    R100
  * @date       27 Aug 2014
  * @brief      Memcpy Driver for STM32L (uses DMA)
  * @ingroup    Memcpy
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>
//#include <platform/memcpy.h>



/** Platform memcpy Routine <BR>
  * ========================================================================<BR>
  * Similar to standard implementation of "memcpy"
  * Behavior is always blocking.  These functions either use the DMA or, if not
  * configured this way in the board support header, they use Duff's Device for
  * loop unrolling.
  */

#define DUFF_DEVICE_8(DST_OP, SRC_OP, LEN)      \
    if (LEN > 0) {                              \
        ot_int loops = (LEN + 7) >> 3;          \
        switch (LEN & 0x7) {                    \
            case 0: do {    DST_OP = SRC_OP;    \
            case 7:         DST_OP = SRC_OP;    \
            case 6:         DST_OP = SRC_OP;    \
            case 5:         DST_OP = SRC_OP;    \
            case 4:         DST_OP = SRC_OP;    \
            case 3:         DST_OP = SRC_OP;    \
            case 2:         DST_OP = SRC_OP;    \
            case 1:         DST_OP = SRC_OP;    \
                    } while (--loops > 0);      \
        }                                       \
    }



#if MCU_CONFIG(MEMCPYDMA)
#   define MEMCPY_DMA_INT  (1 << ((MEMCPY_DMA_CHAN_ID-1)*4))

void sub_memcpy_dma(ot_u8* dest, ot_u8* src, ot_uint length) {
/// Use 8, 16, or 32 bit chunks based on detected alignment
    static const ot_u16 ccr[4]      = { 0x4AD1, 0x40D1, 0x45D1, 0x40D1 };
    static const ot_u16 len_div[4]  = { 2, 0, 1, 0 };
    ot_int align;

    platform_disable_interrupts();

    MEMCPY_DMACHAN->CCR     = 0;
    MEMCPY_DMA->IFCR        = MEMCPY_DMA_INT;
    MEMCPY_DMACHAN->CPAR    = (ot_u32)dest;
    MEMCPY_DMACHAN->CMAR    = (ot_u32)src;
    align                   = ((ot_u32)dest | (ot_u32)src | (ot_u32)length) & 3;
    length                >>= len_div[align];
    MEMCPY_DMACHAN->CNDTR   = length;
    MEMCPY_DMACHAN->CCR     = ccr[align];
    while((MEMCPY_DMA->ISR & MEMCPY_DMA_INT) == 0);

    platform_enable_interrupts();
}


void sub_memcpy2_dma(ot_u16* dest, ot_u16* src, ot_uint length) {
/// Use 16 or 32 bit chunks based on detected alignment
    ot_u16 ccr_val = 0x45D1;

    platform_disable_interrupts();

    // Set to 32 bit chunks if alignment allows it
    if ((((ot_u32)dest | (ot_u32)src | (ot_u32)length) & 3) == 0) {
        length >>= 1;
        ccr_val += 0x0500;
    }
    MEMCPY_DMACHAN->CCR     = 0;
    MEMCPY_DMA->IFCR        = MEMCPY_DMA_INT;
    MEMCPY_DMACHAN->CPAR    = (ot_u32)dest;
    MEMCPY_DMACHAN->CMAR    = (ot_u32)src;
    MEMCPY_DMACHAN->CNDTR   = length;
    MEMCPY_DMACHAN->CCR     = ccr_val;
    while((MEMCPY_DMA->ISR & MEMCPY_DMA_INT) == 0);

    platform_enable_interrupts();
}


void sub_memcpy4_dma(ot_u32* dest, ot_u32* src, ot_uint length) {
/// 32 bit chunks based on detected alignment
    platform_disable_interrupts();

    MEMCPY_DMACHAN->CCR     = 0;
    MEMCPY_DMA->IFCR        = MEMCPY_DMA_INT;
    MEMCPY_DMACHAN->CPAR    = (ot_u32)dest;
    MEMCPY_DMACHAN->CMAR    = (ot_u32)src;
    MEMCPY_DMACHAN->CNDTR   = length;
    MEMCPY_DMACHAN->CCR     = 0x4AD1;
    while((MEMCPY_DMA->ISR & MEMCPY_DMA_INT) == 0);

    platform_enable_interrupts();
}


void sub_memset_dma(ot_u8* dest, ot_u8* src, ot_uint length) {
    /// Use 8, 16, or 32 bit chunks based on detected alignment
    static const ot_u16 ccr[4]      = { 0x4AD1, 0x40D1, 0x45D1, 0x40D1 };
    static const ot_u16 len_div[4]  = { 2, 0, 1, 0 };
    ot_int align;

    platform_disable_interrupts();

    MEMCPY_DMACHAN->CCR     = 0;
    MEMCPY_DMA->IFCR        = MEMCPY_DMA_INT;
    MEMCPY_DMACHAN->CPAR    = (ot_u32)dest;
    MEMCPY_DMACHAN->CMAR    = (ot_u32)src;
    align                   = ((ot_u32)dest | (ot_u32)src | (ot_u32)length) & 3;
    length                >>= len_div[align];
    MEMCPY_DMACHAN->CNDTR   = length;
    MEMCPY_DMACHAN->CCR     = ccr[align];
    while((MEMCPY_DMA->ISR & MEMCPY_DMA_INT) == 0);

    platform_enable_interrupts();
}

#endif  //MCU_CONFIG(MEMCPYDMA)




void ot_memcpy(ot_u8* dst, ot_u8* src, ot_uint length) {
#   if MCU_CONFIG(MEMCPYDMA)
        sub_memcpy_dma(dst, src, length);
#   else
        DUFF_DEVICE_8(*dst++, *src++, length);
#   endif
}

void ot_memcpy2(ot_u16* dst, ot_u16* src, ot_uint length) {
#   if MCU_CONFIG(MEMCPYDMA)
        sub_memcpy2_dma( dst, src, length);
#   else
        ot_memcpy((ot_u8*)dst, (ot_u8*)src, length<<1);
#   endif
}

void ot_memcpy4(ot_u32* dst, ot_u32* src, ot_uint length) {
#   if MCU_CONFIG(MEMCPYDMA)
        sub_memcpy4_dma( dst, src, length);
#   else
        ot_memcpy((ot_u8*)dst, (ot_u8*)src, length<<2);
#   endif
}


void ot_memset(ot_u8* dst, ot_u8 value, ot_uint length) {
    platform_disable_interrupts();
#   if MCU_CONFIG(MEMCPYDMA)
        MEMCPY_DMACHAN->CCR     = 0;
        MEMCPY_DMA->IFCR        = MEMCPY_DMA_INT;       ///@todo see if this can be globalized
        MEMCPY_DMACHAN->CPAR    = (ot_u32)dst;
        MEMCPY_DMACHAN->CMAR    = (ot_u32)&value;
        MEMCPY_DMACHAN->CNDTR   = length;
        MEMCPY_DMACHAN->CCR     = DMA_CCR_DIR      | DMA_CCR_PINC     | \
                                  (0<<DMA_CCR_PL_Pos)   | DMA_CCR_MEM2MEM  | \
                                  DMA_CCR_EN;
        while((MEMCPY_DMA->ISR & MEMCPY_DMA_INT) == 0);
#   else
        DUFF_DEVICE_8(*dst++, value, length);
#   endif
    platform_enable_interrupts();
}


void ot_memset_2(ot_u16* dst, ot_u16 value, ot_uint length) {
    ot_memset( (ot_u8*)dst, (ot_u8)value, length<<1 );
}

void ot_memset_4(ot_u32* dst, ot_u32 value, ot_uint length) {
    ot_memset( (ot_u8*)dst, (ot_u8)value, length<<2 );
}





