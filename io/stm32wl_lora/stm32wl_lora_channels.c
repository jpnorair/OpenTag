/* Copyright 2020 JP Norair
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
  * @file       /io/stm32wl_lora/stm32wl_lora_channels.c
  * @author     JP Norair
  * @version    R100
  * @date       31 Jan 2022
  * @brief      Channel Plans for different regulatory regimes
  * @ingroup    STM32WL_LoRa
  * 
  * @todo this file is a very incomplete port from SX127x to STM32WL LoRa.
  *       It is only used right now for compiling platform tests, and it
  *       will be overhauled shortly. 
  *
  ******************************************************************************
  */


#include <otstd.h>
#include <board.h>
#if defined(__STM32WL_LORA__)

#include <io/stm32wl_lora/interface.h>
#include <io/stm32wl_lora/config.h>

// For handling of phymac struct
#include <m2/radio.h>

/// LoRa channel changing requires changing the base frequency, as there is no
/// channel register for doing adjustments to the base frequency.  DASH7 over
/// LoRa uses the following channel rules:
///
/// 433 MHz band: 15x 116 kHz Subchannels from 433.05 - 434.79 MHz
/// 866 MHz band: 15x 250 kHz Subchannels from 865 - 869.250 MHz
/// 915 MHz band: 15x 1500 kHz Subchannels from 902 - 924.5 MHz

#define BASE_432M992Hz      0x6C3F81
#define BASE_864M875Hz      0xD83800
#define BASE_901M250Hz      0xE15000
#define BASE_901M133Hz      0xE14888
#define SPACE_116kHz        0x76C
#define SPACE_250kHz        0x1000
#define SPACE_1500kHz       0x6000
#define SPACE_1733kHz       0x6EEF
#define __FREQ24(FC)        (0x80 | RFREG_LR_FRFMSB), (((FC)>>16)&0xFF), (((FC)>>8)&0xff), (((FC)>>0)&0xff)
#define FREQ_SIZE           4


/// Bandplan tables must be 60 bytes each: 15 channels * 4 bytes each channel

/// CODE 0: US ISM band 915MHz
static const ot_u8 channels_ISM915[] = {
    __FREQ24(BASE_901M133Hz+(1*SPACE_1733kHz)),
    __FREQ24(BASE_901M133Hz+(2*SPACE_1733kHz)),
    __FREQ24(BASE_901M133Hz+(3*SPACE_1733kHz)),
    __FREQ24(BASE_901M133Hz+(4*SPACE_1733kHz)),
    __FREQ24(BASE_901M133Hz+(5*SPACE_1733kHz)),
    __FREQ24(BASE_901M133Hz+(6*SPACE_1733kHz)),
    __FREQ24(BASE_901M133Hz+(7*SPACE_1733kHz)),
    __FREQ24(BASE_901M133Hz+(8*SPACE_1733kHz)),
    __FREQ24(BASE_901M133Hz+(9*SPACE_1733kHz)),
    __FREQ24(BASE_901M133Hz+(10*SPACE_1733kHz)),
    __FREQ24(BASE_901M133Hz+(11*SPACE_1733kHz)),
    __FREQ24(BASE_901M133Hz+(12*SPACE_1733kHz)),
    __FREQ24(BASE_901M133Hz+(13*SPACE_1733kHz)),
    __FREQ24(BASE_901M133Hz+(14*SPACE_1733kHz)),
    __FREQ24(BASE_901M133Hz+(15*SPACE_1733kHz))
};

/// CODE 1: EU ISM band 866
static const ot_u8 channels_ISM866[] = {
    __FREQ24(BASE_864M875Hz+(1*SPACE_250kHz)),
    __FREQ24(BASE_864M875Hz+(2*SPACE_250kHz)),
    __FREQ24(BASE_864M875Hz+(3*SPACE_250kHz)),
    __FREQ24(BASE_864M875Hz+(4*SPACE_250kHz)),
    __FREQ24(BASE_864M875Hz+(5*SPACE_250kHz)),
    __FREQ24(BASE_864M875Hz+(6*SPACE_250kHz)),
    __FREQ24(BASE_864M875Hz+(7*SPACE_250kHz)),
    __FREQ24(BASE_864M875Hz+(8*SPACE_250kHz)),
    __FREQ24(BASE_864M875Hz+(9*SPACE_250kHz)),
    __FREQ24(BASE_864M875Hz+(10*SPACE_250kHz)),
    __FREQ24(BASE_864M875Hz+(11*SPACE_250kHz)),
    __FREQ24(BASE_864M875Hz+(12*SPACE_250kHz)),
    __FREQ24(BASE_864M875Hz+(13*SPACE_250kHz)),
    __FREQ24(BASE_864M875Hz+(14*SPACE_250kHz)),
    __FREQ24(BASE_864M875Hz+(15*SPACE_250kHz))
};

