/*  Copyright 2010-2012, JP Norair
  *
  * Licensed under the OpenTag License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  * http://www.indigresso.com/wiki/doku.php?id=opentag:license_1_0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  */
/**
  * @file       otsys/veelite.h
  * @author     JP Norair
  * @version    R100
  * @date       31 July 2012
  * @brief      Veelite virtual data interface for ISO 18000-7 Mode 1/2
  * @ingroup    Veelite
  *
  * Veelite is a virtual data interface that provides access to the main data
  * types used in DASH7 Modes 1/2.  Veelite could be used for anything,
  * as long as the data structures used by DASH7 make sense for the application.
  * The data files are:
  * - ISF: Universal Data Block
  * - GFB: Raw Data Block
  * - ISFS: ISF LIST IDs
  *
  * Veelite uses a virtual memory method, implemented in Veelite Core.  The user
  * does not need to interface with functions from the core.
  *
  * Veelite = Virtual EEPROM Lightweight
  ******************************************************************************
  */

#ifndef __VEELITE_H
#define __VEELITE_H

#include <otstd.h>

#include <otlib/utils.h>
#include <m2/tmpl.h>
#include <otsys/veelite_core.h>



typedef enum {
    VL_NULL_BLOCKID = 0,
    VL_GFB_BLOCKID  = 1,
    VL_ISFS_BLOCKID = 2,
    VL_ISF_BLOCKID  = 3
} vlBLOCK;



/** @typedef vlFILE
  * The FILE structure for veelite.  Much like POSIX FILE, it is only ever used
  * by the client as the pointer vlFILE*
  */
typedef ot_u16 (*vlread_fn)(ot_uint);
typedef ot_u8  (*vlwrite_fn)(ot_uint, ot_u16);
  
typedef struct {
    vaddr       header;
    ot_u16      start;
    ot_u16      alloc;
    ot_u16      idmod;
    ot_u16      length;
    vlread_fn   read;
    vlwrite_fn  write;
} vlFILE;

      


/** @typedef M1TAG_struct
  * A struct data type that hold Tag Data.  Tag Data is defined in the
  * ISO 18000-7:2008 standard and includes the following:
  * - 32 bit serial number
  * - 16 bit manufacturer's ID
  * - 24 bit firmware version
  * - 32 bit model number
  * - 8 bit maximum response length
  */
typedef struct {
    ot_u16 manuf_id;
    ot_u32 serial;
    ot_u32 model_no;
    ot_u32 fw_version;
    ot_u16 max_response;
} M1TAG_struct;


/** @typedef M1table_header
  * A struct data type that holds file information pertinent to tables.
  * Currently unused, as tables are unimplemented.  Tables are an optional
  * component of Mode 1, and at present there are no plans to build tables
  * into the official OpenTag distribution.
  */
typedef struct {
    ot_u32 placeholder;
} M1table_header;



/** @typedef vl_header
  * The generic form of the header used for OpenTag data files, used for
  * ISF, ISFS, and GFB.  The mirror field should be set to NULL_vaddr if
  * not used.
  *
  * ot_u16  length      length of data in bytes (0-255)
  * ot_u16  alloc:      max allocation (usually half-word aligned)
  * ot_u8   id:         ID, 0x00 - 0x0FF
  * ot_u8   privileges: 8 bit mod code
  * vaddr   base:       base (start) virtual address
  * vaddr   mirror:     base virtual address of vsram mirrored data (optional)
  */
typedef struct {
    ot_u16  length;
    ot_u16  alloc;
    ot_u16  idmod;
    vaddr   base;
    vaddr   mirror;
} vl_header;



/** @note vl_link type: this may seem redundant.  It is designed to sit
  *       in RAM, and be sorted and searched.  It only makes sense to use them
  *       if you have a lot of user-created files, which may be out-of-order
  *       or otherwise jumbled in memory, or if you have a machine that is not
  *       at all memory-constrained.  For the alpha/beta implementation, the
  *       vl_link feature data is not included.
  */

/** @typedef vl_link
  * The vl_link type is a file handle for block Data.  Typically, vl_link is
  * stored in volatile memory (SRAM).
  *
  * ot_u8   id:         ID value of the ISF Element (8 bits)
  * ot_u8   reserved:   reserved
  * vl_header* header: Pointer to Data file header   
  */
typedef struct {
    ot_u8           id;
    ot_u8           reserved;
    vl_header*    header;
} vl_link;



