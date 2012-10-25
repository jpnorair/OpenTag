/*  Copyright 2010-2011, JP Norair
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
  */
/**
  * @file       /apps/demo_palfi/code_master/extf_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R100
  * @date       10 October 2012
  * @brief      Extension Function Configuration File for PaLFi Demo
  *
  * Don't actually include this.  Include OTAPI.h or OT_config.h instead.
  *
  * This include file specifies all extension functions that should be compiled
  * into the build.  Extension functions are replacements/patches for functions
  * declared in OTlib, so if you define an Extension Function (EXTF), OpenTag
  * will build and link your function instead of the regular OTlib version.
  ******************************************************************************
  */

#ifndef __EXTF_CONFIG_H
#define __EXTF_CONFIG_H

// Special Inclusion, as this app can use two very different boards
#include "platform_config.h"




/// ALP Module EXTFs



/// Auth Module EXTFs



/// Buffer Module EXTFs



/// CRC16 Module EXTFs



/// Encode Module EXTFs



/// External Module EXTFs
///@note EXTF_ext_systask is almost always used externally
#define EXTF_ext_systask



/// M2 DLL Module EXTFs
#define EXTF_dll_sig_rfinit
#define EXTF_dll_sig_rfterminate



/// M2 Network Module EXTFs
//#define EXTF_network_sig_route



/// M2QP Module EXTFs
//#define EXTF_m2qp_sig_errresp
//#define EXTF_m2qp_sig_stdresp
//#define EXTF_m2qp_sig_a2presp
//#define EXTF_m2qp_sig_dsresp
//#define EXTF_m2qp_sig_dsack
#define EXTF_m2qp_sig_udp



/// MPipe EXTFs



/// OT Utils EXTFs



/// OTAPI C EXTFs
#ifdef BOARD_RF430USB_5509
#   define EXTF_otapi_led1_on
#   define EXTF_otapi_led2_on
#   define EXTF_otapi_led1_off
#   define EXTF_otapi_led2_off
#endif



/// OTAPI EXTFs
//#define EXTF_otapi_alpext_proc



/// Queue EXTFs



/// Radio EXTFs


/// Session EXTFs


/// System EXTFs
#define EXTF_sys_sig_panic
//#define EXTF_sys_sig_powerdown



/// Veelite Core EXTFs


/// Veelite Module EXTFs




#endif 
