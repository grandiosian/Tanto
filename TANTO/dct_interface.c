// dct_interface.c
#include "dct_interface.h"

typedef struct {
    kmg_header_t header;    // En-tête KMG
    uint16_t* texture_data; // Données de texture
} TextureKMG;

TextureKMG* load_kmg_texture(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    
    TextureKMG* tex = malloc(sizeof(TextureKMG));
    
    // Lire l'en-tête KMG
    if (fread(&tex->header, sizeof(kmg_header_t), 1, f) != 1) {
        free(tex);
        fclose(f);
        return NULL;
    }
    
    // Vérifier la signature KMG
    if (tex->header.magic != KMG_MAGIC) {
        free(tex);
        fclose(f);
        return NULL;
    }
    
    // Allouer et charger les données de texture
    size_t data_size = tex->header.width * tex->header.height * 2; // Pour ARGB4444
    tex->texture_data = malloc(data_size);
    
    if (fread(tex->texture_data, 1, data_size, f) != data_size) {
        free(tex->texture_data);
        free(tex);
        fclose(f);
        return NULL;
    }
    
    fclose(f);
    return tex;
}



const unsigned char FONT_COMPLETE[] = 
{
    // Espace (32)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ! (33)
    0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00,
    // " (34)
    0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // # (35)
    0x28, 0x28, 0x7C, 0x28, 0x7C, 0x28, 0x28, 0x00,
    // $ (36)
    0x38, 0x54, 0x50, 0x38, 0x14, 0x54, 0x38, 0x00,
    // % (37)
    0x60, 0x64, 0x08, 0x10, 0x20, 0x4C, 0x0C, 0x00,
    // & (38)
    0x20, 0x50, 0x50, 0x20, 0x54, 0x48, 0x34, 0x00,
    // ' (39)
    0x30, 0x30, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00,
    // ( (40)
    0x08, 0x10, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00,
    // ) (41)
    0x20, 0x10, 0x08, 0x08, 0x08, 0x10, 0x20, 0x00,
    // * (42)
    0x00, 0x28, 0x10, 0x7C, 0x10, 0x28, 0x00, 0x00,
    // + (43)
    0x00, 0x10, 0x10, 0x7C, 0x10, 0x10, 0x00, 0x00,
    // , (44)
    0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x20, 0x40,
    // - (45)
    0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00,
    // . (46)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00,
    // / (47)
    0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00,
    // 0 (48)
    0x38, 0x44, 0x4C, 0x54, 0x64, 0x44, 0x38, 0x00,
    // 1 (49)
    0x10, 0x30, 0x50, 0x10, 0x10, 0x10, 0x7C, 0x00,
    // 2 (50)
    0x38, 0x44, 0x04, 0x08, 0x10, 0x20, 0x7C, 0x00,
    // 3 (51)
    0x38, 0x44, 0x04, 0x18, 0x04, 0x44, 0x38, 0x00,
    // 4 (52)
    0x08, 0x18, 0x28, 0x48, 0x7C, 0x08, 0x08, 0x00,
    // 5 (53)
    0x7C, 0x40, 0x78, 0x04, 0x04, 0x44, 0x38, 0x00,
    // 6 (54)
    0x38, 0x44, 0x40, 0x78, 0x44, 0x44, 0x38, 0x00,
    // 7 (55)
    0x7C, 0x04, 0x08, 0x10, 0x20, 0x20, 0x20, 0x00,
    // 8 (56)
    0x38, 0x44, 0x44, 0x38, 0x44, 0x44, 0x38, 0x00,
    // 9 (57)
    0x38, 0x44, 0x44, 0x3C, 0x04, 0x44, 0x38, 0x00,
    // : (58)
    0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x00, 0x00,
    // ; (59)
    0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x20, 0x40,
    // < (60)
    0x08, 0x10, 0x20, 0x40, 0x20, 0x10, 0x08, 0x00,
    // = (61)
    0x00, 0x00, 0x7C, 0x00, 0x7C, 0x00, 0x00, 0x00,
    // > (62)
    0x40, 0x20, 0x10, 0x08, 0x10, 0x20, 0x40, 0x00,
    // ? (63)
    0x38, 0x44, 0x04, 0x08, 0x10, 0x00, 0x10, 0x00,
    // @ (64)
    0x38, 0x44, 0x4C, 0x54, 0x5C, 0x40, 0x38, 0x00,
    // A (65)
    0x38, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x44, 0x00,
    // B (66)
    0x78, 0x44, 0x44, 0x78, 0x44, 0x44, 0x78, 0x00,
    // C (67)
    0x38, 0x44, 0x40, 0x40, 0x40, 0x44, 0x38, 0x00,
    // D (68)
    0x78, 0x44, 0x44, 0x44, 0x44, 0x44, 0x78, 0x00,
    // E (69)
    0x7C, 0x40, 0x40, 0x78, 0x40, 0x40, 0x7C, 0x00,
    // F (70)
    0x7C, 0x40, 0x40, 0x78, 0x40, 0x40, 0x40, 0x00,
    // G (71)
    0x38, 0x44, 0x40, 0x5C, 0x44, 0x44, 0x38, 0x00,
    // H (72)
    0x44, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x44, 0x00,
    // I (73)
    0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00,
    // J (74)
    0x04, 0x04, 0x04, 0x04, 0x44, 0x44, 0x38, 0x00,
    // K (75)
    0x44, 0x48, 0x50, 0x60, 0x50, 0x48, 0x44, 0x00,
    // L (76)
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7C, 0x00,
    // M (77)
    0x44, 0x6C, 0x54, 0x54, 0x44, 0x44, 0x44, 0x00,
    // N (78)
    0x44, 0x64, 0x54, 0x4C, 0x44, 0x44, 0x44, 0x00,
    // O (79)
    0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00,
    // P (80)
    0x78, 0x44, 0x44, 0x78, 0x40, 0x40, 0x40, 0x00,
    // Q (81)
    0x38, 0x44, 0x44, 0x44, 0x54, 0x48, 0x34, 0x00,
    // R (82)
    0x78, 0x44, 0x44, 0x78, 0x50, 0x48, 0x44, 0x00,
    // S (83)
    0x38, 0x44, 0x40, 0x38, 0x04, 0x44, 0x38, 0x00,
    // T (84)
    0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00,
    // U (85)
    0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00,
    // V (86)
    0x44, 0x44, 0x44, 0x44, 0x44, 0x28, 0x10, 0x00,
    // W (87)
    0x44, 0x44, 0x44, 0x54, 0x54, 0x6C, 0x44, 0x00,
    // X (88)
    0x44, 0x44, 0x28, 0x10, 0x28, 0x44, 0x44, 0x00,
    // Y (89)
    0x44, 0x44, 0x28, 0x10, 0x10, 0x10, 0x10, 0x00,
    // Z (90)
    0x7C, 0x04, 0x08, 0x10, 0x20, 0x40, 0x7C, 0x00,
    // [ (91)
    0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00,
    // \ (92)
    0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00,
    // ] (93)
    0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00,
    // ^ (94)
    0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00,
    // _ (95)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00,
    // ` (96)
    0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // a (97)
    0x00, 0x00, 0x38, 0x04, 0x3C, 0x44, 0x3C, 0x00,
    // b (98)
    0x40, 0x40, 0x78, 0x44, 0x44, 0x44, 0x78, 0x00,
    // c (99)
    0x00, 0x00, 0x38, 0x44, 0x40, 0x44, 0x38, 0x00,
    // d (100)
    0x04, 0x04, 0x3C, 0x44, 0x44, 0x44, 0x3C, 0x00,
    // e (101)
    0x00, 0x00, 0x38, 0x44, 0x7C, 0x40, 0x38, 0x00,
    // f (102)
    0x0C, 0x10, 0x38, 0x10, 0x10, 0x10, 0x10, 0x00,
    // g (103)
    0x00, 0x00, 0x3C, 0x44, 0x44, 0x3C, 0x04, 0x38,
    // h (104)
    0x40, 0x40, 0x78, 0x44, 0x44, 0x44, 0x44, 0x00,
    // i (105)
    0x10, 0x00, 0x30, 0x10, 0x10, 0x10, 0x38, 0x00,
    // j (106)
    0x04, 0x00, 0x04, 0x04, 0x04, 0x44, 0x38, 0x00,
    // k (107)
    0x40, 0x40, 0x48, 0x50, 0x60, 0x50, 0x48, 0x00,
    // l (108)
    0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00,
    // m (109)
    0x00, 0x00, 0x68, 0x54, 0x54, 0x44, 0x44, 0x00,
    // n (110)
    0x00, 0x00, 0x78, 0x44, 0x44, 0x44, 0x44, 0x00,
    // o (111)
    0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00,
    // p (112)
    0x00, 0x00, 0x78, 0x44, 0x44, 0x78, 0x40, 0x40,
    // q (113) suite
    0x00, 0x00, 0x3C, 0x44, 0x44, 0x3C, 0x04, 0x04,
    // r (114)
    0x00, 0x00, 0x58, 0x64, 0x40, 0x40, 0x40, 0x00,
    // s (115)
    0x00, 0x00, 0x38, 0x40, 0x38, 0x04, 0x78, 0x00,
    // t (116)
    0x10, 0x10, 0x38, 0x10, 0x10, 0x10, 0x0C, 0x00,
    // u (117)
    0x00, 0x00, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00,
    // v (118)
    0x00, 0x00, 0x44, 0x44, 0x44, 0x28, 0x10, 0x00,
    // w (119)
    0x00, 0x00, 0x44, 0x44, 0x54, 0x54, 0x28, 0x00,
    // x (120)
    0x00, 0x00, 0x44, 0x28, 0x10, 0x28, 0x44, 0x00,
    // y (121)
    0x00, 0x00, 0x44, 0x44, 0x44, 0x3C, 0x04, 0x38,
    // z (122)
    0x00, 0x00, 0x7C, 0x08, 0x10, 0x20, 0x7C, 0x00,
    // { (123)
    0x08, 0x10, 0x10, 0x20, 0x10, 0x10, 0x08, 0x00,
    // | (124)
    0x10, 0x10, 0x10, 0x00, 0x10, 0x10, 0x10, 0x00,
    // } (125)
    0x20, 0x10, 0x10, 0x08, 0x10, 0x10, 0x20, 0x00,
    // ~ (126)
    0x20, 0x54, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
    // é (233)
    0x10, 0x20, 0x38, 0x44, 0x7C, 0x40, 0x38, 0x00,
    // è (232)
    0x20, 0x10, 0x38, 0x44, 0x7C, 0x40, 0x38, 0x00,
    // ê (234)
    0x10, 0x28, 0x38, 0x44, 0x7C, 0x40, 0x38, 0x00,
    // à (224)
    0x20, 0x10, 0x38, 0x04, 0x3C, 0x44, 0x3C, 0x00,
    // â (226)
    0x10, 0x28, 0x38, 0x04, 0x3C, 0x44, 0x3C, 0x00,
    // û (251)
    0x10, 0x28, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00,
    // ù (249)
    0x20, 0x10, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00,
    // î (238)
    0x10, 0x28, 0x00, 0x30, 0x10, 0x10, 0x38, 0x00,
    // ï (239)
    0x28, 0x00, 0x30, 0x10, 0x10, 0x10, 0x38, 0x00,
    // ç (231)
    0x00, 0x38, 0x44, 0x40, 0x44, 0x38, 0x10, 0x20


};




