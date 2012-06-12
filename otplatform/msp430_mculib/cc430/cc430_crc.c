/* Copyright 2009 JP Norair
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
/** @file       /Platforms/CC430/cc430_lib/cc430_crc.c
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Dec 2009
  * @brief      Library resources for CRC peripheral
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#include "cc430_lib.h"


/*
MSP430F5 CRC Registers for HW computation of CRC16

Register                        Short Form      Type            Access  Offset  Initial

CRC Data In                     CRCDI           Read/write      Word    0000h   0000h
                                CRCDI_L         Read/write      Byte    0000h   00h
                                CRCDI_H         Read/write      Byte    0001h   00h

CRC Data In Reverse Byte(1)     CRCDIRB         Read/write      Word    0002h   0000h
                                CRCDIRB_L       Read/write      Byte    0002h   00h
                                CRCDIRB_H       Read/write      Byte    0003h   00h
                                
CRC Initialization and Result   CRCINIRES       Read/write      Word    0004h   FFFFh
                                CRCINIRES_L     Read/write      Byte    0004h   FFh
                                CRCINIRES_H     Read/write      Byte    0005h   FFh

CRC Result Reverse(1)           CRCRESR         Read only       Word    0006h   FFFFh
                                CRCRESR_L       Read/write      Byte    0006h   FFh
                                CRCRESR_H       Read/write      Byte    0007h   FFh

*/



void CRC_Init( u16 init_word ) {
    CRCINIRES = init_word;
}



void CRC_Calc( u16 word, u16 direction ) {
    
    if (direction == CRC_DIR_FORWARD) {
        CRCDI = word;
    }
    else {
        CRCDIRB = word;
    }
}



u16 CRC_Result( u16 direction ) {

    if (direction == CRC_DIR_FORWARD) {
        return (u16)CRCINIRES;
    }
    return (u16)CRCRESR;
}


