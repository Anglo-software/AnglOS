#include "nvme.h"
#include "boot/interrupts/isr.h"
#include "device/apic/apic.h"
#include "device/apic/timer.h"
#include "device/pci/pci.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "mm/paging/paging.h"
#include "mm/pmm/pmm.h"

nvme_isr_frame_t* nvme_reg             = (nvme_isr_frame_t*)0xFFFFFF8000000000;
msix_entry_t* msix_addr                = (msix_entry_t*)0xFFFFFF8100000000;
nvme_submission_t* nvme_admin_sub_addr = (nvme_submission_t*)0xFFFFFF8200000000;
nvme_completion_t* nvme_admin_com_addr = (nvme_completion_t*)0xFFFFFF8300000000;
nvme_submission_t* nvme_io_sub_addr    = (nvme_submission_t*)0xFFFFFF8400000000;
nvme_completion_t* nvme_io_com_addr    = (nvme_completion_t*)0xFFFFFF8500000000;

nvme_identify_t* id;
uint32_t* namespace_list;
nvme_namespace_t* namespace;

static uint32_t msix_size = 0;
static nvme_capabilities_t cap;
static uint64_t mem_page_min;
static uint64_t mem_page_max;
static uint64_t doorbell_stride;

static uint16_t admin_queue_size        = 8;
static volatile uint16_t admin_sub_tail = 0;
static volatile uint16_t admin_com_head = 0;
static uint8_t admin_new_phase          = 1;

static uint16_t io_queue_size           = 32;
static volatile uint16_t io_sub_tail    = 0;
static volatile uint16_t io_com_head    = 0;
static uint8_t io_new_phase             = 1;

static void putAdminCommand(nvme_submission_t* command);
static void ringAdminSub(uint16_t num);
static void getAdminResponse(nvme_completion_t* response);
static void ringAdminCom(uint16_t num);

static void putIOCommand(nvme_submission_t* command);
static void ringIOSub(uint16_t num);
static void getIOResponse(nvme_completion_t* response);
static void ringIOCom(uint16_t num);

void irqNVMeHandler();

#pragma GCC push_options
#pragma GCC optimize("O0")