const CharMapEntry CHAR_MAP_COMPLETE[] = 
{   
    {' ', 0},    // index 0 (32 * 8)
    {'!', 8},    // index 1
    {'"', 16},   // index 2
    {'#', 24},
    {'$', 32},
    {'%', 40},
    {'&', 48},
    {'\'', 56},
    {'(', 64},
    {')', 72},
    {'*', 80},
    {'+', 88},
    {',', 96},
    {'-', 104},
    {'.', 112},
    {'/', 120},
    {'0', 128},
    {'1', 136},
    {'2', 144},
    {'3', 152},
    {'4', 160},
    {'5', 168},
    {'6', 176},
    {'7', 184},
    {'8', 192},
    {'9', 200},
    {':', 208},
    {';', 216},
    {'<', 224},
    {'=', 232},
    {'>', 240},
    {'?', 248},
    {'@', 256},
    {'A', 264},
    {'B', 272},
    {'C', 280},
    {'D', 288},
    {'E', 296},
    {'F', 304},
    {'G', 312},
    {'H', 320},
    {'I', 328},
    {'J', 336},
    {'K', 344},
    {'L', 352},
    {'M', 360},
    {'N', 368},
    {'O', 376},
    {'P', 384},
    {'Q', 392},
    {'R', 400},
    {'S', 408},
    {'T', 416},
    {'U', 424},
    {'V', 432},
    {'W', 440},
    {'X', 448},
    {'Y', 456},
    {'Z', 464},
    {'[', 472},
    {'\\', 480},
    {']', 488},
    {'^', 496},
    {'_', 504},
    {'`', 512},
    {'a', 520},
    {'b', 528},
    {'c', 536},
    {'d', 544},
    {'e', 552},
    {'f', 560},
    {'g', 568},
    {'h', 576},
    {'i', 584},
    {'j', 592},
    {'k', 600},
    {'l', 608},
    {'m', 616},
    {'n', 624},
    {'o', 632},
    {'p', 640},
    {'q', 648},
    {'r', 656},
    {'s', 664},
    {'t', 672},
    {'u', 680},
    {'v', 688},
    {'w', 696},
    {'x', 704},
    {'y', 712},
    {'z', 720},
    {'{', 728},
    {'|', 736},
    {'}', 744},
    {'~', 752},
    {233, 760},  // é (0xE9)
    {232, 768},  // è (0xE8)
    {234, 776},  // ê (0xEA)
    {224, 784},  // à (0xE0)
    {226, 792},  // â (0xE2)
    {251, 800},  // û (0xFB)
    {249, 808},  // ù (0xF9)
    {238, 816},  // î (0xEE)
    {239, 824},  // ï (0xEF)
    {231, 832}   // ç (0xE7)

};