/// CODE 2: EU ISM 433
static const ot_u8 channels_ISM433[] = {
    __FREQ24(BASE_432M992Hz+(1*SPACE_116kHz)),
    __FREQ24(BASE_432M992Hz+(2*SPACE_116kHz)),
    __FREQ24(BASE_432M992Hz+(3*SPACE_116kHz)),
    __FREQ24(BASE_432M992Hz+(4*SPACE_116kHz)),
    __FREQ24(BASE_432M992Hz+(5*SPACE_116kHz)),
    __FREQ24(BASE_432M992Hz+(6*SPACE_116kHz)),
    __FREQ24(BASE_432M992Hz+(7*SPACE_116kHz)),
    __FREQ24(BASE_432M992Hz+(8*SPACE_116kHz)),
    __FREQ24(BASE_432M992Hz+(9*SPACE_116kHz)),
    __FREQ24(BASE_432M992Hz+(10*SPACE_116kHz)),
    __FREQ24(BASE_432M992Hz+(11*SPACE_116kHz)),
    __FREQ24(BASE_432M992Hz+(12*SPACE_116kHz)),
    __FREQ24(BASE_432M992Hz+(13*SPACE_116kHz)),
    __FREQ24(BASE_432M992Hz+(14*SPACE_116kHz)),
    __FREQ24(BASE_432M992Hz+(15*SPACE_116kHz))
};


/// Symbol Timing (miti) per spreading factor, per region
// ------------------------------------------
// SF11 @ 125 kHz = 16.384 ms/sym (17180)
// SF11 @ 250 kHz = 8.192 ms/sym (8590)
// SF11 @ 500 kHz = 4.096 ms/sym (4295)
// ------------------------------------------
// SF10 @ 125 kHz = 8.192 ms/sym (8590)
// SF10 @ 250 kHz = 4.096 ms/sym (4295)
// SF10 @ 500 kHz = 2.048 ms/sym (2148)
// ------------------------------------------
// SF9  @ 125 kHz = 4.096 ms/sym (4295)
// SF9  @ 250 kHz = 2.056 ms/sym (2148)
// SF9  @ 500 kHz = 1.024 ms/sym (1074)
// ------------------------------------------
// SF7  @ 41.7 kHz = 3.076 ms/sym (3222)
// SF7  @ 62.5 kHz = 2.048 ms/sym (2148)
// SF7  @ 125 kHz = 1.024 ms/sym (1074)
// SF7  @ 250 kHz = 0.512 ms/sym (537)
// SF7  @ 500 kHz = 0.256 ms/sym (269)
// ------------------------------------------
// SF5  @ 125 kHz = 0.256 ms/sym (269)
// SF5  @ 250 kHz = 0.128 ms/sym (135)
// SF5  @ 500 kHz = 0.064 ms/sym (68)
// ------------------------------------------
static const ot_u16 mitipersym_ISM915[] = {
    4295,
    1074,
    269,
    68
};
static const ot_u16 mitipersym_ISM866[] = {
    17180,
    4295,
    1074,
    269
};
static const ot_u16 mitipersym_ISM433[] = {
    8590,
    2148,
    537,
    135
};



static const ot_u16 tiperhsc_ISM915[] = {
    51,     // 12 symbols, 11 bits each, 132 bits
    17,     // 16 symbols, 9 bits each, 144 bits
    6,      // 20 symbols, 7 bits each, 140 bits
    2       // 28 symbols, 5 bits each, 140 bits
};
static const ot_u16 tiperhsc_ISM866[] = {
    202,
    68,
    21,
    8
};
static const ot_u16 tiperhsc_ISM433[] = {
    101,
    34,
    11,
    4
};




/// Bandplan LUT
static const ot_u8* channel_map[] = {
    channels_ISM915,
    channels_ISM866,
    channels_ISM433,
    channels_ISM915,     // LUT must have a power-of-two regimes, default is ISM915
};


/// Bandwidth LUT
static const ot_u8 bandwidth_lut[] = {
    _BW_500_KHZ,
    _BW_125_KHZ,
    _BW_250_KHZ,
    _BW_500_KHZ,     // LUT must have a power-of-two regimes, default is ISM915
};


