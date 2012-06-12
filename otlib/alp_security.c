/* Copyright 2010-2011 JP Norair
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
  * @file       /OTlib/alp_sensor.c
  * @author     JP Norair
  * @version    V1.0
  * @date       1 July 2011
  * @brief      ALP to Security protocol processor
  * @ingroup    ALP
  *
  * The security protocol and what it does are still being defined within the 
  * DASH7 Alliance.  As far as I know, there will be at least two supported
  * security protocols, one for asymmetric and one for symmetric.
  * 
  ******************************************************************************
  */


#include "alp.h"

#if (   (OT_FEATURE(SERVER) == ENABLED) \
     && (OT_FEATURE(ALP) == ENABLED) \
     && (OT_FEATURE(SECURITY) == ENABLED) )

#include "auth.h"


// No implementaton yet!
void alp_proc_sec_example(alp_record* in_rec, alp_record* out_rec,
                            Queue* in_q, Queue* out_q, id_tmpl* user_id    ) {
    
}


#endif


