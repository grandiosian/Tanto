#include <kos.h>

static pvr_vertex_t vertices[] = {
    // Centre du fan
    {PVR_CMD_VERTEX, 320.0f, 240.0f, 1.0f, 0.0f, 0.0f, 0xFF0000FF, 0.0f}, // Rouge

    // Points autour (hexagone)
    {PVR_CMD_VERTEX, 370.0f, 240.0f, 1.0f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 345.0f, 280.0f, 1.0f, 0.0f, 0.0f, 0xFF0000FF, 0.0f}, 
    {PVR_CMD_VERTEX, 295.0f, 280.0f, 1.0f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 270.0f, 240.0f, 1.0f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 295.0f, 200.0f, 1.0f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 345.0f, 200.0f, 1.0f, 0.0f, 0.0f, 0xFF0000FF, 0.0f}
};

int main() {
    pvr_init_defaults();

    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
    cxt.gen.culling = PVR_CULLING_NONE;
    pvr_poly_compile(&hdr, &cxt);

    pvr_ptr_t addr = 0xFFFFFFFF;

    printf("print addr: %p \n",(void*)addr);

    while(1) {
        pvr_wait_ready();
        pvr_scene_begin();
        pvr_list_begin(PVR_LIST_OP_POLY);

        pvr_prim(&hdr, sizeof(hdr));

        // Pour chaque triangle du fan
        for(int i = 1; i < 6; i++) { // 6 points périphériques = 4 triangles
            // Centre
            vertices[0].flags = PVR_CMD_VERTEX;
            pvr_prim(&vertices[0], sizeof(pvr_vertex_t));
            
            // Vertex courant
            vertices[i].flags = PVR_CMD_VERTEX;
            pvr_prim(&vertices[i], sizeof(pvr_vertex_t));
            
            // Vertex suivant (avec EOL)
            vertices[i+1].flags = PVR_CMD_VERTEX_EOL;
            pvr_prim(&vertices[i+1], sizeof(pvr_vertex_t));
        }

        pvr_list_finish();
        pvr_scene_finish();
    }

    return 0;
}