static const int TOTAL_CHARS = 105;  // 95 ASCII + 10 étendus


// Fonctions utilitaires
static uint16_t create_argb4444_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    uint16_t a4 = (a >> 4) & 0xF;
    uint16_t r4 = (r >> 4) & 0xF;
    uint16_t g4 = (g >> 4) & 0xF;
    uint16_t b4 = (b >> 4) & 0xF;
    return (a4 << 12) | (r4 << 8) | (g4 << 4) | b4;
}

static int next_pow2(int n) {
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

static uint32_t parse_color_tag(const char* tag) {
    uint32_t color;
    sscanf(tag, "%x", &color);
    return color;
}

static UTF8Char get_next_utf8_char(const unsigned char* str, int* pos) {
    UTF8Char result = {{0}, 0};
    unsigned char c = str[*pos];

    if (c < 0x80) {
        result.bytes[0] = c;
        result.length = 1;
    }
    else if ((c & 0xE0) == 0xC0) {
        result.bytes[0] = c;
        result.bytes[1] = str[*pos + 1];
        result.length = 2;
    }
    else if ((c & 0xF0) == 0xE0) {
        result.bytes[0] = c;
        result.bytes[1] = str[*pos + 1];
        result.bytes[2] = str[*pos + 2];
        result.length = 3;
    }

    *pos += result.length;
    return result;
}

static int utf8_to_extended_char(UTF8Char utf8) {
    if (utf8.length == 1) {
        return utf8.bytes[0];
    }
    else if (utf8.length == 2) {
        return ((utf8.bytes[0] & 0x1F) << 6) | (utf8.bytes[1] & 0x3F);
    }
    return -1;
}

static int find_char_index(int charcode) {
    for(int i = 0; i < TOTAL_CHARS; i++) {
        if(CHAR_MAP_COMPLETE[i].charcode == charcode) {
            return i;
        }
    }
    return -1;
}

// Fonctions de gestion de la texture
static TextureAlpha* create_alpha_texture(uint32_t width, uint32_t height) {
    TextureAlpha* tex = malloc(sizeof(TextureAlpha));
    if (!tex) return NULL;
    
    tex->texture = pvr_mem_malloc(width * height * 2);
    if (!tex->texture) {
        free(tex);
        return NULL;
    }
    
    tex->width = width;
    tex->height = height;
    tex->content_width = 0;
    tex->content_height = 0;
    
    return tex;
}

static void free_alpha_texture(TextureAlpha* tex) {
    if (tex) {
        if (tex->texture) {
            pvr_mem_free(tex->texture);
        }
        free(tex);
    }
}

// Initialisation de la police
static FontInfo* init_basic_font() {
    FontInfo* font = malloc(sizeof(FontInfo));
    if(!font) return NULL;

    font->char_width = 8;
    font->char_height = 8;
    
    int total_size = TOTAL_CHARS * 64;
    font->bitmap = malloc(total_size);
    if(!font->bitmap) {
        free(font);
        return NULL;
    }
    
    for(int c = 0; c < TOTAL_CHARS; c++) {
        const CharMapEntry* entry = &CHAR_MAP_COMPLETE[c];
        
        for(int row = 0; row < 8; row++) {
            unsigned char line = FONT_COMPLETE[entry->offset + row];
            for(int bit = 0; bit < 8; bit++) {
                int idx = (c * 64) + (row * 8) + bit;
                font->bitmap[idx] = (line & (1 << (7 - bit))) ? 1 : 0;
            }
        }
    }
    
    return font;
}

// Création et mise à jour de textures
static TextureAlpha* create_text_texture_with_colors(const char* text, FontInfo* font, uint32_t bg_color) {
    const unsigned char* utext = (const unsigned char*)text;
    
    // Premier passage : compter les caractères et les lignes
    int pos = 0;
    int max_chars_per_line = 0;
    int current_line_length = 0;
    int num_lines = 1;
    
    while (utext[pos]) {
        if (utext[pos] == '\n') {
            num_lines++;
            max_chars_per_line = (current_line_length > max_chars_per_line) ? 
                                current_line_length : max_chars_per_line;
            current_line_length = 0;
            pos++;
            continue;
        }
        else if (utext[pos] == '\\' && utext[pos + 1] == 'c' && utext[pos + 2] == '[') {
            pos += 10;
            continue;
        }
        
        UTF8Char ch = get_next_utf8_char(utext, &pos);
        current_line_length++;
    }
    max_chars_per_line = (current_line_length > max_chars_per_line) ? 
                         current_line_length : max_chars_per_line;
    
    // Calculer les dimensions
    int content_width = max_chars_per_line * 8;
    int content_height = num_lines * 8;
    
    int tex_width = next_pow2(content_width);
    int tex_height = next_pow2(content_height);
    
    TextureAlpha* tex = create_alpha_texture(tex_width, tex_height);
    if (!tex) return NULL;
    
    tex->content_width = content_width;
    tex->content_height = content_height;
    
    // Créer le buffer
    uint16_t* buffer = malloc(tex_width * tex_height * 2);
    if (!buffer) {
        free_alpha_texture(tex);
        return NULL;
    }
    
    // Initialiser avec la couleur de fond
    uint8_t bg_a = (bg_color >> 24) & 0xFF;
    uint8_t bg_r = (bg_color >> 16) & 0xFF;
    uint8_t bg_g = (bg_color >> 8) & 0xFF;
    uint8_t bg_b = bg_color & 0xFF;
    uint16_t bg_color4444 = create_argb4444_color(bg_r, bg_g, bg_b, bg_a);
    
    for(int y = 0; y < content_height; y++) {
        for(int x = 0; x < content_width; x++) {
            buffer[y * tex_width + x] = bg_color4444;
        }
    }
    
    // Rendu du texte
    pos = 0;
    int char_pos = 0;
    int current_line = 0;
    uint32_t current_color = 0xFFFFFF;
    
    while (utext[pos]) {
        if (utext[pos] == '\n') {
            current_line++;
            char_pos = 0;
            pos++;
            continue;
        }
        
        if (utext[pos] == '\\' && utext[pos + 1] == 'c' && utext[pos + 2] == '[') {
            char color_str[7];
            memcpy(color_str, &utext[pos + 3], 6);
            color_str[6] = 0;
            current_color = parse_color_tag(color_str);
            pos += 10;
            continue;
        }
        
        UTF8Char utf8_char = get_next_utf8_char(utext, &pos);
        int char_code = utf8_to_extended_char(utf8_char);
        int char_index = find_char_index(char_code);
        
        if (char_index >= 0) {
            uint8_t r = (current_color >> 16) & 0xFF;
            uint8_t g = (current_color >> 8) & 0xFF;
            uint8_t b = current_color & 0xFF;
            uint16_t color4444 = create_argb4444_color(r, g, b, 0xFF);
            
            int base_x = char_pos * 8;
            int base_y = current_line * 8;
            
            for(int y = 0; y < 8; y++) {
                if(base_y + y >= content_height) break;
                
                for(int x = 0; x < 8; x++) {
                    if(base_x + x >= content_width) break;
                    
                    int bitmap_idx = (char_index * 64) + (y * 8) + x;
                    if(font->bitmap[bitmap_idx]) {
                        buffer[(base_y + y) * tex_width + (base_x + x)] = color4444;
                    }
                }
            }
            char_pos++;
        }
    }
    
    pvr_txr_load_ex(buffer, tex->texture, tex_width, tex_height,
                    PVR_TXRLOAD_16BPP | PVR_TXRFMT_ARGB4444);
    
    free(buffer);
    return tex;
}

static void draw_alpha_texture(TextureAlpha* tex, float x, float y) {
    if (!tex) return;
    
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;
    
    pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444,
                     tex->width, tex->height, tex->texture, PVR_FILTER_NONE);
    
    cxt.gen.alpha = PVR_ALPHA_ENABLE;
    cxt.blend.src = PVR_BLEND_SRCALPHA;
    cxt.blend.dst = PVR_BLEND_INVSRCALPHA;
    cxt.gen.culling = PVR_CULLING_NONE;
    
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    float real_u = (float)tex->content_width / (float)tex->width;
    float real_v = (float)tex->content_height / (float)tex->height;
    
    vert.flags = PVR_CMD_VERTEX;
    vert.argb = 0xFFFFFFFF;
    vert.oargb = 0;
    vert.z = 1.0f;
    
    // Point en haut à gauche
    vert.x = x;
    vert.y = y;
    vert.u = 0.0f;
    vert.v = 0.0f;
    pvr_prim(&vert, sizeof(vert));
    
    // Point en haut à droite
    vert.x = x + tex->content_width;
    vert.y = y;
    vert.u = real_u;
    vert.v = 0.0f;
    pvr_prim(&vert, sizeof(vert));
    
    // Point en bas à gauche
    vert.x = x;
    vert.y = y + tex->content_height;
    vert.u = 0.0f;
    vert.v = real_v;
    pvr_prim(&vert, sizeof(vert));
    
    // Point en bas à droite
    vert.flags = PVR_CMD_VERTEX_EOL;
    vert.x = x + tex->content_width;
    vert.y = y + tex->content_height;
    vert.u = real_u;
    vert.v = real_v;
    pvr_prim(&vert, sizeof(vert));
}

