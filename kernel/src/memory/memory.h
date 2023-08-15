#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <video/video.h>

#define PAGE_SIZE 4096
#define PAGE_FLAGS 0xFFF
#define PAGE_PRESENT 0x1
#define PAGE_RW 0x2
#define PAGE_USER 0x4
#define PAGE_2MB 0x80
#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3FF)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3FF)
#define GET_PAGE_ADDRESS(x) ((x) & ~0xFFF)
#define USER_STACK_SIZE 8192
#define KERNEL_STACK_SIZE 8192

extern void *user_stack;
extern void *kernel_stack;

typedef struct page
{
    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t unused : 7;
    uint32_t frame : 20;
} page_t;

typedef struct page_table
{
    page_t pages[1024];
} page_table_t;

typedef struct page_directory
{
    page_table_t *tables[1024];
    uint32_t tablesPhysical[1024];
    uint32_t physicalAddr;
} page_directory_t;

typedef struct Page
{
    struct Page *next;
} Page;

typedef struct
{
    size_t num_pages;
} Header;

extern page_directory_t *current_directory;
extern page_directory_t *kernel_directory;

void init_memory(struct limine_memmap_entry **entries, uint64_t entry_count);
void detect_total_memory(struct limine_memmap_entry **entries, uint64_t entry_count);
void *malloc(size_t size);
void free(void *ptr);
Page *alloc_page();
void free_page(Page *page);
void map_page(void *physaddr, void *virtualaddr, uint32_t flags);
void unmap_page(void *virtualaddr);
void switch_page_directory(page_directory_t *new_directory);
page_directory_t *clone_directory(page_directory_t *src);
void init_vmm(struct limine_memmap_entry **entries, uint64_t entry_count);

#endif