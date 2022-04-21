/*  Copyright 2022, JP Norair
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted provided that the following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright 
  *    notice, this list of conditions and the following disclaimer in the 
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
  * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
  * POSSIBILITY OF SUCH DAMAGE.
  */
/**
  * @file       otsys/pseudofs.h
  * @author     JP Norair
  * @version    R100
  * @date       15 April 2012
  * @brief      Pseudo File System for Core Data Management
  * @ingroup    PseudoFS
  *
  * Pseudo FS replaces Veelite in OpenTag 2.0.  It is more general purpose 
  * than Veelite and more efficient for the basic types of workflows that 
  * OpenTag ended-up using the most.  It expects that all file content data is
  * buffered/mirrored in RAM during runtime.  A Veelite shim exists for 
  * backwards compatibility with veelite.
  *
  ******************************************************************************
  */

#ifndef __PSEUDOFS_H
#define __PSEUDOFS_H

#include <otstd.h>

#include <otlib/utils.h>

#include <otsys/veelite_core.h>



#define PFS_TYPE_RECORD		(0)
#define PFS_TYPE_DYNRECORD	(1)
#define PFS_TYPE_BYTEFILE	(2)
#define PFS_TYPE_EXT		(3)

#define PFS_FLASH_NONE      (0)
#define PFS_FLASH_HDRONLY   (1)
#define PFS_FLASH_READONLY	(2)
#define PFS_FLASH_READWRITE	(3)

#define PFS_FLAG_ISOPEN		(1<<0)
#define PFS_FLAG_ISWRITTEN	(1<<1)




/** @brief Access Control Parameters / Modbits for ByteFile types
  *
  * 8 modification bits: YZrwxrwx
  * - root access can do rwx for any file
  * - user access is the most significant rwx, user mgmt is path of auth module.
  * - guest access is the least significant rwx.
  * - Y bit is reserved for future use (RFU)
  * - Z bit describes if the file contents are executable at all.
  *
  * These are specific to the ByteFile types.  Other types don't use them.
  */
#define BF_ACCESS_GUEST     (ot_u8)b00000111
#define BF_ACCESS_USER      (ot_u8)b00111000
#define BF_ACCESS_SU        (ot_u8)b00111111
#define BF_ACCESS_R         (ot_u8)b00100100
#define BF_ACCESS_W         (ot_u8)b00010010
#define BF_ACCESS_X         (ot_u8)b01001001
#define BF_ACCESS_RW        (BF_ACCESS_R | BF_ACCESS_W)


///@todo need to declare Action Flags
#define BF_ACTION_MASK		(1)



// Backwards compatibility for unported projects that use Veelite
#if (OT_FEATURE(PSEUDOFS) != ENABLED)
#	define PFS_PARAM_MAXFILEPTRS        OT_PARAM(VLFPS)
#	define PFS_FEATURE_ALLOC         	OT_FEATURE(VLNEW)
#	define PFS_FEATURE_MODTIME 			OT_FEATURE(VLMODTIME)
#	define PFS_FEATURE_ACCTIME			OT_FEATURE(VLACCTIME)

#else
#	define PFS_PARAM_MAXFILEPTRS        OT_PARAM(FSMAXFPS)
#	define PFS_FEATURE_ALLOC        	OT_FEATURE(FSALLOC)
#	define PFS_FEATURE_MODTIME 			OT_FEATURE(FSMODTIME)
#	define PFS_FEATURE_ACCTIME			OT_FEATURE(FSACCTIME)

#endif



/** @brief PseudoFS file types
  *
  * PFS_Record 		: Raw data record, defined at compile-time, backed in flash.
  * PFS_Dynrecord   : Raw data record, dynamically allocated in runtime, can be malloc.
  * PFS_Bytefile    : Minimal file, can be defined at compile-time or dynamic, includes
  *                   some features like cursors, user authentication, file actions.
  * PFS_Ext         : An extension payload, which can link to any external filesystem.
  *                   The filesystem is user-supplied and user must deal with linking.
  */
typedef enum {
	PFS_Record		= PFS_TYPE_RECORD,
	PFS_Dynrecord	= PFS_TYPE_DYNRECORD,
	PFS_Bytefile    = PFS_TYPE_BYTEFILE,
	PFS_Ext         = PFS_TYPE_EXT
} pfs_type_e;


