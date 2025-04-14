#include <kos.h>
#include "data.h"
#include "dct_guiText.h"
#include "dct_input.h"

extern uint8 romdisk[];
extern uint8 romdisk_end[];

pvr_init_params_t pvr_params = {
    { PVR_BINSIZE_16, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0 },
    512 * 1024
};

pvr_poly_hdr_t hdr;


typedef struct {
    int start_index;     
    int vertex_count;    
} strip_info_t;


void setup(void) {
    pvr_poly_cxt_t cxt;
    pvr_init(&pvr_params);
    pvr_set_bg_color(0.2f, 0.2f, 0.2f);

    pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
    cxt.gen.alpha = PVR_ALPHA_DISABLE;
    cxt.gen.shading = PVR_SHADE_GOURAUD;
    cxt.gen.fog_type = PVR_FOG_DISABLE;
    cxt.gen.culling = PVR_CULLING_NONE;
    cxt.gen.color_clamp = PVR_CLRCLAMP_DISABLE;
    cxt.gen.clip_mode = PVR_USERCLIP_DISABLE;
    cxt.depth.comparison = PVR_DEPTHCMP_ALWAYS;
    cxt.depth.write = PVR_DEPTHWRITE_DISABLE;
    cxt.blend.src = PVR_BLEND_ONE;
    cxt.blend.dst = PVR_BLEND_ZERO;
    cxt.blend.src_enable = PVR_BLEND_DISABLE;
    cxt.blend.dst_enable = PVR_BLEND_DISABLE;
    
    pvr_poly_compile(&hdr, &cxt);
}
/* Fonction pour analyser les vertices et créer les strip_info sans utiliser realloc */
strip_info_t* analyze_strips(pvr_vertex_t* vertices, int total_vertices, int* num_strips) {
    int strip_count = 0;
    int current_vertex = 0;
    strip_info_t* strips = NULL;
    
    // Première passe: compter le nombre de strips
    while(current_vertex < total_vertices) {
        strip_count++;
        
        // Saute tous les vertices jusqu'au EOL
        while(current_vertex < total_vertices) {
            if(vertices[current_vertex].flags == PVR_CMD_VERTEX_EOL) {
                current_vertex++;
                break;
            }
            current_vertex++;
        }
    }
    
    // Allocation avec le nombre exact de strips
    strips = memalign(32, sizeof(strip_info_t) * strip_count);
    if(!strips) {
        *num_strips = 0;
        return NULL;
    }
    
    // Deuxième passe: remplir les informations des strips
    current_vertex = 0;
    int current_strip = 0;
    
    while(current_vertex < total_vertices && current_strip < strip_count) {
        // Début d'une nouvelle bande
        strips[current_strip].start_index = current_vertex;
        strips[current_strip].vertex_count = 0;
        
        // Compte les vertices jusqu'au EOL
        while(current_vertex < total_vertices) {
            strips[current_strip].vertex_count++;
            
            // Si on trouve un EOL, c'est la fin de la bande
            if(vertices[current_vertex].flags == PVR_CMD_VERTEX_EOL) {
                current_vertex++;
                break;
            }
            current_vertex++;
        }
        
        current_strip++;
    }
    
    *num_strips = strip_count;
    return strips;
}


void debugStrips(strip_info_t* strips, int num_strips)
{
    printf("START >>> [DEBUG] (STRIP) \n");
    for (int i=0; i<num_strips; i++)
    {
        printf("strip %d start index :%d vertCount :%d  \n",i,strips[i].start_index, strips[i].vertex_count);
    }
    printf("[DEBUG] (STRIP) <<< END \n");
}

/* Exemple d'utilisation */
void draw_frame(vec3f_t pos, vec3f_t rot, vec3f_t scale, pvr_vertex_t* vertices, int total_vertices,pvr_vertex_t *originalVtx) {
    pvr_vertex_t *vert;
    pvr_dr_state_t dr_state;
    int num_strips;
    int i, j;
    mat_identity();
    //mat_rotate_x(F_PI/-2.0f);
    mat_translate(320.0f,240.0f,1.0f);
    mat_scale(10.0f,10.0f,10.0F);
    
    
    mat_translate(pos.x,pos.y,pos.z);
    mat_rotate(rot.x,rot.y,rot.z);
    mat_scale(scale.x,scale.y,scale.z);

    for( int i=0; i<total_vertices; i++)
    {   
        float x = originalVtx[i].x;
        float y = originalVtx[i].y;
        float z = originalVtx[i].z;
        
        mat_trans_single3_nodiv(x,y,z);
        vertices[i].x = x;
        vertices[i].y = y;
        vertices[i].z = z;


    }
    // Analyse automatique des strips
    strip_info_t* strips = analyze_strips(vertices, total_vertices, &num_strips);
    
    
    
    vid_border_color(10, 10, 10);
    pvr_wait_ready();
    //vid_border_color(255, 0, 0);
    pvr_scene_begin();
    pvr_list_begin(PVR_LIST_OP_POLY);
    pvr_prim(&hdr, sizeof(hdr));
    pvr_dr_init(&dr_state);
    // Dessin de chaque strip
    for(i = 0; i < num_strips; i++) {
        
        
        for(j = 0; j < strips[i].vertex_count; j++) {
            vert = pvr_dr_target(dr_state);
            *vert = vertices[strips[i].start_index + j];
            pvr_dr_commit(vert);
        }
    }

    pvr_list_finish();
    pvr_scene_finish();
    //vid_border_color(0, 255, 0);
    
    // Libération de la mémoire
    free(strips);
}

