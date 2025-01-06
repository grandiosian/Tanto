#ifndef DCT_GUIDEBUGMODE_H
#define DCT_GUIDEBUGMODE_H

#include <kos.h>
#include "dct_fontDebugMode.h"
#include "dct_scene.h"
#include "dct_types.h"
#include "dct_input.h"


/*
Mémoire totale disponible : 16777216 octets (16 MB)
Mémoire allouée max : 16223256 octets
Mémoire système (KOS) : 553960 octets (≈ 541 KB)
PVR RAM:
Libre: 5653 KB
AICA RAM totale: 2MB
*/

typedef struct 
{
    uint64_t frame_last_time;     /**< \brief Ready-to-Ready length for the last frame in nanoseconds */
    uint64_t reg_last_time;       /**< \brief Registration time for the last frame in nanoseconds */
    uint64_t rnd_last_time;       /**< \brief Rendering time for the last frame in nanoseconds */
    uint64_t buf_last_time;       /**< \brief DMA buffer file time for the last frame in nanoseconds */
    size_t   frame_count;         /**< \brief Total number of rendered/viewed frames */
    size_t   vbl_count;           /**< \brief VBlank count */
    size_t   vtx_buffer_used;     /**< \brief Number of bytes used in the vertex buffer for the last frame */
    size_t   vtx_buffer_used_max; /**< \brief Number of bytes used in the vertex buffer for the largest frame */
    float    frame_rate;          /**< \brief Current frame rate (per second) */
    
    size_t   total_ram;    // RAM totale disponible
    size_t   used_ram;     // RAM utilisée
    uint32_t pvr_free;   // PVR libre
} perf_stats_t;


typedef struct
{
    char *name;
    char *type;
    void *data;
} DataDescriptor;

typedef struct
{
    char           *name;
    DataDescriptor *fields;
    
    int            fieldsCount;
    union 
    {
        struct StructDescriptor *childListDesc;
        int                     childListDescCount;
    };


} StructDescriptor;


DataDescriptor dataDescriptorInt[]=
{
    { "value", "int", NULL}
};

DataDescriptor dataDescriptorFloat[]=
{
    { "value", "float", NULL}
};

DataDescriptor dataDescriptorChar[]=
{
    { "value", "char", NULL}
};

DataDescriptor dataDescriptorVector3[]=
{
    { "x", "float", NULL},
    { "y", "float", NULL},
    { "z", "float", NULL}
};

DataDescriptor dataDescriptorPvrVertex[]=
{
    { "flags", "uint32_t", NULL},
    { "x", "float", NULL},
    { "y", "float", NULL},
    { "z", "float", NULL},
    { "u", "float", NULL},
    { "v", "float", NULL},
    { "argb", "uint32_t", NULL},
    { "oargb", "uint32_t", NULL}
};

DataDescriptor dataDescriptorPvrPolyHdr[]=
{
    { "cmd", "uint32_t", NULL},
    { "mode1", "uint32_t", NULL},
    { "mode2", "uint32_t", NULL},
    { "mode3", "uint32_t", NULL},
    { "d1", "uint32_t", NULL},
    { "d2", "uint32_t", NULL},
    { "d3", "uint32_t", NULL},
    { "d4", "uint32_t", NULL}
};

DataDescriptor dataDescriptorPvrPolyCxtGen[]=
{
    { "alpha", "int", NULL},
    { "shading", "int", NULL},
    { "fog_type", "int", NULL},
    { "culling", "int", NULL},
    { "color_clamp", "int", NULL},
    { "clip_mode", "int", NULL},
    { "modifier_mode", "int", NULL},
    { "specular", "int", NULL},
    { "alpha2", "int", NULL},
    { "fog_type2", "int", NULL},
    { "color_clamp2", "int", NULL}
};

DataDescriptor dataDescriptorPvrPolyCxtBlend[]=
{
    { "src", "int", NULL},
    { "dst", "int", NULL},
    { "src_enable", "int", NULL},
    { "dst_enable", "int", NULL},
    { "src2", "int", NULL},
    { "dst2", "int", NULL},
    { "src_enable2", "int", NULL},
    { "dst_enable2", "int", NULL}
};

DataDescriptor dataDescriptorPvrPolyCxtFmt[]=
{
    { "color", "int", NULL},
    { "uv", "int", NULL},
    { "modifier", "int", NULL}
};

DataDescriptor dataDescriptorPvrPolyCxtDepth[]=
{
    { "comparison", "int", NULL},
    { "write", "int", NULL}
};

DataDescriptor dataDescriptorPvrPolyCxtTxr[]=
{
    { "enable", "int", NULL},
    { "filter", "int", NULL},
    { "mipmap", "int", NULL},
    { "mipmap_bias", "int", NULL},
    { "uv_flip", "int", NULL},
    { "uv_clamp ", "int", NULL},
    { "alpha", "int", NULL},
    { "env", "int", NULL},
    { "width", "int", NULL},
    { "height", "int", NULL},
    { "format ", "int", NULL},
    { "base", "ptr", NULL}
};


