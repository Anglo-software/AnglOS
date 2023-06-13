#pragma once
#include <basic_includes.h>


typedef struct {
    char     signature[8];
    uint8_t  checksum;
    char     oemid[6];
    uint8_t  revision;
    uint32_t rsdt_addr;
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t  xchecksum;
    uint8_t  reserved[3];
} __attribute__ ((packed)) rsdp_t;

typedef struct {
  char     signature[4];
  uint32_t length;
  uint8_t  revision;
  uint8_t  checksum;
  char     oemid[6];
  char     oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} acpi_sdt_header_t;

void init_acpi();
acpi_sdt_header_t* acpi_find_sdt(const char* signature);