#include <kos.h>
#include "dct_utils.h"
#include "dct_model.h"
#include "dct_camera.h"
#include "dct_types.h"
#include "dct_scene.h"
#include "dct_guiText.h"
#include "dct_input.h"


extern uint8 romdisk[];
extern uint8 romdisk_end[];

vec3f_t CAM_POSITION;




int main(void) {
    //pvr_init_defaults();
    
    pvr_init_params_t params = 
    {
        { PVR_BINSIZE_16, PVR_BINSIZE_16, PVR_BINSIZE_16, PVR_BINSIZE_16, PVR_BINSIZE_0 },
        512*1024,
        0,
        0,
        0
    };
    pvr_init(&params);
    fs_romdisk_mount("/rd", romdisk, romdisk_end - romdisk);

    pvr_set_bg_color(0.2f, 0.2f, 0.2f);

    matrix_t mat_persp;

    CAM_POSITION.x = 0.0f;
    CAM_POSITION.y = 0.0f;
    CAM_POSITION.z = -2.0f;
    printf("start \n");
    dct_model_t penta;
    uint32_t c[5] = {0xFFFFFFFF, 0xFF808080,0xFF00FF00,0xFF0000FF,0xFFFF0000};
    createPentahedre(&penta,"penta",1.0f,c);
    


    dct_model_t cube;
    
    uint32_t c1[6] = {0xFF0000FF,0xFFFFFFFF,0xFF00FF00,0xFFFF0000,0xFF808000,0xFF008080};
    uint32_t c2[6] = {0xFF000030,0xFF303030,0xFF003000,0xFF300000,0xFF303000,0xFF003030};
    uint32_t c3[6] = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};
    createCube( &cube,"Cube",1.0f, c1);
    cube.position.y=0.0f;
    cube.rotation.y= 1.0f;

    cube.modifier = createModifierVolumeCube();

    cube.meshes[0].type = MESH_TPCM_OP;
    cube.meshes[0].renderVtx_PCM = (pvr_vertex_pcm_t*)memalign(32,sizeof(pvr_vertex_pcm_t)*cube.meshes[0].vtxCount);
    cube.meshes[0].renderVtx_TPCM = (pvr_vertex_tpcm_t*)memalign(32,sizeof(pvr_vertex_tpcm_t)*cube.meshes[0].vtxCount);
    cube.meshes[0].currentCxtSelected = 0;
    int i_3 =0;
    for (int i_v=0; i_v < cube.meshes[0].vtxCount; i_v++)
    {
        printf("couleur rgb indice c: %d \n",i_3-1);
        if(i_v%6==0){i_3+=1;}
        printf("increment i_3 %d \n",i_3);
        cube.meshes[0].renderVtx_PCM[i_v].flags = cube.meshes[0].originalVtx[i_v].flags;
        cube.meshes[0].renderVtx_PCM[i_v].x = cube.meshes[0].originalVtx[i_v].x;
        cube.meshes[0].renderVtx_PCM[i_v].y = cube.meshes[0].originalVtx[i_v].y;
        cube.meshes[0].renderVtx_PCM[i_v].z = cube.meshes[0].originalVtx[i_v].z;

        cube.meshes[0].renderVtx_PCM[i_v].argb0 = cube.meshes[0].originalVtx[i_v].argb;
        //cube.meshes[0].renderVtx_PCM[i_v].argb0 = 0xFFFF0000;
        cube.meshes[0].renderVtx_PCM[i_v].argb1 = c2[i_3-1];
        cube.meshes[0].renderVtx_PCM[i_v].d1 = 0;
        cube.meshes[0].renderVtx_PCM[i_v].d2 = 0;



        cube.meshes[0].renderVtx_TPCM[i_v].flags = cube.meshes[0].originalVtx[i_v].flags;
        cube.meshes[0].renderVtx_TPCM[i_v].x = cube.meshes[0].originalVtx[i_v].x;
        cube.meshes[0].renderVtx_TPCM[i_v].y = cube.meshes[0].originalVtx[i_v].y;
        cube.meshes[0].renderVtx_TPCM[i_v].z = cube.meshes[0].originalVtx[i_v].z;
        cube.meshes[0].renderVtx_TPCM[i_v].u0 = cube.meshes[0].originalVtx[i_v].u;
        cube.meshes[0].renderVtx_TPCM[i_v].v0 = cube.meshes[0].originalVtx[i_v].v;
        cube.meshes[0].renderVtx_TPCM[i_v].u1 = cube.meshes[0].originalVtx[i_v].u;
        cube.meshes[0].renderVtx_TPCM[i_v].v1 = cube.meshes[0].originalVtx[i_v].v;

        cube.meshes[0].renderVtx_TPCM[i_v].argb0 = cube.meshes[0].originalVtx[i_v].argb;
        cube.meshes[0].renderVtx_TPCM[i_v].oargb0 = cube.meshes[0].originalVtx[i_v].oargb;
        cube.meshes[0].renderVtx_TPCM[i_v].argb1 = cube.meshes[0].originalVtx[i_v].argb;
        cube.meshes[0].renderVtx_TPCM[i_v].oargb1 = cube.meshes[0].originalVtx[i_v].oargb;
        //cube.meshes[0].renderVtx_PCM[i_v].argb0 = 0xFFFF0000;
        cube.meshes[0].renderVtx_TPCM[i_v].argb1 = c2[i_3-1];
        cube.meshes[0].renderVtx_TPCM[i_v].d1 = 0;
        cube.meshes[0].renderVtx_TPCM[i_v].d2 = 0;
        
    }
    printf("cube.meshes[0].vtxCount %d \n",cube.meshes[0].vtxCount);
    for (int i_v=0; i_v < cube.meshes[0].vtxCount; i_v++)
    {
        pvr_vertex_pcm_t v = cube.meshes[0].renderVtx_PCM[i_v];
        pvr_vertex_tpcm_t v1 = cube.meshes[0].renderVtx_TPCM[i_v];
    
        printf("pvr vertext pcm %x %f %f %f %x %x \n",v.flags, v.x, v.y, v.z, v.argb0, v.argb1);
        printf("pvr vertext tpcm %x %f %f %f %x %x \n",v1.flags, v1.x, v1.y, v1.z, v1.argb0, v1.argb1);
    }
    cube.texturesCount= 1;
    cube.textures = malloc(sizeof(dct_texture_t)*cube.texturesCount);
    
    //dct_texture_t texture_TPCM;
    load_dct_textureTPCM(&cube.textures[0], "/rd/Pave_stylized_128x128.kmg","/rd/VMU_BAKE_256.kmg");



    dct_model_t sphere;
    createSphere(&sphere,"sphere",1.0f,0xFF00FF00);
    sphere.scale.x = 0.6f;
    sphere.scale.y = 0.6f;
    sphere.scale.z = 0.6f;
    sphere.position.y = -3.0f;


    dct_scene_t *sc;
    sc = getCurrentScene();
    //sc->modelSceneCount = 3;
    //sc->ptrListModelScene = (dct_model_t*)malloc(sizeof(dct_model_t*)*3);
    sc->ptrListModelScene[0] = &penta;
    sc->ptrListModelScene[1] = &cube;
    sc->ptrListModelScene[2] = &sphere;


    //setVertexColorToAllMesh(&sphere->model.meshes[0],0xFF101010 );


    matrix_t mat_dbg;

    bool modifierVisibity = false;
    
    dct_camera_t *currentCam = getCurrentCamera();
    initDefaultCurrentCamera(currentCam);
    setCurrentCameraPosition(currentCam,0.0f,0.5f,-3.0f);
    
    
    // Boucle principale
    dct_controllerState_t controller = {0};  // État du contrôleur

    BitmapFont* font = dct_init_font("/rd", "/rd/Arial.fnt");
    if (!font) return 1;

    TextProperties props = {
        .x = 20.0f,
        .y = 20.0f,
        .scale = 1.0f,
        .color = 0xFFFFFFFF
    };

    //initDebugInterface();
    while(1) {

        pvr_set_bg_color(0.2f, 0.2f, 0.2f);

        dct_input_update(0, &controller);
        //---------------------------------//
        //----   MATRIX TRANSFORMATION ----//
        //---------------------------------//
        


        updateCurrentCamera(currentCam);


        //-------------------------------------------------//
        //----   VERTEX TRANSFORMATION BASED ON MATRIX ----//
        //-------------------------------------------------//


        
        updateModel(&cube);
        

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
        
        renderModel(&cube);
        
        pvr_list_finish();

        

        pvr_list_begin(PVR_LIST_OP_MOD);
        applyModifier(&cube.modifier);
        pvr_list_finish();


        pvr_list_begin(PVR_LIST_TR_POLY);
        dct_draw_text(font,"Demo 3d Modifier With 2 textures \n\\c[A0A0A0]Dpad to move modifier(cube volume) and rotate cube \nButton A to display modifier" ,props);

         if(modifierVisibity)
        {
            debugRenderModifier(&cube,1);
        }
       
        pvr_list_finish();



        pvr_scene_finish();


        

        //---------------------------//
        //---- UPDATE CONTROLLER ----//
        //---------------------------//



        if(controller.just_pressed.a)
        {
            modifierVisibity= !modifierVisibity;
        }
        

        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
            if(st->buttons & CONT_START)
            {
                return -1;
            }
            if(st->buttons & CONT_DPAD_UP)
            {
                cube.rotation.y += 0.1f;
                cube.modifier.transform[3][2] = cube.modifier.transform[3][2]-0.1f;
            }
            if(st->buttons & CONT_DPAD_DOWN)
            {
                cube.rotation.y -= 0.1f;
                cube.modifier.transform[3][2] = cube.modifier.transform[3][2]+0.1f;
            }
            if(st->buttons & CONT_DPAD_LEFT)
            {
                cube.rotation.x += 0.1f;
                cube.modifier.transform[3][0] = cube.modifier.transform[3][0]+0.1f;
            }
            if(st->buttons & CONT_DPAD_RIGHT)
            {
                cube.modifier.transform[3][0] = cube.modifier.transform[3][0]-0.1f;
                cube.rotation.x -= 0.1f;
            }
            
        MAPLE_FOREACH_END()
    }

    //free(menu);

    return 0;
}