DataDescriptor dataDescriptorPvrPolyCxt[]=
{
    { "list_type", "int", NULL},
    { "gen", "gen", NULL},
    { "blend", "blend", NULL},
    { "fmt", "fmt", NULL},
    { "depth", "depth", NULL},
    { "txr", "txr", NULL},
    { "txr2", "txr", NULL}
};


StructDescriptor meshDescriptor[]=
{
    {"vtxCount", "int", 1,{NULL,0}},
    {"sizeOfVtx", "int", 1,{NULL,0}},
    {"originalVtx", dataDescriptorPvrVertex, 8,{NULL,0}},
    {"animatedVtx", dataDescriptorPvrVertex, 8,{NULL,0}},
    {"RenderVtx", dataDescriptorPvrVertex, 8,{NULL,0}},
    {"hdr", dataDescriptorPvrPolyHdr, 8,{NULL,0}},
    {"cxt", dataDescriptorPvrPolyCxt, 48,{NULL,0}}
};


StructDescriptor modelDescriptor[]=
{
    {"name", dataDescriptorChar, 1,{NULL,0}},
    {"meshesCount", dataDescriptorInt, 1,{NULL,0}},
    {"meshes", meshDescriptor, 1,{NULL,0}},
    {"position", dataDescriptorVector3, 3,{NULL,0}},
    {"rotation", dataDescriptorVector3, 3,{NULL,0}},
    {"scale", dataDescriptorVector3, 3,{NULL,0}}
};


typedef struct
{
    StructDescriptor currentData;
    char             *menuLabel;
    int              currentLocationHierachy[4]; // 0-0-0-0 
} dct_menu_descriptor;


static StructDescriptor currentMenu;


typedef struct str_menu
{
    struct str_menu ****menu;  // 4 niveaux de pointeurs
    int menuCount;
    char *label;
    int strCurrentLabelCount;
} str_menu;

void initDctMenuDescriptor()
{
    str_menu menus;
    menus.menuCount = 2;
    
    // Niveau 1
    menus.menu = (str_menu****)malloc(sizeof(str_menu***) * menus.menuCount);
    
    for (int i = 0; i < menus.menuCount; i++)
    {
        // Niveau 2
        menus.menu[i] = (str_menu***)malloc(sizeof(str_menu**) * menus.menuCount);
        
        for (int i1 = 0; i1 < menus.menuCount; i1++)
        {
            // Niveau 3
            menus.menu[i][i1] = (str_menu**)malloc(sizeof(str_menu*) * menus.menuCount);
            
            for (int i2 = 0; i2 < menus.menuCount; i2++)
            {
                // Niveau 4 (final)
                menus.menu[i][i1][i2] = (str_menu*)malloc(sizeof(str_menu));
                
                char x[100];
                sprintf(x,"menu %d-%d-%d\n", i, i1, i2);
                menus.menu[i][i1][i2]->label = strdup(x);
                menus.menu[i][i1][i2]->menuCount = 0;
                menus.menu[i][i1][i2]->menu = NULL;
            }
        }
    }
}


typedef struct 
{
    DCT_DEBUG_DATA_TYPE    type;
    void                   *data;
    pvr_ptr_t              *icone;
    dct_textureAlpha       *texture;

} dct_dbg_templateBtnLvl1;

typedef struct 
{
    DCT_DEBUG_DATA_TYPE    type;
    void                   *data;
    dct_textureAlpha       *texture;
    
} dct_dbg_templateBtnLvl2;

typedef struct 
{
    DCT_DEBUG_DATA_TYPE    type;
    void                   *data;
    dct_textureAlpha       *texture;
    int                    indicesCurrentDataSelected[3]; // exemple : 0,1,1 donc pour model : 0:name 1:1ere lettre 1:ne correspond a rien  exemple : 3,1,0 3:position 1:position.x 0:ne correspond a rien         sur 3 niveau max [0]premier element sel [1] deuxieme [2] troisieme
    int                    positionCurrentDataSelectedOnScreen[2];

    DCT_DEBUG_DATA_TYPE    typeDataSelected;
    void                   *dataSelected;

} dct_dbg_templateBtnLvl3;

typedef struct 
{
    DCT_DEBUG_DATA_TYPE     type;
    void                    *data;
    dct_textureAlpha        *texture;

} dct_dbg_templateBtnLvl4;


typedef struct
{


} dct_dbg_templateBtn_pvrVertexData;


