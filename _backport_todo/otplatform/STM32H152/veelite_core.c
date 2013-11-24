#include "OT_types.h"
#include "OT_config.h"
#include "OT_platform.h"

#include "OTAPI.h"              // for logging faults

#include "veelite_core.h"

#if (VSRAM_SIZE > 0)
    static ot_u16 vsram[ (VSRAM_SIZE/2) ];
#else
#error VSRAM_SIZE_is_zero
#endif

volatile ot_u16*  _vworm;

ot_u16 vworm_read(vaddr addr) {
/*    ot_u16 ret;
    ret = _vworm[addr >> 1];
    //debug_printf("%04x = vworm_read(%04x) at %p\r\n", ret, addr, &_vworm[addr >> 1]);
    return ret;*/
    if (addr > 4095) {
        /* STM32L1xx: reading beyond memory would probably cause hard fault */
        for (;;)
            asm("nop"); // this device only has 4k of eeprom
    }

    return _vworm[addr >> 1];
}

ot_u8 vworm_write(vaddr addr, ot_u16 data) {
    ot_u32 Address;
    FLASH_Status status;

#ifdef RADIO_DEBUG   // 18
    debug_printf("vworm_write(%x, %x)\r\n", addr, data);
#endif /* RADIO_DEBUG */
    DATA_EEPROM_Unlock();

    /* Clear all pending flags */
    FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
                            FLASH_FLAG_SIZERR | FLASH_FLAG_OPTVERR);
    Address = EEPROM_START_ADDR + addr;
    status = DATA_EEPROM_ProgramHalfWord(Address, data);

    DATA_EEPROM_Lock();

    if (status == FLASH_COMPLETE)
        return 0;
    else
        return 1;   // return non-zero on fault
}

ot_u16 vsram_read(vaddr addr) {
    addr -= VSRAM_BASE_VADDR;
    addr >>= 1;
    //debug_printf("%04x = vsram_read(%x)\r\n", vsram[addr], addr);
    if (addr > (VSRAM_SIZE/2) ) {
        for (;;)
            asm("nop");
    }
    return vsram[addr];
}

ot_u8 vsram_mark(vaddr addr, ot_u16 value) {
    addr -= VSRAM_BASE_VADDR;
    addr >>= 1;
#ifdef RADIO_DEBUG   // 24
    debug_printf("vsram_mark(%x, %x)\r\n", addr, value);
#endif /* RADIO_DEBUG */

    if ( addr > (VSRAM_SIZE/2) )
        return 1;   // non-zero for fault

    vsram[addr] = value;

    return 0;
}

ot_u8* vsram_get(vaddr addr) {
    // todo
#if (VSRAM_SIZE <= 0)
    return NULL;
#else
    ot_u8* output;
    addr   -= VSRAM_BASE_VADDR;
    output  = (ot_u8*)vsram + addr;
	//debug_printf("%p = vsram_get(%x)\r\n", output, addr);
    return output;
#endif
}

ot_u8 vworm_mark(vaddr addr, ot_u16 value) {
#if (VWORM_SIZE <= 0)
    return ~0;
#else
    return vworm_write(addr, value);
#endif
}

ot_u8 vworm_wipeblock(vaddr addr, ot_uint wipe_span) {
#if (VWORM_SIZE <= 0)
    return ~0;
    
#else
    ot_u8 output = 0;
    
    wipe_span += addr;
    for (; ((addr < (vaddr)wipe_span) && (output == 0)); addr+=2) {
        output |= vworm_write(addr, NULL_vaddr);
    }
    
    return output;

#endif
}


/* accessing files here prevents compiler from optimizing them away */
extern const ot_u8 overhead_files[];
extern const ot_u8 isfs_stock_codes[];
extern const ot_u8 gfb_stock_files[];
extern const ot_u8 isf_stock_files[];

ot_u8 vworm_init( )
{

#ifdef RADIO_DEBUG
    debug_printf("vworm_init()\r\n");
#endif /* RADIO_DEBUG */

    /* sanity checks for file locations vs. vaddr definitions */
    if (overhead_files != (ot_u8 *)EEPROM_START_ADDR) {
#ifdef RADIO_DEBUG   // 19
        debug_printf("overhead_files bad addr.  %p vs %p\r\n", overhead_files, (ot_u8 *)EEPROM_START_ADDR);
#endif /* RADIO_DEBUG */
        for (;;)
            asm("nop");
    }

    if ( isfs_stock_codes != (ot_u8 *)(EEPROM_START_ADDR + ISFS_START_VADDR) ) {
#ifdef RADIO_DEBUG   // 20
        debug_printf("isfs_stock_codes at %p bad vs %p\r\n",
            isfs_stock_codes, (ot_u8 *)(EEPROM_START_ADDR + ISFS_START_VADDR) );
#endif /* RADIO_DEBUG */
        for (;;)
            asm("nop");
    } 

#if (GFB_TOTAL_BYTES > 0)
    if ( gfb_stock_files != (ot_u8 *)(EEPROM_START_ADDR + GFB_START_VADDR) ) {
#ifdef RADIO_DEBUG   // 21
        debug_printf("gfb_stock_files bad addr %x\r\n", EEPROM_START_ADDR + GFB_START_VADDR);
#endif /* RADIO_DEBUG */
        for (;;)
            asm("nop");
    }
#endif /* if (GFB_TOTAL_BYTES > 0) */
    

    if ( isf_stock_files != (ot_u8 *)(EEPROM_START_ADDR + ISF_START_VADDR) ) {
#ifdef RADIO_DEBUG   // 22
        debug_printf("isf_stock_files bad addr %x\r\n", EEPROM_START_ADDR + ISF_START_VADDR);
#endif /* RADIO_DEBUG */
        for (;;)
            asm("nop");
    }

    /* initialize pointer to eeprom */
    _vworm = (volatile ot_u16 *)EEPROM_START_ADDR;

    //debug_printf("vsram at %p\r\n", vsram);

    return 0;
}

