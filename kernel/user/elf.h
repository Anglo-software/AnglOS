#pragma once
#include <basic_includes.h>
#include "mm/addressspace/addressspace.h"

typedef struct {
    uint8_t magic[4];
    uint8_t bits;
    uint8_t endianness;
    uint8_t header_version;
    uint8_t abi;
    uint64_t rev0;
    uint16_t type;
    uint16_t isa;
    uint32_t version;
    uint64_t program_entry_pos;
    uint64_t program_header_table_pos;
    uint64_t section_header_table_pos;
    uint32_t flags;
    uint16_t header_size;
    uint16_t program_header_entry_size;
    uint16_t program_header_num_entries;
    uint16_t section_header_entry_size;
    uint16_t section_header_num_entries;
    uint16_t section_name_index;
} __attribute__((packed)) elf_header_t;

typedef struct {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t rev0;
    uint64_t size_file;
    uint64_t size_mem;
    uint64_t alignment;
} __attribute__((packed)) elf_program_t;

#define ELF_ARCH_NONSPECIFIC 0x00
#define ELF_ARCH_SPARC       0x02
#define ELF_ARCH_X86         0x03
#define ELF_ARCH_MIPS        0x08
#define ELF_ARCH_POWERPC     0x14
#define ELF_ARCH_ARM         0x28
#define ELF_ARCH_SUPERH      0x2A
#define ELF_ARCH_IA_64       0x32
#define ELF_ARCH_X86_64      0x3E
#define ELF_ARCH_AARCH64     0xB7
#define ELF_ARCH_RISC_V      0xF3

#define ELF_SEGMENT_NULL     0
#define ELF_SEGMENT_LOAD     1
#define ELF_SEGMENT_DYNA     2
#define ELF_SEGMENT_INTP     3
#define ELF_SEGMENT_NOTE     4

#define ELF_FLAG_EXECUTABLE  (1 << 0)
#define ELF_FLAG_WRITEABLE   (1 << 1)
#define ELF_FLAG_READABLE    (1 << 2)

uint64_t elfLoad(address_space_t* space, void* file);