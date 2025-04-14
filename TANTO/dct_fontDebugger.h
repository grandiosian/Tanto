#ifndef DCT_FONTDEBUGMODE_H
#define DCT_FONTDEBUGMODE_H
#include "kos.h"
//#include <errno.h>
//#include <unistd.h> // pour usleep
#include "dct_datafontDebugger.h"

typedef struct {
    int x, y;
    int width, height;
    int xoffset, yoffset;
    int xadvance;
    int page;
} dct_bitmapChar;


// Structure pour gérer les caractères UTF-8
typedef struct {
    unsigned char bytes[4];  // Un caractère UTF-8 peut faire jusqu'à 4 bytes
    int length;             // Nombre de bytes pour ce caractère
} UTF8Char;

// Structure pour stocker les informations de police
typedef struct {
    uint8_t* bitmap;
    int char_width;
    int char_height;
    int first_char;
    int last_char;
} dct_fontInfo;



typedef struct {
    float x, y;        // Position à l'écran
    float scale;       // Échelle du texte
    uint32_t color;    // Couleur ARGB
} dct_textProperties;



// Structure pour stocker la couleur courante
typedef struct {
    uint32_t stack[32];  // Stack de couleurs
    int depth;           // Profondeur actuelle
} dct_colorStack;


typedef struct {
    pvr_ptr_t texture;
    uint32_t width;      // Largeur totale (puissance de 2)
    uint32_t height;     // Hauteur totale (puissance de 2)
    uint32_t content_width;  // Largeur réelle du contenu
    uint32_t content_height; // Hauteur réelle du contenu
} dct_textureAlpha;

typedef struct {
    char character;
    int offset;
} dct_characterMapping;



uint32_t parse_color_tag(const char* tag);
UTF8Char get_next_utf8_char(const unsigned char* str, int* pos);
int utf8_to_extended_char(UTF8Char utf8);
int find_char_index(int charcode);
dct_fontInfo* init_basic_font();
dct_textureAlpha* create_alpha_texture(uint32_t width, uint32_t height);
dct_textureAlpha* create_text_texture_with_colors(const char* text, dct_fontInfo* font, uint32_t bg_color, int pwidth, int pheight);
void draw_alpha_texture(dct_textureAlpha* tex, float x, float y);
void draw_colored_background();
void update_text_texture(dct_textureAlpha* tex, const char* text, dct_fontInfo* font, uint32_t bg_color);
void dma_callback(void* data);
void free_alpha_texture(dct_textureAlpha* tex);









#endif