/* KallistiOS ##version##
   Modified example with 3D cubes instead of 2D squares
   Based on the modifier volume example by Lawrence Sebald
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <dc/pvr.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>

#define NUM_CUBES 20  // Reduced number due to increased vertices per object
#define VERTICES_PER_CUBE 24  // 6 faces * 4 vertices per face

static pvr_vertex_pcm_t verts[NUM_CUBES * VERTICES_PER_CUBE];

static pvr_poly_mod_hdr_t phdr;
static pvr_mod_hdr_t mhdr, mhdr2;
static float mx = 320.0f, my = 240.0f;
static pvr_list_t list = PVR_LIST_OP_POLY;

// Helper function to set a single face of the cube
void set_cube_face(pvr_vertex_pcm_t *v, int idx, float x, float y, float z, 
                  float size, uint32 argb0, uint32 argb1, int face) {
    float half = size / 2.0f;
    
    switch(face) {
        case 0: // Front face
            v[0].x = x - half; v[0].y = y + half; v[0].z = z + half;
            v[1].x = x - half; v[1].y = y - half; v[1].z = z + half;
            v[2].x = x + half; v[2].y = y + half; v[2].z = z + half;
            v[3].x = x + half; v[3].y = y - half; v[3].z = z + half;
            break;
        case 1: // Back face
            v[0].x = x + half; v[0].y = y + half; v[0].z = z - half;
            v[1].x = x + half; v[1].y = y - half; v[1].z = z - half;
            v[2].x = x - half; v[2].y = y + half; v[2].z = z - half;
            v[3].x = x - half; v[3].y = y - half; v[3].z = z - half;
            break;
        case 2: // Top face
            v[0].x = x - half; v[0].y = y + half; v[0].z = z - half;
            v[1].x = x - half; v[1].y = y + half; v[1].z = z + half;
            v[2].x = x + half; v[2].y = y + half; v[2].z = z - half;
            v[3].x = x + half; v[3].y = y + half; v[3].z = z + half;
            break;
        case 3: // Bottom face
            v[0].x = x - half; v[0].y = y - half; v[0].z = z + half;
            v[1].x = x - half; v[1].y = y - half; v[1].z = z - half;
            v[2].x = x + half; v[2].y = y - half; v[2].z = z + half;
            v[3].x = x + half; v[3].y = y - half; v[3].z = z - half;
            break;
        case 4: // Right face
            v[0].x = x + half; v[0].y = y + half; v[0].z = z + half;
            v[1].x = x + half; v[1].y = y - half; v[1].z = z + half;
            v[2].x = x + half; v[2].y = y + half; v[2].z = z - half;
            v[3].x = x + half; v[3].y = y - half; v[3].z = z - half;
            break;
        case 5: // Left face
            v[0].x = x - half; v[0].y = y + half; v[0].z = z - half;
            v[1].x = x - half; v[1].y = y - half; v[1].z = z - half;
            v[2].x = x - half; v[2].y = y + half; v[2].z = z + half;
            v[3].x = x - half; v[3].y = y - half; v[3].z = z + half;
            break;
    }

    for(int i = 0; i < 4; i++) {
        v[i].flags = (i == 3) ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;
        v[i].argb0 = argb0;
        v[i].argb1 = argb1;
        v[i].d1 = v[i].d2 = 0;
    }
}

void setup(void) {
    pvr_poly_cxt_t cxt;
    int i;
    float x, y, z;
    uint32 argb = list == PVR_LIST_OP_POLY ? 0xFF0000FF : 0x80FF00FF;
    float cube_size = 100.0f;  // Size of each cube

    pvr_poly_cxt_col_mod(&cxt, list);
    pvr_poly_mod_compile(&phdr, &cxt);

    pvr_mod_compile(&mhdr, list + 1, PVR_MODIFIER_OTHER_POLY, PVR_CULLING_NONE);
    pvr_mod_compile(&mhdr2, list + 1, PVR_MODIFIER_INCLUDE_LAST_POLY,
                    PVR_CULLING_NONE);

    for(i = 0; i < NUM_CUBES; ++i) {
        x = rand() % 640;
        y = rand() % 480;
        z = rand() % 100 + 50;  // Increased Z range for better 3D visibility

        // Create all six faces of the cube
        for(int face = 0; face < 6; face++) {
            set_cube_face(&verts[i * VERTICES_PER_CUBE + face * 4], i, 
                         x, y, z, cube_size, argb,
                         0xFF000000 | (rand() & 0x00FFFFFF), face);
        }
    }
}

int check_start(void) {
    maple_device_t *cont;
    cont_state_t *state;
    static int taken = 0;

    cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);

    if(cont) {
        state = (cont_state_t *)maple_dev_status(cont);

        if(!state)
            return 0;

        if(state->buttons & CONT_START)
            return 1;

        if(state->buttons & CONT_DPAD_UP)
            my -= 1.0f;

        if(state->buttons & CONT_DPAD_DOWN)
            my += 1.0f;

        if(state->buttons & CONT_DPAD_LEFT)
            mx -= 1.0f;

        if(state->buttons & CONT_DPAD_RIGHT)
            mx += 1.0f;

        if((state->buttons & CONT_A) && !taken) {
            list = list == PVR_LIST_OP_POLY ? PVR_LIST_TR_POLY :
                   PVR_LIST_OP_POLY;
            setup();
            taken = 1;
        }
        else if(!(state->buttons & CONT_A)) {
            taken = 0;
        }
    }

    return 0;
}

void do_frame(void) {
    pvr_modifier_vol_t mod;
    int i, j;

    pvr_wait_ready();
    pvr_scene_begin();
    pvr_list_begin(list);

    pvr_prim(&phdr, sizeof(phdr));

    // Render all faces of all cubes
    for(i = 0; i < NUM_CUBES; ++i) {
        for(j = 0; j < VERTICES_PER_CUBE; j += 4) {
            pvr_prim(&verts[i * VERTICES_PER_CUBE + j], sizeof(pvr_vertex_pcm_t));
            pvr_prim(&verts[i * VERTICES_PER_CUBE + j + 1], sizeof(pvr_vertex_pcm_t));
            pvr_prim(&verts[i * VERTICES_PER_CUBE + j + 2], sizeof(pvr_vertex_pcm_t));
            pvr_prim(&verts[i * VERTICES_PER_CUBE + j + 3], sizeof(pvr_vertex_pcm_t));
        }
    }

    pvr_list_finish();

    // Modifier volume rendering (as a 3D volume)
    pvr_list_begin(list + 1);
    
    // First modifier volume
    pvr_prim(&mhdr, sizeof(mhdr));

    // Front faces of modifier volume
    float mv_size = 50.0f;  // Size of modifier volume
    float mv_depth = 300.0f; // Depth of modifier volume
    
    // Front face
    mod.flags = PVR_CMD_VERTEX_EOL;
    mod.ax = mx - mv_size/2;
    mod.ay = my + mv_size/2;
    mod.az = 150.0f + mv_depth/2;
    mod.bx = mx - mv_size/2;
    mod.by = my - mv_size/2;
    mod.bz = 150.0f + mv_depth/2;
    mod.cx = mx + mv_size/2;
    mod.cy = my + mv_size/2;
    mod.cz = 150.0f + mv_depth/2;
    mod.d1 = mod.d2 = mod.d3 = mod.d4 = mod.d5 = mod.d6 = 0;
    pvr_prim(&mod, sizeof(mod));

    // Second modifier for completing the volume
    pvr_prim(&mhdr2, sizeof(mhdr2));

    // Back face
    mod.flags = PVR_CMD_VERTEX_EOL;
    mod.ax = mx - mv_size/2;
    mod.ay = my - mv_size/2;
    mod.az = 150.0f + mv_depth/2;
    mod.bx = mx + mv_size/2;
    mod.by = my - mv_size/2;
    mod.bz = 150.0f + mv_depth/2;
    mod.cx = mx + mv_size/2;
    mod.cy = my + mv_size/2;
    mod.cz = 150.0f + mv_depth/2;
    mod.d1 = mod.d2 = mod.d3 = mod.d4 = mod.d5 = mod.d6 = 0;
    pvr_prim(&mod, sizeof(mod));

    pvr_list_finish();
    pvr_scene_finish();
}

static pvr_init_params_t pvr_params = {
    /* Enable Opaque and Translucent polygons + modifiers */
    {
        PVR_BINSIZE_16, PVR_BINSIZE_16, PVR_BINSIZE_16, PVR_BINSIZE_16,
        PVR_BINSIZE_0
    },
    512 * 1024  // Vertex buffer size
};

int main(int argc, char *argv[]) {
    printf("--- KallistiOS PVR 3D Cube Modifier Example ---\n");
    printf("Press A to toggle between translucent and opaque cubes.\n");
    printf("Use the DPAD to move the modifier volume (starts at (320, 240))\n");
    printf("Press Start to exit.\n");

    srand(time(NULL));

    pvr_init(&pvr_params);
    setup();

    while(!check_start()) {
        do_frame();
    }

    pvr_shutdown();

    return 0;
}