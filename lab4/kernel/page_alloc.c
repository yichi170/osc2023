#include "page_alloc.h"

#include "math.h"
#include "print.h"
#include "malloc.h"

static struct frame** frame_list;
static struct frame*  frame_array;

void init_allocator() {
  frame_array = simple_malloc(NUM_FRAME * sizeof(struct frame));
  printf("NUM_FRAME: %d\n", NUM_FRAME);

  for (int i = 0; i < NUM_FRAME; i++) {
    frame_array[i].val = FREE_FOR_ALLOC;
    frame_array[i].index = i;
    frame_array[i].next = NULL;
    frame_array[i].prev = NULL;
    frame_array[i].state = ALLOCATABLE;
  }
  frame_array[0].val = MAX_ORDER;

  frame_list = simple_malloc((MAX_ORDER + 1) * sizeof(struct frame *));

  frame_list[0] = &frame_array[0];
  for (int i = 1; i <= MAX_ORDER; i++)
    frame_list[i] = NULL;
}

int allocate_frame(unsigned int reqorder) {
  if (reqorder > MAX_ORDER || reqorder < 0) {
    printf("[ERR] request order larger than MAX_ORDER or smaller than 0\n");
    return -1;
  }

  int upper_free_order = -1;
  if (frame_list[MAX_ORDER - reqorder] == NULL) {
    /* search the upper free frame */
    for (int i = reqorder + 1; i <= MAX_ORDER; i++) {
      if (frame_list[MAX_ORDER - i] != NULL) {
        upper_free_order = i;
        break;
      }
    }

    if (upper_free_order == -1)
      return -1;

    while (upper_free_order > reqorder) {
      /* move the upper free frame to the next, and ready to split the current upper free frame */
      struct frame *l_frame = frame_list[MAX_ORDER - upper_free_order];
      frame_list[MAX_ORDER - upper_free_order] = l_frame->next;
      if (frame_list[MAX_ORDER - upper_free_order] != NULL)
        frame_list[MAX_ORDER - upper_free_order]->prev = NULL;
      
      int r_off = pow(2, l_frame->val - 1);

      struct frame *r_frame = &frame_array[l_frame->index + r_off];

      l_frame->val -= 1;
      l_frame->prev = NULL;
      l_frame->next = r_frame;
      l_frame->state = ALLOCATABLE;

      r_frame->val = l_frame->val;
      r_frame->prev = l_frame;
      r_frame->next = NULL;
      r_frame->state = ALLOCATABLE;

      upper_free_order--;
      frame_list[MAX_ORDER - upper_free_order] = l_frame;
    }
  }

  int index = frame_list[MAX_ORDER - reqorder]->index;
  for (int i = 0; i < (1 << reqorder); i++) {
    frame_array[index + i].state = ALLOCATED;
  }

  /* move the ret out of the frame_list[MAX_ORDER - reqorder] */
  struct frame *ret = frame_list[MAX_ORDER - reqorder];
  frame_list[MAX_ORDER - reqorder] = frame_list[MAX_ORDER - reqorder]->next;
  frame_list[MAX_ORDER - reqorder]->prev = NULL;

  ret->prev = NULL;
  ret->next = NULL;

  printf("[INFO] allocated page frame: index=%d, order=%d\n", ret->index, reqorder);

  return ret->index;
}

void deallocate_frame(int idx) {
  struct frame *target = &frame_array[idx];

  if (target->state != ALLOCATED) {
    printf("[WARN] The frame isn't allocated, so no need to free.\n");
    return;
  }

  int target_order = target->val;

  for (int order = target_order; order < MAX_ORDER; order++) {
    int buddy_idx = idx ^ (1 << order);
    struct frame *buddy = &frame_array[buddy_idx];

    if (buddy->state == ALLOCATABLE && order < MAX_ORDER && order == target->val) {
      printf("[INFO] merge frame %d with frame %d (order: %d -> %d)\n", idx, buddy_idx, order, order + 1);

      /* remove buddy from the frame_list[order] */
      if (buddy->prev != NULL) // buddy isn't the head of frame_list[order]
        buddy->prev->next = buddy->next;
      else
        frame_list[MAX_ORDER - order] = buddy->next;

      if (buddy->next != NULL)
        buddy->next->prev = buddy->prev;

      buddy->prev = NULL;
      buddy->next = NULL;

      target->state = ALLOCATABLE;
      buddy->state = ALLOCATABLE;

      // when buddy is the left frame and target is the right one
      if (idx > buddy_idx) {
        idx = buddy_idx;
        buddy->val = order + 1;
        target->val = FREE_FOR_ALLOC;
        target = buddy;
      } else {
        target->val = order + 1;
        buddy->val = FREE_FOR_ALLOC;
      }
    } else {
      printf("[INFO] free page frame: index=%d, order=%d\n", idx, order);
      target->val = order;
      target->state = ALLOCATABLE;
      target->prev = NULL;
      target->next = NULL;

      if (frame_list[MAX_ORDER - order] == NULL) {
        frame_list[MAX_ORDER - order] = target;
      } else if (target->index < frame_list[MAX_ORDER - order]->index) {
        target->next = frame_list[MAX_ORDER - order];
        frame_list[MAX_ORDER - order] = target;
      } else {
        frame_list[MAX_ORDER - order]->next = target;
      }
      break;
    }
  }
}

__attribute__ ((always_inline))
inline void *id_to_mm_addr(int idx) {
  return (void *)(MEM_START + idx * FRAME_SIZE);
}

__attribute__ ((always_inline))
inline int mm_addr_to_id(void *mm_addr) {
  return ((uint64_t)mm_addr - MEM_START) / FRAME_SIZE;
}

void *kmalloc(uint64_t size) {
  int reqorder = 0;
  for (int c = 1; c * FRAME_SIZE < size; c <<= 1, reqorder++);

  int idx = allocate_frame(reqorder);
  void *mm_addr = id_to_mm_addr(idx);

  return mm_addr;
}

void kfree(void *addr) {
  int idx = mm_addr_to_id(addr);
  deallocate_frame(idx);
  addr = NULL;
  return;
}

void demo_frame() {
  print("============== demo frame allocator ==============\n");
  
  void *A = kmalloc(2 * FRAME_SIZE);
  void *B = kmalloc(1 * FRAME_SIZE);
  void *C = kmalloc(2 * FRAME_SIZE);
  kfree(A);
  void *D = kmalloc(1 * FRAME_SIZE);
  kfree(B);
  kfree(D);
  kfree(C);
  print("==================================================\n");
}