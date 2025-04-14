#include <kos.h>
#include "dct_utils.h"
#include "dct_model.h"
#include "dct_camera.h"

#include "dct_scene.h"

vec3f_t CAM_POSITION;

#include "dct_guiText.h"
#include "dct_input.h"


extern uint8 romdisk[];
extern uint8 romdisk_end[];



int main(void) {
    pvr_init_defaults();
    fs_romdisk_mount("/rd", romdisk, romdisk_end - romdisk);
    pvr_set_bg_color(0.2f, 0.2f, 0.2f);

    matrix_t mat_persp;

    CAM_POSITION.x = 0.0f;
    CAM_POSITION.y = 0.0f;
    CAM_POSITION.z = -10.0f;
    printf("start \n");
    dct_model_t penta;
    uint32_t c[5] = {0xFFFFFFFF, 0xFF808080,0xFF00FF00,0xFF0000FF,0xFFFF0000};
    createPentahedre(&penta,"penta",1.0f,c);
    
    dct_model_t cube;
    uint32_t c1[6] = {0xFF0000FF,0xFFFFFFFF,0xFF00FF00,0xFFFF0000,0xFF808000,0xFF008080};
    createCube( &cube,"Cube",1.0f, c1);
    cube.position.x=-5.0f;

    dct_model_t sphere;
    createSphere(&sphere,"sphere",1.0f,0xFF00FF00);
    sphere.scale.x = 0.6f;
    sphere.scale.y = 0.6f;
    sphere.scale.z = 0.6f;
    sphere.position.x = 5.0f;


    dct_scene_t *sc;
    sc = getCurrentScene();
    //sc->modelSceneCount = 3;
    //sc->ptrListModelScene = (dct_model_t*)malloc(sizeof(dct_model_t*)*3);
    sc->ptrListModelScene[0] = &penta;
    sc->ptrListModelScene[1] = &cube;
    sc->ptrListModelScene[2] = &sphere;


    //setVertexColorToAllMesh(&sphere->model.meshes[0],0xFF101010 );


    matrix_t mat_dbg;


    
    dct_camera_t *currentCam = getCurrentCamera();
    initDefaultCurrentCamera(currentCam);
    setCurrentCameraPosition(currentCam,0.0f,0.5f,-10.0f);
    
    BitmapFont* font = dct_init_font("/rd", "/rd/Arial.fnt");
    if (!font) return 1;

    TextProperties props = {
        .x = 20.0f,
        .y = 20.0f,
        .scale = 1.0f,
        .color = 0xFFFFFFFF
    };

    // Boucle principale
    dct_controllerState_t controller = {0};  // État du contrôleur


    //initDebugInterface();
    while(1) {

        pvr_set_bg_color(0.2f, 0.2f, 0.2f);

        dct_input_update(0, &controller);
        //---------------------------------//
        //----   MATRIX TRANSFORMATION ----//
        //---------------------------------//
        //updateDebugMenu(menu, &scene, &controller);
        //displayDebugMenu(menu, &scene);

        updateCurrentCamera(currentCam);

        //-------------------------------------------------//
        //----   VERTEX TRANSFORMATION BASED ON MATRIX ----//
        //-------------------------------------------------//

        cube.rotation.z = fmod(cube.rotation.z + 0.01f, (2.0f * F_PI));
        cube.rotation.x = fmod(cube.rotation.x + 0.01f, (2.0f * F_PI));
        penta.rotation.y = fmod(penta.rotation.y + 0.01f, (2.0f * F_PI));
        sphere.rotation.x = fmod(sphere.rotation.x + 0.01f, (2.0f * F_PI));

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
        
        //drawDebugInterface();
        

        pvr_list_finish();

        pvr_list_begin(PVR_LIST_TR_POLY);
        dct_draw_text(font,"Demo Basic 3d Solid \n\\c[A0A0A0]Dpad move camera  " ,props);
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
                currentCam->position.z += 0.1f;
            }
            if(st->buttons & CONT_DPAD_DOWN)
            {
                //penta.rotation.y -= 0.1f;


                //penta->model.rotation.y -= 0.1f;
                //penta->model.position.z -= 0.1f;
                currentCam->position.z -= 0.1f;
            }
            if(st->buttons & CONT_DPAD_LEFT)
            {
                //penta.rotation.x -= 0.1f;


                //penta->model.position.x -= 0.1f;
                currentCam->position.x += 0.1f;
            }
            if(st->buttons & CONT_DPAD_RIGHT)
            {
                //penta.rotation.x += 0.1f;


                //penta->model.position.x += 0.1f;
                currentCam->position.x -= 0.1f;
            }
            
        MAPLE_FOREACH_END()
    }

    //free(menu);

    return 0;
}