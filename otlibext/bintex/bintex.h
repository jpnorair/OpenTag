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
  */

/**
  * @file       bintex.h
  * @author     JP Norair
  * @version    V1.0
  * @date       13 May 2012
  * @brief      BinTex Parser, for clients with STD C libraries (i.e. POSIX)
  * @defgroup   BinTex
  * @ingroup    BinTex
  *
  * @note This version of BinTex is fully modular, with no other requirement.
  * 
  * "BinTex" is a markup langauge (or in modern jargon a "markdown" language)
  * for working with raw, byte-wise data.  It allows integration of multiple
  * input formats, and it will parse them into a big-endian / network-ordered
  * binary stream.
  *
  * The official BinTex reference can be found on the included Wiki link.  A 
  * quickstart reference is posted in the comments below.
  * http://www.indigresso.com/wiki/
  * 
  *
  * Single Data Expressions: <BR>
  * Single Data Expressions are of a consistent input format, and they are 
  * bounded by whitespace.  Hex and Decimal input formats are available.
  *
  * 1. Variable Length Hex: (example, x098a52B248) <BR>
  * In this mode, an ASCII string that starts with a leading "x" and ends with
  * whitespace will be parsed as a single hex data stream.  Valid characters 
  * are 0-9, a-f, and A-F.  Non valid characters will be converted to 0.  The
  * hex stream terminates when the next character read is whitespace or an end-
  * bracket "]".
  *
  * 2. 8/16/32 bit integer: (examples, d9, d-13252, d8uc, d8l ...)
  * An ASCII string that starts with a leading "d" and ends with whitespace will
  * be parsed as a single decimal number.  Valid chars are digits (0-9) and 
  * minus sign (-).  Supplied numerals are parsed into 8, 16, or 32 bits.  The
  * parser will use the minimum container to fit the supplied number unless you
  * explicitly specify a type-code at the end of the number.  The parser will 
  * consider the number terminated after it reads whitespace, an end-parenthesis
  * ")" or a supported type-code.
  * 
  * Supported Integer Type-code characters: <BR>
  * - u: unsigned (must be first) <BR>
  * - c: char (8 bits) <BR>
  * - s: short (16 bits) <BR>
  * - l: long (32 bits)
  *
  * Supported Integer Type-codes:
  * - [none]: signed, implicit length <BR>
  * - u: unsigned, implicit length <BR>
  * - uc: unsigned 8 bits <BR>
  * - us: unsigned 16 bits <BR>
  * - ul: unsigned 32 bits <BR>
  * - c: signed, 8 bits <BR>
  * - s: signed, 16 bits <BR>
  * - l: signed, 32 bits <BR>
  * 
  *
  * Multiple data expressions: <BR>
  * Multiple data expressions are bounded by open and close characters (such as
  * [], (), and "").  Whitespace can exist inside the open and close characters.
  * 
  * 1. Multiple Hex expression: <BR>
  * Use the square brackets [] to enclose one or more hex sequences.  Inside the 
  * brackets, the leading "x" is not included.  Therefore, an example can be: 
  * [0346 83c6 35 2b89 28a860f3].  If you want to load items from a struct,
  * this format can make it easier, since each element is separated.
  *
  * 2. Multiple Integer Expression: <BR>
  * Use the parenthesis () to enclose one or more decimal integers.  An example
  * can be (84 13 -93s 25026ul).
  *
  * 3. ASCII string: <BR>
  * Use the double-quotes "" to enclose an ASCII string.  The escape sequence
  * is the backslash \, and the input rules are the same as those from printf. 
  ******************************************************************************
  */

#ifndef __BINTEX_H
#define __BINTEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>


//#define __DEBUG__



