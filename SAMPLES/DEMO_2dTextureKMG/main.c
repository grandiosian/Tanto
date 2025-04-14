#include <kos.h>
#include <kmg/kmg.h>

#include "dct_guiText.h"
#include "dct_input.h"


extern uint8 romdisk[];
extern uint8 romdisk_end[];

// Variables globales pour la texture
pvr_ptr_t texture_addr = NULL;    // Adresse de la texture en mémoire PVR
pvr_poly_hdr_t hdr;               // En-tête du polygone

// Structure pour stocker les dimensions de la texture
struct {
    uint32 width;
    uint32 height;
} tex_dims;

// Fonction pour charger la texture
int load_texture(const char *fname) {
    kos_img_t texture;       // Structure locale de l'image KMG
    
    // Charger le fichier KMG
    if(kmg_to_img(fname, &texture) != 0) {
        printf("Erreur chargement texture %s\n", fname);
        return -1;
    }
    
    // Sauvegarder les dimensions
    tex_dims.width = texture.w;
    tex_dims.height = texture.h;
    
    // Allouer l'espace en mémoire PVR
    texture_addr = pvr_mem_malloc(texture.byte_count);
    if(texture_addr == NULL) {
        printf("Erreur allocation mémoire PVR\n");
        return -1;
    }
    
    // Charger la texture en mémoire PVR (pour une texture VQ)
    pvr_txr_load_kimg(&texture, texture_addr, 
        PVR_TXRLOAD_FMT_TWIDDLED | PVR_TXRLOAD_FMT_VQ);
    
    // Préparer le contexte du polygone
    pvr_poly_cxt_t cxt;
    pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY,
                     PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_RGB565,
                     texture.w, texture.h,
                     texture_addr,
                     PVR_FILTER_BILINEAR);

    // Compiler le contexte en en-tête
    pvr_poly_compile(&hdr, &cxt);
    
    return 0;
}

// Fonction pour dessiner un quad texturé
void draw_textured_quad(float x1, float y1, float x2, float y2) {
    pvr_vertex_t vert;
    
    // Envoyer l'en-tête
    pvr_prim(&hdr, sizeof(hdr));
    
    // Premier vertex (haut gauche)
    vert.flags = PVR_CMD_VERTEX;
    vert.x = x1;
    vert.y = y1;
    vert.z = 1.0f;
    vert.u = 0.0f;
    vert.v = 0.0f;
    vert.argb = 0xFFFFFFFF;
    vert.oargb = 0;
    pvr_prim(&vert, sizeof(vert));
    
    // Deuxième vertex (haut droite)
    vert.x = x2;
    vert.y = y1;
    vert.u = 1.0f;
    vert.v = 0.0f;
    pvr_prim(&vert, sizeof(vert));
    
    // Troisième vertex (bas gauche)
    vert.x = x1;
    vert.y = y2;
    vert.u = 0.0f;
    vert.v = 1.0f;
    pvr_prim(&vert, sizeof(vert));
    
    // Dernier vertex (bas droite)
    vert.flags = PVR_CMD_VERTEX_EOL;
    vert.x = x2;
    vert.y = y2;
    vert.u = 1.0f;
    vert.v = 1.0f;
    pvr_prim(&vert, sizeof(vert));
}

// Fonction pour libérer la texture
void unload_texture() {
    if(texture_addr) {
        pvr_mem_free(texture_addr);
        texture_addr = NULL;
    }
}

// Exemple d'utilisation dans le main
int main() {
    // Initialiser KOS
    pvr_init_defaults();
    pvr_set_bg_color(0.2f,0.2f,0.2f);

    fs_romdisk_mount("/rd", romdisk, romdisk_end - romdisk);

    // Ajoutez un printf pour vérifier le montage
    printf("Romdisk monté: taille = %d octets\n", romdisk_end - romdisk);

    printf("Tentative d'ouverture de %s\n", "/rd/Pave_stylized_128x128.kmg");
    file_t f = fs_open("/rd/Pave_stylized_128x128.kmg", O_RDONLY);
    if (f < 0) {
        printf("Erreur fs_open: %d\n", f);
        // Liste le contenu du répertoire /rd
        file_t d = fs_open("/rd", O_RDONLY | O_DIR);
        if (d >= 0) {
            dirent_t *de;
            printf("Contenu du répertoire /rd:\n");
            while ((de = fs_readdir(d)) != NULL) {
                printf(" - %s\n", de->name);
            }
            fs_close(d);
        }
    }
    
    // Charger la texture
    if(load_texture("/rd/Pave_stylized_128x128.kmg") != 0) {
        return 1;
    }

    float x1 = 320 - tex_dims.width/2;
    float y1 = 240 - tex_dims.height/2;
    float x2 = 320 + tex_dims.width/2;
    float y2 = 240 + tex_dims.height/2;
    float speed = 3.0f;


    BitmapFont* font = dct_init_font("/rd", "/rd/Arial.fnt");
    if (!font) return 1;

    TextProperties props = {
        .x = 20.0f,
        .y = 20.0f,
        .scale = 1.0f,
        .color = 0xFFFFFFFF
    };
    
    // Boucle principale
    while(1) {
        pvr_wait_ready();
        pvr_scene_begin();
        
        pvr_list_begin(PVR_LIST_TR_POLY);
        
        // Dessiner un quad texturé centré à l'écran
        draw_textured_quad(
            x1,  // x1
            y1, // y1
            x2,  // x2
            y2  // y2
        );

        dct_draw_text(font,"Demo Texture KMG \n\\c[005000]Dpad to move  " ,props);

        pvr_list_finish();
        
        pvr_scene_finish();


         MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
            if(st->buttons & CONT_START)
            {
                return -1;
            }
            if(st->buttons & CONT_DPAD_UP)
            {
                y1 -= 1.0f*speed;
                y2 -= 1.0f*speed;
            }
            if(st->buttons & CONT_DPAD_DOWN)
            {
                y1 += 1.0f*speed;
                y2 += 1.0f*speed;
            }
            if(st->buttons & CONT_DPAD_LEFT)
            {
                x1 -= 1.0f*speed;
                x2 -= 1.0f*speed;
            }
            if(st->buttons & CONT_DPAD_RIGHT)
            {
                x1 += 1.0f*speed;
                x2 += 1.0f*speed;
            }
            
        MAPLE_FOREACH_END()
    }
    
    // Nettoyer
    unload_texture();
    pvr_shutdown();
    
    return 0;
}