typedef struct
{
    dct_dbg_templateBtnLvl1 *btnsLvl1;
    dct_dbg_templateBtnLvl2 *btnsLvl2;
    dct_dbg_templateBtnLvl3 *btnsLvl3;
    dct_dbg_templateBtnLvl4 *btnsLvl4;

    int                  btnsLvl1Count;
    int                  btnsLvl2Count;
    int                  btnsLvl3Count;
    int                  btnsLvl4Count;

    int                  indiceCurrentBtnSelectedLvl1;
    int                  indiceCurrentBtnSelectedLvl2;
    int                  indiceCurrentBtnSelectedLvl3;
    int                  indiceCurrentBtnSelectedLvl4;


    

} dct_dbg_menu;

dct_dbg_menu menu = {0};







static bool currentSceneDataLoaded = false;
static bool currentSceneMeshDataLoaded = false;
static bool currentSceneModelDataLoaded = false;

static int update_counter = 0;
static float ratioRamAvailUsed = 0.0f;
static float ratioPvrRamAvailUsed = 0.0f;
static perf_stats_t       stats;

static dct_textureAlpha   *text_tex = NULL;
static dct_fontInfo       *font=NULL;

static dct_scene *currentScene_refDbgMode = NULL;

static int currentIndiceLocationDebugMode = 1;
static int currentIndiceModelSelected = 0;
static int currentIndiceModelDataSelected[3];

typedef enum {
    DISPLAY,
    EDIT

}DEBUG_MODE_STATUS; 

static DEBUG_MODE_STATUS STATUS;

static DCT_ControllerState controller = {0};

void initDebugInterface()
{

    dct_input_init();

    font = init_basic_font();
    if (!font) {
        printf("Font init failed\n");
        return;
    }

    // Variables globales

    currentScene_refDbgMode = getCurrentScene();
    
    

    vec3f_t position = {10.0f,300.0f,1.0f};
    char *dataTypeFPS = "---------------------------------\n\\c[FF0000]FPS : %d\n\\c[00FF00]Frame Time : %.2f\n\\c[0000FF]RAM totale : %lu\n\nRAM utilisée : %lu\nRAM libre : %lu\nPVR libre : %lu";

    // Dans votre boucle principale
    if(text_tex) {
        free_alpha_texture(text_tex);
        text_tex = NULL;
    }

    
   pvr_get_stats(&stats);


    void* heap_end = sbrk(0);
    void* heap_start = (void*)0x8c010000;  // Début typique du heap
    
    float statsFPS = stats.frame_rate;
    uint64_t statsFrameTime = stats.rnd_last_time;
    int statsRAMTotale = 16 * 1024 * 1024;
    int statsRAMUsed = (char*)heap_end - (char*)heap_start;
    int statsPVRFree = pvr_mem_available() / 1024;

    int ramPVR = 1024 * 8;
    size_t curr = stats.vtx_buffer_used;
    size_t peak = stats.vtx_buffer_used_max;

    printf("FPS: %.2f\n", statsFPS );
    printf("Render Frame Time: %llu ms\n", statsFrameTime);
    printf("RAM Totale: %d \n", statsRAMTotale );
    printf("RAM Utilisée: %d \n", statsRAMUsed );
    printf("PVR Libre: %d KB\n", statsPVRFree);
    printf("Current: %lu bytes\n",curr);
    printf("Peak:    %lu bytes\n", peak);


    ratioPvrRamAvailUsed = 1 - ( (float)(  8192 - (8192-statsPVRFree) ) / (float) 8192 );
    ratioRamAvailUsed    = 1 - ( (float)(statsRAMTotale - statsRAMUsed) / (float)statsRAMTotale );
    
    printf("ratio ram %.3f \n",ratioRamAvailUsed);
    printf("ratio pvr %.3f \n",ratioPvrRamAvailUsed);

    char buffer[256];  // Plus grand buffer pour être sûr
    //sprintf(buffer, stats.fps , stats.frame_time , stats.total_ram/1024, stats.used_ram/1024 ,((stats.total_ram - stats.used_ram) / 1024), stats.pvr_free / 1024  );
    char *txt = "\\c[0080FF]Tanto Engine Dreamcast \n\\c[FFFFFF]-FPS:   %.2f \n-FrameTime :   %d\n-RAM total:   %lu\n-RAM utilisée :   %lu \n-PVR libre :  %lu \n"; 
    sprintf(  buffer,txt,statsFPS,statsFrameTime,statsRAMTotale,statsRAMUsed, statsPVRFree ,curr,peak);

    text_tex = create_text_texture_with_colors(buffer, font, 0x305FACFF,256,256);





    
    menu.btnsLvl1Count = 4;
    menu.btnsLvl1 = malloc(sizeof(dct_dbg_templateBtnLvl1)*menu.btnsLvl1Count);
    menu.btnsLvl1[0].data = "Load Disk \nScenes";
    menu.btnsLvl1[0].type = DCT_DBG_MENU_PRINCIPAL_NAME;
    menu.btnsLvl1[0].texture = create_text_texture_with_colors(menu.btnsLvl1[0].data , font, 0x005FACFF,64,64);
    //menu.btnsLvl1[0].indiceCurrentDataSelected[0] = 0;

    menu.btnsLvl1[1].data = "Scene \nModels";
    menu.btnsLvl1[1].type = DCT_DBG_MENU_PRINCIPAL_NAME;
    menu.btnsLvl1[1].texture = create_text_texture_with_colors(menu.btnsLvl1[1].data , font, 0x005FACFF,64,64);
    menu.btnsLvl1[2].data = "Scene \nMeshes";
    menu.btnsLvl1[2].type = DCT_DBG_MENU_PRINCIPAL_NAME;
    menu.btnsLvl1[2].texture = create_text_texture_with_colors(menu.btnsLvl1[2].data , font, 0x005FACFF,64,64);
    menu.btnsLvl1[3].data = "Scene \nGameObjects";
    menu.btnsLvl1[3].type = DCT_DBG_MENU_PRINCIPAL_NAME;
    menu.btnsLvl1[3].texture = create_text_texture_with_colors(menu.btnsLvl1[3].data , font, 0x005FACFF,64,64);


    menu.indiceCurrentBtnSelectedLvl1 = 0;
    menu.indiceCurrentBtnSelectedLvl2 = 0;
    menu.indiceCurrentBtnSelectedLvl3 = 0;
    menu.indiceCurrentBtnSelectedLvl4 = 0;
       


}

