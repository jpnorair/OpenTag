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
  * @file       /otplatform/cc430/chronoslcd.h
  * @author     JP Norair
  * @version    R100
  * @date       1 Dec 2012
  * @brief      Chronos LCD Special Functions, Constants, and Macros
  *
  ******************************************************************************
  */

#include "OT_platform.h"

#if (   defined(BOARD_eZ430Chronos) \
    &&  OT_FEATURE(MPIPE)           \
    &&  !defined(MPIPE_UART)        \
    &&  !defined(MPIPE_I2C)         )

// Defines section

// Display function modes
#define DISPLAY_LINE_UPDATE_FULL                (BIT0)
#define DISPLAY_LINE_UPDATE_PARTIAL             (BIT1)
#define DISPLAY_LINE_CLEAR                      (BIT2)

// Definitions for line view style
#define DISPLAY_DEFAULT_VIEW                    (0u)
#define DISPLAY_ALTERNATIVE_VIEW                (1u)

// Definitions for line access
#define LINE1                                   (1u)
#define LINE2                                   (2u)

// LCD display modes
#define SEG_OFF                                 (0u)
#define SEG_ON                                  (1u)
#define SEG_ON_BLINK_ON                         (2u)
#define SEG_ON_BLINK_OFF                        (3u)
#define SEG_OFF_BLINK_OFF                       (4u)

// 7-segment character bit assignments
#define SEG_A                   (BIT4)
#define SEG_B                   (BIT5)
#define SEG_C                   (BIT6)
#define SEG_D                   (BIT7)
#define SEG_E                   (BIT2)
#define SEG_F                   (BIT0)
#define SEG_G                   (BIT1)

// ------------------------------------------
// LCD symbols for easier access
//
// xxx_SEG_xxx          = Seven-segment character (sequence 5-4-3-2-1-0)
// xxx_SYMB_xxx         = Display symbol, e.g. "AM" for ante meridiem
// xxx_UNIT_xxx         = Display unit, e.g. "km/h" for kilometers per hour
// xxx_ICON_xxx         = Display icon, e.g. heart to indicate reception of heart rate data
// xxx_L1_xxx           = Item is part of Line1 information
// xxx_L2_xxx           = Item is part of Line2 information

// Symbols for Line1
#define LCD_SYMB_AM                             0
#define LCD_SYMB_PM                             1
#define LCD_SYMB_ARROW_UP                       2
#define LCD_SYMB_ARROW_DOWN                     3
#define LCD_SYMB_PERCENT                        4

// Symbols for Line2
#define LCD_SYMB_TOTAL                          5
#define LCD_SYMB_AVERAGE                        6
#define LCD_SYMB_MAX                            7
#define LCD_SYMB_BATTERY                        8

// Units for Line1
#define LCD_UNIT_L1_FT                          9
#define LCD_UNIT_L1_K                           10
#define LCD_UNIT_L1_M                           11
#define LCD_UNIT_L1_I                           12
#define LCD_UNIT_L1_PER_S                       13
#define LCD_UNIT_L1_PER_H                       14
#define LCD_UNIT_L1_DEGREE                      15

// Units for Line2
#define LCD_UNIT_L2_KCAL                        16
#define LCD_UNIT_L2_KM                          17
#define LCD_UNIT_L2_MI                          18

// Icons
#define LCD_ICON_HEART                          19
#define LCD_ICON_STOPWATCH                      20
#define LCD_ICON_RECORD                         21
#define LCD_ICON_ALARM                          22
#define LCD_ICON_BEEPER1                        23
#define LCD_ICON_BEEPER2                        24
#define LCD_ICON_BEEPER3                        25

// Line1 7-segments
#define LCD_SEG_L1_3                            26
#define LCD_SEG_L1_2                            27
#define LCD_SEG_L1_1                            28
#define LCD_SEG_L1_0                            29
#define LCD_SEG_L1_COL                          30
#define LCD_SEG_L1_DP1                          31
#define LCD_SEG_L1_DP0                          32

// Line2 7-segments
#define LCD_SEG_L2_5                            33
#define LCD_SEG_L2_4                            34
#define LCD_SEG_L2_3                            35
#define LCD_SEG_L2_2                            36
#define LCD_SEG_L2_1                            37
#define LCD_SEG_L2_0                            38
#define LCD_SEG_L2_COL1                         39
#define LCD_SEG_L2_COL0                         40
#define LCD_SEG_L2_DP                           41

// Line1 7-segment arrays
#define LCD_SEG_L1_3_0                          70
#define LCD_SEG_L1_2_0                          71
#define LCD_SEG_L1_1_0                          72
#define LCD_SEG_L1_3_1                          73
#define LCD_SEG_L1_3_2                          74