/// Access Control parameters
#define VL_ACCESS_GUEST     (ot_u8)b00000111
#define VL_ACCESS_USER      (ot_u8)b00111000
#define VL_ACCESS_SU        (ot_u8)b00111111
#define VL_ACCESS_R         (ot_u8)b00100100
#define VL_ACCESS_W         (ot_u8)b00010010
#define VL_ACCESS_X         (ot_u8)b10001001
#define VL_ACCESS_RW        (VL_ACCESS_R | VL_ACCESS_W)
#define VL_ACCESS_CRYPTO    (ot_u8)b01000000





#if (OT_FEATURE(VEELITE) == ENABLED)

/// Virtual Address Shortcuts for the header blocks (VWORM)
/// Header blocks for: GFB Elements, ISFS IDs, and ISF Elements
#define GFB_Header_START        OVERHEAD_START_VADDR
#define GFB_Header_START_USER   (GFB_Header_START + (GFB_NUM_STOCK_FILES*sizeof(vl_header)))
#define ISFS_Header_START       (GFB_Header_START + (GFB_NUM_FILES*sizeof(vl_header)))
#define ISFS_Header_START_USER  (ISFS_Header_START + (ISFS_NUM_STOCK_LISTS*sizeof(vl_header)))
#define ISF_Header_START        (ISFS_Header_START + (ISFS_NUM_LISTS*sizeof(vl_header)))
#define ISF_Header_START_USER   (ISF_Header_START + (ISF_NUM_STOCK_FILES*sizeof(vl_header)))


/// ISFS HEAP Virtual address shortcuts (VWORM)
/// @todo Hardcoded for the time being... fix later
#define ISFS_HEAP_START         ISFS_START_VADDR
#define ISFS_HEAP_USER_START    (ISFS_START_VADDR+ISFS_STOCK_HEAP_BYTES)
#define ISFS_HEAP_END           (ISFS_START_VADDR+ISFS_TOTAL_BYTES)


/// GFB HEAP Virtual address shortcuts (VWORM)
/// @todo Hardcoded for the time being... fix later
#define GFB_HEAP_START          GFB_START_VADDR
#define GFB_HEAP_USER_START     (GFB_START_VADDR+(GFB_NUM_STOCK_FILES*GFB_FILE_BYTES))
#define GFB_HEAP_END            (GFB_START_VADDR+GFB_TOTAL_BYTES)


/// ISF HEAP Virtual address shortcuts (VWORM)
/// @todo Hardcoded for the time being... fix later
#define ISF_HEAP_START          ISF_START_VADDR
#define ISF_HEAP_STOCK_START    ISF_START_VADDR
#define ISF_HEAP_USER_START     (ISF_START_VADDR+ISF_VWORM_STOCK_BYTES)
#define ISF_HEAP_END            (ISF_START_VADDR+ISF_TOTAL_BYTES)


/// ISF MIRROR Virtual address shortcuts (VSRAM)
#if (ISF_MIRROR_HEAP_BYTES > 0)
#   define ISF_MIRROR_BASE      VSRAM_BASE_VADDR
#   undef VSRAM_USED
#   define  VSRAM_USED          1
#endif


/// M1TAG HEAP address shortcuts ("secret" memory)
/// The optional data for M1TAG (Mode 1 only) is stored at some location in
/// physical memory (typically flash).  It never needs to be changed during
/// runtime.
#define  M1TAG_BASE             NULL_vaddr
#define  M1TAG_BASE_PHYSICAL    M1TAG_SPACE




// veelite functions

/** @brief initializes the veelite subsystem.  Run after SRAM resets.
  * @param none
  * @retval none
  * @ingroup Veelite
  */
void vl_init();




// General File functions

/** @brief  Returns an active file pointer when supplied an active file descriptor
  * @param  fd          (ot_int) Active file descriptor
  * @retval vlFILE*     A veelite file pointer
  * @ingroup Veelite
  *
  * Behavior is undefined if vl_get_fp() is supplied with an inactive fd.
  */
vlFILE* vl_get_fp(ot_int fd);


/** @brief  Returns an active file descriptor when supplied an active file pointer
  * @param  fp          (vlFILE*) Active file pointer
  * @retval ot_int      file descriptor integer
  * @ingroup Veelite
  *
  * Behavior is undefined if vl_get_fd() is supplied with an inactive fp.
  */
ot_int  vl_get_fd(vlFILE* fp);


