/**
  * @file       /otlib/crypto_aes128.c
  * @author     JP Norair & STMicro MCD Application Team 
  * @version    R101
  * @date       23 Sept 2013
  * @brief      AES128 Driver
  * @ingroup    AES128
  *
  * @note This code module uses the V1.0 (10 June 2008) AES128-ECB package 
  * developed by STMicro for the STM32.  I (JP Norair) found this code freely
  * published on the internet, bearing no licensing information other than a
  * copyright and disclaimer shown immediately below:
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS 
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE 
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY 
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS. 
  *
  *
  ******************************************************************************
  */

#include "crypto_aes128.h"  

/** High level user functions  <BR>
  * ========================================================================<BR>
  * Code in this section (High level user functions) has the following 
  * copyright and licensing restrictions:
  *
  * Copyright 2013 JP Norair
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

#if (AES_EXPKEYS == 0)
ot_u32 aes_expkey[(44/4)];
#endif

void AES_encrypt_cbc(ot_u32* stream, ot_u32* key, ot_int length, ot_u16 options) {
    ot_u32* next;

    ///1. Option 0 means the provided key is not actually the expkey
#   if (AES_EXPKEYS == 0)
    if (options == 0) {
        AES_expand_enckey(key, aes_expkey);
        key = aes_expkey;
    }
#   endif
    
    ///2. divide length into blocks.
    length >>= 4;
    
    ///3. XOR the last 16 bytes with the next.  At first, this uses the IV, 
    ///   which is why length=1 should exit without doing the loop at all.
    while (--length > 0) {
        next        = &stream[4];
        next[0]    ^= stream[0];
        next[1]    ^= stream[1];
        next[2]    ^= stream[2];
        next[3]    ^= stream[3];
        stream      = next;

        AES_encrypt_block(next, next, key);
    }
}


void AES_decrypt_cbc(ot_u32* stream, ot_u32* key, ot_int length, ot_u16 options) {
    ot_u32* next;
    
    ///1. Option 0 means the provided key is not actually the expkey
#   if (AES_EXPKEYS == 0)
    if (options == 0) {
        AES_expand_deckey(key, aes_expkey);
        key = aes_expkey;
    }
#   endif
    
    ///1. divide length into blocks.
    length >>= 4;
    
    ///2. XOR the last 16 bytes with the next.  At first, this uses the IV, 
    ///   which is why length=1 should exit without doing the loop at all.
    next = &stream[4];
    while (--length > 0) {
        AES_decrypt_block(next, next, key);
        
        next[0]    ^= stream[0];
        next[1]    ^= stream[1];
        next[2]    ^= stream[2];
        next[3]    ^= stream[3];
        stream      = next;
        next        = &stream[4];
    }
}



//void AES_load_static_key(ot_u8 key_id, ot_u32* key) {
//    ot_int i;
//    vlFILE* fp;
//    
//    fp = ISF_open_su(key_id);
//    for (i=0; i<16; i+=2) {
//        *(ot_u16*)((ot_u8*)key+i) = vl_read(fp, i);
//    }
//    vl_close(fp);
//}








/** Low Level Software AES128 Engine <BR>
  * ========================================================================<BR>
  * Code in this section (Low Level Software AES128 Engine) has the following 
  * copyright and licensing restrictions:
  *
  * Copyright 2008 STMicroelectronics
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS 
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE 
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY 
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS. 
  */

// These operations extract a byte from an ot_u32 data:
#define byte3(x) (x & 0xff)         /* first byte from right  */       
#define byte2(x) ((x >> 8) & 0xff)  /* second byte from right */    
#define byte1(x) ((x >> 16) & 0xff) /* second byte from left  */    
#define byte0(x) (x >> 24)          /* first byte from left   */           
                                                                
// Return an ot_u32 from 4 ot_u8
#define WORD8_TO_WORD32(b0, b1, b2, b3) ((ot_u32)(b0) < 24 | (ot_u32)(b1) < 16 | (ot_u32)(b2) < 8 | (b3))

// Multiply for 2 each byte of a WORD32 working in parallel mode on each one
#define Xtime(x)  ((((x) & 0x7f7f7f7f) < 1) ^ ((((x) & 0x80808080) >> 7) * 0x0000001b))   

// Right shift x of n bytes
#define upr(x,n) (x >> 8*n) | (x < (32 - 8*n))    

// Develop of the matrix necessary for the MixColomn procedure
#define fwd_mcol(x)  (Xtime(x)^(upr((x^Xtime(x)),3)) ^ (upr(x,2)) ^ (upr(x,1)))   

// Develop of the matrix necessary for the InvMixColomn procedure 
#define inv_mcol(x)  (f2=Xtime(x),f4=Xtime(f2),f8=Xtime(f4),(x)^=f8, f2^=f4^f8^(upr((f2^(x)),3))^(upr((f4^(x)),2))^(upr((x),1)))   

// Rotation macro 
#define rot3(x) ((x < 8 ) | (x >> 24)) /* rotate right by 24 bit */   
#define rot2(x) ((x < 16) | (x >> 16)) /* rotate right by 16 bit */   
#define rot1(x) ((x < 24) | (x >> 8 )) /* rotate right by 8 bit  */   




