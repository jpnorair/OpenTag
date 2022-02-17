/*
******************************************************************************
**
**  File        : stm32wl55xc_64_16_CMOPLUS.ld
**
**  Author      : STM32CubeIDE
**
**  Abstract    : Linker script for STM32WL55xC Device
**                      64Kbytes FLASH
**                      16Kbytes RAM
**                      Sections aligned to back of each memory space
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**  Distribution: The file is distributed as is without any warranty
**                of any kind.
**
*****************************************************************************
** @attention
**
** Copyright (c) 2021 STMicroelectronics.
** All rights reserved.
**
** This software is licensed under terms that can be found in the LICENSE file
** in the root directory of this software component.
** If no LICENSE file comes with this software, it is provided AS-IS.
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* DEFAULT STACK SIZES: */
/* used by the startup in order to allocate stacks for the different modes. */
_Min_Heap_Size = 512 ;                  /* required amount of heap  */
_Min_Stack_Size = 1024 ;                /* required amount of stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);    /* end of "RAM" Ram type memory */

/* start of veelite fs */
_sveelite   = ORIGIN(FLASHOV);

/* Memories definition */
MEMORY
{
    RAM    (xrw)    : ORIGIN = 0x2000C000, LENGTH = 12K
    FSRAM  (xrw)    : ORIGIN = 0x2000F000,  LENGTH = 4K
    FLASH   (rx)    : ORIGIN = 0x08030000, LENGTH = 60K
  
    /* This section for Veelite FS backed in FLASH and cached in SRAM */
    /* We use the upper 4KB of FLASH Bank0 for this purpose */
    FLASHOV (r)     : ORIGIN = 0x0803F000,  LENGTH = 0x360
    FLASHISFS (r)   : ORIGIN = 0x0803F360,  LENGTH = 0xA0
  /*FLASHGFB (r)    : ORIGIN = 0x0803F400,  LENGTH = 0x400*/
    FLASHISF (r)    : ORIGIN = 0x0803F400,  LENGTH = 0xC00
    
    /* This section for Option Bytes, a.k.a parameter rom */
    CONFIG (r)      : ORIGIN = 0x1FFF7800,   LENGTH = 2K
    
    /* This section for OTP Data */
    OTP (r)         : ORIGIN = 0x1FFF7400,   LENGTH = 1K

	/* This section for System Flash, which can be booted-into by CPU2.  */
	/* From factory it contains the ST factory bootloader */
    SYSBOOT (rx)    : ORIGIN = 0x1FFF7400,   LENGTH = 28K
}

/* Sections */
SECTIONS
{
  /* The startup code into "FLASH" Rom type memory */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data into "FLASH" Rom type memory */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data into "FLASH" Rom type memory */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab   : {
    . = ALIGN(4);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } >FLASH

  .ARM : {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } >FLASH

  .preinit_array     :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .init_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .fini_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } >FLASH

  /* Used by the startup to initialize data */
  _sidata = LOADADDR(.data);
  
  /* FLASH Option Bytes (2KB) are configuration ROM */
  .optionbytes :
  {
    . = ALIGN(4);
    KEEP(*(.optionbytes))
    . = ALIGN(4);
  } > CONFIG
  
  /* OTP Section (1KB) is user one-time programmable, 64 bit aligned */
  .otp :
  {
    . = ALIGN(8);
    KEEP(*(.otp))
    . = ALIGN(8);
  } > OTP
  
  /* SYSBOOT Section (28KB) is generally read-only, but it is possible to use it */
  .sysboot :
  {
    . = ALIGN(4);
    KEEP(*(.sysboot))
    . = ALIGN(4);
  } > SYSBOOT
  
  
  /* VEELITE FS Sections */
    .vl_ov : {
	    . = ALIGN(4);
		KEEP(*(.vl_ov))
	} > FLASHOV
	
	.vl_isfs : {
	    . = ALIGN(4);
		KEEP(*(.vl_isfs))
		. = ALIGN(4);
	} > FLASHISFS
	
/*	.vl_gfb : {
	    . = ALIGN(4);
		KEEP(*(.vl_gfb))
		. = ALIGN(4);
	} > FLASHGFB
*/
	
	.vl_isf : {
	    . = ALIGN(4);
		KEEP(*(.vl_isf))
		. = ALIGN(4);
	} > FLASHISF
    
    .vl_sram : {
	    . = ALIGN(4);
		KEEP(*(.vl_sram))
		. = ALIGN(4);
	} > FSRAM
    

  /* Initialized data sections into "RAM" Ram type memory */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */

  } >RAM AT> FLASH

  /* Uninitialized data section into "RAM" Ram type memory */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  /* User_heap_stack section, used to check that there is enough "RAM" Ram  type memory left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /* Remove information from the compiler libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
