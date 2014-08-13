/**
  ******************************************************************************
  * @file    system_stm32l1xx.c
  * @author  JP Norair (original work by STMicro MCD Application Team)
  * @version R100
  * @date    31-December-2012
  * @brief   CMSIS Cortex-M3 Device Peripheral Access Layer System Source File.
  *          This file contains the system clock configuration for STM32L1xx Ultra
  *          Low Power devices, and is generated by the clock configuration
  *          tool "STM32L1xx_Clock_Configuration_V1.1.0.xls".
  *      
  * 1. After each device reset the MSI (2.1 MHz Range) is used as system clock source.
  *    Then SystemInit() function is called, in "startup_stm32l1xx_xx.s" file, to
  *    configure the system clock before to branch to main program.    
  *    
  * 2. If the system clock source selected by user fails to startup, the SystemInit()
  *    function will do nothing and MSI still used as system clock source. User can 
  *    add some code to deal with this issue inside the SetSysClock() function.
  * 
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
  

//#include "stm32l1xx.h"
#include "OT_platform.h"    //get settings




/** @addtogroup STM32L1xx_System_Private_Defines
  * @{
  */

/*!< Uncomment the following line if you need to use external SRAM mounted
     on STM32L152D_EVAL board as data memory  */
/* #define DATA_IN_ExtSRAM */
  
/*!< Uncomment the following line if you need to relocate your vector Table in
     Internal SRAM. */ 
/* #define VECT_TAB_SRAM */
#define VECT_TAB_OFFSET  0x0 /*!< Vector Table base offset field. 
                                  This value must be a multiple of 0x200. */
/**
  * @}
  */



/** @addtogroup STM32L1xx_System_Private_Variables
  * @{
  */
//uint32_t SystemCoreClock        = PLATFORM_HSCLOCK_HZ;
//__I uint8_t PLLMulTable[9]      = {3, 4, 6, 8, 12, 16, 24, 32, 48};
//__I uint8_t AHBPrescTable[16]   = {0,0,0,0,0,0,0,0,1,2,3,4,6,7,8,9};

/**
  * @}
  */

/** @addtogroup STM32L1xx_System_Private_FunctionPrototypes
  * @{
  */

void sub_msflash_config(void);
void sub_hsflash_config(void);
void sub_voltage_config(void);
void sub_hsosc_config(void);
void sub_clock_select(void);
void sub_hsclock_config(void);

static void SetSysClock(void);
#ifdef DATA_IN_ExtSRAM
    static void SystemInit_ExtMemCtl(void); 
#endif /* DATA_IN_ExtSRAM */




#if MCU_CONFIG(MULTISPEED)
#   define _BASE_CLOCK_HZ   PLATFORM_MSCLOCK_Hz
#   define _TURBO_CLOCK_HZ  PLATFORM_HZCLOCK_Hz
#else
#   define _BASE_CLOCK_HZ   PLATFORM_HSCLOCK_Hz
#endif



#if ((BOARD_PARAM_HFmult != 1) || (BOARD_PARAM_HFdiv != 1))
#   define _USE_PLL
#   if (BOARD_PARAM_HFmult == 3)
#       define _PLL_MULT    (0<<18)
#   elif (BOARD_PARAM_HFmult == 4)
#       define _PLL_MULT    (1<<18)
#   elif (BOARD_PARAM_HFmult == 6)
#       define _PLL_MULT    (2<<18)
#   elif (BOARD_PARAM_HFmult == 8)
#       define _PLL_MULT    (3<<18)
#   elif (BOARD_PARAM_HFmult == 12)
#       define _PLL_MULT    (4<<18)
#   elif (BOARD_PARAM_HFmult == 16)
#       define _PLL_MULT    (5<<18)
#   elif (BOARD_PARAM_HFmult == 24)
#       define _PLL_MULT    (6<<18)
#   elif (BOARD_PARAM_HFmult == 32)
#       define _PLL_MULT    (7<<18)
#   elif (BOARD_PARAM_HFmult == 48)
#       define _PLL_MULT    (8<<18)
#   else
#   error "PLL Multiplier from BOARD_PARAM_HFmult is out of range"
#   endif

#   if (BOARD_PARAM_HFdiv == 2)
#       define _PLL_DIV    (1<<22)
#   elif (BOARD_PARAM_HFdiv == 3)
#       define _PLL_DIV    (2<<22)
#   elif (BOARD_PARAM_HFdiv == 4)
#       define _PLL_DIV    (3<<22)
#   else
#   error "PLL Divider from BOARD_PARAM_HFdiv is out of range"
#   endif

