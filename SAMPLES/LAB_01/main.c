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



// Test avec les deux formats pour le même point (-1, -1, -1, 1)
void testMatrixMultiply() {
    // Format 1: Linéaire 
    float test_matrix[16] = { 
        240.0f, 0.0f, 0.0f, 0.0f,  // Première ligne
        124.61f, 314.51f, 0.3894f, 0.3894f,
        -294.74f, -127.60f, -0.9211f, -0.9211f,
        384.0f, 408.0f, 0.18f, 1.20f
    };

    // Format 2: Comme dans KOS
    float matrix_kos[4][4] = {
        {240.0f, 124.61f, -294.74f, 384.0f},    // Première colonne
        {0.0f, 314.51f, -127.60f, 408.0f},
        {0.0f, 0.3894f, -0.9211f, 0.18f},
        {0.0f, 0.3894f, -0.9211f, 1.20f}
    };

    // Point de test
    float x = -1.0f, y = -1.0f, z = 1.0f, w = 1.0f;

    // Calcul 1: Format linéaire
    float res1_x = test_matrix[0]*x + test_matrix[1]*y + test_matrix[2]*z + test_matrix[3]*w;
    float res1_y = test_matrix[4]*x + test_matrix[5]*y + test_matrix[6]*z + test_matrix[7]*w;
    float res1_z = test_matrix[8]*x + test_matrix[9]*y + test_matrix[10]*z + test_matrix[11]*w;
    float res1_w = test_matrix[12]*x + test_matrix[13]*y + test_matrix[14]*z + test_matrix[15]*w;

    printf("Format linéaire: x=%f y=%f z=%f w=%f\n", res1_x, res1_y, res1_z, res1_w);

    // Calcul 2: Format KOS
    float res2_x = matrix_kos[0][0]*x + matrix_kos[1][0]*y + matrix_kos[2][0]*z + matrix_kos[3][0]*w;
    float res2_y = matrix_kos[0][1]*x + matrix_kos[1][1]*y + matrix_kos[2][1]*z + matrix_kos[3][1]*w;
    float res2_z = matrix_kos[0][2]*x + matrix_kos[1][2]*y + matrix_kos[2][2]*z + matrix_kos[3][2]*w;
    float res2_w = matrix_kos[0][3]*x + matrix_kos[1][3]*y + matrix_kos[2][3]*z + matrix_kos[3][3]*w;

    printf("Format KOS: x=%f y=%f z=%f w=%f\n", res2_x, res2_y, res2_z, res2_w);
}
void transformVertex(float *x, float *y, float *z, float *w, matrix_t matrix) {
    // Multiplication matrice transposée
    float new_x = matrix[0][0]*(*x) + matrix[1][0]*(*y) + matrix[2][0]*(*z) + matrix[3][0];
    float new_y = matrix[0][1]*(*x) + matrix[1][1]*(*y) + matrix[2][1]*(*z) + matrix[3][1];
    float new_z = matrix[0][2]*(*x) + matrix[1][2]*(*y) + matrix[2][2]*(*z) + matrix[3][2];
    float new_w = matrix[0][3]*(*x) + matrix[1][3]*(*y) + matrix[2][3]*(*z) + matrix[3][3];

    if (new_w < 0) {
        // Trouver t où le point traverse le near plane
        float t = (0.1f - (*w)) / (new_w - (*w));  // 0.1f est le near plane
        
        // Interpoler position au near plane
        new_x = (*x) + t * (new_x - (*x));
        new_y = (*y) + t * (new_y - (*y));
        new_z = 0.1f;  // Sur le near plane
        new_w = 0.1f;  // w minimum
    }

    // Division perspective
    float w_div = new_w;
    *x = new_x / w_div;
    *y = new_y / w_div;
    *z = new_z / w_div;
    *w = 1.0f / w_div;
}
// Test
void testMatrixTransform() {
   matrix_t test_matrix = {
       {240.0f, 0.0f, 0.0f, 0.0f},
       {124.61f, 314.51f, 0.3894f, 0.3894f},
       {-294.74f, -127.60f, -0.9211f, -0.9211f},
       {384.0f, 408.0f, 0.18f, 1.20f}
   };

   float test_points[][3] = {
       {-1.0f, -1.0f, -1.0f},
       {-1.0f, -1.0f, 1.0f},
       {1.0f, 1.0f, -1.0f}
   };

   for(int i = 0; i < 3; i++) {
       float x = test_points[i][0];
       float y = test_points[i][1];
       float z = test_points[i][2];
       float w = 1.0f;

       printf("\nTest point %d (%f, %f, %f):\n", i, x, y, z);

       // Software
       printf("\nCALCUL SOFTWARE:\n");
       printf("Avant transform: x=%f y=%f z=%f w=%f\n", x, y, z, w);
       transformVertex(&x, &y, &z, &w, test_matrix);
       printf("Après transform: x=%f y=%f z=%f w=%f\n", x, y, z, w);

       // KOS
       printf("\nCALCUL KOS:\n");
       float kos_x = test_points[i][0];
       float kos_y = test_points[i][1];
       float kos_z = test_points[i][2];
       float kos_w = 1.0f;
       mat_load(&test_matrix);
       mat_trans_single4(kos_x, kos_y, kos_z, kos_w);
       printf("Transformed: x=%f y=%f z=%f w=%f\n", kos_x, kos_y, kos_z, kos_w);
   }
}

