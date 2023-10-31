#pragma once
#include <basic_includes.h>

#define ADDRSPACE_TYPE_CODE        0
#define ADDRSPACE_TYPE_DATA        1
#define ADDRSPACE_TYPE_RODATA      2
#define ADDRSPACE_TYPE_BSS         3
#define ADDRSPACE_TYPE_HEAP        4
#define ADDRSPACE_TYPE_STACK       5

#define ADDRSPACE_FLAG_EXECUTABLE  (1 << 0)
#define ADDRSPACE_FLAG_WRITEABLE   (1 << 1)
#define ADDRSPACE_FLAG_READABLE    (1 << 2)
#define ADDRSPACE_FLAG_UNCACHEABLE (1 << 3)
#define ADDRSPACE_FLAG_USER        (1 << 4)

typedef struct {
    uint32_t type;
    uint32_t flags;
    uint64_t size;
    uint64_t addr_start;
} address_space_entry_t;

typedef struct {
    uint64_t cr3;
    uint64_t num_entries;
    address_space_entry_t* entries;
} address_space_t;

address_space_t* addressSpaceNew(uint64_t cr3);
void addressSpaceAddEntry(address_space_t* space, uint32_t type, uint32_t flags,
                          uint64_t size, uint64_t start);
uint32_t addressSpaceChangeFlags(address_space_t* space, uint64_t entry,
                                 uint32_t flags);