void updateLocationMenuBtnsLvl1()
{
    if (controller.just_pressed.left )
    {
        printf("[DBGMENU] UP location btnsLvl1 indice:%d \n",menu.indiceCurrentBtnSelectedLvl1);
        if( (menu.indiceCurrentBtnSelectedLvl1 - 1)>=0 ) 
        {
            menu.indiceCurrentBtnSelectedLvl1 -= 1;
        } 
        
    }

    if (controller.just_pressed.right )
    {
        printf("[DBGMENU] DOWN location btnsLvl1 indice:%d \n",menu.indiceCurrentBtnSelectedLvl1);
        if( (menu.indiceCurrentBtnSelectedLvl1 + 1)<menu.btnsLvl1Count ) 
        {
            menu.indiceCurrentBtnSelectedLvl1 += 1;
        } 
    }

    if (controller.just_pressed.down )
    {
        printf("[DBGMENU] DOWN \n");
        currentIndiceLocationDebugMode += 1;
    }

}

void findCurrentDataSelectedMenuLvl3()
{
    dct_model_t *m = menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].data;
    int nbChiffresEntiersX;
    int nbChiffresEntiersY;
    int partieEntiere;
    float partieDecimale;

    if(menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].type == DCT_DBG_MODEL)
    {
        
        switch(menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[0])
        {
            case 0:
                switch(menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[1])
                {
                    case 0:
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].typeDataSelected = DCT_DBG_MODEL_POSITION_X;
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].dataSelected = &m->position.x;
                        break;
                    case 1:
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].typeDataSelected = DCT_DBG_MODEL_POSITION_Y;
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].dataSelected = &m->position.y;
                        break;
                    case 2:
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].typeDataSelected = DCT_DBG_MODEL_POSITION_Z;
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].dataSelected = &m->position.z;
                        break;
                }
                
                break;
            case 1:
                switch(menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[1])
                {
                    case 0:
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].typeDataSelected = DCT_DBG_MODEL_ROTATION_X;
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].dataSelected = &m->rotation.x;
                        break;
                    case 1:
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].typeDataSelected = DCT_DBG_MODEL_ROTATION_Y;
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].dataSelected = &m->rotation.y;
                        break;
                    case 2:
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].typeDataSelected = DCT_DBG_MODEL_ROTATION_Z;
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].dataSelected = &m->rotation.z;
                        break;
                }
                break;
            case 2:
                switch(menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[1])
                {
                    case 0:
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].typeDataSelected = DCT_DBG_MODEL_SCALE_X;
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].dataSelected = &m->scale.x;
                        break;
                    case 1:
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].typeDataSelected = DCT_DBG_MODEL_SCALE_Y;
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].dataSelected = &m->scale.y;
                        break;
                    case 2:
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].typeDataSelected = DCT_DBG_MODEL_SCALE_Z;
                        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].dataSelected = &m->scale.z;
                        break;
                }
                break;


        }


        // POUR LES MODEL ON NE PEUT EDITER QUE POSITION ROTATION SCALE
        if(menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[0]>=0 && 
        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[0]<=2 )
        {
            switch (menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[0])
            {
                case 0 :
                    //position
                    
                    menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].positionCurrentDataSelectedOnScreen[1]= 240-150+30+16;

                    break;
                case 1 :
                    //rotation
                    
                    menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].positionCurrentDataSelectedOnScreen[1]= 240-150+30+24;
                    break;
                case 2 :
                    //scale
                    menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].positionCurrentDataSelectedOnScreen[1]= 240-150+30+32;
                    break;

            }

            switch (menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[1])
            {
                //int sizeOfXValue = m->position.x
                case 0 :
                    //x
                    
                    menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].positionCurrentDataSelectedOnScreen[0]= 130+8*7;
                    break;
                case 1 :
                    //y
                    float nombre = m->position.x;
                    partieEntiere = (int)m->position.x;  // 123
                    partieDecimale = m->position.x - partieEntiere;  // 0.456

                    // Compter les chiffres de la partie entière
                    nbChiffresEntiersX = (int)log10((partieEntiere+1))+3;

                    printf("length posX:%d  posX:%f\n",nbChiffresEntiersX,m->position.x);
                    menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].positionCurrentDataSelectedOnScreen[0]= 130+8*(7+nbChiffresEntiersX+2);
                    break;
                case 2 :
                    //z
                    partieEntiere = (int)m->position.x;
                    nbChiffresEntiersX = (int)log10((partieEntiere+1))+3;

                    partieEntiere = (int)m->position.y;  // 123
                    partieDecimale = m->position.y - partieEntiere;  // 0.456

                    // Compter les chiffres de la partie entière
                    nbChiffresEntiersY = (int)log10((partieEntiere+1))+3;

                    printf("length posY:%d  posY:%f\n",nbChiffresEntiersY,m->position.y);
                    menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].positionCurrentDataSelectedOnScreen[0]= 130+8*(7+nbChiffresEntiersX+2+nbChiffresEntiersY+2);
                    break;

            }

        }
    }
    
}

