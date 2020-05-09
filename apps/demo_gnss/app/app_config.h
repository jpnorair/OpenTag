/* Copyright 2010-2014 JP Norair
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
  * @file       /apps/htdemo/app/app_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R101
  * @date       31 Aug 2014
  * @brief      Application Configuration File for PongLT Demo
  *
  * In your application code, you could include this file as <app/app_config.h>
  * but this is not recommended.  Instead, use:
  *
  * #include <otstd.h>
  * or
  * #include <otsys/config.h>
  *
  * As these are tested to properly bundle everything properly through the 
  * compiler.
  *
  ******************************************************************************
  */

#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#include <app/build_config.h>

/** Filesystem constants, setup, and boundaries <BR>
  * ========================================================================<BR>
  * For this app, we use default features from: 
  * /apps/_common/features_default_config.h
  *
  * Before the #include statement below, you can set-up some overriding feature
  * and parameter settings.  Take a look at features_default_config.h to see
  * what the features are.
  */

// OPTIONAL: custom feature/param overrides
// #define OT_FEATURE_...
// #define OT_PARAM_... 
// ...etc

///@todo There is a bug I cannot figure-out, where on ST Discovery boards, the
///      project will not build if OT_FEATURE_MPIPE is defined here, and on
///      HayTags, the project will not build if it isn't.
///      The trouble is that in otsys/mpipe.c, OT_FEATURE_MPIPE seems to take
///      on the value DISABLED if it is set here, whereas in other files it
///      takes on the proper value.
//#define OT_FEATURE_MPIPE            (BOARD_FEATURE_MPIPE)
#if (defined(BOARD_HayTag_LoRa_GPS) || defined(BOARD_HayTag_LoRa_R2_GPS))
#   define OT_FEATURE_MPIPE           DISABLED
#endif

// Application task functions (implemented somewhere, probably main.c)
void gnssreporter_systask(void* arg);
void ping_systask(void* arg);

// This is a GPS/GNSS demo
#define OT_FEATURE_GNSS          ENABLED

// Use comma separated with TASK_xxx nomenclature (e.g. TASK_0, TASK_1, TASK_2)
#if (OT_FEATURE_GNSS == ENABLED)
#   define OT_PARAM_EXOTASKS           1
#   define OT_PARAM_EXOTASK_IDS        TASK_gnss
#   define OT_PARAM_EXOTASK_HANDLES    &gnss_systask
#   define OT_PARAM_KERNELTASKS        2
#   define OT_PARAM_KERNELTASK_IDS     TASK_gnssreporter, TASK_ping
#   define OT_PARAM_KERNELTASK_HANDLES &gnssreporter_systask, &ping_systask

#else
#   define OT_PARAM_KERNELTASKS        1
#   define OT_PARAM_KERNELTASK_IDS     TASK_ping
#   define OT_PARAM_KERNELTASK_HANDLES &ping_systask

#endif


#define OT_PARAM_SSTACK_ALLOC       1024

#define OT_FEATURE_EXT_TASK         DISABLED

#define ALP_ASAPI                   ENABLED

#define M2_FEATURE_RSCODE           DISABLED
#define M2_FEATURE_HSCODE           ENABLED

#include <../_common/features_default_config.h>





/** Filesystem constants, setup, and boundaries <BR>
  * ========================================================================<BR>
  * For this app, we are just using the default FS setup, which is located in
  * /apps/_common/
  */
#include <../_common/fs_default_config.h>


#endif 
