/**
  ******************************************************************************
  * @file    USB_Device/CDC_Standalone/Inc/main.h 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    18-June-2014
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32072b_eval.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h" 
#include "usbd_cdc_interface.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Uncomment the line below to select your USB clock source */
#define USE_USB_CLKSOURCE_CRSHSI48   1
//#define USE_USB_CLKSOURCE_PLL        1

#if !defined (USE_USB_CLKSOURCE_PLL) && !defined (USE_USB_CLKSOURCE_CRSHSI48)
 #error "Missing USB clock definition"
#endif
/* Exported functions ------------------------------------------------------- */
void Toggle_Leds(void);

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
