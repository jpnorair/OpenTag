/* Copyright 2016 JP Norair
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
  * @file       /otsys/system_queue.c
  * @author     JP Norair
  * @version    R102
  * @date       20 Oct 2016
  * @brief      System Queue implementation
  * @ingroup    System
  *
  * The "System Queue" is a queue for scheduling tasks or processes.  It is a
  * general purpose queue with insertion sorting and other such features.  The
  * Mode 2 Session system is an adaptation of the System Queue.
  *
  ******************************************************************************
  */


#include <otsys/sysqueue.h>
#include <otlib/memcpy.h>
#include <otlib/rand.h>

// For testing purposes, even though this should be included in the platform
#if (defined(__STDC__) || defined (__POSIX__))
#   include <stdio.h>
#endif


#define _1ST    0
#define _2ND    1
#define _LAST   (sq->size-1)
#define _END    (sq->size)

#define _HEAP_1ST(SQ)   (SQ->heap[0])
#define _HEAP_2ND(SQ)   (SQ->heap[1])   
#define _HEAP_LAST(SQ)  (SQ->heap[SQ->size-1])
#define _HEAP_END(SQ)   (SQ->heap[SQ->size])        // Position of Terminator Node


typedef struct {
    void*   handle;
    ot_u32  counter;
} sqnodeC32;

#define _Counter32(NODE)    (((sqnodeC32*)(NODE))->counter)






/** sq_init() <BR>
  * ======================================================================= <BR>
  */

#ifndef EXTF_sq_init
void sq_init(ot_sq* sq, ot_sqnode* array, ot_uint size) {
    sq->length  = 0;
    sq->size    = size;
    sq->heap    = array;
    sq->top     = &array[size];
}
#endif



/** sq_activate...() functions <BR>
  * ======================================================================= <BR>
  * "Activate" functions find the counter value on the top node, return it,
  * and they set the stored couter value to zero.
  */

#ifndef EXTF_sq_activate
OT_WEAK ot_uint sq_activate(ot_sq* sq) {
    ot_uni32 cnt32;
    cnt32.ulong = sq_activate_cnt32(sq);
    return cnt32.ushort[0];
}
#endif

#ifndef EXTF_sq_activate_cnt32
OT_WEAK ot_u32 sq_activate_cnt32(ot_sq* sq) {
    ot_u32 wait = 0xFFFFFFFF;
    if (sq->length != 0) {
        wait                = _Counter32(sq->top);
        sq->top->counter    = 0;
    }
    return wait;
}
#endif



/** sq_clock...() functions <BR>
  * ======================================================================= <BR>
  * "Clock" functions subtract an amount of ticks from each node counter.
  * Lowest value is 0.  They return the counter value of the top node.
  */

#ifndef EXTF_sq_clock
OT_WEAK ot_uint sq_clock(ot_sq* sq, ot_u16 ticks) {
    ot_sqnode* node = sq->top;
    while (node < &_HEAP_END(sq)) {
        if (node->counter != 0) {
            node->counter -= ticks;
        }
        node++;   
    }
    return sq->top->counter;
}
#endif

#ifndef EXTF_sq_clock_cnt32
OT_WEAK ot_u32 sq_clock_cnt32(ot_sq* sq, ot_u32 ticks) {
    ot_sqnode* node = sq->top;
    while (node < &_HEAP_END(sq)) {
        if (_Counter32(node) != 0) {
            _Counter32(node) -= ticks;
        }
        node++;   
    }
    return _Counter32(sq->top);
}
#endif






ot_sqnode* sub_storenode(ot_sqnode* store, ot_sqnode* nodedata) {
/// Attach node to location specified
    *store = *nodedata;
    return store;
}

ot_sqnode* sub_storenode_fromargs(ot_sqnode* store, void* handle, ot_u32 cntext) {
/// Attach node to location specified
    store->handle       = handle;
    _Counter32(store)   = cntext;
    return store;
}




/** sq_new...() functions <BR>
  * ======================================================================= <BR>
  * sq_new...() variants use a binary insertion sort to place a new node in
  * the right place in the queue.  The client gets a comparison function
  * callback in order to determine how the queue should be sorted.
  */

