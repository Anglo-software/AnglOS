#include "nvme.h"
#include "device/pci/pci.h"
#include "mm/paging/paging.h"
#include "libc/stdio.h"
#include "device/apic/timer.h"

static uint64_t* nvme_addr = NULL;
static uint64_t* msix_addr = NULL;
static uint32_t msix_size = 0;
static nvme_capabilities_t cap;
static uint64_t mem_page_min;
static uint64_t mem_page_max;
static uint64_t doorbell_stride;

void init_nvme() {
    uint8_t nvme_bus = 0;
    uint8_t nvme_dev = 0;

    for (uint8_t bus = 0; bus <= 255; bus++) {
        for (uint8_t dev = 0; dev <= 31; dev++) {
            if (pciGetBaseClass(bus, dev, 0) == 0x1 &&
                pciGetSubClass(bus, dev, 0) == 0x8) {
                nvme_bus = bus;
                nvme_dev = dev;
                goto PCISEARCHEND;
            }
        }
    }

PCISEARCHEND:

    uint64_t bar0 = pciGetBAR(nvme_bus, nvme_dev, 0, 0);
    uint64_t bar1 = pciGetBAR(nvme_bus, nvme_dev, 0, 1);

    uint64_t* nvme_paddr = (uint64_t*)((bar0 & 0xFFFFFFF0) + ((bar1 & 0xFFFFFFFF) << 32));
    uint64_t* nvme_vaddr = 0xFFFFFF8000000000;
    uint64_t* msix_paddr = pciGetMessageTableBaseAddr(nvme_bus, nvme_dev, 0) +
                           pciGetMessageTableOffset(nvme_bus, nvme_dev, 0);
    uint64_t* msix_vaddr = 0xFFFFFF9000000000;
    msix_size = pciGetMessageTableSize(nvme_bus, nvme_dev, 0);

    if (videntity(nvme_vaddr, nvme_paddr, 1, PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE)) {
        nvme_addr = nvme_vaddr;
    }

    if (videntity(msix_vaddr, msix_paddr, 1, PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE)) {
        msix_addr = msix_vaddr;
    }

    cap.capabilities = nvme_addr[0];
    mem_page_min = 1 << (12 + cap.g.mem_page_min);
    mem_page_max = 1 << (12 + cap.g.mem_page_max);
    doorbell_stride = 1 << (2 + cap.g.doorbell_stride);

    pci_command_t command;

    command.g.bus_master = 1;
    command.g.io_space = 1;
    command.g.mem_space = 1;
    command.g.int_disable = 1;

    pciWriteCommand(nvme_bus, nvme_dev, 0, command.command);

    pciEnableMSIX(nvme_bus, nvme_dev, 0);
}

uint32_t nvmeReadStatus() {
    return (nvme_addr[3] >> 32) & 0xFFFFFFFF;
}

uint32_t nvmeReadConfig() {
    return (nvme_addr[2] >> 32) & 0xFFFFFFFF;
}

void nvmeWriteConfig(uint32_t config) {
    uint64_t data = nvme_addr[2];
    nvme_addr[2] = (data & 0xFFFFFFFF) | ((uint64_t)config << 32);
}