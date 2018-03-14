/**
  ******************************************************************************
  * @note   This file and others in the HAL_PCD have been modified by JP Norair
  *         in September 2014, for usage with OpenTag.  They have also been
  *         optimized for size and speed.  Some improvements have been made to
  *         data structuring, integer manipulation, and other such things that
  *         may prevent this distro from being interchangeable with files or 
  *         functions from the original STM32Cube distro.
  * 
  * @file    stm32l0xx_hal_pcd_ex.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    18-June-2014
  * @brief   Extended PCD HAL module driver.
  *          This file provides firmware functions to manage the following 
  *          functionalities of the USB Peripheral Controller:
  *           + Configururation of the PMA for EP
  * 
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

/** @addtogroup STM32L0xx_HAL_Driver
  * @{
  */

/** @defgroup PCDEx 
  * @brief PCDEx HAL module driver
  * @{
  */

#ifdef HAL_PCD_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/** @defgroup PCDEx_Private_Functions
  * @{
  */

/** @defgroup PCDEx_Group1 Initialization and de-initialization functions 
 *  @brief    Initialization and Configuration functions 
 *
@verbatim    
 ===============================================================================
                 ##### Peripheral extended features functions #####
 ===============================================================================
@endverbatim
  * @{
  */

/**
  * @brief Configure PMA for EP
  * @param  pdev : Device instance
  * @param  ep_addr: endpoint address
  * @param  ep_Kind: endpoint Kind
  *                  USB_SNG_BUF: Single Buffer used
  *                  USB_DBL_BUF: Double Buffer used
  * @param  pmaadress: EP address in The PMA: In case of single buffer endpoint
  *                   this parameter is 16-bit value providing the address
  *                   in PMA allocated to endpoint.
  *                   In case of double buffer endpoint this parameter
  *                   is a 32-bit value providing the endpoint buffer 0 address
  *                   in the LSB part of 32-bit value and endpoint buffer 1 address
  *                   in the MSB part of 32-bit value.
  * @retval : status
  */

HAL_StatusTypeDef  HAL_PCDEx_PMAConfig( PCD_HandleTypeDef *hpcd, 
                                        uint16_t ep_addr,
                                        uint16_t ep_kind,
                                        uint32_t pmaadress) {
    PCD_EPTypeDef *ep;
    uint8_t num;
    
    /* initialize ep structure*/
    num = ep_addr & 0x7F;
    ep  = (0x80 & ep_addr) ? hpcd->IN_ep : hpcd->OUT_ep;
    ep  = &ep[num];
  
    /* Here we check if the endpoint is single or double Buffer*/
    ///@todo [JPN] This is horribly ugly and must be streamlined
    ep->doublebuffer    = (ep_kind != PCD_SNG_BUF);
    ep->pmaadress       = (uint16_t)pmaadress;              // Used with single-bufferring
    
    //ep->pmaaddr0        = (uint16_t)pmaadress;              // Used with double-bufferring
    //ep->pmaaddr1        = (uint16_t)(pmaadress >> 16);      // Used with double-bufferring
    
    //ep->pmaaddr0        = ((uint16_t*)&pmaadress)[0];      // Used with double-bufferring
    //ep->pmaaddr1        = ((uint16_t*)&pmaadress)[1];      // Used with double-bufferring
    
    *((uint32t*)&ep->pmaaddr0) = pmaadress;                 // Used with double-bufferring
    
    return HAL_OK; 
}
/**
  * @}
  */


/**
  * @}
  */

#endif /* HAL_PCD_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
