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
  *
  */
/**
  * @file       OTlib/external.h
  * @author     JP Norair
  * @version    V1.0
  * @date       10 Sept 2011
  * @brief      External Data & Event Wrappers
  * @defgroup   External (External Module)
  * @ingroup    External
  *
  * @note       WORK IN PROGRESS.  Sensors/External Events are not supported
  *             in this version of OpenTag.
  ******************************************************************************
  */


#include "OT_config.h"
#include "external.h"


void ext_init() { }


#ifndef EXTF_ext_get_m2appflags
ot_u8 ext_get_m2appflags() {
/// This is a dummy implementation with no app flag support
    return 0;
}
#endif
