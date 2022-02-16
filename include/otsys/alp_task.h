/*  Copyright 2008-2022, JP Norair
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted provided that the following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright 
  *    notice, this list of conditions and the following disclaimer in the 
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
  * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
  * POSSIBILITY OF SUCH DAMAGE.
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





