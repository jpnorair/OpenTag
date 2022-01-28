/* Copyright 2010-2016 JP Norair
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
BOARD_IKR001            | Supported | ST            | EOL           | STM32L
BOARD_Jupiter_R1        | Supported | Haystack      | EOL           | STM32L
BOARD_Jupiter_R2        | Supported | Haystack      | EOL           | STM32L
BOARD_HayTag_R1         | Supported | Haystack      | Available     | STM32L
BOARD_HayTag_LI9R1      | Supported | Haystack      | Available     | STM32L
BOARD_HayTag_LI9T1      | Supported | Haystack      | Available     | STM32L
BOARD_HayTag_LI30R1     | Supported | Haystack      | Available     | STM32L
BOARD_HayTag_LI30T1     | Supported | Haystack      | Available     | STM32L
BOARD_Discovery_LoRa    | Supported | ST            | Available     | STM32L0
BOARD_Discovery_LoRa_GPS| Supported | ST+Haystack   | Available     | STM32L0
BOARD_foam_discovery    | Supported | ST+Haystack   | Available     | STM32L0
BOARD_Nucleo_L053       | Supported | ST            | Available     | STM32L0
BOARD_Nucleo_L073       | Supported | ST            | Available     | STM32L0
BOARD_Nucleo_LRWAN1     | Supported | ST            | Available     | STM32L0
BOARD_Nucleo_L412_LoRa  | Supported | Haystack      | Proto-only    | STM32L4
BOARD_Nucleo_WL55       | Supported | ST            | Proto         | STM32WL
BOARD_Launchpad_CC1310  | Pending   | TI            | Available     | CC1310
BOARD_Launchpad_CC1350  | Pending   | TI            | Available     | CC1310
BOARD_HayTag_LoRa1      | Supported | Haystack/UDEA | Proto-only    | STM32L1
BOARD_HayTag_LoRa       | Supported | Haystack/UDEA | Prototyping   | STM32L0
BOARD_HayTag_LoRa_R2    | Supported | Haystack/UDEA | Prototyping   | STM32L0
BOARD_HayTag_LoRa_R3    | Supported | Haystack/UDEA | Prototyping   | STM32L4
BOARD_HayTag_LoRa_MiniGW| Supported | Haystack/UDEA | Available     | STM32L0
BOARD_HayTag_CC13xx     | Pending   | Haystack      | In Dev.       | CC13xx
*/ 


#define BOARD_HAS_CC430()         \
   (defined(BOARD_OMG_CC430) || \
    defined(BOARD_EM430RF))

#define BOARD_HAS_MSP430F5()      \
   (defined(RF430USB_5509))

#define BOARD_HAS_STM32L0()     \
   (defined(BOARD_Discovery_LoRa) || \
    defined(BOARD_Discovery_LoRa_GPS) || \
    defined(BOARD_foam_discovery) || \
    defined(BOARD_Nucleo_L053) || \
    defined(BOARD_Nucleo_L073) || \
    defined(BOARD_Nucleo_LRWAN1) || \
    defined(BOARD_HayTag_LoRa) || \
    defined(BOARD_HayTag_LoRa_GPS) || \
    defined(BOARD_HayTag_LoRa_GPS_ACCEL) || \
    defined(BOARD_HayTag_LoRa_R2) || \
    defined(BOARD_HayTag_LoRa_R2_GPS) || \
    defined(BOARD_HayTag_LoRa_R2_GPS_ACCEL) || \
	defined(BOARD_HayTag_LoRa_MiniGW) )

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

#define BOARD_HAS_STM32L4()     \
   (defined(BOARD_Nucleo_L412_LoRa) || \
    defined(BOARD_HayTag_LoRa_R3) )

#define BOARD_HAS_STM32WL55()     \
   (defined(BOARD_Nucleo_WL55) )

#define BOARD_HAS_CC13XX()     \
   (defined(BOARD_Launchpad_CC1310) || \
    defined(BOARD_Launchpad_CC1350) || \
    defined(BOARD_HayTag2_R0))

#define BOARD_IS_OFFICIAL() \
   (BOARD_HAS_CC430() || \
    BOARD_HAS_MSP430F5() || \
    BOARD_HAS_STM32WL55() || \
    BOARD_HAS_STM32L4() || \
    BOARD_HAS_STM32L1() || \
    BOARD_HAS_STM32L0() || \
    BOARD_HAS_CC13XX() )

#define BOARD_IS_SUPPORTED() \
   (defined(BOARD_Nucleo_L053) || \
    defined(BOARD_Nucleo_L073) || \
    defined(BOARD_Nucleo_LRWAN1) || \
    defined(BOARD_Nucleo_L412_LoRa) || \
    defined(BOARD_Nucleo_WL55) || \
    defined(BOARD_Discovery_LoRa) || \
    defined(BOARD_Discovery_LoRa_GPS) || \
    defined(BOARD_foam_discovery) || \
    defined(BOARD_IKR001) || \
    defined(BOARD_Jupiter_R1) || \
    defined(BOARD_Jupiter_R2) || \
    defined(BOARD_HayTag_R1) || \
    defined(BOARD_HayTag_LI9R1) || \
    defined(BOARD_HayTag_LI9T1) || \
    defined(BOARD_HayTag_LI30R1) || \
    defined(BOARD_HayTag_LI30T1) || \
    defined(BOARD_HayTag_LoRa) || \
    defined(BOARD_HayTag_LoRa_GPS) || \
    defined(BOARD_HayTag_LoRa_GPS_ACCEL) || \
    defined(BOARD_HayTag_LoRa_R2) || \
    defined(BOARD_HayTag_LoRa_R2_GPS) || \
    defined(BOARD_HayTag_LoRa_R2_GPS_ACCEL) \
    defined(BOARD_HayTag_LoRa_R3))

#define BOARD_SUPPORTS_GULP() \
    (BOARD_HAS_CC430() || BOARD_HAS_MSP430F5())
    
#define BOARD_SUPPORTS_HICCULP() \
   (BOARD_HAS_STM32WL55() || \
    BOARD_HAS_STM32L4() || \
    BOARD_HAS_STM32L1() || \
    BOARD_HAS_STM32L0() )



#endif 
