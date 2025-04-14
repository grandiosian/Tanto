#include "dct_scene.h"

static dct_scene_t currentScenePtr = {
    .name = "",                                               // String vide
    .pad1State = {{0,0,0,0,0,0,0,0,0,0.0f,0.0f,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0}},   // Structure à zéro
    .camera = {
                    .transform[0][0] = 1.0f,
                    .transform[0][1] = 0.0f,
                    .transform[0][2] = 0.0f,
                    .transform[0][3] = 0.0f,

                    .transform[1][0] = 0.0f,
                    .transform[1][1] = 1.0f,
                    .transform[1][2] = 0.0f,
                    .transform[1][3] = 0.0f,

                    .transform[2][0] = 0.0f,
                    .transform[2][1] = 0.0f,
                    .transform[2][2] = 1.0f,
                    .transform[2][3] = 0.0f,

                    .transform[3][0] = 0.0f,
                    .transform[3][1] = 0.0f,
                    .transform[3][2] = 0.0f,
                    .transform[3][3] = 1.0f,

                    .position.x = 0.0f,
                    .position.y = 0.0f,   
                    .position.z = 0.0f,
                    .position.w = 0.0f,

                    .rotation.x = 0.0f,
                    .rotation.y = 0.0f,
                    .rotation.z = 0.0f,

                    .offsetCenterScreen[0] = 640.0f/2.0f,
                    .offsetCenterScreen[1] = 480.0f/2.0f,

                    .up.x = 0.0f,
                    .up.y = 1.0f,
                    .up.z = 0.0f,
                    .up.w = 0.0f,

                    .target.x = 0.0f,
                    .target.y = 0.0f,
                    .target.z = 0.0f,
                    .target.w = 0.0f,
                    .viewAngle = 90.0f,
                    .fovDeg = 90.0/2.0f,
                    .fovRad = 0.0f,
                    .cot    = 0.0f,
                    .znear = 0.01f,
                    .zfar  = 1000.0f  
    }, 
                                              // Structure à zéro
    .ptrListGameObjects = {0},                               // Tableau à zéro
    .ptrListGUI = {0},                                       // Tableau à zéro
    .ptrListModelScene = {NULL},                             // Tableau de pointeurs à NULL
    .ptrListPvrMeshOP = {NULL},                             // Tableau de pointeurs à NULL
    .ptrListPvrMeshTR = {NULL}                              // Tableau de pointeurs à NULL
};



static int isInitialized = 0;

void initSceneSystem(void) {
    // Allocation dynamique après l'initialisation du système
    initScene(&currentScenePtr);
}

dct_scene_t* getCurrentScene(void) {
    return &currentScenePtr;
}

void initScene(dct_scene_t *sc)
{

    //initDefaultCurrentCamera(&sc->camera);
    //strcpy(sc->name, "scene courante");
    //memset(&sc->pad1State, 0, sizeof(sc->pad1State));
    //memset(&sc->camera, 0, sizeof(sc->camera));



}


void updateScene(dct_scene_t *sc)
{
    dct_input_update(0, &sc->pad1State);
    updateCurrentCamera(&sc->camera);
}


void drawScene(dct_scene_t *sc)
{
    pvr_wait_ready();

    pvr_list_begin(PVR_LIST_OP_POLY);

    // DRAW POLY MESH OPAQUE
    for (int i_obj=0; i_obj<MAX_MESHOP_SCENE; i_obj++)
    {
        drawMesh( sc->ptrListPvrMeshOP[i_obj] );
    }



    pvr_list_begin(PVR_LIST_TR_POLY);

    // DRAW POLY TRANSPARENT
     for (int i_obj=0; i_obj<MAX_MESHTR_SCENE; i_obj++)
    {
        drawMesh( sc->ptrListPvrMeshTR[i_obj] );
    }
    pvr_scene_finish();

}


int initDcEngine()
{
    // Par défaut PAL RGB 565

    if(pvr_init_defaults() == -1)
    {
        return -1;
        printf("\n[ENGINE-KOS] Error : pvr has already been initialized. \n");
    }

    pvr_set_pal_format(PVR_PAL_RGB565);


    return 1;
}
