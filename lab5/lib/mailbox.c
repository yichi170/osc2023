#include "mailbox.h"
#include "print.h"

int mailbox_call(unsigned char channel, volatile unsigned int *mbox) {
  unsigned int msg = (((unsigned long)mbox & ~0xF) | (channel & 0xF));

  while ((*MAILBOX_STATUS) & MAILBOX_FULL);

  *MAILBOX_WRITE = msg;

  while (1) {
    while ((*MAILBOX_STATUS) & MAILBOX_EMPTY);

    if (msg == *MAILBOX_READ)
      return mbox[1] == REQUEST_SUCCEED;
  }
  // should not return from here.
  return 0;
}

void get_board_revision() {
  volatile unsigned int __attribute__((aligned(16))) mailbox[7];
  mailbox[0] = 7 * 4;
  mailbox[1] = REQUEST_CODE;
  mailbox[2] = GET_BOARD_REVISION;
  mailbox[3] = 4;
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0;
  mailbox[6] = END_TAG;

  if (mailbox_call(8, mailbox)) {
    printf("Board Revision: %#X\n", mailbox[5]);
  }
  return; // return unsigned int;
}

void get_arm_memory() {
  volatile unsigned int __attribute__((aligned(16))) mailbox[8];
  mailbox[0] = 8 * 4;
  mailbox[1] = REQUEST_CODE;
  mailbox[2] = GET_ARM_MEMORY;
  mailbox[3] = 8;
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0;
  mailbox[6] = 0;
  mailbox[7] = END_TAG;

  if (mailbox_call(8, mailbox)) {
    printf("Memory Base Address: %#X\n", mailbox[5]);
    printf("Memory Size: %#X\n", mailbox[6]);
  }
}
