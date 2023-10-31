#include "addressspace.h"
#include "libc/stdio.h"
#include "mm/heap/heap.h"
#include "mm/paging/paging.h"

static uint64_t convAddrSpaceFlagsToPaging(uint32_t flags) {
    uint64_t paging_flags = PAGE_FLAG_PRESENT;
    if (!(flags & ADDRSPACE_FLAG_EXECUTABLE)) {
        paging_flags |= PAGE_FLAG_EXECDBLE;
    }
    if (flags & ADDRSPACE_FLAG_WRITEABLE) {
        paging_flags |= PAGE_FLAG_READWRITE;
    }
    if (flags & ADDRSPACE_FLAG_UNCACHEABLE) {
        paging_flags |= PAGE_FLAG_CACHEDBLE;
    }
    if (flags & ADDRSPACE_FLAG_USER) {
        paging_flags |= PAGE_FLAG_USERSUPER;
    }
    return paging_flags;
}

address_space_t* addressSpaceNew(uint64_t cr3) {
    address_space_t* addrspace =
        (address_space_t*)kmalloc(sizeof(address_space_t));
    addrspace->cr3         = cr3;
    addrspace->num_entries = 0;
    addrspace->entries     = NULL;
}

void addressSpaceAddEntry(address_space_t* space, uint32_t type, uint32_t flags,
                          uint64_t size, uint64_t start) {
    if (space->entries == NULL) {
        space->entries =
            (address_space_entry_t*)kmalloc(sizeof(address_space_entry_t));
    }
    else {
        space->entries = (address_space_entry_t*)krealloc(
            space->entries, (space->num_entries + 1) * sizeof(address_space_t));
    }
    space->entries[space->num_entries].type       = type;
    space->entries[space->num_entries].flags      = flags;
    space->entries[space->num_entries].size       = size;
    space->entries[space->num_entries].addr_start = start;
    uint64_t start_page                           = ALIGN_TO_PAGE(start);
    uint64_t num_pages                            = GET_NUM_PAGES(size);
    vmallocCR3(start_page, num_pages, convAddrSpaceFlagsToPaging(flags), space->cr3);
    space->num_entries++;
}

uint32_t addressSpaceChangeFlags(address_space_t* space, uint64_t entry,
                                 uint32_t flags) {
    uint32_t old_flags          = space->entries[entry].flags;
    space->entries[entry].flags = flags;
    uint64_t start_page = ALIGN_TO_PAGE(space->entries[entry].addr_start);
    uint64_t num_pages  = GET_NUM_PAGES(space->entries[entry].size);
    vflagsCR3(start_page, num_pages, convAddrSpaceFlagsToPaging(flags), space->cr3);
    return old_flags;
}