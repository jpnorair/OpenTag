/* Copyright 2010-2014 JP Norair
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
  * @file       /otlib/OT_types.h
  * @author     JP Norair
  * @version    R101
  * @date       26 Aug 2014
  * @brief      Common casted types
  *
  ******************************************************************************
  */


#ifndef __OT_TYPES_H
#define __OT_TYPES_H



/** Data & Programming Constants: 
  * OpenTag uses several constants for the purpose of simplifying and 
  * elucidating the code.
  */

// Reset these if already defined.  
// True/False is defined in ot_bool type (see types section)
#   ifdef True
#       undef    True
#   endif
#   ifdef False
#       undef    False
#   endif

#   ifndef ENABLED
#       ifdef DISABLED
#           define ENABLED  (~DISABLED)
#       else
#           define ENABLED  1
#       endif
#   endif
#   ifndef DISABLED
#       define DISABLED     0
#   endif
#   if (DISABLED == ENABLED)
#       undef ENABLED
#       define ENABLED      (~DISABLED)
#   endif

#   ifndef NULL
#       define NULL 0
#   endif



/** @typedef ot_bool
  * Boolean, takes True or False
  * NOTE: even if <stdbool.h> is available, sometimes the enumerated version
  *       still results in more optimized code.  Experiment.
  */
#ifdef __STDC_VERSION__
#   if (__STDC_VERSION__ >= 199901L)
#       define __HAS_C99
#   endif
#endif
#if defined(__HAS_C99) || 1
#   include <stdbool.h>
    typedef bool    ot_bool;
#   define True     true
#   define False    false
#else
    typedef enum { False = 0, True = !False } ot_bool;
#endif



/** @typedef ot_s8, ot_u8, ot_s16, ot_u16, ot_s32, ot_u32
  * Signed and unsigned integers of explicit bit widths
  */
typedef signed char         ot_s8;
typedef unsigned char       ot_u8;
typedef signed short        ot_s16;
typedef unsigned short      ot_u16;
typedef signed long         ot_s32;
typedef unsigned long       ot_u32;


    
/** @typedef ot_int, ot_uint
  * Signed and Unsigned integers that have at least 16 bits of scope.  
  * They could be implemented as larger integers, however, if the target
  * architecture works better with a larger size (e.g. 32 bits)
  *
  * @todo optimize these conditionally based on machine specs
  */
typedef signed short        ot_int;
typedef unsigned short      ot_uint;


/** @typedef ot_long, ot_ulong
  * Signed and Unsigned integers that have at least 32 bits of scope.  
  * They could be implemented as larger integers, however, if the target
  * architecture works better with a larger size (e.g. 64 bits)
  *
  * @todo optimize these conditionally based on machine specs
  */
typedef signed long         ot_long;
typedef unsigned long       ot_ulong;



    
        
/** @typedef ot_uni16
  * A union of 2 byte (16 bit) raw data types.  Useful for datastream parsing.
  */
typedef union {
    ot_u16 ushort;
    ot_s16 sshort;
    ot_u8  ubyte[2];
    ot_s8  sbyte[2];
} ot_uni16;

/** @typedef ot_uni32
  * A union of 4 byte (32 bit) raw data types.  Useful for datastream parsing.
  */
typedef union {
    ot_u32 ulong;
    ot_s32 slong;
    ot_u16 ushort[2];
    ot_s16 sshort;
    ot_u8  ubyte[4];
    ot_s8  sbyte[4];
} ot_uni32;




/** @typedef ot_sub
  * Pure subroutine pointer that can be used nicely in manual jump tables
  * looks like: void subroutine(void)
  */
typedef void (*ot_sub)(void);


/** @typedef ot_sig
  * One-argument signal callback (classic POSIX)
  * looks like: void callback(ot_int)
  */
typedef void (*ot_sig)(ot_int);


/** @typedef ot_sig2
  * Two-argument signal callback
  * looks like: void callback(ot_int, ot_int)
  */