// Suite des fonctions publiques

DctResult dct_init(DctContext* ctx) {
    if (!ctx) return DCT_ERROR_INVALID_PARAM;
    
    InternalContext* internal = malloc(sizeof(InternalContext));
    if (!internal) return DCT_ERROR_MEMORY;
    
    // Initialiser la police
    internal->font_info = init_basic_font();
    if (!internal->font_info) {
        free(internal);
        return DCT_ERROR_INIT;
    }
    
    // Initialiser les valeurs par défaut
    internal->texture = NULL;
    internal->x = 0.0f;
    internal->y = 0.0f;
    internal->bg_color = 0x00000000; // Transparent par défaut
    
    ctx->font = internal;
    ctx->current_texture = NULL;
    ctx->x = 0.0f;
    ctx->y = 0.0f;
    ctx->bg_color = 0x00000000;
    
    return DCT_SUCCESS;
}

void dct_shutdown(DctContext* ctx) {
    if (!ctx) return;
    
    InternalContext* internal = (InternalContext*)ctx->font;
    if (internal) {
        if (internal->texture) {
            free_alpha_texture(internal->texture);
        }
        if (internal->font_info) {
            free(internal->font_info->bitmap);
            free(internal->font_info);
        }
        free(internal);
    }
    
    // Nettoyer le contexte public
    memset(ctx, 0, sizeof(DctContext));
}