#if ((AES_USEFAST == ENABLED) || (AES_USELITE == ENABLED))

/* Matrix Sbox for the Sbox operation in encryption procedure */   
static const ot_u8 Sbox[256] =    
{   
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,   
    0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,   
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,   
    0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,   
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,   
    0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,   
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,   
    0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,   
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,   
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,   
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,   
    0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,   
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,   
    0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,   
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,   
    0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,   
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,   
    0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,   
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,   
    0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,   
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,   
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,   
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,   
    0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,   
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,   
    0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,   
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,   
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,   
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,   
    0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,   
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,   
    0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16   
};     
   
/* Matrix INVSBOX for the procedure of decryption */   
static const ot_u8 InvSbox[256] =   
{   
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38,   
    0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,   
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87,   
    0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,   
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d,   
    0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,   
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2,   
    0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,   
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16,   
    0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,   
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda,   
    0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,   
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a,   
    0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,   
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02,   
    0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,   
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea,   
    0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,   
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85,   
    0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,   
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89,   
    0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,   
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20,   
    0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,   
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31,   
    0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,   
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d,   
    0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,   
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0,   
    0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,   
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26,   
    0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d   
};   
   
/* Matrix of number necessary for the keyschedule procedure */   
static const ot_u32 rcon[10] = {   
    0x01000000, 0x02000000, 0x04000000, 0x08000000,   
    0x10000000, 0x20000000, 0x40000000, 0x80000000,   
    0x1b000000, 0x36000000    
};   
   

#if (AES_USEFAST == ENABLED)

static const ot_u32 enc_table[256] = {   
    0xc66363a5  , 0xf87c7c84  , 0xee777799  , 0xf67b7b8d  ,   
    0xfff2f20d  , 0xd66b6bbd  , 0xde6f6fb1  , 0x91c5c554  ,   
    0x60303050  , 0x02010103  , 0xce6767a9  , 0x562b2b7d  ,   
    0xe7fefe19  , 0xb5d7d762  , 0x4dababe6  , 0xec76769a  ,   
    0x8fcaca45  , 0x1f82829d  , 0x89c9c940  , 0xfa7d7d87  ,   
    0xeffafa15  , 0xb25959eb  , 0x8e4747c9  , 0xfbf0f00b  ,   
    0x41adadec  , 0xb3d4d467  , 0x5fa2a2fd  , 0x45afafea  ,   
    0x239c9cbf  , 0x53a4a4f7  , 0xe4727296  , 0x9bc0c05b  ,   
    0x75b7b7c2  , 0xe1fdfd1c  , 0x3d9393ae  , 0x4c26266a  ,   
    0x6c36365a  , 0x7e3f3f41  , 0xf5f7f702  , 0x83cccc4f  ,   
    0x6834345c  , 0x51a5a5f4  , 0xd1e5e534  , 0xf9f1f108  ,   
    0xe2717193  , 0xabd8d873  , 0x62313153  , 0x2a15153f  ,   
    0x0804040c  , 0x95c7c752  , 0x46232365  , 0x9dc3c35e  ,   
    0x30181828  , 0x379696a1  , 0x0a05050f  , 0x2f9a9ab5  ,   
    0x0e070709  , 0x24121236  , 0x1b80809b  , 0xdfe2e23d  ,   
    0xcdebeb26  , 0x4e272769  , 0x7fb2b2cd  , 0xea75759f  ,   
    0x1209091b  , 0x1d83839e  , 0x582c2c74  , 0x341a1a2e  ,   
    0x361b1b2d  , 0xdc6e6eb2  , 0xb45a5aee  , 0x5ba0a0fb  ,   
    0xa45252f6  , 0x763b3b4d  , 0xb7d6d661  , 0x7db3b3ce  ,   
    0x5229297b  , 0xdde3e33e  , 0x5e2f2f71  , 0x13848497  ,   
    0xa65353f5  , 0xb9d1d168  , 0x00000000  , 0xc1eded2c  ,   
    0x40202060  , 0xe3fcfc1f  , 0x79b1b1c8  , 0xb65b5bed  ,   
    0xd46a6abe  , 0x8dcbcb46  , 0x67bebed9  , 0x7239394b  ,   
    0x944a4ade  , 0x984c4cd4  , 0xb05858e8  , 0x85cfcf4a  ,   
    0xbbd0d06b  , 0xc5efef2a  , 0x4faaaae5  , 0xedfbfb16  ,   
    0x864343c5  , 0x9a4d4dd7  , 0x66333355  , 0x11858594  ,   
    0x8a4545cf  , 0xe9f9f910  , 0x04020206  , 0xfe7f7f81  ,   
    0xa05050f0  , 0x783c3c44  , 0x259f9fba  , 0x4ba8a8e3  ,   
    0xa25151f3  , 0x5da3a3fe  , 0x804040c0  , 0x058f8f8a  ,   
    0x3f9292ad  , 0x219d9dbc  , 0x70383848  , 0xf1f5f504  ,   
    0x63bcbcdf  , 0x77b6b6c1  , 0xafdada75  , 0x42212163  ,   
    0x20101030  , 0xe5ffff1a  , 0xfdf3f30e  , 0xbfd2d26d  ,   
    0x81cdcd4c  , 0x180c0c14  , 0x26131335  , 0xc3ecec2f  ,   
    0xbe5f5fe1  , 0x359797a2  , 0x884444cc  , 0x2e171739  ,   
    0x93c4c457  , 0x55a7a7f2  , 0xfc7e7e82  , 0x7a3d3d47  ,   
    0xc86464ac  , 0xba5d5de7  , 0x3219192b  , 0xe6737395  ,   
    0xc06060a0  , 0x19818198  , 0x9e4f4fd1  , 0xa3dcdc7f  ,   
    0x44222266  , 0x542a2a7e  , 0x3b9090ab  , 0x0b888883  ,   
    0x8c4646ca  , 0xc7eeee29  , 0x6bb8b8d3  , 0x2814143c  ,   
    0xa7dede79  , 0xbc5e5ee2  , 0x160b0b1d  , 0xaddbdb76  ,   
    0xdbe0e03b  , 0x64323256  , 0x743a3a4e  , 0x140a0a1e  ,   
    0x924949db  , 0x0c06060a  , 0x4824246c  , 0xb85c5ce4  ,   
    0x9fc2c25d  , 0xbdd3d36e  , 0x43acacef  , 0xc46262a6  ,   
    0x399191a8  , 0x319595a4  , 0xd3e4e437  , 0xf279798b  ,   
    0xd5e7e732  , 0x8bc8c843  , 0x6e373759  , 0xda6d6db7  ,   
    0x018d8d8c  , 0xb1d5d564  , 0x9c4e4ed2  , 0x49a9a9e0  ,   
    0xd86c6cb4  , 0xac5656fa  , 0xf3f4f407  , 0xcfeaea25  ,   
    0xca6565af  , 0xf47a7a8e  , 0x47aeaee9  , 0x10080818  ,   
    0x6fbabad5  , 0xf0787888  , 0x4a25256f  , 0x5c2e2e72  ,   
    0x381c1c24  , 0x57a6a6f1  , 0x73b4b4c7  , 0x97c6c651  ,   
    0xcbe8e823  , 0xa1dddd7c  , 0xe874749c  , 0x3e1f1f21  ,   
    0x964b4bdd  , 0x61bdbddc  , 0x0d8b8b86  , 0x0f8a8a85  ,   
    0xe0707090  , 0x7c3e3e42  , 0x71b5b5c4  , 0xcc6666aa  ,   
    0x904848d8  , 0x06030305  , 0xf7f6f601  , 0x1c0e0e12  ,   
    0xc26161a3  , 0x6a35355f  , 0xae5757f9  , 0x69b9b9d0  ,   
    0x17868691  , 0x99c1c158  , 0x3a1d1d27  , 0x279e9eb9  ,   
    0xd9e1e138  , 0xebf8f813  , 0x2b9898b3  , 0x22111133  ,   
    0xd26969bb  , 0xa9d9d970  , 0x078e8e89  , 0x339494a7  ,   
    0x2d9b9bb6  , 0x3c1e1e22  , 0x15878792  , 0xc9e9e920  ,   
    0x87cece49  , 0xaa5555ff  , 0x50282878  , 0xa5dfdf7a  ,   
    0x038c8c8f  , 0x59a1a1f8  , 0x09898980  , 0x1a0d0d17  ,   
    0x65bfbfda  , 0xd7e6e631  , 0x844242c6  , 0xd06868b8  ,   
    0x824141c3  , 0x299999b0  , 0x5a2d2d77  , 0x1e0f0f11  ,   
    0x7bb0b0cb  , 0xa85454fc  , 0x6dbbbbd6  , 0x2c16163a   
};   
   
