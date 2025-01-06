#include <kos.h>
#include "dct_utils.h"
#include "dct_model.h"
#include "dct_camera.h"
#include "dct_scene.h"
#include "dct_debugger.h"

vec3f_t CAM_POSITION;



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

int main(void) {
    pvr_init_defaults();
    pvr_set_bg_color(0.2f, 0.2f, 0.2f);

    testMatrixTransform();

    printf("\n\n START GAME \n\n");

    initDefaultCurrentCamera();
    dct_camera *currentCam = getCurrentCamera();
    setCurrentCameraPosition(0.0f,0.5f,-3.0f);

  
    dct_model_t penta;
    uint32_t c1[5] = {0xFFFFFFFF, 0xFF808080,0xFF00FF00,0xFF0000FF,0xFFFF0000};
    createPentahedre(&penta,"pentahedre",1.0f,c1);
    
    

    dct_model_t cube;
    uint32_t c2[6] = {0xFFFFFFFF, 0xFF808080,0xFF00FF00,0xFF0000FF,0xFFFF0000,0xFFFF00FF};
    createCube(&cube,"cube01",1.0f,c2);
    cube.position.z = 3.1f;
    cube.rotation.x = 0.4f;

    load_dct_texture(&cube.texture01,"/cd/Pave_stylized_128x128.kmg"); 

    dct_model_t triangle;
    uint32_t c3[3] = {0xFFFF0000, 0xFF00FF00,0xFF0000FF};
    createPrimitiveTriangle(&triangle,"triangle",-1.0f, 0.0f, 1.0f, 1.0f, 0.0f,1.0f, 0.0f, -1.0f, -5.0f  , c3);
    


    
    dct_scene *sc;
    sc = getCurrentScene();
    sc->modelSceneCount = 3;
    sc->ptrListModelScene = (dct_model_t*)malloc(sizeof(dct_model_t*)*3);
    sc->ptrListModelScene[0] = &penta;
    sc->ptrListModelScene[1] = &triangle;
    sc->ptrListModelScene[2] = &cube;

    float angleY = 0.0f;

    createMenuDebugTest();

    dct_model_t *mod = sc->ptrListModelScene[0];
    
    // Boucle principale
    DCT_ControllerState controller = {0};  // État du contrôleur

    dct_menu_debugTest *debugger = getDebuggerMenu();
 
    while(1) {

        pvr_set_bg_color(0.2f, 0.2f, 0.2f);

        dct_input_update(0, &controller);
        //---------------------------------//
        //----   MATRIX TRANSFORMATION ----//
        //---------------------------------//

        
        updateDebugger(&controller);

        
        updateCurrentCamera();
        //cube.rotation.y = fmod(cube.rotation.y + 0.01f, (2.0f * F_PI));
        updateModel(&cube); 
        //updateModel(&triangle); 

        pvr_wait_ready();
        pvr_scene_begin();

        renderModel(&cube);
        //renderModel(&triangle);
        pvr_list_begin(PVR_LIST_OP_POLY);

        pvr_list_finish();

        drawTextureDebugMode();
        pvr_list_finish();

        
        pvr_scene_finish();


        

        //---------------------------//
        //---- UPDATE CONTROLLER ----//
        //---------------------------//

        if(controller.just_pressed.a)
        {
            triangle.DEBUG_MODEL_VTX = true;
            cube.DEBUG_MODEL_VTX = true;
        }else
        {
            triangle.DEBUG_MODEL_VTX = false;
            cube.DEBUG_MODEL_VTX = false;
        }

      
        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        
        
        if(debugger->currentState != EDIT && debugger->currentState != NAVIGATE)
        {
            if(st->buttons & CONT_DPAD_UP )
            {
                cube.position.z += 0.1f;
                triangle.position.z += 0.1f;
            }
            if(st->buttons & CONT_DPAD_DOWN )
            {
                cube.position.z -= 0.1f;
                triangle.position.z -= 0.1f;
            }

        }
        
            
        MAPLE_FOREACH_END()
    }


    return 0;
}