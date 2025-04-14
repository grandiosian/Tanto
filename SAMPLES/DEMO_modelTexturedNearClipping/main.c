#include <kos.h>
#include "dct_utils.h"
#include "dct_types.h"
#include "dct_model.h"
#include "dct_camera.h"
#include "dct_scene.h"
#include "dct_fontDebugger.h"

//#include "dct_debugger.h"

vec3f_t CAM_POSITION;

extern uint8 romdisk[];
extern uint8 romdisk_end[];



typedef struct dct_offsetDebugger
{
    DCT_IDENT  ident;
    int        offset_REFNEXTmemberModel[50];
    int        offset_REFPREVmemberModel[50];

} dct_offsetDebugger_t;

typedef struct dct_listMeshesDebugger
{
    DCT_IDENT   ident;
    uint32_t    *meshesBuffer[20];
} dct_listMeshesDebugger_t;


typedef struct dct_fnDebug
{
    DCT_IDENT       ident;
    void            (*fnDebugPtr)(uint32_t *ptr);
    uint32_t        *fnDebugPtrArg;
}   dct_fnDebug_t;

typedef struct dct_tinyDebugger
{

    dct_fontInfo     *font;
    dct_textureAlpha *textureDisplay;

    int              indiceCurrentModel;
    int              indiceCurrentMemberModel;

    dct_fnDebug_t    fnDebug;

    int              currentOffset;
    dct_listMeshesDebugger_t listMeshes;
    dct_offsetDebugger_t offsets[10];
    
    uint32_t         *lastNavigationData[10]; // pour revenir en arriere dans la hierarchie
    uint32_t         *breadCrumb[10]; 
    void             *currentStruct;
    void             *currentDataToNavigate;
    void             *currentDataToEdit;
    bool             currentDataToEdit_IntOrFloat; 
    bool             isInsideDataStructure; // position or  position.x ? 


} dct_tinyDebugger_t;


void debugTinyDebugger(dct_tinyDebugger_t *debugger)
{
    printf("========  PRINT DEBUGGER VALUES =========\n");
    printf("==> indiceCurrentModel        : %d \n",debugger->indiceCurrentModel); 
    printf("==> indiceCurrentMemberModel  : %d \n",debugger->indiceCurrentMemberModel); 
    printf("-------------------------------------\n");
    for (int i_off=0; i_off<10; i_off++)
    {
        for(int i=0; i<50; i++)
        {
            printf("==> offset_REFNEXTmemberModel[%d]:%d\n",i,debugger->offsets[i_off].offset_REFNEXTmemberModel[i]); 
            printf("==> offset_REFPREVmemberModel[%d]:%d\n",i,debugger->offsets[i_off].offset_REFPREVmemberModel[i]); 
        }
    }
    
    printf("-------------------------------------\n");
    for (int i=0; i<10; i++)
    {
        printf("==> lastNavigationData[%d]       :%x\n",i,debugger->lastNavigationData[i]); 
    }
    printf("-------------------------------------\n");

    printf("==> currentDataToNavigate       : %x  \n", debugger->currentDataToNavigate);
    printf("==> currentDataToEdit           : %x  \n", debugger->currentDataToEdit);
    printf("==> currentDataToEdit_IntOrFloat: %x  \n", debugger->currentDataToEdit_IntOrFloat);

}



void displayCurrentDataSelected(dct_tinyDebugger_t *debugger)
{
    // identifier a quoi correspond l'adresse de donnée courante  
}

bool isTwoCharIdentical(char *txtA, char *txtB)
{
    //printf("txtA :%s txtB :%s \n",txtA,txtB);
    if(strlen(txtA)!=strlen(txtB)){return false;}
    for (int i=0; i<sizeof(txtA)/sizeof(char); i++)
    {
        if(txtA[i]!=txtB[i])
        {
            //printf("return false \n");
            return false;
        }

    }
    //printf("return true\n");
    return true;
}


void formatNumber(char *dest, float value, int addBrackets, const char *type) 
{
   
    if (!dest || !type) {
        printf("Error: NULL pointer passed to formatNumber\n");
        return;
    }

    char temp_value[128] = {0};  // Buffer temporaire initialisé à zéro

    if (isTwoCharIdentical(type, "INT")) {
        if (addBrackets) {
            snprintf(temp_value, sizeof(temp_value), "[%d]\\c[A0A0A0]", (int)value);
        } else {
            snprintf(temp_value, sizeof(temp_value), "%d", (int)value);
        }
    }
    else if (isTwoCharIdentical(type, "FLOAT")) {
        if (addBrackets) {
            snprintf(temp_value, sizeof(temp_value), "\\c[A00000][%.3f]\\c[A0A0A0]", value);
        } else {
            snprintf(temp_value, sizeof(temp_value), "%.3f", value);
        }
    }
    else if (isTwoCharIdentical(type, "CXT_ALPHA")) {
        const char *status = ((int)value == 0) ? "DISABLE" : "ENABLE";
        if (addBrackets) {
            snprintf(temp_value, sizeof(temp_value), "\\c[A00000][%s]\\c[A0A0A0]", status);
        } else {
            snprintf(temp_value, sizeof(temp_value), "%s", status);
        }
    }
    else if (isTwoCharIdentical(type, "BOOL")) {
        const char *status = ((int)value == 0) ? "OFF" : "ON";
        if (addBrackets) {
            snprintf(temp_value, sizeof(temp_value), "\\c[A00000][%s]\\c[A0A0A0]", status);
        } else {
            snprintf(temp_value, sizeof(temp_value), "%s", status);
        }
    }

    else if (isTwoCharIdentical(type, "ENABLE")) {
        const char *status = ((int)value == 0) ? "X" : "O";
        if (addBrackets) {
            snprintf(temp_value, sizeof(temp_value), "\\c[A00000][%s]\\c[A0A0A0]", status);
        } else {
            snprintf(temp_value, sizeof(temp_value), "%s", status);
        }
    }

    else if (isTwoCharIdentical(type, "HEX32BITS")) {
        if (addBrackets) 
        {
            snprintf(temp_value, sizeof(temp_value), "\\c[A00000][%x]\\c[A0A0A0]",(uint32)value);
        }else
        {
            snprintf(temp_value, sizeof(temp_value), "%x",value);
        }
    }

    else if (isTwoCharIdentical(type, "COLOR_FMT")) {
        switch((int)value)
        {
            case 0:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][ARGBPACKED]\\c[A0A0A0]",value);
                }else
                {
                    snprintf(temp_value, sizeof(temp_value), "ARGBPACKED",value);
                }
                break;
            case 1:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][4FLOATS]\\c[A0A0A0]",value);
                }else
                {
                    snprintf(temp_value, sizeof(temp_value), "4FLOATS",value);
                }
                break;
            case 2:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][INTENSITY]\\c[A0A0A0]",value);
                }else
                {
                    snprintf(temp_value, sizeof(temp_value), "INTENSITY",value);
                }
                break;
            case 3:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][INTENSITY_PREV]\\c[A0A0A0]",value);
                }else
                {
                    snprintf(temp_value, sizeof(temp_value), "INTENSITY_PREV",value);
                }
                break;
        }
    }

    else if (isTwoCharIdentical(type, "UV_FMT")) {
        switch((int)value)
        {
            case 0:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][32BIT]\\c[A0A0A0]",value);
                }else
                {
                    snprintf(temp_value, sizeof(temp_value), "32BIT",value);
                }
                break;
            case 1:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][16BIT]\\c[A0A0A0]",value);
                }else
                {
                    snprintf(temp_value, sizeof(temp_value), "16BIT",value);
                }
                break;
        }
    }


    else if (isTwoCharIdentical(type, "SHADING")) {
        switch((int)value)
        {
            case 0:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][PVR_SHADE_FLAT]\\c[A0A0A0]");
                }else
                {
                    snprintf(temp_value, sizeof(temp_value), "PVR_SHADE_FLAT");
                }
                
                break;
            case 1:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][PVR_SHADE_GOURAUD]\\c[A0A0A0]");
                }else
                {
                    snprintf(temp_value, sizeof(temp_value), "PVR_SHADE_GOURAUD");

                }
                break;
        }
    }

     else if (isTwoCharIdentical(type, "LIST_TYPE")) {
        switch((int)value)
        {
            case 0:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][PVR_LIST_OP_POLY]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "PVR_LIST_OP_POLY");
                }
                break;
            case 1:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][PVR_LIST_OP_MOD]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "PVR_LIST_OP_MOD");
                }
                break;
            case 2:
            if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][PVR_LIST_TR_POLY]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "PVR_LIST_TR_POLY");
                }
                break;
            case 3:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][PVR_LIST_TR_MOD]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "PVR_LIST_TR_MOD");
                }
                break;
            case 4:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][PVR_LIST_PT_POLY]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "PVR_LIST_PT_POLY");
                }
                break;
        }
    }

    else if (isTwoCharIdentical(type, "FOG_TYPE")) {
        switch((int)value)
        {
            case 0:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][PVR_FOG_TABLE]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "PVR_FOG_TABLE");
                }
                break;
            case 1:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][PVR_FOG_VERTEX]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "PVR_FOG_VERTEX");
                }
                break;
            case 2:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][PVR_FOG_DISABLE]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "PVR_FOG_DISABLE");
                }
                break;
            case 3:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][PVR_FOG_TABLE2]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "PVR_FOG_TABLE2");
                }
                break;
        }
    }

    else if (isTwoCharIdentical(type, "CULLING")) {
        switch((int)value)
        {
            case 0:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][PVR_CULLING_NONE]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "PVR_CULLING_NONE");
                }
                break;
            case 1:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][PVR_CULLING_SMALL]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "PVR_CULLING_SMALL");
                }
                break;
            case 2:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][PVR_CULLING_CCW]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "PVR_CULLING_CCW");
                }
                break;
            case 3:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][PVR_CULLING_CW]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "PVR_CULLING_CW");
                }
                break;
        }
    }

    else if (isTwoCharIdentical(type, "CLIP_MODE")) {
        switch((int)value)
        {
            case 0:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][DISABLE]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "DISABLE");
                }
                break;
            case 1:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][INSIDE]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "INSIDE");
                }
                break;
            case 2:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][OUTSIDE]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "OUTSIDE");
                }
                break;
        }
    }

    else if (isTwoCharIdentical(type, "BLEND_MODE")) {
        switch((int)value)
        {
            case 0:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][ZERO]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "ZERO");
                }
                break;
            case 1:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][ONE]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "ONE");
                }
                break;
            case 2:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][DESTCOLOR]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "DESTCOLOR");
                }
                break;
            case 3:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][INVDESTCOLOR]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "INVDESTCOLOR");
                }
                break;
            case 4:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][SRCALPHA]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "SRCALPHA");
                }
                break;
            case 5:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][INVSRCALPHA]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "INVSRCALPHA");
                }
                break;
            case 6:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][INVDESTALPHA]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "INVDESTALPHA");
                }
                break;
        }
    }

     else if (isTwoCharIdentical(type, "DEPTH")) {
        switch((int)value)
        {
            case 0:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][NEVER]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "NEVER");
                }
                break;
            case 1:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][LESS]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "LESS");
                }
                break;
            case 2:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][EQUAL]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "EQUAL");
                }
                break;
            case 3:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][LEQUAL]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "LEQUAL");
                }
                break;
            case 4:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][GREATER]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "GREATER");
                }
                break;
            case 5:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][NOTEQUAL]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "NOTEQUAL");
                }
                break;
            case 6:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][GEQUAL]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "GEQUAL");
                }
                break;
            case 7:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][ALWAYS]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "ALWAYS");
                }
                break;
        }
    }

    else if (isTwoCharIdentical(type, "FILTER")) {
        switch((int)value)
        {
            case 0:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][NONE]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "NONE");
                }
                break;
            case 1:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][NEAREST]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "NEAREST");
                }
                break;
            case 2:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][BILINEAR]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "BILINEAR");
                }
                break;
            case 3:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][3]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "3");
                }
                break;
            case 4:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][TRILINEAR1]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "TRILINEAR1");
                }
                break;
            case 5:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][5]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "5");
                }
                break;
            case 6:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][TRILINEAR2]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "TRILINEAR2");
                }
                break;
        }
    }

    else if (isTwoCharIdentical(type, "MIPMAP_BIAS")) {
        switch((int)value)
        {
            case 0:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][0 NULL]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "0 NULL");
                }
                break;
            case 1:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][0.25]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "0.25");
                }
                break;
            case 2:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][0.50]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "0.50");
                }
                break;
            case 3:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][0.75]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "0.75");
                }
                break;
            case 4:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][1.00]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "1.00");
                }
                break;
            case 5:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][1.25]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "1.25");
                }
                break;
            case 6:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][1.50]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "1.50");
                }
                break;
            case 7:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][1.75]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "1.75");
                }
                break;
            case 8:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][2.00]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "2.00");
                }
                break;
            case 9:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][2.25]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "2.25");
                }
                break;
            case 10:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][2.50]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "2.50");
                }
                break;
            case 11:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][2.75]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "2.75");
                }
                break;
            case 12:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][3.00]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "3.00");
                }
                break;
            case 13:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][3.25]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "3.25");
                }
                break;
            case 14:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][3.50]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "3.50");
                }
                break;
            case 15:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][3.75]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "3.75");
                }
                break;
        }
    }



    else if (isTwoCharIdentical(type, "UV_FLIP")) {
        switch((int)value)
        {
            case 0:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][NONE]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "NONE");
                }
                break;
            case 1:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][V]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "V");
                }
                break;
            case 2:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][U]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "U");
                }
                break;
            case 3:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][UV]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "UV");
                }
                break;
        }
    }

    else if (isTwoCharIdentical(type, "UV_CLAMP")) {
        switch((int)value)
        {
            case 0:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][NONE]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "NONE");
                }
                break;
            case 1:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][V]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "V");
                }
                break;
            case 2:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][U]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "U");
                }
                break;
            case 3:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][UV]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "UV");
                }
                break;
        }
    }


    else if (isTwoCharIdentical(type, "ENV")) {
        switch((int)value)
        {
            case 0:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][REPLACE]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "REPLACE");
                }
                break;
            case 1:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][MODULATE]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "MODULATE");
                }
                break;
            case 2:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][DECAL]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "DECAL");
                }
                break;
            case 3:
                if (addBrackets) 
                {
                    snprintf(temp_value, sizeof(temp_value), "\\c[A00000][MODULATEALPHA]\\c[A0A0A0]");
                }
                else
                {
                    snprintf(temp_value, sizeof(temp_value), "MODULATEALPHA");
                }
                break;
        }
    }
    




    size_t current_len = strlen(dest);
    size_t temp_len = strlen(temp_value);
    strncpy(dest + current_len, temp_value, sizeof(temp_value));
}