void initNVMe() {
    uint8_t nvme_bus = 0;
    uint8_t nvme_dev = 0;

    for (uint16_t bus = 0; bus <= 255; bus++) {
        for (uint8_t dev = 0; dev <= 31; dev++) {
            if (pciGetBaseClass(bus, dev, 0) == PCI_CLASS_MASS_STORAGE &&
                pciGetSubClass(bus, dev, 0) == PCI_SUBCLASS_NVM) {
                nvme_bus = bus;
                nvme_dev = dev;
                goto PCISEARCHEND;
            }
        }
    }

PCISEARCHEND:

    uint64_t bar0 = pciGetBAR(nvme_bus, nvme_dev, 0, 0);
    uint64_t bar1 = pciGetBAR(nvme_bus, nvme_dev, 0, 1);

    void* nvme_paddr =
        (uint64_t*)((bar0 & 0xFFFFFFF0) + ((bar1 & 0xFFFFFFFF) << 32));
    void* msix_paddr =
        (void*)(pciGetMessageTableBaseAddr(nvme_bus, nvme_dev, 0) +
                pciGetMessageTableOffset(nvme_bus, nvme_dev, 0));
    void* admin_sub_paddr = pmalloc(1);
    void* admin_com_paddr = pmalloc(1);
    void* sub_paddr       = pmalloc(1);
    void* com_paddr       = pmalloc(1);

    msix_size             = pciGetMessageTableSize(nvme_bus, nvme_dev, 0);

    videntity(nvme_reg, nvme_paddr, 2,
              PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_CACHEDBLE |
                  PAGE_FLAG_EXECDBLE);
    videntity(msix_addr, msix_paddr, 2,
              PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_CACHEDBLE |
                  PAGE_FLAG_EXECDBLE);
    videntity(nvme_admin_sub_addr, admin_sub_paddr, 1,
              PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_CACHEDBLE |
                  PAGE_FLAG_EXECDBLE);
    videntity(nvme_admin_com_addr, admin_com_paddr, 1,
              PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_CACHEDBLE |
                  PAGE_FLAG_EXECDBLE);
    videntity(nvme_io_sub_addr, sub_paddr, 1,
              PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_CACHEDBLE |
                  PAGE_FLAG_EXECDBLE);
    videntity(nvme_io_com_addr, com_paddr, 1,
              PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_CACHEDBLE |
                  PAGE_FLAG_EXECDBLE);

    cap.capabilities = nvme_reg->capabilities;
    mem_page_min     = 1 << (12 + cap.g.mem_page_min);
    mem_page_max     = 1 << (12 + cap.g.mem_page_max);
    doorbell_stride  = 1 << (2 + cap.g.doorbell_stride);

    pci_command_t command;

    command.g.bus_master  = 1;
    command.g.io_space    = 0;
    command.g.mem_space   = 1;
    command.g.int_disable = 1;

    pciWriteCommand(nvme_bus, nvme_dev, 0, command.command);

    pciEnableMSIX(nvme_bus, nvme_dev, 0);

    msix_entry_t admin_int;
    admin_int.masked          = 0;
    admin_int.message_addr_lo = 0xFEE00000;
    admin_int.message_addr_hi = 0;
    admin_int.message_data    = IRQ3;
    admin_int.rev0            = 0;
    lapicWriteReg(APIC_ICRH, 0);
    pciWriteMSIXEntry(msix_addr, 0, &admin_int);
    irqRegisterHandler(IRQ3, (void*)irqNVMeHandler);

    nvme_status_t status;
    nvme_config_t config;

    status.status   = nvmeReadStatus();

    config.g.enable = 0;

    nvmeWriteConfig(config.config);

    while (status.g.ready) {
        apicNanosleep(100);
        status.status = nvmeReadStatus();
    }

    config.g.mem_page_size    = 0;
    config.g.com_entry_size   = 4;
    config.g.sub_entry_size   = 6;
    config.g.arbitration_mech = 0;
    config.g.command_set      = 0;
    config.g.shutdown         = 0;
    config.g.enable           = 1;

    nvme_reg->admin_attr =
        ((admin_queue_size - 1) << 16) | (admin_queue_size - 1);

    nvme_reg->admin_sub_base = (uint64_t)admin_sub_paddr;
    nvme_reg->admin_com_base = (uint64_t)admin_com_paddr;

    nvmeWriteConfig(config.config);

    while (!status.g.ready) {
        apicNanosleep(100);
        status.status = nvmeReadStatus();
    }

    void* paddr = pmalloc(1);
    nvmeIdentifyController(paddr);
    ringAdminSub(1);

    __asm__ volatile("hlt");

    nvme_completion_t response;
    getAdminResponse(&response);
    ringAdminCom(1);

    id    = (nvme_identify_t*)((uint64_t)paddr + 0xFFFF800000000000);

    paddr = pmalloc(1);
    nvmeIdentifyNamespaceList(paddr);
    ringAdminSub(1);

    __asm__ volatile("hlt");

    getAdminResponse(&response);
    ringAdminCom(1);

    namespace_list = (uint32_t*)((uint64_t)paddr + 0xFFFF800000000000);

    paddr          = pmalloc(1);
    nvmeIdentifyNamespace(paddr, 1);
    ringAdminSub(1);

    __asm__ volatile("hlt");

    getAdminResponse(&response);
    ringAdminCom(1);

    namespace = (nvme_namespace_t*)((uint64_t)paddr + 0xFFFF800000000000);

    nvmeCreateComQueue(com_paddr, 1, io_queue_size,
                       NVME_QUEUE_FLAG_PC | NVME_QUEUE_FLAG_INT, 1);
    ringAdminSub(1);

    __asm__ volatile("hlt");

    getAdminResponse(&response);
    ringAdminCom(1);

    nvmeCreateSubQueue(sub_paddr, 1, io_queue_size, NVME_QUEUE_FLAG_PC, 1);
    ringAdminSub(1);

    __asm__ volatile("hlt");

    getAdminResponse(&response);
    ringAdminCom(1);

    msix_entry_t io_int;
    io_int.masked          = 0;
    io_int.message_addr_lo = 0xFEE00000;
    io_int.message_addr_hi = 0;
    io_int.message_data    = IRQ3;
    io_int.rev0            = 0;
    pciWriteMSIXEntry(msix_addr, 1, &io_int);
}

