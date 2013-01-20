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


/** @brief  System Clock Up-Booster, to begin "Turbo Mode" operation
  * @param  None
  * @retval None
  * @ingroup STM32L1xx_OpenTag_Functions
  * @sa SystemUnboost
  *
  * Call this function during normal runtime to put the system in Turbo Mode,
  * as specified in STM32L board support headers.  Turbo Mode stipulates the
  * usage of HSI, HSE, or PLL as a system clock.  It is optional, but it is the
  * only way to utilize these clocks when your board/platform/system uses the
  * STOP mode (MSI is activated in HW when going from STOP->ACTIVE).  If you
  * are not using STOP mode, you can still use it before running code that 
  * benefits from a fast clock.
  */
void SystemBoost(void);



/** @brief  System Clock Down-Booster, to terminate "Turbo Mode" operation
  * @param  None
  * @retval None
  * @ingroup STM32L1xx_OpenTag_Functions
  * @sa SystemBoost
  * @sa SystemResume
  *
  * Call this function during Turbo runtime to disable the Turbo mode.  Normal
  * Runtime (unboosted) uses the MSI clock as the system clock.  The clock rate
  * of MSI is usually 1.05, 2.1, or 4.2 MHz, but it can also use other values,
  * and the value for your board/platform/system should be specified in your
  * OpenTag board support header.
  *
  * @note If you are utilizing the STOP mode, upon entry to STOP all clocks are
  * disabled and MSI clock is automatically the clock used when coming out of
  * STOP.  This is the same behavior as calling this function, except for the 
  * fact that this function adjusts the flash wait states.  As such, there is
  * the function SystemResume() that you can use coming out of STOP to do 
  * nothing more than update the flash wait-state for MSI (unboosted) regime.
  * SystemUnboost() is most useful in apps/builds that do not utilize STOP mode
  * where it is used at the end of a demanding kernel task.
  */
void SystemUnboost(void);




/** @brief  Optimizes System for MSI clocking, when resuming from STOP mode
  * @param  None
  * @retval None
  * @ingroup STM32L1xx_OpenTag_Functions
  * @sa SystemBoost
  * @sa SystemUnboost
  *
  * On the STM32L, it takes about 8us to come out of STOP mode, after which 
  * point the system clock is MSI.  If you have HSCLOCK (turbo mode) set to the
  * HSI or a divided HSI, then the system will require an additional 8us for 
  * HSI to stabilize.  HSE and PLL modes can take up to 100us.  In any case, 
  * these modes will only be invoked if you call SystemBoost() on return from
  * STOP.  If you intend to use MSI coming out of stop, it is a good practice
  * to call SystemResume() to optimize the RCC for MSI operation.  The function
  * is very minimal and will run to completion in 0-to-8 instructions.
  */
void SystemResume(void);


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