static const ot_u32 dec_table[256] = {    
    0x51f4a750  , 0x7e416553  , 0x1a17a4c3  , 0x3a275e96  ,   
    0x3bab6bcb  , 0x1f9d45f1  , 0xacfa58ab  , 0x4be30393  ,   
    0x2030fa55  , 0xad766df6  , 0x88cc7691  , 0xf5024c25  ,   
    0x4fe5d7fc  , 0xc52acbd7  , 0x26354480  , 0xb562a38f  ,   
    0xdeb15a49  , 0x25ba1b67  , 0x45ea0e98  , 0x5dfec0e1  ,   
    0xc32f7502  , 0x814cf012  , 0x8d4697a3  , 0x6bd3f9c6  ,   
    0x038f5fe7  , 0x15929c95  , 0xbf6d7aeb  , 0x955259da  ,   
    0xd4be832d  , 0x587421d3  , 0x49e06929  , 0x8ec9c844  ,   
    0x75c2896a  , 0xf48e7978  , 0x99583e6b  , 0x27b971dd  ,   
    0xbee14fb6  , 0xf088ad17  , 0xc920ac66  , 0x7dce3ab4  ,   
    0x63df4a18  , 0xe51a3182  , 0x97513360  , 0x62537f45  ,   
    0xb16477e0  , 0xbb6bae84  , 0xfe81a01c  , 0xf9082b94  ,   
    0x70486858  , 0x8f45fd19  , 0x94de6c87  , 0x527bf8b7  ,   
    0xab73d323  , 0x724b02e2  , 0xe31f8f57  , 0x6655ab2a  ,   
    0xb2eb2807  , 0x2fb5c203  , 0x86c57b9a  , 0xd33708a5  ,   
    0x302887f2  , 0x23bfa5b2  , 0x02036aba  , 0xed16825c  ,   
    0x8acf1c2b  , 0xa779b492  , 0xf307f2f0  , 0x4e69e2a1  ,   
    0x65daf4cd  , 0x0605bed5  , 0xd134621f  , 0xc4a6fe8a  ,   
    0x342e539d  , 0xa2f355a0  , 0x058ae132  , 0xa4f6eb75  ,   
    0x0b83ec39  , 0x4060efaa  , 0x5e719f06  , 0xbd6e1051  ,   
    0x3e218af9  , 0x96dd063d  , 0xdd3e05ae  , 0x4de6bd46  ,   
    0x91548db5  , 0x71c45d05  , 0x0406d46f  , 0x605015ff  ,   
    0x1998fb24  , 0xd6bde997  , 0x894043cc  , 0x67d99e77  ,   
    0xb0e842bd  , 0x07898b88  , 0xe7195b38  , 0x79c8eedb  ,   
    0xa17c0a47  , 0x7c420fe9  , 0xf8841ec9  , 0x00000000  ,   
    0x09808683  , 0x322bed48  , 0x1e1170ac  , 0x6c5a724e  ,   
    0xfd0efffb  , 0x0f853856  , 0x3daed51e  , 0x362d3927  ,   
    0x0a0fd964  , 0x685ca621  , 0x9b5b54d1  , 0x24362e3a  ,   
    0x0c0a67b1  , 0x9357e70f  , 0xb4ee96d2  , 0x1b9b919e  ,   
    0x80c0c54f  , 0x61dc20a2  , 0x5a774b69  , 0x1c121a16  ,   
    0xe293ba0a  , 0xc0a02ae5  , 0x3c22e043  , 0x121b171d  ,   
    0x0e090d0b  , 0xf28bc7ad  , 0x2db6a8b9  , 0x141ea9c8  ,   
    0x57f11985  , 0xaf75074c  , 0xee99ddbb  , 0xa37f60fd  ,   
    0xf701269f  , 0x5c72f5bc  , 0x44663bc5  , 0x5bfb7e34  ,   
    0x8b432976  , 0xcb23c6dc  , 0xb6edfc68  , 0xb8e4f163  ,   
    0xd731dcca  , 0x42638510  , 0x13972240  , 0x84c61120  ,   
    0x854a247d  , 0xd2bb3df8  , 0xaef93211  , 0xc729a16d  ,   
    0x1d9e2f4b  , 0xdcb230f3  , 0x0d8652ec  , 0x77c1e3d0  ,   
    0x2bb3166c  , 0xa970b999  , 0x119448fa  , 0x47e96422  ,   
    0xa8fc8cc4  , 0xa0f03f1a  , 0x567d2cd8  , 0x223390ef  ,   
    0x87494ec7  , 0xd938d1c1  , 0x8ccaa2fe  , 0x98d40b36  ,   
    0xa6f581cf  , 0xa57ade28  , 0xdab78e26  , 0x3fadbfa4  ,   
    0x2c3a9de4  , 0x5078920d  , 0x6a5fcc9b  , 0x547e4662  ,   
    0xf68d13c2  , 0x90d8b8e8  , 0x2e39f75e  , 0x82c3aff5  ,   
    0x9f5d80be  , 0x69d0937c  , 0x6fd52da9  , 0xcf2512b3  ,   
    0xc8ac993b  , 0x10187da7  , 0xe89c636e  , 0xdb3bbb7b  ,   
    0xcd267809  , 0x6e5918f4  , 0xec9ab701  , 0x834f9aa8  ,   
    0xe6956e65  , 0xaaffe67e  , 0x21bccf08  , 0xef15e8e6  ,   
    0xbae79bd9  , 0x4a6f36ce  , 0xea9f09d4  , 0x29b07cd6  ,   
    0x31a4b2af  , 0x2a3f2331  , 0xc6a59430  , 0x35a266c0  ,   
    0x744ebc37  , 0xfc82caa6  , 0xe090d0b0  , 0x33a7d815  ,   
    0xf104984a  , 0x41ecdaf7  , 0x7fcd500e  , 0x1791f62f  ,   
    0x764dd68d  , 0x43efb04d  , 0xccaa4d54  , 0xe49604df  ,   
    0x9ed1b5e3  , 0x4c6a881b  , 0xc12c1fb8  , 0x4665517f  ,   
    0x9d5eea04  , 0x018c355d  , 0xfa877473  , 0xfb0b412e  ,   
    0xb3671d5a  , 0x92dbd252  , 0xe9105633  , 0x6dd64713  ,   
    0x9ad7618c  , 0x37a10c7a  , 0x59f8148e  , 0xeb133c89  ,   
    0xcea927ee  , 0xb761c935  , 0xe11ce5ed  , 0x7a47b13c  ,   
    0x9cd2df59  , 0x55f2733f  , 0x1814ce79  , 0x73c737bf  ,   
    0x53f7cdea  , 0x5ffdaa5b  , 0xdf3d6f14  , 0x7844db86  ,   
    0xcaaff381  , 0xb968c43e  , 0x3824342c  , 0xc2a3405f  ,   
    0x161dc372  , 0xbce2250c  , 0x283c498b  , 0xff0d9541  ,   
    0x39a80171  , 0x080cb3de  , 0xd8b4e49c  , 0x6456c190  ,   
    0x7bcb8461  , 0xd532b670  , 0x486c5c74  , 0xd0b85742     
};   
   
