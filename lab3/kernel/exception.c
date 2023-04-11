#include "exception.h"
#include "timer.h"
#include "print.h"

#define BUFFER_SIZE 1024
#define CORE0_IRQ_SRC ((volatile unsigned int*)(0x40000060))

extern void core_timer_irq_handler();

char recv_buf[BUFFER_SIZE];
char send_buf[BUFFER_SIZE];

int recv_begin, recv_end; // current available region
int send_begin, send_end;

#define put_char_to_sendbuf(x) if (send_end >= BUFFER_SIZE) \
  send_end -= BUFFER_SIZE; \
  send_buf[send_end++] = x;

#define put_char_to_recvbuf(x) if (recv_end >= BUFFER_SIZE) \
  recv_end -= BUFFER_SIZE; \
  recv_buf[recv_end++] = x;

void async_print(const char *str) {
  for (int i = 0; *(str + i) != '\0'; i++) {
    if (str[i] == '\r' || str[i] == '\n') {
      put_char_to_sendbuf('\r');
      put_char_to_sendbuf('\n');
      continue;
    }
    put_char_to_sendbuf(str[i]);
  }
  if (send_begin != send_end)
    *AUX_MU_IER |= 2;
}

unsigned int async_read(char *str, unsigned int size) {
  *AUX_MU_IER |= 1;
  for (int i = 0; i < size; i++) {
    while (recv_begin == recv_end)  asm volatile("nop"); // nothing can be received
    str[i] = recv_buf[recv_begin++];
    uart_send(str[i]);
  }
  volatile unsigned int ier_val = *AUX_MU_IER;
  ier_val &= (~1);
  *AUX_MU_IER = ier_val;
  return size;
}

void send_from_buf() {
  if (send_begin == send_end) { // no need to send
    volatile unsigned int ier_val = *AUX_MU_IER;
    ier_val &= (~2);
    *AUX_MU_IER = ier_val;
    return;
  }

  uart_send(send_buf[send_begin++]);
  if (send_begin >= BUFFER_SIZE)
    send_begin -= BUFFER_SIZE;
}

void recv_to_buf() {
  char ch = uart_recv();
  put_char_to_recvbuf(ch);
}

void gpu_interrupt_handler() {
  if (*GPU_PENDING1 & (1 << 29)) { // check whether the interrupt is caused by uart
    if (*AUX_MU_IIR & (1 << 2)) { // able to receive
      recv_to_buf();
    } else if (*AUX_MU_IIR & (1 << 1)) { // able to transmit
      send_from_buf();
    }
  }
}

void c_el1_irq_handler() { // el1
  volatile uint64_t irq_src = *CORE0_IRQ_SRC;
  if (irq_src & (1 << 8)) { // GPU interrupt
    // print("GPU interrupt\n");
    gpu_interrupt_handler();
  } else if (irq_src & (1 << 1)) { // CNTPNSIRQ interrupt
    // print("timer interrupt\n");
    el1_timer_irq_handler();
  } else {
    print("Unknown interrupt\n");
  }
  return;
}

void c_el0_irq_handler() { // el0
  volatile uint64_t irq_src = *CORE0_IRQ_SRC;
  if (irq_src & (1 << 8)) { // GPU interrupt
    print("GPU interrupt\n");
  } else if (irq_src & (1 << 1)) { // CNTPNSIRQ interrupt
    print("el0 timer irq\n");
    core_timer_irq_handler();
  } else {
    print("Unknown interrupt\n");
  }
  return;
}