#endif



#if (BOARD_PARAM_AHBCLKDIV == 1)
#   define _AHB_DIV     (0<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 2)
#   define _AHB_DIV     (8<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 4)
#   define _AHB_DIV     (9<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 8)
#   define _AHB_DIV     (10<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 16)
#   define _AHB_DIV     (11<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 64)
#   define _AHB_DIV     (12<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 128)
#   define _AHB_DIV     (13<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 256)
#   define _AHB_DIV     (14<<4)
#elif (BOARD_PARAM_AHBCLKDIV == 512)
#   define _AHB_DIV     (15<<4)
#else
#   error "BOARD_PARAM_AHBCLKDIV not set to a value permitted by this HW"
#endif

#if (BOARD_PARAM_APB1CLKDIV == 1)
#   define _APB1_DIV    (0<<8)
#elif (BOARD_PARAM_APB1CLKDIV == 2)
#   define _APB1_DIV    (4<<8)
#elif (BOARD_PARAM_APB1CLKDIV == 4)
#   define _APB1_DIV    (5<<8)
#elif (BOARD_PARAM_APB1CLKDIV == 8)
#   define _APB1_DIV    (6<<8)
#elif (BOARD_PARAM_APB1CLKDIV == 16)
#   define _APB1_DIV    (7<<8)
#else
#   error "BOARD_PARAM_APB1CLKDIV not set to a value permitted by this HW"
#endif

#if (BOARD_PARAM_APB2CLKDIV == 1)
#   define _APB2_DIV    (0<<11)
#elif (BOARD_PARAM_APB2CLKDIV == 2)
#   define _APB2_DIV    (4<<11)
#elif (BOARD_PARAM_APB2CLKDIV == 4)
#   define _APB2_DIV    (5<<11)
#elif (BOARD_PARAM_APB2CLKDIV == 8)
#   define _APB2_DIV    (6<<11)
#elif (BOARD_PARAM_APB2CLKDIV == 16)
#   define _APB2_DIV    (7<<11)
#else
#   error "BOARD_PARAM_APB2CLKDIV not set to a value permitted by this HW"
#endif









/**
  * @}
  */

/** @addtogroup STM32L1xx_System_Private_Functions
  * @{
  */


void SystemInit (void) {
    RCC->CR    |= (uint32_t)0x00000100;     // Set MSION bit
    RCC->CFGR  &= (uint32_t)0x88FFC00C;     // Reset SW[1:0], HPRE[3:0], PPRE1[2:0], PPRE2[2:0], 
                                            //   MCOSEL[2:0], and MCOPRE[2:0] bits
    RCC->CR    &= (uint32_t)0xEEFEFFFE;     // Reset HSION, HSEON, CSSON and PLLON bits
    RCC->CR    &= (uint32_t)0xFFFBFFFF;     // Reset HSEBYP bit
    RCC->CFGR  &= (uint32_t)0xFF02FFFF;     // Reset PLLSRC, PLLMUL[3:0] and PLLDIV[1:0] bits 
    RCC->CIR    = 0x00000000;               // Disable all interrupts

#   ifdef DATA_IN_ExtSRAM
        SystemInit_ExtMemCtl(); 
#   endif /* DATA_IN_ExtSRAM */
    
    // Configure the System clock frequency, AHB/APBx prescalers and Flash settings
    sub_clock_select();

#   ifdef VECT_TAB_SRAM
        SCB->VTOR   = SRAM_BASE | VECT_TAB_OFFSET;  // Vector Table Relocation in Internal SRAM.
#   else
        SCB->VTOR   = FLASH_BASE | VECT_TAB_OFFSET; // Vector Table Relocation in Internal FLASH.
#   endif
}



