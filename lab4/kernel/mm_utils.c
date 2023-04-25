#include "mm_utils.h"
#include "type.h"
#include "malloc.h"
#include "page_alloc.h"
#include "print.h"

static struct mm_block* reserve_memory;

void memory_reserve(void *start, void *end) {
  if ((uint64_t)end <= (uint64_t)start) {
    print("[WARN] Memory reservation failed: The specified memory address range is invalid. ");
    printf("The starting address (%#X) must be lower than the ending address (%#X)\n",
            (uint64_t)start, (uint64_t)end);
    return;
  }

  struct mm_block *new_block = simple_malloc(sizeof(struct mm_block));
  new_block->start = start;
  new_block->end = end;
  new_block->prev = NULL;
  new_block->next = NULL;

  if (reserve_memory == NULL) {
    reserve_memory = new_block;
  } else {
    struct mm_block* curr = reserve_memory;
    while (curr != NULL) {
      if (new_block->end < curr->start) {
        new_block->prev = curr->prev;
        new_block->next = curr;
        if (curr->prev != NULL)
          curr->prev->next = new_block;
        else
          reserve_memory = new_block;
        curr->prev = new_block;
        break;
      } else if (new_block->start > curr->end) {
        curr = curr->next;
      } else {
        curr->start = (new_block->start < curr->start) ? new_block->start : curr->start;
        curr->end = (new_block->end > curr->end) ? new_block->end : curr->end;
        break;
      }
    }
    if (curr == NULL) {
      new_block->prev = reserve_memory;
      reserve_memory->next = new_block;
    }
  }
}

void show_reserve() {
  printf("== reserved memory address ==\n");
  struct mm_block* mm_ptr = reserve_memory;
  int i = 0;
  while (mm_ptr) {
    printf("%d: %#X - %#X\n",
      i, (uint64_t)(mm_ptr->start), (uint64_t)(mm_ptr->end));
    mm_ptr = mm_ptr->next;
    ++i;
  }
  printf("=============================\n");
}

void mark_reserve_memory() {
  struct mm_block* mm_ptr = reserve_memory;
  while (mm_ptr) {
    int start = mm_addr_to_id(mm_ptr->start);
    int end = mm_addr_to_id(mm_ptr->end);
    allocate_frame_by_id_range(start, end + 1);

    mm_ptr = mm_ptr->next;
  }
}
