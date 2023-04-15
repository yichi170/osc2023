#ifndef _SHELL_H
#define _SHELL_H

#include "type.h"

void print_hello();
void print_help();
void print_unsupport(char *);
void print_exception(uint64_t, uint64_t, uint64_t, uint64_t);
void shell();

#endif