void updateLocationMenuBtnsLvl2()
{
    if (controller.just_pressed.up )
    {
        printf("[DBGMENU] UP location btnsLvl1 indice:%d \n",menu.indiceCurrentBtnSelectedLvl2);
        if( (menu.indiceCurrentBtnSelectedLvl2 - 1)< 0 )
        {
            currentIndiceLocationDebugMode -=1;
        }
        if( (menu.indiceCurrentBtnSelectedLvl2 - 1)>=0 ) 
        {
            menu.indiceCurrentBtnSelectedLvl2 -= 1;
            currentSceneModelDataLoaded = false;
            currentIndiceModelSelected = menu.indiceCurrentBtnSelectedLvl2;
        } 
        
        
    }

    if (controller.just_pressed.down )
    {
        printf("[DBGMENU] DOWN location btnsLvl1 indice:%d \n",menu.indiceCurrentBtnSelectedLvl2);
        if( (menu.indiceCurrentBtnSelectedLvl2 + 1)<menu.btnsLvl2Count ) 
        {
            menu.indiceCurrentBtnSelectedLvl2 += 1;
            currentSceneModelDataLoaded = false;
            currentIndiceModelSelected = menu.indiceCurrentBtnSelectedLvl2;
        } 
    }

    if (controller.just_pressed.right )
    {
        printf("\n LVL2 PRESS RIGHT \n");
       currentIndiceLocationDebugMode += 1;
       //menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[0] =0;
       //menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[1] =0;
    }

}

void printStateIndiceCurrentBtnSelectedLvl3()
{
    printf("menu indiceCurrentBtnSelectedLvl3: %d , menu btnlvl3 current indicecurrentDataSelected first:%d Sedond:%d Third:%d \n",menu.indiceCurrentBtnSelectedLvl3, 
        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[0],
        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[1],
        menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[2]
        
        );

}
void updateLocationMenuBtnsLvl3()
{
    
    if (controller.just_pressed.left )
    {
        if(STATUS==DISPLAY)
        {
            if(menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[1]<=0)
            {
                currentIndiceLocationDebugMode -= 1;
            }
            if(menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[1]>0)
            {
                menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[1]-=1;
            }

        }

        if(STATUS == EDIT)
        {
            float *v = (menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].dataSelected);
            *v -= 1.0f;
            
        }
        
        
        
        findCurrentDataSelectedMenuLvl3();
        printStateIndiceCurrentBtnSelectedLvl3();

    }
    if (controller.just_pressed.right )
    {
        if(STATUS==DISPLAY)
        {
            if(menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[1]<2)
            {

                menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[1]+=1;
            }
        }

         if(STATUS == EDIT)
        {
            float *v = (menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].dataSelected);
            *v += 1.0f;
            


        }
        
        findCurrentDataSelectedMenuLvl3();
        printStateIndiceCurrentBtnSelectedLvl3();
    }

    if (controller.just_pressed.up )
    {
        if(STATUS==DISPLAY)
        {
            if(menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[0]-1>=0 && 
            menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[0]<=2 )
            {
                menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[0]-=1;
            }
        }
        
        
        findCurrentDataSelectedMenuLvl3();
        printStateIndiceCurrentBtnSelectedLvl3();
    }

    if (controller.just_pressed.down )
    {
        if(STATUS==DISPLAY)
        {
            if(menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[0]>=0 && 
            menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[0]+1<=2 )
            {
                menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].indicesCurrentDataSelected[0]+=1;
            }
        }
        
        findCurrentDataSelectedMenuLvl3();
        printStateIndiceCurrentBtnSelectedLvl3();
    }


    if (controller.just_pressed.a )
    {
        

        switch (STATUS)
        {
            case DISPLAY:
                STATUS = EDIT;
                printf("\n STATUS EDIT \n");
                break;
            
            case EDIT:
                STATUS = DISPLAY;
                printf("\n STATUS DISPLAY \n");
                break;
        }
    }




}


