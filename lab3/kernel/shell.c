#include "shell.h"
#include "string.h"
#include "print.h"
#include "read.h"
#include "mini_uart.h"
#include "mailbox.h"
#include "reboot.h"
#include "cpio.h"
#include "malloc.h"
#include "timer.h"
#include "exception.h"

#define BUF_SIZE 32

extern void core_timer_disable();

void print_hello() {
  print("Hello World!\n");
}

void print_help() {
  print("help\t: print this help menu\n");
  print("hello\t: print Hello World!\n");
  print("reboot\t: reboot the device\n");
  print("info\t: print hardware info\n");
  print("ls\t: list directory contents\n");
  print("cat [file ...]\t: concatenate and print files\n");
  print("exec file\t: execute the executable file\n");
  print("test_async\t: test async print\n");
  print("timer [test/end]: test/shutdown timer\n");
  print("timer <msg> <sec>: set timer\n");
}

void print_unsupport(char *buf) {
  printf("Command not found: %s\n", buf);
}

void print_exception(uint64_t spsr, uint64_t elr, uint64_t esr, uint64_t invalid) {
  if (invalid == 1) {
    print("Unexpected exception\n");
    return;
  }

  print("===== exception info =====\n");
  printf("spsr_el1:\t%#X\n", spsr);
  printf("elr_el1:\t%#X\n", elr);
  printf("esr_el1:\t%#X\n", esr);
}

void print_timer_irq(uint64_t frq_timer, uint64_t cnt_timer) {
  print("===== timer IRQ info =====\n");
  printf("current timestamp:\t%#X\n", cnt_timer / frq_timer);
}

void print_msg(void *data) {
  print((char *)data);
  print("\n");
}

void shell() {
  char buf[BUF_SIZE];
  bool interact = true;

  print("Welcome to rpi-os shell!\n");
  print("Enter \"help\" for a list of built-in commands.\n");

  while (1) {
    print("# ");
    if (readline(buf, BUF_SIZE, interact) <= 0 || *buf == '\0')
      continue;
    if (streq(buf, "help") == 0) {
      print_help();
    }
    else if (streq(buf, "hello") == 0) {
      print_hello();
    }
    else if (streq(buf, "reboot") == 0) {
      reset(100);
    }
    else if (streq(buf, "info") == 0) {
      get_board_revision();
      get_arm_memory();
    }
    else if (streq(buf, "ls") == 0) {
      cpio_ls();
    }
    else if (strstartwith(buf, "cat ") == 0) {
      char *files = buf + 4;
      char *start = files;
      logdf("%s\n", files);
      while (*files) {
        if (*files == ' ') {
          cpio_cat(start, files - start);
          start = ++files;
        } else {
          ++files;
        }
      }
      cpio_cat(start, files - start);
    }
    else if (strstartwith(buf, "exec ") == 0) {
      char *file = buf + 5;
      execute_usrprogram(file);
    }
    else if (streq(buf, "test_async") == 0) {
      print("test async: expected output = Hello World\n");
      async_print("Hello World\n");
      print("input five characters to test the async read: ");
      char ar[6];
      async_read(ar, 5);
      ar[5] = '\0';
      printf("\nreceived: %s\n", ar);
    }
    else if (strstartwith(buf, "timer") == 0) {
      char *msg = simple_malloc(BUF_SIZE * sizeof(char)); // need to align with 32-bit
      char *msg_ptr = msg;
      char *ptr = buf + strlen("timer") + 1;

      while (*ptr) {
        *msg_ptr++ = *ptr++;
        if (*ptr == ' ') {
          break;
        }
      }
      *msg_ptr = '\0';
      if (streq(msg, "test") == 0) {
        print("start to test timer\n");
        add_timer(print_msg, "msg1", 3);
        add_timer(print_msg, "msg2", 2);
        add_timer(print_msg, "msg3", 1);
      } else if (streq(msg, "end") == 0) {
        core_timer_disable();
        continue;
      } else {
        int sec = strtoi(++ptr, 10);
        printf("print %s after %d seconds\n", msg, sec);
        add_timer(print_msg, msg, sec);
      }
    }
    else {
      print_unsupport(buf);
    }
  }
}
