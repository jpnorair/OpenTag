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
  * @file       /lib/CMSIS/Include/_local/cm_saturation.h
  * @author     JP Norair
  * @version    R100
  * @date       21 Aug 2014
  * @brief      Saturation Instructions for ARM Cortex-M
  *
  * This file should be included from cm0_endian.h, cm3_endian.h, etc.
  * 
  ******************************************************************************
  */
  

#ifndef __CM_SATURATION_H
#define __CM_SATURATION_H

// Using integer limit (aligns to power of 2)
#define __saturate_u32int(input, limit)     __USAT(input, (31 - __CLZ(limit)))
#define __saturate_s32int(input, limit)     __SSAT(input, (31 - __CLZ(limit)))

// Using power of 2 limit
#define __saturate_u32(input, bitlimit)     __USAT(input, bitlimit)
#define __saturate_s32(input, bitlimit)     __SSAT(input, bitlimit)    
    
    
#endif
