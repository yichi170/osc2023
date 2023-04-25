#include "page_alloc.h"

#include "math.h"
#include "print.h"
#include "malloc.h"

static struct frame** frame_list;
static struct frame*  frame_array;

int get_upper_free_order(int reqorder) {
  for (int i = reqorder + 1; i <= MAX_ORDER; i++)
    if (frame_list[MAX_ORDER - i] != NULL)
      return i;
  return -1;
}

void add_frame_to_list(int order, int frame_idx) {
  if (frame_list[MAX_ORDER - order]) {
    frame_array[frame_idx].next = frame_list[MAX_ORDER - order];
  }
  frame_list[MAX_ORDER - order] = &frame_array[frame_idx];
}

void init_allocator() {
  frame_array = simple_malloc(NUM_FRAME * sizeof(struct frame));
  printf("NUM_FRAME: %d\n", NUM_FRAME);

  for (int i = 0; i < NUM_FRAME; i++) {
    frame_array[i].val = FREE_FOR_ALLOC;
    frame_array[i].index = i;
    frame_array[i].state = ALLOCATABLE;
    if (i % (1 << MAX_ORDER) == 0) {
      frame_array[i].val = MAX_ORDER;
      frame_array[i].prev = &frame_array[i - (1 << MAX_ORDER)];
      frame_array[i].next = &frame_array[i + (1 << MAX_ORDER)];
    } else {
      frame_array[i].prev = NULL;
      frame_array[i].next = NULL;
    }
  }

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

  if (frame_list[MAX_ORDER - reqorder] == NULL) {
    /* search the upper free frame */
    int upper_free_order = get_upper_free_order(reqorder);

    if (upper_free_order == -1) {
      printf("[ERR] It seems that no free frame can be allocated\n");
      return -1;
    }

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

void allocate_frame_by_id_range(int start_id, int end_id) {
  printf("allocate frame from id: %d - %d\n", start_id, end_id);

  while (start_id < end_id) {
    int i;
    for (i = 0; i <= MAX_ORDER; i++)
      if (start_id + (1 << i) >= end_id || ((start_id >> i) & 0x1))
        break;

    if (start_id + (1 << i) > end_id)  --i;

    allocate_frame_id_order(start_id, i);

    start_id = start_id + (1 << i);
  }
}

int allocate_frame_id_order(int id, int reqorder) {
  if (id % (1 << reqorder) != 0) {
    printf("[WARN] frame #%d cannot be allocated in order %d.\n",
      id, reqorder);
    return -1;
  }

  if (frame_array[id].state != ALLOCATABLE) {
    printf("[WARN] frame #%d isn't allocatable\n", id);
    return -1;
  }

  int block_idx = id;
  int upper_order = reqorder;

  while (frame_array[block_idx].val == FREE_FOR_ALLOC) {
    int buddy_idx = block_idx ^ (1 << upper_order);
    block_idx = (block_idx < buddy_idx) ? block_idx : buddy_idx;
    upper_order++;
  }

  struct frame *upper_frame = &frame_array[block_idx];

  while (upper_frame->val > reqorder) {
    struct frame *l_frame = upper_frame;
    int upper_free_order = l_frame->val;
    if (upper_frame == frame_list[MAX_ORDER - upper_free_order])
      frame_list[MAX_ORDER - upper_free_order] = l_frame->next;
    else
      upper_frame->prev->next = l_frame->next;
    if (l_frame->next != NULL)
      l_frame->next->prev = NULL;

    int r_off = pow(2, l_frame->val - 1);

    struct frame *r_frame = &frame_array[l_frame->index + r_off];

    l_frame->val -= 1;
    r_frame->val = l_frame->val;
    l_frame->state = ALLOCATABLE;
    r_frame->state = ALLOCATABLE;

    upper_free_order--;

    if (r_frame->index <= id) {
      add_frame_to_list(upper_free_order, l_frame->index);
      add_frame_to_list(upper_free_order, r_frame->index);
      upper_frame = r_frame;
    } else  {
      add_frame_to_list(upper_free_order, r_frame->index);
      add_frame_to_list(upper_free_order, l_frame->index);
      upper_frame = l_frame;
    }
  }

  int index = frame_list[MAX_ORDER - reqorder]->index;
  for (int i = 0; i < (1 << frame_list[MAX_ORDER - reqorder]->val); i++) {
    frame_array[index + i].state = ALLOCATED;
  }

  struct frame *ret = frame_list[MAX_ORDER - reqorder];
  frame_list[MAX_ORDER - reqorder] = frame_list[MAX_ORDER - reqorder]->next;
  if (frame_list[MAX_ORDER - reqorder] != NULL)
    frame_list[MAX_ORDER - reqorder]->prev = NULL;

  ret->prev = NULL;
  ret->next = NULL;

  printf("[INFO] allocate frame#%d in order#%d\n", ret->index, reqorder);

  return ret->index;
}

void deallocate_frame(int idx) {
  struct frame *target = &frame_array[idx];

  if (target->state != ALLOCATED) {
    printf("[WARN] The frame isn't allocated, so no need to free.\n");
    return;
  }

  int target_order = target->val;

  for (int order = target_order; order <= MAX_ORDER; order++) {
    int buddy_idx = idx ^ (1 << order);

    if (buddy_idx < NUM_FRAME && frame_array[buddy_idx].state == ALLOCATABLE &&
        order < MAX_ORDER && order == target->val) {
      struct frame *buddy = &frame_array[buddy_idx];
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

void *frame_malloc(uint64_t size) {
  int reqorder = 0;
  for (int c = 1; c * FRAME_SIZE < size; c <<= 1, reqorder++);

  printf("[INFO] request a frame in order#%d\n", reqorder);

  int idx = allocate_frame(reqorder);
  void *mm_addr = id_to_mm_addr(idx);

  return mm_addr;
}

void free_frame(void *addr) {
  int idx = mm_addr_to_id(addr);
  deallocate_frame(idx);
  addr = NULL;
  return;
}

void demo_frame() {
  print("============== demo frame allocator ==============\n");
  void *A = frame_malloc(2 * FRAME_SIZE);
  void *B = frame_malloc(1 * FRAME_SIZE);
  void *C = frame_malloc(2 * FRAME_SIZE);
  free_frame(A);
  void *D = frame_malloc(1 * FRAME_SIZE);
  free_frame(B);
  free_frame(D);
  free_frame(C);
  print("==================================================\n");
}