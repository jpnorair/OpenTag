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
  * @file       /otlib/alp_api_client.c
  * @author     JP Norair
  * @version    R100
  * @date       22 Aug 2013
  * @brief      ALP message building routines for the client.
  * @ingroup    ALP
  *
  * The functions within are intended to be used on the client side to build
  * the API message from simpler primitives.  In other words, client sends API
  * messages, server receives them and does something (whatever the API message
  * requires).
  * 
  ******************************************************************************
  */


#include <otlib/alp.h>

#if ((OT_FEATURE(ALP) == ENABLED) && (OT_FEATURE(CLIENT) == ENABLED))

#include <otplatform.h>
#include <otlib/buffers.h>
#include <otlib/queue.h>

///@todo All this stuff.  Much can be copied (and cleaned up) from testbeds 
///      I made for other purposes.


#endif