/** @brief PseudoFS Mode types
  *
  * Typical Read, Write, Append.  In PseudoFS, write and readwrite are interchangeable.
  */
typedef enum {
	PFS_Read,
	PFS_Write,
	PFS_Append,
	PFS_Readwrite
} pfs_mode_e;


/** @brief PseudoFS file specifier datatype
  * @sa pfs_stat_t
  * @sa pfs_file_t
  *
  * File ID and information related to how it is stored.
  */
typedef struct {
	ot_u16	type : 2;
	ot_u16  flash : 2;
	ot_u16	flags : 2;
	ot_u16  id : 10;
} pfs_spec_t;


/** @brief PseudoFS file status datatype
  * @sa pfs_file_t
  *
  * Stores frequently updated and fungible status data, such as modification times and
  * action handle.
  *
  * @note Both GCC and Clang handle empty structs as 0 length datatypes.  These are the
  *       only two compilers supported by OpenTag.
  * 
  * @todo align to 32 bits
  *
  * @todo make sure the OT_FEATURES
  */
typedef struct OT_PACKED {
#if PFS_FEATURE_MODTIME
    ot_u32 modtime;
#endif
#if PFS_FEATURE_ACCTIME
    ot_u32 acctime;
#endif
    ot_procv action;
} pfs_stat_t;


/** @brief PseudoFS Non-Volatile Handle datatype
  *
  * pfs_handle_t stores the entirety of file metadata that is stored in the 
  * file table.  Additional File Metadata may be stored in the file header,
  * depending on the file type.
  */
typedef struct OT_PACKED {
	ot_u32		front;
	pfs_spec_t	spec;
    ot_u16      alloc;
} pfs_handle_t;


/** @brief PseudoFS Dynamic Record Header
  * 
  * Dynamic records contain a header in their first 4 bytes of file data, which must 
  * contain a backlink to the file handle.  This is an important attribute that allows
  * malloc-like usage of dynamic records.
  */
typedef struct OT_PACKED {
	pfs_handle_t* handle;
} pfs_drechdr_t;


/** @brief PseudoFS ByteFile Header
  * 
  * ByteFile types contain a header in their first 4-16 bytes of file data: the length
  * of the header depends on the amount of file features selected at compile-time.
  * The length of the header is always 4-byte aligned.
  */
typedef struct OT_PACKED {
	ot_u16  	flags;
    ot_u16  	length;
    pfs_stat_t	stat;
} pfs_bfhdr_t;


/** @brief PseudoFS ByteFile Header
  * 
  * ByteFile types contain a header in their first 4-16 bytes of file data: the length
  * of the header depends on the amount of file features selected at compile-time.
  * The length of the header is always 4-byte aligned.
  */
typedef struct OT_PACKED {
	void*		hdr;
	void*		data;
	pfs_spec_t	spec;
	ot_u16      alloc;
} pfs_file_t;


/** @brief PseudoFS User ID
  * 
  * This datatype is only important to PseudoFS as a pointer.
  */
typedef struct {
} pfs_user_t;






/** @brief initializes the PseudoFS subsystem.  Run after SRAM resets.
  * @param none
  * @retval error code (ot_int), negative on error
  * @ingroup PseudoFS
  */
ot_int pfs_init(void);


/** @brief  Returns an active file pointer when supplied an active file descriptor
  * @param  fd          (ot_int) Active file descriptor
  * @retval pfs_file_t* A veelite file pointer
  * @ingroup PseudoFS
  *
  * Behavior is undefined if pfs_get_fp() is supplied with an inactive fd.
  */
pfs_file_t* pfs_get_fp(ot_int fd);


/** @brief  Returns an active file descriptor when supplied an active file pointer
  * @param  fp          (pfs_file_t*) Active file pointer
  * @retval ot_int      file descriptor integer
  * @ingroup PseudoFS
  *
  * Behavior is undefined if pfs_get_fd() is supplied with an inactive fp.
  */
ot_int pfs_get_fd(pfs_file_t* fp);