void SystemBoost(void) {
/// "Boost" is an HSI or HSE based clock mode, using PLL only if necessary.  
/// In the present implementation, the voltage regime must be constant during
/// the operation of the device.
#if MCU_CONFIG(MULTISPEED)
    if (RCC->CR & RCC_CR_MSION) {
        // Add a Flash wait state if necessary (but only if necessary)
#       if (   ((MCU_CONFIG_VOLTLEVEL == 3) && (PLATFORM_MSCLOCK_HZ <= 2000000)) \
        ||  (MCU_CONFIG_VOLTLEVEL == 2) || (MCU_CONFIG_VOLTLEVEL == 1) )
#       if (    ((MCU_CONFIG_VOLTLEVEL == 3) && (PLATFORM_HSCLOCK_HZ > 2000000)) \
            ||  ((MCU_CONFIG_VOLTLEVEL == 2) && (PLATFORM_HSCLOCK_HZ > 8000000)) \
            ||  ((MCU_CONFIG_VOLTLEVEL == 1) && (PLATFORM_HSCLOCK_HZ > 16000000)))
            FLASH->ACR |= FLASH_ACR_LATENCY;
#       endif
#       endif

        // Turn on the HS OSC
        sub_hsosc_config();

        // Turn off MSI because HS clock is now active
        RCC->CR &= ~RCC_CR_MSION;
    }
#endif
}


void SystemUnboost(void) {
/// "Rest" is an MSI-based clock mode
/// Going into STOP will automatically put system into rest (Boost off)
/// In the present implementation, the voltage regime must be constant during
/// the operation of the device.
#if MCU_CONFIG(MULTISPEED)
    if ((RCC->CR & RCC_CR_MSION) == 0) {
        uint32_t counter;
    
        // Turn-on MSI clock and wait for it to be ready
        RCC->CR    |= RCC_CR_MSION;
        counter     = MSI_STARTUP_TIMEOUT;      // MSI startup <= 8us
        while (((RCC->CR & RCC_CR_MSIRDY) == 0) && (--counter != 0));
        
        // Set MSI as system clock by clearing whatever other clock is in use,
        // and wait for the Hardware to acknowledge
        RCC->CFGR  &= ~3;
        while (RCC->CFGR & 0xC);
        
        // Turn off non-MSI clocks to save power
        RCC->CR    &= ~(RCC_CR_PLLON | RCC_CR_HSEON | RCC_CR_HSION);
        
        // Clear the Flash wait state if necessary (but only if necessary)
#       if (    ((MCU_CONFIG_VOLTLEVEL == 3) && (PLATFORM_HSCLOCK_HZ > 2000000)) \
            ||  ((MCU_CONFIG_VOLTLEVEL == 2) && (PLATFORM_HSCLOCK_HZ > 8000000)) \
            ||  ((MCU_CONFIG_VOLTLEVEL == 1) && (PLATFORM_HSCLOCK_HZ > 16000000)))
#       if (   ((MCU_CONFIG_VOLTLEVEL == 3) && (PLATFORM_MSCLOCK_HZ <= 2000000)) \
        ||  (MCU_CONFIG_VOLTLEVEL == 2) || (MCU_CONFIG_VOLTLEVEL == 1) )       
            FLASH->ACR &= ~FLASH_ACR_LATENCY;
#       endif
#       endif
    }
#endif
}



void SystemResume(void) {
/// This should be called after exiting STOP, unless you call SystemBoost().
/// Calling SystemResume() will optimize the system for MSI clock operation. 
/// If your system is designed in a way that exists in a single flash regime,
/// such as 0 wait state, you should still call this function because in these
/// cases it will be compiled into nothing, so there is no penalty.
    sub_msflash_config();
}