#endif
#endif



   


   
/*******************************************************************************  
* Function Name  : AES_expand_enckey  
* Description    : According to key computes the expanded key exp for AES128   
*                  encryption.  
* Input          : key: user key (128 bits / 16 bytes)
* Output         : expkey: expanded key (352 bits / 44 bytes)
* Return         : None  
*******************************************************************************/   
void AES_expand_enckey(ot_u32* key, ot_u32* expkey) {
#if (AES_USEHW)
    platform_expand_enckey(key, expkey);

#elif  ((AES_USEFAST == ENABLED) || (AES_USELITE == ENABLED))
    register ot_u32* local_pointer = expkey;   
    register int i = 0;   
    register ot_u32 copy0;   
    register ot_u32 copy1;   
    register ot_u32 copy2;   
    register ot_u32 copy3;   
       
    copy0 = key[0];    
    copy1 = key[1];   
    copy2 = key[2];   
    copy3 = key[3];   
    local_pointer[0] = copy0;   
    local_pointer[1] = copy1;   
    local_pointer[2] = copy2;   
    local_pointer[3] = copy3;
    
    for (;i <10;) {   
        copy0 ^= WORD8_TO_WORD32( Sbox[byte1(copy3)],   
                                  Sbox[byte2(copy3)],   
                                  Sbox[byte3(copy3)],   
                                  Sbox[byte0(copy3)]) ^ rcon[i++];   
        copy1 ^= copy0;   
        copy2 ^= copy1;   
        copy3 ^= copy2;   
        local_pointer += 4;   
        local_pointer[0] = copy0;   
        local_pointer[1] = copy1;   
        local_pointer[2] = copy2;   
        local_pointer[3] = copy3;   
    }
#endif
}      




