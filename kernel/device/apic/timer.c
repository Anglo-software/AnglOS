#include "timer.h"
#include "apic.h"
#include "device/hpet/hpet.h"
#include "device/io.h"
#include "boot/interrupts/isr.h"

uint64_t ticks_per_second = 0;
uint64_t nanoseconds_per_tick = 0;

static uint64_t time_counter = 0;

static void irq_timer_handler(registers_t* registers) {
    time_counter++;
    apic_send_eoi();
}

void init_timer() {
    register_interrupt_handler(IRQ0, irq_timer_handler);
    write_lapic_reg(APIC_TMRDIV, 0x00);
    write_lapic_reg(APIC_LVT_TMR, IRQ0);

    hpet_start();
    write_lapic_reg(APIC_TMRINITCNT, 0xFFFFFFFF);
    hpet_millisleep(100);
    write_lapic_reg(APIC_LVT_TMR, APIC_LVT_MASK);
    hpet_stop();

    uint32_t ticks = 0xFFFFFFFF - read_lapic_reg(APIC_TMRCURRCNT);
    ticks_per_second = ticks * 10;
    nanoseconds_per_tick = (1000000000 / ticks_per_second) + ((1000000000 % ticks_per_second) != 0);
}

uint64_t get_resolution() {
    return nanoseconds_per_tick;
}

void timer_one_shot(uint64_t time) { // time is in nanoseconds
    if (time < nanoseconds_per_tick) {
        time = nanoseconds_per_tick;
    }

    write_lapic_reg(APIC_TMRINITCNT, time / nanoseconds_per_tick);
    write_lapic_reg(APIC_LVT_TMR, IRQ0);
}

void timer_periodic_start(uint64_t time) { // time is in nanoseconds
    if (time < nanoseconds_per_tick) {
        time = nanoseconds_per_tick;
    }

    write_lapic_reg(APIC_TMRINITCNT, time / nanoseconds_per_tick);
    write_lapic_reg(APIC_LVT_TMR, IRQ0 | TMR_PERIODIC);
}

void timer_periodic_stop() {
    write_lapic_reg(APIC_LVT_TMR, APIC_LVT_MASK);
}

void nanosleep(uint64_t nanos) {
    timer_one_shot(nanos);
    while (!time_counter) {
        __asm__ volatile ("hlt;");
    }
    write_lapic_reg(APIC_LVT_TMR, APIC_LVT_MASK);
    time_counter = 0;
}

void millisleep(uint64_t millis) {
    nanosleep(millis * 1000000);
}