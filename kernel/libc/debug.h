#pragma once
#include <basic_includes.h>

#define UNUSED                     __attribute__((unused))
#define NO_RETURN                  __attribute__((noreturn))
#define NO_INLINE                  __attribute__((noinline))
#define kprintf_FORMAT(FMT, FIRST) __attribute__((format(printf, FMT, FIRST)))
#define NOT_REACHED()              for (;;)