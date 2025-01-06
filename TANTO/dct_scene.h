#ifndef DCT_ENGINE_H
#define DCT_ENGINE_H

#include <kos.h>
#include "dct_utils.h"
#include "dct_model.h"

//---------------------------------------//
//---------------------------------------//
//------   DREAMCAST TANTO ENGINE  ------//
//---------------------------------------//
//---------------------------------------//


typedef struct 
{
    char            name[30];
    uint32_t        gameObjectsCount;
    uint32_t        *ptrListGameObjects;
    uint32_t        GUICount;   
    uint32_t        *ptrListGUI;             // UI usually stay the same between scenes
    maple_device_t  *controller1; 
    maple_device_t  *controller2;
    maple_device_t  *controller3;
    maple_device_t  *controller4;

    uint32_t        modelSceneCount;
    dct_model_t     **ptrListModelScene;

    // RENDERING PVR MODEL
    // LIST MESH OPAQUE
    uint32_t        pvrMeshOPCount;
    dct_mesh_t      **ptrListPvrMeshOP;
    // LIST MESH TRANSPARENT
    uint32_t        pvrMeshTRCount;
    dct_mesh_t      **ptrListPvrMeshTR;

    // RENDERING SPRITES


} dct_scene;

static dct_scene currentScene = {
    .name = "scene courante",
    .gameObjectsCount = 0,
    .ptrListGameObjects = NULL,
    .GUICount = 0,
    .ptrListGUI = NULL,
    .controller1 = NULL,
    .controller2 = NULL,
    .controller3 = NULL,
    .controller4 = NULL,
    .pvrMeshOPCount = 0,
    .ptrListPvrMeshOP = NULL,
    .pvrMeshTRCount = 0,
    .ptrListPvrMeshTR = NULL

};

dct_scene* getCurrentScene()
{
    return &currentScene;
}


void createScene(dct_scene *sc)
{





}




void drawScene(dct_scene *sc)
{
    pvr_wait_ready();

    pvr_list_begin(PVR_LIST_OP_POLY);

    // DRAW POLY MESH OPAQUE
    for (int i_obj=0; i_obj<sc->pvrMeshOPCount; i_obj++)
    {
        drawMesh( sc->ptrListPvrMeshOP[i_obj] );
    }



    pvr_list_begin(PVR_LIST_TR_POLY);

    // DRAW POLY TRANSPARENT
     for (int i_obj=0; i_obj<sc->pvrMeshTRCount; i_obj++)
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






#endif


