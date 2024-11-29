#include <kos.h>
#include "data.h"

/* On utilise directement pvr_vertex_t comme structure de données */
/*
pvr_vertex_t test_vertices[] = {
    // Premier triangle
    {   // Rouge
        PVR_CMD_VERTEX,     // flags
        100.0f, 100.0f, 1.0f,  // x, y, z
        0.0f, 0.0f,            // u, v
        0xffff0000,           // argb
        0                     // oargb
    },
    {   // Vert
        PVR_CMD_VERTEX,     
        100.0f, 200.0f, 1.0f,
        0.0f, 0.0f,
        0xff00ff00,
        0
    },
    {   // Bleu
        PVR_CMD_VERTEX,     
        200.0f, 100.0f, 1.0f,
        0.0f, 0.0f,
        0xff0000ff,
        0
    },
    {   // Jaune
        PVR_CMD_VERTEX,     
        200.0f, 200.0f, 1.0f,
        0.0f, 0.0f,
        0xffffff00,
        0
    },
    {   // Magenta (avec EOL)
        PVR_CMD_VERTEX,  
        300.0f, 100.0f, 1.0f,
        0.0f, 0.0f,
        0xffff00ff,
        0
    },
    {   // Magenta (avec EOL)
        PVR_CMD_VERTEX_EOL,  
        300.0f, 200.0f, 1.0f,
        0.0f, 0.0f,
        0xff8000ff,
        0
    },

    {   // Rouge
        PVR_CMD_VERTEX,     // flags
        300.0f, 100.0f, 1.0f,  // x, y, z
        0.0f, 0.0f,            // u, v
        0xffff0000,           // argb
        0                     // oargb
    },
    {   // Vert
        PVR_CMD_VERTEX,     
        300.0f, 200.0f, 1.0f,
        0.0f, 0.0f,
        0xff00ff00,
        0
    },
    {   // Bleu
        PVR_CMD_VERTEX,     
        400.0f, 100.0f, 1.0f,
        0.0f, 0.0f,
        0xff0000ff,
        0
    },
    {   // Jaune
        PVR_CMD_VERTEX,     
        400.0f, 200.0f, 1.0f,
        0.0f, 0.0f,
        0xffffff00,
        0
    },
    {   // Magenta (avec EOL)
        PVR_CMD_VERTEX_EOL,  
        500.0f, 100.0f, 1.0f,
        0.0f, 0.0f,
        0xffff00ff,
        0
    }



};
*/

/*
pvr_vertex_t test_vertices[] = {
    {
        PVR_CMD_VERTEX,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f,
        0xFFFF0000,  // Blanc
        0.0f   // oargb aussi en blanc
    },
    {
        PVR_CMD_VERTEX,
        100.0f, 0.0f, 1.0f,
        0.0f, 0.0f,
        0xFF00FF00,  // Blanc
        0.0f
    },
    {
        PVR_CMD_VERTEX,
        50.0f, 100.0f, 1.0f,
        0.0f, 0.0f,
        0xFF0000FF,  // Blanc
        0.0f
    },
    {
        PVR_CMD_VERTEX,
        100.0f, 100.0f, 1.0f,
        0.0f, 0.0f,
        0xFF00FF00,  // Blanc
        0.0f
    },
    {
        PVR_CMD_VERTEX,
        150.0f, 100.0f, 1.0f,
        0.0f, 0.0f,
        0xFF00FF00,  // Blanc
        0.0f
    },
    {
        PVR_CMD_VERTEX_EOL,
        150.0f, 150.0f, 1.0f,
        0.0f, 0.0f,
        0xFF00FF00,  // Blanc
        0.0f
    }

};
*/

pvr_init_params_t pvr_params = {
    { PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0 },
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
    cxt.blend.dst = PVR_BLEND_ONE;
    cxt.blend.src_enable = PVR_BLEND_ENABLE;
    cxt.blend.dst_enable = PVR_BLEND_ENABLE;
    
    pvr_poly_compile(&hdr, &cxt);
}

/* Fonction pour analyser les vertices et créer les strip_info */
strip_info_t* analyze_strips(pvr_vertex_t* vertices, int total_vertices, int* num_strips) {
    strip_info_t* strips = NULL;
    int max_strips = 32;  // Valeur initiale, sera réallouée si nécessaire
    int current_strip = 0;
    int current_vertex = 0;
    
    // Allocation initiale
    strips = malloc(sizeof(strip_info_t) * max_strips);
    
    while(current_vertex < total_vertices) {
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
        
        // Si on a besoin de plus d'espace
        if(current_strip >= max_strips) {
            max_strips *= 2;
            strips = realloc(strips, sizeof(strip_info_t) * max_strips);
        }
    }
    
    *num_strips = current_strip;
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
void draw_frame(pvr_vertex_t* vertices, int total_vertices) {
    pvr_vertex_t *vert;
    pvr_dr_state_t dr_state;
    int num_strips;
    int i, j;

    mat_rotate_x(F_PI/-2.0f);
    
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
    pvr_init(&pvr_params);
    setup();

    
    // Calcul du nombre total de vertices
    int total_vertices = sizeof(test_vertices) / sizeof(pvr_vertex_t);

    int num = 0;
    strip_info_t* strips = analyze_strips(test_vertices, total_vertices, &num);
    debugStrips(strips,num);

    for (int i_v=0; i_v<total_vertices; i_v++)
    {
            test_vertices[i_v].x = (test_vertices[i_v].x * 10.0f) + 320;
            test_vertices[i_v].y = (test_vertices[i_v].y * 10.0f) + 240;
            test_vertices[i_v].z = test_vertices[i_v].z + 1.0f;
    }

    

    while(1) {
        draw_frame(test_vertices, total_vertices);
    }

    return 0;
}