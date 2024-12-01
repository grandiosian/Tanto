#include <kos.h>

typedef struct {
    pvr_ptr_t texture;
    uint32_t width;      // Largeur totale (puissance de 2)
    uint32_t height;     // Hauteur totale (puissance de 2)
    uint32_t content_width;  // Largeur réelle du contenu
    uint32_t content_height; // Hauteur réelle du contenu
} TextureAlpha;

typedef struct {
    char character;
    int offset;
} CharacterMapping;

// Structure pour stocker les informations de police
typedef struct {
    uint8_t* bitmap;
    int char_width;
    int char_height;
    int first_char;
    int last_char;
} FontInfo;


// Structure pour gérer les caractères UTF-8
typedef struct {
    unsigned char bytes[4];  // Un caractère UTF-8 peut faire jusqu'à 4 bytes
    int length;             // Nombre de bytes pour ce caractère
} UTF8Char;

// Helper pour créer une couleur RGB565 avec alpha
uint16_t create_rgb565_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // Si alpha est 0, retourner 0 (complètement transparent)
    if (a == 0) return 0;
    
    // Convertir en RGB565
    uint16_t r5 = (r >> 3) & 0x1F;
    uint16_t g6 = (g >> 2) & 0x3F;
    uint16_t b5 = (b >> 3) & 0x1F;
    
    return (r5 << 11) | (g6 << 5) | b5;
}

// Helper pour la couleur avec alpha
uint16_t create_argb4444_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    uint16_t a4 = (a >> 4) & 0xF;
    uint16_t r4 = (r >> 4) & 0xF;
    uint16_t g4 = (g >> 4) & 0xF;
    uint16_t b4 = (b >> 4) & 0xF;
    return (a4 << 12) | (r4 << 8) | (g4 << 4) | b4;
}

// Fonction utilitaire pour trouver la prochaine puissance de 2
int next_pow2(int n) {
    int p = 1;
    while (p < n) {
        p *= 2;
    }
    return p;
}


// Fonction utilitaire pour vérifier si on est sur un caractère de contrôle
int is_control_sequence(const unsigned char* text, int pos, const char* sequence) {
    while(*sequence) {
        if(text[pos] != *sequence) return 0;
        pos++;
        sequence++;
    }
    return 1;
}

// Conversion RGB888 vers RGB565
uint16_t rgb888_to_rgb565(uint32_t rgb888) {
    uint8_t r = (rgb888 >> 16) & 0xFF;
    uint8_t g = (rgb888 >> 8) & 0xFF;
    uint8_t b = rgb888 & 0xFF;
    
    // Conversion en RGB565
    uint16_t r5 = (r >> 3) & 0x1F;
    uint16_t g6 = (g >> 2) & 0x3F;
    uint16_t b5 = (b >> 3) & 0x1F;
    
    return (r5 << 11) | (g6 << 5) | b5;
}

// Fonction pour extraire une couleur d'une balise
uint32_t parse_color_tag(const char* tag) {
    uint32_t color;
    sscanf(tag, "%x", &color);
    return color;
}

// Fonction pour lire le prochain caractère UTF-8 dans une chaîne
UTF8Char get_next_utf8_char(const unsigned char* str, int* pos) {
    UTF8Char result = {{0}, 0};
    unsigned char c = str[*pos];

    if (c < 0x80) {  // ASCII standard
        result.bytes[0] = c;
        result.length = 1;
    }
    else if ((c & 0xE0) == 0xC0) {  // 2 bytes
        result.bytes[0] = c;
        result.bytes[1] = str[*pos + 1];
        result.length = 2;
    }
    else if ((c & 0xF0) == 0xE0) {  // 3 bytes
        result.bytes[0] = c;
        result.bytes[1] = str[*pos + 1];
        result.bytes[2] = str[*pos + 2];
        result.length = 3;
    }

    // Avancer la position
    *pos += result.length;
    return result;
}


