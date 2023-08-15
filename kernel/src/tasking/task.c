#include "task.h"

struct Task *current_task = NULL;
struct Task *tasks = NULL;

void create_task(void *entry_point)
{
    struct Task *task = (struct Task *)malloc(sizeof(struct Task));
    uint64_t *stack = (uint64_t *)malloc(STACK_SIZE);

    task->page_directory = clone_directory(current_directory);

    task->stack_pointer = stack + STACK_SIZE;
    task->next = tasks;
    task->task_started = 0;
    tasks = task;
    task->stack_pointer--;
    *(task->stack_pointer) = (uint64_t)entry_point;

    if (current_task == NULL)
    {
        current_task = task;
    }

    log(LOG_INFO, "Created a new task:\n");
    log(LOG_INFO, "   Entry Point: 0x%x\n", (uint64_t)entry_point);
    log(LOG_INFO, "   Page Directory: 0x%x\n", (uint64_t)task->page_directory);
    log(LOG_INFO, "   Stack Pointer: 0x%x\n", (uint64_t)task->stack_pointer);
}

void switch_to(struct Task *task)
{
    asm volatile(
        "mov %%rsp, %[old_sp];"
        "mov %[new_sp], %%rsp;"
        :
        : [old_sp] "m"(current_task->stack_pointer), [new_sp] "r"(task->stack_pointer)
        :);
}

void yield()
{
    asm volatile("cli");

    if (current_task->task_started)
    {
        asm volatile(
            "push %%rax;"
            "push %%rbx;"
            "push %%rcx;"
            "push %%rdx;"
            "push %%rsi;"
            "push %%rdi;"
            "push %%r8;"
            "push %%r9;"
            "push %%r10;"
            "push %%r11;"
            "push %%r12;"
            "push %%r13;"
            "push %%r14;"
            "push %%r15;"
            "mov %%rsp, %[sp];"
            :
            : [sp] "m"(current_task->stack_pointer)
            :);
    }

    current_task = (current_task->next) ? current_task->next : tasks;
    switch_page_directory(current_task->page_directory);

    if (current_task->task_started)
    {
        asm volatile(
            "mov %[sp], %%rsp;"
            "pop %%r15;"
            "pop %%r14;"
            "pop %%r13;"
            "pop %%r12;"
            "pop %%r11;"
            "pop %%r10;"
            "pop %%r9;"
            "pop %%r8;"
            "pop %%rdi;"
            "pop %%rsi;"
            "pop %%rdx;"
            "pop %%rcx;"
            "pop %%rbx;"
            "pop %%rax;"
            :
            : [sp] "m"(current_task->stack_pointer)
            :);
    }
    else
    {
        asm("mov %0, %%rsp;" ::"r"(current_task->stack_pointer));
        current_task->task_started = 1;
        asm("pop %%rax;"
            "jmp *%%rax;" ::
                : "rax");
    }

    asm volatile("sti");
}
