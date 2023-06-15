#pragma once
#include <basic_includes.h>

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

void init_pci();
