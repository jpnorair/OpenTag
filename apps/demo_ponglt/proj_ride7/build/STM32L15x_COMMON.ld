/*
Common part of the linker scripts for STM32 devices
Copyright RAISONANCE 2009
You can use, modify and distribute this file freely, but without any waranty.
*/


/* default stack sizes. 

These are used by the startup in order to allocate stacks for the different modes.
*/

__Stack_Size = 1024 ;

PROVIDE ( _Stack_Size = __Stack_Size ) ;

__Stack_Init = _estack  - __Stack_Size ;

/*"PROVIDE" allows to easily override these values from an object file or the commmand line.*/
PROVIDE ( _Stack_Init = __Stack_Init ) ;

/*
There will be a link error if there is not this amount of RAM free at the end.
*/
PROVIDE ( _Minimum_Stack_Size = 0x100 );



/*
this sends all unreferenced IRQHandlers to reset
*/


PROVIDE (   Undefined_Handler = 0 ) ;
PROVIDE (   SWI_Handler = 0 ) ;
PROVIDE (   IRQ_Handler = 0 ) ;
PROVIDE (   Prefetch_Handler = 0 ) ;
PROVIDE (   Abort_Handler = 0 ) ;
PROVIDE (   FIQ_Handler = 0 ) ;

PROVIDE (   NMI_Handler = 0 ) ;
PROVIDE (   HardFault_Handler = 0 ) ;
PROVIDE (   MemManage_Handler = 0 ) ;
PROVIDE (   BusFault_Handler = 0 ) ;
PROVIDE (   UsageFault_Handler = 0 ) ;
PROVIDE (   SVC_Handler = 0 ) ;
PROVIDE (   DebugMon_Handler = 0 ) ;
PROVIDE (   PendSVC_Handler = 0 ) ;
PROVIDE (   SysTick_Handler = 0 ) ;
PROVIDE (   WWDG_IRQHandler = 0 ) ;
PROVIDE (   PVD_IRQHandler = 0 ) ;
PROVIDE (   TAMPER_IRQHandler = 0 ) ;
PROVIDE (   RTC_WKUP_IRQHandler = 0 ) ;
PROVIDE (   FLASH_IRQHandler = 0 ) ;
PROVIDE (   RCC_IRQHandler = 0 ) ;
PROVIDE (   EXTI0_IRQHandler = 0 ) ;
PROVIDE (   EXTI1_IRQHandler = 0 ) ;
PROVIDE (   EXTI2_IRQHandler = 0 ) ;
PROVIDE (   EXTI3_IRQHandler = 0 ) ;
PROVIDE (   EXTI4_IRQHandler = 0 ) ;
PROVIDE (   DMA1_Channel1_IRQHandler = 0 ) ;
PROVIDE (   DMA1_Channel2_IRQHandler = 0 ) ;
PROVIDE (   DMA1_Channel3_IRQHandler = 0 ) ;
PROVIDE (   DMA1_Channel4_IRQHandler = 0 ) ;
PROVIDE (   DMA1_Channel5_IRQHandler = 0 ) ;
PROVIDE (   DMA1_Channel6_IRQHandler = 0 ) ;
PROVIDE (   DMA1_Channel7_IRQHandler = 0 ) ;
PROVIDE (   ADC1_IRQHandler = 0 ) ;
PROVIDE (   USB_HP_CAN_TX_IRQHandler = 0 ) ;
PROVIDE (   USB_LP_CAN_RX0_IRQHandler = 0 ) ;
PROVIDE (   CAN_RX1_IRQHandler = 0 ) ;
PROVIDE (   CAN_SCE_IRQHandler = 0 ) ;
PROVIDE (   EXTI9_5_IRQHandler = 0 ) ;
PROVIDE (   TIM1_BRK_IRQHandler = 0 ) ;
PROVIDE (   TIM1_UP_IRQHandler = 0 ) ;
PROVIDE (   TIM1_TRG_COM_IRQHandler = 0 ) ;
PROVIDE (   TIM1_CC_IRQHandler = 0 ) ;
PROVIDE (   TIM2_IRQHandler = 0 ) ;
PROVIDE (   TIM3_IRQHandler = 0 ) ;
PROVIDE (   TIM4_IRQHandler = 0 ) ;
PROVIDE (   I2C1_EV_IRQHandler = 0 ) ;
PROVIDE (   I2C1_ER_IRQHandler = 0 ) ;
PROVIDE (   I2C2_EV_IRQHandler = 0 ) ;
PROVIDE (   I2C2_ER_IRQHandler = 0 ) ;
PROVIDE (   SPI1_IRQHandler = 0 ) ;
PROVIDE (   SPI2_IRQHandler = 0 ) ;
PROVIDE (   USART1_IRQHandler = 0 ) ;
PROVIDE (   USART2_IRQHandler = 0 ) ;
PROVIDE (   USART3_IRQHandler = 0 ) ;
PROVIDE (   EXTI15_10_IRQHandler = 0 ) ;
PROVIDE (   RTCAlarm_IRQHandler = 0 ) ;
PROVIDE (   USBWakeUp_IRQHandler = 0 ) ;



/******************************************************************************/
/*                       Peripheral memory map                                */
/******************************************************************************/
/*this allows to compile the ST lib in "non-debug" mode*/


