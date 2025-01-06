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
    DCT_DBG_MODEL_MESH_CXT

} DCT_DEBUG_DATA_TYPE;

typedef enum 
{
    DCT_CAMERA,
    DCT_INPUT,
    DCT_MESH,
    DCT_MODEL,
    PVR_VERTEX_T,
    PVR_POLY_HDR_T,
    PVR_POLY_CTX_T

} DCT_TYPE;



typedef struct 
{
    DCT_TYPE type;
    void     *data;
} GameObject;


#define IS_TYPE(obj,type) ( (obj)->type == type )

#endif