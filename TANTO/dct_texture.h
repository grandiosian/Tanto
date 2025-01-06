#ifndef DCT_TEXTURE_H
#define DCT_TEXTURE_H

#include <kos.h>
#include <kmg/kmg.h>

typedef struct
{
    uint32         width;
    uint32         height;
    pvr_ptr_t      addr; 

    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;

    kos_img_t      texture;  

} dct_texture_t;


int load_dct_texture(dct_texture_t *dct_texture, const char *fname);
void unload_dct_texture(dct_texture_t *texture);


#endif