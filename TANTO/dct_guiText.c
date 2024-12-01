#include "dct_guiText.h"



// Fonctions utilitaires privées
static uint32_t parse_color(const char* hex) {
    unsigned int color;
    sscanf(hex, "%x", &color);
    return 0xFF000000 | color;
}

static unsigned int get_utf8_char(const char** text) {
    const unsigned char* str = (const unsigned char*)*text;
    unsigned int c = *str;
    int len = 1;

    if (c >= 0x80) {
        if ((c & 0xE0) == 0xC0) {
            c = ((c & 0x1F) << 6) | (str[1] & 0x3F);
            len = 2;
        } else if ((c & 0xF0) == 0xE0) {
            c = ((c & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
            len = 3;
        }
    }

    *text += len;
    return c;
}

// Implémentation des fonctions publiques
TextAnimation* dct_init_text_animation(const char* text, float chars_per_second) {
    TextAnimation* anim = malloc(sizeof(TextAnimation));
    if(!anim) return NULL;
    
    anim->characters_per_second = chars_per_second;
    anim->current_time = 0;
    anim->chars_visible = 0;
    anim->total_length = strlen(text);
    anim->is_complete = false;
    
    return anim;
}

void dct_update_text_animation(TextAnimation* anim, float delta_time) {
    if(anim->is_complete) return;
    
    anim->current_time += delta_time;
    anim->chars_visible = (int)(anim->current_time * anim->characters_per_second);
    
    if(anim->chars_visible >= anim->total_length) {
        anim->chars_visible = anim->total_length;
        anim->is_complete = true;
    }
}

TextSequence* dct_init_text_sequence(const char** texts, int count, float chars_per_second) {
    TextSequence* seq = malloc(sizeof(TextSequence));
    if (!seq) return NULL;

    seq->elements = malloc(sizeof(TextElement) * count);
    if (!seq->elements) {
        free(seq);
        return NULL;
    }

    seq->count = count;
    seq->current_index = 0;
    seq->sequence_finished = false;
    seq->chars_per_second = chars_per_second;

    for (int i = 0; i < count; i++) {
        seq->elements[i].text = strdup(texts[i]);
        seq->elements[i].animation = dct_init_text_animation(texts[i], chars_per_second);
        seq->elements[i].is_complete = false;
    }

    return seq;
}

void dct_update_text_sequence(TextSequence* seq, float delta_time, bool next_button_pressed) {
    if (seq->sequence_finished) return;

    TextElement* current = &seq->elements[seq->current_index];
    dct_update_text_animation(current->animation, delta_time);

    if (next_button_pressed) {
        if (!current->animation->is_complete) {
            current->animation->chars_visible = current->animation->total_length;
            current->animation->is_complete = true;
            current->is_complete = true;
        }
        else if (seq->current_index < seq->count - 1) {
            seq->current_index++;
        }
        else {
            seq->sequence_finished = true;
        }
    }
}


void dct_draw_text_sequence(TextSequence* seq, BitmapFont* font, TextProperties props) {
    if (seq->sequence_finished) return;
    TextElement* current = &seq->elements[seq->current_index];
    dct_draw_animated_text(font, current->text, props, current->animation);
}

void dct_free_text_sequence(TextSequence* seq) {
    if (seq) {
        for (int i = 0; i < seq->count; i++) {
            free(seq->elements[i].text);
            free(seq->elements[i].animation);
        }
        free(seq->elements);
        free(seq);
    }
}

void dct_draw_animated_text(BitmapFont* font, const char* text, TextProperties props, TextAnimation* anim) {
    float start_x = props.x;
    ColorStack colors = { .depth = 0 };
    colors.stack[0] = props.color;
    
    int visible_count = 0;
    const char* ptr = text;
    
    while (*ptr && visible_count < anim->chars_visible) {
        if (*ptr == '\n') {
            props.x = start_x;
            props.y += font->line_height * props.scale;
            ptr++;
            continue;
        }
        else if (*ptr == '\\' && *(ptr + 1) == 'c' && *(ptr + 2) == '[') {
            ptr += 3;
            char color_str[7];
            strncpy(color_str, ptr, 6);
            color_str[6] = '\0';
            colors.stack[++colors.depth] = parse_color(color_str);
            props.color = colors.stack[colors.depth];
            ptr += 7;
            continue;
        }
        else if (*ptr == '\\' && *(ptr + 1) == 'c' && *(ptr + 2) == 'e') {
            ptr += 3;
            if (colors.depth > 0) {
                colors.depth--;
                props.color = colors.stack[colors.depth];
            }
            continue;
        }

        unsigned int c = get_utf8_char(&ptr);
        if (c < 256 && font->chars[c].width > 0) {
            dct_draw_char(font, c, &props);
            visible_count++;
        }
    }
}



// Partie mise à jour de la fonction init_font
BitmapFont* dct_init_font(const char* base_path, const char* fnt_file) {
    printf("\ndct init font \n");
    BitmapFont* font = malloc(sizeof(BitmapFont));
    if (!font) return NULL;

    FILE* fnt = fopen(fnt_file, "r");
    if (!fnt) {
        free(font);
        return NULL;
    }
    printf("\nbefore fgets\n");
    // Lire d'abord le nombre de pages
    char line[256];
    int scaleW,scaleH = 0;
    while(fgets(line, sizeof(line), fnt)) {
        if(strncmp(line, "common", 6) == 0) {
            sscanf(line, "common lineHeight=%d base=%d scaleW=%d scaleH=%d pages=%d",
                   &font->line_height, &font->base, &scaleW, &scaleH , &font->pageCount);
            break;
        }
    }
    printf("\nafter fget font lineheight: %d base %d scaleW:%d scaleH:%d pageCount :%d\n",font->line_height, font->base, scaleW, scaleH ,font->pageCount);
    // Allouer le tableau des pages
    font->pages = malloc(sizeof(FontPage) * font->pageCount);
    if (!font->pages) {
        free(font);
        fclose(fnt);
        return NULL;
    }
    printf("\nfgets fnt page \n");
    // Charger chaque page
    rewind(fnt);
    while(fgets(line, sizeof(line), fnt)) {
        if(strncmp(line, "page", 4) == 0) {
            int page_id;
            char page_file[256];
            char full_path[512];
            sscanf(line, "page id=%d file=\"%[^\"]\"", &page_id, page_file);
            printf("\npage file : %s \n",page_file);
            // Remplacer .png par .kmg si nécessaire
            char* ext = strrchr(page_file, '.');
            if(ext && strcmp(ext, ".png") == 0) {
                strcpy(ext, ".kmg");
            }
            
            snprintf(full_path, sizeof(full_path), "%s/%s", base_path, page_file);
            
            kos_img_t img;
            if (kmg_to_img(full_path, &img) == 0) {
                font->pages[page_id].width = img.w;
                font->pages[page_id].height = img.h;
                font->pages[page_id].texture = pvr_mem_malloc(img.byte_count);
                
                if(font->pages[page_id].texture) {
                    pvr_txr_load_kimg(&img, font->pages[page_id].texture, 
                        PVR_TXRLOAD_FMT_TWIDDLED | PVR_TXRLOAD_FMT_VQ);
                }
            }
        }
    }
    printf("\nafter fgets fnt page \n");
    // Charger les caractères
    font->chars = calloc(256, sizeof(BitmapChar));
    if (!font->chars) {
        for(int i = 0; i < font->pageCount; i++) {
            if(font->pages[i].texture) pvr_mem_free(font->pages[i].texture);
        }
        free(font->pages);
        free(font);
        fclose(fnt);
        return NULL;
    }

    rewind(fnt);
    while(fgets(line, sizeof(line), fnt)) {
        if(strncmp(line, "char", 4) == 0) {
            int id, x, y, width, height, xoffset, yoffset, xadvance, page;
            sscanf(line, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d",
                   &id, &x, &y, &width, &height, &xoffset, &yoffset, &xadvance, &page);
            
            if(id < 256) {
                font->chars[id].x = x;
                font->chars[id].y = y;
                font->chars[id].width = width;
                font->chars[id].height = height;
                font->chars[id].xoffset = xoffset;
                font->chars[id].yoffset = yoffset;
                font->chars[id].xadvance = xadvance;
                font->chars[id].page = page;
            }
        }
    }
    
    fclose(fnt);
    return font;
}


void dct_free_font(BitmapFont* font) {
    if (font) {
        if (font->pages) {
            for (int i = 0; i < font->pageCount; i++) {
                if (font->pages[i].texture) {
                    pvr_mem_free(font->pages[i].texture);
                }
            }
            free(font->pages);
        }
        if (font->chars) {
            free(font->chars);
        }
        free(font);
    }
}



// Partie mise à jour de la fonction draw_char
void dct_draw_char(BitmapFont* font, char c, TextProperties* props) {
    unsigned char index = (unsigned char)c;
    BitmapChar* ch = &font->chars[index];
    if (ch->width == 0) return;

    // Utiliser la texture de la page appropriée
    FontPage* page = &font->pages[ch->page];
    
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

    pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY,
                     PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_ARGB4444,
                     page->width, page->height,
                     page->texture,
                     PVR_FILTER_BILINEAR);
    
    cxt.gen.alpha = PVR_ALPHA_ENABLE;
    cxt.blend.src = PVR_BLEND_SRCALPHA;
    cxt.blend.dst = PVR_BLEND_INVSRCALPHA;
    cxt.gen.culling = PVR_CULLING_NONE;
    
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    float x = props->x + (ch->xoffset * props->scale);
    float y = props->y + (ch->yoffset * props->scale);
    float width = ch->width * props->scale;
    float height = ch->height * props->scale;

    float u1 = (float)ch->x / page->width;
    float v1 = (float)ch->y / page->height;
    float u2 = u1 + ((float)ch->width / page->width);
    float v2 = v1 + ((float)ch->height / page->height);

    vert.flags = PVR_CMD_VERTEX;
    vert.argb = props->color;
    vert.oargb = 0;
    vert.z = 1.0f;

    // Rendu des vertices
    // Top-left
    vert.x = x; vert.y = y; vert.u = u1; vert.v = v1;
    pvr_prim(&vert, sizeof(vert));

    // Top-right
    vert.x = x + width; vert.u = u2;
    pvr_prim(&vert, sizeof(vert));

    // Bottom-left
    vert.x = x; vert.y = y + height; vert.u = u1; vert.v = v2;
    pvr_prim(&vert, sizeof(vert));

    // Bottom-right
    vert.flags = PVR_CMD_VERTEX_EOL;
    vert.x = x + width; vert.u = u2;
    pvr_prim(&vert, sizeof(vert));

    props->x += ch->xadvance * props->scale;
}




void dct_draw_text(BitmapFont* font, const char* text, TextProperties props) {
    float start_x = props.x;
    ColorStack colors = { .depth = 0 };
    colors.stack[0] = props.color;

    while (*text) {
        if (*text == '\n') {
            props.x = start_x;
            props.y += font->line_height * props.scale;
            text++;
            continue;
        }
        else if (*text == '\\' && *(text + 1) == 'c' && *(text + 2) == '[') {
            text += 3;
            char color_str[7];
            strncpy(color_str, text, 6);
            color_str[6] = '\0';
            colors.stack[++colors.depth] = parse_color(color_str);
            props.color = colors.stack[colors.depth];
            text += 7;
            continue;
        }
        else if (*text == '\\' && *(text + 1) == 'c' && *(text + 2) == 'e') {
            text += 3;
            if (colors.depth > 0) {
                colors.depth--;
                props.color = colors.stack[colors.depth];
            }
            continue;
        }

        unsigned int c = get_utf8_char(&text);
        if (c < 256 && font->chars[c].width > 0) {
            dct_draw_char(font, c, &props);
        }
    }
}

void dct_draw_full_texture(BitmapFont* font, int page_index, float x, float y, float scale) {
    FontPage* page = &font->pages[page_index];

    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;
    
    pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY,
                     PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_RGB565,
                     page->width, page->height,
                     page->texture,
                     PVR_FILTER_BILINEAR);

    cxt.gen.alpha = PVR_ALPHA_ENABLE;
    cxt.blend.src = PVR_BLEND_SRCALPHA;
    cxt.blend.dst = PVR_BLEND_INVSRCALPHA;
    cxt.gen.culling = PVR_CULLING_NONE;
    
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    float width = page->width * scale;
    float height = page->height * scale;

    vert.flags = PVR_CMD_VERTEX;
    vert.argb = 0xFFFFFFFF;
    vert.oargb = 0;
    vert.z = 1.0f;

    vert.x = x;
    vert.y = y;
    vert.u = 0.0f;
    vert.v = 0.0f;
    pvr_prim(&vert, sizeof(vert));

    vert.x = x + width;
    vert.u = 1.0f;
    pvr_prim(&vert, sizeof(vert));

    vert.x = x;
    vert.y = y + height;
    vert.u = 0.0f;
    vert.v = 1.0f;
    pvr_prim(&vert, sizeof(vert));

    vert.flags = PVR_CMD_VERTEX_EOL;
    vert.x = x + width;
    vert.u = 1.0f;
    pvr_prim(&vert, sizeof(vert));
}