void navigateToCxtMesh(dct_cxt_t *dctcxt, dct_tinyDebugger_t *debugger,  char *buffer)
{

    pvr_poly_cxt_t     *cxt = &dctcxt->cxt; 

    int indice_memberModel[50]   ={0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
                                           0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0}; 
    void resetInt(int indice)
    {
        for(int i=0; i<sizeof(indice)/sizeof(int); i++)
        {
            //indice_model[i] = 0;
            indice_memberModel[i]=0;
            
        }
    }

    switch(debugger->indiceCurrentMemberModel)
    {
        case 0:
            resetInt(indice_memberModel);
            indice_memberModel[0]=1;
            debugger->isInsideDataStructure = false;
            debugger->currentDataToEdit=NULL;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
        //==========================
        //========== CXT GEN =======
        //========================== 
        case 1:
            resetInt(indice_memberModel);
            indice_memberModel[1]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->gen.alpha;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        case 2:
            resetInt(indice_memberModel);
            indice_memberModel[2]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->gen.shading;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        case 3:
            resetInt(indice_memberModel);
            indice_memberModel[3]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->gen.fog_type;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        case 4:
            resetInt(indice_memberModel);
            indice_memberModel[4]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->gen.culling;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        case 5:
            resetInt(indice_memberModel);
            indice_memberModel[5]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->gen.color_clamp;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        case 6:
            resetInt(indice_memberModel);
            indice_memberModel[6]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->gen.clip_mode;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        case 7:
            resetInt(indice_memberModel);
            indice_memberModel[7]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->gen.modifier_mode;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        case 8:
            resetInt(indice_memberModel);
            indice_memberModel[8]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->gen.specular;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        case 9:
            resetInt(indice_memberModel);
            indice_memberModel[9]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->gen.alpha2;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        case 10:
            resetInt(indice_memberModel);
            indice_memberModel[10]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->gen.fog_type2;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        case 11:
            resetInt(indice_memberModel);
            indice_memberModel[11]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->gen.color_clamp2;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        //==========================
        //========== CXT BLEND =======
        //========================== 

        case 12:
            resetInt(indice_memberModel);
            indice_memberModel[12]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->blend.src;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

        case 13:
            resetInt(indice_memberModel);
            indice_memberModel[13]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->blend.dst;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

        case 14:
            resetInt(indice_memberModel);
            indice_memberModel[14]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->blend.src_enable;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

        case 15:
            resetInt(indice_memberModel);
            indice_memberModel[15]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->blend.dst_enable;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

        case 16:
            resetInt(indice_memberModel);
            indice_memberModel[16]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->blend.src2;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
        case 17:
            resetInt(indice_memberModel);
            indice_memberModel[17]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->blend.dst2;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
        case 18:
            resetInt(indice_memberModel);
            indice_memberModel[18]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->blend.src_enable2;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
        case 19:
            resetInt(indice_memberModel);
            indice_memberModel[19]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->blend.dst_enable2;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

        
        //==========================
        //========== CXT FMT =======
        //========================== 

        case 20:
            resetInt(indice_memberModel);
            indice_memberModel[20]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->fmt.color;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
        case 21:
            resetInt(indice_memberModel);
            indice_memberModel[21]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->fmt.uv;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
        case 22:
            resetInt(indice_memberModel);
            indice_memberModel[22]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->fmt.modifier;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

        //============================
        //========== CXT DEPTH =======
        //============================ 
        
        case 23:
            resetInt(indice_memberModel);
            indice_memberModel[23]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->depth.comparison;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

        case 24:
            resetInt(indice_memberModel);
            indice_memberModel[24]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->depth.write;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
        //============================
        //========== CXT TXR 1 =======
        //============================ 

        case 25:
            resetInt(indice_memberModel);
            indice_memberModel[25]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr.enable;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
        case 26:
            resetInt(indice_memberModel);
            indice_memberModel[26]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr.filter;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
        case 27:
            resetInt(indice_memberModel);
            indice_memberModel[27]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr.mipmap;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
        case 28:
            resetInt(indice_memberModel);
            indice_memberModel[28]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr.mipmap_bias;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
        case 29:
            resetInt(indice_memberModel);
            indice_memberModel[29]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr.uv_flip;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

         case 30:
            resetInt(indice_memberModel);
            indice_memberModel[30]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr.uv_clamp;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
         case 31:
            resetInt(indice_memberModel);
            indice_memberModel[31]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr.alpha;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

         case 32:
            resetInt(indice_memberModel);
            indice_memberModel[32]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr.env;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

         case 33:
            resetInt(indice_memberModel);
            indice_memberModel[33]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=NULL;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

         case 34:
            resetInt(indice_memberModel);
            indice_memberModel[34]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=NULL;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

         case 35:
            resetInt(indice_memberModel);
            indice_memberModel[35]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=NULL;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

        //============================
        //========== CXT TXR 2 =======
        //============================ 

        case 36:
            resetInt(indice_memberModel);
            indice_memberModel[36]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr2.enable;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
        case 37:
            resetInt(indice_memberModel);
            indice_memberModel[37]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr2.filter;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
        case 38:
            resetInt(indice_memberModel);
            indice_memberModel[38]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr2.mipmap;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
        case 39:
            resetInt(indice_memberModel);
            indice_memberModel[39]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr2.mipmap_bias;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
        case 40:
            resetInt(indice_memberModel);
            indice_memberModel[40]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr2.uv_flip;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

         case 41:
            resetInt(indice_memberModel);
            indice_memberModel[41]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr2.uv_clamp;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;
        
         case 42:
            resetInt(indice_memberModel);
            indice_memberModel[42]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr2.alpha;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

         case 43:
            resetInt(indice_memberModel);
            indice_memberModel[43]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=&cxt->txr2.env;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

         case 44:
            resetInt(indice_memberModel);
            indice_memberModel[44]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=NULL;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

         case 45:
            resetInt(indice_memberModel);
            indice_memberModel[45]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=NULL;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

         case 46:
            resetInt(indice_memberModel);
            indice_memberModel[46]=1;
            debugger->isInsideDataStructure = true;
            debugger->currentDataToEdit=NULL;
            debugger->currentDataToEdit_IntOrFloat = 0;
            break;

        
    }
    // GEN ALPHA SHADING FOG_TYPE CULLING COLOR_CLAMP CLIP_MODE MODIFIER_MODE SPECULAR ALPHA2 FOG_TYPE2 COLOR_CLAMP2
    // BLEND SRC DST SRC_ENABLE DST_ENABLE SRC2 DST2 SRC_ENABLE2 DST_ENABLE2
    // FMT  COLOR UV MODIFIER
    // DEPTH COMPARISON WRITE
    // TXR ENABLE FILTER MIPMAP MIPMAP_BIAS UV_FLIP UV_CLAMP ALPHA ENV WIDTH HEIGHT FORMAT BASE
    // TXR2 ENABLE FILTER MIPMAP MIPMAP_BIAS UV_FLIP UV_CLAMP ALPHA ENV WIDTH HEIGHT FORMAT BASE

    // TEST_ENABLE_VALUE:[X] [O]

    // CXT 
    // 1  LIST_TYPE [PVR_LIST_OP_POLY]      // [PVR_LIST_TR_POLY] [PVR_LIST_TR_MOD] [PVR_LIST_PT_POLY]
    // 2  GEN :  
    //    -- ALPHA:[X] SHADING:[PVR_SHADE_FLAT]  
    // 3  -- FOG_TYPE:[PVR_FOG_TABLE] CULLING:[PVR_CULLING_SMALL]
    // 4  -- COLOR_CLAMP:[X] CLIP_MODE:[X] MODIFIER_MODE:[X]
    // 5  -- SPECULAR:[0XFFFFFFFF] ALPHA2:[X]
    // 1  -- FOG_TYPE2:[PVR_FOG_TABLE] COLOR_CLAMP2:[X]
    // 2  BLEND:  
    //    -- SRC:[INVDESTCOLOR] DST:[INVDESTCOLOR] 
    // 3  -- SRC_ENABLE:[X] DST_ENABLE:[X]
    // 4  -- SRC2:[INVDESTCOLOR] DST2:[INVDESTCOLOR] 
    // 5  -- SRC_ENABLE2:[X] DST_ENABLE2:[X]
    // 1  DEPTH:
    // 2  -- COMPARISON:[NOTEQUAL] WRITE:[X]
    // 3  TXR:
    // 4  -- ENABLE:[X] FILTER:[TRILINEAR2] 
    // 5  -- MIPMAP:[X] MIPMAPBIAS:[NORMAL] UV_FLIP:[UV]
    // 1  -- UV_CLAMP:[UV] ALPHA:[X] ENV:[MODULATEALPHA]
    // 2  -- WIDTH:256 HEIGHT:256 FORMAT:[0xFFFFFFFF]
    // 3  TXR2:
    // 4  -- ENABLE:[X] FILTER:[TRILINEAR2]
    // 5  -- MIPMAP:[X] MIPMAPBIAS:[NORMAL] UV_FLIP:[UV]
    // 1  -- UV_CLAMP:[UV] ALPHA:[X] ENV:[MODULATEALPHA]
    // 2  -- WIDTH:256 HEIGHT:256 FORMAT:[0xFFFFFFFF]
    // 3



    if(indice_memberModel[0]==1)
    {
        strcat(buffer, "\\c[A0A0A0]-");
    }else
    {
        strcat(buffer, "-");
    }
    strcat(buffer, "\\c[A0A0A0]LIST_TYPE :");
    formatNumber(buffer, cxt->list_type, indice_memberModel[0],"LIST_TYPE"); // 1er nombre avec crochets
    strcat(buffer, "\nGEN:\n");
    strcat(buffer, "-- ALPHA:");
    formatNumber(buffer, cxt->gen.alpha, indice_memberModel[1],"CXT_ALPHA"); // 2e nombre avec crochets
    strcat(buffer, " SHADING:");
    formatNumber(buffer, cxt->gen.shading, indice_memberModel[2],"SHADING"); // 3e nombre avec crochets
    
    strcat(buffer, "\n");
    strcat(buffer, "-- FOG_TYPE:");
    formatNumber(buffer, cxt->gen.fog_type, indice_memberModel[3],"FOG_TYPE");
    strcat(buffer, " CULLING:");
    formatNumber(buffer, cxt->gen.culling, indice_memberModel[4],"CULLING");
    strcat(buffer, "\n");

    strcat(buffer, "-- COLOR_CLAMP:");
    formatNumber(buffer, cxt->gen.color_clamp, indice_memberModel[5],"ENABLE");
    strcat(buffer, " CLIP_MODE:");
    formatNumber(buffer, cxt->gen.clip_mode, indice_memberModel[6],"CLIP_MODE");
    strcat(buffer, " MODIFIER_MODE:");
    formatNumber(buffer, cxt->gen.modifier_mode, indice_memberModel[7],"ENABLE");
    strcat(buffer, "\n");

    strcat(buffer, "-- SPECULAR:");
    formatNumber(buffer, cxt->gen.specular, indice_memberModel[8],"HEX32BITS");
    strcat(buffer, " ALPHA2:");
    formatNumber(buffer, cxt->gen.alpha2, indice_memberModel[9],"ENABLE");
    strcat(buffer, "\n");

    strcat(buffer, "-- FOG_TYPE2:");
    formatNumber(buffer, cxt->gen.fog_type2, indice_memberModel[10],"FOG_TYPE");
    strcat(buffer, " COLOR_CLAMP2:");
    formatNumber(buffer, cxt->gen.color_clamp2, indice_memberModel[11],"ENABLE");
    strcat(buffer, "\n");

    strcat(buffer, "BLEND:\n--");
    strcat(buffer, " SRC:");
    formatNumber(buffer, cxt->blend.src, indice_memberModel[12],"BLEND_MODE");
    strcat(buffer, " DST:");
    formatNumber(buffer, cxt->blend.dst, indice_memberModel[13],"BLEND_MODE");
    strcat(buffer,"\n--");
    strcat(buffer, " SRC_ENABLE:");
    formatNumber(buffer, cxt->blend.src_enable, indice_memberModel[14],"ENABLE");
    strcat(buffer, " DST_ENABLE:");
    formatNumber(buffer, cxt->blend.dst_enable, indice_memberModel[15],"ENABLE");
    strcat(buffer,"\n--");
    strcat(buffer, " SRC2:");
    formatNumber(buffer, cxt->blend.src2, indice_memberModel[16],"BLEND_MODE");
    strcat(buffer, " DST2:");
    formatNumber(buffer, cxt->blend.dst2, indice_memberModel[17],"BLEND_MODE");
    strcat(buffer,"\n--");
    strcat(buffer, " SRC_ENABLE2:");
    formatNumber(buffer, cxt->blend.src_enable2, indice_memberModel[18],"ENABLE");
    strcat(buffer, " DST_ENABLE2:");
    formatNumber(buffer, cxt->blend.dst_enable2, indice_memberModel[19],"ENABLE");
    strcat(buffer, "\nFMT:\n--");
    strcat(buffer, " COLOR:");
    formatNumber(buffer, cxt->fmt.color, indice_memberModel[20],"COLOR_FMT");

    strcat(buffer, " UV:");
    formatNumber(buffer, cxt->fmt.uv, indice_memberModel[21],"UV_FMT");

    strcat(buffer, " MODIFIER:");
    formatNumber(buffer, cxt->fmt.modifier, indice_memberModel[22],"ENABLE");


    strcat(buffer, "\nDEPTH:\n--");
    strcat(buffer, " COMPARISON:");
    formatNumber(buffer, cxt->depth.comparison, indice_memberModel[23],"DEPTH");
    strcat(buffer, " WRITE:");
    formatNumber(buffer, cxt->depth.write, indice_memberModel[24],"ENABLE");
    strcat(buffer,"\n--");
    strcat(buffer, " ENABLE:");
    formatNumber(buffer, cxt->txr.enable, indice_memberModel[25],"ENABLE");
    strcat(buffer, " FILTER:");
    formatNumber(buffer, cxt->txr.filter, indice_memberModel[26],"FILTER");
    strcat(buffer,"\n--");
    strcat(buffer, " MIPMAP:");
    formatNumber(buffer, cxt->txr.mipmap, indice_memberModel[27],"ENABLE");
    strcat(buffer, " MIPMAP_BIAS:");
    formatNumber(buffer, cxt->txr.mipmap_bias, indice_memberModel[28],"MIPMAP_BIAS");
    strcat(buffer, " UV_FLIP:");
    formatNumber(buffer, cxt->txr.uv_flip, indice_memberModel[29],"UV_FLIP");
    strcat(buffer,"\n--");
    strcat(buffer, " UV_CLAMP:");
    formatNumber(buffer, cxt->txr.uv_clamp, indice_memberModel[30],"UV_CLAMP");
    strcat(buffer, " ALPHA:");
    formatNumber(buffer, cxt->txr.alpha, indice_memberModel[31],"ENABLE");
    strcat(buffer, " ENV:");
    formatNumber(buffer, cxt->txr.env, indice_memberModel[32],"ENV");
    strcat(buffer,"\n--");
    strcat(buffer, " WIDTH:");
    formatNumber(buffer, cxt->txr.width, indice_memberModel[33],"INT");
    strcat(buffer, " HEIGHT:");
    formatNumber(buffer, cxt->txr.height, indice_memberModel[34],"INT");
    strcat(buffer, " FORMAT:");
    formatNumber(buffer, cxt->txr.format, indice_memberModel[35],"INT");

    strcat(buffer, "\nTXR2:\n");
    strcat(buffer,"--ENABLE:");
    
    formatNumber(buffer, cxt->txr2.enable, indice_memberModel[36],"ENABLE");
    strcat(buffer, " FILTER:");
    formatNumber(buffer, cxt->txr2.filter, indice_memberModel[37],"FILTER");
    strcat(buffer, "\n-- MIPMAP:");
    formatNumber(buffer, cxt->txr2.mipmap, indice_memberModel[38],"ENABLE");
    strcat(buffer, " MIPMAP_BIAS:");
    formatNumber(buffer, cxt->txr2.mipmap_bias, indice_memberModel[39],"MIPMAP_BIAS");
    strcat(buffer, " UV_FLIP:");
    formatNumber(buffer, cxt->txr2.uv_flip, indice_memberModel[40],"UV_FLIP");
    strcat(buffer, "\n-- UV_CLAMP:");
    formatNumber(buffer, cxt->txr2.uv_clamp, indice_memberModel[41],"UV_CLAMP");
    strcat(buffer, " ALPHA:");
    formatNumber(buffer, cxt->txr2.alpha, indice_memberModel[42],"ENABLE");
    strcat(buffer, " END:");
    formatNumber(buffer, cxt->txr2.env, indice_memberModel[43],"ENV");
    strcat(buffer, "\n-- WIDTH:");
    formatNumber(buffer, cxt->txr2.width, indice_memberModel[44],"INT");
    strcat(buffer, " HEIGHT:");
    formatNumber(buffer, cxt->txr2.height, indice_memberModel[45],"INT");
    strcat(buffer, " FORMAT:");
    formatNumber(buffer, cxt->txr2.format, indice_memberModel[46],"INT");
    



    
    if(indice_memberModel[0]==1)
    {
        strcat(buffer, "]\\c[A0A0A0]\n");
    }else
    {
        strcat(buffer, "\n");
    }

}

