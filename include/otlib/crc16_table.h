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
  * @file       otlib/crc16_table.h
  * @author     JP Norair
  * @version    R100
  * @date       8 Oct 2013
  * @brief      Constants to use for a CRC computation table
  * @ingroup    CRC16
  *
  * This is a header defining a table that can optionally be used to compute a
  * CRC16 value.  This is much faster than the bitwise computation method,
  * although it is slower than using CRC in hardware (if you have it).
  *
  ******************************************************************************
  */


#ifndef __CRC16_TABLE_H
#define __CRC16_TABLE_H

//#include <otsys/types.h>
//#include <otsys/config.h>


///Reference SW bytewise impls
#define CRC16_CALCBYTE(TABLE, CHECKSUM, INPUT)  \
    do { INPUT   ^= ((ot_u8*)&CHECKSUM)[UPPER]; \
         CHECKSUM = (CHECKSUM<<8) ^ TABLE[INPUT]; } while(0)


///CRC16 Table
#define CRC_TABLE_LENGTH    256
#define CRC_TABLE_SIZE      512


/// CRC16 CCITT
#if (0)
#define CRCx00 0x0000
#define CRCx01 0x1021
#define CRCx02 0x2042
#define CRCx03 0x3063
#define CRCx04 0x4084
#define CRCx05 0x50A5
#define CRCx06 0x60C6
#define CRCx07 0x70E7
#define CRCx08 0x8108
#define CRCx09 0x9129
#define CRCx0A 0xA14A
#define CRCx0B 0xB16B
#define CRCx0C 0xC18C
#define CRCx0D 0xD1AD
#define CRCx0E 0xE1CE
#define CRCx0F 0xF1EF
#define CRCx10 0x1231
#define CRCx11 0x0210
#define CRCx12 0x3273
#define CRCx13 0x2252
#define CRCx14 0x52B5
#define CRCx15 0x4294
#define CRCx16 0x72F7
#define CRCx17 0x62D6
#define CRCx18 0x9339
#define CRCx19 0x8318
#define CRCx1A 0xB37B
#define CRCx1B 0xA35A
#define CRCx1C 0xD3BD
#define CRCx1D 0xC39C
#define CRCx1E 0xF3FF
#define CRCx1F 0xE3DE
#define CRCx20 0x2462
#define CRCx21 0x3443
#define CRCx22 0x0420
#define CRCx23 0x1401
#define CRCx24 0x64E6
#define CRCx25 0x74C7
#define CRCx26 0x44A4
#define CRCx27 0x5485
#define CRCx28 0xA56A
#define CRCx29 0xB54B
#define CRCx2A 0x8528
#define CRCx2B 0x9509
#define CRCx2C 0xE5EE
#define CRCx2D 0xF5CF
#define CRCx2E 0xC5AC
#define CRCx2F 0xD58D
#define CRCx30 0x3653
#define CRCx31 0x2672
#define CRCx32 0x1611
#define CRCx33 0x0630
#define CRCx34 0x76D7
#define CRCx35 0x66F6
#define CRCx36 0x5695
#define CRCx37 0x46B4
#define CRCx38 0xB75B
#define CRCx39 0xA77A
#define CRCx3A 0x9719
#define CRCx3B 0x8738
#define CRCx3C 0xF7DF
#define CRCx3D 0xE7FE
#define CRCx3E 0xD79D
#define CRCx3F 0xC7BC
#define CRCx40 0x48C4
#define CRCx41 0x58E5
#define CRCx42 0x6886
#define CRCx43 0x78A7
#define CRCx44 0x0840
#define CRCx45 0x1861
#define CRCx46 0x2802
#define CRCx47 0x3823
#define CRCx48 0xC9CC
#define CRCx49 0xD9ED
#define CRCx4A 0xE98E
#define CRCx4B 0xF9AF
#define CRCx4C 0x8948
#define CRCx4D 0x9969
#define CRCx4E 0xA90A
#define CRCx4F 0xB92B
#define CRCx50 0x5AF5
#define CRCx51 0x4AD4
#define CRCx52 0x7AB7
#define CRCx53 0x6A96
#define CRCx54 0x1A71
#define CRCx55 0x0A50
#define CRCx56 0x3A33
#define CRCx57 0x2A12
#define CRCx58 0xDBFD
#define CRCx59 0xCBDC
#define CRCx5A 0xFBBF
#define CRCx5B 0xEB9E
#define CRCx5C 0x9B79
#define CRCx5D 0x8B58
#define CRCx5E 0xBB3B
#define CRCx5F 0xAB1A
#define CRCx60 0x6CA6
#define CRCx61 0x7C87
#define CRCx62 0x4CE4
#define CRCx63 0x5CC5
#define CRCx64 0x2C22
#define CRCx65 0x3C03
#define CRCx66 0x0C60
#define CRCx67 0x1C41
#define CRCx68 0xEDAE
#define CRCx69 0xFD8F
#define CRCx6A 0xCDEC
#define CRCx6B 0xDDCD
#define CRCx6C 0xAD2A
#define CRCx6D 0xBD0B
#define CRCx6E 0x8D68
#define CRCx6F 0x9D49
#define CRCx70 0x7E97
#define CRCx71 0x6EB6
#define CRCx72 0x5ED5
#define CRCx73 0x4EF4
#define CRCx74 0x3E13
#define CRCx75 0x2E32
#define CRCx76 0x1E51
#define CRCx77 0x0E70
#define CRCx78 0xFF9F
#define CRCx79 0xEFBE
#define CRCx7A 0xDFDD
#define CRCx7B 0xCFFC
#define CRCx7C 0xBF1B
#define CRCx7D 0xAF3A
#define CRCx7E 0x9F59
#define CRCx7F 0x8F78
#define CRCx80 0x9188
#define CRCx81 0x81A9
#define CRCx82 0xB1CA
#define CRCx83 0xA1EB
#define CRCx84 0xD10C
#define CRCx85 0xC12D
#define CRCx86 0xF14E
#define CRCx87 0xE16F
#define CRCx88 0x1080
#define CRCx89 0x00A1
#define CRCx8A 0x30C2
#define CRCx8B 0x20E3
#define CRCx8C 0x5004
#define CRCx8D 0x4025
#define CRCx8E 0x7046
#define CRCx8F 0x6067
#define CRCx90 0x83B9
#define CRCx91 0x9398
#define CRCx92 0xA3FB
#define CRCx93 0xB3DA
#define CRCx94 0xC33D
#define CRCx95 0xD31C
#define CRCx96 0xE37F
#define CRCx97 0xF35E
#define CRCx98 0x02B1
#define CRCx99 0x1290
#define CRCx9A 0x22F3
#define CRCx9B 0x32D2
#define CRCx9C 0x4235
#define CRCx9D 0x5214
#define CRCx9E 0x6277
#define CRCx9F 0x7256
#define CRCxA0 0xB5EA
#define CRCxA1 0xA5CB
#define CRCxA2 0x95A8
#define CRCxA3 0x8589
#define CRCxA4 0xF56E
#define CRCxA5 0xE54F
#define CRCxA6 0xD52C
#define CRCxA7 0xC50D
#define CRCxA8 0x34E2
#define CRCxA9 0x24C3
#define CRCxAA 0x14A0
#define CRCxAB 0x0481
#define CRCxAC 0x7466
#define CRCxAD 0x6447
#define CRCxAE 0x5424
#define CRCxAF 0x4405
#define CRCxB0 0xA7DB
#define CRCxB1 0xB7FA
#define CRCxB2 0x8799
#define CRCxB3 0x97B8
#define CRCxB4 0xE75F
#define CRCxB5 0xF77E
#define CRCxB6 0xC71D
#define CRCxB7 0xD73C
#define CRCxB8 0x26D3
#define CRCxB9 0x36F2
#define CRCxBA 0x0691
#define CRCxBB 0x16B0
#define CRCxBC 0x6657
#define CRCxBD 0x7676
#define CRCxBE 0x4615
#define CRCxBF 0x5634
#define CRCxC0 0xD94C
#define CRCxC1 0xC96D
#define CRCxC2 0xF90E
#define CRCxC3 0xE92F
#define CRCxC4 0x99C8
#define CRCxC5 0x89E9
#define CRCxC6 0xB98A
#define CRCxC7 0xA9AB
#define CRCxC8 0x5844
#define CRCxC9 0x4865
#define CRCxCA 0x7806
#define CRCxCB 0x6827
#define CRCxCC 0x18C0
#define CRCxCD 0x08E1
#define CRCxCE 0x3882
#define CRCxCF 0x28A3
#define CRCxD0 0xCB7D
#define CRCxD1 0xDB5C
#define CRCxD2 0xEB3F
#define CRCxD3 0xFB1E
#define CRCxD4 0x8BF9
#define CRCxD5 0x9BD8
#define CRCxD6 0xABBB
#define CRCxD7 0xBB9A
#define CRCxD8 0x4A75
#define CRCxD9 0x5A54
#define CRCxDA 0x6A37
#define CRCxDB 0x7A16
#define CRCxDC 0x0AF1
#define CRCxDD 0x1AD0
#define CRCxDE 0x2AB3
#define CRCxDF 0x3A92
#define CRCxE0 0xFD2E
#define CRCxE1 0xED0F
#define CRCxE2 0xDD6C
#define CRCxE3 0xCD4D
#define CRCxE4 0xBDAA
#define CRCxE5 0xAD8B
#define CRCxE6 0x9DE8
#define CRCxE7 0x8DC9
#define CRCxE8 0x7C26
#define CRCxE9 0x6C07
#define CRCxEA 0x5C64
#define CRCxEB 0x4C45
#define CRCxEC 0x3CA2
#define CRCxED 0x2C83
#define CRCxEE 0x1CE0
#define CRCxEF 0x0CC1
#define CRCxF0 0xEF1F
#define CRCxF1 0xFF3E
#define CRCxF2 0xCF5D
#define CRCxF3 0xDF7C
#define CRCxF4 0xAF9B
#define CRCxF5 0xBFBA
#define CRCxF6 0x8FD9
#define CRCxF7 0x9FF8
#define CRCxF8 0x6E17
#define CRCxF9 0x7E36
#define CRCxFA 0x4E55
#define CRCxFB 0x5E74
#define CRCxFC 0x2E93
#define CRCxFD 0x3EB2
#define CRCxFE 0x0ED1
#define CRCxFF 0x1EF0


