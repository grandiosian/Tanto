#ifndef DCT_ENGINE_H
#define DCT_ENGINE_H


#define MAX_MODELS_SCENE 10
#define MAX_MESHOP_SCENE 20
#define MAX_MESHTR_SCENE 20
#define MAX_GUIS_SCENE 10

#include <kos.h>
#include "dct_utils.h"
#include "dct_model.h"
#include "dct_input.h"
#include "dct_camera.h"

//---------------------------------------//
//---------------------------------------//
//------   DREAMCAST TANTO ENGINE  ------//
//---------------------------------------//
//---------------------------------------//


typedef struct dct_scene
{
    char                          name[30];

    dct_controllerState_t         pad1State;
    dct_camera_t                  camera;

    uint32_t                      ptrListGameObjects[MAX_MODELS_SCENE];
    uint32_t                      ptrListGUI[MAX_GUIS_SCENE];             // UI usually stay the same between scenes
    
    dct_model_t                   *ptrListModelScene[MAX_MODELS_SCENE];

    // RENDERING PVR MODEL
    // LIST MESH OPAQUE
    dct_mesh_t                    *ptrListPvrMeshOP[MAX_MESHOP_SCENE];
    // LIST MESH TRANSPARENT
    dct_mesh_t                    *ptrListPvrMeshTR[MAX_MESHTR_SCENE];

    // RENDERING SPRITES


} dct_scene_t;





void initSceneSystem(void);

dct_scene_t* getCurrentScene();
void initScene(dct_scene_t *sc);
void updateScene(dct_scene_t *sc);
void drawScene(dct_scene_t *sc);
int initDcEngine();


#endif