void navigateToListMeshes(dct_tinyDebugger_t *debugger, char *buffer)
{
    int indice_memberModel[50]   ={0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
                                           0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0}; 
    for(int i=0; i<sizeof(indice_memberModel)/sizeof(int); i++)
    {
        indice_memberModel[i]=0;
    }

    
    for (int i=0; i<sizeof(debugger->listMeshes.meshesBuffer)/sizeof(uint32_t); i++ )
    {
        if(debugger->listMeshes.meshesBuffer[i]!=0)
        {

            char temp_value[100];
            dct_mesh_t *mesh = debugger->listMeshes.meshesBuffer[i];
            
            switch(debugger->indiceCurrentMemberModel)
            {
                case 0:
                    indice_memberModel[0]=1;
                    debugger->currentDataToEdit=NULL;
                    break;
                case 1:
                    indice_memberModel[1]=1;
                    debugger->currentDataToEdit=mesh;
                    break;
            
            }
            if(indice_memberModel[0]==1)
            {
                strcat(buffer, "\\c[E0E0E0][LIST MESHES]");
                strcat(buffer, "\n");
            }
            else
            {
                strcat(buffer, "\\c[A0A0A0]LIST MESHES :");
                strcat(buffer, "\n");
            }

            if(indice_memberModel[1]==1)
            {
                
                snprintf(temp_value, sizeof(temp_value), "\\c[E0E0E0][MESH : %s]",mesh->name);
                strcat(buffer, temp_value);
                strcat(buffer, "\n");
            }
            else
            {
                snprintf(temp_value, sizeof(temp_value), "\\c[A0A0A0]MESH : %s",mesh->name);
                strcat(buffer, temp_value);
                strcat(buffer, "\n");
            }

        }
        

        //formatNumber(buffer, mod->meshes[0], indice_memberModel[5],"FLOAT"); // 1er nombre sans crochets
        
        

    }



}

