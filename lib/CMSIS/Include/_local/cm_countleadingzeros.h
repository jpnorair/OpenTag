/* Copyright 2014 JP Norair
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
  * @file       /lib/CMSIS/Include/_local/cm_countleadingzeros.h
  * @author     JP Norair
  * @version    R100
  * @date       21 Aug 2014
  * @brief      CLZ setup for ARM Cortex-M
  *
  * This file should be included from cm3_countleadingzeros.h, etc.
  *
  * The Cortex M0 and M0+ do not have this command.
  * 
  ******************************************************************************
  */
  

#ifndef __CM_COUNTLEADINGZEROS_H
#define __CM_COUNTLEADINGZEROS_H


#define __countleadingzeros(x)  __CLZ(x)


#endif
