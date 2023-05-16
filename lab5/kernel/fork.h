#ifndef _FORK_H
#define _FORK_H

#include "process.h"

#define NULL (void *)0

int sys_fork();
void test_fork();
void test_fork_process_entry();

#endif
