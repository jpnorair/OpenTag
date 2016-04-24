/**
  ******************************************************************************
  * @file    usb_mem.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    28-August-2012
  * @brief   Utility functions for memory transfers to/from PMA
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
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

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include <otlib.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : UserToPMABufferCopy
* Description    : Copy a buffer from user memory area to packet memory area (PMA)
* Input          : - pbUsrBuf: pointer to user memory area.
*                  - wPMABufAddr: address into PMA.
*                  - wNBytes: no. of bytes to be copied.
* Output         : None.
* Return         : None	.
*******************************************************************************/
void UserToPMABufferCopy(uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes) {
    uint16_t*   pdwVal  = (uint16_t *)((wPMABufAddr<<1) + PMAAddr);
    int16_t     count   = (wNBytes >> 1) + 1;
    uint32_t    temp1;
    uint32_t    temp2;
  
    while (--count) {
        temp1       = (uint16_t)*pbUsrBuf;
        pbUsrBuf++;
        temp2       = temp1 | (uint16_t)*pbUsrBuf << 8;
        pbUsrBuf++;
        *pdwVal++   = temp2;
        pdwVal++;
    }
    
    ///@note version below uses OTLib function memcpy2, and it also requires 
    /// that pbUsrBuf is 16bit aligned.  Not really a problem.
    //if (wNBytes != 0) {
    //    memcpy2(pdwVal, (uint16_t*)pbUsrBuf, wNBytes>>1);
    //}
}





/*******************************************************************************
* Function Name  : PMAToUserBufferCopy
* Description    : Copy a buffer from user memory area to packet memory area (PMA)
* Input          : - pbUsrBuf    = pointer to user memory area.
*                  - wPMABufAddr = address into PMA.
*                  - wNBytes     = no. of bytes to be copied.
* Output         : None.
* Return         : None.
*******************************************************************************/
void PMAToUserBufferCopy(uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes) {
    uint32_t*   pdwVal  = (uint32_t *)((wPMABufAddr<<1) + PMAAddr);
    int16_t     count   = (wNBytes >> 1) + 1;
    
    while (--count) {
        uint16_t scratch    = *pdwVal++;
        *pbUsrBuf++         = ((uint8_t*)&scratch)[0];
        *pbUsrBuf++         = ((uint8_t*)&scratch)[1];
    }
    
    ///@note version below uses OTLib function memcpy2, and it also requires 
    /// that pbUsrBuf is 16bit aligned.  Not really a problem.
    //if (wNBytes != 0) {
    //    memcpy2((uint16_t*)pbUsrBuf, pdwVal, wNBytes>>1);
    //}
    
}




/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