// Line2 7-segment arrays
#define LCD_SEG_L2_5_0                          90
#define LCD_SEG_L2_4_0                          91
#define LCD_SEG_L2_3_0                          92
#define LCD_SEG_L2_2_0                          93
#define LCD_SEG_L2_1_0                          94
#define LCD_SEG_L2_5_2                          95
#define LCD_SEG_L2_3_2                          96
#define LCD_SEG_L2_5_4                          97
#define LCD_SEG_L2_4_2                          98

// LCD controller memory map
#define LCD_MEM_1                               ((u8*)0x0A20)
#define LCD_MEM_2                               ((u8*)0x0A21)
#define LCD_MEM_3                               ((u8*)0x0A22)
#define LCD_MEM_4                               ((u8*)0x0A23)
#define LCD_MEM_5                               ((u8*)0x0A24)
#define LCD_MEM_6                               ((u8*)0x0A25)
#define LCD_MEM_7                               ((u8*)0x0A26)
#define LCD_MEM_8                               ((u8*)0x0A27)
#define LCD_MEM_9                               ((u8*)0x0A28)
#define LCD_MEM_10                              ((u8*)0x0A29)
#define LCD_MEM_11                              ((u8*)0x0A2A)
#define LCD_MEM_12                              ((u8*)0x0A2B)

// Memory assignment
#define LCD_SEG_L1_0_MEM                        (LCD_MEM_6)
#define LCD_SEG_L1_1_MEM                        (LCD_MEM_4)
#define LCD_SEG_L1_2_MEM                        (LCD_MEM_3)
#define LCD_SEG_L1_3_MEM                        (LCD_MEM_2)
#define LCD_SEG_L1_COL_MEM                      (LCD_MEM_1)
#define LCD_SEG_L1_DP1_MEM                      (LCD_MEM_1)
#define LCD_SEG_L1_DP0_MEM                      (LCD_MEM_5)
#define LCD_SEG_L2_0_MEM                        (LCD_MEM_8)
#define LCD_SEG_L2_1_MEM                        (LCD_MEM_9)
#define LCD_SEG_L2_2_MEM                        (LCD_MEM_10)
#define LCD_SEG_L2_3_MEM                        (LCD_MEM_11)
#define LCD_SEG_L2_4_MEM                        (LCD_MEM_12)
#define LCD_SEG_L2_5_MEM                        (LCD_MEM_12)
#define LCD_SEG_L2_COL1_MEM                     (LCD_MEM_1)
#define LCD_SEG_L2_COL0_MEM                     (LCD_MEM_5)
#define LCD_SEG_L2_DP_MEM                       (LCD_MEM_9)
#define LCD_SYMB_AM_MEM                         (LCD_MEM_1)
#define LCD_SYMB_PM_MEM                         (LCD_MEM_1)
#define LCD_SYMB_ARROW_UP_MEM                   (LCD_MEM_1)
#define LCD_SYMB_ARROW_DOWN_MEM                 (LCD_MEM_1)
#define LCD_SYMB_PERCENT_MEM                    (LCD_MEM_5)
#define LCD_SYMB_TOTAL_MEM                      (LCD_MEM_11)
#define LCD_SYMB_AVERAGE_MEM                    (LCD_MEM_10)
#define LCD_SYMB_MAX_MEM                        (LCD_MEM_8)
#define LCD_SYMB_BATTERY_MEM                    (LCD_MEM_7)
#define LCD_UNIT_L1_FT_MEM                      (LCD_MEM_5)
#define LCD_UNIT_L1_K_MEM                       (LCD_MEM_5)
#define LCD_UNIT_L1_M_MEM                       (LCD_MEM_7)
#define LCD_UNIT_L1_I_MEM                       (LCD_MEM_7)
#define LCD_UNIT_L1_PER_S_MEM                   (LCD_MEM_5)
#define LCD_UNIT_L1_PER_H_MEM                   (LCD_MEM_7)
#define LCD_UNIT_L1_DEGREE_MEM                  (LCD_MEM_5)
#define LCD_UNIT_L2_KCAL_MEM                    (LCD_MEM_7)
#define LCD_UNIT_L2_KM_MEM                      (LCD_MEM_7)
#define LCD_UNIT_L2_MI_MEM                      (LCD_MEM_7)
#define LCD_ICON_HEART_MEM                      (LCD_MEM_2)
#define LCD_ICON_STOPWATCH_MEM                  (LCD_MEM_3)
#define LCD_ICON_RECORD_MEM                     (LCD_MEM_1)
#define LCD_ICON_ALARM_MEM                      (LCD_MEM_4)
#define LCD_ICON_BEEPER1_MEM                    (LCD_MEM_5)
#define LCD_ICON_BEEPER2_MEM                    (LCD_MEM_6)
#define LCD_ICON_BEEPER3_MEM                    (LCD_MEM_7)