/** @brief  Creates a new file
  * @param  fp_new      (vlFILE**) A file pointer handle for new file
  * @param  block_id    (vlBLOCK) Block ID of new file (GFB, ISFB, ISFSB, etc)
  * @param  data_id     (ot_u8) 0-255 file ID of new file
  * @param  mod         (ot_u8) Permissions for new file
  * @param  max_length  (ot_uint) Maximum length for new file (alloc)
  * @param  user_id     (id_tmpl*) User ID that is trying to create new file
  * @retval ot_u8       Return code: 0 on success, non-zero on error
  * @ingroup Veelite
  *
  * Most of the parameters should be self-explanatory.  The user_id parameter is
  * usually passed from the source address of the DASH7 request (if the new file
  * request is coming from DASH7).  If the new file request is called internally
  * by the system (root), you can pass NULL, which will always be resolved to
  * root.
  *
  * The return value is a numerical code.
  * <LI>   0: Success                                           </LI>
  * <LI>   2: File already exists, can't create new one         </LI>
  * <LI>   4: User does not have access to create a new file    </LI>
  * <LI>   6: Not enough room for a new file                    </LI> 
  * <LI> 255: Miscellaneous Error                               </LI>
  */
ot_u8   vl_new(vlFILE** fp_new, vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, ot_uint max_length, id_tmpl* user_id);


/** @brief  Deletes a file
  * @param  block_id    (vlBLOCK) Block ID of file to delete (GFB, ISFB, ISFSB, etc)
  * @param  data_id     (ot_u8) 0-255 file ID of file to delete
  * @param  user_id     (id_tmpl*) User ID that is trying to create new file
  * @retval ot_u8       Return code: 0 on success, non-zero on error
  * @ingroup Veelite
  *
  * @note Files that are built-in to the DASH7 spec cannot be deleted.  Also,
  * other files declared at compile-time as "stock" files are protected against
  * deletion by any user.
  * 
  * Most of the parameters should be self-explanatory.  The user_id parameter is
  * usually passed from the source address of the DASH7 request (if the delete
  * request is coming from DASH7).  If the delete request is called internally
  * by the system (root), you can pass NULL, which will always be resolved to
  * root.
  *
  * The return value is a numerical code.
  * <LI>   0: Success                                           </LI>
  * <LI>   1: File could not be found                           </LI>
  * <LI>   4: User does not have access to create a new file    </LI>
  * <LI> 255: Miscellaneous Error                               </LI>
  */
ot_u8   vl_delete(vlBLOCK block_id, ot_u8 data_id, id_tmpl* user_id);


/** @brief  Returns a file header as the vaddr of the header
  * @param  header      (vaddr*) Output header vaddr
  * @param  block_id    (vlBLOCK) Block ID of file header to get
  * @param  data_id     (ot_u8) 0-255 file ID of file header to get
  * @param  mod         (ot_u8) Method of access for file (read, write, etc)
  * @param  user_id     (id_tmpl*) User ID that is trying to create new file
  * @retval ot_u8       Return code: 0 on success, non-zero on error
  * @ingroup Veelite
  *
  * Most of the parameters should be self-explanatory.  The user_id parameter is
  * usually passed from the source address of the DASH7 request (if the request
  * is coming from DASH7).  If the request is called internally by the system 
  * (root), you can pass NULL, which will always be resolved to root.
  *
  * This function is intended for use with File ALP protocols.
  *
  * The return value is a numerical code.
  * <LI>   0: Success                                           </LI>
  * <LI>   1: File could not be found                           </LI>
  * <LI>   4: User does not have sufficient access to this file </LI>
  * <LI> 255: Miscellaneous Error                               </LI>
  */

ot_u8   vl_getheader_vaddr(vaddr* header, vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, id_tmpl* user_id);


/** @brief  Returns file header as a vl_header datastruct
  * @param  header      (vl_header*) Output header datastruct
  * @param  block_id    (vlBLOCK) Block ID of file header to get
  * @param  data_id     (ot_u8) 0-255 file ID of file header to get
  * @param  mod         (ot_u8) Method of access for file (read, write, etc)
  * @param  user_id     (id_tmpl*) User ID that is trying get header
  * @retval ot_u8       Return code: 0 on success, non-zero on error
  * @ingroup Veelite
  * @sa vl_getheader_vaddr()
  *
  * This function is a wrapper for vl_getheader_vaddr(), but it also copies the
  * header data into a vl_header struct that the user must allocate and supply.
  *
  * This function is intended for use with File ALP protocols.
  */
ot_u8   vl_getheader(vl_header* header, vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, id_tmpl* user_id);


/** @brief  Opens a file from the virtual address of its header
  * @param  header      (vaddr) virtual address of the file header to open
  * @retval vlFILE*     File Pointer (NULL on error)
  * @ingroup Veelite
  *
  * In order to be secure, do not use this function without first getting the
  * header from vl_getheader_vaddr(), which has user authentication.
  *
  * This function is intended for use with File ALP protocols.
  */
