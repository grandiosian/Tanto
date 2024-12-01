// dct_interface.h
#ifndef DCT_INTERFACE_H
#define DCT_INTERFACE_H

#include <kos.h>
#include <kmg/kmg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>



// Structure pour définir une police bitmap personnalisée 
typedef struct {
    unsigned char* bitmap_data;     // Données de la police
    int char_width;                 // Largeur de chaque caractère
    int char_height;               // Hauteur de chaque caractère
    struct {
        unsigned short charcode;    // Code du caractère
        int offset;                 // Offset dans le bitmap
    } *char_map;                   // Table de mapping des caractères
    int total_chars;               // Nombre total de caractères
} DctBitmapFont;

typedef struct {
    unsigned short charcode;    
    int x, y;                  // Position dans la texture
    int width, height;         // Dimensions
    int xoffset, yoffset;      // Ajustements
    int xadvance;             // Espacement
} BMFontChar;




// Structure principale pour gérer le contexte de texte
typedef struct {
    void* font;              // Police de caractères
    void* current_texture;   // Texture courante
    float x;                 // Position X
    float y;                 // Position Y
    uint32_t bg_color;      // Couleur de fond (ARGB)
} DctContext;

// Codes de retour
typedef enum {
    DCT_SUCCESS = 0,
    DCT_ERROR_INIT = -1,
    DCT_ERROR_MEMORY = -2,
    DCT_ERROR_INVALID_PARAM = -3
} DctResult;


// Structures pour la gestion du texte
typedef struct {
    pvr_ptr_t texture;
    uint32_t width;
    uint32_t height;
    uint32_t content_width;
    uint32_t content_height;
} TextureAlpha;

typedef struct {
    uint8_t* bitmap;
    int char_width;
    int char_height;
    int first_char;
    int last_char;
} FontInfo;

typedef struct {
    unsigned char bytes[4];
    int length;
} UTF8Char;

typedef struct {
    unsigned short charcode;
    int offset;
} CharMapEntry;

// Structure interne pour le contexte
typedef struct {
    FontInfo* font_info;
    TextureAlpha* texture;
    float x;
    float y;
    uint32_t bg_color;
} InternalContext;


// Fonction pour charger une police personnalisée
DctResult dct_load_font(DctContext* ctx, const DctBitmapFont* font_data);

// Fonction pour revenir à la police par défaut
DctResult dct_use_default_font(DctContext* ctx);

// Initialisation et nettoyage
DctResult dct_init(DctContext* ctx);
void dct_shutdown(DctContext* ctx);

// Fonctions de manipulation du texte
DctResult dct_set_position(DctContext* ctx, float x, float y);
DctResult dct_set_background(DctContext* ctx, uint32_t color);

// Création et mise à jour du texte
// Le texte supporte:
// - \n pour les sauts de ligne
// - \c[RRGGBB] pour les couleurs (format hexa)
DctResult dct_create_text(DctContext* ctx, const char* text);
DctResult dct_update_text(DctContext* ctx, const char* text);

// Affichage
DctResult dct_draw(DctContext* ctx);

#endif // DCT_INTERFACE_H