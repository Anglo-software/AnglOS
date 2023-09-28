#include "nvme.h"
#include "device/pci/pci.h"
#include "mm/paging/paging.h"
#include "mm/pmm/pmm.h"
#include "libc/stdio.h"
#include "device/apic/timer.h"
#include "libc/string.h"

nvme_registers_t* nvme_reg             = 0xFFFFFF8000000000;
msix_entry_t* msix_addr                = 0xFFFFFF8100000000;
nvme_submission_t* nvme_admin_sub_addr = 0xFFFFFF8200000000;
nvme_completion_t* nvme_admin_com_addr = 0xFFFFFF8300000000;

static uint32_t msix_size = 0;
static nvme_capabilities_t cap;
static uint64_t mem_page_min;
static uint64_t mem_page_max;
static uint64_t doorbell_stride;

static uint16_t admin_queue_size = 2;
static uint16_t admin_sub_head = 0;
static uint16_t admin_sub_tail = 0;
static uint16_t admin_com_head = 0;
static uint16_t admin_com_tail = 0;

static uint16_t queue_size = 32;

static void putAdminCommand(nvme_submission_t* command);
static void ringAdminSub(uint16_t num);
static void getAdminResponse(nvme_completion_t* response);
static void ringAdminCom(uint16_t num);

void init_nvme() {
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

    void* nvme_paddr = (uint64_t*)((bar0 & 0xFFFFFFF0) + ((bar1 & 0xFFFFFFFF) << 32));
    void* msix_paddr = pciGetMessageTableBaseAddr(nvme_bus, nvme_dev, 0) +
                       pciGetMessageTableOffset(nvme_bus, nvme_dev, 0);
    void* admin_sub_paddr = pmalloc(1);
    void* admin_com_paddr = pmalloc(1);
    
    msix_size = pciGetMessageTableSize(nvme_bus, nvme_dev, 0);

    videntity(nvme_reg, nvme_paddr, 2, PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_CACHEDBLE);
    videntity(msix_addr, msix_paddr, 2, PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_CACHEDBLE);
    videntity(nvme_admin_sub_addr, admin_sub_paddr, 1, PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_CACHEDBLE);
    videntity(nvme_admin_com_addr, admin_com_paddr, 1, PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_CACHEDBLE);

    cap.capabilities = nvme_reg->capabilities;
    mem_page_min = 1 << (12 + cap.g.mem_page_min);
    mem_page_max = 1 << (12 + cap.g.mem_page_max);
    doorbell_stride = 1 << (2 + cap.g.doorbell_stride);

    pci_command_t command;

    command.g.bus_master = 1;
    command.g.io_space = 0;
    command.g.mem_space = 1;
    command.g.int_disable = 1;

    pciWriteCommand(nvme_bus, nvme_dev, 0, command.command);

    pciEnableMSIX(nvme_bus, nvme_dev, 0);

    nvme_status_t status;
    nvme_config_t config;

    status.status = nvmeReadStatus();

    config.g.enable = 0;

    nvmeWriteConfig(config.config);

    while (status.g.ready) {
        nanosleep(1000);
        status.status = nvmeReadStatus();
    }

    config.g.mem_page_size = 0;
    config.g.com_entry_size = 4;
    config.g.sub_entry_size = 6;
    config.g.arbitration_mech = 0;
    config.g.command_set = 0;
    config.g.shutdown = 0;
    config.g.enable = 1;

    nvme_reg->admin_attr =  ((admin_queue_size - 1) << 16) | (admin_queue_size - 1);

    nvme_reg->admin_sub_base = (uint32_t)admin_sub_paddr;
    nvme_reg->admin_com_base = (uint32_t)admin_com_paddr;

    nvmeWriteConfig(config.config);

    while (!status.g.ready) {
        nanosleep(1000);
        status.status = nvmeReadStatus();
    }

    void* paddr = pmalloc(1);
    nvmeIdentifyController(paddr);
    ringAdminSub(1);

    millisleep(100);

    nvme_completion_t response;
    getAdminResponse(&response);
    ringAdminCom(1);

    nvme_identify_t* id = (nvme_identify_t*)((uint64_t)paddr + 0xFFFF800000000000);
}

uint32_t nvmeReadStatus() {
    return nvme_reg->status;
}

uint32_t nvmeReadConfig() {
    return nvme_reg->config;
}

void nvmeWriteConfig(uint32_t config) {
    nvme_reg->config = config;
}

void nvmeCreateSubQueue(void* paddr, uint16_t id, uint16_t size, uint16_t flags, uint16_t com_id) {

}

void nvmeCreateComQueue(void* paddr, uint16_t id, uint16_t size, uint16_t flags, uint16_t int_vector) {
    
}

void nvmeIdentifyNamespace(void* paddr, uint32_t nsid) {
    
}

void nvmeIdentifyController(void* paddr) {
    nvme_submission_t command;
    command.c.opcode = NVME_IDENTIFY;
    command.c.fused = 0;
    command.c.id = 0xE621;
    command.c.prp_sgl = 0;
    command.nsid = 0;
    command.rev0 = 0;
    command.meta = 0;
    command.prp0 = (uint64_t)paddr;
    command.prp1 = 0;
    command.com_spec0 = 1;
    command.com_spec1 = 0;
    command.com_spec2 = 0;
    command.com_spec3 = 0;
    command.com_spec4 = 0;
    command.com_spec5 = 0;
    putAdminCommand(&command);
}

void nvmeIdentifyNamespaceList(void* paddr) {
    
}

static void putAdminCommand(nvme_submission_t* command) {
    void* dest = &nvme_admin_sub_addr[admin_sub_tail];
    memcpy(dest, (void*)command, sizeof(nvme_submission_t));
}

static void ringAdminSub(uint16_t num) {
    admin_sub_tail = (admin_sub_tail + num) % admin_queue_size;
    volatile uint16_t* reg = (uint16_t*)((uint64_t)(&nvme_reg->doorbell_base) + 0 * doorbell_stride);
    *reg = admin_sub_tail;
}

static void getAdminResponse(nvme_completion_t* response) {
    void* src = &nvme_admin_com_addr[admin_com_head];
    memcpy((void*)response, src, sizeof(nvme_completion_t));
}

static void ringAdminCom(uint16_t num) {
    admin_com_head = (admin_com_head + num) % admin_queue_size;
    volatile uint16_t* reg = (uint16_t*)((uint64_t)(&nvme_reg->doorbell_base) + 1 * doorbell_stride);
    *reg = admin_com_head;
}