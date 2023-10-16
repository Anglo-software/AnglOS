#include "pci.h"
#include "device/io.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "mm/paging/paging.h"

extern void* page_direct_base;

void initPCI() {}

uint8_t pciConfigReadByte(uint8_t bus, uint8_t device, uint8_t func,
                          uint8_t offset) {
    uint32_t addr;
    uint32_t lbus    = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunc   = (uint32_t)func;

    addr = (uint32_t)((lbus << 16) | (ldevice << 11) | (lfunc << 8) |
                      (offset & 0xFC) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDR, addr);

    return (uint8_t)((inl(PCI_CONFIG_DATA) >> ((offset & 0b11) * 8)) & 0xFF);
}

uint16_t pciConfigReadWord(uint8_t bus, uint8_t device, uint8_t func,
                           uint8_t offset) {
    uint32_t addr;
    uint32_t lbus    = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunc   = (uint32_t)func;

    addr = (uint32_t)((lbus << 16) | (ldevice << 11) | (lfunc << 8) |
                      (offset & 0xFC) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDR, addr);

    return (uint16_t)((inl(PCI_CONFIG_DATA) >> ((offset & 0b10) * 8)) & 0xFFFF);
}

uint32_t pciConfigReadLong(uint8_t bus, uint8_t device, uint8_t func,
                           uint8_t offset) {
    uint32_t addr;
    uint32_t lbus    = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunc   = (uint32_t)func;

    addr = (uint32_t)((lbus << 16) | (ldevice << 11) | (lfunc << 8) |
                      (offset & 0xFC) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDR, addr);

    return inl(PCI_CONFIG_DATA);
}

void pciConfigWriteLong(uint8_t bus, uint8_t device, uint8_t func,
                        uint8_t offset, uint32_t data) {
    uint32_t addr;
    uint32_t lbus    = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunc   = (uint32_t)func;

    addr = (uint32_t)((lbus << 16) | (ldevice << 11) | (lfunc << 8) |
                      (offset & 0xFC) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDR, addr);

    outl(PCI_CONFIG_DATA, data);
}

uint8_t pciGetBaseClass(uint8_t bus, uint8_t device, uint8_t func) {
    return pciConfigReadByte(bus, device, func, 0xB);
}

uint8_t pciGetSubClass(uint8_t bus, uint8_t device, uint8_t func) {
    return pciConfigReadByte(bus, device, func, 0xA);
}

uint16_t pciReadStatus(uint8_t bus, uint8_t device, uint8_t func) {
    return pciConfigReadWord(bus, device, func, 0x6);
}

uint16_t pciReadCommand(uint8_t bus, uint8_t device, uint8_t func) {
    return pciConfigReadWord(bus, device, func, 0x4);
}

void pciWriteCommand(uint8_t bus, uint8_t device, uint8_t func,
                     uint16_t command) {
    uint32_t data = pciReadStatus(bus, device, func);
    data          = (data << 16) | command;

    pciConfigWriteLong(bus, device, func, 0x4, data);
}

uint32_t pciGetBAR(uint8_t bus, uint8_t device, uint8_t func, uint8_t bar) {
    return pciConfigReadLong(bus, device, func, 0x10 + (bar * 4));
}

uint8_t pciGetCapPointer(uint8_t bus, uint8_t device, uint8_t func) {
    return pciConfigReadByte(bus, device, func, 0x34);
}

uint8_t pciGetCapabilityOffset(uint8_t bus, uint8_t device, uint8_t func,
                               uint8_t cap) {
    uint8_t cap_base = pciGetCapPointer(bus, device, func);
    uint8_t curr_cap;
    uint8_t next = 255;
    while (next != 0) {
        curr_cap = pciConfigReadByte(bus, device, func, cap_base);
        next     = pciConfigReadByte(bus, device, func, cap_base + 1);
        if (curr_cap == cap) {
            return cap_base;
        }
        cap_base = next;
    }
    return 0;
}

uint64_t pciGetMessageTableBaseAddr(uint8_t bus, uint8_t device, uint8_t func) {
    uint8_t msix_base = pciGetCapabilityOffset(bus, device, func, 0x11);
    if (!msix_base) {
        return -1;
    }

    uint8_t bar_idx =
        pciConfigReadByte(bus, device, func, msix_base + 0x4) & 0b111;
    uint64_t bar0 = pciGetBAR(bus, device, func, bar_idx);
    bool x64_bar  = ((bar0 >> 1) & 0b11) == 2;

    if (x64_bar) {
        uint64_t bar1 = pciGetBAR(bus, device, func, bar_idx + 1);
        return (bar0 & 0xFFFFFFF0) + ((bar1 & 0xFFFFFFFF) << 32);
    }
    else {
        return bar0 & 0xFFFFFFF0;
    }
}

uint32_t pciGetMessageTableOffset(uint8_t bus, uint8_t device, uint8_t func) {
    uint8_t msix_base = pciGetCapabilityOffset(bus, device, func, 0x11);
    if (!msix_base) {
        return -1;
    }

    return pciConfigReadLong(bus, device, func, msix_base + 0x4) & 0xFFFFFFF8;
}

uint32_t pciGetMessageTableSize(uint8_t bus, uint8_t device, uint8_t func) {
    uint8_t msix_base = pciGetCapabilityOffset(bus, device, func, 0x11);
    if (!msix_base) {
        return -1;
    }

    return pciConfigReadWord(bus, device, func, msix_base + 0x2) & 0x7FF;
}

uint8_t pciEnableMSIX(uint8_t bus, uint8_t device, uint8_t func) {
    uint8_t msix_base = pciGetCapabilityOffset(bus, device, func, 0x11);
    if (!msix_base) {
        return -1;
    }

    uint32_t reg = pciConfigReadLong(bus, device, func, msix_base);
    pciConfigWriteLong(bus, device, func, msix_base,
                       (reg & 0x3FFFFFFF) | 0x80000000);
    return 0;
}

void pciWriteMSIXEntry(msix_entry_t* msix_base, uint16_t idx,
                       msix_entry_t* entry) {
    void* dest = &msix_base[idx];
    memcpy(dest, (void*)entry, sizeof(msix_entry_t));
}

void pciReadMSIXEntry(msix_entry_t* msix_base, uint16_t idx,
                      msix_entry_t* entry) {
    void* src = &msix_base[idx];
    memcpy((void*)entry, src, sizeof(msix_entry_t));
}

void pciMSIXSetMask(msix_entry_t* msix_base, uint16_t idx, bool mask) {
    msix_base[idx].masked = mask;
}