//CRC16
#elif (1)
#define CRCx00 0x0000
#define CRCx01 0x8005
#define CRCx02 0x800F
#define CRCx03 0x000A
#define CRCx04 0x801B
#define CRCx05 0x001E
#define CRCx06 0x0014
#define CRCx07 0x8011
#define CRCx08 0x8033
#define CRCx09 0x0036
#define CRCx0A 0x003C
#define CRCx0B 0x8039
#define CRCx0C 0x0028
#define CRCx0D 0x802D
#define CRCx0E 0x8027
#define CRCx0F 0x0022
#define CRCx10 0x8063
#define CRCx11 0x0066
#define CRCx12 0x006C
#define CRCx13 0x8069
#define CRCx14 0x0078
#define CRCx15 0x807D
#define CRCx16 0x8077
#define CRCx17 0x0072
#define CRCx18 0x0050
#define CRCx19 0x8055
#define CRCx1A 0x805F
#define CRCx1B 0x005A
#define CRCx1C 0x804B
#define CRCx1D 0x004E
#define CRCx1E 0x0044
#define CRCx1F 0x8041
#define CRCx20 0x80C3
#define CRCx21 0x00C6
#define CRCx22 0x00CC
#define CRCx23 0x80C9
#define CRCx24 0x00D8
#define CRCx25 0x80DD
#define CRCx26 0x80D7
#define CRCx27 0x00D2
#define CRCx28 0x00F0
#define CRCx29 0x80F5
#define CRCx2A 0x80FF
#define CRCx2B 0x00FA
#define CRCx2C 0x80EB
#define CRCx2D 0x00EE
#define CRCx2E 0x00E4
#define CRCx2F 0x80E1
#define CRCx30 0x00A0
#define CRCx31 0x80A5
#define CRCx32 0x80AF
#define CRCx33 0x00AA
#define CRCx34 0x80BB
#define CRCx35 0x00BE
#define CRCx36 0x00B4
#define CRCx37 0x80B1
#define CRCx38 0x8093
#define CRCx39 0x0096
#define CRCx3A 0x009C
#define CRCx3B 0x8099
#define CRCx3C 0x0088
#define CRCx3D 0x808D
#define CRCx3E 0x8087
#define CRCx3F 0x0082
#define CRCx40 0x8183
#define CRCx41 0x0186
#define CRCx42 0x018C
#define CRCx43 0x8189
#define CRCx44 0x0198
#define CRCx45 0x819D
#define CRCx46 0x8197
#define CRCx47 0x0192
#define CRCx48 0x01B0
#define CRCx49 0x81B5
#define CRCx4A 0x81BF
#define CRCx4B 0x01BA
#define CRCx4C 0x81AB
#define CRCx4D 0x01AE
#define CRCx4E 0x01A4
#define CRCx4F 0x81A1
#define CRCx50 0x01E0
#define CRCx51 0x81E5
#define CRCx52 0x81EF
#define CRCx53 0x01EA
#define CRCx54 0x81FB
#define CRCx55 0x01FE
#define CRCx56 0x01F4
#define CRCx57 0x81F1
#define CRCx58 0x81D3
#define CRCx59 0x01D6
#define CRCx5A 0x01DC
#define CRCx5B 0x81D9
#define CRCx5C 0x01C8
#define CRCx5D 0x81CD
#define CRCx5E 0x81C7
#define CRCx5F 0x01C2
#define CRCx60 0x0140
#define CRCx61 0x8145
#define CRCx62 0x814F
#define CRCx63 0x014A
#define CRCx64 0x815B
#define CRCx65 0x015E
#define CRCx66 0x0154
#define CRCx67 0x8151
#define CRCx68 0x8173
#define CRCx69 0x0176
#define CRCx6A 0x017C
#define CRCx6B 0x8179
#define CRCx6C 0x0168
#define CRCx6D 0x816D
#define CRCx6E 0x8167
#define CRCx6F 0x0162
#define CRCx70 0x8123
#define CRCx71 0x0126
#define CRCx72 0x012C
#define CRCx73 0x8129
#define CRCx74 0x0138
#define CRCx75 0x813D
#define CRCx76 0x8137
#define CRCx77 0x0132
#define CRCx78 0x0110
#define CRCx79 0x8115
#define CRCx7A 0x811F
#define CRCx7B 0x011A
#define CRCx7C 0x810B
#define CRCx7D 0x010E
#define CRCx7E 0x0104
#define CRCx7F 0x8101
#define CRCx80 0x8303
#define CRCx81 0x0306
#define CRCx82 0x030C
#define CRCx83 0x8309
#define CRCx84 0x0318
#define CRCx85 0x831D
#define CRCx86 0x8317
#define CRCx87 0x0312
#define CRCx88 0x0330
#define CRCx89 0x8335
#define CRCx8A 0x833F
#define CRCx8B 0x033A
#define CRCx8C 0x832B
#define CRCx8D 0x032E
#define CRCx8E 0x0324
#define CRCx8F 0x8321
#define CRCx90 0x0360
#define CRCx91 0x8365
#define CRCx92 0x836F
#define CRCx93 0x036A
#define CRCx94 0x837B
#define CRCx95 0x037E
#define CRCx96 0x0374
#define CRCx97 0x8371
#define CRCx98 0x8353
#define CRCx99 0x0356
#define CRCx9A 0x035C
#define CRCx9B 0x8359
#define CRCx9C 0x0348
#define CRCx9D 0x834D
#define CRCx9E 0x8347
#define CRCx9F 0x0342
#define CRCxA0 0x03C0
#define CRCxA1 0x83C5
#define CRCxA2 0x83CF
#define CRCxA3 0x03CA
#define CRCxA4 0x83DB
#define CRCxA5 0x03DE
#define CRCxA6 0x03D4
#define CRCxA7 0x83D1
#define CRCxA8 0x83F3
#define CRCxA9 0x03F6
#define CRCxAA 0x03FC
#define CRCxAB 0x83F9
#define CRCxAC 0x03E8
#define CRCxAD 0x83ED
#define CRCxAE 0x83E7
#define CRCxAF 0x03E2
#define CRCxB0 0x83A3
#define CRCxB1 0x03A6
#define CRCxB2 0x03AC
#define CRCxB3 0x83A9
#define CRCxB4 0x03B8
#define CRCxB5 0x83BD
#define CRCxB6 0x83B7
#define CRCxB7 0x03B2
#define CRCxB8 0x0390
#define CRCxB9 0x8395
#define CRCxBA 0x839F
#define CRCxBB 0x039A
#define CRCxBC 0x838B
#define CRCxBD 0x038E
#define CRCxBE 0x0384
#define CRCxBF 0x8381
#define CRCxC0 0x0280
#define CRCxC1 0x8285
#define CRCxC2 0x828F
#define CRCxC3 0x028A
#define CRCxC4 0x829B
#define CRCxC5 0x029E
#define CRCxC6 0x0294
#define CRCxC7 0x8291
#define CRCxC8 0x82B3
#define CRCxC9 0x02B6
#define CRCxCA 0x02BC
#define CRCxCB 0x82B9
#define CRCxCC 0x02A8
#define CRCxCD 0x82AD
#define CRCxCE 0x82A7
#define CRCxCF 0x02A2
#define CRCxD0 0x82E3
#define CRCxD1 0x02E6
#define CRCxD2 0x02EC
#define CRCxD3 0x82E9
#define CRCxD4 0x02F8
#define CRCxD5 0x82FD
#define CRCxD6 0x82F7
#define CRCxD7 0x02F2
#define CRCxD8 0x02D0
#define CRCxD9 0x82D5
#define CRCxDA 0x82DF
#define CRCxDB 0x02DA
#define CRCxDC 0x82CB
#define CRCxDD 0x02CE
#define CRCxDE 0x02C4
#define CRCxDF 0x82C1
#define CRCxE0 0x8243
#define CRCxE1 0x0246
#define CRCxE2 0x024C
#define CRCxE3 0x8249
#define CRCxE4 0x0258
#define CRCxE5 0x825D
#define CRCxE6 0x8257
#define CRCxE7 0x0252
#define CRCxE8 0x0270
#define CRCxE9 0x8275
#define CRCxEA 0x827F
#define CRCxEB 0x027A
#define CRCxEC 0x826B
#define CRCxED 0x026E
#define CRCxEE 0x0264
#define CRCxEF 0x8261
#define CRCxF0 0x0220
#define CRCxF1 0x8225
#define CRCxF2 0x822F
#define CRCxF3 0x022A
#define CRCxF4 0x823B
#define CRCxF5 0x023E
#define CRCxF6 0x0234
#define CRCxF7 0x8231
#define CRCxF8 0x8213
#define CRCxF9 0x0216
#define CRCxFA 0x021C
#define CRCxFB 0x8219
#define CRCxFC 0x0208
#define CRCxFD 0x820D
#define CRCxFE 0x8207
#define CRCxFF 0x0202
#endif



#endif
