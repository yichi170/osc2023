#ifndef _PAGE_ALLOC_H
#define _PAGE_ALLOC_H

#include "type.h"

#define ALLOCATABLE     0          // state
#define ALLOCATED       -1         // state
#define FREE_FOR_ALLOC  -2         // val
#define NULL            (void *)0
#define MAX_ORDER       6

#define FRAME_SIZE    0x1000     // 4096 (4KB)
#define MEM_START     (uint64_t)0x10000000
#define MEM_END       (uint64_t)0x10100000
#define TOTAL_MEMORY  (MEM_END - MEM_START)
#define NUM_FRAME     (TOTAL_MEMORY / FRAME_SIZE)

struct frame {
  unsigned int index;
  int val;
  int state;
  struct frame *next, *prev;
};

void init_allocator();
int allocate_frame(unsigned int);
void deallocate_frame(int);
void *frame_malloc(uint64_t);
void free_frame(void *);
void demo_frame();

#endif