/** @typedef Queue
  * 
  * The Queue data type does not contain the data in the queues themselves, just
  * information on how to get that data as well as any other useful variables.
  *
  * uint16_t alloc        Allocation of the queue data, in bytes
  *
  * uint16_t length       The current extent of the queue data, in bytes
  * Twobytes options    User flags
  * uint8_t* front        First address of queue data
  * uint8_t* back         Used for boundary checking (user adjustable)
  * uint8_t* getcursor    Cursor address for reading from queue
  * uint8_t* putcursor    Cursor address for writing to queue
  */
typedef struct {
    int         alloc;
    int         length;
    uint16_t    options;
    uint8_t*    front;
    uint8_t*    back;
    uint8_t*    getcursor;
    uint8_t*    putcursor;
} ot_queue;




/** @brief  Parse a complete Bintex File, outputting binary to stream
  * @param  file        (FILE*) input file, nominally encoded as UTF-8
  * @param  stream_out  (unsigned char*) byte-wise, binary output stream
  * @param  size        (int) allocation limit of stream_out
  * @retval (int)       negative on error, else number of bytes output to stream
  * @ingroup BinTex
  * @sa bintex_ss()
  */
int bintex_fs(FILE* file, unsigned char* stream_out, int size);



/** @brief  Parse a complete Bintex null-terminated string, outputting binary to stream
  * @param  string      (unsigned char*) input string
  * @param  stream_out  (unsigned char*) byte-wise, binary output stream
  * @param  size        (int) allocation limit of stream_out
  * @retval (int)       negative on error, else number of bytes output to stream
  * @ingroup BinTex
  * @sa bintex_fs()
  *
  * @note bintex_ss() will increment the string pointer (*string)
  */
int bintex_ss(unsigned char* string, unsigned char* stream_out, int size);



/** @brief  Iteratively parses a Bintex File, outputting to persistent Queue
  * @param  file        (FILE*) input file, nominally encoded as UTF-8
  * @param  msg         (Queue*) output Queue of binary datastream
  * @retval (int)       negative on error, else number of bytes written to queue
  * @ingroup BinTex
  * @sa bintex_iter_sq
  *
  * This function is different from bintex_fs() because it will return after
  * parsing each input BinTex expression in the file.  The File and Queue 
  * objects should be retained by the caller/user.
  */
int bintex_iter_fq(FILE* file, ot_queue* msg);



/** @brief  Iteratively parses a Bintex null-terminated string, outputting to persistent Queue
  * @param  string      (unsigned char**) input string handle
  * @param  msg         (Queue*) output Queue of binary datastream
  * @retval (int)       negative on error, else number of bytes written to queue
  * @ingroup BinTex
  * @sa bintex_iter_fq
  *
  * This function is different from bintex_ss() because it will return after
  * parsing each input BinTex expression in the input string.  The String and 
  * Queue objects should be retained by the caller/user.
  */
int bintex_iter_sq(unsigned char** string, ot_queue* msg, int size);





// Input Parser Tester
#ifdef __DEBUG__
int main(int argc, char** argv);
#endif











/** @brief Generic initialization routine for Queues.
  * @param q        (ot_queue*) Pointer to the Queue ADT
  * @param buffer   (uint8_t*) Queue data buffer
  * @param alloc    (uint16_t) allocated bytes for queue
  * @retval none
  * @ingroup Queue
  */
void q_init(ot_queue* q, uint8_t* buffer, uint16_t alloc);


/** @brief Reposition the Queue pointers to a new buffer, don't change attributes
  * @param q        (ot_queue*) Pointer to the Queue ADT
  * @param buffer   (uint8_t*) Queue data buffer
  * @retval none
  * @ingroup Queue
  *
  * Most commonly used when multiple frames are in the same queue.
  */
void q_rebase(ot_queue* q, uint8_t* buffer);



void q_copy(ot_queue* q1, ot_queue* q2);



/** @brief Empties the supplied Queue, but doesn't actually erase data
  * @param q        (ot_queue*) Pointer to the Queue ADT
  * @retval none
  * @ingroup Queue
  */
