#pragma once
#include <basic_includes.h>

#define DISP_IOPORT_INDEX 0x01CE
#define DISP_IOPORT_DATA  0x01CF

#define DISP_DISABLED 0x0000
#define DISP_ENABLED  0x0001

#define DISP_INDEX_ID          0x0000
#define DISP_INDEX_XRES        0x0001
#define DISP_INDEX_YRES        0x0002
#define DISP_INDEX_BPP         0x0003
#define DISP_INDEX_ENABLE      0x0004
#define DISP_INDEX_BANK        0x0005
#define DISP_INDEX_VIRT_WIDTH  0x0006
#define DISP_INDEX_VIRT_HEIGHT 0x0007
#define DISP_INDEX_X_OFFSET    0x0008
#define DISP_INDEX_Y_OFFSET    0x0009

#define DISP_BPP_4  0x0004
#define DISP_BPP_8  0x0008
#define DISP_BPP_15 0x000F
#define DISP_BPP_16 0x0010
#define DISP_BPP_24 0x0018
#define DISP_BPP_32 0x0020

int init_graphics(uint16_t width, uint16_t height, uint16_t depth);
void putpixel(uint16_t pos_x, uint16_t pos_y, uint32_t color);