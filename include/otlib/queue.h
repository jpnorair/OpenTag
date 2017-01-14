/* Copyright 2010-2014 JP Norair
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
  * @file       otlib/queue.h
  * @author     JP Norair
  * @version    R102
  * @date       18 Sept 2014
  * @brief      A module and ADT for data stream management
  * @defgroup   Queue (Queue Module)
  * @ingroup    Queue
  *
  * The queue mechanism is for storage and management of data streams. There is 
  * no facilty for dynamic queue allocation, which shouldn't be a problem given 
  * the purpose-built, embedded nature of the platform.  Even so, it wouldn't 
  * take a lot of effort to work dynamic functionality into it.
  ******************************************************************************
  */

#ifndef __QUEUE_H
#define __QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <otstd.h>
//#include <otsys/threads.h>


/** @typedef ot_queue
  * 
  * The ot_queue data type does not contain the data in the queues themselves, just
  * information on how to get that data as well as any other useful variables.
  *
  * ot_u16 alloc        Allocation of the queue data, in bytes
  * ot_uni16 options    User flags
  * ot_u8* front        First address of queue data
  * ot_u8* back         Used for boundary checking (user adjustable)
  * ot_u8* getcursor    Cursor address for reading from queue
  * ot_u8* putcursor    Cursor address for writing to queue
  */
typedef struct {
    ot_u16      alloc;
    ot_uni16    options;
  //ot_tmask    tmask;    
    ot_u8*      getcursor;
    ot_u8*      putcursor;
    ot_u8*      front;
    ot_u8*      back;
} ot_queue;



/** Queue "Object" functions
  * ========================
  */

/** @brief Generic initialization routine for ot_queues.
  * @param q        (ot_queue*) Pointer to the ot_queue ADT
  * @param buffer   (ot_u8*) ot_queue data buffer
  * @param alloc    (ot_u16) allocated bytes for queue
  * @retval none
  * @ingroup Queue
  */
void q_init(ot_queue* q, ot_u8* buffer, ot_u16 alloc);


/** @brief Reposition the ot_queue pointers to a new buffer, don't change attributes
  * @param q        (ot_queue*) Pointer to the ot_queue ADT
  * @param buffer   (ot_u8*) ot_queue data buffer
  * @retval none
  * @ingroup Queue
  *
  * Most commonly used when multiple frames are in the same queue.
  */
void q_rebase(ot_queue* q, ot_u8* buffer);


/** @brief Copies one ot_queue "object" to another, without copying the data
  * @param q1       (ot_queue*) ot_queue to copy into
  * @param q2       (ot_queue*) ot_queue to copy from
  * @retval none
  * @ingroup Queue
  */
void q_copy(ot_queue* q1, ot_queue* q2);





/** Queue Info functions
  * ====================
  */

/** @brief Returns the accessible length of the queue (bytes between front and back)
  * @param q        (ot_queue*) ot_queue to determine length 
  * @retval none
  * @ingroup Queue
  */
ot_int q_length(ot_queue* q);

/** @brief Returns the active length of the queue (bytes between get and put)
  * @param q        (ot_queue*) ot_queue to determine span 
  * @retval none
  * @ingroup Queue
  */
ot_int q_span(ot_queue* q);

/** @brief Returns the amount of unused bytes at end of the queue (bytes
  *        between put and back)
  * @param q        (ot_queue*) ot_queue to determine space 
  * @retval none
  * @ingroup Queue
  */
ot_int q_space(ot_queue* q);




/** Queue Threading/Blocking functions
  * ==================================
  */
  
/** @brief Returns an estimated amount of ticks between now and when the queue
  *        will become unblocked.
  * @param q        (ot_queue*) ot_queue to inspect blocktime
  * @retval none
  * @ingroup Queue
  * @sa q_blockwrite()
  *
  * q_blocktime() returns an estimated number of ticks between now and when the
  * supplied queue will be unblocked.  This amount of ticks can be used to 
  * improve the intelligence of task scheduling, although it is merely advisory
  * and not a guarantee that a queue will be unblocked.
  * 
  * I/O drivers and tasks that may share a queue with other I/O or tasks need
  * to be careful about ruining each others' data.  A queue-user can call
  * q_blockwrite() to inform other users that the queue is not to be written-to
  * or emptied as long as the blocking user is keeping it blocked.  
  */
ot_uint q_blocktime(ot_queue* q);


/** @brief Blocks a queue against writing or emptying until unblocked
  * @param q        (ot_queue*) ot_queue to inspect blocktime
  * @retval none
  * @ingroup Queue
  * @sa q_blocktime()
  *
  * I/O drivers and tasks that may share a queue with other I/O or tasks need
  * to be careful about ruining each others' data.  A queue-user can call
  * q_blockwrite() to inform other users that the queue is not to be written-to
  * or emptied as long as the blocking user is keeping it blocked.  
  * 
  * Keep in mind that queue blocking is not more than a "gentlemen's agreement"
  * and that the data is not protected by any special means.  The blocktime
  * parameter the user must supply is indeed an advisory amount, and the user
  * may update it however frequently the author/architect determines is 
  * necessary.
  */
void q_blockwrite(ot_queue* q, ot_uint blocktime);