/* 
  * @brief  Update SystemCoreClock according to Clock Register Values
  *         The SystemCoreClock variable contains the core clock (HCLK), it can
  *         be used by the user application to setup the SysTick timer or configure
  *         other parameters.
  *           
  * @note   Each time the core clock (HCLK) changes, this function must be called
  *         to update SystemCoreClock variable value. Otherwise, any configuration
  *         based on this variable will be incorrect.         
  *     
  * @note   - The system frequency computed by this function is not the real 
  *           frequency in the chip. It is calculated based on the predefined 
  *           constant and the selected clock source:
  *             
  *           - If SYSCLK source is MSI, SystemCoreClock will contain the MSI 
  *             value as defined by the MSI range.
  *                                   
  *           - If SYSCLK source is HSI, SystemCoreClock will contain the HSI_VALUE(*)
  *                                              
  *           - If SYSCLK source is HSE, SystemCoreClock will contain the HSE_VALUE(**)
  *                          
  *           - If SYSCLK source is PLL, SystemCoreClock will contain the HSE_VALUE(**)
  *             or HSI_VALUE(*) multiplied/divided by the PLL factors.
  *         
  *         (*) HSI_VALUE is a constant defined in stm32l1xx.h file (default value
  *             16 MHz) but the real value may vary depending on the variations
  *             in voltage and temperature.   
  *    
  *         (**) HSE_VALUE is a constant defined in stm32l1xx.h file (default value
  *              8 MHz), user has to ensure that HSE_VALUE is same as the real
  *              frequency of the crystal used. Otherwise, this function may
  *              have wrong result.
  *                
  *         - The result of this function could be not correct when using fractional
  *           value for HSE crystal.
  * @param  None
  * @retval None

void SystemCoreClockUpdate (void) {
    uint32_t tmp        = 0; 
    uint32_t pllmul     = 0;
    uint32_t plldiv     = 0;
    uint32_t pllsource  = 0;
    uint32_t msirange   = 0;

    // Get SYSCLK source
    tmp = RCC->CFGR & RCC_CFGR_SWS;
  
    switch (tmp) {
        // MSI used as system clock
        case 0x00:  msirange        = (RCC->ICSCR & RCC_ICSCR_MSIRANGE) >> 13;
                    SystemCoreClock = (32768 * (1 << (msirange + 1)));
                    break;
        
        // HSI used as system clock
        case 0x04:  SystemCoreClock = HSI_VALUE;
                    break;
        
        // HSE used as system clock
        case 0x08:  SystemCoreClock = HSE_VALUE;
                    break;
        
        // PLL used as system clock
        case 0x0C:  // Get PLL clock source and multiplication factor
                    pllmul  = RCC->CFGR & RCC_CFGR_PLLMUL;
                    plldiv  = RCC->CFGR & RCC_CFGR_PLLDIV;
                    pllmul  = PLLMulTable[(pllmul >> 18)];
                    plldiv  = (plldiv >> 22) + 1;
      
                    pllsource   = RCC->CFGR & RCC_CFGR_PLLSRC;
                    if (pllsource == 0x00) {
                        // HSI oscillator clock selected as PLL clock entry
                        SystemCoreClock = (((HSI_VALUE) * pllmul) / plldiv);
                    }
                    else {
                        // HSE selected as PLL clock entry
                        SystemCoreClock = (((HSE_VALUE) * pllmul) / plldiv);
                    }
                    break;
                    
        default: // MSI used as system clock
            msirange        = (RCC->ICSCR & RCC_ICSCR_MSIRANGE) >> 13;
            SystemCoreClock = (32768 * (1 << (msirange + 1)));
            break;
    }
  
    // Compute HCLK clock frequency
    // Get HCLK prescaler
    tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
    
    // HCLK clock frequency
    SystemCoreClock >>= tmp;
}
  */





void sub_msflash_config(void) {
/// Enable 64-bit flash access (must be done first), and then the 
/// prefetch buffer + 0 or 1 wait states.
    FLASH->ACR |= FLASH_ACR_ACC64;
#   if (   ((MCU_CONFIG_VOLTLEVEL == 3) && (PLATFORM_MSCLOCK_HZ <= 2000000)) \
        ||  (MCU_CONFIG_VOLTLEVEL == 2) || (MCU_CONFIG_VOLTLEVEL == 1) )
        FLASH->ACR |= (FLASH_ACR_PRFTEN);
#   else
        FLASH->ACR |= (FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY);
#   endif
}


void sub_hsflash_config(void) {
/// Enable 64-bit flash access (must be done first), and then the 
/// prefetch buffer + 0 or 1 wait states.
    FLASH->ACR |= FLASH_ACR_ACC64;
#   if (    ((MCU_CONFIG_VOLTLEVEL == 3) && (PLATFORM_HSCLOCK_HZ <= 2000000)) \
        ||  ((MCU_CONFIG_VOLTLEVEL == 2) && (PLATFORM_HSCLOCK_HZ <= 8000000)) \
        ||  ((MCU_CONFIG_VOLTLEVEL == 1) && (PLATFORM_HSCLOCK_HZ <= 16000000)))
        FLASH->ACR |= (FLASH_ACR_PRFTEN);
#   else
        FLASH->ACR |= (FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY);
#   endif
}


