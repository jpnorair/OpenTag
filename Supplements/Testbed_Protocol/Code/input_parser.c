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
  * @defgroup   Input_Parser
  * @ingroup    Input_Parser
  *
  ******************************************************************************
  */

#include "input_parser.h"


#define IS_WHITESPACE(VAL)  ((VAL==' ')||(VAL=='\r')||(VAL=='\n')||(VAL=='\t'))




typedef enum {
    DATA_EOF = 0,
    DATA_error,
    DATA_lineterm,
    DATA_comment,
    DATA_ascii,
    DATA_binnum,
    DATA_hexnum,
    DATA_hexblock,
    DATA_decnum,
    DATA_decblock
} Data_type;




Data_type sub_parse_header(FILE* stream);
int sub_passcomment(FILE* stream);
int sub_getascii(FILE* stream, Queue* msg);
int sub_gethexblock(FILE* stream, Queue* msg);
int sub_getdecblock(FILE* stream, Queue* msg);
int sub_gethexnum(int* status, FILE* stream, Queue* msg);
char sub_char2hex(char* output, char input);
int sub_getdecnum(int* status, FILE* stream, Queue* msg);
int sub_buffernum(int* status, FILE* stream, char* buf, int limit);








int parse_sequence(FILE* stream, Queue* msg) {
    int status;

    switch (sub_parse_header(stream)) {
        case DATA_EOF:      return -1;
        case DATA_error:    return -2;
        case DATA_lineterm: return -3;
        case DATA_comment:  return sub_passcomment(stream);
        case DATA_ascii:    return sub_getascii(stream, msg);
        case DATA_binnum:   return sub_getbinnum(&status, stream, msg);
        case DATA_hexnum:   return sub_gethexnum(&status, stream, msg);
        case DATA_hexblock: return sub_gethexblock(stream, msg);
        case DATA_decnum:   return sub_getdecnum(&status, stream, msg);
        case DATA_decblock: return sub_getdecblock(stream, msg);
    }
    
    return -2;
}




// Input Parser Tester (comment out when using library)
#ifdef DEBUG_ON
int input_parser_main(int argc, char** argv) {
    FILE*   fp;
    Queue   msg;
    ot_u8   msg_buffer[512];
    ot_bool usage_error = True;
    
    if (argc > 1) {
        fp = fopen(argv[1], "r");
        
        if (fp != NULL) {
            usage_error = False;
        }
        else {
             fprintf(stderr, "Error, could not open file: %s\n", argv[1]);
        }
    }
    
    if (usage_error) {
        fprintf(stderr, "Usage: tbprotocol [filename]\n");
        return 0;
    }
    
    q_init(&msg, msg_buffer, 512);
    
    while (1) {
        int test;
        int i;
        ot_u8* s;
        
        test = parse_sequence(fp, &msg);
        
        if (test < 0) break;
        
        fprintf(stdout, "Data written to Queue\n");
        
        for (s=(msg.putcursor-test), i=0; s<msg.putcursor; s++, i++) {
            if ((i & 3) == 0) {
                fprintf(stdout, "%04X: ", (ot_u16)s);
            }
            
            fprintf(stdout, "%02X ", *s);
            
            if ((i & 3) == 3) {
                fputc('\n', stdout);
            }
        } 
        
        if ((i & 3) != 0) {
            fputc('\n', stdout);
        }
        
        fputc('\n', stdout);
    }
    
    
    
    
    return 0;
}
#endif















Data_type sub_parse_header(FILE* stream) {
    char next;

    parse_header_getchar:
    next = fgetc(stream);
    switch (next) {
        case '\n':
        case '\r':
        case '\t':
        case '0':
        case ' ':   goto parse_header_getchar;
        
        case '#':   return DATA_comment;
        case '"':   return DATA_ascii;
        case 'b':   return DATA_binnum;
        case 'x':   return DATA_hexnum;
        case '[':   return DATA_hexblock;
        case 'd':   return DATA_decnum;
        case '(':   return DATA_decblock;
        
        case ';':   return DATA_lineterm;
        case -1:    return DATA_EOF;
        
        default:    return DATA_error;
    }
}



int sub_passcomment(FILE* stream) {
    char subcomment[8];
    int next;
    int i = 0;
    FILE* outfp = NULL;
    int action = 0;
    
    //buffer subcomment
    while (i<8) {
        next            = fgetc(stream);
        subcomment[i++] = next;
        
        switch (next) {
            case -1:    return -1;
            case '\n':  return 0;
            case '\r':
            case '\t':
            case ' ':   goto sub_passcomment_passws;
            
            //check subcomment (this could grow in the future)
            case '>':   outfp   = stdout;
                        action  = 1;
                        goto sub_passcomment_passws;
        }
    }
    
    //bypass whitespace after subcomment
    sub_passcomment_passws:
    next = fgetc(stream);
    switch (next) {
        case -1:    return -1;
        case '\n':  return 0;
        case '\r':
        case '\t':
        case ' ':   goto sub_passcomment_passws;
    }
    
    // do something with the comment, if action requires
    while (next >= 0) {
        switch (action) {
            case 1: fputc(next, outfp); 
                    break;
        }
        if (next == '\n') 
            return 0;
        
        next = fgetc(stream);
    } 
    
    return -1;
}