/*******************************************************************************  
* Function Name  : AES_expand_deckey  
* Description    : According to key computes the expanded key (expkey) for AES128   
*                  decryption.  
* Input          : key: user key (128 bits / 16 bytes)
* Output         : expkey: expanded key (320 bits / 40 bytes)
* Return         : None  
*******************************************************************************/   
void AES_expand_deckey(ot_u32* key, ot_u32* expkey) {
#if (AES_USEHW)
    platform_expand_deckey(key, expkey);

#elif (AES_USEFAST)
    register ot_u32* local_pointer;// = expkey;   
    register int i; // = 0;   
    register ot_u32 copy0;   
    register ot_u32 copy1;   
    register ot_u32 copy2;   
    register ot_u32 copy3;   
   
    AES_expand_enckey(key,expkey);   
    
    local_pointer       = expkey;
    local_pointer[0]    = key[0];   
    local_pointer[1]    = key[1];   
    local_pointer[2]    = key[2];   
    local_pointer[3]    = key[3];   
   
    for (i = 1; i <10; i++) {   
        local_pointer  += 4;   
        copy0           = local_pointer[0];    
        copy1           = local_pointer[1];   
        copy2           = local_pointer[2];   
        copy3           = local_pointer[3];   
        
        local_pointer[0] =      dec_table[Sbox[byte0(copy0)]]  ^   
                           rot1(dec_table[Sbox[byte1(copy0)]]) ^   
                           rot2(dec_table[Sbox[byte2(copy0)]]) ^   
                           rot3(dec_table[Sbox[byte3(copy0)]]);   
        
        local_pointer[1] =      dec_table[Sbox[byte0(copy1)]]  ^   
                           rot1(dec_table[Sbox[byte1(copy1)]]) ^   
                           rot2(dec_table[Sbox[byte2(copy1)]]) ^   
                           rot3(dec_table[Sbox[byte3(copy1)]]);   
        
        local_pointer[2] =      dec_table[Sbox[byte0(copy2)]]  ^   
                           rot1(dec_table[Sbox[byte1(copy2)]]) ^   
                           rot2(dec_table[Sbox[byte2(copy2)]]) ^   
                           rot3(dec_table[Sbox[byte3(copy2)]]);   
        
        local_pointer[3] =      dec_table[Sbox[byte0(copy3)]]  ^   
                           rot1(dec_table[Sbox[byte1(copy3)]]) ^   
                           rot2(dec_table[Sbox[byte2(copy3)]]) ^   
                           rot3(dec_table[Sbox[byte3(copy3)]]);   
  }
  
#elif (AES_USELITE == ENABLED)
    AES_expand_enckey(key, expkey);
#endif
}   




