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
  * @file       /hb_asapi/hb_asapi_cliutils.h
  * @author     JP Norair
  * @version    R100
  * @date       1 Oct 2013
  * @brief      ASAPI Client Utilities
  * @ingroup    hb_asapi
  *
  ******************************************************************************
  */



#include <alp/asapi.h>

#if (OT_FEATURE(CLIENT))


/** @brief Create a new dialog template from simplified primitives
  * @param dialog       (asapi_dialog_tmpl*) Pointer to allocated dialog
  * @param channel      (ot_u8) channel code to use for request
  * @param resp_window  (ot_u16*) Number of ticks for response window duration
  * @retval ot_u16      actual window duration in ticks, after conversion to 
  *                     EXP+MANT form
  * @ingroup asapi
  *
  * This builds a *simplified* dialog template.  The subnet is fixed to 0xFF, 
  * which is a general purpose subnet.  As well, the request and response 
  * channels are always the same.
  *
  * The response window is encoded into a 7bit exponent + mantissa form, i.e.
  * scientific notation.  The return value is the corrected response window.
  * The actual formula is:
  * Resp window = { (4^EXP)*(MANT+1)    if EXP!=0
  *                 (4^EXP)*(MANT)      if EXP==0 }
  * Ranges:
  * 0<=EXP<=7, 0<=MANT<=15
  *
  * So, for short response windows, you can specify listening in 4 tick 
  * increments up to 64 ticks.  For longer windows, 256 tick increments up to 
  * 4096 ticks (4 seconds).  Many other combinations are possible above, below,
  * and in between.
  */
ot_u16 asapi_new_dialog_ez(asapi_dialog_tmpl* dialog, ot_u8 channel, ot_u16 resp_window);





/** @brief Create a search template to find Bintex substring in recipient FS
  * @param search       (asapi_search_tmpl*) Pointer to allocated search
  * @param bt_string    (ot_u8*) Bintex string to use as search token
  * @param block        (ot_u8) file block for search (2=ISS, 3=ISF)
  * @param file         (ot_u8) file id for search
  * @retval ot_int      Zero on success
  * @ingroup asapi
  *
  * This builds a FIND operation into a search template.  FINDING doesn't
  * return any data, it just verifies that it is there.  It acts as a filter.
  *
  * Bintex is a text-based markup language for stream representation.  Bintex
  * is included with OpenTag as a POSIX STD C library, in /otlibext/bintex.
  *
  * asapi_new_btsearch() returns 0 if the search template was filled without
  * issue.  Else, it will return the index of the argument that proved to be
  * the problem (i.e. search = 1, bt_string = 2, etc).
  */
ot_int asapi_new_btsearch(asapi_search_tmpl* search, ot_u8* bt_string, ot_u8 block, ot_u8 file);









#endif

