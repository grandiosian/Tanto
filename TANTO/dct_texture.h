#ifndef DCT_TEXTURE_H
#define DCT_TEXTURE_H

#include <kos.h>
#include <kmg/kmg.h>
#include "dct_types.h"

typedef struct pvr_poly_cxt_t
{
        DCT_IDENT          ident;
        pvr_poly_cxt_t     cxt;
} dct_cxt_t;


typedef struct
{
    DCT_IDENT        ident;
    char             checker[13]; // DCT_DATA_TYPE
    DCT_TEXTURE_TYPE type;

    uint32         width;
    uint32         height;
    
    pvr_ptr_t      addrA;
    pvr_ptr_t      addrB;

    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;

    kos_img_t      textureA; // La texture de base utilisé couramment
    kos_img_t      textureB; // pour TPCM qui fonctionne  en tandem avec 2 textures masquées


} dct_texture_t;


int load_dct_texture(dct_texture_t *dct_texture, dct_cxt_t *pcxt , pvr_poly_hdr_t *phdr, const char *fname);
int load_dct_textureTPCM(dct_texture_t *dct_texture, const char *fnameA, const char *fnameB);
void unload_dct_texture(dct_texture_t *texture);


#endif