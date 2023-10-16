#include "hpet.h"
#include "boot/interrupts/isr.h"
#include "device/apic/apic.h"

extern uint8_t* page_direct_base;

static hpet_t* hpet_table_addr;
static uint64_t hpet_addr;
static uint16_t minimum_tick;
static uint64_t period;
static uint64_t freqency;

static uint64_t hpetReadReg(uint64_t reg)
{
    return *(uint64_t*)(hpet_addr + reg);
}

static void hpetWriteReg(uint64_t reg, uint64_t data)
{
    uint64_t* ptr = (uint64_t*)(hpet_addr + reg);
    *ptr          = data;
}

static uint64_t time_counter = 0;

static void irqHPETHandler(registers_t* registers)
{
    time_counter++;
    apicSendEOI();
}

void initHPET()
{
    hpet_table_addr = (hpet_t*)acpiFindSDT("HPET");
    hpet_addr = hpet_table_addr->address.address + (uint64_t)page_direct_base;
    minimum_tick = hpet_table_addr->minimum_tick;
    period       = hpetReadReg(0x000) >> 32;
    freqency     = 1000000000000000 / period;

    isrRegisterHandler(IRQ2, irqHPETHandler);
    ioapic_redirection_t redir = {.vector = IRQ2};
    ioapicWriteRedir(0x16, &redir);
    hpetWriteReg(hpet_config_reg(0),
                 hpetReadReg(hpet_config_reg(0)) | 0x16 << 9);
}

void hpetStart()
{
    hpetWriteReg(HPET_GENERAL_CONFIG_REG,
                 hpetReadReg(HPET_GENERAL_CONFIG_REG) | 0x1);
}

void hpetStop()
{
    hpetWriteReg(HPET_GENERAL_CONFIG_REG,
                 hpetReadReg(HPET_GENERAL_CONFIG_REG) ^ 0x1);
}

void hpetOneShot(uint64_t time)
{ // time is in femptoseconds
    if (time < period) {
        time = period;
    }

    hpetWriteReg(hpet_config_reg(0),
                 hpetReadReg(hpet_config_reg(0)) | 0x01 << 2);
    hpetWriteReg(hpet_compar_reg(0),
                 hpetReadReg(HPET_MAIN_COUNTER_REG) + time / period);
}

void hpetPeriodicStart(uint64_t time)
{ // time is in femptoseconds
    if (time < period) {
        time = period;
    }

    hpetWriteReg(hpet_config_reg(0), hpetReadReg(hpet_config_reg(0)) |
                                         0x01 << 2 | 0x01 << 3 | 0x01 << 6);
    hpetWriteReg(hpet_compar_reg(0),
                 hpetReadReg(HPET_MAIN_COUNTER_REG) + time / period);
    hpetWriteReg(hpet_compar_reg(0), time / period);
}

void hpetPeriodicStop()
{
    hpetWriteReg(hpet_config_reg(0),
                 hpetReadReg(hpet_config_reg(0)) ^ (0x01 << 2 | 0x01 << 3));
}

void hpetNanosleep(uint64_t nanos)
{
    hpetStart();
    hpetOneShot(nanos * 1000000);
    while (!time_counter) {
        __asm__ volatile("hlt;");
    }
    hpetStop();
    time_counter = 0;
}

void hpetMillisleep(uint64_t millis) { hpetNanosleep(millis * 1000000); }