DctResult dct_set_position(DctContext* ctx, float x, float y) {
    if (!ctx) return DCT_ERROR_INVALID_PARAM;
    
    ctx->x = x;
    ctx->y = y;
    return DCT_SUCCESS;
}

DctResult dct_set_background(DctContext* ctx, uint32_t color) {
    if (!ctx) return DCT_ERROR_INVALID_PARAM;
    
    ctx->bg_color = color;
    return DCT_SUCCESS;
}

DctResult dct_create_text(DctContext* ctx, const char* text) {
    if (!ctx || !text) return DCT_ERROR_INVALID_PARAM;
    
    InternalContext* internal = (InternalContext*)ctx->font;
    if (!internal) return DCT_ERROR_INIT;
    
    // Libérer l'ancienne texture si elle existe
    if (internal->texture) {
        free_alpha_texture(internal->texture);
        internal->texture = NULL;
    }
    
    // Créer la nouvelle texture
    internal->texture = create_text_texture_with_colors(text, internal->font_info, ctx->bg_color);
    if (!internal->texture) return DCT_ERROR_MEMORY;
    
    ctx->current_texture = internal->texture;
    return DCT_SUCCESS;
}

DctResult dct_update_text(DctContext* ctx, const char* text) {
    if (!ctx || !text) return DCT_ERROR_INVALID_PARAM;
    
    InternalContext* internal = (InternalContext*)ctx->font;
    if (!internal) return DCT_ERROR_INIT;
    
    // Si aucune texture n'existe, en créer une nouvelle
    if (!internal->texture) {
        return dct_create_text(ctx, text);
    }
    
    // Recréer la texture avec le nouveau texte
    TextureAlpha* new_texture = create_text_texture_with_colors(text, internal->font_info, ctx->bg_color);
    if (!new_texture) return DCT_ERROR_MEMORY;
    
    // Libérer l'ancienne texture
    free_alpha_texture(internal->texture);
    
    // Mettre à jour les pointeurs
    internal->texture = new_texture;
    ctx->current_texture = new_texture;
    
    return DCT_SUCCESS;
}

