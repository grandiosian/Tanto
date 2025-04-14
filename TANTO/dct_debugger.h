// dct_debugMenu.h
#ifndef DCT_DEBUGGER_H
#define DCT_DEBUGGER_H

#include <kos.h>
#include <dc/pvr.h>
#include "dct_utils.h"
#include "dct_types.h"
#include "dct_scene.h"
#include "dct_input.h"
#include "dct_model.h"
#include "dct_fontdebugger.h"


#define NUM_ENTITIES_TO_DRAW 10
#define NUM_MAIN_MENU_ELEMENTS 20
#define NUM_BREADCRUMB_ELEMENTS 5
#define NUM_ENTITIES 200
#define NUM_CHILDREN_ENTITIES 200
#define DEBUG_PRINT_ENABLE 0

// MODEL-1
// MODEL-1 POS
// MODEL-1 ROT
// MODEL-1 SCALE
// MESHES
// MESH-1
// MESH-1 CXT
// ORIGINAL VTX
// RENDER VTX


// MODEL-1
// --- MODEL-1 POS
// --- MODEL-1 ROT
// --- MODEL-1 SCALE
// --- MESHES
// ------ MESH-1
// -----------MESH-1 CXT
// -----------ORIGINAL VTX
// -----------RENDER VTX


// -------------------------------------- //
// ---------- STATS DREAMCAST ----------- //
/* -------------------------------------- //
Mémoire totale disponible : 16777216 octets (16 MB)
Mémoire allouée max : 16223256 octets
Mémoire système (KOS) : 553960 octets (≈ 541 KB)
PVR RAM:
Libre: 5653 KB
AICA RAM totale: 2MB
//-----------------------------------------*/
// ----------------------------------------//


typedef struct 
{
    uint64_t       frame_last_time;     /**< \brief Ready-to-Ready length for the last frame in nanoseconds */
    uint64_t       reg_last_time;       /**< \brief Registration time for the last frame in nanoseconds */
    uint64_t       rnd_last_time;       /**< \brief Rendering time for the last frame in nanoseconds */
    uint64_t       buf_last_time;       /**< \brief DMA buffer file time for the last frame in nanoseconds */
    size_t         frame_count;         /**< \brief Total number of rendered/viewed frames */
    size_t         vbl_count;           /**< \brief VBlank count */
    size_t         vtx_buffer_used;     /**< \brief Number of bytes used in the vertex buffer for the last frame */
    size_t         vtx_buffer_used_max; /**< \brief Number of bytes used in the vertex buffer for the largest frame */
    float          frame_rate;          /**< \brief Current frame rate (per second) */
    
    size_t         total_ram;    // RAM totale disponible
    size_t         used_ram;     // RAM utilisée
    uint32_t       pvr_free;   // PVR libre
    float          ratioPvrRamAvailUsed;
    float          ratioRamAvailUsed;
} perf_stats_t;


typedef enum 
{
    OFF,
    MINIFY,
    DISPLAY,
    NAVIGATE,
    EDIT

} dct_debug_state;

typedef struct dct_menu_entityGameObject
{
    DCT_DEBUG_DATA_TYPE type;
    uint32_t            levelMenu;
    uint32_t            *data;
    int                 indiceCurrentDataSelected;
    int                 currentDataSelectedCount;
    uint32_t            dataIndiceList; // si l'entité courante est un element d'une liste voici l'indice
    uint32_t            *prevEntSibling;
    uint32_t            *nextEntSibling;
    uint32_t            *parentEntity;
    
    uint32_t            *childEntities[NUM_CHILDREN_ENTITIES];
    int                 childEntitiesCount;
    
}dct_menu_entityGameObject;


typedef struct dct_menu_debugger_t
{
    perf_stats_t              stats;
    char                      textePvrStats[32][32];

    dct_menu_entityGameObject breadCrumb[NUM_BREADCRUMB_ELEMENTS]; //fil d'ariane ex: models>modelname>meshes>vtx>
    int                       breadCrumbLength;
    char                      breadCrumbTexte[64];

    dct_menu_entityGameObject switchBtn;
    dct_menu_entityGameObject MainMenu[NUM_MAIN_MENU_ELEMENTS];

    
    dct_menu_entityGameObject entities[NUM_ENTITIES];
    int                       entityCount;
    
    
    dct_menu_entityGameObject *currentEntitySelected;
    

    dct_menu_entityGameObject *entitiesToDraw[NUM_ENTITIES_TO_DRAW];
    bool                      state; // 0: VIEW  1: EDIT 
    
    dct_fontInfo              *font;

    dct_textureAlpha          *textureDisplay;
    dct_textureAlpha          *textureMiniBtnSwitch;
    dct_textureAlpha          *textureHelper;
    char                      currentTextToDisplay[4096];

    dct_model_t               *currentModelSelected;

    dct_debug_state           currentState;


} dct_menu_debugger_t;


//static dct_menu_debugger_t debugger;


dct_menu_debugger_t* testDebug();

dct_menu_debugger_t* getDebuggerMenu();
void draw_rect_tr(float x, float y, float width, float height, uint32_t color);
void freeMenuEntityGameObject(dct_menu_entityGameObject *ent);
void freeDebugMode();
void freeDebugMenu();

bool isCurrentEntitySelectedInMainMenu();
bool isInEntitiesToDraw(dct_menu_entityGameObject *ent);
void calculateNumberOfParentEntityOfCurrentEntitySelected(dct_menu_entityGameObject *ent, char* temp_buffer, size_t buffer_size);
void createAllTextureToDraw();

dct_menu_debugger_t* createMenuDebugger();
void initCurrentEntitySelected();
void compileCxtHdrOfCurrentModelSelected(dct_menu_entityGameObject *ent);
void refreshEntitiesToDraw();
void updateTexteTextureBreadcrumb();
void updateTexteTextureSwitch();
void updateTexteTextureMain();
void updateDebuggerStateNavigate(dct_controllerState_t *controller);
void updateDebuggerStateEdit(dct_controllerState_t *controller);
void updateDebuggerStateMinify(dct_controllerState_t *controller);
void updateDebugger(dct_controllerState_t *controller);
void drawTextureDebugMode();



#endif