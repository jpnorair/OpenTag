/* Replace License */
/**
  * @file       /apps/demo_ponglt/code/extf_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R100
  * @date       10 October 2012
  * @brief      Extension Function Configuration File for PongLT Demo
  *
  * Don't actually include this.  Include OTAPI.h or OT_config.h instead.
  *
  * This include file specifies all extension functions that should be compiled
  * into the build.  There are two kinds of Extension Functions (EXTFs):
  * <LI> Static Callbacks -- these are used in most apps </LI>
  * <LI> Patched Library Functions -- for really customized apps only </LI>
  *
  * Almost every function in otlib (and many in otplatform, otradio) can be
  * patched.  Some functions in otlib are callbacks.  These depend on the user
  * to implement them for a particular app.  You may have gotten this file by
  * copying it from another app.  Most apps do not have patches, so the EXTF
  * function definitions in this file are *most likely* for callbacks.
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
///@note EXTF_ext_systask is almost always used externally
//#define EXTF_ext_systask



/// M2 DLL Module EXTFs
//#define EXTF_dll_sig_rfinit
//#define EXTF_dll_sig_rfterminate



/// M2 Network Module EXTFs
//#define EXTF_network_sig_route



/// M2QP Module EXTFs
//#define EXTF_m2qp_sig_errresp
//#define EXTF_m2qp_sig_stdresp
//#define EXTF_m2qp_sig_a2presp
//#define EXTF_m2qp_sig_dsresp
//#define EXTF_m2qp_sig_dsack
//#define EXTF_m2qp_sig_udp



/// MPipe EXTFs



/// OT Utils EXTFs



/// OTAPI C EXTFs



/// OTAPI EXTFs
//#define EXTF_alp_ext_proc


/// ot_queue EXTFs


/// Radio EXTFs


/// Session EXTFs


/// System EXTFs
#define EXTF_sys_sig_panic
#define EXTF_sys_sig_powerdown


/// Veelite Core EXTFs


/// Veelite Module EXTFs




#endif 