#define VRAM_ADDR 0xA5000000

void render_frame() {
    //pvr_wait_ready();
    //pvr_scene_begin();
    //pvr_set_bg_color(0.3f,0.3f,0.3f);
    // Rendu PVR normal ici
    
    //pvr_scene_finish();
    //pvr_wait_ready();

    // Accès au framebuffer
    uint16_t *vram = (uint16_t*)VRAM_ADDR;
    
    // Dessiner plusieurs lignes horizontales de différentes couleurs
    for(int y = 100; y < 300; y += 20) {
        for(int x = 100; x < 500; x++) {
            switch((y - 100) / 20) {
                case 0:
                    vram[y * 640 + x] = 0xF800;  // Rouge
                    break;
                case 1:
                    vram[y * 640 + x] = 0x07E0;  // Vert
                    break;
                case 2:
                    vram[y * 640 + x] = 0x001F;  // Bleu
                    break;
                case 3:
                    vram[y * 640 + x] = 0xFFE0;  // Jaune
                    break;
                case 4:
                    vram[y * 640 + x] = 0xF81F;  // Magenta
                    break;
                default:
                    vram[y * 640 + x] = 0xFFFF;  // Blanc
            }
        }
    }

    // Dessiner aussi quelques lignes verticales
    for(int x = 50; x < 200; x += 30) {
        for(int y = 50; y < 350; y++) {
            vram[y * 640 + x] = 0xFFFF;  // Lignes blanches
        }
    }

    vid_border_color(0, 0, 0);
}

void draw_line(float x1, float y1, float x2, float y2, uint32_t color) {
    pvr_vertex_t vertices[2];
    
    // Premier point
    vertices[0].flags = PVR_CMD_VERTEX;
    vertices[0].x = x1;
    vertices[0].y = y1;
    vertices[0].z = 1.0f;
    vertices[0].argb = color;
    vertices[0].oargb = 0;
    
    // Deuxième point
    vertices[1].flags = PVR_CMD_VERTEX_EOL; // EOL important pour la fin de la ligne
    vertices[1].x = x2;
    vertices[1].y = y2;
    vertices[1].z = 1.0f;
    vertices[1].argb = color;
    vertices[1].oargb = 0;
    
    // Soumettre au PVR
    pvr_prim(&vertices[0], 2);
}

// Dans votre boucle de rendu :
void render_scene() {
    pvr_wait_ready();
    pvr_scene_begin();
    pvr_set_bg_color(0.3f,0.3f,0.3f);
    // Liste opaque
    pvr_list_begin(PVR_LIST_OP_POLY);
    
    // Configuration du mode de rendu pour les lignes
    pvr_poly_cxt_t cxt;
    pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
    cxt.gen.culling = PVR_CULLING_NONE;
    
    pvr_poly_hdr_t hdr;
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));
    
    // Dessiner une ligne rouge
    draw_line(100.0f, 100.0f, 200.0f, 200.0f, 0xFFFF0000);
    
    pvr_list_finish();
    
    pvr_scene_finish();
}

int main() {
    pvr_init_defaults();
    testMatrixTransform();
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
    cxt.gen.culling = PVR_CULLING_NONE;
    pvr_poly_compile(&hdr, &cxt);

    pvr_ptr_t addr = 0xFFFFFFFF;

    printf("print addr: %p \n",(void*)addr);

    while(1) {
        // pvr_wait_ready();
        // pvr_scene_begin();
        // pvr_list_begin(PVR_LIST_OP_POLY);

        // pvr_prim(&hdr, sizeof(hdr));

        // // Pour chaque triangle du fan
        // for(int i = 1; i < 6; i++) { // 6 points périphériques = 4 triangles
        //     // Centre
        //     vertices[0].flags = PVR_CMD_VERTEX;
        //     pvr_prim(&vertices[0], sizeof(pvr_vertex_t));
            
        //     // Vertex courant
        //     vertices[i].flags = PVR_CMD_VERTEX;
        //     pvr_prim(&vertices[i], sizeof(pvr_vertex_t));
            
        //     // Vertex suivant (avec EOL)
        //     vertices[i+1].flags = PVR_CMD_VERTEX_EOL;
        //     pvr_prim(&vertices[i+1], sizeof(pvr_vertex_t));
        // }

        // pvr_list_finish();
        // pvr_scene_finish();

        render_scene();
    }

    return 0;
}