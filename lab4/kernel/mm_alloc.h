#ifndef _MM_ALLOC_H
#define _MM_ALLOC_H

#include "type.h"

#define FRAME_SIZE    0x1000     // 4096 (4KB)

typedef struct page_header {
  struct page_header *prev, *next;
  uint8_t pool_id;
  uint32_t num_chunk;
  uint32_t num_free;
  uint64_t bitmap[2]; // use flexible array instead?
} page_t;

typedef struct mm_pool {
  uint8_t id;
  uint32_t chunk_size;
  uint32_t total_free;
  page_t *pages;
} mm_pool_t;

void init_pools();
void mem_chunk_create(mm_pool_t *);
void *mem_chunk_alloc(mm_pool_t *);
void mem_chunk_free(page_t *page, int idx);
void *kmalloc(uint64_t);
void kfree(void *);
void demo_pool();

#endif
