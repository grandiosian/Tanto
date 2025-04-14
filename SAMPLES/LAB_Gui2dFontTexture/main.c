#include <kos.h>


// Structure pour une page de texture
typedef struct {
    pvr_ptr_t texture;      // Texture PVR
    int width;              // Largeur de la texture
    int height;             // Hauteur de la texture
} FontPage;


// Structure pour l'animation du texte
typedef struct {
    float characters_per_second;  // Vitesse d'apparition des caractères
    float current_time;          // Temps écoulé
    int chars_visible;           // Nombre de caractères visibles
    int total_length;            // Longueur totale du texte
    bool is_complete;            // Animation terminée ?
} TextAnimation;



// Structure pour un élément de texte
typedef struct {
    char* text;               // Le texte à afficher
    TextAnimation* animation; // L'animation de ce texte
    bool is_complete;        // Texte entièrement affiché ?
} TextElement;



// Structure pour gérer la séquence de textes
typedef struct {
    TextElement* elements;    // Tableau des éléments de texte
    int count;               // Nombre total d'éléments
    int current_index;       // Index de l'élément courant
    bool sequence_finished;  // Séquence terminée ?
    float chars_per_second;  // Vitesse d'animation par défaut
} TextSequence;





// Structure pour stocker la couleur courante
typedef struct {
    uint32_t stack[32];  // Stack de couleurs
    int depth;           // Profondeur actuelle
} ColorStack;

// Fonction pour parser une couleur hexadécimale
uint32_t parse_color(const char* hex) {
    unsigned int color;
    sscanf(hex, "%x", &color);
    return 0xFF000000 | color;  // Force alpha à 255
}
// Ajout du numéro de page dans BitmapChar
typedef struct {
    int x, y;              // Position dans la texture
    int width, height;     // Dimensions
    int xoffset, yoffset;  // Décalages
    int xadvance;          // Espacement
    int page;              // Numéro de la page de texture
} BitmapChar;


typedef struct {
    FontPage* pages;        // Tableau des pages de texture
    int pageCount;         // Nombre de pages
    BitmapChar* chars;      // Infos des caractères
    int line_height;        // Hauteur de ligne
    int base;               // Ligne de base
} BitmapFont;


typedef struct {
    float x, y;        // Position à l'écran
    float scale;       // Échelle du texte
    uint32_t color;    // Couleur ARGB
} TextProperties;


// Fonction pour remplacer l'extension .png par .kmg
void replace_extension(char* filename) {
    char* ext = strrchr(filename, '.');
    if(ext && strcmp(ext, ".png") == 0) {
        strcpy(ext, ".kmg");
    }
}


// Initialisation de l'animation
TextAnimation* init_text_animation(const char* text, float chars_per_second) {
    TextAnimation* anim = malloc(sizeof(TextAnimation));
    if(!anim) return NULL;
    
    anim->characters_per_second = chars_per_second;
    anim->current_time = 0;
    anim->chars_visible = 0;
    anim->total_length = strlen(text);
    anim->is_complete = false;
    
    return anim;
}


// Mise à jour de l'animation
void update_text_animation(TextAnimation* anim, float delta_time) {
    if(anim->is_complete) return;
    
    anim->current_time += delta_time;
    anim->chars_visible = (int)(anim->current_time * anim->characters_per_second);
    
    if(anim->chars_visible >= anim->total_length) {
        anim->chars_visible = anim->total_length;
        anim->is_complete = true;
    }
}



// Initialisation de la séquence
TextSequence* init_text_sequence(const char** texts, int count, float chars_per_second) {
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

    // Initialiser chaque élément
    for (int i = 0; i < count; i++) {
        seq->elements[i].text = strdup(texts[i]);
        seq->elements[i].animation = init_text_animation(texts[i], chars_per_second);
        seq->elements[i].is_complete = false;
    }

    return seq;
}

