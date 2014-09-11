/* Copyright 2010-2012 JP Norair
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
  * @file       /otlib/alp_sensor.c
  * @author     JP Norair
  * @version    V1.0
  * @date       20 July 2011
  * @brief      ALP to Sensor protocol processor
  * @ingroup    ALP
  *
  * Placeholder for future sensor protocol ALP wrapper
  * 
  ******************************************************************************
  */


#include <otlib/alp.h>

#if (   (OT_FEATURE(SERVER) == ENABLED) \
     && (OT_FEATURE(ALP) == ENABLED) \
     && (OT_FEATURE(SENSORS) == ENABLED) )

#include <otlib/auth.h>
#include <otsys/sysext.h>
#include <otlib/queue.h>


// No implementaton yet!
OT_WEAK ot_bool alp_proc_sensor(alp_tmpl* alp, id_tmpl* user_id) {
    return True;
}


#endif


