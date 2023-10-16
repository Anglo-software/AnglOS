#pragma once

void initConsole();
enum console_mode { NORMAL_MODE = 0, EMERGENCY_MODE = 1 };
void consoleSetMode(enum console_mode);
void consolePrintStats();