typedef void (*ot_sig2)(ot_int, ot_int);


/** @typedef ot_sigv
  * void-pointer-argument signal callback (endless possibilities)
  * looks like: void callback(void*)
  */
typedef void (*ot_sigv)(void*);


/** @typedef ot_sigv2
  * Double void-pointer-argument signal callback
  * looks like: void callback(void*)
  */
typedef void (*ot_sigv2)(void*, void*);






/** Binary shortcuts
  * Sometimes, like when writing bitmasks or bit registers, it is nice to use
  * binary instead of the Hex that comes with C.  So, here is the way to do it,
  * from 1 bit through 8 bit values.
  */
  
#define b0 0
#define b1 1

#define b00 0
#define b01 1
#define b10 2
#define b11 3

#define b000 0
#define b001 1
#define b010 2
#define b011 3
#define b100 4
#define b101 5
#define b110 6
#define b111 7 

#define b0000 0
#define b0001 1
#define b0010 2
#define b0011 3
#define b0100 4
#define b0101 5
#define b0110 6
#define b0111 7 
#define b1000 8
#define b1001 9
#define b1010 10
#define b1011 11
#define b1100 12
#define b1101 13
#define b1110 14
#define b1111 15 

#define b00000 0
#define b00001 1
#define b00010 2
#define b00011 3
#define b00100 4
#define b00101 5
#define b00110 6
#define b00111 7 
#define b01000 8
#define b01001 9
#define b01010 10
#define b01011 11
#define b01100 12
#define b01101 13
#define b01110 14
#define b01111 15 
#define b10000 16
#define b10001 17
#define b10010 18
#define b10011 19
#define b10100 20
#define b10101 21
#define b10110 22
#define b10111 23 
#define b11000 24
#define b11001 25
#define b11010 26
#define b11011 27
#define b11100 28
#define b11101 29
#define b11110 30
#define b11111 31

#define b000000 0
#define b000001 1
#define b000010 2
#define b000011 3
#define b000100 4
#define b000101 5
#define b000110 6
#define b000111 7 
#define b001000 8
#define b001001 9
#define b001010 10
#define b001011 11
#define b001100 12
#define b001101 13
#define b001110 14
#define b001111 15 
#define b010000 16
#define b010001 17
#define b010010 18
#define b010011 19
#define b010100 20
#define b010101 21
#define b010110 22
#define b010111 23 
#define b011000 24
#define b011001 25
#define b011010 26
#define b011011 27
#define b011100 28
#define b011101 29
#define b011110 30
#define b011111 31
#define b100000 32
#define b100001 33
#define b100010 34
#define b100011 35
#define b100100 36
#define b100101 37
#define b100110 38
#define b100111 39 
#define b101000 40
#define b101001 41
#define b101010 42
#define b101011 43
#define b101100 44
#define b101101 45
#define b101110 46
#define b101111 47 
#define b110000 48
#define b110001 49
#define b110010 50
#define b110011 51
#define b110100 52
#define b110101 53
#define b110110 54
#define b110111 55 
#define b111000 56
#define b111001 57
#define b111010 58
#define b111011 59
#define b111100 60
#define b111101 61
#define b111110 62
#define b111111 63

