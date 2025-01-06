 #include "dct_texture.h"


int load_dct_texture(dct_texture_t *dct_texture, const char *fname) {

    //dct_texture_t dct_texture;
    dct_texture->addr = NULL;
    
    // Charger le fichier KMG
    if(kmg_to_img(fname, &dct_texture->texture) != 0) {
        printf("Erreur chargement texture %s\n", fname);
        return -1;
    }
    
    // Sauvegarder les dimensions
    dct_texture->width = dct_texture->texture.w;
    dct_texture->height = dct_texture->texture.h;
    
    // Allouer l'espace en mémoire PVR
    dct_texture->addr = pvr_mem_malloc(dct_texture->texture.byte_count);
    if(dct_texture->addr == NULL) {
        printf("Erreur allocation mémoire PVR\n");
        return -1;
    }
    printf("before load kimg \n");
    // Charger la texture en mémoire PVR (pour une texture VQ)
    pvr_txr_load_kimg(&dct_texture->texture, dct_texture->addr, 
        PVR_TXRLOAD_FMT_TWIDDLED | PVR_TXRLOAD_FMT_VQ);
    printf("before pvr_poly_cxt_txr \n");
    // Préparer le contexte du polygone
    //pvr_poly_cxt_t cxt;
    pvr_poly_cxt_txr(&dct_texture->cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_RGB565, dct_texture->width, dct_texture->height, dct_texture->addr, PVR_FILTER_BILINEAR);
    dct_texture->cxt.gen.culling = PVR_CULLING_NONE; 
    dct_texture->cxt.depth.write = PVR_DEPTHWRITE_ENABLE;
    dct_texture->cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
    dct_texture->cxt.blend.src = PVR_BLEND_SRCALPHA;
    dct_texture->cxt.blend.dst = PVR_BLEND_INVSRCALPHA;
    dct_texture->cxt.blend.src_enable = PVR_BLEND_ENABLE;
    dct_texture->cxt.blend.dst_enable = PVR_BLEND_ENABLE; 
    
    printf("before pvr_poly_compile\n");
    // Compiler le contexte en en-tête
    pvr_poly_compile(&dct_texture->hdr, &dct_texture->cxt);
    
    return 1;
}



void unload_dct_texture(dct_texture_t *text) {

    kos_img_free(&text->texture,1);

    if(text->addr) {
        pvr_mem_free(text->addr);
        text->addr = NULL;
    }
}
