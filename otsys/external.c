/*  Copyright 2010-2014, JP Norair
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
  * @file       /otsys/sysext.c
  * @author     JP Norair
  * @version    R101
  * @date       27 Aug 2014
  * @brief      External Task Wrapper
  * @ingroup    System-External
  *
  ******************************************************************************
  */


#include <otstd.h>
#include <otsys/sysext.h>


#ifndef EXTF_ext_init
OT_WEAK void ext_init() { 
/// See comments from external.h.  BEST PRACTICE is to leave this impl empty.
}
#endif


#ifndef EXTF_ext_systask
OT_WEAK void ext_systask(ot_task task) {
/// See comments from external.h.  BEST PRACTICE is to leave this impl empty.
}
#endif