DctResult dct_draw(DctContext* ctx) {
    if (!ctx) return DCT_ERROR_INVALID_PARAM;
    
    InternalContext* internal = (InternalContext*)ctx->font;
    if (!internal || !internal->texture) return DCT_ERROR_INIT;
    
    draw_alpha_texture(internal->texture, ctx->x, ctx->y);
    return DCT_SUCCESS;
}


static FontInfo* convert_bitmap_font(const DctBitmapFont* font_data) {
    if (!font_data || !font_data->bitmap_data || !font_data->char_map) {
        return NULL;
    }

    FontInfo* font = malloc(sizeof(FontInfo));
    if (!font) return NULL;

    // Calculer la taille totale nécessaire pour le bitmap
    int total_size = font_data->total_chars * 
                    (font_data->char_width * font_data->char_height);
    
    font->bitmap = malloc(total_size);
    if (!font->bitmap) {
        free(font);
        return NULL;
    }

    // Copier les paramètres
    font->char_width = font_data->char_width;
    font->char_height = font_data->char_height;
    
    // Copier les données du bitmap
    memcpy(font->bitmap, font_data->bitmap_data, total_size);

    return font;
}

DctResult dct_load_font(DctContext* ctx, const DctBitmapFont* font_data) {
    if (!ctx || !font_data) return DCT_ERROR_INVALID_PARAM;

    InternalContext* internal = (InternalContext*)ctx->font;
    if (!internal) return DCT_ERROR_INIT;

    // Convertir la nouvelle police
    FontInfo* new_font = convert_bitmap_font(font_data);
    if (!new_font) return DCT_ERROR_MEMORY;

    // Libérer l'ancienne police
    if (internal->font_info) {
        free(internal->font_info->bitmap);
        free(internal->font_info);
    }

    // Mettre à jour la police
    internal->font_info = new_font;

    return DCT_SUCCESS;
}

