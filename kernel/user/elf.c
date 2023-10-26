#include "elf.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "mm/paging/paging.h"

uint64_t elfLoad(void* file) {
    if (file == NULL) {
        return -1UL;
    }

    elf_header_t* header = (elf_header_t*)(file);

    if (strncmp(header->magic,
                "\x7F"
                "ELF",
                4)) {
        return -1UL;
    }

    elf_program_t* segments =
        (elf_program_t*)((uint64_t)header + header->program_header_table_pos);

    uint64_t page_flags;
    char* data;
    char* vaddr;
    uint64_t start_page;
    uint64_t num_bytes;
    uint64_t num_pages;
    uint64_t last_allocated_page = 0;

    for (uint64_t i = 0; i < header->program_header_num_entries; i++) {
        page_flags = PAGE_FLAG_PRESENT | PAGE_FLAG_USERSUPER;
        if (segments[i].type == ELF_SEGMENT_LOAD) {
            if (segments[i].flags & ELF_FLAG_WRITEABLE) {
                page_flags |= PAGE_FLAG_READWRITE;
            }
            if (!(segments[i].flags & ELF_FLAG_EXECUTABLE)) {
                page_flags |= PAGE_FLAG_EXECDBLE;
            }
            data       = (char*)((uint64_t)header + segments[i].offset);
            vaddr      = (char*)(segments[i].vaddr);
            start_page = ALIGN_TO_PAGE((uint64_t)vaddr);
            num_bytes  = segments[i].size_mem;
            num_pages  = GET_NUM_PAGES(num_bytes);
            start_page = (uint64_t)vmalloc(
                (void*)start_page, num_pages,
                PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_USERSUPER);
            memset((void*)vaddr, 0, num_bytes);
            memcpy((void*)vaddr, (void*)data, num_bytes);
            vflags((void*)start_page, num_pages, page_flags);
        }
        else {
            return -1UL;
        }
    }

    return header->program_entry_pos;
}