#pragma GCC pop_options

void irqNVMeHandler() { apicSendEOI(); }

uint32_t nvmeReadStatus() { return nvme_reg->status; }

uint32_t nvmeReadConfig() { return nvme_reg->config; }

void nvmeWriteConfig(uint32_t config) { nvme_reg->config = config; }

void nvmeCreateSubQueue(void* paddr, uint16_t id, uint16_t size, uint16_t flags,
                        uint16_t com_id) {
    nvme_submission_t command;
    command.c.opcode  = NVME_CREATE_SUBMISSION_QUEUE;
    command.c.fused   = 0;
    command.c.id      = NVME_CREATE_SUBMISSION_QUEUE;
    command.c.prp_sgl = 0;
    command.nsid      = 0;
    command.rev0      = 0;
    command.meta      = 0;
    command.prp0      = (uint64_t)paddr;
    command.prp1      = 0;
    command.com_spec0 = id | ((size - 1) << 16);
    command.com_spec1 = flags | (com_id << 16);
    command.com_spec2 = 0;
    command.com_spec3 = 0;
    command.com_spec4 = 0;
    command.com_spec5 = 0;
    putAdminCommand(&command);
}

void nvmeCreateComQueue(void* paddr, uint16_t id, uint16_t size, uint16_t flags,
                        uint16_t int_vector) {
    nvme_submission_t command;
    command.c.opcode  = NVME_CREATE_COMPLETION_QUEUE;
    command.c.fused   = 0;
    command.c.id      = NVME_CREATE_COMPLETION_QUEUE;
    command.c.prp_sgl = 0;
    command.nsid      = 0;
    command.rev0      = 0;
    command.meta      = 0;
    command.prp0      = (uint64_t)paddr;
    command.prp1      = 0;
    command.com_spec0 = id | ((size - 1) << 16);
    command.com_spec1 = flags | (int_vector << 16);
    command.com_spec2 = 0;
    command.com_spec3 = 0;
    command.com_spec4 = 0;
    command.com_spec5 = 0;
    putAdminCommand(&command);
}

void nvmeIdentifyNamespace(void* paddr, uint32_t nsid) {
    nvme_submission_t command;
    command.c.opcode  = NVME_IDENTIFY;
    command.c.fused   = 0;
    command.c.id      = NVME_IDENTIFY;
    command.c.prp_sgl = 0;
    command.nsid      = nsid;
    command.rev0      = 0;
    command.meta      = 0;
    command.prp0      = (uint64_t)paddr;
    command.prp1      = 0;
    command.com_spec0 = 0;
    command.com_spec1 = 0;
    command.com_spec2 = 0;
    command.com_spec3 = 0;
    command.com_spec4 = 0;
    command.com_spec5 = 0;
    putAdminCommand(&command);
}

void nvmeIdentifyController(void* paddr) {
    nvme_submission_t command;
    command.c.opcode  = NVME_IDENTIFY;
    command.c.fused   = 0;
    command.c.id      = NVME_IDENTIFY;
    command.c.prp_sgl = 0;
    command.nsid      = 0;
    command.rev0      = 0;
    command.meta      = 0;
    command.prp0      = (uint64_t)paddr;
    command.prp1      = 0;
    command.com_spec0 = 1;
    command.com_spec1 = 0;
    command.com_spec2 = 0;
    command.com_spec3 = 0;
    command.com_spec4 = 0;
    command.com_spec5 = 0;
    putAdminCommand(&command);
}

void nvmeIdentifyNamespaceList(void* paddr) {
    nvme_submission_t command;
    command.c.opcode  = NVME_IDENTIFY;
    command.c.fused   = 0;
    command.c.id      = NVME_IDENTIFY;
    command.c.prp_sgl = 0;
    command.nsid      = 0;
    command.rev0      = 0;
    command.meta      = 0;
    command.prp0      = (uint64_t)paddr;
    command.prp1      = 0;
    command.com_spec0 = 2;
    command.com_spec1 = 0;
    command.com_spec2 = 0;
    command.com_spec3 = 0;
    command.com_spec4 = 0;
    command.com_spec5 = 0;
    putAdminCommand(&command);
}

