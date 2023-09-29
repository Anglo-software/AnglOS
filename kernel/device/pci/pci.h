#pragma once
#include <basic_includes.h>
#include "device/acpi/acpi.h"

typedef struct {
    acpi_sdt_header_t header;
    uint64_t rsv1;
    uint64_t addr;
    uint16_t segment_group;
    uint8_t start_bus;
    uint8_t end_bus;
    uint32_t rsv2;
} __attribute__((packed)) mcfg_t;

typedef struct {
    union {
        struct {
            uint16_t io_space        : 1;
            uint16_t mem_space       : 1;
            uint16_t bus_master      : 1;
            uint16_t spec_cycles     : 1;
            uint16_t mem_wr_inv_en   : 1;
            uint16_t vga_pal_snoop   : 1;
            uint16_t parity_err_resp : 1;
            uint16_t rev0            : 1;
            uint16_t serr_en         : 1;
            uint16_t fbtb_en         : 1;
            uint16_t int_disable     : 1;
            uint16_t rev1            : 5;
        } g;
        uint32_t command;
    };
} pci_command_t;

typedef struct {
    union {
        struct {
            uint16_t rev0            : 3;
            uint16_t int_status      : 1;
            uint16_t cap_list        : 1;
            uint16_t mhz66_cap       : 1;
            uint16_t rev1            : 1;
            uint16_t fbtb_cap        : 1;
            uint16_t mast_parity_err : 1;
            uint16_t devsel_timing   : 2;
            uint16_t sig_targ_abort  : 1;
            uint16_t rec_targ_abort  : 1;
            uint16_t rec_mast_abort  : 1;
            uint16_t sig_sys_err     : 1;
            uint16_t det_parity_err  : 1;
        } g;
        uint32_t status;
    };
} pci_status_t;

typedef struct {
    uint32_t message_addr_lo;
    uint32_t message_addr_hi;
    uint32_t message_data;
    uint32_t masked : 1;
    uint32_t rev0   : 31;
} __attribute__((packed)) msix_entry_t;

#define PCI_CLASS_UNCLASSIFIED              0
#define PCI_SUBCLASS_NON_VGA_COMPATIBLE     0
#define PCI_SUBCLASS_VGA_COMPATIBLE         1

#define PCI_CLASS_MASS_STORAGE              1
#define PCI_SUBCLASS_SCSI                   0
#define PCI_SUBCLASS_IDE                    1
#define PCI_SUBCLASS_FLOPPY                 2
#define PCI_SUBCLASS_IPI                    3
#define PCI_SUBCLASS_RAID                   4
#define PCI_SUBCLASS_ATA                    5
#define PCI_SUBCLASS_SATA                   6
#define PCI_SUBCLASS_SASCSI                 7
#define PCI_SUBCLASS_NVM                    8
#define PCI_SUBCLASS_OTHER                  128

#define PCI_CLASS_NETWORK                   2
#define PCI_SUBCLASS_ETHERNET               0
#define PCI_SUBCLASS_TOKEN                  1
#define PCI_SUBCLASS_FDDI                   2
#define PCI_SUBCLASS_ATM                    3
#define PCI_SUBCLASS_ISDN                   4
#define PCI_SUBCLASS_OTHER                  128

#define PCI_CLASS_DISPLAY                   3
#define PCI_SUBCLASS_VGA                    0
#define PCI_SUBCLASS_XDA                    1
#define PCI_SUBCLASS_3D                     2
#define PCI_SUBCLASS_OTHER                  128

#define PCI_CLASS_MULTIMEDIA                4

#define PCI_CLASS_MEMORY                    5

#define PCI_CLASS_BRIDGE                    6
#define PCI_SUBCLASS_HOST                   0
#define PCI_SUBCLASS_ISA                    1
#define PCI_SUBCLASS_EISA                   2
#define PCI_SUBCLASS_MCA                    3
#define PCI_SUBCLASS_PCI                    4
#define PCI_SUBCLASS_PCMCIA                 5
#define PCI_SUBCLASS_NUBUS                  6
#define PCI_SUBCLASS_CARDBUS                7
#define PCI_SUBCLASS_RACEWAY                8
#define PCI_SUBCLASS_OTHER                  128

#define PCI_CLASS_COMM                      7

#define PCI_CLASS_BASE_PERIPHERAL           8
#define PCI_SUBCLASS_PIC                    0
#define PCI_SUBCLASS_DMA                    1
#define PCI_SUBCLASS_TIMER                  2
#define PCI_SUBCLASS_RTC                    3
#define PCI_SUBCLASS_HOTPLUG                128

#define PCI_CLASS_INPUT                     9

#define PCI_CLASS_DOCK                      10

#define PCI_CLASS_PROCESSOR                 11
#define PCI_SUBCLASS_386                    0
#define PCI_SUBCLASS_486                    1
#define PCI_SUBCLASS_PENTIUM                2
#define PCI_SUBCLASS_ALPHA                  16
#define PCI_SUBCLASS_PPC                    32
#define PCI_SUBCLASS_MIPS                   48
#define PCI_SUBCLASS_COPROC                 64

#define PCI_CLASS_SERIAL_BUS                12
#define PCI_SUBCLASS_FIREWIRE               0
#define PCI_SUBCLASS_ACCESS                 1
#define PCI_SUBCLASS_SSA                    2
#define PCI_SUBCLASS_USB                    3
#define PCI_SUBCLASS_FIBER                  4

#define PCI_USB_IFACE_UHCI                  0
#define PCI_USB_IFACE_OHCI                  16
#define PCI_USB_IFACE_EHCI                  32

#define PCI_CLASS_UNDEF                     255

#define PCI_CONFIG_ADDR                     0xCF8
#define PCI_CONFIG_DATA                     0xCFC

void init_pci();
uint8_t pciConfigReadByte(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
uint16_t pciConfigReadWord(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
uint32_t pciConfigReadLong(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
void pciConfigWriteLong(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t data);
uint8_t pciGetBaseClass(uint8_t bus, uint8_t device, uint8_t func);
uint8_t pciGetSubClass(uint8_t bus, uint8_t device, uint8_t func);
uint16_t pciReadStatus(uint8_t bus, uint8_t device, uint8_t func);
uint16_t pciReadCommand(uint8_t bus, uint8_t device, uint8_t func);
void pciWriteCommand(uint8_t bus, uint8_t device, uint8_t func, uint16_t command);
uint32_t pciGetBAR(uint8_t bus, uint8_t device, uint8_t func, uint8_t bar);
uint8_t pciGetCapPointer(uint8_t bus, uint8_t device, uint8_t func);
uint8_t pciGetCapabilityOffset(uint8_t bus, uint8_t device, uint8_t func, uint8_t cap);
uint64_t pciGetMessageTableBaseAddr(uint8_t bus, uint8_t device, uint8_t func);
uint32_t pciGetMessageTableOffset(uint8_t bus, uint8_t device, uint8_t func);
uint32_t pciGetMessageTableSize(uint8_t bus, uint8_t device, uint8_t func);
uint8_t pciEnableMSIX(uint8_t bus, uint8_t device, uint8_t func);
void pciWriteMSIXEntry(msix_entry_t* msix_base, uint16_t idx, msix_entry_t* entry);
void pciReadMSIXEntry(msix_entry_t* msix_base, uint16_t idx, msix_entry_t* entry);
void pciMSIXSetMask(msix_entry_t* msix_base, uint16_t idx, bool mask);