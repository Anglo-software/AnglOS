#include "graphics.h"
#include "boot/limine.h"
#include "drivers/io.h"
#include "text.h"

static volatile struct limine_framebuffer_request fb_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static uint8_t* fb_base = 0;
static uint64_t fb_w;   // Width
static uint64_t fb_h;   // Height
static uint64_t fb_p;   // Pitch
static uint64_t fb_bpp; // Bytes per pixel

static void write_reg(uint16_t index, uint16_t data) {
    outw(0x01CE, index);
    outw(0x01CF, data);
}

static uint16_t read_reg(uint16_t index) {
    outw(0x01CE, index);
    uint16_t in = inw(0x01CF);
    return in;
}

static int change_resolution(uint16_t width, uint16_t height) {
    write_reg(DISP_INDEX_ENABLE, DISP_DISABLED);
    write_reg(DISP_INDEX_XRES, width);
    write_reg(DISP_INDEX_YRES, height);
    write_reg(DISP_INDEX_ENABLE, DISP_ENABLED);

    return read_reg(DISP_INDEX_XRES) != width || read_reg(DISP_INDEX_YRES) != height;
}

static int change_bit_depth(uint16_t depth) {
    write_reg(DISP_INDEX_ENABLE, DISP_DISABLED);
    write_reg(DISP_INDEX_BPP, depth);
    write_reg(DISP_INDEX_ENABLE, DISP_ENABLED);

    return read_reg(DISP_INDEX_BPP) != depth;
}

int init_graphics(uint16_t width, uint16_t height, uint16_t depth) {
    if (fb_request.response->framebuffer_count == 0) {
        return 1;
    }

    fb_base = fb_request.response->framebuffers[0]->address;

    if (change_resolution(width, height)) {
        return 1;
    }

    if (change_bit_depth(depth)) {
        return 1;
    }

    fb_w = width;
    fb_h = height;
    fb_bpp = depth / 8;
    fb_p = width * fb_bpp;

    move_cursor(4, 4);

    return 0;
}

uint64_t get_res_x() {
    return fb_w;
}

uint64_t get_res_y() {
    return fb_h;
}

uint64_t get_pitch() {
    return fb_p;
}

uint64_t get_bytes_per_pixel() {
    return fb_bpp;
}

uint8_t* get_fb_base() {
    return fb_base;
}

void putpixel(uint16_t x, uint16_t y, uint32_t color) {
    unsigned where = x * fb_bpp + y * fb_p;
    fb_base[where]     = (color >> 0x00) & 0xFF;
    fb_base[where + 1] = (color >> 0x08) & 0xFF;
    fb_base[where + 2] = (color >> 0x10) & 0xFF;
}