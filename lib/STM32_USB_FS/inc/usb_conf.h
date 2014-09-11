/*  Copyright 2014, JP Norair
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

/** @note [JP Norair] This file will pick a usb_conf[...].h file based on the
  * setup of OpenTag features.
  */

#ifndef __USB_CONF_H
#define __USB_CONF_H

// OpenTag Platform header contains board and feature information
#include <platform/config.h>

///@todo If more modes of USB operation are present, then this will need to be
///      expanded and made more intelligent.

#if MCU_CONFIG(MPIPECDC)
#   include "usb_conf[acm].h"
#else
#   warning "usb_conf.h is referenced, but the board header does not enable USB."
#endif


#endif
