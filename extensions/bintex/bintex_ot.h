/*  Copyright 2008-2022, JP Norair
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
  * @file       bintex.h
  * @author     JP Norair
  * @version    V1.0
  * @date       13 May 2012
  * @brief      BinTex Parser, for clients with STD C libraries (i.e. POSIX)
  * @defgroup   BinTex
  * @ingroup    BinTex
  *
  * @note Bintex module requires the OpenTag ot_queue, /otlib/queue.h (queue.c).
  *       You must include the ot_queue module in your makefile
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

#include <otlib/queue.h>
#include <stdio.h>


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
  * @param  string      (unsigned char**) input string handle
  * @param  stream_out  (unsigned char*) byte-wise, binary output stream
  * @param  size        (int) allocation limit of stream_out
  * @retval (int)       negative on error, else number of bytes output to stream
  * @ingroup BinTex
  * @sa bintex_fs()
  *
  * @note bintex_ss() will increment the string pointer (*string)
  */
int bintex_ss(unsigned char** string, unsigned char* stream_out, int size);



/** @brief  Iteratively parses a Bintex File, outputting to persistent ot_queue
  * @param  file        (FILE*) input file, nominally encoded as UTF-8
  * @param  msg         (ot_queue*) output ot_queue of binary datastream
  * @retval (int)       negative on error, else number of bytes written to queue
  * @ingroup BinTex
  * @sa bintex_iter_sq
  *
  * This function is different from bintex_fs() because it will return after
  * parsing each input BinTex expression in the file.  The File and ot_queue 
  * objects should be retained by the caller/user.
  */
int bintex_iter_fq(FILE* file, ot_queue* msg);



/** @brief  Iteratively parses a Bintex null-terminated string, outputting to persistent ot_queue
  * @param  string      (unsigned char**) input string handle
  * @param  msg         (ot_queue*) output ot_queue of binary datastream
  * @retval (int)       negative on error, else number of bytes written to queue
  * @ingroup BinTex
  * @sa bintex_iter_fq
  *
  * This function is different from bintex_ss() because it will return after
  * parsing each input BinTex expression in the input string.  The String and 
  * ot_queue objects should be retained by the caller/user.
  */
int bintex_iter_sq(unsigned char** string, ot_queue* msg, int size);




#ifdef __cplusplus
}
#endif
#endif