int sub_getascii(FILE* stream, Queue* msg) {
    char    next;
    int     bytes_written = msg->length;
    
    while (1) {
        next = fgetc(stream);
        
        if (next == '"') {
            break;   
        }
        
        if (next == '\\') {
            switch (fgetc(stream)) {
                case 'a':   next = '\a';    break;
                case '\\':  next = '\\';    break;
                case 'b':   next = '\b';    break;
                case 'r':   next = '\r';    break;
                case '"':   next = '\"';    break;
                case 'f':   next = '\f';    break;
                case 't':   next = '\t';    break;
                case 'n':   next = '\n';    break;
                case '0':   next = '\0';    break;
                case '\'':  next = '\'';    break;
                case 'v':   next = '\v';    break;
                case '?':   next = '\?';    break;
            } 
        }
        
        q_writebyte(msg, next);
    }
    
    bytes_written = (msg->length - bytes_written);
    return bytes_written;
}




int sub_gethexblock(FILE* stream, Queue* msg) {
    int status = 0;
    int bytes_written = msg->length;

    while (status == 0) {
        sub_gethexnum(&status, stream, msg);
    }

    bytes_written = msg->length - bytes_written;
    return bytes_written;
}




int sub_getdecblock(FILE* stream, Queue* msg) {
    int status = 0;
    int bytes_written = msg->length;

    while (status == 0) {
        sub_getdecnum(&status, stream, msg);
    }

    bytes_written = msg->length - bytes_written;
    return bytes_written;
}



int sub_getbinnum(int* status, FILE* stream, Queue* msg) {
    int     digits;
    int     i = 0;
    int     shift;
    char    next;
    char    byte;
    char    buf[33];
    
    digits = sub_buffernum(status, stream, buf, 32);
    
    // If the length of digits is not byte-aligned, pad first byte
    shift = (digits & 7);
    
    do {
        while (shift > 0) {
            shift--;
            next    = buf[i++];
            next   &= 1;    // '0' = 48, '1' = 49, so just take lsb
            byte   |= next << shift;
        }
        if (i != 0) {
            q_writebyte(msg, byte);
        }
        shift   = 8;
        byte    = 0;
    } 
    while (i < digits);
    
    return (i+7)/8;
}



int sub_gethexnum(int* status, FILE* stream, Queue* msg) {
    int     digits;
    int     i = 0;
    char    next;
    char    buf[33];
    
    digits = sub_buffernum(status, stream, buf, 32);
    
    // If the length of digits is odd, write the first hex nibble as a byte
    if (digits & 1) {      
        if (sub_char2hex(&next, buf[i++]) != 0) {
            next = 0;
        }
        q_writebyte(msg, next);
    }
    
    while (i < digits) {       
        char byte_data;
        
        if (sub_char2hex(&next, buf[i++]) != 0) {
            next = 0;
        }
        byte_data = (next << 4) & 0xF0;
        
        if (sub_char2hex(&next, buf[i++]) != 0) {
            next = 0;
        }
        byte_data |= next & 0x0F;
        
        q_writebyte(msg, byte_data);        
    }
    
    return (i+1)/2;
}




char sub_char2hex(char* output, char input) {
    int status = 0;
    *output = input;

    if ((input >= '0') && (input <= '9')) {
        *output -= '0';
    }
    else if ((input >= 'a') && (input <= 'f')) {
        *output -= ('a' - 10);
        //*output += 10;
    }
    else if ((input >= 'A') && (input <= 'F')) {
        *output -= ('A' - 10);
        //*output += 10;
    }
    else {
        status = 255;  //input error
    }
    
    return status;
}





int sub_getdecnum(int* status, FILE* stream, Queue* msg) {
    int     digits;
    char    next;
    char    buf[16];
    int     sign    = 1;
    int     force_u = 0;
    int     number  = 0;
    int     i       = 0;
    int     size    = 0;
    
    // Buffer until whitespace or ')' delimiter 
    digits = sub_buffernum(status, stream, buf, 15);
    
    // Deal with leading minus sign
    if (buf[i] == '-') {
        sign = -1;
        i++;
    }
    
    // Go through the digits & footer
    // - load in numerical value, one digit at a time
    // - also look for the type footer: ul, us, uc, u, l, s, c, or none
    while (i < digits) {
        if ((buf[i] >= '0') && (buf[i] <= '9')) {
            number *= 10;
            number += (buf[i++] - '0');
        }
        else {
            force_u = (buf[i] == 'u');
            i      += force_u;

            if (buf[i] == 'c')      size = 1;   // c: char (1 byte)
            else if (buf[i] == 's') size = 2;   // s: short (2 bytes)
            else if (buf[i] == 'l') size = 3;   // l: long (4 bytes)
            break;
        }
    }
    
    // Determine size in case where footer is not explicitly provided
    if (size == 0) {
        int j;
        int bound[] = {128, 256, 32768, 65536, 0, 0};
        int max     = number - (sign < 0);
        
        for (j=force_u, size=1;  ; j+=2, size++) {
            if ((bound[j]==0) || (bound[j]>=max)) break;
        }
    }

    number *= sign;

    switch (size & 3) {
        case 0:
        case 1: q_writebyte(msg, (ot_u8)number);
                break;
        
        case 2: q_writeshort(msg, (ot_u16)number);
                break;
        
        case 3: size = 4;
                q_writelong(msg, (ot_u32)number);
                break;
    }
    
    return size;
}


                
                



int sub_buffernum(int* status, FILE* stream, char* buf, int limit) {
    int digits;
    digits  = 0;
    *status = 0;
        
    while(digits < limit) {
        buf[digits] = fgetc(stream);
        
        if ((buf[digits] == ')') || (buf[digits] == ']')) {
            *status = 255;
            break;
        }
        if (IS_WHITESPACE(buf[digits])) {
            break;
        }
        
        digits++;
    }
    
    return digits;
}