/** @brief Blocks the user thread until the queue is unblocked, then provides
  *        blocking to any other user trying to access it.
  * @param q        (ot_queue*) ot_queue to wait for
  * @retval none
  * @ingroup Queue
  * @sa q_unlock()
  *
  * q_lock() and q_unlock() implement a built-in type of Mutex available to 
  * queues.  In builds of OpenTag without threads, q_lock() implements a 
  * blocking wait and therefore it is perfectly functional to use q_lock() in
  * threaded as well as non-threaded builds of OpenTag.  However, in non-
  * threaded builds q_lock() can be inefficient and lead to unresponsiveness,
  * so it is not recommended for usage in deep parts of the kernel or drivers.
  *
  * After q_lock() returns in user-thread X, all other users (e.g. W,Y,Z) 
  * waiting for their own calls to q_lock() will keep waiting.  When user X 
  * calls q_unlock(), one of the other threads (W,Y,Z) will have q_lock() 
  * return and will have control of the queue.  This is how mutexes work.
  */
void q_lock(ot_queue* q);


/** @brief Unlocks a queue that the user is finished manipulating
  * @param q        (ot_queue*) ot_queue to wait for
  * @retval none
  * @ingroup Queue
  * @sa q_lock()
  *
  * See q_lock() for documentation
  */
void q_unlock(ot_queue* q);






/** Data Control functions
  * ======================
  */

/** @brief Empties the supplied ot_queue, but doesn't actually erase data
  * @param q        (ot_queue*) Pointer to the ot_queue ADT
  * @retval none
  * @ingroup Queue
  */
void q_empty(ot_queue* q);

/** @brief Rewinds active data to the front of the queue.  Overwrites old data.
  * @param q        (ot_queue*) Pointer to the ot_queue ADT
  * @retval none
  * @ingroup Queue
  *
  * q_rewind() sets getcursor to front, and moves all data between getcursor
  * and putcursor to front of queue.  Putcursor is also rewound.
  */
void q_rewind(ot_queue* q);


/** @brief Starts a queue by loading in config data
  * @param q        (ot_queue*) Pointer to the ot_queue ADT
  * @param offset   (ot_uint) bytes to offset the fist data writes from the front
  * @param options  (ot_u16) option bits.  user-defined usage.
  * @retval ot_u8*  Pointer to queue get & putcursor, or NULL if an error
  * @ingroup Queue
  */
ot_u8* q_start(ot_queue* q, ot_uint offset, ot_u16 options);






/** Data Read/Write functions
  * =========================
  */

/** @brief Returns the current getcursor position, and then moves it forward
  * @param q        (ot_queue*) Pointer to the ot_queue ADT
  * @param shift    (ot_int) bytes to move getcursor forward
  * @retval ot_u8*  Pointer to getcursor at original position
  * @ingroup Queue
  */
ot_u8* q_markbyte(ot_queue* q, ot_int shift);


/** @brief Writes a byte to a ot_queue's putcursor, and advances it
  * @param q        (ot_queue*) Pointer to the ot_queue ADT
  * @param byte_in  (ot_u8) byte to write
  * @retval none
  * @ingroup Queue
  */
void q_writebyte(ot_queue* q, ot_u8 byte_in);


/** @brief Writes a 16 bit short integer to the ot_queue's putcursor, and advances it
  * @param q        (ot_queue*) Pointer to the ot_queue ADT
  * @param short_in (ot_u16) Short integer to write
  * @retval none
  * @ingroup Queue
  * @note The _be variant will use big endian, which is fast for copying data
  *       from the UDB, given that the UDB is big endian and DASH7 also uses big
  *       endian for data streams.  The normal variant will do endian conversion
  *       in order to move integers from memory to the queue.
  */
void q_writeshort(ot_queue* q, ot_u16 short_in);
void q_writeshort_be(ot_queue* q, ot_u16 short_in);



/** @brief Writes a 32 bit long integer to the ot_queue's putcursor, and advances it
  * @param q        (ot_queue*) Pointer to the ot_queue ADT
  * @param long_in  (ot_u32) Long integer to write
  * @retval none
  * @ingroup ot_queue
  * @note The _be variant will use big endian, which is fast for copying data
  *       from the UDB, given that the UDB is big endian and DASH7 also uses big
  *       endian for data streams.  The normal variant will do endian conversion
  *       in order to move integers from memory to the queue.
  */
void q_writelong(ot_queue* q, ot_u32 long_in);



/** @brief Reads a byte at the ot_queue's getcursor, and advances it
  * @param q        (ot_queue*) Pointer to the ot_queue ADT
  * @retval ot_u8   Byte read
  * @ingroup Queue
  */
ot_u8 q_readbyte(ot_queue* q);


/** @brief Reads a 16 bit short integer at the ot_queue's getcursor, and advances it
  * @param q        (ot_queue*) Pointer to the ot_queue ADT
  * @retval ot_u16  Short integer read.
  * @ingroup Queue
  * @note The _be variant will use big endian, which is fast for copying data
  *       from the UDB, given that the UDB is big endian and DASH7 also uses big
  *       endian for data streams.  The normal variant will do endian conversion
  *       in order to move integers from memory to the queue.
  */
ot_u16 q_readshort(ot_queue* q);
ot_u16 q_readshort_be(ot_queue* q);


/** @brief Reads a 32 bit long integer at the ot_queue's getcursor, and advances it
  * @param q        (ot_queue*) Pointer to the ot_queue ADT
  * @retval ot_u32  Long integer read.
  * @ingroup Queue
  * @note The _be variant will use big endian, which is fast for copying data
  *       from the UDB, given that the UDB is big endian and DASH7 also uses big
  *       endian for data streams.  The normal variant will do endian conversion
  *       in order to move integers from memory to the queue.
  */
ot_u32 q_readlong(ot_queue* q);


void q_writestring(ot_queue* q, ot_u8* string, ot_int length);
void q_readstring(ot_queue* q, ot_u8* string, ot_int length);



#if (defined(__STDC__) || defined (__POSIX__))
void q_print();

#endif





#ifdef __cplusplus
}
#endif


#endif