// Bit masks for write access
#define LCD_SEG_L1_0_MASK                       (BIT2 + BIT1 + BIT0 + BIT7 + BIT6 + BIT5 + BIT4)
#define LCD_SEG_L1_1_MASK                       (BIT2 + BIT1 + BIT0 + BIT7 + BIT6 + BIT5 + BIT4)
#define LCD_SEG_L1_2_MASK                       (BIT2 + BIT1 + BIT0 + BIT7 + BIT6 + BIT5 + BIT4)
#define LCD_SEG_L1_3_MASK                       (BIT2 + BIT1 + BIT0 + BIT7 + BIT6 + BIT5 + BIT4)
#define LCD_SEG_L1_COL_MASK                     (BIT5)
#define LCD_SEG_L1_DP1_MASK                     (BIT6)
#define LCD_SEG_L1_DP0_MASK                     (BIT2)
#define LCD_SEG_L2_0_MASK                       (BIT3 + BIT2 + BIT1 + BIT0 + BIT6 + BIT5 + BIT4)
#define LCD_SEG_L2_1_MASK                       (BIT3 + BIT2 + BIT1 + BIT0 + BIT6 + BIT5 + BIT4)
#define LCD_SEG_L2_2_MASK                       (BIT3 + BIT2 + BIT1 + BIT0 + BIT6 + BIT5 + BIT4)
#define LCD_SEG_L2_3_MASK                       (BIT3 + BIT2 + BIT1 + BIT0 + BIT6 + BIT5 + BIT4)
#define LCD_SEG_L2_4_MASK                       (BIT3 + BIT2 + BIT1 + BIT0 + BIT6 + BIT5 + BIT4)
#define LCD_SEG_L2_5_MASK                       (BIT7)
#define LCD_SEG_L2_COL1_MASK                    (BIT4)
#define LCD_SEG_L2_COL0_MASK                    (BIT0)
#define LCD_SEG_L2_DP_MASK                      (BIT7)
#define LCD_SYMB_AM_MASK                        (BIT1 + BIT0)
#define LCD_SYMB_PM_MASK                        (BIT0)
#define LCD_SYMB_ARROW_UP_MASK                  (BIT2)
#define LCD_SYMB_ARROW_DOWN_MASK                (BIT3)
#define LCD_SYMB_PERCENT_MASK                   (BIT4)
#define LCD_SYMB_TOTAL_MASK                     (BIT7)
#define LCD_SYMB_AVERAGE_MASK                   (BIT7)
#define LCD_SYMB_MAX_MASK                       (BIT7)
#define LCD_SYMB_BATTERY_MASK                   (BIT7)
#define LCD_UNIT_L1_FT_MASK                     (BIT5)
#define LCD_UNIT_L1_K_MASK                      (BIT6)
#define LCD_UNIT_L1_M_MASK                      (BIT1)
#define LCD_UNIT_L1_I_MASK                      (BIT0)
#define LCD_UNIT_L1_PER_S_MASK                  (BIT7)
#define LCD_UNIT_L1_PER_H_MASK                  (BIT2)
#define LCD_UNIT_L1_DEGREE_MASK                 (BIT1)
#define LCD_UNIT_L2_KCAL_MASK                   (BIT4)
#define LCD_UNIT_L2_KM_MASK                     (BIT5)
#define LCD_UNIT_L2_MI_MASK                     (BIT6)
#define LCD_ICON_HEART_MASK                     (BIT3)
#define LCD_ICON_STOPWATCH_MASK                 (BIT3)
#define LCD_ICON_RECORD_MASK                    (BIT7)
#define LCD_ICON_ALARM_MASK                     (BIT3)
#define LCD_ICON_BEEPER1_MASK                   (BIT3)
#define LCD_ICON_BEEPER2_MASK                   (BIT3)
#define LCD_ICON_BEEPER3_MASK                   (BIT3)



#define CLCD_FLAG_FULLUPDATE        (1<<10)
#define CLCD_FLAG_PARTUPDATE        (1<<9)
#define CLCD_FLAG_L1UPDATE          (1<<8)
#define CLCD_FLAG_L2UPDATE          (1<<7)
#define CLCD_FLAG_TIMEUPDATE        (1<<6)
#define CLCD_FLAG_CGRAPHUPDATE      (1<<5)
#define CLCD_FLAG_TEMPUPDATE        (1<<4)
#define CLCD_FLAG_VOLTUPDATE        (1<<3)
#define CLCD_FLAG_DATEUPDATE        (1<<2)
#define CLCD_FLAG_ALARMUPDATE       (1<<1)
#define CLCD_FLAG_ACCELUPDATE       (1<<0)
#define CLCD_FLAG_ALL               (0x07FF)


typedef void (*lcd_sub)(ot_u8, ot_u8);

typedef struct {
    ot_u16  flags;
    ot_u8   itoa_str[8];
    lcd_sub line1_fn;
    lcd_sub line2_fn;
} clcd_struct;

extern clcd_struct clcd;

extern const u8 lcd_font[];
extern const u8 *segments_lcdmem[];
extern const u8 segments_bitmask[];
extern const u8 int_to_array_conversion_table[][3];


#endif

