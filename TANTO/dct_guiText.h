#ifndef DCT_GUITEXT_H
#define DCT_GUITEXT_H

#include <kos.h>
#include <stdbool.h>
#include <kmg/kmg.h>

typedef struct {
    pvr_ptr_t texture;
    int width;
    int height;
} FontPage;

typedef struct {
    int x, y;
    int width, height;
    int xoffset, yoffset;
    int xadvance;
    int page;
} BitmapChar;

typedef struct {
    FontPage* pages;
    int pageCount;
    BitmapChar* chars;
    int line_height;
    int base;
} BitmapFont;

typedef struct {
    float x, y;
    float scale;
    uint32_t color;
} TextProperties;



// Structures privées
typedef struct {
    float characters_per_second;
    float current_time;
    int chars_visible;
    int total_length;
    bool is_complete;
}TextAnimation;

typedef struct {
    char* text;
    TextAnimation* animation;
    bool is_complete;
} TextElement;

typedef struct {
    TextElement* elements;
    int count;
    int current_index;
    bool sequence_finished;
    float chars_per_second;
} TextSequence;

typedef struct {
    uint32_t stack[32];
    int depth;
} ColorStack;

//typedef struct TextAnimation TextAnimation;
//typedef struct TextElement TextElement;
//typedef struct TextSequence TextSequence;

// Fonctions de gestion du texte et des animations
TextAnimation* dct_init_text_animation(const char* text, float chars_per_second);
TextSequence* dct_init_text_sequence(const char** texts, int count, float chars_per_second);
void dct_update_text_animation(TextAnimation* anim, float delta_time);
void dct_update_text_sequence(TextSequence* seq, float delta_time, bool next_button_pressed);
void dct_draw_text_sequence(TextSequence* seq, BitmapFont* font, TextProperties props);
void dct_free_text_sequence(TextSequence* seq);

// Fonctions de gestion des polices et du rendu
BitmapFont* dct_init_font(const char* base_path, const char* fnt_file);
void dct_free_font(BitmapFont* font);
void dct_draw_char(BitmapFont* font, char c, TextProperties* props);
void dct_draw_text(BitmapFont* font, const char* text, TextProperties props);
void dct_draw_animated_text(BitmapFont* font, const char* text, TextProperties props, TextAnimation* anim);
void dct_draw_full_texture(BitmapFont* font, int page_index, float x, float y, float scale);

#endif