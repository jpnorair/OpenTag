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
  * @file       /otsys/_unittest/pseudofs.c
  * @author     JP Norair
  * @version    R100
  * @date       15 Apr 2022
  * @brief      Unit Test for PseudoFS
  * @ingroup    PseudoFS
  *
  *
  ******************************************************************************
  */

#if defined(__PSEUDOFS_TEST__) && defined (__UNIT_TEST__)

#include <stdio.h>
#include <assert.h>

#include <otsys/pseudofs.h>
#include <otsys/veelite.h>

#include "../pfs_ext/memmgr.h"


void test_memmgr();

///@todo create Posix unit test platform and unit test shadow app (steal from mpc)

int main(void) {

	/// 1. First test is of the optional malloc code.
	test_memmgr();

	return 0;
}




// A rudimentary test of the memory manager.
// Runs assuming default flags in memmgr.h:
//
// #define POOL_SIZE 8 * 1024
// #define MIN_POOL_ALLOC_QUANTAS 16
//
// And a 32-bit machine (sizeof(unsigned long) == 4)
//
void test_memmgr()
{
    if (sizeof(void*) != 4) {
        printf("WARNING: this test was designed for systems with pointer size = 4\n");
    }

    byte *p[30] = {0};
    int i;

    // Each header uses 8 bytes, so this allocates
    // 3 * (2048 + 8) = 6168 bytes, leaving us
    // with 8192 - 6168 = 2024
    //
    for (i = 0; i < 3; ++i)
    {
        p[i] = memmgr_alloc(2048);
        assert(p[i]);
    }

    // Allocate all the remaining memory
    //
    p[4] = memmgr_alloc(2016);
    assert(p[4]);

    // Nothing left...
    //
    p[5] = memmgr_alloc(1);
    assert(p[5] == 0);

    // Release the second block. This frees 2048 + 8 bytes.
    //
    memmgr_free(p[1]);
    p[1] = 0;

    // Now we can allocate several smaller chunks from the
    // free list. There, they can be smaller than the
    // minimal allocation size.
    // Allocations of 100 require 14 quantas (13 for the
    // requested space, 1 for the header). So it allocates
    // 112 bytes. We have 18 allocations to make:
    //
    for (i = 10; i < 28; ++i)
    {
        p[i] = memmgr_alloc(100);
        assert(p[i]);
    }

    // Not enough for another one...
    //
    p[28] = memmgr_alloc(100);
    assert(p[28] == 0);

    // Now free everything
    //
    for (i = 0; i < 30; ++i)
    {
        if (p[i])
            memmgr_free(p[i]);
    }

    memmgr_print_stats();
}


#endif
