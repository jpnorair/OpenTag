/*  Copyright 2017, JP Norair
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
  * @file       /include/otsys/alp_task.h
  * @author     JP Norair
  * @version    R101
  * @date       2 May 2017
  * @brief      ALP Task interface
  * @ingroup    ALP_Task
  *
  * The ALP task can process an ALP stream, and deliver callbacks to apps that
  * are hooked-in to protocols.
  *
  ******************************************************************************
  */


#ifndef __OTSYS_ALP_TASK_H
#define __OTSYS_ALP_TASK_H

#include <otstd.h>
#include <otsys/syskern.h>
#include <otlib/alp.h>




//typedef struct {
//    
//} ot_alp;



/** @brief  Reads data out from byte stream into a user buffer
  * @param  alp_id      (ot_u8) ID of ALP to read-out from ALP stream
  * @param  ctl_flags   (ot_u8) Control Flags, for customizing operation
  * @param  limit       (ot_u16) byte limit of destination buffer
  * @param  dst         (ot_u8*) byte buffer for readout
  * @retval ot_uint     Number of bytes written
  * @ingroup ALP_Task
  *
  * ctl_flags:
  * 0 - Standard usage: only reads-out completed messages, and deletes them
  * 1 - Readout contents received, even if message incomplete
  * 2 - Include ALP Headers in readout.
  * 4 - Don't delete messages from ALP stream
  
  
  */
ot_uint alp_readout(ot_u8 alp_id, ot_u8 ctl_flags, ot_u16 limit, ot_u8* dst);



/** @brief  ALP Task runtime function
  * @param  task        (ot_task) Task marker containing task parameters
  * @retval None
  * @ingroup ALP_Task
  *
  */
void alp_systask(ot_task task);

#endif





