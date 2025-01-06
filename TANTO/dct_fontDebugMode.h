#ifndef DCT_FONTDEBUGMODE_H
#define DCT_FONTDEBUGMODE_H

#include "dct_datafontDebugMode.h"

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
        //printf("UTF8 decode: %02X %02X -> %d\n", utf8.bytes[0], utf8.bytes[1], value);
        return value;
    }
    return -1;
}

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

// Fonction d'initialisation modifiée
dct_fontInfo* init_basic_font() {
    dct_fontInfo* font = malloc(sizeof(dct_fontInfo));
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

dct_textureAlpha* create_alpha_texture(uint32_t width, uint32_t height) {
    dct_textureAlpha* tex = malloc(sizeof(dct_textureAlpha));
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


dct_textureAlpha* create_text_texture_with_colors(const char* text, dct_fontInfo* font, uint32_t bg_color, int pwidth, int pheight) {
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
    int tex_width = pwidth;
    while (tex_width < content_width) {
        tex_width *= 2;
    }
    
    int tex_height = pheight;
    while (tex_height < content_height) {
        tex_height *= 2;
    }
    
    printf("Creating texture %dx%d for content %dx%d\n", 
           tex_width, tex_height, content_width, content_height);
    
    dct_textureAlpha* tex = create_alpha_texture(tex_width, tex_height);
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
    //printf("Taille final de la texture width:%d height:%d \n",tex_width,tex_height);
    // Charger la texture
    /*
    pvr_txr_load_ex(buffer, tex->texture, tex_width, tex_height,
                    PVR_TXRLOAD_16BPP | PVR_TXRFMT_ARGB4444);
    */
    pvr_txr_load_ex(buffer, tex->texture, tex_width, tex_height,
                    PVR_TXRLOAD_16BPP | PVR_TXRFMT_ARGB4444 | PVR_TXRLOAD_DMA );

    free(buffer);
    return tex;
}


void draw_alpha_texture(dct_textureAlpha* tex, float x, float y) {
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
    cxt.depth.write = PVR_DEPTHWRITE_DISABLE;
    
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    // Calculer les UV en fonction du contenu réel
    float real_u = (float)tex->content_width / (float)tex->width;
    float real_v = (float)tex->content_height / (float)tex->height;
    
    vert.flags = PVR_CMD_VERTEX;
    vert.argb = 0xFFFFFFFF;
    vert.oargb = 0;
    vert.z = 10.0f;
    
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
void update_text_texture(dct_textureAlpha* tex, const char* text, dct_fontInfo* font, uint32_t bg_color) {
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

// Fonction pour libérer la mémoire
void free_alpha_texture(dct_textureAlpha* tex) {
    if (tex) {
        if (tex->texture) {
            pvr_mem_free(tex->texture);  // Correct car texture est déjà un pvr_ptr_t
        }
        free(tex);
    }
}



#endif