void addBufferBreadCrumb(dct_tinyDebugger_t *debugger,char *buffer)
{

    printf("\n\n");
    printf("\n");
    char breadcrumb[64]={0};
    for (int i=9; i>-1; i--)
    {
        //printf("debugger->lastNavigationData[%d]=%d \n",i,debugger->lastNavigationData[i]); 
        if(debugger->lastNavigationData[i]!=0)
        {  
            
            DCT_IDENT id = *(DCT_IDENT*)debugger->lastNavigationData[i];
            switch(id)
            {
                case DCT_MODEL:
                    dct_model_t *mod = (dct_model_t*)debugger->lastNavigationData[i];
                    printf("/%s",mod->name );
                    snprintf(breadcrumb, sizeof(breadcrumb), "/%s", mod->name);
                    strcat(buffer,breadcrumb);
                    break;
                case DCT_LISTMESHESDEBUGGER_T:
                    dct_listMeshesDebugger_t *list = (dct_listMeshesDebugger_t*)debugger->lastNavigationData[i];
                    printf("/MESHES");
                    snprintf(breadcrumb, sizeof(breadcrumb), "/MESHES" );
                    strcat(buffer,breadcrumb);
                    break;
                case DCT_MESH:
                    dct_mesh_t *mesh = (dct_mesh_t*)debugger->lastNavigationData[i];
                    printf("/%s",mesh->name );
                    snprintf(breadcrumb, sizeof(breadcrumb), "/%s", mesh->name);
                    strcat(buffer,breadcrumb);
                    break;
                case DCT_CXT_T:
                    dct_cxt_t *cxt = (dct_cxt_t*)debugger->lastNavigationData[i];
                    printf("/CXT\n");
                    snprintf(breadcrumb, sizeof(breadcrumb), "/CXT");
                    strcat(buffer,breadcrumb);
                    break;
                
            }     
            
        }
    }


    DCT_IDENT id = *(DCT_IDENT*)debugger->currentStruct;
    switch(id)
    {
        case DCT_MODEL:
            dct_model_t *mod = (dct_model_t*)debugger->currentStruct;
            printf("/%s",mod->name );
            snprintf(breadcrumb, sizeof(breadcrumb), "/%s", mod->name);
            strcat(buffer,breadcrumb);
            break;
        case DCT_LISTMESHESDEBUGGER_T:
            dct_listMeshesDebugger_t *list = (dct_listMeshesDebugger_t*)debugger->currentStruct;
            printf("/MESHES");
            snprintf(breadcrumb, sizeof(breadcrumb), "/MESHES" );
            strcat(buffer,breadcrumb);
            break;
        case DCT_MESH:
            dct_mesh_t *mesh = (dct_mesh_t*)debugger->currentStruct;
            printf("/%s",mesh->name );
            snprintf(breadcrumb, sizeof(breadcrumb), "/%s", mesh->name);
            strcat(buffer,breadcrumb);
            break;
        case DCT_CXT_T:
            dct_cxt_t *cxt = (dct_cxt_t*)debugger->currentStruct;
            printf("/CXT\n");
            snprintf(breadcrumb, sizeof(breadcrumb), "/CXT");
            strcat(buffer,breadcrumb);
            break;
        
    }    


    printf("\n");
    snprintf(breadcrumb, sizeof(breadcrumb), "\n");
    strcat(buffer,breadcrumb);
    
}