// Fonction corrigée pour convertir les caractères UTF-8
int utf8_to_extended_char(UTF8Char utf8) {
    if (utf8.length == 1) {
        return utf8.bytes[0];
    }
    else if (utf8.length == 2) {
        int value = ((utf8.bytes[0] & 0x1F) << 6) | (utf8.bytes[1] & 0x3F);
        printf("UTF8 decode: %02X %02X -> %d\n", utf8.bytes[0], utf8.bytes[1], value);
        return value;
    }
    return -1;
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

// Table de mapping complète incluant les caractères étendus
typedef struct {
    unsigned short charcode;  // Utiliser unsigned short pour gérer les codes > 127
    int offset;
} CharMapEntry;


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

const int TOTAL_CHARS = 105;  // 95 ASCII + 10 étendus
// Nouvelle fonction de recherche avec debug
int find_char_index(int charcode) {
    //printf("Finding char index for code: %d (0x%X)\n", charcode, charcode);
    
    for(int i = 0; i < TOTAL_CHARS; i++) {
        //printf("  Comparing with entry %d: code=%d (0x%X)\n", i, CHAR_MAP_COMPLETE[i].charcode, CHAR_MAP_COMPLETE[i].charcode);
               
        if(CHAR_MAP_COMPLETE[i].charcode == charcode) {
            //printf("  Found match at index %d\n", i);
            return i;
        }
    }
    //printf("  No match found\n");
    return -1;
}
TextureAlpha* create_alpha_texture(uint32_t width, uint32_t height) {
    TextureAlpha* tex = malloc(sizeof(TextureAlpha));
    if (!tex) return NULL;
    
    // Allouer la mémoire pour la texture
    tex->texture = pvr_mem_malloc(width * height * 2);  // 2 bytes par pixel (ARGB4444)
    if (!tex->texture) {
        free(tex);
        return NULL;
    }
    
    // Stocker les dimensions totales de la texture
    tex->width = width;
    tex->height = height;
    
    // Initialiser les dimensions du contenu à 0
    tex->content_width = 0;
    tex->content_height = 0;
    
    return tex;
}


TextureAlpha* create_text_texture_with_colors(const char* text, FontInfo* font, uint32_t bg_color) {
    const unsigned char* utext = (const unsigned char*)text;
    uint32_t current_color = 0xFFFFFF;
    
    // Premier passage : compter les caractères et les lignes
    int pos = 0;
    int max_chars_per_line = 0;
    int current_line_length = 0;
    int num_lines = 1;
    
    while (utext[pos] != 0) {
        if (utext[pos] == 0x0A) {
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
    
    // Calculer les dimensions réelles nécessaires
    int content_width = max_chars_per_line * 8;
    int content_height = num_lines * 8;
    
    // Calculer les dimensions de texture (prochaines puissances de 2)
    int tex_width = 256;
    while (tex_width < content_width) {
        tex_width *= 2;
    }
    
    int tex_height = 256;
    while (tex_height < content_height) {
        tex_height *= 2;
    }
    
    printf("Creating texture %dx%d for content %dx%d\n", 
           tex_width, tex_height, content_width, content_height);
    
    TextureAlpha* tex = create_alpha_texture(tex_width, tex_height);
    if (!tex) return NULL;
    
    tex->width = tex_width;
    tex->height = tex_height;
    tex->content_width = content_width;
    tex->content_height = content_height;
    
    // Créer et initialiser le buffer
    uint16_t* buffer = (uint16_t*)malloc(tex_width * tex_height * 2);
    if (!buffer) {
        free_alpha_texture(tex);
        return NULL;
    }
    
    // Initialiser tout le buffer à transparent
    memset(buffer, 0, tex_width * tex_height * 2);
    
    // Convertir la couleur de fond en ARGB4444
    uint8_t bg_a = (bg_color >> 24) & 0xFF;
    uint8_t bg_r = (bg_color >> 16) & 0xFF;
    uint8_t bg_g = (bg_color >> 8) & 0xFF;
    uint8_t bg_b = bg_color & 0xFF;
    
    uint16_t bg_color4444 = ((bg_a >> 4) << 12) |
                           ((bg_r >> 4) << 8) |
                           ((bg_g >> 4) << 4) |
                           (bg_b >> 4);
    
    // Remplir la zone de contenu avec la couleur de fond
    for(int y = 0; y < content_height; y++) {
        for(int x = 0; x < content_width; x++) {
            buffer[y * tex_width + x] = bg_color4444;
        }
    }
    
    // Rendu du texte
    pos = 0;
    int char_pos = 0;
    int current_line = 0;
    current_color = 0xFFFFFF;
    
    while (utext[pos] != 0) {
        if (utext[pos] == 0x0A) {
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
        
        if(char_index >= 0 && char_pos < max_chars_per_line) {
            // Convertir la couleur courante en ARGB4444
            uint8_t r = (current_color >> 16) & 0xFF;
            uint8_t g = (current_color >> 8) & 0xFF;
            uint8_t b = current_color & 0xFF;
            uint16_t color4444 = (0xF << 12) |      // Alpha opaque
                                ((r >> 4) << 8) |    // Rouge
                                ((g >> 4) << 4) |    // Vert
                                (b >> 4);            // Bleu
            
            // Position de base dans le buffer pour ce caractère
            int base_x = char_pos * 8;
            int base_y = current_line * 8;
            
            // Dessiner le caractère
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
    
    // Charger la texture
    pvr_txr_load_ex(buffer, tex->texture, tex_width, tex_height,
                    PVR_TXRLOAD_16BPP | PVR_TXRFMT_ARGB4444);
    
    free(buffer);
    return tex;
}


void draw_alpha_texture(TextureAlpha* tex, float x, float y) {
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;
    
    // Utiliser la taille totale de la texture pour le contexte
    pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444,
                     tex->width, tex->height, tex->texture, PVR_FILTER_NONE);
    
    cxt.gen.alpha = PVR_ALPHA_ENABLE;
    cxt.blend.src = PVR_BLEND_SRCALPHA;
    cxt.blend.dst = PVR_BLEND_INVSRCALPHA;
    cxt.gen.culling = PVR_CULLING_NONE;
    
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    // Calculer les UV en fonction du contenu réel
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
    
    // Point en bas à droite (dernier vertex)
    vert.flags = PVR_CMD_VERTEX_EOL;
    vert.x = x + tex->content_width;
    vert.y = y + tex->content_height;
    vert.u = real_u;
    vert.v = real_v;
    pvr_prim(&vert, sizeof(vert));
}
// Fonction pour libérer la mémoire
void free_alpha_texture(TextureAlpha* tex) {
    pvr_mem_free(tex->texture);
    free(tex);
}


// Fonction d'initialisation modifiée
FontInfo* init_basic_font() {
    FontInfo* font = malloc(sizeof(FontInfo));
    if(!font) return NULL;

    font->char_width = 8;
    font->char_height = 8;
    
    // Copier FONT_DATA pour les caractères ASCII
    //memcpy(FONT_COMPLETE, FONT_DATA, NUM_CHARS * 8);
    
    // Allouer l'espace pour la police complète
    int total_size = TOTAL_CHARS * 64;  // 64 pixels par caractère (8x8)
    font->bitmap = malloc(total_size);
    if(!font->bitmap) {
        free(font);
        return NULL;
    }
    
    // Convertir la police en bitmap
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
    
    printf("Font initialized with %d characters\n", 
           TOTAL_CHARS);
           
    return font;
}

void draw_colored_background() {
    pvr_vertex_t vert;
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    
    pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));
    
    vert.flags = PVR_CMD_VERTEX;
    vert.oargb = 0;
    vert.z = 1.0f;
    
    // Premier triangle (coin supérieur gauche)
    vert.argb = PVR_PACK_COLOR(255, 255, 0, 0);  // Rouge
    vert.x = 0.0f;
    vert.y = 0.0f;
    pvr_prim(&vert, sizeof(vert));
    
    vert.argb = PVR_PACK_COLOR(255, 0, 255, 0);  // Vert
    vert.x = 640.0f;
    vert.y = 0.0f;
    pvr_prim(&vert, sizeof(vert));
    
    vert.argb = PVR_PACK_COLOR(255, 0, 0, 255);  // Bleu
    vert.x = 0.0f;
    vert.y = 480.0f;
    pvr_prim(&vert, sizeof(vert));
    
    // Deuxième triangle (coin inférieur droit)
    vert.argb = PVR_PACK_COLOR(255, 0, 255, 0);  // Vert
    vert.x = 640.0f;
    vert.y = 0.0f;
    pvr_prim(&vert, sizeof(vert));
    
    vert.argb = PVR_PACK_COLOR(255, 255, 255, 0);  // Jaune
    vert.x = 640.0f;
    vert.y = 480.0f;
    pvr_prim(&vert, sizeof(vert));
    
    vert.flags = PVR_CMD_VERTEX_EOL;
    vert.argb = PVR_PACK_COLOR(255, 0, 0, 255);  // Bleu
    vert.x = 0.0f;
    vert.y = 480.0f;
    pvr_prim(&vert, sizeof(vert));
}

// Fonction pour mettre à jour une texture existante
void update_text_texture(TextureAlpha* tex, const char* text, FontInfo* font, uint32_t bg_color) {
    if(!tex || !text || !font) return;

    // Allouer le buffer temporaire (utiliser la taille totale de la texture)
    uint16_t* buffer = (uint16_t*)malloc(tex->width * tex->height * 2);
    if(!buffer) return;
    
    // Initialiser avec la couleur de fond
    uint8_t bg_a = (bg_color >> 24) & 0xFF;
    uint8_t bg_r = (bg_color >> 16) & 0xFF;
    uint8_t bg_g = (bg_color >> 8) & 0xFF;
    uint8_t bg_b = bg_color & 0xFF;
    
    uint16_t bg_color4444 = ((bg_a >> 4) << 12) |
                           ((bg_r >> 4) << 8) |
                           ((bg_g >> 4) << 4) |
                           (bg_b >> 4);

    // Remplir la zone de contenu
    for(int y = 0; y < tex->content_height; y++) {
        for(int x = 0; x < tex->content_width; x++) {
            buffer[y * tex->width + x] = bg_color4444;
        }
    }

    // Rendu du texte
    const unsigned char* utext = (const unsigned char*)text;
    int pos = 0;
    int char_pos = 0;
    int current_line = 0;
    uint32_t current_color = 0xFFFFFF;

    while(utext[pos] != 0) {
        if(utext[pos] == 0x0A) {
            current_line++;
            char_pos = 0;
            pos++;
            continue;
        }

        if(utext[pos] == '\\' && utext[pos + 1] == 'c' && utext[pos + 2] == '[') {
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

        if(char_index >= 0) {
            uint8_t r = (current_color >> 16) & 0xFF;
            uint8_t g = (current_color >> 8) & 0xFF;
            uint8_t b = current_color & 0xFF;
            uint16_t color4444 = (0xF << 12) |      // Alpha opaque
                                ((r >> 4) << 8) |    // Rouge
                                ((g >> 4) << 4) |    // Vert
                                (b >> 4);            // Bleu

            // Position de base du caractère
            int base_x = char_pos * 8;
            int base_y = current_line * 8;

            // Dessiner le caractère
            for(int y = 0; y < 8; y++) {
                if(base_y + y >= tex->content_height) break;
                
                for(int x = 0; x < 8; x++) {
                    if(base_x + x >= tex->content_width) break;
                    
                    int bitmap_idx = (char_index * 64) + (y * 8) + x;
                    if(font->bitmap[bitmap_idx]) {
                        int buffer_idx = (base_y + y) * tex->width + (base_x + x);
                        buffer[buffer_idx] = color4444;
                    }
                }
            }
            char_pos++;
        }
    }

    // Mettre à jour la texture
    pvr_txr_load_ex(buffer, tex->texture, tex->width, tex->height,
                    PVR_TXRLOAD_16BPP | PVR_TXRFMT_ARGB4444);

    free(buffer);
}


void main() {
    printf("Starting program\n");
    
    if (pvr_init_defaults() != 0) {
        printf("Failed to initialize PVR\n");
        return;
    }
    
    pvr_set_bg_color(0.2f, 0.2f, 0.2f);
    
    // Debug de la police
    printf("Font data debug - First few bytes:\n");
    for(int i = 0; i < 16; i++) {
        printf("0x%02X ", FONT_COMPLETE[i]);
    }
    printf("\n");
    
    // Debug des données pour 'T'
    int t_offset = ('T' - 32) * 8;
    printf("Offset for 'T': %d\n", t_offset);
    printf("Data for 'T':\n");
    for(int i = 0; i < 8; i++) {
        printf("0x%02X ", FONT_COMPLETE[t_offset + i]);
    }
    printf("\n");
    
    FontInfo* font = init_basic_font();
    if (!font) {
        printf("Font init failed\n");
        return;
    }

    // Variables globales
    TextureAlpha* text_tex = NULL;

    vec3f_t position = {0.0f,1.0f,2.0f};
    char *textInput = "---------------------------------\n\\c[FF0000]Texte de Couleur Rouge \n\\c[00FF00]Texte de couleur Verte\n\\c[0000FF]Texte de couleur bleue \n\nposition \nx:%f %f %f ";

    // Dans votre boucle principale
    if(text_tex) {
        free_alpha_texture(text_tex);
        text_tex = NULL;
    }

    char buffer[256];  // Plus grand buffer pour être sûr
    sprintf(buffer, textInput, position.x, position.y, position.z);
    text_tex = create_text_texture_with_colors(buffer, font, 0x305FACFF);


    while(1) {
        
        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
            if(st->buttons & CONT_START)
            {
                return -1;
            }
            if(st->buttons & CONT_DPAD_UP)
            {
                position.y += 1.0f;
                sprintf(buffer, textInput, position.x, position.y, position.z);
                //update_text_texture(text_tex, buffer, font, 0x305FACFF);
            }
            if(st->buttons & CONT_DPAD_DOWN)
            {
                position.y -= 1.0f;
                sprintf(buffer, textInput, position.x, position.y, position.z);
                //update_text_texture(text_tex, buffer, font, 0x305FACFF);
            }
            if(st->buttons & CONT_DPAD_LEFT)
            {
                position.x -= 1.0f;
                sprintf(buffer, textInput, position.x, position.y, position.z);
                //update_text_texture(text_tex, buffer, font, 0x305FACFF);
            }
            if(st->buttons & CONT_DPAD_RIGHT)
            {
                position.x += 1.0f;
                sprintf(buffer, textInput, position.x, position.y, position.z);
                //update_text_texture(text_tex, buffer, font, 0x305FACFF);
            }
            
        MAPLE_FOREACH_END()

        

        
        pvr_wait_ready();
        pvr_scene_begin();

        pvr_list_begin(PVR_LIST_OP_POLY);
        pvr_list_finish();

        pvr_list_begin(PVR_LIST_TR_POLY);
        draw_alpha_texture(text_tex, position.x+100.0f, position.y+100.0f);
        pvr_list_finish();

        pvr_scene_finish();
    }
}