#include "kmodule.h"
#include "libc/string.h"

static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};

struct limine_module_response* module_response;

void init_kmodules() {
    module_response = module_request.response;
}

uint64_t num_kmodules() {
    return module_response->module_count;
}

kmodule_t* get_kmodule(uint64_t num) {
    if (num > num_kmodules()) {
        return NULL;
    }
    return module_response->modules[num];
}

kmodule_t* find_by_path(const char* path) {
    for (int i = 0; i < num_kmodules(); i++) {
        if (!strcmp(path, module_response->modules[i]->path)) {
            return module_response->modules[i];
        }
    }
    return NULL;
}

uint8_t kmod_getc(kmodule_t* file, uint64_t index) {
    if (file && index < file->size) {
        return ((uint8_t*)(file->address))[index];
    }
    return 0;
}