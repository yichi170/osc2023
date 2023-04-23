#ifndef _MM_UTILS_H
#define _MM_UTILS_H

#define NULL (void *)0

struct mm_block {
  void *start, *end;
  struct mm_block *prev, *next;
};

void memory_reserve(void *, void *);
void show_reserve();
void mark_reserve_memory();

#endif
