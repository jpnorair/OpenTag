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



ot_int coap_parse_message(Queue* in_q, Queue* out_q) {
    
    /// 1. First Byte:  <BR>
    /// b7:6    Version Number (always 01)
    /// b5:4    Type (Confirmable=0, Non-Confirmable=1, Acknowledgement=2, Reset=3)
    /// b3:0    Option Count
    scratch = q_readbyte(in_q);
    options = scratch & 0xF;
    type    = (scratch >> 4) & 0x03;
    
    if ((scratch & 0xC0) != 0x40) {
        return -1;  // Bad Version
    }
    
    
    
    

}



ot_int sub_parse_option(in_q) {
    ot_int length;
    
    
    
}