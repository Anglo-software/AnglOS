#pragma once
#include <basic_includes.h>

typedef struct {
    union {
        struct {
            uint64_t max_queue_entries : 16;
            uint64_t contiguous_queues : 1;
            uint64_t arbitration_mech  : 2;
            uint64_t rev0              : 5;
            uint64_t timeout           : 8;
            uint64_t doorbell_stride   : 4;
            uint64_t subsys_reset_supp : 1;
            uint64_t command_set_supp  : 8;
            uint64_t rev1              : 3;
            uint64_t mem_page_min      : 4;
            uint64_t mem_page_max      : 4;
            uint64_t rev2              : 8;
        } g;

        uint32_t capabilities;
    };
} nvme_capabilities_t;

typedef struct {
    union {
        struct {
            uint32_t ready    : 1;
            uint32_t fatal    : 1;
            uint32_t shutdown : 2;
            uint32_t reset    : 1;
            uint32_t rev0     : 27
        } g;

        uint32_t status;
    };
} nvme_status_t;

typedef struct {
    union {
        struct {
            uint32_t enable           : 1;
            uint32_t rev0             : 3;
            uint32_t command_set      : 3;
            uint32_t mem_page_size    : 4;
            uint32_t arbitration_mech : 3;
            uint32_t shutdown         : 2;
            uint32_t sub_entry_size   : 4;
            uint32_t com_entry_size   : 4;
            uint32_t rev1             : 8;
        } g;

        uint32_t config;
    };
} nvme_config_t;

void init_nvme();
uint32_t nvmeReadStatus();
uint32_t nvmeReadConfig();
void nvmeWriteConfig(uint32_t config);