/// MiTi per symbol
static const ot_u16* mitipersym_lut[] = {
    (ot_u16*)mitipersym_ISM915,
    (ot_u16*)mitipersym_ISM866,
    (ot_u16*)mitipersym_ISM433,
    (ot_u16*)mitipersym_ISM915,    // LUT must have a power-of-two regimes, default is ISM915
};


/// Ti per smallest LoRa symbol group that contains 128 bits
static const ot_u16* tiperhsc_lut[] = {
    (ot_u16*)tiperhsc_ISM915,
    (ot_u16*)tiperhsc_ISM866,
    (ot_u16*)tiperhsc_ISM433,
    (ot_u16*)tiperhsc_ISM915,    // LUT must have a power-of-two regimes, default is ISM915
};



const ot_u8* wllora_get_bandplan(ot_u8 region_code) {
    region_code &= 3;
    return channel_map[region_code];
}


const ot_u8* wllora_get_chanregs(ot_u8 region_code, ot_u8 chan_ordinal) {
    const ot_u8* channel    = wllora_get_bandplan(region_code);
    const ot_u8* chanregs   = &channel[chan_ordinal * FREQ_SIZE];
    return chanregs;
}


void wllora_configure_chan(ot_u8 region_code, ot_u8 chan_ordinal) {
    const ot_u8* chanregs = wllora_get_chanregs(region_code, chan_ordinal);
// Commented to get tests working on WL
//    wllora_spibus_io(4, 0, chanregs);
}


ot_u8 wllora_get_bw(ot_u8 region_code) {
    region_code &= 3;

    return bandwidth_lut[region_code];
}




///@todo this is subject to change
ot_u16 wllora_symbol_miti(ot_u8 region_code, ot_u8 rate_code) {
    const ot_u16* mitipersym;
    region_code &= 3;
    rate_code   &= 3;

    mitipersym = mitipersym_lut[region_code];

    return mitipersym[rate_code];
}


///@todo this is subject to change
ot_u16 wllora_block_miti(const void* phy_handle) {
    const phymac_struct* pm = phy_handle;
    ot_u8 regime;
    ot_u8 rcode;
    ot_u16 miti;

    regime  = pm->flags & 3;
    rcode   = (pm->channel >> 4) & 3;
    miti    = (mitipersym_lut[regime])[rcode];
    miti   *= (pm->channel & 0x80) ? 4 : 5;

    return miti;
}


ot_u16 wllora_hscblock_ti(const void* phy_handle) {
    const phymac_struct* pm = phy_handle;
    ot_u8 regime;
    ot_u8 rcode;

    regime  = pm->flags & 3;
    rcode   = (pm->channel >> 4) & 3;
    return (tiperhsc_lut[regime])[rcode];
}




// LoRaWAN Stuff: not sure what to do with this, if anything
#if (0)

/** Set power for LoRaWAN */
#ifndef _PABOOST_OFFSET
#   define _PABOOST_OFFSET  0
#else
#   undef _PABOOST_OFFSET
#   define _PABOOST_OFFSET  3
#endif

#define __SX127x_DBM_TO_TXPWR(DBM)  ((DBM+1-_PABOOST_OFFSET) & 0xF)

const ot_u8 lorawan_915_powercodes[11] = {
    __SX127x_DBM_TO_TXPWR(30),
    __SX127x_DBM_TO_TXPWR(28),
    __SX127x_DBM_TO_TXPWR(26),
    __SX127x_DBM_TO_TXPWR(24),
    __SX127x_DBM_TO_TXPWR(22),
    __SX127x_DBM_TO_TXPWR(20),
    __SX127x_DBM_TO_TXPWR(18),
    __SX127x_DBM_TO_TXPWR(16),
    __SX127x_DBM_TO_TXPWR(14),
    __SX127x_DBM_TO_TXPWR(12),
    __SX127x_DBM_TO_TXPWR(10)
};

const ot_u8 lorawan_868_powercodes[6] = {
    __SX127x_DBM_TO_TXPWR(20),
    __SX127x_DBM_TO_TXPWR(14),
    __SX127x_DBM_TO_TXPWR(11),
    __SX127x_DBM_TO_TXPWR(8),
    __SX127x_DBM_TO_TXPWR(5),
    __SX127x_DBM_TO_TXPWR(2)
};

const ot_u8 lorawan_433_powercodes[6] = {
    __SX127x_DBM_TO_TXPWR(10),
    __SX127x_DBM_TO_TXPWR(7),
    __SX127x_DBM_TO_TXPWR(4),
    __SX127x_DBM_TO_TXPWR(1),
    __SX127x_DBM_TO_TXPWR(-2),
    __SX127x_DBM_TO_TXPWR(-5)
};

