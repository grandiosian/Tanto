#include <kos.h>

int main() {
    vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);
    
    // Pour le texte de debug
    int offset = 640 * BFONT_HEIGHT + BFONT_THIN_WIDTH * 2;
    bfont_set_encoding(BFONT_CODE_ISO8859_1);
    bfont_draw_str(vram_s + offset, 640, 1, "Hello Dreamcast!");
    
    
    while(1) {
    }
}