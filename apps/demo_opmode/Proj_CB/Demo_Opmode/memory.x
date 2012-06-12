MEMORY {
  sfr              : ORIGIN = 0x0000, LENGTH = 0x0010 /* END=0x0010, size 16 */
  peripheral_8bit  : ORIGIN = 0x0010, LENGTH = 0x00f0 /* END=0x0100, size 240 */
  peripheral_16bit : ORIGIN = 0x0100, LENGTH = 0x0100 /* END=0x0200, size 256 */
  bsl              : ORIGIN = 0x1000, LENGTH = 0x0800 /* END=0x1800, size 2K as 4 512-byte segments */
  infomem          : ORIGIN = 0x1800, LENGTH = 0x0200 /* END=0x1a00, size 512 as 4 128-byte segments */
  infod            : ORIGIN = 0x1800, LENGTH = 0x0080 /* END=0x1880, size 128 */
  infoc            : ORIGIN = 0x1880, LENGTH = 0x0080 /* END=0x1900, size 128 */
  infob            : ORIGIN = 0x1900, LENGTH = 0x0080 /* END=0x1980, size 128 */
  infoa            : ORIGIN = 0x1980, LENGTH = 0x0080 /* END=0x1a00, size 128 */
  ram (wx)         : ORIGIN = 0x1c00, LENGTH = 0x0ffe /* END=0x2bfe, size 4094 */
  flash_vl_ov      : ORIGIN = 0x8000, LENGTH = 0x0360   /* OVERHEAD_START_VADDR, OVERHEAD_TOTAL_BYTES */
  flash_vl_isfs    : ORIGIN = 0x8360, LENGTH = 0x00a0   /* ISFS_START_VADDR, ISFS_TOTAL_BYTES */
/*flash_vl_gfb     : ORIGIN = 0x8400, LENGTH = 0x0000    GFB_START_VADDR, GFB_TOTAL_BYTES */
  flash_vl_isf     : ORIGIN = 0x8400, LENGTH = 0x0600   /* ISF_START_VADDR, ISF_TOTAL_BYTES */
  rom (rx)         : ORIGIN = 0x8a00, LENGTH = 0x7580 /* END=0xff80, size */
  vectors          : ORIGIN = 0xff80, LENGTH = 0x0080 /* END=0x10000, size 128 as 64 2-byte segments */
  /* Remaining banks are absent */
  far_rom          : ORIGIN = 0x00000000, LENGTH = 0x00000000
}
REGION_ALIAS("REGION_TEXT", rom);
REGION_ALIAS("REGION_DATA", ram);
REGION_ALIAS("REGION_FAR_ROM", far_rom);
PROVIDE (__info_segment_size = 0x80);
PROVIDE (__infod = 0x1800);
PROVIDE (__infoc = 0x1880);
PROVIDE (__infob = 0x1900);
PROVIDE (__infoa = 0x1980);

SECTIONS
{
    .vl_ov   : { KEEP(*(.vl_ov)) }   > flash_vl_ov
    .vl_isfs : { KEEP(*(.vl_isfs)) } > flash_vl_isfs
/*    .vl_gfb  : { KEEP(*(.vl_gfb)) }  > flash_vl_gfb */ /* is GFB_TOTAL_BYTES == 0 ? */
    .vl_isf  : { KEEP(*(.vl_isf)) }  > flash_vl_isf
}

