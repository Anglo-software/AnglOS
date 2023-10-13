#pragma once

void init_console();
enum console_mode {NORMAL_MODE = 0, EMERGENCY_MODE = 1};
void console_set_mode(enum console_mode);
void console_print_stats();