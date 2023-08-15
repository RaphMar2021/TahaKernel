#include "memory.h"

Page *free_pages = NULL;
page_directory_t *current_directory;
page_directory_t *kernel_directory;

void *user_stack __attribute__((aligned(16)));
void *kernel_stack __attribute__((aligned(16)));

uint64_t kernel_physical_base;
uint64_t kernel_virtual_base;
struct limine_file *kernel_file;
uint64_t kernel_size;

struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
    .response = NULL};

struct limine_kernel_file_request kernel_file_request = {
    .id = LIMINE_KERNEL_FILE_REQUEST,
    .revision = 0,
    .response = NULL};

void init_memory(struct limine_memmap_entry **entries, uint64_t entry_count)
{
    log(LOG_DEBUG, "Initializing memory...\n");

    uint64_t total_memory = 0;

    for (uint64_t i = 0; i < entry_count; i++)
    {
        struct limine_memmap_entry *entry = entries[i];
        if (entry->type != LIMINE_MEMMAP_USABLE)
        {
            continue;
        }

        total_memory += entry->length;

        uint64_t num_pages = entry->length / PAGE_SIZE;

        for (uint64_t j = 0; j < num_pages; j++)
        {
            uint64_t page_address = entry->base + (j * PAGE_SIZE);
            Page *page = (Page *)page_address;
            page->next = free_pages;
            free_pages = page;
        }
    }

    kernel_stack = malloc(KERNEL_STACK_SIZE);
    if (!kernel_stack)
    {
        log(LOG_ERROR, "Unable to alloc memory to kernel stack!\n");
    }

    kernel_stack = (void *)((uint64_t)kernel_stack + KERNEL_STACK_SIZE);

    user_stack = malloc(KERNEL_STACK_SIZE);
    if (!user_stack)
    {
        log(LOG_ERROR, "Unable to alloc memory to kernel stack!\n");
    }

    user_stack = (void *)((uint64_t)user_stack + KERNEL_STACK_SIZE);
}

void detect_total_memory(struct limine_memmap_entry **entries, uint64_t entry_count)
{
    uint64_t total_memory = 0;

    for (uint64_t i = 0; i < entry_count; i++)
    {
        struct limine_memmap_entry *entry = entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE)
        {
            total_memory += entry->length;
        }
    }

    uint64_t total_memory_in_mb = total_memory / (1024 * 1024);

    log(LOG_INFO, "Total usable memory: %lu MB\n", total_memory_in_mb);
}

void *malloc(size_t size)
{
    size_t num_pages = (size / PAGE_SIZE) + 1;
    Header *header = (Header *)alloc_page();
    if (!header)
    {
        log(LOG_ERROR, "Failed to allocate header page for malloc.\n");
        return NULL;
    }
    header->num_pages = num_pages;

    for (size_t i = 1; i < num_pages; i++)
    {
        if (!alloc_page())
        {
            log(LOG_ERROR, "Failed to allocate additional page %llu for malloc.\n", i);
            for (size_t j = i - 1; j > 0; j--)
            {
                free_page((Page *)((uintptr_t)(header + 1) + j * PAGE_SIZE));
            }
            free_page((Page *)header);
            return NULL;
        }
    }

    return header + 1;
}

void free(void *ptr)
{
    Header *header = (Header *)ptr - 1;
    for (size_t i = 0; i < header->num_pages; i++)
    {
        free_page((Page *)((uintptr_t)(header + 1) + i * PAGE_SIZE));
    }
}

Page *alloc_page()
{
    if (!free_pages)
    {
        log(LOG_ERROR, "No available frames.\n");
        while (1)
        {
            asm("hlt");
        }
    }

    Page *page = free_pages;
    free_pages = free_pages->next;
    page->next = NULL;
    return page;
}

void free_page(Page *page)
{
    page->next = free_pages;
    free_pages = page;
}