// Exemple d'appel
int main(int argc, char **argv) {
    //pvr_init(&pvr_params);
    setup();
    fs_romdisk_mount("/rd", romdisk, romdisk_end - romdisk);
    vec3f_t pos = {0.0f,0.0f,0.0f};
    vec3f_t rot = {0.0f,0.0f,0.0f};
    vec3f_t scale = {1.0f,1.0f,1.0f};
    // Calcul du nombre total de vertices
    int total_vertices = sizeof(test_vertices) / sizeof(pvr_vertex_t);

    int num = 0;
    strip_info_t* strips = analyze_strips(test_vertices, total_vertices, &num);
    debugStrips(strips,num);
    printf(" total_vertices :%d \n",total_vertices);
    printf("sizeof test_vertices :%d \n",sizeof(test_vertices));

    pvr_vertex_t *originalVtx = (pvr_vertex_t*)memalign(32,sizeof(pvr_vertex_t)*total_vertices);
    memcpy(originalVtx,test_vertices,sizeof(test_vertices));

    // for (int i_v=0; i_v<total_vertices; i_v++)
    // {
    //         originalVtx[i_v].x = (originalVtx[i_v].x * 10.0f) + 320;
    //         originalVtx[i_v].y = (originalVtx[i_v].y * 10.0f) + 240;
    //         originalVtx[i_v].z = originalVtx[i_v].z + 1.0f;

    //         test_vertices[i_v].x = (test_vertices[i_v].x * 10.0f) + 320;
    //         test_vertices[i_v].y = (test_vertices[i_v].y * 10.0f) + 240;
    //         test_vertices[i_v].z = test_vertices[i_v].z + 1.0f;
    // }

    BitmapFont* font = dct_init_font("/rd", "/rd/Arial.fnt");
    if (!font) return 1;

    TextProperties props = {
        .x = 50.0f,
        .y = 50.0f,
        .scale = 1.0f,
        .color = 0xFFFFFFFF
    };

    while(1) {
        pvr_vertex_t *vert;
        pvr_dr_state_t dr_state;
        int num_strips;
        int i, j;
        mat_identity();
        //mat_rotate_x(F_PI/-2.0f);
        mat_translate(320.0f,240.0f,1.0f);
        mat_scale(10.0f,10.0f,10.0F);
        
        
        mat_translate(pos.x,pos.y,pos.z);
        mat_rotate(rot.x,rot.y,rot.z);
        mat_scale(scale.x,scale.y,scale.z);

        for( int i=0; i<total_vertices; i++)
        {   
            float x = originalVtx[i].x;
            float y = originalVtx[i].y;
            float z = originalVtx[i].z;
            
            mat_trans_single3_nodiv(x,y,z);
            test_vertices[i].x = x;
            test_vertices[i].y = y;
            test_vertices[i].z = z;


        }
        // Analyse automatique des strips
        strip_info_t* strips = analyze_strips(test_vertices, total_vertices, &num_strips);
        
        
        
        vid_border_color(10, 10, 10);
        pvr_wait_ready();
        //vid_border_color(255, 0, 0);
        pvr_scene_begin();
        pvr_list_begin(PVR_LIST_OP_POLY);
        pvr_prim(&hdr, sizeof(hdr));
        pvr_dr_init(&dr_state);
        // Dessin de chaque strip
        for(i = 0; i < num_strips; i++) {
            
            
            for(j = 0; j < strips[i].vertex_count; j++) {
                vert = pvr_dr_target(dr_state);
                *vert = test_vertices[strips[i].start_index + j];
                pvr_dr_commit(vert);
            }
        }
        
        pvr_list_finish();

        // pvr_list_begin(PVR_LIST_TR_POLY);
        
        // dct_draw_text(font,"Gui texte \\c[FF0000]Rouge \\c[00FF00]Vert \\c[0000FF]Bleu \n \\c[FFFFFF]Ligne 2 \n Ligne 3 " ,props);
        
        // pvr_list_finish();

        pvr_scene_finish();
        //vid_border_color(0, 255, 0);
        
        // Libération de la mémoire
        free(strips);
        

        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
            if(st->buttons & CONT_START)
            {
                return -1;
            }
            if(st->buttons & CONT_DPAD_UP)
            {
                pos.y -= 1.0F;
                printf("pos %f %f %f \n",pos.x,pos.y,pos.z);
            }
            if(st->buttons & CONT_DPAD_DOWN)
            {
                pos.y += 1.0F;
                printf("pos %f %f %f \n",pos.x,pos.y,pos.z);
            }
            if(st->buttons & CONT_DPAD_LEFT)
            {
                pos.x -= 1.0F;
                printf("pos %f %f %f \n",pos.x,pos.y,pos.z);
            }
            if(st->buttons & CONT_DPAD_RIGHT)
            {
                pos.x += 1.0F;
                printf("pos %f %f %f \n",pos.x,pos.y,pos.z);
            }
            if(st->joyy > 64)
            {
                scale.y += 0.1f;
            }
            if(st->joyy < -64)
            {
                scale.y -= 0.1f;
            }
            if(st->joyx < -64)
            {
                scale.x += 0.1f;
            }
            if(st->joyx > 64)
            {
                scale.x -= 0.1f;
            }
            if(st->ltrig > 20)
            {
                rot.z += 0.1F;
            }
            if(st->rtrig > 20)
            {
                rot.z -= 0.1F;
            }
        MAPLE_FOREACH_END()
    }

    return 0;
}