#ifndef EXTF_sq_new
OT_WEAK ot_sqnode* sq_new(ot_sq* sq, ot_sqcmp cmpfn, ot_sqnode* node) {
    ot_sqnode*  pos;

    // Always reserve an extra node for extension.
    // i.e. There must be two or more free nodes to do sq_new()
    if (sq->top <= &_HEAP_2ND(sq)) {
        return NULL;
    }
    
    // If the queue is empty, then no search needed, just dump at the end.
    pos = &_HEAP_LAST(sq);
    if (sq->length == 0) {
        sq->top--;
    }
    
    // If queue not empty, some data must be shifted.
    // If a comparison function is supplied, it will be used to implement 
    // binary insertion sort.  If no function is supplied, data will just get
    // sent to the back of the queue.
    else {
        int copybytes;
        ot_u8* src;
        
        if (cmpfn != NULL) {
            ot_int lo = 0;
            ot_int hi = sq->length - 1;
            ot_int i;
            ot_int cmp;
            
            while (lo <= hi) {
                i   = lo + ((hi - lo) >> 1);
                cmp = cmpfn(&sq->top[i], node);
                if (cmp == 0) {
                    break;
                }
                if (cmp > 0) {      // search next in lower part
                    hi = i - 1;
                    continue;   
                }
                lo = i + 1;         // search next in upper part
            }
            
            // Due to the way the copying works, we want to decrement the node 
            // position only if the new node is less than the last comparison
            i -= (cmp > 0);
            //printf("cnt=%X, i=%d (%d)\n", node->counter, i, sq->length);
            pos = &sq->top[i];
        }
        
        // Shift forward all nodes ahead of the new node, then write the new node
        // into the newly available slot.
        src         = (ot_u8*)sq->top;
        sq->top--;
        copybytes   = ((ot_u8*)pos - (ot_u8*)sq->top);
        if (copybytes > 0) {
            //printf("memcpy( %X, %X, %d)\n", (unsigned int)sq->top, (unsigned int)src, copybytes);
            memcpy( (ot_u8*)sq->top, src, copybytes);
        }
    }
        
    // Store the node data into the queue, at "pos," and increase length
    sq->length++;
    return sub_storenode(pos, node);
}
#endif

#ifndef EXTF_sq_new_fromcnt32
OT_WEAK ot_sqnode* sq_new_fromcnt32(ot_sq* sq, ot_sqcmp cmpfn, void* handle, ot_u32 wait32) {
/// Wrapper to load to local node
    ot_sqnode node;
    node.handle         = handle;
    _Counter32(&node)   = wait32;
    return sq_new(sq, cmpfn, &node);    
}
#endif

#ifndef EXTF_sq_new_fromargs
OT_WEAK ot_sqnode* sq_new_fromargs(ot_sq* sq, ot_sqcmp cmpfn, void* handle, ot_u16 wait, ot_u16 ext) {
/// Wrapper for breaking-out wait and ext
    ot_uni32 cntext;
    cntext.ushort[0] = wait;
    cntext.ushort[1] = ext;
    return sq_new_fromcnt32(sq, cmpfn, handle, cntext.ulong);
}
#endif








/** sq_extend...() functions <BR>
  * ======================================================================= <BR>
  * sq_extend...() variants use a linear search starting from the front of
  * the queue.  If you have a bunch of sequenced nodes that must stay in
  * sequence you use extend to plop a new node after them.
  */

#ifndef EXTF_sq_extend
OT_WEAK ot_sqnode* sq_extend(ot_sq* sq, ot_sqcond condfn, ot_sqnode* node) {
    ot_sqnode* pos;

    // If not one free node, there's no room!
    if (sq->top <= &_HEAP_1ST(sq)) {
        return NULL;
    }

    // We're adding a new node...
    sq->top--;
    pos = sq->top;
    sq->length++;

    // Only do linear extension search if there is at least one pre-existing
    // node in the queue.  If not, search is not necessary.
    if (sq->length > 1) {
        do {
            pos++;              // Pre-increment node
            if (condfn(pos)) {   // stop search on extension-check == True
                break;
            }
        } while (pos < &_HEAP_LAST(sq));

        // Shift the stack down one notch, from top to extension point.
        memcpy( (ot_u8*)sq->top,
                (ot_u8*)&sq->top[1],
                (ot_uint)((ot_u8*)pos - (ot_u8*)sq->top) );
    }

    return sub_storenode(pos, node);
}
#endif

#ifndef EXTF_sq_extend_fromcnt32
OT_WEAK ot_sqnode* sq_extend_fromcnt32(ot_sq* sq, ot_sqcond condfn, void* handle, ot_u32 wait32) {
/// Wrapper to load to local node
    ot_sqnode node;
    node.handle         = handle;
    _Counter32(&node)   = wait32;
    return sq_extend(sq, condfn, &node); 
}
#endif

#ifndef EXTF_sq_extend_fromargs
OT_WEAK ot_sqnode* sq_extend_fromargs(ot_sq* sq, ot_sqcond condfn, void* handle, ot_u16 wait, ot_u16 ext) {
/// Wrapper for breaking-out wait and ext
    ot_uni32 cntext;
    cntext.ushort[0] = wait;
    cntext.ushort[1] = ext;
    return sq_extend_fromcnt32(sq, condfn, handle, cntext.ulong);
}
#endif




