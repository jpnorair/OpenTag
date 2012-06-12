/*  Copyright 2010-2011, JP Norair
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




#include "OTAPI.h"





/** Method Registry <BR>
  * ------------------------------------------------------------------------<BR>
  * Described in 11.1.1 of the CoAP spec.  Methods are request commands, and
  * there are 4 described in CoAP.
  */
static const ot_u8 method_1[]   = {
    3,  'G','E','T',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 method_2[]   = {
    4,  'P','O','S','T',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 method_3[]   = {
    3,  'P','U','T',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 method_4[]   = {
    6,  'D','E','L','E','T','E',
    9,  '[','R','F','C','X','X','X','X',']'
};

static const ot_u8* method_data[] = {
    NULL,
    method_1,
    method_2,
    method_3,
    method_4,
};

ot_u8* otcoap_getreg_method(ot_u8 index) {
    if (index > 4) {
        index = 0; 
    }
    return method_data[index];
}





/** Response Registry <BR>
  * ------------------------------------------------------------------------<BR>
  * Described in 11.1.2 of the CoAP spec.  Response Codes are included after 
  * completion of a request Method.
  */
static const ot_u8 response_65[] = {
    12, '2','.','0','1',' ','C','r','e','a','t','e','d',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_66[] = {
    12, '2','.','0','2',' ','D','e','l','e','t','e','d',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_67[] = {
    10, '2','.','0','3',' ','V','a','l','i','d',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_68[] = {
    12, '2','.','0','4',' ','C','h','a','n','g','e','d',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_69[] = {
    12, '2','.','0','5',' ','C','o','n','t','e','n','t',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_128[] = {
    16, '4','.','0','0',' ','B','a','d',' ','R','e','q','u','e','s','t',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_129[] = {
    17, '4','.','0','1',' ','U','n','a','u','t','h','o','r','i','z','e','d',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_130[] = {
    15, '4','.','0','2',' ','B','a','d',' ','O','p','t','i','o','n',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_131[] = {
    14, '4','.','0','3',' ','F','o','r','b','i','d','d','e','n',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_132[] = {
    14, '4','.','0','4',' ','N','o','t',' ','F','o','u','n','d',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_133[] = {
    23, '4','.','0','5',' ','M','e','t','h','o','d',' ','N','o','t',' ','A','l','l','o','w','e','d',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_134[] = {
    19, '4','.','0','6',' ','N','o','t',' ','A','c','c','e','p','t','a','b','l','e',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_140[] = {
    24, '4','.','1','2',' ','P','r','e','c','o','n','d','i','t','i','o','n',' ','F','a','i','l','e','d',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_141[] = {
    29, '4','.','1','3',' ','R','e','q','u','e','s','t',' ','E','n','t','i','t','y',' ','T','o','o',' ','L','a','r','g','e',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_143[] = {
    27, '4','.','1','5',' ','U','n','s','u','p','p','o','r','t','e','d',' ','M','e','d','i','a',' ','T','y','p','e',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_160[] = {
    26, '5','.','0','0',' ','I','n','t','e','r','n','a','l',' ','S','e','r','v','e','r',' ','E','r','r','o','r',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_161[] = {
    20, '5','.','0','1',' ','N','o','t',' ','I','m','p','l','e','m','e','n','t','e','d',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_162[] = {
    16, '5','.','0','2',' ','B','a','d',' ','G','a','t','e','w','a','y',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_163[] = {
    24, '5','.','0','3',' ','S','e','r','v','i','c','e',' ','U','n','a','v','a','i','l','a','b','l','e',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_164[] = {
    20, '5','.','0','4',' ','G','a','t','e','w','a','y',' ','T','i','m','e','o','u','t',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 response_165[] = {
    27, '5','.','0','5',' ','P','r','o','x','y','i','n','g',' ','N','o','t',' ','S','u','p','p','o','r','t','e','d',
    9,  '[','R','F','C','X','X','X','X',']'
};

///@todo put in search function once I know how this gets used





/** Option Registry <BR>
  * ------------------------------------------------------------------------<BR>
  * Described in 11.2 of the CoAP spec.  Options are present (optionally) in
  * requests.
  */
static const ot_u8 option_1[]  = { 
    12, 'C','o','n','t','e','n','t','-','T','y','p','e',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 option_2[]  = { 
    7,  'M','a','x','-','A','g','e',
    9,  '[','R','F','C','X','X','X','X',']' 
};
static const ot_u8 option_3[]  = { 
    9,  'P','r','o','x','y','-','U','r','i',
    9,  '[','R','F','C','X','X','X','X',']' 
};
static const ot_u8 option_4[]  = { 
    4,  'E','t','a','g',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 option_5[]  = { 
    8,  'U','r','i','_','H','o','s','t',
    9,  '[','R','F','C','X','X','X','X',']' 
};
static const ot_u8 option_6[]  = { 
    13, 'L','o','c','a','t','i','o','n','-','P','a','t','h',
    9,  '[','R','F','C','X','X','X','X',']' 
};
static const ot_u8 option_7[]  = { 
    8,  'U','r','i','-','P','o','r','t',
    9,  '[','R','F','C','X','X','X','X',']' 
};
static const ot_u8 option_8[]  = { 
    14  'L','o','c','a','t','i','o','n','-','Q','u','e','r','y',
    9,  '[','R','F','C','X','X','X','X',']' 
};
static const ot_u8 option_9[]  = { 
    8   'U','r','i','-','P','a','t','h',
    9,  '[','R','F','C','X','X','X','X',']' 
};
static const ot_u8 option_11[] = { 
    5,  'T','o','k','e','n',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 option_12[] = { 
    6   'A','c','c','e','p','t',
    9,  '[','R','F','C','X','X','X','X',']'
};
static const ot_u8 option_13[] = { 
    8,  'I','f','-','M','a','t','c','h',
    9,  '[','R','F','C','X','X','X','X',']' 
};
static const ot_u8 option_15[] = { 
    9   'U','r','i','-','Q','u','e','r','y',
    9,  '[','R','F','C','X','X','X','X',']' 
};
static const ot_u8 option_21[] = { 
    13, 'I','f','-','N','o','n','e','-','M','a','t','c','h',
    9,  '[','R','F','C','X','X','X','X',']' 
};

static const ot_u8* option_data[] = {
    NULL, 
    option_1,
    option_2,
    option_3,
    option_4,
    option_5,
    option_6,
    option_7,
    option_8,
    option_9,
    NULL,
    option_11,
    option_12,
    option_13,
    NULL,
    option_15,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    option_21
};

ot_u8* coapreg_get_option(ot_u8 index) {
    if (index > 21) {
        index = 0; 
    }
    return option_data[index];
}





/** Media Type Registry <BR>
  * ------------------------------------------------------------------------<BR>
  * Described in 11.3 of the CoAP spec.  Media types are identifiers of a 
  * format for request or response data.
  */
static const ot_u8 media_list[] = { 
    0, 40, 41, 47, 50
};
static const ot_u8 media_0[] = { 
    25, 't','e','x','t','/','p','l','a','i','n',';',' ','c','h','a','r','s','e','t','=','u','t','f','-','8',
    27, '[','R','F','C','2','0','4','6',']','[','R','F','C','3','6','7','6',']','[','R','F','C','5','1','4','7',']' 
};
static const ot_u8 media_40[] = { 
    23, 'a','p','p','l','i','c','a','t','i','o','n','/','l','i','n','k','-','f','o','r','m','a','t',
    27, '[','I','-','D','.','i','e','t','f','-','c','o','r','e','-','l','i','n','k','-','f','o','r','m','a','t',']'
};
static const ot_u8 media_41[] = { 
    15, 'a','p','p','l','i','c','a','t','i','o','n','/','x','m','l',
    9,  '[','R','F','C','3','0','2','3',']'
};
static const ot_u8 media_42[] = { 
    24, 'a','p','p','l','i','c','a','t','i','o','n','/','o','c','t','e','t','-','s','t','r','e','a','m',
    18, '[','R','F','C','2','0','4','5',']','[','R','F','C','2','0','4','6',']'
};
static const ot_u8 media_47[] = { 
    15, 'a','p','p','l','i','c','a','t','i','o','n','/','e','x','i',
    9,  '[','E','X','I','M','I','M','E',']'
};
static const ot_u8 media_50[] = { 
    16, 'a','p','p','l','i','c','a','t','i','o','n','/','j','s','o','n',
    9,  '[','R','F','C','4','6','2','7',']'
};

static const ot_u8 media_data[] = {
    media_0,
    media_40,
    media_41,
    media_42,
    media_47,
    media_50
}

ot_u8* coapreg_get_mediatype(ot_u8 index) {
    for (i=0; i<sizeof(media_list); i++) {
        if (media_list[i] == index) {
            return media_data[i];
        }
    }
    return NULL;
}