void sub_voltage_config(void) {
    // Set Power Configuration based on Voltage Level parameters
    RCC->APB1ENR   |= RCC_APB1ENR_PWREN;    // Power enable
#   if (MCU_CONFIG_VOLTLEVEL == 3)
    PWR->CR         = PWR_CR_VOS_1V2;
#   elif (MCU_CONFIG_VOLTLEVEL == 2)
    PWR->CR         = PWR_CR_VOS_1V5;
#   elif (MCU_CONFIG_VOLTLEVEL == 1)
    PWR->CR         = PWR_CR_VOS_1V8;
#   else
#   error "MCU_CONFIG_VOLTLEVEL do not have compatible settings"
#   endif 
    
    // Wait Until the Voltage Regulator is ready
    while((PWR->CSR & PWR_CSR_VOSF) != RESET) { }
}


void sub_hsosc_config(void) {
    // Configure PLL only if required (and Setup the Bus Dividers as specified)
#   if (PLATFORM_HSCLOCK_HZ != BOARD_PARAM_HFHz)
    RCC->CFGR  &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV));
    RCC->CFGR  |= (uint32_t)(_PLL_SRC | _PLL_MULT | _PLL_DIV | _AHB_DIV | _APB1_DIV | _APB2_DIV);
    RCC->CR    |= RCC_CR_PLLON;
    while((RCC->CR & RCC_CR_PLLRDY) == 0) { }       
        
    // Select PLL as system clock source, Wait until PLL is used as system clock source
    RCC->CFGR  &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
    RCC->CFGR  |= (uint32_t)RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL) { }

    // Configure HSE as clock source (and Setup the Bus Dividers as specified)
#   elif (BOARD_FEATURE_HFXTAL == ENABLED)
    RCC->CFGR  |= (RCC_CFGR_SW_HSE | _AHB_DIV | _APB1_DIV | _APB2_DIV);
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_HSE) { }
    
    // Configure HSI as clock source (and Setup the Bus Dividers as specified)
#   else 
    RCC->CFGR  |= (RCC_CFGR_SW_HSI | _AHB_DIV | _APB1_DIV | _APB2_DIV);
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_HSE) { }

#   endif
}


void sub_clock_select() {
    // Configure the System clock frequency, AHB/APBx prescalers and Flash settings
#   if MCU_CONFIG(MULTISPEED)
#       if ((PLATFORM_MSCLOCK_HZ == 4200000)   \
         || (PLATFORM_MSCLOCK_HZ == 2100000)   \
         || (PLATFORM_MSCLOCK_HZ == 1050000)   \
         || (PLATFORM_MSCLOCK_HZ == 524000)    \
         || (PLATFORM_MSCLOCK_HZ == 262000)    \
         || (PLATFORM_MSCLOCK_HZ == 131000)    \
         || (PLATFORM_MSCLOCK_HZ == 655000)  ))
            Set_MSI_Clock();
#       else
#           error "PLATFORM_MSCLOCK_HZ is not set to a value matching HW options"
#       endif

    // Use HF XTAL as primary clock
#   elif (BOARD_FEATURE_HFXTAL == ENABLED)
        Set_HSE_Clock();
    
    // Use HF Internal osc as primary clock
#   elif (  (PLATFORM_HSCLOCK_HZ == 32000000)       \
         || (PLATFORM_HSCLOCK_HZ == 16000000)       \
         || (PLATFORM_HSCLOCK_HZ == 8000000)        \
         || (PLATFORM_HSCLOCK_HZ == 4000000)        \
         || (PLATFORM_HSCLOCK_HZ == 2000000) ))
        Set_HSI_Clock();
        
#   else
#       error "PLATFORM_HSCLOCK_HZ is not set to a value matching HW options"

#   endif
}


void sub_hsclock_config() {
#   if (BOARD_FEATURE_HFXTAL == ENABLED)
#       define _PLL_SRC RCC_CFGR_PLLSRC_HSE
#   else
#       define _PLL_SRC RCC_CFGR_PLLSRC_HSI
#   endif
    sub_hsflash_config();
    sub_voltage_config();
    sub_hsosc_config();
}



/**
  * @brief  Configures the System clock frequency, AHB/APBx prescalers and Flash 
  *         settings.
  * @note   This function should be called only once the RCC clock configuration  
  *         is reset to the default reset state (done in SystemInit() function).
  * @param  None
  * @retval None
  */
