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
#include "dct_fontDebugMode.h"


#define NUM_ENTITIES_TO_DRAW 10
#define NUM_MAIN_MENU_ELEMENTS 2
#define NUM_BREADCRUMB_ELEMENTS 5

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
    
    uint32_t            **childEntities;
    int                 childEntitiesCount;
    
}dct_menu_entityGameObject;


typedef struct dct_menu_debugTest
{
    perf_stats_t              stats;
    char                      textePvrStats[32][32];

    dct_menu_entityGameObject breadCrumb[NUM_BREADCRUMB_ELEMENTS]; //fil d'ariane ex: models>modelname>meshes>vtx>
    int                       breadCrumbLength;
    char                      breadCrumbTexte[64];

    dct_menu_entityGameObject switchBtn;
    dct_menu_entityGameObject MainMenu[NUM_MAIN_MENU_ELEMENTS];

    
    dct_menu_entityGameObject *entities;
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


} dct_menu_debugTest;


static dct_menu_debugTest debugger;

static DCT_ControllerState controller = {0};


dct_menu_debugTest* getDebuggerMenu()
{
    return &debugger;
}



void draw_rect_tr(float x, float y, float width, float height, uint32_t color) {
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

    pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    vert.flags = PVR_CMD_VERTEX;
    vert.x = x;
    vert.y = y;
    vert.z = 10.0f;
    vert.argb = color;
    pvr_prim(&vert, sizeof(vert));

    vert.flags = PVR_CMD_VERTEX;
    vert.x = x + width;
    vert.y = y;
    pvr_prim(&vert, sizeof(vert));

    vert.flags = PVR_CMD_VERTEX;
    vert.x = x;
    vert.y = y + height;
    pvr_prim(&vert, sizeof(vert));

    vert.flags = PVR_CMD_VERTEX_EOL;
    vert.x = x + width;
    vert.y = y + height;
    pvr_prim(&vert, sizeof(vert));
}


void freeMenuEntityGameObject(dct_menu_entityGameObject *ent)
{
    for( int i=0; i<ent->childEntitiesCount; i++)
    {
        free(ent->childEntities[i]);
        ent->childEntities[i] = NULL;
    }
    free(ent->childEntities);
    ent->childEntities = NULL;

}

void freeDebugMode()
{
    freeMenuEntityGameObject(&debugger.switchBtn);
    freeMenuEntityGameObject(debugger.entities);
    freeMenuEntityGameObject(debugger.currentEntitySelected);
}

void freeDebugMenu() {
    // Libérer les textures
    if (debugger.textureDisplay) {
        free_alpha_texture(debugger.textureDisplay);
        debugger.textureDisplay = NULL;
    }
    
    if (debugger.textureMiniBtnSwitch) {
        free_alpha_texture(debugger.textureMiniBtnSwitch);
        debugger.textureMiniBtnSwitch = NULL;
    }
    
    if (debugger.textureHelper) {
        free_alpha_texture(debugger.textureHelper);
        debugger.textureHelper = NULL;
    }
    
    // Libérer la police
    if (debugger.font) {
        if (debugger.font->bitmap) {
            free(debugger.font->bitmap);
        }
        free(debugger.font);
        debugger.font = NULL;
    }
    
    // Libérer les entities 
    if (debugger.entities) {
        // Libérer d'abord les childEntities de chaque entité
        for (int i = 0; i < debugger.entityCount; i++) {
            if (debugger.entities[i].childEntities) {
                free(debugger.entities[i].childEntities);
                debugger.entities[i].childEntities = NULL;
            }
        }
        free(debugger.entities);
        debugger.entities = NULL;
    }
    
    // Libérer les childEntities du menu principal et du switch button
    if (debugger.switchBtn.childEntities) {
        free(debugger.switchBtn.childEntities);
        debugger.switchBtn.childEntities = NULL;
    }
    
    // Reset des pointeurs
    debugger.currentEntitySelected = NULL;
    
    for (int i = 0; i < NUM_ENTITIES_TO_DRAW; i++) {
        debugger.entitiesToDraw[i] = NULL;
    }
    
    // Réinitialiser les autres variables
    debugger.entityCount = 0;
    debugger.breadCrumbLength = 0;
    debugger.currentTextToDisplay[0] = '\0';
    debugger.breadCrumbTexte[0] = '\0';
}


bool isCurrentEntitySelectedInMainMenu()
{

    for (int i=0; i<NUM_MAIN_MENU_ELEMENTS; i++)
    {   
        dct_menu_entityGameObject *entity = &debugger.MainMenu[i];
        if(entity == debugger.currentEntitySelected)
        {
            return true;
        }
    }
    return false;
}

bool isInEntitiesToDraw(dct_menu_entityGameObject *ent)
{
    for (int i=0; i<NUM_ENTITIES_TO_DRAW; i++)
    {

        if(debugger.entitiesToDraw[i]==ent)
        {
            return true;
        }
    }
    return false;
}


void calculateNumberOfParentEntityOfCurrentEntitySelected(dct_menu_entityGameObject *ent, char* temp_buffer, size_t buffer_size)
{
    if(ent == NULL || ent->parentEntity == NULL) return;

    // Récursivement remonter jusqu'à la racine d'abord
    calculateNumberOfParentEntityOfCurrentEntitySelected(ent->parentEntity, temp_buffer, buffer_size);
    
    // Maintenant, en redescendant, on ajoute chaque niveau
    dct_menu_entityGameObject *currEntParent = ent->parentEntity;
    char l[30] = {0};
    
    switch(currEntParent->type)
    {
        case NONE:
            snprintf(l, sizeof(l), "NONE");
            break;
        case DCT_DBG_MENU_PRINCIPAL_NAME:
            snprintf(l, sizeof(l), "%s",currEntParent->data);
            break;
        case DCT_DBG_MODEL:
            snprintf(l, sizeof(l), "MODEL");
            break;
        case DCT_DBG_MODEL_NAME:
            dct_model_t* n = (dct_model_t*)currEntParent->data;
            snprintf(l, sizeof(l), "MODE-%s", n->name);
            break;
        case DCT_DBG_MODEL_MESHES:
            snprintf(l, sizeof(l), "MESHES");
            break;
        case DCT_DBG_MODEL_MESH_INDICE:
            int* d = (int*)currEntParent->data;
            snprintf(l, sizeof(l), "MESH-%d", *d);
            break;
        case DCT_DBG_MODEL_MESH_ORIGINALVTX:
            snprintf(l, sizeof(l), "ORIGVTX");
            break;
        case DCT_DBG_MODEL_MESH_ANIMATEDVTX:
            snprintf(l, sizeof(l), "ANIMVTX");
            break;
        case DCT_DBG_MODEL_MESH_RENDERVTX:
            snprintf(l, sizeof(l), "RENDVTX");
            break;
        case DCT_DBG_MODEL_MESH_ORIGINALVTX_VTX:
            snprintf(l, sizeof(l), "VTX");
            break;
        case DCT_DBG_MODEL_MESH_HDR:
            snprintf(l, sizeof(l), "HDR");
            break;
        case DCT_DBG_MODEL_MESH_CXT:
            snprintf(l, sizeof(l), "CXT");
            break;
    }
    
    debugger.breadCrumbLength++;
    // Ajouter le séparateur et le label au buffer temporaire
    size_t current_len = strlen(temp_buffer);
    snprintf(temp_buffer + current_len, buffer_size - current_len, "/%s", l);
}


void createAllTextureToDraw()
{
    debugger.font = init_basic_font();
    if (!debugger.font) {
        printf("Font init failed\n");
        return;
    }
    
    char fill[4096];
    for (int i=0;i<52;i++)
    {
        strcat(fill,"                                                                \n");
    }
        

    debugger.textureDisplay = create_text_texture_with_colors( fill, debugger.font, 0xE05FACFF,512,512);
    debugger.textureMiniBtnSwitch = create_text_texture_with_colors("\\c[0080FF]TANTO ENGINE FPS\nMEM DEBUGGER RAM\nMIN 16KO     PVR\nMAX 540KO %5d", debugger.font, 0xE05FACFF,128,32 );
    debugger.textureHelper = create_text_texture_with_colors("PRESS [->] EXPAND                                               ",debugger.font, 0xE05FACFF,512,8);
}   