void navigateToModel(dct_model_t *mod, dct_tinyDebugger_t *debugger, char *buffer)
{
    
    printf("-- FN Navigate Model => model name %s type %d meshesCount %d \n", mod->name, mod->type , mod->meshesCount );
    
    switch(mod->type)
    {
        case MODEL_STD:
            // Buffer pour stocker la chaîne finale
            

            // Fonction pour formater un nombre avec ou sans crochets

            int indice_model[50]         ={0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
                                           0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0}; 
            int indice_memberModel[50]   ={0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
                                           0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0}; 
             // 0 => model 1 => member direcot of model 2 => exemple model=>meshe=>
            
            void resetInt(int indice)
            {
                for(int i=0; i<sizeof(indice)/sizeof(int); i++)
                {
                    indice_model[i] = 0;
                    indice_memberModel[i]=0;
                    
                }
            }


            switch(debugger->indiceCurrentMemberModel)
            {
                
                case 0:
                    resetInt(indice_memberModel);
                    debugger->isInsideDataStructure = false;
                    indice_memberModel[0]=1;
                    break;
                case 1:
                    resetInt(indice_memberModel);
                    indice_memberModel[1]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mod->position.x;
                    debugger->currentDataToEdit_IntOrFloat = 1;
                    break;
                case 2:
                    resetInt(indice_memberModel);
                    indice_memberModel[2]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mod->position.y;
                    debugger->currentDataToEdit_IntOrFloat = 1;
                    break;
                case 3:
                    resetInt(indice_memberModel);
                    indice_memberModel[3]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mod->position.z;
                    debugger->currentDataToEdit_IntOrFloat = 1;
                    break;
                case 4:
                    resetInt(indice_memberModel);
                    debugger->isInsideDataStructure = false;
                    indice_memberModel[4]=1;
                    break;
                case 5:
                    resetInt(indice_memberModel);
                    indice_memberModel[5]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mod->rotation.x;
                    debugger->currentDataToEdit_IntOrFloat = 1;
                    break;
                case 6:
                    resetInt(indice_memberModel);
                    indice_memberModel[6]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mod->rotation.y;
                    debugger->currentDataToEdit_IntOrFloat = 1;
                    break;
                
                case 7:
                    resetInt(indice_memberModel);
                    indice_memberModel[7]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mod->rotation.z;
                    debugger->currentDataToEdit_IntOrFloat = 1;
                    break;
                case 8:
                    resetInt(indice_memberModel);
                    debugger->isInsideDataStructure = false;
                    indice_memberModel[8]=1;
                    break;
                case 9:
                    resetInt(indice_memberModel);
                    indice_memberModel[9]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mod->scale.x;
                    debugger->currentDataToEdit_IntOrFloat = 1;
                    break;
                case 10:
                    resetInt(indice_memberModel);
                    indice_memberModel[10]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mod->scale.y;
                    debugger->currentDataToEdit_IntOrFloat = 1;
                    break;
                case 11:
                    resetInt(indice_memberModel);
                    indice_memberModel[11]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mod->scale.z;
                    debugger->currentDataToEdit_IntOrFloat = 1;
                    break;
                case 12:
                    resetInt(indice_memberModel);
                    indice_memberModel[12]=1;
                    
                    debugger->currentDataToEdit=NULL;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                case 13:
                    resetInt(indice_memberModel);
                    indice_memberModel[13]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mod->meshes[0].cxt.gen.alpha;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                case 14:
                    resetInt(indice_memberModel);
                    indice_memberModel[14]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mod->meshes[0].cxt.gen.shading;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                case 15:
                    resetInt(indice_memberModel);
                    indice_memberModel[15]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mod->meshes[0].cxt.gen.fog_type;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                case 16:
                    resetInt(indice_memberModel);
                    indice_memberModel[16]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mod->meshes[0].cxt.gen.culling;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                case 17:
                    resetInt(indice_memberModel);
                    indice_memberModel[17]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mod->meshes[0].cxt.gen.color_clamp;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                case 18:
                    resetInt(indice_memberModel);
                    indice_memberModel[18]=1;
                    break;
                case 19:
                    resetInt(indice_memberModel);
                    indice_memberModel[19]=1;
                    break;
                case 20:
                    resetInt(indice_memberModel);
                    indice_memberModel[20]=1;
                    break;
                case 21:
                    resetInt(indice_memberModel);
                    indice_memberModel[21]=1;
                    break;
                case 22:
                    resetInt(indice_memberModel);
                    indice_memberModel[22]=1;
                    debugger->isInsideDataStructure = false;
                    debugger->currentDataToEdit=&mod->meshes[0];
                    break;
                case 23:
                    resetInt(indice_memberModel);
                    indice_memberModel[23]=1;
                    debugger->isInsideDataStructure = false;
                    debugger->currentDataToEdit=&debugger->listMeshes;
                    break;

                case 24:
                    resetInt(indice_memberModel);
                    indice_memberModel[24]=1;
                    debugger->isInsideDataStructure = false;
                    debugger->currentDataToEdit=NULL;
                    break;
                
                case 25:
                    resetInt(indice_memberModel);
                    indice_memberModel[25]=1;
                    
                    debugger->isInsideDataStructure = true;
                    debugger->fnDebug.ident=DCT_FN_PRINT_MODEL;
                    debugger->fnDebug.fnDebugPtr = debug_print_model;
                    debugger->fnDebug.fnDebugPtrArg = mod;
                    debugger->currentDataToEdit=&debugger->fnDebug;
                    break;
                    

            }

            if(indice_memberModel[0]==1)
            {
                strcat(buffer, "\\c[E0E0E0][position : ");
                printf("postion indiceCurrentmember mode 0 \n");
            }else
            {
                strcat(buffer, "\\c[A0A0A0]position : ");
            }
            
            
            formatNumber(buffer, mod->position.x, indice_memberModel[1],"FLOAT"); // 1er nombre avec crochets
            strcat(buffer, " ");
            formatNumber(buffer, mod->position.y, indice_memberModel[2],"FLOAT"); // 2e nombre sans crochets
            strcat(buffer, " ");
            formatNumber(buffer, mod->position.z, indice_memberModel[3],"FLOAT"); // 3e nombre avec crochets
            
            if(indice_memberModel[0]==1)
            {
                strcat(buffer, "]\\c[A0A0A0]\n");
            }else
            {
                strcat(buffer, "\n");
            }
            //strcat(buffer, "\n");


            if(indice_memberModel[4]==1)
            {
                strcat(buffer, "\\c[E0E0E0][rotation : ");
            }
            else
            {
                strcat(buffer, "rotation : ");
            }
            
            formatNumber(buffer, mod->rotation.x, indice_memberModel[5],"FLOAT"); // 1er nombre sans crochets
            strcat(buffer, " ");
            formatNumber(buffer, mod->rotation.y, indice_memberModel[6],"FLOAT"); // 2e nombre avec crochets
            strcat(buffer, " ");
            formatNumber(buffer, mod->rotation.z, indice_memberModel[7],"FLOAT"); // 3e nombre sans crochets
            
            if(indice_memberModel[4]==1)
            {
                strcat(buffer, "]\\c[A0A0A0]\n");
            }else
            {
                strcat(buffer, "\n");
            }
            //strcat(buffer, "\n");

            if(indice_memberModel[8]==1)
            {
                strcat(buffer, "\\c[E0E0E0][scale    : ");
            }else
            {
                strcat(buffer, "scale    : ");
            }
            
            formatNumber(buffer, mod->scale.x, indice_memberModel[9],"FLOAT"); // 1er nombre avec crochets
            strcat(buffer, " ");
            formatNumber(buffer, mod->scale.y, indice_memberModel[10],"FLOAT"); // 2e nombre avec crochets
            strcat(buffer, " ");
            formatNumber(buffer, mod->scale.z, indice_memberModel[11],"FLOAT"); // 3e nombre avec crochets
            
            if(indice_memberModel[8]==1)
            {
                strcat(buffer, "]\\c[A0A0A0]\n");
            }else
            {
                strcat(buffer, "\n");
            }

            if(indice_memberModel[12]==1)
            {
                strcat(buffer, "\\c[E0E0E0]cxt    : ");
            }else
            {
                strcat(buffer, "cxt    : ");
            }
            
            formatNumber(buffer, mod->meshes[0].cxt.list_type, indice_memberModel[13],"INT"); // 1er nombre avec crochets
            strcat(buffer, " ");
            formatNumber(buffer, mod->meshes[0].cxt.gen.alpha, indice_memberModel[14],"CXT_ALPHA"); // 2e nombre avec crochets
            strcat(buffer, " ");
            formatNumber(buffer, mod->meshes[0].cxt.gen.shading, indice_memberModel[15],"INT"); // 3e nombre avec crochets
            strcat(buffer, " ");
            formatNumber(buffer, mod->meshes[0].cxt.gen.fog_type, indice_memberModel[16],"INT");
            strcat(buffer, " ");
            formatNumber(buffer, mod->meshes[0].cxt.gen.culling, indice_memberModel[17],"INT");
            strcat(buffer, " ");
            formatNumber(buffer, mod->meshes[0].cxt.gen.color_clamp, indice_memberModel[18],"INT");
            strcat(buffer, " ");
            formatNumber(buffer, mod->meshes[0].cxt.gen.clip_mode, indice_memberModel[19],"INT");
            strcat(buffer, " ");
            formatNumber(buffer, mod->meshes[0].cxt.gen.modifier_mode, indice_memberModel[20],"INT");
            strcat(buffer, " ");
            formatNumber(buffer, mod->meshes[0].cxt.gen.specular, indice_memberModel[21],"INT");
            
            if(indice_memberModel[12]==1)
            {
                strcat(buffer, "]\\c[A0A0A0]\n");
            }else
            {
                strcat(buffer, "\n");
            }
            if(indice_memberModel[22]==1)
            {
                strcat(buffer, "\\c[E0E0E0][CXT MESH]\\c[A0A0A0]");
            }
            else
            {
                strcat(buffer, "CXT MESH");
            }
            strcat(buffer, "\n");

            if(indice_memberModel[23]==1)
            {
                strcat(buffer, "\\c[E0E0E0][LIST MESHES]\\c[A0A0A0]");
            }
            else
            {
                strcat(buffer, "LIST MESHES");
            }
            strcat(buffer, "\n");
            if(indice_memberModel[24]==1)
            {
                strcat(buffer, "\\c[E0E0E0][DEBUG PRINT] :\\c[A0A0A0]");
            }
            else
            {
                strcat(buffer, "DEBUG PRINT :");
            }

            if(indice_memberModel[25]==1)
            {
                strcat(buffer, "\\c[E0E0E0] [MODEL]\\c[A0A0A0]");
            }
            else
            {
                strcat(buffer, "MODEL");
            }
            strcat(buffer, " ");
            // Affichage du résultat
            printf("%s", buffer);
    
            break;
        
        case MODEL_PCM:
            printf("position : %.3f %.3f %.3f \n", mod->position.x, mod->position.y,mod->position.z );

            printf("rotation : %.3f %.3f %.3f \n", mod->rotation.x,mod->rotation.y,mod->rotation.z);
            
            printf("scale    : %.3f %.3f %.3f \n", mod->scale.x,mod->scale.y,mod->scale.z );
            
            printf("modifier : name %s triCount %d \n",mod->modifier.name, mod->modifier.triCount );
            printf("modifier transform : %f %f %f %f ", mod->modifier.transform[0][0],mod->modifier.transform[0][1],mod->modifier.transform[0][2],mod->modifier.transform[0][3] );


            break;
        case MODEL_TPCM:

            break;
    }

    printf("NAVIGATE TO MODEL AFTER SWITCH \n");
    // for (int i_mesh=0; i_mesh<mod->meshesCount; i_mesh++)
    // {

    //     printf("meshe %d type %s vtxCount %d triCount %d \n",i_mesh)
    // }

    for (int i_txt=0; i_txt<mod->texturesCount; i_txt++ )
    {
        if(mod->textures!=NULL)
        {
            if(mod->textures[i_txt].addrA!=NULL)
            {
                printf("-- cxt %d %d %d %d \n",mod->textures[i_txt].cxt.list_type, mod->textures[i_txt].cxt.gen.alpha,mod->textures[i_txt].cxt.gen.shading, mod->textures[i_txt].cxt.gen.fog_type);
                printf("-- texture A w:%d h:%d \n", mod->textures[i_txt].textureA.w,mod->textures[i_txt].textureA.h );
            
            }

            if(mod->textures[i_txt].addrB!=NULL)

            {
                printf("-- texture B w:%d h:%d \n", mod->textures[i_txt].textureB.w,mod->textures[i_txt].textureB.h );
            }
        
        
        }
        
    }



    
}

void wrapper_debug_print_model(uint32_t *ptr)
{
    dct_model_t *mod = ptr;
    debug_print_model(mod);
}

