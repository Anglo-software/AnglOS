#pragma once
#include "boot/limine.h"
#include <basic_includes.h>

typedef struct limine_file kmodule_t;

void initKmodules();
uint64_t kmodulesGetNum();
kmodule_t* kmoduleGet(uint64_t num);
kmodule_t* kmoduleFindByPath(const char* path);
uint8_t kmoduleGetc(kmodule_t* file, uint64_t index);