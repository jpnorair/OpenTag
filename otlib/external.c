/*  Copyright 2010-2013, JP Norair
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
  * @file       otlib/external.h
  * @author     JP Norair
  * @version    R100
  * @date       12 Oct 2013
  * @brief      External Data & Event Wrappers
  * @defgroup   External (External Module)
  * @ingroup    External
  *
  ******************************************************************************
  */


#include "OT_config.h"
#include "external.h"


#ifndef EXTF_ext_init
OT_WEAK void ext_init() { 
/// See comments from external.h.  BEST PRACTICE is to leave this impl empty.
}
#endif


#ifndef EXTF_ext_get_m2appflags
OT_WEAK ot_u8 ext_get_m2appflags() {
/// See comments from external.h.  BEST PRACTICE is to leave this impl as-is.
    return 0;
}
#endif



#ifndef EXTF_ext_systask
OT_WEAK void ext_systask(ot_task task) {
/// See comments from external.h.  BEST PRACTICE is to leave this impl empty.
}
#endif
