#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

#define SYSCALL_WRITE 0
#define SYSCALL_READ 1

#define IA32_STAR 0xC0000081
#define IA32_LSTAR 0xC0000082
#define IA32_FMASK 0xC0000084

void test_syscall();

#endif