vlFILE* vl_open_file(vaddr header);


/** @brief  Normal File Open Function
  * @param  block_id    (vlBLOCK) Block ID of file to open
  * @param  data_id     (ot_u8) 0-255 file ID of file to open
  * @param  mod         (ot_u8) Method of access for file (read, write, etc)
  * @param  user_id     (id_tmpl*) User ID that is trying to open file
  * @retval vlFILE*     File Pointer (NULL on error)
  * @ingroup Veelite
  *
  * This is the normal function to use to open a file in Veelite.  The other
  * methods for opening are for special cases (they exist to make the filedata
  * ALP run faster and smaller).
  *
  * There are several, specialized alias functions for vl_open().  These should
  * only be used internally (not in protocol routines).  The function aliases 
  * that contain "_su" at the end are super-user (root) calls.
  */
vlFILE* vl_open(vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, id_tmpl* user_id);
vlFILE* GFB_open_su( ot_u8 id );
vlFILE* ISFS_open_su( ot_u8 id );
vlFILE* ISF_open_su( ot_u8 id );
vlFILE* GFB_open( ot_u8 id, ot_u8 mod, id_tmpl* user_id );
vlFILE* ISFS_open( ot_u8 id, ot_u8 mod, id_tmpl* user_id );
vlFILE* ISF_open( ot_u8 id, ot_u8 mod, id_tmpl* user_id );


/** @brief  File Change Mods (like chmod on POSIX)
  * @param  block_id    (vlBLOCK) Block ID of file to chmod
  * @param  data_id     (ot_u8) 0-255 file ID of file to chmod
  * @param  mod         (ot_u8) New mod setting
  * @param  user_id     (id_tmpl*) User ID that is trying to chmod
  * @retval ot_u8     File Pointer (NULL on error)
  * @ingroup Veelite
  *
  * The file mod value in Veelite is one byte.  It looks like this.         <BR>
  * B7: Encryption bit      : (Proprietary) The file data is encrypted      <BR>
  * B6: Runable             : (Proprietary) The file data contains a program<BR>
  * B5: User Read           : User can read                                 <BR>
  * B4: User Write          : User can write                                <BR>
  * B3: Run                 : User can run (if file is runable)             <BR>
  * B2: Guest Read          : Guest can read                                <BR>
  * B1: Guest Write         : Guest can write                               <BR>
  * B0: Guest Run           : Guest can run (if file is runable)            <BR>
  *
  * The return value is a numerical code.
  * <LI>   0: Success                                           </LI>
  * <LI>   1: File could not be found                           </LI>
  * <LI>   4: User does not have sufficient access to this file </LI>
  * <LI> 255: Miscellaneous Error                               </LI>
  * 
  * There are several, specialized alias functions for vl_chmod().  These should
  * only be used internally (not in protocol routines).  The function aliases 
  * that contain "_su" at the end are super-user (root) calls.
  */
ot_u8 vl_chmod(vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, id_tmpl* user_id);
ot_u8 GFB_chmod_su( ot_u8 id, ot_u8 mod );
ot_u8 ISFS_chmod_su( ot_u8 id, ot_u8 mod );
ot_u8 ISF_chmod_su( ot_u8 id, ot_u8 mod );


/** @brief  Reads 16 bits at a time from the open file (GFB, ISF, ISFS)
  * @param  fp          (vlFILE*) file pointer of open file
  * @param  offset      (ot_uint) byte offset into the file
  * @retval (ot_u16)    16 bits data from the given offset
  * @ingroup Veelite
  *
  * Odd offset values are rounded down.
  */
ot_u16 vl_read( vlFILE* fp, ot_uint offset );


/** @brief  Writes 16 bits at a time to the open file (GFB, ISF, ISFS)
  * @param  fp          (vlFILE*) file pointer of open file
  * @param  offset      (ot_uint) byte offset into the file
  * @param  data        (ot_u16) 16 bits data to write
  * @retval (ot_u8)     Non-zero on failure
  * @ingroup Veelite
  *
  * Note: to write a odd number of bytes, supply an odd value for
  * offset.  The odd byte in data (upper byte in little endian, lower byte in
  * big endian, or UPPER byte in OpenTag TwoBytes data union) will be written
  * and the even byte will be discarded.
  */
ot_u8 vl_write( vlFILE* fp, ot_uint offset, ot_u16 data );



