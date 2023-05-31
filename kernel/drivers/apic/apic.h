#pragma once
#include <basic_includes.h>
#include "drivers/acpi/acpi.h"

typedef struct {
    acpi_sdt_header_t header;
    uint32_t          lapic_addr;
    uint32_t          flags;
} madt_t;

typedef struct {
    uint8_t type;
    uint8_t length;
} madt_entry_header_t;

typedef struct {
    madt_entry_header_t header;
    uint8_t  acpi_processor_id;
    uint8_t  apic_id;
    uint32_t flags;
} madt_entry_type_0_t; // Processor Local APIC

typedef struct {
    madt_entry_header_t header;
    uint8_t  ioapic_id;
    uint8_t  reserved;
    uint32_t ioapic_address;
    uint32_t global_system_interrupt_base;
} madt_entry_type_1_t; // I/O APIC

typedef struct {
    madt_entry_header_t header;
    uint8_t  bus_source;
    uint8_t  irq_source;
    uint32_t global_system_interrupt;
    uint16_t flags;
} madt_entry_type_2_t; // IO/APIC Interrupt Source Override

typedef struct {
    madt_entry_header_t header;
    uint8_t  nmi_source;
    uint8_t  reserved;
    uint16_t flags;
    uint32_t global_system_interrupt;
} madt_entry_type_3_t; // IO/APIC Non-Maskable Interrupt Source

typedef struct {
    madt_entry_header_t header;
    uint8_t  apic_processor_id;
    uint16_t flags;
    uint8_t  lint_number;
} madt_entry_type_4_t; // Local APIC Non-Maskable Interrupts

typedef struct {
    madt_entry_header_t header;
    uint16_t reserved;
    uint64_t lapic_address;
} madt_entry_type_5_t; // Local APIC Address Override

typedef struct {
    madt_entry_header_t header;
    uint16_t reserved;
    uint32_t processor_lx2apic_id;
    uint32_t flags;
    uint32_t acpi_id;
} madt_entry_type_9_t; // Processor Local x2APIC

#define IA32_APIC_BASE_MSR        0x01B
#define IA32_APIC_BASE_MSR_BSP    0x100
#define IA32_APIC_BASE_MSR_ENABLE 0x800
#define IOAPIC_ACTIVE_FLAG 0x2  // 0: high, 1: low
#define IOAPIC_TRIGGER_FLAG 0x8 // 0: edge, 1: level

#define APIC_APICID     0x020
#define APIC_APICVER    0x030
#define APIC_TASKPRIOR  0x080
#define APIC_EOI        0x0B0
#define APIC_LDR        0x0D0
#define APIC_DFR        0x0E0
#define APIC_SPURIOUS   0x0F0
#define APIC_ESR        0x280
#define APIC_ICRL       0x300
#define APIC_ICRH       0x310
#define APIC_LVT_TMR    0x320
#define APIC_LVT_PERF   0x340
#define APIC_LVT_LINT0  0x350
#define APIC_LVT_LINT1  0x360
#define APIC_LVT_ERR    0x370
#define APIC_TMRINITCNT 0x380
#define APIC_TMRCURRCNT 0x390
#define APIC_TMRDIV     0x3E0
#define APIC_LAST       0x38F
#define APIC_DISABLE    0x10000
#define APIC_SW_ENABLE  0x100
#define APIC_CPUFOXUS   0x200
#define APIC_NMI        (4 << 8)
#define TMR_PERIODIC    0x20000
#define TMR_BASEDIV     (1 << 20)

typedef struct {
    uint64_t vector           : 8;
    uint64_t delivery_mode    : 3;
    uint64_t destination_mode : 1;
    uint64_t delivery_status  : 1;
    uint64_t pin_polarity     : 1;
    uint64_t remote_irr       : 1;
    uint64_t trigger_mode     : 1;
    uint64_t mask             : 1;
    uint64_t reserved         : 39;
    uint64_t destination      : 8;
} ioapic_redirection_t;

uint32_t read_lapic_reg(uint32_t reg);
void write_lapic_reg(uint32_t reg, uint32_t data);
uint32_t read_ioapic_reg(uint32_t reg);
void write_ioapic_reg(uint32_t reg, uint32_t data);
void init_apic();
void read_ioapic_redir(uint8_t irq, ioapic_redirection_t* entry);
void write_ioapic_redir(uint8_t irq, ioapic_redirection_t* entry);
void apic_send_eoi();