void navigateToMesh(dct_mesh_t *mesh, dct_tinyDebugger_t *debugger, char *buffer)
{
    
    printf("-- FN Navigate Model => model name %s type %d vtxCount %d \n", mesh->name, mesh->type , mesh->vtxCount );

    switch(mesh->type)
    {
        case MESH_STD_OP:
            // Buffer pour stocker la chaîne finale
            

            // Fonction pour formater un nombre avec ou sans crochets

            int indice_model[50]         ={0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
                                           0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0}; 
            int indice_memberModel[50]   ={0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
                                           0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0}; 
            
             // 0 => model 1 => member direcot of model 2 => exemple model=>meshe=>
            
            void resetInt(int indice)
            {
                for(int i=0; i<sizeof(indice)/sizeof(int); i++)
                {
                    indice_model[i] = 0;
                    indice_memberModel[i]=0;
                    
                }
            }


            switch(debugger->indiceCurrentMemberModel)
            {
                
                case 0:
                    resetInt(indice_memberModel);
                    indice_memberModel[0]=1;
                    debugger->currentDataToEdit=NULL;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                
                case 1:
                    resetInt(indice_memberModel);
                    indice_memberModel[1]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mesh->shading;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                
                case 2:
                    resetInt(indice_memberModel);
                    debugger->currentDataToEdit=NULL;
                    indice_memberModel[2]=1;
                    //debugger->currentDataToEdit=&mod->position.x;
                    //debugger->currentDataToEdit_IntOrFloat = 1;
                    break;
                

                
                case 3:
                    resetInt(indice_memberModel);
                    indice_memberModel[3]=1;
                    debugger->isInsideDataStructure = false;
                    //debugger->currentDataToEdit=&mod->meshes[0];
                    break;
                
                case 4:
                    resetInt(indice_memberModel);
                    indice_memberModel[4]=1;
                    break;

                case 5:
                    resetInt(indice_memberModel);
                    indice_memberModel[5]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mesh->cxt.list_type;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                case 6:
                    resetInt(indice_memberModel);
                    indice_memberModel[6]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mesh->cxt.gen.alpha;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                case 7:
                    resetInt(indice_memberModel);
                    indice_memberModel[7]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mesh->cxt.gen.shading;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                case 8:
                    resetInt(indice_memberModel);
                    indice_memberModel[8]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mesh->cxt.gen.fog_type;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                case 9:
                    resetInt(indice_memberModel);
                    indice_memberModel[9]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mesh->cxt.gen.culling;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                
                case 10:
                    resetInt(indice_memberModel);
                    indice_memberModel[10]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mesh->cxt.gen.color_clamp;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                case 11:
                    resetInt(indice_memberModel);
                    indice_memberModel[11]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mesh->cxt.gen.clip_mode;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                case 12:
                    resetInt(indice_memberModel);
                    indice_memberModel[12]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mesh->cxt.gen.modifier_mode;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                case 13:
                    resetInt(indice_memberModel);
                    indice_memberModel[13]=1;
                    debugger->isInsideDataStructure = true;
                    debugger->currentDataToEdit=&mesh->cxt.gen.specular;
                    debugger->currentDataToEdit_IntOrFloat = 0;
                    break;
                case 14:
                    resetInt(indice_memberModel);
                    indice_memberModel[14]=1;
                    debugger->isInsideDataStructure = false;
                    debugger->currentDataToEdit=&mesh->cxtMesh;
                    break;

            }

            // Exemple : ajouter des crochets uniquement autour du 1er et du 3e nombre
            if(indice_memberModel[0]==1)
            {
                strcat(buffer, "\\c[E0E0E0][shading : ");
            }else
            {
                strcat(buffer, "\\c[A0A0A0]shading : ");
            }
            
            formatNumber(buffer, mesh->shading, indice_memberModel[0],"BOOL"); // 1er nombre avec crochets
            //strcat(buffer, "\n");

            if(indice_memberModel[0]==1)
            {
                strcat(buffer, "]\\c[A0A0A0]\n");
            }else
            {
                strcat(buffer, "\n");
            }

            // Exemple : ajouter des crochets uniquement autour du 1er et du 3e nombre
            if(indice_memberModel[2]==1)
            {
                strcat(buffer, "\\c[A0A0A0][vtx : ");
            }else
            {
                strcat(buffer, "vtx : ");
            }
            
            formatNumber(buffer, mesh->vtxCount, 0,"INT"); // 1er nombre avec crochets
            //strcat(buffer, "\n");

            if(indice_memberModel[2]==1)
            {
                strcat(buffer, "]\\c[A0A0A0]\n");
            }else
            {
                strcat(buffer, "\n");
            }


             if(indice_memberModel[3]==1)
            {
                strcat(buffer, "\\c[E0E0E0][tris : ");
            }else
            {
                strcat(buffer, "tri : ");
            }
            
            formatNumber(buffer, mesh->triCount, 0,"INT"); // 1er nombre avec crochets
            //strcat(buffer, "\n");

            if(indice_memberModel[3]==1)
            {
                strcat(buffer, "]\\c[A0A0A0]\n");
            }else
            {
                strcat(buffer, "\n");
            }


            if(indice_memberModel[4]==1)
            {
                strcat(buffer, "\\c[E0E0E0][CXT : ");
            }else
            {
                strcat(buffer, "CXT : ");
            }

            formatNumber(buffer, mesh->cxt.list_type, indice_memberModel[5],"INT"); // 1er nombre avec crochets
            strcat(buffer, " ");
            formatNumber(buffer, mesh->cxt.gen.alpha, indice_memberModel[6],"CXT_ALPHA"); // 2e nombre avec crochets
            strcat(buffer, " ");
            formatNumber(buffer, mesh->cxt.gen.shading, indice_memberModel[7],"INT"); // 3e nombre avec crochets
            strcat(buffer, " ");
            formatNumber(buffer, mesh->cxt.gen.fog_type, indice_memberModel[8],"INT");
            strcat(buffer, " ");
            formatNumber(buffer, mesh->cxt.gen.culling, indice_memberModel[9],"INT");
            strcat(buffer, " ");
            formatNumber(buffer, mesh->cxt.gen.color_clamp, indice_memberModel[10],"INT");
            strcat(buffer, " ");
            formatNumber(buffer, mesh->cxt.gen.clip_mode, indice_memberModel[11],"INT");
            strcat(buffer, " ");
            formatNumber(buffer, mesh->cxt.gen.modifier_mode, indice_memberModel[12],"INT");
            strcat(buffer, " ");
            formatNumber(buffer, mesh->cxt.gen.specular, indice_memberModel[13],"INT");


            if(indice_memberModel[4]==1)
            {
                strcat(buffer, "]\\c[A0A0A0]\n");
            }else
            {
                strcat(buffer, "\n");
            }


             if(indice_memberModel[14]==1)
            {
                strcat(buffer, "\\c[E0E0E0][CXT MESH] ");
            }else
            {
                strcat(buffer, "CXT MESH : ");
            }
            
            
            strcat(buffer, " ");
            // Affichage du résultat
            printf("%s", buffer);
    
            break;
        
        
    }
    
}

void listMemoryValues(void* startAddress) {
    uint32_t* ptr = (uint32_t*)startAddress;
    
    printf("Memory dump starting at address %p:\n", startAddress);
    for(int i = 0; i < 100; i++) {
        printf("Addr: %p | Value: 0x%08x\n", 
            (void*)(ptr + i),  // adresse
            *(ptr + i)         // valeur
        );
    }
}

bool isNextCurrentDataToEditBasedOnIndiceCurrentMemberModelValid(uint32_t addr)
{

    DCT_IDENT checkType = *(DCT_IDENT*)addr;
    printf("isNextCurrentDataToEditBasedOnIndiceCurrentMemberModelValid %08x \n",checkType);

    if(IS_VALID_DCT_IDENT_VAL(checkType))
    {
        return true;
    }

    return false;
}


void updateTinyDebuggerTexture(dct_tinyDebugger_t *debugger)
{
    dct_scene_t *sc;
    sc = getCurrentScene();
    char buffer[4096];
    buffer[0] = '\0'; // Initialisation du buffer

    
    // pour DCT_MODEL indice 0 correspond à position.x 
    // type = DCT_MODEL indice = 0  offset = offset jusqu'a la data posiiton.x offsetTo(model->position.x)
    // type = DCT_MODEL incide = 1  offset = model->position.y

    for (int i=0; i<MAX_MODELS_SCENE; i++)
    {
        if(sc->ptrListModelScene[i]!=NULL)
        {
        if(i==debugger->indiceCurrentModel)
            {
                
            }
        }
        
    }
    //printf("debugger->indiceCurrentMemberModel %d \n ",debugger->indiceCurrentMemberModel);
    //dct_model_t *mod = sc->ptrListModelScene[i];
    uint32_t *currentEntity = debugger->currentDataToNavigate;
    DCT_IDENT checkType = *(DCT_IDENT*)debugger->currentDataToNavigate;
    //printf("DCT_MESH %08x \n",DCT_MESH);
    //printf("IS_VALID %08x \n",*(DCT_IDENT*)debugger->currentDataToNavigate);
    //printf("IS_VALIDcheckType %08x \n", checkType);
    //printf("IS_VALID_DCT_IDENT_VAL %d \n",IS_VALID_DCT_IDENT_VAL(0xAB03));

    addBufferBreadCrumb(debugger,buffer);
    if(IS_VALID_DCT_IDENT_VAL(checkType))
    {
        //uint32_t *mod = NULL;
        switch(checkType)
        {
            case DCT_MODEL:
                printf("DCT_MODEL_T DETECTED !\n");
                dct_model_t *mod = (dct_model_t*)currentEntity;
                debugger->currentOffset = 0;
                setDebuggerListMeshesCurrentModel(debugger,mod);
                navigateToModel(mod, debugger, buffer);
                break;
            case DCT_LISTMESHESDEBUGGER_T:
                printf("DCT_LISTMESHDEBUGGER DETECTED !\n");
                //dct_mesh_t *mesh = (dct_mesh_t*)currentEntity;
                debugger->currentOffset = 3;
                navigateToListMeshes(debugger, buffer);
                break;
            case DCT_MESH:
                printf("DCT_MESH DETECTED !\n");
                debugger->currentOffset = 1;
                dct_mesh_t *mesh = (dct_mesh_t*)currentEntity;
                navigateToMesh(mesh, debugger, buffer);
                break;
            case DCT_CXT_T:
                printf("DCT_CXT_T DETECTED !\n");
                dct_cxt_t *cxt = (dct_cxt_t*)currentEntity;
                debugger->currentOffset = 2;
                navigateToCxtMesh(cxt, debugger, buffer);
                break;
        }

    }
    
    update_text_texture( debugger->textureDisplay, buffer, debugger->font, 0xE0505055);
}

void insertNavigationData(dct_tinyDebugger_t *debugger, uint32_t *ptr)
{
    uint32_t *tmpNavigationData[9] = {0,0,0, 0,0,0 ,0,0,0};
    for (int i=0; i<10-1; i++)
    {
        tmpNavigationData[i]=debugger->lastNavigationData[i];
    }

    for (int i=0; i<10-1; i++)
    {
        if(i==0)
        {
            debugger->lastNavigationData[i]=ptr;
        }else
        {
            debugger->lastNavigationData[i]=tmpNavigationData[i-1];
        }
        
    }
}

void removeFirstNavigationData(dct_tinyDebugger_t *debugger)
{
    uint32_t *tmpNavigationData[10] = {0,0,0, 0,0,0 ,0,0,0, 0};
    
    debugger->currentDataToNavigate = debugger->lastNavigationData[0];

    for (int i=0; i<10; i++)
    {
        tmpNavigationData[i]=debugger->lastNavigationData[i];
    }

    for (int i=0; i<10-1; i++)
    {
        debugger->lastNavigationData[i]=debugger->lastNavigationData[i+1];
    }
    

}


void setDebuggerListMeshesCurrentModel(dct_tinyDebugger_t *debugger,dct_model_t *mod)
{
    // RESET 
    for (int i=0; i<20; i++)
    {
        debugger->listMeshes.ident = 0;
        debugger->listMeshes.meshesBuffer[i] = 0;
    }
    // SET
    for(int i=0; i<mod->meshesCount; i++)
    {
        printf("INSERT LISTMESHES ELEMENT %d meshesCount %d \n",i, mod->meshesCount );
        debugger->listMeshes.ident = DCT_LISTMESHESDEBUGGER_T;
        debugger->listMeshes.meshesBuffer[i] = &mod->meshes[i];
    }
}