void Set_HSE_Clock(void) {
    uint32_t counter;
  
    // SYSCLK, HCLK, PCLK2 and PCLK1 configuration
    // Enable HSE
    RCC->CR |= ((uint32_t)RCC_CR_HSEON);
 
    counter = HSE_STARTUP_TIMEOUT
    while (((RCC->CR & RCC_CR_HSERDY) == 0) && (--counter != 0));

    if (RCC->CR & RCC_CR_HSIRDY) {
        sub_hsclock_config();
    }
    else {
        // If HSE fails to start-up, the application will have wrong clock
        // configuration. User can add here some code to deal with this error 
    }
}



void Set_HSI_Clock(void) {
    uint32_t counter;
  
    // SYSCLK, HCLK, PCLK2 and PCLK1 configuration: Enable HSI
    RCC->CR |= ((uint32_t)RCC_CR_HSION);
 
    // Wait until HSI is ready and if Time out is reached exit
    counter = HSI_STARTUP_TIMEOUT
    while (((RCC->CR & RCC_CR_HSIRDY) == 0) && (--counter != 0));

    if (RCC->CR & RCC_CR_HSIRDY) {
        sub_hsclock_config();
    }
    else {
        // If HSE fails to start-up, the application will have wrong clock
        // configuration. User can add here some code to deal with this error 
    }
}



void Set_MSI_Clock(void) {
/// MSI is already started, and at init it is 2.1 MHz.  This function will 
/// change the divider to a different setting from 2.1MHz.

    sub_msflash_config();
    sub_voltage_config();
    
    // Setup the Bus Dividers as specified (MSI already selected as system clock)
    RCC->CFGR  |= (_AHB_DIV | _APB1_DIV | _APB2_DIV);

    // Change MSI to required frequency
#   if ((PLATFORM_MSCLOCK_HZ == 4200000)
    RCC->ICSCR ^= 0x00006000;           //setting 110
#   elif (PLATFORM_MSCLOCK_HZ == 2100000)
                                        //setting 101 (default)
#   elif (PLATFORM_MSCLOCK_HZ == 1050000)
    RCC->ICSCR &= ~0x00006000;          //setting 100
#   elif (PLATFORM_MSCLOCK_HZ == 524000)
    RCC->ICSCR ^= 0x0000C000;           //setting 011
#   elif (PLATFORM_MSCLOCK_HZ == 262000)
    RCC->ICSCR ^= 0x0000E000;           //setting 010
#   elif (PLATFORM_MSCLOCK_HZ == 131000)
    RCC->ICSCR &= ~0x0000C000;          //setting 001
#   elif (PLATFORM_MSCLOCK_HZ == 655000)
    RCC->ICSCR &= ~0x0000E000;          //setting 000
#   endif
}





#ifdef DATA_IN_ExtSRAM
/**
  * @brief  Setup the external memory controller.
  *         Called in SystemInit() function before jump to main.
  *         This function configures the external SRAM mounted on STM32L152D_EVAL board
  *         This SRAM will be used as program data memory (including heap and stack).
  * @param  None
  * @retval None
  */