void updateController()
{
    dct_input_update(0, &controller);

    switch (currentIndiceLocationDebugMode)
    {
        case -1:
            break;

        case 0:
        // btnsLvl1
        updateLocationMenuBtnsLvl1();
            break;

        case 1:
        updateLocationMenuBtnsLvl2();
        // btnsLvl2
            break;

        case 2:
        updateLocationMenuBtnsLvl3();
        // btnsLvl3
            break;

        case 3:
        // btnsLvl4
            break;

    }

    
}


void updateDebugInterface()
{
    

    if(currentScene_refDbgMode!=NULL && currentSceneDataLoaded==0)
    {
        menu.btnsLvl2Count = currentScene_refDbgMode->modelSceneCount;
        menu.btnsLvl2 = malloc(sizeof(dct_dbg_templateBtnLvl2)*menu.btnsLvl2Count);

        

        for(int i=0;  i<currentScene_refDbgMode->modelSceneCount; i++)
        {
            printf("init menu btns lvl2 \n");
            menu.btnsLvl2[i].data=currentScene_refDbgMode->ptrListModelScene[i];
            menu.btnsLvl2[i].texture=create_text_texture_with_colors(currentScene_refDbgMode->ptrListModelScene[i]->name , font, 0x005FACFF,64,64); 
            menu.btnsLvl2[i].type=DCT_DBG_MODEL;

        }
        currentSceneDataLoaded = true;
    }


    if(currentScene_refDbgMode!=NULL && currentSceneDataLoaded==true && currentSceneModelDataLoaded==false)
    {
        if(menu.btnsLvl3)
        {
            for(int i=0; i<menu.btnsLvl3Count;i++)
            {
                printf("free texture btnslvl3\n");
                pvr_mem_free(menu.btnsLvl3[i].texture->texture);
            }
        }
        free(menu.btnsLvl3);
        
        menu.btnsLvl3Count = currentScene_refDbgMode->ptrListModelScene[currentIndiceModelSelected]->meshesCount;
        menu.btnsLvl3 = malloc(sizeof(dct_dbg_templateBtnLvl3)*menu.btnsLvl3Count);
        
        printf("create btnlvl3 btns:%d\n",menu.btnsLvl3Count);
        for (int i_m=0; i_m<menu.btnsLvl3Count; i_m++)
        {
            menu.btnsLvl3[i_m].data = currentScene_refDbgMode->ptrListModelScene[currentIndiceModelSelected];
            menu.btnsLvl3[i_m].type = DCT_DBG_MODEL;
            menu.btnsLvl3[i_m].indicesCurrentDataSelected[0]=0;
            menu.btnsLvl3[i_m].indicesCurrentDataSelected[1]=0;
            menu.btnsLvl3[i_m].indicesCurrentDataSelected[2]=0;
            dct_model_t *m = menu.btnsLvl3[i_m].data;
            char finalModel[300]="";
            char *model = "name:%s \nmeshesCount:%d \npos   :x%.1f y%.1f z%.1f \nrot   :x%.1f y%.1f z%.1f \nscale :x%.1f y%.1f z%.1f ";
            
            
            
            sprintf(finalModel, model,m->name,m->meshesCount,m->position.x,m->position.y,m->position.z,m->rotation.x,m->rotation.y,m->rotation.z,m->scale.x,m->scale.y,m->scale.z);
            char meshTxt[100]="";
            for (int i=0; i<m->meshesCount; i++)
            {
                char txt[10];
                sprintf(txt,"\nmesh %d\n" ,i);
                strcat(meshTxt,txt);
            }
            
            strcat(finalModel,meshTxt);
            
            //strcat(finalModel, m->name);
            printf(finalModel);
            printf("\n");
            menu.btnsLvl3[i_m].texture = create_text_texture_with_colors(finalModel , font, 0x005FACFF,128,128);
            
            
        }

        currentSceneModelDataLoaded = true;
    }else
    {
        dct_model_t *m = menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].data;
        char finalModel[300]="";
        char *model = "name:%s \nmeshesCount:%d \npos   :x%.1f y%.1f z%.1f \nrot   :x%.1f y%.1f z%.1f \nscale :x%.1f y%.1f z%.1f ";
        sprintf(finalModel, model,m->name,m->meshesCount,m->position.x,m->position.y,m->position.z,m->rotation.x,m->rotation.y,m->rotation.z,m->scale.x,m->scale.y,m->scale.z);
        
        char meshTxt[100]="";
        for (int i=0; i<m->meshesCount; i++)
        {
            char txt[10];
            sprintf(txt,"\nmesh %d\n" ,i);
            strcat(meshTxt,txt);
        }
        
        strcat(finalModel,meshTxt);

        printf("finalModel %s \n",finalModel);
        
        update_text_texture(menu.btnsLvl3[menu.indiceCurrentBtnSelectedLvl3].texture, finalModel, font, 0x005FACFF);
    }



    if(currentScene_refDbgMode!=NULL && currentSceneDataLoaded==true && currentSceneModelDataLoaded==true && currentSceneMeshDataLoaded==false)
    {
        menu.btnsLvl4Count = currentScene_refDbgMode->ptrListModelScene[currentIndiceModelSelected]->meshes[0].vtxCount;
        menu.btnsLvl4 = malloc(sizeof(dct_dbg_templateBtnLvl4)*menu.btnsLvl4Count);
        printf("create btnlvl4 btns:%d\n",menu.btnsLvl4Count);
        for (int i_vtx=0; i_vtx<menu.btnsLvl4Count; i_vtx++)
        {
            menu.btnsLvl4[i_vtx].data = &currentScene_refDbgMode->ptrListModelScene[currentIndiceModelSelected]->meshes[0].renderVtx[i_vtx];
            menu.btnsLvl4[i_vtx].type = DCT_DBG_MODEL_MESH_RENDERVTX;
            
            pvr_vertex_t *vtx = menu.btnsLvl4[i_vtx].data;
            char finalVtx[200]="";
            char *vtxdatas="";
            char *vtxFlags="";
            switch (vtx->flags)
            {
                case 0xe0000000:
                    vtxFlags = "PVR_CMD_VTX";
                    break;
                case 0xf0000000:
                    vtxFlags = "PVR_CMD_VTX_EOL";
                    break;
                case 0x20000000:
                    vtxFlags = "PVR_CMD_USERCLIP";
                    break;
                case 0x80000000:
                    vtxFlags = "PVR_CMD_MODIFIER";
                    break;
                case 0xA0000000:
                    vtxFlags = "PVR_CMD_SPRITE";
                    break;
            }

            sprintf(vtxdatas," x%.1f y%.1f z%.1f u%.1f v%.1f argb%x oargb%x",vtx->x,vtx->y,vtx->z,vtx->u,vtx->v,vtx->argb,vtx->oargb);

            strcat(finalVtx,vtxFlags);
            strcat(finalVtx,vtxdatas);
            printf("finalVtx:%s,",finalVtx);

            printf("\n");
            menu.btnsLvl4[i_vtx].texture= create_text_texture_with_colors(finalVtx , font, 0x005FACFF,64,64); 

        }

        currentSceneMeshDataLoaded = true;
    }

    

    pvr_get_stats(&stats);


    void* heap_end = sbrk(0);
    void* heap_start = (void*)0x8c010000;  // Début typique du heap
    
    float statsFPS = stats.frame_rate;
    uint64_t statsFrameTime = stats.rnd_last_time;
    int statsRAMTotale = 16 * 1024 ;
    int statsRAMUsed = ((char*)heap_end - (char*)heap_start)/1024;
    int statsRAMAvailable = statsRAMTotale - statsRAMUsed;
    int statsPVRFree = pvr_mem_available() / 1024;
    size_t curr = stats.vtx_buffer_used;
    size_t peak = stats.vtx_buffer_used_max;

    ratioPvrRamAvailUsed = 1 - ( (float)(  8192 - (8192-statsPVRFree) ) / (float) 8192 );
    ratioRamAvailUsed    = 1 - ( (float)(statsRAMTotale - statsRAMUsed) / (float)statsRAMTotale );
    
    char buffer_stats[200];  // Plus grand buffer pour être sûr
    char *txt = "\\c[0080FF]-- Tanto Engine Dreamcast -- \n\n\\c[FFFFFF]-FPS:             %.2f \n-FrameTime :      %llu\n-RAM total(KB):   %lu\n-RAM libre  (KB): %lu \n-PVR libre(KB):   %lu \n "; 
    char addtxt[100] = " line1 zifsfohjdskfhdsjkfhdskfhdskfhdsflkjsfldsh";
    
    char buffer_all[32][32];

    //strcpy( buffer_all[0] ,"\\c[0080FF]- Tanto Engine Dreamcast -\n");
    
    
    sprintf(  buffer_all,txt, statsFPS,statsFrameTime,statsRAMTotale,statsRAMAvailable, statsPVRFree ,curr,peak);
    //strcat(buffer_all , buffer_stats);
    //strcat(buffer_all, addtxt);
    //char *txt ="\\c[0080FF]FPS:%.2f \n";
    
    //sprintf(  buffer,txt,statsFPS);
    
    update_text_texture(text_tex, buffer_all, font, 0x305FACFF);
    

    
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
    vert.z = 1.0f;
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