#define b0000000 0
#define b0000001 1
#define b0000010 2
#define b0000011 3
#define b0000100 4
#define b0000101 5
#define b0000110 6
#define b0000111 7 
#define b0001000 8
#define b0001001 9
#define b0001010 10
#define b0001011 11
#define b0001100 12
#define b0001101 13
#define b0001110 14
#define b0001111 15 
#define b0010000 16
#define b0010001 17
#define b0010010 18
#define b0010011 19
#define b0010100 20
#define b0010101 21
#define b0010110 22
#define b0010111 23 
#define b0011000 24
#define b0011001 25
#define b0011010 26
#define b0011011 27
#define b0011100 28
#define b0011101 29
#define b0011110 30
#define b0011111 31
#define b0100000 32
#define b0100001 33
#define b0100010 34
#define b0100011 35
#define b0100100 36
#define b0100101 37
#define b0100110 38
#define b0100111 39 
#define b0101000 40
#define b0101001 41
#define b0101010 42
#define b0101011 43
#define b0101100 44
#define b0101101 45
#define b0101110 46
#define b0101111 47 
#define b0110000 48
#define b0110001 49
#define b0110010 50
#define b0110011 51
#define b0110100 52
#define b0110101 53
#define b0110110 54
#define b0110111 55 
#define b0111000 56
#define b0111001 57
#define b0111010 58
#define b0111011 59
#define b0111100 60
#define b0111101 61
#define b0111110 62
#define b0111111 63
#define b1000000 64
#define b1000001 65
#define b1000010 66
#define b1000011 67
#define b1000100 68
#define b1000101 69
#define b1000110 70
#define b1000111 71 
#define b1001000 72
#define b1001001 73
#define b1001010 74
#define b1001011 75
#define b1001100 76
#define b1001101 77
#define b1001110 78
#define b1001111 79 
#define b1010000 80
#define b1010001 81
#define b1010010 82
#define b1010011 83
#define b1010100 84
#define b1010101 85
#define b1010110 86
#define b1010111 87 
#define b1011000 88
#define b1011001 89
#define b1011010 90
#define b1011011 91
#define b1011100 92
#define b1011101 93
#define b1011110 94
#define b1011111 95
#define b1100000 96
#define b1100001 97
#define b1100010 98
#define b1100011 99
#define b1100100 100
#define b1100101 101
#define b1100110 102
#define b1100111 103
#define b1101000 104
#define b1101001 105
#define b1101010 106
#define b1101011 107
#define b1101100 108
#define b1101101 109
#define b1101110 110
#define b1101111 111
#define b1110000 112
#define b1110001 113
#define b1110010 114
#define b1110011 115
#define b1110100 116
#define b1110101 117
#define b1110110 118
#define b1110111 119
#define b1111000 120
#define b1111001 121
#define b1111010 122
#define b1111011 123
#define b1111100 124
#define b1111101 125
#define b1111110 126
#define b1111111 127

