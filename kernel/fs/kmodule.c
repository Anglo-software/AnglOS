#include "kmodule.h"
#include "libc/string.h"

static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST, .revision = 0};

struct limine_module_response* module_response;

void initKmodules() { module_response = module_request.response; }

uint64_t kmodulesGetNum() { return module_response->module_count; }

kmodule_t* kmoduleGet(uint64_t num)
{
    if (num > kmodulesGetNum()) {
        return NULL;
    }
    return module_response->modules[num];
}

kmodule_t* kmoduleFindByPath(const char* path)
{
    for (unsigned int i = 0; i < kmodulesGetNum(); i++) {
        if (!strcmp(path, module_response->modules[i]->path)) {
            return module_response->modules[i];
        }
    }
    return NULL;
}

uint8_t kmoduleGetc(kmodule_t* file, uint64_t index)
{
    if (file && index < file->size) {
        return ((uint8_t*)(file->address))[index];
    }
    return 0;
}