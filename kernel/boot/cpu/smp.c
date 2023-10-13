#include "smp.h"
#include "cpu.h"
#include "boot/limine.h"
#include "libc/stdio.h"
#include "device/io.h"
#include <stdatomic.h>

static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0,
    .flags = 0
};

int num_cpus = 1;

void init_smp() {
    if (smp_request.response == NULL) {
        return;
    }
    num_cpus = smp_request.response->cpu_count;

    for (int i = 0; i < num_cpus; i++) {
        cpus[i].cpu_id = smp_request.response->cpus[i]->processor_id;
        cpus[i].lapic_id = smp_request.response->cpus[i]->lapic_id;
    }
    cpus[0].started = 1;
}

void smpStartAP(uint64_t _start_func, uint64_t id) {
    atomic_store((uint64_t*)(&(smp_request.response->cpus[id]->goto_address)), _start_func);
}