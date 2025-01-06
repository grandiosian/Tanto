#include <kos.h>
#include "dct_utils.h"
#include "dct_model.h"
#include "dct_camera.h"

#include "dct_scene.h"
#include "dct_guiDebugMode.h"

vec3f_t CAM_POSITION;
//extern int update_counter = 0;




int main(void) {
    pvr_init_defaults();
    pvr_set_bg_color(0.2f, 0.2f, 0.2f);

    matrix_t mat_persp;

    CAM_POSITION.x = 0.0f;
    CAM_POSITION.y = 0.0f;
    CAM_POSITION.z = -2.0f;

    dct_model_t penta;
    uint32_t c[5] = {0xFFFFFFFF, 0xFF808080,0xFF00FF00,0xFF0000FF,0xFFFF0000};
    createPentahedre(&penta,"penta",1.0f,c);
    
    dct_model_t cube;
    uint32_t c1[6] = {0xFF0000FF,0xFFFFFFFF,0xFF00FF00,0xFFFF0000,0xFF808000,0xFF008080};
    createCube( &cube,"Cube",1.0f, c1);
    cube.position.y=3.0f;

    dct_model_t sphere;
    createSphere(&sphere,"sphere",1.0f,0xFF00FF00);
    sphere.scale.x = 0.6f;
    sphere.scale.y = 0.6f;
    sphere.scale.z = 0.6f;
    sphere.position.y = -3.0f;


    dct_scene *sc;
    sc = getCurrentScene();
    sc->modelSceneCount = 3;
    sc->ptrListModelScene = (dct_model_t*)malloc(sizeof(dct_model_t*)*3);
    sc->ptrListModelScene[0] = &penta;
    sc->ptrListModelScene[1] = &cube;
    sc->ptrListModelScene[2] = &sphere;


    //setVertexColorToAllMesh(&sphere->model.meshes[0],0xFF101010 );


    matrix_t mat_dbg;


    initDefaultCurrentCamera();
    dct_camera *currentCam = getCurrentCamera();
    setCurrentCameraPosition(0.0f,0.5f,-3.0f);
    
    
    // Boucle principale
    DCT_ControllerState controller = {0};  // État du contrôleur


    initDebugInterface();
    while(1) {

        pvr_set_bg_color(0.2f, 0.2f, 0.2f);

        dct_input_update(0, &controller);
        //---------------------------------//
        //----   MATRIX TRANSFORMATION ----//
        //---------------------------------//
        //updateDebugMenu(menu, &scene, &controller);
        //displayDebugMenu(menu, &scene);
        



        updateCurrentCamera();
        //debugCurrentCamera();


        //-------------------------------------------------//
        //----   VERTEX TRANSFORMATION BASED ON MATRIX ----//
        //-------------------------------------------------//


        updateModel(&penta);
        
        updateModel(&cube);
        
        updateModel(&sphere);

        //----------------------------------//
        //----------------------------------//
        //----------------------------------//
        //------------- RENDU --------------//
        //----------------------------------//
        //----------------------------------//
        //----------------------------------//
        
        //----------------------------------//
        //---- MODEL OPAQUES EN PREMIER ----//
        //----------------------------------//
        //--------------------------------------//
        //---- MODEL TRANSPARENT EN SECOND ----//
        //--------------------------------------//

        

        pvr_wait_ready();
        pvr_scene_begin();

        renderModel(&penta);
        renderModel(&cube);
        renderModel(&sphere);
        
        drawDebugInterface();
        

        pvr_list_finish();
        pvr_scene_finish();


        

        //---------------------------//
        //---- UPDATE CONTROLLER ----//
        //---------------------------//


       

        

        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
            if(st->buttons & CONT_START)
            {
                return -1;
            }
            if(st->buttons & CONT_DPAD_UP)
            {
                //penta.rotation.y += 0.1f;


                //penta->model.rotation.y += 0.1f;
                //penta->model.position.z += 0.1f;
                //currentCam->position.z += 0.1f;
            }
            if(st->buttons & CONT_DPAD_DOWN)
            {
                //penta.rotation.y -= 0.1f;


                //penta->model.rotation.y -= 0.1f;
                //penta->model.position.z -= 0.1f;
                //currentCam->position.z -= 0.1f;
            }
            if(st->buttons & CONT_DPAD_LEFT)
            {
                //penta.rotation.x -= 0.1f;


                //penta->model.position.x -= 0.1f;
                //currentCam->position.x -= 0.1f;
            }
            if(st->buttons & CONT_DPAD_RIGHT)
            {
                //penta.rotation.x += 0.1f;


                //penta->model.position.x += 0.1f;
                //currentCam->position.x += 0.1f;
            }
            
        MAPLE_FOREACH_END()
    }

    //free(menu);

    return 0;
}