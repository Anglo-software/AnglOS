#pragma once
#include <basic_includes.h>

typedef struct {
    uint64_t capabilities;
    uint32_t version;
    uint32_t int_mask_set;
    uint32_t int_mask_clear;
    uint32_t config;
    uint32_t rev0;
    uint32_t status;
    uint32_t reset;
    uint32_t admin_attr;
    uint64_t admin_sub_base;
    uint64_t admin_com_base;
    uint32_t cmb_addr;
    uint32_t cmb_size;
    uint32_t bp_info;
    uint32_t bp_read_select;
    uint64_t bp_addr;
    uint64_t cmb_control;
    uint32_t cmb_status;
    uint32_t rev1;
    uint64_t rev2[436];
    uint32_t persist_cap;
    uint32_t persist_control;
    uint32_t persist_status;
    uint32_t persist_size;
    uint32_t persist_through;
    uint32_t persist_lower;
    uint32_t persist_upper;
    uint32_t set_specific[121];
    uint32_t doorbell_base;
} __attribute__((packed)) nvme_registers_t;

typedef struct {
    uint16_t vendor;
    uint16_t subsys_vendor;
    uint32_t serial[5];
    uint64_t model[5];
    uint64_t revision;
    uint8_t arbitration_burst;
    uint8_t ieee_id[3];
    uint8_t multipath_ns_sharing;
    uint8_t max_transfer_size;
    uint16_t controller_id;
    uint32_t version;
    uint32_t rtd3_resume_latency;
    uint32_t rtd3_entry_latency;
    uint32_t async_event_support;
    uint32_t controller_attr;
    uint16_t read_recovery_levels;
    uint8_t rev0[9];
    uint8_t controller_type;
    uint64_t fguid[2];
    uint16_t retry_delay1;
    uint16_t retry_delay2;
    uint16_t retry_delay3;
    uint8_t rev1[106];
    uint8_t idkwhatthisis[26];

} __attribute__((packed)) nvme_identify_t;

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
        uint64_t capabilities;
    };
} nvme_capabilities_t;

typedef struct {
    union {
        struct {
            uint32_t ready    : 1;
            uint32_t fatal    : 1;
            uint32_t shutdown : 2;
            uint32_t reset    : 1;
            uint32_t rev0     : 27;
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

typedef struct {
    union {
        struct {
            uint32_t opcode  : 8;
            uint32_t fused   : 2;
            uint32_t rev0    : 4;
            uint32_t prp_sgl : 2;
            uint32_t id      : 16;
        } c;
        uint32_t command;
    };
    uint32_t nsid;
    uint64_t rev0;
    uint64_t meta;
    uint64_t prp0;
    uint64_t prp1;
    uint32_t com_spec0;
    uint32_t com_spec1;
    uint32_t com_spec2;
    uint32_t com_spec3;
    uint32_t com_spec4;
    uint32_t com_spec5;
} __attribute__((packed)) nvme_submission_t;

typedef struct {
    uint64_t com_spec : 32;
    uint64_t rev0     : 32;
    uint64_t sub_head : 16;
    uint64_t sub_id   : 16;
    uint64_t com_id   : 16;
    uint64_t phase    : 1;
    uint64_t status   : 15;
} __attribute__((packed)) nvme_completion_t;

#define NVME_CREATE_SUBMISSION_QUEUE 0x01
#define NVME_CREATE_COMPLETION_QUEUE 0x05
#define NVME_IDENTIFY                0x06
#define NVME_READ                    0x02
#define NVME_WRITE                   0x01

void init_nvme();
uint32_t nvmeReadStatus();
uint32_t nvmeReadConfig();
void nvmeWriteConfig(uint32_t config);
void nvmeCreateSubQueue(void* paddr, uint16_t id, uint16_t size, uint16_t flags, uint16_t com_id);
void nvmeCreateComQueue(void* paddr, uint16_t id, uint16_t size, uint16_t flags, uint16_t int_vector);
void nvmeIdentifyNamespace(void* paddr, uint32_t nsid);
void nvmeIdentifyController(void* paddr);
void nvmeIdentifyNamespaceList(void* paddr);