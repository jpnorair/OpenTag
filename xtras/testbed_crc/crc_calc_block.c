/* Copyright 2014 Haystack Technologies
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

#include <stdio.h>
#include <stdint.h>


#   ifdef __BIG_ENDIAN__
#       define UPPER    0
#       define LOWER    1
#       define B0       3
#       define B1       2
#       define B2       1
#       define B3       0
#   else
#       define UPPER    1
#       define LOWER    0
#       define B0       0
#       define B1       1
#       define B2       2
#       define B3       3
#   endif


/** @note CRCBASE, CRCPOLY
  * These make the "magic number" to use when starting up CRC, written to the
  * crc.value variable during init_crc.value.  They are not currently used,
  * having been substituted for the somewhat optimized tabular approach.  If
  * program memory becomes a problem, a bitwise CRC routine may be used in place
  * of the tabular method.
  */
#ifndef CRCBASE
#define CRCBASE    0xFFFF
#define CRCBASE_LO 0xFF
#define CRCBASE_HI 0xFF
#endif

#ifndef CRCPOLY
#define CRCPOLY    0x8005
#define CRCPOLY_LO 0x05
#define CRCPOLY_HI 0x80
#endif




// CRC16 table, used to compute CRC

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

static const uint16_t crc16_table[256] = { 
    CRCx00, CRCx01, CRCx02, CRCx03, CRCx04, CRCx05, CRCx06, CRCx07, 
    CRCx08, CRCx09, CRCx0A, CRCx0B, CRCx0C, CRCx0D, CRCx0E, CRCx0F, 
    CRCx10, CRCx11, CRCx12, CRCx13, CRCx14, CRCx15, CRCx16, CRCx17, 
    CRCx18, CRCx19, CRCx1A, CRCx1B, CRCx1C, CRCx1D, CRCx1E, CRCx1F, 
    CRCx20, CRCx21, CRCx22, CRCx23, CRCx24, CRCx25, CRCx26, CRCx27, 
    CRCx28, CRCx29, CRCx2A, CRCx2B, CRCx2C, CRCx2D, CRCx2E, CRCx2F, 
    CRCx30, CRCx31, CRCx32, CRCx33, CRCx34, CRCx35, CRCx36, CRCx37, 
    CRCx38, CRCx39, CRCx3A, CRCx3B, CRCx3C, CRCx3D, CRCx3E, CRCx3F, 
    CRCx40, CRCx41, CRCx42, CRCx43, CRCx44, CRCx45, CRCx46, CRCx47, 
    CRCx48, CRCx49, CRCx4A, CRCx4B, CRCx4C, CRCx4D, CRCx4E, CRCx4F, 
    CRCx50, CRCx51, CRCx52, CRCx53, CRCx54, CRCx55, CRCx56, CRCx57, 
    CRCx58, CRCx59, CRCx5A, CRCx5B, CRCx5C, CRCx5D, CRCx5E, CRCx5F, 
    CRCx60, CRCx61, CRCx62, CRCx63, CRCx64, CRCx65, CRCx66, CRCx67, 
    CRCx68, CRCx69, CRCx6A, CRCx6B, CRCx6C, CRCx6D, CRCx6E, CRCx6F, 
    CRCx70, CRCx71, CRCx72, CRCx73, CRCx74, CRCx75, CRCx76, CRCx77, 
    CRCx78, CRCx79, CRCx7A, CRCx7B, CRCx7C, CRCx7D, CRCx7E, CRCx7F, 
    CRCx80, CRCx81, CRCx82, CRCx83, CRCx84, CRCx85, CRCx86, CRCx87, 
    CRCx88, CRCx89, CRCx8A, CRCx8B, CRCx8C, CRCx8D, CRCx8E, CRCx8F, 
    CRCx90, CRCx91, CRCx92, CRCx93, CRCx94, CRCx95, CRCx96, CRCx97, 
    CRCx98, CRCx99, CRCx9A, CRCx9B, CRCx9C, CRCx9D, CRCx9E, CRCx9F, 
    CRCxA0, CRCxA1, CRCxA2, CRCxA3, CRCxA4, CRCxA5, CRCxA6, CRCxA7, 
    CRCxA8, CRCxA9, CRCxAA, CRCxAB, CRCxAC, CRCxAD, CRCxAE, CRCxAF, 
    CRCxB0, CRCxB1, CRCxB2, CRCxB3, CRCxB4, CRCxB5, CRCxB6, CRCxB7, 
    CRCxB8, CRCxB9, CRCxBA, CRCxBB, CRCxBC, CRCxBD, CRCxBE, CRCxBF, 
    CRCxC0, CRCxC1, CRCxC2, CRCxC3, CRCxC4, CRCxC5, CRCxC6, CRCxC7, 
    CRCxC8, CRCxC9, CRCxCA, CRCxCB, CRCxCC, CRCxCD, CRCxCE, CRCxCF, 
    CRCxD0, CRCxD1, CRCxD2, CRCxD3, CRCxD4, CRCxD5, CRCxD6, CRCxD7, 
    CRCxD8, CRCxD9, CRCxDA, CRCxDB, CRCxDC, CRCxDD, CRCxDE, CRCxDF, 
    CRCxE0, CRCxE1, CRCxE2, CRCxE3, CRCxE4, CRCxE5, CRCxE6, CRCxE7, 
    CRCxE8, CRCxE9, CRCxEA, CRCxEB, CRCxEC, CRCxED, CRCxEE, CRCxEF, 
    CRCxF0, CRCxF1, CRCxF2, CRCxF3, CRCxF4, CRCxF5, CRCxF6, CRCxF7, 
    CRCxF8, CRCxF9, CRCxFA, CRCxFB, CRCxFC, CRCxFD, CRCxFE, CRCxFF
}; 



// CRC Calculation function

uint16_t crc_calc_block(uint8_t* src, size_t size) {
    uint16_t init = CRCBASE;
    uint8_t index;
    
    while (size-- != 0) {
        index   = ((uint8_t*)&init)[UPPER] ^ *src++;
        init    = (init<<8) ^ crc16_table[index];
    }
    
    return init;
}







// Test program

int main(void) {
    const char refstring[] = "123456789";
    uint16_t crc_result;
    
    crc_result = crc_calc_block((uint8_t*)refstring, 9);
    
    printf("CRC16 reference = 0x%04X\n", crc_result);
    printf("(Should be 0xAEE7)\n");
    
    return 0;
}


