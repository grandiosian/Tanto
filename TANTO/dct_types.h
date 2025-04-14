#ifndef DCT_TYPES_H
#define DCT_TYPES_H


typedef enum
{
    NONE,
    DCT_DBG_MENU_PRINCIPAL_NAME,

    
    DCT_DBG_MODEL_ROOT,
    DCT_DBG_MODEL,
    DCT_DBG_MODEL_NAME,
    DCT_DBG_MODEL_POSITION,
    DCT_DBG_MODEL_POSITION_X,
    DCT_DBG_MODEL_POSITION_Y,
    DCT_DBG_MODEL_POSITION_Z,
    DCT_DBG_MODEL_ROTATION,
    DCT_DBG_MODEL_ROTATION_X,
    DCT_DBG_MODEL_ROTATION_Y,
    DCT_DBG_MODEL_ROTATION_Z,
    DCT_DBG_MODEL_SCALE,
    DCT_DBG_MODEL_SCALE_X,
    DCT_DBG_MODEL_SCALE_Y,
    DCT_DBG_MODEL_SCALE_Z,

    DCT_DBG_MODEL_POSROTSC,
    DCT_DBG_MODEL_TRANSFORM,
    DCT_DBG_MODEL_MESHES,
    DCT_DBG_MODEL_MESH_NAME,
    DCT_DBG_MODEL_MESH_INDICE,
    DCT_DBG_MODEL_MESH_ORIGINALVTX,
    DCT_DBG_MODEL_MESH_ANIMATEDVTX,
    DCT_DBG_MODEL_MESH_RENDERVTX,
    DCT_DBG_MODEL_MESH_ORIGINALVTX_VTX,
    DCT_DBG_MODEL_MESH_ANIMATEDVTX_VTX,
    DCT_DBG_MODEL_MESH_RENDERVTX_VTX,
    DCT_DBG_MODEL_MESH_HDR,
    DCT_DBG_MODEL_MESH_CXT,

    

} DCT_DEBUG_DATA_TYPE;


typedef enum 
{
    // Masque de base pour les 16 bits de poids fort (0xCAFE comme exemple distinctif)
    DCT_BASE_MASK = 0xCAFE0000,  // 1100 1010 1111 1110 0000 0000 0000 0000

    // Types de base (0x0000 - 0x000F)
    INT             = DCT_BASE_MASK | 0x0001,    // 0xCAFE0001
    UINT32_T        = DCT_BASE_MASK | 0x0002,    // 0xCAFE0002
    CHAR            = DCT_BASE_MASK | 0x0003,    // 0xCAFE0003
    FLOAT           = DCT_BASE_MASK | 0x0004,    // 0xCAFE0004
    
    // Types composés (0x0010 - 0x001F)
    BOOL            = DCT_BASE_MASK | 0x0010,    // 0xCAFE0010
    VEC3F_T         = DCT_BASE_MASK | 0x0011,    // 0xCAFE0011
    MATRIX_T        = DCT_BASE_MASK | 0x0012,    // 0xCAFE0012
    
    // Types DCT spécifiques (0x0020 - 0x002F)
    DCT_VERTEXWEIGHT_T      = DCT_BASE_MASK | 0x0020,    // 0xCAFE0020
    DCT_ARMATURE_T          = DCT_BASE_MASK | 0x0021,    // 0xCAFE0021
    DCT_ANIMATION_T         = DCT_BASE_MASK | 0x0022,    // 0xCAFE0022
    DCT_ANIMATION_STATE_T   = DCT_BASE_MASK | 0x0023,    // 0xCAFE0023
    
    // Composants du moteur (0x0030 - 0x003F)
    DCT_CAMERA              = DCT_BASE_MASK | 0x0030,    // 0xCAFE0030
    DCT_INPUT               = DCT_BASE_MASK | 0x0031,    // 0xCAFE0031
    DCT_MESH                = DCT_BASE_MASK | 0x0032,    // 0xCAFE0032
    DCT_MESH_MODIFIER_VOL_T = DCT_BASE_MASK | 0x0033,    // 0xCAFE0033
    DCT_TEXTURE_T           = DCT_BASE_MASK | 0x0034,    // 0xCAFE0034
    
    // Types PVR (0x0040 - 0x004F)
    DCT_MODEL               = DCT_BASE_MASK | 0x0040,    // 0xCAFE0040
    PVR_VERTEX_T            = DCT_BASE_MASK | 0x0041,    // 0xCAFE0041
    PVR_VERTEX_PCM_T        = DCT_BASE_MASK | 0x0042,    // 0xCAFE0042
    PVR_VERTEX_TPCM_T       = DCT_BASE_MASK | 0x0043,    // 0xCAFE0043
    
    PVR_POLY_HDR_T          = DCT_BASE_MASK | 0x0050,    // 0xCAFE0050
    PVR_POLY_CXT_T          = DCT_BASE_MASK | 0x0051,    // 0xCAFE0051
    DCT_CXT_T               = DCT_BASE_MASK | 0x0052,
    DCT_LISTMESHESDEBUGGER_T= DCT_BASE_MASK | 0x0053,
    DCT_FN_PRINT_MODEL      = DCT_BASE_MASK | 0x0054,

} DCT_IDENT;



typedef enum
{
        TXT_NONE,
        TXT_STD,
        TXT_TPCM,
        TXT_ENV,
        TXT_BUMP
        
} DCT_TEXTURE_TYPE;


typedef enum
{
    MESH_STD_OP,
    MESH_STD_OP_TXR,
    MESH_STD_TR,
    MESH_STD_TR_TXR,
    MESH_PCM_OP,
    MESH_PCM_TR,
    MESH_TPCM_OP,
    MESH_TPCM_TR
    
} DCT_MESH_TYPE;


typedef enum
{
    MODEL_STD,
    MODEL_PCM,
    MODEL_TPCM
} DCT_MODEL_TYPE;


typedef struct 
{
    DCT_IDENT type;
    void     *data;
} GameObject;



#define IS_VALID_DCT_IDENT_PTR(id)   ((*(id) & 0xFFFF0000) == DCT_BASE_MASK)
#define IS_VALID_DCT_IDENT_VAL(id)   (((uint32_t)(id) & 0xFFFF0000) == DCT_BASE_MASK)

#define IS_TYPE(obj,type) ( (obj)->type == type )

#endif