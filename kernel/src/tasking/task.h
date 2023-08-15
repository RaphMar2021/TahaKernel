#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <memory/memory.h>
#include <video/video.h>

#define STACK_SIZE 4096

struct Task
{
    uint64_t *stack_pointer;
    struct Task *next;
    int task_started;
    page_directory_t *page_directory;
};

extern struct Task *current_task;
extern struct Task *tasks;

void create_task(void *entry_point);
void yield();

#endif