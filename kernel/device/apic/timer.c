#include "timer.h"
#include "apic.h"
#include "boot/interrupts/isr.h"
#include "device/hpet/hpet.h"
#include "device/input/input.h"
#include "device/io.h"

uint64_t ticks_per_second     = 0;
uint64_t nanoseconds_per_tick = 0;

static uint64_t time_counter  = 0;

extern bool cursor_enabled;

static void irqAPICTimerHandler() {
    time_counter++;
    if (lapicReadReg(APIC_APICID) >> 24 == 0) {
        if (cursor_enabled) {
            if (!inputFull()) {
                inputPutc(0x91);
            }
        }
        else {
            if (!inputFull()) {
                inputPutc(0x92);
            }
        }
    }
    apicSendEOI();
}

void initAPICTimer() {
    irqRegisterHandler(IRQ0, (void*)irqAPICTimerHandler);
    lapicWriteReg(APIC_TMRDIV, 0x00);
    lapicWriteReg(APIC_LVT_TMR, IRQ0);

    hpetStart();
    lapicWriteReg(APIC_TMRINITCNT, 0xFFFFFFFF);
    hpetMillisleep(100);
    lapicWriteReg(APIC_LVT_TMR, APIC_LVT_MASK);
    hpetStop();

    uint32_t ticks       = 0xFFFFFFFF - lapicReadReg(APIC_TMRCURRCNT);
    ticks_per_second     = ticks * 10;
    nanoseconds_per_tick = (1000000000 / ticks_per_second) +
                           ((1000000000 % ticks_per_second) != 0);
}

void initAPAPICTimer() {
    lapicWriteReg(APIC_TMRDIV, 0x00);
    lapicWriteReg(APIC_LVT_TMR, IRQ0);
}

uint64_t apicTimerGetResolution() { return nanoseconds_per_tick; }

void apicTimerOneShot(uint64_t time) { // time is in nanoseconds
    if (time < nanoseconds_per_tick) {
        time = nanoseconds_per_tick;
    }

    lapicWriteReg(APIC_LVT_TMR, IRQ0);
    lapicWriteReg(APIC_TMRINITCNT, time / nanoseconds_per_tick);
}

void apicTimerPeriodicStart(uint64_t time) { // time is in nanoseconds
    if (time < nanoseconds_per_tick) {
        time = nanoseconds_per_tick;
    }

    lapicWriteReg(APIC_LVT_TMR, IRQ0 | TMR_PERIODIC);
    lapicWriteReg(APIC_TMRINITCNT, time / nanoseconds_per_tick);
}

void apicTimerPeriodicStop() { lapicWriteReg(APIC_LVT_TMR, APIC_LVT_MASK); }

void apicNanosleep(uint64_t nanos) {
    apicTimerOneShot(nanos);
    while (!time_counter) {
        __asm__ volatile("hlt;");
    }
    lapicWriteReg(APIC_LVT_TMR, APIC_LVT_MASK);
    time_counter = 0;
}

void apicMillisleep(uint64_t millis) { apicNanosleep(millis * 1000000UL); }