/** @brief  Creates a new file
  * @param  file_type   (pfs_type_e) File Type for new file 
  * @param  alloc		(ot_u16) Maximum size in bytes of the file
  * @param  flags       (ot_u16) Flags, contents depend on type of file
  * @param  user_id     (pfs_user_t*) User ID that is trying to create new file
  * @retval ot_int      Returns File ID on success, or negative value on error
  * @ingroup PseudoFS
  *
  * Most of the parameters should be self-explanatory.  Some need more description.
  * 
  * The user_id parameter is very optional and only useful for authenticated files types, 
  * such as ByteFile or possibly EXT (external FS) if you've implemented one.  Setting 
  * user_id to NULL is the normal policy, which creates a file as "Root" user.
  *
  * The flags parameter also can be ignored (set to 0).  It is available as a variable
  * parameter for EXT implementations.  For ByteFile types, it specifies the 8-bit
  * permissions.
  *
  * The return value is a numerical code.  If it's a non-negative integer, it represents
  * the ID of the newly created file.  If the values are negative, they correspond to
  * errors, as shown below.
  * <LI>  -2: File already exists, can't create new one         </LI>
  * <LI>  -4: User does not have access to create a new file    </LI>
  * <LI>  -6: Not enough room for a new file                    </LI> 
  * <LI>-255: Miscellaneous Error                               </LI>
  */
ot_int pfs_new(pfs_type_e file_type, ot_u16 alloc, ot_u16 flags, const pfs_user_t* user_id);


/** @brief  Deletes a file
  * @param  file_id     (ot_u16) ID of file to delete
  * @param  user_id     (pfs_user_t*) User ID that is trying to create new file
  * @retval ot_int      Return code: 0 on success, non-zero on error
  * @ingroup PseudoFS
  *
  * @note Not all files can be deleted.  Only files that are created during runtime with
  *       pfs_new() can be deleted.
  * 
  * The user_id parameter is very optional and only useful for authenticated files types, 
  * such as ByteFile or possibly EXT (external FS) if you've implemented one.  Setting 
  * user_id to NULL is the normal policy, which deletes a file as "Root".
  *
  * The return value is a numerical code.
  * <LI>   0: Success                                           </LI>
  * <LI>  -1: File could not be found                           </LI>
  * <LI>  -3: File is not deletable                             </LI>
  * <LI>  -4: User does not have access to delete this file     </LI>
  * <LI>-255: Miscellaneous Error                               </LI>
  */
ot_int pfs_delete(ot_u16 file_id, const pfs_user_t* user_id);


/** @brief  Normal File Open Function
  * @param[out] fp      (pfs_file_t**) A file pointer handle output
  * @param  file_id     (ot_u16) ID of file to open.
  * @param  mode        (pfs_mode_e) Method of access for file (read, write, etc)
  * @param  user_id     (pfs_user_t*) User ID that is trying to create new file
  * @retval (ot_int)    Non-negative File Descriptor, or negative error code.
  * @ingroup PseudoFS
  *
  * This is the normal function to use to open a file in PseudoFS.
  *
  * The return value is a numerical code.
  * <LI> >=0: File Descriptor (Success)                         </LI>
  * <LI>  -1: File ID not found                                 </LI>
  * <LI>  -4: User does not have access to this file            </LI>
  * <LI> -16: Out of file descriptors                           </LI>
  * <LI>-255: Miscellaneous Error                               </LI>
  */
ot_int pfs_open(pfs_file_t** fp, ot_u16 file_id, pfs_mode_e mode, const pfs_user_t* user_id);


/** @brief  Returns a pointer to file data, for reading.
  * @param  fp              (pfs_file_t*) file pointer of open file
  * @retval (const ot_u8*)  byte pointer for reading file data
  * @ingroup PseudoFS
  *
  * Will return NULL if the file doesn't support reading.
  */
const ot_u8* pfs_read(pfs_file_t* fp);


/** @brief  Returns a pointer to file data, for writing.
  * @param  fp          (pfs_file_t*) file pointer of open file
  * @retval (ot_u8*)    byte pointer for writing file data
  * @ingroup PseudoFS
  *
  * Will return NULL if the file doesn't support writing.
  */
ot_u8* pfs_write(pfs_file_t* fp);