#define lorawan_780_powercodes  lorwan_433_powercodes


/** Set Data Rate for LoRaWAN */

const ot_u8 lorawan_915_drcodes[16][2] = {
    {_BW_125_KHZ, _SF10},
    {_BW_125_KHZ, _SF9},
    {_BW_125_KHZ, _SF8},
    {_BW_125_KHZ, _SF7},
    {_BW_500_KHZ, _SF8},
    {_BW_500_KHZ, _SF7},    // RFU in spec: defaulted
    {_BW_500_KHZ, _SF7},    // RFU in spec: defaulted
    {_BW_500_KHZ, _SF7},    // RFU in spec: defaulted
    {_BW_500_KHZ, _SF12},
    {_BW_500_KHZ, _SF11},
    {_BW_500_KHZ, _SF10},
    {_BW_500_KHZ, _SF9},
    {_BW_500_KHZ, _SF8},
    {_BW_500_KHZ, _SF7},
    {_BW_500_KHZ, _SF7},    // RFU in spec: defaulted
    {_BW_500_KHZ, _SF7}     // RFU in spec: defaulted
};

const ot_u8 lorawan_868_drcodes[8][2] = {
    {_BW_125_KHZ, _SF12},
    {_BW_125_KHZ, _SF11},
    {_BW_125_KHZ, _SF10},
    {_BW_125_KHZ, _SF9},
    {_BW_125_KHZ, _SF8},
    {_BW_125_KHZ, _SF7},
    {_BW_250_KHZ, _SF7},
    {0, 0}                  // FSK placeholder
};

#define lorawan_780_drcodes lorawan_868_drcodes
#define lorawan_433_drcodes lorawan_868_drcodes


/** LoRaWAN RX Window channel */
// Channel Hertz: (replace with reg vals)
// DR code: index in lorawan_xxx_drcodes

const ot_u32 lorawan_915_rxwnd2[2] = { 923300000, 8};
const ot_u32 lorawan_868_rxwnd2[2] = { 869525000, 0};
const ot_u32 lorawan_780_rxwnd2[2] = { 786000000, 0};
const ot_u32 lorawan_433_rxwnd2[2] = { 434665000, 0};


/** LoRaWAN Bands */

const ot_uint lorawan_915_bandspec[1][4] = {
    { 1, 5, 0, 0 }  // 5=20dBm, 1=100% duty
};

const ot_uint lorawan_868_bandspec[5][4] = {
    { 100,  1, 0, 0 },  // 1=14dBm, 100=1% duty
    { 100,  1, 0, 0 },  // 1=14dBm, 100=1% duty
    { 1000, 1, 0, 0 },  // 1=14dBm, 1000=0.1% duty
    { 10,   1, 0, 0 },  // 1=14dBm, 10=10% duty
    { 100,  1, 0, 0 }   // 1=14dBm, 100=1% duty
};

const ot_uint lorawan_780_bandspec[1][4] = {
    { 100, 0, 0, 0 }  // 0=10dBm, 100=1% duty
};

#define lorawan_433_bandspec lorawan_780_bandspec


/** LoRaWAN Default Channels */

//Todo: rubric for 915 channels
// LORA_MAX_NB_CHANNELS must be 8 or more
// 125 kHz channels go from 0 to (LORA_MAX_NB_CHANNELS-8)
// 500 kHz channels go from (LORA_MAX_NB_CHANNELS-8) to LORA_MAX_NB_CHANNELS
// Just set a pointer to some line in the drcodes matrix

const ot_u8 lorawan_868_defchan[3][3] = {
    { 868100000, (DR_5<<4) | DR_0, 0 },
    { 868300000, (DR_5<<4) | DR_0, 0 },
    { 868500000, (DR_5<<4) | DR_0, 0 }
};

const ot_u8 lorawan_780_defchan[3][3] = {
    { 779500000, (DR_5<<4) | DR_0, 0 },
    { 779700000, (DR_5<<4) | DR_0, 0 },
    { 779900000, (DR_5<<4) | DR_0, 0 }
};

const ot_u8 lorawan_433_defchan[3][3] = {
    { 433175000, (DR_5<<4) | DR_0, 0 },
    { 433375000, (DR_5<<4) | DR_0, 0 },
    { 433575000, (DR_5<<4) | DR_0, 0 }
};




/** LoRaWAN Duty Cycle Back-off Procedure */




/** LoRaWAN mega value table */

#endif // End of LoRaWAN stuff



#endif


