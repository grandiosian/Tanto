#include <kos.h>
#include "dct_utils.h"
#include "dct_model.h"
#include "dct_camera.h"
#include "dct_scene.h"
#include "dct_skinning.h"
#include "dct_debugger.h"

vec3f_t CAM_POSITION;

extern uint8 romdisk[];
extern uint8 romdisk_end[];



int main(void) {
    pvr_init_defaults();
    fs_romdisk_mount("/rd", romdisk, romdisk_end - romdisk);
    pvr_set_bg_color(0.2f, 0.2f, 0.2f);

    
    //testInternalMatrix();
    printf("\n\n START GAME \n\n");


    
    dct_model_t *suzanne = load_pvr("/rd/models/Suzanne_02.pvr");
    suzanne->meshes[0].shading = true;
    suzanne->meshes[1].shading = true;
    dct_animation_init(&suzanne->anim_state, &suzanne->animations[0]);
    dct_animation_play(&suzanne->anim_state);
    dct_animation_set_loop(&suzanne->anim_state, true);
    //debug_print_model(suzanne);

    //printf("\n\ndebug mat After Debug print model bone %s \n\n",suzanne->armature->bones[3].name);
    //print_matrix(&suzanne->armature->bones[3].localMatrix);





    // dct_model_t *padDreamcast = load_pvr("/cd/models/PAD_DREAMCAST.pvr");
    // padDreamcast->rotation.x = 3.14f/2.0f;
    // debug_print_model(padDreamcast);
    // padDreamcast->meshes[0].currentCxtSelected = 0;
    // padDreamcast->meshes[1].currentCxtSelected = 1;
    // padDreamcast->meshes[0].shadingContrast = 1.0f;
    // padDreamcast->meshes[1].shadingContrast = 1.0f;
    // recompileAllTexturesModelHeader(padDreamcast);
    // for(int i=0; i<padDreamcast->anim_count; i++)
    // {
    //     printf("animation %s \n",padDreamcast->animations[i].name);
        
    // }
    // dct_animation_init(&padDreamcast->anim_state, &padDreamcast->animations[0]);
    // dct_animation_play(&padDreamcast->anim_state);
    // dct_animation_set_loop(&padDreamcast->anim_state, true);



    // dct_model_t *Triangle = load_pvr("/cd/models/TRIANGLE.pvr");
    // dct_animation_init(&Triangle->anim_state, &Triangle->animations[0]);
    // dct_animation_play(&Triangle->anim_state);
    // dct_animation_set_loop(&Triangle->anim_state, true);
    //debug_print_model(Triangle);
    
    dct_camera_t *currentCam = getCurrentCamera();
    initDefaultCurrentCamera(currentCam);
    setCurrentCameraPosition(currentCam,0.0f,0.5f,-3.0f);

  
    // dct_model_t penta;
    // uint32_t c1[5] = {0xFFFFFFFF, 0xFF808080,0xFF00FF00,0xFF0000FF,0xFFFF0000};
    // createPentahedre(&penta,"pentahedre",1.0f,c1);
    

    // dct_model_t cube;
    // uint32_t c2[6] = {0xFFFFFFFF, 0xFF808080,0xFF00FF00,0xFF0000FF,0xFFFF0000,0xFFFF00FF};
    // createCube(&cube,"cube01",1.0f,c2);
    // cube.position.z = 3.1f;
    // cube.rotation.x = 0.4f;

    //load_dct_texture(&cube.textures[0],"/cd/Pave_stylized_128x128.kmg"); 

    // dct_model_t triangle;
    // uint32_t c3[3] = {0xFFFF0000, 0xFF00FF00,0xFF0000FF};
    // createPrimitiveTriangle(&triangle,"triangle",-1.0f, 0.0f, 1.0f, 1.0f, 0.0f,1.0f, 0.0f, -1.0f, -5.0f  , c3);
    

    

    // --  SCENE FOR DEBUGGER 
    dct_scene_t *sc;
    sc = getCurrentScene();
    //sc->modelSceneCount = 1;
    //sc->ptrListModelScene = (dct_model_t*)malloc(sizeof(dct_model_t*)*sc->modelSceneCount);
    sc->ptrListModelScene[0] = suzanne;
    //sc->ptrListModelScene[1] = &triangle;
    //sc->ptrListModelScene[2] = &cube;
    //sc->ptrListModelScene[3] = padDreamcast;



    // -- DEBUGGER 
    
    //dct_menu_debugger_t *debugger = getDebuggerMenu();
    // dct_menu_debugger_t *debugger = createMenuDebugger();
    // debugger->currentState = MINIFY;


    float angleY = 0.0f;

    



    dct_controllerState_t controller = {0};  // État du contrôleur
    
    matrix_t dbg ={0};

    matrix_t m1 = 
    {
        {  0.0,  -1.0,   0.0,   0.0 },
        {  1.0,   0.0,   0.0,   0.0 },
        {  0.0,   0.0,   1.0,   0.0 },
        {  0.2,  -0.4,   0.6,   1.0 }
    };

    matrix_t m2 = 
    {
        {  0.0,   0.0,   1.0,   0.0 },
        {  0.0,  -1.0,   0.0,   0.0 },
        { -1.0,   0.0,   0.0,   0.0 },
        {  0.5,   0.0,   0.0,   1.0 }
    };


    mat_identity();
    //mat_apply(suzanne->armature->bones[0].children[0]->bindMatrix);
    //mat_apply(suzanne->armature->bones[0].bindMatrix);
    
    
    mat_apply(m1);
    mat_apply(m2);
    mat_store(&dbg);
    print_matrix(dbg);

    mat_identity();
    mat_rotate_x(3.14f/2.0f);
    mat_store(&dbg);
    printf("matrix rotate X 90\n");
    print_matrix(dbg);
    mat_identity();
    mat_rotate_y(3.14f/2.0f);
    printf("matrix rotate Y 90\n");
    mat_store(&dbg);
    print_matrix(dbg);
    mat_identity();
    mat_rotate_z(3.14f/2.0f);
    printf("matrix rotate Z 90\n");
    mat_store(&dbg);
    print_matrix(dbg);

    printf("matrix armature bones bind matrix \n");


    //debug_print_model(suzanne);


    bool activation_animation = false;
    
 
    while(1) {

        pvr_set_bg_color(0.2f, 0.2f, 0.2f);

        dct_input_update(0, &controller);
        //---------------------------------//
        //----   MATRIX TRANSFORMATION ----//
        //---------------------------------//

        
        //updateDebugger(&controller);

        
        updateCurrentCamera(currentCam);
        //cube.rotation.y = fmod(cube.rotation.y + 0.01f, (2.0f * F_PI));

        //updateModel(&cube); 

        // updateModel(&triangle); 
        // updateModel(&penta); 


        // updateModel(Triangle);
        //dct_animation_update(&Triangle->anim_state,0.016f,Triangle);
        // dct_armature_update(Triangle);
        // dct_deform_mesh(&Triangle->meshes[0],Triangle->armature);
        
        //updateModel(padDreamcast); 

        //updateModel(suzanne);
        updateModel_optimize(suzanne);
        
        if(activation_animation)
        {
            dct_animation_update(&suzanne->anim_state,0.016f,suzanne);
        }
        
        dct_armature_update(suzanne);
        dct_deform_model(suzanne);

        
        //dct_deform_mesh(&suzanne->meshes[0],suzanne->armature);
        //dct_deform_mesh(&suzanne->meshes[1],suzanne->armature);


        pvr_wait_ready();
        pvr_scene_begin();

        //renderModel(&cube);
        // renderModel(&triangle);
        // renderModel(&penta); 
        //renderModel(Triangle);
        renderModel(suzanne);
        //renderModel(padDreamcast);

        pvr_list_begin(PVR_LIST_OP_POLY);


        //dct_draw_armature(Triangle->armature);
        //dct_draw_armature(suzanne->armature);
        dct_draw_gizmo();

        pvr_list_finish();

        //drawTextureDebugMode();
        

        
        pvr_scene_finish();


        

        //---------------------------//
        //---- UPDATE CONTROLLER ----//
        //---------------------------//

        if(controller.just_pressed.a)
        {
            switch (activation_animation)
            {
                case true:
                    activation_animation = false;
                    break;
                case false:
                    activation_animation = true;
                    break;
            }
            
            //dct_deform_mesh(&Triangle->meshes[0],Triangle->armature);
            //printAllArmature(suzanne->armature->bones);
            //Triangle->DEBUG_MODEL_VTX = true;
            //cube.DEBUG_MODEL_VTX = true;
            //suzanne->DEBUG_RENDER_VTX = true;
        }else
        {
            //Triangle->DEBUG_MODEL_VTX = false;
            //cube.DEBUG_MODEL_VTX = false;
            //suzanne->DEBUG_RENDER_VTX = false;
        }

      
        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        
        if(st->buttons & CONT_DPAD_UP )
            {
                suzanne->rotation.x += 0.1f;
            }
        if(st->buttons & CONT_DPAD_DOWN )
            {
                suzanne->rotation.x -= 0.1f;
            }
             if(st->buttons & CONT_DPAD_LEFT )
            {
                suzanne->rotation.y += 0.1f;
                }
                if(st->buttons & CONT_DPAD_RIGHT )
            {
                suzanne->rotation.y -= 0.1f;
                }


        // if(debugger->currentState != EDIT && debugger->currentState != NAVIGATE)
        // {
        //     if(st->buttons & CONT_DPAD_UP )
        //     {
        //         suzanne->rotation.x += 0.1f;
        //         // suzanne->armature->bones[0].localMatrix[12] += 0.1f*suzanne->armature->bones[0].localMatrix[4];
        //         // suzanne->armature->bones[0].localMatrix[13] += 0.1f*suzanne->armature->bones[0].localMatrix[5];
        //         // suzanne->armature->bones[0].localMatrix[14] += 0.1f*suzanne->armature->bones[0].localMatrix[6];
        //         //currentCam->position.z += 0.1f;
        //         // cube.position.z += 0.1f;
        //         // triangle.position.z += 0.1f;
        //         //print_matrix(currentCam->transform);
        //         // currentCam->viewAngle += 0.1;
        //         // printf("valuangle to apply :%f \n",currentCam->viewAngle);
        //         // setCurrentCameraFOV(currentCam,currentCam->viewAngle);
                
        //     }
        //     if(st->buttons & CONT_DPAD_DOWN )
        //     {
        //         suzanne->rotation.x -= 0.1f;
        //         // float m[16] = {0};
        //         // mat_identity();
        //         // mat_apply(suzanne->armature->bones[1].localMatrix);
        //         // mat_rotate_x(-0.1f);
        //         // mat_store(&suzanne->armature->bones[1].localMatrix);


        //         //suzanne->armature->bones[0].localMatrix[13] += 0.1f;
        //         //currentCam->position.z -= 0.1f;
        //         // cube.position.z -= 0.1f;
        //         // triangle.position.z -= 0.1f;
        //         //print_matrix(currentCam->transform);
        //         // currentCam->viewAngle -= 0.1f;
        //         // printf("valuangle to apply :%f \n",currentCam->viewAngle);
        //         // setCurrentCameraFOV(currentCam,currentCam->viewAngle);
        //     }

        //     if(st->buttons & CONT_DPAD_LEFT )
        //     {
        //         suzanne->rotation.y += 0.1f;
        //         //mat_store(&suzanne->armature->bones[0].finalMatrix);

        //         //currentCam->position.x += 0.1f;
        //         //currentCam->rotation.y += 0.1f;

        //     }
        //     if(st->buttons & CONT_DPAD_RIGHT )
        //     {
        //         suzanne->rotation.y -= 0.1f;
        //         //mat_store(&suzanne->armature->bones[0].localMatrix);
        //         //mat_store(&suzanne->armature->bones[1].finalMatrix);
        //         //currentCam->position.x -= 0.1f;
        //         //currentCam->rotation.y -= 0.1f;
        //     }

        // }
        
            
        MAPLE_FOREACH_END()
    }


    return 0;
}