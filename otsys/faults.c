/* Copyright 2018 JP Norair
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
  * @file       /otsys/faults.c
  * @author     JP Norair
  * @version    R100
  * @date       31 Jan 2018
  * @brief      System Fault Handling Implementation
  * @ingroup    Faults
  *
  * Faults are saved in a veelite file that must be known during compile-time.
  * The standardized file for tracking faults is ...
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otsys/time.h>
#include <otsys/veelite.h>

/*
typedef struct {
    ot_u16 file_id;
    ot_u16 line;
} ot_faultctx;

static ot_faultctx faultctx;




ot_int faults_init(vlACTION* onfault, ) {
#if OT_FEATURE(FAULTS)
    

#else
    return 0;
#endif
}
*/
