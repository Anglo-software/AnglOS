#include <basic_includes.h>

/**
 * 4-level paging: total of 256TB of VA space
 * 0x0000000000000000 - 0x00007FFFFFFFFFFF for user
 * 0xFFFF800000000000 - 0xFFFFFFFFFFFFFFFF for kernel
 *
 * 64-bit virtual address:
 * |0000000000000000|000000000|000000000|000000000|000000000|000000000000|
 * |[SIGN-EXTENSION]|[  P4E  ]|[ PDPTE ]|[  PDE  ]|[  PTE  ]|[  OFFSET  ]|
 * |63            48|47     39|38     30|29     21|20     12|11         0|
 *
 * P4E   = Page Map Level 4 Entry
 * P4T   = Page Map Level 4 Table
 *
 * PDPTE = Page Directory Pointer Table Entry
 * PDPT  = Page Directory Pointer Table
 *
 * PDE   = Page Directory Entry
 * PDT   = Page Directory Table
 *
 * PTE   = Page Table Entry
 * PT    = Page Table
 *
 *
 * For 64-bit VAs (virtual addresses) and 52-bit PAs (physical addresses):
 *
 * CR3 Register -> P4T PA
 *
 * The PAs here are the bits 12-51 of the actual physical addresses
 * This is because each structure table is aligned on a page and one page in
 * size And as such the least significant bits 0-11 are just all 0's.
 *
 * Page Map Level 4 Entry:
 * |0|00000000000|0000000000000000000000000000000000000000|0000|00000000| AVL =
 * 0x001 |X|[   AVL   ]|[               PDPT  PA               ]|[AV]|[ FLAG ]|
 * |D|62       52|51                                    12|11 8|7      0|
 *
 * Page Directory Pointer Table Entry:
 * |0|00000000000|0000000000000000000000000000000000000000|0000|00000000| AVL =
 * 0x002 |X|[   AVL   ]|[                PDT PA                ]|[AV]|[ FLAG ]|
 * |D|62       52|51                                    12|11 8|7      0|
 *
 * Page Directory Entry:
 * |0|00000000000|0000000000000000000000000000000000000000|0000|00000000| AVL =
 * 0x004 |X|[   AVL   ]|[                PT  PA                ]|[AV]|[ FLAG ]|
 * |D|62       52|51                                    12|11 8|7      0|
 *
 * XD       = Execution Disable
 * AVL = AV = Available for use
 * FLAG = [RESERVED][AVL][A][PCD][PWT][U/S][R/W][P] 1-bit each
 *      -> RESERVED = 0
 *      -> AVL      = Available
 *      -> A        = Accessed
 *      -> PCD      = Cache Disable
 *      -> PWT      = Write-Through
 *      -> U/S      = User/Supervisor
 *      -> R/W      = Read/Write
 *      -> P        = Present
 *
 *
 * Page Table Entry:
 * |0|0000|0000000|0000000000000000000000000000000000000000|000|000000000| AVL =
 * 0x008 |X|[PK]|[ AVL ]|[         PHYSICAL PAGE NUMBER         ]|[A]|[ FLAGS ]|
 * |D|    |58   52|51                                    12| V |8       0|
 *
 * XD       = Execution Disable
 * PK       = Protection Key
 * AVL = AV = Available for use
 * FLAG = [G][PAT][D][A][PCD][PWT][U/S][R/W][P] 1-bit each
 *      -> G   = Global
 *      -> PAT = Page Attribute Table
 *      -> D   = Dirty
 *      -> A   = Accessed
 *      -> PCD = Cache Disable
 *      -> PWT = Write-Through
 *      -> U/S = User/Supervisor
 *      -> R/W = Read/Write
 *      -> P   = Present
 *
 * PAT -> If PAT is supported, then PAT along with PCD and PWT shall indicate
 * the memory caching type, otherwise it is reserved and must be set to 0.
 *
 * G   -> Tells the processor not to invalidate the TLB entry correstponding to
 * the page upon a MOV to CR3 instruction. Bit 7 (PGE) in CR4 must be set to
 * enable global pages.
 *
 * D   -> Used to determine whether a page has been written to.
 *
 * A   -> Used to discover whether a table entry was read during a VA
 * translation. If it has, then the bit is set, otherwise, it is not. The bit
 * will not be cleared by the CPU, that burden falls on the OS (if it needs this
 * bit at all).
 *
 * PCD -> If the bit is set, the page will not be cached, otherwise, it will be.
 *
 * PWT -> If the bit is set, write-through caching is enabled, otherwise
 * write-back is enabled instead.
 *
 * U/S -> Controls access to the page based on privilege level.
 *        If the bit is set, then the page may be accessed by all.
 *        If it is cleared, then only the supervisor can access it.
 *
 * R/W -> If the bit is set, the page is read/write, otherwise the page is
 * read-only. The WP bit in CR0 determines if this is only applied to userland,
 * always giving the kernel write access.
 *
 * P   -> If this bit is set, the page is actually in physical memory at the
 * moment.
 *
 * Page fault error codes pushed onto stack: (these bits are the only ones used,
 * all others are reserved) Bit 0  (P)    present flag Bit 1  (R/W)  read/write
 * flag Bit 2  (U/S)  user/supervisor flag Bit 3  (RSVD) indicates whether a
 * reserved bit was set in some page-structure entry Bit 4  (I/D)
 * instruction/data flag (1 = instruction fetch, 0 = data access) Bit 5  (PK)
 * protection key violation Bit 6  (SS)   shadow-stack access fault Bit 15 (SGX)
 * SGX violation
 *
 *
 *
 * Virtual memory mapping:
 *
 * Kernel:
 * FFFF 8000 0000 0000 - FFFF 8007 FFFF FFFF -> 32  GB   direct mapping of all
 * physical memory (page_direct_base) FFFF 9000 0000 0000 - FFFF 9020 FFFF FFFF
 * -> 128 GB   kernel heap (kheap_base) (start at 1GB) FFFF A000 0000 0000 -
 * FFFF A00F FFFF FFFF -> 64  GB   lapic_base FFFF A100 0000 0000 - FFFF A10F
 * FFFF FFFF -> 64  GB   ioapic_base FFFF B000 0000 0000 - FFFF B00F FFFF FFFF
 * -> 64  GB   kernel and irq/isr stacks (kstack_base) (start at 512MB) FFFF
 * FF80 0000 0000 - FFFF FF85 0000 FFFF -> ??  GB   NVMe FFFF FFFF 8000 0000 -
 * FFFF FFFF FFFF FFFF -> 2   GB   kernel text mapping (kernel_base)
 */

