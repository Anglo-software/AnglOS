#include <basic_includes.h>

#ifndef GDT_H
#define GDT_H

struct gdt_segment_descriptor {
    uint16_t limit;       // Lower 16 bits of limit
    uint16_t base_low;    // Lower 16 bits of base
    uint8_t  base_middle; // Middle 8 bits of base
    uint8_t  access;      // Access bits
    uint8_t  granularity; // High 4 bits of limit and flags
    uint8_t  base_high;   // High 8 bits of base
} __attribute__((packed));
/**
 * Flags:
 * [G][D][L][R]
 *  3  2  1  0
 * G - Granularity   : Clear for byte granularity for limit
 *                     Set for page granularity (4 KiB) for limit
 * D - Size flag     : Clear for 16-bit segment, set for 32-bit
 *                     Clear if L is set
 * L - Long mode flag: Set if 64-bit code segment, 
 *     clear if D is set or other segment type
 * R - Reserved
 * 
 * 
 * 
 * Access:
 * [P][DPL][D][E][DC][RW][A]
 *  7  6 5  4  3  2   1   0
 * P   - Set for any valid segment
 * DPL - 0 = ring 0 (kernel), 3 = ring 3 (user)
 * D   - Clear for system segment, Set otherwise
 * E   - Clear for data segment, Set for code
 * DC  - Data segment: Clear if growing up, set if growing down
 *       Code segment: Clear if only exec from DPL ring
 *                    Set if exec from equal or lower DPL
 * RW  - Code segment: Clear for no read access, set for read access
 *                     Writes never allowed
 *       Data segment: Clear for no write access, set for write access
 *                     Reads always allowed
 * A   - Best to clear, CPU will set as necessary
 */

struct gdt_system_segment_descriptor {
    uint16_t limit_low;      // Lower 16 bits of limit
    uint16_t base_low;       // Lower 16 bits of base
    uint8_t  base_middle;    // Middle 8 bits of base
    uint8_t  type       : 4; // Type
    uint8_t  dt         : 1; // Descriptor type bit
    uint8_t  dpl        : 2; // Descriptor privilege level
    uint8_t  p          : 1; // Present bit
    uint8_t  limit_high : 4; // High 4 bits of limit
    uint8_t  flags      : 4; // Flags
    uint8_t  base_high;      // High 8 bits of base
    uint32_t base_extra;     // Extra 32 bits of base
    uint32_t reserved;       // Reserved
};
/**
 * Flags:
 * [G][D][L][R]
 *  3  2  1  0
 * G - Granularity   : Clear for byte granularity for limit
 *                     Set for page granularity (4 KiB) for limit
 * D - Size flag     : Clear for 16-bit segment, set for 32-bit
 *                     Clear if L is set
 * L - Long mode flag: Set if 64-bit code segment, 
 *     clear if D is set or other segment type
 * R - Reserved
 * 
 * 
 * 
 * Access:
 * [P][DPL][D][TYPE]
 *  7  6 5  4  3  0
 * P - Set for any valid segment
 * DPL - 0 = ring 0 (kernel), 3 = ring 3 (user)
 * D - Clear for system segment, Set otherwise
 * T - 0x2: LDT (Local Descriptor Table)
 *     0x9: TSS (Available)
 *     0xB: TSS (Busy)
 */

struct gdt_ptr_struct {
    uint16_t limit;  // Size of GDT in bytes minus 1
    uint64_t ptr;    // Linear address of the GDT (virtual address)
} __attribute__((packed));

typedef struct gdt_segment_descriptor gdt_seg_t;
typedef struct gdt_system_segment_descriptor gdt_sys_seg_t;
typedef struct gdt_ptr_struct gdt_ptr_t;

void init_gdt();

#endif