void displayDbgMenuLvl1(dct_dbg_menu *menu)
{
    for(int i_btn=0; i_btn<menu->btnsLvl1Count; i_btn++)
    {
        if(i_btn == menu->indiceCurrentBtnSelectedLvl1)
        {
            draw_rect_tr(10+i_btn*100, 240-150,100,30,0xDDFF0000);
        }
        draw_rect_tr(10+i_btn*100, 240-150,100,30,0x700000FF);
        draw_alpha_texture(menu->btnsLvl1[i_btn].texture, 10+i_btn*100,240-150  );
    }

}

void displayDbgMenuLvl2(dct_dbg_menu *menu)
{
    
    for(int i_btn=0; i_btn<menu->btnsLvl2Count; i_btn++)
    {
        if(i_btn == menu->indiceCurrentBtnSelectedLvl2)
        {
             draw_rect_tr(10, 240-150+30+i_btn*20  ,120,20,0xDDFF0000);
        }
        draw_rect_tr(10, 240-150+30+i_btn*20  ,120,20,0x505C81BD);
        draw_alpha_texture(menu->btnsLvl2[i_btn].texture, 10, 240-150+30+i_btn*20  );

        //display meshes model

        

    }


}

void displayDbgMenuLvl3(dct_dbg_menu *menu)
{
     for(int i_btn=0; i_btn<menu->btnsLvl3Count; i_btn++)
    {
        draw_rect_tr(130, 240-150+30+i_btn*20  ,180,180,0x606E9BE3);
        draw_alpha_texture(menu->btnsLvl3[i_btn].texture, 130, 240-150+30+i_btn*20  );

        //draw_rect_tr(100, 240-150+30+menu->btnsLvl3[menu->indiceCurrentBtnSelectedLvl3].indiceCurrentDataSelected[0]*10  ,50,20,0xEE5FACFF);
        uint32_t col= 0xFF000000;
        if(STATUS==DISPLAY)
        {
            col = 0x6000ACFF;
        }
        if(STATUS==EDIT)
        {
            col = 0x60FFACFF;
        }
        draw_rect_tr(menu->btnsLvl3[menu->indiceCurrentBtnSelectedLvl3].positionCurrentDataSelectedOnScreen[0] , menu->btnsLvl3[menu->indiceCurrentBtnSelectedLvl3].positionCurrentDataSelectedOnScreen[1] ,8*4 ,8,col);
    }

    

}

