/*  Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  * http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  */
/**
  * @file       /Apps/Testbed_Protocol/input_parser.c
  * @author     JP Norair
  * @version    V1.0
  * @date       13 October 2011
  * @brief      Input data parser
  * @defgroup   TB_Input_Parser
  * @ingroup    TB_Input_Parser
  *
  * Single Data Expressions: <BR>
  * Single Data Expressions are bounded by whitespace.  Hex and Decimal input
  * types are available.
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



#include "OTAPI.h"
#include <stdio.h>


/** @brief Parses a single data expression from ASCII to raw binary
  * @param stream   (FILE*) file-pointer to the input ascii
  * @param msg      (Queue*) queue-pointer to the output queue
  * @retval (int)   negative on error, else number of bytes written to queue
  * @ingroup TB_Input_Parser
  *
  * The input data expressions are documented at the head of this document
  */
int parse_sequence(FILE* stream, Queue* msg);


// Input Parser Tester (comment out when using library)
#ifdef __DEBUG__
int input_parser_main(int argc, char** argv);
#endif