void nvmeRead(nvme_read_info_t* info) {
    nvme_submission_t command;
    command.c.opcode  = NVME_READ;
    command.c.fused   = 0;
    command.c.id      = NVME_READ;
    command.c.prp_sgl = 0;
    command.nsid      = info->nsid;
    command.rev0      = 0;
    command.meta      = info->meta;
    command.prp0      = info->prp0;
    command.prp1      = info->prp1;
    command.com_spec0 = info->lba_start & 0xFFFFFFFF;
    command.com_spec1 = (info->lba_start >> 32) & 0xFFFFFFFF;
    command.com_spec2 = ((info->limited_retry & 0x1) << 31) |
                        ((info->force_unit_access & 0x1) << 30) |
                        ((info->protection_info & 0xF) << 26) |
                        (info->lba_num & 0xFFFF);
    command.com_spec3 = 0;
    command.com_spec4 = 0;
    command.com_spec5 = 0;
    putIOCommand(&command);
}

void nvmeWrite(nvme_write_info_t* info) {
    nvme_submission_t command;
    command.c.opcode  = NVME_WRITE;
    command.c.fused   = 0;
    command.c.id      = NVME_WRITE;
    command.c.prp_sgl = 0;
    command.nsid      = info->nsid;
    command.rev0      = 0;
    command.meta      = info->meta;
    command.prp0      = info->prp0;
    command.prp1      = info->prp1;
    command.com_spec0 = info->lba_start & 0xFFFFFFFF;
    command.com_spec1 = (info->lba_start >> 32) & 0xFFFFFFFF;
    command.com_spec2 = ((info->limited_retry & 0x1) << 31) |
                        ((info->force_unit_access & 0x1) << 30) |
                        ((info->protection_info & 0xF) << 26) |
                        (info->lba_num & 0xFFFF);
    command.com_spec3 = 0;
    command.com_spec4 = 0;
    command.com_spec5 = 0;
    putIOCommand(&command);
}

static void putAdminCommand(nvme_submission_t* command) {
    void* dest = &nvme_admin_sub_addr[admin_sub_tail];
    memcpy(dest, (void*)command, sizeof(nvme_submission_t));
}

static void ringAdminSub(uint16_t num) {
    admin_sub_tail = (admin_sub_tail + num) % admin_queue_size;
    volatile uint16_t* reg =
        (uint16_t*)((uint64_t)(&nvme_reg->doorbell_base) + 0 * doorbell_stride);
    *reg = admin_sub_tail;
}

static void getAdminResponse(nvme_completion_t* response) {
    void* src = &nvme_admin_com_addr[admin_com_head];
    memcpy((void*)response, src, sizeof(nvme_completion_t));
}

static void ringAdminCom(uint16_t num) {
    admin_com_head = (admin_com_head + num) % admin_queue_size;
    if (admin_com_head == 0) {
        admin_new_phase = admin_new_phase ^ 0x1;
    }
    volatile uint16_t* reg =
        (uint16_t*)((uint64_t)(&nvme_reg->doorbell_base) + 1 * doorbell_stride);
    *reg = admin_com_head;
}

static void putIOCommand(nvme_submission_t* command) {
    void* dest = &nvme_io_sub_addr[io_sub_tail];
    memcpy(dest, (void*)command, sizeof(nvme_submission_t));
}

static void ringIOSub(uint16_t num) {
    io_sub_tail = (io_sub_tail + num) % io_queue_size;
    volatile uint16_t* reg =
        (uint16_t*)((uint64_t)(&nvme_reg->doorbell_base) + 2 * doorbell_stride);
    *reg = io_sub_tail;
}

static void getIOResponse(nvme_completion_t* response) {
    void* src = &nvme_io_com_addr[io_com_head];
    memcpy((void*)response, src, sizeof(nvme_completion_t));
}

static void ringIOCom(uint16_t num) {
    io_com_head = (io_com_head + num) % io_queue_size;
    if (io_com_head == 0) {
        io_new_phase = io_new_phase ^ 0x1;
    }
    volatile uint16_t* reg =
        (uint16_t*)((uint64_t)(&nvme_reg->doorbell_base) + 3 * doorbell_stride);
    *reg = io_com_head;
}