void map_page(void *physaddr, void *virtualaddr, uint32_t flags)
{
    log(LOG_DEBUG, "Mapping physical address 0x%x to virtual address 0x%x with flags 0x%x\n", (uint64_t)physaddr, (uint64_t)virtualaddr, flags);

    uint32_t dir_idx = PAGE_DIRECTORY_INDEX((uint32_t)virtualaddr);
    uint32_t tbl_idx = PAGE_TABLE_INDEX((uint32_t)virtualaddr);
    log(LOG_DEBUG, "Directory index: %d, Table index: %d\n", dir_idx, tbl_idx);

    if (current_directory->tables[dir_idx] == NULL)
    {
        current_directory->tables[dir_idx] = (page_table_t *)malloc(sizeof(page_table_t));
        if (!current_directory->tables[dir_idx])
        {
            log(LOG_ERROR, "Failed to allocate space for the page table!\n");
            return;
        }
        memset(current_directory->tables[dir_idx], 0, PAGE_SIZE);
    }

    current_directory->tables[dir_idx]->pages[tbl_idx].frame = (uint32_t)physaddr >> 12;
    current_directory->tables[dir_idx]->pages[tbl_idx].present = 1;
    current_directory->tables[dir_idx]->pages[tbl_idx].rw = (flags & 0x2) ? 1 : 0;
    current_directory->tables[dir_idx]->pages[tbl_idx].user = (flags & 0x4) ? 1 : 0;

    log(LOG_DEBUG, "Mapped physical address 0x%x to virtual address 0x%x successfully\n", (uint64_t)physaddr, (uint64_t)virtualaddr);
}

void unmap_page(void *virtualaddr)
{
    log(LOG_DEBUG, "Unmapping virtual address 0x%x\n", (uint64_t)virtualaddr);
    uint32_t dir_idx = PAGE_DIRECTORY_INDEX((uint32_t)virtualaddr);
    uint32_t tbl_idx = PAGE_TABLE_INDEX((uint32_t)virtualaddr);

    if (current_directory->tables[dir_idx] == NULL)
    {
        log(LOG_WARN, "Page table not present for virtual address 0x%x\n", (uint64_t)virtualaddr);
        return;
    }

    current_directory->tables[dir_idx]->pages[tbl_idx].present = 0;
    log(LOG_DEBUG, "Unmapped virtual address 0x%x successfully\n", (uint64_t)virtualaddr);
}

void switch_page_directory(page_directory_t *new_directory)
{
    // log(LOG_DEBUG, "Switching to page directory at 0x%x\n", (uint64_t)new_directory);
    current_directory = new_directory;
    asm volatile("mov %%cr3, %0"
                 : "=r"(new_directory));
    // log(LOG_DEBUG, "Switched to page directory successfully\n");
}

page_directory_t *clone_directory(page_directory_t *src)
{
    log(LOG_DEBUG, "Cloning page directory at 0x%x\n", (uint64_t)src);
    page_directory_t *dir = (page_directory_t *)malloc(sizeof(page_directory_t));
    if (!dir)
    {
        log(LOG_ERROR, "Failed to allocate space for the cloned directory!\n");
        return NULL;
    }
    memset(dir, 0, sizeof(page_directory_t));

    for (int i = 0; i < 1024; i++)
    {
        if (src->tables[i])
        {
            dir->tables[i] = (page_table_t *)malloc(sizeof(page_table_t));
            if (!dir->tables[i])
            {
                log(LOG_ERROR, "Failed to allocate space for the cloned page table!\n");
                return NULL;
            }
            memcpy(dir->tables[i], src->tables[i], sizeof(page_table_t));
            log(LOG_DEBUG, "Cloned page table at index %d\n", i);
        }
    }
    log(LOG_DEBUG, "Cloned page directory successfully to 0x%x\n", (uint64_t)dir);
    return dir;
}

void init_vmm(struct limine_memmap_entry **entries, uint64_t entry_count)
{
    kernel_file = kernel_file_request.response->kernel_file;
    kernel_size = kernel_file->size;

    kernel_physical_base = kernel_address_request.response->physical_base;
    kernel_virtual_base = kernel_address_request.response->virtual_base;

    uint64_t kernel_start = kernel_virtual_base;
    uint64_t kernel_end = kernel_virtual_base + kernel_size;

    kernel_directory = (page_directory_t *)malloc(sizeof(page_directory_t));
    if (!kernel_directory)
    {
        log(LOG_ERROR, "Failed to allocate space for the kernel page directory!\n");
        return;
    }
    memset(kernel_directory, 0, sizeof(page_directory_t));

    for (uint64_t i = kernel_start; i < kernel_end; i += PAGE_SIZE)
    {
        map_page((void *)(i - kernel_start + kernel_physical_base), (void *)i, PAGE_PRESENT | PAGE_RW);
    }

    switch_page_directory(kernel_directory);

    log(LOG_INFO, "VMM initialized and kernel pages mapped using PML4 at 0x%llx\n", kernel_directory);
}