// Mise à jour de la séquence
void update_text_sequence(TextSequence* seq, float delta_time, bool next_button_pressed) {
    if (seq->sequence_finished) return;

    TextElement* current = &seq->elements[seq->current_index];
    
    // Mettre à jour l'animation du texte courant
    update_text_animation(current->animation, delta_time);

    // Si le bouton est pressé
    if (next_button_pressed) {
        // Si le texte actuel est en cours d'animation, le terminer
        if (!current->animation->is_complete) {
            current->animation->chars_visible = current->animation->total_length;
            current->animation->is_complete = true;
            current->is_complete = true;
        }
        // Sinon, passer au texte suivant
        else if (seq->current_index < seq->count - 1) {
            seq->current_index++;
        }
        else {
            seq->sequence_finished = true;
        }
    }
    //printf("current index : %d \n",seq->sequence_finished);
}

// Affichage de la séquence
void draw_text_sequence(TextSequence* seq, BitmapFont* font, TextProperties props) {
    if (seq->sequence_finished) return;

    TextElement* current = &seq->elements[seq->current_index];
    draw_animated_text(font, current->text, props, current->animation);
}

// Libération de la mémoire
void free_text_sequence(TextSequence* seq) {
    if (seq) {
        for (int i = 0; i < seq->count; i++) {
            free(seq->elements[i].text);
            free(seq->elements[i].animation);
        }
        free(seq->elements);
        free(seq);
    }
}


