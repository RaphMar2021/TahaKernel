#include <stdint.h>
#include <stddef.h>
#include <video/video.h>
#include <memory/memory.h>
#include <tasking/task.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include <syscall/syscall.h>
#include <device/pci/pci.h>
#include <disk/fat.h>
#include <disk/iso9660.h>
#include <limine/limine.h>

struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
    .response = NULL};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0};

void task1()
{
    while (1)
    {
        draw_rectangle(685, 0, 350, 350, 0xff0000);
        yield();
    }
}

void task2()
{
    while (1)
    {
        draw_rectangle(685, 395, 350, 350, 0x00ff00);
        yield();
    }
}

void memory_test()
{
    log(LOG_INFO, "Starting memory test...\n");

    log(LOG_INFO, "Allocating memory...\n");
    char *test_block1 = (char *)malloc(1000);
    char *test_block2 = (char *)malloc(2000);

    if (test_block1)
    {
        log(LOG_INFO, "Writing to allocated memory block 1...\n");
        strcpy(test_block1, "Hello, World!");
        log(LOG_INFO, "Reading from allocated memory block 1\n");
    }
    else
    {
        log(LOG_ERROR, "Failed to allocate memory block 1.\n");
    }

    if (test_block2)
    {
        log(LOG_INFO, "Writing to allocated memory block 2...\n");
        strcpy(test_block2, "Testing 123...");
        log(LOG_INFO, "Reading from allocated memory block 2\n");
    }
    else
    {
        log(LOG_ERROR, "Failed to allocate memory block 2.\n");
    }

    log(LOG_INFO, "Freeing allocated memory...\n");
    if (test_block1)
    {
        free(test_block1);
    }
    if (test_block2)
    {
        free(test_block2);
    }

    log(LOG_INFO, "Memory test completed.\n");
}

void callback1()
{
    printf("Callback 1!\n");
}

void userfunc()
{
    printf("Callback 2!\n");

    while(1){}
}

void JumpToUserMode(void *userArgs, void *entryPoint, void *userStack) {
    asm volatile (
        "movq $0x1b, %%rax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "pushq %%rax\n"
        "pushq %0\n"
        "pushq $0x202\n"
        "pushq $0x23\n"
        "pushq %1\n"
        "iretq\n"
        :
        : "r"(userStack), "r"(entryPoint)
        : "rax"
    );
}


void _start(void)
{
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    struct limine_memmap_entry *memmap_entry = memmap_request.response->entries;

    init_video(framebuffer);

    clear_screen(0x000000);

    print_animated("Welcome to Taha");
    set_color(0x00FFFF);
    printf("!\n\n");
    set_color(0xFFFFFF);

    init_gdt((uint64_t)&kernel_stack + sizeof(kernel_stack));
    init_idt();
    init_memory(memmap_entry, memmap_request.response->entry_count);
    init_vmm(memmap_entry, memmap_request.response->entry_count);
    memory_test();
    pci_enum();
    init_syscalls();
    printf("\n");
    set_color(0x00ff00);
    detect_disks();
    set_color(0xffffff);

    bs = malloc(sizeof(struct boot_sector));
    if (bs == NULL)
    {
        log(LOG_ERROR, "Failed to allocate memory for boot sector.\n");
        while (1)
            ;
    }
    printf("\n");

    read_sector(0, bs);
    current_directory_cluster = bs->root_cluster;

    print_prompt();
    asm("cli");
    JumpToUserMode(NULL, (void*)userfunc, user_stack);

    while (1)
        ;
}
