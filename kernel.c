#define VIDEO_WIDTH 80
#define VIDEO_HEIGHT 25

void main() {
    char* video_memory = (char*) 0xb8000;
    for (int i = 0; i < VIDEO_HEIGHT; i++) {
        for (int j = 0; j < VIDEO_WIDTH*2; j += 2) {
            video_memory[j + i * VIDEO_WIDTH * 2] = ' ';
        }
    }
    video_memory[78 + 12 * VIDEO_WIDTH * 2] = ':';
    video_memory[80 + 12 * VIDEO_WIDTH * 2] = '3';

    video_memory[70 + 13 * VIDEO_WIDTH * 2] = 'U';
    video_memory[72 + 13 * VIDEO_WIDTH * 2] = 'n';
    video_memory[74 + 13 * VIDEO_WIDTH * 2] = 'n';
    video_memory[76 + 13 * VIDEO_WIDTH * 2] = 'a';
    video_memory[78 + 13 * VIDEO_WIDTH * 2] = 'm';
    video_memory[80 + 13 * VIDEO_WIDTH * 2] = 'e';
    video_memory[82 + 13 * VIDEO_WIDTH * 2] = 'd';
    video_memory[84 + 13 * VIDEO_WIDTH * 2] = '-';
    video_memory[86 + 13 * VIDEO_WIDTH * 2] = 'O';
    video_memory[88 + 13 * VIDEO_WIDTH * 2] = 'S';
}