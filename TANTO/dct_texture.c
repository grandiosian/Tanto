 #include "dct_texture.h"


int load_dct_texture(dct_texture_t *dct_texture, dct_cxt_t *pcxt , pvr_poly_hdr_t *phdr, const char *fname) {

    //dct_texture_t dct_texture;
    dct_texture->addrA = NULL;
    dct_texture->addrB = NULL;
    dct_texture->textureA.data = NULL;
    dct_texture->textureA.w = 0;
    dct_texture->textureA.h = 0;
    dct_texture->textureA.fmt = 0;
    dct_texture->textureA.byte_count = 0;
    dct_texture->textureB.data = NULL;
    dct_texture->textureB.w = 0;
    dct_texture->textureB.h = 0;
    dct_texture->textureB.fmt = 0;
    dct_texture->textureB.byte_count = 0;
    
    // Charger le fichier KMG
    if(kmg_to_img(fname, &dct_texture->textureA) != 0) {
        printf("Erreur chargement texture A %s\n", fname);
        return -1;
    }
    
    // Sauvegarder les dimensions
    dct_texture->width = dct_texture->textureA.w;
    dct_texture->height = dct_texture->textureA.h;
    
    // Allouer l'espace en mémoire PVR
    dct_texture->addrA = pvr_mem_malloc(dct_texture->textureA.byte_count);
    if(dct_texture->addrA == NULL) {
        printf("Erreur allocation mémoire Addr A PVR\n");
        return -1;
    }
    printf("before load kimg \n");
    // Charger la texture en mémoire PVR (pour une texture VQ)
    pvr_txr_load_kimg(&dct_texture->textureA, dct_texture->addrA, 
        PVR_TXRLOAD_FMT_TWIDDLED | PVR_TXRLOAD_FMT_VQ);
    printf("before pvr_poly_cxt_txr \n");
    // Préparer le contexte du polygone
    //pvr_poly_cxt_t cxt;
   
    pvr_poly_cxt_txr(&dct_texture->cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_RGB565, dct_texture->width, dct_texture->height, dct_texture->addrA, PVR_FILTER_BILINEAR);
    dct_texture->cxt.gen.culling = PVR_CULLING_NONE; 
    dct_texture->cxt.depth.write = PVR_DEPTHWRITE_ENABLE;
    dct_texture->cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
    dct_texture->cxt.blend.src = PVR_BLEND_SRCALPHA;
    dct_texture->cxt.blend.dst = PVR_BLEND_INVSRCALPHA;
    dct_texture->cxt.blend.src_enable = PVR_BLEND_DISABLE;
    dct_texture->cxt.blend.dst_enable = PVR_BLEND_DISABLE; 
    dct_texture->cxt.txr.uv_flip = 2;
    dct_texture->cxt.txr.enable = 1;
    
    pcxt->ident = DCT_CXT_T;
    pvr_poly_cxt_txr(&pcxt->cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_RGB565, dct_texture->width, dct_texture->height, dct_texture->addrA, PVR_FILTER_BILINEAR);
    pcxt->cxt.gen.culling = PVR_CULLING_NONE; 
    pcxt->cxt.depth.write = PVR_DEPTHWRITE_ENABLE;
    pcxt->cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
    pcxt->cxt.blend.src = PVR_BLEND_SRCALPHA;
    pcxt->cxt.blend.dst = PVR_BLEND_INVSRCALPHA;
    pcxt->cxt.blend.src_enable = PVR_BLEND_DISABLE;
    pcxt->cxt.blend.dst_enable = PVR_BLEND_DISABLE; 
    pcxt->cxt.txr.uv_flip = 2;
    pcxt->cxt.txr.enable = 1;


    // Compiler le contexte en en-tête
    pvr_poly_compile(phdr, &pcxt->cxt);
    
    return 1;
}


int load_dct_textureTPCM(dct_texture_t *dct_texture, const char *fnameA, const char *fnameB) {

    dct_texture->addrA = NULL;
    
    // Charger le fichier KMG
    if(kmg_to_img(fnameA, &dct_texture->textureA) != 0) {
        printf("Erreur chargement textureA %s\n", fnameB);
        return -1;
    }

    if(strcmp(fnameA,fnameB)==0 && sizeof(fnameA)==sizeof(fnameB))
    {
        printf("les deux texture sont identique, on utilise les données des deux textures \n");
        memcpy( &dct_texture->textureA, &dct_texture->textureB, sizeof(kos_img_t));
    }
    else
    {
        if(kmg_to_img(fnameB, &dct_texture->textureB) != 0) 
        {
            printf("Erreur chargement textureB %s\n", fnameB);
            return -1;
        }
    }
    
    
    // Sauvegarder les dimensions
    dct_texture->width = dct_texture->textureA.w;
    dct_texture->height = dct_texture->textureA.h;
    
    // Allouer l'espace en mémoire PVR
    dct_texture->addrA = pvr_mem_malloc(dct_texture->textureA.byte_count);
    if(dct_texture->addrA == NULL) {
        printf("Erreur allocation mémoire AddrA PVR\n");
        return -1;
    }
    dct_texture->addrB = pvr_mem_malloc(dct_texture->textureB.byte_count);
    if(dct_texture->addrB == NULL) {
        printf("Erreur allocation mémoire AddrB PVR\n");
        return -1;
    }
    printf("before load kimg \n");
    // Charger la texture en mémoire PVR (pour une texture VQ)
    pvr_txr_load_kimg(&dct_texture->textureA, dct_texture->addrA, 
        PVR_TXRLOAD_FMT_TWIDDLED | PVR_TXRLOAD_FMT_VQ);
    pvr_txr_load_kimg(&dct_texture->textureB, dct_texture->addrB, 
        PVR_TXRLOAD_FMT_TWIDDLED | PVR_TXRLOAD_FMT_VQ);
    printf("before pvr_poly_cxt_txr \n");
    // Préparer le contexte du polygone
    //pvr_poly_cxt_t cxt;
   
    pvr_poly_cxt_txr_mod(
    &dct_texture->cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_RGB565, 
    dct_texture->textureA.w, dct_texture->textureA.h, dct_texture->addrA, PVR_FILTER_BILINEAR, PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_RGB565, 
    dct_texture->textureB.w, dct_texture->textureB.h, dct_texture->addrB, PVR_FILTER_BILINEAR
    );


     dct_texture->cxt.gen.culling = PVR_CULLING_NONE; 
    dct_texture->cxt.depth.write = PVR_DEPTHWRITE_ENABLE;
    dct_texture->cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
    dct_texture->cxt.blend.src = PVR_BLEND_SRCALPHA;
    dct_texture->cxt.blend.dst = PVR_BLEND_INVSRCALPHA;
    dct_texture->cxt.blend.src_enable = PVR_BLEND_ENABLE;
    dct_texture->cxt.blend.dst_enable = PVR_BLEND_ENABLE; 
    dct_texture->cxt.txr.uv_flip = 2;
    dct_texture->cxt.txr.enable = 1;
    printf("before pvr_poly_compile\n");
    // Compiler le contexte en en-tête
    pvr_poly_mod_compile(&dct_texture->hdr, &dct_texture->cxt);

    
    return 1;
}


void unload_dct_texture(dct_texture_t *text) {

    kos_img_free(&text->textureA,1);
    kos_img_free(&text->textureB,1);

    if(text->addrA) {
        pvr_mem_free(text->addrA);
        text->addrA = NULL;
    }
    if(text->addrB) {
        pvr_mem_free(text->addrB);
        text->addrB = NULL;
    }
}