void createMenuDebugTest()
{
    dct_scene *sc = getCurrentScene();
    if(sc==NULL){ printf("\n pointeur null \n");}

    debugger.currentState = MINIFY;
    debugger.MainMenu[0].type = DCT_DBG_MENU_PRINCIPAL_NAME;
    debugger.MainMenu[0].data = "LOAD SCENE";
    debugger.MainMenu[0].parentEntity = &debugger.switchBtn;

    debugger.MainMenu[1].type = DCT_DBG_MENU_PRINCIPAL_NAME;
    debugger.MainMenu[1].data = "SCENE GAME-OBJS";
    debugger.MainMenu[1].parentEntity = &debugger.switchBtn;

    //debugger.MainMenu[2].type = NONE;
    //debugger.MainMenu[3].type = NONE;

    debugger.MainMenu[0].nextEntSibling = &debugger.MainMenu[1];
    debugger.MainMenu[0].prevEntSibling = NULL;
    debugger.MainMenu[1].nextEntSibling = NULL;
    debugger.MainMenu[1].prevEntSibling = &debugger.MainMenu[0];

    debugger.switchBtn.childEntitiesCount = 2;
    debugger.switchBtn.childEntities = (dct_menu_entityGameObject*)malloc(sizeof(dct_menu_entityGameObject*)*debugger.switchBtn.childEntitiesCount);
    debugger.switchBtn.childEntities[0] = &debugger.MainMenu[0];
    debugger.switchBtn.childEntities[1] = &debugger.MainMenu[1];

    // -------------------------------------------------- //
    // ---------------   COUNTER ENTITY ----------------- // 
    // -------------------------------------------------- //
    int countEntity = 0;
    for (int i_model=0; i_model<sc->modelSceneCount; i_model++)
    {
        dct_model_t *m = sc->ptrListModelScene[i_model];
        countEntity += 1; // name
        countEntity += 1; // pos
        countEntity += 1; // rot
        countEntity += 1; // scale
        countEntity += 1; // meshes
        for (int i_mesh=0; i_mesh<m->meshesCount; i_mesh++ )
        {
            countEntity += 1; // mesh name
            countEntity += 1; // orig vtx
            countEntity += 1; // anim vtx
            countEntity += 1; // render vtx
            countEntity += 1; // cxt 
            countEntity += 1; // hdr
            dct_mesh_t *mesh = &m->meshes[i_mesh];

            for(int i_vtx=0; i_vtx<mesh->vtxCount; i_vtx++)
            {
                // 0 1 2 3 4 5 6 7 8 9 10 11 12 13
                pvr_vertex_t *vtx = &mesh->originalVtx[i_vtx];
                if(i_vtx<9 || i_vtx>=mesh->vtxCount-9 )
                {
                    countEntity += 1; // pvr vtx 1
                }
                
            }
            for(int i_vtx=0; i_vtx<mesh->vtxCount; i_vtx++)
            {
                pvr_vertex_t *vtx = &mesh->originalVtx[i_vtx];
                if(i_vtx<9 || i_vtx>=mesh->vtxCount-9 )
                {
                    countEntity += 1; // pvr vtx 1
                }
                
            }
            for(int i_vtx=0; i_vtx<mesh->vtxCount; i_vtx++)
            {
                pvr_vertex_t *vtx = &mesh->originalVtx[i_vtx];
                
                if(i_vtx<9 || i_vtx>=mesh->vtxCount-9 )
                {
                    countEntity += 1; // pvr vtx 1
                }
                
            }

        }
    }

    // -------------------------------------------------- //
    // ---------------  CREATION ENTITY ----------------- // 
    // -------------------------------------------------- //
    debugger.entities = (dct_menu_entityGameObject*)malloc(sizeof(dct_menu_entityGameObject)*countEntity);
    debugger.entityCount = countEntity;
    
    countEntity = 0;
    
    // -------------------------------------------------- //
    // --------------- FILL DATA ENTITY ----------------- // 
    // -------------------------------------------------- //

    for (int i_model=0; i_model<sc->modelSceneCount; i_model++)
    {
        dct_model_t *m = sc->ptrListModelScene[i_model];
        
        debugger.entities[countEntity].prevEntSibling = NULL;
        debugger.entities[countEntity].nextEntSibling = NULL;
        uint32_t *entityAddr_model = &debugger.entities[countEntity];


        debugger.entities[countEntity].levelMenu = 0;
        debugger.entities[countEntity].indiceCurrentDataSelected = -1;
        debugger.entities[countEntity].currentDataSelectedCount = 0;
        debugger.entities[countEntity].type = DCT_DBG_MODEL_NAME;
        debugger.entities[countEntity].parentEntity = &debugger.MainMenu[1];
        debugger.entities[countEntity].childEntitiesCount = 0;
        debugger.entities[countEntity].childEntities = NULL;
        
        
        debugger.entities[countEntity].data = &m->name;
        countEntity += 1; // name

        uint32_t *entityAddr_modelPos = &debugger.entities[countEntity];

        debugger.entities[countEntity].prevEntSibling = NULL;
        debugger.entities[countEntity].indiceCurrentDataSelected = -1;
        debugger.entities[countEntity].currentDataSelectedCount = 3-1;
        debugger.entities[countEntity].levelMenu = 1;
        debugger.entities[countEntity].type = DCT_DBG_MODEL_POSITION;
        debugger.entities[countEntity].parentEntity = entityAddr_model;
        debugger.entities[countEntity].childEntities = NULL;
        debugger.entities[countEntity].childEntitiesCount = 0;
        debugger.entities[countEntity].data = &m->position;


        countEntity += 1; // pos

        debugger.entities[countEntity-1].nextEntSibling = &debugger.entities[countEntity];
        debugger.entities[countEntity].prevEntSibling = entityAddr_modelPos;
        debugger.entities[countEntity].indiceCurrentDataSelected = -1;
        debugger.entities[countEntity].currentDataSelectedCount = 3-1;
        debugger.entities[countEntity].levelMenu = 1;
        debugger.entities[countEntity].type = DCT_DBG_MODEL_ROTATION;
        debugger.entities[countEntity].parentEntity = entityAddr_model;
        debugger.entities[countEntity].childEntities = NULL;
        debugger.entities[countEntity].childEntitiesCount = 0;
        debugger.entities[countEntity].data = &m->rotation;
        countEntity += 1; // rot

        debugger.entities[countEntity-1].nextEntSibling = &debugger.entities[countEntity];
        debugger.entities[countEntity].prevEntSibling = &debugger.entities[countEntity-1];
        debugger.entities[countEntity].indiceCurrentDataSelected = -1;
        debugger.entities[countEntity].currentDataSelectedCount = 3-1;
        debugger.entities[countEntity].levelMenu = 1;
        debugger.entities[countEntity].type = DCT_DBG_MODEL_SCALE;
        debugger.entities[countEntity].parentEntity = entityAddr_model;
        debugger.entities[countEntity].childEntities = NULL;
        debugger.entities[countEntity].childEntitiesCount = 0;
        debugger.entities[countEntity].data = &m->scale;
        countEntity += 1; // scale

        uint32_t *entityAddr_modelMeshes = &debugger.entities[countEntity];

        debugger.entities[countEntity-1].nextEntSibling = &debugger.entities[countEntity];
        debugger.entities[countEntity].prevEntSibling = &debugger.entities[countEntity-1];
        debugger.entities[countEntity].indiceCurrentDataSelected = -1;
        debugger.entities[countEntity].currentDataSelectedCount = 0;
        debugger.entities[countEntity].levelMenu = 1;
        debugger.entities[countEntity].type = DCT_DBG_MODEL_MESHES;
        debugger.entities[countEntity].parentEntity = entityAddr_model;
        debugger.entities[countEntity].childEntities = NULL;
        debugger.entities[countEntity].childEntitiesCount = 0;
        debugger.entities[countEntity].data = m->meshes;
        debugger.entities[countEntity].nextEntSibling = NULL;
        countEntity += 1; // meshes
        for (int i_mesh=0; i_mesh<m->meshesCount; i_mesh++ )
        {
            dct_mesh_t *mesh = &m->meshes[i_mesh];
            uint32_t *entityAddr_modelMesh = &debugger.entities[countEntity];
            debugger.entities[countEntity].prevEntSibling = NULL;
            debugger.entities[countEntity].indiceCurrentDataSelected = -1;
            debugger.entities[countEntity].currentDataSelectedCount = 0;
            debugger.entities[countEntity].levelMenu = 2;
            debugger.entities[countEntity].type = DCT_DBG_MODEL_MESH_INDICE;
            debugger.entities[countEntity].parentEntity = entityAddr_modelMeshes;
            debugger.entities[countEntity].childEntities = NULL;
            debugger.entities[countEntity].childEntitiesCount = 0;
            debugger.entities[countEntity].data = &mesh->indice;
            debugger.entities[countEntity].dataIndiceList = &mesh->indice;
            debugger.entities[countEntity].nextEntSibling = NULL;
            debugger.entities[countEntity].prevEntSibling = NULL;
            countEntity += 1; // mesh name

            debugger.entities[countEntity].prevEntSibling = NULL;
            debugger.entities[countEntity].indiceCurrentDataSelected = -1;
            debugger.entities[countEntity].currentDataSelectedCount = 0;
            uint32_t *entityAddr_modelMeshOrigVtx = &debugger.entities[countEntity];
            debugger.entities[countEntity].levelMenu = 3;
            debugger.entities[countEntity].type = DCT_DBG_MODEL_MESH_ORIGINALVTX;
            debugger.entities[countEntity].parentEntity = entityAddr_modelMesh;
            debugger.entities[countEntity].childEntities = NULL;
            debugger.entities[countEntity].childEntitiesCount = 0;
            debugger.entities[countEntity].data = mesh->originalVtx;
            

            countEntity += 1; // orig vtx
            
            uint32_t *entityAddr_modelMeshAnimVtx = &debugger.entities[countEntity];
            debugger.entities[countEntity].indiceCurrentDataSelected = -1;
            debugger.entities[countEntity].currentDataSelectedCount = 0;
            debugger.entities[countEntity-1].nextEntSibling = &debugger.entities[countEntity];
            debugger.entities[countEntity].prevEntSibling = &debugger.entities[countEntity-1];

            debugger.entities[countEntity].levelMenu = 3;
            debugger.entities[countEntity].type = DCT_DBG_MODEL_MESH_ANIMATEDVTX;
            debugger.entities[countEntity].parentEntity = entityAddr_modelMesh;
            debugger.entities[countEntity].childEntities = NULL;
            debugger.entities[countEntity].childEntitiesCount = 0;
            debugger.entities[countEntity].data = mesh->animatedVtx;


            countEntity += 1; // anim vtx

            uint32_t *entityAddr_modelMeshRndVtx = &debugger.entities[countEntity];

            debugger.entities[countEntity-1].nextEntSibling = &debugger.entities[countEntity];
            debugger.entities[countEntity].prevEntSibling = &debugger.entities[countEntity-1];
            debugger.entities[countEntity].indiceCurrentDataSelected = -1;
            debugger.entities[countEntity].currentDataSelectedCount = 0;
            debugger.entities[countEntity].levelMenu = 3;
            debugger.entities[countEntity].type = DCT_DBG_MODEL_MESH_RENDERVTX;
            debugger.entities[countEntity].parentEntity = entityAddr_modelMesh;
            debugger.entities[countEntity].childEntities = NULL;
            debugger.entities[countEntity].childEntitiesCount = 0;
            debugger.entities[countEntity].data = mesh->renderVtx;

            countEntity += 1; // render vtx

            debugger.entities[countEntity-1].nextEntSibling = &debugger.entities[countEntity];
            debugger.entities[countEntity].prevEntSibling = &debugger.entities[countEntity-1];
            debugger.entities[countEntity].indiceCurrentDataSelected = -1;
            debugger.entities[countEntity].currentDataSelectedCount = 49-1;
            debugger.entities[countEntity].levelMenu = 3;
            debugger.entities[countEntity].type = DCT_DBG_MODEL_MESH_CXT;
            debugger.entities[countEntity].parentEntity = entityAddr_modelMesh;
            debugger.entities[countEntity].childEntities = NULL;
            debugger.entities[countEntity].childEntitiesCount = 0;
            debugger.entities[countEntity].data = &mesh->cxt;
            countEntity += 1; // cxt 

            debugger.entities[countEntity-1].nextEntSibling = &debugger.entities[countEntity];
            debugger.entities[countEntity].prevEntSibling = &debugger.entities[countEntity-1];
            debugger.entities[countEntity].indiceCurrentDataSelected = -1;
            debugger.entities[countEntity].currentDataSelectedCount = 0;
            debugger.entities[countEntity].levelMenu = 3;
            debugger.entities[countEntity].type = DCT_DBG_MODEL_MESH_HDR;
            debugger.entities[countEntity].parentEntity = entityAddr_modelMesh;
            debugger.entities[countEntity].childEntities = NULL;
            debugger.entities[countEntity].childEntitiesCount = 0;
            debugger.entities[countEntity].data = &mesh->hdr;

            debugger.entities[countEntity].nextEntSibling = NULL;
            countEntity += 1; // hdr
            

            for(int i_vtx=0; i_vtx<mesh->vtxCount; i_vtx++)
            {
                pvr_vertex_t *vtx = &mesh->originalVtx[i_vtx];
                if(  i_vtx<9 || i_vtx>=mesh->vtxCount-9 )
                {
                    debugger.entities[countEntity].levelMenu = 4;
                    debugger.entities[countEntity].indiceCurrentDataSelected = -1;
                    debugger.entities[countEntity].currentDataSelectedCount = 8-1;
                    debugger.entities[countEntity].type = DCT_DBG_MODEL_MESH_ORIGINALVTX_VTX;
                    debugger.entities[countEntity].parentEntity = entityAddr_modelMeshOrigVtx;
                    debugger.entities[countEntity].childEntities = NULL ;
                    debugger.entities[countEntity].childEntitiesCount = 0;
                    debugger.entities[countEntity].data = vtx;
                    debugger.entities[countEntity].dataIndiceList = i_vtx;
                    countEntity += 1; // pvr vtx 1

                    debugger.entities[countEntity-1].nextEntSibling = &debugger.entities[countEntity];
                    debugger.entities[countEntity].prevEntSibling = &debugger.entities[countEntity-1];
                    if(i_vtx==0)
                    {
                        debugger.entities[countEntity-1].prevEntSibling = NULL;
                    }

                    if(i_vtx==mesh->vtxCount-1)
                    {
                        debugger.entities[countEntity-1].nextEntSibling = NULL;
                    }
                    

                }
                
                
            }

            for(int i_vtx=0; i_vtx<mesh->vtxCount; i_vtx++)
            {
                pvr_vertex_t *vtxAnim = &mesh->animatedVtx[i_vtx];
                if(  i_vtx<9 || i_vtx>=mesh->vtxCount-9 )
                {
                    debugger.entities[countEntity].indiceCurrentDataSelected = -1;
                    debugger.entities[countEntity].currentDataSelectedCount = 8-1;
                    debugger.entities[countEntity].levelMenu = 4;
                    debugger.entities[countEntity].type = DCT_DBG_MODEL_MESH_ANIMATEDVTX_VTX;
                    debugger.entities[countEntity].parentEntity = entityAddr_modelMeshAnimVtx;
                    debugger.entities[countEntity].childEntities = NULL ;
                    debugger.entities[countEntity].childEntitiesCount = 0;
                    debugger.entities[countEntity].data = vtxAnim;
                    debugger.entities[countEntity].dataIndiceList = i_vtx;
                    countEntity += 1; // pvr vtx 1
                    debugger.entities[countEntity-1].nextEntSibling = &debugger.entities[countEntity];
                    debugger.entities[countEntity].prevEntSibling = &debugger.entities[countEntity-1];
                    if(i_vtx==0)
                    {
                        debugger.entities[countEntity-1].prevEntSibling = NULL;
                    }

                    if(i_vtx==mesh->vtxCount-1)
                    {
                        debugger.entities[countEntity-1].nextEntSibling = NULL;
                    }

                }
            }

            for(int i_vtx=0; i_vtx<mesh->vtxCount; i_vtx++)
            {
                pvr_vertex_t *vtxRnd = &mesh->renderVtx[i_vtx];
                if(  i_vtx<9 || i_vtx>=mesh->vtxCount-9 )
                {
                    debugger.entities[countEntity].indiceCurrentDataSelected = -1;
                    debugger.entities[countEntity].currentDataSelectedCount = 8-1;
                    debugger.entities[countEntity].levelMenu = 4;
                    debugger.entities[countEntity].type = DCT_DBG_MODEL_MESH_RENDERVTX_VTX;
                    debugger.entities[countEntity].parentEntity = entityAddr_modelMeshRndVtx;
                    debugger.entities[countEntity].childEntities = NULL ;
                    debugger.entities[countEntity].childEntitiesCount = 0;
                    debugger.entities[countEntity].data = vtxRnd;
                    debugger.entities[countEntity].dataIndiceList = i_vtx;
                    countEntity += 1; // pvr vtx 1
                    debugger.entities[countEntity-1].nextEntSibling = &debugger.entities[countEntity];
                    debugger.entities[countEntity].prevEntSibling = &debugger.entities[countEntity-1];
                    if(i_vtx==0)
                    {
                        debugger.entities[countEntity-1].prevEntSibling = NULL;
                    }

                    if(i_vtx==mesh->vtxCount-1)
                    {
                        debugger.entities[countEntity-1].nextEntSibling = NULL;
                    }

                }
            }


        }
    }

    for (int i_entity=0; i_entity<debugger.entityCount; i_entity++)
    {
        dct_menu_entityGameObject *ent = &debugger.entities[i_entity];
        if(ent->childEntitiesCount>0)
        {
            printf("entity with childEntityCount >0 : %d \n",i_entity);
        }
        if(ent->childEntities!=NULL)
        {
            printf("entity with childEntity Non NULL : %d \n",i_entity);
        }

    }

    // initialisation des childEntities
    for (int i_entity=0; i_entity<debugger.entityCount; i_entity++)
    {
        dct_menu_entityGameObject *ent = &debugger.entities[i_entity];
        if(ent->parentEntity!=NULL)
        {
            dct_menu_entityGameObject *entityParent = ent->parentEntity;
            // ajouter la connection enfant a l'entité parente
            entityParent->childEntitiesCount += 1;
            entityParent->childEntities = (dct_menu_entityGameObject*)realloc(entityParent->childEntities  ,sizeof(dct_menu_entityGameObject)*entityParent->childEntitiesCount);
            if(DEBUG_PRINT_ENABLE) {printf("\nentity number :%d addressEntity :%x child entities Size %d \n", i_entity, ent, sizeof(dct_menu_entityGameObject)*entityParent->childEntitiesCount) ;}
            if(entityParent->childEntities  != NULL)
            {
                entityParent->childEntities[entityParent->childEntitiesCount-1] = ent;
            }
            
        }

    }

    
    
    for (int i_menuElem=0; i_menuElem<NUM_MAIN_MENU_ELEMENTS; i_menuElem++)
    {
        
        for (int i_model=0; i_model<debugger.MainMenu[i_menuElem].childEntitiesCount; i_model++)
        {
            dct_menu_entityGameObject *ent = debugger.MainMenu[i_menuElem].childEntities[i_model];
            //dct_menu_entityGameObject *ent = debugger.entityRoot.childEntities[i_model];
            dct_menu_entityGameObject *prevEnt;
            dct_menu_entityGameObject *nextEnt;
            if(i_model==0)
            {
                ent->prevEntSibling = NULL;
                if(debugger.MainMenu[i_menuElem].childEntitiesCount>1)
                {
                    nextEnt = debugger.MainMenu[i_menuElem].childEntities[i_model+1];
                    ent->nextEntSibling = nextEnt;
                }
            }
            if(i_model>0 && i_model<debugger.MainMenu[i_menuElem].childEntitiesCount-1)
            {
                nextEnt = debugger.MainMenu[i_menuElem].childEntities[i_model+1];
                prevEnt = debugger.MainMenu[i_menuElem].childEntities[i_model-1];
                ent->prevEntSibling =  prevEnt;
                ent->nextEntSibling =  nextEnt;
            }

            if( i_model == debugger.MainMenu[i_menuElem].childEntitiesCount-1)
            {
                prevEnt = debugger.MainMenu[i_menuElem].childEntities[i_model-1];
                ent->prevEntSibling =  prevEnt;
                ent->nextEntSibling =  NULL;
            }


            if(DEBUG_PRINT_ENABLE) { printf("\n MainMenu entityModels %d addr %x prevSibling:%x nexSibling:%x \n",i_model, (unsigned int)ent, (unsigned int)ent->prevEntSibling, (unsigned int)ent->nextEntSibling);}

        }
    }


    initCurrentEntitySelected();
    
    updateTexteTextureMain();
    updateTexteTextureBreadcrumb();

    createAllTextureToDraw();



}


void initCurrentEntitySelected()
{
    debugger.currentEntitySelected = &debugger.switchBtn;
    
    // CLEAN 
    for (int i=0; i<NUM_ENTITIES_TO_DRAW; i++)
    {
        debugger.entitiesToDraw[i]=NULL;
    }

    debugger.entitiesToDraw[0] = debugger.currentEntitySelected;
    
}


void compileCxtHdrOfCurrentModelSelected(dct_menu_entityGameObject *ent) 
{
    if (ent == NULL) {
        printf("Error: ent is NULL\n");
        return;
    }

    printf("Debug: Starting compilation for entity type %d\n", ent->type);

    dct_mesh_t *mesh = NULL;
    
    // Remonter la hiérarchie jusqu'au mesh parent
    dct_menu_entityGameObject *current = ent;
    while (current != NULL) {
        printf("Debug: Checking entity type %d\n", current->type);
        if (current->type == DCT_DBG_MODEL_MESH_INDICE) {
            printf("Debug: Found mesh entity\n");
            mesh = (dct_mesh_t*)current->data;
            break;
        }
        current = current->parentEntity;
    }

    if (mesh == NULL) {
        printf("Error: Could not find parent mesh\n");
        return;
    }

    pvr_poly_cxt_t *cxt = ent->data;  // Le contexte actuel qu'on modifie
    printf("Debug: Found mesh at %p, cxt list_type: %d\n", (void*)mesh, cxt->list_type);

    // Compiler le contexte en utilisant le cxt qu'on modifie
    printf("Debug: Starting cxt compilation...\n");
    // Copier les modifications dans le mesh
    memcpy(&mesh->cxt, cxt, sizeof(pvr_poly_cxt_t));
    pvr_poly_compile(&mesh->hdr, &mesh->cxt);
    printf("Debug: Compilation complete\n");
}




void refreshEntitiesToDraw()
{
    for (int i=0; i<NUM_ENTITIES_TO_DRAW; i++)
    {
        debugger.entitiesToDraw[i]=NULL;
    }

    dct_menu_entityGameObject **childs = debugger.currentEntitySelected->childEntities;
    
    for(int i=0; i<debugger.currentEntitySelected->childEntitiesCount; i++)
    {
        if(i<NUM_ENTITIES_TO_DRAW-1)
        {
            debugger.entitiesToDraw[i] = childs[i];
        }
        
    }
}

void updateTexteTextureBreadcrumb()
{
    char temp_buffer[256] = {0};  // Buffer temporaire assez grand
    debugger.breadCrumbLength = 0;
    debugger.breadCrumbTexte[0] = '\0';  // S'assurer que la chaîne est vide
    
    calculateNumberOfParentEntityOfCurrentEntitySelected(debugger.currentEntitySelected, temp_buffer, sizeof(temp_buffer));
    
    // Copier le résultat final dans breadCrumbTexte
    strncpy(debugger.breadCrumbTexte, temp_buffer, sizeof(debugger.breadCrumbTexte)-1);
    debugger.breadCrumbTexte[sizeof(debugger.breadCrumbTexte)-1] = '\0';  // S'assurer d'avoir un 0 terminal
    
    if(DEBUG_PRINT_ENABLE==1)
    {printf("BreadCrumb Length: %d\n", debugger.breadCrumbLength);
    printf("Breadcrumb texte: %s\n", debugger.breadCrumbTexte);}

    //update_text_texture(debugger.textureDisplayBreadcrumb, debugger.breadCrumbTexte, debugger.font, 0x305FACFF);
    
}

void updateTexteTextureSwitch()
{
    pvr_get_stats(&debugger.stats);


    void* heap_end = sbrk(0);
    void* heap_start = (void*)0x8c010000;  // Début typique du heap
    
    float statsFPS = debugger.stats.frame_rate;
    uint64_t statsFrameTime = debugger.stats.rnd_last_time;
    int statsRAMTotale = 16 * 1024 ;
    int statsRAMUsed = ((char*)heap_end - (char*)heap_start)/1024;
    int statsRAMAvailable = statsRAMTotale - statsRAMUsed;
    int statsPVRFree = pvr_mem_available() / 1024;
    size_t curr = debugger.stats.vtx_buffer_used;
    size_t peak = debugger.stats.vtx_buffer_used_max;

    debugger.stats.ratioPvrRamAvailUsed = 1 - ( (float)(  8192 - (8192-statsPVRFree) ) / (float) 8192 );
    debugger.stats.ratioRamAvailUsed    = 1 - ( (float)(statsRAMTotale - statsRAMUsed) / (float)statsRAMTotale );
    debugger.stats.frame_rate = statsFPS;
    

    char labelMiniBtnSwitch[64*64];
    char labelTextureHelper[128];
    if( debugger.currentEntitySelected==&debugger.switchBtn)
    {
        sprintf(labelMiniBtnSwitch,"\\c[FFFFFF]TANTO ENGINE \\c[303030]FPS\nMEM DEBUGGER RAM\nMIN 16KO     PVR\nMAX  540KO %5d",debugger.stats.pvr_free);
        sprintf(labelTextureHelper,"PRESS [->] EXPAND                                               ",debugger.stats.pvr_free);
    }else
    {
        sprintf(labelMiniBtnSwitch,"\\c[FFFFFF]TANTO ENGINE \\c[303030]FPS\nMEM DEBUGGER RAM\nMIN 16KO     PVR\nMAX  540KO %5d",debugger.stats.pvr_free);
        sprintf(labelTextureHelper,"PRESS [<-] FOR MINIFY                                           ",debugger.stats.pvr_free);
    }


    update_text_texture(debugger.textureMiniBtnSwitch, labelMiniBtnSwitch, debugger.font, 0xA0808080);
    update_text_texture(debugger.textureHelper, labelTextureHelper, debugger.font, 0xA0808080);

}