/** @brief  Returns a pointer to file data, for reading or writing.
  * @param  fp          (pfs_file_t*) file pointer of open file
  * @retval (ot_u8*)    byte pointer for reading/writing file data
  * @ingroup PseudoFS
  *
  * Will return NULL if the file doesn't support reading or writing.
  */
ot_u8* pfs_readwrite(pfs_file_t* fp);


/** @brief Closes the data read/write session of the open file
  * @param fp           (pfs_file_t*) file pointer of open file
  * @param w_cursor     (void*) final write cursor pointer, can be NULL
  * @retval (ot_int) :  Non-zero on failure
  * @ingroup PseudoFS
  *
  * Closes an open file, updates header contents, and applies actions.
  * 
  * w_cursor is only useful for ByteFile or (potentially) EXT type files.  EXT files
  * are EXTensions, and only perform linkage to a secondary, user-supplied file system.
  *
  * If w_cursor is supplied and fp is for a ByteFile, the value of w_cursor will be
  * checked for validity and applied to the active cursor position for the ByteFile.
  * If the value of w_cursor is invalid with respect to the file allocation boundaries,
  * it will be treated as NULL.
  * 
  * If w_cursor is NULL, it will have no effect on the file closing process.
  */
ot_int pfs_close(pfs_file_t* fp, void* w_cursor);


/** @brief  Gets file data without opening it.
  * @param[out] file_alloc  (ot_u16*) file allocation.  Input can be NULL.
  * @param  file_id         (ot_u16) ID of file to get.
  * @retval (ot_u8*) : pointer to file data, NULL if no file found.
  * @ingroup PseudoFS
  *
  * Will return a pointer to file data.  It's up to the user to act correctly and
  * not break the FS, there are no protections.  Always opens the supplied file ID as 
  * root.
  *
  * Any actions attached to a file will not be called via pfs_getdata().
  */
ot_u8* pfs_getdata(ot_u16* file_alloc, ot_u16 file_id);


/** @brief Returns the length of the open file
  * @param fp           (pfs_file_t*) file pointer of open file
  * @retval (ot_uint) : length in bytes
  * @ingroup PseudoFS
  *
  * For Record type files, the length and allocation are interchangeable, and the size
  * of the allocation will be returned.
  *
  * Will return 0 on invalid fp.
  */
ot_uint pfs_checklength(pfs_file_t* fp);


/** @brief Returns the allocation size of the open file
  * @param fp           (pfs_file_t*) file pointer of open file
  * @retval (ot_uint) : allocation size in bytes
  * @ingroup PseudoFS
  *
  * Will return 0 on invalid fp.
  */
ot_uint pfs_checkalloc(pfs_file_t* fp);


/** @brief Restores the filesystem to its factory-defaults.
  * @param none
  * @retval ot_int : Negative value on error, Zero on success.
  * @ingroup PseudoFS
  *
  * The filesystem contents stored in flash and RAM will be reset to their factory
  * defaults.  Not every implementation is guaranteed to support this feature.
  *
  * @note User should restart/reset OpenTag after running this function.
  *       Behavior of OpenTag after pfs_restore() is unpredictable.
  */
ot_int pfs_restore(void);


/** @brief Reflashes the NV-backed file section with RAM-backed file contents
  * @param none
  * @retval ot_int : Negative value on error, Zero on success.
  * @ingroup PseudoFS
  *
  * The filesystem contents stored in flash will be updated to contain the latest 
  * RAM-backed filesystem contents.
  *
  * The filesystem as stored in flash may be compressed, encrypted, or otherwise 
  * transformed from the data structure as found in the RAM-backed section.
  *
  * @note This function can be used anytime, but it can take a non-negligible
  *       amount of time (say, 10s to 100s of ms) to write to flash.  It is best
  *       to use from within a low-priority, high latency idle-time task.
  */
ot_int pfs_repack(void);


/** @brief Unpacks that NV-backed file section and inserts it into RAM buffer.
  * @param none
  * @retval ot_int : Negative value on error, Zero on success.
  * @ingroup PseudoFS
  *
  * The filesystem contents stored in flash will be transformed (if necessary) and
  * written to the RAM-backed filesystem.
  *
  * The filesystem as stored in flash may be compressed, encrypted, or otherwise 
  * transformed from the data structure as found in the RAM-backed section.
  *
  * @note Behavior of OpenTag is unpredictable if running this function after the 
  *       OpenTag kernel starts up.  It's best to run only from within pfs_init(),
  *       which itself will get run automatically during OT startup.
  */
