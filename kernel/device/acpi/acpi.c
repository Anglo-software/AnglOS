#include "acpi.h"
#include "boot/limine.h"
#include "libc/string.h"

extern void* page_direct_base;

static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST, .revision = 0};

static rsdp_t* rsdp_addr;
static acpi_sdt_header_t* xsdt_addr;
static uint32_t num_xsdt_entries;

void initACPI()
{
    rsdp_addr = rsdp_request.response->address;
    xsdt_addr =
        (acpi_sdt_header_t*)((uint64_t)page_direct_base + rsdp_addr->xsdt_addr);
    num_xsdt_entries = (xsdt_addr->length - sizeof(acpi_sdt_header_t)) / 8;
}

acpi_sdt_header_t* acpiFindSDT(const char* signature)
{
    for (unsigned int i = 0; i < num_xsdt_entries; i++) {
        uint64_t* tmp        = (uint64_t*)((uint64_t)xsdt_addr +
                                    sizeof(acpi_sdt_header_t) + 8 * i);
        acpi_sdt_header_t* h = *tmp + page_direct_base;
        if (!strncmp(signature, h->signature, 4)) {
            return h;
        }
    }

    return NULL;
}