void SystemInit_ExtMemCtl(void)
{
/*-- GPIOs Configuration -----------------------------------------------------*/
/*
 +-------------------+--------------------+------------------+------------------+
 +                       SRAM pins assignment                                   +
 +-------------------+--------------------+------------------+------------------+
 | PD0  <-> FSMC_D2  | PE0  <-> FSMC_NBL0 | PF0  <-> FSMC_A0 | PG0 <-> FSMC_A10 |
 | PD1  <-> FSMC_D3  | PE1  <-> FSMC_NBL1 | PF1  <-> FSMC_A1 | PG1 <-> FSMC_A11 |
 | PD4  <-> FSMC_NOE | PE7  <-> FSMC_D4   | PF2  <-> FSMC_A2 | PG2 <-> FSMC_A12 |
 | PD5  <-> FSMC_NWE | PE8  <-> FSMC_D5   | PF3  <-> FSMC_A3 | PG3 <-> FSMC_A13 |
 | PD8  <-> FSMC_D13 | PE9  <-> FSMC_D6   | PF4  <-> FSMC_A4 | PG4 <-> FSMC_A14 |
 | PD9  <-> FSMC_D14 | PE10 <-> FSMC_D7   | PF5  <-> FSMC_A5 | PG5 <-> FSMC_A15 |
 | PD10 <-> FSMC_D15 | PE11 <-> FSMC_D8   | PF12 <-> FSMC_A6 | PG10<-> FSMC_NE2 |
 | PD11 <-> FSMC_A16 | PE12 <-> FSMC_D9   | PF13 <-> FSMC_A7 |------------------+
 | PD12 <-> FSMC_A17 | PE13 <-> FSMC_D10  | PF14 <-> FSMC_A8 | 
 | PD13 <-> FSMC_A18 | PE14 <-> FSMC_D11  | PF15 <-> FSMC_A9 | 
 | PD14 <-> FSMC_D0  | PE15 <-> FSMC_D12  |------------------+
 | PD15 <-> FSMC_D1  |--------------------+ 
 +-------------------+
*/

  /* Enable GPIOD, GPIOE, GPIOF and GPIOG interface clock */
  RCC->AHBENR   = 0x000080D8;
  
  /* Connect PDx pins to FSMC Alternate function */
  GPIOD->AFR[0]  = 0x00CC00CC;
  GPIOD->AFR[1]  = 0xCCCCCCCC;
  /* Configure PDx pins in Alternate function mode */  
  GPIOD->MODER   = 0xAAAA0A0A;
  /* Configure PDx pins speed to 40 MHz */  
  GPIOD->OSPEEDR = 0xFFFF0F0F;
  /* Configure PDx pins Output type to push-pull */  
  GPIOD->OTYPER  = 0x00000000;
  /* No pull-up, pull-down for PDx pins */ 
  GPIOD->PUPDR   = 0x00000000;

  /* Connect PEx pins to FSMC Alternate function */
  GPIOE->AFR[0]  = 0xC00000CC;
  GPIOE->AFR[1]  = 0xCCCCCCCC;
  /* Configure PEx pins in Alternate function mode */ 
  GPIOE->MODER   = 0xAAAA800A;
  /* Configure PEx pins speed to 40 MHz */ 
  GPIOE->OSPEEDR = 0xFFFFC00F;
  /* Configure PEx pins Output type to push-pull */  
  GPIOE->OTYPER  = 0x00000000;
  /* No pull-up, pull-down for PEx pins */ 
  GPIOE->PUPDR   = 0x00000000;

  /* Connect PFx pins to FSMC Alternate function */
  GPIOF->AFR[0]  = 0x00CCCCCC;
  GPIOF->AFR[1]  = 0xCCCC0000;
  /* Configure PFx pins in Alternate function mode */   
  GPIOF->MODER   = 0xAA000AAA;
  /* Configure PFx pins speed to 40 MHz */ 
  GPIOF->OSPEEDR = 0xFF000FFF;
  /* Configure PFx pins Output type to push-pull */  
  GPIOF->OTYPER  = 0x00000000;
  /* No pull-up, pull-down for PFx pins */ 
  GPIOF->PUPDR   = 0x00000000;

  /* Connect PGx pins to FSMC Alternate function */
  GPIOG->AFR[0]  = 0x00CCCCCC;
  GPIOG->AFR[1]  = 0x00000C00;
  /* Configure PGx pins in Alternate function mode */ 
  GPIOG->MODER   = 0x00200AAA;
  /* Configure PGx pins speed to 40 MHz */ 
  GPIOG->OSPEEDR = 0x00300FFF;
  /* Configure PGx pins Output type to push-pull */  
  GPIOG->OTYPER  = 0x00000000;
  /* No pull-up, pull-down for PGx pins */ 
  GPIOG->PUPDR   = 0x00000000;
  
/*-- FSMC Configuration ------------------------------------------------------*/
  /* Enable the FSMC interface clock */
  RCC->AHBENR    = 0x400080D8;

  /* Configure and enable Bank1_SRAM3 */
  FSMC_Bank1->BTCR[4]  = 0x00001011;
  FSMC_Bank1->BTCR[5]  = 0x00000300;
  FSMC_Bank1E->BWTR[4] = 0x0FFFFFFF;
/*
  Bank1_SRAM3 is configured as follow:

  p.FSMC_AddressSetupTime = 0;
  p.FSMC_AddressHoldTime = 0;
  p.FSMC_DataSetupTime = 3;
  p.FSMC_BusTurnAroundDuration = 0;
  p.FSMC_CLKDivision = 0;
  p.FSMC_DataLatency = 0;
  p.FSMC_AccessMode = FSMC_AccessMode_A;

  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM3;
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);
*/
  
}
#endif /* DATA_IN_ExtSRAM */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2012 STMicroelectronics *****END OF FILE****/