void displayDbgMenuLvl4(dct_dbg_menu *menu)
{
     for(int i_btn=0; i_btn<menu->btnsLvl4Count; i_btn++)
    {
        draw_rect_tr(100, 240-150+30+i_btn*20  ,130,20,0x505FACFF);
        draw_alpha_texture(menu->btnsLvl4[i_btn].texture, 100, 240-150+30+i_btn*20  );
    }

}








void drawDebugInterface()
{
    // Propriétés du texte
    // dct_textProperties props = {
    //     .x = 50.0f,
    //     .y = 50.0f,
    //     .scale = 1.0f,
    //     .color = 0xFFFFFFFF  // Blanc
    // };

    updateController();

    if(update_counter++ % 30 == 0) 
    {
        updateDebugInterface(); // on update toutes les 30 images environ 2 fois par seconde
    }


    pvr_list_begin(PVR_LIST_TR_POLY);
    // STATS
    draw_alpha_texture(text_tex,10,10);

    switch (currentIndiceLocationDebugMode)
    {
        case -1:
            break;
        
        case 0:
            displayDbgMenuLvl1(&menu);
            break;
        
        case 1:
            displayDbgMenuLvl1(&menu);
            displayDbgMenuLvl2(&menu);
            break;
        
        case 2:
            displayDbgMenuLvl1(&menu);
            displayDbgMenuLvl2(&menu);
            displayDbgMenuLvl3(&menu);
            break;
        
        case 3:
            displayDbgMenuLvl1(&menu);
            displayDbgMenuLvl2(&menu);
            displayDbgMenuLvl4(&menu);
            break;

    }
    
   
    
    

    // RAM PVR RAM
    draw_rect_tr(230, 25,400,40,0x30E3D700);

    draw_rect_tr(250, 30,350,10,0xC00000CC);
    draw_rect_tr(250, 30,350*ratioRamAvailUsed,10,0xC0FF0000);

    draw_rect_tr(250, 50,350,10,0xC00000CC);
    draw_rect_tr(250, 50,350*ratioPvrRamAvailUsed,10,0xC0FF0000);


    // MODELS
    if(currentScene_refDbgMode != NULL)
    {
        for (int i=0; i<currentScene_refDbgMode->modelSceneCount; i++)
        {
            // if(i = currentIndiceModelSelected )
            // {
            //     draw_rect_tr(100, 200+40*i,100,30,0x60FFACFF);
            // }
            //draw_rect_tr(100, 200+40*i,100,30,0x60FFACFF);

        }
    }
    


}





#endif