/*  Copyright 2008-2022, JP Norair
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted provided that the following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright 
  *    notice, this list of conditions and the following disclaimer in the 
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
  * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
  * POSSIBILITY OF SUCH DAMAGE.
  */
/**
  * @file       /apps/_common/board_select.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       16 Oct 2016
  * @brief      Board Selector macros
  *
  ******************************************************************************
  */

#ifndef __BOARD_SELECT_H
#define __BOARD_SELECT_H



/** List of all known boards
Board Name              | OT Status | Manufacturer  | Man Status    |   Notes
--------------------------------------------------------------------------------
BOARD_OMG_CC430         | EOL       | JP Norair     | EOL           | CC430
BOARD_EM430RF           | EOL       | TI            | EOL           | CC430
BOARD_eZ430Chronos      | EOL       | TI            | Available     | CC430
BOARD_RF430USB_5509     | EOL       | TI            | Unknown       | MSP430
BOARD_IKR001            | Supported | ST            | EOL           | STM32L1
BOARD_Jupiter_R1        | Supported | Haystack      | EOL           | STM32L1
BOARD_Jupiter_R2        | Supported | Haystack      | EOL           | STM32L1
BOARD_HayTag_R1         | Supported | Haystack      | Available     | STM32L1
BOARD_HayTag_LI9R1      | Supported | Haystack      | Available     | STM32L1
BOARD_HayTag_LI9T1      | Supported | Haystack      | Available     | STM32L1
BOARD_HayTag_LI30R1     | Supported | Haystack      | Available     | STM32L1
BOARD_HayTag_LI30T1     | Supported | Haystack      | Available     | STM32L1
BOARD_Nucleo_L053       | Supported | ST            | Available     | STM32L0
BOARD_Nucleo_L073       | Supported | ST            | Available     | STM32L0
BOARD_Nucleo_LRWAN1     | Supported | ST            | Available     | STM32L0
BOARD_Discovery_LoRa    | Supported | ST            | Available     | STM32L0
BOARD_Discovery_LoRa_GPS| Supported | ST + Haystack | Available     | STM32L0
BOARD_Launchpad_CC1310  | Pending   | TI            | Available     | CC1310
BOARD_Launchpad_CC1350  | Pending   | TI            | Available     | CC1310
BOARD_HayTag_LoRa1      | Pending   | Haystack/UDEA | EOL           | STM32L1
BOARD_HayTag_LoRa       | Pending   | Haystack/UDEA | Prototyping   | STM32L0
BOARD_HayTag2_R0        | Pending   | Haystack      | In Dev.       | CC13xx
*/ 


#define BOARD_HAS_CC430()         \
   (defined(BOARD_OMG_CC430) || \
    defined(BOARD_EM430RF))

#define BOARD_HAS_MSP430F5()      \
   (defined(RF430USB_5509))

#define BOARD_HAS_STM32L0()     \
   (defined(BOARD_Discovery_LoRa) || \
    defined(BOARD_Discovery_LoRa_GPS) || \
    defined(BOARD_Nucleo_L053) || \
    defined(BOARD_Nucleo_L073) || \
    defined(BOARD_Nucleo_LRWAN1) || \
    defined(BOARD_HayTag_LoRa))

#define BOARD_HAS_STM32L1()     \
   (defined(BOARD_IKR001) || \
    defined(BOARD_Jupiter_R1) || \
    defined(BOARD_Jupiter_R2) || \
    defined(BOARD_HayTag_R1) || \
    defined(BOARD_HayTag_LI9R1) || \
    defined(BOARD_HayTag_LI9T1) || \
    defined(BOARD_HayTag_LI30R1) || \
    defined(BOARD_HayTag_LI30T1) || \
    defined(BOARD_HayTag_LoRa1))

#define BOARD_HAS_CC13XX()     \
   (defined(BOARD_Launchpad_CC1310) || \
    defined(BOARD_Launchpad_CC1350) || \
    defined(BOARD_HayTag2_R0))

#define BOARD_IS_OFFICIAL() \
   (BOARD_HAS_CC430() || \
    BOARD_HAS_MSP430F5() || \
    BOARD_HAS_STM32L1() || \
    BOARD_HAS_STM32L0() || \
    BOARD_HAS_CC13XX() )

#define BOARD_IS_SUPPORTED() \
   (defined(BOARD_Nucleo_L053) || \
    defined(BOARD_Nucleo_L073) || \
    defined(BOARD_Nucleo_LRWAN1) || \
    defined(BOARD_IKR001) || \
    defined(BOARD_Jupiter_R1) || \
    defined(BOARD_Jupiter_R2) || \
    defined(BOARD_HayTag_R1) || \
    defined(BOARD_HayTag_LI9R1) || \
    defined(BOARD_HayTag_LI9T1) || \
    defined(BOARD_HayTag_LI30R1) || \
    defined(BOARD_HayTag_LI30T1) || \
    defined(BOARD_HayTag_R2) \\ \
    defined(BOARD_Launchpad_CC1310) || \
    defined(BOARD_Launchpad_CC1350) || \
    defined(BOARD_HayTag2_R0))

#define BOARD_SUPPORTS_GULP() \
    (BOARD_HAS_CC430() || BOARD_HAS_MSP430F5())
    
#define BOARD_SUPPORTS_HICCULP() \
    (BOARD_HAS_STM32L1() || BOARD_HAS_STM32L0())



#endif 