/** @brief  Loads the contents of a file into a supplied byte-buffer
  * @param  fp          (vlFILE*) file pointer of open file
  * @param  length      (ot_uint) number of bytes to load, starting from beginning of file
  * @param  data        (ot_u8*) byte buffer to load into
  * @retval (ot_uint)   Number of bytes loaded into byte-buffer
  * @ingroup Veelite
  *
  * This function will not read more bytes than the current length of the file.
  */
ot_uint vl_load( vlFILE* fp, ot_uint length, ot_u8* data );



/** @brief  Stores supplied byte-buffer into a file, replacing existing contents
  * @param  fp          (vlFILE*) file pointer of open file
  * @param  length      (ot_uint) number of bytes to store, starting from beginning of file
  * @param  data        (ot_u8*) byte buffer to write to file
  * @retval (ot_u8)     Non-zero on failure
  * @ingroup Veelite
  */
ot_u8 vl_store( vlFILE* fp, ot_uint length, ot_u8* data );



ot_u8 vl_append( vlFILE* fp, ot_uint length, ot_u8* data );




/** @brief  Crops (or erases) a file contents without deleting the file
  * @param  fp          (vlFILE*) file pointer of open file
  * @param  offset      (ot_uint) file data offset to crop after
  * @retval (ot_u8)     Non-zero on failure
  * @ingroup Veelite
  *
  * vl_crop() can erase partial contents of a file.  What it does is set the
  * fp->length to a certain value and also alter the hard file header to the 
  * same value.
  *
  * vl_erase() is a wrapper that calls vl_crop(fp, 0)
  
ot_u8 vl_crop(vlFILE* fp, ot_uint offset);
ot_u8 vl_erase(vlFILE* fp);
*/


/** @brief Closes the data read/write session of the open file
  * @param none
  * @retval (ot_u8) : Non-zero on failure
  * @ingroup Veelite
  */
ot_u8 vl_close( vlFILE* fp );

/** @brief Returns the length of the open file (GFB, ISF, ISFS)
  * @param none
  * @retval (ot_uint) : length in bytes
  * @ingroup Veelite
  */
ot_uint vl_checklength( vlFILE* fp );

/** @brief Returns the length of the open file (GFB, ISF, ISFS)
  * @param none
  * @retval (ot_uint) : length in bytes
  * @ingroup Veelite
  */
ot_uint vl_checkalloc( vlFILE* fp );


//Compatibility definitions (deprecated)
#define GFB_close(FP)                 vl_close(FP)
#define ISF_close(FP)                 vl_close(FP)
#define ISFS_close(FP)             vl_close(FP)

#define GFB_read(FP, VAL)               vl_read(FP, VAL)
#define ISF_read(FP, VAL)               vl_read(FP, VAL)
#define ISFS_read(FP, VAL)           vl_read(FP, VAL)

#define GFB_write(FP, VAL1, VAL2)       vl_write(FP, VAL1, VAL2)
#define ISF_write(FP, VAL1, VAL2)       vl_write(FP, VAL1, VAL2)
#define ISFS_write(FP, VAL1, VAL2)   vl_write(FP, VAL1, VAL2)







/** @brief Syncs main ISF file data with the data in the mirror
  * @param none
  * @retval ot_u8 : Non-zero on failure
  * @ingroup Veelite
  *
  * Only works on ISF files that are mirrored.  In certain implementations,
  * this function may do nothing at all.  It should really only be used by the
  * root user.
  */
ot_u8 ISF_syncmirror( );

/** @brief loads file data from vworm to data in the mirror
  * @param none
  * @retval ot_u8 : Non-zero on failure
  * @ingroup Veelite
  *
  * Only works on ISF files that are mirrored.  In certain implementations,
  * this function may do nothing at all.
  */
ot_u8 ISF_loadmirror( );











/* @brief Writes the M1TAG struct, which bears certain Mode 1 config data.
  * @param new_m1tag : (M1TAG_struct*) pointer to struct bearing the new data.
  * @retval ot_u8 : Non-zero on failure
  * @ingroup Veelite
  * 
  * M1TAG_write() is typically only used or needed at the time of manufacture or
  * "commission."
  *
ot_u8 M1TAG_write( M1TAG_struct* new_m1tag );

* @brief Gets a copy of the M1TAG struct
  * @param m1tag : (M1TAG_struct*) pointer to struct to return
  * @retval ot_u8 : Non-zero on failure
  * @ingroup Veelite
  * 
  * the m1tag argument must be allocated by the caller, otherwise you will 
  * almost certainly run into a segmentation fault or similar problem!
  *
ot_u8 M1TAG_get( M1TAG_struct* m1tag );
*/



#endif // if (OT_FEATURE(VEELITE) == ENABLED)
#endif // ifndef __VEELITE_H





