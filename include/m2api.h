/* Copyright 2017 JP Norair
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
/**
  * @file       /include/m2api.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R100
  * @date       7 Oct 2017
  * @brief      Include file that combines m2 headers
  *
  ******************************************************************************
  */

#ifndef __M2API_H
#define __M2API_H

// @todo get all the externally useful protocol constants into session.h or a new file
#include <m2/capi.h>
#include <m2/m2task.h>
#include <m2/session.h>

#include <m2/network.h>
#include <m2/dll.h>
#include <m2/radio.h>
#include <m2/transport.h>
#include <otsys/veelite.h>

///@todo m2alp is defined in m2/transport.c
///      This is temporary code until more elegant structuring is performed.
extern alp_tmpl m2alp;

#endif