DctResult dct_use_default_font(DctContext* ctx) {
    if (!ctx) return DCT_ERROR_INVALID_PARAM;

    InternalContext* internal = (InternalContext*)ctx->font;
    if (!internal) return DCT_ERROR_INIT;

    // Créer une nouvelle instance de la police par défaut
    FontInfo* default_font = init_basic_font();
    if (!default_font) return DCT_ERROR_MEMORY;

    // Libérer l'ancienne police
    if (internal->font_info) {
        free(internal->font_info->bitmap);
        free(internal->font_info);
    }

    // Mettre à jour avec la police par défaut
    internal->font_info = default_font;

    return DCT_SUCCESS;
}



// Modifier la fonction de conversion pour utiliser le KMG
DctBitmapFont convert_to_dct_font(BMFontChar* bmchars, int num_chars, 
                                 const char* kmg_file) {
    DctBitmapFont font;
    
    // Charger la texture KMG
    TextureKMG* kmg = load_kmg_texture(kmg_file);
    if (!kmg) {
        // Gérer l'erreur
        font.bitmap_data = NULL;
        return font;
    }
    
    font.char_width = kmg->header.width;
    font.char_height = kmg->header.height;
    
    // Copier les données dans une texture PVR
    pvr_ptr_t texture = pvr_mem_malloc(font.char_width * font.char_height * 2);
    if (texture) {
        pvr_txr_load_ex(kmg->texture_data, texture,
                       font.char_width, font.char_height,
                       PVR_TXRLOAD_16BPP | PVR_TXRFMT_ARGB4444);
    }
    
    font.bitmap_data = texture;
    font.total_chars = num_chars;
    
    // Créer le mapping
    font.char_map = malloc(num_chars * sizeof(*font.char_map));
    for (int i = 0; i < num_chars; i++) {
        font.char_map[i].charcode = bmchars[i].charcode;
        font.char_map[i].offset = bmchars[i].y * font.char_width + bmchars[i].x;
    }
    
    // Nettoyer la texture KMG
    free(kmg->texture_data);
    free(kmg);
    
    return font;
}


BMFontChar* parse_bmfont_file(const char* filename, int* num_chars) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;
    
    // Lire le nombre de caractères
    char line[256];
    int char_count = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "chars count=", 11) == 0) {
            sscanf(line, "chars count=%d", &char_count);
            break;
        }
    }
    
    if (char_count == 0) {
        fclose(file);
        return NULL;
    }
    
    // Allouer le tableau de caractères
    BMFontChar* chars = malloc(sizeof(BMFontChar) * char_count);
    
    // Lire chaque caractère
    int char_index = 0;
    rewind(file);
    
    while (fgets(line, sizeof(line), file) && char_index < char_count) {
        if (strncmp(line, "char id=", 8) == 0) {
            BMFontChar* c = &chars[char_index];
            sscanf(line, "char id=%hu x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d",
                   &c->charcode, &c->x, &c->y, &c->width, &c->height, 
                   &c->xoffset, &c->yoffset, &c->xadvance);
            char_index++;
        }
    }
    
    fclose(file);
    *num_chars = char_count;
    return chars;
}


