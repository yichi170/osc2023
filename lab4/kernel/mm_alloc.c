#include "mm_alloc.h"

#include "print.h"
#include "string.h"
#include "page_alloc.h"

#define NUM_POOLS 6
#define NULL (void *)0

// static struct mm_block* reserve_memory;
static mm_pool_t pools[NUM_POOLS];

void memory_reserve(void *start, void *end) {
  if ((uint64_t)end <= (uint64_t)start) {
    print("[WARN] Memory reservation failed: The specified memory address range is invalid. ");
    printf("The starting address (%#X) must be lower than the ending address (%#X)\n",
            (uint64_t)start, (uint64_t)end);
    return;
  }

  // struct mm_block new_block = {start, end, NULL, NULL};

  // if (reserve_memory == NULL) {
  //   reserve_memory = new_block;
  // } else {
  //   struct mm_block* curr = reserve_memory;
  //   while (curr != NULL) {
  //     if (new_block->end < curr->start) {
  //       new_block->prev = curr->prev;
  //       new_block->next = curr;
  //       if (curr->prev != NULL)
  //         curr->prev->next = new_block;
  //       else
  //         reserve_memory = new_block;
  //       curr->prev = new_block;
  //       break;
  //     } else if (new_block->start > curr->end) {
  //       curr = curr->next;
  //     } else {
  //       curr->start = (new_block->start < curr->start) ? new_block->start : curr->start;
  //       curr->end = (new_block->end > curr->end) ? new_block->end : curr->end;
  //       break;
  //     }
  //   }
  //   if (curr == NULL) {
  //     new_block->prev = reserve_memory;
  //     reserve_memory->next = new_block;
  //   }
  // }
}

void init_pools() {
  for (int i = 0; i < NUM_POOLS; i++) {
    pools[i].chunk_size = 1 << (i + 5);
    pools[i].id = i;
  }
}

void *kmalloc(uint64_t size) {
  if (size > 1024)
    return frame_malloc(size);

  int reqorder = 0; // 0: 32, 1: 64, 2: 128, 3: 256, 4: 512, 5: 1024
  for (int c = 32; c < size; c <<= 1, reqorder++);
  
  if (pools[reqorder].total_free == 0) {
    mem_chunk_create(&pools[reqorder]);
  }

  return mem_chunk_alloc(&pools[reqorder]);
}

void mem_chunk_create(mm_pool_t *pool) {
  page_t *page = frame_malloc(FRAME_SIZE);
  page->next = pool->pages;
  if (pool->pages != NULL)
    pool->pages->prev = page;
  pool->pages = page; // put the new page in the head of pages

  memset(page->bitmap, 0, FRAME_SIZE - sizeof(page_t));
  page->num_chunk = (FRAME_SIZE - sizeof(page_t)) / pool->chunk_size;
  page->num_free = page->num_chunk;
  page->pool_id = pool->id;

  pool->total_free += page->num_chunk;
}

void *mem_chunk_alloc(mm_pool_t *pool) {
  page_t *page = pool->pages;
  
  while (page) {
    if (page->num_free > 0) {
      for (int i = 0; i < page->num_chunk; i++) {
        if ((page->bitmap[i / 64] & (1 << i % 64)) == 0) {
          page->bitmap[i / 64] |= (1 << (i % 64));
          page->num_free--;
          pool->total_free--;
          printf("[INFO] allocate chunk in page address: %#X\n",
                  (uint64_t)((void *)page));
          return (void *)page + sizeof(page_t) + i * pool->chunk_size;
        }
      }
    }
    page = page->next;
  }
  return NULL;
}

void mem_chunk_free(page_t *page, int idx) {
  if ((page->bitmap[idx / 64] & (1 << idx % 64)) != 0) {
    page->bitmap[idx / 64] &= ~(1 << (idx % 64));
    page->num_free++;
  }

  printf("[INFO] free chunk: idx=%d, size=%d, page address: %#X\n", 
          idx, (1 << (page->pool_id + 5)), (uint64_t)(void *)page);
}

void kfree(void *addr) {
  page_t *page = (page_t *)((uint64_t)addr & ~(FRAME_SIZE - 1));
  mm_pool_t *pool = &pools[page->pool_id];
  int idx = (uint64_t)(addr - (void *)page - sizeof(page_t)) / pool->chunk_size;

  mem_chunk_free(page, idx);

  // check whether the page is still in use
  for (int i = 0; i < 2; i++) {
    if (page->bitmap[i] != 0)
      return;
  }

  page_t *pool_pages = pool->pages;

  while (pool_pages) {
    if (pool_pages == page) {
      pool_pages->prev->next = pool_pages->next;
      pool_pages->next->prev = pool_pages->prev;
      pools[page->pool_id].pages = pool_pages->next;
      free_frame(page);
      break;
    }
    pool_pages = pool_pages->next;
  }
}

void demo_pool() {
  void *A = kmalloc(32);
  void *B = kmalloc(32);
  void *C = kmalloc(64);
  void *D = kmalloc(64);
  printf("[DEMO] kmalloc A: size=32, address=%#X\n", A);
  printf("[DEMO] kmalloc B: size=32, address=%#X\n", B);
  printf("[DEMO] kmalloc C: size=64, address=%#X\n", C);
  printf("[DEMO] kmalloc D: size=64, address=%#X\n", D);
  kfree(A);
  kfree(B);
  kfree(C);
  kfree(D);
}