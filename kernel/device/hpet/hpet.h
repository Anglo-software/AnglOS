#pragma once
#include "device/acpi/acpi.h"
#include <basic_includes.h>

typedef struct {
    uint8_t address_space_id; // 0 - system memory, 1 - system I/O
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t reserved;
    uint64_t address;
} __attribute__((packed)) hpet_addr_t;

typedef struct {
    acpi_sdt_header_t header;
    uint8_t hardware_rev_id;
    uint8_t comparator_count   : 5;
    uint8_t counter_size       : 1;
    uint8_t reserved           : 1;
    uint8_t legacy_replacement : 1;
    uint16_t pci_vendor_id;
    hpet_addr_t address;
    uint8_t hpet_number;
    uint16_t minimum_tick;
    uint8_t page_protection;
} __attribute__((packed)) hpet_t;

#define HPET_GENERAL_CAPABILITIES_REG   0x000
#define HPET_GENERAL_CONFIG_REG         0x010
#define HPET_INTERRUPT_STATUS_REG       0x020
#define HPET_MAIN_COUNTER_REG           0x0F0
#define hpet_config_reg(n)              (0x100 + 0x20 * n)
#define hpet_compar_reg(n)              (0x108 + 0x20 * n)
#define hpet_fsb_interrupt_route_reg(n) (0x110 + 0x20 * n)

void initHPET();
void hpetStart();
void hpetStop();
void hpetOneShot(uint64_t time);
void hpetPeriodicStart(uint64_t time);
void hpetPeriodicStop();
void hpetNanosleep(uint64_t nanos);
void hpetMillisleep(uint64_t millis);