void updateTexteTextureMain()
{
    updateTexteTextureBreadcrumb();
    if(DEBUG_PRINT_ENABLE==1)
    {
        printf("-- draw entities -- \n");
    }
    strcpy(debugger.currentTextToDisplay,"");

    pvr_get_stats(&debugger.stats);


    void* heap_end = sbrk(0);
    void* heap_start = (void*)0x8c010000;  // Début typique du heap
    
    float statsFPS = debugger.stats.frame_rate;
    uint64_t statsFrameTime = debugger.stats.rnd_last_time;
    int statsRAMTotale = 16 * 1024 ;
    int statsRAMUsed = ((char*)heap_end - (char*)heap_start)/1024;
    int statsRAMAvailable = statsRAMTotale - statsRAMUsed;
    int statsPVRFree = pvr_mem_available() / 1024;
    size_t curr = debugger.stats.vtx_buffer_used;
    size_t peak = debugger.stats.vtx_buffer_used_max;

    debugger.stats.pvr_free = statsPVRFree;
    debugger.stats.ratioPvrRamAvailUsed = 1 - ( (float)(  8192 - (8192-statsPVRFree) ) / (float) 8192 );
    debugger.stats.ratioRamAvailUsed    = 1 - ( (float)(statsRAMTotale - statsRAMUsed) / (float)statsRAMTotale );
    

    sprintf(debugger.currentTextToDisplay,"\\c[303030]++------------------ \\c[FFFFFF]TANTO ENGINE DREAMCAST \\c[303030]------------------++\n++------- STATS FPS:\\c[FFFFFF]%.2f\\c[303030] RAM(KB): \\c[FFFFFF]%5lu\\c[303030] PVR(KB):\\c[FFFFFF]%5lu\\c[303030] -------++\n\\c[FFFFFF]%-64s\n\n",


    statsFPS,statsRAMAvailable,statsPVRFree,debugger.breadCrumbTexte );
    char tmp[10000];
    
    if(isCurrentEntitySelectedInMainMenu())
    {
        for (int i=0; i<NUM_MAIN_MENU_ELEMENTS; i++)
        {
            if(&debugger.MainMenu[i]!=NULL)
            {
                dct_menu_entityGameObject *entity = &debugger.MainMenu[i];
                
                    if(entity == debugger.currentEntitySelected)
                    {
                        if(DEBUG_PRINT_ENABLE==1){printf(">");}
                        strcat(debugger.currentTextToDisplay,"\\c[FFFFFF]>\\c[303030]");
                    }
                    switch (entity->type)
                    {
                        case DCT_DBG_MENU_PRINCIPAL_NAME:
                            //dct_menu_entityGameObject *entity = entity->data;
                            if(entity == debugger.currentEntitySelected)
                            {
                                snprintf(tmp, sizeof(tmp), "\\c[FFFFFF]-%s \n", entity->data);
                            }else
                            {
                                snprintf(tmp, sizeof(tmp), "\\c[303030]-%s \n", entity->data);
                            }
                            
                            if(DEBUG_PRINT_ENABLE==1){printf("-%s Addr%x dataAddr:%x \n", entity->data, (unsigned int)entity, (unsigned int)entity->data);}
                            strcat(debugger.currentTextToDisplay, tmp);
                            break;
                    }
                
                
            }
        }
    }
    else
    {
        for (int i=0; i<NUM_ENTITIES_TO_DRAW; i++)
    {

        if(debugger.entitiesToDraw[i]!=NULL)
        {
            dct_menu_entityGameObject *entity = debugger.entitiesToDraw[i];
            //printf("\n check type %d \n", entity->type);
            if(entity == debugger.currentEntitySelected)
            {
                if(DEBUG_PRINT_ENABLE==1){printf(">");}
                strcat(debugger.currentTextToDisplay,"\\c[FFFFFF]>\\c[303030]");
            }
            switch (entity->type)
            {
               
                case DCT_DBG_MODEL:
                    dct_model_t *mod1 = entity->data;
                    if(entity == debugger.currentEntitySelected)
                    {
                        snprintf(tmp, sizeof(tmp), "\\c[FFFFFF]-DCT MODEL %s Addr%x dataAddr:%x \n", mod1->name, (unsigned int)entity, (unsigned int)entity->data);
                    }else
                    {
                        snprintf(tmp, sizeof(tmp), "\\c[303030]-DCT MODEL %s Addr%x dataAddr:%x \n", mod1->name, (unsigned int)entity, (unsigned int)entity->data);
                    }
                    
                    if(DEBUG_PRINT_ENABLE==1){printf("-DCT MODEL %s Addr%x dataAddr:%x \n", mod1->name, (unsigned int)entity, (unsigned int)entity->data);}
                    strcat(debugger.currentTextToDisplay, tmp);
                    break;
                
                case DCT_DBG_MODEL_NAME:
                    dct_model_t *mod2 = entity->data;
                    if(entity == debugger.currentEntitySelected)
                    {
                        snprintf(tmp, sizeof(tmp), "\\c[FFFFFF]-DCT MODEL NAME %s Addr:%x dataAddr:%x \n",mod2->name, (unsigned int)entity, (unsigned int)entity->data);

                    }else
                    {
                        snprintf(tmp, sizeof(tmp), "\\c[303030]-DCT MODEL NAME %s Addr:%x dataAddr:%x \n",mod2->name, (unsigned int)entity, (unsigned int)entity->data);
                    }
                    if(DEBUG_PRINT_ENABLE==1){printf("-DCT MODEL NAME %s Addr:%x dataAddr:%x \n",mod2->name, (unsigned int)entity, (unsigned int)entity->data);}
                    strcat(debugger.currentTextToDisplay, tmp);
                    break;

                case DCT_DBG_MODEL_POSITION:
                    vec3f_t *position = entity->data;
                    //char dctmodPosTxtOr = "-DCT MODEL POSITION x:%f y:%f z:%f addr:%x dataAddr:%x parentAddr:%x\n";
                    char dctmodPosTxtRes[200];
                    if(debugger.currentEntitySelected==entity && debugger.state==1)
                    {
                        switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
                        {
                            case -1:
                                snprintf(dctmodPosTxtRes,sizeof(dctmodPosTxtRes), "\\c[FFFFFF]-[DCT MODEL POSITION]\\c[303030] x:%f y:%f z:%f addr:%x dataAddr:%x parentAddr:%x\n" ,position->x, position->y, position->z, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity );
                                break;
                            case 0:
                                snprintf(dctmodPosTxtRes,sizeof(dctmodPosTxtRes), "\\c[FFFFFF]-[DCT MODEL POSITION]\\c[303030] \\c[FFFFFF][x:%f]\\c[303030] y:%f z:%f addr:%x dataAddr:%x parentAddr:%x\n" ,position->x, position->y, position->z, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity );
                                break;
                            case 1:
                                snprintf(dctmodPosTxtRes,sizeof(dctmodPosTxtRes), "\\c[FFFFFF]-[DCT MODEL POSITION]\\c[303030] x:%f \\c[FFFFFF][y:%f]\\c[303030] z:%f addr:%x dataAddr:%x parentAddr:%x\n" ,position->x, position->y, position->z, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity );
                                break;
                            case 2:
                                snprintf(dctmodPosTxtRes,sizeof(dctmodPosTxtRes), "\\c[FFFFFF]-[DCT MODEL POSITION]\\c[303030] x:%f y:%f \\c[FFFFFF][z:%f]\\c[303030] addr:%x dataAddr:%x parentAddr:%x\n" ,position->x, position->y, position->z, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity );
                                break;
                            
                        }

                    }else
                    {
                        if(entity == debugger.currentEntitySelected)
                        {
                            snprintf(dctmodPosTxtRes,sizeof(dctmodPosTxtRes), "\\c[FFFFFF]-DCT MODEL POSITION x:%f y:%f z:%f addr:%x dataAddr:%x parentAddr:%x\n" ,position->x, position->y, position->z, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity );
                        }
                        else
                        {
                            snprintf(dctmodPosTxtRes,sizeof(dctmodPosTxtRes), "\\c[303030]-DCT MODEL POSITION x:%f y:%f z:%f addr:%x dataAddr:%x parentAddr:%x\n" ,position->x, position->y, position->z, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity );
                        }

                    }                    
                    strcat(debugger.currentTextToDisplay, dctmodPosTxtRes);
                    if(DEBUG_PRINT_ENABLE==1){printf(dctmodPosTxtRes);}
                    break;
                
                case DCT_DBG_MODEL_ROTATION:
                    vec3f_t *rotation = entity->data;

                    char dctmodRotTxtRes[200];
                    if(debugger.currentEntitySelected==entity && debugger.state==1)
                    {
                        switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
                        {
                            case -1:
                                snprintf(dctmodRotTxtRes,sizeof(dctmodRotTxtRes), "\\c[FFFFFF]-[DCT MODEL ROTATION]\\c[303030] x:%f y:%f z:%f addr:%x dataAddr:%x parentAddr:%x\n", rotation->x, rotation->y, rotation->z,  (unsigned int)entity, (unsigned int)entity->data, (unsigned int) entity->parentEntity  );
                                break;
                            case 0:
                                snprintf(dctmodRotTxtRes,sizeof(dctmodRotTxtRes), "\\c[FFFFFF]-[DCT MODEL ROTATION]\\c[303030] \\c[FFFFFF][x:%f]\\c[303030] y:%f z:%f addr:%x dataAddr:%x parentAddr:%x\n", rotation->x, rotation->y, rotation->z, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity  );
                                break;
                            case 1:
                                snprintf(dctmodRotTxtRes,sizeof(dctmodRotTxtRes), "\\c[FFFFFF]-[DCT MODEL ROTATION]\\c[303030] x:%f \\c[FFFFFF][y:%f]\\c[303030] z:%f addr:%x dataAddr:%x parentAddr:%x\n", rotation->x, rotation->y, rotation->z, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity  );
                                break;
                            case 2:
                                snprintf(dctmodRotTxtRes,sizeof(dctmodRotTxtRes), "\\c[FFFFFF]-[DCT MODEL ROTATION]\\c[303030] x:%f y:%f \\c[FFFFFF][z:%f]\\c[303030] addr:%x dataAddr:%x parentAddr:%x\n", rotation->x, rotation->y, rotation->z, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity   );
                                break;
                            
                        }

                    }else
                    {
                        if(entity == debugger.currentEntitySelected)
                        {
                            snprintf(dctmodRotTxtRes,sizeof(dctmodRotTxtRes), "\\c[FFFFFF]-DCT MODEL ROTATION x:%f y:%f z:%f addr:%x dataAddr:%x parentAddr:%x\n", rotation->x, rotation->y, rotation->z, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity  );
                        }else
                        {
                            snprintf(dctmodRotTxtRes,sizeof(dctmodRotTxtRes), "\\c[303030]-DCT MODEL ROTATION x:%f y:%f z:%f addr:%x dataAddr:%x parentAddr:%x\n", rotation->x, rotation->y, rotation->z, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity  );
                        }

                    } 

                    //printf("-DCT MODEL ROTATION x:%f y:%f z:%f addr:%x dataAddr:%x parentAddr:%x\n", rotation->x, rotation->y, rotation->z, entity, entity->data, entity->parentEntity  );
                    strcat(debugger.currentTextToDisplay, dctmodRotTxtRes);
                    if(DEBUG_PRINT_ENABLE==1){printf(dctmodRotTxtRes);}
                    break;

                case DCT_DBG_MODEL_SCALE:
                    vec3f_t *scale = entity->data;

                    char dctmodScTxtRes[200];
                    if(debugger.currentEntitySelected==entity && debugger.state==1)
                    {
                        switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
                        {
                            case -1:
                                snprintf(dctmodScTxtRes,sizeof(dctmodScTxtRes), "\\c[FFFFFF]-[DCT MODEL SCALE]\\c[303030] x:%f y:%f z:%f addr:%x dataAddr:%x parentAddr:%x\n", scale->x, scale->y, scale->z, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity );
                                break;
                            case 0:
                                snprintf(dctmodScTxtRes,sizeof(dctmodScTxtRes), "\\c[FFFFFF]-[DCT MODEL SCALE]\\c[303030] \\c[FFFFFF][x:%f]\\c[303030] y:%f z:%f addr:%x dataAddr:%x parentAddr:%x\n", scale->x, scale->y, scale->z, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity );
                                break;
                            case 1:
                                snprintf(dctmodScTxtRes,sizeof(dctmodScTxtRes), "\\c[FFFFFF]-[DCT MODEL SCALE]\\c[303030] x:%f \\c[FFFFFF][y:%f]\\c[303030] z:%f addr:%x dataAddr:%x parentAddr:%x\n", scale->x, scale->y, scale->z, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity );
                                break;
                            case 2:
                                snprintf(dctmodScTxtRes,sizeof(dctmodScTxtRes), "\\c[FFFFFF]-[DCT MODEL SCALE]\\c[303030] x:%f y:%f \\c[FFFFFF][z:%f]\\c[303030] addr:%x dataAddr:%x parentAddr:%x\n", scale->x, scale->y, scale->z, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity );
                                break;
                            
                        }

                    }else
                    {
                        if(entity == debugger.currentEntitySelected)
                        {
                            snprintf(dctmodScTxtRes,sizeof(dctmodScTxtRes),"\\c[FFFFFF]-DCT MODEL SCALE x:%f y:%f z:%f addr:%x dataAddr:%x parentAddr:%x\n", scale->x, scale->y, scale->z, entity, (unsigned int)entity->data,(unsigned int) entity->parentEntity );
                        }
                        else
                        {
                            snprintf(dctmodScTxtRes,sizeof(dctmodScTxtRes),"\\c[303030]-DCT MODEL SCALE x:%f y:%f z:%f addr:%x dataAddr:%x parentAddr:%x\n", scale->x, scale->y, scale->z, entity, (unsigned int)entity->data,(unsigned int) entity->parentEntity );
                        }

                    } 

                    //printf("-DCT MODEL SCALE x:%f y:%f z:%f addr:%x dataAddr:%x parentAddr:%x\n", scale->x, scale->y, scale->z, entity, entity->data, entity->parentEntity );
                    strcat(debugger.currentTextToDisplay, dctmodScTxtRes);
                    if(DEBUG_PRINT_ENABLE==1){printf(dctmodScTxtRes);}
                    break;

                case DCT_DBG_MODEL_MESHES:
                    if(DEBUG_PRINT_ENABLE==1){printf("-DCT MODEL MESHES addr:%x dataAddr:%x parentAddr:%x\n", (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity);}
                    if(entity == debugger.currentEntitySelected)
                    {
                        snprintf(tmp, sizeof(tmp),"\\c[FFFFFF]-DCT MODEL MESHES addr:%x dataAddr:%x parentAddr:%x\n", (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity);
                    }else
                    {
                        snprintf(tmp, sizeof(tmp),"\\c[303030]-DCT MODEL MESHES addr:%x dataAddr:%x parentAddr:%x\n", (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity);
                    }
                    strcat(debugger.currentTextToDisplay, tmp);
                    break;

                case DCT_DBG_MODEL_MESH_NAME:
                    dct_mesh_t *mesh = entity->data;
                    if(DEBUG_PRINT_ENABLE==1){printf("-DCT MODEL MESH NAME %s addr:%x dataAddr:%x parentAddr:%x\n",mesh->name, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity);}
                    if(entity == debugger.currentEntitySelected)
                    {
                    snprintf(tmp, sizeof(tmp),"\\c[FFFFFF]-DCT MODEL MESH NAME %s addr:%x dataAddr:%x parentAddr:%x\n",mesh->name, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity);
                    }else
                    {
                    snprintf(tmp, sizeof(tmp),"\\c[303030]-DCT MODEL MESH NAME %s addr:%x dataAddr:%x parentAddr:%x\n",mesh->name, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity);
                    }
                    strcat(debugger.currentTextToDisplay, tmp);
                    break;
                
                case DCT_DBG_MODEL_MESH_INDICE:
                    dct_mesh_t *meshIndice = entity->data;
                    if(DEBUG_PRINT_ENABLE==1){printf("-DCT MODEL MESH INDICE %d addr:%x dataAddr:%x parentAddr:%x \n",meshIndice->indice, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity);}
                    if(entity == debugger.currentEntitySelected)
                    {
                    snprintf(tmp, sizeof(tmp),"\\c[FFFFFF]-DCT MODEL MESH INDICE %d addr:%x dataAddr:%x parentAddr:%x \n",meshIndice->indice, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity);
                    }else
                    {
                    snprintf(tmp, sizeof(tmp),"\\c[303030]-DCT MODEL MESH INDICE %d addr:%x dataAddr:%x parentAddr:%x \n",meshIndice->indice, (unsigned int)entity, (unsigned int)entity->data, (unsigned int)entity->parentEntity);
                    }
                    strcat(debugger.currentTextToDisplay, tmp);
                    break;

                case DCT_DBG_MODEL_MESH_ORIGINALVTX:
                    pvr_vertex_t *vtxOrig = entity->data;
                    if(DEBUG_PRINT_ENABLE==1){printf("-DCT MODEL MESH ORIGINAL VTX addr:%x dataAddr:%x parent:%x\n",vtxOrig, (unsigned int)entity, (unsigned int)entity->data,(unsigned int)entity->parentEntity);}
                    if(entity == debugger.currentEntitySelected)
                    {
                    snprintf(tmp, sizeof(tmp),"\\c[FFFFFF]-DCT MODEL MESH ORIGINAL VTX addr:%x dataAddr:%x parent:%x\n",vtxOrig, (unsigned int)entity, (unsigned int)entity->data,(unsigned int)entity->parentEntity);
                    }else
                    {
                    snprintf(tmp, sizeof(tmp),"\\c[303030]-DCT MODEL MESH ORIGINAL VTX addr:%x dataAddr:%x parent:%x\n",vtxOrig, (unsigned int)entity, (unsigned int)entity->data,(unsigned int)entity->parentEntity);
                    }
                    strcat(debugger.currentTextToDisplay, tmp);
                    break;
                
                case DCT_DBG_MODEL_MESH_ANIMATEDVTX:
                    pvr_vertex_t *vtxAni = entity->data;
                    if(DEBUG_PRINT_ENABLE==1){printf("-DCT MODEL MESH ANIMATED VTX addr:%x dataAddr:%x parent:%x\n",vtxAni, (unsigned int)entity, (unsigned int)entity->data,(unsigned int)entity->parentEntity);}
                    if(entity == debugger.currentEntitySelected)
                    {
                    snprintf(tmp, sizeof(tmp),"\\c[FFFFFF]-DCT MODEL MESH ANIMATED VTX addr:%x dataAddr:%x parent:%x\n",vtxAni, (unsigned int)entity, (unsigned int)entity->data,(unsigned int)entity->parentEntity);
                    }else
                    {
                    snprintf(tmp, sizeof(tmp),"\\c[303030]-DCT MODEL MESH ANIMATED VTX addr:%x dataAddr:%x parent:%x\n",vtxAni, (unsigned int)entity, (unsigned int)entity->data,(unsigned int)entity->parentEntity);
                    }
                    strcat(debugger.currentTextToDisplay, tmp);
                    
                    break;
                
                case DCT_DBG_MODEL_MESH_RENDERVTX:
                    pvr_vertex_t *vtxRnd = entity->data;
                    if(DEBUG_PRINT_ENABLE==1){printf("-DCT MODEL MESH RENDER VTX addr:%x dataAddr:%x parent:%x\n",vtxRnd, (unsigned int)entity, (unsigned int)entity->data,(unsigned int)entity->parentEntity);}
                    if(entity == debugger.currentEntitySelected)
                    {
                    snprintf(tmp, sizeof(tmp),"\\c[FFFFFF]-DCT MODEL MESH RENDER VTX addr:%x dataAddr:%x parent:%x\n",vtxRnd, (unsigned int)entity, (unsigned int)entity->data,(unsigned int)entity->parentEntity);
                    }else
                    {
                    snprintf(tmp, sizeof(tmp),"\\c[303030]-DCT MODEL MESH RENDER VTX addr:%x dataAddr:%x parent:%x\n",vtxRnd, (unsigned int)entity, (unsigned int)entity->data,(unsigned int)entity->parentEntity);
                    }
                    strcat(debugger.currentTextToDisplay, tmp);
                    
                    break;
                
                case DCT_DBG_MODEL_MESH_ORIGINALVTX_VTX:
                    pvr_vertex_t *vtx = entity->data;

                    char dctmodOrigVtxTxtRes[200];
                    int indiceOrVtx = entity->dataIndiceList;
                    if(debugger.currentEntitySelected==entity && debugger.state==1)
                    {
                        switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
                        {
                            case -1:
                                snprintf(dctmodOrigVtxTxtRes,sizeof(dctmodOrigVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ORIGINAL VTX] VTX:%.2d \\c[303030]\nflags:%x x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceOrVtx,(unsigned int)vtx->flags,vtx->x,vtx->y,vtx->z,vtx->u,vtx->v,(unsigned int)vtx->argb,(unsigned int)vtx->oargb);
                                break;
                            case 0:
                                snprintf(dctmodOrigVtxTxtRes,sizeof(dctmodOrigVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ORIGINAL VTX] VTX:%.2d \\c[303030]\n\\c[FFFFFF][flags:%x]\\c[303030] x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceOrVtx,(unsigned int)vtx->flags,vtx->x,vtx->y,vtx->z,vtx->u,vtx->v,(unsigned int)vtx->argb,(unsigned int)vtx->oargb);
                                break;
                            case 1:
                                snprintf(dctmodOrigVtxTxtRes,sizeof(dctmodOrigVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ORIGINAL VTX] VTX:%.2d \\c[303030]\nflags:%x \\c[FFFFFF][x:%.1f]\\c[303030] y:%.1f z:%.1f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceOrVtx,(unsigned int)vtx->flags,vtx->x,vtx->y,vtx->z,vtx->u,vtx->v,(unsigned int)vtx->argb,(unsigned int)vtx->oargb);
                                break;
                            case 2:
                                snprintf(dctmodOrigVtxTxtRes,sizeof(dctmodOrigVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ORIGINAL VTX] VTX:%.2d \\c[303030]\nflags:%x x:%.1f \\c[FFFFFF][y:%.1f]\\c[303030] z:%.1f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceOrVtx,(unsigned int)vtx->flags,vtx->x,vtx->y,vtx->z,vtx->u,vtx->v,(unsigned int)vtx->argb,(unsigned int)vtx->oargb);
                                break;
                            case 3:
                                snprintf(dctmodOrigVtxTxtRes,sizeof(dctmodOrigVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ORIGINAL VTX] VTX:%.2d \\c[303030]\nflags:%x x:%.1f y:%.1f \\c[FFFFFF][z:%.1f]\\c[303030] u:%.1f v:%.1f argb:%x oargb:%x\n", indiceOrVtx,(unsigned int)vtx->flags,vtx->x,vtx->y,vtx->z,vtx->u,vtx->v,(unsigned int)vtx->argb,(unsigned int)vtx->oargb);
                                break;
                            case 4:
                                snprintf(dctmodOrigVtxTxtRes,sizeof(dctmodOrigVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ORIGINAL VTX] VTX:%.2d \\c[303030]\nflags:%x x:%.1f y:%.1f z:%.1f \\c[FFFFFF][u:%.1f]\\c[303030] v:%.1f argb:%x oargb:%x\n", indiceOrVtx,(unsigned int)vtx->flags,vtx->x,vtx->y,vtx->z,vtx->u,vtx->v,(unsigned int)vtx->argb,(unsigned int)vtx->oargb);
                                break;
                            case 5:
                                snprintf(dctmodOrigVtxTxtRes,sizeof(dctmodOrigVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ORIGINAL VTX] VTX:%.2d \\c[303030]\nflags:%x x:%.1f y:%.1f z:%.1f u:%.1f \\c[FFFFFF][v:%.1f]\\c[303030] argb:%x oargb:%x\n", indiceOrVtx,(unsigned int)vtx->flags,vtx->x,vtx->y,vtx->z,vtx->u,vtx->v,(unsigned int)vtx->argb,(unsigned int)vtx->oargb);
                                break;
                            case 6:
                                snprintf(dctmodOrigVtxTxtRes,sizeof(dctmodOrigVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ORIGINAL VTX] VTX:%.2d \\c[303030]\nflags:%x x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f \\c[FFFFFF][argb:%x]\\c[303030] oargb:%x\n", indiceOrVtx,(unsigned int)vtx->flags,vtx->x,vtx->y,vtx->z,vtx->u,vtx->v,(unsigned int)vtx->argb,(unsigned int)vtx->oargb);
                                break;
                            case 7:
                                snprintf(dctmodOrigVtxTxtRes,sizeof(dctmodOrigVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ORIGINAL VTX] VTX:%.2d\\c[303030] \nflags:%x x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f argb:%x \\c[FFFFFF][oargb:%x]\\c[303030]\n", indiceOrVtx,(unsigned int)vtx->flags,vtx->x,vtx->y,vtx->z,vtx->u,vtx->v,(unsigned int)vtx->argb,(unsigned int)vtx->oargb);
                                break;
                            
                        }

                    }else
                    {
                        if(entity == debugger.currentEntitySelected)
                        {
                            snprintf(dctmodOrigVtxTxtRes,sizeof(dctmodOrigVtxTxtRes),"\\c[FFFFFF]-DCT MODEL MESH ORIGINAL VTX VTX:%.2d \nflags:%x x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceOrVtx,(unsigned int)vtx->flags,vtx->x,vtx->y,vtx->z,vtx->u,vtx->v,(unsigned int)vtx->argb,(unsigned int)vtx->oargb);
                        }else
                        {
                            snprintf(dctmodOrigVtxTxtRes,sizeof(dctmodOrigVtxTxtRes),"\\c[303030]-DCT MODEL MESH ORIGINAL VTX VTX:%.2d \nflags:%x x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceOrVtx,(unsigned int)vtx->flags,vtx->x,vtx->y,vtx->z,vtx->u,vtx->v,(unsigned int)vtx->argb,(unsigned int)vtx->oargb);
                        }

                    } 

                    
                    //printf("-DCT MODEL MESH ORIGINAL VTX VTX:%.2d flags:%x x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceOrVtx,vtx->flags,vtx->x,vtx->y,vtx->z,vtx->u,vtx->v,vtx->argb,vtx->oargb);
                    strcat(debugger.currentTextToDisplay, dctmodOrigVtxTxtRes);
                    if(DEBUG_PRINT_ENABLE==1){printf(dctmodOrigVtxTxtRes);}
                    break;
                
                case DCT_DBG_MODEL_MESH_ANIMATEDVTX_VTX:
                    pvr_vertex_t *vtxAnim = entity->data;
                    int indiceAnVtx = entity->dataIndiceList;
                    char dctmodAnimVtxTxtRes[200];

                    if(debugger.currentEntitySelected==entity && debugger.state==1)
                    {
                        switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
                        {
                            case -1:
                                snprintf(dctmodAnimVtxTxtRes,sizeof(dctmodAnimVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ANIMATED VTX] VTX:%.2d\\c[303030] \nflags:%x x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceAnVtx,(unsigned int)vtxAnim->flags,vtxAnim->x,vtxAnim->y,vtxAnim->z,vtxAnim->u,vtxAnim->v,(unsigned int)vtxAnim->argb,(unsigned int)vtxAnim->oargb);
                                break;
                            case 0:
                                snprintf(dctmodAnimVtxTxtRes,sizeof(dctmodAnimVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ANIMATED VTX] VTX:%.2d\\c[303030] \n\\c[FFFFFF][flags:%x]\\c[303030] x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceAnVtx,(unsigned int)vtxAnim->flags,vtxAnim->x,vtxAnim->y,vtxAnim->z,vtxAnim->u,vtxAnim->v,(unsigned int)vtxAnim->argb,(unsigned int)vtxAnim->oargb);
                                break;
                            case 1:
                                snprintf(dctmodAnimVtxTxtRes,sizeof(dctmodAnimVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ANIMATED VTX] VTX:%.2d\\c[303030] \nflags:%x \\c[FFFFFF][x:%.1f]\\c[303030] y:%.1f z:%.1f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceAnVtx,(unsigned int)vtxAnim->flags,vtxAnim->x,vtxAnim->y,vtxAnim->z,vtxAnim->u,vtxAnim->v,(unsigned int)vtxAnim->argb,(unsigned int)vtxAnim->oargb);
                                break;
                            case 2:
                                snprintf(dctmodAnimVtxTxtRes,sizeof(dctmodAnimVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ANIMATED VTX] VTX:%.2d\\c[303030] \nflags:%x x:%.1f \\c[FFFFFF][y:%.1f]\\c[303030] z:%.1f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceAnVtx,(unsigned int)vtxAnim->flags,vtxAnim->x,vtxAnim->y,vtxAnim->z,vtxAnim->u,vtxAnim->v,(unsigned int)vtxAnim->argb,(unsigned int)vtxAnim->oargb);
                                break;
                            case 3:
                                snprintf(dctmodAnimVtxTxtRes,sizeof(dctmodAnimVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ANIMATED VTX] VTX:%.2d\\c[303030] \nflags:%x x:%.1f y:%.1f \\c[FFFFFF][z:%.1f]\\c[303030] u:%.1f v:%.1f argb:%x oargb:%x\n", indiceAnVtx,(unsigned int)vtxAnim->flags,vtxAnim->x,vtxAnim->y,vtxAnim->z,vtxAnim->u,vtxAnim->v,(unsigned int)vtxAnim->argb,(unsigned int)vtxAnim->oargb);
                                break;
                            case 4:
                                snprintf(dctmodAnimVtxTxtRes,sizeof(dctmodAnimVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ANIMATED VTX] VTX:%.2d\\c[303030] \nflags:%x x:%.1f y:%.1f z:%.1f \\c[FFFFFF][u:%.1f]\\c[303030] v:%.1f argb:%x oargb:%x\n", indiceAnVtx,(unsigned int)vtxAnim->flags,vtxAnim->x,vtxAnim->y,vtxAnim->z,vtxAnim->u,vtxAnim->v,(unsigned int)vtxAnim->argb,(unsigned int)vtxAnim->oargb);
                                break;
                            case 5:
                                snprintf(dctmodAnimVtxTxtRes,sizeof(dctmodAnimVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ANIMATED VTX] VTX:%.2d\\c[303030] \nflags:%x x:%.1f y:%.1f z:%.1f u:%.1f \\c[FFFFFF][v:%.1f]\\c[303030] argb:%x oargb:%x\n", indiceAnVtx,(unsigned int)vtxAnim->flags,vtxAnim->x,vtxAnim->y,vtxAnim->z,vtxAnim->u,vtxAnim->v,(unsigned int)vtxAnim->argb,(unsigned int)vtxAnim->oargb);
                                break;
                            case 6:
                                snprintf(dctmodAnimVtxTxtRes,sizeof(dctmodAnimVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ANIMATED VTX] VTX:%.2d\\c[303030] \nflags:%x x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f \\c[FFFFFF][argb:%x]\\c[303030] oargb:%x\n", indiceAnVtx,(unsigned int)vtxAnim->flags,vtxAnim->x,vtxAnim->y,vtxAnim->z,vtxAnim->u,vtxAnim->v,(unsigned int)vtxAnim->argb,(unsigned int)vtxAnim->oargb);
                                break;
                            case 7:
                                snprintf(dctmodAnimVtxTxtRes,sizeof(dctmodAnimVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH ANIMATED VTX] VTX:%.2d\\c[303030] \nflags:%x x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f argb:%x \\c[FFFFFF][oargb:%x]\\c[303030]\n", indiceAnVtx,(unsigned int)vtxAnim->flags,vtxAnim->x,vtxAnim->y,vtxAnim->z,vtxAnim->u,vtxAnim->v,(unsigned int)vtxAnim->argb,(unsigned int)vtxAnim->oargb);
                                break;
                            
                        }

                    }else
                    {
                        if(entity == debugger.currentEntitySelected)
                        {
                        snprintf(dctmodAnimVtxTxtRes,sizeof(dctmodAnimVtxTxtRes),"\\c[FFFFFF]-DCT MODEL MESH ANIMATED VTX VTX:%.2d \nflags:%x x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceAnVtx,(unsigned int)vtxAnim->flags,vtxAnim->x,vtxAnim->y,vtxAnim->z,vtxAnim->u,vtxAnim->v,(unsigned int)vtxAnim->argb,(unsigned int)vtxAnim->oargb);
                        }else
                        {
                        snprintf(dctmodAnimVtxTxtRes,sizeof(dctmodAnimVtxTxtRes),"\\c[303030]-DCT MODEL MESH ANIMATED VTX VTX:%.2d \nflags:%x x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceAnVtx,(unsigned int)vtxAnim->flags,vtxAnim->x,vtxAnim->y,vtxAnim->z,vtxAnim->u,vtxAnim->v,(unsigned int)vtxAnim->argb,(unsigned int)vtxAnim->oargb);
                        }

                    } 

                    //printf("-DCT MODEL MESH ANIMATED VTX VTX:%.2d flags:%x x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceAnVtx,vtxAnim->flags,vtxAnim->x,vtxAnim->y,vtxAnim->z,vtxAnim->u,vtxAnim->v,vtxAnim->argb,vtxAnim->oargb);
                    strcat(debugger.currentTextToDisplay, dctmodAnimVtxTxtRes);
                    if(DEBUG_PRINT_ENABLE==1){printf(dctmodAnimVtxTxtRes);}
                    break;
                
                case DCT_DBG_MODEL_MESH_RENDERVTX_VTX:
                    pvr_vertex_t *vtxRender = entity->data;
                    int indiceRndVtx = entity->dataIndiceList;
                    char dctmodRendVtxTxtRes[200];
                    if(debugger.currentEntitySelected==entity && debugger.state==1)
                    {
                        switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
                        {
                            case -1:
                                snprintf(dctmodRendVtxTxtRes,sizeof(dctmodRendVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH RENDER   VTX] VTX:%.2d\\c[303030] \nflags:%x x:%.1f y:%.1f z:%.3f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceRndVtx,(unsigned int)vtxRender->flags,vtxRender->x,vtxRender->y,vtxRender->z,vtxRender->u,vtxRender->v,(unsigned int)vtxRender->argb,(unsigned int)vtxRender->oargb);
                                break;
                            case 0:
                                snprintf(dctmodRendVtxTxtRes,sizeof(dctmodRendVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH RENDER   VTX] VTX:%.2d\\c[303030] \n\\c[FFFFFF][flags:%x]\\c[303030] x:%.1f y:%.1f z:%.3f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceRndVtx,(unsigned int)vtxRender->flags,vtxRender->x,vtxRender->y,vtxRender->z,vtxRender->u,vtxRender->v,(unsigned int)vtxRender->argb,(unsigned int)vtxRender->oargb);
                                break;
                            case 1:
                                snprintf(dctmodRendVtxTxtRes,sizeof(dctmodRendVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH RENDER   VTX] VTX:%.2d\\c[303030] \nflags:%x \\c[FFFFFF][x:%.1f]\\c[303030] y:%.1f z:%.3f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceRndVtx,(unsigned int)vtxRender->flags,vtxRender->x,vtxRender->y,vtxRender->z,vtxRender->u,vtxRender->v,(unsigned int)vtxRender->argb,(unsigned int)vtxRender->oargb);
                                break;
                            case 2:
                                snprintf(dctmodRendVtxTxtRes,sizeof(dctmodRendVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH RENDER   VTX] VTX:%.2d\\c[303030] \nflags:%x x:%.1f \\c[FFFFFF][y:%.1f]\\c[303030] z:%.3f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceRndVtx,(unsigned int)vtxRender->flags,vtxRender->x,vtxRender->y,vtxRender->z,vtxRender->u,vtxRender->v,(unsigned int)vtxRender->argb,(unsigned int)vtxRender->oargb);
                                break;
                            case 3:
                                snprintf(dctmodRendVtxTxtRes,sizeof(dctmodOrigVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH RENDER   VTX] VTX:%.2d\\c[303030] \nflags:%x x:%.1f y:%.1f \\c[FFFFFF][z:%.3f]\\c[303030] u:%.1f v:%.1f argb:%x oargb:%x\n", indiceRndVtx,(unsigned int)vtxRender->flags,vtxRender->x,vtxRender->y,vtxRender->z,vtxRender->u,vtxRender->v,(unsigned int)vtxRender->argb,(unsigned int)vtxRender->oargb);
                                break;
                            case 4:
                                snprintf(dctmodRendVtxTxtRes,sizeof(dctmodRendVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH RENDER   VTX] VTX:%.2d\\c[303030] \nflags:%x x:%.1f y:%.1f z:%.1f \\c[FFFFFF][u:%.1f]\\c[303030] v:%.1f argb:%x oargb:%x\n", indiceRndVtx,(unsigned int)vtxRender->flags,vtxRender->x,vtxRender->y,vtxRender->z,vtxRender->u,vtxRender->v,(unsigned int)vtxRender->argb,(unsigned int)vtxRender->oargb);
                                break;
                            case 5:
                                snprintf(dctmodRendVtxTxtRes,sizeof(dctmodRendVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH RENDER   VTX] VTX:%.2d\\c[303030] \nflags:%x x:%.1f y:%.1f z:%.1f u:%.1f \\c[FFFFFF][v:%.1f]\\c[303030] argb:%x oargb:%x\n", indiceRndVtx,(unsigned int)vtxRender->flags,vtxRender->x,vtxRender->y,vtxRender->z,vtxRender->u,vtxRender->v,(unsigned int)vtxRender->argb,(unsigned int)vtxRender->oargb);
                                break;
                            case 6:
                                snprintf(dctmodRendVtxTxtRes,sizeof(dctmodRendVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH RENDER   VTX] VTX:%.2d\\c[303030] \nflags:%x x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f \\c[FFFFFF][argb:%x]\\c[303030] oargb:%x\n", indiceRndVtx,(unsigned int)vtxRender->flags,vtxRender->x,vtxRender->y,vtxRender->z,vtxRender->u,vtxRender->v,(unsigned int)vtxRender->argb,(unsigned int)vtxRender->oargb);
                                break;
                            case 7:
                                snprintf(dctmodRendVtxTxtRes,sizeof(dctmodRendVtxTxtRes), "\\c[FFFFFF]-[DCT MODEL MESH RENDER   VTX] VTX:%.2d\\c[303030] \nflags:%x x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f argb:%x \\c[FFFFFF][oargb:%x]\\c[303030]\n", indiceRndVtx,(unsigned int)vtxRender->flags,vtxRender->x,vtxRender->y,vtxRender->z,vtxRender->u,vtxRender->v,(unsigned int)vtxRender->argb,(unsigned int)vtxRender->oargb);
                                break;
                            
                        }

                    }else
                    {
                        if(entity == debugger.currentEntitySelected)
                        {
                        snprintf(dctmodRendVtxTxtRes,sizeof(dctmodRendVtxTxtRes),"\\c[FFFFFF]-DCT MODEL MESH RENDER   VTX VTX:%.2d \nflags:%x x:%.1f y:%.1f z:%.3f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceRndVtx,(unsigned int)vtxRender->flags,vtxRender->x,vtxRender->y,vtxRender->z,vtxRender->u,vtxRender->v,(unsigned int)vtxRender->argb,(unsigned int)vtxRender->oargb);
                        }else
                        {
                        snprintf(dctmodRendVtxTxtRes,sizeof(dctmodRendVtxTxtRes),"\\c[303030]-DCT MODEL MESH RENDER   VTX VTX:%.2d \nflags:%x x:%.1f y:%.1f z:%.3f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceRndVtx,(unsigned int)vtxRender->flags,vtxRender->x,vtxRender->y,vtxRender->z,vtxRender->u,vtxRender->v,(unsigned int)vtxRender->argb,(unsigned int)vtxRender->oargb);
                        }

                    } 
                    
                    //printf("-DCT MODEL MESH RENDER   VTX VTX:%.2d flags:%x x:%.1f y:%.1f z:%.1f u:%.1f v:%.1f argb:%x oargb:%x\n", indiceRndVtx,vtxRender->flags,vtxRender->x,vtxRender->y,vtxRender->z,vtxRender->u,vtxRender->v,vtxRender->argb,vtxRender->oargb);
                    strcat(debugger.currentTextToDisplay, dctmodRendVtxTxtRes);
                    if(DEBUG_PRINT_ENABLE==1){printf(dctmodRendVtxTxtRes);}
                    break;
                
                case DCT_DBG_MODEL_MESH_HDR:
                    pvr_poly_hdr_t *hdr = entity->data;

                    if(DEBUG_PRINT_ENABLE){printf("-DCT MODEL MESH HDR cmd:%x mode1:%x mode2:%x mode3:%x d1:%x d2:%x d3:%x d4:%x\n", hdr->cmd, hdr->mode1, hdr->mode2, hdr->mode3, hdr->d1, hdr->d2, hdr->d3, hdr->d4 );}
                    if(entity == debugger.currentEntitySelected)
                    {
                    snprintf(tmp, sizeof(tmp),"\\c[FFFFFF]-DCT MODEL MESH HDR cmd:%x mode1:%x mode2:%x mode3:%x d1:%x d2:%x d3:%x d4:%x\n", hdr->cmd, hdr->mode1, hdr->mode2, hdr->mode3, hdr->d1, hdr->d2, hdr->d3, hdr->d4);
                    }else
                    {
                    snprintf(tmp, sizeof(tmp),"\\c[303030]-DCT MODEL MESH HDR cmd:%x mode1:%x mode2:%x mode3:%x d1:%x d2:%x d3:%x d4:%x\n", hdr->cmd, hdr->mode1, hdr->mode2, hdr->mode3, hdr->d1, hdr->d2, hdr->d3, hdr->d4);
                    }
                    strcat(debugger.currentTextToDisplay, tmp);
                    break;
                
                case DCT_DBG_MODEL_MESH_CXT:
                    pvr_poly_cxt_t *cxt = entity->data;

                    char dctmodMeshCxtTxtRes[1000];
                    if(debugger.currentEntitySelected==entity && debugger.state==1)
                    {
                        char *tmp;
                        switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
                        {
                                                                                case -1:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                                                            
                                                                                    break;
                                                                                case 0:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- \\c[FFFFFF][listTypes:%d]\\c[303030] \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 1:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { \\c[FFFFFF][alpha:%d]\\c[303030] shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 2:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d \\c[FFFFFF][shading:%d]\\c[303030] fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 3:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d \\c[FFFFFF][fogtype:%d]\\c[303030] culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 4:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d \\c[FFFFFF][culling:%d]\\c[303030] colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 5:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d \\c[FFFFFF][colorclamp:%d]\\c[303030] \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 6:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   \\c[FFFFFF][clipmode:%d]\\c[303030] modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 7:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d \\c[FFFFFF][modifiermode:%d]\\c[303030] specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 8:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d \\c[FFFFFF][specular:%d]\\c[303030] alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 9:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d \\c[FFFFFF][alpha2:%d]\\c[303030] \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 10:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   \\c[FFFFFF][fogtype2:%d]\\c[303030] colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 11:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d \\c[FFFFFF][colorclamp2:%d]\\c[303030] } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 12:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { \\c[FFFFFF][src:%d]\\c[303030] dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 13:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d \\c[FFFFFF][dst:%d]\\c[303030] src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 14:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d \\c[FFFFFF][src_enable:%d]\\c[303030] dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 15:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d \\c[FFFFFF][dst_enable:%d]\\c[303030] src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 16:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d \\c[FFFFFF][src2:%d]\\c[303030]\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 17:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   \\c[FFFFFF][dst2:%d]\\c[303030] src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 18:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d \\c[FFFFFF][src_enable2:%d]\\c[303030] dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 19:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d \\c[FFFFFF][dst_enable2:%d]\\c[303030] } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 20:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { \\c[FFFFFF][color:%d]\\c[303030]  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 21:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  \\c[FFFFFF][uv:%d]\\c[303030] modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 22:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d \\c[FFFFFF][modifier:%d]\\c[303030] } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 23:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { \\c[FFFFFF][comparison:%d]\\c[303030] write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 24:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d \\c[FFFFFF][write:%d]\\c[303030] } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 25:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { \\c[FFFFFF][enable:%d]\\c[303030] filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 26:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d \\c[FFFFFF][filter:%d]\\c[303030] mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 27:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d \\c[FFFFFF][mipmap:%d]\\c[303030] mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 28:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d \\c[FFFFFF][mipmap_bias:%d]\\c[303030] uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 29:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d \\c[FFFFFF][uv_flip:%d]\\c[303030] \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 30:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   \\c[FFFFFF][uv_clamp:%d]\\c[303030] alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 31:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d \\c[FFFFFF][alpha:%d]\\c[303030] env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 32:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d \\c[FFFFFF][env:%d]\\c[303030] width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 33:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d \\c[FFFFFF][width:%d]\\c[303030]\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 34:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   \\c[FFFFFF][height:%d]\\c[303030] format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 35:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d \\c[FFFFFF][format:%d]\\c[303030] base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 36:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d \\c[FFFFFF][base:%p]\\c[303030]  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 37:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { \\c[FFFFFF][enable:%d]\\c[303030] filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 38:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d \\c[FFFFFF][filter:%d]\\c[303030] mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 39:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d \\c[FFFFFF][mipmap:%d]\\c[303030] mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 40:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d \\c[FFFFFF][mipmap_bias:%d]\\c[303030] uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 41:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d \\c[FFFFFF][uv_flip:%d]\\c[303030]\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 42:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   \\c[FFFFFF][uv_clamp:%d]\\c[303030] alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 43:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d \\c[FFFFFF][alpha:%d]\\c[303030] env:%d width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 44:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d \\c[FFFFFF][env:%d]\\c[303030] width:%d\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 45:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d \\c[FFFFFF][width:%d]\\c[303030]\n   height:%d format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 46:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   \\c[FFFFFF][height:%d]\\c[303030] format:%d base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                                case 47:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d \\c[FFFFFF][format:%d]\\c[303030] base:%p  }  \n";
                                                                                
                                                                                    break;
                                                                case 48:
                                                                                tmp =  "\\c[FFFFFF]-[DCT MODEL MESH CXT]\\c[303030] \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d \\c[FFFFFF][base:%p]\\c[303030]  }  \n";
                                                                    
                                                                break;
                            
                            
                        }
                        
                        snprintf(dctmodMeshCxtTxtRes,sizeof(dctmodMeshCxtTxtRes), tmp,
                                                        (int)cxt->list_type,(int)cxt->gen.alpha,(int)cxt->gen.shading,(int)cxt->gen.fog_type,(int)cxt->gen.culling,(int)cxt->gen.color_clamp,
                                        (int)cxt->gen.clip_mode,(int)cxt->gen.modifier_mode,(int)cxt->gen.specular,(int)cxt->gen.alpha2,
                                        (int)cxt->gen.fog_type2,(int)cxt->gen.color_clamp2, 
                                        cxt->blend.src, cxt->blend.dst,cxt->blend.src_enable,cxt->blend.dst_enable,cxt->blend.src2, cxt->blend.dst2,cxt->blend.src_enable2,cxt->blend.dst_enable2,
                                        (int)cxt->fmt.color,(int)cxt->fmt.uv,(int)cxt->fmt.modifier,
                                        (int)cxt->depth.comparison,(int)cxt->depth.write,
                                        (int)cxt->txr.enable,(int)cxt->txr.filter,(int)cxt->txr.mipmap,
                                        (int)cxt->txr.mipmap_bias,(int)cxt->txr.uv_flip,(int)cxt->txr.uv_clamp,(int)cxt->txr.alpha,(int)cxt->txr.env,
                                        (int)cxt->txr.width,(int)cxt->txr.height,(int)cxt->txr.format,(void*)cxt->txr.base,(int)cxt->txr2.enable,(int)cxt->txr2.filter,(int)cxt->txr2.mipmap,
                                        (int)cxt->txr2.mipmap_bias,(int)cxt->txr2.uv_flip,(int)cxt->txr2.uv_clamp,(int)cxt->txr2.alpha,(int)cxt->txr2.env,
                                        (int)cxt->txr2.width,(int)cxt->txr2.height,(int)cxt->txr2.format,(void*)cxt->txr2.base );
                        

                    }else
                    {
                        if(entity == debugger.currentEntitySelected)
                        {
                            snprintf(dctmodMeshCxtTxtRes,sizeof(dctmodMeshCxtTxtRes),"\\c[FFFFFF]-DCT MODEL MESH CXT \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n",
                    (int)cxt->list_type,(int)cxt->gen.alpha,(int)cxt->gen.shading,(int)cxt->gen.fog_type,(int)cxt->gen.culling,(int)cxt->gen.color_clamp,
            (int)cxt->gen.clip_mode,(int)cxt->gen.modifier_mode,(int)cxt->gen.specular,(int)cxt->gen.alpha2,
            (int)cxt->gen.fog_type2,(int)cxt->gen.color_clamp2, 
            cxt->blend.src, cxt->blend.dst,cxt->blend.src_enable,cxt->blend.dst_enable,cxt->blend.src2, cxt->blend.dst2,cxt->blend.src_enable2,cxt->blend.dst_enable2,
            (int)cxt->fmt.color,(int)cxt->fmt.uv,(int)cxt->fmt.modifier,
            (int)cxt->depth.comparison,(int)cxt->depth.write,
            (int)cxt->txr.enable,(int)cxt->txr.filter,(int)cxt->txr.mipmap,
            (int)cxt->txr.mipmap_bias,(int)cxt->txr.uv_flip,(int)cxt->txr.uv_clamp,(int)cxt->txr.alpha,(int)cxt->txr.env,
            (int)cxt->txr.width,(int)cxt->txr.height,(int)cxt->txr.format,(void*)cxt->txr.base,(int)cxt->txr2.enable,(int)cxt->txr2.filter,(int)cxt->txr2.mipmap,
            (int)cxt->txr2.mipmap_bias,(int)cxt->txr2.uv_flip,(int)cxt->txr2.uv_clamp,(int)cxt->txr2.alpha,(int)cxt->txr2.env,
            (int)cxt->txr2.width,(int)cxt->txr2.height,(int)cxt->txr2.format,(void*)cxt->txr2.base );
                        }else
                        {
                            snprintf(dctmodMeshCxtTxtRes,sizeof(dctmodMeshCxtTxtRes),"\\c[303030]-DCT MODEL MESH CXT \n-- listTypes:%d \n-- gen\n { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d \n   clipmode:%d modifiermode:%d specular:%d alpha2:%d \n   fogtype2:%d colorclamp2:%d } \n-- blend\n { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d\n   dst2:%d src_enable2:%d dst_enable2:%d } \n-- fmt\n { color:%d  uv:%d modifier:%d } \n-- depth\n { comparison:%d write:%d } \n-- txr \n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d \n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  } \n-- txr2\n { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d\n   uv_clamp:%d alpha:%d env:%d width:%d\n   height:%d format:%d base:%p  }  \n",
                    (int)cxt->list_type,(int)cxt->gen.alpha,(int)cxt->gen.shading,(int)cxt->gen.fog_type,(int)cxt->gen.culling,(int)cxt->gen.color_clamp,
            (int)cxt->gen.clip_mode,(int)cxt->gen.modifier_mode,(int)cxt->gen.specular,(int)cxt->gen.alpha2,
            (int)cxt->gen.fog_type2,(int)cxt->gen.color_clamp2, 
            cxt->blend.src, cxt->blend.dst,cxt->blend.src_enable,cxt->blend.dst_enable,cxt->blend.src2, cxt->blend.dst2,cxt->blend.src_enable2,cxt->blend.dst_enable2,
            (int)cxt->fmt.color,(int)cxt->fmt.uv,(int)cxt->fmt.modifier,
            (int)cxt->depth.comparison,(int)cxt->depth.write,
            (int)cxt->txr.enable,(int)cxt->txr.filter,(int)cxt->txr.mipmap,
            (int)cxt->txr.mipmap_bias,(int)cxt->txr.uv_flip,(int)cxt->txr.uv_clamp,(int)cxt->txr.alpha,(int)cxt->txr.env,
            (int)cxt->txr.width,(int)cxt->txr.height,(int)cxt->txr.format,(void*)cxt->txr.base,(int)cxt->txr2.enable,(int)cxt->txr2.filter,(int)cxt->txr2.mipmap,
            (int)cxt->txr2.mipmap_bias,(int)cxt->txr2.uv_flip,(int)cxt->txr2.uv_clamp,(int)cxt->txr2.alpha,(int)cxt->txr2.env,
            (int)cxt->txr2.width,(int)cxt->txr2.height,(int)cxt->txr2.format,(void*)cxt->txr2.base );
                        }

                        

                    } 

                    // printf("-DCT MODEL MESH CXT \n-DCT MODEL MESH CXT -- listTypes:%d \n-DCT MODEL MESH CXT -- gen    { alpha:%d shading:%d fogtype:%d culling:%d colorclamp:%d clipmode:%d \n-DCT MODEL MESH CXT             modifiermode:%d specular:%d alpha2:%d fogtype2:%d colorclamp2:%d } \n-DCT MODEL MESH CXT -- blend  { src:%d dst:%d src_enable:%d dst_enable:%d src2:%d dst2:%d src_enable2:%d dst_enable2:%d } \n-DCT MODEL MESH CXT -- fmt    { color:%d  uv:%d modifier:%d } depth { comparison:%d write:%d } \n-DCT MODEL MESH CXT -- txr    { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d uv_clamp:%d \n-DCT MODEL MESH CXT             alpha:%d env:%d width:%d height:%d format:%d base:%d  } \n-DCT MODEL MESH CXT -- txr2   { enable:%d filter:%d mipmap:%d mipmap_bias:%d uv_flip:%d uv_clamp:%d \n-DCT MODEL MESH CXT             alpha:%d env:%d width:%d height:%d format:%d base:%d  }  \n",
                    // cxt->list_type,cxt->gen.alpha,cxt->gen.shading,cxt->gen.fog_type,cxt->gen.culling,cxt->gen.color_clamp,
                    // cxt->gen.clip_mode,cxt->gen.modifier_mode,cxt->gen.specular,cxt->gen.specular,cxt->gen.alpha2,
                    // cxt->gen.fog_type2,cxt->gen.color_clamp2, cxt->fmt.color,cxt->fmt.uv,cxt->fmt.modifier,
                    // cxt->depth.comparison,cxt->depth.write,cxt->txr.enable,cxt->txr.filter,cxt->txr.mipmap,
                    // cxt->txr.mipmap_bias,cxt->txr.uv_flip,cxt->txr.uv_clamp,cxt->txr.alpha,cxt->txr.env,
                    // cxt->txr.width,cxt->txr.height,cxt->txr.format,cxt->txr.base,cxt->txr2.enable,cxt->txr2.filter,cxt->txr2.mipmap,
                    // cxt->txr2.mipmap_bias,cxt->txr2.uv_flip,cxt->txr2.uv_clamp,cxt->txr2.alpha,cxt->txr2.env,
                    // cxt->txr2.width,cxt->txr2.height,cxt->txr2.format,cxt->txr2.base 
                    // );
                    strcat(debugger.currentTextToDisplay, dctmodMeshCxtTxtRes);
                    if(DEBUG_PRINT_ENABLE==1){printf(dctmodMeshCxtTxtRes);}
                    break;
                

            }


        }
        

        
    }


    }
    

    if(DEBUG_PRINT_ENABLE==1){printf("currentTextToDisplay :\n%s \n",debugger.currentTextToDisplay);}
    if(debugger.textureDisplay)
    {
        update_text_texture(debugger.textureDisplay, debugger.currentTextToDisplay, debugger.font, 0xB0808080);
    }
}



void updateDebuggerStateNavigate(DCT_ControllerState *controller)
{

    if(controller->just_pressed.down)
    {   
        if(DEBUG_PRINT_ENABLE){printf("\nbtn just pressed down \n");
        printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}
        
        if(debugger.currentEntitySelected->nextEntSibling != NULL)
        {
            debugger.currentEntitySelected = debugger.currentEntitySelected->nextEntSibling;
        }
        
        
        if(DEBUG_PRINT_ENABLE){printf("\ncurrent entity selected After Addr:%x \n",debugger.currentEntitySelected);
        printf("\n--- MODE %d ----indiceCurrentDataSelected:%d  \n",debugger.state,debugger.currentEntitySelected->indiceCurrentDataSelected);}

        
        updateTexteTextureMain();
    }
    
    if(controller->just_pressed.up)
    {
        if(DEBUG_PRINT_ENABLE){printf("\nbtn just pressed up \n");
        printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}
        if(debugger.currentEntitySelected->prevEntSibling != NULL)
        {
            debugger.currentEntitySelected = debugger.currentEntitySelected->prevEntSibling;

        }
        
        if(DEBUG_PRINT_ENABLE){printf("\ncurrent entity selected After Addr:%x \n",debugger.currentEntitySelected);
        printf("\n--- MODE %d ----indiceCurrentDataSelected:%d  \n",debugger.state,debugger.currentEntitySelected->indiceCurrentDataSelected);}

        updateTexteTextureMain();
    }
    
    if(controller->just_pressed.left)
    {
        if(DEBUG_PRINT_ENABLE){printf("\nbtn just pressed left \n");
        printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}

        if(debugger.currentEntitySelected->parentEntity != NULL)
        {
            debugger.currentEntitySelected = debugger.currentEntitySelected->parentEntity;

            for (int i=0; i<NUM_ENTITIES_TO_DRAW; i++)
            {
                debugger.entitiesToDraw[i]=NULL;
            }
            
            if(debugger.currentEntitySelected->parentEntity!=NULL)
            {
                dct_menu_entityGameObject *p2 = debugger.currentEntitySelected->parentEntity;
                dct_menu_entityGameObject **childs = p2->childEntities;
                //printf("\nreinit Entities to Draw Based on parentEntity Childs \n");
                for(int i=0; i<p2->childEntitiesCount; i++)
                {
                    //printf("\nchild to add \n");
                    if(i<NUM_ENTITIES_TO_DRAW-1)
                    {
                        debugger.entitiesToDraw[i] = childs[i];
                    }
                    
                }
            }
            else
            {
                if(DEBUG_PRINT_ENABLE){printf("\nPas de Parent Au dessus \n");}
            }

            if( debugger.currentEntitySelected==&debugger.switchBtn)
            {
                if(DEBUG_PRINT_ENABLE){printf("\nfree texture display \n");}
                free_alpha_texture(debugger.textureDisplay);
                //free(debugger.textureDisplay);
                debugger.textureDisplay = NULL;
                debugger.currentState = MINIFY;
            }

            

        }
            
        

        if(DEBUG_PRINT_ENABLE){printf("\ncurrent entity selected After Addr:%x \n",debugger.currentEntitySelected);
        printf("\n--- MODE %d ----indiceCurrentDataSelected:%d  \n",debugger.state,debugger.currentEntitySelected->indiceCurrentDataSelected);}


        updateTexteTextureMain();
        updateTexteTextureBreadcrumb();

    }
    
    if(controller->just_pressed.right)
    {
        if(DEBUG_PRINT_ENABLE){printf("\nbtn just pressed right \n");
        printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}
        if(debugger.currentEntitySelected->data != NULL && debugger.currentEntitySelected->childEntities == NULL && isCurrentEntitySelectedInMainMenu()==false )
        {
            if(debugger.state ==0)
            {
                if(DEBUG_PRINT_ENABLE){printf("fn press right change menu state \n");}
                debugger.state = 1; 
                debugger.currentState = EDIT;
            }
        }

        // DISPLAY
        // right --> childs
        if(debugger.currentEntitySelected->childEntities != NULL)
        {
            
            for (int i=0; i<NUM_ENTITIES_TO_DRAW; i++)
            {
                debugger.entitiesToDraw[i]=NULL;
            }

            dct_menu_entityGameObject **childs = debugger.currentEntitySelected->childEntities;
            
            for(int i=0; i<debugger.currentEntitySelected->childEntitiesCount; i++)
            {
                if(i<NUM_ENTITIES_TO_DRAW-1)
                {
                    debugger.entitiesToDraw[i] = childs[i];
                }
                
            }

            debugger.currentEntitySelected = debugger.entitiesToDraw[0];

        }

        if( debugger.currentEntitySelected!=&debugger.switchBtn && debugger.textureDisplay==NULL)
        {
            // Utiliser une allocation statique pour le buffer temporaire
            char *fill = (char*)malloc(4096);
            if (!fill) {
                printf("Erreur allocation buffer\n");
                return;
            }
            
            memset(fill, 0, 4096); // Initialiser à 0
            for (int i=0;i<52 && strlen(fill)+65<4096;i++)
            {
                strcat(fill,"                                                                \n");
            }
            debugger.textureDisplay = create_text_texture_with_colors(fill , debugger.font, 0x305FACFF,512,512 );
            free(fill);
        }

        if(DEBUG_PRINT_ENABLE){printf("\n--- MODE %d ----indiceCurrentDataSelected:%d  \n",debugger.state,debugger.currentEntitySelected->indiceCurrentDataSelected);
        printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}

        
        updateTexteTextureMain();
        updateTexteTextureBreadcrumb();
    }

}


void updateDebuggerStateEdit(DCT_ControllerState *controller)
{

    if(controller->just_pressed.down)
    {   
        if(DEBUG_PRINT_ENABLE){printf("\nbtn just pressed down \n");
        printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}
        
        switch(debugger.currentEntitySelected->type)
        {
            case DCT_DBG_MODEL_POSITION:
                vec3f_t *pos = debugger.currentEntitySelected->data;
                switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
                {
                    case 0:
                        pos->x -= 0.1f;
                        break;
                    case 1:
                        pos->y -= 0.1f;
                        break;
                    case 2:
                        pos->z -= 0.1f;
                        break;
                }
                break;
            case DCT_DBG_MODEL_ROTATION:
                vec3f_t *rot = debugger.currentEntitySelected->data;
                switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
                {
                    case 0:
                        rot->x -= 0.1f;
                        break;
                    case 1:
                        rot->y -= 0.1f;
                        break;
                    case 2:
                        rot->z -= 0.1f;
                        break;
                }
                break;
            case DCT_DBG_MODEL_MESH_CXT:
                pvr_poly_cxt_t *cxt = debugger.currentEntitySelected->data;
                switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
                {
                    /*(int)cxt->list_type,(int)cxt->gen.alpha,(int)cxt->gen.shading,(int)cxt->gen.fog_type,(int)cxt->gen.culling,(int)cxt->gen.color_clamp,
                                (int)cxt->gen.clip_mode,(int)cxt->gen.modifier_mode,(int)cxt->gen.specular,(int)cxt->gen.alpha2,
                                (int)cxt->gen.fog_type2,(int)cxt->gen.color_clamp2, 
                                cxt->blend.src, cxt->blend.dst,cxt->blend.src_enable,cxt->blend.dst_enable,cxt->blend.src2, cxt->blend.dst2,cxt->blend.src_enable2,cxt->blend.dst_enable2,
                                (int)cxt->fmt.color,(int)cxt->fmt.uv,(int)cxt->fmt.modifier,
                                (int)cxt->depth.comparison,(int)cxt->depth.write,
                                (int)cxt->txr.enable,(int)cxt->txr.filter,(int)cxt->txr.mipmap,
                                (int)cxt->txr.mipmap_bias,(int)cxt->txr.uv_flip,(int)cxt->txr.uv_clamp,(int)cxt->txr.alpha,(int)cxt->txr.env,
                                (int)cxt->txr.width,(int)cxt->txr.height,(int)cxt->txr.format,(void*)cxt->txr.base,(int)cxt->txr2.enable,(int)cxt->txr2.filter,(int)cxt->txr2.mipmap,
                                (int)cxt->txr2.mipmap_bias,(int)cxt->txr2.uv_flip,(int)cxt->txr2.uv_clamp,(int)cxt->txr2.alpha,(int)cxt->txr2.env,
                                (int)cxt->txr2.width,(int)cxt->txr2.height,(int)cxt->txr2.format,(void*)cxt->txr2.base
                    */
                    case 0:
                        if(cxt->list_type>0)
                        {
                            cxt->list_type -= 1; // 0 1 2 3  PVR_LIST_OP_POLY   ,PVR_LIST_OP_MOD  ,PVR_LIST_TR_POLY ,PVR_LIST_TR_MOD ,PVR_LIST_PT_POLY 
                        }
                        break;
                    case 1:
                        if(cxt->gen.alpha >0)
                        {
                            cxt->gen.alpha -= 1; // 0 1 PVR_ALPHA_DISABLE ,PVR_ALPHA_ENABLE  
                        }
                        break;
                    case 2:
                        if(cxt->gen.shading >0)
                        {
                            cxt->gen.shading -= 1; // 0 1 FLAT GOURAUD
                        }
                        break;
                    case 3:
                        if(cxt->gen.fog_type >0)
                        {
                            cxt->gen.fog_type -= 1; // 0 1 2 3 PVR_FOG_TABLE ,PVR_FOG_VERTEX  ,PVR_FOG_DISABLE ,PVR_FOG_TABLE2  
                        }
                        break;
                    case 4:
                        if(cxt->gen.culling >0)
                        {
                            cxt->gen.culling -= 1; // 0 1 2 3 PVR_CULLING_NONE, PVR_CULLING_SMALL ,PVR_CULLING_CCW  ,PVR_CULLING_CW   
                        }
                        break;
                    case 5:
                        if(cxt->gen.color_clamp >0)
                        {
                            cxt->gen.color_clamp -= 1; // 0 1 
                        }
                        break;
                    case 6:
                        if(cxt->gen.clip_mode >0)
                        {
                            cxt->gen.clip_mode -= 1; // 0 2 3   PVR_USERCLIP_DISABLE  , PVR_USERCLIP_INSIDE     ,PVR_USERCLIP_OUTSIDE    
                        }
                        break;
                    case 7:
                        if(cxt->gen.modifier_mode >0)
                        {
                            cxt->gen.modifier_mode -= 1; // 0 1
                        }
                        break;
                    case 8:
                        if(cxt->gen.specular >0)
                        {
                            cxt->gen.specular -= 1; // 0 1
                        }
                        break;
                    case 9:
                        if(cxt->gen.alpha2 >0)
                        {
                            cxt->gen.alpha2 -= 1; // 0 1 PVR_ALPHA_DISABLE ,PVR_ALPHA_ENABLE  
                        }
                        break;
                    case 10:
                        if(cxt->gen.fog_type2 >0)
                        {
                            cxt->gen.fog_type2 -= 1; // 0 1 2 3
                        }
                        break;
                    case 11:
                        if(cxt->gen.color_clamp2 >0)
                        {
                            cxt->gen.color_clamp2 -= 1; // 0 1
                        }
                        break;
                    case 12:
                        if(cxt->blend.src >0)
                        {
                            cxt->blend.src -= 1; // 0 1 2 3 4 5 6 7 PVR_BLEND_ZERO ,PVR_BLEND_ONE ,PVR_BLEND_DESTCOLOR
                        }
                        break;
                    case 13:
                        if(cxt->blend.dst >0)
                        {
                            cxt->blend.dst -= 1; // 0 1 2 3 4 5 6 7 PVR_BLEND_ZERO ,PVR_BLEND_ONE ,PVR_BLEND_DESTCOLOR
                        }
                        break;
                    case 23:
                        if(cxt->depth.comparison >0)
                        {
                            cxt->depth.comparison -= 1; // 0 1 2 3 4 5 6 7 PVR_DEPTHCMP_NEVER,PVR_DEPTHCMP_LESS,PVR_DEPTHCMP_EQUAL,PVR_DEPTHCMP_LEQUAL,PVR_DEPTHCMP_GREATER,PVR_DEPTHCMP_NOTEQUAL,PVR_DEPTHCMP_GEQUAL,PVR_DEPTHCMP_ALWAYS     
                        }
                        break;
                    case 24:
                        if(cxt->depth.write >0)
                        {
                            cxt->depth.write -= 1; // 0 1 
                        }
                        break;
                }
                compileCxtHdrOfCurrentModelSelected(debugger.currentEntitySelected);
                break;
            case DCT_DBG_MODEL_MESH_ANIMATEDVTX_VTX:
                pvr_vertex_t *vtx = debugger.currentEntitySelected->data;
                switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
                {
                    case 1:
                        vtx->x -= 0.1f;
                        break;
                    case 2:
                        vtx->y -= 0.1f;
                        break;
                    case 3:
                        vtx->z -= 0.1f;
                        break;
                }
                break;
        }
        
        if(DEBUG_PRINT_ENABLE){printf("\ncurrent entity selected After Addr:%x \n",debugger.currentEntitySelected);
        printf("\n--- MODE %d ----indiceCurrentDataSelected:%d  \n",debugger.state,debugger.currentEntitySelected->indiceCurrentDataSelected);}

        
        updateTexteTextureMain();
    }
    
    if(controller->just_pressed.up)
    {
        if(DEBUG_PRINT_ENABLE){printf("\nbtn just pressed up \n");
        printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}
        
        switch(debugger.currentEntitySelected->type)
        {
            case DCT_DBG_MODEL_POSITION:
                vec3f_t *pos = debugger.currentEntitySelected->data;
                switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
                {
                    case 0:
                        pos->x += 0.1f;
                        break;
                    case 1:
                        pos->y += 0.1f;
                        break;
                    case 2:
                        pos->z += 0.1f;
                        break;
                }
                break;
            case DCT_DBG_MODEL_ROTATION:
                vec3f_t *rot = debugger.currentEntitySelected->data;
                switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
                {
                    case 0:
                        rot->x += 0.1f;
                        break;
                    case 1:
                        rot->y += 0.1f;
                        break;
                    case 2:
                        rot->z += 0.1f;
                        break;
                }
                break;
            
            case DCT_DBG_MODEL_MESH_CXT:
                pvr_poly_cxt_t *cxt = debugger.currentEntitySelected->data;
                switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
                {
                    /*(int)cxt->list_type,(int)cxt->gen.alpha,(int)cxt->gen.shading,(int)cxt->gen.fog_type,(int)cxt->gen.culling,(int)cxt->gen.color_clamp,
                                (int)cxt->gen.clip_mode,(int)cxt->gen.modifier_mode,(int)cxt->gen.specular,(int)cxt->gen.alpha2,
                                (int)cxt->gen.fog_type2,(int)cxt->gen.color_clamp2, 
                                cxt->blend.src, cxt->blend.dst,cxt->blend.src_enable,cxt->blend.dst_enable,cxt->blend.src2, cxt->blend.dst2,cxt->blend.src_enable2,cxt->blend.dst_enable2,
                                (int)cxt->fmt.color,(int)cxt->fmt.uv,(int)cxt->fmt.modifier,
                                (int)cxt->depth.comparison,(int)cxt->depth.write,
                                (int)cxt->txr.enable,(int)cxt->txr.filter,(int)cxt->txr.mipmap,
                                (int)cxt->txr.mipmap_bias,(int)cxt->txr.uv_flip,(int)cxt->txr.uv_clamp,(int)cxt->txr.alpha,(int)cxt->txr.env,
                                (int)cxt->txr.width,(int)cxt->txr.height,(int)cxt->txr.format,(void*)cxt->txr.base,(int)cxt->txr2.enable,(int)cxt->txr2.filter,(int)cxt->txr2.mipmap,
                                (int)cxt->txr2.mipmap_bias,(int)cxt->txr2.uv_flip,(int)cxt->txr2.uv_clamp,(int)cxt->txr2.alpha,(int)cxt->txr2.env,
                                (int)cxt->txr2.width,(int)cxt->txr2.height,(int)cxt->txr2.format,(void*)cxt->txr2.base
                    */
                    case 0:
                        if(cxt->list_type<3)
                        {
                            cxt->list_type += 1; // 0 1 2 3  PVR_LIST_OP_POLY   ,PVR_LIST_OP_MOD  ,PVR_LIST_TR_POLY ,PVR_LIST_TR_MOD ,PVR_LIST_PT_POLY 
                        }
                        break;
                    case 1:
                        if(cxt->gen.alpha <1)
                        {
                            cxt->gen.alpha += 1; // 0 1 PVR_ALPHA_DISABLE ,PVR_ALPHA_ENABLE  
                        }
                        break;
                    case 2:
                        if(cxt->gen.shading <1)
                        {
                            cxt->gen.shading += 1; // 0 1 FLAT GOURAUD
                        }
                        break;
                    case 3:
                        if(cxt->gen.fog_type <3)
                        {
                            cxt->gen.fog_type += 1; // 0 1 2 3 PVR_FOG_TABLE ,PVR_FOG_VERTEX  ,PVR_FOG_DISABLE ,PVR_FOG_TABLE2  
                        }
                        break;
                    case 4:
                        if(cxt->gen.culling <3)
                        {
                            cxt->gen.culling += 1; // 0 1 2 3 PVR_CULLING_NONE, PVR_CULLING_SMALL ,PVR_CULLING_CCW  ,PVR_CULLING_CW   
                        }
                        break;
                    case 5:
                        if(cxt->gen.color_clamp <1)
                        {
                            cxt->gen.color_clamp += 1; // 0 1 
                        }
                        break;
                    case 6:
                        if(cxt->gen.clip_mode <3)
                        {
                            cxt->gen.clip_mode += 1; // 0 2 3   PVR_USERCLIP_DISABLE  , PVR_USERCLIP_INSIDE     ,PVR_USERCLIP_OUTSIDE    
                        }
                        break;
                    case 7:
                        if(cxt->gen.modifier_mode <1)
                        {
                            cxt->gen.modifier_mode += 1; // 0 1
                        }
                        break;
                    case 8:
                        if(cxt->gen.specular <1)
                        {
                            cxt->gen.specular += 1; // 0 1
                        }
                        break;
                    case 9:
                        if(cxt->gen.alpha2 <1)
                        {
                            cxt->gen.alpha2 += 1; // 0 1 PVR_ALPHA_DISABLE ,PVR_ALPHA_ENABLE  
                        }
                        break;
                    case 10:
                        if(cxt->gen.fog_type2 <3)
                        {
                            cxt->gen.fog_type2 += 1; // 0 1 2 3
                        }
                        break;
                    case 11:
                        if(cxt->gen.color_clamp2 <1)
                        {
                            cxt->gen.color_clamp2 += 1; // 0 1
                        }
                        break;
                    case 12:
                        if(cxt->blend.src <7)
                        {
                            cxt->blend.src += 1; // 0 1 2 3 4 5 6 7 PVR_BLEND_ZERO ,PVR_BLEND_ONE ,PVR_BLEND_DESTCOLOR
                        }
                        break;
                    case 13:
                        if(cxt->blend.dst <7)
                        {
                            cxt->blend.dst += 1; // 0 1 2 3 4 5 6 7 PVR_BLEND_ZERO ,PVR_BLEND_ONE ,PVR_BLEND_DESTCOLOR
                        }
                        break;
                    case 23:
                        if(cxt->depth.comparison <7)
                        {
                            cxt->depth.comparison += 1; // 0 1 2 3 4 5 6 7 PVR_DEPTHCMP_NEVER,PVR_DEPTHCMP_LESS,PVR_DEPTHCMP_EQUAL,PVR_DEPTHCMP_LEQUAL,PVR_DEPTHCMP_GREATER,PVR_DEPTHCMP_NOTEQUAL,PVR_DEPTHCMP_GEQUAL,PVR_DEPTHCMP_ALWAYS     
                        }
                        break;
                    case 24:
                        if(cxt->depth.write <1)
                        {
                            cxt->depth.write += 1; // 0 1 
                        }
                        break;
                }
                compileCxtHdrOfCurrentModelSelected(debugger.currentEntitySelected);
                break;
            case DCT_DBG_MODEL_MESH_ANIMATEDVTX_VTX:
                pvr_vertex_t *vtx = debugger.currentEntitySelected->data;
                switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
                {
                    case 1:
                        vtx->x += 0.1f;
                        break;
                    case 2:
                        vtx->y += 0.1f;
                        break;
                    case 3:
                        vtx->z += 0.1f;
                        break;
                }
                break;
        }
        
        
        if(DEBUG_PRINT_ENABLE){printf("\ncurrent entity selected After Addr:%x \n",debugger.currentEntitySelected);
        printf("\n--- MODE %d ----indiceCurrentDataSelected:%d  \n",debugger.state,debugger.currentEntitySelected->indiceCurrentDataSelected);}

        updateTexteTextureMain();
    }
    
    if(controller->just_pressed.left)
    {
        if(DEBUG_PRINT_ENABLE){printf("\nbtn just pressed left \n");
        printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}

        if(debugger.currentEntitySelected->indiceCurrentDataSelected >= 0)
        {
            debugger.currentEntitySelected->indiceCurrentDataSelected -= 1;
        }
        if(debugger.currentEntitySelected->indiceCurrentDataSelected < 0)
        {
            debugger.state = 0; 
            debugger.currentState = NAVIGATE;
        }
        

        if(DEBUG_PRINT_ENABLE){printf("\ncurrent entity selected After Addr:%x \n",debugger.currentEntitySelected);
        printf("\n--- MODE %d ----indiceCurrentDataSelected:%d  \n",debugger.state,debugger.currentEntitySelected->indiceCurrentDataSelected);}


        updateTexteTextureMain();
        updateTexteTextureBreadcrumb();

    }
    
    if(controller->just_pressed.right)
    {
        if(DEBUG_PRINT_ENABLE){printf("\nbtn just pressed right \n");
        printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}
        // EDIT
        if(debugger.currentEntitySelected->indiceCurrentDataSelected <  debugger.currentEntitySelected->currentDataSelectedCount)
        {
            debugger.currentEntitySelected->indiceCurrentDataSelected += 1;
        }

        if(DEBUG_PRINT_ENABLE){printf("\n--- MODE %d ----indiceCurrentDataSelected:%d  \n",debugger.state,debugger.currentEntitySelected->indiceCurrentDataSelected);
        printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}

        
        updateTexteTextureMain();
        updateTexteTextureBreadcrumb();
    }
}

void updateDebuggerStateMinify(DCT_ControllerState *controller)
{
    
    if(controller->just_pressed.right)
    {
        if(DEBUG_PRINT_ENABLE){printf("\nbtn just pressed right \n");
        printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}

        if(debugger.currentEntitySelected->childEntities != NULL)
        {
            
            for (int i=0; i<NUM_ENTITIES_TO_DRAW; i++)
            {
                debugger.entitiesToDraw[i]=NULL;
            }

            dct_menu_entityGameObject **childs = debugger.currentEntitySelected->childEntities;
            
            for(int i=0; i<debugger.currentEntitySelected->childEntitiesCount; i++)
            {
                if(i<NUM_ENTITIES_TO_DRAW-1)
                {
                    debugger.entitiesToDraw[i] = childs[i];
                }
                
            }

            debugger.currentEntitySelected = debugger.entitiesToDraw[0];

        }
        debugger.currentState = NAVIGATE;
        //printf("updateDebuggerStateMinify creat texture currentState :%d \n",debugger.currentState);
        if( debugger.currentEntitySelected!=&debugger.switchBtn && debugger.textureDisplay==NULL)
        {
            
            // Utiliser une allocation statique pour le buffer temporaire
            debugger.currentState = NAVIGATE;
            
            char *fill = (char*)malloc(4096);
            if (!fill) {
                printf("Erreur allocation buffer\n");
                return;
            }
            
            memset(fill, 0, 4096); // Initialiser à 0
            for (int i=0;i<52 && strlen(fill)+65<4096;i++)
            {
                strcat(fill,"                                                                \n");
            }
            debugger.textureDisplay = create_text_texture_with_colors(fill , debugger.font, 0xE0FF0000,512,512 );
            free(fill);

            
        }

        if(DEBUG_PRINT_ENABLE){printf("\n--- MODE %d ----indiceCurrentDataSelected:%d  \n",debugger.state,debugger.currentEntitySelected->indiceCurrentDataSelected);
        printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}

        
        updateTexteTextureMain();
        updateTexteTextureBreadcrumb();
    }

}

void updateDebugger(DCT_ControllerState *controller)
{
    switch(debugger.currentState)
    {
        case OFF:
            break;
        case MINIFY:
            updateDebuggerStateMinify(controller);
            break;
        case DISPLAY:
            if(controller->just_pressed.b)
            {
                // switch to NAVIGATE 
                printf("switch to navigate \n");
                debugger.currentState =  NAVIGATE;
            }
            
            break;
        case NAVIGATE:
            updateDebuggerStateNavigate(controller);
            if(controller->just_pressed.b)
            {
                // switch to display 
                printf("switch to display \n");
                debugger.currentState = DISPLAY;
            }
            break;
        case EDIT:
            updateDebuggerStateEdit(controller);
            break;
        

    }
    
    
    // if(controller->just_pressed.down)
    // {   
    //     if(DEBUG_PRINT_ENABLE){printf("\nbtn just pressed down \n");
    //     printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}
        
    //     switch(debugger.state)
    //     {
    //         case 0:
    //             // DISPLAY
    //             // move down --> nextSibling
    //             if(debugger.currentEntitySelected->nextEntSibling != NULL)
    //             {
    //                 debugger.currentEntitySelected = debugger.currentEntitySelected->nextEntSibling;
    //             }
    //             break;
    //         case 1:
    //             switch(debugger.currentEntitySelected->type)
    //             {
    //                 case DCT_DBG_MODEL_POSITION:
    //                     vec3f_t *pos = debugger.currentEntitySelected->data;
    //                     switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
    //                     {
    //                         case 0:
    //                             pos->x -= 0.1f;
    //                             break;
    //                         case 1:
    //                             pos->y -= 0.1f;
    //                             break;
    //                         case 2:
    //                             pos->z -= 0.1f;
    //                             break;
    //                     }
    //                     break;
    //                 case DCT_DBG_MODEL_ROTATION:
    //                     vec3f_t *rot = debugger.currentEntitySelected->data;
    //                     switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
    //                     {
    //                         case 0:
    //                             rot->x -= 0.1f;
    //                             break;
    //                         case 1:
    //                             rot->y -= 0.1f;
    //                             break;
    //                         case 2:
    //                             rot->z -= 0.1f;
    //                             break;
    //                     }
    //                     break;
    //                 case DCT_DBG_MODEL_MESH_CXT:
    //                     pvr_poly_cxt_t *cxt = debugger.currentEntitySelected->data;
    //                     switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
    //                     {
    //                         /*(int)cxt->list_type,(int)cxt->gen.alpha,(int)cxt->gen.shading,(int)cxt->gen.fog_type,(int)cxt->gen.culling,(int)cxt->gen.color_clamp,
    //                                     (int)cxt->gen.clip_mode,(int)cxt->gen.modifier_mode,(int)cxt->gen.specular,(int)cxt->gen.alpha2,
    //                                     (int)cxt->gen.fog_type2,(int)cxt->gen.color_clamp2, 
    //                                     cxt->blend.src, cxt->blend.dst,cxt->blend.src_enable,cxt->blend.dst_enable,cxt->blend.src2, cxt->blend.dst2,cxt->blend.src_enable2,cxt->blend.dst_enable2,
    //                                     (int)cxt->fmt.color,(int)cxt->fmt.uv,(int)cxt->fmt.modifier,
    //                                     (int)cxt->depth.comparison,(int)cxt->depth.write,
    //                                     (int)cxt->txr.enable,(int)cxt->txr.filter,(int)cxt->txr.mipmap,
    //                                     (int)cxt->txr.mipmap_bias,(int)cxt->txr.uv_flip,(int)cxt->txr.uv_clamp,(int)cxt->txr.alpha,(int)cxt->txr.env,
    //                                     (int)cxt->txr.width,(int)cxt->txr.height,(int)cxt->txr.format,(void*)cxt->txr.base,(int)cxt->txr2.enable,(int)cxt->txr2.filter,(int)cxt->txr2.mipmap,
    //                                     (int)cxt->txr2.mipmap_bias,(int)cxt->txr2.uv_flip,(int)cxt->txr2.uv_clamp,(int)cxt->txr2.alpha,(int)cxt->txr2.env,
    //                                     (int)cxt->txr2.width,(int)cxt->txr2.height,(int)cxt->txr2.format,(void*)cxt->txr2.base
    //                         */
    //                         case 0:
    //                             if(cxt->list_type>0)
    //                             {
    //                                 cxt->list_type -= 1; // 0 1 2 3  PVR_LIST_OP_POLY   ,PVR_LIST_OP_MOD  ,PVR_LIST_TR_POLY ,PVR_LIST_TR_MOD ,PVR_LIST_PT_POLY 
    //                             }
    //                             break;
    //                         case 1:
    //                             if(cxt->gen.alpha >0)
    //                             {
    //                                 cxt->gen.alpha -= 1; // 0 1 PVR_ALPHA_DISABLE ,PVR_ALPHA_ENABLE  
    //                             }
    //                             break;
    //                         case 2:
    //                             if(cxt->gen.shading >0)
    //                             {
    //                                 cxt->gen.shading -= 1; // 0 1 FLAT GOURAUD
    //                             }
    //                             break;
    //                         case 3:
    //                             if(cxt->gen.fog_type >0)
    //                             {
    //                                 cxt->gen.fog_type -= 1; // 0 1 2 3 PVR_FOG_TABLE ,PVR_FOG_VERTEX  ,PVR_FOG_DISABLE ,PVR_FOG_TABLE2  
    //                             }
    //                             break;
    //                         case 4:
    //                             if(cxt->gen.culling >0)
    //                             {
    //                                 cxt->gen.culling -= 1; // 0 1 2 3 PVR_CULLING_NONE, PVR_CULLING_SMALL ,PVR_CULLING_CCW  ,PVR_CULLING_CW   
    //                             }
    //                             break;
    //                         case 5:
    //                             if(cxt->gen.color_clamp >0)
    //                             {
    //                                 cxt->gen.color_clamp -= 1; // 0 1 
    //                             }
    //                             break;
    //                         case 6:
    //                             if(cxt->gen.clip_mode >0)
    //                             {
    //                                 cxt->gen.clip_mode -= 1; // 0 2 3   PVR_USERCLIP_DISABLE  , PVR_USERCLIP_INSIDE     ,PVR_USERCLIP_OUTSIDE    
    //                             }
    //                             break;
    //                         case 7:
    //                             if(cxt->gen.modifier_mode >0)
    //                             {
    //                                 cxt->gen.modifier_mode -= 1; // 0 1
    //                             }
    //                             break;
    //                         case 8:
    //                             if(cxt->gen.specular >0)
    //                             {
    //                                 cxt->gen.specular -= 1; // 0 1
    //                             }
    //                             break;
    //                         case 9:
    //                             if(cxt->gen.alpha2 >0)
    //                             {
    //                                 cxt->gen.alpha2 -= 1; // 0 1 PVR_ALPHA_DISABLE ,PVR_ALPHA_ENABLE  
    //                             }
    //                             break;
    //                         case 10:
    //                             if(cxt->gen.fog_type2 >0)
    //                             {
    //                                 cxt->gen.fog_type2 -= 1; // 0 1 2 3
    //                             }
    //                             break;
    //                         case 11:
    //                             if(cxt->gen.color_clamp2 >0)
    //                             {
    //                                 cxt->gen.color_clamp2 -= 1; // 0 1
    //                             }
    //                             break;
    //                         case 12:
    //                             if(cxt->blend.src >0)
    //                             {
    //                                 cxt->blend.src -= 1; // 0 1 2 3 4 5 6 7 PVR_BLEND_ZERO ,PVR_BLEND_ONE ,PVR_BLEND_DESTCOLOR
    //                             }
    //                             break;
    //                         case 13:
    //                             if(cxt->blend.dst >0)
    //                             {
    //                                 cxt->blend.dst -= 1; // 0 1 2 3 4 5 6 7 PVR_BLEND_ZERO ,PVR_BLEND_ONE ,PVR_BLEND_DESTCOLOR
    //                             }
    //                             break;
    //                         case 23:
    //                             if(cxt->depth.comparison >0)
    //                             {
    //                                 cxt->depth.comparison -= 1; // 0 1 2 3 4 5 6 7 PVR_DEPTHCMP_NEVER,PVR_DEPTHCMP_LESS,PVR_DEPTHCMP_EQUAL,PVR_DEPTHCMP_LEQUAL,PVR_DEPTHCMP_GREATER,PVR_DEPTHCMP_NOTEQUAL,PVR_DEPTHCMP_GEQUAL,PVR_DEPTHCMP_ALWAYS     
    //                             }
    //                             break;
    //                         case 24:
    //                             if(cxt->depth.write >0)
    //                             {
    //                                 cxt->depth.write -= 1; // 0 1 
    //                             }
    //                             break;
    //                     }
    //                     compileCxtHdrOfCurrentModelSelected(debugger.currentEntitySelected);
    //                     break;
    //             case DCT_DBG_MODEL_MESH_ANIMATEDVTX_VTX:
    //                     pvr_vertex_t *vtx = debugger.currentEntitySelected->data;
    //                     switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
    //                     {
    //                         case 1:
    //                             vtx->x -= 0.1f;
    //                             break;
    //                         case 2:
    //                             vtx->y -= 0.1f;
    //                             break;
    //                         case 3:
    //                             vtx->z -= 0.1f;
    //                             break;
    //                     }
    //                     break;
    //             }
    //             break;

    //         break;
    //     }
        
        
    //     if(DEBUG_PRINT_ENABLE){printf("\ncurrent entity selected After Addr:%x \n",debugger.currentEntitySelected);
    //     printf("\n--- MODE %d ----indiceCurrentDataSelected:%d  \n",debugger.state,debugger.currentEntitySelected->indiceCurrentDataSelected);}

        
    //     updateTexteTextureMain();
    // }
    
    // if(controller->just_pressed.up)
    // {
    //     if(DEBUG_PRINT_ENABLE){printf("\nbtn just pressed up \n");
    //     printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}
    //      switch(debugger.state)
    //     {
    //         case 0:
    //             // move up --> prevSibling
    //             if(debugger.currentEntitySelected->prevEntSibling != NULL)
    //             {
    //                 debugger.currentEntitySelected = debugger.currentEntitySelected->prevEntSibling;

    //             }
    //             break;
    //         case 1:
    //             //EDIT currentvalue ptr ++
    //             switch(debugger.currentEntitySelected->type)
    //             {
    //                 case DCT_DBG_MODEL_POSITION:
    //                     vec3f_t *pos = debugger.currentEntitySelected->data;
    //                     switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
    //                     {
    //                         case 0:
    //                             pos->x += 0.1f;
    //                             break;
    //                         case 1:
    //                             pos->y += 0.1f;
    //                             break;
    //                         case 2:
    //                             pos->z += 0.1f;
    //                             break;
    //                     }
    //                     break;
    //                 case DCT_DBG_MODEL_ROTATION:
    //                     vec3f_t *rot = debugger.currentEntitySelected->data;
    //                     switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
    //                     {
    //                         case 0:
    //                             rot->x += 0.1f;
    //                             break;
    //                         case 1:
    //                             rot->y += 0.1f;
    //                             break;
    //                         case 2:
    //                             rot->z += 0.1f;
    //                             break;
    //                     }
    //                     break;
                    
    //                 case DCT_DBG_MODEL_MESH_CXT:
    //                     pvr_poly_cxt_t *cxt = debugger.currentEntitySelected->data;
    //                     switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
    //                     {
    //                         /*(int)cxt->list_type,(int)cxt->gen.alpha,(int)cxt->gen.shading,(int)cxt->gen.fog_type,(int)cxt->gen.culling,(int)cxt->gen.color_clamp,
    //                                     (int)cxt->gen.clip_mode,(int)cxt->gen.modifier_mode,(int)cxt->gen.specular,(int)cxt->gen.alpha2,
    //                                     (int)cxt->gen.fog_type2,(int)cxt->gen.color_clamp2, 
    //                                     cxt->blend.src, cxt->blend.dst,cxt->blend.src_enable,cxt->blend.dst_enable,cxt->blend.src2, cxt->blend.dst2,cxt->blend.src_enable2,cxt->blend.dst_enable2,
    //                                     (int)cxt->fmt.color,(int)cxt->fmt.uv,(int)cxt->fmt.modifier,
    //                                     (int)cxt->depth.comparison,(int)cxt->depth.write,
    //                                     (int)cxt->txr.enable,(int)cxt->txr.filter,(int)cxt->txr.mipmap,
    //                                     (int)cxt->txr.mipmap_bias,(int)cxt->txr.uv_flip,(int)cxt->txr.uv_clamp,(int)cxt->txr.alpha,(int)cxt->txr.env,
    //                                     (int)cxt->txr.width,(int)cxt->txr.height,(int)cxt->txr.format,(void*)cxt->txr.base,(int)cxt->txr2.enable,(int)cxt->txr2.filter,(int)cxt->txr2.mipmap,
    //                                     (int)cxt->txr2.mipmap_bias,(int)cxt->txr2.uv_flip,(int)cxt->txr2.uv_clamp,(int)cxt->txr2.alpha,(int)cxt->txr2.env,
    //                                     (int)cxt->txr2.width,(int)cxt->txr2.height,(int)cxt->txr2.format,(void*)cxt->txr2.base
    //                         */
    //                         case 0:
    //                             if(cxt->list_type<3)
    //                             {
    //                                 cxt->list_type += 1; // 0 1 2 3  PVR_LIST_OP_POLY   ,PVR_LIST_OP_MOD  ,PVR_LIST_TR_POLY ,PVR_LIST_TR_MOD ,PVR_LIST_PT_POLY 
    //                             }
    //                             break;
    //                         case 1:
    //                             if(cxt->gen.alpha <1)
    //                             {
    //                                 cxt->gen.alpha += 1; // 0 1 PVR_ALPHA_DISABLE ,PVR_ALPHA_ENABLE  
    //                             }
    //                             break;
    //                         case 2:
    //                             if(cxt->gen.shading <1)
    //                             {
    //                                 cxt->gen.shading += 1; // 0 1 FLAT GOURAUD
    //                             }
    //                             break;
    //                         case 3:
    //                             if(cxt->gen.fog_type <3)
    //                             {
    //                                 cxt->gen.fog_type += 1; // 0 1 2 3 PVR_FOG_TABLE ,PVR_FOG_VERTEX  ,PVR_FOG_DISABLE ,PVR_FOG_TABLE2  
    //                             }
    //                             break;
    //                         case 4:
    //                             if(cxt->gen.culling <3)
    //                             {
    //                                 cxt->gen.culling += 1; // 0 1 2 3 PVR_CULLING_NONE, PVR_CULLING_SMALL ,PVR_CULLING_CCW  ,PVR_CULLING_CW   
    //                             }
    //                             break;
    //                         case 5:
    //                             if(cxt->gen.color_clamp <1)
    //                             {
    //                                 cxt->gen.color_clamp += 1; // 0 1 
    //                             }
    //                             break;
    //                         case 6:
    //                             if(cxt->gen.clip_mode <3)
    //                             {
    //                                 cxt->gen.clip_mode += 1; // 0 2 3   PVR_USERCLIP_DISABLE  , PVR_USERCLIP_INSIDE     ,PVR_USERCLIP_OUTSIDE    
    //                             }
    //                             break;
    //                         case 7:
    //                             if(cxt->gen.modifier_mode <1)
    //                             {
    //                                 cxt->gen.modifier_mode += 1; // 0 1
    //                             }
    //                             break;
    //                         case 8:
    //                             if(cxt->gen.specular <1)
    //                             {
    //                                 cxt->gen.specular += 1; // 0 1
    //                             }
    //                             break;
    //                         case 9:
    //                             if(cxt->gen.alpha2 <1)
    //                             {
    //                                 cxt->gen.alpha2 += 1; // 0 1 PVR_ALPHA_DISABLE ,PVR_ALPHA_ENABLE  
    //                             }
    //                             break;
    //                         case 10:
    //                             if(cxt->gen.fog_type2 <3)
    //                             {
    //                                 cxt->gen.fog_type2 += 1; // 0 1 2 3
    //                             }
    //                             break;
    //                         case 11:
    //                             if(cxt->gen.color_clamp2 <1)
    //                             {
    //                                 cxt->gen.color_clamp2 += 1; // 0 1
    //                             }
    //                             break;
    //                         case 12:
    //                             if(cxt->blend.src <7)
    //                             {
    //                                 cxt->blend.src += 1; // 0 1 2 3 4 5 6 7 PVR_BLEND_ZERO ,PVR_BLEND_ONE ,PVR_BLEND_DESTCOLOR
    //                             }
    //                             break;
    //                         case 13:
    //                             if(cxt->blend.dst <7)
    //                             {
    //                                 cxt->blend.dst += 1; // 0 1 2 3 4 5 6 7 PVR_BLEND_ZERO ,PVR_BLEND_ONE ,PVR_BLEND_DESTCOLOR
    //                             }
    //                             break;
    //                         case 23:
    //                             if(cxt->depth.comparison <7)
    //                             {
    //                                 cxt->depth.comparison += 1; // 0 1 2 3 4 5 6 7 PVR_DEPTHCMP_NEVER,PVR_DEPTHCMP_LESS,PVR_DEPTHCMP_EQUAL,PVR_DEPTHCMP_LEQUAL,PVR_DEPTHCMP_GREATER,PVR_DEPTHCMP_NOTEQUAL,PVR_DEPTHCMP_GEQUAL,PVR_DEPTHCMP_ALWAYS     
    //                             }
    //                             break;
    //                         case 24:
    //                             if(cxt->depth.write <1)
    //                             {
    //                                 cxt->depth.write += 1; // 0 1 
    //                             }
    //                             break;
    //                     }
    //                     compileCxtHdrOfCurrentModelSelected(debugger.currentEntitySelected);
    //                     break;
    //                 case DCT_DBG_MODEL_MESH_ANIMATEDVTX_VTX:
    //                     pvr_vertex_t *vtx = debugger.currentEntitySelected->data;
    //                     switch(debugger.currentEntitySelected->indiceCurrentDataSelected)
    //                     {
    //                         case 1:
    //                             vtx->x += 0.1f;
    //                             break;
    //                         case 2:
    //                             vtx->y += 0.1f;
    //                             break;
    //                         case 3:
    //                             vtx->z += 0.1f;
    //                             break;
    //                     }
    //                     break;
    //             }
    //             break;
    //     }
        
        
    //     if(DEBUG_PRINT_ENABLE){printf("\ncurrent entity selected After Addr:%x \n",debugger.currentEntitySelected);
    //     printf("\n--- MODE %d ----indiceCurrentDataSelected:%d  \n",debugger.state,debugger.currentEntitySelected->indiceCurrentDataSelected);}

    //     updateTexteTextureMain();
    // }
    
    // if(controller->just_pressed.left)
    // {
    //     if(DEBUG_PRINT_ENABLE){printf("\nbtn just pressed left \n");
    //     printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}

    //     switch(debugger.state)
    //     {
    //         case 0:
    //         // DISPLAY
    //             // left --> parent
    //             if(debugger.currentEntitySelected->parentEntity != NULL)
    //             {
    //                 debugger.currentEntitySelected = debugger.currentEntitySelected->parentEntity;

    //                 for (int i=0; i<NUM_ENTITIES_TO_DRAW; i++)
    //                 {
    //                     debugger.entitiesToDraw[i]=NULL;
    //                 }
                    
    //                 if(debugger.currentEntitySelected->parentEntity!=NULL)
    //                 {
    //                     dct_menu_entityGameObject *p2 = debugger.currentEntitySelected->parentEntity;
    //                     dct_menu_entityGameObject **childs = p2->childEntities;
    //                     //printf("\nreinit Entities to Draw Based on parentEntity Childs \n");
    //                     for(int i=0; i<p2->childEntitiesCount; i++)
    //                     {
    //                         //printf("\nchild to add \n");
    //                         if(i<NUM_ENTITIES_TO_DRAW-1)
    //                         {
    //                             debugger.entitiesToDraw[i] = childs[i];
    //                         }
                            
    //                     }
    //                 }
    //                 else
    //                 {
    //                     if(DEBUG_PRINT_ENABLE){printf("\nPas de Parent Au dessus \n");}
    //                 }

    //                 if( debugger.currentEntitySelected==&debugger.switchBtn)
    //                 {
    //                     if(DEBUG_PRINT_ENABLE){printf("\nfree texture display \n");}
    //                     free_alpha_texture(debugger.textureDisplay);
    //                     //free(debugger.textureDisplay);
    //                     debugger.textureDisplay = NULL;
    //                 }

                    

    //             }

    //             break;

    //         case 1:
    //         // EDIT
    //                 if(debugger.currentEntitySelected->indiceCurrentDataSelected >= 0)
    //                 {
    //                     debugger.currentEntitySelected->indiceCurrentDataSelected -= 1;
    //                 }
    //                 if(debugger.currentEntitySelected->indiceCurrentDataSelected < 0)
    //                 {
    //                     debugger.state = 0; 
    //                 }
    //             break;
    //     }
            
        

    //     if(DEBUG_PRINT_ENABLE){printf("\ncurrent entity selected After Addr:%x \n",debugger.currentEntitySelected);
    //     printf("\n--- MODE %d ----indiceCurrentDataSelected:%d  \n",debugger.state,debugger.currentEntitySelected->indiceCurrentDataSelected);}


    //     updateTexteTextureMain();
    //     updateTexteTextureBreadcrumb();

    // }
    
    // if(controller->just_pressed.right)
    // {
    //     if(DEBUG_PRINT_ENABLE){printf("\nbtn just pressed right \n");
    //     printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}
    //     switch(debugger.state)
    //     {
    //         case 0:

    //             if(debugger.currentEntitySelected->data != NULL && debugger.currentEntitySelected->childEntities == NULL && isCurrentEntitySelectedInMainMenu()==false )
    //             {
    //                 if(debugger.state ==0)
    //                 {
    //                     if(DEBUG_PRINT_ENABLE){printf("fn press right change menu state \n");}
    //                     debugger.state = 1; 
    //                 }
    //             }

    //             // DISPLAY
    //             // right --> childs
    //             if(debugger.currentEntitySelected->childEntities != NULL)
    //             {
                    
    //                 for (int i=0; i<NUM_ENTITIES_TO_DRAW; i++)
    //                 {
    //                     debugger.entitiesToDraw[i]=NULL;
    //                 }

    //                 dct_menu_entityGameObject **childs = debugger.currentEntitySelected->childEntities;
                    
    //                 for(int i=0; i<debugger.currentEntitySelected->childEntitiesCount; i++)
    //                 {
    //                     if(i<NUM_ENTITIES_TO_DRAW-1)
    //                     {
    //                         debugger.entitiesToDraw[i] = childs[i];
    //                     }
                        
    //                 }

    //                 debugger.currentEntitySelected = debugger.entitiesToDraw[0];

    //             }

    //             if( debugger.currentEntitySelected!=&debugger.switchBtn && debugger.textureDisplay==NULL)
    //             {
    //                 // Utiliser une allocation statique pour le buffer temporaire
    //                 char *fill = (char*)malloc(4096);
    //                 if (!fill) {
    //                     printf("Erreur allocation buffer\n");
    //                     return;
    //                 }
                    
    //                 memset(fill, 0, 4096); // Initialiser à 0
    //                 for (int i=0;i<52 && strlen(fill)+65<4096;i++)
    //                 {
    //                     strcat(fill,"                                                                \n");
    //                 }
    //                 debugger.textureDisplay = create_text_texture_with_colors(fill , debugger.font, 0x305FACFF,512,512 );
    //                 free(fill);
    //             }

                
    //             break;
    //         case 1:
    //         // EDIT
    //             if(debugger.currentEntitySelected->indiceCurrentDataSelected <  debugger.currentEntitySelected->currentDataSelectedCount)
    //             {
    //                 debugger.currentEntitySelected->indiceCurrentDataSelected += 1;
    //             }
    //             break;
    //     }

    //     if(DEBUG_PRINT_ENABLE){printf("\n--- MODE %d ----indiceCurrentDataSelected:%d  \n",debugger.state,debugger.currentEntitySelected->indiceCurrentDataSelected);
    //     printf("\ncurrent entity selected Before Addr:%x \n",debugger.currentEntitySelected);}

        
    //     updateTexteTextureMain();
    //     updateTexteTextureBreadcrumb();
    // }
    
    
    pvr_get_stats(&debugger.stats);

    if( (debugger.stats.frame_count%30)==1 )
    {
        if( debugger.currentEntitySelected==&debugger.switchBtn)
        {
            updateTexteTextureSwitch();
        }
        else
        {
            updateTexteTextureMain();
            updateTexteTextureSwitch();
            updateTexteTextureBreadcrumb();
            
        }
        
    }
    
}

void drawTextureDebugMode()
{
    if( debugger.currentEntitySelected!=&debugger.switchBtn)
    {
        pvr_list_begin(PVR_LIST_TR_POLY);
        draw_alpha_texture(debugger.textureDisplay,64,40);
    }
    

    pvr_list_begin(PVR_LIST_TR_POLY);
    draw_alpha_texture(debugger.textureMiniBtnSwitch,0,0);

    pvr_list_begin(PVR_LIST_TR_POLY);
    draw_alpha_texture(debugger.textureHelper,128,24);

    // RAM PVR RAM
    float ratioFps = (float)debugger.stats.frame_rate/(float)60.0f;
    
    
    draw_rect_tr(64+64, 0,512-64,8,0xC00000CC);
    draw_rect_tr(64+64, 0,(512-64)*ratioFps,8,0xC000FF00);

    draw_rect_tr(64+64, 8,512-64,8,0xC00000CC);
    draw_rect_tr(64+64, 8,(512-64)*debugger.stats.ratioRamAvailUsed,8,0xC0FF0000);

    draw_rect_tr(64+64, 16,512-64,8,0xC00000CC);
    draw_rect_tr(64+64, 16,(512-64)*debugger.stats.ratioPvrRamAvailUsed,8,0xC0FF0000);


}




#endif