/*
Common part of the linker scripts for STR71x devices in FLASH mode
(that is, the FLASH is seen at 0)
Copyright RAISONANCE 2005
You can use, modify and distribute thisfile freely, but without any waranty.
*/

/* avoid link error if ___SK_limit___ is not defined at all */
PROVIDE ( ___SK_limit___ = 0 ) ;


/* Sections Definitions */

SECTIONS
{

    /* for Cortex devices, the beginning of the startup code is stored in the .isr_vector section, which goes to FLASH */
    .isr_vector :
    {
	. = ALIGN(4);
        KEEP(*(.isr_vector))            /* Startup code */
	. = ALIGN(4);
    } >FLASH
    
    /* for Cortex LPC devices, the CRP forces to have a separate region for the vectors at the beginning of the falsh */
    .flash_start :
    {
	. = ALIGN(4);
        KEEP(*(.flash_start))            /* vectors */
        KEEP(*(.flash_start.*))          /* rest of startup */
	. = ALIGN(4);
    } >STARTFLASH
    
    /* for Cortex LPC devices, the CRP contains a code to protect the flash */
    .crp_patch :
    {
	. = ALIGN(4);
        KEEP(*(.crp_patch))            /* CRP word */
	. = ALIGN(4);
    } >CRPPATCH
 
    /* for some STRx devices, the beginning of the startup code is stored in the .flashtext section, which goes to FLASH */
    .flashtext :
    {
	. = ALIGN(4);
        *(.flashtext)            /* Startup code */
	. = ALIGN(4);
    } >FLASH

    
    /* the program code is stored in the .text section, which goes to Flash */
    .text :
    {
	    . = ALIGN(4);
	    
        *(.text)                   /* remaining code */
        *(.text.*)                   /* remaining code */
        *(.rodata)                 /* read-only data (constants) */
        *(.rodata.*)
        *(.glue_7)
        *(.glue_7t)

    /* now make sure we keep the C++ constructors */
	*(.eh_frame)
    . = ALIGN(4);
    KEEP (*(.init))
    . = ALIGN(4);
    KEEP (*(.fini))

    . = ALIGN(4);
    __preinit_array_start = .;
    KEEP (*(.preinit_array))
    __preinit_array_end = .;

    . = ALIGN(4);
    __init_array_start = .;
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    __init_array_end = .;


    . = ALIGN(4);
    __fini_array_start = .;
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    __fini_array_end = .;

    . = ALIGN(0x4);
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*crtend.o(.ctors))

    . = ALIGN(0x4);
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*crtend.o(.dtors))

	    . = ALIGN(4);
        _etext = .;
    } >FLASH
    

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >FLASH
    
    .ARM :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } >FLASH
    
    
	/* This is used by the startup in order to initialize the .data secion */
    _sidata = .;

 
    /* for some LPC devices, there is a FLASH patch to place at a specified address */
    .flashpatch :
    {
	. = ALIGN(4);
        KEEP(*(.flashpatch))            /* flashpatch data */
	. = ALIGN(4);
    } >FLASHPATCH
    
    /* for some LPC devices, there is a FLASH patch to place at a specified address
    and then there is the rest of the flash */
    .endflash :
    {
	. = ALIGN(4);
        *(.endflash)            /* endflash code */
	. = ALIGN(4);
	_e_endflash = . ;
    } >ENDFLASH

    
    /* for some devices, there is FLASH config to place at a specified address*/
    .config :
    {
	. = ALIGN(4);
        KEEP(*(.config))            /* config data */
	. = ALIGN(4);
    } > CONFIG
    
    /* for STM32 devices, there are FLASH Option Bytes to place at a specified address*/
    .optionbytes :
    {
	. = ALIGN(4);
        KEEP(*(.optionbytes))            /* option bytes data */
	. = ALIGN(4);
	_e_config = . ;
    } > CONFIG
    
    /* for some devices, there are two config regions*/
    .config2 :
    {
	. = ALIGN(4);
        KEEP(*(.config2))            /* config data */
	. = ALIGN(4);
    } > CONFIG2
    
    /* for some STM32 devices, there are two Option Bytes regions*/
    .optionbytes2 :
    {
	. = ALIGN(4);
        KEEP(*(.optionbytes2))            /* option bytes data */
	. = ALIGN(4);
	_e_config2 = . ;
    } > CONFIG2
    
    
    /* This is the initialized data section
    The program executes knowing that the data is in the RAM
    but the loader puts the initial values in the FLASH (inidata).
    It is one task of the startup to copy the initial values from FLASH to RAM. */
    .data  : AT ( _sidata )
    {
	    . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data secion */
        _sdata = . ;
        _data = . ;
        __data_start__ = .;

        KEEP(*(.ram_start1))
		. = ALIGN(4);
        KEEP(*(.ram_start2))
		. = ALIGN(4);
        KEEP(*(.ram_start3))
		. = ALIGN(4);
        
        *(.data)
        *(.data.*)
        *(.RAMtext)

	    . = ALIGN(4);
	    /* This is used by the startup in order to initialize the .data secion */
        _edata = . ;
        __data_end__ = .;
    } >RAM
    
    /*calculate end address of idata*/
    _eidata = _sidata + (_edata - _sdata);
    
    /*check that idata remains in FLASH region*/
    _lastdataromaddress = ORIGIN(FLASH) + LENGTH(FLASH);
    ___toobig___ = ( _eidata > _lastdataromaddress) ? 1 : 0 ;
    ___SK_limit_msg___ = ( ___toobig___ & ___SK_limit___) ? "!!! RKit-ARM Lite limit reached. See !Starter Kit Limited! linker option !!!" : 0 ;
    ___toobigmsg___ = ( ___toobig___ ) ? "!!!!!!!!! FLASH IS FULL !!!!!!!!!" : 0 ;
    
    
    /* This is the uninitialized data section */
    .bss :
    {
	    . = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss secion */
        _sbss = .;
        _bss = .;
        __bss_start__ = .;
        
        *(.bss)
        *(.bss.*)
        *(COMMON)
        
	    . = ALIGN(4);
	    /* This is used by the startup in order to initialize the .bss secion */
        _ebss = . ;
        __bss_end__ = .;
    } >RAM
    
    PROVIDE ( end = _ebss );
    PROVIDE ( _end = _ebss );
    
    /* This is the user stack section 
    This is just to check that there is enough RAM left for the User mode stack
    It should generate an error if it's full.
     */
    ._usrstack :
    {
	    . = ALIGN(4);
        _susrstack = . ;
        
        . = . + _Minimum_Stack_Size ;
        
	    . = ALIGN(4);
        _eusrstack = . ;
    } >RAM

    

    /* this is the FLASH Bank1 */
    /* the C or assembly source must explicitly place the code or data there
    using the "section" attribute */
    .b1text :
    {
        *(.b1text)                   /* remaining code */
        *(.b1rodata)                 /* read-only data (constants) */
        *(.b1rodata*)
    } >FLASHB1

    /* this is the EXTMEM */
    /* the C or assembly source must explicitly place the code or data there
    using the "section" attribute */
    
    /* EXTMEM Bank0 */
    .eb0text :
    {
        *(.eb0text)                   /* remaining code */
        *(.eb0rodata)                 /* read-only data (constants) */
        *(.eb0rodata*)
    } >EXTMEMB0
    
    /* EXTMEM Bank1 */
    .eb1text :
    {
        *(.eb1text)                   /* remaining code */
        *(.eb1rodata)                 /* read-only data (constants) */
        *(.eb1rodata*)
    } >EXTMEMB1
    
    /* EXTMEM Bank2 */
    .eb2text :
    {
        *(.eb2text)                   /* remaining code */
        *(.eb2rodata)                 /* read-only data (constants) */
        *(.eb2rodata*)
    } >EXTMEMB2
    
    /* EXTMEM Bank0 */
    .eb3text :
    {
        *(.eb3text)                   /* remaining code */
        *(.eb3rodata)                 /* read-only data (constants) */
        *(.eb3rodata*)
    } >EXTMEMB3
    

    /* after that it's only debugging information. */
    
    /* Remove debug information from the standard libraries */
    /DISCARD/ :
    {
        libc.a ( * )
        libc_s.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 : { *(.ARM.attributes) } 
}


