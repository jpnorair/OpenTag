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
  * @file       /sys/version.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       22 August 2011
  * @brief      OpenTag Version Data
  *
  * The rules for the build versioning are shown below.  These constants should
  * be defined in the build_config.h file (stored in each application).  If you
  * leave out one of the build string elements, the default will be used.  This
  * build string will go into the "Firmware Version" field of the Device
  * Features file (ISF 1).  In most cases, you can just use the defaults. The
  * most relevant constant is OT_BUILDTYPE, which you might want to specify
  * explicitly. <BR><BR>
  *
  * #define OT_VERSION_MAJOR                                <BR>
  * Major version number, such as 1, 2, 3, etc              <BR><BR>
  *
  * #define OT_BUILDTYPE                                    <BR>
  * Build Type is a single character described as follows:  <BR>
  * <LI> Uppercase - official builds                        </LI>
  * <LI> Lowercase - unofficial builds                      </LI>
  * <LI> B/b: Beta                                          </LI>
  * <LI> F/f: Built onto FreeRTOS                           </LI>
  * <LI> P/p: Built onto POSIX                              </LI>
  * <LI> S/s: Standard release                              </LI>
  * <LI> R/r: Built onto some other embedded RTOS           </LI>
  * <LI> T/t: Built onto TinyOS                             </LI>
  * <LI> u:   Unknown                                       </LI>
  *
  * #define OT_BUILDID                                      <BR>
  * A value 0-65535 (16 bits) that encapsulates minor version information.  You
  * can use this however you want.  Official builds break it into two bytes, the
  * first one being the minor version and the second being the revision.
  * <BR><BR>
  *
  * #define OT_FEATURELIST                                  <BR>
  * A 32 bit mask indicating compiled-in features, from app_config_xxx.h.  You
  * should not define this (just use default) unless you have some unusual
  * feature in mind.  <BR>
  *
  ******************************************************************************
  */

#ifndef __OT_VERSION_H
#define __OT_VERSION_H

#include <otsys/config.h>



//BASE64 Converter
#define __B64_1(X)      (X+((25<=X)*65))
#define __B64_2(X)      ((X+((26<=X)&(X<=51))*4) + (((26<=X)&(X<=51))*__B64_1(X)))
#define __B64_3(X)      ((X-((52<=X)&(X<=61))*4) + (((52<=X)&(X<=61))*__B64_2(X)))
#define __B64_4(X)      ((X==62)*47 + ((X==62)*__B64_3(X)))
#define __B64_5(X)      (((X==63)*47) + ((X==63)*__B64_4(X)))
#define BASE64_EN(BYTE) ((ot_u8)__B64_5( (BYTE & 0x3F) ))


//OpenTag version code computation
#ifndef OT_VERSION_MAJOR
#   define OT_VERSION_MAJOR     0
#endif
#ifndef OT_BUILDTYPE
#   define OT_BUILDTYPE         'u'
#endif
#ifndef OT_BUILDID
#   define OT_BUILDID           0x0300
#endif
#ifndef OT_FEATURELIST
#   define OT_FEATURELIST       DEV_FEATURES_BITMAP
#endif


#define OT_BUILDCODE0   BASE64_EN((ot_u8)(OT_BUILDID>>10))
#define OT_BUILDCODE1   BASE64_EN((ot_u8)(OT_BUILDID>>4))
#define OT_BUILDCODE2   BASE64_EN((ot_u8)( (OT_BUILDID<<2)|(ot_u8)(OT_FEATURELIST>>30)) )
#define OT_BUILDCODE3   BASE64_EN((ot_u8)(OT_FEATURELIST>>24))
#define OT_BUILDCODE4   BASE64_EN((ot_u8)(OT_FEATURELIST>>18))
#define OT_BUILDCODE5   BASE64_EN((ot_u8)(OT_FEATURELIST>>12))
#define OT_BUILDCODE6   BASE64_EN((ot_u8)(OT_FEATURELIST>>6))
#define OT_BUILDCODE7   BASE64_EN((ot_u8)(OT_FEATURELIST>>0))



// Device Scheduler Settings
#if (OT_FEATURE(TIME) == ENABLED)
#   define OT_SUPPORTED_SCHEDULER   (((ot_u16)M2_FEATURE(RTCHOLD) << 15)    | \
                                    ((ot_u16)M2_FEATURE(RTCSLEEP) << 14)    | \
                                    ((ot_u16)M2_FEATURE(RTCBEACON) << 13))
#   if (M2_FEATURE(RTCSLEEP) == ENABLED)
#       define OT_ACTIVE_SCHEDULER  0x4000
#   elif (M2_FEATURE(RTCHOLD) == ENABLED)
#       define OT_ACTIVE_SCHEDULER  0x8000
#   elif (M2_FEATURE(RTCBEACON) == ENABLED)
#       define OT_ACTIVE_SCHEDULER  0x2000
#   else
#       define OT_ACTIVE_SCHEDULER  0x0000
#   endif

#else
#   define OT_SUPPORTED_SCHEDULER   0x0000
#   define OT_ACTIVE_SCHEDULER      0x0000
#endif


// Device Class Settings
#define OT_SUPPORTED_CLASSES   (((ot_u16)M2_FEATURE(GATEWAY) << 10)         | \
                                ((ot_u16)M2_FEATURE(SUBCONTROLLER) << 9)   | \
                                ((ot_u16)M2_FEATURE(ENDPOINT) << 8))
#if (M2_FEATURE(GATEWAY) == ENABLED)
#   define OT_ACTIVE_CLASS      0x0400
#elif (M2_FEATURE(SUBCONTROLLER) == ENABLED)
#   define OT_ACTIVE_CLASS      0x0200
#elif (M2_FEATURE(ENDPOINT) == ENABLED)
#   define OT_ACTIVE_CLASS      0x0100
#else
#   define OT_ACTIVE_CLASS      0x0000
#endif


// Device Datastreaming Settings
#define OT_SUPPORTED_M2DP 		((M2_FEATURE(M2DP) << 6) | (M2_FEATURE(M2DP) << 7))
#define OT_ACTIVE_M2DP          OT_SUPPORTED_M2DP


// Device FEC Settings
#define OT_SUPPORTED_FEC        (((ot_u16)M2_FEATURE(FECTX) << 5)     | \
                                 ((ot_u16)M2_FEATURE(FECRX) << 4))
#define OT_ACTIVE_FEC           OT_SUPPORTED_FEC


// Channel Usage Settings
#define OT_SUPPORTED_CHANS      ((0 << 3)     | \
                                 ((ot_u16)M2_FEATURE(TURBO) << 2)     | \
                                 ((ot_u16)M2_FEATURE(NORMAL) << 1)    | \
                                 (0))
#define OT_ACTIVE_CHANS         OT_SUPPORTED_CHANS


// WRAPPER
#define OT_SUPPORTED_SETTINGS   (OT_SUPPORTED_SCHEDULER     | \
                                 OT_SUPPORTED_CLASSES       | \
                                 OT_SUPPORTED_M2DP          | \
                                 OT_SUPPORTED_FEC           | \
                                 OT_SUPPORTED_CHANS)

#define OT_ACTIVE_SETTINGS      (OT_ACTIVE_SCHEDULER    | \
                                 OT_ACTIVE_CLASS        | \
                                 OT_ACTIVE_M2DP         | \
                                 OT_ACTIVE_FEC          | \
                                 OT_ACTIVE_CHANS)





#endif