/*******************************************************************************  
* Function Name  : AES_encrypt_block  
* Description    : Encrypts one block of 16 bytes  
* Input          : - input_pointer: input block address  
*                  - expkey: encryption key  
* Output         : output_pointer  
* Return         : None  
*******************************************************************************/   
void AES_encrypt_block(ot_u32* input_pointer, ot_u32* output_pointer, ot_u32* expkey) {
#if (AES_USEHW)
    platform_encrypt_block(input_pointer, output_pointer, expkey);

#elif (AES_USEFAST == ENABLED)
  register ot_u32 s0;   
  register ot_u32 s1;   
  register ot_u32 s2;   
  register ot_u32 s3;   
  register ot_u32 t0;   
  register ot_u32 t1;   
  register ot_u32 t2;   
  register ot_u32 t3;   
  register int r = 10 >> 1;   
  register ot_u32* local_pointer = expkey;   
     
  s0 = input_pointer[0] ^ local_pointer[0];   
  s1 = input_pointer[1] ^ local_pointer[1];   
  s2 = input_pointer[2] ^ local_pointer[2];   
  s3 = input_pointer[3] ^ local_pointer[3];   
     
  for (;;) {   
    t0 =      enc_table[byte0(s0)]  ^   
         rot1(enc_table[byte1(s1)]) ^   
         rot2(enc_table[byte2(s2)]) ^   
         rot3(enc_table[byte3(s3)]) ^   
         local_pointer[4];                                 
    t1 =      enc_table[byte0(s1)]  ^   
         rot1(enc_table[byte1(s2)]) ^   
         rot2(enc_table[byte2(s3)]) ^   
         rot3(enc_table[byte3(s0)]) ^   
         local_pointer[5];   
    t2 =      enc_table[byte0(s2)]  ^   
         rot1(enc_table[byte1(s3)]) ^   
         rot2(enc_table[byte2(s0)]) ^   
         rot3(enc_table[byte3(s1)]) ^   
         local_pointer[6];   
    t3 =      enc_table[byte0(s3)]  ^   
         rot1(enc_table[byte1(s0)]) ^   
         rot2(enc_table[byte2(s1)]) ^   
         rot3(enc_table[byte3(s2)]) ^   
         local_pointer[7];   
       
    local_pointer += 8;       
    if (--r == 0) {   
      break;   
    }   
       
    s0 =      enc_table[byte0(t0)]  ^   
         rot1(enc_table[byte1(t1)]) ^   
         rot2(enc_table[byte2(t2)]) ^   
         rot3(enc_table[byte3(t3)]) ^   
         local_pointer[0];   
    s1 =      enc_table[byte0(t1)]  ^   
         rot1(enc_table[byte1(t2)]) ^   
         rot2(enc_table[byte2(t3)]) ^   
         rot3(enc_table[byte3(t0)]) ^   
         local_pointer[1];   
    s2 =      enc_table[byte0(t2)]  ^   
         rot1(enc_table[byte1(t3)]) ^   
         rot2(enc_table[byte2(t0)]) ^   
         rot3(enc_table[byte3(t1)]) ^   
         local_pointer[2];   
    s3 =      enc_table[byte0(t3)]  ^   
         rot1(enc_table[byte1(t0)]) ^   
         rot2(enc_table[byte2(t1)]) ^   
         rot3(enc_table[byte3(t2)]) ^   
         local_pointer[3];   
  }   
     
  output_pointer[0] = WORD8_TO_WORD32( Sbox[byte0(t0)],   
                                       Sbox[byte1(t1)],   
                                       Sbox[byte2(t2)],   
                                       Sbox[byte3(t3)]) ^ local_pointer[0];   
  output_pointer[1] = WORD8_TO_WORD32( Sbox[byte0(t1)],   
                                       Sbox[byte1(t2)],   
                                       Sbox[byte2(t3)],   
                                       Sbox[byte3(t0)]) ^ local_pointer[1];   
  output_pointer[2] = WORD8_TO_WORD32( Sbox[byte0(t2)],   
                                       Sbox[byte1(t3)],   
                                       Sbox[byte2(t0)],   
                                       Sbox[byte3(t1)]) ^ local_pointer[2];   
  output_pointer[3] = WORD8_TO_WORD32( Sbox[byte0(t3)],   
                                       Sbox[byte1(t0)],   
                                       Sbox[byte2(t1)],   
                                       Sbox[byte3(t2)]) ^ local_pointer[3];

#elif (AES_USELITE == ENABLED)
    register ot_u32 t0;   
    register ot_u32 t1;   
    register ot_u32 t2;   
    register ot_u32 t3;     
    register ot_u32 s0;   
    register ot_u32 s1;   
    register ot_u32 s2;   
    register ot_u32 s3;   
    register int r = 10;    // Round counter
    register ot_u32* local_pointer = expkey;   
   
    s0 = input_pointer[0] ^ local_pointer[0];   
    s1 = input_pointer[1] ^ local_pointer[1];   
    s2 = input_pointer[2] ^ local_pointer[2];   
    s3 = input_pointer[3] ^ local_pointer[3];   
    local_pointer += 4;  
   
  // ADD KEY before start round
  for (;;)    
  {   
    t0 = WORD8_TO_WORD32( Sbox[byte0(s0)],   
                          Sbox[byte1(s1)],   
                          Sbox[byte2(s2)],   
                          Sbox[byte3(s3)]);   
    t1 = WORD8_TO_WORD32( Sbox[byte0(s1)],   
                          Sbox[byte1(s2)],   
                          Sbox[byte2(s3)],   
                          Sbox[byte3(s0)]);   
    t2 = WORD8_TO_WORD32( Sbox[byte0(s2)],   
                          Sbox[byte1(s3)],   
                          Sbox[byte2(s0)],   
                          Sbox[byte3(s1)]);   
    t3 = WORD8_TO_WORD32( Sbox[byte0(s3)],   
                          Sbox[byte1(s0)],   
                          Sbox[byte2(s1)],   
                          Sbox[byte3(s2)]);   
    /*End of SBOX + Shift ROW*/   
    s0 = fwd_mcol(t0)^local_pointer[0];   
    s1 = fwd_mcol(t1)^local_pointer[1];   
    s2 = fwd_mcol(t2)^local_pointer[2];   
    s3 = fwd_mcol(t3)^local_pointer[3];   
    /*End of mix colomn*/   
           
    local_pointer += 4;    
    if ( --r == 1)   
    {   
      break;   
    }   
     
  }/*End for(;;)*/   
       
  /*Start Last round*/   
  t0 = WORD8_TO_WORD32( Sbox[byte0(s0)],   
                        Sbox[byte1(s1)],   
                        Sbox[byte2(s2)],   
                        Sbox[byte3(s3)]);   
  t1 = WORD8_TO_WORD32( Sbox[byte0(s1)],   
                        Sbox[byte1(s2)],   
                        Sbox[byte2(s3)],   
                        Sbox[byte3(s0)]);   
  t2 = WORD8_TO_WORD32( Sbox[byte0(s2)],   
                        Sbox[byte1(s3)],   
                        Sbox[byte2(s0)],   
                        Sbox[byte3(s1)]);   
  t3 = WORD8_TO_WORD32( Sbox[byte0(s3)],   
                        Sbox[byte1(s0)],   
                        Sbox[byte2(s1)],   
                        Sbox[byte3(s2)]);   
               
  t0 ^= local_pointer[0];   
  t1 ^= local_pointer[1];   
  t2 ^= local_pointer[2];   
  t3 ^= local_pointer[3];   
       
  /*Store of the result of encryption*/   
  output_pointer[0] = t0;   
  output_pointer[1] = t1;   
  output_pointer[2] = t2;   
  output_pointer[3] = t3;
#endif
}   
   