/* Peripheral and SRAM base address in the alias region */
PERIPH_BB_BASE        = 0x42000000;
SRAM_BB_BASE          = 0x22000000;

/* Peripheral and SRAM base address in the bit-band region */
SRAM_BASE             = 0x20000000;
PERIPH_BASE           = 0x40000000;

/* Flash registers base address */
PROVIDE ( FLASH_BASE            = 0x40023C00);
/* Flash Option Bytes base address */
PROVIDE ( OB_BASE               = 0x1FF80000);

/* Peripheral memory map */
APB1PERIPH_BASE      = PERIPH_BASE  ;
APB2PERIPH_BASE      = (PERIPH_BASE + 0x10000) ;
AHBPERIPH_BASE       = (PERIPH_BASE + 0x20000) ;

PROVIDE ( TIM2            = (APB1PERIPH_BASE + 0x0000) ) ;
PROVIDE ( TIM3            = (APB1PERIPH_BASE + 0x0400) ) ;
PROVIDE ( TIM4            = (APB1PERIPH_BASE + 0x0800) ) ;
PROVIDE ( TIM6            = (APB1PERIPH_BASE + 0x1000) ) ;
PROVIDE ( TIM7            = (APB1PERIPH_BASE + 0x1400) ) ;
PROVIDE ( LCD             = (APB1PERIPH_BASE + 0x2400) ) ;
PROVIDE ( RTC             = (APB1PERIPH_BASE + 0x2800) ) ;
PROVIDE ( WWDG            = (APB1PERIPH_BASE + 0x2C00) ) ;
PROVIDE ( IWDG            = (APB1PERIPH_BASE + 0x3000) ) ;
PROVIDE ( SPI2            = (APB1PERIPH_BASE + 0x3800) ) ;
PROVIDE ( USART2          = (APB1PERIPH_BASE + 0x4400) ) ;
PROVIDE ( USART3          = (APB1PERIPH_BASE + 0x4800) ) ;
PROVIDE ( I2C1            = (APB1PERIPH_BASE + 0x5400) ) ;
PROVIDE ( I2C2            = (APB1PERIPH_BASE + 0x5800) ) ;
PROVIDE ( PWR             = (APB1PERIPH_BASE + 0x7000) ) ;
PROVIDE ( DAC             = (APB1PERIPH_BASE + 0x7400) ) ;
PROVIDE ( COMP            = (APB1PERIPH_BASE + 0x7C00) ) ;
PROVIDE ( RI              = (APB1PERIPH_BASE + 0x7C04) ) ;

PROVIDE ( SYSCFG          = (APB2PERIPH_BASE + 0x0000) ) ;
PROVIDE ( EXTI            = (APB2PERIPH_BASE + 0x0400) ) ;
PROVIDE ( TIM9            = (APB2PERIPH_BASE + 0x0800) ) ;
PROVIDE ( TIM10           = (APB2PERIPH_BASE + 0x0C00) ) ;
PROVIDE ( TIM11           = (APB2PERIPH_BASE + 0x1000) ) ;
PROVIDE ( ADC1            = (APB2PERIPH_BASE + 0x2400) ) ;
PROVIDE ( ADC             = (APB2PERIPH_BASE + 0x2700) ) ;
PROVIDE ( TIM1            = (APB2PERIPH_BASE + 0x2C00) ) ;
PROVIDE ( SPI1            = (APB2PERIPH_BASE + 0x3000) ) ;
PROVIDE ( USART1          = (APB2PERIPH_BASE + 0x3800) ) ;

PROVIDE ( GPIOA           = (AHBPERIPH_BASE + 0x0000) ) ;
PROVIDE ( GPIOB			  = (AHBPERIPH_BASE + 0x0400) ) ;
PROVIDE ( GPIOC			  = (AHBPERIPH_BASE + 0x0800) ) ;
PROVIDE ( GPIOD			  = (AHBPERIPH_BASE + 0x0C00) ) ;
PROVIDE ( GPIOE			  = (AHBPERIPH_BASE + 0x1000) ) ;
PROVIDE ( GPIOF    		  = (AHBPERIPH_BASE + 0x1400) ) ;
PROVIDE ( CRC    		  = (AHBPERIPH_BASE + 0x3000) ) ;
PROVIDE ( RCC             = (AHBPERIPH_BASE + 0x3800) ) ;

DMA1_BASE            = (AHBPERIPH_BASE + 0x6000);

PROVIDE ( DMA_Channel1    = (DMA1_BASE + 0x0008) ) ;
PROVIDE ( DMA_Channel2    = (DMA1_BASE + 0x001C) ) ;
PROVIDE ( DMA_Channel3    = (DMA1_BASE + 0x0030) ) ;
PROVIDE ( DMA_Channel4    = (DMA1_BASE + 0x0044) ) ;
PROVIDE ( DMA_Channel5    = (DMA1_BASE + 0x0058) ) ;
PROVIDE ( DMA_Channel6    = (DMA1_BASE + 0x006C) ) ;
PROVIDE ( DMA_Channel7    = (DMA1_BASE + 0x0080) ) ;

/* System Control Space memory map */
SCS_BASE              = 0xE000E000;

PROVIDE ( SysTick         = (SCS_BASE + 0x0010) ) ;
PROVIDE ( NVIC            = (SCS_BASE + 0x0100) ) ;
PROVIDE ( SCB             = (SCS_BASE + 0x0D00) ) ;

