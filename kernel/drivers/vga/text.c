#include "text.h"
#include "graphics.h"

static uint16_t cursor_x         = 10; // x position of cursor, upper left corner, in pixels
static uint16_t cursor_y         = 16; // y position of cursor, upper left corenr, in pixels
static uint16_t margin_x         = 10; // margin of left and right side for text
static uint16_t margin_y         = 16; // margin of top and bottom side for text
static uint16_t spacing_x        = 2;  // spacing between characters on x axis
static uint16_t spacing_y        = 6;  // spacing between lines
static uint16_t character_size_x = 8;  // Size of a single character in pixels on x axis
static uint16_t character_size_y = 16; // Size of a single character in pixels on y axis