/** sq data API functions <BR>
  * ======================================================================= <BR>
  */

#ifndef EXTF_sq_pop
OT_WEAK void sq_pop(ot_sq* sq) {
/// Boundary checked pointer increment to pop a node
    if (sq->length != 0) {
        sq->length--;
        sq->top++;
    }
}
#endif

#ifndef EXTF_sq_flush
OT_WEAK void sq_flush(ot_sq* sq, ot_sqcond condfn) {
    while (sq->length != 0) {
        sq->length--;
        if (condfn(sq->top)) {
            break;
        }
        sq->top++;
    }
}
#endif

#ifndef EXTF_sq_top
OT_WEAK ot_sqnode* sq_top(ot_sq* sq) {
    return sq->top;
}
#endif

#ifndef EXTF_sq_follower
OT_WEAK ot_sqnode* sq_follower(ot_sq* sq) {
    return (sq->length > 1) ? &sq->top[1] : NULL;
}
#endif

#ifndef EXTF_sq_numfree
OT_WEAK ot_int sq_numfree(ot_sq* sq) {
/// Additional -1 because we always keep one free for extensions
    return sq->size - sq->length - 1;
}
#endif

#ifndef EXTF_sq_notempty
OT_WEAK ot_bool sq_notempty(ot_sq* sq) {
    return (sq->length != 0);
}
#endif





#if (defined(__STDC__) || defined (__POSIX__))

OT_WEAK void sq_print(ot_sq* sq) {
    unsigned int i;
    ot_sqnode* test;

    i = ((ot_u8*)&_HEAP_END(sq) - (ot_u8*)sq->top);
    printf("Counted Length of Heap in Nodes:    %lu\n", i/sizeof(ot_sqnode));
    printf("Recorded Heap Length (Size):        %hu (%hu)\n", sq->length, sq->size);

    if (i > 0) {
        printf("===  CNT    EXT    HANDLE\n");
        test = sq->top;
        do {
            printf("%02lu:  0x%04X 0x%04X 0x%X\n",
                ((unsigned int)((ot_u8*)test - (ot_u8*)sq->top)) / sizeof(ot_sqnode),
                test->counter,
                test->ext,
                (unsigned int)test->handle);

            test++;
        } while (test < &_HEAP_END(sq));
    }

    printf("\n");
}



/* For your amusement, if you want to test this module on its own.
 */
#if defined(__UNITTEST_SYSQUEUE__)

ot_int _cnt16_cmp(ot_sqnode* a, ot_sqnode* b) {
    ot_int x, y;
    x = (a->counter >= b->counter);
    y = (a->counter <= b->counter);
    //printf("cmp: a=%04X, b=%04X, result=%d\n", a->counter, b->counter, x-y);
    return x-y;
}

ot_bool _evenext_cond(ot_sqnode* a) {
    return (ot_bool)~(a->ext & 1);
}

ot_bool _false_cond(ot_sqnode* a) {
    return False;
}

int main(void) {
    ot_sq       test_sq;
    ot_sqnode   sq_heap[10];
    ot_sqnode*  node;

    sq_init(&test_sq, sq_heap, 10);
    
    printf("sizeof(ot_sqnode)   = %lu\n", sizeof(ot_sqnode));
    printf("sq->heap            = %016X\n", (unsigned int)test_sq.heap);
    printf("sq->top             = %016X\n\n", (unsigned int)test_sq.top);
    
    sq_print(&test_sq);
    
    node = sq_new_fromargs(&test_sq, &_cnt16_cmp, NULL, 0x1234, 0x1234);
    node = sq_new_fromargs(&test_sq, &_cnt16_cmp, NULL, 0x0101, 0x0101);
    node = sq_new_fromargs(&test_sq, &_cnt16_cmp, NULL, 0x8310, 0x8310);
    node = sq_new_fromargs(&test_sq, &_cnt16_cmp, NULL, 0x5555, 0x5555);
    sq_print(&test_sq);
    
    node = sq_new_fromargs(&test_sq, &_cnt16_cmp, NULL, 0x0700, 0x0700);
    sq_print(&test_sq);
    
    sq_clock(&test_sq, 0x0100);
    sq_print(&test_sq);
    
    node = sq_extend_fromargs(&test_sq, &_evenext_cond, NULL, 0, 0);
    sq_print(&test_sq);

    sq_flush(&test_sq, &_false_cond);
    sq_print(&test_sq);

    node = sq_new_fromargs(&test_sq, &_cnt16_cmp, NULL, 0x0500, 0x0500);
    sq_print(&test_sq);

    return 0;
}

#endif


#endif