ot_int pfs_unpack(void);


/** @brief  File Change Mods (like chmod on POSIX)
  * @param  file_id     (ot_u16) file ID to chmod
  * @param  modbits     (ot_u16) New modbits setting
  * @param  user_id     (pfs_user_t*) User ID that is trying to chmod
  * @retval ot_int      0 on success, negative on error
  * @ingroup PseudoFS
  *
  * The file mod in PseudoFS are only strictly implemented in ByteFile types.
  * For ByteFile types, it is only 8 bits, and the lower 8 bits of the supplied
  * value will be used.  For EXT file types, the entire 16 bits can be used, and
  * the specific implementation is up to the user.
  *
  * Modbits for ByteFile type is described below.
  * B7: RFU                 : (Proprietary) Reserved for future use
  * B6: Runable             : The file data contains a program
  * B5: User Read           : User can read
  * B4: User Write          : User can write
  * B3: Run                 : User can run (if file is runable)
  * B2: Guest Read          : Guest can read
  * B1: Guest Write         : Guest can write
  * B0: Guest Run           : Guest can run (if file is runable)
  *
  * The return value is a numerical code.
  * <LI>   0: Success                                           </LI>
  * <LI>  -1: File could not be found                           </LI>
  * <LI>  -4: User does not have sufficient access to this file </LI>
  * <LI>-255: Miscellaneous Error                               </LI>
  */
ot_int pfs_chmod(ot_u16 file_id, ot_u16 modbits, const pfs_user_t* user_id);


/** @brief  Attach a file action to a specified file
  * @param  file_id     (ot_u16) file ID to attach action onto
  * @param  act_flags   (ot_u16) Action flags, which control how action is applied
  * @param  action      (ot_procv) A function pointer to the action
  * @retval ot_int      0 on success, negative on error
  * @ingroup PseudoFS
  *
  * File actions in PseudoFS are only strictly implemented in ByteFile types.
  * For ByteFile types, a function can be called upon closing a file with attached
  * actions.  Generally, it's on-modify or on-access.  File actions are not 
  * explicitly implemented for other file types.
  *
  * A file action can be removed by running this function with the action
  * parameter set to NULL.
  *
  * The return value is a numerical code.
  * <LI>   0: Success                                           </LI>
  * <LI>  -1: File could not be found                           </LI>
  * <LI>  -3: Operation not supported (File not actionable)     <LI>
  * <LI>-255: Miscellaneous Error                               </LI>
  */
ot_int pfs_attach_action(ot_u16 file_id, ot_u16 act_flags, ot_procv action);


#endif



/** @brief  Returns a file header as the vaddr of the header
  * @param  header      (vaddr*) Output header vaddr
  * @param  block_id    (vlBLOCK) Block ID of file header to get
  * @param  data_id     (ot_u8) 0-255 file ID of file header to get
  * @param  mod         (ot_u8) Method of access for file (read, write, etc)
  * @param  user_id     (pfs_user_t*) User ID that is trying to create new file
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
//ot_u8   vl_getheader_vaddr(vaddr* header, vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, const pfs_user_t* user_id);


/** @brief  Returns file header as a vl_header datastruct
  * @param  header      (vl_header*) Output header datastruct
  * @param  block_id    (vlBLOCK) Block ID of file header to get
  * @param  data_id     (ot_u8) 0-255 file ID of file header to get
  * @param  mod         (ot_u8) Method of access for file (read, write, etc)
  * @param  user_id     (pfs_user_t*) User ID that is trying get header
  * @retval ot_u8       Return code: 0 on success, non-zero on error
  * @ingroup Veelite
  * @sa vl_getheader_vaddr()
  *
  * This function is a wrapper for vl_getheader_vaddr(), but it also copies the
  * header data into a vl_header struct that the user must allocate and supply.
  *
  * This function is intended for use with File ALP protocols.
  */
//ot_u8   vl_getheader(vl_header* header, vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, const pfs_user_t* user_id);


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
//vlFILE* vl_open_file(vaddr header);





