#include <arch/idt.h>
#include <video/video.h>
#include "syscall.h"

static inline uint64_t read_msr(uint32_t msr_id)
{
        uint32_t low, high;
        asm volatile("rdmsr"
                     : "=a"(low), "=d"(high)
                     : "c"(msr_id));
        return ((uint64_t)high << 32) | low;
}

void syscall_handler(struct interrupt_frame *frame)
{
        switch (frame->rax)
        {
        case 1:
                frame->rax = frame->rax;
                break;
        default:
                printf("Unknown syscall: %d\n", frame->rax);
                break;
        }
}

void test_syscall()
{
        uint64_t lstar = read_msr(IA32_LSTAR);
        uint64_t efer = read_msr(0xC0000080);
        uint64_t star = read_msr(IA32_STAR);

        log(LOG_INFO, "Testing syscall MSRs...\n");
        log(LOG_INFO, "EFER MSR: 0x%llx\n", efer);
        log(LOG_INFO, "STAR MSR: 0x%llx\n", star);
        log(LOG_INFO, "LSTAR MSR: 0x%llx\n", lstar);

        if ((efer & 0x1) == 0)
        {
                log(LOG_ERROR, "EFER MSR value is incorrect!\n");
        }
        else
        {
                log(LOG_INFO, "EFER MSR value is correct!\n");
        }

        if (star != ((star & 0xFFFFFFFF) | ((uint64_t)0x00180008 << 32)))
        {
                log(LOG_ERROR, "STAR MSR value is incorrect!\n");
        }
        else
        {
                log(LOG_INFO, "STAR MSR value is correct!\n");
        }

        if (lstar != (uint64_t)syscall_handler)
        {
                log(LOG_ERROR, "LSTAR MSR value is incorrect!\n");
        }
        else
        {
                log(LOG_INFO, "LSTAR MSR value is correct!\n");
        }
}
