#include <kos.h>
#include <kmg/kmg.h>

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
    
    // Charger la texture
    if(load_texture("/cd/Pave_stylized_128x128.kmg") != 0) {
        return 1;
    }
    
    // Boucle principale
    while(1) {
        pvr_wait_ready();
        pvr_scene_begin();
        
        pvr_list_begin(PVR_LIST_TR_POLY);
        
        // Dessiner un quad texturé centré à l'écran
        draw_textured_quad(
            320 - tex_dims.width/2,  // x1
            240 - tex_dims.height/2, // y1
            320 + tex_dims.width/2,  // x2
            240 + tex_dims.height/2  // y2
        );
        
        pvr_list_finish();
        pvr_scene_finish();
    }
    
    // Nettoyer
    unload_texture();
    pvr_shutdown();
    
    return 0;
}