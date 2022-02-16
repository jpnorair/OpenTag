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
  * @file       /apps/demo_ponglt/code/app_config.h
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

// OPTIONAL: custom feature/param overrides (None present)
// #define OT_FEATURE_...
// #define OT_PARAM_... 
// ...etc

#include <../_common/features_default_config.h>





/** Filesystem constants, setup, and boundaries <BR>
  * ========================================================================<BR>
  * For this app, we are just using the default FS setup, which is located in
  * /apps/_common/
  */
#include <../_common/fs_default_config.h>


#endif 
