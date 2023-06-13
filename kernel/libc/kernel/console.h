#pragma once

void console_init (void);
enum console_mode { NORMAL_MODE = 0, EMERGENCY_MODE };
void console_set_mode (enum console_mode);
void console_print_stats (void);