// Ajouter cette fonction pour gérer les caractères UTF-8
unsigned int get_utf8_char(const char** text) {
    const unsigned char* str = (const unsigned char*)*text;
    unsigned int c = *str;
    int len = 1;

    if (c >= 0x80) {
        if ((c & 0xE0) == 0xC0) {      // 2 bytes
            c = ((c & 0x1F) << 6) | (str[1] & 0x3F);
            len = 2;
        } else if ((c & 0xF0) == 0xE0) { // 3 bytes
            c = ((c & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
            len = 3;
        }
    }

    *text += len;
    return c;
}


// Version modifiée de draw_text avec animation
void draw_animated_text(BitmapFont* font, const char* text, TextProperties props, TextAnimation* anim) {
    float start_x = props.x;
    ColorStack colors = { .depth = 0 };
    colors.stack[0] = props.color;
    
    int visible_count = 0;  // Compteur de caractères visibles
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
            draw_char(font, c, &props);
            visible_count++;
        }
    }
}
// Fonction modifiée pour charger plusieurs pages
BitmapFont* init_font(const char* base_path, const char* fnt_file) {
    BitmapFont* font = malloc(sizeof(BitmapFont));
    if (!font) return NULL;
    printf("\n before fopen \n");
    char line[256];
    FILE* fnt = fopen(fnt_file, "r");
    if (!fnt) {
        free(font);
        return NULL;
    }
    printf("\n before fgets \n");
    // Lire le nombre de pages
    while(fgets(line, sizeof(line), fnt)) {
        if(strncmp(line, "common", 6) == 0) {
            int pages;
            sscanf(line, "common lineHeight=%d base=%d scaleW=%*d scaleH=%*d pages=%d",
                   &font->line_height, &font->base, &pages);
            font->pageCount = pages;
            break;
        }
    }

    // Allouer le tableau des pages
    font->pages = malloc(sizeof(FontPage) * font->pageCount);
    if (!font->pages) {
        free(font);
        fclose(fnt);
        return NULL;
    }

    // Lire les infos de chaque page
    rewind(fnt);
    char page_filename[256];
    char full_path[512];
    int page_id;

    while(fgets(line, sizeof(line), fnt)) {
        if(strncmp(line, "page id=", 8) == 0) {
            sscanf(line, "page id=%d file=\"%[^\"]\"", &page_id, page_filename);
            replace_extension(page_filename);
            // Construire le chemin complet
            snprintf(full_path, sizeof(full_path), "%s/%s", base_path, page_filename);

            // Charger la texture KMG
            kos_img_t img;
            if (kmg_to_img(full_path, &img) != 0) {
                printf("Erreur chargement texture %s\n", full_path);
                continue;
            }

            // Sauvegarder les dimensions
            font->pages[page_id].width = img.w;
            font->pages[page_id].height = img.h;

            // Allouer l'espace en mémoire PVR
            font->pages[page_id].texture = pvr_mem_malloc(img.byte_count);
            if(font->pages[page_id].texture == NULL) {
                printf("Erreur allocation mémoire PVR pour page %d\n", page_id);
                continue;
            }

            // Charger la texture en mémoire PVR
            pvr_txr_load_kimg(&img, font->pages[page_id].texture, PVR_TXRLOAD_16BPP);
        }
    }

    // Allouer et initialiser la table de caractères
    font->chars = calloc(256, sizeof(BitmapChar));

    // Lire les caractères avec leur page
    rewind(fnt);
    while (fgets(line, sizeof(line), fnt)) {
        if (strncmp(line, "char id=", 8) == 0) {
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


void free_font(BitmapFont* font) {
    if (font) {
        if (font->pages) {
            for(int i = 0; i < font->pageCount; i++) {
                if(font->pages[i].texture) {
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


// Affichage d'un caractère
void draw_char(BitmapFont* font, char c, TextProperties* props) {
    unsigned char index = (unsigned char)c;
    BitmapChar* ch = &font->chars[index];
    if (ch->width == 0) return;

    // Récupérer la page correcte
    FontPage* page = &font->pages[ch->page];

    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

    /*
    pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED,
                     page->width, page->height, page->texture, PVR_FILTER_NONE);
    */

    /*
    pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED,
                     font->tex_width, font->tex_height, font->texture, PVR_FILTER_NONE);
    */
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

    props->x += (ch->xadvance + 2) * props->scale*0.2f;
    // Position avec les offsets corrects
    float x = props->x + ((ch->xoffset) * props->scale) ;
    float y = props->y + (ch->yoffset * props->scale);

    // Dimensions à l'échelle
    float width = ch->width * props->scale;
    float height = ch->height * props->scale;

    // UV précis
    float u1 = (float)ch->x / page->width;
    float v1 = (float)ch->y / page->height;
    float u2 = u1 + ((float)ch->width / page->width);
    float v2 = v1 + ((float)ch->height / page->height);

    vert.flags = PVR_CMD_VERTEX;
    vert.argb = props->color;
    vert.oargb = 0;
    vert.z = 1.0f;

    // Haut gauche
    vert.x = x;
    vert.y = y;
    vert.u = u1;
    vert.v = v1;
    pvr_prim(&vert, sizeof(vert));

    // Haut droite
    vert.x = x + width;
    vert.y = y;
    vert.u = u2;
    vert.v = v1;
    pvr_prim(&vert, sizeof(vert));

    // Bas gauche
    vert.x = x;
    vert.y = y + height;
    vert.u = u1;
    vert.v = v2;
    pvr_prim(&vert, sizeof(vert));

    // Bas droite
    vert.flags = PVR_CMD_VERTEX_EOL;
    vert.x = x + width;
    vert.y = y + height;
    vert.u = u2;
    vert.v = v2;
    pvr_prim(&vert, sizeof(vert));

    props->x += ch->xadvance * props->scale;
}

// Fonction modifiée pour le rendu de texte avec support des tags et sauts de ligne
void draw_text(BitmapFont* font, const char* text, TextProperties props) {
    float start_x = props.x;
    ColorStack colors = { .depth = 0 };
    colors.stack[0] = props.color;  // Couleur de base

    while (*text) {
        if (*text == '\n') {
            // Saut de ligne
            props.x = start_x;
            props.y += font->line_height * props.scale;
            text++;
            continue;
        }
        else if (*text == '\\' && *(text + 1) == 'c' && *(text + 2) == '[') {
            // Tag de couleur : \c[RRGGBB]
            text += 3;  // Sauter \c[
            char color_str[7];
            strncpy(color_str, text, 6);
            color_str[6] = '\0';
            colors.stack[++colors.depth] = parse_color(color_str);
            props.color = colors.stack[colors.depth];
            text += 7;  // 6 caractères de couleur + ]
            continue;
        }
        else if (*text == '\\' && *(text + 1) == 'c' && *(text + 2) == 'e') {
            // Fermeture de la couleur : \ce
            text += 3;
            if (colors.depth > 0) {
                colors.depth--;
                props.color = colors.stack[colors.depth];
            }
            continue;
        }

        // Rendu normal du caractère
        unsigned int c = get_utf8_char(&text);
        if (c < 256 && font->chars[c].width > 0) {
            draw_char(font, c, &props);
        }
    }
}

// Affichage de la texture complète
void draw_full_texture(BitmapFont* font, float x, float y, float scale) {


    // Récupérer la page correcte
    FontPage* page = &font->pages[0];

    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;
    
    /*
    pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED,
                     font->tex_width, font->tex_height, font->texture, PVR_FILTER_NONE);
    */
    
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

    // Haut gauche
    vert.x = x;
    vert.y = y;
    vert.u = 0.0f;
    vert.v = 0.0f;
    pvr_prim(&vert, sizeof(vert));

    // Haut droite
    vert.x = x + width;
    vert.y = y;
    vert.u = 1.0f;
    vert.v = 0.0f;
    pvr_prim(&vert, sizeof(vert));

    // Bas gauche
    vert.x = x;
    vert.y = y + height;
    vert.u = 0.0f;
    vert.v = 1.0f;
    pvr_prim(&vert, sizeof(vert));

    // Bas droite
    vert.flags = PVR_CMD_VERTEX_EOL;
    vert.x = x + width;
    vert.y = y + height;
    vert.u = 1.0f;
    vert.v = 1.0f;
    pvr_prim(&vert, sizeof(vert));
}


int main() {
    // Initialisation
    pvr_init_defaults();
    pvr_set_bg_color(0.2f,0.2f,0.2f);
    // Charger la police
    BitmapFont* font = init_font("/cd", "/cd/Arial.fnt");
    if (!font) {
        printf("Erreur chargement police\n");
        return 1;
    }
    printf("\n after init font \n");
    const char* texts[] = {
        "Premier texte\n\\c[FF0000]en rouge\\ce !",
        "Deuxième texte qui apparaît\naprès le premier",
        "\\c[00FF00]Dernier message\\ce\nde la séquence"
    };
    
    TextSequence* seq = init_text_sequence(texts, 3, 20.0f);


    // Propriétés du texte
    TextProperties props = {
        .x = 50.0f,
        .y = 50.0f,
        .scale = 1.0f,
        .color = 0xFFFFFFFF  // Blanc
    };
    
    uint64_t last_time = timer_ms_gettime64();

    while(1) {
        uint64_t current_time = timer_ms_gettime64();
        float delta = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        bool button_pressed = false;

        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
            if (st->buttons & CONT_A) {
                button_pressed = true;
            }
            if (st->buttons & CONT_START) {
                goto cleanup;
            }
        MAPLE_FOREACH_END()



        // Mise à jour et affichage
        update_text_sequence(seq, delta, button_pressed);


        pvr_wait_ready();
        pvr_scene_begin();

        pvr_list_begin(PVR_LIST_TR_POLY);

        // Afficher la texture complète
        draw_full_texture(font, 50.0f, 50.0f, 1.0f);

        // Afficher du texte
        props.y = 300.0f;
        draw_text_sequence(seq, font, props);


        pvr_list_finish();
        pvr_scene_finish();

        
    }

    cleanup:
    free_text_sequence(seq);
    free_font(font);
    return 0;
}