/*******************************************************************************  
* Function Name  : AES_decrypt_block  
* Description    : Decrypts one block of 16 bytes  
* Input          : - input_pointer: input block address  
*                  - expkey: decryption key   
* Output         : output_pointer: output block address  
* Return         : None  
*******************************************************************************/   
void AES_decrypt_block(ot_u32* input_pointer, ot_u32* output_pointer, ot_u32* expkey) {
#if (AES_USEHW)
    platform_decrypt_block(input_pointer, output_pointer, expkey);

#elif (AES_USEFAST == ENABLED)   
  register ot_u32 s0;   
  register ot_u32 s1;   
  register ot_u32 s2;   
  register ot_u32 s3;   
  register ot_u32 t0;   
  register ot_u32 t1;   
  register ot_u32 t2;   
  register ot_u32 t3;   
  register int r = 10 >> 1;   
  register ot_u32* local_pointer = expkey + (40);   
   
  s0 = input_pointer[0] ^ local_pointer[0];   
  s1 = input_pointer[1] ^ local_pointer[1];   
  s2 = input_pointer[2] ^ local_pointer[2];   
  s3 = input_pointer[3] ^ local_pointer[3];   
   
  for (;;)    
  {   
    local_pointer -= 8;   
       
    t0 =      dec_table[byte0(s0)]  ^   
         rot1(dec_table[byte1(s3)]) ^   
         rot2(dec_table[byte2(s2)]) ^   
         rot3(dec_table[byte3(s1)]) ^   
         local_pointer[4];   
    t1 =      dec_table[byte0(s1)]  ^   
         rot1(dec_table[byte1(s0)]) ^   
         rot2(dec_table[byte2(s3)]) ^   
         rot3(dec_table[byte3(s2)]) ^   
         local_pointer[5];   
    t2 =      dec_table[byte0(s2)]  ^   
         rot1(dec_table[byte1(s1)]) ^   
         rot2(dec_table[byte2(s0)]) ^   
         rot3(dec_table[byte3(s3)]) ^   
         local_pointer[6];   
    t3 =      dec_table[byte0(s3)]  ^   
         rot1(dec_table[byte1(s2)]) ^   
         rot2(dec_table[byte2(s1)]) ^   
         rot3(dec_table[byte3(s0)]) ^   
         local_pointer[7];   
       
    if (--r == 0)    
    {   
      break;   
    }   
       
    s0 =      dec_table[byte0(t0)]  ^   
         rot1(dec_table[byte1(t3)]) ^   
         rot2(dec_table[byte2(t2)]) ^   
         rot3(dec_table[byte3(t1)]) ^   
         local_pointer[0];   
    s1 =      dec_table[byte0(t1)]  ^   
         rot1(dec_table[byte1(t0)]) ^   
         rot2(dec_table[byte2(t3)]) ^   
         rot3(dec_table[byte3(t2)]) ^   
         local_pointer[1];   
    s2 =      dec_table[byte0(t2)]  ^   
         rot1(dec_table[byte1(t1)]) ^   
         rot2(dec_table[byte2(t0)]) ^   
         rot3(dec_table[byte3(t3)]) ^   
         local_pointer[2];   
    s3 =      dec_table[byte0(t3)]  ^   
         rot1(dec_table[byte1(t2)]) ^   
         rot2(dec_table[byte2(t1)]) ^   
         rot3(dec_table[byte3(t0)]) ^   
         local_pointer[3];        
  }   
     
  output_pointer[0] = WORD8_TO_WORD32( InvSbox[byte0(t0)],   
                                       InvSbox[byte1(t3)],   
                                       InvSbox[byte2(t2)],   
                                       InvSbox[byte3(t1)]) ^ local_pointer[0];   
  output_pointer[1] = WORD8_TO_WORD32( InvSbox[byte0(t1)],   
                                       InvSbox[byte1(t0)],   
                                       InvSbox[byte2(t3)],   
                                       InvSbox[byte3(t2)]) ^ local_pointer[1];   
  output_pointer[2] = WORD8_TO_WORD32( InvSbox[byte0(t2)],   
                                       InvSbox[byte1(t1)],   
                                       InvSbox[byte2(t0)],   
                                       InvSbox[byte3(t3)]) ^ local_pointer[2];   
  output_pointer[3] = WORD8_TO_WORD32( InvSbox[byte0(t3)],   
                                       InvSbox[byte1(t2)],   
                                       InvSbox[byte2(t1)],   
                                       InvSbox[byte3(t0)]) ^ local_pointer[3];

#elif (AES_USELITE == ENABLED)
      /* Register: ask to the compiler to maintain this variable in the   
   processor's registers and don't store it in RAM */   
  register ot_u32 t0;    
  register ot_u32 t1;   
  register ot_u32 t2;   
  register ot_u32 t3;     
  register ot_u32 s0;   
  register ot_u32 s1;   
  register ot_u32 s2;   
  register ot_u32 s3;   
  register int r =  10; /* Count the round */    
  register ot_u32* local_pointer = expkey + 40;   
  ot_u32 f2,f4,f8;   
       
  s0 = input_pointer[0] ^ local_pointer[0];   
  s1 = input_pointer[1] ^ local_pointer[1];   
  s2 = input_pointer[2] ^ local_pointer[2];   
  s3 = input_pointer[3] ^ local_pointer[3];   
       
  /* First add key: before start the rounds */   
  local_pointer -= 8;   
       
  for (;;) /* Start round */    
  {   
    t0 = WORD8_TO_WORD32( InvSbox[byte0(s0)],   
                          InvSbox[byte1(s3)],   
                          InvSbox[byte2(s2)],   
                          InvSbox[byte3(s1)]) ^ local_pointer[4];   
    t1 = WORD8_TO_WORD32( InvSbox[byte0(s1)],   
                          InvSbox[byte1(s0)],   
                          InvSbox[byte2(s3)],   
                          InvSbox[byte3(s2)]) ^ local_pointer[5];   
    t2 = WORD8_TO_WORD32( InvSbox[byte0(s2)],   
                          InvSbox[byte1(s1)],   
                          InvSbox[byte2(s0)],   
                          InvSbox[byte3(s3)]) ^ local_pointer[6];   
    t3 = WORD8_TO_WORD32( InvSbox[byte0(s3)],   
                          InvSbox[byte1(s2)],   
                          InvSbox[byte2(s1)],   
                          InvSbox[byte3(s0)]) ^ local_pointer[7];   
    /*End of InvSbox,  INVshiftRow,  add key*/   
    s0=inv_mcol(t0);   
    s1=inv_mcol(t1);   
    s2=inv_mcol(t2);   
    s3=inv_mcol(t3);   
    /*End of INVMix column */          
    local_pointer -= 4; /*Follow the key sheduler to choose the right round key*/   
         
    if (--r == 1)    
    {   
      break;   
    }   
     
  }/*End of round*/   
     
  /*Start last round :is the only one different from the other*/   
  t0 = WORD8_TO_WORD32( InvSbox[byte0(s0)],   
                        InvSbox[byte1(s3)],   
                        InvSbox[byte2(s2)],   
                        InvSbox[byte3(s1)]) ^ local_pointer[4];   
  t1 = WORD8_TO_WORD32( InvSbox[byte0(s1)],   
                        InvSbox[byte1(s0)],   
                        InvSbox[byte2(s3)],   
                        InvSbox[byte3(s2)]) ^ local_pointer[5];   
  t2 = WORD8_TO_WORD32( InvSbox[byte0(s2)],   
                        InvSbox[byte1(s1)],   
                        InvSbox[byte2(s0)],   
                        InvSbox[byte3(s3)]) ^ local_pointer[6];   
  t3 = WORD8_TO_WORD32( InvSbox[byte0(s3)],   
                        InvSbox[byte1(s2)],   
                        InvSbox[byte2(s1)],   
                        InvSbox[byte3(s0)]) ^ local_pointer[7];   
  output_pointer[0] = t0;   
  output_pointer[1] = t1;   
  output_pointer[2] = t2;   
  output_pointer[3] = t3;
#endif
}   


   
   
   
   