#define b00000000 0
#define b00000001 1
#define b00000010 2
#define b00000011 3
#define b00000100 4
#define b00000101 5
#define b00000110 6
#define b00000111 7 
#define b00001000 8
#define b00001001 9
#define b00001010 10
#define b00001011 11
#define b00001100 12
#define b00001101 13
#define b00001110 14
#define b00001111 15 
#define b00010000 16
#define b00010001 17
#define b00010010 18
#define b00010011 19
#define b00010100 20
#define b00010101 21
#define b00010110 22
#define b00010111 23 
#define b00011000 24
#define b00011001 25
#define b00011010 26
#define b00011011 27
#define b00011100 28
#define b00011101 29
#define b00011110 30
#define b00011111 31
#define b00100000 32
#define b00100001 33
#define b00100010 34
#define b00100011 35
#define b00100100 36
#define b00100101 37
#define b00100110 38
#define b00100111 39 
#define b00101000 40
#define b00101001 41
#define b00101010 42
#define b00101011 43
#define b00101100 44
#define b00101101 45
#define b00101110 46
#define b00101111 47 
#define b00110000 48
#define b00110001 49
#define b00110010 50
#define b00110011 51
#define b00110100 52
#define b00110101 53
#define b00110110 54
#define b00110111 55 
#define b00111000 56
#define b00111001 57
#define b00111010 58
#define b00111011 59
#define b00111100 60
#define b00111101 61
#define b00111110 62
#define b00111111 63
#define b01000000 64
#define b01000001 65
#define b01000010 66
#define b01000011 67
#define b01000100 68
#define b01000101 69
#define b01000110 70
#define b01000111 71 
#define b01001000 72
#define b01001001 73
#define b01001010 74
#define b01001011 75
#define b01001100 76
#define b01001101 77
#define b01001110 78
#define b01001111 79 
#define b01010000 80
#define b01010001 81
#define b01010010 82
#define b01010011 83
#define b01010100 84
#define b01010101 85
#define b01010110 86
#define b01010111 87 
#define b01011000 88
#define b01011001 89
#define b01011010 90
#define b01011011 91
#define b01011100 92
#define b01011101 93
#define b01011110 94
#define b01011111 95
#define b01100000 96
#define b01100001 97
#define b01100010 98
#define b01100011 99
#define b01100100 100
#define b01100101 101
#define b01100110 102
#define b01100111 103
#define b01101000 104
#define b01101001 105
#define b01101010 106
#define b01101011 107
#define b01101100 108
#define b01101101 109
#define b01101110 110
#define b01101111 111
#define b01110000 112
#define b01110001 113
#define b01110010 114
#define b01110011 115
#define b01110100 116
#define b01110101 117
#define b01110110 118
#define b01110111 119
#define b01111000 120
#define b01111001 121
#define b01111010 122
#define b01111011 123
#define b01111100 124
#define b01111101 125
#define b01111110 126
#define b01111111 127
#define b10000000 128
#define b10000001 129
#define b10000010 130
#define b10000011 131
#define b10000100 132
#define b10000101 133
#define b10000110 134
#define b10000111 135
#define b10001000 136
#define b10001001 137
#define b10001010 138
#define b10001011 139
#define b10001100 140
#define b10001101 141
#define b10001110 142
#define b10001111 143
#define b10010000 144
#define b10010001 145
#define b10010010 146
#define b10010011 147
#define b10010100 148
#define b10010101 149
#define b10010110 150
#define b10010111 151
#define b10011000 152
#define b10011001 153
#define b10011010 154
#define b10011011 155
#define b10011100 156
#define b10011101 157
#define b10011110 158
#define b10011111 159
#define b10100000 160
#define b10100001 161
#define b10100010 162
#define b10100011 163
#define b10100100 164
#define b10100101 165
#define b10100110 166
#define b10100111 167
#define b10101000 168
#define b10101001 169
#define b10101010 170
#define b10101011 171
#define b10101100 172
#define b10101101 173
#define b10101110 174
#define b10101111 175
#define b10110000 176
#define b10110001 177
#define b10110010 178
#define b10110011 179
#define b10110100 180
#define b10110101 181
#define b10110110 182
#define b10110111 183
#define b10111000 184
#define b10111001 185
#define b10111010 186
#define b10111011 187
#define b10111100 188
#define b10111101 189
#define b10111110 190
#define b10111111 191
#define b11000000 192
#define b11000001 193
#define b11000010 194
#define b11000011 195
#define b11000100 196
#define b11000101 197
#define b11000110 198
#define b11000111 199
#define b11001000 200
#define b11001001 201
#define b11001010 202
#define b11001011 203
#define b11001100 204
#define b11001101 205
#define b11001110 206
#define b11001111 207
#define b11010000 208
#define b11010001 209
#define b11010010 210
#define b11010011 211
#define b11010100 212
#define b11010101 213
#define b11010110 214
#define b11010111 215
#define b11011000 216
#define b11011001 217
#define b11011010 218
#define b11011011 219
#define b11011100 220
#define b11011101 221
#define b11011110 222
#define b11011111 223
#define b11100000 224
#define b11100001 225
#define b11100010 226
#define b11100011 227
#define b11100100 228
#define b11100101 229
#define b11100110 230
#define b11100111 231
#define b11101000 232
#define b11101001 233
#define b11101010 234
#define b11101011 235
#define b11101100 236
#define b11101101 237
#define b11101110 238
#define b11101111 239
#define b11110000 240
#define b11110001 241
#define b11110010 242
#define b11110011 243
#define b11110100 244
#define b11110101 245
#define b11110110 246
#define b11110111 247
#define b11111000 248
#define b11111001 249
#define b11111010 250
#define b11111011 251
#define b11111100 252
#define b11111101 253
#define b11111110 254
#define b11111111 255



#endif 

