#pragma once
#include <basic_includes.h>
#include "boot/limine.h"

typedef struct limine_file kmodule_t;

void init_kmodules();
uint64_t num_kmodules();
kmodule_t* get_kmodule(uint64_t num);
kmodule_t* find_by_path(const char* path);
uint8_t kmod_getc(kmodule_t* file, uint64_t index);