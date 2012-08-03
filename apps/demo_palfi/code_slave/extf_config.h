/*  Copyright 2010-2012, JP Norair
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
  * @file       /apps/demo_palfi/code_slave/extf_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       31 July 2012
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

/// ALP Module EXTFs


/// Auth Module EXTFs


/// Buffer Module EXTFs


/// CRC16 Module EXTFs


/// Encode Module EXTFs



/// External Module EXTFs


/// M2 Network Module EXTFs
//#define EXTF_network_sig_route



/// M2QP Module EXTFs



/// MPipe EXTFs
//#define EXTF_mpipe_sig_txdone
//#define EXTF_mpipe_sig_rxdone
//#define EXTF_mpipe_sig_rxdetect



/// NDEF module EXTFs


/// OT Utils EXTFs



/// OTAPI C EXTFs



/// OTAPI EXTFs
//#define EXTF_otapi_alpext_proc


/// Queue EXTFs



/// Radio EXTFs



/// Session EXTFs


/// System EXTFs
//#define EXTF_sys_sig_panic
#define EXTF_sys_sig_rfainit
#define EXTF_sys_sig_rfaterminate
#define EXTF_sys_sig_extprocess



/// Veelite Core EXTFs



/// Veelite Module EXTFs



#endif 