void q_empty(ot_queue* q);


/** @brief Starts a queue by loading in config data
  * @param q        (ot_queue*) Pointer to the Queue ADT
  * @param offset   (int) bytes to offset the fist data writes from the front
  * @param options  (uint16_t) option bits.  user-defined usage.
  * @retval uint8_t*  Pointer to queue get & putcursor, or NULL if an error
  * @ingroup Queue
  */
uint8_t* q_start(ot_queue* q, int offset, uint16_t options);



/** @brief Returns the current getcursor position, and then moves it forward
  * @param q        (ot_queue*) Pointer to the Queue ADT
  * @param shift    (int) bytes to move getcursor forward
  * @retval uint8_t*  Pointer to getcursor at original position
  * @ingroup Queue
  */
uint8_t* q_markbyte(ot_queue* q, int shift);


/** @brief Writes a byte to a Queue's putcursor, and advances it
  * @param q        (ot_queue*) Pointer to the Queue ADT
  * @param byte_in  (uint8_t) byte to write
  * @retval none
  * @ingroup Queue
  */
void q_writebyte(ot_queue* q, uint8_t byte_in);


/** @brief Writes a 16 bit short integer to the Queue's putcursor, and advances it
  * @param q        (ot_queue*) Pointer to the Queue ADT
  * @param short_in (uint16_t) Short integer to write
  * @retval none
  * @ingroup Queue
  * @note The _be variant will use big endian, which is fast for copying data
  *       from the UDB, given that the UDB is big endian and DASH7 also uses big
  *       endian for data streams.  The normal variant will do endian conversion
  *       in order to move integers from memory to the queue.
  */
void q_writeshort(ot_queue* q, uint16_t short_in);
void q_writeshort_be(ot_queue* q, uint16_t short_in);



/** @brief Writes a 32 bit long integer to the Queue's putcursor, and advances it
  * @param q        (ot_queue*) Pointer to the Queue ADT
  * @param long_in  (uint32_t) Long integer to write
  * @retval none
  * @ingroup Queue
  * @note The _be variant will use big endian, which is fast for copying data
  *       from the UDB, given that the UDB is big endian and DASH7 also uses big
  *       endian for data streams.  The normal variant will do endian conversion
  *       in order to move integers from memory to the queue.
  */
void q_writelong(ot_queue* q, uint32_t long_in);



/** @brief Reads a byte at the Queue's getcursor, and advances it
  * @param q        (ot_queue*) Pointer to the Queue ADT
  * @retval uint8_t   Byte read
  * @ingroup Queue
  */
uint8_t q_readbyte(ot_queue* q);


/** @brief Reads a 16 bit short integer at the Queue's getcursor, and advances it
  * @param q        (ot_queue*) Pointer to the Queue ADT
  * @retval uint16_t  Short integer read.
  * @ingroup Queue
  * @note The _be variant will use big endian, which is fast for copying data
  *       from the UDB, given that the UDB is big endian and DASH7 also uses big
  *       endian for data streams.  The normal variant will do endian conversion
  *       in order to move integers from memory to the queue.
  */
uint16_t q_readshort(ot_queue* q);
uint16_t q_readshort_be(ot_queue* q);


/** @brief Reads a 32 bit long integer at the Queue's getcursor, and advances it
  * @param q        (ot_queue*) Pointer to the Queue ADT
  * @retval uint32_t  Long integer read.
  * @ingroup Queue
  * @note The _be variant will use big endian, which is fast for copying data
  *       from the UDB, given that the UDB is big endian and DASH7 also uses big
  *       endian for data streams.  The normal variant will do endian conversion
  *       in order to move integers from memory to the queue.
  */
uint32_t q_readlong(ot_queue* q);


void q_writestring(ot_queue* q, uint8_t* string, int length);
void q_readstring(ot_queue* q, uint8_t* string, int length);











#ifdef __cplusplus
}
#endif
#endif