#define PAGE_SIZE                 4096

#define PAGE_FLAG_PRESENT         0x0000000000000001 // P4E/PDPTE/PDE/PTE
#define PAGE_FLAG_READWRITE       0x0000000000000002 // P4E/PDPTE/PDE/PTE
#define PAGE_FLAG_USERSUPER       0x0000000000000004 // P4E/PDPTE/PDE/PTE
#define PAGE_FLAG_WRITETHRU       0x0000000000000008 // P4E/PDPTE/PDE/PTE
#define PAGE_FLAG_CACHEDBLE       0x0000000000000010 // P4E/PDPTE/PDE/PTE
#define PAGE_FLAG_ACCESSED        0x0000000000000020 // P4E/PDPTE/PDE/PTE
#define PAGE_FLAG_AVAILABLE       0x0000000000000040 // P4E/PDPTE/PDE
#define PAGE_FLAG_RESERVED        0x0000000000000080 // P4E/PDPTE/PDE
#define PAGE_FLAG_EXECDBLE        0x8000000000000000 // P4E/PDPTE/PDE/PTE

#define PAGE_FLAG_DIRTY           0x0000000000000040 // PTE
#define PAGE_FLAG_ATTRIBUTE       0x0000000000000080 // PTE
#define PAGE_FLAG_GLOBAL          0x0000000000000100 // PTE

#define PAGE_FAULT_FLAG_PRESENT   0x0001
#define PAGE_FAULT_FLAG_READWRITE 0x0002
#define PAGE_FAULT_FLAG_USERSUPER 0x0004
#define PAGE_FAULT_FLAG_RESERVED  0x0008
#define PAGE_FAULT_FLAG_INSTDATA  0x0010
#define PAGE_FAULT_FLAG_PROTKEY   0x0020
#define PAGE_FAULT_FLAG_SHADOW    0x0040
#define PAGE_FAULT_FLAG_SGXVIOL   0x8000

#define PAGE_STRUCTURE_P4         0x0010000000000000 // P4E
#define PAGE_STRUCTURE_P3         0x0020000000000000 // PDPTE
#define PAGE_STRUCTURE_P2         0x0040000000000000 // PDE
#define PAGE_STRUCTURE_P1         0x0080000000000000 // PTE

#define PAGE_OFFSET(vptr)         ((vptr >> 0) & 0xFFF)
#define PAGE_P1E(vptr)            ((vptr >> 12) & 0x1FF)
#define PAGE_P2E(vptr)            ((vptr >> 21) & 0x1FF)
#define PAGE_P3E(vptr)            ((vptr >> 30) & 0x1FF)
#define PAGE_P4E(vptr)            ((vptr >> 39) & 0x1FF)

typedef struct {
    uint64_t xd_flag : 1;
    uint64_t avl_1   : 11;
    uint64_t ppn     : 40;
    uint64_t avl_2   : 4;
    uint64_t flags   : 8;
} __attribute__((packed)) page_structure_entry_t;

typedef struct {
    uint64_t xd_flag : 1;
    uint64_t pk      : 4;
    uint64_t avl_1   : 7;
    uint64_t ppn     : 40;
    uint64_t avl_2   : 3;
    uint64_t flags   : 9;
} __attribute__((packed)) page_table_entry_t;

void initPaging();
void initAPPaging();

/**
 * alloc  - allocate one table (page) and return its physical address
 *        --- could also allocate a lot of pages at once and keep a counter of
 * how many are used
 *        --- allocate more if they are all used up
 * free   - free one table (page) using its physical address
 * add    - create an entry at an index in a page structure pointing to a
 * physical address with flags update - update an entry at an index in a page
 * structure pointing to a physical address with flags remove - remove an entry
 * at an index in a page structure and free its children if it is not a PTE
 * getpfv - do a page walk on a virtual address
 * updcr3 - update the CR3 register with a physical address to the P4T
 * vmaloc - allocate some number of contiguous pages in virtual memory starting
 * at vptr with flags vfree  - free some number of contiguous pages in virtual
 * memory starting at vptr
 */
void* pagingAllocTable();
void pagingFreeTable(void* table_pptr);
void pagingAddEntry(void* table_pptr, uint16_t entry_num, void* entry_pptr,
                    uint64_t flags);
void pagingUpdateEntry(void* table_pptr, uint16_t entry_num, void* entry_pptr,
                       uint64_t flags);
void pagingRemoveEntry(void* table_pptr, uint16_t level, uint16_t entry_num,
                       bool do_pfree);
void* pagingGetCR3();
void pagingSetCR3(void* pptr);
void* vmalloc(void* vptr, size_t pages, uint64_t flags);
void vfree(void* vptr, size_t pages, bool do_pfree);
void* videntity(void* vptr, void* pptr, size_t pages, uint64_t flags);
void* pagingCreateUser();
void pagingDumpTable(void* vptr, uint16_t level);