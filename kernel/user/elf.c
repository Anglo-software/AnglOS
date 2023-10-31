#include "elf.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "mm/paging/paging.h"

uint64_t elfLoad(address_space_t* space, void* file) {
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

    uint64_t flags;
    char* data;
    char* vaddr;
    uint64_t num_bytes;
    uint32_t type;

    for (uint64_t i = 0; i < header->program_header_num_entries; i++) {
        flags = ADDRSPACE_FLAG_USER;
        if (segments[i].type == ELF_SEGMENT_LOAD) {
            if (segments[i].flags & ELF_FLAG_WRITEABLE) {
                flags |= ADDRSPACE_FLAG_WRITEABLE;
            }
            if (segments[i].flags & ELF_FLAG_EXECUTABLE) {
                flags |= ADDRSPACE_FLAG_EXECUTABLE;
            }
            data      = (char*)((uint64_t)header + segments[i].offset);
            vaddr     = (char*)(segments[i].vaddr);
            num_bytes = segments[i].size_mem;
            addressSpaceAddEntry(space, type,
                                 ADDRSPACE_FLAG_USER |
                                     ADDRSPACE_FLAG_WRITEABLE |
                                     ADDRSPACE_FLAG_EXECUTABLE,
                                 num_bytes, vaddr);
            memset((void*)vaddr, 0, num_bytes);
            memcpy((void*)vaddr, (void*)data, num_bytes);
            addressSpaceChangeFlags(space, i, flags);
        }
        else {
            return -1UL;
        }
    }

    return header->program_entry_pos;
}