void createTextures(dct_tinyDebugger_t *debugger)
{
    dct_scene_t *sc;
    sc = getCurrentScene();

    printf("FN CREATE TEXTURES CHECK IDENT->DCT_MODEL CUBE %x \n", *(DCT_IDENT*)debugger->currentDataToNavigate);

    debugger->indiceCurrentModel = 1;
    debugger->indiceCurrentMemberModel = 0;
    debugger->isInsideDataStructure = false;
    int temp_nextMesh[24]={0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0};
    int temp[25]    ={4,0,0,0,4, 0,0,0,4,0, 0,0,10,0,0, 0,0,0,0,0, 0,0,0,0,0};
    int tempprev[25]={0,0,0,0,4, 0,0,0,4,0, 0,0,4,0,0,  0,0,0,0,0, 0,0,10,0,0};

    int tempMeshNext[50] = {2,0,1,1,10, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
                            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
    int tempMeshprev[50] = {0,0,2,1,1, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
                            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
    
    int tempCxtNext[50] = {0,0,0,0,0,0 ,0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0 ,0,0,0,0,
                            0,0,0,0,0,0 ,0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0 ,0,0,0,0};
    int tempCxtprev[50] = {0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0 ,0,0,0,0,
                            0,0,0,0,0,0 ,0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0 ,0,0,0,0};
    
    int tempListMeshesNext[50] = {1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,0,
                            0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
    int tempListMeshesprev[50] = {0,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1,
                            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0};
    

    int tempModelNext[50] = {4,0,0,0,4 ,0,0,0,4,0, 0,0,10,0,0, 0,0,0,0,0 ,0,0,1,1,0,
                            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};

    int tempModelprev[50] = {0,0,0,0,4 ,0,0,0,4,0, 0,0,4,0,0, 0,0,0,0,0, 0,0,10,1,1,
                            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
    
    debugger->currentOffset = 0;


    // dct_offsetDebugger_t offset_model;
    // offset_model.ident = DCT_MODEL;
    // memcpy(offset_model.offset_REFNEXTmemberModel[0] , temp, sizeof(offset_model.offset_REFNEXTmemberModel[0]) );
    // memcpy(offset_model.offset_REFPREVmemberModel[0] , tempprev, sizeof(offset_model.offset_REFPREVmemberModel[0]) );

    // dct_offsetDebugger_t offset_mesh;
    // offset_mesh.ident = DCT_MESH;
    // memcpy(offset_mesh.offset_REFNEXTmemberModel[0] , temp_nextMesh, sizeof(offset_mesh.offset_REFNEXTmemberModel[0]) );
    // memcpy(offset_mesh.offset_REFPREVmemberModel[0] , temp_nextMesh, sizeof(offset_mesh.offset_REFPREVmemberModel[0]) );

    
    // memcpy(&debugger->offsets[0] , &offset_model, sizeof(debugger->offsets[0]) );
    // memcpy(&debugger->offsets[1] , &offset_mesh, sizeof(debugger->offsets[1]) );
    

    // memcpy(debugger->offset_REFNEXTmemberModel, temp, sizeof(debugger->offset_REFNEXTmemberModel));
    // memcpy(debugger->offset_REFPREVmemberModel, tempprev, sizeof(debugger->offset_REFPREVmemberModel));


    for (int i=0; i<10; i++)
    {
        debugger->lastNavigationData[i] = 0;
    }
    for (int i=0; i<20; i++)
    {
        debugger->listMeshes.ident = 0;
        debugger->listMeshes.meshesBuffer[i] = 0;
    }
    // DCT_MODEL { {POSITIONX, 130},{POSITIONY, 130},{POSITIONZ, 130]   }
    debugger->fnDebug.ident = 0;
    debugger->fnDebug.fnDebugPtr = NULL;
    debugger->fnDebug.fnDebugPtrArg = NULL;

    // DEBUG OFFSET DEBUGGER 

    for (int i=0; i<10; i++)
    {
        for (int j=0; j<50; j++)
        {
            if(i==0)
            {
                debugger->offsets[i].ident = DCT_MODEL;
                debugger->offsets[i].offset_REFNEXTmemberModel[j] =  tempModelNext[j];
                debugger->offsets[i].offset_REFPREVmemberModel[j] =  tempModelprev[j];
            }

            if(i==1)
            {
                debugger->offsets[i].ident = DCT_MESH;
                debugger->offsets[i].offset_REFNEXTmemberModel[j] =  tempMeshNext[j];
                debugger->offsets[i].offset_REFPREVmemberModel[j] =  tempMeshprev[j];
            }

            if(i==2)
            {
                debugger->offsets[i].ident = DCT_CXT_T;
                debugger->offsets[i].offset_REFNEXTmemberModel[j] =  tempCxtNext[j];
                debugger->offsets[i].offset_REFPREVmemberModel[j] =  tempCxtprev[j];
            }
            if(i==3)
            {
                debugger->offsets[i].ident = DCT_LISTMESHESDEBUGGER_T;
                debugger->offsets[i].offset_REFNEXTmemberModel[j] =  tempListMeshesNext[j];
                debugger->offsets[i].offset_REFPREVmemberModel[j] =  tempListMeshesprev[j];
            }
            if(i>3)
            {
                debugger->offsets[i].ident = DCT_BASE_MASK;
                debugger->offsets[i].offset_REFNEXTmemberModel[j] =  -1;
                debugger->offsets[i].offset_REFPREVmemberModel[j] =  -1;
            }
            
        }
    }

    
    // for (int i=0; i<10; i++)
    // {
    //     for (int j=0; j<25; j++)
    //     {
    //         printf("offset ident %s \n",debugger->offsets[i].ident);
    //         printf("offsets %d -> offset_REFPREVmemberModel %d : %d \n",i,j,debugger->offsets[i].offset_REFPREVmemberModel[j]);
    //         printf("offsets %d -> offset_REFNEXTmemberModel %d : %d \n",i,j,debugger->offsets[i].offset_REFNEXTmemberModel[j]);
    //     }
    // }




    debugger->font = init_basic_font();
    if (!debugger->font) {
        printf("Font init failed\n");
        return;
    }

    char fill[4096];
    fill[0]='\0';
    for (int i=0;i<52;i++)
    {
        if(i==0)
        {
            strcat(fill,"TANTO ENGINE Tiny debugger                                     \n");
        }
        else
        {
            strcat(fill,"                                                                \n");
        }
        
    }
    debugger->textureDisplay = create_text_texture_with_colors( fill, debugger->font, 0xB03030FF,512,512);
    
    
    updateTinyDebuggerTexture(debugger);
}


bool isCurrentDataADTC_DATA_TYPE(char *checker)
{
    char validation[13] = "DCT_DATA_TYPE";
    for (int i=0; i<13; i++)
    {
        if(checker[i]!=validation[i] )
        {
            return false;
        }
    }

    return true;
}


void drawtinyDebugger(dct_tinyDebugger_t *debugger)
{
    pvr_list_begin(PVR_LIST_TR_POLY);
    draw_alpha_texture(debugger->textureDisplay,64,40);
    float x = 64;
    float y = 40;
    dct_textureAlpha* tex = debugger->textureDisplay;

    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;
    
    // Utiliser la taille totale de la texture pour le contexte
    pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444,
                     tex->width, tex->height, tex->texture, PVR_FILTER_NONE);
    
    cxt.gen.alpha = PVR_ALPHA_ENABLE;
    cxt.blend.src = PVR_BLEND_SRCALPHA;
    cxt.blend.dst = PVR_BLEND_INVSRCALPHA;
    cxt.blend.src_enable = PVR_BLEND_ENABLE;
    cxt.blend.dst_enable = PVR_BLEND_ENABLE;
    cxt.gen.culling = PVR_CULLING_NONE;
    cxt.depth.write = PVR_DEPTHWRITE_DISABLE;
    
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    // Calculer les UV en fonction du contenu réel
    float real_u = (float)tex->content_width / (float)tex->width;
    float real_v = (float)tex->content_height / (float)tex->height;
    
    vert.flags = PVR_CMD_VERTEX;
    vert.argb = 0xFFFFFFFF;
    vert.oargb = 0;
    vert.z = 9.0f;
    
    // Point en haut à gauche
    vert.x = x;
    vert.y = y;
    vert.u = 0.0f;
    vert.v = 0.0f;
    pvr_prim(&vert, sizeof(vert));
    
    // Point en haut à droite
    vert.x = x + tex->content_width;
    vert.y = y;
    vert.u = real_u;
    vert.v = 0.0f;
    pvr_prim(&vert, sizeof(vert));
    
    // Point en bas à gauche
    vert.x = x;
    vert.y = y + tex->content_height;
    vert.u = 0.0f;
    vert.v = real_v;
    pvr_prim(&vert, sizeof(vert));
    
    // Point en bas à droite (dernier vertex)
    vert.flags = PVR_CMD_VERTEX_EOL;
    vert.x = x + tex->content_width;
    vert.y = y + tex->content_height;
    vert.u = real_u;
    vert.v = real_v;
    pvr_prim(&vert, sizeof(vert));
    
    pvr_list_finish();
}



int main(void) {
    pvr_init_defaults();
    fs_romdisk_mount("/rd", romdisk, romdisk_end - romdisk);
    pvr_set_bg_color(0.2f, 0.2f, 0.2f);

    
    printf("\n\n START GAME \n\n");

    
    dct_camera_t *currentCam = getCurrentCamera();
    initDefaultCurrentCamera(currentCam);
    setCurrentCameraPosition(currentCam,0.0f,0.5f,-3.0f);

 
    dct_model_t penta;
    uint32_t c1[5] = {0xFFFFFFFF, 0xFF808080,0xFF00FF00,0xFF0000FF,0xFFFF0000};
    createPentahedre(&penta,"pentahedre",1.0f,c1);
   

    dct_model_t cube;
    uint32_t c2[6] = {0xFFFFFFFF, 0xFF808080,0xFF00FF00,0xFF0000FF,0xFFFF0000,0xFFFF00FF};
    createCube(&cube,"cube01",1.0f,c2);
    cube.type = MODEL_STD;
    cube.position.z = 3.1f;
    cube.rotation.x = 0.4f;
    cube.meshes[0].shadingContrast = 1.0f;
    cube.meshes[0].shadingColor = 0xFF200030;

    //load_dct_texture(&cube.textures[0],"/rd/Pave_stylized_128x128.kmg"); 
    //cube.meshes[0].currentCxtSelected = 0;

    // dct_model_t triangle;
    // uint32_t c3[3] = {0xFFFF0000, 0xFF00FF00,0xFF0000FF};
    // createPrimitiveTriangle(&triangle,"triangle",-1.0f, 0.0f, 1.0f, 1.0f, 0.0f,1.0f, 0.0f, -1.0f, -5.0f  , c3);
    


    // --  SCENE FOR DEBUGGER 
    initSceneSystem();
    dct_scene_t *sc;
    sc = getCurrentScene();
    //sc->modelSceneCount = 2;
    //sc->ptrListModelScene = (dct_model_t*)malloc(sizeof(dct_model_t*)*3);
    sc->ptrListModelScene[0] = &penta;
    sc->ptrListModelScene[1] = &cube;

    pvr_ptr_t texture;

    //texture = pvr_mem_malloc(256 * 256 *2);  // 2 bytes par pixel (ARGB4444)



    // -- DEBUGGER 
    // 
    //dct_menu_debugger_t *debugger=testDebug();
    //dct_menu_debugger_t *debugger = createMenuDebugger();


    printf("CHECK IDENT->DCT_MODEL CUBE %x \n", cube.ident);

    dct_tinyDebugger_t tinyDebugger;
    tinyDebugger.currentDataToNavigate = sc->ptrListModelScene[1];
    tinyDebugger.currentStruct         = sc->ptrListModelScene[1];
    createTextures(&tinyDebugger);

    



    float angleY = 0.0f;

    

    //dct_model_t *mod = sc->ptrListModelScene[0];


    dct_controllerState_t controller = {0};  // État du contrôleur

    
 
    while(1) {

        //pvr_set_bg_color(0.6f, 0.2f, 0.2f);

        dct_input_update(0, &controller);
        //---------------------------------//
        //----   MATRIX TRANSFORMATION ----//
        //---------------------------------//

        
        //updateDebugger(&controller);

        
        updateCurrentCamera(currentCam);
        

        updateModel(&cube); 
        updateModel(&penta); 

        pvr_wait_ready();
        pvr_scene_begin();

        renderModel(&cube);


        drawtinyDebugger(&tinyDebugger);
        
        pvr_scene_finish();


        

        //---------------------------//
        //---- UPDATE CONTROLLER ----//
        //---------------------------//

        if(controller.just_pressed.left)
        {
            for (int i=0; i<10;i++)
            {
                printf("\ntinyDebugger.lastNavigationData[%d]:%x \n",i,tinyDebugger.lastNavigationData[i] );
            }
            
            if( tinyDebugger.lastNavigationData[0]!=0 && tinyDebugger.isInsideDataStructure == false )
            {
                printf("\n\n[TOUCHE GAUCHE] MOVE HIERARCHY ↖ MONTE  \n");
                tinyDebugger.indiceCurrentMemberModel = 0;
                //tinyDebugger.currentDataToNavigate = tinyDebugger.lastNavigationData[0];
                tinyDebugger.currentDataToEdit = NULL;
                
                removeFirstNavigationData(&tinyDebugger);
                tinyDebugger.currentStruct         = tinyDebugger.currentDataToNavigate;
                updateTinyDebuggerTexture(&tinyDebugger);
                
            }
            else if(tinyDebugger.indiceCurrentMemberModel - 1 >= 0)
            {

                printf("\n\n[TOUCHE GAUCHE] MOVE STRUCTURE MEMBRE  PREV <- indiceCurrMemberModel :%d\n",tinyDebugger.indiceCurrentMemberModel);
                tinyDebugger.indiceCurrentMemberModel -=1;
                updateTinyDebuggerTexture(&tinyDebugger);
            }
            
        }   
        
        if(controller.just_pressed.right)
        {
            

            
            // uint32_t nextAddr = (uint32_t)(tinyDebugger.currentDataToNavigate)+tinyDebugger.indiceCurrentMemberModel-1;
            // printf("\n currentDataToNavigate : %x indiceCurrentMemberModel %x \n",tinyDebugger.currentDataToNavigate,tinyDebugger.indiceCurrentMemberModel);
            // printf("\n nextAddr : %x \n",tinyDebugger.currentDataToEdit );

            
            if( isNextCurrentDataToEditBasedOnIndiceCurrentMemberModelValid(tinyDebugger.currentDataToEdit) && tinyDebugger.currentDataToNavigate!=tinyDebugger.currentDataToEdit ) 
            {
                //listMemoryValues(tinyDebugger.currentDataToEdit);
                printf("\n\n[TOUCHE DROITE] MOVE HIERARCHY DESCEND ↘ \n");
                tinyDebugger.indiceCurrentMemberModel = 0;
                //tinyDebugger.lastNavigationData[0] = tinyDebugger.currentDataToNavigate;

                insertNavigationData(&tinyDebugger,tinyDebugger.currentDataToNavigate);
                tinyDebugger.currentDataToNavigate = tinyDebugger.currentDataToEdit;
                tinyDebugger.currentStruct         = tinyDebugger.currentDataToEdit;
                updateTinyDebuggerTexture(&tinyDebugger);
                
            }
            else
            {
                printf("\n\n[TOUCHE DROITE] MOVE STRUCTURE VOISINE NEXT ->  indiceCurrMemberModel :%d\n",tinyDebugger.indiceCurrentMemberModel);
                tinyDebugger.indiceCurrentMemberModel +=1;
                updateTinyDebuggerTexture(&tinyDebugger);

            }
            

        }

        if(controller.just_pressed.up)
        {
            if(tinyDebugger.offsets[tinyDebugger.currentOffset].offset_REFPREVmemberModel[tinyDebugger.indiceCurrentMemberModel] > 0) 
            {
                // Move Up to next member selection
                printf("\n\n[TOUCHE HAUT] MOVE STRUCTURE VOISINE PREV  ↑ indiceCurrMemberModel :%d \n",tinyDebugger.indiceCurrentMemberModel);
                //int testNext = tinyDebugger.indiceCurrentMemberModel - tinyDebugger.offset_REFPREVmemberModel[tinyDebugger.indiceCurrentMemberModel];
                int testNext = tinyDebugger.indiceCurrentMemberModel - tinyDebugger.offsets[tinyDebugger.currentOffset].offset_REFPREVmemberModel[tinyDebugger.indiceCurrentMemberModel];
                
                //printf("\n\n test Next %d \n\n",testNext);
                if(testNext>=0)
                {
                    //tinyDebugger.indiceCurrentMemberModel -= tinyDebugger.offset_REFPREVmemberModel[tinyDebugger.indiceCurrentMemberModel];
                    tinyDebugger.indiceCurrentMemberModel -= tinyDebugger.offsets[tinyDebugger.currentOffset].offset_REFPREVmemberModel[tinyDebugger.indiceCurrentMemberModel];
                }
            }
            else
            {
                if( isNextCurrentDataToEditBasedOnIndiceCurrentMemberModelValid(tinyDebugger.currentDataToEdit)==false)
                {
                    
                    printf("\n\n[TOUCHE HAUT] INCREMENTATION  VALUE ++ \n");

                    // edit the current value
                    if(tinyDebugger.currentDataToEdit_IntOrFloat==0)
                    {
                        int *data = (int *)tinyDebugger.currentDataToEdit;
                        (*data) += 1;
                    }
                    if(tinyDebugger.currentDataToEdit_IntOrFloat==1)
                    {
                        float *data = (float *)tinyDebugger.currentDataToEdit;
                        (*data) += 0.1f;
                    }

                   
                    

                }
                else
                {
                    printf("\n\n isNextCurrentDataToEditBasedOnIndiceCurrentMemberModelValid !! \n");
                    DCT_IDENT id = *(DCT_IDENT*)tinyDebugger.currentDataToEdit; 
                    if(id == DCT_FN_PRINT_MODEL)
                    {
                        printf("\nCurrentDataToEdit => FN PRINT MODEL \n");
                        dct_fnDebug_t data = *(dct_fnDebug_t*)tinyDebugger.currentDataToEdit; 
                        debug_print_model(data.fnDebugPtrArg);
                    }
                }                

            }
                

            
            updateTinyDebuggerTexture(&tinyDebugger);

            // if(tinyDebugger.currentDataToEdit!=NULL)
            //     {
            //         printf("increment currendataToEdit \n");
            //         if(tinyDebugger.currentDataToEdit_IntOrFloat==0)
            //         {
            //             int *data = (int *)tinyDebugger.currentDataToEdit;
            //             (*data) += 1;
            //         }
            //         if(tinyDebugger.currentDataToEdit_IntOrFloat==1)
            //         {
            //             float *data = (float *)tinyDebugger.currentDataToEdit;
            //             (*data) += 0.1f;
            //         }
            //         updateTinyDebuggerTexture(&tinyDebugger);

            //     }
            
            
        }

        if(controller.just_pressed.down )
        {
            
            if(tinyDebugger.offsets[tinyDebugger.currentOffset].offset_REFNEXTmemberModel[tinyDebugger.indiceCurrentMemberModel] > 0) 
            {
                printf("\n\n[TOUCHE BAS] MOVE STRUCTURE VOISINE PREV  ↓ indiceCurrMemberModel :%d \n",tinyDebugger.indiceCurrentMemberModel);

                //tinyDebugger.indiceCurrentMemberModel += tinyDebugger.offset_REFNEXTmemberModel[tinyDebugger.indiceCurrentMemberModel];
                tinyDebugger.indiceCurrentMemberModel += tinyDebugger.offsets[tinyDebugger.currentOffset].offset_REFNEXTmemberModel[tinyDebugger.indiceCurrentMemberModel];

                
                //tinyDebugger.indiceCurrentMemberModel += 1;
            }
            else
            {
                
                if( isNextCurrentDataToEditBasedOnIndiceCurrentMemberModelValid(tinyDebugger.currentDataToEdit)==false)
                {
                    
                    printf("\n\n[TOUCHE BAS] INCREMENTATION VALUE -- \n");
                    if(tinyDebugger.currentDataToEdit_IntOrFloat==0)
                    {
                        int *data = (int *)tinyDebugger.currentDataToEdit;
                        (*data) -= 1;
                    }
                    if(tinyDebugger.currentDataToEdit_IntOrFloat==1)
                    {
                        float *data = (float *)tinyDebugger.currentDataToEdit;
                        (*data) -= 0.1f;
                    }

                }
                

            }

            printf("\n\n currentOffset : %d  indiceCurrMemberModel :%d \n\n",tinyDebugger.currentOffset,tinyDebugger.indiceCurrentMemberModel);

            updateTinyDebuggerTexture(&tinyDebugger);

            /*
            if(tinyDebugger.currentDataToEdit!=NULL)
            {
                printf("incrementation \n");
                if(tinyDebugger.currentDataToEdit_IntOrFloat==0)
                {
                    
                    int *data = (int *)tinyDebugger.currentDataToEdit;
                    (*data) -= 1;
                }
                if(tinyDebugger.currentDataToEdit_IntOrFloat==1)
                {
                    float *data = (float *)tinyDebugger.currentDataToEdit;
                    (*data) -= 0.1f;
                }

                
                updateTinyDebuggerTexture(&tinyDebugger);
            }
            */
            
        }

        if(controller.just_pressed.a)
        {
            //triangle.DEBUG_MODEL_VTX = true;
            //cube.DEBUG_MODEL_VTX = true;
            //debugCurrentCamera(currentCam);
            //debug_print_model(&cube);

            debugTinyDebugger(&tinyDebugger);
        }else
        {
            //triangle.DEBUG_MODEL_VTX = false;
            //cube.DEBUG_MODEL_VTX = false;

        }

      
        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        

        
        if(st->buttons & CONT_DPAD_RIGHT )
        {
            //currentCam->position.y += 0.1f;
            cube.rotation.z += 0.1f;
            // triangle.position.z += 0.1f;
        }
        if(st->buttons & CONT_DPAD_LEFT )
        {
            //currentCam->position.y -= 0.1f;
            cube.rotation.z -= 0.1f;
            // triangle.position.z -= 0.1f;
        }


            
        MAPLE_FOREACH_END()
    }


    return 0;
}