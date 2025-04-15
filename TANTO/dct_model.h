#ifndef DCT_MODEL_H
#define DCT_MODEL_H

#include <kos.h>
#include <kmg/kmg.h>
#include <math.h>
#include "dct_utils.h"
#include "dct_camera.h"
#include "dct_types.h"
#include "dct_texture.h"
#include "dct_skinning.h"


typedef enum 
{
    MODIFIER_STATE_NONE,
    MODIFIER_STATE_INACTIF,
    MODIFIER_STATE_ACTIF
} MESH_MODIFIER_STATE;



typedef struct dct_mesh_modifier_vol 
{
    MESH_MODIFIER_STATE state;
    char               name[32];     

    int                triCount;
    pvr_modifier_vol_t *modifierTrianglesVolOriginal;
    pvr_modifier_vol_t *modifierTrianglesVolFinal;


    matrix_t           transform;


} dct_mesh_modifier_vol_t;

typedef struct dct_mesh 
{
    DCT_IDENT          ident;
    DCT_MESH_TYPE      type;
    bool               shading; // ON OFF calcul ou pas les vertex lighting
    int                indice;
    char               name[32];     

    int                vtxCount;
    int                triCount;
    int                sizeOfVtx;      // sizeof(originalVtx) pour optimiser sq_cpy et ne pas avoir à le calculer à chaque frame
    pvr_vertex_t       *originalVtx;   // les vertex en "bindPose"
    pvr_vertex_t       *animatedVtx;   // les vertex animés par le squelette
    pvr_vertex_t       *renderVtx;     // les vertex transformés par mat_trans_single3 afin d'eviter une double transformation constante car mat_trans_single incrémente les vertex en input   

    pvr_vertex_pcm_t   *renderVtx_PCM;  // pour les masques de volumes filtrage des couleurs  meme valeur que render vtx mais pas d'uv et argb0 argb1 
    pvr_vertex_tpcm_t  *renderVtx_TPCM; // pour les masques de volumes filtrage des textures avec couleurs 2 argb oargb et 2 uv

    dct_vertexWeight_t *vertexWeights;   // weight and bone indice for each vertice
    vec3f_t            *vertexNormals;     // Normales par vertex
    float              *smoothingWeights;   // Poid
    float              shadingContrast;    // Contrôle du contraste (par exemple 0.5 à 2.0)
    uint32_t           shadingColor;      // Couleur des ombres en ARGB
    
    pvr_vertex_t       *clippingTrianglesVtx;
    int                clippingTrianglesCount;
    int                currentClippingTrianglesCount; // 1 ou 2 triangle a dessiné en plus pour chaque triangle

    int                indiceOfCurrentTextureMapped;

    
    pvr_poly_hdr_t     hdr; 
    pvr_poly_cxt_t     cxt;

    dct_cxt_t          cxtMesh;

    int                currentCxtSelected; // exemple : -1 => mesh cxt ,  0=>model textures[0] ,  1=>model textures[1] 

} dct_mesh_t;




typedef struct dct_model
{
    DCT_IDENT            ident;
    DCT_MODEL_TYPE       type;
    char                 name[64];

    int                  meshesCount;
    dct_mesh_t           *meshes;

    dct_mesh_modifier_vol_t modifier;

    
    dct_texture_t        *textures;
    int                  texturesCount;
    
    vec3f_t              position;
    vec3f_t              rotation;
    vec3f_t              scale;

    dct_armature_t       *armature;
    dct_animation_t      *animations;
    uint32_t              anim_count;
    dct_animation_state_t anim_state;

    bool                  DEBUG_MODEL_VTX;
    bool                  DEBUG_RENDER_VTX;

} dct_model_t;



pvr_vertex_t* createVerticesCone(float radiusTop, float radiusBottom, float height, int segments, uint32_t* color, int* vertexCount);
dct_mesh_modifier_vol_t createModifierVolumeCone();
dct_mesh_modifier_vol_t createModifierVolumeCube();
dct_model_t* load_pvr(const char *filename);
void debug_print_model(dct_model_t* model);
void initModel( dct_model_t *model);
void setMeshHeader(dct_mesh_t *mesh, int SETTING);
void setModelMeshPCM(dct_mesh_t *mesh, uint32_t color);
void initMeshesHeader(dct_model_t *model);
void setVertexColorToAllMesh(dct_mesh_t *mesh, uint32_t col);
void setMeshTexture(dct_model_t *model, int indiceMeshTexture,const char *fileTextureName);
void displayModelData(dct_model_t *model);
void updateModel( dct_model_t *model );
void updateModel_optimize(dct_model_t *model);
int  clipTriangle_optimize(pvr_vertex_t* input[3], pvr_vertex_t* output, float nearZ, dct_model_t* model, dct_mesh_t* mesh) ;
void calculateVertexLighting_optimized(dct_mesh_t* mesh, vec3f_t lightDir);

void renderMeshENV(dct_model_t *model, int i_mesh);
void renderMeshSTD(dct_model_t *model,int meshIndice);
void renderMeshSTD_optimize(dct_model_t *model, int i_mesh);
void renderMeshPCM(dct_model_t *model,int meshIndice);
void renderMeshTPCM(dct_model_t *model, int i_mesh);
void renderModel(dct_model_t *model);
void freeModel(dct_model_t *model);
void applyModifier(dct_mesh_modifier_vol_t *mod);
void debugRenderModifier(dct_model_t *model, int switchPvrList);
void debug_print_polyCxt(pvr_poly_cxt_t *cxt);
void debugCurrentModelActivatedCxt(dct_model_t *model);


void dct_deform_model(dct_model_t* model);
void dct_deform_mesh(dct_mesh_t* mesh, dct_armature_t* armature);
void dct_armature_bones_update(dct_bone_t *bone);
void dct_armature_update(dct_model_t *model);
void dct_animation_update(dct_animation_state_t* state, float delta_time, dct_model_t* model);
void dct_applyParentBonesArmatureMatrix(dct_bone_t *bone);
void dct_draw_armature(dct_armature_t* armature);
void dct_draw_gizmo();

void testInternalMatrix();

void matrix_multiply(matrix_t result, matrix_t a, matrix_t b);

void createPrimitiveTriangle(dct_model_t *triangle, const char *name, float x1, float y1, float z1, float x2, float y2, float z2 ,float x3, float y3, float z3, uint32_t color[3]);
void createPrimitiveRect(dct_model_t *rect, const char *name,float x, float y, float width, float height,uint32_t color);
void createPrimitiveLine(dct_model_t *line, const char *name,float x1, float y1, float x2, float y2,float thickness, uint32_t color);
void createPrimitiveCircle(dct_model_t *circle, const char *name,float posX, float posY, float radius,int segments, uint32_t color);
void createPrimitivePolygon(dct_model_t *polygon, const char *name, float *points, int numPoints, uint32_t color);
void createPrimitivePath(dct_model_t *path, const char *name, float *points, int numPoints, float thickness, uint32_t color);
void createPlane(dct_model_t *plane, const char *name, float size, uint32_t color);
void createCube(dct_model_t *cube, const char *name, float size, uint32_t color[6]);
void createPentahedre(dct_model_t *penta, const char *name, float size, uint32_t colors[5]);
void createSphere(dct_model_t *sphere, const char *name ,float size, uint32_t color);

#endif