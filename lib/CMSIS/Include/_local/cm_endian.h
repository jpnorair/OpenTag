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
  * @file       /lib/CMSIS/Include/_local/cm_endian.h
  * @author     JP Norair
  * @version    R100
  * @date       21 Aug 2014
  * @brief      Endian setup for ARM Cortex-M
  *
  * This file should be included from cm0_endian.h, cm3_endian.h, etc.
  * 
  ******************************************************************************
  */
  

#ifndef __CM_ENDIAN_H
#define __CM_ENDIAN_H

// Conversions to/from network endian
#define __netendian16(x)        __REV16(x)

#define __netendian32(x)        __REV(x)

#define __netendian16_imm(n)    (uint16_t)( (((uint16_t)n)<<8) | (((uint16_t)n)>>8) )

#define __netendian32_imm(n)    (uint32_t)( \
                                ((((uint32_t)n) << 24) & (0xff000000)) \
                              | ((((uint32_t)n) << 8)  & (0x00ff0000)) \
                              | ((((uint32_t)n) >> 8)  & (0x0000ff00)) \
                              | ((((uint32_t)n) >> 24) & (0x000000ff)) )

#endif
