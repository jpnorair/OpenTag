/**
  ******************************************************************************
  * @file    system_stm32l1xx.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    24-January-2012
  * @brief   CMSIS Cortex-M3 Device Peripheral Access Layer System Header File.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * FOR MORE INFORMATION PLEASE READ CAREFULLY THE LICENSE AGREEMENT FILE
  * LOCATED IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/** @addtogroup CMSIS
  * @{
  */

/** @addtogroup stm32l1xx_system
  * @{
  */  
  
/**
  * @brief Define to prevent recursive inclusion
  */
#ifndef __SYSTEM_STM32L1XX_H
#define __SYSTEM_STM32L1XX_H

#ifdef __cplusplus
 extern "C" {
#endif 

/** @addtogroup STM32L1xx_System_Includes
  * @{
  */

/**
  * @}
  */


/** @addtogroup STM32L1xx_System_Exported_types
  * @{
  */

// Deprecated
//extern uint32_t SystemCoreClock;

/**
  * @}
  */

/** @addtogroup STM32L1xx_System_Exported_Constants
  * @{
  */

/**
  * @}
  */

/** @addtogroup STM32L1xx_System_Exported_Macros
  * @{
  */

/**
  * @}
  */


/** @brief  System Clock & Power Initialization function
  * @param  None
  * @retval None
  * @ingroup STM32L1xx_System_Exported_Functions
  *
  * This function is typically called by one of the CMSIS startup routines.
  */
void SystemInit(void);


/** @brief  System Clock & Power Update Function (deprecated)
  * @param  None
  * @retval None
  * @ingroup STM32L1xx_System_Exported_Functions
  *
  * This function is part of the CMSIS distribution, but it is deprecated for
  * usage within OpenTag.  Instead, OpenTag utilizes its own "Boost/Rest" 
  * functions to change clocks during runtime.  These have fewer parameters,
  * but they are lighter, simpler to use, and they draw parameters purely from
  * the board support header.
  */
//void SystemCoreClockUpdate(void);




#ifdef __cplusplus
}
#endif

#endif /*__SYSTEM_STM32L1XX_H */

/**
  * @}
  */
  
/**
  * @}
  */  
/******************* (C) COPYRIGHT 2012 STMicroelectronics *****END OF FILE****/
