/*  Copyright 2010-2012, JP Norair
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
/**
  * @file       /otlibext/applets_std/mpipe_sig_done.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2012
  * @brief      Standard MPipe Signal Routines, for packet done
  *
  * 
  */

#include <otstd.h>
#include <board.h>
#include <otsys/mpipe.h>


#ifdef EXTF_mpipe_sig_txdone
void mpipe_sig_txdone(void* tmpl) { 
    otapi_ndef_idle(tmpl); 
}
#endif

#ifdef EXTF_mpipe_sig_rxdone
void mpipe_sig_rxdone(void* tmpl) {
    otapi_ndef_proc(tmpl); 
}
#endif


