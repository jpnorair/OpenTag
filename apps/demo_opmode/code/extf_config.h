/* Replace License */
/**
  * @file       /apps/demo_opmode/code/extf_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       31 July 2012
  * @brief      Extension Function Configuration File for Opmode Demo
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
#define EXTF_network_sig_route



/// M2QP Module EXTFs
//#define EXTF_m2qp_sig_errresp
//#define EXTF_m2qp_sig_stdresp
//#define EXTF_m2qp_sig_a2presp
//#define EXTF_m2qp_sig_dsresp
//#define EXTF_m2qp_sig_dsack
//#define EXTF_m2qp_sig_udpreq



/// MPipe EXTFs
#define EXTF_mpipe_sig_txdone
#define EXTF_mpipe_sig_rxdone


/// NDEF module EXTFs


/// OT Utils EXTFs


/// OTAPI C EXTFs


/// OTAPI EXTFs
#define EXTF_otapi_alpext_proc


/// Queue EXTFs


/// Radio EXTFs


/// Session EXTFs


/// System EXTFs
//#define EXTF_sys_sig_loadapp
//#define EXTF_sys_sig_panic
//#define EXTF_sys_sig_rfainit
//#define EXTF_sys_sig_rfaterminate


/// Veelite Core EXTFs


/// Veelite Module EXTFs




#endif 
