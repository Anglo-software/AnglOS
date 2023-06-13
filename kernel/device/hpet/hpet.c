#include "hpet.h"
#include "boot/interrupts/isr.h"
#include "device/apic/apic.h"

extern uint8_t* page_direct_base;

static hpet_t* hpet_table_addr;
static uint64_t hpet_addr;
static uint16_t minimum_tick;
static uint64_t period;
static uint64_t freqency;

static uint64_t read_hpet_reg(uint64_t reg) {
    return *(uint64_t*)(hpet_addr + reg);
}

static void write_hpet_reg(uint64_t reg, uint64_t data) {
    uint64_t* ptr = (uint64_t*)(hpet_addr + reg);
    *ptr = data;
}

static uint64_t time_counter = 0;

static void irq_hpet_handler(registers_t* registers) {
    time_counter++;
    apic_send_eoi();
}

void init_hpet() {
    hpet_table_addr = acpi_find_sdt("HPET");
    hpet_addr = hpet_table_addr->address.address + (uint64_t)page_direct_base;
    minimum_tick = hpet_table_addr->minimum_tick;
    period = read_hpet_reg(0x000) >> 32;
    freqency = 1000000000000000 / period;

    register_interrupt_handler(IRQ2, irq_hpet_handler);
    ioapic_redirection_t redir = {.vector = IRQ2};
    write_ioapic_redir(0x16, &redir);
    write_hpet_reg(hpet_config_reg(0), read_hpet_reg(hpet_config_reg(0)) | 0x16 << 9);
}

void hpet_start() {
    write_hpet_reg(HPET_GENERAL_CONFIG_REG, read_hpet_reg(HPET_GENERAL_CONFIG_REG) | 0x1);
}

void hpet_stop() {
    write_hpet_reg(HPET_GENERAL_CONFIG_REG, read_hpet_reg(HPET_GENERAL_CONFIG_REG) ^ 0x1);
}

void hpet_one_shot(uint64_t time) { // time is in femptoseconds
    if (time < period) {
        time = period;
    }

    write_hpet_reg(hpet_config_reg(0), read_hpet_reg(hpet_config_reg(0)) | 0x01 << 2);
    write_hpet_reg(hpet_compar_reg(0), read_hpet_reg(HPET_MAIN_COUNTER_REG) + time / period);
}

void hpet_periodic_start(uint64_t time) { // time is in femptoseconds
    if (time < period) {
        time = period;
    }

    write_hpet_reg(hpet_config_reg(0), read_hpet_reg(hpet_config_reg(0)) | 0x01 << 2 | 0x01 << 3 | 0x01 << 6);
    write_hpet_reg(hpet_compar_reg(0), read_hpet_reg(HPET_MAIN_COUNTER_REG) + time / period);
    write_hpet_reg(hpet_compar_reg(0), time / period);
}

void hpet_periodic_stop() {
    write_hpet_reg(hpet_config_reg(0), read_hpet_reg(hpet_config_reg(0)) ^ (0x01 << 2 | 0x01 << 3));
}

void hpet_nanosleep(uint64_t nanos) {
    hpet_start();
    hpet_one_shot(nanos * 1000000);
    while (!time_counter) {
        __asm__ volatile ("hlt;");
    }
    hpet_stop();
    time_counter = 0;
}

void hpet_millisleep(uint64_t millis) {
    hpet_nanosleep(millis * 1000000);
}