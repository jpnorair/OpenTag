/* Copyright 2013-14 JP Norair
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
  * @file       /include/platform/data.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Mar 2014
  * @brief      Platform Data Elements
  * @ingroup    Platform
  *
  ******************************************************************************
  */


#ifndef __PLATFORM_DATA_H
#define __PLATFORM_DATA_H

#include <otsys/types.h>
#include <otsys/config.h>
#include <app/build_config.h>



/** Required Platform Data <BR>
  * ========================================================================<BR>
  */

typedef struct {
	ot_u16	error_module;	
    ot_int	error_code;
} platform_struct;

extern platform_struct platform;



#endif
